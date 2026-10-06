// Reconstructed from vota_web_wasm.wasm (unit u31).
// Original file UNKNOWN - path inferred: uenux2/mock/app/simulador/wasm/cwasmtimer.h
//
// simulador::CWasmTimer : api::ITimer - a periodic GUI timer (status-bar clock, blinking fields, battery
// icon, CImageFieldUpdate pages...). The urna's api::CTimer runs a std::thread; this build has no pthreads
// (api::CTimer::Start, func 10850, would throw "thread constructor failed"), so the web timer re-arms itself
// through the Emscripten import emscripten_async_call(func, arg, ms), i.e. a JavaScript setTimeout.
//
// RTTI: simulador::CWasmTimer (typeinfo @1532112, si) : api::ITimer (typeinfo @1532124); vtable @1532088
//       (6 slots, table entries 839..844). Created only by CWasmTimerScheduler::CreateTimer (func 7871) with
//       std::make_shared (control block vtable @1532212); its State with std::make_shared too (@1532140).
#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>

#include "api/util/itimerscheduler.h"   // api::ITimer (slots: 0/1 dtor, 2 Start, 3 Stop, 4 IsRunning, 5 SetInterval)

namespace simulador {

class CWasmTimer : public api::ITimer {
public:
    // Shared between the timer and every pending JavaScript callback (48 bytes).
    struct State {
        bool ativo = false;                  // +0   running
        std::uint64_t geracao = 0;           // +8   bumped by every Start/Stop: stale callbacks see a mismatch
        int intervalo;                       // +16  period in ms (std::chrono::milliseconds truncated to int)
        std::function<void()> callback;      // +24  (__f_ at +40)

        State(int ms, std::function<void()> cb) : intervalo(ms), callback(std::move(cb)) {}
    };

    // Inlined into func 7871.
    CWasmTimer(const std::chrono::milliseconds& intervalo, std::function<void()> callback);
    ~CWasmTimer() override;                              // slot 0 (3347), slot 1 deleting (7926)

    void Start() override;                               // slot 2 (7918)
    void Stop() override;                                // slot 3 (7934)
    bool IsRunning() const override;                     // slot 4 (7902)
    void SetInterval(std::int64_t ms) override;          // slot 5 (7896)

private:
    // What emscripten_async_call carries (16 bytes, heap-allocated per arming).
    struct SAgendamento {
        std::shared_ptr<State> estado;       // +0/+4
        std::uint64_t geracao;               // +8
    };
    static void Dispara(void* arg);          // wasm func 7909, table slot 838 (the tools left it "unknown_f7909")

    std::shared_ptr<State> m_estado;         // +4/+8   (object = 12 bytes)
};

}  // namespace simulador
