// FRAGMENT reconstructed by unit u39 from vota_web_wasm.wasm.
// Original file (path inferred by u27): uenux2/src/app/vota/operador/outrasopcoes/cperguntafilaeleitorvazia.cpp
// Class declaration: vota/operador/estadosoperador.u34.h. Constructor/GetInst (5426): u27-foreign-fragments.cpp.
// ProcessInput (10713): cperguntafilaeleitorvazia.u34.cpp (CORRIGE "não" -> CAguardaEleitoresVotarem,
// CONFIRMA "sim" -> CPedeTituloEncerramento).
//
// First question of the ENCERRAMENTO on the MT: "Todas as pessoas presentes já votaram?" - the law lets
// every voter who is in the line at the closing time vote. Reached from CIniciaFinalizacao (10706).
// RTTI: comum::CAppState <- vota::CPerguntaFilaEleitorVazia (typeinfo @1587736, vtable @1587700)
//   [2] StartState 10715  [7] ProcessInput 10713
//
// WEB BUILD: dead code (operator thread not run).
#include "vota/log/clogvota.h"
#include "vota/operador/estadosoperador.u34.h"

namespace vota {

// wasm func 10715 - vtable slot 2
void CPerguntaFilaEleitorVazia::StartState()
{
    m_proximoEstado = this;
    m_form->Show();
    CLogVota::GetInst().Loga("Operador indagado se todas as pessoas presentes votaram");   // api_f233
}

}  // namespace vota
