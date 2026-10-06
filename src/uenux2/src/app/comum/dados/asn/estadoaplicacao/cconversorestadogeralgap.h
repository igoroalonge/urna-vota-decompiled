// uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversorestadogeralgap.h
// Reconstructed from vota_web_wasm.wasm (unit u03). Conventions: see asn/rdv/cconversorvoto.h.
#pragma once

#include "comum/asn/iconversorasn.h"
#include "comum/dados/md/estadoaplicacao/cestadogeralgap.h"
#include "ModuloEstadoGeralGap.h"

namespace comum::asn {

// RTTI: CConversorEstadoGeralGap
//         : IConversorASN<ModuloEstadoGeralGap::EstadoGeralGap, md::estadoaplicacao::CEstadoGeralGap>
// vtable @1568496: [0] 174 [1] 144 [2] 11394 DoConverte [3] 11395 DoDesconverte
// Used through comum::IServicoEstado<CEstadoGeralGap, CConversorEstadoGeralGap> for trab1|trab2/gap.bin.
//
// md::estadoaplicacao::CEstadoGeralGap (44 bytes), from its constructor (func 5626):
//   +0  std::vector<CDadoCorrespondencia> correspondencias (96-byte elements)
//   +12 EUrnaAplicativo appId             +16 EUrnaAplicativo appAnteriorId
//   +20 bool executadoRED  +21 bool identificadoATUE  +22 bool audio  +23 bool data2T
//   +24 CNumViasImpressasRelatorios numVias1T (4 x uebyte)   +28 numVias2T
//   +32 std::optional<api::CDate> dataSegundoTurno (CDate = 4 x short: day, month, year, weekday; flag +40)
class CConversorEstadoGeralGap
    : public IConversorASN<ModuloEstadoGeralGap::EstadoGeralGap, md::estadoaplicacao::CEstadoGeralGap>
{
public:
    EUrnaAplicativo DesconverteUrnaAplicativo(const ModuloEstadoGeralGap::UrnaAplicativo& app) const;   // wasm func 5690
    ModuloEstadoGeralGap::UrnaAplicativo ConverteUrnaAplicativo(const EUrnaAplicativo app) const;       // wasm func 5689

protected:
    TEntidade DoConverte(const TDado& estado) const override;       // wasm func 11394
    TDado DoDesconverte(const TEntidade& estado) const override;    // wasm func 11395
};

} // namespace comum::asn
