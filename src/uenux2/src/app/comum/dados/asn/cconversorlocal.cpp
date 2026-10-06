// uenux2/src/app/comum/dados/asn/cconversorlocal.cpp
// Reconstructed from vota_web_wasm.wasm (unit u03).
//
// Where this urna is: file <mun><zona><secao>-lo.dat (e.g. 0000100010001-lo.dat) and the local record
// written back by the urna:
//   Local ::= SEQUENCE { idPE INTEGER (0..99999), pais GeneralString, uf UF {sigla, nome}, municipio Municipio,
//                        complementoMunicipio ComplementoMunicipio,
//                        id CHOICE { secao [1] SecaoEleitoral, contingencia [2] IdentificacaoUrnaContingencia } }
// A section urna carries its SecaoEleitoral; a contingency (spare) urna only carries municipality + zone.
#include "comum/dados/asn/cconversorlocal.h"

#include <string>

#include "comum/dados/asn/municipiozona/cconversorcomplementomunicipio.h"
#include "comum/dados/asn/cconversormunicipio.h"
#include "comum/dados/asn/cconversorsecaoeleitoral.h"

namespace comum::asn {

// wasm func 11452 (vtable slot 3; srcloc line 70)
md::CLocal CConversorLocal::DoDesconverte(const TEntidade& local) const
{
    const TPEID idPE = local.get_idPE();
    const std::string pais = local.get_pais();
    const std::string siglaUf = local.get_uf().get_sigla();
    const std::string nomeUf = local.get_uf().get_nome();

    const md::CMunicipio municipio = CConversorMunicipio().Desconverte(local.get_municipio());
    const md::CComplementoMunicipio complemento =
        CConversorComplementoMunicipio().Desconverte(local.get_complementoMunicipio());
    // Municipio.comBiometria is [2] BOOLEAN OPTIONAL: absent means "false".
    const bool comBiometria = local.get_municipio().comBiometria_isPresent() && local.get_municipio().get_comBiometria();

    md::CInfoMunicipio info(municipio.GetCodigo(), municipio.GetNome(), complemento.GetFuso(), comBiometria);   // func 5676
    if (complemento.GetHorarioVerao().has_value()) {
        info = md::CInfoMunicipio(municipio.GetCodigo(), municipio.GetNome(), complemento.GetFuso(), comBiometria,
                                  *complemento.GetHorarioVerao());                                              // func 5674
    }

    const auto& id = local.get_id();
    switch (id.currentSelection()) {
    case 0: {   // secao
        const md::CSecaoEleitoral secao = CConversorSecaoEleitoral().Desconverte(id.get_secao());
        // Inline md::CLocal constructor for a section urna; ends with md::CLocal::ValidaCriacao() (func 5673).
        return md::CLocal(idPE, pais, siglaUf, nomeUf, info, secao);
    }
    case 1: {   // contingencia
        const auto& municipioZona = id.get_contingencia().get_municipioZona();
        const md::CContingencia contingencia(municipioZona.get_municipio(), municipioZona.get_zona());   // func 5888
        return md::CLocal(idPE, pais, siglaUf, nomeUf, info, contingencia);
    }
    }
    throw CDadosError(7887, "CHOICE de id inválida.");   // line 70
}

// wasm func 11451 (vtable slot 2; srcloc lines 120, 124, 128)
ModuloLocal::Local CConversorLocal::DoConverte(const TDado& local) const
{
    ModuloLocal::Local entidade;
    entidade.set_idPE(local.GetIdPE());
    entidade.set_pais(local.GetPais());

    ModuloTiposEleitorais::UF uf;
    uf.set_sigla(local.GetUf());
    uf.set_nome(local.GetNomeUf());
    entidade.set_uf(uf);

    const md::CInfoMunicipio& info = local.GetInfoMunicipio();
    entidade.set_municipio(CConversorMunicipio().Converte(
        md::CMunicipio(info.GetCodigo(), info.GetNome(), info.GetComBiometria())));
    entidade.set_complementoMunicipio(CConversorComplementoMunicipio().Converte(
        info.GetHorarioVerao().has_value()
            ? md::CComplementoMunicipio(info.GetCodigo(), info.GetFuso(), *info.GetHorarioVerao())   // func 3710
            : md::CComplementoMunicipio(info.GetCodigo(), info.GetFuso())));                        // func 3711

    if (local.EhContingencia()) {   // optional<CContingencia> engaged (+136); checked BEFORE the section
        ModuloLocal::IdentificacaoUrnaContingencia contingencia;
        contingencia.ref_municipioZona().set_municipio(local.GetContingencia().GetMunicipio());
        contingencia.ref_municipioZona().set_zona(local.GetContingencia().GetZona());
        entidade.ref_id().select_contingencia() = contingencia;          // CHOICE alternative 1
    } else if (local.EhSecao()) {  // optional<CSecaoEleitoral> engaged (+120)
        entidade.ref_id().select_secao() = CConversorSecaoEleitoral().Converte(local.GetSecao());   // alternative 0
    } else {
        throw CDadosError(7888, "Tipo de local indefinido.");                          // line 120
    }

    if (!entidade.isValid()) {
        throw CDadosError(7889, "A entidade não está válida.");                        // line 124
    }
    if (!entidade.isStrictlyValid()) {
        throw CDadosError(7890, "A entidade não está estritamente válida.");           // line 128
    }
    return entidade;
}

} // namespace comum::asn

// ----------------------------------------------------------------------------------------------------------
// Emitted in this TU (inline constructors of md::CInfoMunicipio, real home md/cinfomunicipio.h; both end with
// md::CInfoMunicipio::ValidaCriacao(), func 5675; also called by CConversorDadosDisponiveisCarga::vf3):
//  * wasm func 5676 — CInfoMunicipio(TMunicipioID codigo, const std::string& nome, short fuso, bool comBiometria):
//    horarioVerao = nullopt.
//  * wasm func 5674 — CInfoMunicipio(codigo, nome, fuso, comBiometria, const CHorarioVerao& hv):
//    horarioVerao = hv (20 bytes).
