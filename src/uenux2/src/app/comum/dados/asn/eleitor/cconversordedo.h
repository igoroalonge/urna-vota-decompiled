// uenux2/src/app/comum/dados/asn/eleitor/cconversordedo.h
// Reconstructed from vota_web_wasm.wasm (unit u03). Conventions: see asn/rdv/cconversorvoto.h.
#pragma once

#include <vector>

#include "api/biometria/sxyt.h"
#include "comum/asn/iconversorasn.h"
#include "comum/dados/md/eleitor/cdedo.h"
#include "ModuloEleitores.h"

namespace comum::asn {

// RTTI: CConversorDedo : IConversorASN<ModuloEleitores::Dedo, md::CDedo>
// vtable @1566488: [0] 174 [1] 144 [2] 11415 (base DoConverte, "não implementado") [3] 11416 DoDesconverte
//
// md::CDedo (20 bytes): +0 TipoDedo tipo (1..10) +4 uedword qtdMinucias +8 std::vector<api::SXYT> minucias
// api::SXYT (16 bytes): +0 x, +4 y, +8 t (angle, 9 bits in the file), +12 = 0 (unused by this decoder)
class CConversorDedo : public IConversorASN<ModuloEleitores::Dedo, md::CDedo>
{
public:
    static md::CDedo::TipoDedo DesconverteTipo(ModuloTiposEleitorais::TipoDedo::NamedNumber tipo);   // inlined

protected:
    TDado DoDesconverte(const TEntidade& dedo) const override;   // wasm func 11416
};

} // namespace comum::asn
