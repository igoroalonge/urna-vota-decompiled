// Reconstructed from vota_web_wasm.wasm (unit u39).
// Original (path inferred): uenux2/src/app/vota/operador/outrasopcoes/cfimaquisicaovotos.cpp
//
// WEB BUILD: dead code (operator thread not run).
#include "vota/operador/outrasopcoes/cfimaquisicaovotos.h"

#include "api/ipc/cmessagequeue.h"
#include "comum/appinfo/cappinfo.h"                                        // comum::EhModoDemonstracaoSemTreinamentoEleitor (func 2520)
#include "comum/comparecimentomesario/estados/estadosregistromesarios.h"   // comum::CRegistrarMesarios
#include "vota/eleitor/cthreadeleitor.h"
#include "vota/operador/cfinalizaoperador.h"

namespace vota {

namespace {
// Voter-thread message 7: CAguardaMensagem::ProcessMessage(7) (func 7481) sets estadoVota = GERARBU and
// starts the chain CGeraBU -> CGeraRelatorios -> CInicioBU -> ... (u08/u09).           name inferred
constexpr std::int16_t MSG_ELEITOR_GERAR_BU = 7;
}  // namespace

// wasm func 10737 - vtable slot 2
void CFimAquisicaoVotos::StartState()
{
    // func 2520 (analysis DB: comum_f2520) = func 1950 && !(fase treinamento && EstadoGeralVota.treinamentoEleitor):
    // the election identifies mesários (cfg +400, outside demo mode; func 1950 carries the srcloc of the inlined
    // CInformacaoEleicao::EhModoDemonstracao and is misnamed after it) and this is not a "treinamento de eleitor"
    // urna. Declared by cappinfo.h (u09 reconstruction) as EhModoDemonstracaoSemTreinamentoEleitor, a misleading
    // name for what it computes; u09/u20 comments call it DeveRegistrarMesarios.
    if (comum::EhModoDemonstracaoSemTreinamentoEleitor()) {
        // Registration of the mesários present at the closing (period FINAL). When it ends,
        // CControladorRegistraMesariosVota slot 15 posts the same message 7 and slot 12 returns
        // CFinalizaOperador (see ccontroladorregistramesariosvota.cpp).
        m_proximoEstado = &comum::CRegistrarMesarios::GetInst();                // comum_f3610
        return;
    }

    auto& fila = CThreadEleitor::GetInst().GetFila();                           // func 316, +36
    fila.Add(api::SMessage{MSG_ELEITOR_GERAR_BU, &fila}, 1);                    // rhvoice_f501 (priority 1)
    m_proximoEstado = &CFinalizaOperador::GetInst();                            // func 5342 (vota_f764, flags 0)
}

}  // namespace vota
