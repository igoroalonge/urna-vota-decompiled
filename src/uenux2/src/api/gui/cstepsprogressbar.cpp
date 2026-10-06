// Reconstructed from vota_web_wasm.wasm (unit u16). Original: uenux2/src/api/gui/cstepsprogressbar.cpp
// (srcloc record :428 in the constructor, inlined into vota::CPreShowProgressBar::PreShow, wasm func 13550).
//
// How the bar is laid out (all of it runs inside Draw the first time, then the segments are cached):
//   1. two strategies are tried, each from a start font size down to 10 (TentaLayout, func 5507):
//        CabeLarguraIgual        (func 10972, table slot 3442): all segments equally wide;
//        CabeLarguraProporcional (func 10971, table slot 3443, listed in unit u34): minimum width per label,
//                                 remaining space shared in proportion to the label widths;
//      the one that fits with the larger font wins (ties -> equal widths); if none fits even at size 10,
//      equal widths are used and the labels are dropped (segments without text);
//   2. each segment becomes a closed path: rounded outer ends, arrow tip on the right of every segment but
//      the last, notch on the left of every segment but the first. Neighbouring segments overlap by
//      PROFUNDIDADE_SETA - ESPACO = 6 px so that the tip of one sits in the notch of the next.
// The path elements follow Qt's QPainterPath (moveTo/lineTo/arcTo/closeSubpath; the web glue js_path even
// calls the angles "qtStartAngle"/"qtSweepAngle"), so on the urna IScreen is probably backed by Qt.
#include "api/gui/cstepsprogressbar.h"

#include <algorithm>

#include "api/gui/ctextsource.h"   // api::CFixedText

