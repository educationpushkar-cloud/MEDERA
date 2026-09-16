#ifndef RANKING_H
#define RANKING_H

#include <vector>
#include "models.h"

// ---------------------------------------------------------------------------
// Phase III best-match recommendation (Section 6.5 of the synopsis report).
//
// score = w1*(1 - distNorm) + w2*ratingNorm + w3*bedNorm + w4*ambulanceNorm
//
// Candidates are first expected to be filtered by live available_beds > 0
// by the caller (Section 3.2 / 6.5) -- this module only ranks whatever list
// it's given. Retrieval of the top K uses a bounded min-heap, O(n log K),
// rather than sorting the full candidate list at O(n log n).
// ---------------------------------------------------------------------------

struct RankingWeights
{
    double distance  = 0.4;
    double rating     = 0.3;
    double beds       = 0.2;
    double ambulance = 0.1;
};

struct RankedHospital
{
    Hospital hospital;
    double distanceKm;
    double score;
};

// Computes a single hospital's weighted score given its distance from the
// patient and the max distance/beds/ambulance count observed across the
// current candidate set (used to normalise into 0-1 ranges).
double computeScore(const Hospital &h, double distanceKm,
                     double maxDistanceKm, int maxBeds, int maxAmbulance,
                     const RankingWeights &w = RankingWeights());

// Ranks candidates by distance from (patientLat, patientLon) and returns the
// top K by weighted score, highest first.
std::vector<RankedHospital> topKHospitals(const std::vector<Hospital> &candidates,
                                            double patientLat, double patientLon,
                                            int k,
                                            const RankingWeights &w = RankingWeights());

#endif
