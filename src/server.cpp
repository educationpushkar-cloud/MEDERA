// ---------------------------------------------------------------------------
// Phase II web server — full implementation.
//
// Concurrency: all shared state (hospitals, doctors, patients, appointments,
// ratings, emergencies, indexes) is guarded by a single recursive mutex.
// This is coarse but correct. Reads and writes are both serialized; the
// throughput is more than enough for a demo and for a small hospital's
// traffic. A finer-grained scheme (separate mutexes per entity) is a
// Phase III optimization.
//
// Error handling: a top-level exception handler catches anything a route
// handler throws and returns a 500 JSON response. The server never crashes
// because one request was malformed.
//
// Route patterns that need a parameter use ordinary C++ string syntax with
// escaped backslashes ("/api/emergency/(\\d+)") rather than raw string
// literals (R"(...)"): a regex that ends with ')' followed by the raw-string
// terminator ')"' would terminate the literal one character too early.
// ---------------------------------------------------------------------------

#include <thread>
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <memory>
#include <algorithm>

#include "httplib.h"

#include "models.h"
#include "storage.h"
#include "hashindex.h"
#include "ranking.h"
#include "emergency_fsm.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <string>

// ---------------------------------------------------------------------------
// Staff record — kept local to the server because it is an auth concern,
// not a domain entity the rest of the app needs to know about.
// ---------------------------------------------------------------------------
struct Staff
{
    int id;
    std::string name;
    std::string staff_id;
    std::string phone;
    std::string password;
    int hospital_id;
};

// ---------------------------------------------------------------------------
// In-memory state. All guarded by g_mutex.
// ---------------------------------------------------------------------------
static std::vector<Hospital>         g_hospitals;
static std::vector<Doctor>           g_doctors;
static std::vector<Patient>          g_patients;
static std::vector<Appointment>      g_appointments;
static std::vector<Rating>           g_ratings;
static std::vector<EmergencyRequest> g_emergencies;
static std::vector<Staff>            g_staff;
static HospitalIndex                 g_index;

static std::recursive_mutex g_mutex;

// File paths
static const std::string F_HOSPITALS    = "data/hospitals.txt";
static const std::string F_DOCTORS      = "data/doctors.txt";
static const std::string F_PATIENTS     = "data/patients.txt";
static const std::string F_APPOINTMENTS = "data/appointments.txt";
static const std::string F_RATINGS      = "data/ratings.txt";
static const std::string F_EMERGENCIES  = "data/emergencies.txt";
static const std::string F_STAFF        = "data/staff.txt";

// ---------------------------------------------------------------------------
// Staff loader — same pipe-delimited format as the storage module.
// ---------------------------------------------------------------------------
static std::vector<Staff> loadStaff(const std::string &path)
{
    std::vector<Staff> result;
    std::ifstream in(path);
    if (!in.is_open()) return result;

    std::string line;
    while (std::getline(in, line))
    {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::vector<std::string> f;
        std::string field;
        while (std::getline(ss, field, '|')) f.push_back(field);
        if (f.size() < 6) continue;

        try
        {
            Staff s;
            s.id          = std::stoi(f[0]);
            s.name        = f[1];
            s.staff_id    = f[2];
            s.phone       = f[3];
            s.password    = f[4];
            s.hospital_id = std::stoi(f[5]);
            result.push_back(s);
        }
        catch (...) { continue; }
    }
    return result;
}

// ---------------------------------------------------------------------------
// Next-ID helpers
// ---------------------------------------------------------------------------
static int nextAppointmentId()
{
    int maxId = 0;
    for (const auto &a : g_appointments) maxId = std::max(maxId, a.id);
    return maxId + 1;
}
static int nextRatingId()
{
    int maxId = 0;
    for (const auto &r : g_ratings) maxId = std::max(maxId, r.id);
    return maxId + 1;
}
static int nextEmergencyId()
{
    int maxId = 0;
    for (const auto &e : g_emergencies) maxId = std::max(maxId, e.id);
    return maxId + 1;
}

