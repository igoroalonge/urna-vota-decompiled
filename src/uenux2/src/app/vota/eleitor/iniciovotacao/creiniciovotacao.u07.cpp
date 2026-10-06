// FRAGMENT reconstructed by unit u07 from vota_web_wasm.wasm.
// Original file (path inferred): uenux2/src/app/vota/eleitor/iniciovotacao/creiniciovotacao.cpp
// Class vota::CReinicioVotacao (RTTI typeinfo @1546672, vtable @1546636; other methods in unit u34).
// "Reinício da votação": the state entered when VOTA restarts (power loss, reboot) during election day.
//
// vtable: [0] ~dtor (icf 244: releases m_tela)  [1] deleting dtor (387)  [2] StartState (11835)
//         [3] NeedChangeState  [4] GetNextState  [5] FinishState (nop)  [6] ProcessMessage (nop)
//         [7] ProcessInput (11834: "Mesário confirmou o reinício da votação" / "Mesário selecionou outras opções")
//         [8] ProcessTick (nop)
// layout (20 bytes): comum::CAppState {+0 vptr, +4 m_proximoEstado, +8/+9/+10 flags} ; +12 CFormInterativoTelaVota m_tela
#include "vota/eleitor/iniciovotacao/creiniciovotacao.h"

#include <mutex>

#include "api/util/csystem.h"
#include "vota/eleitor/comum/ctelasvota.h"
#include "vota/eleitor/iniciovotacao/cconfirmaregerarzeresima.h"
#include "vota/eleitor/iniciovotacao/cquerimprimirzeresima.h"
#include "vota/eleitor/iniciovotacao/cquerreimprimirzeresima.h"
#include "vota/log/clogvota.h"
#include "comum/appinfo/cappinfo.h"
#include "comum/cpath.h"
#include "comum/dados/cjustificador.h"
#include "comum/dados/crdvvota.h"

namespace vota {

static std::unique_ptr<CReinicioVotacao> s_pInstancia;   // @1834968
static std::mutex s_mutexInstancia;                        // @1834944

// wasm func 5938 (called by testeteclado::CRetomada slots 11 and 12)                 // name inferred
CReinicioVotacao& CReinicioVotacao::GetInst()
{
    std::lock_guard<std::mutex> lock(s_mutexInstancia);
    if (!s_pInstancia)
        s_pInstancia.reset(new CReinicioVotacao());
    return *s_pInstancia;
}

// constructor (inlined into 5938)
CReinicioVotacao::CReinicioVotacao()
    : comum::CAppState(2 /* keyboard */),                                        // func 224
      m_tela(CTelasVota::GetInst().m_telaReinicioVotacao)                        // CTelasVota +148
{
}

// wasm func 11835 (vtable slot 2)                                                    // name inferred (slot)
void CReinicioVotacao::StartState()
{
    m_proximoEstado = this;

    // Votes or justifications already recorded: ask the mesário to confirm the restart.
    auto& rdv = comum::CRdvVota::GetInst();                                      // func 555
    auto& justificador = comum::CJustificador::GetInst();                        // func 1391
    if (rdv.ComparecimentoMaximo() != 0 || justificador.GetQuantidade() != 0) {  // func 1269 (max over eleições); +8 ?
        CLogVota::GetInst().Loga(api::INFO, "Apresentada tela do reinício da votação");
        m_tela->Exibe();                                                          // form slot 2
        return;
    }

    // Nothing recorded yet: go back to the zerésima questions.
    if (comum::EhTreinamentoEleitor()) {                                          // func 697
        // voter-training mode: "ze.dat" marks a zerésima already generated
        const bool zeresimaGerada =
            api::CSystem::IsRegularFile(comum::CPath::GetPathTrab(comum::MI) / "ze.dat");   // func 412 (S_IFREG test)
        m_proximoEstado = zeresimaGerada ? static_cast<comum::CAppState*>(&CQuerReimprimirZeresima::GetInst())   // func 5940
                                         : static_cast<comum::CAppState*>(&CQuerImprimirZeresima::GetInst());    // func 5957
        return;
    }

    const auto idUrna = comum::GetEstadoGeral(comum::CAppInfo::GetInst()).GetIdUrna();      // CEstadoGeral +60
    const auto& vota = comum::GetEstadoVota(comum::CAppInfo::GetInst());                     // CEstadoGeralVota (turno atual)
    if (vota.urnaIdGerouZeresima.has_value() && *vota.urnaIdGerouZeresima != idUrna)          // +16 engaged, +12 value
        m_proximoEstado = &CConfirmaRegerarZeresima::GetInst();   // inlined singleton @1834940: CAppState(2), tela = CTelasVota +84
    else
        m_proximoEstado = &CQuerReimprimirZeresima::GetInst();
}

} // namespace vota
