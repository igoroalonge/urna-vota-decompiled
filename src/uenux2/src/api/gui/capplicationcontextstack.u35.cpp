// FRAGMENT of uenux2/src/api/gui/capplicationcontextstack.cpp (attested by srcloc :45; declarations in
// capplicationcontextstack.u15.h, unit u15; CApplicationContext's operator== / move assignment in the u36 fragment).
// Reconstructed from vota_web_wasm.wasm (unit u35: the guard's destructor, which the tools attributed to app:comum).
#include <algorithm>
#include <exception>

#include "api/gui/capplicationcontextstack.u15.h"

namespace api {

// wasm func 675 - observed executing (every guarded step: SalvaEstado 491, AssinarUE 1277, CGravaResultado,
// CGeraBU, CGeraRelatorios, CCopiaResultadoParaMR, CPedeAnoNascimento...).
//
// The guard removes ITS context from the global stack only when the scope ends normally. When the scope is left by
// an exception (std::uncaught_exceptions() != 0; env.__cxa_uncaught_exceptions in this build) the context is left on
// the stack ON PURPOSE: the monitor thread (vota::CThreadMonitor) takes the top of the stack to build the fatal-error
// screen ("<título> (<código>)", mensagem, recommended actions, QR code).
// The search starts from the top (the most recent equal context) and the element is erased with the usual
// vector erase (move-assign the ones above it down, destroy the last).
CApplicationContextGuard::~CApplicationContextGuard()
{
    if (std::uncaught_exceptions() == 0) {
        std::vector<CApplicationContext>& pilha = m_pilha.m_pilha;                     // @1839212
        const auto it = std::find(pilha.rbegin(), pilha.rend(), m_contexto);          // operator== = func 5556
        if (it != pilha.rend())
            pilha.erase(std::next(it).base());                                        // move = func 5552, ~ = 1468
    }
}                                                                                     // ~m_contexto (func 1468)

} // namespace api
