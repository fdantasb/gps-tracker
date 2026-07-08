#pragma once
// ============================================================
// i18n — textos da interface em pt-BR e en-US.
//
// Cada idioma é uma tabela `Lang` de const char* nomeados.
// Use L() para o idioma atual: L().menuRace, etc.
// Strings de formato (%s/%u/...) mantêm os mesmos especificadores
// nos dois idiomas — só o texto ao redor muda.
//
// O idioma escolhido persiste em /lang.txt no SD (langLoad/langSave).
// ============================================================

enum LangId { LANG_PT = 0, LANG_EN = 1, LANG_COUNT };

struct Lang {
    // Splash
    const char* splashSub;        // "Corridas & Trajetos"

    // Seleção de carro
    const char* carSelTitle;      // "QUAL CARRO?"
    const char* carSelNew;        // "+ Novo carro"
    const char* carSelHint;       // "[;] cima  [.] baixo  [ENTER] ok"

    // Novo carro
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

    // Aguardando fix
    const char* waitTitle;        // "AGUARDANDO GPS"
    const char* waitSearch;       // "Buscando fix..."
    const char* waitCancel;       // "[`] cancelar"

    // Corrida — armar
    const char* armTitle;         // "CORRIDA - ARMAR"
    const char* armLine1;         // "Cruze a linha de largada e"
    const char* armLine2;         // "pressione [ENTER] (>5 km/h)"
    const char* back;             // "[`] voltar"

    // Corrida — ao vivo
    const char* raceTitle;        // "CORRIDA"
    const char* raceLapFmt;       // "VOLTA %u"
    const char* stop;             // "[S] parar"
    const char* raceLastFmt;      // "ULTIMA  %s"
    const char* raceBestFmt;      // "MELHOR V%u  %s"

    // Corrida — resultado
    const char* raceResTitle;     // "RESULTADO DA CORRIDA"
    const char* raceResLapsFmt;   // "Voltas: %u   Carro: %s"
    const char* raceResBestFmt;   // "Melhor (V%u): %s"
    const char* raceResWorstFmt;  // "Pior:   %s"
    const char* raceResAvgFmt;    // "Media:  %s"
    const char* raceResIdealFmt;  // "Volta ideal: %s"
    const char* raceResHint;      // "[V] voltas   [ENTER] sair"

    // Detalhe da volta
    const char* lapTitle;         // "DETALHE DA VOLTA"
    const char* lapFmt;           // "VOLTA %u / %u"
    const char* lapBest;          // "MELHOR VOLTA"
    const char* lapHint;          // "[;] ant  [.] prox  [`] voltar"

    // Histórico
    const char* histTitle;        // "HISTORICO"
    const char* histEmpty;        // "Nenhuma sessao gravada."
    const char* histHint;         // "[;][.] navegar [ENTER] abrir [`] sair"

    // Trajeto — ao vivo
    const char* tripTitle;        // "TRAJETO"
    const char* tripTimeFmt;      // "TEMPO %s"
    const char* tripDistFmt;      // "DIST  %.2f km"
    const char* tripMaxAvgFmt;    // "MAX %.0f  MED %.0f"

    // Trajeto — resultado
    const char* tripResTitle;     // "RESULTADO DO TRAJETO"
    const char* tripResCarFmt;    // "Carro: %s"
    const char* tripResDurFmt;    // "Duracao:    %s"
    const char* tripResDistFmt;   // "Distancia:  %.2f km"
    const char* tripResMaxFmt;    // "Vel. maxima: %.1f km/h"
    const char* tripResAvgFmt;    // "Vel. media:  %.1f km/h"
    const char* tripResHint;      // "[ENTER] sair"

    // Genéricos
    const char* busyTitle;        // "AGUARDE"
    const char* errTitle;         // "ERRO"

    // Mensagens (main.cpp)
    const char* busyImport;       // "Importando GPX..."
    const char* errImport;        // "Falha ao importar o GPX."
};

// Idioma atual (referência sempre válida).
const Lang& L();

LangId langCur();
void    langSet(LangId id);
void    langToggle();       // cicla pt <-> en e salva no SD

void    langLoad();         // lê /lang.txt do SD (chamar após initSd)
void    langSave();         // grava /lang.txt no SD
