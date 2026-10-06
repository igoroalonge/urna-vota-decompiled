// ecourna-lib/ecourna/app/dados/asn/federacoes/cconversorfederacoes.cpp   (path inferred: the data class
//   CFederacao is srcloc-attested in ecourna/app/dados/federacoes/cfederacao.cpp)
// Reconstructed from vota_web_wasm.wasm (unit u11). Only DoDeconverte (+ its std::transform) is in u11.
//
// "Federações partidárias" (party federations, file <..>-fe.dat). Every scenario of the simulator ships
// 25-byte files that hold only the cabeçalho: the optional list is absent, so the result is empty.
#include <algorithm>
#include <iterator>
#include <vector>

#include "ecourna/api/asn/iconversorasn.hpp"
#include "ecourna/app/dados/asn/cconversorcabecalhoentidade.h"
#include "ecourna/app/dados/asn/federacoes/cconversorfederacao.h"   // 9216/9217
#include "ecourna/app/dados/federacoes/cfederacoes.h"
#include "ModuloFederacoes.h"

namespace ecourna::app::dados::asn {

// RTTI: CConversorFederacoes : IConversorASN<ModuloFederacoes::EntidadeFederacoes, CFederacoes>
// vtable @1124660: [0] 174  [1] 144  [2] 9212 DoConverte (unit u40)  [3] 9207 DoDeconverte
//
//   EntidadeFederacoes ::= SEQUENCE { cabecalho CabecalhoEntidade, federacoes SEQUENCE OF Federacao OPTIONAL }
//   Federacao ::= SEQUENCE { identificador INTEGER (100..999), sigla GeneralString (SIZE(1..55)),
//                            nome GeneralString (SIZE(1..84)), partidos SEQUENCE OF INTEGER (0..99) }
class CConversorFederacoes
    : public api::asn::IConversorASN<ModuloFederacoes::EntidadeFederacoes, CFederacoes>
{
protected:
    TEntidade DoConverte(const TDado& dado) const override;          // wasm func 9212 (unit u40)
    TDado DoDeconverte(const TEntidade& entidade) const override;    // wasm func 9207
};

// wasm func 9207 (vtable slot 3; name already curated). Observed at run time (votaInit, -fe.dat).
CFederacoes CConversorFederacoes::DoDeconverte(const TEntidade& entidade) const
{
    const CConversorCabecalhoEntidade conversorCabecalho;   // vptr @1123740
    const CConversorFederacao conversorFederacao;           // vptr @1123956

    const CCabecalhoEntidade cabecalho = conversorCabecalho.Deconverte(entidade.get_cabecalho());   // func 2665

    std::vector<CFederacao> federacoes;
    if (entidade.hasOptionalField(TEntidade::e_federacoes)) {
        std::vector<CFederacao> lista;
        // wasm func 9206: this std::transform; the lambda (IConversorASN<Federacao>::Deconverte, srcloc :66
        // @1125132 -> CConversorFederacao::DoDeconverte 9216) is inlined into it.
        std::transform(entidade.get_federacoes().begin(), entidade.get_federacoes().end(), std::back_inserter(lista),
                       [&conversorFederacao](const ModuloFederacoes::Federacao& federacao) {
                           return conversorFederacao.Deconverte(federacao);
                       });
        federacoes = std::move(lista);                       // move assignment (old buffer freed by func 5798)
    }
    return CFederacoes(cabecalho, federacoes);                // func 9045 (copies the vector)
}

// -------------------------------------------------------------------------------------------------------
// Template instantiations emitted for this file (library code, summarised):
//   wasm func 9206  std::transform<SEQUENCE_OF<Federacao>::const_iterator,
//                   std::back_insert_iterator<std::vector<CFederacao>>, $lambda above>
//                   (40-byte CFederacao moved in the fast path; slow path func 9204 "libcxx_f9204")
//   func 1272 ("unknown_f1272") = std::vector<CFederacao>::__destroy_vector, func 5798 = vector deallocate
// -------------------------------------------------------------------------------------------------------

}  // namespace ecourna::app::dados::asn
