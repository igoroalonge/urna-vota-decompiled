// uenux2/src/app/comum/dados/md/municipiozona/ccomplementomunicipio.cpp  -- FRAGMENT written by unit u21
// (the rest of the file, GetHorarioVerao and ValidaCriacao, is in ccomplementomunicipio.cpp, unit u05).
//
// Layout check from these constructors: +0 codigo (uint32), +4 fuso (int16), +8 std::optional<CHorarioVerao>
// (20 bytes, engaged flag +28). Nothing is stored at +6: the "bool m_desligaColetaBiometria // +6 ?" of the u05
// header does not exist in this build (the ASN.1 field is dropped by CConversorComplementoMunicipio).
#include "ccomplementomunicipio.h"

namespace comum::md {

// wasm func 3710 - name inferred. Callers: CConversorComplementoMunicipio::DoDesconverte (11382),
// CConversorDadosDisponiveisCarga::DoConverte (11424), CConversorLocal::DoConverte (11451).
CComplementoMunicipio::CComplementoMunicipio(TMunicipioID codigo, std::int16_t fuso, const CHorarioVerao& horarioVerao)
    : m_codigoMunicipio(codigo), m_fuso(fuso), m_horarioVerao(horarioVerao)
{
    ValidaCriacao();   // func 5647: "Código de município inválido." (8113) / "Fuso-horário inválido." (8114)
}

// wasm func 3711 - name inferred. Same callers. Observed executing.
CComplementoMunicipio::CComplementoMunicipio(TMunicipioID codigo, std::int16_t fuso)
    : m_codigoMunicipio(codigo), m_fuso(fuso), m_horarioVerao(std::nullopt)
{
    ValidaCriacao();
}

}  // namespace comum::md
