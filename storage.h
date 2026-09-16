#ifndef STORAGE_H
#define STORAGE_H

#include <string>
#include <vector>
#include "models.h"

// ---------------------------------------------------------------------------
// Phase II storage layer.
//
// Replaces the Phase I compile-time array (hos h[3][4][3]) with plain
// pipe-delimited text files under data/. This satisfies the "file-backed"
// half of the proposal's "file-backed / lightweight SQL store" without
// requiring an external database library, so it compiles anywhere with only
// the standard library.
//
// Each entity has its own file (data/hospitals.txt, data/doctors.txt, ...).
// One record per line, fields separated by '|'. Loading happens once at
// program start; saving happens whenever a record is added or changed
// (e.g. a hospital updating its own available_beds).
// ---------------------------------------------------------------------------

std::vector<Hospital> loadHospitals(const std::string &path);
void saveHospitals(const std::string &path, const std::vector<Hospital> &hospitals);

std::vector<Doctor> loadDoctors(const std::string &path);
void saveDoctors(const std::string &path, const std::vector<Doctor> &doctors);

std::vector<Patient> loadPatients(const std::string &path);
void savePatients(const std::string &path, const std::vector<Patient> &patients);

std::vector<Appointment> loadAppointments(const std::string &path);
void saveAppointments(const std::string &path, const std::vector<Appointment> &appointments);

std::vector<Rating> loadRatings(const std::string &path);
void saveRatings(const std::string &path, const std::vector<Rating> &ratings);

std::vector<EmergencyRequest> loadEmergencies(const std::string &path);
void saveEmergencies(const std::string &path, const std::vector<EmergencyRequest> &emergencies);

#endif
