// Reconstructed from vota_web_wasm.wasm (unit u08).
// Original: uenux2/src/app/vota/eleitor/fimvotacao/cgerarelatorios.h (path inferred)
//
// "Gera relatórios" = second step of the encerramento chain, right after CGeraBU. It renders, as text
// files in the work area of the internal flash, the other end-of-day reports:
//   buj.dat   Boletim de Justificativa Eleitoral (BUJ)            always
//   bim.dat   Boletim de Identificação de Mesários (BIM)           when mesário identification is on
//   behb.dat  Eleitores com habilitação biográfica (BEHB)          on biometric urnas, outside demo mode
// and mirrors each one (CSincronizaVota::SincronizaRelatorios). EstadoVota must be EAVGERARRELATORIOS
// (60); afterwards EAVIMPRIMIRBU (61) and the next state is CInicioBU (printing of the BU copies).
//
// RTTI: api::CState <- comum::CAppState <- vota::CGeraRelatorios (typeinfo @1540576, vtable @1540524)
//   [0] 244 dtor (ICF)  [1] 387 deleting dtor (ICF)  [2] StartState 12105  [3] NeedChangeState 7480
//   [4] GetNextState 1661  [5..8] nops
#pragma once

#include <memory>

#include "comum/cappstate.h"

namespace vota {

class CPreShowFormVota;   // vota/eleitor/comum (unit u07)

class CGeraRelatorios final : public comum::CAppState {
public:
    /// Lazy singleton, wasm func 6084 (not in this unit), 20 bytes:
    ///   CAppState(0); m_telaPreparandoDados = "Preparando dados para encerramento" (func 6583)
    static CGeraRelatorios& GetInst();

    void StartState() override;       // wasm func 12105 (srcloc cgerarelatorios.cpp:51)

private:
    std::shared_ptr<CPreShowFormVota> m_telaPreparandoDados;   // +12 (+16); size 20
};

}  // namespace vota
