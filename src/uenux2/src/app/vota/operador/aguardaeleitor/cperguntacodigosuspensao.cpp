// Reconstructed from vota_web_wasm.wasm (unit u39).
// Original (path inferred by u17): uenux2/src/app/vota/operador/aguardaeleitor/cperguntacodigosuspensao.cpp
// Constructor/GetInst: operador/u17-foreign-fragments.cpp. ProcessInput (10420): operador/u19-foreign-fragments.cpp
// (on a valid título: LogaAviso("Título {} é válido para suspender a votação"), message 2 to the voter
// thread, m_suspensaoEnviada = true; on an invalid one: m_formInvalido->Show(), CSystem::Sleep(3000)
// = emscripten_sleep(3000) behind the flag @1584624, m_form->Show()).
//
// WEB BUILD: dead code (operator thread not run).
#include "vota/operador/aguardaeleitor/cperguntacodigosuspensao.h"

#include "vota/log/clogvota.h"
#include "vota/operador/aguardaeleitor/celeitorvotounaovotou.h"
#include "vota/operador/aguardaeleitor/cmostraeleitorvotando.h"
#include "vota/operador/aguardaeleitor/csincronismooperador.h"

namespace vota {

// ---------------------------------------------------------------------------------------------------------
// wasm func 10421 - vtable slot 2
void CPerguntaCodigoSuspensao::StartState()
{
    m_proximoEstado = this;
    CLogVota::GetInst().Loga("Solicitado título eleitoral para suspensão do eleitor");   // api_f233 (severity 1)
    m_form->Show();                                                                      // +12, IForm slot 2
    m_suspensaoEnviada = false;                                                       // +28
}

// ---------------------------------------------------------------------------------------------------------
// wasm func 10419 - vtable slot 6. br_table on (mensagem - 2): only 2, 5 and 13 change the state.
// Message names: EMensagemOperadorRecebida (cmostraeleitorvotando.h).
void CPerguntaCodigoSuspensao::ProcessMessage(uebyte mensagem)
{
    switch (static_cast<EMensagemOperadorRecebida>(mensagem)) {
    case EMensagemOperadorRecebida::EleitorNaoVotou: {           // 2: the voter's session ended without a vote
        m_suspensaoEnviada = false;
        auto& tela = CEleitorVotouNaoVotou::GetInst();            // func 1901
        tela.m_votou = false;                                     // +11
        m_proximoEstado = &tela;
        break;
    }
    case EMensagemOperadorRecebida::EleitorVoltouADigitar:       // 5: the voter resumed - drop the question
        m_proximoEstado = &CMostraEleitorVotando::GetInst();      // func 1150
        break;
    case EMensagemOperadorRecebida::SincronizaVoto: {            // 13: the voter confirmed the last cargo meanwhile
        m_suspensaoEnviada = false;
        auto& sincronismo = CSincronismoOperador::GetInst();      // func 1897
        sincronismo.m_suspensaoAutomatica = true;                 // +11: after "FIM" show "votou" (CEleitorVotouNaoVotou)
        m_proximoEstado = &sincronismo;
        break;
    }
    default:
        break;
    }
}

}  // namespace vota
