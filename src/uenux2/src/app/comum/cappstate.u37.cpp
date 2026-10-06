// uenux2/src/app/comum/cappstate.cpp (path inferred) -- written by unit u37 as cappstate.u37.cpp (no srcloc names the file; the class and
// its context are included as "comum/cappstate.h" by the thread classes of units u07/u17/u18/u35)
// Reconstructed from vota_web_wasm.wasm by unit u37.
//
// comum::CAppState::NeedChangeState and the six forwarding helpers of comum::CAppStateContext that the two
// VOTA thread loops call (CThreadEleitor::Processar, func 4349 - which also runs inside votaTick in
// the web build - and CThreadOperador::Run, func 10204). They are tiny, but they were not inlined: each one
// is its own wasm function. Observed executing in the recorded votes: 7480, 3843, 5909, 5910, 5911.
//
// Where exactly these helpers live is not attested: they could equally be members of the template base
// api::CStateContext<comum::CAppState> (header api/pattern/cstatecontext.h), since they only touch the
// state pointer (+4 of the context) and the state's public flags.                                        ?
#include "comum/cappstate.h"          // comum::CAppStateContext (unit u35)
#include "comum/cappstate.u37.h"      // comum::CAppState

namespace comum {

// wasm func 7480 (vtable comum::CAppState slot 3; inherited unchanged by CAguardaMensagem, CIniciodeCiclo,
// CConfirmaVotoSemCandidato, CMostraTelaContinuaVotacao, CFimVotoEleitor, CSincronismoEleitor,
// CAjusteInicial, testeteclado::CBase, CMostraEleitorVotando, CSuspensaoAutomaticaEleitor, ...).
// Observed executing. The state wants to be left when slot 4 no longer answers itself.
bool CAppState::NeedChangeState()
{
    return GetNextState() != this;                                   // virtual call, slot 4
}

// ---------------------------------------------------------------------------------------------------------
// comum::CAppStateContext (8 bytes: +0 vptr @1551312, +4 CAppState* m_estado). Every helper tolerates an
// empty context (m_estado == nullptr) by doing nothing / answering false.
// ---------------------------------------------------------------------------------------------------------

// wasm func 3842                                                               name inferred (as in u07/u17)
bool CAppStateContext::AceitaMensagens() const
{
    return m_estado != nullptr && m_estado->RecebeMensagens();       // byte +8 of the state
}

// wasm func 5909 - observed executing                                         name inferred
bool CAppStateContext::AceitaTeclado() const
{
    return m_estado != nullptr && m_estado->RecebeTeclado();         // byte +9
}

// wasm func 5908                                                               name inferred
bool CAppStateContext::AceitaTicks() const
{
    return m_estado != nullptr && m_estado->RecebeTicks();           // byte +10
}

// wasm func 3843 - observed executing (CAguardaMensagem, CSincronismoEleitor receive their messages here)
void CAppStateContext::ProcessMessage(uebyte mensagem)
{
    if (m_estado)
        m_estado->ProcessMessage(mensagem);                          // slot 6
}

// wasm func 5911 - observed executing (every key the voter presses)
void CAppStateContext::ProcessInput()
{
    if (m_estado)
        m_estado->ProcessInput();                                    // slot 7
}

// wasm func 5910 - observed executing (every expired tick of the voter thread)
void CAppStateContext::ProcessTick(uebyte tick)
{
    if (m_estado)
        m_estado->ProcessTick(tick);                                 // slot 8
}

} // namespace comum
