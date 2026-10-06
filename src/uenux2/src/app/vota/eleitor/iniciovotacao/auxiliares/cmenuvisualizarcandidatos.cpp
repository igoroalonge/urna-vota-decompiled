// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cmenuvisualizarcandidatos.cpp
//
// "Visualização de candidatos" menu: "Por cargo", "Por partido", "Candidato específico". Each item carries
// (std::any) a pair {first filter state, second filter state}; after the choice the two filter menus are
// chained and end in CVisualizarCandidatos:
//     Por cargo            -> CMenuFiltrarCandidatosPorCargo   -> CMenuFiltrarCandidatosPorPartido -> viewer
//     Por partido          -> CMenuFiltrarCandidatosPorPartido -> CMenuFiltrarCandidatosPorCargo   -> viewer
//     Candidato específico -> CMenuFiltrarCandidatosPorCargo   -> CMenuFiltrarCandidatosPorNumero  -> viewer
//
// srcloc evidence:
//   :55  StartState  api::IInputKbd lookup (Read() of the menu, inlined)
//
// WARNING (web build): the Read() loop runs inside StartState and api::CInputMenuField::Read (func 10894)
// flushes the keyboard and then waits with emscripten_sleep(5): it aborts the wasm (no Asyncify).

#include "vota/eleitor/iniciovotacao/auxiliares/cmenuvisualizarcandidatos.h"

#include <any>
#include <memory>
#include <utility>

#include "api/gui/cformbuilder.h"
#include "api/gui/cinputmenufield.h"
#include "api/hwil/iinput.h"             // api::IInputKbd
#include "api/pattern/cpolysingletonlist.h"
#include "vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatosporcargo.h"
#include "vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatosporpartido.h"
#include "vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatospornumero.h"
#include "vota/eleitor/iniciovotacao/auxiliares/cvisualizarcandidatos.h"
#include "vota/eleitor/iniciovotacao/cmaisinformacoes.h"
#include "vota/log/clogvota.h"

namespace vota {

using ParEstados = std::pair<comum::CAppState*, comum::CAppState*>;   // typeinfo @1539540

// wasm func 11881 — vtable slot 2
void CMenuVisualizarCandidatos::StartState()
{
    CLogVota::GetInst().Loga("Opção de visualização de candidatos selecionada");

    auto* porCargo   = &CMenuFiltrarCandidatosPorCargo::GetInst();     // inline singleton @1834576, CAppState(2)
    auto* porPartido = &CMenuFiltrarCandidatosPorPartido::GetInst();   // inline singleton @1834604, CAppState(2)
    auto* porNumero  = &CMenuFiltrarCandidatosPorNumero::GetInst();    // func 5954

    // Screen "telaMenuVisualizacaoCandidatos" (builder inlined, probably a CTelasVota factory)
    api::CFormBuilder b;
    b.AddStatusHeader(5);
    b.AddLabel("Visualização de candidatos", {320, 40}, FONTE_TITULO /*474888*/, 2, 2, 1);
    auto menu = b.AddInputMenu("Selecione um filtro: ", {40, 85}, FONTE_TEXTO /*474992*/, 32767);  // func 3675
    menu->AddItem("Por cargo")->m_dado            = std::any(ParEstados{porCargo, porPartido});   // func 1693/1763
    menu->AddItem("Por partido")->m_dado          = std::any(ParEstados{porPartido, porCargo});
    menu->AddItem("Candidato específico")->m_dado = std::any(ParEstados{porCargo, porNumero});
    b.AddLabeledInputControl({{'C', "Executar opção"}, {'D', "Retornar"}});                     // func 653
    auto tela = api::CriaFormInterativo(b, "telaMenuVisualizacaoCandidatos");                 // func 576

    for (;;) {
        tela->Exibe();                                                  // shown again before every read
        auto& teclado = api::CPolySingletonList::instance<api::IInputKbd>();   // :55
        switch (menu->Read(teclado)) {                                  // CInputMenuField::Read (10894), slot 10
        case api::EInputResult::Corrige:                                // 5
            m_proximoEstado = &CMaisInformacoes::GetInst();             // api_f1280
            return;
        case api::EInputResult::Confirma: {                             // 9
            const auto [primeiro, segundo] = std::any_cast<ParEstados>(menu->GetItemSelecionado()->m_dado);
            m_proximoEstado = primeiro;                                 // (bad_any_cast -> ecourna_f3861)
            primeiro->m_proximoEstado = segundo;
            segundo->m_proximoEstado = &CVisualizarCandidatos::GetInst();   // func 1279
            auto& visualizar = CVisualizarCandidatos::GetInst();
            visualizar.m_cargo.reset();                                 // +25 = false
            visualizar.m_partido.reset();                               // +38 = false
            visualizar.m_numero.reset();                                // +32 = false
            return;
        }
        default:
            break;                                                      // keep reading
        }
    }
}

}  // namespace vota
