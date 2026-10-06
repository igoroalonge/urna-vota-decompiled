// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/fimvotacao/cmostraqrcodecertificado.cpp
//
// srcloc evidence:
//   :39  ProcessInput  "Número de inputs inválido"   (CUeVotaError 9408)
// Not executed in the recorded sessions.

#include "vota/eleitor/fimvotacao/cmostraqrcodecertificado.h"

#include <format>
#include <functional>
#include <memory>
#include <source_location>
#include <string>

#include "api/gui/cformbuilder.h"
#include "api/gui/cqrcodeimage.h"
#include "vota/comum/votadefs.h"
#include "vota/eleitor/fimvotacao/cmostraqrcodebu.h"

namespace vota {

namespace {

// Data source (function-table slot 1529 = func 12039, attributed to unit u08):
//   MontaConteudoQRCode(MontaQRCodesCertificado(CAppInfo::GetEstadoGeral(), /*dividir*/ false)[0])
//   = "QRCE:1:1 IDUE:<numeroInternoUrna> MDUE:<modelo> CERT:<certificate in hex>"
std::string ConteudoQRCodeCertificado();

}  // namespace

// wasm func 12040 — vtable slot 2 (analyzer name vota::CMostraQRCodeCertificado::vf2). The screen
// factory (probably CTelasVota::CriaTelaQRCodeCertificado) is inlined.
void CMostraQRCodeCertificado::StartState()
{
    const std::function<std::string()> fonte = &ConteudoQRCodeCertificado;
    CTelasVota::GetInst();                                                   // (result unused)

    api::CFormBuilder b;
    b.AddStatusHeader(5);                                                    // func 502
    b.AddImage(std::make_shared<api::CQRCodeImage>(380, fonte), {/*left side*/});   // funcs 3662, 5547
    b.AddLabel(std::format("Versão: {}", api::CApplication::GetVersao()),   // version string @1839192
               {/*x*/ largura - 80, 10}, FONTE_475008, 0, 2, 1);
    b.AddLabel("Certificado digital", {10, 45}, FONTE_TITULO /*474888*/, 0, 2, 1);
    b.AddMultiLineLabel("O QR code ao lado contém o certificado desta urna.",
                        /*rect clamped to x <= 95, y >= 10*/ FONTE_PEQUENA /*474896*/);    // vota_f2782
    b.AddLabeledInputControl({{'D', "Retornar"}});                           // CORRIGE key
    m_tela = api::CriaFormInterativo(b, "telaQRCodeCertificado");            // func 576

    m_tela->Exibe();
    m_proximoEstado = this;
}

// wasm func 12038 — vtable slot 7
void CMostraQRCodeCertificado::ProcessInput()
{
    if (m_tela->GetInputs().size() != 1)
        throw CUeVotaError(9408, "Número de inputs inválido", std::source_location::current());   // :39
    if (m_tela->Read() == api::EInputResult::Corrige)                     // cinteractiveform.h:57
        m_proximoEstado = &CMostraQRCodeBU::GetInst();                    // func 2888
}

}  // namespace vota
