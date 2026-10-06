// ecourna-lib/ecourna/app/dados/asn/federacoes/cconversorfederacao.cpp  (path inferred: no
// std::source_location in this class; directory by analogy with asn/midias/, asn/parametrizacaourna/)
// Reconstructed from vota_web_wasm.wasm, unit u14 (DoConverte only; DoDeconverte func 9216 is in unit u40).
//
//   Federacao ::= SEQUENCE { identificador INTEGER (100..999), sigla GeneralString (SIZE(1..55)),
//                            nome GeneralString (SIZE(1..84)), partidos SEQUENCE OF INTEGER (0..99) }
//   <-> CFederacao
#include "ecourna/app/dados/asn/cconversores.h"

namespace ecourna::app::dados::asn {

// wasm func 9217 (vtable slot 2; the tool shows it as "CConversorFederacao::vf2")
CConversorFederacao::TEntidade CConversorFederacao::DoConverte(const TDado& federacao) const
{
    ModuloFederacoes::Federacao entidade;
    entidade.set_identificador(federacao.GetID());                // Constrained_INTEGER<100, 999>
    entidade.set_sigla(federacao.GetSigla());
    entidade.set_nome(federacao.GetNome());
    const auto partidos = federacao.GetPartidos();                // copied (auto, not auto&)
    for (const auto numero : partidos) {
        // Constrained_INTEGER<0, 99>(numero), cloned (INTEGER::do_clone) into the SEQUENCE OF
        entidade.ref_partidos().push_back(ASN1::Constrained_INTEGER<ASN1::FixedConstraint, 0, 99>(numero));
    }
    return entidade;
}

} // namespace ecourna::app::dados::asn
