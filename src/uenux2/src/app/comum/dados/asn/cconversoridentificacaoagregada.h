// uenux2/src/app/comum/dados/asn/cconversoridentificacaoagregada.h   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u21).
#pragma once

#include "comum/asn/iconversorasn.h"
#include "comum/dados/md/csecaoeleitoral.h"     // md::CIdentificacaoAgregada
#include "ModuloLocal.h"

namespace comum::asn {

// IdentificacaoAgregada ::= SEQUENCE { numero INTEGER (1..9999), tipoLocalOrigem TipoLocalVotacao }
// RTTI: CConversorIdentificacaoAgregada : IConversorASN<ModuloLocal::IdentificacaoAgregada, md::CIdentificacaoAgregada>
// vtable @1561564: [0] 174 [1] 144 [2] 11456 DoConverte (other unit) [3] 11455 DoDesconverte. sizeof 4.
class CConversorIdentificacaoAgregada
    : public IConversorASN<ModuloLocal::IdentificacaoAgregada, md::CIdentificacaoAgregada>
{
protected:
    TEntidade DoConverte(const TDado& agregada) const override;      // wasm func 11456 (other unit)
    TDado DoDesconverte(const TEntidade& agregada) const override;   // wasm func 11455
};

} // namespace comum::asn
