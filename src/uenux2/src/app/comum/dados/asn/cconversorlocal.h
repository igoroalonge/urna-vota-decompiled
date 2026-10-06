// uenux2/src/app/comum/dados/asn/cconversorlocal.h
// Reconstructed from vota_web_wasm.wasm (unit u03). Conventions: see asn/rdv/cconversorvoto.h.
#pragma once

#include "comum/asn/iconversorasn.h"
#include "comum/dados/md/clocal.h"
#include "ModuloLocal.h"

namespace comum::asn {

// RTTI: CConversorLocal : IConversorASN<ModuloLocal::Local, comum::md::CLocal>
// vtable @1561996: [0] 174 [1] 144 [2] 11451 DoConverte [3] 11452 DoDesconverte
//
// md::CLocal (140 bytes), from DoDesconverte:
//   +0 TPEID idPE  +4 std::string pais  +16 std::string uf (sigla)  +28 std::string nomeUf
//   +40 md::CInfoMunicipio municipio (44 bytes: +0 codigo, +4 nome, +16 short fuso, +18 bool comBiometria,
//       +20 std::optional<md::CHorarioVerao> (flag +40))
//   +84 std::optional<md::CSecaoEleitoral> secao (flag +120)
//   +124 std::optional<md::CContingencia> contingencia {?, +4 municipio, +8 ushort zona} (flag +136)
class CConversorLocal : public IConversorASN<ModuloLocal::Local, md::CLocal>
{
protected:
    TEntidade DoConverte(const TDado& local) const override;       // wasm func 11451
    TDado DoDesconverte(const TEntidade& local) const override;    // wasm func 11452
};

} // namespace comum::asn
