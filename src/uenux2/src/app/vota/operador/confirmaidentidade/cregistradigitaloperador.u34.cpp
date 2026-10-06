// FRAGMENT reconstructed by unit u34 from vota_web_wasm.wasm.
// Original files: uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp (attested, srclocs
// :104/:105) and uenux2/src/app/comum/comparecimentomesario/estados/cpededigitalmesario.cpp (attested, :298/:299).
// Both FinishState methods are already written in those files (units u10 and u22); this is the shared body.
//
// WEB BUILD: dead code (operator thread not run; no fingerprint reader registered).
#include <source_location>

#include "api/hwil/ifingerscanner.h"
#include "api/pattern/cpolysingletonlist.h"

namespace vota {
namespace {

// wasm func 6012 (tools: api_f6012) - merged body (merge-similar-functions) of
//   vota::CRegistraDigitalOperador::FinishState (10453: records cregistradigitaloperador.cpp:104 / :105)
//   comum::CPedeDigitalMesario::FinishState     (10315: records cpededigitalmesario.cpp:298 / :299)
// The two std::source_location records of the poly-singleton lookups are the parameters. wasm signature:
// api_f6012(this (unused), locFim (:105 / :299), locLed (:104 / :298)) - the SECOND record is used first.
//                                                                                          name inferred
void FinalizaLeitorDigital(const std::source_location& locLed, const std::source_location& locFim)
{
    auto& leitor = api::CPolySingletonList::instance<api::IFingerScanner>(locLed);   // func 816
    leitor.SetLed(api::ELedLeitor::APAGADO);                                           // slot 7 (0)
    api::CPolySingletonList::instance<api::IFingerScanner>(locFim).FinalizaCaptura();  // slot 6
}

}  // namespace
}  // namespace vota
