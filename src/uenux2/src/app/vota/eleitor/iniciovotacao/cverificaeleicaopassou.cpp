// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/iniciovotacao/cverificaeleicaopassou.cpp (+ .h, declared here)
//
// "Verifica se a eleição passou" = before the zerésima, refuse to run an election whose date is over
// (except in the training phase): the urna shows "data da eleição inválida" and stays there.
// Otherwise the keyboard test that precedes the zerésima starts (testeteclado::CPreZeresima).
//
// srcloc evidence:
//   :56  StartState  api::IBeep lookup
//   :70  StartState  "Estado nao esperado"   (CUeVotaError 9376)
//
// RTTI: comum::CAppState <- vota::CVerificaEleicaoPassou (typeinfo @1545044, vtable @1544976)
//   [0] 174 [1] 144 [2] StartState 11914 [3..8] defaults

#include <memory>
#include <mutex>
#include <source_location>

#include "api/hwil/ibeep.h"
#include "api/ipc/cmessagequeue.h"
#include "api/util/cdatetime.h"
#include "comum/cappinfo.h"
#include "comum/cappstate.h"
#include "comum/dados/cconfiguracaoeleicao.h"
#include "vota/comum/votadefs.h"
#include "vota/eleitor/comum/ctelasvota.h"
#include "vota/eleitor/iniciovotacao/testeteclado/cprezeresima.h"
#include "vota/log/clogvota.h"

namespace vota {

using comum::md::estadoaplicacao::EEstadoVota;

class CVerificaEleicaoPassou final : public comum::CAppState {
public:
    static CVerificaEleicaoPassou& GetInst();   // not in this unit; 12 bytes
    void StartState() override;                 // wasm func 11914 (srcloc 56, 70)
};

// wasm func 11914 — vtable slot 2
void CVerificaEleicaoPassou::StartState()
{
    m_proximoEstado = this;
    auto& appInfo = comum::CAppInfo::GetInst();

    if (appInfo.GetEstadoGeral().GetFase() != comum::EUrnaFase::Treinamento) {      // +48 != '3'
        const api::CDateTime limite = comum::CConfiguracaoEleicao::GetInst().GetDataLimiteEleicao();  // cfg +580..+591  name inferred
        const api::CDateTime agora;                                                  // api_f479
        if (agora.Compare(limite) >= 0) {                                           // func 759
            CLogVota::GetInst().Loga(2, "Data da eleição inválida");                 // CLoga::loga level 2
            CTelasVota::GetInst().m_telaDataEleicaoInvalida->Exibe();                // +76/+80  name inferred
            // tells the other thread (message 0 = stop, priority 100) through the queue @270 (+36)
            api::CPriorityMessageQueue<api::SMessage>::GetInst().Envia(api::SMessage{0}, 100);   // 270 + 501
            api::IBeep::GetInst().BeepErro();                                        // :56, IBeep slot 3  name inferred
            return;                                                                  // stays here forever
        }
    }

    const auto estado = appInfo.GetVota().GetEstadoVota();
    if (estado != EEstadoVota::EAVINICIAL && estado != EEstadoVota::EAVGERADADOSDINAMICOS &&
        estado != EEstadoVota::EAVAGUARDAHORAZERESIMA) {                             // not in 49..51
        CLogVota::GetInst().LogaEstadoNaoEsperado();   // func 5881 "Erro estado do aplicativo não esperado"
        throw CUeVotaError(9376, "Estado nao esperado", std::source_location::current());   // :70
    }
    // lazily created singleton (@1834744, mutex @1834720): vota_f5948 (testeteclado::CBase ctor),
    // vtable CPreZeresima, api_f1000(+36, 0)
    m_proximoEstado = &testeteclado::CPreZeresima::GetInst();
}

}  // namespace vota
