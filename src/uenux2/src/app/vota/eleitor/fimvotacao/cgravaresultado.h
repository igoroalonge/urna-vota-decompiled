// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/fimvotacao/cgravaresultado.h (path inferred from
// cgravaresultado.cpp, attested by std::source_location records at lines 63 and 79).
//
// "Grava resultado" = the encerramento step that writes the official RESULT FILES of the section
// (ASN.1 Boletim de Urna, RDV, justificativas, envelopes of the printed BU and zerésima, hashes,
// biometrics, versions of the ASN.1 contracts, log), copies them to the result directory of the
// internal flash (MI), has them signed by the SAVD service (the urna's signing daemon, HSM/MSE),
// and mirrors them onto the memory card (MV). EstadoVota must be EAVGRAVARRESULTADOS (62); the
// next state is CCopiaResultadoParaMR with EstadoVota = EAVCOPIARESULTADOSMR (63).
//
// RTTI: api::CState <- comum::CAppState <- vota::CGravaResultado (typeinfo @1540704, vtable @1540636)
//   [0] 174 trivial dtor (ICF)  [1] 144 operator delete  [2] StartState = func 12098
//   [3] NeedChangeState 7480    [4] GetNextState 1661     [5..8] no-ops
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "comum/cappstate.h"
#include "comum/gravadores/cassinador.h"     // comum::CAssinador (unit u23)
#include "comum/gravadores/igravador.h"      // comum::IGravador   (unit u23)

namespace comum { class CAbstractTelaProgresso; }

namespace vota {

class CGravaResultado final : public comum::CAppState {
public:
    /// Lazy singleton, wasm func 6037 (not in this unit): vota_f764(mutex @1833572,
    /// &s_inst @1833596, vtable @1540636, flags 0). 12 bytes = CAppState only.
    static CGravaResultado& GetInst();

    void StartState() override;       // wasm func 12098 (srcloc 63, 79)

private:
    CGravaResultado() : comum::CAppState(0) {}
};

}  // namespace vota

namespace comum {

/// Object built on CGravaResultado::StartState's stack (constructor = wasm func 5824, attributed to
/// cgravaresultado.cpp; its Executa() is inlined into func 12098). Not polymorphic, so no RTTI name:
/// the class name is inferred. Original file probably uenux2/src/app/comum/gravadores/ (unit u23).
class CGravacaoResultados {                                            // name inferred
public:
    CGravacaoResultados(const std::vector<std::shared_ptr<IGravador>>& gravadores,
                        const std::vector<std::shared_ptr<IGravador>>& gravadoresComplementares,
                        const CAssinador& assinador,
                        bool copiaParaMV,
                        std::shared_ptr<CAbstractTelaProgresso> progresso);   // wasm func 5824

    void Executa();                                                    // inlined into func 12098

private:
    std::vector<std::shared_ptr<IGravador>> m_gravadores;              // +0
    std::vector<std::shared_ptr<IGravador>> m_gravadoresComplementares; // +12 (always empty here)
    CAssinador m_assinador;                                            // +24 (48 bytes; sliced copy of
                                                                       //      vota::CAssinadorVota)
    bool m_copiaParaMV;                                                // +72 (= !EhFaseTreinamento())
    std::shared_ptr<CAbstractTelaProgresso> m_progresso;               // +76 (+80 control block)
};

}  // namespace comum
