// uenux2/src/api/util/ctickmanager.cpp (attested by srclocs :31, :60, :69, :128) -- FRAGMENT written by
// unit u37: the polling side of the tick table ("the polling side is in another unit", u20).
// Layout (from ctickmanager.cpp, unit u20): std::map<uebyte, STick> m_ticks; tree node = 56 bytes, key at
// +16, STick at +24 {+0 uint64 intervalo (ms), +8 timeval proximo {int64 tv_sec, int32 tv_usec},
// +24 parado (read as a 32-bit value: == 1 means stopped)}.
#include <cmath>
#include <cstdint>
#include <sys/time.h>
#include <vector>

#include "api/util/ctickmanager.h"

namespace api {

// wasm func 5450 (tools: api::CTickManager::GetTicksExpirados) - observed executing (every votaTick:
// CThreadEleitor::Processar 4349 and CExecucaoVotaCooperativa slot 7; also CThreadOperador::Run 10204
// on the urna).
// name as used by units u07/u10 (name inferred; the class otherwise uses English verbs: AddTick, StopTick...)
//
// Returns the ids of the running ticks whose expiry time has passed, in id order, and re-arms each of them.
// Re-arming skips every period that was missed: the new expiry is the first multiple of the interval after
// "now", so a thread that was busy for several periods sees the tick ONCE (no burst of ticks).
// The clock is gettimeofday() (wall clock, not monotonic).
std::vector<uebyte> CTickManager::GetTicksExpirados()
{
    std::vector<uebyte> expirados;
    timeval agora;
    ::gettimeofday(&agora, nullptr);                                                  // api_f1294

    for (auto& [id, tick] : m_ticks) {
        if (tick.parado)
            continue;
        const bool venceu = agora.tv_sec > tick.proximo.tv_sec ||
                            (agora.tv_sec == tick.proximo.tv_sec && agora.tv_usec >= tick.proximo.tv_usec);
        if (!venceu)
            continue;
        expirados.push_back(id);

        // --- re-arm: proximo += k * intervalo, k = whole periods elapsed + 1 (arithmetic as compiled) ---
        const double periodo = static_cast<double>(tick.intervalo) / 1000.0;         // seconds
        int difUsec = agora.tv_usec - tick.proximo.tv_usec;
        const std::int64_t difSeg = (agora.tv_sec - tick.proximo.tv_sec) - (difUsec < 0 ? 1 : 0);
        const double periodos = static_cast<double>(difSeg) / periodo;
        const double inteiros = static_cast<int>(periodos);                          // truncation
        const double avanco = periodo * inteiros;                                    // seconds, whole periods
        std::int64_t avancoSeg = static_cast<int>(avanco);
        if (difUsec < 0)
            difUsec += 1000000;
        // NOTE: the period in microseconds is computed in 32 bits: intervals above 2147 s overflow.
        const int periodoUsec = static_cast<int>(tick.intervalo) * 1000;
        const int restoUsec = static_cast<int>(std::round(periodo * (periodos - inteiros) * 1e6));
        int avancoUsec = ((restoUsec + difUsec) / periodoUsec + 1) * periodoUsec +
                         static_cast<int>(std::round((avanco - static_cast<double>(avancoSeg)) * 1e6));
        while (avancoUsec >= 1000000) {                                               // closed form in the binary
            avancoUsec -= 1000000;
            ++avancoSeg;
        }
        // timeradd(proximo, {avancoSeg, avancoUsec})
        const int usec = tick.proximo.tv_usec + avancoUsec;
        tick.proximo.tv_usec = usec % 1000000;
        tick.proximo.tv_sec += avancoSeg + usec / 1000000;
    }
    return expirados;
}

} // namespace api
