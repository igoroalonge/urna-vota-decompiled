// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/gui/cinputfieldmt.cpp (path inferred).
#include "api/gui/cinputfieldmt.h"

#include <string>

#include "api/gui/cfixedtext.h"
#include "api/gui/iscreen.h"

namespace api {

// wasm func 10726 (slot 2)
// Renders the input as "<digits>____" padded with '_' up to the maximum length, e.g. "0123____________".
// The blinking cursor is the first free '_': while the field has the focus, on the "off" phase of the 600 ms
// blink timer it is replaced by a space.
void CInputFieldMT::Draw(IScreenMT& tela) const
{
    const std::size_t maximo = m_tamanhoMaximo;
    const std::string texto = m_texto;

    std::string exibido = m_mascarado ? std::string(texto.size(), '*') : texto;
    if (exibido.size() < maximo)                                         // func 3511 (pad-right helper)
        exibido.append(maximo - exibido.size(), '_');

    if (m_foco && texto.size() < maximo && !m_cursorVisivel)
        exibido[texto.size()] = ' ';

    tela.Write(m_pos, CFixedText(ETextAlignment::Left, exibido));       // IScreenMT slot 3
}

} // namespace api
