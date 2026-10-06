// uenux2/src/app/comum/dados/asn/processoeleitoral/cconversorcodigocargoconsulta.cpp   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u35).
#include "comum/dados/asn/processoeleitoral/cconversorcodigocargoconsulta.h"

namespace comum::asn {

// wasm func 11592 - vtable slot 2. Only writer: the BU (CConversorEntidadeBU::DoConverte, func 10273: codigoCargo
// of every ResultadoVotacao). Not observed executing.
ModuloTiposEleitorais::CodigoCargoConsulta CConversorCodigoCargoConsulta::DoConverte(const uebyte& codigo) const
{
    ModuloTiposEleitorais::CodigoCargoConsulta entidade;                             // CHOICE(info @1141132)
    if (codigo <= 24)
        entidade.set_cargoConstitucional(ModuloTiposEleitorais::CargoConstitucional(codigo));   // alternative 0
    else
        entidade.set_numeroCargoConsultaLivre(codigo);                              // alternative 1 (INTEGER 25..99)
    return entidade;
}

// wasm func 11591 - vtable slot 3. Reached through func 5828 (IConversorASN<CodigoCargoConsulta>::Desconverte, a
// full copy with the CHOICE validity checks inlined) from CConversorCargo (11373) and CConversorCandidaturas (11445)
// while the election data is loaded. Neither was caught by the sampling profiler.
// Returns the low byte of the selected alternative's value, whichever alternative it is.
uebyte CConversorCodigoCargoConsulta::DoDesconverte(const ModuloTiposEleitorais::CodigoCargoConsulta& codigo) const
{
    return static_cast<uebyte>(codigo.getSelection().asInt());                     // value byte at +8 of the choice
}

} // namespace comum::asn
