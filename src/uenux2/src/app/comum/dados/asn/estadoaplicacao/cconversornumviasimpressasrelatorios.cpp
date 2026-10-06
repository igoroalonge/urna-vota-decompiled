// uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversornumviasimpressasrelatorios.cpp   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u35).
#include "comum/dados/asn/estadoaplicacao/cconversornumviasimpressasrelatorios.h"

#include <cstdint>

namespace comum::asn {

// wasm func 11386 - vtable slot 2. Not observed executing by the sampler (vota.bin is written at every start-up and
// vote synchronisation, through the thunk func 3726 -> merged Converte 1563; a 4-store body is easily missed).
ModuloEstadoGeralDefs::NumViasImpressasRelatorios CConversorNumViasImpressasRelatorios::DoConverte(
    const md::estadoaplicacao::CNumViasImpressasRelatorios& vias) const
{
    ModuloEstadoGeralDefs::NumViasImpressasRelatorios entidade;   // SEQUENCE(info @1139500)
    entidade.set_numViasEstadoUrna(vias.GetNumViasEstadoUrna());  // byte +0
    entidade.set_numViasEleitores(vias.GetNumViasEleitores());    // byte +1
    entidade.set_numViasVersoesDados(vias.GetNumViasVersoesDados()); // byte +2
    entidade.set_numViasPU(vias.GetNumViasPU());                  // byte +3
    return entidade;
}

// wasm func 11387 - vtable slot 3.
// NOTE: the ASN.1 range is 0..999 but only the LOW BYTE of each INTEGER is read (i32.load8_u of the value):
// a counter of 256..999 in the file becomes value % 256 in memory (the md class stores uint8_t).
md::estadoaplicacao::CNumViasImpressasRelatorios CConversorNumViasImpressasRelatorios::DoDesconverte(
    const ModuloEstadoGeralDefs::NumViasImpressasRelatorios& vias) const
{
    return md::estadoaplicacao::CNumViasImpressasRelatorios(
        static_cast<std::uint8_t>(vias.get_numViasEstadoUrna()),
        static_cast<std::uint8_t>(vias.get_numViasEleitores()),
        static_cast<std::uint8_t>(vias.get_numViasVersoesDados()),
        static_cast<std::uint8_t>(vias.get_numViasPU()));                               // func 5629
}

} // namespace comum::asn
