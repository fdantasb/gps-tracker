#include "GpsService.h"
#include "../config.h"
#include "../geo/geo.h"

void GpsService::begin() {
    serial_.begin(GPS_BAUD, SERIAL_8N1, GPS_UART_RX_PIN, GPS_UART_TX_PIN);
    delay(100);

    // Taxa de atualização: 10 Hz (PCAS02, período em ms).
    sendCasic("PCAS02,100");

    // Habilita apenas GGA (1º campo) e RMC (5º campo) — menos parsing/tráfego.
    // Campos: GGA,GLL,GSA,GSV,RMC,VTG,ZDA,ANT,DHV,LPS,res,res,UTC,GST
    sendCasic("PCAS03,1,0,0,0,1,0,0,0,0,0,,,0,0");
}

void GpsService::sendCasic(const char* body) {
    uint8_t cs = 0;
    for (const char* p = body; *p; ++p) cs ^= (uint8_t)*p;
    char buf[96];
    snprintf(buf, sizeof(buf), "$%s*%02X\r\n", body, cs);
    serial_.print(buf);
}

bool GpsService::update() {
    bool encoded = false;
    while (serial_.available()) {
        if (gps_.encode(serial_.read())) encoded = true;
    }
    if (!encoded) return false;

    // Só considera "fix novo" quando a posição foi atualizada nesta rodada.
    if (!gps_.location.isUpdated()) return false;

    fix_.valid = gps_.location.isValid() && gps_.date.isValid() && gps_.time.isValid();
    if (!fix_.valid) return false;

    fix_.lat       = gps_.location.lat();
    fix_.lon       = gps_.location.lng();
    fix_.altitudeM = gps_.altitude.isValid() ? (float)gps_.altitude.meters() : 0.0f;
    fix_.speedKmh  = gps_.speed.isValid() ? (float)gps_.speed.kmph() : 0.0f;
    fix_.courseDeg = gps_.course.isValid() ? (float)gps_.course.deg() : fix_.courseDeg;
    fix_.hdop      = gps_.hdop.isValid() ? (float)gps_.hdop.hdop() : 99.0f;
    fix_.sats      = (uint8_t)gps_.satellites.value();
    fix_.epochMs   = toEpochMs(gps_.date.year(), gps_.date.month(), gps_.date.day(),
                               gps_.time.hour(), gps_.time.minute(),
                               gps_.time.second(), gps_.time.centisecond());

    // GGA e RMC do mesmo ciclo carregam o mesmo timestamp — reporta só uma vez.
    if (fix_.epochMs == lastEpochMs_) return false;
    lastEpochMs_ = fix_.epochMs;
    return true;
}
