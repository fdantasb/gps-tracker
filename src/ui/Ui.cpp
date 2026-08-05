#include "Ui.h"
#include "../i18n/Strings.h"

static constexpr int W = 240, H = 135;
static constexpr uint16_t C_BG     = TFT_BLACK;
static constexpr uint16_t C_FG     = TFT_WHITE;
static constexpr uint16_t C_ACCENT = TFT_GREEN;
static constexpr uint16_t C_WARN   = TFT_ORANGE;
static constexpr uint16_t C_DIM    = 0x7BEF;  // gray

void Ui::begin() {
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.setBrightness(200);
    canvas_.createSprite(W, H);
}

void Ui::push() { canvas_.pushSprite(0, 0); }

void Ui::fmtLap(uint32_t ms, char* out, size_t n) {
    if (ms == 0) { snprintf(out, n, "--:--.---"); return; }
    snprintf(out, n, "%lu:%02lu.%03lu",
             (unsigned long)(ms / 60000),
             (unsigned long)((ms / 1000) % 60),
             (unsigned long)(ms % 1000));
}

void Ui::fmtClock(uint32_t ms, char* out, size_t n) {
    snprintf(out, n, "%lu:%02lu:%02lu",
             (unsigned long)(ms / 3600000),
             (unsigned long)((ms / 60000) % 60),
             (unsigned long)((ms / 1000) % 60));
}

void Ui::header(const char* title, const GpsFix* fix) {
    canvas_.fillSprite(C_BG);
    canvas_.setTextDatum(top_left);
    canvas_.setTextColor(C_DIM, C_BG);
    canvas_.setTextSize(1);
    canvas_.drawString(title, 4, 3);
    if (fix) {
        char st[32];
        snprintf(st, sizeof(st), "SAT %u  HDOP %.1f", fix->sats, fix->hdop);
        canvas_.setTextDatum(top_right);
        canvas_.setTextColor(fix->valid ? C_ACCENT : C_WARN, C_BG);
        canvas_.drawString(st, W - 4, 3);
    }
    canvas_.drawFastHLine(0, 14, W, C_DIM);
    canvas_.setTextDatum(top_left);
    canvas_.setTextColor(C_FG, C_BG);
}

void Ui::drawSplash() {
    canvas_.fillSprite(C_BG);
    canvas_.setTextDatum(middle_center);
    canvas_.setTextSize(3);
    canvas_.setTextColor(C_ACCENT, C_BG);
    canvas_.drawString("GPS Tracker", W / 2, 48);
    canvas_.setTextSize(1);
    canvas_.setTextColor(C_FG, C_BG);
    canvas_.drawString(L().splashSub, W / 2, 80);
    canvas_.setTextColor(C_DIM, C_BG);
    canvas_.drawString("Cardputer ADV + Cap LoRa-1262", W / 2, 112);
    canvas_.setTextDatum(top_left);
    push();
}

void Ui::drawCarSelect(const CarRegistry& cars, uint8_t sel) {
    header(L().carSelTitle, nullptr);
    canvas_.setTextSize(1);

    // Scroll window: 6 visible rows, entries = cars + "new".
    const uint8_t total = cars.count() + 1;
    const uint8_t rows = 6;
    uint8_t first = (sel >= rows) ? sel - rows + 1 : 0;
    int y = 22;
    for (uint8_t i = first; i < total && i < first + rows; ++i, y += 15) {
        const bool isNew = (i == cars.count());
        const char* label = isNew ? L().carSelNew : cars.name(i);
        if (i == sel) {
            canvas_.fillRect(4, y - 2, W - 8, 13, C_ACCENT);
            canvas_.setTextColor(C_BG, C_ACCENT);
        } else {
            canvas_.setTextColor(isNew ? C_DIM : C_FG, C_BG);
        }
        canvas_.drawString(label, 10, y);
    }
    canvas_.setTextColor(C_DIM, C_BG);
    canvas_.drawString(L().carSelHint, 10, 122);
    push();
}

