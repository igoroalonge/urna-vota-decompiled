// FRAGMENT of uenux2/mock/app/simulador/wasm/cwasmscreen.cpp (path inferred: simulador::CWasmScreen, vtable
// @1528824) reconstructed by unit u29 from vota_web_wasm.wasm. The class and its 39 virtual slots belong to unit
// u31 (docs/03-js-wasm-interface.md §8); this file holds a private helper method and a
// file-local function used by those slots.
//
// CWasmScreen is the web implementation of api::IScreen: the application draws in a 640 x 480 space and every
// coordinate is multiplied by sx = width / 640.0 (+16) and sy = height / 480.0 (+24) before it crosses to the
// canvas (js_* imports).

#include <string>
#include <vector>

#include "api/gui/iscreen.h"     // api::SPathElement {int tipo; double x, y, w, h, inicio, varredura;} (56 bytes)

namespace simulador {

// ------------------------------------------------------------------------------------------------
// wasm func 5113 (arguments sret, this, path: the shape of a const member function; private, name inferred).
// Callers: CWasmScreen::FillPath (vf17, func 9146) and DrawPath (vf18, func 9125).
// Scales a Qt-style path (QPainterPath heritage: MoveTo / LineTo / ArcTo(rect, start, sweep) / Close) to canvas
// pixels. Elements of any other type are dropped.                                           name inferred
// (The 56-byte push_back is the libc++ instantiation std::vector<api::SPathElement>::push_back, wasm func 2670.)
// ------------------------------------------------------------------------------------------------
std::vector<api::SPathElement> CWasmScreen::EscalaCaminho(const std::vector<api::SPathElement>& caminho) const
{
    const CWasmScreen& tela = *this;                 // sx at +16, sy at +24
    std::vector<api::SPathElement> escalado;
    escalado.reserve(caminho.size());
    for (const api::SPathElement& e : caminho) {
        switch (e.tipo) {
        case api::SPathElement::MOVE_TO:          // 0
            escalado.push_back({api::SPathElement::MOVE_TO, e.x * tela.m_sx, e.y * tela.m_sy, 0, 0, 0, 0});
            break;
        case api::SPathElement::LINE_TO:          // 1
            escalado.push_back({api::SPathElement::LINE_TO, e.x * tela.m_sx, e.y * tela.m_sy, 0, 0, 0, 0});
            break;
        case api::SPathElement::ARC_TO:           // 2: bounding rectangle scaled, angles (degrees) unchanged
            escalado.push_back({api::SPathElement::ARC_TO, e.x * tela.m_sx, e.y * tela.m_sy,
                                e.w * tela.m_sx, e.h * tela.m_sy, e.inicio, e.varredura});
            break;
        case api::SPathElement::CLOSE:            // 3
            escalado.push_back({api::SPathElement::CLOSE, 0, 0, 0, 0, 0, 0});
            break;
        default:
            break;
        }
    }
    return escalado;
}

namespace {

// ------------------------------------------------------------------------------------------------
// wasm func 5098. Callers: CWasmScreen::Write (vf20, func 9104) and TextWidth (vf32, func 9009).
// The application's strings are Latin-1 (ISO-8859-1); the js_text / js_measure_text_width imports decode their
// argument as UTF-8 (UTF8ToString). Each byte >= 0x80 becomes the two-byte UTF-8 sequence of the same code
// point (U+0080..U+00FF).                                                                   name inferred
// ------------------------------------------------------------------------------------------------
std::string Latin1ParaUtf8(const std::string& latin1)
{
    std::string utf8;
    utf8.reserve(latin1.size() * 2);
    for (const char c : latin1) {
        const auto byte = static_cast<unsigned char>(c);
        if (byte >= 0x80) {
            utf8.push_back(static_cast<char>(0xC0 | (byte >> 6)));
            utf8.push_back(static_cast<char>(byte & 0xBF));
        } else {
            utf8.push_back(c);
        }
    }
    return utf8;
}

}  // namespace
}  // namespace simulador

// Also attributed to this area by the tools, but library code:
//   wasm func 2662  std::vector<double>::insert(const_iterator, const double*, const double*) (range insert;
//                   one ICF body shared by CWasmScreen::FillPath/DrawPath - which append 6 doubles per path
//                   element for js_path - and RHVoice's sample buffers, func 2209)
//   wasm func 2670  std::vector<api::SPathElement>::push_back(const SPathElement&) (56-byte trivially copyable)
