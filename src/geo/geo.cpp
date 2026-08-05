#include "geo.h"
#include <stdint.h>

static constexpr double kEarthRadiusM = 6371000.0;
static constexpr double kDeg2Rad = M_PI / 180.0;

void GeoProjector::setRef(double lat, double lon) {
    refLat_ = lat;
    refLon_ = lon;
    cosLat_ = cos(lat * kDeg2Rad);
    set_ = true;
}

Vec2 GeoProjector::project(double lat, double lon) const {
    Vec2 v;
    v.x = (float)((lon - refLon_) * kDeg2Rad * cosLat_ * kEarthRadiusM);
    v.y = (float)((lat - refLat_) * kDeg2Rad * kEarthRadiusM);
    return v;
}

double haversineMeters(double lat1, double lon1, double lat2, double lon2) {
    double dLat = (lat2 - lat1) * kDeg2Rad;
    double dLon = (lon2 - lon1) * kDeg2Rad;
    double a = sin(dLat / 2) * sin(dLat / 2) +
               cos(lat1 * kDeg2Rad) * cos(lat2 * kDeg2Rad) *
               sin(dLon / 2) * sin(dLon / 2);
    return 2.0 * kEarthRadiusM * atan2(sqrt(a), sqrt(1.0 - a));
}

bool segmentsIntersect(const Vec2& p1, const Vec2& p2,
                       const Vec2& q1, const Vec2& q2, float& tOut) {
    const float rx = p2.x - p1.x, ry = p2.y - p1.y;
    const float sx = q2.x - q1.x, sy = q2.y - q1.y;
    const float denom = rx * sy - ry * sx;
    if (fabsf(denom) < 1e-9f) return false;  // parallel

    const float qpx = q1.x - p1.x, qpy = q1.y - p1.y;
    const float t = (qpx * sy - qpy * sx) / denom;
    const float u = (qpx * ry - qpy * rx) / denom;
    if (t < 0.0f || t > 1.0f || u < 0.0f || u > 1.0f) return false;
    tOut = t;
    return true;
}

// Algoritmo "days from civil" (Howard Hinnant) — evita depender de timegm().
static int64_t daysFromCivil(int y, unsigned m, unsigned d) {
    y -= m <= 2;
    const int era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = (unsigned)(y - era * 400);
    const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return (int64_t)era * 146097 + (int64_t)doe - 719468;
}

uint64_t toEpochMs(int year, int month, int day,
                   int hour, int minute, int second, int centisecond) {
    int64_t days = daysFromCivil(year, (unsigned)month, (unsigned)day);
    int64_t secs = days * 86400 + hour * 3600 + minute * 60 + second;
    return (uint64_t)secs * 1000ULL + (uint64_t)centisecond * 10ULL;
}
