// Reconstructed from vota_web_wasm.wasm (unit u04). Original: uenux2/src/app/comum/dados/cintegridadereferencial.cpp
#include "cintegridadereferencial.h"

#include <format>

namespace comum {

using CErroDados = ecourna::api::exception::CBaseError<EUeComumDadosError>;

// wasm func 2263 (srcloc line 344). Callers: comum_f2543 (post-load check, called at the end of the
// CompleteLoad wrapper 6734, by vota::CGravaResultado and CSincronismoVotoEleitor) and 7787.
void CIntegridadeReferencial::Lanca(const TResultado& resultado)
{
    if (!resultado.first)
        throw CErroDados(EUeComumDadosError{7871}, std::format("Integridade referencial: {}", resultado.second));
}

}  // namespace comum
