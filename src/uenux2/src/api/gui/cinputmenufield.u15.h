// FRAGMENT reconstructed by unit u15 from vota_web_wasm.wasm.
// Original: uenux2/src/api/gui/cinputmenufield.h (implementation: cinputmenufield.cpp, srclocs lines
// 49..477). A numbered menu for the voter screen: items "[<search text>] - <text>" in columns, an
// instruction line ("Digite a sua opção: ") and one CFramedText box where the voter types the item's
// number (search text). Used by the "visualizar candidatos" menus of vota.
#pragma once

#include <any>
#include <functional>
#include <list>
#include <memory>
#include <string>

#include "api/gui/cframedtext.u15.h"

namespace api {

class CInputMenuField;
class CMenuItem;
using TMenuCallback = std::function<void(CMenuItem&)>;

// Menu item: members end at +108, sizeof 112 (8-byte aligned because of std::function's buffer at +32;
// the std::list node is 120 bytes and the item starts at node+8):
class CMenuItem {
public:
    CMenuItem& SetSearchText(const std::string& texto);           // func 3655, cinputmenufield.cpp:142
    void SetFormatString(const std::string& formato);             // func 11884 (other unit), :156
    void UpdateRect();                                            // func 3657, :49
    void Draw() const;                                            // inlined in 10897, :74
    std::string GetDisplayText() const;                           // func 5492            name inferred
    ~CMenuItem();                                                 // func 5490            name inferred

    CInputMenuField* m_menu;          // +0
    int m_numero;                     // +4   1-based position in the menu
    std::string m_texto;              // +8   (%T)
    SPoint m_pos;                     // +20
    SFont m_fonte;                    // +24  copied from the menu (+96)
    TMenuCallback m_callback;         // +32  (24 bytes, __f_ at +48)
    SRect m_rect;                     // +56
    bool m_destacado = false;         // +64  highlighted (selected / blinking)
    bool m_visivel = true;            // +65  (false -> drawn in colour 3 and not selectable)
    std::string m_textoBusca;         // +68  (%S) what the voter types; default std::to_string(m_numero)
    std::any m_dado;                  // +80  user data (libc++ any: handler at +80)
    std::string m_formato;            // +96  per-item format; empty -> the menu's format
};

// CMenuValidation (vtable @1583308): valid characters "1234567890" + back pointer to the menu.
class CMenuValidation : public IInputValidation {
public:
    bool IsValid(const std::string& texto) const override;        // func 10900 (slot 2)
    bool IsValidChar(char c) const override;                      // func 10899 (slot 3, cinputmenufield.u32.cpp;
                                                                  // the base version is func 12473)
private:
    bool matchesExactly(const std::string& texto) const;          // :198 (inlined)
    bool matchesPartially(const std::string& texto) const;        // :210 (inlined)
    CInputMenuField* m_menu;                                      // +16
};

// CInputMenuField (typeinfo @1583572, vtable @1583364): CInputField<CFramedText> (92 bytes) +
//   +92  SPoint m_pos                    first item position
//   +96  SFont  m_fonte                  item and instruction font
//   +104 SRect  m_rectItens              bounding box of all items
//   +112 std::string m_textoInstrucao    default "Digite a sua opção: " (static @1832432)
//   +124 SRect  m_rectInstrucao
//   +132 SRect  (initialised to {pos,pos}; purpose unknown)                  ?
//   +140 std::list<CMenuItem> m_itens    (sentinel +140/+144, size +148)
//   +152 CMenuItem* m_selecionado
//   +156 TPosition m_alturaMaxima
//   +160 std::string m_formato           default "[%S] - %T"
class CInputMenuField : public CInputField<CFramedText> {
public:
    CInputMenuField(const SPoint& pos, const SFont& fonte, TPosition alturaMaxima);   // inlined in 3675
    ~CInputMenuField() override;                                                     // 5491 / 10898

    CMenuItem& AddItem(const std::string& texto, TMenuCallback callback);           // 1693, :298/:320/:326/:339
    void SetInstructionText(const std::string& texto);                               // :477 (inlined in 3675)
    void SetFormatString(const std::string& formato);                                // :468 (func 11884)

    void Draw(IScreen& tela) const override;                                         // 10897 (slot 2)
    std::string GetClassName() const override { return "CInputMenuField"; }          // 10893 (slot 7)
    SRect Rect() const override;                                                     // 10896 (slot 8)
    void Move(const SPoint& pos) override;                                           // 10895 (slot 9), :278
    EInputResult Read(IInput& entrada) override;                                     // 10894 (slot 10), :419

    static inline const std::string ms_textoInstrucaoPadrao = "Digite a sua opção: ";   // @1832432

private:
    friend class CMenuItem;
    friend class CMenuValidation;
    SPoint NextItemPosition() const;                  // :298 (inlined in AddItem)
    SRect GetInstructionTextRect() const;             // :251 (inlined)
    void SelecionaPorTextoBusca(const std::string& texto);   // func 5489            name inferred
    void LimpaSelecao();                              // inlined in Read/5489            name inferred
    void AtualizaLayout();                            // func 2770                       name inferred
    size_t MaiorTextoBusca() const;                   // inlined (AddItem, SetSearchText) name inferred

    SPoint m_pos;                                     // +92
    SFont m_fonte;                                    // +96
    SRect m_rectItens;                                // +104
    std::string m_textoInstrucao;                     // +112
    SRect m_rectInstrucao;                            // +124
    SRect m_rect132;                                  // +132  ?
    std::list<CMenuItem> m_itens;                     // +140
    CMenuItem* m_selecionado = nullptr;               // +152
    TPosition m_alturaMaxima;                         // +156
    std::string m_formato = "[%S] - %T";              // +160
};

} // namespace api