// ---------------------------------------------------------------------------
// JSON serializers
// ---------------------------------------------------------------------------
static std::string hospitalToJson(const Hospital &h)
{
    std::ostringstream j;
    j << "{"
      << "\"id\":" << h.id << ","
      << "\"name\":\"" << h.name << "\","
      << "\"city\":\"" << h.city << "\","
      << "\"area\":\"" << h.area << "\","
      << "\"latitude\":" << h.latitude << ","
      << "\"longitude\":" << h.longitude << ","
      << "\"available_beds\":" << h.available_beds << ","
      << "\"total_beds\":" << h.total_beds << ","
      << "\"ambulance_count\":" << h.ambulance_count << ","
      << "\"rating\":" << h.rating
      << "}";
    return j.str();
}

static std::string doctorToJson(const Doctor &d)
{
    std::ostringstream j;
    j << "{"
      << "\"id\":" << d.id << ","
      << "\"hospital_id\":" << d.hospital_id << ","
      << "\"name\":\"" << d.name << "\","
      << "\"specialization\":\"" << d.specialization << "\""
      << "}";
    return j.str();
}

static std::string appointmentToJson(const Appointment &a)
{
    std::ostringstream j;
    j << "{"
      << "\"id\":" << a.id << ","
      << "\"patient_id\":" << a.patient_id << ","
      << "\"hospital_id\":" << a.hospital_id << ","
      << "\"doctor_id\":" << a.doctor_id << ","
      << "\"status\":\"" << appointmentStatusToString(a.status) << "\""
      << "}";
    return j.str();
}

static std::string ratingToJson(const Rating &r)
{
    std::ostringstream j;
    j << "{"
      << "\"id\":" << r.id << ","
      << "\"appointment_id\":" << r.appointment_id << ","
      << "\"score\":" << r.score
      << "}";
    return j.str();
}

static std::string emergencyToJson(const EmergencyRequest &e)
{
    std::ostringstream j;
    j << "{"
      << "\"id\":" << e.id << ","
      << "\"patient_id\":" << e.patient_id << ","
      << "\"hospital_id\":" << e.hospital_id << ","
      << "\"patient_lat\":" << e.patient_lat << ","
      << "\"patient_lon\":" << e.patient_lon << ","
      << "\"state\":\"" << emergencyStateToString(e.state) << "\""
      << "}";
    return j.str();
}

static std::string rankedHospitalToJson(const RankedHospital &rh)
{
    std::ostringstream j;
    j << "{"
      << "\"hospital\":" << hospitalToJson(rh.hospital) << ","
      << "\"distance_km\":" << rh.distanceKm << ","
      << "\"score\":" << rh.score
      << "}";
    return j.str();
}

template <typename T, typename Fn>
static std::string toJsonArray(const std::vector<T> &v, Fn fn)
{
    std::ostringstream body;
    body << "[";
    for (size_t i = 0; i < v.size(); i++)
    {
        if (i > 0) body << ",";
        body << fn(v[i]);
    }
    body << "]";
    return body.str();
}

static void jsonError(httplib::Response &res, int status, const std::string &msg)
{
    res.status = status;
    res.set_content("{\"error\":\"" + msg + "\"}", "application/json");
}

static void jsonOk(httplib::Response &res,
                   const std::string &body = "{\"status\":\"ok\"}")
{
    res.set_content(body, "application/json");
}

// ---------------------------------------------------------------------------
// Request helpers
// ---------------------------------------------------------------------------
static bool getIntParam(const httplib::Request &req, const std::string &name, int &out)
{
    if (!req.has_param(name)) return false;
    try { out = std::stoi(req.get_param_value(name)); return true; }
    catch (...) { return false; }
}
static bool getDoubleParam(const httplib::Request &req, const std::string &name, double &out)
{
    if (!req.has_param(name)) return false;
    try { out = std::stod(req.get_param_value(name)); return true; }
    catch (...) { return false; }
}
static bool getStringParam(const httplib::Request &req, const std::string &name, std::string &out)
{
    if (!req.has_param(name)) return false;
    out = req.get_param_value(name);
    return !out.empty();
}

