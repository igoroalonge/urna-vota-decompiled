// uenux2/src/app/comum/dados/ccargos.cpp  --  FRAGMENT written by unit u36 (file owned by u04).
// Reconstructed from vota_web_wasm.wasm.
//
// CCargos (ccargos.h): +0 m_indice, +4 std::vector<SCargoEleicao> m_todos (every cargo of the pleito),
// +16 std::vector<SCargoEleicao> m_cargos (the list being walked: filtered for the current voter while he
// votes, all cargos again at the end of the day).
//
// The second function of this unit that belongs here, OrdenaPorOrdemImpressao (wasm 3782: std::sort of
// m_cargos with the comparator of table slot 2358 = wasm 11558, then m_indice = 0), is already written in
// ccargos.cpp by unit u04.
#include "comum/dados/ccargos.h"

namespace comum {

// wasm func 3784                                                            // name inferred
// Undoes FiltraPorAbrangencia (ccargos.cpp:249): the walk list becomes the full list of cargos again.
// Only the vector is copied (libc++ vector::__assign_with_size, wasm 3783); the cursor is NOT rewound -
// every caller calls OrdenaPorOrdemImpressao() or First() next.
// Callers: vota::CGeraBU::StartState (12110) and vota::CGravaResultado::StartState (12098), i.e. before the
// BU, the RDV file and the BU QR codes walk the cargos. Other units' code calls it "Inicio()".
void CCargos::LimpaFiltro()
{
    m_cargos = m_todos;
}

}  // namespace comum
