#pragma once
#include <Arduino.h>
#include <SD.h>
#include "../config.h"

// Escrita de GPX 1.1 em streaming para o microSD.
// Buffer em RAM com flush a cada GPX_FLUSH_BYTES — evita desgaste do
// cartão e travadas de escrita a 10 Hz.

class GpxWriter {
public:
    // Monta SPI + SD. Chamar uma vez no boot. Retorna false sem cartão.
    static bool initSd();

    // Abre /gpx/YYYYMMDD_HHMMSS_<sufixo>.gpx. trackName vai no <name>.
    bool open(uint64_t epochMs, const char* suffix, const char* trackName);

    void addPoint(double lat, double lon, float eleM, float speedKmh, uint64_t epochMs);

    // Fecha as tags e o arquivo. Retorna o caminho gravado.
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
