// FRAGMENT reconstructed by unit u34 from vota_web_wasm.wasm.
// Original file: uenux2/src/app/vota/eleitor/iniciovotacao/cmaisinformacoes.cpp  (path inferred)
// Class declaration: estadosiniciovotacao.u34.h. GetInst (func 1280) is in u26-foreign-fragments.cpp,
// StartState (func 11897) in unit u39.
//
// The "Mais informações" menu of the voter screen before voting (reached with BRANCO from the zerésima /
// restart questions). Each option is a report printed on the urna's thermal printer; the sub-states return
// to this menu with CMaisInformacoes::GetInst(nullptr), which keeps m_retorno.
//
// WEB BUILD: never reached (and the printer is CWasmNullPrinter).
#include <algorithm>
#include <memory>
#include <mutex>
#include <string>

#include "ecourna/api/util/cstringutils.hpp"
#include "vota/eleitor/iniciovotacao/auxiliares/cmenuvisualizarcandidatos.h"
#include "vota/eleitor/iniciovotacao/estadosiniciovotacao.u34.h"
#include "vota/eleitor/iniciovotacao/impressoes.h"   // CImpressaoEstadoUrna, CImpressaoListaEleitores,
                                                      // CImpressaoVersaoPacotes, CImpressaoPU (path ?)

namespace vota {

// wasm func 11896 - vtable slot 7 (ProcessInput). srcloc cinteractiveform.h:57 @1545360.
void CMaisInformacoes::ProcessInput()
{
    switch (m_form->Read()) {
    case api::EInputResult::CORRIGE:
        m_proximoEstado = m_retorno;                  // +20: the state that opened the menu
        m_retorno = nullptr;
        return;
    case api::EInputResult::CONFIRMA:
        break;
    default:
        return;
    }

    // First input field of the form (.at(0): std::out_of_range if the form has none), text at +24.
    const std::string opcao = m_form->GetEntrada(0).GetTexto();
    if (opcao.empty() || !std::ranges::all_of(opcao, [](char c) { return c >= '0' && c <= '9'; }))
        return;

    // The five report states are lazy singletons of 12 bytes (CAppState(2), no members) created inline:
    //   CImpressaoEstadoUrna    @1834408 (mutex @1834384, vtable @1545064) - StartState 11911
    //   CImpressaoListaEleitores@1834436 (mutex @1834412, vtable @1545120) - StartState 11908
    //   CImpressaoVersaoPacotes @1834464 (mutex @1834440, vtable @1545176) - StartState 11905
    //   CImpressaoPU            @1834492 (mutex @1834468, vtable @1545232) - StartState 11902
    switch (ecourna::api::util::CStringUtils::ToInt32(opcao)) {                  // func 2200
    case 1: m_proximoEstado = &CImpressaoEstadoUrna::GetInst();      break;    // "Estado da urna"
    case 2: m_proximoEstado = &CImpressaoListaEleitores::GetInst();  break;    // "Lista de eleitores"
    case 3: m_proximoEstado = &CImpressaoVersaoPacotes::GetInst();   break;    // "Versões de pacotes"
    case 4: m_proximoEstado = &CImpressaoPU::GetInst();              break;    // "Parâmetros de urna"
    case 5: m_proximoEstado = &CMenuVisualizarCandidatos::GetInst(); break;    // api_f2288 (merged body 764)
    default: break;                                                            // any other number: stay
    }
}

}  // namespace vota
