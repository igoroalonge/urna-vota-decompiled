// FRAGMENT of uenux2/src/app/comum/dados/asn/cconversorpartido.cpp (path inferred; file of unit u22, which wrote
// DoDesconverte = func 11459; class declared in cconversorentidadepartidos.h).
// Reconstructed from vota_web_wasm.wasm (unit u35: DoConverte = func 11460).
//
//   Partido ::= SEQUENCE { numero INTEGER (0..99), sigla GeneralString (SIZE(1..24)), nome GeneralString (SIZE(1..55)) }
// md::CPartido: +0 uint16 número, +4 std::string sigla, +16 std::string nome.
#include "comum/dados/asn/cconversorentidadepartidos.h"

namespace comum::asn {

// wasm func 11460 - vtable slot 2. Not observed executing (the urna never writes a party file).
// Fields are assigned in the order nome, sigla, número.
ModuloPartidos::Partido CConversorPartido::DoConverte(const md::CPartido& partido) const
{
    ModuloPartidos::Partido entidade;                   // SEQUENCE(info @1146428)
    entidade.set_nome(partido.GetNome());               // field 2 <- +16
    entidade.set_sigla(partido.GetSigla());             // field 1 <- +4
    entidade.set_numero(partido.GetNumero());           // field 0 <- uint16 +0
    return entidade;
}

} // namespace comum::asn
