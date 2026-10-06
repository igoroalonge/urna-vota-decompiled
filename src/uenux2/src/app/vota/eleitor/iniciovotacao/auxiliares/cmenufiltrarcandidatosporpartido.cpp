// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatosporpartido.cpp
//
// "Filtragem por partido": lists the party numbers of the candidacies that match the filters, plus
// "Todos"; the choice goes to CVisualizarCandidatos::m_partido. Only the "Todos" item gets its own
// format "%S - %T" (shown as "0 - Todos"); the menu-wide format is set to "%S", so every party item
// is shown as its NUMBER ONLY (search text), without sigla or name (item text is empty).
//
// srcloc evidence:
//   :39  StartState                       api::IInputKbd lookup (argument of the menu field's Read)
//   api/gui/cinputmenufield.cpp:156       CMenuItem::SetFormatString        (inlined, "Todos" item)
//   api/gui/cinputmenufield.cpp:468       CInputMenuField::SetFormatString  (inlined, whole menu)
//                                         (both look up api::IScreen to re-measure the items)
// WARNING (web build): same blocking Read() as CMenuVisualizarCandidatos (emscripten_sleep -> abort).
// LATENT BUG: when no party passes the filters, the menu field is never created, yet the read loop
// still calls menu->Read() through the null shared_ptr (wasm: vptr loaded from address 0 ->
// call_indirect of table entry 0 -> trap; native: segfault). Only reachable with no candidacy at all.

#include "vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatosporpartido.h"

#include <any>
#include <set>
#include <string>

#include "api/gui/cformbuilder.h"
#include "api/gui/cinputmenufield.h"
#include "api/hwil/iinput.h"             // api::IInputKbd
#include "api/pattern/cpolysingletonlist.h"
#include "vota/eleitor/iniciovotacao/auxiliares/cmenuvisualizarcandidatos.h"
#include "vota/eleitor/iniciovotacao/auxiliares/cvisualizarcandidatos.h"

namespace vota {

// wasm func 11884 — vtable slot 2
void CMenuFiltrarCandidatosPorPartido::StartState()
{
    auto& visualizar = CVisualizarCandidatos::GetInst();                                       // func 1279
    visualizar.CarregaCandidaturas();                                                          // func 5953

    std::set<uint16_t> partidos;
    for (const auto& c : visualizar.m_candidaturas) {
        if (visualizar.m_numero && *visualizar.m_numero != 0 && c.GetNumero() != *visualizar.m_numero)       continue;
        if (visualizar.m_cargo && *visualizar.m_cargo != 0 && c.GetCargo() != *visualizar.m_cargo)           continue;
        if (visualizar.m_partido && *visualizar.m_partido != 0 && c.GetPartido() != *visualizar.m_partido)   continue;
        partidos.insert(c.GetPartido());
    }

    // Screen "telaFiltragemCandidatosPorPartido"
    api::CFormBuilder b;
    b.AddStatusHeader(5);
    b.AddLabel("Visualização de candidatos", {320, 40}, FONTE_TITULO, 2, 2, 1);
    b.AddLabel("Filtragem por partido", {320, 75}, FONTE_TEXTO, 2, 2, 1);
    std::shared_ptr<api::CInputMenuField> menu;
    if (partidos.empty()) {
        b.AddLabel("Nenhum partido disponível!", {320, 200}, FONTE_TEXTO, 2, 2, 1);
        b.AddLabeledInputControl({{'D', "Retornar"}});
    } else {
        menu = b.AddInputMenu("Selecione o partido: ", {10, 120}, FONTE_PEQUENA, 310);
        auto todos = menu->AddItem("Todos");
        todos->m_dado = std::any(uint16_t{0});                  // any handler slot 1109
        todos->SetSearchText("0");
        todos->SetFormatString("%S - %T");                      // item format (cinputmenufield.cpp:156): "0 - Todos"
        menu->SetFormatString("%S");                            // menu format (cinputmenufield.cpp:468): number only
        for (const uint16_t numero : partidos) {
            auto item = menu->AddItem("");                      // empty item text: only "%S" is shown
            item->m_dado = std::any(numero);
            item->SetSearchText(std::to_string(numero));        // ecourna_f296
        }
        b.AddLabeledInputControl({{'C', "Confirmar seleção"}, {'D', "Retornar"}});
    }
    auto tela = api::CriaFormInterativo(b, "telaFiltragemCandidatosPorPartido");

    tela->Exibe();
    auto& teclado = api::CPolySingletonList::instance<api::IInputKbd>();   // :39
    for (;;) {
        switch (menu->Read(teclado)) {                          // CInputMenuField::Read (10894), slot 10;
                                                                // null menu when partidos is empty (see top)
        case api::EInputResult::Corrige:
            m_proximoEstado = &CMenuVisualizarCandidatos::GetInst();       // api_f2288
            return;
        case api::EInputResult::Confirma:
            visualizar.m_partido = std::any_cast<uint16_t>(menu->GetItemSelecionado()->m_dado);   // +36/+38
            return;
        default:
            break;
        }
    }
}

}  // namespace vota
