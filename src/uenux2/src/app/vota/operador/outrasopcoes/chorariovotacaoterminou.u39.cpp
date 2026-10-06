// FRAGMENT reconstructed by unit u39 from vota_web_wasm.wasm.
// Original file (path inferred by u27): uenux2/src/app/vota/operador/outrasopcoes/chorariovotacaoterminou.cpp
// Class declaration: vota/operador/estadosoperador.u34.h. Constructor + GetInst (func 5430):
// chorariovotacaoterminou.u34.cpp. ProcessInput (10757, CORRIGE -> CPedeIdentidade): u27-foreign-fragments.cpp.
//
// "Horario de votacao terminou! / Favor encerrar a urna! / CORRIGE": the voting period is over and voting
// was blocked by the clock (IInformacaoThreadOperador::VotacaoBloqueadaPorHorario); the mesário may only
// close the urna.
// RTTI: comum::CAppState <- vota::CHorarioVotacaoTerminou (typeinfo @1586868, vtable @1586832)
//   [0] ICF 244  [1] ICF 387  [2] StartState 10758  [7] ProcessInput 10757
//
// WEB BUILD: dead code (operator thread not run).
#include "vota/log/clogvota.h"
#include "vota/operador/estadosoperador.u34.h"

namespace vota {

// wasm func 10758 - vtable slot 2
void CHorarioVotacaoTerminou::StartState()
{
    CLogVota::GetInst().Loga(api::ESeveridade{1}, "Horário de votação terminou");   // CLoga::loga inline, severity 1
    m_form->Show();
    m_proximoEstado = this;
}

}  // namespace vota
