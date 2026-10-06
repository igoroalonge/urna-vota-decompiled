// uenux2/src/app/comum/dados/asn/cconversorpartido.cpp  (path inferred: md/cpartido.cpp is attested,
// and the converters of top-level md classes sit in dados/asn/, e.g. cconversorlocal.cpp)
// Reconstructed from vota_web_wasm.wasm (unit u22). The md validation it inlines is attested by
// std::source_location records cpartido.cpp:30, :33, :36 (CPartido::ValidaCriacao).
//
// RTTI: comum::asn::CConversorPartido : IConversorASN<ModuloPartidos::Partido, md::CPartido>
//   vtable @1561268: [0] 174 [1] 144 [2] 11460 DoConverte (other unit) [3] 11459 DoDesconverte
//   Partido ::= SEQUENCE { numero INTEGER (0..99), sigla GeneralString (SIZE(1..24)),
//                          nome GeneralString (SIZE(1..55)) }                (file <...>-pa.dat)
// md::CPartido (28 bytes): +0 uint16 numero, +4 std::string sigla, +16 std::string nome.
#include "comum/dados/asn/cconversorpartido.h"

#include "comum/dados/dadosdefs.h"   // CDadosError

namespace comum {

namespace md {
// cpartido.cpp:30..36 (inlined into func 11459). Only upper bounds are checked: an EMPTY sigla or
// nome passes (the ASN.1 lower bound SIZE(1..) is left to the decoder).
void CPartido::ValidaCriacao() const
{
    if (m_numero >= 100)
        throw CDadosError(EUeComumDadosError{8050}, "Número do partido inválido");    // :30
    if (m_sigla.size() >= 25)
        throw CDadosError(EUeComumDadosError{8051}, "Sigla do partido inválida");     // :33
    if (m_nome.size() >= 56)
        throw CDadosError(EUeComumDadosError{8052}, "Nome do partido inválido");      // :36
}
} // namespace md

namespace asn {
// wasm func 11459 (vtable slot 3)
md::CPartido CConversorPartido::DoDesconverte(const ModuloPartidos::Partido& partido) const
{
    return md::CPartido(static_cast<std::uint16_t>(partido.get_numero()), partido.get_sigla(),
                        partido.get_nome());           // constructor calls ValidaCriacao()
}
} // namespace asn

} // namespace comum
