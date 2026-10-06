// uenux2/src/app/comum/dados/asn/processoeleitoral/cconversorprocessoeleitoral.h
// Reconstructed from vota_web_wasm.wasm (unit u03). Conventions: see asn/rdv/cconversorvoto.h.
#pragma once

#include "comum/asn/iconversorasn.h"
#include "comum/dados/md/processoeleitoral/cprocessoeleitoraldto.h"
#include "ModuloProcessoEleitoral.h"

namespace comum::asn {

// RTTI: CConversorProcessoEleitoral
//         : IConversorASN<ModuloProcessoEleitoral::EntidadeProcessoEleitoral, md::CProcessoEleitoralDTO>
// vtable @1570416: [0] 174 [1] 144 [2] 11362 (base DoConverte, "não implementado") [3] 11363 DoDesconverte
//
// md::CProcessoEleitoralDTO (92 bytes), from the inlined constructor:
//   +0 TPEID id  +4 std::string nome  +16 bool utilizaBiometria  +20 md::EOrigemConfiguracao origem
//   +24 md::CPleitoDTO pleito1 {TPleitoID id; std::string nome; api::CDate data}  (24 bytes)
//   +48 std::optional<md::CPleitoDTO> pleito2 (engaged flag +72)
//   +76 ETipoIdentificadorEleitor tipoIdentificadorPrincipal
//   +80 std::vector<ETipoIdentificadorEleitor> tiposIdentificadoresPermitidos
class CConversorProcessoEleitoral
    : public IConversorASN<ModuloProcessoEleitoral::EntidadeProcessoEleitoral, md::CProcessoEleitoralDTO>
{
protected:
    TDado DoDesconverte(const TEntidade& processo) const override;   // wasm func 11363
};

} // namespace comum::asn
