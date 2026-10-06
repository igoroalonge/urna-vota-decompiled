// Reconstructed from vota_web_wasm.wasm (unit u30).
// Original: uenux2/mock/app/simulador/wasm/cwasmbeep.h (path inferred: every simulador::CWasm* mock of the web
// build lives in uenux2/mock/app/simulador/wasm/, e.g. cwasmthread.cpp and cwasmclp.cpp are attested by srcloc).
//
// simulador::CWasmBeep : api::IBeep   (typeinfo @1530348, vtable @1530312, 4 bytes = vptr only)
// Registered as the api::IBeep poly-singleton by simulador::CSimuladorWasm::Executa (func 8302, unit u19).
// The urna's buzzer becomes Web Audio square-wave tones queued in JavaScript (Module.uenuxBeep, created by
// js_wasm_beep_init: gain 0.2, queue of at most 32 tones, unlocked on the first pointer/key/touch event).
#pragma once

#include <vector>

// api::IBeep (uenux2/src/api/hwil/ibeep.h, path inferred) - slot order from the CWasmBeep vtable; the
// destructor is declared LAST in the interface (slots 7/8). Method names inferred from the callers:
//
//   class IBeep {
//   public:
//       virtual void SetVolume(int volume) = 0;                       // [0]
//       virtual void Beep(int frequenciaHz, int duracao) = 0;         // [1] duracao in 1/100 s (10 ms units)
//       virtual void BeepConfirma(int vezes) = 0;                     // [2] vezes 1 = cargo confirmed
//                                                                     //     (CEleitorVotando::NeedChangeState),
//                                                                     //     4 = end of the vote (CFimVotoEleitor)
//       virtual void BeepAlerta() = 0;                                // [3] CVerificaEleicaoPassou::StartState
//       virtual void BeepFinalizacao() = 0;                           // [4] CThreadMonitor::SaiPorVotacaoSuspensa
//                                                                     //     (std::async), CInformacaoZeresimaTardia
//       virtual void BeepSuspensao() = 0;                             // [5] CEleitorVotando::DescartaVotos
//       virtual void BeepErro() = 0;                                  // [6] refused key (CVotacaoStateAudio::
//                                                                     //     PlayKey, CInputMenuField, CMenuValidation)
//       virtual ~IBeep();                                             // [7] / [8]
//   };
#include "api/hwil/ibeep.h"

extern "C" {
// TSE imports implemented in upstream/site/wasm/vota_web_wasm.js
void js_wasm_beep_init();                                   // creates Module.uenuxBeep (once)
void js_wasm_beep_queue(int frequenciaHz, int duracaoMs);   // one square-wave tone, 1-8 ms fade in/out
void js_wasm_beep_set_volume(int volume);                   // gain = volume / 100
}

namespace simulador {

class CWasmBeep final : public api::IBeep {
public:
    CWasmBeep();                                           // inlined into func 8302
    ~CWasmBeep() override = default;                       // slot 7 = ICF 174 "return this", slot 8 = ICF 144

    void SetVolume(int volume) override;                   // wasm func 8541  slot 0
    void Beep(int frequenciaHz, int duracao) override;     // wasm func 8538  slot 1
    void BeepConfirma(int vezes) override;                 // wasm func 8533  slot 2
    void BeepAlerta() override;                            // wasm func 8523  slot 3
    void BeepFinalizacao() override;                       // wasm func 8514  slot 4
    void BeepSuspensao() override;                         // wasm func 8505  slot 5
    void BeepErro() override;                              // wasm func 8494  slot 6

private:
    struct STom {                                          // 8 bytes: {frequência Hz, duração em 10 ms}
        int frequencia;
        int duracao;
    };
    void Toca(const std::vector<STom>& tons);              // inlined in every melody               name inferred
};

}  // namespace simulador
