// Reconstructed from vota_web_wasm.wasm (unit u39 = StartState/FinishState; GetInst by u02, tick by u26).
// Original (path inferred; included by u26 as "vota/eleitor/iniciovotacao/testeteclado/cesperaretestar.h"):
// uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cesperaretestar.h
//
// "Espera para retestar": after a failed keypad test the mesário chose "Repetir teste"; the voter screen
// shows "Por favor, espere {}s para a / realização de uma nova tentativa / de execução do teste." with a
// countdown. The wait grows by 5 s on every retry (5, 10, 15 ... s) and is never reset while the
// application runs (the singleton keeps m_segundosEspera).
//
// RTTI: comum::CAppState <- vota::CEstadoComDesligamentoAutomatico <- vota::testeteclado::CEsperaRetestar
//   (typeinfo @1546820, vtable @1546780, 44 bytes)
//   [0] ICF 785 [1] ICF 1560 [2] StartState 11825 [3] 7480 [4] 1661 [5] FinishState 11824 [6] nop [7] nop
//   [8] CEstadoComDesligamentoAutomatico::ProcessTick 12061 [9] ProcessTickNaoDesligamento 11823 (u26)
// GetInst = func 5936 (u02): CEstadoComDesligamentoAutomatico(4 = ticks), screen "telaEsperaRepetirTesteTeclado",
// m_tick = CThreadEleitor::CriaTick(100 ms) (func 807), m_segundosEspera = 0.
#pragma once

#include <cstdint>

#include "vota/eleitor/cestadocomdesligamentoautomatico.h"
#include "vota/eleitor/comum/ctelasvota.h"

namespace vota::testeteclado {

class CEsperaRetestar final : public CEstadoComDesligamentoAutomatico {
public:
    static CEsperaRetestar& GetInst();                             // wasm func 5936 (u02)

    void StartState() override;                                    // [2] wasm func 11825
    void FinishState() override;                                   // [5] wasm func 11824
    void ProcessTickNaoDesligamento(uebyte tick) override;         // [9] wasm func 11823 (u26)

private:
    CEsperaRetestar();

    // +0..+27 CEstadoComDesligamentoAutomatico
    CFormInterativoTelaVota m_tela;                                // +28 (+32)
    uebyte m_tick;                                                 // +36 100 ms countdown refresh
    std::uint32_t m_segundosEspera;                                // +40 5, 10, 15... (name inferred)
};

}  // namespace vota::testeteclado
