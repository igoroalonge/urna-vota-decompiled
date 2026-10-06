// uenux2/src/api/ipc/cmessagequeue.h
//
// Reconstructed from vota_web_wasm.wasm. Unit u18.
//
// api::CPriorityMessageQueue<TMessage>: the inter-thread message queue of VOTA. Each state-machine thread
// owns one (vota::CMessageEleitor at CThreadEleitor+36, vota::CMessageOperador at CThreadOperador+36):
// other threads Add() messages, the owner Remove()s them in priority order (FIFO within one priority).
// Synchronisation: an api::ISyncCtl lock around the container and an api::ISemaphore counting messages.
//
// RTTI: api::CPriorityMessageQueue<api::SMessage> (typeinfo @1534748, vtable @1534772: [0] dtor 3162,
// [1] deleting dtor 3157). Both concrete queues are [vmi] classes
//   vota::CMessageEleitor / vota::CMessageOperador : CPriorityMessageQueue<SMessage>, api::CMessageInterface
// with vtable [2] = Recebe (4343 / thunk 4337 for the CMessageInterface sub-object).
//
// Functions (wasm):
//   ctor              inlined (srclocs cmessagequeue.h:113 semaphore factory, :114 lock factory); the tools
//                     named the two functions that contain it after it: 316 = vota::CThreadEleitor::GetInst,
//                     270 = vota::CThreadOperador::GetInst (reconstructed in src/uenux2/src/app/vota/
//                     u18-foreign-fragments.cpp and cthreadeleitor.cpp)
//   Add               rhvoice_f501 (misattributed to RHVoice; name inferred) - not in u18; = wasm func 501, unit u41
//   Remove            2073 (srcloc :153)
//   Recebe (slot 2)   4343 (name inferred) - not in u18
//   ~                 3162 / 3157 - not in u18
//
// In the web build the lock is a CPosixMutex (its Lock/Unlock only count, pthread calls are stubs) and the
// semaphore a CPosixSemaphore whose Unlock (= sem_post) and Wait are no-ops (see cposixsemaphore.cpp).
#pragma once

#include <cstdint>
#include <memory>
#include <queue>
#include <string>
#include <vector>

#include "api/ipc/euipcerror.h"                     // api::EUeIpcError, api::CUeIpcError ({6150, 6350})
#include "api/ipc/isemaphore.h"                     // api::ISemaphore, api::ESemaphoreResult
#include "api/ipc/isyncctl.h"                       // api::ISyncCtl
#include "api/pattern/cpolysingletonlist.h"         // api::CPolySingletonList
#include "api/pattern/igenericfactory.h"            // api::IGenericFactory<T>

namespace api {

// 8 bytes. `id` is the message code each state machine switches on (see u07/u10 docs for the values).
// The second word is written by every sender with the address of the destination queue (e.g. 6018:
// `SMessage m; m.id = 7; m.destino = &fila;`) and is never read by the consumers seen.   member names inferred
struct SMessage {
    std::int16_t id = 0;          // +0
    const void*  destino = nullptr;   // +4  ?
};

// Mixin that stores the last message received with Recebe() and the semaphore result.
class CMessageInterface {                                    // 16 bytes; name from RTTI
public:
    virtual ~CMessageInterface() = default;                  // slots 0/1 (4341/4338 thunks)
    virtual void Recebe(std::size_t timeoutMs) = 0;          // slot 2 (4337 thunk -> 4343)    name inferred
protected:
    SMessage         m_ultima;                               // +4
    ESemaphoreResult m_resultado = ESemaphoreResult(0);      // +12
};

template <typename TMessage>
class CPriorityMessageQueue {
public:
    // Inlined (srclocs cmessagequeue.h:113 and :114).
    CPriorityMessageQueue()
        : m_pSemaforo(CPolySingletonList::instance<IGenericFactory<ISemaphore>>().Create())   // line 113 (CPosixSemaphore(0))
        , m_pLock(CPolySingletonList::instance<IGenericFactory<ISyncCtl>>().Create())          // line 114 (CPosixMutex)
    {
    }

    // wasm 3162 (complete) / 3157 (deleting): members only.
    virtual ~CPriorityMessageQueue() = default;

