// uenux2/src/api/ipc/cthread.cpp
//
// Reconstructed from vota_web_wasm.wasm. Unit u18.
//
// Errors: api::EUeIpcError (CBaseError limits {6150, 6350}); 6231 at line 68.
// CPolySingletonList::instance<I>() is inlined in the constructor with its own checks
// (cpolysingleton.h:78 EPatternErr 1301 "PolySingleton - solicitada uma instancia nao criada ...",
//  cpolysingletonlist.h:99 EUePatternError 6754 "{}: solicitada uma instância não criada de {}",
//  cpolysingletonlist.h:105 6755 "{}: instância {} não corresponde a interface solicitada de {}") - that code
// belongs to api/pattern and is not repeated here.
#include "api/ipc/cthread.h"

#include "api/ipc/euipcerror.h"
#include "api/pattern/cpolysingletonlist.h"
#include "api/pattern/igenericfactory.h"

namespace api {

// wasm func 3598 (srcloc lines 31, 32). Called by the three thread singletons: 316 (CThreadEleitor),
// 270 (CThreadOperador), 1898 (CThreadMonitor::GetInst).
// Observed executing (votaInit creates both state-machine threads' objects).
CThread::CThread()
    : m_pImpl(CPolySingletonList::instance<IGenericFactory<IThreadImpl>>().Create())   // line 31 (web: CWasmThread)
    , m_pSync(CPolySingletonList::instance<IGenericFactory<ISyncCtl>>().Create())      // line 32 (CPosixMutex)
{
}

// wasm func 2721 (vtable slot 0). Members in reverse order: m_pSync, then m_pImpl.
CThread::~CThread() = default;

// wasm func 1684 (srcloc line 68). Only called by the urna execution policy vota::CExecucaoVota
// (10233 / 10235 / 10236); never in the web build (vota::CExecucaoVotaCooperativa).
void CThread::Start()
{
    if (m_estado == EXECUTANDO)
        return;
    m_estado = EXECUTANDO;
    m_bParar = false;
    if (!m_pImpl)
        throw CUeIpcError(EUeIpcError(6231), "Nao foi especificado o tipo de implementação");   // line 68
    m_pImpl->Create(&CThread::ThreadProc, this);                                               // IThreadImpl slot 2
}

// wasm func 1900 (name inferred; tools: vota_f1900). Joins a running thread. Callers:
// vota::CExecucaoVota::vf2 (10236) and ::vf4 (10234), once per thread.
void CThread::Wait()
{
    if (m_estado == EXECUTANDO) {
        m_pImpl->Wait();                                                                       // IThreadImpl slot 3
        m_estado = PARADA;
    }
}

// wasm func 10255 (tools: shared_f10255, table slot 4527; not in u18). Entry point handed to IThreadImpl.
// No try/catch here: CThreadVota::Run handles its own exceptions (slots 3/4).
// Unit u41 re-checked it (24 bytes: call_indirect vtable slot 2 = Run, i32.store +4 = 3, return 0). Never
// executed in the web build: its only user is CThread::Start (1684, `m_pImpl->Create(slot 4527, this)`),
// and only the urna's vota::CExecucaoVota calls Start. The simulator runs its threads cooperatively.
void* CThread::ThreadProc(void* parametro)
{
    auto* thread = static_cast<CThread*>(parametro);
    thread->Run();                                                                             // slot 2
    thread->m_estado = TERMINADA;
    return nullptr;
}

} // namespace api
