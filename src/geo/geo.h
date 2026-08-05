#pragma once
#include <math.h>

// Local planar geometry (ENU) + geographic utilities.
// Racetrack / urban-trip scale: an equirectangular projection is enough.

// Note: explicit constructors (instead of NSDMI) to allow brace
// initialization in C++11 (the ESP32 core's gnu++11).
struct Vec2 {
    float x, y;   // meters (E, N) relative to the reference
    Vec2() : x(0), y(0) {}
    Vec2(float x_, float y_) : x(x_), y(y_) {}
};

class GeoProjector {
public:
    void setRef(double lat, double lon);
    bool isSet() const { return set_; }
    Vec2 project(double lat, double lon) const;

private:
    double refLat_ = 0, refLon_ = 0, cosLat_ = 1;
    bool set_ = false;
};

// Distance between two geographic points (m).
double haversineMeters(double lat1, double lon1, double lat2, double lon2);

// Intersection of segment P1->P2 with Q1->Q2.
// Returns true and the fraction t (0..1) along P1->P2 at the crossing point.
bool segmentsIntersect(const Vec2& p1, const Vec2& p2,
                       const Vec2& q1, const Vec2& q2, float& tOut);

// Unix epoch (ms) from UTC date/time.
uint64_t toEpochMs(int year, int month, int day,
                   int hour, int minute, int second, int centisecond);
