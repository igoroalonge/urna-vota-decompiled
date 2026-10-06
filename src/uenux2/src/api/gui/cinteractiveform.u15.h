// FRAGMENT reconstructed by unit u15 from vota_web_wasm.wasm.
// Original file: uenux2/src/api/gui/cinteractiveform.h (srclocs :57 Read(), :62 WaitAndRead(),
// :148 ClearKeyboardInput()). Two instantiations exist: <IScreen, IInputKbd> (voter screen + urna
// keyboard, vtable @1579176) and <IScreenMT, IInputMT> (microterminal of the poll worker, vtable @1580288).
// Read() itself is always inlined into the vota states that call it (see u15-foreign-fragments.cpp).
#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "api/gui/gui-common.u15.h"
#include "api/pattern/cpolysingletonlist.h"

namespace api {

// IForm<MEDIA> virtual protocol (vtable IForm<IScreen> @1578040):
//   slot 0/1 dtor
//   slot 2  Show()      - clear the global form stack (@1832632 for IScreen, @1832892 for IScreenMT),
//                          push this form and activate it (func 5541 / 5520)                  name inferred
//   slot 3  ShowOnTop() - deactivate the current top, move this form to the top, activate it and
//                          redraw (func 5540 / 5519)                                          name inferred
//   slot 4  Redraw()    - if active: every field RedrawIfDirty(media), then media slot 27 (flush)
//   slot 5  MEDIA& GetRenderForm() const
//   slot 6  SetFocus(IInputField*) - no-op in IForm, overridden below
//   slot 7  OnActivate() - start the fields (IForm::vf7 = func 5539 / 5518; 11043 is the MT slot 3)  name inferred
//
// CInteractiveForm layout (92 bytes for <IScreen, IInputKbd>):
//   +72 std::vector<IInputField<MEDIA>*> m_entradas      input fields, in form order
//   +84 size_t m_entradaAtual                             index of the focused field
//   +88 bool m_limpaTeclado                               flush the keyboard buffer on Show (ctor flag)
template <class MEDIA, class INPUT>
class CInteractiveForm : public IForm<MEDIA> {
public:
    ~CInteractiveForm() override = default;       // 11079/11078 (IScreen), 11032/11031 (IScreenMT)

    // slot 2 - funcs 11077 (IScreen, observed executing) / 11030 (IScreenMT)
    void Show() override
    {
        ResetEntradas();
        IForm<MEDIA>::Show();                     // func 5541 / 5520
        ClearKeyboardInput();
    }

    // slot 3 - funcs 11075 (IScreen) / 11029 (IScreenMT)
    void ShowOnTop() override
    {
        ResetEntradas();
        IForm<MEDIA>::ShowOnTop();                // func 5540 / 5519
        ClearKeyboardInput();
    }

    // slot 6 - func 5529 (ICF body shared by both instantiations)
    void SetFocus(IInputField<MEDIA>* campo) override
    {
        auto it = std::find(m_entradas.begin(), m_entradas.end(), campo);
        if (it != m_entradas.end())
            SetFocus(static_cast<size_t>(it - m_entradas.begin()));
    }

    // slot 7 - funcs 11074 (IScreen, observed executing) / 11028 (IScreenMT)
    void OnActivate() override
    {
        IForm<MEDIA>::OnActivate();               // func 5539 (IScreen) / 5518 (IScreenMT)
        SetFocus(m_entradaAtual);
    }

    EInputResult Read();                          // cinteractiveform.h:57, always inlined (see below)

protected:
    // wasm func 1401 (tools: api_f1401; observed executing; the body is shared by both
    // instantiations - identical layouts)                                         name inferred
    void SetFocus(size_t indice)
    {
        if (indice >= m_entradas.size())
            return;
        if (m_entradaAtual < m_entradas.size() && indice != m_entradaAtual)
            m_entradas[m_entradaAtual]->PerdeFoco();   // inlined, see below
        m_entradas[indice]->GanhaFoco();               // inlined, see below
        m_entradaAtual = indice;
    }
    // Inlined IInputField helpers used above (+44 m_tamanhoMaximo, +48 m_foco, +49 m_cursorVisivel,
    // +56 m_timerCursor):
    //   PerdeFoco(): if (m_foco && m_tamanhoMaximo) { m_foco = false;
    //                   if (m_timerCursor->IsRunning()) { m_timerCursor->Stop(); m_cursorVisivel = true; Invalidate(); } }
    //   GanhaFoco(): if (!m_foco && m_tamanhoMaximo) { m_foco = true;
    //                   if (m_timerCursor->IsRunning()) { m_timerCursor->Stop(); m_cursorVisivel = true; Invalidate(); }
    //                   else m_timerCursor->Start(); }

private:
    // Every input field is cleared (IInputField slot 11 for IScreen, slot 9 for IScreenMT fields) and
    // the focus goes back to the first one.
    void ResetEntradas()
    {
        for (auto* campo : m_entradas)
            campo->Clear();
        SetFocus(0);
    }

    // srcloc cinteractiveform.h:148 (inlined in the four Show/ShowOnTop functions)
    void ClearKeyboardInput()
    {
        if (m_limpaTeclado)
            CPolySingletonList::instance<INPUT>().Flush();   // IInputKbd / IInputMT slot 4
    }

    std::vector<IInputField<MEDIA>*> m_entradas;   // +72
    size_t m_entradaAtual = 0;                      // +84
    bool m_limpaTeclado = false;                    // +88
};

// cinteractiveform.h:57 (inlined in ~50 vota functions, e.g. funcs 10588 and 10624 of this unit):
template <class MEDIA, class INPUT>
EInputResult CInteractiveForm<MEDIA, INPUT>::Read()
{
    auto& entrada = CPolySingletonList::instance<INPUT>();          // srcloc :57
    return m_entradas.at(m_entradaAtual)->Read(entrada);            // IInputField slot 8 (MT) / 10
}

} // namespace api
