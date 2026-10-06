// Reconstructed from vota_web_wasm.wasm (unit u31).
// Original file UNKNOWN - path inferred: uenux2/mock/app/simulador/wasm/cwasmtimer.cpp
//
// Observed executing: 7918 (Start), 7934 (Stop), 7909 (Dispara). One municipal vote arms 179 callbacks
// (162 x 1000 ms, 11 x 500 ms, 6 x 600 ms; docs/03-js-wasm-interface.md s15); the count grows with the wall-clock
// length of the session (381 emscripten_async_call in a headless run of the same keys, --trace-imports).
#include "simulador/wasm/cwasmtimer.h"

#include <emscripten.h>   // emscripten_async_call

namespace simulador {

// Inlined into CWasmTimerScheduler::CreateTimer (func 7871).
CWasmTimer::CWasmTimer(const std::chrono::milliseconds& intervalo, std::function<void()> callback)
    : m_estado(std::make_shared<State>(static_cast<int>(intervalo.count()), std::move(callback)))
{
}

// wasm func 3347 - slot 0 (and func 7926 - slot 1, the deleting destructor, which calls it then frees).
// Stopping bumps the generation, so a callback already queued in JavaScript becomes a no-op.
CWasmTimer::~CWasmTimer()
{
    Stop();                                                                     // inlined
}

// wasm func 7918 - slot 2
void CWasmTimer::Start()
{
    Stop();                                                                     // inlined: ativo = false, ++geracao
    m_estado->ativo = true;
    const std::uint64_t geracao = ++m_estado->geracao;
    emscripten_async_call(&CWasmTimer::Dispara, new SAgendamento{m_estado, geracao}, m_estado->intervalo);
}

// wasm func 7934 - slot 3
void CWasmTimer::Stop()
{
    m_estado->ativo = false;
    ++m_estado->geracao;
}

// wasm func 7902 - slot 4
bool CWasmTimer::IsRunning() const
{
    return m_estado->ativo;
}

// wasm func 7896 - slot 5 (takes effect at the next arming)
void CWasmTimer::SetInterval(std::int64_t ms)
{
    m_estado->intervalo = static_cast<int>(ms);
}

// wasm func 7909 (table slot 838; component "unknown" in the tools) - runs from a JavaScript setTimeout
// (glue: _emscripten_async_call -> safeSetTimeout -> callUserCallback(getWasmTableEntry(838)(arg))), i.e.
// OUTSIDE votaTick and outside its try/catch.
// Unit u41 compared this body with the wasm again and found it faithful. Details:
//   * the std::function call is libc++'s: `__f_` (State +40) == nullptr -> std::__throw_bad_function_call
//     (func 648), otherwise __base vtable slot 6 (operator()).
//   * the re-arming copies the shared_ptr (use count +1 at ctrl+4, skipped when the control block is null)
//     and stores the State's current geracao, which is equal to agendamento->geracao at that point.
//   * `delete agendamento` = ~shared_ptr (ctrl slot 2 __on_zero_shared + __release_weak when the count was 0)
//     and free().
//   * Each pending call owns a reference to the State. If the callback destroys the CWasmTimer (~CWasmTimer
//     -> Stop -> ++geracao), the State and the std::function that is running stay alive until this call ends.
//     The re-check then fails and nothing is re-armed.
void CWasmTimer::Dispara(void* arg)
{
    auto* agendamento = static_cast<SAgendamento*>(arg);
    State& estado = *agendamento->estado;
    if (estado.ativo && estado.geracao == agendamento->geracao) {
        estado.callback();          // plain call_indirect, no invoke_*: std::bad_function_call if empty, and any
                                    // exception escapes to JavaScript (callUserCallback -> handleException ->
                                    // quit_ rethrows it out of the setTimeout). The timer is then never re-armed,
                                    // `agendamento` leaks together with its reference to the State (so the State
                                    // and the callback's captures are never freed), and __stack_pointer is not
                                    // restored (no invoke_* wrapper does stackRestore on this path).
        if (estado.ativo && estado.geracao == agendamento->geracao) {
            emscripten_async_call(&CWasmTimer::Dispara,
                                  new SAgendamento{agendamento->estado, agendamento->geracao}, estado.intervalo);
        }
    }
    delete agendamento;
}

}  // namespace simulador
