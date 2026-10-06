// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/iniciovotacao/cquerreimprimirzeresima.cpp
// (GetInst, func 5940, is in the u07 fragment cquerreimprimirzeresima.u07.cpp)
//
// srcloc evidence:
//   :45  StartState  "Erro na verificação do comparecimento"   (CUeVotaError 9375)
//   api/gui/cinteractiveform.h:57  Read(), inlined into ProcessInput

#include "vota/eleitor/iniciovotacao/cquerreimprimirzeresima.h"

#include <memory>
#include <source_location>

#include "comum/dados/crdvvota.h"
#include "comum/informacao/cinformacaoeleicao.h"
#include "vota/comum/crelvotautil.h"
#include "vota/comum/votadefs.h"
#include "vota/eleitor/iniciovotacao/ciniciovotacao.h"
#include "vota/eleitor/iniciovotacao/creimprimindoresumozeresima.h"
#include "vota/log/clogvota.h"

namespace vota {

// wasm func 11849 — vtable slot 2
void CQuerReimprimirZeresima::StartState()
{
    if (comum::CRdvVota::GetInst().Comparecimento() != 0) {                  // shared_f1269
        CLogVota::GetInst().LogaErro("Erro na verificação do comparecimento");   // func 2282 (level 3)
        throw CUeVotaError(9375, "Erro na verificação do comparecimento",
                           std::source_location::current());                   // :45
    }
    CLogVota::GetInst().LogaMesarioIndagadoImprimirZeresima();                // func 5882
    m_tela->Exibe();
    m_proximoEstado = this;
}

// wasm func 11848 — vtable slot 7 (analyzer name vota::CQuerReimprimirZeresima::vf7)
void CQuerReimprimirZeresima::ProcessInput()
{
    auto& campo = *m_tela->GetInputs().at(0);                                 // throws out_of_range if none
    switch (m_tela->Read()) {                                                 // cinteractiveform.h:57
    case api::EInputResult::Branco:                                           // 3
        m_proximoEstado = &CMaisInformacoes::GetInst();                       // api_f1280 (24 bytes, CAppState(2))
        break;

    case api::EInputResult::Corrige:                                          // 5
        if (!campo.GetTexto().empty()) {
            campo.Limpa();                                                    // input slot 11
        } else if (comum::DeveRegistrarMesarios()) {                          // func 2520: IdentificaMesarios()
            m_proximoEstado = &CReinicioComparecimentoMesario::GetInst();     //   && !EhTreinamentoEleitor()
        } else {                                                              //   (vota_f3864)
            m_proximoEstado = &CInicioVotacao::GetInst();                     // vota_f2880
        }
        break;

    case api::EInputResult::Confirma: {                                       // 9
        const std::string& texto = campo.GetTexto();                          // input +24
        if (texto == "1") {
            CRelVotaUtil::CortaPapel();                                       // func 2882
            m_proximoEstado = &CReimprimindoZeresima::GetInst();              // func 5941
        } else if (texto == "2") {
            CRelVotaUtil::CortaPapel();
            m_proximoEstado = &CReimprimindoResumoZeresima::GetInst();        // api_f5942
        } else {
            campo.Limpa();                                                    // invalid option: clear
        }
        break;
    }

    default:
        break;
    }
}

// wasm func 5941 — CReimprimindoZeresima::GetInst(): vota_f764(@1834804, @1834828, vtable @1546276,
// flags 0) (merged lazy-singleton body, see unit u06). Also returned by CRegeraResumoZeresima's slot 10.
CReimprimindoZeresima& CReimprimindoZeresima::GetInst()
{
    return ObtemInstancia<CReimprimindoZeresima>(s_mutexReimprimindo, s_pReimprimindo, /*flags*/ 0);
}

}  // namespace vota
