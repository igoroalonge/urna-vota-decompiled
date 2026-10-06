// uenux2/src/app/comum/dados/asn/processoeleitoral/cconversornomescargo.cpp  (path inferred: next to
// cconversordetalhecandidato.cpp, which uses this converter)
// Reconstructed from vota_web_wasm.wasm (unit u22). The md constructor it inlines is attested by
// std::source_location records cnomescargo.cpp:28, :31, :34, :37.
//
// RTTI: comum::asn::CConversorNomesCargo : IConversorASN<ModuloEleicao::NomesCargo, md::CNomesCargo>
//   vtable @1570172: [0] 174 [1] 144 [2] 11368 (base DoConverte: "não implementado") [3] 11369 DoDesconverte
//   NomesCargo ::= SEQUENCE { nomeNeutro GeneralString (SIZE(1..36)), nomeMasculino (1..36),
//                             nomeFeminino (1..36), nomeAbreviado (1..10) }
#include "comum/dados/asn/processoeleitoral/cconversornomescargo.h"

#include "comum/dados/dadosdefs.h"   // CDadosError

namespace comum {

namespace md {
// md::CNomesCargo constructor (cnomescargo.cpp, inlined into func 11369). Only emptiness is checked;
// the ASN.1 size limits are enforced by the BER decoder.
CNomesCargo::CNomesCargo(const std::string& neutro, const std::string& masculino,
                         const std::string& feminino, const std::string& abreviado)
    : m_neutro(neutro), m_masculino(masculino), m_feminino(feminino), m_abreviado(abreviado)
{
    if (m_neutro.empty())
        throw CDadosError(EUeComumDadosError{8155}, "Nome neutro inválido.");        // cnomescargo.cpp:28
    if (m_masculino.empty())
        throw CDadosError(EUeComumDadosError{8156}, "Nome masculino inválido.");     // :31
    if (m_feminino.empty())
        throw CDadosError(EUeComumDadosError{8157}, "Nome feminino inválido.");      // :34
    if (m_abreviado.empty())
        throw CDadosError(EUeComumDadosError{8158}, "Nome abreviado inválido.");     // :37
}
} // namespace md

namespace asn {
// wasm func 11369 (vtable slot 3)
md::CNomesCargo CConversorNomesCargo::DoDesconverte(const ModuloEleicao::NomesCargo& nomes) const
{
    return md::CNomesCargo(nomes.get_nomeNeutro(), nomes.get_nomeMasculino(),
                           nomes.get_nomeFeminino(), nomes.get_nomeAbreviado());
}
} // namespace asn

} // namespace comum
