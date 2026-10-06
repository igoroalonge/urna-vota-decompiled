// FRAGMENT of uenux2/src/app/comum/dados/md/estadoaplicacao/cnumviasimpressasrelatorios.cpp (path inferred from
// the srcloc IConversorASN<ModuloEstadoGeralDefs::NumViasImpressasRelatorios,
// comum::md::estadoaplicacao::CNumViasImpressasRelatorios>) reconstructed by unit u29 from vota_web_wasm.wasm.
//
// Number of copies ("vias") already printed of four reports, kept in vota.bin so that a restarted urna knows
// what it printed: ASN.1 {numViasEstadoUrna, numViasEleitores, numViasVersoesDados, numViasPU} (0..999 in the
// schema, one byte each in memory: 4 bytes).
#include <cstdint>

namespace comum::md::estadoaplicacao {

// wasm func 5629. Callers: CConversorNumViasImpressasRelatorios::DoDesconverte (func 11387) and func 5324.
//                                                                                 parameter names from the ASN.1
CNumViasImpressasRelatorios::CNumViasImpressasRelatorios(std::uint8_t estadoUrna, std::uint8_t eleitores,
                                                         std::uint8_t versoesDados, std::uint8_t pu)
    : m_estadoUrna(estadoUrna), m_eleitores(eleitores), m_versoesDados(versoesDados), m_pu(pu)
{
}

// wasm func 5324 (a copy made through this constructor) is a helper of the web fixture: see
// src/uenux2/mock/app/comum/cappinfobuilder.u29.cpp.

}  // namespace comum::md::estadoaplicacao
