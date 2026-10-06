// Reconstructed from vota_web_wasm.wasm (unit u39).
// Original (path inferred): uenux2/src/app/vota/eleitor/csincronismoeleitor.cpp
// ProcessMessage (7181): src/uenux2/src/app/vota/eleitor/u19-foreign-fragments.cpp.
#include "vota/eleitor/csincronismoeleitor.h"

#include "api/ipc/cmessagequeue.h"
#include "vota/eleitor/comum/ctelasvota.h"
#include "vota/operador/cthreadoperador.h"

namespace vota {

// wasm func 7178 - vtable slot 2. Observed executing at the end of every recorded vote.
// In the web build the message below is never consumed: after one municipal vote the operator queue holds
// 6, 9, 9, 13, 1 (u10 §2); the page reloads for every voter, so the backlog stays small.
void CSincronismoEleitor::StartState()
{
    auto& fila = CThreadOperador::GetInst().GetFila();                 // func 270, +36 (vota::CMessageOperador)
    fila.Add(api::SMessage{13, &fila}, 1);                             // rhvoice_f501: EMensagemOperadorRecebida::SincronizaVoto
    m_proximoEstado = this;

    auto& telas = CTelasVota::GetInst();                               // func 407
    telas.m_barraProgresso->SetValor(telas.m_barraProgresso->GetMinimo());   // +180; func 3667 with CProgressBar +28
    telas.m_telaProgressoRegistroVoto->Show();                         // +172 "telaProgressoRegistroVoto" ("Gravando")
}

}  // namespace vota
