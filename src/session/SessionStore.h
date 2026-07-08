#pragma once
#include <Arduino.h>
#include "../config.h"
#include "../track/LapTimer.h"
#include "../track/TripRecorder.h"
#include "../car/CarRegistry.h"

// Persistência dos resultados de cada sessão em um arquivo .ses (texto)
// ao lado do GPX. O GPX guarda só a trilha; o .ses guarda os tempos —
// é o que permite reabrir corridas antigas sem recalcular nada.
//
// Formato (linhas):
//   R | T                      tipo
//   car <nome>
//   gpx <caminho>
//   L <ms> <s1> <s2> <s3>      uma por volta (corrida)
//   S <durMs> <distM> <maxKmh_x10> <avgKmh_x10>   (trajeto)

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
    char file[44] = "";        // nome do arquivo (sem diretório)
    char label[40] = "";       // "03/07 14:32 COR Civic-Type-R"
    bool isRace = false;
    bool needsImport = false;  // .gpx órfão (sem .ses): reprocessar ao abrir
};

class SessionStore {
public:
    static void buildRaceResult(const LapTimer& lt, const char* car,
                                const char* gpxPath, RaceResult& out);
    static void buildTripResult(const TripRecorder& tr, const char* car,
                                const char* gpxPath, TripResult& out);

    // Gravam o .ses derivando o caminho do gpxPath (vazio = não grava).
    static void save(const RaceResult& r);
    static void save(const TripResult& t);

    // Lista os .ses do SD, mais recentes primeiro. Retorna a quantidade.
    static uint8_t list(SessionEntry* out, uint8_t maxEntries);

    static bool load(const char* file, RaceResult& out);   // arquivos R
    static bool load(const char* file, TripResult& out);   // arquivos T

private:
    static void computeStats(RaceResult& r);
    static bool sesPathFromGpx(const char* gpxPath, char* out, size_t n);
    static void makeLabel(const char* file, bool isRace, char* out, size_t n);
};
