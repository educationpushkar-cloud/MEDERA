// src/phase1_plus_2.cpp
//
// Combined Phase I + Phase II demonstration:
//   Part A: the Phase I console menu (patient → city → area → hospital list)
//   Part B: Phase II storage + hash index loading the same data
//   Part C: Phase III emergency FSM with ranking
//
// This file exists ONLY for the presentation; it does not replace
// main.cpp (Phase I) or phase2_3_demo.cpp (Phase II/III core).

#include "function.h"      // Phase I
#include "models.h"
#include "storage.h"
#include "hashindex.h"
#include "ranking.h"
#include "emergency_fsm.h"

#include <iostream>
#include <iomanip>

int main()
{
    std::cout << "=========================================\n";
    std::cout << "  MedEra -- Combined Phase I + II Demo\n";
    std::cout << "=========================================\n";

    // ---- PART A: Phase I console (same calls as src/main.cpp) -----------
    std::cout << "\n[ PART A ] Phase I console flow\n\n";
    welcomepage();
    clearscreen();
    loginpage();   // runs the interactive menu; returns when user navigates

    // ---- PART B: Phase II storage + hash index --------------------------
    std::cout << "\n[ PART B ] Phase II storage + hash index\n\n";
    const std::string HOSPITAL_FILE = "data/hospitals.txt";

    auto hospitals = loadHospitals(HOSPITAL_FILE);
    if (hospitals.empty())
    {
        std::cout << "(No data yet -- seed it by running phase2_3_demo.exe first)\n";
        return 0;
    }

    std::cout << "Loaded " << hospitals.size() << " hospitals from "
              << HOSPITAL_FILE << "\n";

    HospitalIndex index = buildHospitalIndex(hospitals);
    auto clement = lookupHospitals(index, "DEHRADUN", "CLEMENT TOWN");
    std::cout << "Hash-index lookup DEHRADUN/CLEMENT TOWN -> "
              << clement.size() << " hospital(s):\n";
    for (const auto &h : clement)
        std::cout << "  - " << h.name << "  beds=" << h.available_beds << "\n";

    // ---- PART C: Phase III emergency FSM --------------------------------
    std::cout << "\n[ PART C ] Phase III ranking + emergency FSM\n\n";

    std::vector<Hospital> candidates;
    for (const auto &h : hospitals)
        if (h.available_beds > 0) candidates.push_back(h);

    auto ranked = topKHospitals(candidates, 30.30, 78.02, 3);
    std::cout << "Top " << ranked.size() << " hospitals by weighted score:\n";
    for (const auto &r : ranked)
        std::cout << std::fixed << std::setprecision(2)
                  << "  - " << r.hospital.name
                  << "  dist=" << r.distanceKm << " km"
                  << "  score=" << r.score << "\n";

    if (!ranked.empty())
    {
        EmergencyRequest req{1, 101, ranked[0].hospital.id, 30.30, 78.02,
                             EmergencyState::Requested};
        std::vector<int> tried;

        applyTransition(req, EmergencyState::HospitalNotified);
        tried.push_back(req.hospital_id);
        std::cout << "  State: " << emergencyStateToString(req.state) << "\n";

        applyTransition(req, EmergencyState::Rejected);
        fallbackToNextHospital(req, ranked, tried);
        std::cout << "  After fallback: hospital_id=" << req.hospital_id
                  << " state=" << emergencyStateToString(req.state) << "\n";
    }

    std::cout << "\nDemo complete.\n";
    return 0;
}