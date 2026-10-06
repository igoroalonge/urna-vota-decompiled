// ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorcomparecimentomesario.cpp
// (original build path: /home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/app/dados/asn/resultadournacadastro/cconversorcomparecimentomesario.cpp)
// Reconstructed from vota_web_wasm.wasm, unit u14.
//
//   ComparecimentoMesario ::= SEQUENCE { identificacaoMesario RegistroIdentificacaoEleitor,
//        dataHoraColeta DataHoraJE, estadoColetaDigital EstadoColetaDigital (1..4) OPTIONAL }
//   <-> CComparecimentoMesario (EEstadoColetaDigital 0 = absent, 1..4 = the ASN values)
// A mesário (poll worker) registered as present at the opening or the closing of the section
// (parameter registrarMesarios).
#include "ecourna/app/dados/asn/cconversores.h"
#include "ecourna/app/dados/asn/resultadournacadastro/cconversorregistroidentificacaoeleitor.h"   // unit u40
#include "ecourna/app/dados/dadoserros.h"

namespace ecourna::app::dados::asn {

// wasm func 9108 (srcloc line 68): thunk into the shared body func 6151 (count 4): identity with
// range check [1, 4].
ModuloResultadoUrnaCadastro::EstadoColetaDigital
CConversorComparecimentoMesario::ConverteEstadoColetaDigital(CComparecimentoMesario::EEstadoColetaDigital estado) const
{
    if (static_cast<unsigned>(estado - 1) >= 4) {
        throw CAsnResultadoUrnaCadastroError(2635, "Estado de coleta de digital inválido.");   // line 68
    }
    return ModuloResultadoUrnaCadastro::EstadoColetaDigital(
        static_cast<ModuloResultadoUrnaCadastro::EstadoColetaDigital::NamedNumber>(estado));
}

// wasm func 9103 (srcloc line 89): identity with range check [1, 4].
CComparecimentoMesario::EEstadoColetaDigital
CConversorComparecimentoMesario::DeconverteEstadoColetaDigital(ModuloResultadoUrnaCadastro::EstadoColetaDigital estado) const
{
    if (static_cast<unsigned>(estado.asInt() - 1) >= 4) {
        throw CAsnResultadoUrnaCadastroError(2636, "Estado de coleta de digital inválido.");   // line 89
    }
    return static_cast<CComparecimentoMesario::EEstadoColetaDigital>(estado.asInt());
}

// wasm func 9110 (vtable slot 2)
CConversorComparecimentoMesario::TEntidade CConversorComparecimentoMesario::DoConverte(const TDado& mesario) const
{
    const CConversorRegistroIdentificacaoEleitor conversorRegistro;
    ModuloResultadoUrnaCadastro::ComparecimentoMesario entidade;
    entidade.set_identificacaoMesario(conversorRegistro.Converte(mesario.GetIdentificacaoMesario()));
    entidade.set_dataHoraColeta(ModuloTiposEleitorais::DataHoraJE(FormataDataHoraJE(mesario.GetDataHoraColeta())));   // func 9220
    if (mesario.GetEstadoColetaDigital() != CComparecimentoMesario::NaoInformado) {
        entidade.set_estadoColetaDigital(ConverteEstadoColetaDigital(mesario.GetEstadoColetaDigital()));   // includeOptionalField(0, 2)
    } else {
        entidade.omit_estadoColetaDigital();
    }
    return entidade;
}

// wasm func 9106 (vtable slot 3; shown as "vf3" by the tool)
CConversorComparecimentoMesario::TDado CConversorComparecimentoMesario::DoDeconverte(const TEntidade& mesario) const
{
    const CConversorRegistroIdentificacaoEleitor conversorRegistro;
    const auto identificacao = conversorRegistro.Deconverte(mesario.get_identificacaoMesario());
    const auto dataHora = ConverteDataHoraJE(mesario.get_dataHoraColeta());          // func 1877
    auto estado = CComparecimentoMesario::NaoInformado;
    if (mesario.estadoColetaDigital_isPresent()) {
        estado = DeconverteEstadoColetaDigital(mesario.get_estadoColetaDigital());
    }
    return CComparecimentoMesario(identificacao, dataHora, estado);                   // func 3486
}

} // namespace ecourna::app::dados::asn
