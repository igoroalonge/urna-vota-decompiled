// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/gui/iformfield.h (path inferred: no srcloc names it, but every field class of
// uenux2/src/api/gui derives from these templates and units u16/u17 already include this header name).
//
// RTTI (all "class" typeinfos, i.e. no base):
//   api::IFormFieldBase<api::IScreen>    typeinfo 1537344, vtable @1537360 (8 slots)
//   api::IFormFieldBase<api::IScreenMT>  typeinfo 1579636, vtable @1579652 (8 slots)
//   api::IFormFieldBase<api::IPaper>     typeinfo 1580852, vtable @1580868 (8 slots)
//   api::IFormField<MEDIA> : IFormFieldBase<MEDIA>   (typeinfos 1537332 / 1579624 / 1580840; no vtable of
//                                                     their own is emitted - the classes are abstract)
//
// A "form field" is one widget of a screen (IScreen = the 640x480 voter display), of the poll worker's
// microterminal (IScreenMT = 4 x 40 LCD, "terminal do mesário") or of a printed report (IPaper = the thermal
// printer: BU, zerésima, relatórios). The form (api::IForm<MEDIA>, iform.h) owns a vector of
// shared_ptr<IFormField<MEDIA>> and drives the virtual protocol below.
#pragma once

#include <memory>
#include <mutex>
#include <string>

#include "api/gui/primitives.h"     // SPoint, SRect, TColor, ETextAlignment (declared in gui-common.u15.h by u15;
                                     // that recap also repeats IFormFieldBase/IInputValidation, which are
                                     // defined here and in iinputvalidation.h)

namespace api {

class IScreen;
class IScreenMT;
class IPaper;
template <class MEDIA> class IForm;

// ------------------------------------------------------------------------------------------------------
// Layout (24 bytes):
//   +0  vptr
//   +4  bool          m_precisaRedesenhar   dirty flag ("redraw pending"), consumed by RedrawIfDirty
//   +8  IForm<MEDIA>* m_form                set by SetForm (slot 6); the form keeps "active" at +4 and
//                                           its mutex at +28, its slot 4 is Redraw()
//   +12 std::string   m_nome                unique name "<GetClassName()><n>" given by CFormBuilder::Add
//                                           (IScreen only; stays empty for MT and paper fields)
//
// Virtual protocol:
//   0/1 destructors                       2 Draw(MEDIA&) const = 0
//   3 Start() {} (ICF 218)                4 Stop() {} (ICF 218)
//   5 RedrawIfDirty(MEDIA&) (func 4118)   6 SetForm(IForm<MEDIA>*) (func 3037)
//   7 GetClassName() const = 0
// ------------------------------------------------------------------------------------------------------
template <class MEDIA>
class IFormFieldBase {
public:
    // wasm func 12658 (MEDIA = IScreen), 11052 (IScreenMT), 11013 (IPaper) - vtable slot 0.
    // Each is a 12-byte thunk into the merged body api_f1566 (store the IFormFieldBase<MEDIA> vptr, free
    // m_nome if it is a long string, return this). Because nothing else needs destroying, the same function
    // is also slot 0 of every field whose own members are trivial:
    //   12658: CRectField, CLineField, CFillField
    //   11052: CBeepFieldMT, CBuzzFieldMT, CClockFieldMT, CLedFieldMT
    //   11013: CCutFieldPaper, CNewLineFieldPaper
    // Slot 1 (deleting destructor) is ICF 325 (`unreachable`) in the abstract bases.
    virtual ~IFormFieldBase() = default;

    virtual void Draw(MEDIA& media) const = 0;                           // slot 2

    virtual void Start() {}                                              // slot 3 (fields with timers)
    virtual void Stop() {}                                               // slot 4

    // slot 5 - wasm func 4118 (other unit)
    virtual bool RedrawIfDirty(MEDIA& media)
    {
        const bool redesenhar = m_precisaRedesenhar;
        m_precisaRedesenhar = false;
        if (redesenhar)
            Draw(media);
        return redesenhar;
    }

    // slot 6 - wasm func 3037 (other unit)
    virtual void SetForm(IForm<MEDIA>* form) { m_form = form; }

    virtual std::string GetClassName() const = 0;                        // slot 7

    const std::string& GetNome() const { return m_nome; }                // name inferred
    void SetNome(const std::string& nome) { m_nome = nome; }             // name inferred

protected:
    // The "notify the form" idiom, always inlined (6312, 2783, 5527, 10542, ...):
    //   if (m_form) { lock(form mutex); bool ativo = form +4; unlock; if (ativo) { dirty = true; form->Redraw(); } }
    // (in this build the lock is compiled to the no-op pthread stub, func 150).
    void Invalidate()
    {
        if (m_form == nullptr)
            return;
        if (m_form->IsActive()) {                                        // form +4 under form mutex +28
            m_precisaRedesenhar = true;
            m_form->Redraw();                                            // IForm slot 4
        }
    }

    bool          m_precisaRedesenhar = false;   // +4
    IForm<MEDIA>* m_form = nullptr;              // +8
    std::string   m_nome;                        // +12
};

// ------------------------------------------------------------------------------------------------------
// IFormField<IScreen> adds the geometry of the voter display:
//   slot 8 SRect Rect() const = 0, slot 9 void Move(const SPoint&) = 0.
// IFormField<IScreenMT> / IFormField<IPaper> stop at slot 7 and give GetClassName a default that returns
// the name of the template itself (shared bodies of other units):
//   func 11051 -> "IFormField<IScreenMT>"  (CBeepFieldMT, CBuzzFieldMT, CClockFieldMT, CLedFieldMT,
//                                            CTextFieldMT, CTextFieldUpdateMT, CInputFieldMT, ...)
//   func 11014 -> "IFormField<IPaper>"     (CCutFieldPaper, CNewLineFieldPaper, CQRCodeImageFieldPaper,
//                                            CTextFieldPaper)
// These two bodies are never reached through the builders: only the IScreen builder (CFormBuilder::Add,
// func 426) calls slot 7 to build the "<GetClassName()><n>" names. The microterminal and paper builders
// (funcs 728, 941, 1072/3890, 1151/3674, 1152/5409 -> 1400, 198/3890, 1264, 2775) only push the
// shared_ptr into the field vector, so MT and paper fields keep an empty m_nome.
// ------------------------------------------------------------------------------------------------------
template <class MEDIA>
class IFormField : public IFormFieldBase<MEDIA> {
public:
    std::string GetClassName() const override;                           // 11051 / 11014 (see above)
};

template <>
class IFormField<IScreen> : public IFormFieldBase<IScreen> {
public:
    virtual SRect Rect() const = 0;                                      // slot 8 (attested by CTextField)
    virtual void Move(const SPoint& pos) = 0;                            // slot 9
};

template <>
inline std::string IFormField<IScreenMT>::GetClassName() const { return "IFormField<IScreenMT>"; }   // 11051
template <>
inline std::string IFormField<IPaper>::GetClassName() const { return "IFormField<IPaper>"; }         // 11014

// A pre-show hook runs before the fields are drawn when a form becomes active (IForm slot 7).
// RTTI: api::IPreShow<api::IScreen> / <api::IScreenMT> ("class", no base).
template <class MEDIA>
class IPreShow {
public:
    virtual ~IPreShow() = default;                                       // slots 0/1 (ICF 174 / 144)
    virtual void PreShow(MEDIA& media) = 0;                              // slot 2
};

using SharedIFormFieldScreen = std::shared_ptr<IFormField<IScreen>>;     // name inferred

} // namespace api
