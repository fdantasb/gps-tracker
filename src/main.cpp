// ============================================================
// GPS Tracker — M5 Cardputer ADV + Cap LoRa-1262
// Modos: Corrida (voltas, setores, volta ideal) e Trajeto
// (distância, tempo, vel. máx/média). Sempre grava GPX no SD,
// mais um resumo .ses que alimenta o Histórico.
// ============================================================
#include <M5Cardputer.h>
#include "config.h"
#include "gps/GpsService.h"
#include "track/LapTimer.h"
#include "track/TripRecorder.h"
#include "log/GpxWriter.h"
#include "car/CarRegistry.h"
#include "session/SessionStore.h"
#include "session/GpxImport.h"
#include "i18n/Strings.h"
#include "ui/Ui.h"

enum class State {
    SPLASH,
    CAR_SELECT,     // lista de carros já usados (MRU)
    CAR_INPUT,      // digitação de carro novo
    MENU,
    HISTORY_LIST,   // sessões passadas (.ses no SD)
    WAIT_FIX_RACE,
    WAIT_FIX_TRIP,
    RACE_ARM,       // dirigindo até a linha de largada
    RACE_RUNNING,
    RACE_RESULTS,   // resumo (sessão recém-encerrada ou histórico)
    RACE_LAPS,      // navegação volta a volta
    TRIP_RUNNING,
    TRIP_RESULTS,
};

static constexpr uint32_t SPLASH_MS = 3000;

static GpsService   gps;
static LapTimer     lapTimer;
static TripRecorder trip;
static GpxWriter    gpx;
static CarRegistry  cars;
static Ui           ui;

static State    state = State::SPLASH;
static bool     sdOk = false;
static uint32_t lastUiMs = 0;
static uint32_t lastGpxLogMs = 0;
static uint32_t splashStartMs = 0;

// Navegação do menu principal (índice do item destacado)
static uint8_t menuSel = 0;

// Seleção/digitação de carro
static uint8_t carSel = 0;
static char    carInput[CAR_NAME_LEN + 1] = "";
static uint8_t carInputLen = 0;

// Resultados (da sessão ao vivo ou carregados do histórico)
static RaceResult   raceRes;
static TripResult   tripRes;
static SessionEntry sessions[MAX_SESSIONS];
static uint8_t      sessionCount = 0, sessionSel = 0;
static uint16_t     lapViewIdx = 0;
static bool         fromHistory = false;   // resultados abertos pelo histórico?

// Delta da volta recém-completada vs. melhor anterior (destaque de 7 s).
static int32_t  lapDeltaMs = 0;
static uint32_t deltaUntilMs = 0;

// Relógio estimado entre fixes (para o cronômetro correr suave na tela).
static uint64_t lastFixEpochMs = 0;
static uint32_t lastFixMillis = 0;

static uint64_t nowEpochMs() {
    if (lastFixEpochMs == 0) return 0;
    return lastFixEpochMs + (millis() - lastFixMillis);
}

// Abre o GPX da sessão: /gpx/DATA_HORA_<carro>_<modo>.gpx
static void openSessionGpx(const GpsFix& fix, const char* mode, const char* modeLabel) {
    lastGpxLogMs = 0;
    if (!sdOk) return;
    char san[CAR_NAME_LEN + 1];
    CarRegistry::sanitize(cars.current(), san, sizeof(san));
    char suffix[CAR_NAME_LEN + 8];
    snprintf(suffix, sizeof(suffix), "%s_%s", san, mode);
    char trackName[CAR_NAME_LEN + 16];
    snprintf(trackName, sizeof(trackName), "%s - %s", modeLabel, cars.current());
    gpx.open(fix.epochMs, suffix, trackName);
}

static void startTrip(const GpsFix& fix) {
    trip.start(fix.epochMs);
    openSessionGpx(fix, "trip", "Trajeto");
    state = State::TRIP_RUNNING;
}

static void stopRace() {
    lapTimer.stop();
    const char* gpxPath = gpx.isOpen() ? gpx.close() : "";
    SessionStore::buildRaceResult(lapTimer, cars.current(), gpxPath, raceRes);
    if (sdOk) SessionStore::save(raceRes);
    fromHistory = false;
    state = State::RACE_RESULTS;
}

static void stopTrip() {
    trip.stop();
    const char* gpxPath = gpx.isOpen() ? gpx.close() : "";
    SessionStore::buildTripResult(trip, cars.current(), gpxPath, tripRes);
    if (sdOk) SessionStore::save(tripRes);
    fromHistory = false;
    state = State::TRIP_RESULTS;
}

static void openHistory() {
    sessionCount = sdOk ? SessionStore::list(sessions, MAX_SESSIONS) : 0;
    sessionSel = 0;
    state = State::HISTORY_LIST;
}

