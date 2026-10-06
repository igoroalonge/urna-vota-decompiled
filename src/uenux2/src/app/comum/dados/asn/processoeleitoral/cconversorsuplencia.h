// uenux2/src/app/comum/dados/asn/processoeleitoral/cconversorsuplencia.h   (path inferred; included by u03's
// cconversordetalhecandidato.cpp)
// Reconstructed from vota_web_wasm.wasm (unit u21).
#pragma once

#include "comum/asn/iconversorasn.h"
#include "comum/dados/md/processoeleitoral/csuplencia.h"
#include "ModuloEleicao.h"

namespace comum::asn {

// Suplencias ::= SEQUENCE { codigo CodigoCargoConsulta, ordem INTEGER (1..9), nomes NomesCargo, temFoto BOOLEAN }
// ("suplência" = the substitute / vice attached to a cargo: vice-prefeito, 1º/2º suplente de senador ...)
// md::CSuplencia (52 bytes): +0 uebyte ordem, +1 bool temFoto, +4 md::CNomesCargo nomes (4 std::string:
//   neutro, masculino, feminino, abreviado). The cargo code is not kept.
// RTTI: CConversorSuplencia : IConversorASN<ModuloEleicao::Suplencias, md::CSuplencia>
// vtable @1570584: [0] 174 [1] 144 [2] 11358 (base DoConverte, throws 7655) [3] 11359 DoDesconverte. sizeof 4.
class CConversorSuplencia : public IConversorASN<ModuloEleicao::Suplencias, md::CSuplencia>
{
protected:
    TDado DoDesconverte(const TEntidade& suplencia) const override;   // wasm func 11359
};

} // namespace comum::asn
