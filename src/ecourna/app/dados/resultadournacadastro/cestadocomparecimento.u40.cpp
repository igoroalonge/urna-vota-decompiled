// FRAGMENT of ecourna-lib/ecourna/app/dados/resultadournacadastro/cestadocomparecimento.cpp (srcloc-attested;
// class declared by unit u14). Reconstructed by unit u40 from vota_web_wasm.wasm.
#include "ecourna/app/dados/resultadournacadastro/cestadocomparecimento.h"

namespace ecourna::app::dados {

// wasm func 2658 (table slot 6971). The minimal form: identification + situation; every optional empty (flags
// +32, +40, +92 and the first payload bytes +24/+36/+44 cleared). The richer overloads are funcs 2193 / 2192.
// Callers: CConversorEstadoComparecimento::DoDeconverte (9081), comum::CGravadorRCSecao (11616).
CEstadoComparecimento::CEstadoComparecimento(const CRegistroIdentificacaoEleitor& id, ESituacaoComparecimento situacao)
    : m_identificacao(id)               // +0 (shared_ptr +0/+4 copied with use_count + 1, optional +8/+12, flag +16)
    , m_situacao(situacao)              // +20
{
}

} // namespace ecourna::app::dados
