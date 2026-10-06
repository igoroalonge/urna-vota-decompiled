// uenux2/src/app/comum/dados/md/processoeleitoral/cdetalhecandidato.cpp
// Reconstructed from vota_web_wasm.wasm (unit u22). Attested by the std::source_location record :27.
// Layout (64 bytes, see u03's cconversordetalhecandidato.h): +0 bool temFoto; +4 CNomesCargo nomes
// (4 strings); +52 std::vector<CSuplencia> suplencias (52-byte elements).
#include "comum/dados/md/processoeleitoral/cdetalhecandidato.h"

#include "comum/dados/dadosdefs.h"   // CDadosError

namespace comum::md {

// wasm func 1545 (srcloc :27). `suplente` is 1-based (1 = vice / 1º suplente).
const CSuplencia& CDetalheCandidato::GetSuplente(uebyte suplente) const
{
    if (suplente == 0 || suplente > m_suplencias.size())
        throw CDadosError(EUeComumDadosError{8145}, "Suplente inexistente");               // :27
    return m_suplencias[suplente - 1];
}

} // namespace comum::md
