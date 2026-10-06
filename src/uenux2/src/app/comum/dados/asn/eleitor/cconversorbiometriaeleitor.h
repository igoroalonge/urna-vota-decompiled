// uenux2/src/app/comum/dados/asn/eleitor/cconversorbiometriaeleitor.h
// Reconstructed from vota_web_wasm.wasm (unit u03). Conventions: see asn/rdv/cconversorvoto.h.
#pragma once

#include "comum/asn/iconversorasn.h"
#include "comum/dados/md/eleitor/cbiometriaeleitor.h"
#include "ModuloEleitores.h"

namespace comum::asn {

// RTTI: CConversorBiometriaEleitor : IConversorASN<ModuloEleitores::BiometriaEleitor, md::CBiometriaEleitor>
// vtable @1566000: [0] 174 [1] 144 [2] 11421 (base DoConverte, "não implementado") [3] 11422 DoDesconverte
//
// md::CBiometriaEleitor (40 bytes) as built here:
//   +0  std::optional<ecourna::app::dados::CFoto> foto (CFoto = {int formato; std::vector<uebyte> imagem}; flag +16)
//   +20 std::optional<std::map<...>> dedos-derived data (map header at +20, flag +32; filled by CriaComum)
//   +36 int (0)
class CConversorBiometriaEleitor : public IConversorASN<ModuloEleitores::BiometriaEleitor, md::CBiometriaEleitor>
{
protected:
    TDado DoDesconverte(const TEntidade& biometria) const override;   // wasm func 11422
};

} // namespace comum::asn
