#include "models.h"

std::string emergencyStateToString(EmergencyState s)
{
    switch (s)
    {
        case EmergencyState::Requested:          return "REQUESTED";
        case EmergencyState::HospitalNotified:    return "HOSPITAL_NOTIFIED";
        case EmergencyState::Accepted:            return "ACCEPTED";
        case EmergencyState::Rejected:            return "REJECTED";
        case EmergencyState::CallbackValidation:  return "CALLBACK_VALIDATION";
        case EmergencyState::AmbulanceAssigned:   return "AMBULANCE_ASSIGNED";
        case EmergencyState::Dispatched:          return "DISPATCHED";
        case EmergencyState::PatientPickedUp:     return "PATIENT_PICKED_UP";
        case EmergencyState::HospitalReached:     return "HOSPITAL_REACHED";
        case EmergencyState::Completed:           return "COMPLETED";
    }
    return "UNKNOWN";
}

EmergencyState emergencyStateFromString(const std::string &s)
{
    if (s == "REQUESTED")             return EmergencyState::Requested;
    if (s == "HOSPITAL_NOTIFIED")     return EmergencyState::HospitalNotified;
    if (s == "ACCEPTED")              return EmergencyState::Accepted;
    if (s == "REJECTED")              return EmergencyState::Rejected;
    if (s == "CALLBACK_VALIDATION")   return EmergencyState::CallbackValidation;
    if (s == "AMBULANCE_ASSIGNED")    return EmergencyState::AmbulanceAssigned;
    if (s == "DISPATCHED")            return EmergencyState::Dispatched;
    if (s == "PATIENT_PICKED_UP")     return EmergencyState::PatientPickedUp;
    if (s == "HOSPITAL_REACHED")      return EmergencyState::HospitalReached;
    if (s == "COMPLETED")             return EmergencyState::Completed;
    return EmergencyState::Requested;
}

std::string appointmentStatusToString(AppointmentStatus s)
{
    switch (s)
    {
        case AppointmentStatus::Requested: return "REQUESTED";
        case AppointmentStatus::Confirmed: return "CONFIRMED";
        case AppointmentStatus::Completed: return "COMPLETED";
        case AppointmentStatus::Cancelled: return "CANCELLED";
    }
    return "UNKNOWN";
}

AppointmentStatus appointmentStatusFromString(const std::string &s)
{
    if (s == "CONFIRMED") return AppointmentStatus::Confirmed;
    if (s == "COMPLETED") return AppointmentStatus::Completed;
    if (s == "CANCELLED") return AppointmentStatus::Cancelled;
    return AppointmentStatus::Requested;
}
