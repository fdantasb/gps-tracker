#pragma once
// ============================================================
// GPS Tracker — configuração de hardware e tunáveis
// Alvo: M5 Cardputer ADV + Cap LoRa-1262 (U214)
// ============================================================

// ---- GPS (ATGM336H-6N @ AT6668, NMEA 0183, CASIC) ----------
// Cap-Bus: G13 -> GPS-RX (TX do ESP), G15 <- GPS-TX (RX do ESP)
#define GPS_UART_RX_PIN     15
#define GPS_UART_TX_PIN     13
#define GPS_BAUD            115200
#define GPS_UPDATE_RATE_MS  100     // 10 Hz (máx. do ATGM336H)

// ---- microSD (SPI compartilhado com o LoRa do Cap) ---------
#define SD_SPI_SCK_PIN      40
#define SD_SPI_MISO_PIN     39
#define SD_SPI_MOSI_PIN     14
#define SD_SPI_CS_PIN       12
#define LORA_NSS_PIN        5       // manter HIGH: desseleciona o SX1262 no barramento

// ---- Corrida ------------------------------------------------
#define START_LINE_HALF_WIDTH_M  15.0f  // linha virtual: 30 m de largura total
#define MIN_LAP_TIME_MS          10000  // debounce: ignora cruzamentos < 10 s
#define MIN_COURSE_SPEED_KMH     5.0f   // velocidade mínima p/ curso GPS confiável
#define NUM_SECTORS              3      // setores por volta (volta ideal)
#define MAX_LAPS                 256

// ---- Trajeto ------------------------------------------------
#define TRIP_MIN_SPEED_KMH   2.0f   // abaixo disso não acumula distância (jitter)

// ---- Log GPX ------------------------------------------------
#define GPX_DIR                 "/gpx"
#define GPX_RACE_INTERVAL_MS    100    // 10 Hz em corrida
#define GPX_TRIP_INTERVAL_MS    1000   // 1 Hz em trajeto
#define GPX_FLUSH_BYTES         4096   // flush do buffer para o SD

// ---- UI -----------------------------------------------------
#define UI_REFRESH_MS       200    // 5 Hz
#define DELTA_SHOW_MS       7000   // destaque do delta ao completar volta
