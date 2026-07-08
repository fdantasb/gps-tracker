#pragma once
#include <Arduino.h>
#include "SessionStore.h"
#include "../track/LapTimer.h"

// Reconstrução retroativa de sessões a partir de GPX gerados pelo
// próprio app (corridas antigas sem .ses).
//
// Corrida: o GPX começa a gravar no instante em que a largada é marcada,
// então o 1º ponto é a linha de largada; a direção vem dos primeiros
// pontos (>= 8 m de deslocamento). O arquivo é reprocessado pelo mesmo
// LapTimer da sessão ao vivo.
//
// Trajeto: distância/duração/velocidades recalculadas dos pontos
// (velocidade lida da tag <speed> gravada pelo app).

class GpxImport {
public:
    // file = nome do .gpx (sem diretório). Preenche out e grava o .ses.
    static bool importRace(const char* file, LapTimer& lt, RaceResult& out);
    static bool importTrip(const char* file, TripResult& out);

private:
    static bool parsePoint(const char* line, double& lat, double& lon,
                           float& speedKmh, uint64_t& epochMs);
    static void carFromFilename(const char* file, char* out, size_t n);
};
