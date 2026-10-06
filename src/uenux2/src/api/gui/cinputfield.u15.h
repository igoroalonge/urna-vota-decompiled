// FRAGMENT reconstructed by unit u15 from vota_web_wasm.wasm.
// Original file: uenux2/src/api/gui/cinputfield.h (srcloc cinputfield.h:148 = SetLength), plus the
// Draw of CMaskedTextField<MASK> (header cmaskedtextfield.h, path inferred) whose two instantiations
// were assigned to this unit. The class declarations are in cframedtext.u15.h.
#pragma once

#include "api/gui/cframedtext.u15.h"
#include "api/pattern/cpolysingletonlist.h"

namespace api {

// wasm func 12438 (observed executing) - CInputField<CFramedText>::Draw, vtable slot 2
template <class MASK>
void CInputField<MASK>::Draw(IScreen& tela) const
{
    m_mascara.MaskText(tela, m_texto);
    // Blinking cursor: while focused, on the "off" phase the frame of the next free box is redrawn in
    // the background colour.
    if (m_foco && !m_cursorVisivel && m_texto.size() != m_tamanhoMaximo)
        m_mascara.DesenhaMoldura(m_texto.size(), /*fundo*/ 1, tela);                   // func 2779
}

// wasm func 12411 (observed executing) - slot 7: returns "CInputField" (func 2904 = string(first, last)).
// wasm func 12405 (observed executing) - slot 8: Rect() = m_mascara.Rect() (func 3677).

// wasm func 12402 (observed executing) - slot 9                                        // name inferred
template <class MASK>
void CInputField<MASK>::Move(const SPoint& pos)
{
    m_mascara.Move(pos);          // func 3676
    Invalidate();                 // no "same position" test, unlike IFormFieldBase's Move (func 2241)
}

// wasm func 12387 (not observed) - srcloc cinputfield.h:148, slot 12
template <class MASK>
void CInputField<MASK>::SetLength(size_t tamanho)
{
    if (tamanho == m_tamanhoMaximo)
        return;
    auto& tela = CPolySingletonList::instance<IScreen>();                              // srcloc :148
    tela.FillRect(Rect(), /*fundo*/ 1);           // erase the old boxes

    if (tamanho != m_mascara.m_digitos) {
        m_mascara.m_digitos = tamanho;
        const TPosition larguraTotal = static_cast<TPosition>(
            m_mascara.Espacamento() * (tamanho - 1) + tamanho * m_mascara.m_larguraCaixa);
        // Re-alignment is applied on top of the position that was already shifted for the old width
        // (no undo of the previous shift): harmless for left-aligned masks (the only use, CInputMenuField).
        switch (m_mascara.m_alinhamento) {
        case ETextAlignment::Right:  m_mascara.m_pos.x -= larguraTotal;     break;
        case ETextAlignment::Center: m_mascara.m_pos.x -= larguraTotal / 2; break;
        default: break;
        }
    }
    m_texto.clear();
    m_tamanhoMaximo = tamanho;
    Invalidate();
}

// wasm funcs 12593 / 12708 (both observed executing): CMaskedTextField<CFramedText>::Draw and
// CMaskedTextField<CGrayedFramedText>::Draw, defined inline in cframedtext.u15.h:
//     m_mascara.MaskText(tela, m_texto->GetText());     // IText slot 2 returns the string by value

} // namespace api
