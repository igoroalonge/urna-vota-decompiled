// FRAGMENT of ecourna-lib/ecourna/app/dados/asn/federacoes/cconversorfederacoes.cpp (path inferred; class +
// DoDeconverte by unit u11). Reconstructed by unit u40 from vota_web_wasm.wasm.
#include "ecourna/app/dados/asn/cconversorcabecalhoentidade.h"
#include "ecourna/app/dados/asn/federacoes/cconversorfederacao.h"
#include "ecourna/app/dados/federacoes/cfederacoes.h"

namespace ecourna::app::dados::asn {

// wasm func 9212 (vtable @1124660 slot 2)
//   EntidadeFederacoes ::= SEQUENCE { cabecalho CabecalhoEntidade, federacoes SEQUENCE OF Federacao OPTIONAL }
// An empty list is written as an ABSENT optional field (what the simulator's 25-byte -fe.dat files contain).
ModuloFederacoes::EntidadeFederacoes CConversorFederacoes::DoConverte(const CFederacoes& dado) const
{
    const CConversorCabecalhoEntidade conversorCabecalho;   // vptr @1123740
    const CConversorFederacao conversorFederacao;           // vptr @1123956

    ModuloFederacoes::EntidadeFederacoes entidade;
    entidade.set_cabecalho(conversorCabecalho.Converte(dado.GetCabecalho()));          // 9211 -> 9219
    if (dado.GetFederacoes().empty()) {                                                 // vector at +16
        entidade.omit_federacoes();                                                     // removeOptionalField(0)
    } else {
        // ConverteLista thunk 9210; the generated setter (func 9209) includes the optional field and assigns.
        entidade.set_federacoes(api::asn::ConverteLista(dado.GetFederacoes(), conversorFederacao));
    }
    return entidade;
}

}  // namespace ecourna::app::dados::asn
