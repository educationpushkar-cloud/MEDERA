#include "haversine.h"
#include <cmath>

static const double EARTH_RADIUS_KM = 6371.0;
static const double PI = 3.14159265358979323846;

static double toRadians(double degrees)
{
    return degrees * PI / 180.0;
}

double haversineDistanceKm(double lat1, double lon1, double lat2, double lon2)
{
    double phi1 = toRadians(lat1);
    double phi2 = toRadians(lat2);

    double deltaPhi = toRadians(lat2 - lat1);
    double deltaLambda = toRadians(lon2 - lon1);

    double a = std::sin(deltaPhi / 2) * std::sin(deltaPhi / 2) +
               std::cos(phi1) * std::cos(phi2) *
               std::sin(deltaLambda / 2) * std::sin(deltaLambda / 2);

    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));

    return EARTH_RADIUS_KM * c;
}