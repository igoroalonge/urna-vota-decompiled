// Reconstructed from vota_web_wasm.wasm (unit u10).
// Original: uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaoreconhecida.cpp
// (attested by the std::source_location record of StartState, line 82).
#include "vota/operador/confirmaidentidade/cdigitalnaoreconhecida.h"

#include <format>

#include "api/hwil/ifingermatcher.h"
#include "comum/cconfiguracaoeleicao.h"
#include "comum/dados/celeitores.h"
#include "vota/log/clogvota.h"
#include "vota/operador/comum/ccancelahabilitacaoeleitor.h"
#include "vota/operador/confirmaidentidade/ccontrolareconhecimento.h"
#include "vota/operador/confirmaidentidade/cdigitalnaoreconhecidadecbiometria.h"
#include "vota/operador/confirmaidentidade/cpededigital.h"
#include "vota/operador/confirmaidentidade/cverificadadoeleitor.h"

namespace vota {

using api::SPoint;

// Constructor - inlined into func 5397 together with GetInst (static unique_ptr @1909036).
CDigitalNaoReconhecida::CDigitalNaoReconhecida()
    : comum::CAppState(2)                                                     // keys
    , m_exibeScore(comum::CConfiguracaoEleicao::GetInst().GetExibirScoreBiometria())   // cfg +487
    , m_textoTentativa(std::make_shared<std::string>("Tentativa x de x"))
    , m_textoScore(std::make_shared<std::string>(" "))
{
    const auto esquerda = api::ETextAlignment(0), direita = api::ETextAlignment(1), centro = api::ETextAlignment(2);
    api::CFormBuilderMT campos;
    campos.Add<api::CLedFieldMT>(false);                                                   // func 435
    campos.Add<api::CBeepFieldMT>(1);                                                      // func 1072
    campos.Add<api::CTextFieldMT>(SPoint{1, 1}, std::make_shared<api::CDataTextFmt<std::string (*)(const std::string&)>>(
                                                    esquerda, &comum::CEleitorDadoNomeParaUrna::Text, "{:2}"));   // func 651
    // Note: x = 1 with centred alignment here (x = 20 in the equivalent line of CDigitalNaoReconhecidaPorTempo).
    campos.Add<api::CTextFieldMT>(SPoint{1, 2}, std::make_shared<api::CFixedText>(centro, "Eleitor(a) não reconhecido(a)"));
    campos.Add<api::CTextFieldMT>(SPoint{1, 3}, std::make_shared<api::CDataTextFmt<api::CTextSource>>(
                                                    centro, api::CTextSource(m_textoTentativa), "%s"));  // func 1152
    campos.Add<api::CTextFieldMT>(SPoint{1, 3}, std::make_shared<api::CDataTextFmt<api::CTextSource>>(
                                                    direita, api::CTextSource(m_textoScore), "%s"));
    campos.Add<api::CTextFieldMT>(SPoint{1, 4}, std::make_shared<api::CFixedText>(esquerda, "CORRIGE: cancelar"));
    campos.Add<api::CTextFieldMT>(SPoint{40, 4}, std::make_shared<api::CFixedText>(direita, "CONFIRMA: prosseguir"));
    campos.AddInputControl();                                                              // func 395
    m_form = campos.CriaFormInterativo("", true);                                          // func 301
}

// ---------------------------------------------------------------------------------------------
// wasm func 10474 - vtable slot 2 (srcloc line 82)
void CDigitalNaoReconhecida::StartState()
{
    m_proximoEstado = this;

    // Biometric data that could not be decrypted (CBiometriaEleitor::m_estadoDecifracao > 0, the
    // ModuloResultadoUrnaCadastro.ErroLeituraBiometria code) is not a "wrong finger": divert.
    bool erroDecifracao = false;
    if (!EhTreinamentoSemTreinamentoEleitor())                                          // func 1823
        erroDecifracao = comum::CEleitores::GetInst().GetCurrent().GetBiometria().GetEstadoDecifracao() > 0;
    if (erroDecifracao) {
        m_proximoEstado = &CDigitalNaoReconhecidaDecBiometria::GetInst();   // GetInst + ctor inlined (@1909008)
        return;
    }

    // Both format arguments go through CControlaReconhecimento::GetInst() (func 1256 is called twice:
    // before the byte load of s_tentativa and again before CConfiguracaoEleicao::GetInst()[136]), so the
    // limit is read by an inlined CControlaReconhecimento member, not directly from the configuration.
    *m_textoTentativa = std::format("Tentativa {} de {}",
                                    CControlaReconhecimento::GetInst().GetTentativa(),
                                    CControlaReconhecimento::GetInst().GetNumTentativas());   // name inferred: cfg +136
    const auto& cfg = comum::CConfiguracaoEleicao::GetInst();
    api::IFingerMatcher& comparador = api::CPolySingletonList::instance<api::IFingerMatcher>();   // line 82
    if (m_exibeScore)
        *m_textoScore = std::format("Score: {}", comparador.GetScore());
    CLogVota::GetInst().Loga(std::format(
        "Número de tentativas de reconhecimento do dedo. Tentativa [{}] de [{}]",
        CControlaReconhecimento::GetTentativa(), cfg.GetNumTentativasHabilitacao()));
    m_form->Show();
}

// ---------------------------------------------------------------------------------------------
// wasm func 10473 - vtable slot 7                                             name from slot order
void CDigitalNaoReconhecida::ProcessInput()
{
    switch (m_form->Read()) {                                                // cinteractiveform.h:57 inlined
    case api::EInputResult::CORRIGE:                                         // 5
        CLogVota::GetInst().Loga(2, "Habilitação cancelada durante reconhecimento biométrico");   // func 2495
        m_proximoEstado = &CCancelaHabilitacaoEleitor::GetInst();            // func 1536
        break;
    case api::EInputResult::CONFIRMA:                                        // 9
        if (CControlaReconhecimento::EhUltimaTentativa()) {                   // func 5405
            m_proximoEstado = &CVerificaDadoEleitor::GetInst();              // func 2735: ask the birth year
        } else {
            CControlaReconhecimento::AvancaTentativa();                      // func 5406 (resets the 1x4 index)
            m_proximoEstado = &CPedeDigital::GetInst();                      // func 2740
        }
        break;
    default:
        break;
    }
}

}  // namespace vota
