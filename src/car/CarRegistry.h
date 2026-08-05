#pragma once
#include <Arduino.h>

// Registry of used cars, persisted on the SD (/cars.txt, one per line).
// Ordered by most-recent use (MRU): the chosen car moves to the top.

#define CARS_FILE     "/cars.txt"
#define MAX_CARS      12
#define CAR_NAME_LEN  16

class CarRegistry {
public:
    void load();                          // call after initSd()
    uint8_t count() const { return count_; }
    const char* name(uint8_t i) const { return i < count_ ? names_[i] : ""; }

    // Sets the session car: moves/inserts it at the top of the list and persists.
    void use(const char* name);
    const char* current() const { return current_; }
    bool hasCurrent() const { return current_[0] != 0; }

    // Filename-safe name: [A-Za-z0-9-_], space becomes '-'.
    static void sanitize(const char* in, char* out, size_t n);

private:
    void save();
    char    names_[MAX_CARS][CAR_NAME_LEN + 1] = {};
    uint8_t count_ = 0;
    char    current_[CAR_NAME_LEN + 1] = "";
};
