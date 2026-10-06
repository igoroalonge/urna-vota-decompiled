// FRAGMENT reconstructed by unit u09 from vota_web_wasm.wasm.
// Original file: uenux2/src/api/util/cwait.cpp (attested for CWait::CWait(const size_t) by srcloc :28).
// api::CWait = a deadline set at construction (CWait(ms): +0 = ms*1000 µs, +8 timeval, ...; ms > 4000000
// throws EUeUtilError 7083) and a busy wait until it.

#include "api/util/cwait.h"

#include <sys/time.h>
#include <unistd.h>

namespace api {

extern bool g_esperaHabilitada;   // byte @1584624 (= 1, never written in this build)  name inferred

// wasm func 5446 (attributed by the tools to cinformacaozeresimatardia.cpp; also called by
// CMenuFiltrarCandidatosPorNumero::ProcessInput). Uses usleep(200), NOT emscripten_sleep, so it does
// not abort in the web build, but it spins the (only) thread until the deadline.   name inferred
void CWait::Aguarda() const
{
    if (!g_esperaHabilitada)
        return;
    for (;;) {
        timeval agora{};
        ::gettimeofday(&agora, nullptr);
        if (agora.tv_sec > m_limite.tv_sec ||
            (agora.tv_sec == m_limite.tv_sec && agora.tv_usec >= m_limite.tv_usec))
            return;
        ::usleep(200);
        if (!g_esperaHabilitada)
            return;
    }
}

}  // namespace api
