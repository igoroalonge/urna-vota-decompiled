// FRAGMENT of uenux2/src/app/comum/dados/asn/cconversorsecaoeleitoral.cpp (path inferred; file of unit u21, which wrote
// DoConverte = func 11454 and the class declaration).
// Reconstructed from vota_web_wasm.wasm (unit u35: DoDesconverte = func 11453).
//
//   SecaoEleitoral ::= SEQUENCE { tipo TipoLocalVotacao, secao IdentificacaoSecaoEleitoral {municipioZona {municipio,
//                                 zona}, local, secao}, agregadas SEQUENCE OF IdentificacaoAgregada OPTIONAL }
#include "comum/dados/asn/cconversorsecaoeleitoral.h"

#include <algorithm>
#include <vector>

#include "comum/asn/util.h"
#include "comum/dados/asn/cconversoridentificacaoagregada.h"   // vtable @1561564
#include "comum/md/cidentificacaosecao.h"

namespace comum::asn {

// wasm func 11453 - vtable slot 3. Observed executing: the section file "<mun><zona><seção>-lo.dat" (ModuloLocal) is
// read at votaInit (CConversorLocal -> here).
// IConversorASN<IdentificacaoAgregada>::Desconverte (iconversorasn.h:71, code 7654 "Entidade está inválida: {}",
// with ASN1::trace_invalid) is inlined for every aggregated section, then the list is SORTED by número (std::sort,
// introsort func 5719) before md::CSecaoEleitoral's constructor validates it (csecaoeleitoral.cpp:34/37/41, see
// src/uenux2/src/app/comum/dados/md/csecaoeleitoral.u35.cpp).
md::CSecaoEleitoral CConversorSecaoEleitoral::DoDesconverte(const ModuloLocal::SecaoEleitoral& secao) const
{
    const auto& id = secao.get_secao();
    const md::CIdentificacaoSecao identificacao(id.get_municipioZona().get_municipio(),
                                                static_cast<TZonaID>(id.get_municipioZona().get_zona()),
                                                id.get_local(),
                                                static_cast<TSecaoID>(id.get_secao()));       // func 5887 (u24)
    const ETipoLocalVotacao tipo = Utils::DesconverteTipoLocalVotacao(secao.get_tipo());      // func 3800

    const CConversorIdentificacaoAgregada conversor;
    std::vector<md::CIdentificacaoAgregada> agregadas;
    if (secao.hasOptionalField(0)) {
        for (const auto& agregada : secao.get_agregadas())
            agregadas.push_back(conversor.Desconverte(agregada));                             // slot 3 = 11455
        std::sort(agregadas.begin(), agregadas.end(),
                  [](const auto& a, const auto& b) { return a.GetNumero() < b.GetNumero(); });   // func 5719
    }
    return md::CSecaoEleitoral(tipo, identificacao, agregadas);
}

} // namespace comum::asn
