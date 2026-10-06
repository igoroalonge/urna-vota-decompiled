// uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversorestadogeralsa.cpp
// Reconstructed from vota_web_wasm.wasm (unit u03).
//
// State of the SA (Sistema de Apuração, the counting application used when an urna fails), file sa.bin:
//   EstadoGeralSA ::= SEQUENCE { estadoSA EstadoSA, atualizacaoBloqueada BOOLEAN,
//                                dadosSecao DadoSecaoSA { municipio, zona, secao } }
//   EstadoSA ::= ENUMERATED { inicial(0) ... encerrada(22), ultimoid(23) }   (24 values)
// md EEstadoSA is a char enum: value v (0..22) is stored as '1' + v, i.e. '1'..'9', ':'..'@', 'A'..'G';
// 'H' (the md "ultimo id" sentinel) and the ASN.1 ultimoid(23) are refused.
// Observed at run time: sa.bin after votaInit = {inicial, FALSE, {municipio 1, zona 1, secao 2}}.
#include "comum/dados/asn/estadoaplicacao/cconversorestadogeralsa.h"

#include <format>

namespace comum::asn {

// Inlined into wasm func 11392 (srcloc lines 96, 100)
ModuloEstadoGeralSA::EstadoSA CConversorEstadoGeralSA::ConverteEstadoSA(const EEstadoSA estado) const
{
    const char c = static_cast<char>(estado);
    if (c >= '1' && c <= 'G') {   // compiled as a 23-entry br_table
        return ModuloEstadoGeralSA::EstadoSA(c - '1');
    }
    if (c == 'H') {               // md "ultimo id" sentinel (explicit case inside the switch)
        throw CDadosError(7925, std::format("Valor inválido para EEstadoSA: {}", estado));   // line 96
    }
    throw CDadosError(7926, std::format("Valor inválido para EEstadoSA: {}", estado));       // line 100
}

// Inlined into wasm func 11393 (srcloc lines 156, 160)
EEstadoSA CConversorEstadoGeralSA::DesconverteEstadoSA(const ModuloEstadoGeralSA::EstadoSA& estado) const
{
    const int valor = estado.asInt();
    if (valor >= ModuloEstadoGeralSA::EstadoSA::inicial && valor <= ModuloEstadoGeralSA::EstadoSA::encerrada) {
        return static_cast<EEstadoSA>('1' + valor);
    }
    if (valor == -1 /* ? generated "invalid" enumerator */ || valor == ModuloEstadoGeralSA::EstadoSA::ultimoid) {
        throw CDadosError(7927, std::format("Valor inválido para EstadoSA: {}", estado));   // line 156
    }
    throw CDadosError(7928, std::format("Valor inválido para EstadoSA: {}", estado));       // line 160
}

// wasm func 11392 (vtable slot 2; named after the inlined ConverteEstadoSA)
ModuloEstadoGeralSA::EstadoGeralSA CConversorEstadoGeralSA::DoConverte(const TDado& estado) const
{
    ModuloEstadoGeralSA::EstadoGeralSA entidade;
    entidade.set_estadoSA(ConverteEstadoSA(estado.GetEstado()));
    entidade.set_atualizacaoBloqueada(estado.GetAtualizacaoBloqueada());
    auto& secao = entidade.ref_dadosSecao();
    secao.set_municipio(estado.GetMunicipio());
    secao.set_zona(estado.GetZona());
    secao.set_secao(estado.GetSecao());
    return entidade;
}

// wasm func 11393 (vtable slot 3; named after the inlined DesconverteEstadoSA)
md::estadoaplicacao::CEstadoGeralSA CConversorEstadoGeralSA::DoDesconverte(const TEntidade& estado) const
{
    const auto& secao = estado.get_dadosSecao();
    return md::estadoaplicacao::CEstadoGeralSA(DesconverteEstadoSA(estado.get_estadoSA()),
                                               estado.get_atualizacaoBloqueada(),
                                               secao.get_municipio(),
                                               static_cast<TZonaID>(secao.get_zona()),     // short
                                               static_cast<TSecaoID>(secao.get_secao()));  // short
}

} // namespace comum::asn

// ----------------------------------------------------------------------------------------------------------
// Emitted in this TU: wasm func 5625 — md::estadoaplicacao::CEstadoGeralSA::CEstadoGeralSA(EEstadoSA, bool,
// TMunicipioID, TZonaID, TSecaoID), a plain member-wise constructor (5 stores, no validation). Also used by
// the simulator fixture mock_f10101. Real home: md/estadoaplicacao/cestadogeralsa.{h,cpp} (path inferred).
