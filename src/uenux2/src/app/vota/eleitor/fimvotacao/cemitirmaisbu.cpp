// Reconstructed from vota_web_wasm.wasm (unit u08).
// Original: uenux2/src/app/vota/eleitor/fimvotacao/cemitirmaisbu.cpp
//
// srcloc evidence:
//   :46  virtual void StartState()   Assert (appInfo.GetVota().GetEstadoVota() == EAVENCERRADA)          (3452)
//   :47  virtual void StartState()   Assert (appInfo.GetVota().GetEstadoEncerramento() == EAEFIMDOSTRABALHOS) (3453)
//   :64  virtual void ProcessInput() "Era esperado pelo menos um input"                               (9371)
//   api/gui/cinteractiveform.h:57   CInteractiveForm<IScreen, IInputKbd>::Read(), inlined
//
// None of these functions ran in the recorded sessions: the web simulator only drives the voter side.

#include "vota/eleitor/fimvotacao/cemitirmaisbu.h"

#include <format>
#include <source_location>

#include "api/util/cstringutils.h"                       // ecourna::api::util::CStringUtils::ToByte
#include "comum/cappinfo.h"
#include "vota/eleitor/comum/ctelasvota.h"
#include "vota/eleitor/fimvotacao/cimprimindobu.h"       // CImprimindoBU::ImprimeBU
#include "vota/eleitor/fimvotacao/cmostraqrcodebu.h"
#include "vota/eleitor/fimvotacao/cverificaqtdbusadicionais.h"   // GetQuantidadeMaximaBUsAdicionais
#include "vota/log/clogvota.h"

namespace vota {

using comum::md::estadoaplicacao::EEstadoVota;          // ASN.1 EstadoVota + '1'
using comum::md::estadoaplicacao::EEstadoEncerramento;  // ASN.1 EstadoEncerramento + '1'

// wasm func 12082 — vtable slot 2
void CEmitirMaisBU::StartState()
{
    auto& appInfo = comum::CAppInfo::GetInst();                                            // func 185
    UE_ASSERT(appInfo.GetVota().GetEstadoVota() == EEstadoVota::EAVENCERRADA);            // :46  64
    UE_ASSERT(appInfo.GetVota().GetEstadoEncerramento() == EEstadoEncerramento::EAEFIMDOSTRABALHOS); // :47  52
    // UE_ASSERT throws CBaseError<api::EUeAssertError>(code, "Assert (<expr>)", srcloc) (func 463)

    CLogVota::GetInst().LogaMesarioIndagadoQtdVias();   // func 4563 "Mesário indagado sobre quantidade de vias adicionais"
    CTelasVota::GetInst().m_telaEmitirMaisBU->Exibe();  // CTelasVota +196 ("telaEmitirMaisBU"), form slot 2
    m_proximoEstado = this;
}

// wasm func 12081 — vtable slot 7
void CEmitirMaisBU::ProcessInput()
{
    const auto tela = CTelasVota::GetInst().m_telaEmitirMaisBU;           // shared_ptr copy

    switch (tela->Read()) {                                                // cinteractiveform.h:57
    case api::EInputResult::Corrige:                                       // 5
        CLogVota::GetInst().LogaMesarioNaoSolicitouVias();                 // func 4557
        m_proximoEstado = &CMostraQRCodeBU::GetInst();                     // func 2888
        break;

    case api::EInputResult::Confirma: {                                    // 9
        const comum::TQtdImpresso maximo = GetQuantidadeMaximaBUsAdicionais();   // func 4582
        const auto& campos = tela->GetInputs();
        if (campos.empty())
            throw CUeVotaError(9371, "Era esperado pelo menos um input", std::source_location::current()); // :64

        const std::string texto = campos.front()->GetTexto();
        if (texto.empty()) {
            // nothing typed: log and stay on the question
            CLogVota::GetInst().LogaMesarioNaoSolicitouVias();
            break;
        }

        const uebyte quantidade = ecourna::api::util::CStringUtils::ToByte(std::string(texto.c_str()));
        CLogVota::GetInst().Loga(std::format("Solicitada a emissão de [{}] ", quantidade) +
                                 (quantidade == 1 ? "via adicional" : "vias adicionais"));

        if (quantidade > maximo) {
            CLogVota::GetInst().LogaQtdViasExcedeMaximo();   // func 4556 "Quantidade de vias adicionais excede o máximo permitido"
            CTelasVota::GetInst().m_telaLimiteViasAdicionais->Exibe();    // CTelasVota +204 (name inferred)
            api::CWait::Sleep(3000);   // emscripten_sleep(3000) guarded by the flag @1584624 (== 1)
            CLogVota::GetInst().LogaMesarioIndagadoQtdVias();
            tela->Exibe();                                                 // ask again
            break;
        }

        api::CApplication::RemoveForm(*tela);                              // api_f5987 (name inferred)
        auto& vota = comum::CAppInfo::GetInst().GetVota();
        for (uebyte i = 0; i < quantidade; ++i) {
            if (comum::UrnaDesligando())                                  // global flag @1832936
                return;                                                    // m_proximoEstado unchanged
            const uebyte via = vota.GetQtdBU() + 1;                        // CEstadoGeralVota +8
            CLogVota::GetInst().LogaImpressaoRelatorio(comum::ERelatoriosUE::BU, via);   // func 1127
                                                         // "Imprimindo relatório [BU] via nº [<via>]"
            CImprimindoBU::ImprimeBU(via, false);                          // func 2890
            comum::CAppInfo::GetInst().GetVota().IncrementaQtdBU();        // func 3701 (+8 += 1)
            comum::SalvaEstado();                                          // func 491
        }
        m_proximoEstado = &CMostraQRCodeBU::GetInst();
        break;
    }

    default:
        break;
    }
}

// ------------------------------------------------------------------------------------------------
// CLogVota helpers that the tools attributed to this file (their callers are only here). They are
// methods of vota::CLogVota (uenux2/src/app/vota/log/clogvota.cpp, path inferred): each builds a
// literal std::string and calls CLogVota::Loga(msg) (api_f233 -> api::CLoga::loga(level 1, msg)).
//
// wasm func 4557
void CLogVota::LogaMesarioNaoSolicitouVias()                               // name inferred
{
    Loga("Mesário não solicitou emissão de vias adicionais");
}

// wasm func 4563
void CLogVota::LogaMesarioIndagadoQtdVias()                                // name inferred
{
    Loga("Mesário indagado sobre quantidade de vias adicionais");
}

}  // namespace vota
