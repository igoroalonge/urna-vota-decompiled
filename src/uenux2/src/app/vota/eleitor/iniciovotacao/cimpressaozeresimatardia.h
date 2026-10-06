// Reconstructed from vota_web_wasm.wasm (unit u39 = StartState; ProcessInput by u20, GetInst in u26's notes).
// Original (path inferred): uenux2/src/app/vota/eleitor/iniciovotacao/cimpressaozeresimatardia.h
// (next to cinformacaozeresimatardia.cpp, its successor; the vtable sits between CGeraZeresima and
// CConfirmaImpressaoZeresima).
//
// "Impressão tardia da zerésima": the mesário asked for the zerésima after the time limit for it (more than
// the configured margin after its scheduled time, CVerificaHorarioZeresima). Before generating it the urna
// asks whether its clock is right. CONFIRMA (clock right) -> estadoVota GERARZE, CGeraZeresima;
// CORRIGE (clock wrong) -> CInformacaoZeresimaTardia, which switches the urna off.
//
// RTTI: comum::CAppState <- vota::CImpressaoZeresimaTardia (typeinfo @1544684, vtable @1544648, 20 bytes)
//   [0] ICF 244 [1] ICF 387 [2] StartState 11932 [7] ProcessInput 11931 (u20)
// Lazy singleton @1834240: CAppState(2), m_tela = CTelasVota +60 (inlined into CVerificaHorarioZeresima).
//
// WEB BUILD: unreachable (votaInit jumps straight to estadoVota VOTAR).
#pragma once

#include "comum/cappstate.h"
#include "vota/eleitor/comum/ctelasvota.h"

namespace vota {

class CImpressaoZeresimaTardia final : public comum::CAppState {
public:
    static CImpressaoZeresimaTardia& GetInst();

    void StartState() override;                        // [2] wasm func 11932
    void ProcessInput() override;                      // [7] wasm func 11931 (u20)

private:
    CImpressaoZeresimaTardia();

    CFormInterativoTelaVota m_tela;                    // +12 (+16) = CTelasVota +60
};

}  // namespace vota
