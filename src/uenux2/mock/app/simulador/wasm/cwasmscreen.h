// Reconstructed from vota_web_wasm.wasm (unit u31).
// Original file UNKNOWN - path inferred: uenux2/mock/app/simulador/wasm/cwasmscreen.h
// (TSE convention: class CWasmScreen in cwasmscreen.{h,cpp}, next to cwasmthread.cpp, which is attested by
//  the srcloc record "uenux2/mock/app/simulador/wasm/cwasmthread.cpp:31").
//
// simulador::CWasmScreen is the web build's api::IScreen, i.e. the VOTER display of the urna (640 x 480 on
// the machine). Every drawing primitive ends in one js_* import of the Emscripten glue that paints the
// <canvas id="uenux-screen"> of the page (docs/03-js-wasm-interface.md s8).
//
// RTTI: simulador::CWasmScreen (typeinfo @1529080, si) : api::IScreen (typeinfo @1531404)
//       vtable @1528824, 39 slots (table entries 545..583). Registered as api::IScreen by the simulator
//       start-up (func 8302, unit u19), which also inlines the constructor below.
//
// Coordinates: the application always draws in a 640 x 480 logical space. Every coordinate / length is
// converted with ceil(scale * v) (i32.trunc_sat_f64_s(f64.ceil(...)), a saturating conversion: an
// infinite or NaN value becomes INT_MIN/INT_MAX instead of trapping) before it crosses to JavaScript:
//   x, widths, line widths, radii ............ m_escalaX = canvasWidth  / 640.0
//   y, heights, font sizes, IMAGE SIZES ....... m_escalaY = canvasHeight / 480.0
// Images and animations are scaled by m_escalaY in BOTH directions (their aspect ratio is kept on a 16:10
// canvas), while their position uses m_escalaX.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "api/gui/iscreen.h"                       // api::IScreen, SPoint, SRect, SFont, TColor, IText ...
#include "simulador/wasm/cwasmimagesurfaceops.h"   // simulador::CWasmImageSurfaceOps (member at +44)

namespace api {
class IText;          // ctextsource.h: slot 2 std::string GetText() const, slot 3 ETextAlignment GetAlignment() const
class IImage;         // slot 2 std::vector<uebyte> GetImage() const (encoded PNG/JPEG/GIF/BMP bytes)
class CMovie;         // cmovie.cpp: +0 vector<CMovieFrame> (24-byte frames: vector<uebyte> imagem, int64 duracao),
                      //             +12 size_t m_frameAtual, +16 SPoint m_tamanho
struct SPathElement;  // 56 bytes: int tipo (0 MOVE_TO, 1 LINE_TO, 2 ARC_TO, 3 CLOSE) + 6 doubles
                      //           x, y, w, h, inicio, varredura (Qt QPainterPath::arcTo convention)
using SSize = SPoint; // explicit size {largura, altura} used by DrawImage slot 24   // name inferred
}  // namespace api

namespace simulador {

// EAnchorPoint values as interpreted by every anchored primitive of this file (names inferred):
//   0 top-left (default)  1 top-right  2 top-centre  3 middle-left  4 middle-right  5 centre
//   6 bottom-left         7 bottom-right  8 bottom-centre
// i.e. x -= w (1, 4, 7) or w/2 (2, 5, 8); y -= h/2 (3, 4, 5) or h (6, 7, 8).

class CWasmScreen : public api::IScreen {
public:
    // Inlined into func 8302 (CSimuladorWasm::Executa, unit u19).
    CWasmScreen(short larguraCanvas, short alturaCanvas);
    ~CWasmScreen() override = default;   // slot 0 = api::IScreen::~IScreen (func 5071, ICF), slot 1 = func 8963

    // --- metrics --------------------------------------------------------------------------------------
    void GetMaxCharSize(uebyte& largura, uebyte& altura, const api::SFont& fonte) override;          // slot 2  (9265)
    void GetFontMetrics(api::TPosition& largura, api::TPosition& altura, const api::SFont& fonte,
                        const std::string& texto) override;                                           // slot 3  (9261)

    // --- clearing and filling -----------------------------------------------------------------------------
    void Clear(api::TColor cor) override;                                                             // slot 4  (9254)
    void ClearRect(const api::SRect& area, api::TColor cor) override;                                 // slot 5  (9243)
    void FillRect(const api::SRect& area, api::TColor cor) override;                                  // slot 6  (9232)
    void SetPixel(const api::SPoint& p, api::TColor cor) override;                                    // slot 7  (9223) name inferred

