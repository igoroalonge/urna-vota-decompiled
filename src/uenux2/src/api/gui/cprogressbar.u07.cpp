// FRAGMENT reconstructed by unit u07 from vota_web_wasm.wasm.
// Original file: uenux2/src/api/gui/cprogressbar.cpp (attested by srcloc records :52/:58/:67 of the constructor,
// wasm func 5545). Merge into cprogressbar.cpp.
//
// Layout used here (from the constructor): IFormFieldBase {+0 vptr, +4 bool m_alterado, +8 IForm* m_pForm,
// +12 std::string m_nome}; CProgressBar {+24 uedword m_valor = 0, +28 uedword m_minimo = 0, +32 uedword m_maximo, ...}.
#include "api/gui/cprogressbar.h"

#include <algorithm>
#include <mutex>

namespace api {

// wasm func 5508 (observed executing: votaTick drives it during "Gravando")          // name inferred
// Advances the bar one step (clamped to [min, max]) and asks the owning form to redraw if it is on screen.
// Callers: vota::CTelasVota::AvancaBarraProgresso (func 2369) and vota::CProgressoEncerramento (func 12101).
void CProgressBar::Incrementa()
{
    m_valor = std::clamp(m_valor + 1, m_minimo, m_maximo);

    if (IForm* form = m_pForm) {
        bool visivel;
        {
            std::lock_guard<std::mutex> lock(form->m_mutex);                  // form +28 (lock compiled away)
            visivel = form->m_visivel;                                        // form +4
        }
        if (visivel) {
            m_alterado = true;                                                // +4
            form->Atualiza();                                                 // IForm slot 4 (redraw)
        }
    }
}

// Related (other units): func 3667 CProgressBar::SetValor(uedword) - used by CSincronismoEleitor::StartState
// (reset to m_minimo) and CProgressoEncerramento.

} // namespace api
