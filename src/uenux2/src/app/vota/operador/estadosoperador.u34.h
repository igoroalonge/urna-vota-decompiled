// FRAGMENT header written by unit u34 (reconstructed from vota_web_wasm.wasm).
//
// Declarations of the microterminal (terminal do mesário, "MT") states whose ProcessInput() methods were
// filed in unit u34. None of these classes has a std::source_location record of its own, so every file
// name below is "(path inferred)": class CFooBar lives in cfoobar.{h,cpp} in the directory of its closest
// relatives (operador/{aguardaeleitor, confirmaidentidade, justificativa, outrasopcoes}).
// Split this header into the per-class headers when merging (the other methods and the constructors are
// in units u10, u17, u19, u27, u33 and u39; see each class comment).
//
// Common shape (RTTI: comum::CAppState <- vota::CXxx, single inheritance "si"):
//   +0  vptr
//   +4  comum::CAppState* m_proximoEstado   (this = stay; set by StartState/ProcessInput)
//   +8  bool accepts messages, +9 bool accepts keys, +10 bool accepts ticks   (CAppState(2) = keys only)
//   +12 std::shared_ptr<api::CInteractiveForm<api::IScreenMT, api::IInputMT>> m_form   (+12 ptr, +16 ctrl)
// vtable: [0] ~ (ICF 244: releases the shared_ptr at +12)  [1] deleting ~ (ICF 387)  [2] StartState
//         [3] NeedChangeState (7480)  [4] GetNextState (1661)  [5] FinishState (nop 218)
//         [6] ProcessMessage (nop 425)  [7] ProcessInput  [8] ProcessTick (nop 425)
// Slot 2 is the ICF body 1070 { m_proximoEstado = this; m_form->Show(); } unless stated otherwise.
//
// "m_form->Read()" is api::CInteractiveForm<IScreenMT, IInputMT>::Read() (cinteractiveform.h:57), inlined
// in every ProcessInput: the keypad is looked up with CPolySingletonList::instance<api::IInputMT>(loc)
// (func 383, the srcloc record being the only difference between the inlined copies), then the current
// input field m_campos.at(m_indice) (+72 vector, +84 index; std::out_of_range when past the end) reads
// one key (field vtable slot 8). Result values: 5 = CORRIGE, 9 = CONFIRMA (api::EInputResult).
//
// WEB BUILD: all of these states are dead code. The simulator never runs CThreadOperador::Run
// (func 10204): main() registers a cooperative executor that only steps the voter thread (units u10, u27).
#pragma once

#include <memory>

#include "api/gui/cinteractiveform.h"
#include "comum/cappstate.h"

