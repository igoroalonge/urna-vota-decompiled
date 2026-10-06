// Reconstructed from vota_web_wasm.wasm (unit u39).
// Original (path inferred): uenux2/src/app/vota/operador/leidentidade/celeitornaoencontrado.cpp
//
// vota::CEleitorNaoEncontrado : vota::IEleitorImpedidoVotar (typeinfo @1588496, vtable @1588456): "the typed
// identity is not in this section's roll" ("NÃO CADASTRADO nesta urna", or "CPF não encontrado. Digite o
// Título." / "CONFIRMA: retornar"). Unit u27 declares the class with the macro VOTA_ELEITOR_IMPEDIDO in
// ieleitorimpedidovotar.h and defines its constructor/GetInst (func 5424) in ieleitorimpedidovotar.cpp;
// the only method of its own is the override of IEleitorImpedidoVotar slot 9, below.
//
// Slot 9 of IEleitorImpedidoVotar is a hook called by IEleitorImpedidoVotar::StartState (func 10625) right
// after m_form->Show() (and before the voter photo lookup). The base version is a no-op (ICF 218
// icf_nop_vf0, as in the five sibling classes); only CEleitorNaoEncontrado overrides it, to write a log record. u27 named the slot "AoRetornar"; given where it
// is called, "LogaEntrada" describes it better.                                       name inferred
//
// WEB BUILD: dead code (operator thread not run).
#include "vota/log/clogvota.h"
#include "vota/operador/leidentidade/ieleitorimpedidovotar.h"

namespace vota {

// wasm func 10663 - vtable slot 9 (declare as `void LogaEntrada() override;` / u27's `AoRetornar`)
void CEleitorNaoEncontrado::LogaEntrada()
{
    CLogVota::GetInst().Loga("Eleitor não encontrado para o identificador informado");   // api_f233 (severity 1)
}

}  // namespace vota
