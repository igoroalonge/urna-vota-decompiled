// FRAGMENT reconstructed by unit u09 from vota_web_wasm.wasm.
// Original file (path inferred): uenux2/src/app/vota/eleitor/fimvotacao/caplicacaoencerrada.cpp
// vota::CAplicacaoEncerrada : CEstadoComDesligamentoAutomatico (typeinfo @1542072, vtable @1542032):
// "application finished" screen; [2] StartState = func 12058 (not in this unit).
// Layout: 36 bytes = CEstadoComDesligamentoAutomatico (28) + m_tela shared_ptr (+28/+32).

#include "vota/eleitor/fimvotacao/caplicacaoencerrada.h"

#include <memory>
#include <mutex>

#include "vota/eleitor/comum/ctelasvota.h"

namespace vota {

// wasm func 3877 — lazy singleton (callers: CQuerImprimirBU::ProcessInput, CMostraQRCodeBU)
CAplicacaoEncerrada& CAplicacaoEncerrada::GetInst()
{
    static std::unique_ptr<CAplicacaoEncerrada> s_inst;     // @1833820 (mutex residue @1833796)
    static std::mutex s_mutex;
    std::lock_guard<std::mutex> lock(s_mutex);
    if (!s_inst)
        s_inst.reset(new CAplicacaoEncerrada());
    return *s_inst;
}

CAplicacaoEncerrada::CAplicacaoEncerrada()
    : CEstadoComDesligamentoAutomatico(0),                            // func 1285 (flags 0)
      m_tela(CTelasVota::GetInst().m_telaAplicacaoEncerrada)          // CTelasVota +36/+40  name inferred
{
}

}  // namespace vota
