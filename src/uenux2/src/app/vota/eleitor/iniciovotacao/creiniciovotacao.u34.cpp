// FRAGMENT reconstructed by unit u34 from vota_web_wasm.wasm.
// Original file: uenux2/src/app/vota/eleitor/iniciovotacao/creiniciovotacao.cpp  (path inferred)
// The rest of the class (GetInst 5938, constructor, StartState 11835) is in creiniciovotacao.u07.cpp.
// Class declaration: estadosiniciovotacao.u34.h.
//
// WEB BUILD: never reached (the simulator starts the voter thread in CAguardaMensagem).
#include "vota/eleitor/cdefinerotaposreinicio.h"
#include "vota/eleitor/iniciovotacao/estadosiniciovotacao.u34.h"
#include "vota/log/clogvota.h"

namespace vota {

// wasm func 11834 - vtable slot 7 (ProcessInput). srcloc cinteractiveform.h:57 @1546684 (IInputKbd, func 455).
void CReinicioVotacao::ProcessInput()
{
    switch (m_tela->Read()) {
    case api::EInputResult::CONFIRMA:                                           // 9
        // IEventosLog::Loga inlined: api::CLoga::loga(m_aplicativo, 1, texto) (func 433)
        CLogVota::GetInst().Loga("Mesário confirmou o reinício da votação");  // @123235 (39 chars)
        m_proximoEstado = &CDefineRotaPosReinicio::GetInst();                   // func 5943: resume where it stopped
        break;
    case api::EInputResult::BRANCO:                                             // 3
        CLogVota::GetInst().Loga("Mesário selecionou outras opções");         // @72568 (32 chars)
        m_proximoEstado = &CMaisInformacoes::GetInst(this);                     // func 1280: CORRIGE there comes back here
        break;
    default:
        break;
    }
}

}  // namespace vota
