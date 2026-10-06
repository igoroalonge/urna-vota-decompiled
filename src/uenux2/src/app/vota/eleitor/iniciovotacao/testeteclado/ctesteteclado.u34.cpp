// FRAGMENT reconstructed by unit u34 from vota_web_wasm.wasm.
// Original file: uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.cpp (attested by
// srcloc; class reconstructed in ctesteteclado.cpp). Text source of the "Teste Falhou" screen
// (CTesteFalhou, "telaTeclaErradaTesteTeclado"), bound as CDataText<std::string (*)()> (table slot 1106).
//
// WEB BUILD: the keyboard test ("teste de teclado") runs before the zerésima; not reached in the simulator.
#include <format>
#include <string>

#include "api/hwil/iinput.h"                          // api::KeyName (func 744)
#include "vota/eleitor/comum/cinformacaoeleitor.h"

namespace vota::testeteclado {
namespace {

// wasm func 12903 (table slot 1106)                                                    name inferred
// The two keys were stored in CInformacaoEleitor by CTesteTeclado::ProcessInput (11804) when the wrong key
// was pressed: +9 = expected key, +10 = pressed key.
std::string TextoFalhaTesteTeclado()
{
    const CInformacaoEleitor& info = CInformacaoEleitor::GetInst();                   // func 509
    return std::format("Falha: Esperada {}, pressionada {}",                          // @3201
                       api::KeyName(info.GetTeclaTesteEsperada()),                    // +9
                       api::KeyName(info.GetTeclaTestePressionada()));                // +10
}

}  // namespace
}  // namespace vota::testeteclado
