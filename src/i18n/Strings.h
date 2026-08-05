#pragma once
// ============================================================
// i18n — interface strings in pt-BR and en-US.
//
// Each language is a `Lang` table of named const char*.
// Use L() for the current language: L().menuRace, etc.
// Format strings (%s/%u/...) keep the same specifiers in both
// languages — only the surrounding text changes.
//
// The chosen language persists in /lang.txt on the SD (langLoad/langSave).
// ============================================================

enum LangId { LANG_PT = 0, LANG_EN = 1, LANG_COUNT };

struct Lang {
    // Splash
    const char* splashSub;        // "Races & Trips"

    // Car selection
    const char* carSelTitle;      // "QUAL CARRO?"
    const char* carSelNew;        // "+ Novo carro"
    const char* carSelHint;       // "[;] cima  [.] baixo  [ENTER] ok"

    // New car
    const char* carInTitle;       // "NOVO CARRO"
    const char* carInPrompt;      // "Digite o nome do carro:"
    const char* carInHint;        // "[ENTER] confirmar  [DEL] apagar"
    const char* carInBack;        // "[`] voltar para a lista"

    // Menu
    const char* menuRace;         // "[R] Corrida"
    const char* menuTrip;         // "[T] Trajeto"
    const char* menuHist;         // "[H] Historico"
    const char* menuCarFmt;       // "[C] Carro: %s"
    const char* menuLangFmt;      // "[L] Idioma: %s"
    const char* menuSdOk;         // "SD ok - GPX habilitado"
    const char* menuSdNo;         // "SEM SD! GPX desativado"

    // Waiting for fix
    const char* waitTitle;        // "AGUARDANDO GPS"
    const char* waitSearch;       // "Buscando fix..."
    const char* waitCancel;       // "[`] cancelar"

    // Race — arm
    const char* armTitle;         // "CORRIDA - ARMAR"
    const char* armLine1;         // "Cruze a linha de largada e"
    const char* armLine2;         // "pressione [ENTER] (>5 km/h)"
    const char* back;             // "[`] voltar"

    // Race — live
    const char* raceTitle;        // "CORRIDA"
    const char* raceLapFmt;       // "VOLTA %u"
    const char* stop;             // "[S] parar"
    const char* raceLastFmt;      // "ULTIMA  %s"
    const char* raceBestFmt;      // "MELHOR V%u  %s"

    // Race — results
    const char* raceResTitle;     // "RESULTADO DA CORRIDA"
    const char* raceResLapsFmt;   // "Voltas: %u   Carro: %s"
    const char* raceResBestFmt;   // "Melhor (V%u): %s"
    const char* raceResWorstFmt;  // "Pior:   %s"
    const char* raceResAvgFmt;    // "Media:  %s"
    const char* raceResIdealFmt;  // "Volta ideal: %s"
    const char* raceResHint;      // "[V] voltas   [ENTER] sair"

    // Lap detail
    const char* lapTitle;         // "DETALHE DA VOLTA"
    const char* lapFmt;           // "VOLTA %u / %u"
    const char* lapBest;          // "MELHOR VOLTA"
    const char* lapHint;          // "[;] ant  [.] prox  [`] voltar"

    // History
    const char* histTitle;        // "HISTORICO"
    const char* histEmpty;        // "Nenhuma sessao gravada."
    const char* histHint;         // "[;][.] navegar [ENTER] abrir [`] sair"

    // Trip — live
    const char* tripTitle;        // "TRAJETO"
    const char* tripTimeFmt;      // "TEMPO %s"
    const char* tripDistFmt;      // "DIST  %.2f km"
    const char* tripMaxAvgFmt;    // "MAX %.0f  MED %.0f"

    // Trip — results
    const char* tripResTitle;     // "RESULTADO DO TRAJETO"
    const char* tripResCarFmt;    // "Carro: %s"
    const char* tripResDurFmt;    // "Duracao:    %s"
    const char* tripResDistFmt;   // "Distancia:  %.2f km"
    const char* tripResMaxFmt;    // "Vel. maxima: %.1f km/h"
    const char* tripResAvgFmt;    // "Vel. media:  %.1f km/h"
    const char* tripResHint;      // "[ENTER] sair"

    // Generic
    const char* busyTitle;        // "AGUARDE"
    const char* errTitle;         // "ERRO"

    // Messages (main.cpp)
    const char* busyImport;       // "Importando GPX..."
    const char* errImport;        // "Falha ao importar o GPX."
};

// Current language (always a valid reference).
const Lang& L();

LangId langCur();
void    langSet(LangId id);
void    langToggle();       // cycles pt <-> en and saves to the SD

void    langLoad();         // reads /lang.txt from the SD (call after initSd)
void    langSave();         // writes /lang.txt to the SD
