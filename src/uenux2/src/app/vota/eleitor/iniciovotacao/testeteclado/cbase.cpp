// Reconstructed from vota_web_wasm.wasm (unit u39).
// Original (path inferred): uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cbase.cpp
//
// WEB BUILD: unreachable (the simulator starts in estadoVota VOTAR; no zerésima, no restart).
#include "vota/eleitor/iniciovotacao/testeteclado/cbase.h"

#include "vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.h"
#include "vota/log/clogvota.h"

namespace vota::testeteclado {

// wasm func 1559 - vtable slot 0 of CBase, CPreZeresima and CRetomada (their members are trivially
// destructible). Stores the CBase vptr and releases m_tela; CEstadoComDesligamentoAutomatico has nothing
// to destroy. Direct callers: the deleting destructor (slot 1) of CPreZeresima/CRetomada (ICF 5946), the
// exit-time resets of the two singletons (11833 @1834996, 11864 @1834744; table-only stubs) and the
// replacements of the CRetomada/CPreZeresima instance in CAjusteInicial (7160) and CVerificaEleicaoPassou (11914).
CBase::~CBase() = default;

// wasm func 11875 - vtable slot 2
void CBase::StartState()
{
    m_proximoEstado = this;
    m_tela = CriaTela();                    // slot 10 (sret), move-assigned: the previous screen is released
    m_tela->Show();
}

// wasm func 11874 - vtable slot 7. m_tela->Read() is CInteractiveForm<IScreen, IInputKbd>::Read() inlined
// (srcloc cinteractiveform.h:57: IInputKbd lookup, current input field m_campos.at(m_foco), field slot 10).
void CBase::ProcessInput()
{
    switch (m_tela->Read()) {
    case api::EInputResult::CORRIGE:                                    // 5: "Não testar"
        m_proximoEstado = GetEstadoSemTeste();                          // slot 12
        if (GetNextState() != this)                                     // slot 4
            CLogVota::GetInst().Loga("Teste de Teclado do TE não executado");   // api_f233
        break;
    case api::EInputResult::CONFIRMA: {                                 // 9: "Testar"
        comum::CAppState* const aposTeste = GetEstadoPassouNoTeste();   // slot 11 (called first)
        CTesteTeclado::GetInst().SetEstadoAposTeste(aposTeste);         // func 3855, CTesteTeclado +64
        m_proximoEstado = &CTesteTeclado::GetInst();
        break;
    }
    default:
        break;
    }
}

}  // namespace vota::testeteclado
