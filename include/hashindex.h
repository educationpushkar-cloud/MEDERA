#ifndef HASHINDEX_H
#define HASHINDEX_H

#include <string>
#include <unordered_map>
#include <vector>
#include "models.h"

// ---------------------------------------------------------------------------
// Phase II lookup layer (Section 6.2 of the synopsis report).
//
// Once hospital records are no longer a fixed-size array, direct indexing
// (h[city-1][area-1]) no longer works. This replaces it with a hash map
// keyed on "city|area", built once when data loads from storage. Average
// lookup stays O(1), same as Phase I, but without the fixed-size constraint.
// ---------------------------------------------------------------------------

using HospitalIndex = std::unordered_map<std::string, std::vector<Hospital>>;

// Builds the index from a flat list of hospitals loaded from storage.
HospitalIndex buildHospitalIndex(const std::vector<Hospital> &hospitals);

// Looks up all hospitals in a given city/area. Returns an empty vector if
// no hospitals are registered there yet -- callers should treat that as
// "no results," not an error.
std::vector<Hospital> lookupHospitals(const HospitalIndex &index,
                                       const std::string &city,
                                       const std::string &area);

#endif
