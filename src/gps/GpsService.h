#pragma once
#include <Arduino.h>
#include <TinyGPSPlus.h>

// Driver for the Cap LoRa-1262's GPS (ATGM336H-6N @ AT6668).
// NMEA 0183 over UART 115200; configuration commands via the CASIC protocol ($PCAS).

struct GpsFix {
    bool     valid = false;
    double   lat = 0, lon = 0;
    float    altitudeM = 0;
    float    speedKmh = 0;
    float    courseDeg = 0;   // course over ground (0-360)
    float    hdop = 99.0f;
    uint8_t  sats = 0;
    uint64_t epochMs = 0;     // UTC
};

class GpsService {
public:
    void begin();

    // Consumes UART bytes. Returns true when a NEW fix became available.
    bool update();

    const GpsFix& fix() const { return fix_; }
    bool hasFix() const { return fix_.valid; }
    // Non-const: TinyGPSInteger::value() clears the "updated" flag internally.
    uint8_t satsInView() { return (uint8_t)gps_.satellites.value(); }

private:
    void sendCasic(const char* body);  // builds $body*CS\r\n

    TinyGPSPlus gps_;
    HardwareSerial serial_{2};
    GpsFix fix_;
    uint64_t lastEpochMs_ = 0;  // dedup: GGA and RMC update the position in the same cycle
};
