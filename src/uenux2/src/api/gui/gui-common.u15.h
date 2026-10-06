// FRAGMENT reconstructed by unit u15 from vota_web_wasm.wasm.
// Shared declarations used by the u15 GUI files (cframedtext, cimagefield, cinputmenufield, ...).
// The real declarations live in uenux2/src/api/gui/{primitives.h, iform.h, iformfield.h, iinputfield.h,
// iscreen.h} and uenux2/src/api/util/itimerscheduler.h; this header only records what u15 needed to
// write legible code, with the wasm32 layout observed in the constructors/destructors.
#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>

#include "ecourna/api/exception/cbaseerror.hpp"

namespace api {

using TPosition = std::int16_t;

struct SPoint {                 // 4 bytes, passed as one i32 (x = low 16 bits, y = high 16 bits)
    TPosition x;                // +0
    TPosition y;                // +2
    bool operator==(const SPoint&) const = default;
};

struct SRect {                  // 8 bytes, passed as one i64
    TPosition left;             // +0
    TPosition top;              // +2
    TPosition right;            // +4 (inclusive)
    TPosition bottom;           // +6 (inclusive)

    // wasm func 1915 (tools: api_f1915)                                    // name inferred
    // Moves the rectangle so that its top-left corner is `p`, keeping width and height.
    void MoveTo(const SPoint& p)
    {
        right  = static_cast<TPosition>(right + p.x - left);
        const TPosition oldTop = top;
        top    = p.y;
        left   = p.x;
        bottom = static_cast<TPosition>(bottom + p.y - oldTop);
    }
};

// wasm func 1916 (tools: api_f1916)                                        // name inferred
// Bounding box of two rectangles; the result is normalised (left <= right, top <= bottom).
SRect Union(const SRect& a, const SRect& b);

struct SFont {                  // 8 bytes; e.g. @520920 = {20, 0}, @474896 = {20, 0}, @475008 = {15, 0},
    std::int32_t size;          // +0       @475016 = {35, 0}
    std::int32_t style;         // +4 (0 = normal, 1 = bold?)                // ?
};

// ETextAlignment (values from CFramedText's constructor and CFormBuilder::GetLabeledKeyPos):
enum class ETextAlignment : int { Left = 0, Right = 1, Center = 2 };        // names inferred
// EAnchorPoint: 0 = top-left, 1 = top-right (QR code of the urna-state screen at x = 626) ...
enum class EAnchorPoint : int { TopLeft = 0, TopRight = 1 /*, ... */ };      // names inferred

// Colour indices used by the u15 code (palette of IScreen; names inferred from use):
//   1 = background (erase), 2 = normal frame/text, 3 = highlighted frame / selected item,
//   5 = grey fill (unused digit boxes of CGrayedFramedText).
using TColor = int;

// api::EUeGuiError: error codes 4900..5100 (SErrorLimits{4900, 5100}); typeinfo @1529376, vtable @1529396.
// wasm func 406 (tools: api_f406) is the merged constructor thunk
//     return ecourna_f710(mem, code, std::move(msg), srcloc, vtable CBaseError<EUeGuiError> @1529396);
// i.e. every `throw CUeGuiError(code, msg)` of the api/gui layer goes through it.
// The alias name CUeGuiError is inferred: RTTI only knows the thrown type
// CBaseError<api::EUeGuiError, SErrorLimits{4900, 5100}> (there is no derived class, unlike
// api::CUeDesligandoError), so func 406 is that class's constructor thunk.
enum class EUeGuiError : int;
using CUeGuiError = ecourna::api::exception::CBaseError<EUeGuiError /*, SErrorLimits{4900, 5100}*/>;

// api::EInputResult values used here (names shared with units u02/u06/u07; the meaning of 3 comes from
// IInputField::Read, func 6319: BRANCO ('B') pressed on an empty field):
// 13 ("Tecla" here, name inferred) is what func 6319 returns when the input buffer runs out of keys
// before a terminating key: the text may have changed but the input is not finished.
enum class EInputResult : int { Branco = 3, Corrige = 5, Confirma = 9, Tecla = 13 };

// ---------------------------------------------------------------------------------------------
// Timer interface (vtable of simulador::CWasmTimer @1532088 / api::CTimer @1585360)
class ITimer {
public:
    virtual ~ITimer();                              // slots 0/1
    virtual void Start() = 0;                       // slot 2
    virtual void Stop() = 0;                        // slot 3
    virtual bool IsRunning() const = 0;             // slot 4
    virtual void SetInterval(std::int64_t ms) = 0;  // slot 5 (CWasmTimer only)
};

// ---------------------------------------------------------------------------------------------
// IFormFieldBase<MEDIA> (vtable IFormFieldBase<IScreen> @1537360) - 24 bytes:
//   +0  vptr
//   +4  bool m_precisaRedesenhar      (dirty flag, consumed by slot 5)
//   +8  FormControlBlock* m_form      (set by slot 6; +4 byte = "form active", +28 = mutex,
//                                      vtable slot 4 = request redraw)
//   +12 std::string m_nome            (unique name given by CFormBuilder::Add, wasm func 426)
// Virtual protocol (all field classes of this unit follow it):
//   slot 0/1  destructors
//   slot 2    void Draw(MEDIA&) const
//   slot 3    void Start()             (fields with timers/animations; default no-op)   name inferred
//   slot 4    void Stop()              (default no-op)                                   name inferred
//   slot 5    bool RedrawIfDirty(MEDIA&)  (func 4118: swap m_precisaRedesenhar=false, Draw if it was set)
//   slot 6    void SetForm(FormControlBlock*) (func 3037)
//   slot 7    std::string GetClassName() const (e.g. "CImageField")
//   slot 8    SRect Rect() const
//   slot 9    void Move(const SPoint&)  (func 2241 for most fields)
// Input fields (IInputField<MEDIA>, vtable @1538852) add:
//   slot 10   EInputResult Read(IInput&)   (func 6319, named IInput::GetKey by the tools)
//   slot 11   void Clear()                  (func 6317: empties the typed text)
//   slot 12   void SetLength(size_t)
//
// The recurring "Invalidate" idiom (inlined everywhere):
//     if (m_form) { lock(m_form->mutex); if (m_form->ativo) { m_precisaRedesenhar = true; m_form->RequestRedraw(); } }
template <class MEDIA>
class IFormFieldBase {
public:
    virtual ~IFormFieldBase();
    virtual void Draw(MEDIA& media) const = 0;
    virtual void Start() {}
    virtual void Stop() {}
    virtual bool RedrawIfDirty(MEDIA& media);
    virtual void SetForm(class FormControlBlock* form);
    virtual std::string GetClassName() const = 0;
    virtual SRect Rect() const = 0;
    virtual void Move(const SPoint& pos) = 0;
protected:
    void Invalidate();                                  // inlined idiom described above
    bool m_precisaRedesenhar = false;                   // +4
    class FormControlBlock* m_form = nullptr;           // +8
    std::string m_nome;                                 // +12
};

template <class MEDIA> class IFormField : public IFormFieldBase<MEDIA> {};

// IInputField<MEDIA> - 64 bytes (constructor = wasm func 2024, see cformbuilder.u02.cpp):
//   +24 std::string m_texto                          typed text
//   +36 std::shared_ptr<IInputValidation> m_validacao
//   +44 size_t m_tamanhoMaximo
//   +48 bool m_foco                                  (field owns the keyboard focus)      name inferred
//   +49 bool m_cursorVisivel                         (blink phase)                        name inferred
//   +50..+54 five constructor flags (+52 = "CONFIRMA ends input" ...)
//   +55 char m_ultimaTecla
//   +56 std::shared_ptr<ITimer> m_timerCursor        (600 ms blink timer)

class IInputValidation {                               // vtable @1538704
public:
    virtual ~IInputValidation();
    // slot 2 is __cxa_pure_virtual in vtable @1538704. Func 4022 ("every char passes IsValidChar", empty
    // text = valid) is the ICF body shared by the overrides of CNumberValidation, COptionValidation and
    // CControlValidation; CMenuValidation overrides it with func 10900.
    virtual bool IsValid(const std::string& texto) const = 0;
    virtual bool IsValidChar(char c) const;                  // slot 3 (func 12473: memchr in m_caracteres)
protected:
    std::string m_caracteres;                                // +4
};

} // namespace api
