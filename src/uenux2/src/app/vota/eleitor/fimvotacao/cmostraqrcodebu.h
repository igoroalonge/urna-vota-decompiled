// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/fimvotacao/cmostraqrcodebu.h (path inferred from cmostraqrcodebu.cpp,
// attested by srcloc lines 31, 38, 70, 85, 195).
//
// "Mostra QR code do BU" = last screen of the election day: the BU as QR codes ("BU digital") on the
// urna's screen, one part at a time (keys 3/9 = previous/next part), BRANCO = show the urna's
// certificate QR code (CMostraQRCodeCertificado), CONFIRMA on the last part = application finished
// (CAplicacaoEncerrada). The payloads come from the poly-singleton vota::IQRCodeBUDS
// ((anonymous)::GetQRDSInst(), cmostraqrcodebu.cpp:31/38, func 1956, unit u19), which holds the
// QR texts produced for the printed BU and the index of the part shown.
//
// RTTI: comum::CAppState <- vota::CEstadoComDesligamentoAutomatico <- vota::CMostraQRCodeBU
//   (typeinfo @1542212, vtable @1542124)
//   [0] 785 dtor (ICF) [1] 1560 deleting (ICF) [2] StartState 12055 [7] ProcessInput 12051
//   [8] CEstadoComDesligamentoAutomatico::ProcessTick 12061 [9] ProcessTickNaoDesligamento nop
#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "vota/eleitor/cestadocomdesligamentoautomatico.h"
#include "vota/eleitor/comum/ctelasvota.h"

namespace vota {

/// Data source of the BU QR codes (vtable @1542092: [0] dtor 12050, [1] deleting 12049).
/// Constructor attested: vota::IQRCodeBUDS::IQRCodeBUDS(std::vector<std::string>) (cmostraqrcodebu.cpp:38).
class IQRCodeBUDS {
public:
    explicit IQRCodeBUDS(std::vector<std::string> qrcodes);
    virtual ~IQRCodeBUDS();

    std::vector<std::string> m_qrcodes;   // +4 (12-byte elements)
    std::size_t m_indice = 0;             // +16 part being shown
};

class CMostraQRCodeBU final : public CEstadoComDesligamentoAutomatico {
public:
    /// Lazy singleton, wasm func 2888 (unit u37): 36 bytes, CEstadoComDesligamentoAutomatico(2)
    /// (func 1285), m_tela = {} ; @1833848 (mutex @1833824).
    static CMostraQRCodeBU& GetInst();

    void StartState() override;       // wasm func 12055 (srcloc 70)
    void ProcessInput() override;     // wasm func 12051 (srcloc 85)

    /// Enables/disables the navigation labels for the current part and redraws. wasm func 5982 (:195)
    static void AjustaTela(CFormInterativoTelaVota& tela);

private:
    CFormInterativoTelaVota m_tela;   // +28 (+32) "telaQRCodeBU"
};

}  // namespace vota
