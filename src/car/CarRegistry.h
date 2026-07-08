#pragma once
#include <Arduino.h>

// Registro de carros usados, persistido no SD (/cars.txt, um por linha).
// Ordenado por uso mais recente (MRU): o carro escolhido vai para o topo.

#define CARS_FILE     "/cars.txt"
#define MAX_CARS      12
#define CAR_NAME_LEN  16

class CarRegistry {
public:
    void load();                          // chamar após initSd()
    uint8_t count() const { return count_; }
    const char* name(uint8_t i) const { return i < count_ ? names_[i] : ""; }

    // Define o carro da sessão: move/insere no topo da lista e persiste.
    void use(const char* name);
    const char* current() const { return current_; }
    bool hasCurrent() const { return current_[0] != 0; }

    // Nome seguro p/ arquivo: [A-Za-z0-9-_], espaço vira '-'.
    static void sanitize(const char* in, char* out, size_t n);

private:
    void save();
    char    names_[MAX_CARS][CAR_NAME_LEN + 1] = {};
    uint8_t count_ = 0;
    char    current_[CAR_NAME_LEN + 1] = "";
};