    // --- vector primitives --------------------------------------------------------------------------------
    void DrawLine(const api::SPoint& de, const api::SPoint& ate, api::TColor cor, int espessura) override;   // slot 8  (9213) name inferred
    void DrawRect(const api::SRect& area, api::TColor cor, int espessura) override;                   // slot 9  (9205)
    void DrawCircle(const api::SPoint& centro, api::TPosition raio, api::TColor cor,
                    int espessura) override;                                                          // slot 10 (9194) name inferred
    void DrawRoundRect(const api::SRect& area, api::TPosition raio, api::TColor cor,
                       int espessura) override;                                                       // slot 11 (9184) name inferred
    void FillGrayGradient(const api::SRect& area) override;                                           // slot 12 (9175) name inferred
    // slot 13: NOT overridden - api::IScreen's own no-op default (both vtables hold the same table entry 558
    //          -> ICF 218; the entries of slots overridden here are distinct, e.g. 559 for slot 14).
    void Slot14() override {}                                                                         // slot 14 (table 559 -> ICF 218; pure in IScreen) ?
    void DrawTriangle(const api::SPoint& a, const api::SPoint& b, const api::SPoint& c, api::TColor cor,
                      int espessura) override;                                                        // slot 15 (9165) name inferred
    void DrawPolygon(const std::vector<api::SPoint>& pontos, api::TColor cor, int espessura) override;  // slot 16 (9155) name inferred
    void FillPath(const std::vector<api::SPathElement>& caminho, api::TColor cor) override;           // slot 17 (9146)
    void DrawPath(const std::vector<api::SPathElement>& caminho, api::TColor cor, int espessura) override;  // slot 18 (9125)

    // --- text -----------------------------------------------------------------------------------------------
    api::SRect WriteText(const api::SPoint& pos, const api::IText& texto, const api::SFont& fonte,
                         api::TColor corTexto, api::TColor corFundo) override;                        // slot 19 (9115)
    api::SRect WriteText(const api::SRect& area, const api::IText& texto, const api::SFont& fonte,
                         api::TColor corTexto, api::TColor corFundo) override;                        // slot 20 (9104)
    void WriteText(const api::SPoint& pos, const api::SRect& recorte, const api::IText& texto,
                   const api::SFont& fonte, api::TColor corTexto, api::TColor corFundo) override;     // slot 21 (9090)

    // --- images ---------------------------------------------------------------------------------------------
    void DrawImage(const api::SPoint& pos, const api::IImage& imagem, api::EAnchorPoint ancora) override;   // slot 22 (9079)
    void DrawSurface(const api::SPoint& pos, const void* superficie, api::EAnchorPoint ancora) override;    // slot 23 (9068) ?
    void DrawImage(const api::SPoint& pos, const api::IImage& imagem, const api::SSize& tamanho,
                   api::EAnchorPoint ancora) override;                                                // slot 24 (9058)
    void DrawImage(const api::SPoint& pos, const api::IImage& imagem, const api::SRect& parte) override;   // slot 25 (9049) name inferred
    void DrawMovie(const api::SPoint& pos, const api::CMovie& filme, api::EAnchorPoint ancora) override;   // slot 26 (9038)

    // --- refresh, state -------------------------------------------------------------------------------------
    void Refresh() override;                                                                          // slot 27 (9022)
    void RefreshRect(const api::SRect& area) override;                                                // slot 28 (9018) name inferred
    void SetColors(api::TColor corTexto, api::TColor corFundo) override;                              // slot 29 (9013)
    api::TPosition GetWidth() const override;                                                         // slot 30 (8959)
    api::TPosition GetHeight() const override;                                                        // slot 31 (8953)
    // Returns a full int: the saturated i32.trunc_sat result is returned without i32.extend16_s (a short return
    // would be sign-extended by the callee in the wasm C ABI) and CTextFieldDoubleLine compares it as an i32.
    int GetTextWidth(const std::string& texto, const api::SFont& fonte) override;                     // slot 32 (9009)
    void Clear(int camada, api::TColor cor) override;                                                 // slot 33 (9004) ?
    api::IImageSurfaceOps& GetImageSurfaceOps() override;                                             // slot 34 (8945)
    void Slot35() override {}                                                                         // slot 35 (table 580 -> ICF 218) ?
    void Slot36() override {}                                                                         // slot 36 (table 581 -> ICF 218) ?
    // slots 37, 38: NOT overridden - api::IScreen's own no-op defaults (table entries 582/583 in both vtables).

private:
    // func 5113 (defined by unit u29 in cwasmscreen.u29.cpp): copy of a path with x/w * m_escalaX, y/h * m_escalaY.
    std::vector<api::SPathElement> EscalaCaminho(const std::vector<api::SPathElement>& caminho) const;

    int EscalaX(int v) const;    // ceil(m_escalaX * v), saturating       (inlined everywhere)
    int EscalaY(int v) const;    // ceil(m_escalaY * v), saturating

    // Layout (48 bytes):
    //   +0  vptr (api::IScreen)
    //   +4  bool   = true   (?)  written by the constructor, never read by this class (IScreen member?)
    //   +8  int    = 0      (?)  idem
    double m_escalaX;                         // +16  canvasWidth / 640.0
    double m_escalaY;                         // +24  canvasHeight / 480.0
    api::TPosition m_largura = 640;           // +32  logical size returned by GetWidth()
    api::TPosition m_altura = 480;            // +34  logical size returned by GetHeight()
    api::TColor m_corTexto = 2;               // +36  SetColors(); default 2 (black)          never read here
    api::TColor m_corFundo = 0;               // +40  SetColors(); default 0 (transparent)    never read here
    CWasmImageSurfaceOps m_operacoesImagem;   // +44  (vtable @1528988) returned by GetImageSurfaceOps()
};

}  // namespace simulador
