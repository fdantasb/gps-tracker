#include "GpxImport.h"
#include <SD.h>
#include "../geo/geo.h"
#include "../config.h"

static bool readLine(File& f, char* buf, size_t n) {
    size_t len = 0;
    bool any = false;
    while (f.available()) {
        any = true;
        const char c = (char)f.read();
        if (c == '\n') break;
        if (c != '\r' && len < n - 1) buf[len++] = c;
    }
    buf[len] = 0;
    return any;
}

// Extrai lat/lon/speed/time de uma linha <trkpt .../> gerada pelo GpxWriter.
bool GpxImport::parsePoint(const char* line, double& lat, double& lon,
                           float& speedKmh, uint64_t& epochMs) {
    const char* p = strstr(line, "lat=\"");
    if (!p) return false;
    lat = atof(p + 5);
    p = strstr(line, "lon=\"");
    if (!p) return false;
    lon = atof(p + 5);

    p = strstr(line, "<time>");
    if (!p) return false;
    int y, mo, d, h, mi, s, ms = 0;
    if (sscanf(p + 6, "%4d-%2d-%2dT%2d:%2d:%2d.%3d", &y, &mo, &d, &h, &mi, &s, &ms) < 6)
        return false;
    epochMs = toEpochMs(y, mo, d, h, mi, s, 0) + (uint64_t)ms;

    speedKmh = 0;
    p = strstr(line, "<speed>");
    if (p) speedKmh = (float)atof(p + 7) * 3.6f;   // m/s -> km/h
    return true;
}

// "YYYYMMDD_HHMMSS_<carro>_<modo>.gpx" -> carro (sem des-sanitizar).
void GpxImport::carFromFilename(const char* file, char* out, size_t n) {
    out[0] = 0;
    if (strlen(file) <= 20) { snprintf(out, n, "?"); return; }
    const char* start = file + 16;
    const char* end = strrchr(file, '_');
    if (!end || end <= start) { snprintf(out, n, "?"); return; }
    const size_t len = min((size_t)(end - start), n - 1);
    memcpy(out, start, len);
    out[len] = 0;
}

bool GpxImport::importRace(const char* file, LapTimer& lt, RaceResult& out) {
    char path[80];
    snprintf(path, sizeof(path), GPX_DIR "/%s", file);
    File f = SD.open(path, FILE_READ);
    if (!f) return false;

    char line[224];
    double lat, lon;
    float spd;
    uint64_t t;

    // Passo 1: primeiro ponto (linha de largada) e direção inicial (>= 8 m).
    double lat0 = 0, lon0 = 0;
    uint64_t t0 = 0;
    bool have0 = false;
    float course = -1;
    GeoProjector proj;
    while (readLine(f, line, sizeof(line))) {
        if (!parsePoint(line, lat, lon, spd, t)) continue;
        if (!have0) {
            lat0 = lat; lon0 = lon; t0 = t;
            proj.setRef(lat0, lon0);
            have0 = true;
            continue;
        }
        const Vec2 v = proj.project(lat, lon);
        if (v.x * v.x + v.y * v.y >= 8.0f * 8.0f) {
            course = atan2f(v.x, v.y) * 180.0f / (float)M_PI;
            if (course < 0) course += 360.0f;
            break;
        }
    }
    if (!have0 || course < 0) { f.close(); return false; }

    // Passo 2: reprocessa todos os pontos pelo cronômetro de voltas.
    lt.setStartLine(lat0, lon0, course, t0);
    f.seek(0);
    while (readLine(f, line, sizeof(line))) {
        if (parsePoint(line, lat, lon, spd, t)) lt.onFix(lat, lon, t);
    }
    f.close();
    lt.stop();

    char car[CAR_NAME_LEN + 1];
    carFromFilename(file, car, sizeof(car));
    SessionStore::buildRaceResult(lt, car, path, out);
    if (out.lapCount == 0) return false;
    SessionStore::save(out);   // próxima abertura lê o .ses direto
    return true;
}

bool GpxImport::importTrip(const char* file, TripResult& out) {
    char path[80];
    snprintf(path, sizeof(path), GPX_DIR "/%s", file);
    File f = SD.open(path, FILE_READ);
    if (!f) return false;

    char line[224];
    double lat, lon, prevLat = 0, prevLon = 0;
    float spd;
    uint64_t t, tFirst = 0, tLast = 0;
    double distM = 0;
    float maxKmh = 0;
    bool havePrev = false;

    while (readLine(f, line, sizeof(line))) {
        if (!parsePoint(line, lat, lon, spd, t)) continue;
        if (tFirst == 0) tFirst = t;
        tLast = t;
        maxKmh = max(maxKmh, spd);
        if (havePrev && spd >= TRIP_MIN_SPEED_KMH)
            distM += haversineMeters(prevLat, prevLon, lat, lon);
        prevLat = lat;
        prevLon = lon;
        havePrev = true;
    }
    f.close();
    if (tFirst == 0 || tLast <= tFirst) return false;

    out = TripResult{};
    out.durationMs = (uint32_t)(tLast - tFirst);
    out.distKm = (float)(distM / 1000.0);
    out.maxKmh = maxKmh;
    out.avgKmh = (float)(distM / 1000.0) / ((float)out.durationMs / 3600000.0f);
    carFromFilename(file, out.car, sizeof(out.car));
    strncpy(out.gpxPath, path, sizeof(out.gpxPath) - 1);
    SessionStore::save(out);
    return true;
}
