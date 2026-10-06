// uenux2/src/app/comum/dados/asn/processoeleitoral/cconversororigemconfiguracao.h
// Reconstructed from vota_web_wasm.wasm (unit u03). Conventions: see asn/rdv/cconversorvoto.h.
#pragma once

#include "comum/asn/iconversorasn.h"
#include "comum/dados/md/processoeleitoral/eorigemconfiguracao.h"
#include "ModuloProcessoEleitoral.h"

namespace comum::asn {

// RTTI: CConversorOrigemConfiguracao : IConversorASN<ModuloProcessoEleitoral::OrigemConfiguracao, md::EOrigemConfiguracao>
// vtable @1570248: [0] 174 [1] 144 [2] 11367 DoConverte [3] 11366 DoDesconverte
// md::EOrigemConfiguracao { Oficial = 1, Comunitaria = 2 }   (enumerator names inferred from the ASN.1 names)
class CConversorOrigemConfiguracao
    : public IConversorASN<ModuloProcessoEleitoral::OrigemConfiguracao, md::EOrigemConfiguracao>
{
protected:
    TEntidade DoConverte(const TDado& origem) const override;       // wasm func 11367
    TDado DoDesconverte(const TEntidade& origem) const override;    // wasm func 11366
};

} // namespace comum::asn
