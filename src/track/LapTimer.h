#pragma once
#include <Arduino.h>
#include "../config.h"
#include "../geo/geo.h"

// Cronômetro de voltas por linha de largada virtual.
//
// A linha é um segmento de 2*START_LINE_HALF_WIDTH_M perpendicular ao curso
// do carro no momento da marcação. Cruzamento = interseção do segmento
// (fix anterior -> fix atual) com a linha, com interpolação temporal na
// fração de cruzamento (precisão sub-fix mesmo a 10 Hz).
//
// Setores: a volta 1 define a distância de referência; ela é dividida em
// NUM_SECTORS trechos iguais. A volta ideal = soma dos melhores setores.
// Durante a volta 1 são gravados checkpoints (distância, tempo) para
// calcular retroativamente os setores da própria volta 1.

struct Lap {
    uint32_t timeMs = 0;
    uint32_t sectorMs[NUM_SECTORS] = {0};
};

class LapTimer {
public:
    // Marca a linha de largada na posição/curso atuais e inicia a volta 1.
    void setStartLine(double lat, double lon, float courseDeg, uint64_t epochMs);

    // Alimenta um fix novo. Retorna true se uma volta foi completada.
    bool onFix(double lat, double lon, uint64_t epochMs);

    bool     isRunning() const { return running_; }
    uint16_t lapCount() const { return lapCount_; }
    uint32_t currentLapMs(uint64_t nowEpochMs) const;
    const Lap& lap(uint16_t i) const { return laps_[i]; }

    // Estatísticas (válidas com lapCount() >= 1)
    uint32_t bestLapMs() const;
    uint32_t worstLapMs() const;
    uint32_t avgLapMs() const;
    uint16_t bestLapIndex() const;
    uint32_t idealLapMs() const;                    // soma dos melhores setores
    uint32_t bestSectorMs(uint8_t s) const;

    void stop() { running_ = false; }

private:
    void beginLap(uint64_t epochMs);
    void closeSector(uint8_t idx, uint64_t epochMs);
    void finishLap(uint64_t crossEpochMs);
    void recordCheckpoint(float dist, uint64_t epochMs);
    void backfillLap1Sectors(uint64_t crossEpochMs);

    // Linha de largada (coords ENU, ref = centro da linha)
    GeoProjector proj_;
    Vec2  lineA_, lineB_;
    Vec2  lineDir_;               // direção de travessia válida (unitária)
    bool  lineSet_ = false;
    bool  running_ = false;

    // Fix anterior
    Vec2     prevPos_;
    uint64_t prevEpochMs_ = 0;
    bool     hasPrev_ = false;

    // Volta corrente
    uint64_t lapStartMs_ = 0;
    float    lapDist_ = 0;        // distância acumulada na volta (m)
    uint8_t  nextSector_ = 0;     // próximo limite de setor a cruzar
    uint64_t lastSectorMs_ = 0;   // epoch do fechamento do último setor

    // Referência de setores (definida pela volta 1)
    float refLapDist_ = 0;

    // Checkpoints da volta 1 p/ backfill de setores
    static constexpr uint16_t kMaxCkpts = 512;
    static constexpr float    kCkptStride = 20.0f;  // m
    struct Ckpt { float dist; uint64_t tMs; };
    Ckpt     ckpts_[kMaxCkpts];
    uint16_t ckptCount_ = 0;

    Lap      curLap_;
    Lap      laps_[MAX_LAPS];
    uint16_t lapCount_ = 0;
};
