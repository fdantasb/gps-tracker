#include "CarRegistry.h"
#include <SD.h>

void CarRegistry::load() {
    count_ = 0;
    File f = SD.open(CARS_FILE, FILE_READ);
    if (!f) return;
    char line[CAR_NAME_LEN + 1];
    size_t len = 0;
    while (f.available() && count_ < MAX_CARS) {
        const char c = (char)f.read();
        if (c == '\n' || c == '\r') {
            if (len > 0) {
                line[len] = 0;
                strncpy(names_[count_++], line, CAR_NAME_LEN);
                len = 0;
            }
        } else if (len < CAR_NAME_LEN) {
            line[len++] = c;
        }
    }
    if (len > 0 && count_ < MAX_CARS) {
        line[len] = 0;
        strncpy(names_[count_++], line, CAR_NAME_LEN);
    }
    f.close();
}

void CarRegistry::save() {
    SD.remove(CARS_FILE);
    File f = SD.open(CARS_FILE, FILE_WRITE);
    if (!f) return;
    for (uint8_t i = 0; i < count_; ++i) {
        f.println(names_[i]);
    }
    f.close();
}

void CarRegistry::use(const char* name) {
    if (!name || !name[0]) return;
    strncpy(current_, name, CAR_NAME_LEN);
    current_[CAR_NAME_LEN] = 0;

    // Já existe? Move para o topo (MRU).
    int found = -1;
    for (uint8_t i = 0; i < count_; ++i) {
        if (strcasecmp(names_[i], current_) == 0) { found = i; break; }
    }
    if (found == 0) return;  // já é o primeiro

    if (found > 0) {
        char tmp[CAR_NAME_LEN + 1];
        strncpy(tmp, names_[found], sizeof(tmp));
        for (int i = found; i > 0; --i) strncpy(names_[i], names_[i - 1], sizeof(names_[i]));
        strncpy(names_[0], tmp, sizeof(names_[0]));
    } else {
        // Novo: insere no topo, descarta o último se cheio.
        if (count_ < MAX_CARS) count_++;
        for (int i = count_ - 1; i > 0; --i) strncpy(names_[i], names_[i - 1], sizeof(names_[i]));
        strncpy(names_[0], current_, sizeof(names_[0]));
    }
    save();
}

void CarRegistry::sanitize(const char* in, char* out, size_t n) {
    size_t j = 0;
    for (size_t i = 0; in[i] && j < n - 1; ++i) {
        const char c = in[i];
        if (isalnum((unsigned char)c) || c == '-' || c == '_') out[j++] = c;
        else if (c == ' ') out[j++] = '-';
        // demais caracteres: descartados
    }
    if (j == 0) out[j++] = 'c';  // nunca vazio
    out[j] = 0;
}
