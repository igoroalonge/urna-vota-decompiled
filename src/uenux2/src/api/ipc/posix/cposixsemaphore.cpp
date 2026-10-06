// uenux2/src/api/ipc/posix/cposixsemaphore.cpp
//
// Reconstructed from vota_web_wasm.wasm. Unit u18.
//
// api::CPosixSemaphore : api::ISemaphore (: api::ISyncCtl) - POSIX counting semaphore. Created by
// CDefaultGenericFactory<ISemaphore, CPosixSemaphore> (wasm 10854: operator new(20) + CPosixSemaphore(0));
// used by the message queues (CPriorityMessageQueue ctor, cmessagequeue.h:113) to count pending messages.
//
// RTTI: typeinfo @1600308, vtable @1600184:
//   [0] ~ (5354)  [1] deleting ~ (10249)  [2] Lock = sem_wait (2219, ICF empty)  [3] Unlock = sem_post (2219)
//   [4] Wait() calling slot 2 (10248)  [5] Wait(size_t) (10247, srcloc :48)  [6] GetValue (10246)
// Names: [5] is attested by its referenced srcloc. The dead error paths of Lock, Unlock, GetValue and of the
// second throw of Wait(size_t) left unreferenced srcloc records in the data segment:
//   @1600244 Wait(size_t) line 76, @1600260 "virtual void api::CPosixSemaphore::Lock()" line 90,
//   @1600276 "virtual void api::CPosixSemaphore::Unlock()" line 99,
//   @1600292 "virtual int api::CPosixSemaphore::GetValue() const" line 114.
// Slots 2/3 = Lock/Unlock follow ISyncCtl (CPosixMutex's slot 2 increments, slot 3 decrements); slot 6 is the
// only one that returns an int (the value sem_getvalue stores), so it is GetValue. Only [4] stays inferred.
// Messages of the dead paths: unreferenced strings "Falha ao bloquear semáforo: {}" @4474,
// "Falha ao desbloquear semáforo: {}" @4440, "Falha ao obter valor do semáforo: {}" @4505 (which line uses
// which is inferred). Codes 6161..6164 are inferred from the numbering (6159 = line 23, 6160 = line 48).
// Layout (20 bytes): +0 vptr, +4 sem_t m_semaforo (16 bytes; musl: value, waiters, private flag 128).
//
// Web build: sem_init is musl's (inlined: value / 0 / 128, EINVAL above SEM_VALUE_MAX), but sem_wait,
// sem_post and sem_timedwait are no-op stubs returning 0 and sem_getvalue stores 0. So in the simulator a
// "wait" never blocks and always reports success, and the value is always 0. Nothing here was observed
// executing (the voter thread never blocks on its queue in the web build).
#include "api/ipc/posix/cposixsemaphore.h"

#include <cerrno>
#include <cstring>
#include <ctime>
#include <format>
#include <semaphore.h>

#include "api/ipc/euipcerror.h"

namespace api {

// wasm func 10250 (srcloc line 23), table slot 245 (used by the factory with valorInicial = 0)
CPosixSemaphore::CPosixSemaphore(const unsigned int valorInicial)
{
    if (sem_init(&m_semaforo, 0, valorInicial) == -1)
        throw CUeIpcError(EUeIpcError(6159), std::format("Falha ao criar semáforo: {}", std::strerror(errno)));   // line 23
}

// wasm func 5354 (slot 0) / 10249 (slot 1, deleting)
CPosixSemaphore::~CPosixSemaphore()
{
    sem_destroy(&m_semaforo);                                        // stub                                   // ?
}

// slot 2 = wasm 2219 (ICF empty body; srcloc record of line 90 left by the dead error path)
void CPosixSemaphore::Lock()
{
    if (sem_wait(&m_semaforo) == -1)                                                                    // stub -> 0
        throw CUeIpcError(EUeIpcError(6162), std::format("Falha ao bloquear semáforo: {}", std::strerror(errno)));     // line 90 (code/text ?)
}

// slot 3 = wasm 2219 (ICF empty body; srcloc record of line 99)
void CPosixSemaphore::Unlock()
{
    if (sem_post(&m_semaforo) == -1)                                                                    // stub -> 0
        throw CUeIpcError(EUeIpcError(6163), std::format("Falha ao desbloquear semáforo: {}", std::strerror(errno)));  // line 99 (code/text ?)
}

// wasm func 10248 (slot 4): forwards to slot 2 (virtual call). name inferred
void CPosixSemaphore::Wait()
{
    Lock();
}

// wasm func 10247 (slot 5, srcloc lines 48 and 76). `timeoutMs == 0` waits without a deadline.
// Returns ESemaphoreResult 0 (= signalled; the queue's Recebe then calls Remove). In the web build the
// waits compiled to nothing, so the function always returns 0 and the line-76 path is gone (its srcloc
// record @1600244 is unreferenced). Column 19 of line 48 vs. column 15 of line 76: the clock error is one
// block deeper than the final check, which is why both waits are joined below.                     // ?
ESemaphoreResult CPosixSemaphore::Wait(const std::size_t timeoutMs)
{
    int resultado;
    if (timeoutMs == 0) {
        resultado = sem_wait(&m_semaforo);                                                              // stub -> 0
    } else {
        timespec limite{};
        if (clock_gettime(CLOCK_REALTIME, &limite) == -1)
            throw CUeIpcError(EUeIpcError(6160),
                              std::format("Falha ao obter tempo de relógio: {}", std::strerror(errno)));   // line 48

        limite.tv_sec  += static_cast<time_t>(timeoutMs / 1000);
        limite.tv_nsec += static_cast<long>(timeoutMs % 1000) * 1000000;
        if (limite.tv_nsec >= 1000000000) {
            limite.tv_nsec -= 1000000000;
            limite.tv_sec  += 1;
        }
        resultado = sem_timedwait(&m_semaforo, &limite);                                                // stub -> 0
    }

    if (resultado == -1 && errno == ETIMEDOUT)
        return ESemaphoreResult(1);                                                                     // value ?
    if (resultado == -1)
        throw CUeIpcError(EUeIpcError(6161), std::format("Falha ao bloquear semáforo: {}", std::strerror(errno)));     // line 76 (code/text ?)
    return ESemaphoreResult(0);
}

// wasm func 10246 (slot 6; name attested by the srcloc record of line 114). sem_getvalue is a stub that
// writes 0, so the function returns 0 and the error path is gone.
int CPosixSemaphore::GetValue() const
{
    int valor = 0;
    if (sem_getvalue(const_cast<sem_t*>(&m_semaforo), &valor) == -1)
        throw CUeIpcError(EUeIpcError(6164), std::format("Falha ao obter valor do semáforo: {}", std::strerror(errno)));  // line 114 (code ?)
    return valor;
}

} // namespace api
