// uenux2/src/app/comum/cmenubase.cpp (path inferred)  --  FRAGMENT written by unit u02
//
// comum::CMenuBase (RTTI "class", vtable @1550932: [0] dtor wasm 12235, [1] deleting dtor wasm 6188,
// [2] Monta() wasm 5913). Derived: vota::CMenuMaisInformacoesVota (vtable @1539820, same slots).
// Layout (32 bytes): +0 vptr, +4 std::vector<std::shared_ptr<CItemMenu>> m_itens,
// +16 std::string m_opcoes (accepted option keys), +28 api::CFormBuilder& m_builder.
// CItemMenu (32 bytes, vtable @1539892): +4 int id, +8 std::string texto, +20 SPoint posicao,
// +24 SFonte (first short = size, 20), slot 2 = Disponivel() (pure virtual; e.g. wasm 12244).

#include "comum/cmenubase.h"

#include <format>

namespace comum {

// wasm func 2285 (vota_f2285)                                          // name inferred
void CMenuBase::AdicionaItem(std::shared_ptr<CItemMenu> item)
{
    m_itens.push_back(item);                                              // shared_f1400
    if (item->Disponivel())                                               // CItemMenu vtable slot 2
        m_opcoes += std::format("{}", item->GetId());                     // id formatted as unsigned
}

// wasm func 5913 (vtable slot 2)                                       // name inferred
// Draws "[<id>] - <texto>" for every item and, below the last one, "Escolha a sua opção:" followed by
// a one-character input field accepting only the ids of the available items (COptionValidation(m_opcoes)).
void CMenuBase::Monta()
{
    for (const auto& item : m_itens)
        m_builder.AddText(std::format("[{}] - {}", item->GetId(), item->GetTexto()),
                          item->GetPosicao(), item->GetFonte(), 0,
                          item->Disponivel() ? 2 : 5, 1);        // wasm 202; unavailable items drawn with 5
    const auto ultimo = m_itens.back();                          // shared_ptr copy (refcount ++/--)
    const api::TPosition x = ultimo->GetPosicao().x;
    const api::TPosition y = static_cast<api::TPosition>(ultimo->GetPosicao().y + 2 * ultimo->GetTamanhoFonte());
    m_builder.AddText("Escolha a sua opção:", {x, y}, FONTE_MENU /*@493212*/, 0, 2, 1);
    auto validacao = std::shared_ptr<api::IInputValidation>(new api::COptionValidation(m_opcoes));
    auto campo = std::shared_ptr<api::CInputField<api::CFramedText>>(
        new api::CInputField<api::CFramedText>(validacao, 1, true, true, false, false, false,   // wasm 2024(…, 1, 1,1,0,0,0)
                                               api::CFramedText(1, {static_cast<api::TPosition>(x + 208),
                                                                    static_cast<api::TPosition>(y - 2)},
                                                                FONTE_MENU, 0)));
    m_builder.Add(campo);                                                        // wasm 426
}

} // namespace comum
