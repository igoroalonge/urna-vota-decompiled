// uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversorlocalidadeeleitoral.cpp   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u35: DoConverte). DoDesconverte (func 11389) is in
// src/uenux2/src/app/comum/dados/u05-foreign-fragments.cpp (unit u05).
#include "comum/dados/asn/estadoaplicacao/cconversorlocalidadeeleitoral.h"

namespace comum::asn {

// wasm func 11388 - vtable slot 2. Not in the observed-function list of the recorded votes, although eg.bin is
// written at votaInit: it is normally reached inlined/devirtualised through IConversorASN::Converte (func 5696),
// which the sampling profiler may have missed.
// The município goes through a temporary ASN1::Constrained_INTEGER<1, 99999> and the zona through the
// (1..9999) INTEGER type before being stored: constructed and discarded, no range check is performed there
// (the final Converte() -> isStrictlyValid() of the enclosing entity does the check).
ModuloEstadoGeralUrna::DadoSecao CConversorLocalidadeEleitoral::DoConverte(
    const md::estadoaplicacao::CLocalidadeEleitoral& localidade) const
{
    ModuloEstadoGeralUrna::DadoSecao entidade;                    // SEQUENCE(info @1145680)
    entidade.set_municipio(localidade.GetMunicipio());           // +0
    entidade.set_zona(localidade.GetZona());                     // +4 (uint16)
    entidade.set_secao(localidade.GetSecao());                   // +6 (uint16)
    return entidade;
}

} // namespace comum::asn
