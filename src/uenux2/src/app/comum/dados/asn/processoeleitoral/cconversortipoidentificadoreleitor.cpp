// uenux2/src/app/comum/dados/asn/processoeleitoral/cconversortipoidentificadoreleitor.cpp
// Reconstructed from vota_web_wasm.wasm (unit u03).
//
//   TipoIdentificadorEleitor ::= ENUMERATED { numeroInscricaoEleitoral(1), numeroCPF(2), numeroLivre(3) }
// Which document identifies a voter at the mesário's terminal (título de eleitor, CPF, or a free number).
#include "comum/dados/asn/processoeleitoral/cconversortipoidentificadoreleitor.h"

namespace comum::asn {

using ecourna::app::dados::ETipoIdentificadorEleitor;

// wasm func 11356 (vtable slot 3; srcloc line 51) — thunk into the merged body comum_f6030 (code 8184, count 3)
ETipoIdentificadorEleitor CConversorTipoIdentificadorEleitor::DoDesconverte(const TEntidade& tipo) const
{
    const int valor = tipo.asInt();
    if (valor < 1 || valor > 3) {
        throw CDadosError(8184, "Tipo de identificador de eleitor inválido");   // line 51
    }
    return static_cast<ETipoIdentificadorEleitor>(valor);
}

// wasm func 11357 (vtable slot 2; srcloc line 32)
ModuloTiposEleitorais::TipoIdentificadorEleitor CConversorTipoIdentificadorEleitor::DoConverte(const TDado& tipo) const
{
    const int valor = static_cast<int>(tipo);
    if (valor < 1 || valor > 3) {
        throw CDadosError(8183, "Tipo de identificador de eleitor inválido");   // line 32
    }
    return ModuloTiposEleitorais::TipoIdentificadorEleitor(valor);
}

} // namespace comum::asn
