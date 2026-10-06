// Reconstructed from vota_web_wasm.wasm (unit u20, owner of this file).
// Original: uenux2/src/api/util/cwait.cpp (srcloc cwait.cpp:28).
// Other fragment: cwait.u09.cpp (CWait::Aguarda, func 5446: busy wait with usleep(200)).
//
// Layout (24 bytes): +0 size_t m_microssegundos, +8 timeval m_limite (int64 tv_sec, int32 tv_usec at +16).
#include "api/util/cwait.h"

#include <format>
#include <sys/time.h>

namespace api {

namespace {
constexpr std::size_t MAXIMO_MS = 4000000;       // ~66 minutes
}

// wasm func 5447. Callers: CMenuFiltrarCandidatosPorNumero::ProcessInput, CInformacaoZeresimaTardia's
// constructor (inlined in func 11931, CWait(1000)).                               srcloc cwait.cpp:28
CWait::CWait(const std::size_t milissegundos)
    : m_microssegundos(milissegundos * 1000)     // 32-bit size_t in wasm32; once added to the 32-bit
                                                 // tv_usec it exceeds INT_MAX for ms > ~2 147 483 (still
                                                 // below MAXIMO_MS): deadline in the past (doc §10)
    , m_limite{}
{
    if (milissegundos > MAXIMO_MS)
        throw CUeUtilError(EUeUtilError{7083},
                           std::format("Quantidade excessiva de milissegundos {}", milissegundos));
    ::gettimeofday(&m_limite, nullptr);
    m_limite.tv_usec += m_microssegundos;
    if (m_limite.tv_usec > 1000000) {            // note: == 1000000 is left unnormalised (harmless)
        m_limite.tv_sec += m_limite.tv_usec / 1000000;
        m_limite.tv_usec %= 1000000;
    }
}

} // namespace api
