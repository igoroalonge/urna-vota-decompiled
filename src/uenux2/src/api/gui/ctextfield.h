// Reconstructed from vota_web_wasm.wasm (unit u16). Original: uenux2/src/api/gui/ctextfield.h (path
// inferred from ctextfield.cpp, attested by the srcloc records :30 and :55).
//
// ---------------------------------------------------------------------------------------------------
// Interface recap (declared in iformfield.h / iscreen.h / ctextsource.h, other units; names aligned with
// gui-common.u15.h of unit u15: Invalidate, RedrawIfDirty, Union, SRect::MoveTo, ITimer::Start/Stop).
// Slot numbers are vtable slots; names are inferred unless marked "attested". SFont is written here as
// {TFontSize size (int16, +0), style (+4, 1 = bold)}: the compiled code stores only 16 bits at +0.
//
//   template <class MEDIA> class IFormFieldBase {          // vtable @1537360 for MEDIA = IScreen
//       // +0 vptr, +4 bool m_precisaRedesenhar ("changed": redraw pending), +8 IForm<MEDIA>* m_pForm,
//       // +12 std::string m_nome (unique "<ClassName><n>", set by CFormBuilder::Add, func 426)
//     0/1  virtual ~IFormFieldBase();
//     2    virtual void Draw(MEDIA&) const = 0;
//     3    virtual void Start() {}          // no-op; CTextFieldBlinking/CTextFieldUpdate start their timer
//     4    virtual void Stop() {}           // no-op; ... stop their timer
//     5    virtual bool RedrawIfDirty(MEDIA& m) { bool a = m_precisaRedesenhar; m_precisaRedesenhar = false; if (a) Draw(m); return a; }  // func 4118
//     6    virtual void SetForm(IForm<MEDIA>* f) { m_pForm = f; }                        // func 3037
//     7    virtual std::string GetClassName() const = 0;   // the literal class name ("CTextField")
//   protected:
//     void Invalidate() {           // always inlined:
//         if (m_pForm && m_pForm->IsVisible()) { m_precisaRedesenhar = true; m_pForm->Atualiza(); }  // form +4 under
//     }                                    // the form mutex (+28), IForm slot 4
//   };
//   IFormField<IScreen> adds: 8 virtual SRect Rect() const = 0 (attested); 9 virtual void Move(const SPoint&) = 0
//   (attested: CTextBox::Move). IFormField<IScreenMT> and IFormField<IPaper> stop at slot 7, and (unlike
//   IFormField<IScreen>) they implement it: GetClassName() returns the template's own name
//   ("IFormField<IScreenMT>", func 11051; "IFormField<IPaper>", func 11014). CTextFieldMT and CTextFieldPaper
//   do not override it.
//
//   IText (ctextsource.h): 2 std::string GetText() const; 3 ETextAlignment GetAlignment() const.
//   CFixedText : IText (vtable @1532648) = {vptr, ETextAlignment +4, std::string +8}.
//   using SharedIText = std::shared_ptr<IText> (attested in the srcloc signatures).
//   ETextAlignment: 0 Left, 1 Right (text ends at x), 2 Center (names inferred).
//
//   IScreen (the 640x480 voter display; simulador::CWasmScreen in the web build):
//     3  GetFontMetrics(TPosition& largura, TPosition& altura, const SFont&, const std::string&) (attested)
//     4  Clear(TColor)   5 ClearRect(const SRect&, TColor)   6 FillRect(const SRect&, TColor)
//     9  DrawRect(const SRect&, TColor, uebyte espessura)
//     17 FillPath(const std::vector<SPathElement>&, TColor)   18 DrawPath(..., TColor, uebyte espessura)
//     19 SRect WriteText(const SPoint&, const IText&, const SFont&, TColor frente, TColor fundo)
//     20 SRect WriteText(const SRect&, const IText&, const SFont&, TColor frente, TColor fundo)
//     21 void  WriteText(const SPoint&, const SRect& recorte, const IText&, const SFont&, TColor, TColor)
//     29 SetColors(TColor, TColor)  (?)   32 TPosition GetTextWidth(const std::string&, const SFont&)
//   IScreenMT (mesário micro-terminal, 4 x 40 characters): 3 Write(const SPoint& coluna/linha, const IText&)
//   IPaper (thermal printer): 2 Print(const IText&, IPaper::EStyle)
//   TColor: index into a 38-entry palette (simulador::CWasmScreen @1529596): 0 transparent, 1 white,
//   2 black, 3 #808080, 4 #d3d3d3, 13 #006400, 20 #ffd300, 36 #c9c9c9, ...
// ---------------------------------------------------------------------------------------------------
//
// api::CTextField : api::IFormField<api::IScreen>   (typeinfo 1582436, vtable @1582364, 64 bytes)
// A single line of text at a point. The most common field of the voting screens (via CFormBuilder).
// Vtable: 0 dtor (10950)  1 deleting (10948)  2 Draw (10952)  7 GetClassName (10947)  8 Rect (10951)
//         9 Move (shared body 2241: if (p != m_pos) { m_pos = p; Invalidate(); })
#pragma once

#include <string>

#include "api/gui/iformfield.h"
#include "api/gui/ctextsource.h"

namespace api {

class CTextField : public IFormField<IScreen> {
public:
    CTextField(const SPoint& pos, const SharedIText& texto, const SFont& fonte, TColor corTexto,
               TColor corFundo);                                          // wasm func 1918 (:30)
    ~CTextField() override;                                              // wasm func 10950 / 10948

    void Draw(IScreen& tela) const override;                             // wasm func 10952
    std::string GetClassName() const override;                           // wasm func 10947
    SRect Rect() const override;                                         // wasm func 10951 (:55)
    void Move(const SPoint& pos) override;                               // shared body 2241 (other unit)

private:
    SPoint        m_pos;                     // +24
    SharedIText   m_texto;                   // +28/+32
    SFont         m_fonte;                   // +36
    TColor        m_corTexto;                // +44
    TColor        m_corFundo;                // +48
    mutable SRect m_rectAnterior{};          // +52 area covered by the last Draw
    mutable bool  m_apagarAnterior = false;  // +60 erase m_rectAnterior before drawing (no writer found)
};

} // namespace api
