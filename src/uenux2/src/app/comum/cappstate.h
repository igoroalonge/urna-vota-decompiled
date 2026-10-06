// uenux2/src/app/comum/cappstate.h   (path inferred: the class is included under this name by the thread classes
// reconstructed in units u07/u17/u18; no srcloc record names the file)
// Reconstructed from vota_web_wasm.wasm (unit u35).
//
// comum::CAppState is the base of every state of the urna's state machines (the voter's screens vota::CPede*,
// the poll worker's screens, the encerramento steps...). comum::CAppStateContext is the "current state" holder
// that each cooperative thread (vota::CThreadEleitor, vota::CThreadOperador, vota::CExecucaoVotaCooperativa)
// owns in a std::unique_ptr and advances on every tick.
//
// RTTI:
//   comum::CAppState         : api::CState                           vtable @1551248 (slot 2 pure, slot 3 = 7480)
//   comum::CAppStateContext  : api::CStateContext<comum::CAppState>  vtable @1551312
//                              [0] icf_ret_this_vf0 (trivial destructor)  [1] operator delete (deleting dtor)
#pragma once

#include "api/pattern/cstatecontext.h"   // api::CState, api::CStateContext<T>   (header name ?)

namespace comum {

class CAppState;   // declared by unit u37 in cappstate.u37.h (same original file)

// sizeof 8: +0 vptr, +4 CAppState* m_estado (inherited from api::CStateContext<CAppState>)
class CAppStateContext : public api::CStateContext<CAppState>
{
public:
    // wasm func 3844 - name from RTTI (the only function that stores vtable @1551312). Not observed executing in
    // the recorded votes: it runs once per thread at start-up (callers: vota::CExecucaoVotaCooperativa slot 2
    // (4713), vota::CThreadEleitor slot 2 (7061), vota::CThreadOperador::Run (10204)).
    explicit CAppStateContext(CAppState* estadoInicial)
        : api::CStateContext<CAppState>(estadoInicial)          // +4 = estadoInicial
    {
    }

    ~CAppStateContext() override = default;                     // vtable slot 0 = icf 174, slot 1 = free (144)
};

} // namespace comum
