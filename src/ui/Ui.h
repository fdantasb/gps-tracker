#pragma once
#include <M5Cardputer.h>
#include "../gps/GpsService.h"
#include "../track/LapTimer.h"
#include "../track/TripRecorder.h"
#include "../car/CarRegistry.h"
#include "../session/SessionStore.h"

// Telas do app (display 240x135, desenho em canvas p/ evitar flicker).

class Ui {
public:
    void begin();

    void drawSplash();
    void drawCarSelect(const CarRegistry& cars, uint8_t sel);
    void drawCarInput(const char* text, bool canCancel);
    void drawMenu(const GpsFix& fix, bool sdOk, const char* car, uint8_t sel);
    static constexpr uint8_t MENU_ITEMS = 5;  // Corrida/Trajeto/Historico/Carro/Idioma
    void drawWaitFix(uint8_t sats, float hdop);
    void drawRaceArm(const GpsFix& fix);
    // deltaActive: mostra o delta (volta recém-completada vs. melhor anterior);
    // deltaMs < 0 = melhorou (verde), > 0 = piorou (vermelho).
    void drawRaceLive(const GpsFix& fix, const LapTimer& lt,
                      int32_t deltaMs, bool deltaActive);
    void drawRaceResults(const RaceResult& r);
    void drawLapDetail(const RaceResult& r, uint16_t lapIdx);
    void drawTripLive(const GpsFix& fix, const TripRecorder& tr);
    void drawTripResults(const TripResult& t);
    void drawHistoryList(const SessionEntry* entries, uint8_t count, uint8_t sel);
    void drawError(const char* msg);
    void drawBusy(const char* msg);   // ex.: "Importando GPX..."

    // "M:SS.mmm" / "H:MM:SS"
    static void fmtLap(uint32_t ms, char* out, size_t n);
    static void fmtClock(uint32_t ms, char* out, size_t n);

private:
    void header(const char* title, const GpsFix* fix);
    void push();
    M5Canvas canvas_{&M5Cardputer.Display};
};
