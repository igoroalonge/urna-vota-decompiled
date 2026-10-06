// uenux2/src/app/comum/dados/crdvposicionadorvota.cpp   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u35).
#include "comum/dados/crdvposicionadorvota.h"

#include <algorithm>

namespace comum {

// wasm func 11496 - vtable slot 2. Not observed executing in the recorded (web) votes: the web build's
// CSincronismoVotoEleitorWeb policy does not persist votes into the RDV (analysis/runtime/README.md).
// std::upper_bound over the cargo's votes with the three-way comparison (tipo, then digitado as bytes: memcmp of
// the common length, the shorter string first): a new vote goes AFTER every equal vote. Returns the index.
std::size_t CRdvPosicionadorVota::Posiciona(const md::CVotos& votos, const md::CVoto& voto) const
{
    const auto menor = [](const md::CVoto& a, const md::CVoto& b) {
        if (a.GetTipo() != b.GetTipo())
            return a.GetTipo() < b.GetTipo();
        return a.GetDigitado() < b.GetDigitado();           // std::string operator<=> (memcmp + length)
    };
    const auto it = std::upper_bound(votos.begin(), votos.end(), voto, menor);
    return static_cast<std::size_t>(it - votos.begin());
}

} // namespace comum
