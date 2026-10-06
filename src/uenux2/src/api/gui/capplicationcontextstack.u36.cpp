// uenux2/src/api/gui/capplicationcontextstack.cpp (+ .h)  --  FRAGMENT written by unit u36 (file owned by u15,
// see capplicationcontextstack.u15.h). Reconstructed from vota_web_wasm.wasm.
//
// Two implicitly-defined / defaulted members of api::CApplicationContext (52 bytes):
//   +0 std::string m_detalhe, +12 m_titulo, +24 m_mensagem, +36 std::vector<std::string> m_acoes,
//   +48 a 4-byte member (u15 calls it `bool m_generico`; the binary stores, copies and compares it as an i32:
//       vota_f5557 does `a[12]:int = f`, so it is an int/enum, or a bool the ABI widened).          ?
// They are used by ~CApplicationContextGuard (wasm 675, observed executing) and by vota_f5553 to remove
// the guard's context from the global stack (std::vector<CApplicationContext> @1839212): the stack is
// searched FROM THE TOP for an element equal to the guard's copy, and that element is erased by moving the
// ones above it down (move assignment) and destroying the last one.
#include "api/gui/capplicationcontextstack.u15.h"

namespace api {

// wasm func 5552                                                                       // name inferred
// Implicit move assignment: three strings moved (the source left empty), the vector<string> moved (the
// destination's old strings freed first), then the 4-byte member copied.
CApplicationContext& CApplicationContext::operator=(CApplicationContext&& outro) noexcept = default;

// wasm func 5556 - observed executing                                                  // name inferred
// Defaulted equality: sizes + memcmp of m_detalhe, m_titulo, m_mensagem, then m_acoes element by element,
// then the member at +48.
bool CApplicationContext::operator==(const CApplicationContext& outro) const = default;

}  // namespace api
