// Reconstructed from vota_web_wasm.wasm (unit u08).
// Original: uenux2/src/app/vota/eleitor/cvotacaostateaudio.h (path inferred from cvotacaostateaudio.cpp,
// which is attested by std::source_location records at lines 69..388).
//
// CVotacaoStateAudio ("voting state with audio") is the abstract base of every per-cargo sub-state of
// CEleitorVotando (CPedeMajoritario, CPedeProporcional, CConfirmaVotoEmCargo, IConfereVotoEmCargo,
// CInstrucaoVotacaoAcessibilidade, ...). It adds the *voter audio* (accessibility, headphones) on top
// of comum::CAppState:
//   * after entering the state it waits 1.5 s (tick m_tickInicio) and speaks the state's message
//     (GetMensagemAudio() with its {tags} replaced by FormataMensagem());
//   * when the spoken message ends it arms a 2 s tick (m_tickRepeticao) and repeats the message,
//     forever, until a key is pressed or the state is left;
//   * every key is echoed (PlayKey): digits / BRANCO / CORRIGE are spoken, CONFIRMA is spoken and
//     waited for, an invalid key beeps and logs "Tecla indevida pressionada".
// Subclasses implement the *Audio hooks (slots 9..15).
//
// RTTI: api::CState <- comum::CAppState <- vota::CVotacaoStateAudio (typeinfo @1535192, vtable @1534936)
//       <- CInstrucaoVotacaoAcessibilidade, CConfirmaVotoSemCandidato, CPedeMajoritario,
//          CPedeProporcional, CCompletaProporcional (<- CPedeNominal, CPedeNulo),
//          CConfirmaVotoEmCargo (<- CConfirmaProporcional (<- CCandidatoInapto, CConfirmaVotoLegenda,
//          CCandidatoInexistente, CConfirmaVotoNominal, CProporcionalBranco, CProporcionalNulo),
//          CConfirmaMajoritario (<- CMajoritarioBranco, CMajoritarioNulo, CMajoritarioRepetido,
//          CMajoritarioValido)), IConfereVotoEmCargo (<- CConfereVotoEmCargo<>)
//
// Vtable (16 slots; slots 0..8 come from api::CState / comum::CAppState):
//   [0] ~CVotacaoStateAudio (func 1035)   [1] deleting dtor (ICF 325 = unreachable in the abstract base)
//   [2] StartState (7028)   [3] NeedChangeState (CAppState 7480)   [4] GetNextState (1661)
//   [5] FinishState (7022)  [6] ProcessMessage (nop 425)           [7] ProcessInput (7029)
//   [8] ProcessTick (7010)  [9] ProcessInputAudio = 0              [10] StartStateAudio = 0
//   [11] FinishStateAudio (nop 218)   [12] ProcessTickAudio = 0    [13] EmiteEcoComInputField (3139)
//   [14] FormataMensagem (6999)       [15] GetMensagemAudio = 0
#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <utility>

#include "comum/cappstate.h"                     // comum::CAppState (path inferred)
#include "api/gui/cinteractiveform.h"            // api::EInputResult

namespace api { class IEsperaAudio; }

namespace vota {

using uebyte = std::uint8_t;

class CFormInterativoTelaVota;   // vota/eleitor/comum/ctelasvota.h (unit u07)

/// api::EInputResult values used by this file (names as in unit u06; 13 name inferred)
///   5 = Corrige (CORRIGE key), 9 = Confirma (CONFIRMA key), 13 = Tecla (a digit / BRANCO typed)

class CVotacaoStateAudio : public comum::CAppState {
public:
    // Constructor: inline, emitted as wasm func 1785 (attributed to celeitorvotando.cpp by the tools)
    //   CAppState(flags); m_audioHabilitado = false;
    //   m_tickRepeticao = CThreadEleitor::GetInst().CriaTick(2000);   // func 807
    //   m_tickInicio    = CThreadEleitor::GetInst().CriaTick(1500);
    explicit CVotacaoStateAudio(uebyte flags);

    ~CVotacaoStateAudio() override;                                            // [0] func 1035

    void StartState() override;                                                // [2] func 7028
    void FinishState() override;                                               // [5] func 7022
    void ProcessInput() override;                                              // [7] func 7029 (srcloc 69)
    void ProcessTick(uebyte tick) override;                                    // [8] func 7010 (srcloc 120)

    // ---- hooks for the concrete states --------------------------------------------------------
    virtual void ProcessInputAudio() = 0;                                      // [9]
    virtual void StartStateAudio() = 0;                                        // [10]
    virtual void FinishStateAudio() {}                                         // [11] (nop, func 218)
    virtual void ProcessTickAudio(uebyte tick) = 0;                            // [12]

    /// Reads the (single) input field of the voting screen and echoes the key (srcloc 188).
    virtual std::pair<api::EInputResult, std::string>
    EmiteEcoComInputField(const CFormInterativoTelaVota& tela) const;          // [13] func 3139

    /// Replaces the {tags} of an audio template. name inferred (unit u06 uses the same name)
    virtual std::string FormataMensagem(const std::string& modelo) const;      // [14] func 6999

    /// Audio template of the state, e.g. "Você está votando para {cargo-atual}. {quantidade-digitos}.
    /// Voto {progresso}..." (CPedeProporcional). name inferred
    virtual std::string GetMensagemAudio() const = 0;                          // [15]

    // ---- non-virtual helpers ------------------------------------------------------------------
    void PlayKey(const char tecla) const;                                      // func 1455 (srcloc 136)
    static void PlayInterruptibleMessage(const std::string& mensagem);         // func 3143 (srcloc 172/173)
    static void PlayMessage(const std::string& mensagem);                      // func 1204 (srcloc 178/179)
    api::EInputResult EmiteEcoCorrigeConfirma(const CFormInterativoTelaVota& tela) const; // srcloc 209,
                                                                               //   inlined into func 11754
    static void PlayFile(const std::string& arquivo);                          // srcloc 226, inlined in 1455
    std::string FormatPartyName(const std::string& numeroPartido);             // func 6954 (srcloc 297)

protected:
    void IniciarEsperaFimAudio();                                              // srcloc 388, inlined in 7010
    void CancelaEsperaFimAudio();                                              // name inferred, always inlined
    void PararTicks();                                                         // name inferred, always inlined

    // layout (28 bytes): CAppState = +0 vptr, +4 m_proximoEstado, +8/+9/+10 flags
    bool   m_audioHabilitado = false;                  // +11  voter audio on for this state
    uebyte m_tickRepeticao;                            // +12  2000 ms, repeats the message after it ends
    uebyte m_tickInicio;                               // +13  1500 ms, first utterance after StartState
    std::unique_ptr<api::IEsperaAudio> m_reservado;    // +16  (?) only destroyed in the dtor (slot 1 call)
    std::shared_ptr<api::IEsperaAudio> m_esperaFimAudio; // +20 (+24 control block)
};

}  // namespace vota
