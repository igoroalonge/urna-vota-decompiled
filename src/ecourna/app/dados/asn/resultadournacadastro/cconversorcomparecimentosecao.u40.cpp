// FRAGMENT of ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorcomparecimentosecao.cpp (path
// inferred; class + DoDeconverte by unit u11). Reconstructed by unit u40 from vota_web_wasm.wasm.
#include "ecourna/app/dados/asn/resultadournacadastro/cconversorcomparecimentosecao.h"

#include "ecourna/app/dados/asn/cconversoridentificacaosecaoeleitoral.h"
#include "ecourna/app/dados/asn/resultadournacadastro/cconversorestadocomparecimento.h"   // u14

namespace ecourna::app::dados::asn {

// wasm func 9126 (vtable @1131064 slot 2)
//   ComparecimentoSecao ::= SEQUENCE { identificacao IdentificacaoSecaoEleitoral,
//                                      eleitores SEQUENCE OF EstadoComparecimento }
// The section's attendance: one EstadoComparecimento per voter of the roll.
ModuloResultadoUrnaCadastro::ComparecimentoSecao
CConversorComparecimentoSecao::DoConverte(const CComparecimentoSecao& dado) const
{
    const CConversorIdentificacaoSecaoEleitoral conversorSecao;     // vptr @1130788
    const CConversorEstadoComparecimento conversorEstado;           // vptr @1134808

    ModuloResultadoUrnaCadastro::ComparecimentoSecao entidade;
    entidade.set_identificacao(conversorSecao.Converte(dado.GetIdentificacao()));        // 9124 -> 9129
    // ConverteLista (thunk 9123 -> body 1970) builds a SEQUENCE_OF<EstadoComparecimento>; the generated
    // setter assigns it by copy-and-swap: SEQUENCE_OF(first, last) (thunk 9122 -> body 2926), swap of the
    // element vectors, SEQUENCE_OF_Base::clear() of the temporary.
    entidade.set_eleitores(api::asn::ConverteLista(dado.GetEleitores(), conversorEstado));
    return entidade;
}

// wasm func 9122 (table slot 6874): ASN1::SEQUENCE_OF<ModuloResultadoUrnaCadastro::EstadoComparecimento>::
// SEQUENCE_OF(const_iterator first, const_iterator last) - a 26-byte thunk into the merged template body 2926
// (element vtable @1131584, SEQUENCE_OF vtable @1131436, info @1131396). Library template instantiation.

}  // namespace ecourna::app::dados::asn
