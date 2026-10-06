// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/fimvotacao/cmostraqrcodebu.cpp
//
// srcloc evidence:
//   :70   StartState  comum::IInterfaceInit lookup (GetDemoMode)
//   :85   ProcessInput  "Número de inputs inválido"                     (CUeVotaError 9373)
//   :195  static void AjustaTela(CFormInterativoTelaVota&)  "Campo não encontrado no formulário" (9374)
//   (:31 GetQRDSInst and :38 IQRCodeBUDS ctor live in func 1956, unit u19)
// Not executed in the recorded sessions.

#include "vota/eleitor/fimvotacao/cmostraqrcodebu.h"

#include <format>
#include <memory>
#include <mutex>
#include <source_location>
#include <string>

#include "api/gui/cformbuilder.h"
#include "api/gui/cqrcodeimage.h"
#include "api/gui/ctextfield.h"
#include "comum/iinterfaceinit.h"
#include "vota/comum/votadefs.h"                              // CUeVotaError
#include "vota/eleitor/fimvotacao/caplicacaoencerrada.h"
#include "vota/eleitor/fimvotacao/cmostraqrcodecertificado.h"

namespace vota {

namespace {

IQRCodeBUDS& GetQRDSInst();              // cmostraqrcodebu.cpp:31 (func 1956, unit u19)

// Data sources of the screen, bound as std::function<std::string()> (function-table slots 1505..1507,
// funcs 12054, 12053, 12052; not in this unit).
std::string TextoInstrucaoQRCode();       // slot 1505: "O QR code ao lado contém o resultado da votação para
                                          // esta urna." (+ " Use as teclas 3 e 9 para navegar pelas partes do
                                          // BU." when there is more than one part)
std::string QRCodeAtual();                // slot 1506: m_qrcodes.at(m_indice)
std::string TextoPagina();                // slot 1507: std::to_string(m_indice + 1) + "/" + std::to_string(n)

// Screen "telaQRCodeBU" (builder inlined into StartState; probably a CTelasVota factory).  name inferred
CFormInterativoTelaVota CriaTelaQRCodeBU()
{
    CTelasVota::GetInst();                                             // (result unused)
    api::CFormBuilder b;
    b.AddStatusHeader(5);
    b.AddImage(std::make_shared<api::CQRCodeImage>(380, &QRCodeAtual), {/*left column*/});   // func 3662, 5547
    b.AddLine(/* vertical separator */ 3);                                                   // api_f2244
    // positions relative to the screen rectangle (altura = its bottom, read from a constant @+5238 of a
    // data table; 480 on the urna)
    b.AddLabel(std::format("Versão: {}", api::CApplication::GetVersao()), {10, altura - 79}, FONTE_475008, 0, 2, 1);
    b.AddLabel("BU digital", {10, 45}, FONTE_TITULO /*474888*/, 0, 2, 1);
    b.AddField(std::make_shared<api::CTextFieldMultiLine>(rect, &TextoInstrucaoQRCode, FONTE_PEQUENA));
    b.AddField(std::make_shared<api::CTextField>(rect, &TextoPagina, FONTE_PEQUENA, 2, 1), "Pagina");
    // key + action labels, named so that AjustaTela can find them (the action text starts 4 px to the
    // right of the key label)
    b.AddLabel("9", {10, altura - 27}, FONTE_TECLA /*475032*/, "9");  b.AddLabel("para próxima parte", "Acao9");
    b.AddLabel("3", {10, altura - 48}, FONTE_TECLA, "3");             b.AddLabel("para parte anterior", "Acao3");
    b.AddLabel("CONFIRMA", posConfirma, FONTE_PEQUENA, "Confirma");   b.AddLabel("Continuar", "AcaoConfirma");
    b.AddLabeledInput('B', "Ver certificado", 2);                                            // api_f2243
    return api::CriaFormInterativo(b, "telaQRCodeBU");                                        // func 576
}

}  // namespace

// wasm func 12055 — vtable slot 2
void CMostraQRCodeBU::StartState()
{
    if (comum::IInterfaceInit::GetInst().GetDemoMode()) {             // :70 (func 729)
        m_proximoEstado = &CAplicacaoEncerrada::GetInst();            // demonstration mode: no BU QR code
        return;
    }
    GetQRDSInst().m_indice = 0;
    m_tela = CriaTelaQRCodeBU();
    AjustaTela(m_tela);                                               // also shows it
    m_proximoEstado = this;
}

// wasm func 12051 — vtable slot 7
void CMostraQRCodeBU::ProcessInput()
{
    auto& ds = GetQRDSInst();
    if (m_tela->GetInputs().size() != 1)
        throw CUeVotaError(9373, "Número de inputs inválido", std::source_location::current());   // :85
    const auto& campo = *m_tela->GetInputs().front();

    switch (m_tela->Read()) {                                          // cinteractiveform.h:57
    case api::EInputResult::Branco:                                    // 3: "Ver certificado"
        m_proximoEstado = &CMostraQRCodeCertificado::GetInst();        // created inline: @1833876,
        break;                                                         //  CEstadoComDesligamentoAutomatico(2)
    case api::EInputResult::Confirma:                                  // 9: only accepted on the last part
        if (ds.m_indice + 1 == ds.m_qrcodes.size())
            m_proximoEstado = &CAplicacaoEncerrada::GetInst();         // func 3877
        break;
    case api::EInputResult::Tecla:                                     // 13: a digit
        switch (campo.GetUltimaTecla()) {                              // input field +55
        case '3':                                                      // previous part
            if (ds.m_indice == 0)
                return;
            --ds.m_indice;
            break;
        case '9':                                                      // next part
            if (ds.m_indice + 1 == ds.m_qrcodes.size())
                return;
            ++ds.m_indice;
            break;
        default:
            return;
        }
        AjustaTela(m_tela);
        break;
    default:
        break;
    }
}

// wasm func 5982 — srcloc :195
// Field states (api::IField::SetEstado, func 940; field +44): 1 = hidden, 2 = enabled, 3 = disabled
// (names inferred from how they are combined).
void CMostraQRCodeBU::AjustaTela(CFormInterativoTelaVota& tela)
{
    tela->ReiniciaCampos();          // inlined IForm helper: if the "dirty" flag is set, clear it and call
                                     // slot 4 of every field                               name inferred
    const auto& ds = GetQRDSInst();
    auto confirma     = tela->FindFieldAs<api::CTextField>("Confirma");      // func 1721 (iform.h:142)
    auto acaoConfirma = tela->FindFieldAs<api::CTextField>("AcaoConfirma");
    auto acao3        = tela->FindFieldAs<api::CTextField>("Acao3");
    auto tecla3       = tela->FindFieldAs<api::CTextField>("3");
    auto acao9        = tela->FindFieldAs<api::CTextField>("Acao9");
    auto tecla9       = tela->FindFieldAs<api::CTextField>("9");
    auto pagina       = tela->FindFieldAs<api::CTextField>("Pagina");
    if (!confirma || !acaoConfirma || !acao3 || !tecla3 || !acao9 || !tecla9 || !pagina)
        throw CUeVotaError(9374, "Campo não encontrado no formulário", std::source_location::current());  // :195

    constexpr uebyte OCULTO = 1, HABILITADO = 2, DESABILITADO = 3;   // names inferred
    const std::size_t n = ds.m_qrcodes.size();
    if (n == 1) {                            // single part: no navigation
        confirma->SetEstado(HABILITADO);  acaoConfirma->SetEstado(HABILITADO);
        tecla3->SetEstado(OCULTO); acao3->SetEstado(OCULTO); tecla9->SetEstado(OCULTO); acao9->SetEstado(OCULTO);
        pagina->SetEstado(OCULTO);
    } else {
        pagina->SetEstado(HABILITADO);
        if (ds.m_indice == 0) {              // first part: back disabled, CONFIRMA disabled
            confirma->SetEstado(DESABILITADO); acaoConfirma->SetEstado(DESABILITADO);
            tecla3->SetEstado(DESABILITADO);   acao3->SetEstado(DESABILITADO);
            tecla9->SetEstado(HABILITADO);     acao9->SetEstado(HABILITADO);
        } else if (ds.m_indice + 1 == n) {   // last part: next disabled, CONFIRMA enabled
            confirma->SetEstado(HABILITADO);   acaoConfirma->SetEstado(HABILITADO);
            tecla3->SetEstado(HABILITADO);     acao3->SetEstado(HABILITADO);
            tecla9->SetEstado(DESABILITADO);   acao9->SetEstado(DESABILITADO);
        } else {                             // middle part
            confirma->SetEstado(DESABILITADO); acaoConfirma->SetEstado(DESABILITADO);
            tecla3->SetEstado(HABILITADO);     acao3->SetEstado(HABILITADO);
            tecla9->SetEstado(HABILITADO);     acao9->SetEstado(HABILITADO);
        }
    }
    tela->Exibe();                                                     // form slot 2
}

}  // namespace vota
