// Reconstructed from vota_web_wasm.wasm (unit u39).
// Original (path inferred): uenux2/src/app/vota/operador/caguardainicio.cpp
// Constructor and GetInst (func 5343): src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp.
//
// WEB BUILD: dead code (operator thread not stepped by votaTick).
#include "vota/operador/caguardainicio.h"

#include "comum/comparecimentomesario/estados/estadosregistromesarios.h"   // comum::CRegistrarMesarios
#include "vota/log/clogvota.h"
#include "vota/operador/cthreadoperador.h"
#include "vota/operador/leidentidade/cpedeidentidade.h"
#include "vota/operador/outrasopcoes/cfimaquisicaovotos.h"

namespace vota {

// Messages posted by the voter thread that this state reacts to (values attested, names inferred).
enum EMensagemAguardaInicio : uebyte {
    MSG_REGISTRAR_MESARIOS  = 7,    // CDefineRotaPreVotacao::NeedChangeState (11994), CReinicioComparecimentoMesario
    MSG_VOTACAO_INICIADA    = 8,    // CInicioVotacao (func 5972)
    MSG_FIM_AQUISICAO       = 10,   // CFinalizaAquisicao::StartState (12113), restart during the closing
};

// ---------------------------------------------------------------------------------------------------------
// wasm func 10220 - vtable slot 2
void CAguardaInicio::StartState()
{
    m_proximoEstado = this;
    CThreadOperador::GetInst().StartTick(m_tick);          // func 270 -> api::CTickManager::StartTick (700)
    m_form->Show();                                        // IForm slot 2
}

// ---------------------------------------------------------------------------------------------------------
// wasm func 10219 - vtable slot 5
void CAguardaInicio::FinishState()
{
    CThreadOperador::GetInst().StopTick(m_tick);           // func 422 (CThreadVota::StopTick)
}

// ---------------------------------------------------------------------------------------------------------
// wasm func 10217 - vtable slot 6. br_table on (mensagem - 7): {7, 8, 9 (ignored), 10}.
void CAguardaInicio::ProcessMessage(uebyte mensagem)
{
    switch (mensagem) {
    case MSG_REGISTRAR_MESARIOS:
        m_proximoEstado = &comum::CRegistrarMesarios::GetInst();          // comum_f3610 (ctor inlined there)
        break;
    case MSG_VOTACAO_INICIADA:
        CLogVota::GetInst().Loga("Urna pronta para receber votos");      // api_f233 (severity 1)
        m_proximoEstado = &CPedeIdentidade::GetInst();                    // func 652 (misnamed CTextSource ctor)
        break;
    case MSG_FIM_AQUISICAO:
        m_proximoEstado = &CFimAquisicaoVotos::GetInst();                 // func 5428
        break;
    default:                                                              // 9 and anything else: stay
        break;
    }
}

// ---------------------------------------------------------------------------------------------------------
// wasm func 10218 - vtable slot 8. Every 500 ms the form is shown again, which redraws the date/time
// fields (CDataTextFmt over TextoData/TextoHora).
void CAguardaInicio::ProcessTick(uebyte tick)
{
    if (tick == m_tick)
        m_form->Show();
}

}  // namespace vota
