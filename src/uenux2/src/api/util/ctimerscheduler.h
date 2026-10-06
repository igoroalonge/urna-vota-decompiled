// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/util/ctimerscheduler.h (path inferred; itimerscheduler.cpp of unit u20 already
// includes it). api::CTimer may have its own ctimer.h/.cpp on the urna (path inferred).
//
// The default (POSIX) implementation of the GUI timer service: one std::thread per timer that sleeps on a
// condition variable and calls the callback every `intervalo` milliseconds.
//
//   api::CTimerScheduler : api::ITimerScheduler   typeinfo 1585416, vtable @1585404, 4 bytes (vptr only)
//   api::CTimer          : api::ITimer            typeinfo 1585384, vtable @1585360, 144 bytes
//                                                  (allocated with make_shared: 160-byte control block)
//
// WEB BUILD: the module is compiled without pthreads. std::thread's constructor always fails, so
// CTimer::Start (func 10850) is compiled to "throw std::system_error(138, "thread constructor failed")"
// (errno 138 = ENOTSUP in Emscripten's numbering) and the worker loop was removed as dead code. The
// simulator registers simulador::CWasmTimerScheduler (driven by votaTick) as ITimerScheduler before
// anything asks for a timer (CSimuladorWasm::Executa, unit u19), so these classes are never instantiated
// in the recorded sessions. If ITimerScheduler::GetInst() ran first, it would create a CTimerScheduler
// (itimerscheduler.cpp:23), the simulator's CreateInst<CWasmTimerScheduler> would then throw "Tentativa de
// recriar o singleton", and any timer started would throw std::system_error.
#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>

#include "api/util/itimerscheduler.h"   // api::ITimerScheduler::GetInst / CreateInst (unit u20)

namespace api {

// Interfaces (declared in itimer.h / itimerscheduler.h; repeated here with the slot order seen in the
// vtables of CTimer / simulador::CWasmTimer and CTimerScheduler / simulador::CWasmTimerScheduler).
//
// class ITimer {
// public:
//     virtual ~ITimer();                                     // slots 0/1
//     virtual void Start() = 0;                              // slot 2
//     virtual void Stop() = 0;                               // slot 3
//     virtual bool IsRunning() const = 0;                    // slot 4
//     virtual void SetInterval(std::int64_t ms) = 0;         // slot 5
// };
//
// class ITimerScheduler {                                    // the factory method comes BEFORE the destructor
// public:
//     virtual std::shared_ptr<ITimer> CreateTimer(const std::chrono::milliseconds& periodo,
//                                                 const std::function<void()>& callback) = 0;   // slot 0
//     virtual ~ITimerScheduler();                            // slots 1/2
// };
// (units u02/u15 call slot 0 "Agenda"; u16 "CreateTimer".)

class CTimer : public ITimer {
public:
    // Inlined in CTimerScheduler::CreateTimer (func 10846): everything after the callback is zeroed
    // (memset of the 100 bytes at +44).
    CTimer(const std::chrono::milliseconds& intervalo, std::function<void()> callback)
        : m_intervalo(intervalo.count()), m_callback(std::move(callback))
    {
    }
    ~CTimer() override;                                               // wasm 5445 / 10852

    void Start() override;                                            // wasm 10850 (other unit)
    void Stop() override;                                             // wasm 10847
    bool IsRunning() const override;                                  // wasm 10851
    void SetInterval(std::int64_t ms) override;                       // wasm 10848

private:
    void Executa();                    // thread body - not present in the binary (dead code, see above)   ?

    std::int64_t            m_intervalo;       // +8   period in ms
    std::function<void()>   m_callback;        // +16  (__f_ at +32)
    std::atomic<bool>       m_ativo{false};    // +40
    std::condition_variable m_cv;              // +44  (48 bytes)
    std::mutex              m_mutex;           // +92  used with m_cv by the worker        ?
    std::thread             m_thread;          // +116
    std::mutex              m_mutexThread;     // +120 guards join()                      ?
};

class CTimerScheduler : public ITimerScheduler {
public:
    std::shared_ptr<ITimer> CreateTimer(const std::chrono::milliseconds& periodo,
                                        const std::function<void()>& callback) override;   // wasm 10846
    // slots 1/2: trivial destructor (ICF 174 / operator delete 144)
};

} // namespace api
