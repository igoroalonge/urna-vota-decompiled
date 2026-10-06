// uenux2/src/app/vota/eleitor/iniciovotacao/cverificahorariozeresima.h   (path inferred from
// cverificahorariozeresima.cpp, attested by std::source_location records :64 and :80)
// Reconstructed from vota_web_wasm.wasm (unit u26).
//
// vota::CVerificaHorarioZeresima ("check the zerésima time") is the voter-terminal state that holds the urna
// until the configured date/time for printing the zerésima (the report proving that the urna holds no votes)
// has been reached. While waiting it shows "ATENÇÃO / Esta urna eletrônica só funcionará a partir de ..."
// (CTelasVota::CriaTelaAntesHorarioZeresima, func 6595) with two options: CONFIRMA prints the "estado da urna"
// report, BRANCO opens "Mais informações". A 2 s tick re-checks the clock; the base class also watches the
// battery (automatic power-off).
//
// RTTI: comum::CAppState <- vota::CEstadoComDesligamentoAutomatico <- vota::CVerificaHorarioZeresima
//       (typeinfo @1545936, vtable @1545864):
//   [0] 785 (ICF dtor)  [1] 1560 (ICF deleting dtor)  [2] StartState (11871)  [3] NeedChangeState (7480)
//   [4] GetNextState (1661)  [5] FinishState (11868)  [6] ProcessMessage (nop 425)  [7] ProcessInput (11869)
//   [8] CEstadoComDesligamentoAutomatico::ProcessTick (12061)  [9] ProcessTickNaoDesligamento (11870)
#pragma once

#include <cstdint>

#include "api/util/cdatetime.h"
#include "vota/eleitor/cestadocomdesligamentoautomatico.h"
#include "vota/eleitor/comum/ctelasvota.h"          // CFormInterativoTelaVota

namespace vota {

class CVerificaHorarioZeresima : public CEstadoComDesligamentoAutomatico {
public:
    /// wasm func 5947 (other unit; name inferred). Lazy singleton @1834688 (52 bytes). The constructor
    /// (inlined) calls CEstadoComDesligamentoAutomatico(6), copies the CTelasVota screen +20/+24
    /// (CriaTelaAntesHorarioZeresima), copies CConfiguracaoEleicao +544..+555 (data/hora da zerésima) into
    /// m_horarioZeresima and creates a stopped 2000 ms tick (CTickManager::AddStoppedTick, func 5452).
    static CVerificaHorarioZeresima& GetInst();

    void StartState() override;                             // [2] func 11871 (srcloc :64)
    void FinishState() override;                            // [5] func 11868
    void ProcessInput() override;                           // [7] func 11869
    void ProcessTickNaoDesligamento(uebyte tick) override;  // [9] func 11870 (srcloc :80)

private:
    // CEstadoComDesligamentoAutomatico: +0..+11 CAppState, +12 CDateTime limite de bateria, +24 tick (1 s)
    CFormInterativoTelaVota m_tela;              // +28 / +32
    api::CDateTime m_horarioZeresima;            // +36 (12 bytes) = CConfiguracaoEleicao +544 (func 5947)
    uebyte m_tick;                               // +48 2000 ms tick of the voter thread
};

} // namespace vota
