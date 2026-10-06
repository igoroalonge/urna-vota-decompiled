// uenux2/src/app/comum/dados/asn/rdv/cconversorvotoscargo.h
// Reconstructed from vota_web_wasm.wasm (unit u03). Conventions: see asn/rdv/cconversorvoto.h.
#pragma once

#include <utility>

#include "comum/asn/iconversorasn.h"
#include "comum/dados/asn/rdv/cconversorvoto.h"
#include "comum/dados/md/rdv/cvotoscargos.h"
#include "ModuloRegistroDigitalVoto.h"

namespace comum::asn {

// RTTI: comum::asn::CConversorVotosCargo
//         : IConversorASN<ModuloRegistroDigitalVoto::VotosCargo, std::pair<md::CVotosCargos::SCargoInfo, md::CVotos>>
// vtable @1571004: [0] 174 [1] 144 [2] 11352 DoConverte [3] 11351 DoDesconverte
//
// md::CVotosCargos::SCargoInfo (12 bytes, built from a md::CCargo by CConversorEleicoesVota):
//   +0 uebyte codigo (TCargoID)  +4 md::CCargo::ETipo tipo  +8 uebyte numeroDigitos  +9 uebyte qtdeEscolhas
// md::CVotos: std::vector<md::CVoto> (16-byte elements).
class CConversorVotosCargo
    : public IConversorASN<ModuloRegistroDigitalVoto::VotosCargo,
                           std::pair<md::CVotosCargos::SCargoInfo, md::CVotos>>
{
public:
    // wasm func 5683 (out-of-line copy of the inline constructor)
    explicit CConversorVotosCargo(const md::CVotosCargos::SCargoInfo& cargoInfo)
        : m_cargoInfo(cargoInfo)
    {
    }

protected:
    TEntidade DoConverte(const TDado& votosCargo) const override;      // wasm func 11352
    TDado DoDesconverte(const TEntidade& votosCargo) const override;   // wasm func 11351

private:
    md::CVotosCargos::SCargoInfo m_cargoInfo;   // +4  (12 bytes)
    CConversorVoto m_conversorVoto;             // +16 (vptr only)
};

} // namespace comum::asn
