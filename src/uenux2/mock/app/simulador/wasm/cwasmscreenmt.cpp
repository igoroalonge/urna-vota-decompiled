// Reconstructed from vota_web_wasm.wasm (unit u31).
// Original file UNKNOWN - path inferred: uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp
//
// simulador::CWasmScreenMT - the poll worker's microterminal of the web build (see the header).
// 14 wasm functions: 12 vtable bodies + the two destructors of the local class WasmText.
// None of them ran during the recorded votes except through main() (the constructor, inlined into 8302,
// produces the first js_mt_set("...\nCABINA: LIVRE", 0) of the start-up trace).
#include "simulador/wasm/cwasmscreenmt.h"

#include <algorithm>
#include <string>

#include "api/gui/ctextsource.h"
#include "api/util/cdatetime.h"          // api::CDateTime::Now (func 479), CTime::Format (func 779)
#include "simulador/wasm/cwasmjs.h"      // js_mt_set

namespace simulador {

// Inlined into func 8302 (unit u19).
CWasmScreenMT::CWasmScreenMT()
{
    Clear();                                                                    // the 4 assign(40, ' ') + Refresh()
}

// wasm func 8745 - slot 0 / wasm func 8744 - slot 1 (deleting): destroy the 4 lines, then ~IScreenMT pops
// every microterminal form (IForm<IScreenMT>::RemoveAll, func 3461).
CWasmScreenMT::~CWasmScreenMT() = default;

// wasm func 8810 - slot 2
void CWasmScreenMT::Clear()
{
    for (std::string& linha : m_linhas)
        linha.assign(COLUNAS, ' ');                                             // shared_f1355
    Refresh();                                                                  // slot 14
}

// wasm func 8799 - slot 3: writes a text into line pos.y (1..4) starting at column pos.x (1-based),
// overwriting in place; nothing outside the 4 x 40 grid is ever written.
void CWasmScreenMT::Write(const api::SPoint& pos, const api::IText& texto)
{
    if (pos.y < 1 || pos.y > static_cast<int>(LINHAS))                          // (uint16)(y - 5) < 65532
        return;

    std::string conteudo = texto.GetText();
    if (conteudo.size() > COLUNAS)
        conteudo.resize(COLUNAS);
    std::string& linha = m_linhas[pos.y - 1];

    std::size_t coluna;
    if (texto.GetAlignment() == api::ETextAlignment::Center && conteudo.size() <= COLUNAS - 1) {
        coluna = (COLUNAS - conteudo.size()) / 2;
    } else {
        coluna = pos.x > 0 ? static_cast<std::size_t>(pos.x - 1) : 0;
        if (texto.GetAlignment() == api::ETextAlignment::Right && conteudo.size() < COLUNAS)
            coluna = COLUNAS - conteudo.size();
    }

    if (coluna < linha.size()) {
        const std::size_t n = std::min(linha.size() - coluna, conteudo.size());
        if (n != 0)
            std::copy_n(conteudo.data(), n, linha.data() + coluna);
        Refresh();                                                              // slot 14
    }
}

// wasm func 8787 - slot 6. The urna beeps the MT (e.g. Bipa(50, 3) in CSuspensaoAutomaticaEleitor); the web
// build only repaints.
void CWasmScreenMT::Bipa(int /*frequencia*/, int /*vezes*/)
{
    Refresh();
}

// wasm func 5039 - slots 7 and 19 (identical bodies folded): repaint only.
void CWasmScreenMT::Slot7(int /*valor*/) { Refresh(); }
void CWasmScreenMT::Slot19(int /*valor*/) { Refresh(); }

// wasm func 5038 - slots 8 and 11: LED off -> "CABINA: LIVRE"
void CWasmScreenMT::LedOperacao0()
{
    m_cabinaOcupada = false;
    Refresh();
}
void CWasmScreenMT::LedOperacao3() { LedOperacao0(); }   // same body (5038)

// wasm func 5036 - slots 9 and 10: LED on -> "CABINA: OCUPADA"
void CWasmScreenMT::LedOperacao1()
{
    m_cabinaOcupada = true;
    Refresh();
}
void CWasmScreenMT::LedOperacao2() { LedOperacao1(); }   // same body (5036)

// wasm func 8757 - slot 12 (name from the RTTI of its local class)
void CWasmScreenMT::ShowClock(const api::SPoint& pos)
{
    // Local IText of this function (vtable @1530048): {vptr, ETextAlignment +4 = Left, std::string +8}.
    struct WasmText : api::IText {
        explicit WasmText(std::string texto) : m_texto(std::move(texto)) {}
        std::string GetText() const override { return m_texto; }                // slot 2 = ICF 1139
        api::ETextAlignment GetAlignment() const override { return m_alinhamento; }   // slot 3 = ICF 1661
        api::ETextAlignment m_alinhamento = api::ETextAlignment::Left;          // +4
        std::string m_texto;                                                    // +8
        // wasm func 8748 - slot 0 (~WasmText, merged body shared_f1727)
        // wasm func 8680 - slot 1 (deleting, merged body shared_f1969)
    };

    const api::CDateTime agora = api::CDateTime::Now();                         // func 479 (ISystemDateTime)
    Write(pos, WasmText(agora.GetTime().Format("hh:mm")));                      // func 779; slot 3
}

// wasm func 8746 - slot 14: publishes the whole MT to the page.
void CWasmScreenMT::Refresh()
{
    std::string texto;
    for (const std::string& linha : m_linhas) {
        texto += linha;
        texto += '\n';
    }
    texto += m_cabinaOcupada ? "CABINA: OCUPADA" : "CABINA: LIVRE";
    js_mt_set(texto.c_str(), m_cabinaOcupada);
}

// wasm func 8743 - slot 17 / wasm func 8742 - slot 18: size of the MT LCD in pixels (used to centre images,
// comum::CInfoMTLCD::Update).
api::TPosition CWasmScreenMT::GetWidth() const { return 480; }
api::TPosition CWasmScreenMT::GetHeight() const { return 80; }

}  // namespace simulador
