// uenux2/src/app/comum/dados/asn/processoeleitoral/cconversortipoidentificadoreleitor.h
// Reconstructed from vota_web_wasm.wasm (unit u03). Conventions: see asn/rdv/cconversorvoto.h.
#pragma once

#include "comum/asn/iconversorasn.h"
#include "ecourna/app/dados/etipoidentificadoreleitor.h"
#include "ModuloTiposEleitorais.h"

namespace comum::asn {

// RTTI: CConversorTipoIdentificadorEleitor
//         : IConversorASN<ModuloTiposEleitorais::TipoIdentificadorEleitor, ecourna::app::dados::ETipoIdentificadorEleitor>
// vtable @1570676: [0] 174 [1] 144 [2] 11357 DoConverte [3] 11356 DoDesconverte
// ETipoIdentificadorEleitor uses the ASN.1 numbers: 1 inscrição eleitoral, 2 CPF, 3 número livre.
class CConversorTipoIdentificadorEleitor
    : public IConversorASN<ModuloTiposEleitorais::TipoIdentificadorEleitor, ecourna::app::dados::ETipoIdentificadorEleitor>
{
protected:
    TEntidade DoConverte(const TDado& tipo) const override;       // wasm func 11357
    TDado DoDesconverte(const TEntidade& tipo) const override;    // wasm func 11356
};

} // namespace comum::asn
