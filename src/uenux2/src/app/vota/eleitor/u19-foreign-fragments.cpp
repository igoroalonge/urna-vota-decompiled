// FRAGMENTS reconstructed by unit u19 from vota_web_wasm.wasm.
// Voter-side (eleitor) methods that the tools filed under cpolysingletonlist.h because a GetInst() with
// CPolySingleton<T>::instance is inlined into them. Original files (paths from sibling reconstructions):
//   5925 -> uenux2/src/app/vota/eleitor/votaproporcional/cconfirmavotonominal.cpp (path inferred)
//           (identical body also used as vota::CMajoritarioValido slot 17: ICF)
//   7181 -> uenux2/src/app/vota/eleitor/csincronismoeleitor.cpp (path inferred)
#include <memory>
#include <mutex>

#include "api/pattern/cpolysingleton.h"
#include "vota/eleitor/comum/cpoliticaexecucaoeleitor.h"
#include "vota/eleitor/csincronismovotoeleitor.h"
#include "vota/eleitor/fimvotacao/cfimvotoeleitor.h"
#include "vota/log/clogvota.h"

namespace vota {

// wasm func 5925 = vtable slot 17 of vota::CConfirmaVotoNominal and vota::CMajoritarioValido.
// Slot 17 is a hook of vota::CConfirmaVotoEmCargo (default: no-op, icf_nop 425) that
// CVotacaoStateAudio::EmiteEcoCorrigeConfirma (func 11754) calls with the key just handled
// (5 = CORRIGE, 9 = CONFIRMA).                                                   name inferred: PosTecla
// On CORRIGE on the confirmation screen of a candidate the keypad buffer is flushed, so keys the voter
// typed while the confirmation screen was up are not taken as the next vote.
void CConfirmaVotoNominal::PosTecla(int tecla)
{
    if (tecla != 5)
        return;
    CLogVota::GetInst().Loga("Eleitor corrigiu na tela de confirmação de candidato");   // api_f233
    impl::IPoliticaExecucaoEleitor::GetInst().LimpaBufferInput();                        // vtable slot 2
    // IPoliticaExecucaoEleitor::GetInst() (cpoliticaexecucaoeleitor.cpp:45) is inlined here: lock_guard on
    // a static mutex (@1833324), lazy push of the urna default CPoliticaExecucaoEleitor (push<> = func 5407,
    // called directly: no exists/erase prefix).
    // In the web build main() has already registered (anonymous)::CPoliticaExecucaoEleitorWeb, whose
    // LimpaBufferInput (func 10835) flushes once instead of the urna's 3-6 random pauses (func 13564).
}

// wasm func 5925 again: vota::CMajoritarioValido::PosTecla is the same body (identical code folding).

// wasm func 7181 = vtable slot 6 (ProcessMessage) of vota::CSincronismoEleitor.
// The voter thread waits here for message 5 from the operator thread ("vote synchronised"):
// the vote is made durable (RDV) by ISincronismoVotoEleitor::SincronizaVoto and the voter goes to "FIM".
void CSincronismoEleitor::ProcessMessage(short idMensagem)
{
    m_proximoEstado = this;
    if (idMensagem != 5)
        return;
    // ISincronismoVotoEleitor::GetInst() inlined (csincronismovotoeleitor.cpp:67, static mutex @1833164):
    // lazy default CSincronismoVotoEleitor (vtable @1534164), push<> = func 5398 (no exists/erase prefix).
    // The web build registered (anonymous)::CSincronismoVotoEleitorWeb from main().
    if (impl::ISincronismoVotoEleitor::GetInst().SincronizaVoto())                    // vtable slot 2
        m_proximoEstado = &CFimVotoEleitor::GetInst();   // lazy singleton: unique_ptr @1833132, mutex @1833108,
                                                         // 12-byte object (vtable @1534012)
}

}  // namespace vota
