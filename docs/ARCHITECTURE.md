# Arquitetura — GPS Tracker

## Módulos

```
src/
├── main.cpp           state machine + orquestração
├── config.h           pinos e tunáveis
├── gps/GpsService     UART/NMEA (TinyGPSPlus) + config CASIC (10 Hz, só GGA+RMC)
├── geo/geo            projeção ENU equiretangular, haversine, interseção de segmentos, epoch UTC
├── track/LapTimer     linha virtual, voltas, setores, volta ideal
├── track/TripRecorder distância, duração, vel. máx/média
├── log/GpxWriter      GPX 1.1 streaming no SD (buffer 4KB)
├── car/CarRegistry    carros da sessão, persistidos em /cars.txt (MRU)
├── session/SessionStore  resumo .ses por sessão (histórico), structs de resultado
└── ui/Ui              telas em M5Canvas (240x135)
```

## Decisões principais

**Cronometragem por linha virtual.** Ao marcar a largada, o curso GPS define uma linha de 30 m perpendicular à direção do carro. Cada par de fixes consecutivos forma um segmento; a interseção segmento×linha (produto vetorial 2D) dá a fração `t` do cruzamento, e o tempo da volta é interpolado nesse ponto — a 10 Hz isso rende precisão de ~10–30 ms, muito melhor que usar o timestamp do fix. Cruzamentos na direção errada (dot product < 0) e com menos de 10 s de volta são descartados.

**Setores e volta ideal.** A distância da volta 1 vira referência e é dividida em 3 setores iguais. Nas voltas seguintes o tempo de cada setor é registrado ao cruzar o limite de distância (com interpolação). Para a própria volta 1, checkpoints (distância, tempo) a cada 20 m permitem calcular os setores retroativamente. Volta ideal = soma dos melhores setores entre todas as voltas.

**Velocidade do GPS, não derivada da posição.** `speedKmh` vem do RMC (doppler), que é mais estável que derivar posição. No modo Trajeto, distância só acumula acima de 2 km/h para não integrar jitter parado.

**GPX em streaming.** Nada do trajeto fica em RAM: pontos são formatados e acumulados num buffer de 4 KB, com flush ao encher — protege o cartão e evita travadas no loop. Corrida grava a 10 Hz, Trajeto a 1 Hz. Timestamps ISO-8601 UTC vindos do próprio GPS (algoritmo civil-from-days, sem dependência de RTC/NTP).

**Memória.** Sem PSRAM no ESP32-S3FN8. Custos fixos: canvas 64 KB, voltas 256×16 B = 4 KB, checkpoints 512×16 B = 8 KB, buffer GPX ~4,3 KB. Folga confortável nos 512 KB de SRAM.

**SPI compartilhado.** O SX1262 do Cap divide o barramento com o SD; o firmware fixa `NSS (G5)` em HIGH no boot para desselecioná-lo.

**Importação retroativa de GPX (GpxImport).** GPX órfãos (gravados antes do `.ses` existir) aparecem no histórico com `*` e são reprocessados ao abrir. Corrida: o 1º ponto do arquivo é a linha de largada (a gravação começa na marcação) e a direção vem do deslocamento inicial (≥ 8 m); os pontos passam pelo mesmo `LapTimer` da sessão ao vivo. Trajeto: estatísticas recalculadas dos pontos (velocidade da tag `<speed>`). O `.ses` gerado torna as próximas aberturas instantâneas. Só suporta GPX do próprio app (parser de linha, não XML genérico).

**Histórico via arquivos .ses.** O GPX não guarda linha de largada nem tempos, então recalcular voltas dele seria frágil. Ao encerrar qualquer sessão, um resumo texto `.ses` (voltas + setores, ou estatísticas do trajeto) é gravado ao lado do GPX. O Histórico lista os `.ses` (mais recentes primeiro, até 24) e reabre a mesma tela de resultados; em corridas, `[V]` navega volta a volta com delta vs. melhor e setores destacados. As telas de resultado leem structs (`RaceResult`/`TripResult`) preenchidos tanto pela sessão ao vivo quanto pelo load do arquivo.

**Carro da sessão.** Fluxo de boot: splash (3 s, cobre o init de SD/GPS) → seleção/digitação do carro → menu. `CarRegistry` guarda até 12 nomes (16 chars) em `/cars.txt`, com o mais recente no topo; o nome sanitizado ([A-Za-z0-9-_]) entra no nome do arquivo GPX e o nome original na tag `<name>` da trilha. Sem SD, o nome vale só para a sessão.

## Limitações conhecidas / evolução

- Curso GPS precisa de movimento: a marcação da linha exige ≥ 5 km/h.
- Sessões acima de 256 voltas param de registrar voltas novas (GPX continua).
- Possíveis extensões: exportar resumo `.txt`, telemetria via LoRa, waypoints de setor manuais.
- **Planejado — resultados no celular via WiFi (decisão de 2026-07-03):** sem dados ao vivo e sem BLE (avaliado e descartado: exigiria app/Web Bluetooth, sem suporte no iOS, e é lento para arquivos). Desenho decidido: hotspot sob demanda (tecla `[W]` no menu, nunca durante a corrida, desliga ao sair) + servidor web em `192.168.4.1` com lista de sessões por carro/data, tabela de voltas e setores (melhor/pior/média/volta ideal) e link de download dos `.gpx`. Etapa 2: desenho do traçado colorido por velocidade renderizado dos pontos, sem mapa de fundo (funciona offline).
