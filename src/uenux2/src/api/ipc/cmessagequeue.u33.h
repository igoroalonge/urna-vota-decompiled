// uenux2/src/api/ipc/cmessagequeue.h  -- FRAGMENT written by unit u33 (srcloc cmessagequeue.h:113/114/153; the
// class is reconstructed in cmessagequeue.h by unit u18).
#pragma once

#include <cstdint>

#include "api/ipc/cmessagequeue.h"

namespace api {

// wasm func 3903 (observed executing: votaInit and votaTick)                        // name inferred
// Posts message `id` with priority 1. The SMessage carries the queue itself as its second word ("destino",
// +4): {int16 id, const void* = this}. The Add(msg, 1) wrapper is func 7708 -> rhvoice_f501 (the real
// CPriorityMessageQueue<SMessage>::Add: lock, push {msg, prioridade, sequência++} on the heap, post the
// semaphore).
// Callers are three one-line lambdas of the web entry point that take the voter queue
// (vota::CThreadEleitor::GetInst() + 36), invoked through table slots 57 / 55 / 110:
//   11100  votaInit: MSG_AUDIO_HABILITADO (9) - only when the page option "audioEleitorHabilitado" is true
//          (votaInit stores it in its state byte +3 and tests it), and before message 1
//   11026  votaInit: MSG_INICIA_ELEITOR (1), always
//   10376  votaTick: MSG_SINCRONIZA_VOTO (5), after the four progress-bar steps of CSincronismoEleitor
// Only the 11026 path shows up in the recorded profiles (edge 7840 -> 11026 -> 3903).
// On the urna the same messages come from the operator (mesário) thread through the Add path inlined in
// its states (see vota/operador/confirmaidentidade/cinformaeleitorpodevotar.u33.cpp).
template <typename TMessage>
void CPriorityMessageQueue<TMessage>::Envia(std::int16_t id)
{
    Envia(TMessage{id, this});                   // func 7708, a separate one-line function (only caller: 3903):
                                                 // Envia(const TMessage& m) { Add(m, 1); } - not declared in
                                                 // u18's cmessagequeue.h, which only has Add(mensagem, prioridade)
}

} // namespace api
