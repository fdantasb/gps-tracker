#pragma once
#include <Arduino.h>
#include "../config.h"
#include "../geo/geo.h"

// Lap timer based on a virtual start line.
//
// The line is a segment of 2*START_LINE_HALF_WIDTH_M perpendicular to the
// car's course at the moment of marking. Crossing = intersection of the
// segment (previous fix -> current fix) with the line, with temporal
// interpolation at the crossing fraction (sub-fix precision even at 10 Hz).
//
// Sectors: lap 1 defines the reference distance; it is split into
// NUM_SECTORS equal stretches. The ideal lap = sum of the best sectors.
// During lap 1, checkpoints (distance, time) are recorded to compute
// lap 1's own sectors retroactively.

struct Lap {
    uint32_t timeMs = 0;
    uint32_t sectorMs[NUM_SECTORS] = {0};
};

class LapTimer {
public:
    // Marks the start line at the current position/course and begins lap 1.
    void setStartLine(double lat, double lon, float courseDeg, uint64_t epochMs);

    // Feeds a new fix. Returns true if a lap was completed.
    bool onFix(double lat, double lon, uint64_t epochMs);

    bool     isRunning() const { return running_; }
    uint16_t lapCount() const { return lapCount_; }
    uint32_t currentLapMs(uint64_t nowEpochMs) const;
    const Lap& lap(uint16_t i) const { return laps_[i]; }

    // Statistics (valid with lapCount() >= 1)
    uint32_t bestLapMs() const;
    uint32_t worstLapMs() const;
    uint32_t avgLapMs() const;
    uint16_t bestLapIndex() const;
    uint32_t idealLapMs() const;                    // sum of the best sectors
    uint32_t bestSectorMs(uint8_t s) const;

    void stop() { running_ = false; }

private:
    void beginLap(uint64_t epochMs);
    void closeSector(uint8_t idx, uint64_t epochMs);
    void finishLap(uint64_t crossEpochMs);
    void recordCheckpoint(float dist, uint64_t epochMs);
    void backfillLap1Sectors(uint64_t crossEpochMs);

    // Start line (ENU coords, ref = center of the line)
    GeoProjector proj_;
    Vec2  lineA_, lineB_;
    Vec2  lineDir_;               // valid crossing direction (unit vector)
    bool  lineSet_ = false;
    bool  running_ = false;

    // Previous fix
    Vec2     prevPos_;
    uint64_t prevEpochMs_ = 0;
    bool     hasPrev_ = false;

    // Current lap
    uint64_t lapStartMs_ = 0;
    float    lapDist_ = 0;        // distance accumulated in the lap (m)
    uint8_t  nextSector_ = 0;     // next sector boundary to cross
    uint64_t lastSectorMs_ = 0;   // epoch of the last sector's close

    // Sector reference (defined by lap 1)
    float refLapDist_ = 0;

    // Lap 1 checkpoints for sector backfill
    static constexpr uint16_t kMaxCkpts = 512;
    static constexpr float    kCkptStride = 20.0f;  // m
    struct Ckpt { float dist; uint64_t tMs; };
    Ckpt     ckpts_[kMaxCkpts];
    uint16_t ckptCount_ = 0;

    Lap      curLap_;
    Lap      laps_[MAX_LAPS];
    uint16_t lapCount_ = 0;
};
