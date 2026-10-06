// FRAGMENT reconstructed by unit u15 from vota_web_wasm.wasm.
// Original: uenux2/src/api/gui/capplicationcontextstack.h (+ .cpp, srcloc capplicationcontextstack.cpp:45).
//
// "Application context" = the text the urna shows if something fails while an operation is running.
// Code that performs a risky step (writing the BU, copying results to the MR, signing files...) opens a
// CApplicationContextGuard; if an exception escapes, the monitor thread (vota::CThreadMonitor, func 7710)
// takes the top of the stack and builds the fatal-error screen from it:
//     "<m_titulo> (<error code>)", m_mensagem, the list of recommended actions and a QR code.
#pragma once

#include <string>
#include <vector>

namespace api {

// Recommended action shown on the error screen; text produced by ActionsToText (wasm func 5555).
// Enumerator names are inferred from the texts (the source only reveals the type name "Actions",
// in the fallback message "Actions({}) - não reconhecida").
enum class Actions : int {
    DesligueUrna = 0,                       // "Desligue a urna."
    ReinicieUrna = 1,                       // "Desligue e ligue a urna."
    ReinicieOuSubstituaUrna = 2,            // + "Se o erro persistir, substitua a urna."
    ConfiraImpressora = 3,                  // "Desligue a urna, confira o encaixe da impressora e ligue novamente."
                                            //   + "Se o erro persistir, substitua o módulo impressor."
    ReinicieOuSubstituaMidiaVotacao = 4,    // + "... substitua a mídia de votação."
    ReinicieOuSubstituaMidiaExterna = 5,    // + "... substitua a mídia externa."
    ReinicieOuSubstituaMidiaAplicacao = 6,  // + "... substitua a mídia de aplicação."
    ReinicieOuSubstituaMidiaInterna = 7,    // + "... substitua a mídia interna."
    ReinicieOuSubstituaMidiaResultado = 8,  // + "... substitua a mídia de resultados."
    ReinicieOuApureEmOutraUrna = 9,         // + "Se o erro persistir, faça a apuração em outra urna."
    EnvieParaJunta = 10,                    // "Desligue a urna eletrônica e envie-a para a Junta Eleitoral."
    ReinicieOuFotografeQRCode = 11,         // + "Se o erro persistir, tire uma foto dessa tela ... QR Code ..."
    NovaCarga = 12,                         // "Desligue a urna e faça uma nova carga."
    InsiraMidiaVotacaoDeOutraUrna = 13,     // "Desligue a urna, insira uma mídia de votação usada de outra urna ..."
    InsiraMidiaVotacaoValida = 14,          // "Desligue a urna, insira uma mídia de votação válida e tente novamente."
    SubstituaUrna = 15,                     // "Desligue a urna e substitua-a."
    AjusteHorarioComADH = 16,               // "Desligue a urna, retire a mídia de votação e a mídia de resultado e
                                            //  utilize o ADH para ajustar o horário desta urna, ou leve ..."
};

// 52 bytes (vector element size 52 in func 5554).
struct CApplicationContext {
    // wasm func 5557: member-wise move constructor from all five parts          // name inferred
    CApplicationContext(std::string detalhe, std::string titulo, std::string mensagem,
                        std::vector<std::string> acoes, bool generico);
    // wasm func 3682: context with an Actions code (generico = false)            // name inferred
    CApplicationContext(Actions acao, std::string detalhe, std::string titulo, std::string mensagem);
    // wasm func 5558: "generic" context: fixed QR-code hint, Actions::ReinicieUrna, generico = true
    CApplicationContext(std::string detalhe, std::string titulo);                 // name inferred
    CApplicationContext(const CApplicationContext&);                              // func 1841 (other unit)
    ~CApplicationContext();                                                       // func 1468 (shared_f1468)

    std::string m_detalhe;              // +0   e.g. "Não é possível continuar a execução", "" in most guards
    std::string m_titulo;               // +12  e.g. "Erro inesperado", "Gravando o banco de dados na MV"
    std::string m_mensagem;             // +24  e.g. "Ocorreu um erro durante a persitência dos dados na MV."
    std::vector<std::string> m_acoes;   // +36  ActionsToText(acao)
    bool m_generico;                    // +48  true only for the default/fallback context
};

// wasm func 5555 (tools: vota_f5555)                                             // name inferred
std::vector<std::string> ActionsToText(Actions acao);

// Global stack: std::vector<CApplicationContext> at @1839212 (begin) / @1839216 (end) / @1839220 (cap).
// Static destructor: func 11160 (unknown_f11160).
class CApplicationContextStack {
public:
    static CApplicationContextStack& GetInst();          // the object is a plain global in this build
    bool Push(CApplicationContext contexto);             // func 3684 (void in the binary: DAE)
    void Pop();                                           // used by ~CApplicationContextGuard (func 675)
    CApplicationContext Top() const;                      // func 1695 (vota_f1695, other unit)
    void Clear();                                         // inlined in CApplication::InitApplication
private:
    std::vector<CApplicationContext> m_pilha;
};

// 56 bytes: +0 CApplicationContextStack& m_pilha, +4 CApplicationContext m_contexto.
class CApplicationContextGuard {
public:
    CApplicationContextGuard(CApplicationContextStack& pilha, CApplicationContext contexto);  // srcloc :45
    // The overload that the callers actually use; func 676 is this one, with the
    // (CApplicationContextStack&, CApplicationContext) constructor inlined into it.
    CApplicationContextGuard(Actions acao, std::string detalhe, std::string titulo, std::string mensagem)
        : CApplicationContextGuard(CApplicationContextStack::GetInst(),
                                   CApplicationContext(acao, std::move(detalhe), std::move(titulo),
                                                       std::move(mensagem))) {}
    ~CApplicationContextGuard();                                                  // func 675 (other unit)
private:
    CApplicationContextStack& m_pilha;   // +0 (always @1839212)
    CApplicationContext m_contexto;      // +4
};

} // namespace api