// ---------------------------------------------------------------------------
// Load everything from disk at startup.
// ---------------------------------------------------------------------------
static void loadAllState()
{
    std::lock_guard<std::recursive_mutex> lock(g_mutex);
    g_hospitals    = loadHospitals(F_HOSPITALS);
    g_doctors      = loadDoctors(F_DOCTORS);
    g_patients     = loadPatients(F_PATIENTS);
    g_appointments = loadAppointments(F_APPOINTMENTS);
    g_ratings      = loadRatings(F_RATINGS);
    g_emergencies  = loadEmergencies(F_EMERGENCIES);
    g_staff        = loadStaff(F_STAFF);
    g_index        = buildHospitalIndex(g_hospitals);
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main()
{
    loadAllState();

    std::cout << "Loaded: "
              << g_hospitals.size()    << " hospitals, "
              << g_doctors.size()      << " doctors, "
              << g_patients.size()     << " patients, "
              << g_appointments.size() << " appointments, "
              << g_ratings.size()      << " ratings, "
              << g_emergencies.size()  << " emergency requests, "
              << g_staff.size()        << " staff.\n";

    httplib::Server svr;

    // Every handler runs under this exception guard. A malformed request can
    // throw (std::stoi on garbage, etc.); we turn that into a 500 rather than
    // letting it kill the worker thread.
    svr.set_exception_handler([](const httplib::Request &, httplib::Response &res,
                                 std::exception_ptr ep)
    {
        std::string what = "unknown";
        try { std::rethrow_exception(ep); }
        catch (const std::exception &e) { what = e.what(); }
        catch (...) {}
        std::cerr << "Handler exception: " << what << "\n";
        res.status = 500;
        res.set_content("{\"error\":\"internal server error\"}", "application/json");
    });

    // --- Static frontend -------------------------------------------------
    svr.set_mount_point("/", "./web");

    // =====================================================================
    // HOSPITALS
    // =====================================================================

    svr.Get("/api/hospitals", [](const httplib::Request &req, httplib::Response &res)
    {
        std::string city = req.get_param_value("city");
        std::string area = req.get_param_value("area");
        std::lock_guard<std::recursive_mutex> lock(g_mutex);
        auto results = lookupHospitals(g_index, city, area);
        jsonOk(res, toJsonArray(results, hospitalToJson));
    });

    svr.Get("/api/hospitals/all", [](const httplib::Request &, httplib::Response &res)
    {
        std::lock_guard<std::recursive_mutex> lock(g_mutex);
        jsonOk(res, toJsonArray(g_hospitals, hospitalToJson));
    });

    // =====================================================================
    // STAFF AUTH + BED UPDATE
    // =====================================================================

    svr.Post("/api/staff/login", [](const httplib::Request &req, httplib::Response &res)
    {
        std::string staffId, password;
        if (!getStringParam(req, "staff_id", staffId) ||
            !getStringParam(req, "password", password))
        {
            return jsonError(res, 400, "staff_id and password required");
        }

        std::lock_guard<std::recursive_mutex> lock(g_mutex);
        for (const auto &s : g_staff)
        {
            if (s.staff_id == staffId && s.password == password)
            {
                std::ostringstream body;
                body << "{\"status\":\"ok\","
                     << "\"staff\":{\"id\":" << s.id
                     << ",\"name\":\"" << s.name << "\""
                     << ",\"hospital_id\":" << s.hospital_id << "}}";
                return jsonOk(res, body.str());
            }
        }
        jsonError(res, 401, "invalid credentials");
    });

    svr.Post("/api/hospital/update-beds", [](const httplib::Request &req, httplib::Response &res)
    {
        int id, newAvailable, staffId;
        if (!getIntParam(req, "id", id) ||
            !getIntParam(req, "available_beds", newAvailable) ||
            !getIntParam(req, "staff_id", staffId))
        {
            return jsonError(res, 400, "id, available_beds, staff_id required");
        }
        if (newAvailable < 0)
            return jsonError(res, 400, "available_beds cannot be negative");

        std::lock_guard<std::recursive_mutex> lock(g_mutex);

        // Verify the staff member is authorized for this hospital.
        Staff *staff = nullptr;
        for (auto &s : g_staff) if (s.id == staffId) { staff = &s; break; }
        if (!staff) return jsonError(res, 401, "unknown staff_id");
        if (staff->hospital_id != id)
            return jsonError(res, 403, "staff not authorized for this hospital");

        // Find the hospital and update it.
        Hospital *h = nullptr;
        for (auto &x : g_hospitals) if (x.id == id) { h = &x; break; }
        if (!h) return jsonError(res, 404, "hospital not found");

        if (newAvailable > h->total_beds)
            return jsonError(res, 400, "available_beds exceeds total_beds");

        h->available_beds = newAvailable;
        saveHospitals(F_HOSPITALS, g_hospitals);
        g_index = buildHospitalIndex(g_hospitals);
        jsonOk(res, "{\"status\":\"ok\",\"hospital_id\":" + std::to_string(id) + "}");
    });

    // =====================================================================
    // DOCTORS
    // =====================================================================

    svr.Get("/api/doctors", [](const httplib::Request &req, httplib::Response &res)
    {
        int hospitalId;
        if (!getIntParam(req, "hospital_id", hospitalId))
            return jsonError(res, 400, "hospital_id required");

        std::lock_guard<std::recursive_mutex> lock(g_mutex);
        std::vector<Doctor> out;
        for (const auto &d : g_doctors)
            if (d.hospital_id == hospitalId) out.push_back(d);
        jsonOk(res, toJsonArray(out, doctorToJson));
    });

    // =====================================================================
    // APPOINTMENTS
    // =====================================================================

    svr.Post("/api/appointments", [](const httplib::Request &req, httplib::Response &res)
    {
        int patientId, hospitalId, doctorId;
        if (!getIntParam(req, "patient_id", patientId) ||
            !getIntParam(req, "hospital_id", hospitalId) ||
            !getIntParam(req, "doctor_id", doctorId))
        {
            return jsonError(res, 400, "patient_id, hospital_id, doctor_id required");
        }

        std::lock_guard<std::recursive_mutex> lock(g_mutex);

        bool patientOk = false, hospitalOk = false, doctorOk = false;
        for (const auto &p : g_patients)  if (p.id == patientId)  patientOk  = true;
        for (const auto &h : g_hospitals) if (h.id == hospitalId) hospitalOk = true;
        for (const auto &d : g_doctors)
            if (d.id == doctorId && d.hospital_id == hospitalId) doctorOk = true;

        if (!patientOk)  return jsonError(res, 404, "patient not found");
        if (!hospitalOk) return jsonError(res, 404, "hospital not found");
        if (!doctorOk)   return jsonError(res, 404, "doctor not found at this hospital");

        Appointment a;
        a.id          = nextAppointmentId();
        a.patient_id  = patientId;
        a.hospital_id = hospitalId;
        a.doctor_id   = doctorId;
        a.status      = AppointmentStatus::Requested;

        g_appointments.push_back(a);
        saveAppointments(F_APPOINTMENTS, g_appointments);
        jsonOk(res, appointmentToJson(a));
    });

    svr.Get("/api/appointments", [](const httplib::Request &req, httplib::Response &res)
    {
        int patientId;
        if (!getIntParam(req, "patient_id", patientId))
            return jsonError(res, 400, "patient_id required");

        std::lock_guard<std::recursive_mutex> lock(g_mutex);
        std::vector<Appointment> out;
        for (const auto &a : g_appointments)
            if (a.patient_id == patientId) out.push_back(a);
        jsonOk(res, toJsonArray(out, appointmentToJson));
    });

    // Mark an appointment Completed. Only used for the demo — in a real
    // system this would be triggered by hospital staff after the visit.
    svr.Post("/api/appointments/(\\d+)/complete",
             [](const httplib::Request &req, httplib::Response &res)
    {
        int id = std::stoi(req.matches[1]);

        std::lock_guard<std::recursive_mutex> lock(g_mutex);
        for (auto &a : g_appointments)
        {
            if (a.id == id)
            {
                if (a.status != AppointmentStatus::Confirmed &&
                    a.status != AppointmentStatus::Requested)
                {
                    return jsonError(res, 400, "appointment is not in a completable state");
                }
                a.status = AppointmentStatus::Completed;
                saveAppointments(F_APPOINTMENTS, g_appointments);
                return jsonOk(res, appointmentToJson(a));
            }
        }
        jsonError(res, 404, "appointment not found");
    });

    // =====================================================================
    // RATINGS
    // =====================================================================

    svr.Post("/api/ratings", [](const httplib::Request &req, httplib::Response &res)
    {
        int appointmentId, score;
        if (!getIntParam(req, "appointment_id", appointmentId) ||
            !getIntParam(req, "score", score))
        {
            return jsonError(res, 400, "appointment_id and score required");
        }
        if (score < 1 || score > 5)
            return jsonError(res, 400, "score must be between 1 and 5");

        std::lock_guard<std::recursive_mutex> lock(g_mutex);

        // Rating is post-visit only: the appointment must be Completed.
        Appointment *appt = nullptr;
        for (auto &a : g_appointments) if (a.id == appointmentId) { appt = &a; break; }
        if (!appt) return jsonError(res, 404, "appointment not found");
        if (appt->status != AppointmentStatus::Completed)
            return jsonError(res, 400, "can only rate after the visit is Completed");

        // One rating per appointment.
        for (const auto &r : g_ratings)
            if (r.appointment_id == appointmentId)
                return jsonError(res, 409, "appointment already rated");

        Rating r;
        r.id             = nextRatingId();
        r.appointment_id = appointmentId;
        r.score          = score;
        g_ratings.push_back(r);
        saveRatings(F_RATINGS, g_ratings);

        // Recompute the hospital's aggregate rating from all ratings
        // attached to completed appointments at that hospital.
        int hospitalId = appt->hospital_id;
        double sum = 0;
        int count = 0;
        for (const auto &rr : g_ratings)
        {
            for (const auto &a : g_appointments)
            {
                if (a.id == rr.appointment_id && a.hospital_id == hospitalId)
                {
                    sum += rr.score;
                    count++;
                }
            }
        }
        for (auto &h : g_hospitals)
        {
            if (h.id == hospitalId)
            {
                h.rating = count > 0 ? static_cast<float>(sum / count) : h.rating;
                break;
            }
        }
        saveHospitals(F_HOSPITALS, g_hospitals);
        g_index = buildHospitalIndex(g_hospitals);

        jsonOk(res, ratingToJson(r));
    });

    // =====================================================================
    // EMERGENCY: nearest + lifecycle
    // =====================================================================

    svr.Get("/api/emergency/nearest", [](const httplib::Request &req, httplib::Response &res)
    {
        double lat, lon;
        if (!getDoubleParam(req, "lat", lat) || !getDoubleParam(req, "lon", lon))
            return jsonError(res, 400, "lat and lon required");

        int k = 3;
        if (req.has_param("k")) getIntParam(req, "k", k);

        std::lock_guard<std::recursive_mutex> lock(g_mutex);
        std::vector<Hospital> candidates;
        for (const auto &h : g_hospitals)
            if (h.available_beds > 0) candidates.push_back(h);

        auto ranked = topKHospitals(candidates, lat, lon, k);
        jsonOk(res, toJsonArray(ranked, rankedHospitalToJson));
    });

    // Create an emergency request in the Requested state, then immediately
    // assign the nearest hospital and move it to HospitalNotified.
    svr.Post("/api/emergency/request", [](const httplib::Request &req, httplib::Response &res)
    {
        int patientId;
        double lat, lon;
        if (!getIntParam(req, "patient_id", patientId) ||
            !getDoubleParam(req, "lat", lat) ||
            !getDoubleParam(req, "lon", lon))
        {
            return jsonError(res, 400, "patient_id, lat, lon required");
        }

        std::lock_guard<std::recursive_mutex> lock(g_mutex);

        // Rank candidates (only those with beds).
        std::vector<Hospital> candidates;
        for (const auto &h : g_hospitals)
            if (h.available_beds > 0) candidates.push_back(h);
        auto ranked = topKHospitals(candidates, lat, lon, 5);
        if (ranked.empty())
            return jsonError(res, 503, "no hospital with an available bed");

        EmergencyRequest e;
        e.id          = nextEmergencyId();
        e.patient_id  = patientId;
        e.hospital_id = ranked[0].hospital.id;
        e.patient_lat = lat;
        e.patient_lon = lon;
        e.state       = EmergencyState::Requested;

        applyTransition(e, EmergencyState::HospitalNotified);

        g_emergencies.push_back(e);
        saveEmergencies(F_EMERGENCIES, g_emergencies);

        jsonOk(res, emergencyToJson(e));
    });

    svr.Get("/api/emergency/all", [](const httplib::Request &, httplib::Response &res)
    {
        std::lock_guard<std::recursive_mutex> lock(g_mutex);
        jsonOk(res, toJsonArray(g_emergencies, emergencyToJson));
    });

    svr.Get("/api/emergency/(\\d+)",
            [](const httplib::Request &req, httplib::Response &res)
    {
        int id = std::stoi(req.matches[1]);
        std::lock_guard<std::recursive_mutex> lock(g_mutex);
        for (const auto &e : g_emergencies)
            if (e.id == id) return jsonOk(res, emergencyToJson(e));
        jsonError(res, 404, "emergency not found");
    });

    // Helper used by every lifecycle endpoint below. Capturing `target` and
    // `allowFallback` by value keeps each lambda independent.
    auto emergencyTransition = [](EmergencyState target, bool allowFallback)
    {
        return [target, allowFallback]
               (const httplib::Request &req, httplib::Response &res)
        {
            int id = std::stoi(req.matches[1]);
            std::lock_guard<std::recursive_mutex> lock(g_mutex);

            EmergencyRequest *e = nullptr;
            for (auto &x : g_emergencies) if (x.id == id) { e = &x; break; }
            if (!e) return jsonError(res, 404, "emergency not found");

            // Reject: use the fallback helper so the FSM picks the next
            // hospital and re-enters HospitalNotified.
            if (target == EmergencyState::Rejected && allowFallback)
            {
                if (!applyTransition(*e, EmergencyState::Rejected))
                    return jsonError(res, 400, "cannot reject in current state");

                std::vector<Hospital> candidates;
                for (const auto &h : g_hospitals)
                    if (h.available_beds > 0 && h.id != e->hospital_id)
                        candidates.push_back(h);
                auto ranked = topKHospitals(candidates, e->patient_lat,
                                            e->patient_lon, 10);

                std::vector<int> tried = { e->hospital_id };
                bool ok = fallbackToNextHospital(*e, ranked, tried);
                if (!ok)
                {
                    saveEmergencies(F_EMERGENCIES, g_emergencies);
                    return jsonError(res, 503,
                        "no further candidates; request stalled in Rejected");
                }
                saveEmergencies(F_EMERGENCIES, g_emergencies);
                return jsonOk(res, emergencyToJson(*e));
            }

            if (!applyTransition(*e, target))
                return jsonError(res, 400,
                    "invalid transition from " + emergencyStateToString(e->state));

            saveEmergencies(F_EMERGENCIES, g_emergencies);
            jsonOk(res, emergencyToJson(*e));
        };
    };

    svr.Post("/api/emergency/(\\d+)/notify",
             emergencyTransition(EmergencyState::HospitalNotified, false));
    svr.Post("/api/emergency/(\\d+)/accept",
             emergencyTransition(EmergencyState::Accepted, false));
    svr.Post("/api/emergency/(\\d+)/reject",
             emergencyTransition(EmergencyState::Rejected, true));
    svr.Post("/api/emergency/(\\d+)/callback",
             emergencyTransition(EmergencyState::CallbackValidation, false));
    svr.Post("/api/emergency/(\\d+)/assign",
             emergencyTransition(EmergencyState::AmbulanceAssigned, false));
    svr.Post("/api/emergency/(\\d+)/dispatch",
             emergencyTransition(EmergencyState::Dispatched, false));
    svr.Post("/api/emergency/(\\d+)/pickup",
             emergencyTransition(EmergencyState::PatientPickedUp, false));
    svr.Post("/api/emergency/(\\d+)/reach",
             emergencyTransition(EmergencyState::HospitalReached, false));
    svr.Post("/api/emergency/(\\d+)/complete",
             emergencyTransition(EmergencyState::Completed, false));

    std::cout << "MedEra API server listening on http://localhost:8080\n";
    svr.listen("0.0.0.0", 8080);

    return 0;
}