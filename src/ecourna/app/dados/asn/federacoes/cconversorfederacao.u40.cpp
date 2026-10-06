// FRAGMENT of ecourna-lib/ecourna/app/dados/asn/federacoes/cconversorfederacao.cpp (path inferred; class +
// DoConverte by unit u14). Reconstructed by unit u40 from vota_web_wasm.wasm.
//   Federacao ::= SEQUENCE { identificador INTEGER (100..999), sigla GeneralString (SIZE(1..55)),
//                            nome GeneralString (SIZE(1..84)), partidos SEQUENCE OF INTEGER (0..99) }
// A "federação partidária" is an alliance of parties that acts as one party for four years (since 2022).
#include "ecourna/api/pattern/cbasetype.hpp"
#include "ecourna/app/dados/asn/cconversores.h"

namespace ecourna::app::dados::asn {

// Range-checked id of a federation. Instantiation inlined into 9216; its type name is the static std::string
// "FederacaoID" built by __wasm_call_ctors at @1912196. BASIC_TYPE = 39: the srcloc record the inlined check
// passes (@1124484, cbasetype.hpp:39) carries the full signature
// "CBaseType<unsigned short, 100, 999, 39>::CBaseType(TYPE) [..., BASIC_TYPE = 39]" (between ScoreHabilitacao 38
// and TipoIdentificadorEleitor 40 of cbasetype.hpp's table).
using TFederacaoIDValidado = api::pattern::CBaseType<unsigned short, 100, 999, 39>;
using TNumeroPartidoValidado = api::pattern::CBaseType<unsigned short, 0, 99, 4>;   // "PartidoID", func 9215

// wasm func 9216 (vtable @1123956 slot 3). Not observed (the simulator's -fe.dat files have no federation).
CFederacao CConversorFederacao::DoDeconverte(const ModuloFederacoes::Federacao& entidade) const
{
    // Throws CPatternError(1300, "O tipo 'FederacaoID' deve ter valores no intervalo [100,999].") from
    // cbasetype.hpp:39 (record @1124484) - the std::format of that message is inlined here. The value is the
    // low 16 bits of the INTEGER (i32.load16_u at +8), tested as (v - 100) & 0xFFFF >= 900.
    const TFederacaoIDValidado id(static_cast<unsigned short>(entidade.get_identificador()));

    const ASN1::GeneralString sigla = entidade.get_sigla();    // AbstractString copies (func 1671)
    const ASN1::GeneralString nome = entidade.get_nome();
    const auto partidosAsn = entidade.get_partidos();          // SEQUENCE_OF copy (func 2656)

    TVectorNumeroPartido partidos;
    for (const auto& numero : partidosAsn) {
        // INTEGER copy (func 5082) -> CBaseType<unsigned short, 0, 99, 4> (func 9215, CPatternError 1300 if > 99)
        partidos.push_back(TNumeroPartidoValidado(static_cast<unsigned short>(numero)));   // push_back = func 9214
    }
    return CFederacao(id, sigla.getValue(), nome.getValue(), partidos);                     // func 9047
}

// Library code emitted for this function (listed so the unit's map is complete):
//   wasm func 9214  std::vector<unsigned short>::push_back(const unsigned short&) (with the reallocation path)
//   wasm func 5082  ASN1::INTEGER::INTEGER(const INTEGER&): copies the info pointer (+4) and the value (+8)
//   wasm func 1671  ASN1::AbstractString::AbstractString(const AbstractString&) (ConstrainedObject + std::string)

}  // namespace ecourna::app::dados::asn
