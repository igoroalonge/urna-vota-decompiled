// Reconstructed from vota_web_wasm.wasm (unit u06).
// Original: uenux2/src/app/vota/eleitor/cinstrucaovotacaoacessibilidade.h (path inferred)
//
// First sub-state of CEleitorVotando when the voter uses audio (headphones, accessibility): shows
// "telaInstrucoesAcessibilidade", speaks how to use the keypad and lets the voter tune the speech
// volume (keys 3/9) and speed (keys 4/6). CONFIRMA leaves the state (next = nullptr) and the vote
// for the first cargo starts.
//
// RTTI: comum::CAppState <- vota::CVotacaoStateAudio <- vota::CInstrucaoVotacaoAcessibilidade
//       (typeinfo @1533808, vtable @1533616, 16 slots)
#pragma once

#include <memory>
#include <string>

#include "vota/eleitor/cvotacaostateaudio.h"      // unit u08
#include "vota/eleitor/comum/ctelasvota.h"        // CFormVota / CFormInterativoTelaVota (unit u07)

namespace vota {

class CInstrucaoVotacaoAcessibilidade final : public CVotacaoStateAudio {
public:
    /// Lazy singleton, wasm func 4420 (constructor inlined):
    ///   CVotacaoStateAudio(2 = keyboard), m_tela = CTelasVota::GetInst().m_telaInstrucoesAcessibilidade
    static CInstrucaoVotacaoAcessibilidade& GetInst();

    ~CInstrucaoVotacaoAcessibilidade() override;            // slot 0 func 2466, slot 1 func 7255

    void ProcessInputAudio() override;                      // slot 9  func 7248 (srcloc 54/59/60)
    void StartStateAudio() override;                        // slot 10 func 7242 (srcloc 129/131)
    /// slot 12 is pure in CVotacaoStateAudio; this class overrides it with a no-op (func 425, own
    /// table slot 944), otherwise it could not be instantiated.
    void ProcessTickAudio(uebyte) override {}
    std::string GetMensagemAudio() const override;          // slot 15 func 4407 name inferred

    /// Called at start-up (inlined into func 7787) to pre-synthesise the instruction audio.
    void CacheInstructionAudio() const;                     // srcloc :157/:161, not in this unit

private:
    std::string GetKeyboardPosition() const;                // srcloc :175, inlined into slot 15

    CFormInterativoTelaVota m_tela;                         // +28 (+32 control block); size 36
                                                            // (CFormInterativoTelaVota is itself a shared_ptr:
                                                            //  func 4420 copies CTelasVota +124/+128 and
                                                            //  increments the control block)
};

}  // namespace vota
