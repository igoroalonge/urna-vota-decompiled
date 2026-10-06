// uenux2/src/api/ipc/cthread.h   (path inferred from cthread.cpp, attested by srclocs)
//
// Reconstructed from vota_web_wasm.wasm. Unit u18.
//
// api::CThread: abstract base of VOTA's threads (vota::CThreadVota <- CThreadEleitor / CThreadOperador,
// vota::CThreadMonitor). The OS thread itself is an api::IThreadImpl obtained from the poly-singleton
// IGenericFactory<IThreadImpl>: on the urna a pthread implementation, in the web build
// simulador::CWasmThread (uenux2/mock/app/simulador/wasm/cwasmthread.cpp), whose Wait() polls with
// emscripten_sleep(100) and would abort because the build has no Asyncify.
//
// RTTI: api::CThread (typeinfo @1600028), vtable @1599960:
//   [0] ~CThread (2721)  [1] deleting dtor (325 = ICF `unreachable`: the class is abstract)
//   [2] Run() = 0        [3] TrataExcecao(...) = 0   [4] TrataExcecaoDesconhecida() = 0   (names from
//   the CThreadVota overrides, unit u07)
//
// api::IThreadImpl slots (from simulador::CWasmThread, vtable @1528400): [2] Create(void*(*)(void*), void*)
// (cwasmthread.cpp:31), [3] Wait (log "Wait - polling"), [4] no-op, [5] Yield.
#pragma once

#include <memory>

#include "api/ipc/ithreadimpl.h"          // api::IThreadImpl
#include "api/ipc/isyncctl.h"             // api::ISyncCtl

namespace api {

class CThread {
public:
    enum EEstado : int {                  // values from the code, names inferred
        PARADA     = 0,                   // not started / joined
        EXECUTANDO = 1,                   // set by Start()
        TERMINADA  = 3,                   // set by ThreadProc after Run() returns
    };

    CThread();                            // wasm 3598 (srclocs cthread.cpp:31, :32)
    virtual ~CThread();                   // wasm 2721

    void Start();                         // wasm 1684 (cthread.cpp:68)
    void Wait();                          // wasm 1900 (name inferred)

protected:
    virtual void Run() = 0;                                        // slot 2
    virtual void TrataExcecao(const std::exception& erro) = 0;     // slot 3 (name from CThreadVota, u07)
    virtual void TrataExcecaoDesconhecida() = 0;                   // slot 4 (name from CThreadVota, u07)

    static void* ThreadProc(void* parametro);                      // wasm 10255 (table slot 4527; name inferred)

    EEstado                      m_estado = PARADA;   // +4
    bool                         m_bParar = false;    // +8  stop requested (set by IExecucaoVota::FinalizaThreads)
    bool                         m_bDormindo = false; // +9
    std::unique_ptr<IThreadImpl> m_pImpl;             // +12
    std::unique_ptr<ISyncCtl>    m_pSync;             // +16
};

} // namespace api
