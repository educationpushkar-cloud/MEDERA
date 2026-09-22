// ---------------------------------------------------------------------------
// Phase II web server.
//
// SETUP REQUIRED (one-time, per machine that builds this):
//   Download the single-header library cpp-httplib and place it at
//   include/httplib.h -- https://github.com/yhirose/cpp-httplib
//   (Right-click "Raw" on httplib.h in that repo -> Save As -> include/httplib.h)
//
// This file was written against the documented, stable cpp-httplib API but
// has NOT been compiled in the environment this project was drafted in,
// because that header isn't available there. Build and test it on your own
// machine once httplib.h is in place:
//
//   g++ -std=c++17 -Iinclude src/models.cpp src/storage.cpp src/hashindex.cpp \
//       src/haversine.cpp src/ranking.cpp src/emergency_fsm.cpp \
//       src/server.cpp -lpthread -o medera_server
//   ./medera_server
//   then open web/index.html in a browser (or serve it via the same server,
//   see the mount point set up below)
//
// No JSON library dependency is used on purpose -- responses are built with
// plain string concatenation since our data is simple and flat. If the team
// wants richer JSON later, nlohmann/json (also single-header) is the natural
// next addition.
// ---------------------------------------------------------------------------
#include <thread>
#include <mutex>
#include <condition_variable>

#include "httplib.h"

#include "models.h"
#include "storage.h"
#include "hashindex.h"
#include "ranking.h"
#include "emergency_fsm.h"
#include <iostream>

#include <sstream>
#include <vector>

static const std::string HOSPITAL_FILE = "data/hospitals.txt";

static std::string hospitalToJson(const Hospital &h)
{
    std::ostringstream j;
    j << "{"
      << "\"id\":" << h.id << ","
      << "\"name\":\"" << h.name << "\","
      << "\"city\":\"" << h.city << "\","
      << "\"area\":\"" << h.area << "\","
      << "\"available_beds\":" << h.available_beds << ","
      << "\"total_beds\":" << h.total_beds << ","
      << "\"ambulance_count\":" << h.ambulance_count << ","
      << "\"rating\":" << h.rating
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

int main()
{
    std::vector<Hospital> hospitals = loadHospitals(HOSPITAL_FILE);
    HospitalIndex index = buildHospitalIndex(hospitals);

    httplib::Server svr;

    // Serve the static frontend (web/index.html, style.css, app.js) at "/".
    svr.set_mount_point("/", "./web");

    // GET /api/hospitals?city=DEHRADUN&area=CLEMENT%20TOWN
    svr.Get("/api/hospitals", [&](const httplib::Request &req, httplib::Response &res)
    {
        std::string city = req.get_param_value("city");
        std::string area = req.get_param_value("area");

        auto results = lookupHospitals(index, city, area);

        std::ostringstream body;
        body << "[";
        for (size_t i = 0; i < results.size(); i++)
        {
            if (i > 0) body << ",";
            body << hospitalToJson(results[i]);
        }
        body << "]";

        res.set_content(body.str(), "application/json");
    });

    // GET /api/emergency/nearest?lat=30.30&lon=78.02&k=3
    svr.Get("/api/emergency/nearest", [&](const httplib::Request &req, httplib::Response &res)
    {
        double lat = std::stod(req.get_param_value("lat"));
        double lon = std::stod(req.get_param_value("lon"));
        int k = req.has_param("k") ? std::stoi(req.get_param_value("k")) : 3;

        // Filter to hospitals with a live open bed before ranking (Section 6.5).
        std::vector<Hospital> candidates;
        for (const auto &h : hospitals)
        {
            if (h.available_beds > 0) candidates.push_back(h);
        }

        auto ranked = topKHospitals(candidates, lat, lon, k);

        std::ostringstream body;
        body << "[";
        for (size_t i = 0; i < ranked.size(); i++)
        {
            if (i > 0) body << ",";
            body << rankedHospitalToJson(ranked[i]);
        }
        body << "]";

        res.set_content(body.str(), "application/json");
    });

    // POST /api/hospital/update-beds  (form fields: id, available_beds)
    // This is the endpoint a hospital's own login/dashboard would call to
    // report live bed availability (Section 3.2 / the report's answer to
    // "how is a bed identified as available in real time").
    svr.Post("/api/hospital/update-beds", [&](const httplib::Request &req, httplib::Response &res)
    {
        int id = std::stoi(req.get_param_value("id"));
        int newAvailable = std::stoi(req.get_param_value("available_beds"));

        bool found = false;
        for (auto &h : hospitals)
        {
            if (h.id == id)
            {
                h.available_beds = newAvailable;
                found = true;
                break;
            }
        }

        if (found)
        {
            saveHospitals(HOSPITAL_FILE, hospitals);
            index = buildHospitalIndex(hospitals); // rebuild so lookups see the change immediately
            res.set_content("{\"status\":\"ok\"}", "application/json");
        }
        else
        {
            res.status = 404;
            res.set_content("{\"status\":\"not found\"}", "application/json");
        }
    });

    std::cout << "MedEra API server listening on http://localhost:8080\n";
    svr.listen("0.0.0.0", 8080);

    return 0;
}
