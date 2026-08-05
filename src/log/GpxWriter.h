#pragma once
#include <Arduino.h>
#include <SD.h>
#include "../config.h"

// Streaming GPX 1.1 writer to the microSD.
// RAM buffer flushed every GPX_FLUSH_BYTES — avoids card wear
// and write stalls at 10 Hz.

class GpxWriter {
public:
    // Sets up SPI + SD. Call once at boot. Returns false without a card.
    static bool initSd();

    // Opens /gpx/YYYYMMDD_HHMMSS_<suffix>.gpx. trackName goes in <name>.
    bool open(uint64_t epochMs, const char* suffix, const char* trackName);

    void addPoint(double lat, double lon, float eleM, float speedKmh, uint64_t epochMs);

    // Closes the tags and the file. Returns the written path.
    const char* close();

    bool isOpen() const { return open_; }
    const char* path() const { return path_; }

private:
    void append(const char* s);
    void flush(bool force);
    static void isoTime(uint64_t epochMs, char* out, size_t n);
    static void epochToCivil(uint64_t epochMs, int& y, int& mo, int& d,
                             int& h, int& mi, int& s);

    File file_;
    bool open_ = false;
    char path_[64] = {0};
    char buf_[GPX_FLUSH_BYTES + 256];
    size_t bufLen_ = 0;
};
