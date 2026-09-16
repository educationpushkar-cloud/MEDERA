#ifndef MODELS_H
#define MODELS_H

#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Core entities. These mirror the data model in the Phase I synopsis report
// (Section 3.2): Hospital, Doctor, Patient, Appointment, Emergency Request,
// Rating. Phase I only tracked {city, area, name, rating} inside a fixed
// array; this is the fuller schema Phase II storage is built around.
// ---------------------------------------------------------------------------

struct Hospital
{
    int id;
    std::string name;
    std::string city;
    std::string area;
    double latitude;
    double longitude;
    int total_beds;
    int available_beds;   // updated live by hospital staff
    int ambulance_count;
    float rating;          // aggregated from completed-visit Ratings
};

struct Doctor
{
    int id;
    int hospital_id;       // FK -> Hospital.id
    std::string name;
    std::string specialization;
};

struct Patient
{
    int id;
    std::string name;
    std::string contact;
    std::string blood_group;
};

enum class AppointmentStatus
{
    Requested,
    Confirmed,
    Completed,
    Cancelled
};

struct Appointment
{
    int id;
    int patient_id;         // FK -> Patient.id
    int hospital_id;        // FK -> Hospital.id
    int doctor_id;          // FK -> Doctor.id
    AppointmentStatus status;
};

struct Rating
{
    int id;
    int appointment_id;     // FK -> Appointment.id (post-visit only)
    int score;              // 1-5
};

// Emergency-request state machine states (Section 6.6 of the synopsis).
enum class EmergencyState
{
    Requested,
    HospitalNotified,
    Accepted,
    Rejected,               // triggers fallback to next-nearest hospital
    CallbackValidation,
    AmbulanceAssigned,
    Dispatched,
    PatientPickedUp,
    HospitalReached,
    Completed
};

struct EmergencyRequest
{
    int id;
    int patient_id;          // FK -> Patient.id
    int hospital_id;         // currently-assigned hospital, FK -> Hospital.id (0 = unassigned)
    double patient_lat;
    double patient_lon;
    EmergencyState state;
};

std::string emergencyStateToString(EmergencyState s);
EmergencyState emergencyStateFromString(const std::string &s);
std::string appointmentStatusToString(AppointmentStatus s);
AppointmentStatus appointmentStatusFromString(const std::string &s);

#endif
