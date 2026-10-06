// uenux2/src/app/comum/dados/asn/processoeleitoral/cconversorsuplencia.cpp   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u21).
#include "comum/dados/asn/processoeleitoral/cconversorsuplencia.h"

#include "comum/dados/asn/processoeleitoral/cconversornomescargo.h"   // vtable @1570172

namespace comum::asn {

// wasm func 11359 (vtable slot 3). Observed executing (election configuration load at votaInit).
// The two scalar fields are read before the names are converted (thunk 5688 -> 1008 validates NomesCargo).
md::CSuplencia CConversorSuplencia::DoDesconverte(const ModuloEleicao::Suplencias& suplencia) const
{
    const bool temFoto = suplencia.get_temFoto();                          // field 3
    const auto ordem = static_cast<uebyte>(suplencia.get_ordem());         // field 1 (1..9)
    const md::CNomesCargo nomes = CConversorNomesCargo().Desconverte(suplencia.get_nomes());
    return md::CSuplencia{ordem, temFoto, nomes};                          // aggregate: +0 ordem, +1 temFoto, +4 nomes
}

} // namespace comum::asn
