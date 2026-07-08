#pragma once
#include <math.h>

// Geometria plana local (ENU) + utilidades geográficas.
// Escala de autódromo/trajeto urbano: projeção equiretangular é suficiente.

// Nota: construtores explícitos (em vez de NSDMI) para permitir
// inicialização por chaves em C++11 (gnu++11 do core ESP32).
struct Vec2 {
    float x, y;   // metros (E, N) relativos à referência
    Vec2() : x(0), y(0) {}
    Vec2(float x_, float y_) : x(x_), y(y_) {}
};

class GeoProjector {
public:
    void setRef(double lat, double lon);
    bool isSet() const { return set_; }
    Vec2 project(double lat, double lon) const;

private:
    double refLat_ = 0, refLon_ = 0, cosLat_ = 1;
    bool set_ = false;
};

// Distância entre dois pontos geográficos (m).
double haversineMeters(double lat1, double lon1, double lat2, double lon2);

// Interseção do segmento P1->P2 com Q1->Q2.
// Retorna true e a fração t (0..1) ao longo de P1->P2 no ponto de cruzamento.
bool segmentsIntersect(const Vec2& p1, const Vec2& p2,
                       const Vec2& q1, const Vec2& q2, float& tOut);

// Epoch Unix (ms) a partir de data/hora UTC.
uint64_t toEpochMs(int year, int month, int day,
                   int hour, int minute, int second, int centisecond);