void Ui::drawCarInput(const char* text, bool canCancel) {
    header(L().carInTitle, nullptr);
    canvas_.setTextSize(1);
    canvas_.setTextColor(C_DIM, C_BG);
    canvas_.drawString(L().carInPrompt, 10, 26);

    canvas_.drawRect(8, 44, W - 16, 26, C_DIM);
    canvas_.setTextSize(2);
    canvas_.setTextColor(C_ACCENT, C_BG);
    char shown[CAR_NAME_LEN + 2];
    snprintf(shown, sizeof(shown), "%s_", text);   // cursor
    canvas_.drawString(shown, 14, 50);

    canvas_.setTextSize(1);
    canvas_.setTextColor(C_DIM, C_BG);
    canvas_.drawString(L().carInHint, 10, 96);
    if (canCancel) canvas_.drawString(L().carInBack, 10, 110);
    push();
}

void Ui::drawMenu(const GpsFix& fix, bool sdOk, const char* car, uint8_t sel) {
    header("GPS TRACKER", &fix);
    canvas_.setTextSize(1);

    // Dynamic lines (current car / current language).
    char carLine[48], langLine[32];
    snprintf(carLine, sizeof(carLine), L().menuCarFmt, car);
    snprintf(langLine, sizeof(langLine), L().menuLangFmt,
             langCur() == LANG_EN ? "EN" : "PT");

    const char* rows[MENU_ITEMS] = {
        L().menuRace, L().menuTrip, L().menuHist, carLine, langLine,
    };

    int y = 20;
    for (uint8_t i = 0; i < MENU_ITEMS; ++i, y += 15) {
        if (i == sel) {
            canvas_.fillRect(4, y - 2, W - 8, 13, C_ACCENT);
            canvas_.setTextColor(C_BG, C_ACCENT);
        } else {
            canvas_.setTextColor(C_FG, C_BG);
        }
        canvas_.drawString(rows[i], 10, y);
    }

    canvas_.setTextColor(sdOk ? C_DIM : C_WARN, C_BG);
    canvas_.drawString(sdOk ? L().menuSdOk : L().menuSdNo, 10, 100);
    canvas_.setTextColor(C_DIM, C_BG);
    canvas_.drawString(L().carSelHint, 10, 118);
    push();
}

void Ui::drawWaitFix(uint8_t sats, float hdop) {
    header(L().waitTitle, nullptr);
    canvas_.setTextSize(2);
    canvas_.drawString(L().waitSearch, 20, 40);
    char s[40];
    snprintf(s, sizeof(s), "Sats: %u  HDOP: %.1f", sats, hdop);
    canvas_.setTextSize(1);
    canvas_.setTextColor(C_DIM, C_BG);
    canvas_.drawString(s, 20, 72);
    canvas_.drawString(L().waitCancel, 20, 118);
    push();
}

void Ui::drawRaceArm(const GpsFix& fix) {
    header(L().armTitle, &fix);
    canvas_.setTextSize(3);
    char sp[16];
    snprintf(sp, sizeof(sp), "%3.0f", fix.speedKmh);
    canvas_.drawString(sp, 20, 30);
    canvas_.setTextSize(1);
    canvas_.setTextColor(C_DIM, C_BG);
    canvas_.drawString("km/h", 90, 44);
    canvas_.setTextColor(C_ACCENT, C_BG);
    canvas_.drawString(L().armLine1, 20, 78);
    canvas_.drawString(L().armLine2, 20, 90);
    canvas_.setTextColor(C_DIM, C_BG);
    canvas_.drawString(L().back, 20, 118);
    push();
}

