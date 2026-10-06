// uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cprezeresima.h   (path inferred from cprezeresima.cpp,
// attested by the std::source_location record :56)
// Reconstructed from vota_web_wasm.wasm (unit u26).
//
// vota::testeteclado::CPreZeresima = the keyboard-test question shown before the zerésima is generated.
// While today is before the eve of the zerésima date (CConfiguracaoEleicao +544, the data/hora da zerésima,
// minus one day) the test is optional ("Quer testar o teclado?" Testar / Não testar); from that eve on it is
// mandatory ("Por favor, teste o teclado", Testar only). The zerésima is printed on election day, so in
// practice the switch happens on the eve of the election.
//
// RTTI: CEstadoComDesligamentoAutomatico <- testeteclado::CBase <- testeteclado::CPreZeresima
//       (typeinfo @1546112, vtable @1546044):
//   [2] CBase::StartState (11875: m_tela = CriaTela(); m_tela->Exibe())   [7] CBase::ProcessInput (11874)
//   [10] CriaTela (11862)   [11] GetEstadoPassouNoTeste (5945, attested)   [12] GetEstadoSemTeste (11861)
// CBase::ProcessInput: CONFIRMA -> CTesteTeclado (with GetEstadoPassouNoTeste() as the state after the test);
// CORRIGE -> GetEstadoSemTeste(), logging "Teste de Teclado do TE não executado" when it leaves the state.
#pragma once

#include "api/util/cdatetime.h"
#include "vota/eleitor/iniciovotacao/testeteclado/cbase.h"     // testeteclado::CBase (path inferred)

namespace vota::testeteclado {

class CPreZeresima : public CBase {
public:
    CFormInterativoTelaVota CriaTela() override;                 // [10] func 11862   name as in unit u07
    comum::CAppState* GetEstadoPassouNoTeste() override;         // [11] func 5945 (srcloc :56)
    comum::CAppState* GetEstadoSemTeste() override;              // [12] func 11861   name as in unit u07

private:
    // CBase: +0..+27 (CEstadoComDesligamentoAutomatico), +28 CFormInterativoTelaVota m_tela
    api::CDateTime m_agora;                                      // +36 set by CriaTela
};

} // namespace vota::testeteclado
