// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/fimvotacao/cmostraqrcodecertificado.h (path inferred from the .cpp,
// attested by srcloc line 39).
//
// "Mostra QR code do certificado" = shows the urna's digital certificate as a QR code
// ("O QR code ao lado contém o certificado desta urna."); CORRIGE ("Retornar") goes back to the BU QR codes.
//
// RTTI: comum::CAppState <- vota::CEstadoComDesligamentoAutomatico <- vota::CMostraQRCodeCertificado
//   (typeinfo @1542544, vtable @1542488)
//   [0] 785 (ICF) [1] 1560 (ICF) [2] StartState 12040 [7] ProcessInput 12038 [8] 12061 [9] nop
#pragma once

#include "vota/eleitor/cestadocomdesligamentoautomatico.h"
#include "vota/eleitor/comum/ctelasvota.h"

namespace vota {

class CMostraQRCodeCertificado final : public CEstadoComDesligamentoAutomatico {
public:
    /// Singleton created inline by CMostraQRCodeBU::ProcessInput (@1833876, mutex @1833852):
    /// 36 bytes, CEstadoComDesligamentoAutomatico(2) (func 1285), m_tela = {}.
    static CMostraQRCodeCertificado& GetInst();

    void StartState() override;       // wasm func 12040
    void ProcessInput() override;     // wasm func 12038 (srcloc 39)

private:
    CFormInterativoTelaVota m_tela;   // +28 (+32) "telaQRCodeCertificado", rebuilt at every StartState
};

}  // namespace vota