// "+1.234" / "-0.573" (seconds); above 60 s uses the lap format.
static void fmtDelta(int32_t ms, char* out, size_t n) {
    const uint32_t a = (uint32_t)(ms < 0 ? -ms : ms);
    if (a < 60000) {
        snprintf(out, n, "%c%lu.%03lu", ms < 0 ? '-' : '+',
                 (unsigned long)(a / 1000), (unsigned long)(a % 1000));
    } else {
        snprintf(out, n, "%c%lu:%02lu.%03lu", ms < 0 ? '-' : '+',
                 (unsigned long)(a / 60000),
                 (unsigned long)((a / 1000) % 60),
                 (unsigned long)(a % 1000));
    }
}

void Ui::drawRaceLive(const GpsFix& fix, const LapTimer& lt,
                      int32_t deltaMs, bool deltaActive) {
    header(L().raceTitle, &fix);
    char t[20], line[48];

    // Current lap time (large)
    fmtLap(lt.currentLapMs(fix.epochMs), t, sizeof(t));
    canvas_.setTextSize(3);
    canvas_.setTextColor(C_FG, C_BG);
    canvas_.drawString(t, 12, 24);

    // Speed
    char sp[16];
    snprintf(sp, sizeof(sp), "%3.0f km/h", fix.speedKmh);
    canvas_.setTextSize(2);
    canvas_.setTextColor(C_ACCENT, C_BG);
    canvas_.drawString(sp, 12, 56);

    // Delta of the completed lap vs. previous best (7 s window)
    if (deltaActive) {
        const uint16_t bg = deltaMs <= 0 ? C_ACCENT : TFT_RED;
        canvas_.fillRoundRect(120, 50, 114, 28, 4, bg);
        char d[16];
        fmtDelta(deltaMs, d, sizeof(d));
        canvas_.setTextDatum(middle_center);
        canvas_.setTextSize(2);
        canvas_.setTextColor(deltaMs <= 0 ? TFT_BLACK : TFT_WHITE, bg);
        canvas_.drawString(d, 120 + 114 / 2, 50 + 14);
        canvas_.setTextDatum(top_left);
    }

    // Current lap + last
    canvas_.setTextSize(1);
    canvas_.setTextColor(C_DIM, C_BG);
    snprintf(line, sizeof(line), L().raceLapFmt, lt.lapCount() + 1);
    canvas_.drawString(line, 12, 84);
    canvas_.setTextDatum(top_right);
    canvas_.drawString(L().stop, W - 4, 84);
    canvas_.setTextDatum(top_left);

    if (lt.lapCount() > 0) {
        fmtLap(lt.lap(lt.lapCount() - 1).timeMs, t, sizeof(t));
        snprintf(line, sizeof(line), L().raceLastFmt, t);
        canvas_.setTextColor(C_FG, C_BG);
        canvas_.drawString(line, 12, 98);

        // Best lap in permanent highlight (green band)
        fmtLap(lt.bestLapMs(), t, sizeof(t));
        snprintf(line, sizeof(line), L().raceBestFmt, lt.bestLapIndex() + 1, t);
        canvas_.fillRoundRect(8, 111, 170, 16, 3, C_ACCENT);
        canvas_.setTextColor(TFT_BLACK, C_ACCENT);
        canvas_.drawString(line, 14, 115);
    }
    push();
}

