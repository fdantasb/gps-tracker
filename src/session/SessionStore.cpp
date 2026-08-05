#include "SessionStore.h"
#include <SD.h>

// ---------- build ----------

void SessionStore::buildRaceResult(const LapTimer& lt, const char* car,
                                   const char* gpxPath, RaceResult& out) {
    out = RaceResult{};
    out.lapCount = lt.lapCount();
    for (uint16_t i = 0; i < out.lapCount; ++i) out.laps[i] = lt.lap(i);
    strncpy(out.car, car, CAR_NAME_LEN);
    strncpy(out.gpxPath, gpxPath, sizeof(out.gpxPath) - 1);
    computeStats(out);
}

void SessionStore::buildTripResult(const TripRecorder& tr, const char* car,
                                   const char* gpxPath, TripResult& out) {
    out = TripResult{};
    out.durationMs = tr.durationMs();
    out.distKm = tr.distanceKm();
    out.maxKmh = tr.maxSpeedKmh();
    out.avgKmh = tr.avgSpeedKmh();
    strncpy(out.car, car, CAR_NAME_LEN);
    strncpy(out.gpxPath, gpxPath, sizeof(out.gpxPath) - 1);
}

void SessionStore::computeStats(RaceResult& r) {
    if (r.lapCount == 0) return;
    uint64_t sum = 0;
    r.bestMs = UINT32_MAX;
    r.worstMs = 0;
    r.bestIdx = 0;
    for (uint16_t i = 0; i < r.lapCount; ++i) {
        const uint32_t t = r.laps[i].timeMs;
        sum += t;
        if (t < r.bestMs) { r.bestMs = t; r.bestIdx = i; }
        if (t > r.worstMs) r.worstMs = t;
    }
    r.avgMs = (uint32_t)(sum / r.lapCount);

    r.idealMs = 0;
    bool complete = true;
    for (uint8_t s = 0; s < NUM_SECTORS; ++s) {
        uint32_t best = UINT32_MAX;
        for (uint16_t i = 0; i < r.lapCount; ++i)
            if (r.laps[i].sectorMs[s] > 0) best = min(best, r.laps[i].sectorMs[s]);
        r.bestSectorMs[s] = (best == UINT32_MAX) ? 0 : best;
        if (r.bestSectorMs[s] == 0) complete = false;
        else r.idealMs += r.bestSectorMs[s];
    }
    if (!complete) r.idealMs = 0;
}

// ---------- save ----------

bool SessionStore::sesPathFromGpx(const char* gpxPath, char* out, size_t n) {
    const size_t len = strlen(gpxPath);
    if (len < 5 || len >= n) return false;
    strncpy(out, gpxPath, n);
    strcpy(out + len - 4, ".ses");   // swap ".gpx" for ".ses"
    return true;
}

void SessionStore::save(const RaceResult& r) {
    char path[64];
    if (!sesPathFromGpx(r.gpxPath, path, sizeof(path))) return;
    File f = SD.open(path, FILE_WRITE);
    if (!f) return;
    f.println("R");
    f.printf("car %s\n", r.car);
    f.printf("gpx %s\n", r.gpxPath);
    for (uint16_t i = 0; i < r.lapCount; ++i) {
        f.printf("L %lu", (unsigned long)r.laps[i].timeMs);
        for (uint8_t s = 0; s < NUM_SECTORS; ++s)
            f.printf(" %lu", (unsigned long)r.laps[i].sectorMs[s]);
        f.println();
    }
    f.close();
}

void SessionStore::save(const TripResult& t) {
    char path[64];
    if (!sesPathFromGpx(t.gpxPath, path, sizeof(path))) return;
    File f = SD.open(path, FILE_WRITE);
    if (!f) return;
    f.println("T");
    f.printf("car %s\n", t.car);
    f.printf("gpx %s\n", t.gpxPath);
    f.printf("S %lu %lu %lu %lu\n",
             (unsigned long)t.durationMs,
             (unsigned long)(t.distKm * 1000.0f),
             (unsigned long)(t.maxKmh * 10.0f),
             (unsigned long)(t.avgKmh * 10.0f));
    f.close();
}

// ---------- list ----------

static const char* baseName(const char* p) {
    const char* s = strrchr(p, '/');
    return s ? s + 1 : p;
}

static bool endsWith(const char* s, const char* suf) {
    const size_t ls = strlen(s), lf = strlen(suf);
    return ls >= lf && strcmp(s + ls - lf, suf) == 0;
}

// "YYYYMMDD_HHMMSS_<car>_<mode>.ses" -> "DD/MM HH:MM RAC <car>"
void SessionStore::makeLabel(const char* file, bool isRace, char* out, size_t n) {
    char car[CAR_NAME_LEN + 1] = "?";
    if (strlen(file) > 20) {
        const char* start = file + 16;              // after date_time_
        const char* end = strrchr(file, '_');       // before _mode.ses
        if (end && end > start) {
            size_t len = min((size_t)(end - start), (size_t)CAR_NAME_LEN);
            memcpy(car, start, len);
            car[len] = 0;
        }
        snprintf(out, n, "%.2s/%.2s %.2s:%.2s %s %s",
                 file + 6, file + 4, file + 9, file + 11,
                 isRace ? "RAC" : "TRI", car);
    } else {
        snprintf(out, n, "%s", file);
    }
}