static void openHistoryEntry() {
    if (sessionSel >= sessionCount) return;
    SessionEntry& e = sessions[sessionSel];

    bool ok;
    if (e.needsImport) {
        // GPX órfão (gravado antes do histórico): reprocessa e gera o .ses.
        ui.drawBusy(L().busyImport);
        ok = e.isRace ? GpxImport::importRace(e.file, lapTimer, raceRes)
                      : GpxImport::importTrip(e.file, tripRes);
        if (ok) {
            // Entrada passa a apontar pro .ses recém-criado.
            strcpy(e.file + strlen(e.file) - 4, ".ses");
            e.needsImport = false;
            const size_t l = strlen(e.label);
            if (l >= 2 && strcmp(e.label + l - 2, " *") == 0) e.label[l - 2] = 0;
        } else {
            ui.drawError(L().errImport);
            delay(1500);
            return;
        }
    } else {
        ok = e.isRace ? SessionStore::load(e.file, raceRes)
                      : SessionStore::load(e.file, tripRes);
        if (!ok) return;
    }

    fromHistory = true;
    state = e.isRace ? State::RACE_RESULTS : State::TRIP_RESULTS;
}

static void leaveResults() {
    state = fromHistory ? State::HISTORY_LIST : State::MENU;
}

static void confirmCarInput() {
    if (carInputLen == 0) return;
    cars.use(carInput);
    state = State::MENU;
}

// Executa o item destacado do menu (por seta+ENTER ou por atalho de letra).
static void activateMenu(const GpsFix& fix) {
    switch (menuSel) {
        case 0: state = fix.valid ? State::RACE_ARM : State::WAIT_FIX_RACE; break;
        case 1: if (fix.valid) startTrip(fix); else state = State::WAIT_FIX_TRIP; break;
        case 2: openHistory(); break;
        case 3: carSel = 0; state = State::CAR_SELECT; break;
        case 4: langToggle(); break;   // idioma: alterna e permanece no menu
    }
}

static void handleKey(char raw) {
    const GpsFix& fix = gps.fix();
    char c = (char)tolower((unsigned char)raw);

    // Setas do Cardputer: ; cima  . baixo  , esquerda  / direita.
    // Esquerda = voltar, direita = selecionar — em toda tela exceto a
    // digitação de carro (onde ',' e '/' são caracteres do nome).
    if (state != State::CAR_INPUT) {
        if (raw == ',') c = '`';    // voltar (igual a Esc/`)
        if (raw == '/') c = '\n';   // selecionar (igual a ENTER)
    }

    switch (state) {
        case State::SPLASH:
            break;  // ignora teclas

        case State::CAR_SELECT: {
            const uint8_t total = cars.count() + 1;  // + "novo carro"
            if (c == ';' && carSel > 0) carSel--;
            if (c == '.' && carSel < total - 1) carSel++;
            if (c == '\n') {
                if (carSel < cars.count()) {
                    cars.use(cars.name(carSel));
                    state = State::MENU;
                } else {
                    carInput[0] = 0;
                    carInputLen = 0;
                    state = State::CAR_INPUT;
                }
            }
            break;
        }

        case State::CAR_INPUT:
            if (c == '\n') confirmCarInput();
            else if (raw == '\b') {
                if (carInputLen > 0) carInput[--carInputLen] = 0;
            } else if (c == '`') {
                if (cars.count() > 0) { carSel = 0; state = State::CAR_SELECT; }
            } else if (isprint((unsigned char)raw) && carInputLen < CAR_NAME_LEN) {
                carInput[carInputLen++] = raw;   // preserva maiúsculas
                carInput[carInputLen] = 0;
            }
            break;

        case State::MENU:
            // Navegação por setas + ENTER.
            if (c == ';' && menuSel > 0) menuSel--;
            if (c == '.' && menuSel < Ui::MENU_ITEMS - 1) menuSel++;
            if (c == '\n') activateMenu(fix);
            // Atalhos diretos por letra (aceleradores): posicionam e ativam.
            if (c == 'r') { menuSel = 0; activateMenu(fix); }
            if (c == 't') { menuSel = 1; activateMenu(fix); }
            if (c == 'h') { menuSel = 2; activateMenu(fix); }
            if (c == 'c') { menuSel = 3; activateMenu(fix); }
            if (c == 'l') { menuSel = 4; activateMenu(fix); }
            break;

        case State::HISTORY_LIST:
            if (c == '`') state = State::MENU;
            if (sessionCount > 0) {
                if (c == ';' && sessionSel > 0) sessionSel--;
                if (c == '.' && sessionSel < sessionCount - 1) sessionSel++;
                if (c == '\n') openHistoryEntry();
            }
            break;

        case State::WAIT_FIX_RACE:
        case State::WAIT_FIX_TRIP:
            if (c == '`') state = State::MENU;
            break;

        case State::RACE_ARM:
            if (c == '`') state = State::MENU;
            if (c == '\n' && fix.valid && fix.speedKmh >= MIN_COURSE_SPEED_KMH) {
                lapTimer.setStartLine(fix.lat, fix.lon, fix.courseDeg, fix.epochMs);
                deltaUntilMs = 0;
                openSessionGpx(fix, "race", "Corrida");
                state = State::RACE_RUNNING;
            }
            break;

        case State::RACE_RUNNING:
            if (c == 's') stopRace();
            break;

        case State::TRIP_RUNNING:
            if (c == 's') stopTrip();
            break;

        case State::RACE_RESULTS:
            if (c == 'v' && raceRes.lapCount > 0) {
                lapViewIdx = raceRes.bestIdx;   // começa na melhor volta
                state = State::RACE_LAPS;
            }
            if (c == '\n' || c == '`') leaveResults();
            break;

        case State::RACE_LAPS:
            if (c == ';' && lapViewIdx > 0) lapViewIdx--;
            if (c == '.' && lapViewIdx < raceRes.lapCount - 1) lapViewIdx++;
            if (c == '`' || c == '\n') state = State::RACE_RESULTS;
            break;

        case State::TRIP_RESULTS:
            if (c == '\n' || c == '`') leaveResults();
            break;
    }
}

