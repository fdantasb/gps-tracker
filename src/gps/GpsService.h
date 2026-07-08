#pragma once
#include <Arduino.h>
#include <TinyGPSPlus.h>

// Driver do GPS do Cap LoRa-1262 (ATGM336H-6N @ AT6668).
// NMEA 0183 em UART 115200; comandos de configuração via protocolo CASIC ($PCAS).

struct GpsFix {
    bool     valid = false;
    double   lat = 0, lon = 0;
    float    altitudeM = 0;
    float    speedKmh = 0;
    float    courseDeg = 0;   // curso sobre o solo (0-360)
    float    hdop = 99.0f;
    uint8_t  sats = 0;
    uint64_t epochMs = 0;     // UTC
};

class GpsService {
public:
    void begin();

    // Consome bytes da UART. Retorna true quando um fix NOVO ficou disponível.
    bool update();

    const GpsFix& fix() const { return fix_; }
    bool hasFix() const { return fix_.valid; }
    // Não-const: TinyGPSInteger::value() limpa a flag "updated" internamente.
    uint8_t satsInView() { return (uint8_t)gps_.satellites.value(); }

private:
    void sendCasic(const char* body);  // monta $body*CS\r\n

    TinyGPSPlus gps_;
    HardwareSerial serial_{2};
    GpsFix fix_;
    uint64_t lastEpochMs_ = 0;  // dedup: GGA e RMC atualizam a posição no mesmo ciclo
};
