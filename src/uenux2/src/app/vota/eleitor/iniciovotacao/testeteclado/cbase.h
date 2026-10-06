// Reconstructed from vota_web_wasm.wasm (unit u39; constructor func 5948 is in unit u37).
// Original (path inferred; already included by cprezeresima.h as
// "vota/eleitor/iniciovotacao/testeteclado/cbase.h"): uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cbase.h
//
// testeteclado::CBase = common base of the two states that OFFER the keypad test of the voter terminal
// ("teste do teclado do TE"): CPreZeresima (before the zerésima) and CRetomada (after a restart). The
// subclass builds the question screen (CriaTela) and says where to go when the test passes or is skipped;
// the base shows the screen and reads CONFIRMA ("Testar") / CORRIGE ("Não testar").
//
// RTTI: comum::CAppState <- vota::CEstadoComDesligamentoAutomatico <- vota::testeteclado::CBase
//   (typeinfo @1545828, vtable @1545776, 36 bytes) <- CPreZeresima (@1546044), CRetomada (@1546708)
//   [0] dtor 1559 (also the slot 0 of both subclasses)  [1] ICF 325 deleting  [2] StartState 11875
//   [3] 7480 [4] 1661 [5] nop [6] nop [7] ProcessInput 11874 [8] CEstadoComDesligamentoAutomatico 12061
//   [9] nop [10] CriaTela (pure) [11] GetEstadoPassouNoTeste (pure) [12] GetEstadoSemTeste (pure)
// Slot 10-12 names: unit u26 (cprezeresima.h).
#pragma once

#include "vota/eleitor/cestadocomdesligamentoautomatico.h"
#include "vota/eleitor/comum/ctelasvota.h"

namespace vota::testeteclado {

class CBase : public CEstadoComDesligamentoAutomatico {
public:
    ~CBase() override;                                          // [0] wasm func 1559

    void StartState() override;                                 // [2] wasm func 11875
    void ProcessInput() override;                               // [7] wasm func 11874

    virtual CFormInterativoTelaVota CriaTela() = 0;             // [10]
    virtual comum::CAppState* GetEstadoPassouNoTeste() = 0;     // [11] state after a successful test
    virtual comum::CAppState* GetEstadoSemTeste() = 0;          // [12] state when the test is declined

protected:
    /// wasm func 5948 (unit u37): CEstadoComDesligamentoAutomatico(2 = keys) (func 1285), m_tela = {}.
    CBase();

    // +0..+27 CEstadoComDesligamentoAutomatico
    CFormInterativoTelaVota m_tela;                             // +28 (+32)
};

}  // namespace vota::testeteclado
