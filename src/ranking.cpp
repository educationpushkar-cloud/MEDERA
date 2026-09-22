#include "ranking.h"
#include "haversine.h"

#include <algorithm>
#include <queue>
#include <vector>

double computeScore(const Hospital &h, double distanceKm,
                     double maxDistanceKm, int maxBeds, int maxAmbulance,
                     const RankingWeights &w)
{
    double distNorm = (maxDistanceKm > 0) ? (distanceKm / maxDistanceKm) : 0.0;
    double ratingNorm = h.rating / 5.0; // ratings are on a 0-5 scale
    double bedNorm = (maxBeds > 0) ? (static_cast<double>(h.available_beds) / maxBeds) : 0.0;
    double ambNorm = (maxAmbulance > 0) ? (static_cast<double>(h.ambulance_count) / maxAmbulance) : 0.0;

    return w.distance * (1.0 - distNorm) +
           w.rating * ratingNorm +
           w.beds * bedNorm +
           w.ambulance * ambNorm;
}

std::vector<RankedHospital> topKHospitals(const std::vector<Hospital> &candidates,
                                            double patientLat, double patientLon,
                                            int k,
                                            const RankingWeights &w)
{
    if (candidates.empty() || k <= 0) return {};

    // First pass: compute distance for every candidate, and track the
    // maximums used for normalisation.
    std::vector<RankedHospital> scored;
    scored.reserve(candidates.size());

    double maxDistance = 0.0;
    int maxBeds = 0;
    int maxAmbulance = 0;
    for (const auto &h : candidates)
    {
        double d = haversineDistanceKm(patientLat, patientLon, h.latitude, h.longitude);
        maxDistance = std::max(maxDistance, d);
        maxBeds = std::max(maxBeds, h.available_beds);
        maxAmbulance = std::max(maxAmbulance, h.ambulance_count);
        scored.push_back({h, d, 0.0});
    }

    for (auto &rh : scored)
    {
        rh.score = computeScore(rh.hospital, rh.distanceKm, maxDistance, maxBeds, maxAmbulance, w);
    }

    // Bounded min-heap of size k: keep the k highest-scoring candidates seen
    // so far, popping the current lowest whenever the heap grows past k.
    auto cmp = [](const RankedHospital &a, const RankedHospital &b)
    {
        return a.score > b.score; // min-heap on score
    };
    std::priority_queue<RankedHospital, std::vector<RankedHospital>, decltype(cmp)> heap(cmp);

    for (const auto &rh : scored)
    {
        heap.push(rh);
        if (static_cast<int>(heap.size()) > k)
        {
            heap.pop();
        }
    }

    // Drain the heap and reverse so the result is highest score first.
    std::vector<RankedHospital> result;
    result.reserve(heap.size());
    while (!heap.empty())
    {
        result.push_back(heap.top());
        heap.pop();
    }
    std::reverse(result.begin(), result.end());
    return result;
}
