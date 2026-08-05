#pragma once
// ============================================================
// GPS Tracker — hardware configuration and tunables
// Target: M5 Cardputer ADV + Cap LoRa-1262 (U214)
// ============================================================

// ---- GPS (ATGM336H-6N @ AT6668, NMEA 0183, CASIC) ----------
// Cap-Bus: G13 -> GPS-RX (ESP TX), G15 <- GPS-TX (ESP RX)
#define GPS_UART_RX_PIN     15
#define GPS_UART_TX_PIN     13
#define GPS_BAUD            115200
#define GPS_UPDATE_RATE_MS  100     // 10 Hz (max of the ATGM336H)

// ---- microSD (SPI shared with the Cap's LoRa) --------------
#define SD_SPI_SCK_PIN      40
#define SD_SPI_MISO_PIN     39
#define SD_SPI_MOSI_PIN     14
#define SD_SPI_CS_PIN       12
#define LORA_NSS_PIN        5       // hold HIGH: deselects the SX1262 on the bus

// ---- Race ---------------------------------------------------
#define START_LINE_HALF_WIDTH_M  15.0f  // virtual line: 30 m total width
#define MIN_LAP_TIME_MS          10000  // debounce: ignore crossings < 10 s
#define MIN_COURSE_SPEED_KMH     5.0f   // minimum speed for a reliable GPS course
#define NUM_SECTORS              3      // sectors per lap (ideal lap)
#define MAX_LAPS                 256

// ---- Trip ---------------------------------------------------
#define TRIP_MIN_SPEED_KMH   2.0f   // below this, distance isn't accumulated (jitter)

// ---- GPX logging --------------------------------------------
#define GPX_DIR                 "/gpx"
#define GPX_RACE_INTERVAL_MS    100    // 10 Hz in race
#define GPX_TRIP_INTERVAL_MS    1000   // 1 Hz in trip
#define GPX_FLUSH_BYTES         4096   // buffer flush to the SD

// ---- UI -----------------------------------------------------
#define UI_REFRESH_MS       200    // 5 Hz
#define DELTA_SHOW_MS       7000   // delta highlight when a lap completes