uint8_t SessionStore::list(SessionEntry* out, uint8_t maxEntries) {
    File dir = SD.open(GPX_DIR);
    if (!dir) return 0;
    uint8_t count = 0;

    File f;
    while ((f = dir.openNextFile())) {
        char name[44];
        strncpy(name, baseName(f.name()), sizeof(name) - 1);
        name[sizeof(name) - 1] = 0;
        f.close();

        bool needsImport = false;
        if (endsWith(name, ".gpx")) {
            // Only include it if orphaned (no matching .ses).
            char ses[80];
            snprintf(ses, sizeof(ses), GPX_DIR "/%s", name);
            const size_t l = strlen(ses);
            strcpy(ses + l - 4, ".ses");
            if (SD.exists(ses)) continue;
            needsImport = true;
        } else if (!endsWith(name, ".ses")) {
            continue;
        }

        // Descending ordered insert (date prefix => lexicographic).
        uint8_t pos = count;
        for (uint8_t i = 0; i < count; ++i) {
            if (strcmp(name, out[i].file) > 0) { pos = i; break; }
        }
        if (pos >= maxEntries) continue;
        if (count < maxEntries) count++;
        for (uint8_t i = count - 1; i > pos; --i) out[i] = out[i - 1];
        strncpy(out[pos].file, name, sizeof(out[pos].file) - 1);
        out[pos].file[sizeof(out[pos].file) - 1] = 0;
        out[pos].isRace = endsWith(name, needsImport ? "_race.gpx" : "_race.ses");
        out[pos].needsImport = needsImport;
        makeLabel(out[pos].file, out[pos].isRace, out[pos].label, sizeof(out[pos].label));
        if (needsImport) strncat(out[pos].label, " *", sizeof(out[pos].label) - strlen(out[pos].label) - 1);
    }
    dir.close();
    return count;
}

// ---------- load ----------

static bool readLine(File& f, char* buf, size_t n) {
    size_t len = 0;
    while (f.available()) {
        const char c = (char)f.read();
        if (c == '\n') { buf[len] = 0; return true; }
        if (c != '\r' && len < n - 1) buf[len++] = c;
    }
    buf[len] = 0;
    return len > 0;
}

bool SessionStore::load(const char* file, RaceResult& out) {
    char path[80];
    snprintf(path, sizeof(path), GPX_DIR "/%s", file);
    File f = SD.open(path, FILE_READ);
    if (!f) return false;

    out = RaceResult{};
    char line[96];
    if (!readLine(f, line, sizeof(line)) || line[0] != 'R') { f.close(); return false; }

    while (readLine(f, line, sizeof(line))) {
        if (strncmp(line, "car ", 4) == 0) {
            strncpy(out.car, line + 4, CAR_NAME_LEN);
        } else if (strncmp(line, "gpx ", 4) == 0) {
            strncpy(out.gpxPath, line + 4, sizeof(out.gpxPath) - 1);
        } else if (line[0] == 'L' && out.lapCount < MAX_LAPS) {
            char* p = line + 1;
            Lap& lap = out.laps[out.lapCount];
            lap.timeMs = (uint32_t)strtoul(p, &p, 10);
            for (uint8_t s = 0; s < NUM_SECTORS; ++s)
                lap.sectorMs[s] = (uint32_t)strtoul(p, &p, 10);
            out.lapCount++;
        }
    }
    f.close();
    computeStats(out);
    return out.lapCount > 0;
}

bool SessionStore::load(const char* file, TripResult& out) {
    char path[80];
    snprintf(path, sizeof(path), GPX_DIR "/%s", file);
    File f = SD.open(path, FILE_READ);
    if (!f) return false;

    out = TripResult{};
    char line[96];
    bool ok = false;
    if (!readLine(f, line, sizeof(line)) || line[0] != 'T') { f.close(); return false; }

    while (readLine(f, line, sizeof(line))) {
        if (strncmp(line, "car ", 4) == 0) {
            strncpy(out.car, line + 4, CAR_NAME_LEN);
        } else if (strncmp(line, "gpx ", 4) == 0) {
            strncpy(out.gpxPath, line + 4, sizeof(out.gpxPath) - 1);
        } else if (line[0] == 'S') {
            char* p = line + 1;
            out.durationMs = (uint32_t)strtoul(p, &p, 10);
            out.distKm = (float)strtoul(p, &p, 10) / 1000.0f;
            out.maxKmh = (float)strtoul(p, &p, 10) / 10.0f;
            out.avgKmh = (float)strtoul(p, &p, 10) / 10.0f;
            ok = true;
        }
    }
    f.close();
    return ok;
}
