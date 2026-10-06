// uenux2/src/app/comum/dados/asn/municipiozona/cconversorcomplementosmunicipios.h
// Reconstructed from vota_web_wasm.wasm (unit u03). Conventions: see asn/rdv/cconversorvoto.h.
#pragma once

#include "comum/asn/iconversorasn.h"
#include "comum/dados/md/municipiozona/ccomplementosmunicipiosuf.h"
#include "ModuloComplementosMunicipios.h"

namespace comum::asn {

// RTTI: CConversorComplementosMunicipios
//         : IConversorASN<ModuloComplementosMunicipios::EntidadeComplementosMunicipios, md::CComplementosMunicipiosUF>
// vtable @1569620: [0] 174 [1] 144 [2] 11380 (base DoConverte: "não implementado") [3] 11381 DoDesconverte
class CConversorComplementosMunicipios
    : public IConversorASN<ModuloComplementosMunicipios::EntidadeComplementosMunicipios, md::CComplementosMunicipiosUF>
{
protected:
    TDado DoDesconverte(const TEntidade& entidade) const override;   // wasm func 11381
};

// RTTI: CConversorHorarioVerao : IConversorASN<ModuloComplementosMunicipios::HorarioVerao, md::CHorarioVerao>
// vtable @1569388: [0] 174 [1] 144 [2] 11385 DoConverte [3] 11384 DoDesconverte (unit u35)
// Declared here for convenience; its source file is municipiozona/cconversorhorarioverao.cpp (path inferred).
// md::CHorarioVerao (20 bytes): +0 api::CDate inicio, +8 api::CDate fim, +16 int diferenca (minutes).
class CConversorHorarioVerao
    : public IConversorASN<ModuloComplementosMunicipios::HorarioVerao, md::CHorarioVerao>
{
protected:
    TEntidade DoConverte(const TDado& horario) const override;       // wasm func 11385
    TDado DoDesconverte(const TEntidade& horario) const override;    // wasm func 11384 (not in this unit)
};

} // namespace comum::asn
