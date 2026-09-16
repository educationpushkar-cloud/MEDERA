#ifndef HAVERSINE_H
#define HAVERSINE_H

// ---------------------------------------------------------------------------
// Phase III distance calculation (Section 6.4 of the synopsis report).
//
// Standard great-circle distance formula -- not something we derived, just
// applied. Reference: Sinnott, R.W. "Virtues of the Haversine",
// Sky and Telescope, 1984.
// ---------------------------------------------------------------------------

// Returns the great-circle distance in kilometres between two points given
// in decimal degrees.
double haversineDistanceKm(double lat1, double lon1, double lat2, double lon2);

#endif