namespace api {

// CUeGuiError = ecourna::api::exception::CBaseError<EUeGuiError> (SErrorLimits{4900, 5100}), gui-common.u15.h

namespace {

constexpr TPosition PROFUNDIDADE_SETA = 10;   // depth of the arrow tip / notch, in pixels      // name inferred
constexpr TPosition ESPACO            = 4;    // visible gap between two segments               // name inferred
constexpr TFontSize FONTE_MINIMA      = 10;

// Signature of the two layout strategies (called through the function table).
using TFuncaoLayout = bool (*)(IScreen&, const std::vector<std::string>&, const SRect&, TFontSize,
                               TPosition profundidade, TPosition espaco, std::vector<SRect>& retangulos);

// wasm func 5506                                                                          // name inferred
// Splits `area` into `n` rectangles of (almost) equal width that overlap by `profundidade - espaco`
// pixels. The first `resto` rectangles get one extra pixel; the last one takes whatever is left up to
// area.right.
std::vector<SRect> DivideArea(const SRect& area, std::size_t n, TPosition profundidade, TPosition espaco)
{
    switch (n) {
    case 0:  return {};
    case 1:  return {area};
    default: break;
    }
    const TPosition total = area.right + (n - 1) * (profundidade - espaco) - area.left + 1;
    const TPosition base  = total / static_cast<TPosition>(n);
    std::vector<TPosition> larguras(n, base);
    for (TPosition i = 0; i < total - base * static_cast<TPosition>(n); ++i)
        ++larguras[i];

    const TPosition topo = std::min(area.top, area.bottom);
    const TPosition fundo = std::max(area.top, area.bottom);
    std::vector<SRect> retangulos;
    TPosition x = area.left;
    for (std::size_t i = 0; i < n; ++i) {
        const TPosition largura = (i == n - 1) ? area.right - x + 1 : larguras[i];
        retangulos.push_back(SRect(SPoint{x, topo}, SPoint{static_cast<TPosition>(x + largura - 1), fundo}));
        x += largura + (espaco - profundidade);
    }
    return retangulos;
}

// wasm func 10972 (table slot 3442) - strategy "equal widths"                              // name inferred
// True if every label fits its equal-width segment with font `tamanho` (bold).
bool CabeLarguraIgual(IScreen& tela, const std::vector<std::string>& rotulos, const SRect& area,
                      TFontSize tamanho, TPosition profundidade, TPosition espaco, std::vector<SRect>& retangulos)
{
    if (rotulos.empty()) {
        retangulos.clear();
        return false;
    }
    retangulos = DivideArea(area, rotulos.size(), profundidade, espaco);

    const SFont fonte{tamanho, 1};
    const TPosition alturaArea = area.bottom - area.top + 1;
    const TPosition margens = 2 * std::max<TPosition>(tamanho / 4, 1);
    const TPosition alturaDisponivel = alturaArea - margens;
    // part of the width eaten by the arrow tip at text height
    const TPosition descontoSeta = tamanho * profundidade / std::max<TPosition>(alturaArea, 1);
    const std::size_t ultimo = rotulos.size() - 1;

    for (std::size_t i = 0; i <= ultimo; ++i) {
        TPosition largura = 0, altura = 0;
        tela.GetFontMetrics(largura, altura, fonte, rotulos[i]);                         // IScreen slot 3
        if (altura - tamanho / 5 > alturaDisponivel)
            return false;
        const SRect& r = retangulos[i];
        TPosition disponivel = r.right - r.left + 1 - margens;
        if (i > 0)
            disponivel -= profundidade;                  // notch on the left
        if (i != ultimo)
            disponivel -= descontoSeta;                  // arrow on the right
        if (largura - tamanho / 3 > disponivel)
            return false;
    }
    return true;
}

// wasm func 10971 (table slot 3443) - strategy "proportional widths". Listed in unit u34; written here
// because it is part of this file.                                                        // name inferred
bool CabeLarguraProporcional(IScreen& tela, const std::vector<std::string>& rotulos, const SRect& area,
                             TFontSize tamanho, TPosition profundidade, TPosition espaco,
                             std::vector<SRect>& retangulos)
{
    const std::size_t n = rotulos.size();
    switch (n) {
    case 0:  retangulos.clear();        return false;
    case 1:  retangulos.assign({area}); return true;
    default: break;
    }

    const SFont fonte{tamanho, 1};
    const TPosition alturaArea = area.bottom - area.top + 1;
    const TPosition margem = std::max<TPosition>(tamanho / 4, 1);
    const TPosition alturaDisponivel = alturaArea - 2 * margem;
    const std::size_t ultimo = n - 1;
    const TPosition larguraTotal = ultimo * (profundidade - espaco) + area.right - area.left + 1;
    const TPosition descontoSeta = tamanho * profundidade / std::max<TPosition>(alturaArea, 1);

    std::vector<TPosition> minimos(n, 0);   // minimum width of each segment
    std::vector<TPosition> pesos(n, 1);     // label widths, used to share the remaining space
    TPosition soma = 0;
    for (std::size_t i = 0; i < n; ++i) {
        TPosition largura = 0, altura = 0;
        tela.GetFontMetrics(largura, altura, fonte, rotulos[i]);
        if (altura - tamanho / 5 > alturaDisponivel)
            return false;
        const TPosition texto = largura - tamanho / 3;
        const TPosition bordas = (i != ultimo ? descontoSeta : 0) + 2 * margem + (i > 0 ? profundidade : 0);
        minimos[i] = std::max<TPosition>(bordas + texto, std::max<TPosition>(2 * profundidade, bordas + 1));
        pesos[i] = std::max<TPosition>(texto, 1);
        soma += minimos[i];
    }
    if (soma > larguraTotal)
        return false;

    std::vector<TPosition> larguras = minimos;
    const TPosition sobra = larguraTotal - soma;
    TPosition distribuido = 0;
    TPosition totalPesos = 0;
    for (TPosition p : pesos)
        totalPesos += p;
    if (totalPesos > 0) {
        for (std::size_t i = 0; i < ultimo; ++i) {       // the last one gets the rounding remainder
            const TPosition extra = pesos[i] * sobra / totalPesos;
            larguras[i] += extra;
            distribuido += extra;
        }
    }
    larguras.back() += sobra - distribuido;

    retangulos.clear();
    retangulos.reserve(n);
    const TPosition topo = std::min(area.top, area.bottom);
    const TPosition fundo = std::max(area.top, area.bottom);
    TPosition x = area.left;
    for (std::size_t i = 0; i < n; ++i) {
        const TPosition largura = (i == ultimo) ? area.right - x + 1 : larguras[i];
        retangulos.push_back(SRect(SPoint{x, topo}, SPoint{static_cast<TPosition>(x + largura - 1), fundo}));
        x += largura + (espaco - profundidade);
    }
    return true;
}

// wasm func 5507                                                                          // name inferred
// Tries `cabe` from `tamanhoInicial` down to FONTE_MINIMA; on success fills `resultado`.
bool TentaLayout(IScreen& tela, const std::vector<std::string>& rotulos, const SRect& area,
                 TFontSize tamanhoInicial, TFuncaoLayout cabe, CStepsProgressBar::SLayout& resultado)
{
    for (TFontSize tamanho = tamanhoInicial; tamanho >= FONTE_MINIMA; --tamanho) {
        std::vector<SRect> retangulos;
        if (cabe(tela, rotulos, area, tamanho, PROFUNDIDADE_SETA, ESPACO, retangulos)) {
            resultado.fonte = tamanho;
            resultado.retangulos = std::move(retangulos);
            resultado.rotulos = rotulos;
            return true;
        }
    }
    return false;
}

// Path helper (the pushes are inlined emplace_backs of 56-byte SPathElement records).     // name inferred
struct CCaminho {
    std::vector<SPathElement> elementos;
    void MoveTo(double x, double y) { elementos.push_back({SPathElement::MOVE_TO, x, y, 0, 0, 0, 0}); }
    void LineTo(double x, double y) { elementos.push_back({SPathElement::LINE_TO, x, y, 0, 0, 0, 0}); }
    void ArcTo(double x, double y, double w, double h, double inicio, double varredura)
    {
        elementos.push_back({SPathElement::ARC_TO, x, y, w, h, inicio, varredura});
    }
    void Close() { elementos.push_back({SPathElement::CLOSE, 0, 0, 0, 0, 0, 0}); }
};

// Outline of segment `i` of `ultimo + 1` inside `r` (inlined in Draw).                    // name inferred
std::vector<SPathElement> MontaCaminho(const SRect& r, std::size_t i, std::size_t ultimo)
{
    const double altura   = r.bottom - r.top + 1;
    const double raio     = std::min(altura * 0.5, 10.0);
    const double diametro = raio + raio;
    const double esquerda = r.left;
    const double topo     = r.top;
    const double direita  = esquerda + (r.right - r.left + 1);
    const double base     = topo + altura;
    const double meio     = topo + altura * 0.5;
    const double seta     = PROFUNDIDADE_SETA;

    CCaminho c;
    if (i == 0 && i == ultimo) {                    // single step: rounded rectangle
        c.MoveTo(esquerda + raio, topo);
        c.ArcTo(esquerda, topo, diametro, diametro, 90, 90);
        c.LineTo(esquerda, base - raio);
        c.ArcTo(esquerda, base - diametro, diametro, diametro, 180, 90);
        c.LineTo(direita - raio, base);
        c.ArcTo(direita - diametro, base - diametro, diametro, diametro, 270, 90);
        c.LineTo(direita, topo + raio);
        c.ArcTo(direita - diametro, topo, diametro, diametro, 0, 90);
    } else if (i == 0) {                            // first: rounded left side, arrow tip on the right
        c.MoveTo(esquerda + raio, topo);
        c.ArcTo(esquerda, topo, diametro, diametro, 90, 90);
        c.LineTo(esquerda, base - raio);
        c.ArcTo(esquerda, base - diametro, diametro, diametro, 180, 90);
        c.LineTo(direita - seta, base);
        c.LineTo(direita, meio);
        c.LineTo(direita - seta, topo);
    } else if (i == ultimo) {                       // last: notch on the left, rounded right side
        c.MoveTo(esquerda, topo);
        c.LineTo(direita - raio, topo);
        c.ArcTo(direita - diametro, topo, diametro, diametro, 90, -90);
        c.LineTo(direita, base - raio);
        c.ArcTo(direita - diametro, base - diametro, diametro, diametro, 0, -90);
        c.LineTo(esquerda, base);
        c.LineTo(esquerda + seta, meio);
    } else {                                        // middle: notch on the left, tip on the right
        c.MoveTo(esquerda, topo);
        c.LineTo(direita - seta, topo);
        c.LineTo(direita, meio);
        c.LineTo(direita - seta, base);
        c.LineTo(esquerda, base);
        c.LineTo(esquerda + seta, meio);
    }
    c.Close();
    return std::move(c.elementos);
}

} // namespace

// (inlined into wasm func 13550, vota::CPreShowProgressBar::PreShow; srcloc line 428)
CStepsProgressBar::CStepsProgressBar(const uedword atual, const std::vector<std::string>& rotulos,
                                     const SRect& area, const TColor corConcluido, const TColor corAtual,
                                     const TColor corFuturo, const TColor corTextoConcluido,
                                     const TColor corTextoAtual, const TColor corTextoFuturo,
                                     const TColor corContorno)
    : m_rotulos(rotulos),
      m_area(area),
      m_corConcluido(corConcluido),
      m_corAtual(corAtual),
      m_corFuturo(corFuturo),
      m_corTextoConcluido(corTextoConcluido),
      m_corTextoAtual(corTextoAtual),
      m_corTextoFuturo(corTextoFuturo),
      m_corContorno(corContorno)
{
    if (m_rotulos.empty())                                                                // :428
        throw CUeGuiError(EUeGuiError{5014}, "Quantidade de segmentos deve ser maior que zero");
    SetAtual(atual);
}

// inlined (constructor)                                                                   // name inferred
void CStepsProgressBar::SetAtual(uedword indice)
{
    const uedword novo = std::min<uedword>(indice, m_rotulos.size());
    if (novo == m_atual)
        return;
    m_atual = novo;
    Invalidate();   // inlined IFormFieldBase helper (form visible -> m_precisaRedesenhar, IForm::Atualiza)
}

// wasm func 3665 (D1) / 10973 (D0). Also called explicitly by CPreShowProgressBar::PreShow for its stack
// object.
CStepsProgressBar::~CStepsProgressBar() = default;

// inlined into Draw (wasm func 5504)                                                      // name inferred
void CStepsProgressBar::CalculaSegmentos(IScreen& tela) const
{
    const std::size_t n = m_rotulos.size();
    m_segmentos.clear();
    m_segmentos.reserve(n);                                    // growth helper = wasm func 5505

    const TPosition altura = m_area.bottom - m_area.top + 1;
    const TFontSize tamanhoInicial =
        std::min<TPosition>(std::max<TPosition>(altura * 3 / 5, FONTE_MINIMA), altura);

    SLayout igual{FONTE_MINIMA, {}, m_rotulos};
    const bool cabeIgual = TentaLayout(tela, m_rotulos, m_area, tamanhoInicial, &CabeLarguraIgual, igual);
    SLayout proporcional{FONTE_MINIMA, {}, m_rotulos};
    const bool cabeProporcional =
        TentaLayout(tela, m_rotulos, m_area, tamanhoInicial, &CabeLarguraProporcional, proporcional);

    SLayout layout;
    if (cabeIgual && !(cabeProporcional && igual.fonte < proporcional.fonte)) {
        layout = std::move(igual);
    } else if (cabeProporcional) {
        layout = std::move(proporcional);
    } else {                                     // nothing fits at size 10: equal widths, no labels
        layout.fonte = FONTE_MINIMA;
        layout.retangulos = DivideArea(m_area, n, PROFUNDIDADE_SETA, ESPACO);
        layout.rotulos = std::vector<std::string>(n);
    }
    m_fonte = SFont{layout.fonte, 1};

    for (std::size_t i = 0; i < n; ++i) {
        const std::string& rotulo = layout.rotulos[i];
        TPosition larguraTexto = 0, alturaTexto = 0;
        if (!rotulo.empty())
            tela.GetFontMetrics(larguraTexto, alturaTexto, m_fonte, rotulo);             // IScreen slot 3

        const SRect& r = layout.retangulos[i];
        const TPosition margem = std::max<TPosition>(m_fonte.size / 4, 1);
        const TPosition topoTexto = r.top + margem;
        const TPosition folga =
            ((std::max<TPosition>(topoTexto, r.bottom - margem) - topoTexto + 1) - alturaTexto) / 2;

        SSegmento segmento;
        segmento.caminho = MontaCaminho(r, i, n - 1);
        segmento.posTexto = SPoint{static_cast<TPosition>(r.left + margem + (i > 0 ? PROFUNDIDADE_SETA : 0)),
                                   static_cast<TPosition>(topoTexto + std::max<TPosition>(folga, 0))};
        segmento.texto = rotulo;
        m_segmentos.push_back(std::move(segmento));
    }
    m_recalcular = false;
}

// wasm func 5504 (vtable slot 2). Observed executing (every voting screen, via PreShow).
void CStepsProgressBar::Draw(IScreen& tela) const
{
    if (m_rotulos.empty())
        return;
    if (m_recalcular)
        CalculaSegmentos(tela);

    for (std::size_t i = 0; i < m_segmentos.size(); ++i) {
        const SSegmento& segmento = m_segmentos[i];
        tela.DrawPath(segmento.caminho, m_corContorno, 1);                               // IScreen slot 18
        TColor corTexto;
        if (i >= m_atual) {
            const bool atual = (i == m_atual);
            tela.FillPath(segmento.caminho, atual ? m_corAtual : m_corFuturo);           // IScreen slot 17
            corTexto = atual ? m_corTextoAtual : m_corTextoFuturo;
        } else {
            tela.FillPath(segmento.caminho, m_corConcluido);
            corTexto = m_corTextoConcluido;
        }
        if (segmento.texto.empty())
            continue;
        tela.WriteText(segmento.posTexto, CFixedText(ETextAlignment::Left, segmento.texto), m_fonte,
                       corTexto, TColor{0});                                             // IScreen slot 19
    }
}

// wasm func 10969 (slot 7)
std::string CStepsProgressBar::GetClassName() const
{
    return "CStepsProgressBar";
}

// wasm func 10968 (slot 8)
SRect CStepsProgressBar::Rect() const
{
    return m_area;
}

// wasm func 10970 (slot 9)
void CStepsProgressBar::Move(const SPoint& pos)
{
    if (pos.x == m_area.left && pos.y == m_area.top)
        return;
    m_area.MoveTo(pos);                                                                  // func 1915
    m_recalcular = true;
    m_segmentos.clear();
    Invalidate();
}

// Library code attributed to this file:
//   wasm func 5505  std::vector<CStepsProgressBar::SSegmento>::__swap_out_circular_buffer (moves the
//                   28-byte segments into the new buffer during reserve/push_back).

} // namespace api
