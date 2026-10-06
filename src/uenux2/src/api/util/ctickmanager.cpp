// Reconstructed from vota_web_wasm.wasm (unit u20, owner of this file).
// Original: uenux2/src/api/util/ctickmanager.cpp (srclocs ctickmanager.cpp:31, :60, :69, :128).
//
// api::CTickManager = the periodic timers ("ticks") of a thread. Each thread object (vota::CThreadVota,
// CThreadEleitor / CThreadOperador) embeds one at +20; the state machine asks for ticks by id (a uebyte)
// and receives ProcessTick(id) when a running tick expires (the polling side is in another unit).
//
// Layout: std::map<uebyte, STick> m_ticks at +0 (tree node 56 bytes: key at +16, STick at +24).
#include "api/util/ctickmanager.h"

#include <cstdint>
#include <format>
#include <sys/time.h>

namespace api {

// (declared in ctickmanager.h)
// struct STick {                                   // 32 bytes                             name inferred
//     std::uint64_t intervalo;                     // +0  milliseconds
//     timeval proximo;                             // +8  next expiry (64-bit tv_sec, 32-bit tv_usec)
//     int parado;                                  // +24 stopped: a 4-byte field, not a bool - AddTick,
//                                                  //     StartTick and StopTick write it with i32.store
//                                                  //     (0 / 0 / 1) and the poller (u37) reads it as i32
//                                                  //     (type may be an enum / ueint32)                  // ?
// };

namespace {
// An int, not a size_t: the 7072 message formats it with arg type 3 (int); the compare with the
// size_t argument is unsigned (i32.le_u 29).
constexpr int TICK_MINIMO_MS = 30;

timeval AgoraMais(std::uint64_t ms)                                  // inlined twice   name inferred
{
    timeval agora;
    ::gettimeofday(&agora, nullptr);
    long usec = agora.tv_usec + static_cast<long>(ms % 1000) * 1000;
    timeval r;
    r.tv_sec = agora.tv_sec + static_cast<time_t>(ms / 1000) + usec / 1000000;
    r.tv_usec = usec % 1000000;
    return r;
}
} // namespace

// Inlined into func 3644.                                                srcloc ctickmanager.cpp:128
uebyte CTickManager::SearchNextId() const
{
    for (unsigned id = 0; id < 256; ++id)
        if (m_ticks.find(static_cast<uebyte>(id)) == m_ticks.end())
            return static_cast<uebyte>(id);
    throw CUeUtilError(EUeUtilError{7077}, "Não há mais id disponível para tick");
}

// wasm func 3644 - observed executing                                     srcloc ctickmanager.cpp:31
// Adds a RUNNING tick of `intervalo` ms and returns its id.
uebyte CTickManager::AddTick(std::size_t intervalo)
{
    if (intervalo < TICK_MINIMO_MS)
        throw CUeUtilError(EUeUtilError{7072}, std::format("O tempo do tick deve ser >= {}", TICK_MINIMO_MS));
    const uebyte id = SearchNextId();
    m_ticks.emplace(id, STick{intervalo, AgoraMais(intervalo), 0});
    return id;
}

// wasm func 5451                                                          srcloc ctickmanager.cpp:60
void CTickManager::StopTick(uebyte id)
{
    auto it = m_ticks.find(id);
    if (it == m_ticks.end())
        throw CUeUtilError(EUeUtilError{7074}, std::format("Não existe um tick com o id {}", id));
    it->second.parado = 1;                                     // i32.store 1 at node+48
}

// wasm func 700 - observed executing                                      srcloc ctickmanager.cpp:69
// The wasm function receives the THREAD object and adds 20: it is the inlined forwarding wrapper
// "CThreadVota::StartTick(id) { m_ticks.StartTick(id); }" with this body inside.
void CTickManager::StartTick(uebyte id)
{
    auto it = m_ticks.find(id);
    if (it == m_ticks.end())
        throw CUeUtilError(EUeUtilError{7075}, std::format("Não existe um tick com o id {}", id));
    it->second.parado = 0;
    it->second.proximo = AgoraMais(it->second.intervalo);      // restart the period from now
}

// wasm func 5452 (tools: vota_f5452) - observed executing. Creates a tick that starts STOPPED
// (func 807 = CThreadVota wrapper passing this+20). Used by the constructors of states that arm their
// timers later (CVotacaoStateAudio's 2000/1500 ms ticks, CPedeIdentidade, ...).       name inferred
uebyte CTickManager::AddStoppedTick(std::size_t intervalo)
{
    const uebyte id = AddTick(intervalo);
    StopTick(id);
    return id;
}

} // namespace api
