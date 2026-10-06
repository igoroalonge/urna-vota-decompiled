// uenux2/src/app/comum/dados/asn/municipiozona/cconversorcomplementomunicipio.h   (path inferred; included by u03's
// cconversorcomplementosmunicipios.cpp and cconversorlocal.cpp)
// Reconstructed from vota_web_wasm.wasm (unit u21).
#pragma once

#include "comum/asn/iconversorasn.h"
#include "comum/dados/md/municipiozona/ccomplementomunicipio.h"
#include "ModuloComplementosMunicipios.h"

namespace comum::asn {

// ComplementoMunicipio ::= SEQUENCE { codigoMunicipio INTEGER (1..99999), fuso INTEGER (-720..720),
//                                     desligaColetaBiometria BOOLEAN, horarioVerao HorarioVerao OPTIONAL }
// md::CComplementoMunicipio (32 bytes): +0 TMunicipioID codigo, +4 int16 fuso (minutes),
//   +8 std::optional<CHorarioVerao> (20 bytes, engaged flag +28). desligaColetaBiometria is neither read nor
//   written by this converter.
// RTTI: CConversorComplementoMunicipio
//         : IConversorASN<ModuloComplementosMunicipios::ComplementoMunicipio, md::CComplementoMunicipio>
// vtable @1569480: [0] 174 [1] 144 [2] 11383 DoConverte [3] 11382 DoDesconverte. sizeof 4.
class CConversorComplementoMunicipio
    : public IConversorASN<ModuloComplementosMunicipios::ComplementoMunicipio, md::CComplementoMunicipio>
{
protected:
    TEntidade DoConverte(const TDado& complemento) const override;      // wasm func 11383
    TDado DoDesconverte(const TEntidade& complemento) const override;   // wasm func 11382
};

} // namespace comum::asn
