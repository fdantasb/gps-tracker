# GPS Tracker — Cardputer ADV

**GPS Tracker**: tracker de velocidade e trajeto para carro, rodando no **M5 Cardputer ADV** com o **Cap LoRa-1262** (GPS ATGM336H a 10 Hz). Grava sempre um `.gpx` com timestamps no microSD.

## Fluxo

Splash (3 s) → seleção do carro (lista dos já usados, ordenada por uso recente, ou `+ Novo carro` para digitar) → menu. O carro da sessão entra no nome do arquivo e na trilha do GPX. A lista persiste em `/cars.txt` no SD.

## Modos

**Corrida** — marque a linha de largada cruzando-a e apertando `ENTER`. O app detecta cada cruzamento da linha virtual (interseção de segmentos + interpolação temporal, precisão sub-100ms). Ao parar: melhor volta, pior, média e **volta ideal** (soma dos melhores tempos de cada um dos 3 setores).

**Trajeto** — grava tudo do início ao fim. Ao parar: duração, distância (km), velocidade máxima e média.

## Teclas

| Tecla | Ação |
|---|---|
| `;` / `.` | Setas ↑ / ↓ — navegar em menus e listas |
| `,` / `/` | Setas ← / → — voltar / selecionar |
| `ENTER` | Selecionar · marcar largada · confirmar |
| `R` / `T` | Atalho: Corrida / Trajeto (menu) |
| `H` / `C` / `L` | Atalho: Histórico / Trocar carro / Idioma (menu) |
| `V` | Ver voltas (resultado de corrida) |
| `DEL` | Apagar (digitação do carro) |
| `S` | Parar sessão |
| `` ` `` (Esc) | Voltar |

## Idioma

Interface em **português (pt-BR)** ou **inglês (en-US)**. Alterne com `L` no menu; a escolha persiste em `/lang.txt` no SD. Textos ficam em `src/i18n/Strings.{h,cpp}`.

## Hardware

- Cardputer ADV (ESP32-S3, StampS3A)
- Cap LoRa-1262: GPS em UART — G15 (RX) / G13 (TX), 115200. LoRa não é usado (NSS G5 mantido HIGH).
- microSD: SPI G40/G39/G14, CS G12. Arquivos em `/gpx/YYYYMMDD_HHMMSS_<carro>_{race|trip}.gpx`, com um resumo `.ses` ao lado (alimenta o Histórico); lista de carros em `/cars.txt`.

## Build

```bash
pio run -e cardputer-adv -t upload
```

Detalhes de arquitetura em [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).
