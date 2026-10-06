// uenux2/src/app/comum/md/ccabecalhoentidade.cpp
// Reconstructed from vota_web_wasm.wasm (unit u24).
#include "comum/md/ccabecalhoentidade.h"

#include "comum/md/cabrangencia.h"      // CUeComumMdError

namespace comum::md {

// wasm func 1945 (srclocs lines 24, 27). Callers: CConversorCabecalhoEntidade::DoDesconverte, the result
// writers CGravadorBU / CGravadorRDV / CGravadorHashes / IGravadorEnvelope (all with the pleito id and
// tipo Pleito) and CControlaArmazenamentoDeImagens (fingerprint envelopes).
// The id bound is the IDEleitoral INTEGER (0..99999) range.
CCabecalhoEntidade::CCabecalhoEntidade(const api::CDateTime& dataGeracao, uedword id, ETipoCabecalho tipo)
    : m_dataGeracao(dataGeracao), m_id(id), m_tipo(tipo)
{
    if (id >= 100000)
        throw CUeComumMdError(EUeComumMdError{8906}, "ID inválido.");                    // line 24
    if (static_cast<unsigned>(tipo) >= 3)
        throw CUeComumMdError(EUeComumMdError{8907}, "Tipo de ID inválido.");            // line 27
}

} // namespace comum::md
