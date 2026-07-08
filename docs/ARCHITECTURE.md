# Architecture — GPS Tracker

## Modules

```
src/
├── main.cpp           state machine + orchestration
├── config.h           pins and tunables
├── gps/GpsService     UART/NMEA (TinyGPSPlus) + CASIC config (10 Hz, GGA+RMC only)
├── geo/geo            equirectangular ENU projection, haversine, segment intersection, UTC epoch
├── track/LapTimer     virtual line, laps, sectors, ideal lap
├── track/TripRecorder distance, duration, max/average speed
├── log/GpxWriter      streaming GPX 1.1 to the SD (4 KB buffer)
├── car/CarRegistry    session cars, persisted in /cars.txt (MRU)
├── session/SessionStore  per-session .ses summary (history), result structs
└── ui/Ui              screens on M5Canvas (240x135)
```

## Key decisions

**Virtual-line timing.** When you mark the start, the GPS course defines a 30 m line perpendicular to the car's heading. Each pair of consecutive fixes forms a segment; the segment×line intersection (2D cross product) gives the crossing fraction `t`, and the lap time is interpolated at that point — at 10 Hz this yields ~10–30 ms precision, much better than using the fix timestamp. Crossings in the wrong direction (dot product < 0) or shorter than 10 s per lap are discarded.

**Sectors and ideal lap.** The distance of lap 1 becomes the reference and is split into 3 equal sectors. On subsequent laps, each sector's time is recorded when crossing the distance boundary (with interpolation). For lap 1 itself, checkpoints (distance, time) every 20 m allow the sectors to be computed retroactively. Ideal lap = sum of the best sectors across all laps.

**GPS speed, not derived from position.** `speedKmh` comes from RMC (Doppler), which is more stable than differentiating position. In Trip mode, distance only accumulates above 2 km/h so that jitter while stationary isn't integrated.

**Streaming GPX.** None of the track stays in RAM: points are formatted and accumulated in a 4 KB buffer, flushed when full — this protects the card and avoids stalls in the loop. Race logs at 10 Hz, Trip at 1 Hz. ISO-8601 UTC timestamps come from the GPS itself (civil-from-days algorithm, no RTC/NTP dependency).

**Memory.** No PSRAM on the ESP32-S3FN8. Fixed costs: 64 KB canvas, laps 256×16 B = 4 KB, checkpoints 512×16 B = 8 KB, GPX buffer ~4.3 KB. Comfortable headroom within the 512 KB of SRAM.

**Shared SPI.** The Cap's SX1262 shares the bus with the SD; the firmware holds `NSS (G5)` HIGH at boot to deselect it.

**Retroactive GPX import (GpxImport).** Orphan GPX files (recorded before the `.ses` existed) show up in the history with a `*` and are reprocessed when opened. Race: the file's 1st point is the start line (recording begins at the marking) and the heading comes from the initial displacement (≥ 8 m); the points run through the same `LapTimer` as the live session. Trip: statistics recomputed from the points (speed from the `<speed>` tag). The generated `.ses` makes future opens instantaneous. Only supports GPX from this app (line parser, not generic XML).

**History via .ses files.** GPX doesn't store the start line or lap times, so recomputing laps from it would be fragile. When any session ends, a text `.ses` summary (laps + sectors, or trip statistics) is written alongside the GPX. The History lists the `.ses` files (most recent first, up to 24) and reopens the same results screen; in races, `[V]` steps lap by lap with the delta vs. best and highlighted sectors. The results screens read structs (`RaceResult`/`TripResult`) populated both by the live session and by loading the file.

**Session car.** Boot flow: splash (3 s, covers SD/GPS init) → car selection/entry → menu. `CarRegistry` keeps up to 12 names (16 chars) in `/cars.txt`, most recent on top; the sanitized name ([A-Za-z0-9-_]) goes into the GPX file name and the original name into the track's `<name>` tag. Without an SD, the name is valid for the session only.

## Known limitations / roadmap

- GPS course needs movement: marking the line requires ≥ 5 km/h.
- Sessions past 256 laps stop recording new laps (GPX keeps going).
- Possible extensions: export `.txt` summary, telemetry over LoRa, manual sector waypoints.
- **Planned — results on the phone over WiFi (decision from 2026-07-03):** no live data and no BLE (evaluated and dropped: it would require an app/Web Bluetooth, no iOS support, and it's slow for files). Chosen design: on-demand hotspot (`[W]` key in the menu, never during a race, turns off on exit) + web server at `192.168.4.1` with a list of sessions by car/date, a table of laps and sectors (best/worst/average/ideal lap), and a download link for the `.gpx` files. Stage 2: speed-colored track drawing rendered from the points, no base map (works offline).