namespace vota {

using TFormMT = std::shared_ptr<api::CInteractiveForm<api::IScreenMT, api::IInputMT>>;

// ---------------------------------------------------------------------------------------------------------
// uenux2/src/app/vota/operador/aguardaeleitor/cdesabilitaaudioeleitor.h  (path inferred by u17)
// "Retire o fone de ouvido da urna / CONFIRMA": shown after a voter who voted with headphones finished.
// typeinfo @1592900, vtable @1592864. Lazy singleton func 5393 (@1909260, mutex @1909236; ctor inlined,
// reconstructed in operador/u17-foreign-fragments.cpp).
class CDesabilitaAudioEleitor : public comum::CAppState {
public:
    static CDesabilitaAudioEleitor& GetInst();      // wasm func 5393 (u17)
    void StartState() override;                      // wasm func 10436 (unit u39)
    void ProcessInput() override;                    // wasm func 10435 (this unit)
private:
    CDesabilitaAudioEleitor();
    TFormMT m_form;                                  // +12
};

// ---------------------------------------------------------------------------------------------------------
// uenux2/src/app/vota/operador/aguardaeleitor/cconfirmainspecionada.h  (path inferred: next to
// CAguardaInspecao, the only state that creates it)
// "Inspeção completa / CONFIRMA: continuar a votação". typeinfo @1590824, vtable @1590788. Lazy singleton
// @1908580 (mutex @1908556), GetInst + ctor inlined into CAguardaInspecao::ProcessMessage (func 10530) when
// the voter terminal reports message 11 (inspection of the booth confirmed on the voter keyboard):
//   CAppState(2); CClockFieldMT {33,1}; (20,2) CENTRO "Inspeção completa"; (40,4) DIREITA
//   "CONFIRMA: continuar a votação"; AddInputControl; CriaFormInterativo("", true).
class CConfirmaInspecionada : public comum::CAppState {
public:
    static CConfirmaInspecionada& GetInst();         // inlined into 10530
    void ProcessInput() override;                    // wasm func 10527 (this unit)
private:
    TFormMT m_form;                                  // +12
};

// ---------------------------------------------------------------------------------------------------------
// uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaocapturada.h  (path inferred by u17)
// "Digital não capturada / CONFIRMA: tentar novamente" (the mesário's own fingerprint, used to release a
// voter whose biometrics failed). typeinfo @1592396, vtable @1592360. Lazy singleton @1909120 (mutex
// @1909096), ctor inlined into CRegistraDigitalOperador::ProcessTick (func 10451).
class CDigitalNaoCapturada : public comum::CAppState {
public:
    static CDigitalNaoCapturada& GetInst();          // inlined into 10451
    void ProcessInput() override;                    // wasm func 10458 (this unit)
private:
    TFormMT m_form;                                  // +12
};

// ---------------------------------------------------------------------------------------------------------
// uenux2/src/app/vota/operador/justificativa/{canoinformadoinvalido,celeitormenor16anos}.h (paths inferred
// by u17). Error screens of the justification flow (the mesário typed the voter's birth year in
// CPedeAnoNascimento, func 10590). Constructors/GetInst inlined into 10590, see u17-foreign-fragments.cpp.
//   CAnoInformadoInvalido  "Ano de nascimento inválido." / "CONFIRMA: tentar novamente"
//                          typeinfo @1589704, vtable @1589668, singleton @1905896 (mutex @1905872)
//   CEleitorMenor16Anos    "Eleitor não pode votar ou justificar / por não ter idade mínima" /
//                          "CONFIRMA: tentar novamente"
//                          typeinfo @1589776, vtable @1589740, singleton @1905924 (mutex @1905900)
class CAnoInformadoInvalido : public comum::CAppState {
public:
    static CAnoInformadoInvalido& GetInst();
    void ProcessInput() override;                    // wasm func 10601 -> merged body 6014 (this unit)
    TFormMT m_form;                                  // +12 (public here only for the shared helper)
};

class CEleitorMenor16Anos : public comum::CAppState {
public:
    static CEleitorMenor16Anos& GetInst();
    void ProcessInput() override;                    // wasm func 10598 -> merged body 6014 (this unit)
    TFormMT m_form;                                  // +12
};

// ---------------------------------------------------------------------------------------------------------
// uenux2/src/app/vota/operador/outrasopcoes/ccontadoresbiometria.h  (path inferred)
// "Exibir contadores" of the "outras opções" menu (CEscolheOpcao option 3/4): three counters
// "Habilitação biométrica / biográfica / sem biometria" + "CORRIGE: retornar". typeinfo @1587968,
// vtable @1587932, singleton @1905280 (mutex @1905256), ctor inlined into CEscolheOpcao::ProcessInput (10689).
class CContadoresBiometria : public comum::CAppState {
public:
    static CContadoresBiometria& GetInst();
    void ProcessInput() override;                    // wasm func 10698 (this unit)
private:
    TFormMT m_form;                                  // +12
};

// ---------------------------------------------------------------------------------------------------------
// uenux2/src/app/vota/operador/outrasopcoes/cperguntafilaeleitorvazia.h  (path inferred by u27)
// "Todas as pessoas presentes já votaram? / CORRIGE: não / CONFIRMA: sim" (asked before closing the vote).
// typeinfo @1587736, vtable @1587700 ([2] StartState = 10715, unit u39). Singleton func 5426 (@1905196),
// ctor in operador/u27-foreign-fragments.cpp.
class CPerguntaFilaEleitorVazia : public comum::CAppState {
public:
    static CPerguntaFilaEleitorVazia& GetInst();     // wasm func 5426 (u27)
    void StartState() override;                      // wasm func 10715 (u39)
    void ProcessInput() override;                    // wasm func 10713 (this unit)
private:
    CPerguntaFilaEleitorVazia();
    TFormMT m_form;                                  // +12
};

// "Aguarde até todos os / eleitores presentes votarem" (3 s, then back to CPedeIdentidade).
// typeinfo @1587680, vtable @1587628 (StartState = 10718, caguardaeleitoresvotarem.cpp, unit u27).
// 20 bytes, CAppState(0): no keys, no messages, no ticks. Its GetInst + ctor are inlined into
// CPerguntaFilaEleitorVazia::ProcessInput (10713, this unit): singleton @1905168, mutex @1905144.
class CAguardaEleitoresVotarem : public comum::CAppState {
public:
    static CAguardaEleitoresVotarem& GetInst();      // inlined into 10713 (this unit)
    void StartState() override;                      // wasm func 10718 (u27)
private:
    CAguardaEleitoresVotarem();                      // inlined into 10713 (this unit)
    TFormMT m_form;                                  // +12
};

// ---------------------------------------------------------------------------------------------------------
// uenux2/src/app/vota/operador/outrasopcoes/cconfirmaaudio.h  (path inferred by u27)
// "Deseja realmente ativar/desativar o áudio? / CORRIGE: não / CONFIRMA: sim" (menu option 1).
// typeinfo @1587184, vtable @1587148, singleton @1905000 (mutex @1904976), ctor inlined into 10689.
class CConfirmaAudio : public comum::CAppState {
public:
    static CConfirmaAudio& GetInst();
    void ProcessInput() override;                    // wasm func 10740 (this unit)
private:
    TFormMT m_form;                                  // +12
};

// The state entered on CONFIRMA: switches the voter's audio on/off by hand. typeinfo @1586996,
// vtable @1586960 ([0] ~ = ICF 448, [2] StartState = 10751 (other unit); ProcessInput = CAppState nop).
// 28 bytes. GetInst + ctor inlined into CConfirmaAudio::ProcessInput (10740, this unit):
// singleton @1904972, mutex @1904948.
class CHabilitaAudioManualmente : public comum::CAppState {
public:
    static CHabilitaAudioManualmente& GetInst();     // inlined into 10740 (this unit)
    void StartState() override;                      // wasm func 10751 (other unit)
private:
    CHabilitaAudioManualmente();                     // inlined into 10740 (this unit)
    TFormMT m_form;                                  // +12 (+16 ctrl)
    std::shared_ptr<std::string> m_texto;            // +20 (+24 ctrl): "áudio habilitado/desabilitado",
                                                     //      rewritten by StartState, shown via CTextSource
};

// ---------------------------------------------------------------------------------------------------------
// uenux2/src/app/vota/operador/outrasopcoes/chorariovotacaoterminou.h  (path inferred by u27)
// "Horario de votacao terminou! / Favor encerrar a urna! / CORRIGE" (the first text lacks its accents in the
// binary; the second needs none).
// typeinfo @1586868, vtable @1586832 ([2] StartState 10758, [7] ProcessInput 10757: unit u27).
class CHorarioVotacaoTerminou : public comum::CAppState {
public:
    static CHorarioVotacaoTerminou& GetInst();       // wasm func 5430 (this unit, ctor inlined)
    void StartState() override;                      // wasm func 10758
    void ProcessInput() override;                    // wasm func 10757
private:
    CHorarioVotacaoTerminou();                       // inlined into 5430
    TFormMT m_form;                                  // +12
};

}  // namespace vota
