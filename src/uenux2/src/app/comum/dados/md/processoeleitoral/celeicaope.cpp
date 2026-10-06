// uenux2/src/app/comum/dados/md/processoeleitoral/celeicaope.cpp
// Reconstructed from vota_web_wasm.wasm (unit u22). Attested by std::source_location records :70
// (ValidaOrdem) and :144 (GetCargos). ValidaNome (:63) and ValidaCargos (:79, :87, :92) exist only
// inlined into comum::asn::CConversorEleicaoPE::DoDesconverte (func 11371, unit u03).
//
// comum::md::CEleicaoPE (52 bytes; "PE" = processo eleitoral): +0 TEleicaoID id, ... +28
// TVectorCargo m_cargos (std::vector<CCargo>, 140-byte elements).
#include "comum/dados/md/processoeleitoral/celeicaope.h"

#include <format>

#include "comum/dados/dadosdefs.h"   // CDadosError

namespace comum::md {

// wasm func 3714 (srcloc :70) - each cargo must have a distinct order of acquisition / printing
// (called for "aquisição" and "impressão", `tipo` names the kind in the message).
void CEleicaoPE::ValidaOrdem(std::set<uebyte>& ordens, uebyte ordem, const std::string& tipo) const
{
    if (!ordens.insert(ordem).second)
        throw CDadosError(EUeComumDadosError{8153},
                          std::format("Ordem de {} com valor {} repetido", tipo, ordem));   // :70
}

// wasm func 2257 (srcloc :144) - observed executing
const TVectorCargo& CEleicaoPE::GetCargos() const
{
    if (m_cargos.empty())
        throw CDadosError(EUeComumDadosError{8154}, "Eleição não tem cargos definidos.");   // :144
    return m_cargos;
}

} // namespace comum::md
