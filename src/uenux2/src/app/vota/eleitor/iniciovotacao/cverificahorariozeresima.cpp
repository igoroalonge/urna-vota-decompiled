// uenux2/src/app/vota/eleitor/iniciovotacao/cverificahorariozeresima.cpp
// Reconstructed from vota_web_wasm.wasm (unit u26). std::source_location records: :64 StartState,
// :80 ProcessTickNaoDesligamento (both the UE_ASSERT below).
//
// Web build: not reached (the web page starts the voter terminal in CAguardaMensagem).
#include "vota/eleitor/iniciovotacao/cverificahorariozeresima.h"

#include <format>

#include "comum/appinfo/cappinfo.h"
#include "comum/dados/cconfiguracaoeleicao.h"
#include "comum/informacao/cinformacaoeleicao.h"
#include "comum/relatorios/crelatoriotesteimpressora.h"
#include "vota/eleitor/cthreadeleitor.h"
#include "vota/eleitor/iniciovotacao/ciniciozeresima.h"     // CInicioZeresima::GetInst (func 3865, path ?)
#include "vota/eleitor/iniciovotacao/cmaisinformacoes.h"    // CMaisInformacoes::GetInst (func 1280, path ?)
#include "vota/log/clogvota.h"

namespace vota {

using comum::md::estadoaplicacao::EEstadoVota;

// wasm func 11871 (vtable slot 2)
void CVerificaHorarioZeresima::StartState()
{
    auto& appinfo = comum::CAppInfo::GetInst();
    const api::CDateTime agora;                                                // func 479 (system clock)
    if (agora.Compare(m_horarioZeresima) < 0) {                                // func 759
        CThreadEleitor::GetInst().StartTick(m_tick);                           // api::CTickManager::StartTick
        CLogVota::GetInst().Loga("Aguardando data e hora para emissão da zerésima");   // CLoga::loga(app, 1, ...)
        m_tela->Exibe();                                                       // form slot 2
        m_proximoEstado = this;
    } else {
        UE_ASSERT(appinfo.GetVota().GetEstadoVota() == EEstadoVota::EAVAGUARDAHORAZERESIMA);   // :64 (3481)
        m_proximoEstado = &CInicioZeresima::GetInst();                         // func 3865
    }
}

// wasm func 11868 (vtable slot 5)
void CVerificaHorarioZeresima::FinishState()
{
    CThreadEleitor::GetInst().StopTick(m_tick);                                // func 422
}

// wasm func 11869 (vtable slot 7). The keys offered by the waiting screen.
void CVerificaHorarioZeresima::ProcessInput()
{
    switch (m_tela->Read()) {                               // CInteractiveForm::Read inlined (cinteractiveform.h:57)
    case api::EInputResult::Branco:                         // 3: "Mais informações"
        m_proximoEstado = &CMaisInformacoes::GetInst(this);                    // func 1280 (returns here)
        break;

    case api::EInputResult::Confirma: {                     // 9: print the "estado da urna" report
        const comum::CInformacaoEleicao informacao(comum::CConfiguracaoEleicao::GetInst());   // vota_f603 (+88)
        auto& vota = comum::CAppInfo::GetInst().GetVota(comum::EUrnaTurno::Atual);
        if (informacao.GetNumRelatorioEstado() > vota.GetNumViasImpressas().estadoUrna) {   // func 2865 vs +73
            comum::CRelatorioTesteImpressora relatorio;                                        // func 5586
            CLogVota::GetInst().LogaImprimindoRelatorioEstadoUrna();                           // func 5880
            relatorio.Imprime("estado da urna",
                              std::format("{}ª via", vota.GetNumViasImpressas().estadoUrna + 1));   // func 5591
            ++vota.GetNumViasImpressas().estadoUrna;
            comum::SalvaEstado();                                                              // func 491
        }
        m_tela = CTelasVota::CriaTelaAntesHorarioZeresima();                                   // func 6595
        m_tela->Exibe();
        break;
    }

    default:
        break;
    }
}

// wasm func 11870 (vtable slot 9, srcloc :80). Every 2 s while waiting.
void CVerificaHorarioZeresima::ProcessTickNaoDesligamento(uebyte tick)
{
    if (tick != m_tick)
        return;
    const api::CDateTime agora;
    if (agora.Compare(m_horarioZeresima) < 0)
        return;
    auto& appinfo = comum::CAppInfo::GetInst();
    UE_ASSERT(appinfo.GetVota().GetEstadoVota() == EEstadoVota::EAVAGUARDAHORAZERESIMA);   // :80 (3482)
    m_proximoEstado = &CInicioZeresima::GetInst();
}

} // namespace vota
