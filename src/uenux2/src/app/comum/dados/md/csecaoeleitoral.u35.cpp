// FRAGMENT of uenux2/src/app/comum/dados/md/csecaoeleitoral.cpp (attested by srclocs :34, :37, :41).
// Reconstructed from vota_web_wasm.wasm (unit u35). The constructor exists only inlined into
// comum::asn::CConversorSecaoEleitoral::DoDesconverte (wasm func 11453).
//
// md::CSecaoEleitoral (36 bytes): +0 ETipoLocalVotacao tipo, +4 CIdentificacaoSecao (20 bytes: município +8, zona
// +12, local +16, seção +20 of the object), +24 std::vector<CIdentificacaoAgregada> agregadas (8-byte elements
// {+0 ETipoLocalVotacao tipoLocalOrigem, +4 uint16 numero}).
// "Seção agregada" = a small section whose voters vote on this urna (merged into the principal section).
#include <set>
#include <vector>

#include "comum/dados/dadosdefs.h"   // CDadosError (EUeComumDadosError)

namespace comum::md {

CSecaoEleitoral::CSecaoEleitoral(ETipoLocalVotacao tipo, const CIdentificacaoSecao& secao,
                                 const TVectorIdentificacaoAgregada& agregadas)
    : m_tipo(tipo), m_secao(secao), m_agregadas(agregadas)
{
    std::set<TSecaoID> numeros;
    for (const CIdentificacaoAgregada& agregada : m_agregadas) {
        numeros.insert(agregada.GetNumero());
        if (agregada.GetNumero() < 1 || agregada.GetNumero() > 9999)
            throw CDadosError(EUeComumDadosError{8053}, "Agregada inválida.");               // line 34
        if (agregada.GetNumero() == m_secao.GetSecao())
            throw CDadosError(EUeComumDadosError{8054}, "Agregada igual à principal.");      // line 37
    }
    if (numeros.size() != m_agregadas.size())
        throw CDadosError(EUeComumDadosError{8055}, "Agregada repetida.");                   // line 41
}

} // namespace comum::md
