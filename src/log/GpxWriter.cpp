#include "GpxWriter.h"
#include <SPI.h>

bool GpxWriter::initSd() {
    // Deselect the Cap's SX1262 (same SPI bus).
    pinMode(LORA_NSS_PIN, OUTPUT);
    digitalWrite(LORA_NSS_PIN, HIGH);

    SPI.begin(SD_SPI_SCK_PIN, SD_SPI_MISO_PIN, SD_SPI_MOSI_PIN, SD_SPI_CS_PIN);
    if (!SD.begin(SD_SPI_CS_PIN, SPI, 25000000)) return false;
    if (!SD.exists(GPX_DIR)) SD.mkdir(GPX_DIR);
    return true;
}

// Epoch (ms) -> civil UTC date/time (inverse of daysFromCivil).
void GpxWriter::epochToCivil(uint64_t epochMs, int& y, int& mo, int& d,
                             int& h, int& mi, int& s) {
    int64_t secs = (int64_t)(epochMs / 1000ULL);
    int64_t days = secs / 86400;
    int64_t rem = secs % 86400;
    h = (int)(rem / 3600);
    mi = (int)((rem % 3600) / 60);
    s = (int)(rem % 60);

    // civil-from-days (Howard Hinnant)
    days += 719468;
    const int64_t era = (days >= 0 ? days : days - 146096) / 146097;
    const unsigned doe = (unsigned)(days - era * 146097);
    const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    const int64_t yr = (int64_t)yoe + era * 400;
    const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const unsigned mp = (5 * doy + 2) / 153;
    d = (int)(doy - (153 * mp + 2) / 5 + 1);
    mo = (int)(mp + (mp < 10 ? 3 : -9));
    y = (int)(yr + (mo <= 2));
}

void GpxWriter::isoTime(uint64_t epochMs, char* out, size_t n) {
    int y, mo, d, h, mi, s;
    epochToCivil(epochMs, y, mo, d, h, mi, s);
    snprintf(out, n, "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
             y, mo, d, h, mi, s, (int)(epochMs % 1000ULL));
}

bool GpxWriter::open(uint64_t epochMs, const char* suffix, const char* trackName) {
    if (open_) return false;
    int y, mo, d, h, mi, s;
    epochToCivil(epochMs, y, mo, d, h, mi, s);
    snprintf(path_, sizeof(path_), GPX_DIR "/%04d%02d%02d_%02d%02d%02d_%s.gpx",
             y, mo, d, h, mi, s, suffix);

    file_ = SD.open(path_, FILE_WRITE);
    if (!file_) return false;
    open_ = true;
    bufLen_ = 0;

    append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
           "<gpx version=\"1.1\" creator=\"Cardputer GPS Tracker\"\n"
           "  xmlns=\"http://www.topografix.com/GPX/1/1\">\n"
           "<trk><name>");
    append(trackName);
    append("</name><trkseg>\n");
    return true;
}

void GpxWriter::addPoint(double lat, double lon, float eleM,
                         float speedKmh, uint64_t epochMs) {
    if (!open_) return;
    char ts[32];
    isoTime(epochMs, ts, sizeof(ts));
    char pt[192];
    snprintf(pt, sizeof(pt),
             "<trkpt lat=\"%.7f\" lon=\"%.7f\"><ele>%.1f</ele><time>%s</time>"
             "<extensions><speed>%.2f</speed></extensions></trkpt>\n",
             lat, lon, eleM, ts, speedKmh / 3.6f);  // speed in m/s
    append(pt);
    flush(false);
}

const char* GpxWriter::close() {
    if (!open_) return path_;
    append("</trkseg></trk>\n</gpx>\n");
    flush(true);
    file_.close();
    open_ = false;
    return path_;
}

void GpxWriter::append(const char* s) {
    const size_t len = strlen(s);
    if (bufLen_ + len >= sizeof(buf_)) flush(true);
    memcpy(buf_ + bufLen_, s, len);
    bufLen_ += len;
}

void GpxWriter::flush(bool force) {
    if (!open_) return;
    if (!force && bufLen_ < GPX_FLUSH_BYTES) return;
    if (bufLen_ > 0) {
        file_.write((const uint8_t*)buf_, bufLen_);
        file_.flush();
        bufLen_ = 0;
    }
}
