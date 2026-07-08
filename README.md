# GPS Tracker — Cardputer ADV

**GPS Tracker**: a car speed and trip tracker running on the **M5 Cardputer ADV** with the **Cap LoRa-1262** (ATGM336H GPS at 10 Hz). Always logs a timestamped `.gpx` to the microSD.

## Flow

Splash (3 s) → car selection (list of previously used cars, sorted by most recent use, or `+ New car` to type one) → menu. The session's car goes into the file name and the GPX track. The list persists in `/cars.txt` on the SD.

## Modes

**Race** — mark the start line by crossing it and pressing `ENTER`. The app detects every crossing of the virtual line (segment intersection + time interpolation, sub-100 ms precision). On stop: best lap, worst, average, and **ideal lap** (sum of the best times from each of the 3 sectors).

**Trip** — records everything from start to finish. On stop: duration, distance (km), and max/average speed.

## Keys

| Key | Action |
|---|---|
| `;` / `.` | Arrows ↑ / ↓ — navigate menus and lists |
| `,` / `/` | Arrows ← / → — back / select |
| `ENTER` | Select · mark start line · confirm |
| `R` / `T` | Shortcut: Race / Trip (menu) |
| `H` / `C` / `L` | Shortcut: History / Change car / Language (menu) |
| `V` | View laps (race results) |
| `DEL` | Delete (car name entry) |
| `S` | Stop session |
| `` ` `` (Esc) | Back |

## Language

Interface in **Portuguese (pt-BR)** or **English (en-US)**. Toggle with `L` in the menu; the choice persists in `/lang.txt` on the SD. Strings live in `src/i18n/Strings.{h,cpp}`.

## Hardware

- Cardputer ADV (ESP32-S3, StampS3A)
- Cap LoRa-1262: GPS over UART — G15 (RX) / G13 (TX), 115200. LoRa is unused (NSS G5 held HIGH).
- microSD: SPI G40/G39/G14, CS G12. Files in `/gpx/YYYYMMDD_HHMMSS_<car>_{race|trip}.gpx`, with a `.ses` summary alongside (feeds the History); car list in `/cars.txt`.

## Build

```bash
pio run -e cardputer-adv -t upload
```

Architecture details in [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).
