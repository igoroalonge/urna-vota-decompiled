// uenux2/src/app/comum/dados/asn/eleitor/cconversorimpedido.h
// Reconstructed from vota_web_wasm.wasm (unit u03). Conventions: see asn/rdv/cconversorvoto.h.
#pragma once

#include <vector>

#include "comum/asn/iconversorasn.h"
#include "comum/dados/md/eleitor/cimpedido.h"
#include "ModuloImpedidos.h"

namespace comum::asn {

// RTTI: CConversorImpedido : IConversorASN<ModuloImpedidos::EntidadeImpedidos, std::vector<md::CImpedido>>
// vtable @1567104: [0] 174 [1] 144 [2] 11409 (base DoConverte, "não implementado") [3] 11410 DoDesconverte
//
// md::CImpedido (32 bytes), from its two constructors (funcs 5661/5662):
//   +0 ushort secao  +4 md::CEleitorIdentidade identidade {std::string numero; int tipo}
//   +20 md::ETipoImpedimento impedimentoP1 (0..14)
//   +24 md::ETipoImpedimento impedimentoP2 (15 when absent)  +28 bool temImpedimentoP2
class CConversorImpedido : public IConversorASN<ModuloImpedidos::EntidadeImpedidos, std::vector<md::CImpedido>>
{
public:
    md::CEleitorIdentidade MontaEleitorIdentidade(const ModuloTiposEleitorais::IdentificadorEleitor& id) const;   // inlined
    md::ETipoImpedimento DesconverteTipoImpedimento(ModuloImpedidos::TipoImpedimento tipo) const;               // wasm func 5699

protected:
    TDado DoDesconverte(const TEntidade& entidade) const override;   // wasm func 11410
};

} // namespace comum::asn
