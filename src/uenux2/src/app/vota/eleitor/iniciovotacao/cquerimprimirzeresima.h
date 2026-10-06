// Reconstructed from vota_web_wasm.wasm (unit u39 = StartState; GetInst by u07, ProcessInput by u20).
// Original (path inferred by u07): uenux2/src/app/vota/eleitor/iniciovotacao/cquerimprimirzeresima.h
//
// "Quer imprimir a zerésima?" (zerésima = the report printed before voting that proves every candidate
// starts with zero votes). Asked only in voter-training mode ("treinamento de eleitor"), where printing it
// is optional: CORRIGE -> estadoVota VOTAR, CInicioVotacao; CONFIRMA -> CConfirmaImpressaoZeresima.
//
// RTTI: comum::CAppState <- vota::CQuerImprimirZeresima (typeinfo @1544884, vtable @1544848, 20 bytes)
//   [0] ICF 244 [1] ICF 387 [2] StartState 11921 [7] ProcessInput 11920 (u20)
// Lazy singleton func 5957 (@1834324, mutex @1834300): CAppState(2), m_tela = CTelasVota +228.
//
// WEB BUILD: unreachable (votaInit jumps straight to estadoVota VOTAR).
#pragma once

#include "comum/cappstate.h"
#include "vota/eleitor/comum/ctelasvota.h"

namespace vota {

class CQuerImprimirZeresima final : public comum::CAppState {
public:
    static CQuerImprimirZeresima& GetInst();           // wasm func 5957 (u07)

    void StartState() override;                        // [2] wasm func 11921
    void ProcessInput() override;                      // [7] wasm func 11920 (u20)

private:
    CQuerImprimirZeresima();

    CFormInterativoTelaVota m_tela;                    // +12 = CTelasVota::m_telaQuerImprimirZeresima (+228)
};

}  // namespace vota
