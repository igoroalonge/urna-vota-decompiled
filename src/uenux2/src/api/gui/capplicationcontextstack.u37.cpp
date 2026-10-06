// uenux2/src/api/gui/capplicationcontextstack.cpp (attested by srcloc :45) -- FRAGMENT written by unit u37.
// Declarations: capplicationcontextstack.u15.h (unit u15). The stack is the global
// std::vector<CApplicationContext> @1839212 (begin) / @1839216 (end) / @1839220 (capacity); element 52 bytes.
#include <algorithm>
#include <iterator>

#include "api/gui/capplicationcontextstack.u15.h"

namespace api {

// wasm func 1841 (tools: vota_f1841 / "rhvoice_f1841" in the runtime tables) - observed executing
// (CEleitorVotando::IniciaCiclo pushes its context for every voter).
// The implicit copy constructor: three std::string copies (+0, +12, +24), the std::vector<std::string>
// (+36; vector::__throw_length_error for > 357913941 elements) and the field at +48. u15 declares that field
// as `bool m_generico`, but the binary copies it (here), stores it (5557) and compares it (operator==, 5556)
// as a 32-bit value, so it is more likely an int or an enum.                                            ?
// Callers: comum::SalvaEstado (491) and comum::GravaEstadoGeral (3333) (copy of Top()), the guard
// constructor (676), CApplicationContextStack::Top (1695), CEleitorVotando::IniciaCiclo (7377),
// CPedeDigital::StartState (10468).
CApplicationContext::CApplicationContext(const CApplicationContext&) = default;

// wasm func 5553 (tools: vota_f5553) - observed executing (end of every voter's cycle).
// name as used by unit u27 (cpededigital.cpp)                                          name inferred
// Removes the most recently pushed context equal to `contexto` (operator==, wasm 5556: the three strings,
// the action list and the flag). Nothing happens when it is not on the stack.
// Callers: vota::CEleitorVotando::FinishState (7370) and vota::CPedeDigital::FinishState (10467), which
// pushed their context with Push(). ~CApplicationContextGuard (675) contains the same code, but only
// when std::uncaught_exceptions() == 0: a context whose guarded block is being unwound by an exception
// stays on top so that the fatal-error screen can show it.
void CApplicationContextStack::Remove(const CApplicationContext& contexto)
{
    const auto it = std::find(m_pilha.rbegin(), m_pilha.rend(), contexto);
    if (it != m_pilha.rend())
        m_pilha.erase(std::next(it).base());          // move-assign the tail down (5552), destroy last (1468)
}

} // namespace api
