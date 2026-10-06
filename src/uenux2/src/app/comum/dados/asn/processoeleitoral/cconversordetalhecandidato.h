// uenux2/src/app/comum/dados/asn/processoeleitoral/cconversordetalhecandidato.h
// Reconstructed from vota_web_wasm.wasm (unit u03). Conventions: see asn/rdv/cconversorvoto.h.
#pragma once

#include "comum/asn/iconversorasn.h"
#include "comum/dados/md/processoeleitoral/cdetalhecandidato.h"
#include "ModuloEleicao.h"

namespace comum::asn {

// RTTI: CConversorDetalheCandidato : IConversorASN<ModuloEleicao::DetalheCargo, md::CDetalheCandidato>
// vtable @1569880: [0] 174 [1] 144 [2] 11374 (base DoConverte, "não implementado") [3] 11375 DoDesconverte
//
// md::CDetalheCandidato (64 bytes): +0 bool temFoto; +4 md::CNomesCargo nomes (4 std::string: neutro,
//   masculino, feminino, abreviado); +52 std::vector<md::CSuplencia> suplencias.
// md::CSuplencia (52 bytes): +0 two uebytes (codigo, ordem ?), +4 CNomesCargo nomes (4 strings); temFoto is
//   not kept (the implicit copy constructor, func 2341, copies exactly these fields).
class CConversorDetalheCandidato : public IConversorASN<ModuloEleicao::DetalheCargo, md::CDetalheCandidato>
{
protected:
    TDado DoDesconverte(const TEntidade& detalhe) const override;   // wasm func 11375
};

} // namespace comum::asn
