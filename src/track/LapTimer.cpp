#include "LapTimer.h"

static constexpr double kDeg2Rad = M_PI / 180.0;

void LapTimer::setStartLine(double lat, double lon, float courseDeg, uint64_t epochMs) {
    proj_.setRef(lat, lon);

    // GPS course: 0° = North, clockwise. In ENU: dir = (sin, cos).
    const float h = courseDeg * (float)kDeg2Rad;
    lineDir_ = { sinf(h), cosf(h) };
    const Vec2 perp = { -lineDir_.y, lineDir_.x };
    lineA_ = { perp.x * START_LINE_HALF_WIDTH_M,  perp.y * START_LINE_HALF_WIDTH_M };
    lineB_ = { -lineA_.x, -lineA_.y };

    lineSet_ = true;
    running_ = true;
    hasPrev_ = false;
    lapCount_ = 0;
    refLapDist_ = 0;
    ckptCount_ = 0;
    beginLap(epochMs);
}

void LapTimer::beginLap(uint64_t epochMs) {
    lapStartMs_   = epochMs;
    lastSectorMs_ = epochMs;
    lapDist_      = 0;
    nextSector_   = 0;
    curLap_ = Lap{};
}

uint32_t LapTimer::currentLapMs(uint64_t nowEpochMs) const {
    if (!running_ || nowEpochMs <= lapStartMs_) return 0;
    return (uint32_t)(nowEpochMs - lapStartMs_);
}

void LapTimer::recordCheckpoint(float dist, uint64_t epochMs) {
    if (ckptCount_ >= kMaxCkpts) return;
    if (ckptCount_ > 0 && dist < ckpts_[ckptCount_ - 1].dist + kCkptStride) return;
    ckpts_[ckptCount_++] = { dist, epochMs };
}

void LapTimer::closeSector(uint8_t idx, uint64_t epochMs) {
    if (idx >= NUM_SECTORS) return;
    curLap_.sectorMs[idx] = (uint32_t)(epochMs - lastSectorMs_);
    lastSectorMs_ = epochMs;
}

bool LapTimer::onFix(double lat, double lon, uint64_t epochMs) {
    if (!running_ || !lineSet_) return false;

    const Vec2 pos = proj_.project(lat, lon);
    if (!hasPrev_) {
        prevPos_ = pos;
        prevEpochMs_ = epochMs;
        hasPrev_ = true;
        return false;
    }

    const float dx = pos.x - prevPos_.x, dy = pos.y - prevPos_.y;
    const float segLen = sqrtf(dx * dx + dy * dy);
    const uint64_t dtMs = epochMs - prevEpochMs_;
    lapDist_ += segLen;

    if (lapCount_ == 0) recordCheckpoint(lapDist_, epochMs);

    // Sector boundaries (available from lap 2 onward).
    while (refLapDist_ > 0 && nextSector_ < NUM_SECTORS - 1) {
        const float boundary = refLapDist_ * (float)(nextSector_ + 1) / NUM_SECTORS;
        if (lapDist_ < boundary) break;
        const float frac = segLen > 0.01f
            ? 1.0f - (lapDist_ - boundary) / segLen : 1.0f;
        closeSector(nextSector_, prevEpochMs_ + (uint64_t)(frac * dtMs));
        nextSector_++;
    }

    // Start line crossing.
    bool completed = false;
    float t;
    if (segmentsIntersect(prevPos_, pos, lineA_, lineB_, t)) {
        const bool rightWay = (dx * lineDir_.x + dy * lineDir_.y) > 0;
        const uint64_t crossMs = prevEpochMs_ + (uint64_t)(t * dtMs);
        if (rightWay && crossMs - lapStartMs_ >= MIN_LAP_TIME_MS) {
            finishLap(crossMs);
            lapDist_ = segLen * (1.0f - t);  // remainder of the segment belongs to the new lap
            completed = true;
        }
    }

    prevPos_ = pos;
    prevEpochMs_ = epochMs;
    return completed;
}

void LapTimer::finishLap(uint64_t crossEpochMs) {
    curLap_.timeMs = (uint32_t)(crossEpochMs - lapStartMs_);

    if (lapCount_ == 0) {
        refLapDist_ = lapDist_;
        backfillLap1Sectors(crossEpochMs);
    } else {
        while (nextSector_ < NUM_SECTORS - 1) closeSector(nextSector_++, crossEpochMs);
        curLap_.sectorMs[NUM_SECTORS - 1] = (uint32_t)(crossEpochMs - lastSectorMs_);
    }

    if (lapCount_ < MAX_LAPS) laps_[lapCount_++] = curLap_;
    beginLap(crossEpochMs);
}

// Interpolates the instant when lap 1 reached each sector boundary.
void LapTimer::backfillLap1Sectors(uint64_t crossEpochMs) {
    uint64_t tPrev = lapStartMs_;
    for (uint8_t k = 1; k < NUM_SECTORS; ++k) {
        const float boundary = refLapDist_ * (float)k / NUM_SECTORS;
        uint64_t tB = 0;
        float dLo = 0; uint64_t tLo = lapStartMs_;
        for (uint16_t i = 0; i < ckptCount_; ++i) {
            if (ckpts_[i].dist >= boundary) {
                const float span = ckpts_[i].dist - dLo;
                const float frac = span > 0.01f ? (boundary - dLo) / span : 1.0f;
                tB = tLo + (uint64_t)(frac * (ckpts_[i].tMs - tLo));
                break;
            }
            dLo = ckpts_[i].dist;
            tLo = ckpts_[i].tMs;
        }
        if (tB == 0) { curLap_.sectorMs[k - 1] = 0; continue; }  // no data
        curLap_.sectorMs[k - 1] = (uint32_t)(tB - tPrev);
        tPrev = tB;
    }
    curLap_.sectorMs[NUM_SECTORS - 1] = (uint32_t)(crossEpochMs - tPrev);
}

uint32_t LapTimer::bestLapMs() const {
    uint32_t best = UINT32_MAX;
    for (uint16_t i = 0; i < lapCount_; ++i) best = min(best, laps_[i].timeMs);
    return lapCount_ ? best : 0;
}

uint16_t LapTimer::bestLapIndex() const {
    uint16_t idx = 0;
    for (uint16_t i = 1; i < lapCount_; ++i)
        if (laps_[i].timeMs < laps_[idx].timeMs) idx = i;
    return idx;
}

uint32_t LapTimer::worstLapMs() const {
    uint32_t worst = 0;
    for (uint16_t i = 0; i < lapCount_; ++i) worst = max(worst, laps_[i].timeMs);
    return worst;
}

uint32_t LapTimer::avgLapMs() const {
    if (!lapCount_) return 0;
    uint64_t sum = 0;
    for (uint16_t i = 0; i < lapCount_; ++i) sum += laps_[i].timeMs;
    return (uint32_t)(sum / lapCount_);
}

uint32_t LapTimer::bestSectorMs(uint8_t s) const {
    if (s >= NUM_SECTORS) return 0;
    uint32_t best = UINT32_MAX;
    for (uint16_t i = 0; i < lapCount_; ++i)
        if (laps_[i].sectorMs[s] > 0) best = min(best, laps_[i].sectorMs[s]);
    return best == UINT32_MAX ? 0 : best;
}

uint32_t LapTimer::idealLapMs() const {
    uint32_t sum = 0;
    for (uint8_t s = 0; s < NUM_SECTORS; ++s) {
        const uint32_t b = bestSectorMs(s);
        if (b == 0) return 0;  // insufficient data
        sum += b;
    }
    return sum;
}
