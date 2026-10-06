// uenux2/src/app/comum/md/cidentificacaosecao.cpp
// Reconstructed from vota_web_wasm.wasm (unit u24).
#include "comum/md/cidentificacaosecao.h"

#include "comum/md/cabrangencia.h"      // CUeComumMdError

namespace comum::md {

// wasm func 5887 (srclocs lines 38, 41; the record of line 33 exists but is unreferenced).
// Observed executing (vote_geral_t1 profile): votaInit reads the section file through
// CConversorSecaoEleitoral::vf3 (11453, which has CSecaoEleitoral::CSecaoEleitoral inlined) -> here.
// Two callers: 11453 and vota::CGravaResultado::vf2 (12098).
//
// Every caller passes tipo '1', so wasm-opt specialised the function: the EUrnaTipo parameter is gone
// (the constant 49 is passed straight to the base constructor) and the line-33 test was folded away. Its
// message "Tipo inválido para urna de seção." (@361994) is still in .rodata with no reference, and error
// code 8915 (between 8914 and 8916) does not occur anywhere in the binary.
CIdentificacaoSecao::CIdentificacaoSecao(const EUrnaTipo tipo, const TMunicipioID municipio, const TZonaID zona,
                                         const TLocalID local, const TSecaoID secao)
    : CIdentificacaoUrna(tipo, municipio, zona),                                         // wasm 5886
      m_local(local), m_secao(secao)
{
    if (tipo != EUrnaTipo{'1'})                                                          // folded away
        throw CUeComumMdError(EUeComumMdError{8915}, "Tipo inválido para urna de seção.");  // line 33 ? (code)
    if (local >= 10000)
        throw CUeComumMdError(EUeComumMdError{8916}, "Local inválido.");                 // line 38
    if (secao == 0 || secao >= 10000)                  // compiled as ((secao - 10000) & 0xFFFF) <= 55536
        throw CUeComumMdError(EUeComumMdError{8917}, "Seção inválida.");                 // line 41
}

} // namespace comum::md
