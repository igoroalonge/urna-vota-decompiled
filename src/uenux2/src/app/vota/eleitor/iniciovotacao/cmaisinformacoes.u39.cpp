// FRAGMENT reconstructed by unit u39 from vota_web_wasm.wasm.
// Original file (path inferred; include path "vota/eleitor/iniciovotacao/cmaisinformacoes.h" is used by
// cverificahorariozeresima.cpp and cquerreimprimirzeresima.cpp): uenux2/src/app/vota/eleitor/iniciovotacao/cmaisinformacoes.cpp
// GetInst (func 1280): u26-foreign-fragments.cpp. ProcessInput (11896): unit u34.
//
// "Mais informações" menu of the voter terminal before voting (key BRANCO on the zerésima screens):
//   [1] Estado da urna (n/max)  [2] Lista de eleitores (n/max)  [3] Versões de pacotes (n/max)
//   [4] Parâmetros de urna (n/max)  [5] Visualizar candidatos
// n = copies already printed (EstadoGeralVota.numViasImpressasRelatorios), max = the election's limit;
// an item whose limit was reached cannot be chosen (see the CItem*Vota classes in eleitor/comum/ctelasvota.u39.cpp).
// The screen is REBUILT on every entry (CTelasVota::CriaTelaMaisInformacoes, func 6599) because the counts
// change after each print.
//
// Class declaration: vota/eleitor/iniciovotacao/estadosiniciovotacao.u34.h (unit u34):
//   CMaisInformacoes : comum::CAppState (typeinfo @1545348, vtable @1545312, 24 bytes)
//   +12 CFormInterativoTelaVota m_form, +20 comum::CAppState* m_retorno
#include "vota/eleitor/comum/ctelasvota.h"
#include "vota/eleitor/iniciovotacao/estadosiniciovotacao.u34.h"

namespace vota {

// wasm func 11897 - vtable slot 2
void CMaisInformacoes::StartState()
{
    m_form = CTelasVota::GetInst().CriaTelaMaisInformacoes();   // func 407 + func 6599 (sret; `this` dropped by DAE)
    m_form->Show();
    m_proximoEstado = this;
}

}  // namespace vota
