// uenux2/src/app/comum/cappstate.h (path inferred) -- FRAGMENT written by unit u37: the declaration of
// comum::CAppState. comum::CAppStateContext is declared by unit u35 in cappstate.h; this file only adds the
// state class itself, whose constructor/virtuals were found in u37's functions. Bodies: cappstate.u37.cpp.
//
// comum::CAppState = base class of EVERY state of the urna's state machines (voter screens vota::CPede*,
// the encerramento steps vota::CGeraBU..., the operator screens, the mesário registration states...).
// Each state is a lazily created singleton; the thread that owns the machine (CThreadEleitor,
// CThreadOperador, or the web adapter vota::CExecucaoVotaCooperativa) holds a comum::CAppStateContext whose
// m_estado points at the current state and feeds it messages, keys and timer ticks.
//
// RTTI: api::CState <- comum::CAppState (typeinfo @1551284, vtable @1551248, 9 slots):
//   [0] ~CAppState            icf 174 (trivial)          [1] deleting dtor  icf 325 (unreachable: abstract)
//   [2] StartState()          __cxa_pure_virtual         [3] NeedChangeState()  wasm 7480
//   [4] GetNextState()        icf 1661 (return +4)       [5] FinishState()      icf 218 (no-op)
//   [6] ProcessMessage(uebyte) icf 425 (no-op)           [7] ProcessInput()     icf 218 (no-op)
//   [8] ProcessTick(uebyte)    icf 425 (no-op)
// Slot names: 3 and 7 come from srcloc-named overrides (CDefineRotaPosReinicio::NeedChangeState,
// CPedeMajoritario::ProcessInputAudio -> ProcessInput); the others from their use by the threads (u07/u35).
#pragma once

#include <cstdint>

#include "api/pattern/cstate.h"   // api::CState (RTTI only: no vtable of its own survives)   (header name ?)

namespace comum {

using uebyte = std::uint8_t;

class CAppState : public api::CState {
public:
    // What the owning thread may deliver to the state (constructor argument, a bit mask).   names inferred
    enum EEntradas : int {
        MENSAGENS = 1,      // inter-thread messages (CPriorityMessageQueue<SMessage>)
        TECLADO   = 2,      // key presses (IInputKbd / IInputMT)
        TICKS     = 4,      // timer ticks (api::CTickManager)
    };

    virtual ~CAppState() = default;                                  // [0]/[1]

    virtual void StartState() = 0;                                   // [2]
    virtual bool NeedChangeState();                                  // [3] wasm 7480 (cappstate.u37.cpp)
    virtual CAppState* GetNextState() { return m_proximoEstado; }     // [4] icf 1661
    virtual void FinishState() {}                                    // [5]
    virtual void ProcessMessage(uebyte /*mensagem*/) {}              // [6]
    virtual void ProcessInput() {}                                   // [7]
    virtual void ProcessTick(uebyte /*tick*/) {}                     // [8]

    bool RecebeMensagens() const { return m_recebeMensagens; }       // read by wasm 3842
    bool RecebeTeclado() const { return m_recebeTeclado; }           // read by wasm 5909
    bool RecebeTicks() const { return m_recebeTicks; }               // read by wasm 5908

protected:
    // shared_f224 (merged, other unit): stores the vptr, m_proximoEstado = this and the three flags.
    explicit CAppState(int entradas)
        : m_proximoEstado(this)
        , m_recebeMensagens((entradas & MENSAGENS) != 0)
        , m_recebeTeclado((entradas & TECLADO) != 0)
        , m_recebeTicks((entradas & TICKS) != 0)
    {
    }

    CAppState* m_proximoEstado;     // +4   "stay here" = this; states assign the next state here
    bool m_recebeMensagens;         // +8
    bool m_recebeTeclado;           // +9
    bool m_recebeTicks;             // +10
    // sizeof 12 (+11 is padding or the first byte of a subclass member, e.g. CSincronismoOperador +11)
};

} // namespace comum
