#include "Strings.h"
#include <SD.h>

#define LANG_FILE "/lang.txt"

static const Lang PT = {
    /* splashSub       */ "Corridas & Trajetos",

    /* carSelTitle     */ "QUAL CARRO?",
    /* carSelNew       */ "+ Novo carro",
    /* carSelHint      */ "[;] cima  [.] baixo  [ENTER] ok",

    /* carInTitle      */ "NOVO CARRO",
    /* carInPrompt     */ "Digite o nome do carro:",
    /* carInHint       */ "[ENTER] confirmar  [DEL] apagar",
    /* carInBack       */ "[`] voltar para a lista",

    /* menuRace        */ "[R] Corrida",
    /* menuTrip        */ "[T] Trajeto",
    /* menuHist        */ "[H] Historico",
    /* menuCarFmt      */ "[C] Carro: %s",
    /* menuLangFmt     */ "[L] Idioma: %s",
    /* menuSdOk        */ "SD ok - GPX habilitado",
    /* menuSdNo        */ "SEM SD! GPX desativado",

    /* waitTitle       */ "AGUARDANDO GPS",
    /* waitSearch      */ "Buscando fix...",
    /* waitCancel      */ "[`] cancelar",

    /* armTitle        */ "CORRIDA - ARMAR",
    /* armLine1        */ "Cruze a linha de largada e",
    /* armLine2        */ "pressione [ENTER] (>5 km/h)",
    /* back            */ "[`] voltar",

    /* raceTitle       */ "CORRIDA",
    /* raceLapFmt      */ "VOLTA %u",
    /* stop            */ "[S] parar",
    /* raceLastFmt     */ "ULTIMA  %s",
    /* raceBestFmt     */ "MELHOR V%u  %s",

    /* raceResTitle    */ "RESULTADO DA CORRIDA",
    /* raceResLapsFmt  */ "Voltas: %u   Carro: %s",
    /* raceResBestFmt  */ "Melhor (V%u): %s",
    /* raceResWorstFmt */ "Pior:   %s",
    /* raceResAvgFmt   */ "Media:  %s",
    /* raceResIdealFmt */ "Volta ideal: %s",
    /* raceResHint     */ "[V] voltas   [ENTER] sair",

    /* lapTitle        */ "DETALHE DA VOLTA",
    /* lapFmt          */ "VOLTA %u / %u",
    /* lapBest         */ "MELHOR VOLTA",
    /* lapHint         */ "[;] ant  [.] prox  [`] voltar",

    /* histTitle       */ "HISTORICO",
    /* histEmpty       */ "Nenhuma sessao gravada.",
    /* histHint        */ "[;][.] navegar [ENTER] abrir [`] sair",

    /* tripTitle       */ "TRAJETO",
    /* tripTimeFmt     */ "TEMPO %s",
    /* tripDistFmt     */ "DIST  %.2f km",
    /* tripMaxAvgFmt   */ "MAX %.0f  MED %.0f",

    /* tripResTitle    */ "RESULTADO DO TRAJETO",
    /* tripResCarFmt   */ "Carro: %s",
    /* tripResDurFmt   */ "Duracao:    %s",
    /* tripResDistFmt  */ "Distancia:  %.2f km",
    /* tripResMaxFmt   */ "Vel. maxima: %.1f km/h",
    /* tripResAvgFmt   */ "Vel. media:  %.1f km/h",
    /* tripResHint     */ "[ENTER] sair",

    /* busyTitle       */ "AGUARDE",
    /* errTitle        */ "ERRO",

    /* busyImport      */ "Importando GPX...",
    /* errImport       */ "Falha ao importar o GPX.",
};

