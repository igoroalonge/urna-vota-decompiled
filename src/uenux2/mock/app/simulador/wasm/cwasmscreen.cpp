// Reconstructed from vota_web_wasm.wasm (unit u31).
// Original file UNKNOWN - path inferred: uenux2/mock/app/simulador/wasm/cwasmscreen.cpp
//
// simulador::CWasmScreen - the voter screen of the web build (see cwasmscreen.h for the coordinate rules).
// 32 wasm functions: the constructor is inlined into func 8302; every other method is one vtable slot.
// Observed executing during the recorded votes: 9009, 9022, 9038, 9079, 9104, 9115, 9125, 9146, 9205,
// 9213, 9232, 9243, 9254, 9261.
#include "simulador/wasm/cwasmscreen.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include "api/gui/cmovie.h"
#include "api/gui/ctextsource.h"
#include "simulador/wasm/cwasmjs.h"        // extern "C" js_* imports of the glue (see list below)
#include "simulador/wasm/cwasmutil.h"      // Latin1ParaUtf8 (5098), EmitEvent (3533): unit u29

// Imports used by this file (names from the glue; analysis/imports.tsv):
//   js_init(w, h)                          js_clear(css)                     js_fill(x, y, w, h, css)
//   js_line(x1, y1, x2, y2, css, width)    js_circle(x, y, r, css, width)    js_round_rect(x, y, w, h, r, css, width)
//   js_gray_gradient(x, y, w, h)           js_path(n, int* tipos, double* valores, css, fill, width)
//   js_text(x, y, maxw, utf8, size, bold, italic, css, align)   js_measure_text_width(utf8, size, bold, italic)
//   js_image(bytes*, size, x, y, w, h)     js_log(msg)  (console.debug only when Module.uenuxDebug)
//
// Helpers defined by unit u29 (names inferred there):
//   std::string Latin1ParaUtf8(const std::string&)                          func 5098
//   CWasmScreen::EscalaCaminho(const std::vector<SPathElement>&) const      func 5113 (x/w * sx, y/h * sy)
//   void EmitEvent(const char* nome, const std::string& json)               func 3533 -> js_emit_event

