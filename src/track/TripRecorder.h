#pragma once
#include <Arduino.h>
#include "../config.h"
#include "../geo/geo.h"

// Gravação de trajeto: distância, duração, velocidade máxima e média.

class TripRecorder {
public:
    void start(uint64_t epochMs) {
        startMs_ = epochMs;
        endMs_ = epochMs;
        distM_ = 0;
        maxKmh_ = 0;
        hasPrev_ = false;
        running_ = true;
    }

    void onFix(double lat, double lon, float speedKmh, uint64_t epochMs) {
        if (!running_) return;
        endMs_ = epochMs;
        maxKmh_ = max(maxKmh_, speedKmh);
        if (hasPrev_ && speedKmh >= TRIP_MIN_SPEED_KMH) {
            distM_ += haversineMeters(prevLat_, prevLon_, lat, lon);
        }
        prevLat_ = lat;
        prevLon_ = lon;
        hasPrev_ = true;
    }

    void stop() { running_ = false; }

    bool     isRunning() const { return running_; }
    uint32_t durationMs() const { return (uint32_t)(endMs_ - startMs_); }
    float    distanceKm() const { return (float)(distM_ / 1000.0); }
    float    maxSpeedKmh() const { return maxKmh_; }
    float    avgSpeedKmh() const {
        const uint32_t d = durationMs();
        return d > 0 ? (float)(distM_ / 1000.0) / ((float)d / 3600000.0f) : 0.0f;
    }

private:
    bool     running_ = false;
    uint64_t startMs_ = 0, endMs_ = 0;
    double   distM_ = 0;
    float    maxKmh_ = 0;
    double   prevLat_ = 0, prevLon_ = 0;
    bool     hasPrev_ = false;
};
