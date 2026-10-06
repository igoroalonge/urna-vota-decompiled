// Reconstructed from vota_web_wasm.wasm (unit u10).
// Original: uenux2/src/app/vota/operador/confirmaidentidade/cdigitalreconhecida.cpp
// (attested by the std::source_location record of StartState, line 47).
#include "vota/operador/confirmaidentidade/cdigitalreconhecida.h"

#include <format>
#include <map>

#include "api/hwil/ifingermatcher.h"
#include "comum/cconfiguracaoeleicao.h"
#include "vota/eleitor/cthreadeleitor.h"
#include "vota/log/clogvota.h"
#include "vota/operador/aguardaeleitor/cmostraeleitorvotando.h"
#include "vota/operador/comum/chabilitaaudioeleitor.h"
#include "vota/operador/comum/iinformacaothreadoperador.h"

namespace vota {

using api::SPoint;

// Constructor - inlined into func 5397 together with GetInst (static unique_ptr @1908980).
CDigitalReconhecida::CDigitalReconhecida()
    : comum::CAppState(2)                                                     // keys
    , m_exibeScore(comum::CConfiguracaoEleicao::GetInst().GetExibirScoreBiometria())   // cfg +487
    , m_textoScore(std::make_shared<std::string>(" "))
{
    api::CFormBuilderMT campos;
    campos.Add<api::CLedFieldMT>(false);                                                   // func 435
    campos.Add<api::CBuzzFieldMT>(52, 5);                                                  // func 941
    campos.Add<api::CTextFieldMT>(SPoint{1, 1}, std::make_shared<api::CFixedText>(api::ETextAlignment(0),
                                                                              "ELEITOR(A) RECONHECIDO(A)"));
    // api::CTextSource(shared_ptr<string>) throws CUeGuiError 4977 "Texto nulo" if the pointer is null
    // (ctextsource.h:37) - m_textoScore is never null here.
    campos.Add<api::CTextFieldMT>(SPoint{40, 1}, std::make_shared<api::CDataTextFmt<api::CTextSource>>(
                                                     api::ETextAlignment(1), api::CTextSource(m_textoScore), "%s"));   // func 1152
    campos.Add<api::CTextFieldMT>(SPoint{1, 2}, std::make_shared<api::CFixedText>(api::ETextAlignment(0),
                                                                              "Não assinar o caderno de votação"));
    campos.Add<api::CTextFieldMT>(SPoint{40, 4}, std::make_shared<api::CFixedText>(api::ETextAlignment(1),
                                                                               "CONFIRMA: prosseguir"));
    campos.AddInputControl();                                                              // func 395
    m_form = campos.CriaFormInterativo("", true);                                          // func 301
}

// ---------------------------------------------------------------------------------------------
// wasm func 10482 - vtable slot 2 (srcloc line 47)
void CDigitalReconhecida::StartState()
{
    m_proximoEstado = this;
    api::IFingerMatcher& comparador = api::CPolySingletonList::instance<api::IFingerMatcher>();   // line 47
    if (m_exibeScore)
        *m_textoScore = std::format("Score: {}", comparador.GetScore());      // slot 4 (int)
    m_form->Show();
}

// ---------------------------------------------------------------------------------------------
// wasm func 10481 - vtable slot 7                                             name from slot order
void CDigitalReconhecida::ProcessInput()
{
    if (m_form->Read() != api::EInputResult::CONFIRMA)                        // cinteractiveform.h:57 inlined
        return;

    auto& info = impl::IInformacaoThreadOperador::GetInst();
    if (info.DeveHabilitarAudio()) {                                          // slot 3 (api_f2746)
        m_proximoEstado = &CHabilitaAudioEleitor::GetInst();                  // func 2743
    } else {
        CThreadEleitor::GetInst().EnviaMensagem({CThreadEleitor::MSG_INICIA_ELEITOR}, 1);   // 1: release the urna
        m_proximoEstado = &CMostraEleitorVotando::GetInst();                  // func 1150
    }
    info.SetHabilitacaoBiometrica();                                          // slot 13

    const std::map<uebyte, std::string> tiposHabilitacao{{1, "biométrica"}, {2, "por código"}};   // Latin-1
    CLogVota::GetInst().Loga(std::format("Tipo de habilitação do eleitor [{}]", tiposHabilitacao.at(1)));
    // map::at throws std::out_of_range("map::at:  key not found") - impossible here.
    // func 2499 = std::__tree<...std::string...>::destroy for this map (shared by ICF with the static
    // map of api::getResourceMovie, whose exit-time destructor is func 13471).
}

}  // namespace vota
