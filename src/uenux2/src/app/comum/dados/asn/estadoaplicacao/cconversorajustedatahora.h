// uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversorajustedatahora.h
// Reconstructed from vota_web_wasm.wasm (unit u03). Conventions: see asn/rdv/cconversorvoto.h.
#pragma once

#include "comum/asn/iconversorasn.h"
#include "comum/dados/md/estadoaplicacao/cajustedatahora.h"
#include "ModuloEstadoGeralUrna.h"

namespace comum::asn {

// RTTI: CConversorAjusteDataHora
//         : IConversorASN<ModuloEstadoGeralUrna::AjusteDataHora, md::estadoaplicacao::CAjusteDataHora>
// vtable @1567308: [0] 174 [1] 144 [2] 11405 DoConverte [3] 11406 DoDesconverte
// md::estadoaplicacao::CAjusteDataHora (8 bytes): +0 ETipoDeltaT tipo (0..2), +4 int valor.
class CConversorAjusteDataHora
    : public IConversorASN<ModuloEstadoGeralUrna::AjusteDataHora, md::estadoaplicacao::CAjusteDataHora>
{
public:
    ModuloEstadoGeralUrna::TipoAjusteDataHora
    ConverteTipoAjusteDataHora(const md::estadoaplicacao::CAjusteDataHora::ETipoDeltaT& tipo) const;     // inlined
    md::estadoaplicacao::CAjusteDataHora::ETipoDeltaT
    DesconverteTipoAjusteDataHora(const ModuloEstadoGeralUrna::TipoAjusteDataHora& tipo) const;          // inlined

protected:
    TEntidade DoConverte(const TDado& ajuste) const override;        // wasm func 11405
    TDado DoDesconverte(const TEntidade& ajuste) const override;     // wasm func 11406
};

} // namespace comum::asn
