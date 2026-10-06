// uenux2/src/app/comum/dados/asn/processoeleitoral/cconversorcargo.h
// Reconstructed from vota_web_wasm.wasm (unit u03). Conventions: see asn/rdv/cconversorvoto.h.
#pragma once

#include "comum/asn/iconversorasn.h"
#include "comum/dados/md/processoeleitoral/ccargo.h"
#include "ModuloEleicao.h"

namespace comum::asn {

// RTTI: CConversorCargo : IConversorASN<ModuloEleicao::CargoPergunta, md::CCargo>
// vtable @1569972: [0] 174 [1] 144 [2] 11372 (base DoConverte, "não implementado") [3] 11373 DoDesconverte
// Built by CConversorEleicaoPE::DoDesconverte (func 11371) with the eleição's abrangência, which the converter
// copies into every md::CCargo (CCargo +8).
//
// md::CCargo (140 bytes), from the inlined constructor:
//   +0 uebyte codigo  +4 ETipo tipo (0 majoritário, 1 proporcional, 2 consulta)  +8 ETipoAbrangencia abrangencia
//   +12 numeroDigitos +13 qtdeEscolhas +14 podeRepetir +15 ordemAquisicao +16 ordemImpressao +17 ordemApuracao
//   +18 paginaImpressaoVoto (all uebyte)
//   +20  std::optional<md::CDetalheCandidato> (64 bytes, engaged flag at +84)
//   +88  std::optional<md::CDetalheConsulta>  (48 bytes, engaged flag at +136)
class CConversorCargo : public IConversorASN<ModuloEleicao::CargoPergunta, md::CCargo>
{
public:
    explicit CConversorCargo(md::ETipoAbrangencia abrangencia) : m_abrangencia(abrangencia) {}

    static md::CCargo::ETipo DesconverteTipoCargo(ModuloTiposEleitorais::TipoCargoConsulta::NamedNumber tipo);   // inlined

protected:
    TDado DoDesconverte(const TEntidade& cargo) const override;   // wasm func 11373

private:
    md::ETipoAbrangencia m_abrangencia;   // +4
};

} // namespace comum::asn
