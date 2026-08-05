#pragma once
#include <Arduino.h>
#include "../config.h"
#include "../track/LapTimer.h"
#include "../track/TripRecorder.h"
#include "../car/CarRegistry.h"

// Persists each session's results in a .ses (text) file
// alongside the GPX. The GPX holds only the track; the .ses holds the times —
// this is what allows old races to be reopened without recomputing anything.
//
// Format (lines):
//   R | T                      type
//   car <name>
//   gpx <path>
//   L <ms> <s1> <s2> <s3>      one per lap (race)
//   S <durMs> <distM> <maxKmh_x10> <avgKmh_x10>   (trip)

#define MAX_SESSIONS 24

struct RaceResult {
    uint16_t lapCount = 0;
    uint16_t bestIdx = 0;
    uint32_t bestMs = 0, worstMs = 0, avgMs = 0, idealMs = 0;
    uint32_t bestSectorMs[NUM_SECTORS] = {0};
    Lap      laps[MAX_LAPS];
    char     car[CAR_NAME_LEN + 1] = "";
    char     gpxPath[64] = "";
};

struct TripResult {
    uint32_t durationMs = 0;
    float    distKm = 0, maxKmh = 0, avgKmh = 0;
    char     car[CAR_NAME_LEN + 1] = "";
    char     gpxPath[64] = "";
};

struct SessionEntry {
    char file[44] = "";        // file name (without directory)
    char label[40] = "";       // "03/07 14:32 RAC Civic-Type-R"
    bool isRace = false;
    bool needsImport = false;  // orphan .gpx (no .ses): reprocess on open
};

class SessionStore {
public:
    static void buildRaceResult(const LapTimer& lt, const char* car,
                                const char* gpxPath, RaceResult& out);
    static void buildTripResult(const TripRecorder& tr, const char* car,
                                const char* gpxPath, TripResult& out);

    // Write the .ses, deriving the path from gpxPath (empty = don't write).
    static void save(const RaceResult& r);
    static void save(const TripResult& t);

    // Lists the SD's .ses files, most recent first. Returns the count.
    static uint8_t list(SessionEntry* out, uint8_t maxEntries);

    static bool load(const char* file, RaceResult& out);   // R files
    static bool load(const char* file, TripResult& out);   // T files

private:
    static void computeStats(RaceResult& r);
    static bool sesPathFromGpx(const char* gpxPath, char* out, size_t n);
    static void makeLabel(const char* file, bool isRace, char* out, size_t n);
};