void Ui::drawRaceResults(const RaceResult& r) {
    header(L().raceResTitle, nullptr);
    char t[20], line[64];
    canvas_.setTextSize(1);

    snprintf(line, sizeof(line), L().raceResLapsFmt, r.lapCount, r.car);
    canvas_.drawString(line, 8, 20);

    fmtLap(r.bestMs, t, sizeof(t));
    snprintf(line, sizeof(line), L().raceResBestFmt, r.bestIdx + 1, t);
    canvas_.setTextColor(C_ACCENT, C_BG);
    canvas_.drawString(line, 8, 34);

    fmtLap(r.worstMs, t, sizeof(t));
    snprintf(line, sizeof(line), L().raceResWorstFmt, t);
    canvas_.setTextColor(C_WARN, C_BG);
    canvas_.drawString(line, 8, 48);

    fmtLap(r.avgMs, t, sizeof(t));
    snprintf(line, sizeof(line), L().raceResAvgFmt, t);
    canvas_.setTextColor(C_FG, C_BG);
    canvas_.drawString(line, 8, 62);

    fmtLap(r.idealMs, t, sizeof(t));
    snprintf(line, sizeof(line), L().raceResIdealFmt, t);
    canvas_.setTextColor(C_ACCENT, C_BG);
    canvas_.drawString(line, 8, 76);

    // Best sectors
    canvas_.setTextColor(C_DIM, C_BG);
    int x = 8;
    for (uint8_t s = 0; s < NUM_SECTORS; ++s) {
        fmtLap(r.bestSectorMs[s], t, sizeof(t));
        snprintf(line, sizeof(line), "S%u %s", s + 1, t);
        canvas_.drawString(line, x, 90);
        x += 78;
    }

    if (r.gpxPath[0]) {
        snprintf(line, sizeof(line), "GPX: %s", r.gpxPath);
        canvas_.drawString(line, 8, 106);
    }
    canvas_.drawString(L().raceResHint, 8, 122);
    push();
}

void Ui::drawLapDetail(const RaceResult& r, uint16_t lapIdx) {
    header(L().lapTitle, nullptr);
    char t[20], line[64];
    const Lap& lap = r.laps[lapIdx];

    canvas_.setTextSize(1);
    canvas_.setTextColor(C_DIM, C_BG);
    snprintf(line, sizeof(line), L().lapFmt, lapIdx + 1, r.lapCount);
    canvas_.drawString(line, 8, 20);

    fmtLap(lap.timeMs, t, sizeof(t));
    canvas_.setTextSize(3);
    canvas_.setTextColor(C_FG, C_BG);
    canvas_.drawString(t, 8, 34);

    // Delta vs. best lap
    canvas_.setTextSize(2);
    if (lapIdx == r.bestIdx) {
        canvas_.setTextColor(TFT_BLACK, C_ACCENT);
        canvas_.fillRoundRect(8, 64, 170, 20, 3, C_ACCENT);
        canvas_.drawString(L().lapBest, 14, 67);
    } else {
        char d[16];
        fmtDelta((int32_t)lap.timeMs - (int32_t)r.bestMs, d, sizeof(d));
        canvas_.setTextColor(TFT_RED, C_BG);
        canvas_.drawString(d, 8, 66);
    }

    // Sectors (green when it's the session's best sector)
    canvas_.setTextSize(1);
    int x = 8;
    for (uint8_t s = 0; s < NUM_SECTORS; ++s) {
        fmtLap(lap.sectorMs[s], t, sizeof(t));
        snprintf(line, sizeof(line), "S%u %s", s + 1, t);
        const bool isBest = lap.sectorMs[s] > 0 && lap.sectorMs[s] == r.bestSectorMs[s];
        canvas_.setTextColor(isBest ? C_ACCENT : C_DIM, C_BG);
        canvas_.drawString(line, x, 94);
        x += 78;
    }

    canvas_.setTextColor(C_DIM, C_BG);
    canvas_.drawString(L().lapHint, 8, 122);
    push();
}

void Ui::drawHistoryList(const SessionEntry* entries, uint8_t count, uint8_t sel) {
    header(L().histTitle, nullptr);
    canvas_.setTextSize(1);

    if (count == 0) {
        canvas_.setTextColor(C_DIM, C_BG);
        canvas_.drawString(L().histEmpty, 10, 50);
        canvas_.drawString(L().back, 10, 122);
        push();
        return;
    }

    const uint8_t rows = 6;
    uint8_t first = (sel >= rows) ? sel - rows + 1 : 0;
    int y = 22;
    for (uint8_t i = first; i < count && i < first + rows; ++i, y += 15) {
        if (i == sel) {
            canvas_.fillRect(4, y - 2, W - 8, 13, C_ACCENT);
            canvas_.setTextColor(C_BG, C_ACCENT);
        } else {
            canvas_.setTextColor(entries[i].isRace ? C_FG : C_DIM, C_BG);
        }
        canvas_.drawString(entries[i].label, 10, y);
    }
    canvas_.setTextColor(C_DIM, C_BG);
    canvas_.drawString(L().histHint, 8, 122);
    push();
}

