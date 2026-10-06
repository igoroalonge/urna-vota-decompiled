// Reconstructed from vota_web_wasm.wasm (unit u31).
// Original file: uenux2/mock/app/simulador/wasm/cwasmthread.cpp (attested: srcloc cwasmthread.cpp:31,
// "virtual void simulador::CWasmThread::Create(void *(*)(void *), void *)").
//
// simulador::CWasmThread : api::IThreadImpl - the "OS thread" behind api::CThread in the web build
// (typeinfo @1528424, vtable @1528400: [0] ~ (ICF 174) [1] deleting (ICF 144) [2] Create (9655)
// [3] Wait (9652) [4] no-op(x, y) (ICF 1528) [5] Yield (9651)). Handed out by the factory
// CDefaultGenericFactory<IThreadImpl, CWasmThread> registered by main().
//
// The build has no pthreads and no Asyncify, so a real thread cannot exist. Create() only RECORDS the
// request in a global queue - nothing in the binary ever reads that queue or calls the thread function
// (xrefs of @1832648: only 9652, 9655 and the atexit destructor 9662). The voter and operator "threads" of
// the web build are driven instead by vota::CExecucaoVotaCooperativa from votaTick (unit u29/u07).
// Wait() and Yield() call emscripten_sleep, whose glue implementation is abort("Please compile your program
// with async support..."): calling either one kills the simulator.
// None of these functions ran during the recorded votes.
#include <algorithm>
#include <string>
#include <vector>

#include <emscripten.h>                 // emscripten_sleep

#include "api/ipc/ithreadimpl.h"
#include "simulador/wasm/cwasmjs.h"     // js_thread_log -> console.log("[CWasmThread]", msg) (always printed)

namespace simulador {

namespace {

// One Create() request (16 bytes). The vector below is also declared by unit u29 (cwasmthread.u29.cpp, with its
// atexit destructor func 9662); the names SThreadWasm / s_threads are u29's.
struct SThreadWasm {
    void* (*funcao)(void*);    // +0
    void* argumento;           // +4
    bool terminou = false;     // +8   never set to true by anything
    int id;                    // +12
};

std::vector<SThreadWasm*> s_threads;         // @1832648 (begin/end/cap); freed at exit by func 9662
int s_ultimoId = 0;                          // @1832660

}  // namespace

class CWasmThread : public api::IThreadImpl {
public:
    void Create(void* (*funcao)(void*), void* argumento) override;   // slot 2 (9655)
    void Wait() override;                                           // slot 3 (9652)
    void Slot4(int, int) override {}                                // slot 4 (ICF 1528) ?
    void Yield() override;                                          // slot 5 (9651)
private:
    SThreadWasm* m_contexto = nullptr;                          // +4 (8-byte object)
};

// wasm func 9655 - slot 2                                                        srcloc cwasmthread.cpp:31
void CWasmThread::Create(void* (*funcao)(void*), void* argumento)
{
    if (funcao == nullptr)
        throw api::CUeIpcError(static_cast<api::EUeIpcError>(6231), "CWasmThread::Create sem função válida");

    auto* contexto = new SThreadWasm{funcao, argumento, false, ++s_ultimoId};
    m_contexto = contexto;                      // a previous context of this object is simply forgotten
    s_threads.push_back(contexto);
    js_thread_log(("Create #" + std::to_string(contexto->id) + " - enqueued").c_str());
}

// wasm func 9652 - slot 3 (join)
void CWasmThread::Wait()
{
    SThreadWasm* contexto = m_contexto;
    if (contexto == nullptr) {
        js_thread_log("Wait - no ctx, returning");
        return;
    }
    js_thread_log("Wait - polling");
    while (!contexto->terminou)
        emscripten_sleep(100);                  // aborts: no Asyncify (and `terminou` never becomes true)
    js_thread_log("Wait - finished");
    if (auto it = std::find(s_threads.begin(), s_threads.end(), contexto); it != s_threads.end())
        s_threads.erase(it);
    delete contexto;
    m_contexto = nullptr;
}

// wasm func 9651 - slot 5 (called by CThreadEleitor::Run / CThreadOperador::Run between two iterations -
// loops that the web build never runs)
void CWasmThread::Yield()
{
    js_thread_log("Yield");
    emscripten_sleep(0);                        // aborts: no Asyncify
    js_thread_log("Yield - returned");
}

}  // namespace simulador
