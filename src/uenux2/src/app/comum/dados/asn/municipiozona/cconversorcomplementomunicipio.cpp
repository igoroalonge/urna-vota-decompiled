// uenux2/src/app/comum/dados/asn/municipiozona/cconversorcomplementomunicipio.cpp   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u21).
//
// Both methods were named by the tool after the INLINED IConversorASN<HorarioVerao, CHorarioVerao>::Converte /
// Desconverte (iconversorasn.h:56 / :71); the vtable slots of CConversorComplementoMunicipio give the real names.
#include "comum/dados/asn/municipiozona/cconversorcomplementomunicipio.h"

#include "comum/dados/asn/municipiozona/cconversorcomplementosmunicipios.h"   // CConversorHorarioVerao (vtable @1569388)

namespace comum::asn {

// wasm func 11383 (vtable slot 2). Not observed executing.
ModuloComplementosMunicipios::ComplementoMunicipio
CConversorComplementoMunicipio::DoConverte(const md::CComplementoMunicipio& complemento) const
{
    ModuloComplementosMunicipios::ComplementoMunicipio entidade;
    entidade.set_codigoMunicipio(complemento.GetCodigoMunicipio());   // via a Constrained_INTEGER<1, 99999> temporary
    entidade.set_fuso(complemento.GetFuso());
    // desligaColetaBiometria (field 2) keeps its default (FALSE).
    if (complemento.PossuiHorarioVerao()) {                            // optional flag +28
        entidade.set_horarioVerao(CConversorHorarioVerao().Converte(complemento.GetHorarioVerao()));  // func 2795, :56
    } else {
        entidade.omit_horarioVerao();
    }
    return entidade;
}

// wasm func 11382 (vtable slot 3). Observed executing (-cm.dat and the Local file at votaInit).
md::CComplementoMunicipio
CConversorComplementoMunicipio::DoDesconverte(const ModuloComplementosMunicipios::ComplementoMunicipio& complemento) const
{
    const auto fuso = static_cast<std::int16_t>(complemento.get_fuso());
    const TMunicipioID codigo = complemento.get_codigoMunicipio();
    if (complemento.horarioVerao_isPresent()) {
        const md::CHorarioVerao horario = CConversorHorarioVerao().Desconverte(complemento.get_horarioVerao());  // :71
        return md::CComplementoMunicipio(codigo, fuso, horario);      // func 3710
    }
    return md::CComplementoMunicipio(codigo, fuso);                   // func 3711
}

} // namespace comum::asn