void Ui::drawTripLive(const GpsFix& fix, const TripRecorder& tr) {
    header(L().tripTitle, &fix);
    char sp[16], t[20], line[48];

    snprintf(sp, sizeof(sp), "%3.0f", fix.speedKmh);
    canvas_.setTextSize(4);
    canvas_.drawString(sp, 12, 26);
    canvas_.setTextSize(1);
    canvas_.setTextColor(C_DIM, C_BG);
    canvas_.drawString("km/h", 160, 48);

    fmtClock(tr.durationMs(), t, sizeof(t));
    snprintf(line, sizeof(line), L().tripTimeFmt, t);
    canvas_.setTextColor(C_FG, C_BG);
    canvas_.drawString(line, 12, 76);
    snprintf(line, sizeof(line), L().tripDistFmt, tr.distanceKm());
    canvas_.drawString(line, 12, 90);
    snprintf(line, sizeof(line), L().tripMaxAvgFmt,
             tr.maxSpeedKmh(), tr.avgSpeedKmh());
    canvas_.setTextColor(C_DIM, C_BG);
    canvas_.drawString(line, 12, 104);
    canvas_.setTextDatum(top_right);
    canvas_.drawString(L().stop, W - 4, 122);
    canvas_.setTextDatum(top_left);
    push();
}

void Ui::drawTripResults(const TripResult& t) {
    header(L().tripResTitle, nullptr);
    char ts[20], line[64];
    canvas_.setTextSize(1);

    snprintf(line, sizeof(line), L().tripResCarFmt, t.car);
    canvas_.setTextColor(C_DIM, C_BG);
    canvas_.drawString(line, 8, 20);

    fmtClock(t.durationMs, ts, sizeof(ts));
    snprintf(line, sizeof(line), L().tripResDurFmt, ts);
    canvas_.setTextColor(C_FG, C_BG);
    canvas_.drawString(line, 8, 34);
    snprintf(line, sizeof(line), L().tripResDistFmt, t.distKm);
    canvas_.drawString(line, 8, 50);
    snprintf(line, sizeof(line), L().tripResMaxFmt, t.maxKmh);
    canvas_.setTextColor(C_ACCENT, C_BG);
    canvas_.drawString(line, 8, 66);
    snprintf(line, sizeof(line), L().tripResAvgFmt, t.avgKmh);
    canvas_.setTextColor(C_FG, C_BG);
    canvas_.drawString(line, 8, 82);

    canvas_.setTextColor(C_DIM, C_BG);
    if (t.gpxPath[0]) {
        snprintf(line, sizeof(line), "GPX: %s", t.gpxPath);
        canvas_.drawString(line, 8, 102);
    }
    canvas_.drawString(L().tripResHint, 8, 122);
    push();
}

void Ui::drawBusy(const char* msg) {
    header(L().busyTitle, nullptr);
    canvas_.setTextSize(2);
    canvas_.setTextColor(C_ACCENT, C_BG);
    canvas_.drawString(msg, 16, 56);
    push();
}

void Ui::drawError(const char* msg) {
    header(L().errTitle, nullptr);
    canvas_.setTextColor(C_WARN, C_BG);
    canvas_.setTextSize(1);
    canvas_.drawString(msg, 10, 50);
    canvas_.setTextColor(C_DIM, C_BG);
    canvas_.drawString("[ENTER] menu", 10, 122);
    push();
}
