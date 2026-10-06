// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatosporcargo.cpp
//
// "Filtragem por cargo": lists the cargos that have candidacies matching the filters already chosen,
// plus "Todos" (not offered when the next step is the number entry), and stores the choice in
// CVisualizarCandidatos::m_cargo.
//
// srcloc evidence:
//   :44  StartState  api::IInputKbd lookup (argument of the menu field's Read)
// WARNING (web build): same blocking Read() as CMenuVisualizarCandidatos (emscripten_sleep -> abort).
// LATENT BUG: when no cargo passes the filters ("Nenhum cargo disponível!"), the menu field is never
// created, yet the read loop still calls menu->Read() through the null shared_ptr (wasm: vptr loaded
// from address 0 -> call_indirect of table entry 0 -> trap; native: segfault). Only reachable when
// there is no candidacy at all (every later filter step lists values taken from the candidacies).

#include "vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatosporcargo.h"

#include <any>
#include <compare>
#include <map>
#include <set>

#include "api/gui/cformbuilder.h"
#include "api/gui/cinputmenufield.h"
#include "api/hwil/iinput.h"             // api::IInputKbd
#include "api/pattern/cpolysingletonlist.h"
#include "comum/dados/cconfiguracaoeleicao.h"
#include "vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatospornumero.h"
#include "vota/eleitor/iniciovotacao/auxiliares/cmenuvisualizarcandidatos.h"
#include "vota/eleitor/iniciovotacao/auxiliares/cvisualizarcandidatos.h"

namespace vota {

// Ordering of the cargo set: three-way comparison of the TCargoID (first byte of comum::md::CCargo).
// wasm func 5655 — `(a > b) - (a < b)` on uebyte: std::compare_three_way for the key.  library/inline
struct MenorCargo {
    bool operator()(const comum::md::CCargo& a, const comum::md::CCargo& b) const
    {
        return (a.GetID() <=> b.GetID()) < 0;
    }
};

// wasm func 11888 — vtable slot 2
void CMenuFiltrarCandidatosPorCargo::StartState()
{
    const bool ofereceTodos = m_proximoEstado != &CMenuFiltrarCandidatosPorNumero::GetInst();   // func 5954
    auto& visualizar = CVisualizarCandidatos::GetInst();                                       // func 1279
    visualizar.CarregaCandidaturas();                                                          // func 5953

    const auto& cfg = comum::CConfiguracaoEleicao::GetInst();
    std::set<comum::md::CCargo, MenorCargo> cargos;          // 156-byte nodes: CCargo copied with its
    for (const auto& c : visualizar.m_candidaturas) {        // two optional members (+36/+104)
        if (visualizar.m_numero && *visualizar.m_numero != 0 && c.GetNumero() != *visualizar.m_numero)       continue;
        if (visualizar.m_cargo && *visualizar.m_cargo != 0 && c.GetCargo() != *visualizar.m_cargo)           continue;
        if (visualizar.m_partido && *visualizar.m_partido != 0 && c.GetPartido() != *visualizar.m_partido)   continue;
        cargos.insert(cfg.GetCargo(c.GetCargo()));                                                  // func 861
    }

    // Screen "telaFiltragemCandidatosPorCargo"
    api::CFormBuilder b;
    b.AddStatusHeader(5);
    b.AddLabel("Visualização de candidatos", {320, 40}, FONTE_TITULO, 2, 2, 1);
    b.AddLabel("Filtragem por cargo", {320, 75}, FONTE_TEXTO, 2, 2, 1);
    std::shared_ptr<api::CInputMenuField> menu;
    if (cargos.empty()) {
        b.AddLabel("Nenhum cargo disponível!", {320, 200}, FONTE_TEXTO, 2, 2, 1);
        b.AddLabeledInputControl({{'D', "Retornar"}});
    } else {
        menu = b.AddInputMenu("Selecione o cargo: ", {10, 120}, FONTE_PEQUENA /*474896*/, 310);
        if (ofereceTodos) {
            auto item = menu->AddItem("Todos");
            item->m_dado = std::any(uebyte{0});                    // any handler slot 1108
            item->SetSearchText("0");
        }
        for (const auto& cargo : cargos) {
            auto item = menu->AddItem(cargo.GetNome());           // func 1388 (+16 name)
            item->m_dado = std::any(cargo.GetID());
        }
        b.AddLabeledInputControl({{'C', "Confirmar seleção"}, {'D', "Retornar"}});
    }
    auto tela = api::CriaFormInterativo(b, "telaFiltragemCandidatosPorCargo");
    // (the std::set is destroyed here: func 3862, recursive __tree::destroy that also resets the two
    //  optional members of each CCargo through funcs 267/242 -> library instantiation)

    tela->Exibe();
    auto& teclado = api::CPolySingletonList::instance<api::IInputKbd>();   // :44
    for (;;) {
        switch (menu->Read(teclado)) {                                     // CInputMenuField::Read (10894),
                                                                           // slot 10; null when cargos is empty
        case api::EInputResult::Corrige:                                   // 5
            m_proximoEstado = &CMenuVisualizarCandidatos::GetInst();       // api_f2288
            return;
        case api::EInputResult::Confirma:                                  // 9: keep m_proximoEstado as
            visualizar.m_cargo = std::any_cast<uebyte>(menu->GetItemSelecionado()->m_dado);   // chained
            return;
        default:
            break;
        }
    }
}

}  // namespace vota
