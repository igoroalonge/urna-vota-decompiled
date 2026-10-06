// Reconstructed from vota_web_wasm.wasm (unit u08).
// Original: uenux2/src/app/vota/eleitor/fimvotacao/cgerabu.h (path inferred from cgerabu.cpp)
//
// "Gera BU" = first step of the encerramento chain of the VOTA application: it renders the
// Boletim de Urna (BU, the section's vote count) as a text report "bu.dat" in the work area of the
// internal flash, using comum::CGeradorBU (uenux2/src/app/comum/relatorios/cgeradorbu.cpp), and
// registers the comum::CCalculaCV ("calcula código verificador") poly-singleton that the report
// sources feed while the BU text is produced. EstadoVota must be EAVGERARBU (59); afterwards it is
// EAVGERARRELATORIOS (60) and the next state is CGeraRelatorios.
//
// RTTI: api::CState <- comum::CAppState <- vota::CGeraBU (typeinfo @1540424, vtable @1540372)
//   [0] 448 dtor (ICF)  [1] 765 deleting dtor (ICF)  [2] StartState 12110  [3] NeedChangeState 7480
//   [4] GetNextState 1661  [5..8] nops
#pragma once

#include <memory>

#include "comum/cappstate.h"

namespace vota {

class CPreShowFormVota;   // vota/eleitor/comum (unit u07)

class CGeraBU final : public comum::CAppState {
public:
    /// Lazy singleton, wasm func 6129 (not in this unit), 28 bytes:
    ///   CAppState(0);
    ///   m_telaVotacaoEncerrada = CPreShowFormVota({StatusHeader(5), "Votação encerrada",
    ///                                              "Por favor, aguarde..."}, "telaVotacaoEncerrada");
    ///   m_telaPreparandoDados  = CPreShowFormVota({..., "Preparando dados para encerramento", ...},
    ///                                              "telaPreparandoDadosEncerramento")   (func 6583)
    static CGeraBU& GetInst();

    void StartState() override;       // wasm func 12110 (srcloc cgerabu.cpp:54)

private:
    std::shared_ptr<CPreShowFormVota> m_telaVotacaoEncerrada;   // +12 (+16)
    std::shared_ptr<CPreShowFormVota> m_telaPreparandoDados;    // +20 (+24); size 28
};

}  // namespace vota
