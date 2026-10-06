// FRAGMENT of uenux2/src/app/comum/dados/asn/cconversoridentificacaoagregada.cpp (path inferred; file of unit u21, which
// wrote DoDesconverte = func 11455 and the class declaration).
// Reconstructed from vota_web_wasm.wasm (unit u35: DoConverte = func 11456).
//
//   IdentificacaoAgregada ::= SEQUENCE { numero INTEGER (1..9999), tipoLocalOrigem TipoLocalVotacao }
#include "comum/dados/asn/cconversoridentificacaoagregada.h"

#include "comum/asn/util.h"

namespace comum::asn {

// wasm func 11456 - vtable slot 2. Not observed executing (no aggregated section in the scenarios; only reached by
// CConversorSecaoEleitoral::DoConverte, func 11454).
// The número passes through a temporary ASN1::Constrained_INTEGER<1, 9999> (vtable @1561632) that is built and thrown
// away; the tipo through a temporary TipoLocalVotacao assigned with ENUMERATED::operator= (ecourna_f751).
ModuloLocal::IdentificacaoAgregada CConversorIdentificacaoAgregada::DoConverte(const md::CIdentificacaoAgregada& agregada) const
{
    ModuloLocal::IdentificacaoAgregada entidade;                                               // SEQUENCE(info @1139308)
    entidade.set_numero(agregada.GetNumero());                                                 // uint16 +4
    entidade.set_tipoLocalOrigem(Utils::ConverteTipoLocalVotacao(agregada.GetTipoLocalOrigem()));   // +0, func 3801
    return entidade;
}

} // namespace comum::asn
