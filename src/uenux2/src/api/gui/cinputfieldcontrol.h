// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/gui/cinputfieldcontrol.h (path inferred).
//
// An invisible input field: it only exists to receive the control keys (CONFIRMA, CORRIGE, BRANCO) of a
// form that has no visible input (information screens, "confirma?" screens). Maximum length 0 and a
// CControlValidation (no accepted characters).
//
//   api::CInputFieldControlBase<MEDIA> : IInputField<MEDIA>        (IScreen: typeinfo 1577404, vtable @1577424)
//   api::CInputFieldControl<MEDIA>     : CInputFieldControlBase<MEDIA>
//     IScreen   vtable @1577340 (13 slots), built by CInteractiveFormBuilder::AddLabeledInputControl (653),
//               AddControlInput (901) and func 5530
//     IScreenMT vtable @1580456 (11 slots), built by funcs 395 and 5410 (mesário screens)
// In CInputFieldControlBase<IScreen> slots 7..9 are still pure: GetClassName/Rect/Move are declared by
// CInputFieldControl. For IScreenMT there are no geometry slots and slot 7 keeps the default of
// IFormField<IScreenMT> ("IFormField<IScreenMT>", func 11051).
// The constructor of the base (func 5565 for IScreen) is in cinteractiveformbuilder.u15.cpp (unit u15).
#pragma once

#include <string>

#include "api/gui/iinputfield.u17.h"

namespace api {

template <class MEDIA>
class CInputFieldControlBase : public IInputField<MEDIA> {
public:
    explicit CInputFieldControlBase(unsigned teclas);          // func 5565 (IScreen), see unit u15
};

template <class MEDIA>
class CInputFieldControl : public CInputFieldControlBase<MEDIA> {
public:
    using CInputFieldControlBase<MEDIA>::CInputFieldControlBase;

    void Draw(MEDIA&) const override {}                        // slot 2: ICF 425 (nothing to draw)
};

template <>
class CInputFieldControl<IScreen> : public CInputFieldControlBase<IScreen> {
public:
    using CInputFieldControlBase<IScreen>::CInputFieldControlBase;

    void Draw(IScreen&) const override {}                      // slot 2: ICF 425

    // wasm func 11155 (slot 7; observed executing - every voter screen has one)
    std::string GetClassName() const override
    {
        return "CInputFieldControl";                           // 18-byte string helper func 3891
    }

    // wasm func 11153 (slot 8): an invisible field occupies no area.
    SRect Rect() const override { return SRect{}; }            // 8 zero bytes

    void Move(const SPoint&) override {}                       // slot 9: ICF 425
};

} // namespace api
