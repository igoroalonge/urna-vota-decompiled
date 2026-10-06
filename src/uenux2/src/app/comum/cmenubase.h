// uenux2/src/app/comum/cmenubase.h (path inferred, as in unit u02's cmenubase.u02.cpp)
// Reconstructed from vota_web_wasm.wasm by unit u37 (destructors); AdicionaItem / Monta are unit u02's.
//
// comum::CMenuBase draws a numbered menu into an api::CFormBuilder:
//     [1] - Estado da urna (0/1)
//     [2] - Lista de eleitores (0/1)          <- grey when the item is not Disponivel()
//     ...
//     Escolha a sua opção: [_]                 <- 1-character input accepting only the available ids
// RTTI: comum::CMenuBase (class, typeinfo @1550944, vtable @1550932):
//   [0] ~CMenuBase() wasm 12235   [1] deleting dtor wasm 6188   [2] Monta() wasm 5913 (u02)
// Only subclass: vota::CMenuMaisInformacoesVota (vtable @1539820, same three slots, no extra member).
//
// Layout (32 bytes):
//   +0  vptr
//   +4  std::vector<std::shared_ptr<CItemMenu>> m_itens
//   +16 std::string m_opcoes        ids accepted by the COptionValidation of the input field
//   +28 api::CFormBuilder& m_builder
//
// Correction to u02's AdicionaItem (wasm 2285): the id is appended to m_opcoes only when
// item->Disponivel() (vtable slot 2) is true - not merely when the pointer is non-null. Monta (5913) also
// asks Disponivel() and uses colour 2 (available) or 5 (grey).
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "comum/citemmenu.h"

namespace api { class CFormBuilder; }

namespace comum {

class CMenuBase {
public:
    explicit CMenuBase(api::CFormBuilder& builder) : m_builder(builder) {}
    virtual ~CMenuBase();                                             // wasm 12235 / 6188

    void AdicionaItem(std::shared_ptr<CItemMenu> item);               // wasm 2285 (unit u02)
    virtual void Monta();                                             // wasm 5913 (unit u02)

private:
    std::vector<std::shared_ptr<CItemMenu>> m_itens;                  // +4
    std::string m_opcoes;                                             // +16
    api::CFormBuilder& m_builder;                                     // +28
};

} // namespace comum
