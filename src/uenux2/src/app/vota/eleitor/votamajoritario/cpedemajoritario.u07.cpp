// FRAGMENT reconstructed by unit u07 from vota_web_wasm.wasm.
// Original files: uenux2/src/app/vota/eleitor/votamajoritario/cpedemajoritario.cpp (srcloc :44) and
// uenux2/src/app/vota/eleitor/votaproporcional/cpedeproporcional.cpp (srcloc :42).
//
// wasm func 6050 is the body that wasm-opt's merge-similar-functions made common to
//   CPedeMajoritario::GetTelaCargoAtual()   (thunk func 5920: srcloc cpedemajoritario.cpp:44, code 9380)
//   CPedeProporcional::GetTelaCargoAtual()  (thunk func 5923: srcloc cpedeproporcional.cpp:42, code 9386)
// with the source_location and the error code as extra parameters. (The sibling merged body func 3921,
// for the GetTelaCargoAtual(const std::string&) overloads of CConfirmaVotoEmCargo / CConfirmaVotoSemCandidato /
// CCompletaProporcional, is written out in cconfirmavotoemcargo.cpp by unit u06.)
#include "vota/eleitor/votamajoritario/cpedemajoritario.h"

#include "vota/eleitor/comum/ctelasvota.h"
#include "comum/dados/ccargos.h"

namespace vota {

CFormInterativoTelaVota CPedeMajoritario::GetTelaCargoAtual()
{
    auto& cargos = comum::CCargos::GetInst();                                    // func 273
    if (cargos.IsEnd())                                                          // func 602
        throw CUeVotaError(9380, "O cargo atual nao esta posicionado");         // cpedemajoritario.cpp:44
    return CTelasVota::GetInst().GetTelaCargo(cargos.GetCurrent().GetId(), ETelaVotacao(0));   // func 4135
}

// CPedeProporcional::GetTelaCargoAtual() is identical with code 9386 and srcloc cpedeproporcional.cpp:42.

} // namespace vota
