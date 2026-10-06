// FRAGMENT reconstructed by unit u15 from vota_web_wasm.wasm.
// Original: uenux2/src/api/gui/cframedtext.h, cgrayedframedtext.h, cinputfield.h, cmaskedtextfield.h.
#pragma once

#include <memory>
#include <string>

#include "api/gui/gui-common.u15.h"

namespace api {

class IScreen;
class IText;

// The row of digit boxes of the voting screens (candidate number, title number...).
// vtable @1578700, 28 bytes:
//   +4  size_t   m_digitos        number of boxes
//   +8  SPoint   m_pos            top-left of the first box (after alignment)
//   +12 SFont    m_fonte
//   +20 ETextAlignment m_alinhamento
//   +24 uebyte   m_larguraCaixa   max char width of the font + 2*espacamento
//   +25 uebyte   m_alturaCaixa    max char height + 2 (or 2*(size/10) for fonts >= 10)
// espacamento(i) = size <= 7 ? 1 : size / 8, box i spans
//   x = m_pos.x + (m_larguraCaixa + espacamento) * i .. + m_larguraCaixa,  y = m_pos.y .. + m_alturaCaixa.
class CFramedText {
public:
    CFramedText(size_t digitos, const SPoint& pos, const SFont& fonte, ETextAlignment alinhamento); // 1385
    virtual ~CFramedText() = default;
    virtual void MaskText(IScreen& tela, const std::string& texto) const;                           // 2780
    void Move(const SPoint& pos);            // func 3676 (other unit)
    SRect Rect() const;                      // func 3677 (other unit)
protected:
    TPosition Espacamento() const { return m_fonte.size <= 7 ? 1 : static_cast<TPosition>(m_fonte.size / 8); }
    SRect CaixaRect(size_t i) const;                                  // inlined everywhere          name inferred
    void PreencheCaixa(size_t i, TColor cor, IScreen& tela) const;    // func 5536: FillRect(box, cor) name inferred
    void DesenhaCaracter(size_t i, char c, IScreen& tela) const;      // func 5537 (tools: SRect::Top)  name inferred
    void DesenhaMoldura(size_t i, TColor cor, IScreen& tela) const;   // func 2779: DrawRect(box, cor, 1) name inferred

    friend class CInputMenuField;
    template <class> friend class CInputField;
    size_t m_digitos;                 // +4
    SPoint m_pos;                     // +8
    SFont m_fonte;                    // +12
    ETextAlignment m_alinhamento;     // +20
    unsigned char m_larguraCaixa = 0; // +24
    unsigned char m_alturaCaixa = 0;  // +25
};

// vtable @1578752. Same boxes; typed digits on white, unused boxes filled grey (colour 5).
class CGrayedFramedText : public CFramedText {
public:
    CGrayedFramedText(size_t digitos, const SPoint& pos);   // func 5535 (unit u07)
    void MaskText(IScreen& tela, const std::string& texto) const override;   // 5534
};

// Display-only field that shows an IText through a mask (CFramedText / CGrayedFramedText).
// Layout used by slot 2: +24 MASK m_mascara, +52 IText* m_fonte (slot 2 of IText = GetText()).
template <class MASK>
class CMaskedTextField : public IFormField<IScreen> {
public:
    void Draw(IScreen& tela) const override { m_mascara.MaskText(tela, m_texto->GetText()); }  // 12593 / 12708
private:
    MASK m_mascara;                   // +24
    std::shared_ptr<IText> m_texto;   // +52
};

// IInputField<IScreen> (64 bytes) + CFramedText m_mascara at +64 (92 bytes); vtable @1538768.
template <class MASK>
class CInputField : public IInputField<IScreen> {
public:
    void Draw(IScreen& tela) const override;                          // slot 2  (12438)
    std::string GetClassName() const override { return "CInputField"; }   // slot 7 (12411)
    SRect Rect() const override { return m_mascara.Rect(); }          // slot 8  (12405)
    void Move(const SPoint& pos) override;                            // slot 9  (12402)
    void SetLength(size_t tamanho) override;                          // slot 12 (12387), cinputfield.h:148
protected:
    MASK m_mascara;                   // +64
};

} // namespace api