static void pollKeyboard() {
    if (!M5Cardputer.Keyboard.isChange() || !M5Cardputer.Keyboard.isPressed()) return;
    auto ks = M5Cardputer.Keyboard.keysState();
    if (ks.enter) handleKey('\n');
    if (ks.del) handleKey('\b');
    if (ks.space) handleKey(' ');
    for (char c : ks.word) handleKey(c);
}

static void onNewFix(const GpsFix& fix) {
    lastFixEpochMs = fix.epochMs;
    lastFixMillis = millis();

    // Transições automáticas ao ganhar fix.
    if (state == State::WAIT_FIX_RACE) state = State::RACE_ARM;
    if (state == State::WAIT_FIX_TRIP) { startTrip(fix); return; }

    if (state == State::RACE_RUNNING) {
        // Melhor volta ANTES desta (referência do delta).
        const uint32_t prevBest = lapTimer.lapCount() > 0 ? lapTimer.bestLapMs() : 0;
        if (lapTimer.onFix(fix.lat, fix.lon, fix.epochMs) && prevBest > 0) {
            const uint32_t lastLap = lapTimer.lap(lapTimer.lapCount() - 1).timeMs;
            lapDeltaMs = (int32_t)lastLap - (int32_t)prevBest;
            deltaUntilMs = millis() + DELTA_SHOW_MS;
        }
    } else if (state == State::TRIP_RUNNING) {
        trip.onFix(fix.lat, fix.lon, fix.speedKmh, fix.epochMs);
    }

    // Log GPX (intervalo depende do modo).
    if (gpx.isOpen()) {
        const uint32_t interval = (state == State::RACE_RUNNING)
            ? GPX_RACE_INTERVAL_MS : GPX_TRIP_INTERVAL_MS;
        if (millis() - lastGpxLogMs >= interval) {
            gpx.addPoint(fix.lat, fix.lon, fix.altitudeM, fix.speedKmh, fix.epochMs);
            lastGpxLogMs = millis();
        }
    }
}

static void refreshUi() {
    if (millis() - lastUiMs < UI_REFRESH_MS) return;
    lastUiMs = millis();

    GpsFix fix = gps.fix();
    fix.epochMs = nowEpochMs();  // cronômetro suave entre fixes

    switch (state) {
        case State::SPLASH:        ui.drawSplash(); break;
        case State::CAR_SELECT:    ui.drawCarSelect(cars, carSel); break;
        case State::CAR_INPUT:     ui.drawCarInput(carInput, cars.count() > 0); break;
        case State::MENU:          ui.drawMenu(gps.fix(), sdOk, cars.current(), menuSel); break;
        case State::HISTORY_LIST:  ui.drawHistoryList(sessions, sessionCount, sessionSel); break;
        case State::WAIT_FIX_RACE:
        case State::WAIT_FIX_TRIP: ui.drawWaitFix(gps.satsInView(), gps.fix().hdop); break;
        case State::RACE_ARM:      ui.drawRaceArm(gps.fix()); break;
        case State::RACE_RUNNING:
            ui.drawRaceLive(fix, lapTimer, lapDeltaMs, millis() < deltaUntilMs);
            break;
        case State::RACE_RESULTS:  ui.drawRaceResults(raceRes); break;
        case State::RACE_LAPS:     ui.drawLapDetail(raceRes, lapViewIdx); break;
        case State::TRIP_RUNNING:  ui.drawTripLive(gps.fix(), trip); break;
        case State::TRIP_RESULTS:  ui.drawTripResults(tripRes); break;
    }
}

void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);
    ui.begin();
    ui.drawSplash();             // splash cobre o tempo de init de SD/GPS
    splashStartMs = millis();
    sdOk = GpxWriter::initSd();
    if (sdOk) { langLoad(); cars.load(); }
    gps.begin();
}

void loop() {
    M5Cardputer.update();
    pollKeyboard();
    if (gps.update()) onNewFix(gps.fix());

    if (state == State::SPLASH && millis() - splashStartMs >= SPLASH_MS) {
        carSel = 0;
        state = cars.count() > 0 ? State::CAR_SELECT : State::CAR_INPUT;
    }

    refreshUi();
    delay(5);
}