namespace simulador {

namespace {

// Palette of the urna, index = api::TColor. 38 entries @1529596 (a compiler-generated switch table: a
// second copy starting at index 1 lives @1529448 for the call sites that already know cor != 0).
constexpr const char* PALETA[38] = {
    "rgba(0,0,0,0)",                                                             //  0 transparent
    "#ffffff", "#000000", "#808080", "#d3d3d3", "#acacac", "#404040", "#333333",  //  1..7
    "#ff0000", "#f08080", "#8b0000", "#00ff00", "#90ee90", "#006400",             //  8..13
    "#0000ff", "#add8e6", "#00008b", "#ffff00", "#ffffe0", "#eb7f29",             // 14..19
    "#ffd300", "#ffbe2e", "#dbe2ef", "#d9e8f6", "#2672de", "#1a4480",             // 20..25
    "#f7bbb1", "#e52207", "#8b0a03", "#fef0c8", "#ffbe2e", "#c66900",             // 26..31
    "#b4d0b9", "#008817", "#446443", "#e6e6e6", "#c9c9c9", "#adadad",             // 32..37
};

// Unsigned comparison in the binary (i32.gt_u): negative values are black as well.
const char* CorCss(api::TColor cor)
{
    return static_cast<unsigned>(cor) > 37 ? "#000000" : PALETA[cor];
}

int Ceil(double v) { return static_cast<int>(std::ceil(v)); }   // i32.trunc_sat_f64_s: saturating

// Anchor rule shared by DrawImage / DrawMovie / DrawSurface (x, y = anchor point; w, h = size).
void AplicaAncora(int& x, int& y, int w, int h, api::EAnchorPoint ancora)
{
    switch (static_cast<int>(ancora)) {
    case 1: x -= w;                break;   // top-right
    case 2: x -= w / 2;            break;   // top-centre
    case 3: y -= h / 2;            break;   // middle-left
    case 4: x -= w;     y -= h / 2; break;  // middle-right
    case 5: x -= w / 2; y -= h / 2; break;  // centre
    case 6: y -= h;                break;   // bottom-left
    case 7: x -= w;     y -= h;     break;  // bottom-right
    case 8: x -= w / 2; y -= h;     break;  // bottom-centre
    default:                       break;   // 0: top-left
    }
}

// Common tail of the image primitives: js_image with a size of at least 1 px (16-bit arithmetic).
void DesenhaBytes(const std::vector<uebyte>& dados, int x, int y, int w, int h)
{
    if (dados.empty())
        return;
    const short x2 = static_cast<short>(std::max(w, 1) + x - 1);
    const short y2 = static_cast<short>(std::max(h, 1) + y - 1);
    const short x1 = static_cast<short>(x), y1 = static_cast<short>(y);
    js_image(dados.data(), dados.size(), std::min(x1, x2), std::min(y1, y2),
             static_cast<short>(std::abs(x1 - x2) + 1), static_cast<short>(std::abs(y1 - y2) + 1));
}

}  // namespace

int CWasmScreen::EscalaX(int v) const { return Ceil(m_escalaX * v); }
int CWasmScreen::EscalaY(int v) const { return Ceil(m_escalaY * v); }

// ------------------------------------------------------------------------------------------------------
// Constructor - inlined into func 8302 (unit u19), right before push<api::IScreen>.
CWasmScreen::CWasmScreen(short larguraCanvas, short alturaCanvas)
    : m_escalaX(larguraCanvas / 640.0)
    , m_escalaY(alturaCanvas / 480.0)
{
    js_init(larguraCanvas, alturaCanvas);
    Clear(1);                                                                   // white (virtual call, slot 4)
    js_log(("CWasmScreen this=" + std::to_string(reinterpret_cast<std::uintptr_t>(this))).c_str());
}
// NOTE: larguraCanvas/alturaCanvas are `short` (CSimuladorWasm +16/+18, unit u29). A page URL such as
// ?screenWidth=65536 gives 0 (m_escalaX = 0: every x collapses to 0; verified with tools/run/headless.mjs).

// wasm func 8963 - slot 1 (deleting destructor). ~CWasmScreen is empty; ~IScreen removes every voter form
// (IForm<IScreen>::RemoveAll, func 5069).

// ------------------------------------------------------------------------------------------------------
// wasm func 9265 - slot 2                                                                  name inferred
void CWasmScreen::GetMaxCharSize(uebyte& largura, uebyte& altura, const api::SFont& fonte)
{
    const short meio = static_cast<short>(static_cast<short>(fonte.size) / 2);
    largura = static_cast<uebyte>(meio <= 1 ? 1 : meio);
    altura = static_cast<uebyte>(fonte.size);
}

// wasm func 9261 - slot 3 (overrides the throwing default of iscreen.h:69). Observed executing.
void CWasmScreen::GetFontMetrics(api::TPosition& largura, api::TPosition& altura, const api::SFont& fonte,
                                 const std::string& texto)
{
    largura = static_cast<api::TPosition>(GetTextWidth(texto, fonte));          // slot 32 (i32.store16)
    const short tamanho = static_cast<short>(fonte.size);
    altura = tamanho <= 1 ? 1 : tamanho;
}

// wasm func 9254 - slot 4. Observed executing.
void CWasmScreen::Clear(api::TColor cor)
{
    js_log("Clear(full)");
    js_clear(CorCss(cor));          // the glue also removes the GIF overlays (<div id="uenux-gif-layer">)
}

// wasm func 9243 - slot 5. Observed executing.
void CWasmScreen::ClearRect(const api::SRect& area, api::TColor cor)
{
    FillRect(area, cor);                                                        // slot 6
}

// wasm func 9232 - slot 6. Observed executing.
void CWasmScreen::FillRect(const api::SRect& area, api::TColor cor)
{
    const int x1 = EscalaX(area.left), x2 = EscalaX(area.right);
    const int y1 = EscalaY(area.top), y2 = EscalaY(area.bottom);
    js_fill(std::min(x1, x2), std::min(y1, y2), std::abs(x2 - x1) + 1, std::abs(y2 - y1) + 1, CorCss(cor));
}

// wasm func 9223 - slot 7                                                                  name inferred
void CWasmScreen::SetPixel(const api::SPoint& p, api::TColor cor)
{
    js_fill(EscalaX(p.x), EscalaY(p.y), 1, 1, CorCss(cor));
}

// wasm func 9213 - slot 8. Observed executing.                                             name inferred
void CWasmScreen::DrawLine(const api::SPoint& de, const api::SPoint& ate, api::TColor cor, int espessura)
{
    js_line(EscalaX(de.x), EscalaY(de.y), EscalaX(ate.x), EscalaY(ate.y), CorCss(cor), EscalaX(espessura));
}

// wasm func 9205 - slot 9. Observed executing.
void CWasmScreen::DrawRect(const api::SRect& r, api::TColor cor, int espessura)
{
    DrawLine({r.left, r.top}, {r.right, r.top}, cor, espessura);                // slot 8 x 4
    DrawLine({r.right, r.top}, {r.right, r.bottom}, cor, espessura);
    DrawLine({r.right, r.bottom}, {r.left, r.bottom}, cor, espessura);
    DrawLine({r.left, r.bottom}, {r.left, r.top}, cor, espessura);
}

// wasm func 9194 - slot 10                                                                 name inferred
// The radius is scaled by m_escalaX only (a circle stays a circle, but is wider than the logical box when
// the canvas is not 4:3).
void CWasmScreen::DrawCircle(const api::SPoint& centro, api::TPosition raio, api::TColor cor, int espessura)
{
    js_circle(EscalaX(centro.x), EscalaY(centro.y), EscalaX(raio), CorCss(cor), EscalaX(espessura));
}

// wasm func 9184 - slot 11                                                                 name inferred
void CWasmScreen::DrawRoundRect(const api::SRect& area, api::TPosition raio, api::TColor cor, int espessura)
{
    const int x1 = EscalaX(area.left), x2 = EscalaX(area.right);
    const int y1 = EscalaY(area.top), y2 = EscalaY(area.bottom);
    js_round_rect(std::min(x1, x2), std::min(y1, y2), static_cast<short>(std::abs(x2 - x1) + 1),
                  static_cast<short>(std::abs(y2 - y1) + 1), EscalaX(raio), CorCss(cor), EscalaX(espessura));
}

// wasm func 9175 - slot 12: white-to-black horizontal gradient (the glue's createLinearGradient). name inferred
void CWasmScreen::FillGrayGradient(const api::SRect& area)
{
    const int x1 = EscalaX(area.left), x2 = EscalaX(area.right);
    const int y1 = EscalaY(area.top), y2 = EscalaY(area.bottom);
    js_gray_gradient(std::min(x1, x2), std::min(y1, y2), static_cast<short>(std::abs(x2 - x1) + 1),
                     static_cast<short>(std::abs(y2 - y1) + 1));
}

// wasm func 9165 - slot 15                                                                 name inferred
void CWasmScreen::DrawTriangle(const api::SPoint& a, const api::SPoint& b, const api::SPoint& c, api::TColor cor,
                               int espessura)
{
    DrawLine(a, b, cor, espessura);
    DrawLine(b, c, cor, espessura);
    DrawLine(c, a, cor, espessura);
}

// wasm func 9155 - slot 16: closed polyline (nothing for fewer than 2 points).            name inferred
void CWasmScreen::DrawPolygon(const std::vector<api::SPoint>& pontos, api::TColor cor, int espessura)
{
    if (pontos.size() < 2)
        return;
    for (std::size_t i = 1; i < pontos.size(); ++i)
        DrawLine(pontos[i - 1], pontos[i], cor, espessura);
    DrawLine(pontos.back(), pontos.front(), cor, espessura);
}

// wasm func 9146 - slot 17 (arrow-shaped cargo tabs of CStepsProgressBar). Observed executing.
// The path is converted into two arrays read by the glue straight from HEAP32/HEAPF64 (docs/03 s8.3).
void CWasmScreen::FillPath(const std::vector<api::SPathElement>& caminho, api::TColor cor)
{
    const std::vector<api::SPathElement> escalado = EscalaCaminho(caminho);          // func 5113 (u29)
    std::vector<int> tipos;
    std::vector<double> valores;
    if (!escalado.empty()) {
        tipos.reserve(escalado.size());
        valores.reserve(escalado.size() * 6);
    }
    for (const auto& e : escalado) {
        tipos.push_back(e.tipo >= 1 && e.tipo <= 3 ? e.tipo : 0);    // unknown kinds become MOVE_TO
        const double seis[6] = {e.x, e.y, e.w, e.h, e.inicio, e.varredura};
        valores.insert(valores.end(), seis, seis + 6);               // func 2662
    }
    if (!tipos.empty())
        js_path(static_cast<int>(tipos.size()), tipos.data(), valores.data(), CorCss(cor), /*fill*/ 1, /*width*/ 1);
}

// wasm func 9125 - slot 18. Observed executing.
void CWasmScreen::DrawPath(const std::vector<api::SPathElement>& caminho, api::TColor cor, int espessura)
{
    const std::vector<api::SPathElement> escalado = EscalaCaminho(caminho);          // func 5113 (u29)
    std::vector<int> tipos;
    std::vector<double> valores;
    if (!escalado.empty()) {
        tipos.reserve(escalado.size());
        valores.reserve(escalado.size() * 6);
    }
    for (const auto& e : escalado) {
        tipos.push_back(e.tipo >= 1 && e.tipo <= 3 ? e.tipo : 0);
        const double seis[6] = {e.x, e.y, e.w, e.h, e.inicio, e.varredura};
        valores.insert(valores.end(), seis, seis + 6);
    }
    if (!tipos.empty())
        js_path(static_cast<int>(tipos.size()), tipos.data(), valores.data(), CorCss(cor), /*fill*/ 0,
                EscalaX(espessura));
}

// ------------------------------------------------------------------------------------------------------
// wasm func 9115 - slot 19: text at a point. Observed executing.
// Builds the text box from the metrics and the alignment of the text, then delegates to slot 20.
api::SRect CWasmScreen::WriteText(const api::SPoint& pos, const api::IText& texto, const api::SFont& fonte,
                                  api::TColor corTexto, api::TColor corFundo)
{
    api::TPosition largura = 0, altura = 0;
    GetFontMetrics(largura, altura, fonte, texto.GetText());                    // slot 3
    short x1 = pos.x;
    short x2 = static_cast<short>(pos.x + largura);
    switch (texto.GetAlignment()) {
    case api::ETextAlignment::Right:  x1 -= largura;     x2 -= largura;     break;   // pos is the right edge
    case api::ETextAlignment::Center: x1 -= largura / 2; x2 -= largura / 2; break;
    default: break;
    }
    const short y1 = pos.y, y2 = static_cast<short>(pos.y + altura);
    const api::SRect area{std::min(x1, x2), std::min(y1, y2), std::max(x1, x2), std::max(y1, y2)};
    return WriteText(area, texto, fonte, corTexto, corFundo);                   // slot 20
}

// wasm func 9104 - slot 20: text inside a rectangle. Observed executing (every label of the voter screens).
api::SRect CWasmScreen::WriteText(const api::SRect& area, const api::IText& texto, const api::SFont& fonte,
                                  api::TColor corTexto, api::TColor corFundo)
{
    const std::string utf8 = Latin1ParaUtf8(texto.GetText());                   // func 5098: app strings are ISO-8859-1

    api::TPosition largura = 0, altura = 0;
    GetFontMetrics(largura, altura, fonte, texto.GetText());                    // slot 3
    largura = std::max<api::TPosition>(largura, 1);
    altura = std::max<api::TPosition>(altura, 1);

    // Logical box of the text (returned to the caller, which uses it to erase the text later).
    short left = area.left;
    const short top = area.top;
    switch (texto.GetAlignment()) {
    case api::ETextAlignment::Right:  left = static_cast<short>(area.right - largura + 1); break;
    case api::ETextAlignment::Center:
        left = static_cast<short>(area.left + ((area.right - area.left + 1) - largura) / 2); break;
    default: break;
    }
    const short right = static_cast<short>(left + largura - 1);
    const short bottom = static_cast<short>(top + altura - 1);
    const api::SRect caixa{std::min(left, right), std::min(top, bottom), std::max(left, right), std::max(top, bottom)};

    // Canvas box.
    const int cx1 = std::min(EscalaX(caixa.left), EscalaX(caixa.right));
    const int cx2 = std::max(EscalaX(caixa.left), EscalaX(caixa.right));
    const int cy1 = std::min(EscalaY(caixa.top), EscalaY(caixa.bottom));
    const int cy2 = std::max(EscalaY(caixa.top), EscalaY(caixa.bottom));

    if (corFundo != 0)                                                          // 0 = transparent: no background
        js_fill(cx1, cy1, static_cast<short>(cx2 - cx1 + 1), static_cast<short>(cy2 - cy1 + 1), CorCss(corFundo));

    int xTexto, alinhamentoCanvas;                                              // canvas textAlign 0/1/2 = left/center/right
    if (texto.GetAlignment() == api::ETextAlignment::Center) {
        alinhamentoCanvas = 1;
        xTexto = (cx1 + cx2) / 2;
    } else {
        const bool direita = texto.GetAlignment() == api::ETextAlignment::Right;
        alinhamentoCanvas = direita ? 2 : 0;
        xTexto = direita ? cx2 : cx1;
    }

    std::string trecho = texto.GetText();                                       // the 3rd GetText() of this call
    trecho.resize(std::min<std::size_t>(trecho.size(), 40));
    js_log(("Write '" + trecho + "'").c_str());

    // Baseline: a quarter of the box height above its bottom, plus 2 logical pixels.
    const int yTexto = cy2 + static_cast<short>(cy2 - cy1 + 1) / -4 + Ceil(m_escalaY + m_escalaY);
    js_text(static_cast<short>(xTexto), yTexto, static_cast<short>(cx2 - cx1 + 1) /*ignored by the glue*/,
            utf8.c_str(), EscalaY(fonte.size), fonte.style & 1 /*bold*/, (fonte.style >> 1) & 1 /*italic*/,
            CorCss(corTexto), alinhamentoCanvas);
    return caixa;
}

// wasm func 9090 - slot 21: text clipped to `recorte` (CProgressBar draws the two halves of its label in two
// colours with it). The web build IGNORES the clip rectangle: the whole text is drawn both times.
void CWasmScreen::WriteText(const api::SPoint& pos, const api::SRect& /*recorte*/, const api::IText& texto,
                            const api::SFont& fonte, api::TColor corTexto, api::TColor corFundo)
{
    (void)WriteText(pos, texto, fonte, corTexto, corFundo);                     // slot 19
}

// ------------------------------------------------------------------------------------------------------
// wasm func 9079 - slot 22: image at its natural size. Observed executing (candidate photo, GIF animations,
// battery icon, and - through the BU screens - the QR codes).
//
// The natural size comes from CWasmImageSurfaceOps::GetImageSize (func 3493), which understands GIF, PNG and
// JPEG headers only: a BMP (every QR code of the urna is an 8-bit BMP built by api::CQRCodeImage) measures
// 1 x 1 and is drawn as a 2 x 2 px dot at 1280 x 800. Verified: the BU QR code of vota::CMostraQRCodeBU
// (145 478-byte BMP) reaches js_image(..., 1258, 59, 2, 2).
void CWasmScreen::DrawImage(const api::SPoint& pos, const api::IImage& imagem, api::EAnchorPoint ancora)
{
    const std::vector<uebyte> dados = imagem.GetImage();                       // a full copy of the encoded file
    const api::SPoint tamanho = m_operacoesImagem.GetImageSize(dados);          // func 3493 (direct call)
    int x = EscalaX(pos.x), y = EscalaY(pos.y);
    const int w = EscalaY(tamanho.x);                                           // sic: sy for the width too
    const int h = EscalaY(tamanho.y);
    AplicaAncora(x, y, w, h, ancora);
    DesenhaBytes(dados, x, y, w, h);
}

// wasm func 9068 - slot 23 (?): the argument is not used; the web build only draws a 64 x 64 placeholder
// (grey #acacac box with a #808080 border). Semantics on the urna unknown (a pre-rendered surface?).
void CWasmScreen::DrawSurface(const api::SPoint& pos, const void* /*superficie*/, api::EAnchorPoint ancora)
{
    int x = pos.x, y = pos.y;                                                   // logical coordinates here
    AplicaAncora(x, y, 64, 64, ancora);
    const short x1 = static_cast<short>(x), x2 = static_cast<short>(x + 64);
    const short y1 = static_cast<short>(y), y2 = static_cast<short>(y + 64);
    const api::SRect caixa{std::min(x1, x2), std::min(y1, y2), std::max(x1, x2), std::max(y1, y2)};
    FillRect(caixa, 5);                                                         // slot 6, #acacac
    DrawRect(caixa, 3, 1);                                                      // slot 9, #808080
}

// wasm func 9058 - slot 24: image with an explicit size (falls back to the natural size when the scaled
// size is 0 in either direction).
void CWasmScreen::DrawImage(const api::SPoint& pos, const api::IImage& imagem, const api::SSize& tamanho,
                            api::EAnchorPoint ancora)
{
    const std::vector<uebyte> dados = imagem.GetImage();
    int w = EscalaX(tamanho.x);
    int h = w != 0 ? EscalaY(tamanho.y) : 0;
    if (w == 0 || h == 0) {
        const api::SPoint natural = m_operacoesImagem.GetImageSize(dados);
        h = EscalaY(natural.y);
        w = EscalaY(natural.x);                                                 // sic: sy
    }
    int x = EscalaX(pos.x), y = EscalaY(pos.y);
    AplicaAncora(x, y, w, h, ancora);
    DesenhaBytes(dados, x, y, w, h);
}

// wasm func 9049 - slot 25: part `parte` of the image at `pos` (name inferred). The web build cannot crop:
// it squeezes the WHOLE image into a box of the size of `parte`.
void CWasmScreen::DrawImage(const api::SPoint& pos, const api::IImage& imagem, const api::SRect& parte)
{
    const std::vector<uebyte> dados = imagem.GetImage();
    if (dados.empty())
        return;
    const short xa = pos.x, xb = static_cast<short>(pos.x + parte.right - parte.left);
    const short ya = pos.y, yb = static_cast<short>(pos.y + parte.bottom - parte.top);
    const int x1 = EscalaX(std::min(xa, xb)), x2 = EscalaX(std::max(xa, xb));
    const int y1 = EscalaY(std::min(ya, yb)), y2 = EscalaY(std::max(ya, yb));
    js_image(dados.data(), dados.size(), std::min(x1, x2), std::min(y1, y2),
             static_cast<short>(std::abs(x1 - x2) + 1), static_cast<short>(std::abs(y2 - y1) + 1));
}

// wasm func 9038 - slot 26: current frame of an animation. Observed executing (the GIF animations of the
// voting screens). In the web build a CMovie has ONE frame holding the whole GIF (CWasmResource::GetMovie),
// and the browser animates it in an <img> overlay.
void CWasmScreen::DrawMovie(const api::SPoint& pos, const api::CMovie& filme, api::EAnchorPoint ancora)
{
    int x = EscalaX(pos.x), y = EscalaY(pos.y);
    const int w = EscalaY(filme.m_tamanho.x);                                   // sic: sy
    const int h = EscalaY(filme.m_tamanho.y);
    AplicaAncora(x, y, w, h, ancora);
    const auto& quadro = filme.m_frames.at(filme.m_frameAtual);                 // std::out_of_range if past the end
    DesenhaBytes(quadro.imagem, x, y, w, h);
}

// ------------------------------------------------------------------------------------------------------
// wasm func 9022 - slot 27: end of a redraw. Observed executing (39 times in the municipal session).
void CWasmScreen::Refresh()
{
    js_log("Refresh()");
    EmitEvent("vota:screen", std::string("{\"refreshed\":true}"));             // func 3533 -> js_emit_event
}

// wasm func 9018 - slot 28: partial refresh (not seen in the recorded sessions).           name inferred
void CWasmScreen::RefreshRect(const api::SRect& area)
{
    const int left = std::min(EscalaX(area.right), EscalaX(area.left));
    const int right = std::max(EscalaX(area.right), EscalaX(area.left));
    const int top = std::min(EscalaY(area.bottom), EscalaY(area.top));
    const int bottom = std::max(EscalaY(area.bottom), EscalaY(area.top));
    const std::string json = "{\"refreshed\":true,\"partial\":true,\"rect\":{\"left\":" + std::to_string(left)
                           + ",\"top\":" + std::to_string(top) + ",\"right\":" + std::to_string(right)
                           + ",\"bottom\":" + std::to_string(bottom)
                           + ",\"width\":" + std::to_string(static_cast<short>(std::abs(right - left) + 1))
                           + ",\"height\":" + std::to_string(static_cast<short>(std::abs(bottom - top) + 1)) + "}}";
    EmitEvent("vota:screen", json);
}

// wasm func 9013 - slot 29 (values stored, never used by this class)
void CWasmScreen::SetColors(api::TColor corTexto, api::TColor corFundo)
{
    m_corTexto = corTexto;
    m_corFundo = corFundo;
}

// wasm func 8959 - slot 30
api::TPosition CWasmScreen::GetWidth() const { return m_largura; }

// wasm func 8953 - slot 31
api::TPosition CWasmScreen::GetHeight() const { return m_altura; }

// wasm func 9009 - slot 32: the browser measures the text (canvas measureText) at the real font size and the
// result is brought back to logical pixels, plus one. Observed executing.
// With m_escalaX == 0 (?screenWidth=65536) the division gives +inf, saturated to INT_MAX (-1 once GetFontMetrics
// stores it into its TPosition), or NaN -> 0 for an empty text.
int CWasmScreen::GetTextWidth(const std::string& texto, const api::SFont& fonte)
{
    const std::string utf8 = Latin1ParaUtf8(texto);                              // func 5098
    const int px = js_measure_text_width(utf8.c_str(), EscalaY(fonte.size), fonte.style & 1, (fonte.style >> 1) & 1);
    return Ceil(std::ceil(px / m_escalaX) + 1.0);                               // i32.trunc_sat_f64_s, no extend16
}

// wasm func 9004 - slot 33 (?): clears the whole screen; the first argument is ignored.
void CWasmScreen::Clear(int /*camada*/, api::TColor cor)
{
    Clear(cor);                                                                 // slot 4
}

// wasm func 8945 - slot 34
api::IImageSurfaceOps& CWasmScreen::GetImageSurfaceOps()
{
    return m_operacoesImagem;                                                   // this + 44
}

}  // namespace simulador
