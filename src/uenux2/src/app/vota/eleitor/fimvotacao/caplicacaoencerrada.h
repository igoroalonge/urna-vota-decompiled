// Reconstructed from vota_web_wasm.wasm (unit u39 = StartState; constructor/GetInst by u09).
// Original (path inferred by u09): uenux2/src/app/vota/eleitor/fimvotacao/caplicacaoencerrada.h
//
// "Aplicação encerrada": the last state of the voting application on election day, after the BU copies
// and the BU QR codes (CMostraQRCodeBU / CMostraQRCodeCertificado) or when the mesário declines to print
// the BU in voter-training mode (CQuerImprimirBU). It only shows the final screen; being a
// CEstadoComDesligamentoAutomatico, it switches the urna off after the battery time-out when mains power
// is lost.
//
// RTTI: comum::CAppState <- vota::CEstadoComDesligamentoAutomatico <- vota::CAplicacaoEncerrada
//   (typeinfo @1542072, vtable @1542032, 36 bytes)
//   [0] ICF 785 [1] ICF 1560 [2] StartState 12058 [3] 7480 [4] 1661 [5] nop [6] nop [7] nop
//   [8] CEstadoComDesligamentoAutomatico::ProcessTick 12061 [9] nop
#pragma once

#include "vota/eleitor/cestadocomdesligamentoautomatico.h"
#include "vota/eleitor/comum/ctelasvota.h"

namespace vota {

class CAplicacaoEncerrada final : public CEstadoComDesligamentoAutomatico {
public:
    static CAplicacaoEncerrada& GetInst();             // wasm func 3877 (u09)

    void StartState() override;                        // [2] wasm func 12058

private:
    CAplicacaoEncerrada();                             // CEstadoComDesligamentoAutomatico(0), u09

    // +0..+27 CEstadoComDesligamentoAutomatico (+12 battery deadline, +24 1 s tick)
    CFormInterativoTelaVota m_tela;                    // +28 (+32) = CTelasVota +36 (screen "aplicação encerrada")
};

}  // namespace vota
