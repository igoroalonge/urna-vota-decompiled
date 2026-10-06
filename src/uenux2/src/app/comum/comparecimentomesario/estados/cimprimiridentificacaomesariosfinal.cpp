// uenux2/src/app/comum/comparecimentomesario/estados/cimprimiridentificacaomesariosfinal.cpp
// Reconstructed from vota_web_wasm.wasm (unit u22). Attested by the std::source_location record :229
// (GetNomeMesario). Only this static helper is listed in the unit; it is used by
// vota::CGeraRelatorios::StartState (func 12105, u09) to print the mesários' names on the BIM report
// ("comparecimento de mesários na abertura / no fechamento").
#include "comum/comparecimentomesario/estados/estadosregistromesarios.h"

#include "comum/dados/celeitores.h"
#include "ecourna/api/exception/assert.h"   // UEASSERT -> CBaseError<api::EUeAssertError> (factory func 463)

namespace comum {

// wasm func 5385 (srcloc :229)
std::string CImprimirIdentificacaoMesariosFinal::GetNomeMesario(const std::string& titulo)
{
    const CEleitorDetalhe* mesario = CEleitores::GetInst().Busca(titulo, ETipoIdentificador::TITULO);   // func 2264
    UEASSERT(mesario);                           // throws EUeAssertError 3409 "Assert (mesario)" (:229)
    const auto& eleitor = mesario->GetEleitor();
    // nome social when present, otherwise the name; at most 40 characters (the MT/printer width)
    const std::string& nome = eleitor.GetNomeSocial().empty() ? eleitor.GetNome() : eleitor.GetNomeSocial();
    return nome.substr(0, 40);
}

} // namespace comum