static const Lang EN = {
    /* splashSub       */ "Races & Trips",

    /* carSelTitle     */ "WHICH CAR?",
    /* carSelNew       */ "+ New car",
    /* carSelHint      */ "[;] up  [.] down  [ENTER] ok",

    /* carInTitle      */ "NEW CAR",
    /* carInPrompt     */ "Enter the car name:",
    /* carInHint       */ "[ENTER] confirm  [DEL] delete",
    /* carInBack       */ "[`] back to list",

    /* menuRace        */ "[R] Race",
    /* menuTrip        */ "[T] Trip",
    /* menuHist        */ "[H] History",
    /* menuCarFmt      */ "[C] Car: %s",
    /* menuLangFmt     */ "[L] Language: %s",
    /* menuSdOk        */ "SD ok - GPX enabled",
    /* menuSdNo        */ "NO SD! GPX disabled",

    /* waitTitle       */ "WAITING FOR GPS",
    /* waitSearch      */ "Searching fix...",
    /* waitCancel      */ "[`] cancel",

    /* armTitle        */ "RACE - ARM",
    /* armLine1        */ "Cross the start line and",
    /* armLine2        */ "press [ENTER] (>5 km/h)",
    /* back            */ "[`] back",

    /* raceTitle       */ "RACE",
    /* raceLapFmt      */ "LAP %u",
    /* stop            */ "[S] stop",
    /* raceLastFmt     */ "LAST  %s",
    /* raceBestFmt     */ "BEST L%u  %s",

    /* raceResTitle    */ "RACE RESULTS",
    /* raceResLapsFmt  */ "Laps: %u   Car: %s",
    /* raceResBestFmt  */ "Best (L%u): %s",
    /* raceResWorstFmt */ "Worst:  %s",
    /* raceResAvgFmt   */ "Avg:    %s",
    /* raceResIdealFmt */ "Ideal lap: %s",
    /* raceResHint     */ "[V] laps   [ENTER] exit",

    /* lapTitle        */ "LAP DETAIL",
    /* lapFmt          */ "LAP %u / %u",
    /* lapBest         */ "BEST LAP",
    /* lapHint         */ "[;] prev  [.] next  [`] back",

    /* histTitle       */ "HISTORY",
    /* histEmpty       */ "No sessions recorded.",
    /* histHint        */ "[;][.] navigate [ENTER] open [`] exit",

    /* tripTitle       */ "TRIP",
    /* tripTimeFmt     */ "TIME %s",
    /* tripDistFmt     */ "DIST  %.2f km",
    /* tripMaxAvgFmt   */ "MAX %.0f  AVG %.0f",

    /* tripResTitle    */ "TRIP RESULTS",
    /* tripResCarFmt   */ "Car: %s",
    /* tripResDurFmt   */ "Duration:   %s",
    /* tripResDistFmt  */ "Distance:   %.2f km",
    /* tripResMaxFmt   */ "Max speed:   %.1f km/h",
    /* tripResAvgFmt   */ "Avg speed:   %.1f km/h",
    /* tripResHint     */ "[ENTER] exit",

    /* busyTitle       */ "PLEASE WAIT",
    /* errTitle        */ "ERROR",

    /* busyImport      */ "Importing GPX...",
    /* errImport       */ "Failed to import GPX.",
};

static const Lang* const TABLE[LANG_COUNT] = { &PT, &EN };

static LangId current = LANG_EN;

const Lang& L() { return *TABLE[current]; }

LangId langCur() { return current; }

void langSet(LangId id) {
    if (id < 0 || id >= LANG_COUNT) return;
    current = id;
}

void langToggle() {
    current = (current == LANG_PT) ? LANG_EN : LANG_PT;
    langSave();
}

void langLoad() {
    File f = SD.open(LANG_FILE, FILE_READ);
    if (!f) return;
    char buf[4] = {};
    size_t n = 0;
    while (f.available() && n < sizeof(buf) - 1) {
        const char c = (char)f.read();
        if (c == '\n' || c == '\r') break;
        buf[n++] = c;
    }
    f.close();
    if (strncmp(buf, "en", 2) == 0) current = LANG_EN;
    else if (strncmp(buf, "pt", 2) == 0) current = LANG_PT;
}

void langSave() {
    SD.remove(LANG_FILE);
    File f = SD.open(LANG_FILE, FILE_WRITE);
    if (!f) return;
    f.println(current == LANG_EN ? "en" : "pt");
    f.close();
}
