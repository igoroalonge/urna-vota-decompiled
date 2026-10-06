// FRAGMENT of uenux2/src/app/comum/dados/ccandidaturas.cpp (srcloc-attested; class declared by unit u03 in
// ccandidaturas.h). Reconstructed by unit u40 from vota_web_wasm.wasm (the tools put this function in
// lib:ecourna because of its neighbours in the function table).
// Declaration to add to ccandidaturas.h:
//   std::vector<TCandidatoID> GetNumerosCandidatosAptos(TCargoID cargo) const;   // wasm func 2840
#include "comum/dados/ccandidaturas.h"

#include <vector>

namespace comum {

// wasm func 2840 (table slot 100). Observed executing: every voter screen of a cargo, vota_web_wasm's
// "candidates" JSON (u29), the BU QR-code generator (5604) and the BU candidate list printer
// (CParteCandidatosMajoritarios::Imprime, 11214).                                           // name inferred
// Numbers of the APT candidacies of a cargo, in map order (key = cargo * 1000000 + número). A candidacy is
// kept when its cargo byte (node +20) equals `cargo` and its situação word (node +72) is 0. The sibling
// overload with a party filter is wasm func 2272 (u04).
std::vector<TCandidatoID> CCandidaturas::GetNumerosCandidatosAptos(TCargoID cargo) const
{
    std::vector<TCandidatoID> numeros;
    for (const auto& [chave, candidatura] : m_container) {                  // in-order tree walk
        if (candidatura.GetCargo() != cargo || candidatura.GetSituacao() != 0)
            continue;
        numeros.push_back(candidatura.GetNumero());                         // node +24; vector push_back = 543
    }
    return numeros;
}

} // namespace comum
