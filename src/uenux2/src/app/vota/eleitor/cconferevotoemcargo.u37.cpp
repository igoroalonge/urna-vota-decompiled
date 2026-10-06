// uenux2/src/app/vota/eleitor/cconferevotoemcargo.cpp (attested by srclocs :80, :95) -- FRAGMENT written by
// unit u37. Class: cconferevotoemcargo.h (unit u06).
//
// CORRECTION to cconferevotoemcargo.h: the constructor has ONE parameter (the screen); whether the state
// waits for a tick is not an argument but comes from the configuration (see below).
#include <memory>

#include "comum/dados/cconfiguracaoeleicao.h"
#include "vota/eleitor/cconferevotoemcargo.h"
#include "vota/eleitor/cthreadeleitor.h"

namespace vota {

// wasm func 1165 (tools: vota_f1165) - observed executing (every vote: the GetInst of each
// CConfereVotoEmCargo<TConfirma, TELA> is inlined in CPedeMajoritario::ProcessInputAudio (11683),
// CPedeProporcional::ProcessInputAudio (11711), CPedeNominal slot 16 (11724), CPedeNulo::GetProximoEstado
// (11744): new(40) + this constructor + the vptr of the template class).        name inferred (ctor)
//
// "Conferência" = the transient screen shown right after the voter typed the last digit. It lasts
// ParametrosUrna.tempoConfirmacaoVoto milliseconds (CParametrosUrna +88 = CConfiguracaoEleicao +176;
// 1000 in the published scenarios): a stopped tick of that length is created on the voter thread and
// started by StartStateAudio; when it fires, the CONFIRMA/CORRIGE screen follows. Keys pressed meanwhile
// are rejected ("Tecla indevida pressionada"). With tempoConfirmacaoVoto <= 0 there is no tick and the
// state moves on immediately.
IConfereVotoEmCargo::IConfereVotoEmCargo(ETelaVotacao tela)
    : CVotacaoStateAudio(TECLADO | TICKS)                                     // wasm 1785, flags 6
    , m_tick(0)
    , m_comTick(false)
    , m_tela(tela)                                                            // +30
    , m_espera()                                                              // +32/+36 empty shared_ptr
{
    const int tempoConfirmacao = comum::CConfiguracaoEleicao::GetInst().GetParametrosUrna().GetTempoConfirmacaoVoto();
    if (tempoConfirmacao > 0) {
        m_tick = CThreadEleitor::GetInst().CriaTick(tempoConfirmacao);       // wasm 316 + 807 -> AddStoppedTick
        m_comTick = true;
    }
}

// wasm func 1286 (tools: vota_f1286). Not a TSE function: the out-of-line
//     std::unique_ptr<IConfereVotoEmCargo>::reset()      (delete through the non-virtual-dispatch dtor 1717)
// used by the at-exit destructors of the static `s_inst` of every CConfereVotoEmCargo<...>::GetInst()
// (wasm 11674, 11676, 11678, 11680, 11708, 11718, 11720, 11722, 11740, 11742; see u30-foreign-fragments.cpp).
// Its first parameter is the unused at-exit argument. Dead code in this build (at-exit handlers never run).

} // namespace vota