    // rhvoice_f501 (not in u18; name inferred). Almost all senders use prioridade 1 (u41: one uses 100, below).
    // wasm func 501 (unit u41, which re-checked this body). Observed executing: every message posted to the
    // voter/operator queues goes through it, e.g. votaInit -> 11026 -> 3903 -> 7708 -> 501.
    // 34 direct call sites in 30 functions. All but one pass prioridade 1. The exception is
    // vota::CVerificaEleicaoPassou::StartState (func 11914), which posts message 0 (stop) to the operator
    // queue with prioridade 100 when the election date is invalid.
    // Order in the binary:
    //   1. lock = m_pLock (+24); lock->Lock()   ISyncCtl slot 2, a plain call: it runs before the guard exists
    //   2. seq = m_sequencia++ (+28)           the counter is consumed even if step 3 throws
    //   3. m_fila.push({mensagem (8 bytes copied as one i64), prioridade, seq})   invoke of func 11076
    //      (vector push_back / __swap_out_circular_buffer slow path + push_heap sift-up with CMenorPrioridade:
    //       prioridade compared signed, sequencia unsigned)
    //   4. m_pSemaforo (+20)->Unlock()          ISemaphore slot 3 = CPosixSemaphore::Unlock (sem_post), invoke
    //   5. ~CLockGuard (func 5528)              also on the landing pad of 3/4, then the exception is rethrown
    //                                           (__resumeException)
    // If step 3 or 4 throws, the lock is released and the semaphore is NOT posted. After a step-4 throw the
    // message stays queued without its semaphore count.
    void Add(const TMessage& mensagem, int prioridade)
    {
        CLockGuard lock(*m_pLock);                            // ISyncCtl slot 2 / slot 3 (api_f5528 = guard dtor)
        m_fila.push(SEntrada{mensagem, prioridade, m_sequencia++});   // shared_f11076 = push_back + push_heap
        m_pSemaforo->Unlock();                                // ISyncCtl slot 3 of the semaphore = sem_post
    }

    // wasm func 2073 (srcloc line 153). Not in the profiler samples, but its caller
    // vota::CThreadEleitor::Processar (4349, logs "ProcessarMensagens") is.
    // The lock is taken with an explicit Lock() and is NOT released when the queue is empty: the
    // exception is thrown while the lock is held (no cleanup code exists on that path).
    TMessage Remove()
    {
        m_pLock->Lock();                                      // ISyncCtl slot 2
        if (m_fila.empty())
            throw CUeIpcError(EUeIpcError(6228), "A fila estava vazia.");                                  // line 153
        const TMessage mensagem = m_fila.top().mensagem;
        m_fila.pop();                                         // pop_heap (Floyd sift-down) + pop_back
        if (m_fila.empty())
            m_sequencia = 0;
        m_pLock->Unlock();                                    // ISyncCtl slot 3
        return mensagem;
    }

    bool Vazia() const                                        // inlined in 4349 (name inferred)
    {
        CLockGuard lock(*m_pLock);
        return m_fila.empty();
    }

protected:
    struct SEntrada {                                         // 16 bytes (name inferred)
        TMessage     mensagem;                                // +0
        int          prioridade;                              // +8
        unsigned int sequencia;                               // +12
    };
    // Heap order: higher priority first; same priority -> lower sequence (older) first.
    struct CMenorPrioridade {
        bool operator()(const SEntrada& a, const SEntrada& b) const
        {
            if (a.prioridade != b.prioridade)
                return a.prioridade < b.prioridade;
            return a.sequencia > b.sequencia;
        }
    };

    std::priority_queue<SEntrada, std::vector<SEntrada>, CMenorPrioridade> m_fila;   // +4 (vector begin/end/cap)
    // +16 ? (not initialised by the constructor)
    //   u41: most likely the empty comparator member `comp` of std::priority_queue (libc++ declares
    //   `container_type c; value_compare comp;` without [[no_unique_address]]: 1 byte, padded to 4). That puts
    //   m_pSemaforo at +20 as observed.
    std::unique_ptr<ISemaphore> m_pSemaforo;                 // +20
    std::unique_ptr<ISyncCtl>   m_pLock;                     // +24
    unsigned int                m_sequencia = 0;             // +28
};

// vota::CMessageEleitor / vota::CMessageOperador slot 2 (wasm 4343; name inferred): the blocking receive
// used by the urna's thread loops (not by the web build).
//   void Recebe(std::size_t timeoutMs) override
//   {
//       m_resultado = m_pSemaforo->Wait(timeoutMs);      // ISemaphore slot 5 (CPosixSemaphore::Wait)
//       if (m_resultado == ESemaphoreResult(0))           // signalled
//           m_ultima = Remove();
//   }
// With the web build's no-op semaphore Wait() always returns 0, so Recebe on an empty queue would throw
// "A fila estava vazia." (and keep the lock, see Remove).

} // namespace api
