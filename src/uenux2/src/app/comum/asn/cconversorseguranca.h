// uenux2/src/app/comum/asn/cconversorseguranca.h   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u35; same declaration as in u24-foreign-fragments.cpp).
#pragma once

#include "comum/asn/iconversorasn.h"
#include "comum/md/cseguranca.h"
#include "ModuloTiposEleitorais.h"

namespace comum::asn {

// vtable @1566572: [0] 174 [1] 144 [2] 11413 DoConverte [3] 11412 DoDesconverte. sizeof 4.
class CConversorSeguranca : public IConversorASN<ModuloTiposEleitorais::Seguranca, md::CSeguranca>
{
protected:
    TEntidade DoConverte(const TDado& seguranca) const override;       // wasm func 11413 (this unit)
    TDado DoDesconverte(const TEntidade& seguranca) const override;    // wasm func 11412 (unit u24)
};

} // namespace comum::asn
