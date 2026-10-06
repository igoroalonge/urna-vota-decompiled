// FRAGMENT of uenux2/src/app/comum/relatorios/cpartecandidatos.cpp (attested by srclocs :75 / :174 of its sibling
// classes; owner u25). comum::CParteCandidatosProporcionais is known from RTTI only; its Imprime (func 11213) is in
// src/uenux2/src/app/comum/dados/u04-foreign-fragments.cpp (unit u04).
// Reconstructed from vota_web_wasm.wasm (unit u35: the two destructors).
//
// Layout (88 bytes), from the destructor:
//   +4  shared_ptr<IReportPart> m_cabecalho          +12 m_cabecalhoPartido      +20 m_colunas
//   +28 m_candidato                                   +36 m_rodapePartido         +44 m_rodape
//   +52 m_semCandidatos                               +64 std::function<bool()> m_filtroPartido (__f_ at +80)
// vtable @1576452: [0] 3867 [1] 11210 [2] 11213 Imprime.
#include "comum/relatorios/cpartecandidatos.h"

namespace comum {

// wasm func 3867 - vtable slot 0. Not observed executing (the zerésima is not printed in the recorded sessions).
// Destroys the std::function first (destroy() when the callable is in the small buffer at +64, destroy_deallocate()
// otherwise), then the seven shared_ptrs in reverse order. Also called by
// std::shared_ptr<CParteCandidatosProporcionais>::__on_zero_shared (11958).
CParteCandidatosProporcionais::~CParteCandidatosProporcionais() = default;

// wasm func 11210 - vtable slot 1 (deleting destructor): free(~CParteCandidatosProporcionais(this)) via func 3867.

} // namespace comum
