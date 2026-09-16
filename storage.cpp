#include "storage.h"

#include <fstream>
#include <sstream>
#include <vector>
#include <string>

// Splits a line on '|' into fields. Kept simple and dependency-free on
// purpose -- no CSV library, no external parser -- so any teammate can read
// and modify this without extra setup.
static std::vector<std::string> splitFields(const std::string &line)
{
    std::vector<std::string> fields;
    std::stringstream ss(line);
    std::string field;
    while (std::getline(ss, field, '|'))
    {
        fields.push_back(field);
    }
    return fields;
}

// ---------------------------------------------------------------------------
// Hospital
// Format: id|name|city|area|latitude|longitude|total_beds|available_beds|ambulance_count|rating
// ---------------------------------------------------------------------------
std::vector<Hospital> loadHospitals(const std::string &path)
{
    std::vector<Hospital> hospitals;
    std::ifstream in(path);
    if (!in.is_open()) return hospitals; // empty on first run; caller can seed data

    std::string line;
    while (std::getline(in, line))
    {
        if (line.empty()) continue;
        auto f = splitFields(line);
        if (f.size() < 10) continue; // skip malformed lines rather than crash

        Hospital h;
        h.id              = std::stoi(f[0]);
        h.name            = f[1];
        h.city            = f[2];
        h.area            = f[3];
        h.latitude        = std::stod(f[4]);
        h.longitude       = std::stod(f[5]);
        h.total_beds      = std::stoi(f[6]);
        h.available_beds  = std::stoi(f[7]);
        h.ambulance_count = std::stoi(f[8]);
        h.rating          = std::stof(f[9]);
        hospitals.push_back(h);
    }
    return hospitals;
}

void saveHospitals(const std::string &path, const std::vector<Hospital> &hospitals)
{
    std::ofstream out(path, std::ios::trunc);
    for (const auto &h : hospitals)
    {
        out << h.id << "|" << h.name << "|" << h.city << "|" << h.area << "|"
            << h.latitude << "|" << h.longitude << "|" << h.total_beds << "|"
            << h.available_beds << "|" << h.ambulance_count << "|" << h.rating << "\n";
    }
}

// ---------------------------------------------------------------------------
// Doctor
// Format: id|hospital_id|name|specialization
// ---------------------------------------------------------------------------
std::vector<Doctor> loadDoctors(const std::string &path)
{
    std::vector<Doctor> doctors;
    std::ifstream in(path);
    if (!in.is_open()) return doctors;

    std::string line;
    while (std::getline(in, line))
    {
        if (line.empty()) continue;
        auto f = splitFields(line);
        if (f.size() < 4) continue;

        Doctor d;
        d.id             = std::stoi(f[0]);
        d.hospital_id    = std::stoi(f[1]);
        d.name           = f[2];
        d.specialization = f[3];
        doctors.push_back(d);
    }
    return doctors;
}

void saveDoctors(const std::string &path, const std::vector<Doctor> &doctors)
{
    std::ofstream out(path, std::ios::trunc);
    for (const auto &d : doctors)
    {
        out << d.id << "|" << d.hospital_id << "|" << d.name << "|" << d.specialization << "\n";
    }
}

// ---------------------------------------------------------------------------
// Patient
// Format: id|name|contact|blood_group
// ---------------------------------------------------------------------------
std::vector<Patient> loadPatients(const std::string &path)
{
    std::vector<Patient> patients;
    std::ifstream in(path);
    if (!in.is_open()) return patients;

    std::string line;
    while (std::getline(in, line))
    {
        if (line.empty()) continue;
        auto f = splitFields(line);
        if (f.size() < 4) continue;

        Patient p;
        p.id          = std::stoi(f[0]);
        p.name        = f[1];
        p.contact     = f[2];
        p.blood_group = f[3];
        patients.push_back(p);
    }
    return patients;
}

void savePatients(const std::string &path, const std::vector<Patient> &patients)
{
    std::ofstream out(path, std::ios::trunc);
    for (const auto &p : patients)
    {
        out << p.id << "|" << p.name << "|" << p.contact << "|" << p.blood_group << "\n";
    }
}

// ---------------------------------------------------------------------------
// Appointment
// Format: id|patient_id|hospital_id|doctor_id|status
// ---------------------------------------------------------------------------
std::vector<Appointment> loadAppointments(const std::string &path)
{
    std::vector<Appointment> appointments;
    std::ifstream in(path);
    if (!in.is_open()) return appointments;

    std::string line;
    while (std::getline(in, line))
    {
        if (line.empty()) continue;
        auto f = splitFields(line);
        if (f.size() < 5) continue;

        Appointment a;
        a.id          = std::stoi(f[0]);
        a.patient_id  = std::stoi(f[1]);
        a.hospital_id = std::stoi(f[2]);
        a.doctor_id   = std::stoi(f[3]);
        a.status      = appointmentStatusFromString(f[4]);
        appointments.push_back(a);
    }
    return appointments;
}

void saveAppointments(const std::string &path, const std::vector<Appointment> &appointments)
{
    std::ofstream out(path, std::ios::trunc);
    for (const auto &a : appointments)
    {
        out << a.id << "|" << a.patient_id << "|" << a.hospital_id << "|" << a.doctor_id
            << "|" << appointmentStatusToString(a.status) << "\n";
    }
}

// ---------------------------------------------------------------------------
// Rating
// Format: id|appointment_id|score
// ---------------------------------------------------------------------------
std::vector<Rating> loadRatings(const std::string &path)
{
    std::vector<Rating> ratings;
    std::ifstream in(path);
    if (!in.is_open()) return ratings;

    std::string line;
    while (std::getline(in, line))
    {
        if (line.empty()) continue;
        auto f = splitFields(line);
        if (f.size() < 3) continue;

        Rating r;
        r.id             = std::stoi(f[0]);
        r.appointment_id = std::stoi(f[1]);
        r.score          = std::stoi(f[2]);
        ratings.push_back(r);
    }
    return ratings;
}

void saveRatings(const std::string &path, const std::vector<Rating> &ratings)
{
    std::ofstream out(path, std::ios::trunc);
    for (const auto &r : ratings)
    {
        out << r.id << "|" << r.appointment_id << "|" << r.score << "\n";
    }
}

// ---------------------------------------------------------------------------
// EmergencyRequest
// Format: id|patient_id|hospital_id|patient_lat|patient_lon|state
// ---------------------------------------------------------------------------
std::vector<EmergencyRequest> loadEmergencies(const std::string &path)
{
    std::vector<EmergencyRequest> emergencies;
    std::ifstream in(path);
    if (!in.is_open()) return emergencies;

    std::string line;
    while (std::getline(in, line))
    {
        if (line.empty()) continue;
        auto f = splitFields(line);
        if (f.size() < 6) continue;

        EmergencyRequest e;
        e.id          = std::stoi(f[0]);
        e.patient_id  = std::stoi(f[1]);
        e.hospital_id = std::stoi(f[2]);
        e.patient_lat = std::stod(f[3]);
        e.patient_lon = std::stod(f[4]);
        e.state       = emergencyStateFromString(f[5]);
        emergencies.push_back(e);
    }
    return emergencies;
}

void saveEmergencies(const std::string &path, const std::vector<EmergencyRequest> &emergencies)
{
    std::ofstream out(path, std::ios::trunc);
    for (const auto &e : emergencies)
    {
        out << e.id << "|" << e.patient_id << "|" << e.hospital_id << "|"
            << e.patient_lat << "|" << e.patient_lon << "|"
            << emergencyStateToString(e.state) << "\n";
    }
}
