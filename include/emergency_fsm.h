#ifndef EMERGENCY_FSM_H
#define EMERGENCY_FSM_H

#include <vector>
#include "models.h"
#include "ranking.h"

// ---------------------------------------------------------------------------
// Phase III emergency state machine (Section 6.6 of the synopsis report).
//
//   Requested -> HospitalNotified -> Accepted -> CallbackValidation ->
//   AmbulanceAssigned -> Dispatched -> PatientPickedUp -> HospitalReached ->
//   Completed
//
//   HospitalNotified -> Rejected -> (fall back to next-nearest hospital,
//   re-enter HospitalNotified)
//
// Modelling this as an explicit table means an invalid jump (e.g. marking a
// request Completed before it was Dispatched) simply cannot be applied.
// ---------------------------------------------------------------------------

// Returns true if moving from `from` to `to` is a legal transition.
bool canTransition(EmergencyState from, EmergencyState to);

// Attempts the transition. Returns true and updates req.state on success;
// returns false and leaves req.state untouched if the transition is invalid.
bool applyTransition(EmergencyRequest &req, EmergencyState to);

// Call when the currently-assigned hospital rejects (or times out on) a
// request. Reassigns the request to the next-best hospital from a
// pre-ranked candidate list (see ranking.h) that hasn't already been tried,
// and moves the state back to HospitalNotified. Returns false if no
// untried candidates remain.
bool fallbackToNextHospital(EmergencyRequest &req,
                             const std::vector<RankedHospital> &rankedCandidates,
                             std::vector<int> &triedHospitalIds);

#endif
