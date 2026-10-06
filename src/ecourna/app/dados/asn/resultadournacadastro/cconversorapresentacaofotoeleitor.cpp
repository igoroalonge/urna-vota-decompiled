// ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorapresentacaofotoeleitor.cpp
// (original build path: /home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/app/dados/asn/resultadournacadastro/cconversorapresentacaofotoeleitor.cpp)
// Reconstructed from vota_web_wasm.wasm, unit u14.
//
//   ApresentacaoFotoEleitor ::= SEQUENCE { estadoApresentacao EstadoApresentacaoFotoEleitor (1..3),
//                                          resultadoApresentacao ResultadoApresentacaoFotoEleitor (0..8) }
//   <-> CApresentacaoFotoEleitor (estado 0-based, resultado same numbers). The four helpers are static.
#include "ecourna/app/dados/asn/cconversores.h"
#include "ecourna/app/dados/dadoserros.h"

namespace ecourna::app::dados::asn {

// wasm func 9089 (srcloc line 44)
ModuloResultadoUrnaCadastro::EstadoApresentacaoFotoEleitor
CConversorApresentacaoFotoEleitor::ConverteEstado(CApresentacaoFotoEleitor::EEstado estado)
{
    if (static_cast<unsigned>(estado) >= 3) {
        throw CAsnResultadoUrnaCadastroError(2654, "Estado de apresentação da foto do eleitor inválido.");    // line 44
    }
    return ModuloResultadoUrnaCadastro::EstadoApresentacaoFotoEleitor(
        static_cast<ModuloResultadoUrnaCadastro::EstadoApresentacaoFotoEleitor::NamedNumber>(estado + 1));
}

// inlined into func 9087 (srcloc line 62)
CApresentacaoFotoEleitor::EEstado
CConversorApresentacaoFotoEleitor::DeconverteEstado(ModuloResultadoUrnaCadastro::EstadoApresentacaoFotoEleitor estado)
{
    const unsigned valor = static_cast<unsigned>(estado.asInt() - 1);
    if (valor >= 3) {
        throw CAsnResultadoUrnaCadastroError(2655, "Estado de apresentação da foto do eleitor inválido.");    // line 62
    }
    return static_cast<CApresentacaoFotoEleitor::EEstado>(valor);
}

// wasm func 9088 (srcloc line 90)
ModuloResultadoUrnaCadastro::ResultadoApresentacaoFotoEleitor
CConversorApresentacaoFotoEleitor::ConverteResultado(CApresentacaoFotoEleitor::EResultado resultado)
{
    if (static_cast<unsigned>(resultado) >= 9) {
        throw CAsnResultadoUrnaCadastroError(2656, "Resultado da apresentação da foto do eleitor inválido."); // line 90
    }
    return ModuloResultadoUrnaCadastro::ResultadoApresentacaoFotoEleitor(
        static_cast<ModuloResultadoUrnaCadastro::ResultadoApresentacaoFotoEleitor::NamedNumber>(resultado));
}

// inlined into func 9087 (srcloc line 120)
CApresentacaoFotoEleitor::EResultado
CConversorApresentacaoFotoEleitor::DeconverteResultado(ModuloResultadoUrnaCadastro::ResultadoApresentacaoFotoEleitor resultado)
{
    if (static_cast<unsigned>(resultado.asInt()) >= 9) {
        throw CAsnResultadoUrnaCadastroError(2657, "Resultado da apresentação da foto do eleitor inválido."); // line 120
    }
    return static_cast<CApresentacaoFotoEleitor::EResultado>(resultado.asInt());
}

// wasm func 9091 (vtable slot 2)
CConversorApresentacaoFotoEleitor::TEntidade CConversorApresentacaoFotoEleitor::DoConverte(const TDado& foto) const
{
    ModuloResultadoUrnaCadastro::ApresentacaoFotoEleitor entidade;
    entidade.set_estadoApresentacao(ConverteEstado(foto.GetEstado()));
    entidade.set_resultadoApresentacao(ConverteResultado(foto.GetResultado()));
    return entidade;
}

// wasm func 9087 (vtable slot 3; the tool named it after the inlined DeconverteEstado)
CConversorApresentacaoFotoEleitor::TDado CConversorApresentacaoFotoEleitor::DoDeconverte(const TEntidade& foto) const
{
    const auto estado = DeconverteEstado(foto.get_estadoApresentacao());
    return CApresentacaoFotoEleitor(estado, DeconverteResultado(foto.get_resultadoApresentacao()));   // shared_f1081
}

} // namespace ecourna::app::dados::asn
