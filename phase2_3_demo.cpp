// Demo/integration test for the Phase II/III modules: storage, hash index,
// Haversine distance, weighted ranking, and the emergency FSM.
//
// This is not the final Phase II/III application -- it is a runnable proof
// that the new modules work correctly together, for the team to build the
// real console/web flows on top of. Run it with:
//
//   g++ -Iinclude src/models.cpp src/storage.cpp src/hashindex.cpp
//       src/haversine.cpp src/ranking.cpp src/emergency_fsm.cpp
//       src/phase2_3_demo.cpp -o phase2_3_demo
//   ./phase2_3_demo

#include <iostream>
#include <iomanip>
#include <vector>

#include "models.h"
#include "storage.h"
#include "hashindex.h"
#include "haversine.h"
#include "ranking.h"
#include "emergency_fsm.h"

static std::vector<Hospital> seedDehradunHospitals()
{
    // Approximate real coordinates so Haversine distances are meaningful.
    return {
        {1, "VELMED",    "DEHRADUN", "CLEMENT TOWN", 30.3050, 78.0210, 40, 3, 2, 3.4f},
        {2, "ABC",       "DEHRADUN", "CLEMENT TOWN", 30.3070, 78.0190, 60, 0, 1, 4.5f},
        {3, "XYZ",       "DEHRADUN", "CLEMENT TOWN", 30.3090, 78.0230, 50, 12, 3, 5.0f},
        {4, "AIIMS",     "DEHRADUN", "GHARI CANTT",  30.3350, 78.0420, 200, 25, 6, 3.4f},
        {5, "NURAHH",    "DEHRADUN", "GHARI CANTT",  30.3370, 78.0400, 30, 1, 1, 4.5f},
    };
}

static void printDivider(const std::string &title)
{
    std::cout << "\n=== " << title << " ===\n";
}

static bool step(EmergencyRequest &req, EmergencyState to, const char *label)
{
    bool ok = applyTransition(req, to);
    std::cout << "  " << label << ": "
              << (ok ? "OK -> " : "REJECTED (invalid transition) -> ")
              << emergencyStateToString(req.state) << "\n";
    return ok;
}

int main()
{
    // --- 1. Storage: load, or seed on first run -----------------------
    printDivider("Storage layer");
    const std::string hospitalFile = "data/hospitals.txt";
    std::vector<Hospital> hospitals = loadHospitals(hospitalFile);
    if (hospitals.empty())
    {
        std::cout << "No existing data found -- seeding sample Dehradun hospitals.\n";
        hospitals = seedDehradunHospitals();
        saveHospitals(hospitalFile, hospitals);
    }
    std::cout << "Loaded " << hospitals.size() << " hospital record(s) from " << hospitalFile << "\n";

    // --- 2. Hash index: replace array indexing with city|area lookup ---
    printDivider("Hash-indexed lookup");
    HospitalIndex index = buildHospitalIndex(hospitals);
    auto clementTown = lookupHospitals(index, "DEHRADUN", "CLEMENT TOWN");
    std::cout << "Hospitals in DEHRADUN / CLEMENT TOWN: " << clementTown.size() << "\n";
    for (const auto &h : clementTown)
    {
        std::cout << "  - " << h.name << " (beds available: " << h.available_beds << ")\n";
    }

    // --- 3. Haversine + weighted ranking --------------------------------
    printDivider("Distance + weighted ranking");
    double patientLat = 30.3000, patientLon = 78.0200; // somewhere near Clement Town

    // Filter first by live bed availability (Section 6.5), THEN rank.
    std::vector<Hospital> candidates;
    for (const auto &h : hospitals)
    {
        if (h.available_beds > 0) candidates.push_back(h);
    }
    std::cout << candidates.size() << " of " << hospitals.size()
              << " hospitals currently have an available bed.\n";

    auto ranked = topKHospitals(candidates, patientLat, patientLon, 3);
    std::cout << "Top " << ranked.size() << " best-match hospitals:\n";
    for (const auto &rh : ranked)
    {
        std::cout << std::fixed << std::setprecision(2)
                   << "  - " << rh.hospital.name
                   << " | distance: " << rh.distanceKm << " km"
                   << " | score: " << rh.score << "\n";
    }

    // --- 4. Emergency state machine, including reject + fallback -------
    printDivider("Emergency state machine");
    EmergencyRequest req{1, /*patient_id=*/101, ranked.empty() ? 0 : ranked[0].hospital.id,
                         patientLat, patientLon, EmergencyState::Requested};
    std::vector<int> tried;

    step(req, EmergencyState::HospitalNotified, "Notify nearest hospital");
    tried.push_back(req.hospital_id);

    // Simulate the nearest hospital rejecting the request.
    step(req, EmergencyState::Rejected, "Hospital rejects");
    bool reassigned = fallbackToNextHospital(req, ranked, tried);
    std::cout << "  Fallback to next hospital: " << (reassigned ? "OK" : "NO CANDIDATES LEFT")
              << " -> now assigned to hospital_id " << req.hospital_id
              << " (" << emergencyStateToString(req.state) << ")\n";

    step(req, EmergencyState::Accepted, "Hospital accepts");
    step(req, EmergencyState::CallbackValidation, "Hospital calls patient back");
    step(req, EmergencyState::AmbulanceAssigned, "Ambulance assigned");
    step(req, EmergencyState::Dispatched, "Ambulance dispatched");
    step(req, EmergencyState::PatientPickedUp, "Patient picked up");
    step(req, EmergencyState::HospitalReached, "Hospital reached");
    step(req, EmergencyState::Completed, "Request completed");

    // Demonstrate that an invalid jump is correctly rejected.
    printDivider("Invalid transition check");
    EmergencyRequest badReq{2, 102, 0, 0, 0, EmergencyState::Requested};
    step(badReq, EmergencyState::Completed, "Try to jump straight to Completed");

    return 0;
}
