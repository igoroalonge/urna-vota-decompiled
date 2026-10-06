// Reconstructed from vota_web_wasm.wasm (unit u16). Original: uenux2/src/api/gui/cprogressbar.cpp
// (srcloc records :52, :58, :67 in the constructor, which is inlined into CFormBuilder::AddProgressBar,
// wasm func 5545). Supersedes the fragment cprogressbar.u07.cpp (Incrementa) written by unit u07.
//
// Helpers used below that live in other files:
//   SRect::Adjusted(dl, dt, dr, db)   wasm func 2768 (primitives, see primitives.u16.cpp)
//   SRect::MoveTo(const SPoint&)      wasm func 1915 (primitives, unit u15)
//   SRect::Left(TPosition) / Right(TPosition)  wasm funcs 5488 / 5487 (primitives.cpp:20/:32, assert
//                                     "Assert (right >= l)" / "Assert (r >= left)")
//   CStringUtils::ReplaceAll(s, de, para)       wasm func 2677 (api/util)                 // name inferred
#include "api/gui/cprogressbar.h"

#include <algorithm>
#include <cmath>
#include <format>

#include "api/gui/ctextsource.h"      // api::CFixedText
#include "api/util/cstringutils.h"    // api::CStringUtils::ReplaceAll

namespace api {

// CUeGuiError = ecourna::api::exception::CBaseError<EUeGuiError> (SErrorLimits{4900, 5100}), gui-common.u15.h

// (inlined into wasm func 5545 = CFormBuilder::AddProgressBar; srcloc lines 52, 58, 67)
CProgressBar::CProgressBar(const uedword valorInicial, const uedword minimo, const uedword maximo,
                           const SRect& area, TColor corBarra, TColor corFundo, TColor corTextoPreenchido,
                           TColor corTexto, const std::string& formato, TFontSize tamanhoFonte, uebyte margem,
                           const TColor corBorda)
    : m_minimo(minimo),
      m_maximo(maximo),
      m_area(area),
      m_areaInterna(area.Adjusted(1, 1, -3, -3)),   // constants as compiled (margem 2 folded?)   // ?
      m_corBarra(corBarra),
      m_corFundo(corFundo),
      m_corTextoPreenchido(corTextoPreenchido),
      m_corTexto(corTexto),
      m_formato(formato),
      m_tamanhoFonte(tamanhoFonte),
      m_margem(margem),
      m_corBorda(corBorda)
{
    if (m_minimo >= m_maximo)                                                             // :52
        throw CUeGuiError(EUeGuiError{4944},
                       std::format("Limite inferior ({}) >= limite superior ({})", m_minimo, m_maximo));

    if (m_area.right - m_area.left <= 2 * m_margem)                                       // :58
        throw CUeGuiError(EUeGuiError{4945},
                       std::format("Largura da barra ({}) não comporta a margem ({})",
                                   m_area.right - m_area.left, m_margem));

    if (m_area.bottom - m_area.top <= 2 * m_margem)                                       // :67
        throw CUeGuiError(EUeGuiError{4946},
                       std::format("Altura da barra ({}) não comporta a margem ({})",
                                   m_area.bottom - m_area.top, m_margem));

    SetValor(valorInicial);   // inlined; a no-op for the callers of this binary (valor 0 == m_valor)

    if (m_tamanhoFonte == 0)  // automatic size: 80 % of the height left inside the border
        m_tamanhoFonte = static_cast<TFontSize>(std::floor((m_area.bottom - (m_area.top + 2 * m_margem)) * 0.8));
}

// wasm func 5509 (D1) / 10978 (D0 = D1 + operator delete)
CProgressBar::~CProgressBar() = default;   // destroys m_formato, then IFormFieldBase (m_nome)

// wasm func 10977 (vtable slot 2). Observed executing (the "Gravando" bar after each voter).
void CProgressBar::Draw(IScreen& tela) const
{
    // 1. Text: m_formato with "%p" -> "<percent>%" and "%v" -> "<valor>".
    std::string texto;
    if (!m_formato.empty()) {
        uedword percentual = 100;
        if (m_maximo != m_minimo)
            percentual = (m_valor - m_minimo) * 100 / (m_maximo - m_minimo);
        const std::string comPercentual =
            CStringUtils::ReplaceAll(m_formato, "%p", std::format("{}%", percentual));  // func 2677
        texto = CStringUtils::ReplaceAll(comPercentual, "%v", std::format("{}", m_valor));
    }

    // 2. Background and filled part.
    tela.ClearRect(m_area, m_corFundo);                                                  // IScreen slot 5
    const TPosition larguraInterna = m_areaInterna.right - m_areaInterna.left + 1;
    const int preenchido = static_cast<int>(std::trunc(
        static_cast<double>(static_cast<uedword>(larguraInterna * (m_valor - m_minimo))) /
        static_cast<double>(m_maximo - m_minimo)));
    if (preenchido > 0) {
        tela.FillRect(SRect(SPoint{m_areaInterna.left, m_areaInterna.top},                // IScreen slot 6
                            SPoint{static_cast<TPosition>(m_areaInterna.left + preenchido),
                                   static_cast<TPosition>(m_areaInterna.bottom + 1)}),
                      m_corBarra);
    }

    // 3. Text, centred in the inner area; two colours when the end of the fill cuts through it.
    if (!texto.empty()) {
        const SFont fonte{m_tamanhoFonte, 0};
        TPosition largura = 0, altura = 0;
        tela.GetFontMetrics(largura, altura, fonte, texto);                              // IScreen slot 3
        const CFixedText rotulo(ETextAlignment::Center, texto);

        const TPosition centroY = m_areaInterna.top + (m_areaInterna.bottom + 1 - m_areaInterna.top) / 2;
        const TPosition topo    = centroY - altura / 2 - 1;
        const TPosition base    = centroY + altura / 2 + 1;
        const TPosition centroX = m_areaInterna.left + larguraInterna / 2;
        const TPosition esquerda = centroX - largura / 2 - 1;
        const TPosition direita  = centroX + largura / 2 + 1;
        const TPosition fimPreenchido = m_areaInterna.left + preenchido;
        const SPoint pos{centroX, static_cast<TPosition>(topo + 2)};

        if (fimPreenchido <= esquerda) {
            tela.WriteText(pos, rotulo, fonte, m_corTexto, TColor{0});                  // slot 19
        } else if (direita <= fimPreenchido) {
            tela.WriteText(pos, rotulo, fonte, m_corTextoPreenchido, TColor{0});
        } else {
            // Left part over the bar, right part over the background (IScreen slot 21 = text clipped to
            // a rectangle). NOTE: simulador::CWasmScreen::vf21 (func 9090) ignores the clip rectangle and
            // forwards to vf19 -> vf20, which js_fill's the whole text box when the background colour is not
            // 0. So in the web build the first call paints the whole text box in m_corBarra (the green
            // extends past the end of the bar) and the second call repaints the whole text in m_corTexto.
            SRect recorte{esquerda, topo, direita, base};
            recorte.Right(fimPreenchido);                                                // func 5487
            tela.WriteText(pos, recorte, rotulo, fonte, m_corTextoPreenchido, m_corBarra);
            recorte = SRect{esquerda, topo, direita, base};
            recorte.Left(fimPreenchido);                                                 // func 5488
            tela.WriteText(pos, recorte, rotulo, fonte, m_corTexto, TColor{0});
        }
    }

    // 4. Border, `m_margem` pixels wide, centred on the outer rectangle.
    const TPosition meio = m_margem >> 1;
    tela.DrawRect(m_area.Adjusted(meio, meio, -meio, -meio), m_corBorda, m_margem);     // IScreen slot 9
}

// wasm func 10975 (slot 7)
std::string CProgressBar::GetClassName() const
{
    return "CProgressBar";
}

// wasm func 10974 (slot 8)
SRect CProgressBar::Rect() const
{
    return m_area;
}

// wasm func 10976 (slot 9). Only the outer rectangle moves: m_areaInterna (where the bar and the text are
// drawn) keeps its old position. Harmless here because no caller moves a progress bar.
void CProgressBar::Move(const SPoint& pos)
{
    if (pos.x == m_area.left && pos.y == m_area.top)
        return;
    m_area.MoveTo(pos);                                                                  // func 1915
    Invalidate();   // inlined IFormFieldBase helper: if the form is visible, m_precisaRedesenhar = true and
                           // IForm::Atualiza() (IForm slot 4)
}

// wasm func 3667 (attributed to unit u37; callers vota::CSincronismoEleitor::StartState and
// vota::CProgressoEncerramento)                                                         // name inferred
void CProgressBar::SetValor(uedword valor)
{
    if (valor == m_valor)
        return;
    m_valor = valor;
    if (valor < m_minimo)
        m_valor = m_minimo;
    else if (valor > m_maximo)
        m_valor = m_maximo;
    Invalidate();
}

// wasm func 5508 (attributed to unit u07; see cprogressbar.u07.cpp). One step of the bar.   // name inferred
void CProgressBar::Incrementa()
{
    m_valor = std::clamp(m_valor + 1, m_minimo, m_maximo);
    Invalidate();
}

} // namespace api
