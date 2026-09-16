#include "emergency_fsm.h"

#include <algorithm>

bool canTransition(EmergencyState from, EmergencyState to)
{
    switch (from)
    {
        case EmergencyState::Requested:
            return to == EmergencyState::HospitalNotified;

        case EmergencyState::HospitalNotified:
            return to == EmergencyState::Accepted || to == EmergencyState::Rejected;

        case EmergencyState::Rejected:
            // A rejection only leads back to a (re)notified state, once the
            // caller has reassigned to a different hospital.
            return to == EmergencyState::HospitalNotified;

        case EmergencyState::Accepted:
            return to == EmergencyState::CallbackValidation;

        case EmergencyState::CallbackValidation:
            return to == EmergencyState::AmbulanceAssigned;

        case EmergencyState::AmbulanceAssigned:
            return to == EmergencyState::Dispatched;

        case EmergencyState::Dispatched:
            return to == EmergencyState::PatientPickedUp;

        case EmergencyState::PatientPickedUp:
            return to == EmergencyState::HospitalReached;

        case EmergencyState::HospitalReached:
            return to == EmergencyState::Completed;

        case EmergencyState::Completed:
            return false; // terminal state
    }
    return false;
}

bool applyTransition(EmergencyRequest &req, EmergencyState to)
{
    if (!canTransition(req.state, to))
    {
        return false;
    }
    req.state = to;
    return true;
}

bool fallbackToNextHospital(EmergencyRequest &req,
                             const std::vector<RankedHospital> &rankedCandidates,
                             std::vector<int> &triedHospitalIds)
{
    if (req.state != EmergencyState::Rejected)
    {
        return false; // only meaningful right after a rejection
    }

    for (const auto &rc : rankedCandidates)
    {
        bool alreadyTried = std::find(triedHospitalIds.begin(), triedHospitalIds.end(),
                                       rc.hospital.id) != triedHospitalIds.end();
        if (!alreadyTried)
        {
            req.hospital_id = rc.hospital.id;
            triedHospitalIds.push_back(rc.hospital.id);
            return applyTransition(req, EmergencyState::HospitalNotified);
        }
    }

    return false; // no untried hospital left in the ranked list
}
