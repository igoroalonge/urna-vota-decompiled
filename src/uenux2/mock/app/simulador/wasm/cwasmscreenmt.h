// Reconstructed from vota_web_wasm.wasm (unit u31).
// Original file UNKNOWN - path inferred: uenux2/mock/app/simulador/wasm/cwasmscreenmt.h
//
// simulador::CWasmScreenMT : api::IScreenMT - the poll worker's MICROTERMINAL (MT, "microterminal do
// mesário"): a 4-line x 40-column text LCD plus an LED. In the page it is the <pre id="mt"> box, written in
// one go by the import js_mt_set(texto, ocupado); its border turns green (#34a853) when the LED is lit.
// The LED is shown as a fifth text line "CABINA: LIVRE" / "CABINA: OCUPADA" (booth free / occupied).
//
// RTTI: simulador::CWasmScreenMT (typeinfo @1529836, si) : api::IScreenMT (typeinfo @1531468)
//       vtable @1529756, 20 slots (table entries 615..634). Registered by func 8302 (unit u19).
//       Local class simulador::CWasmScreenMT::ShowClock(api::SPoint const&)::WasmText : api::IText
//       (typeinfo @1530064, vtable @1530048) - it names slot 12.
#pragma once

#include <string>

#include "api/gui/iscreen.h"   // api::IScreenMT, api::SPoint, api::IText

namespace simulador {

class CWasmScreenMT : public api::IScreenMT {
public:
    static constexpr std::size_t LINHAS = 4;
    static constexpr std::size_t COLUNAS = 40;

    CWasmScreenMT();                   // inlined into func 8302: Clear() (4 x 40 blanks) then Refresh()
    ~CWasmScreenMT() override;         // slot 0 = func 8745, slot 1 = func 8744 (deleting)

    void Clear() override;                                                        // slot 2  (8810)
    void Write(const api::SPoint& pos, const api::IText& texto) override;         // slot 3  (8799)
    // slot 4  DesenhaImagem(pos, imagem, limparTela): no-op (ICF 1870) - the web MT shows no images
    // slot 5  no-op (ICF 218) ?
    void Bipa(int frequencia, int vezes) override;                                // slot 6  (8787) name from u10 (?)
    void Slot7(int valor) override;                                               // slot 7 and slot 19 (5039) ?
    void LedOperacao0() override;                                                 // slot 8  and slot 11 (5038)
    void LedOperacao1() override;                                                 // slot 9  and slot 10 (5036)
    void LedOperacao2() override;                                                 // slot 10 (= 5036)
    void LedOperacao3() override;                                                 // slot 11 (= 5038)
    void ShowClock(const api::SPoint& pos) override;                              // slot 12 (8757) name from RTTI
    // slot 13 no-op (ICF 218) ?
    void Refresh() override;                                                      // slot 14 (8746)
    // slots 15, 16 no-ops (ICF 218) ?
    api::TPosition GetWidth() const override;                                     // slot 17 (8743): 480
    api::TPosition GetHeight() const override;                                    // slot 18 (8742): 80
    void Slot19(int valor) override;                                              // slot 19 (= 5039); called with 1 by
                                                                                  // CPedeIdentidade and 0 by CEscolheOpcao
                                                                                  // on urnas of model >= 2020 ?
private:
    // Layout (56 bytes):
    std::string m_linhas[LINHAS];      // +4, +16, +28, +40   each exactly COLUNAS characters
    bool m_cabinaOcupada = false;      // +52  the MT LED ("cabina ocupada" = a voter is in the booth)
};

}  // namespace simulador
