// FRAGMENT reconstructed by unit u17 from vota_web_wasm.wasm.
// Original file: uenux2/src/api/gui/iinputfield.h (path inferred; class api::IInputField<MEDIA>,
// layout in gui-common.u15.h). The tools named both instantiations "api::IInput::GetKey" because
// IInput::GetKey (iinput.h:86) is inlined into them.
//
//   wasm func 6319  IInputField<IScreen>::Read   - slot 10 of IInputField<IScreen>, CInputField<CFramedText>,
//                   CInputFieldControl(Base)<IScreen>; observed executing (every key of the voter).
//   wasm func 11024 IInputField<IScreenMT>::Read - slot 8 of IInputField<IScreenMT>, CInputFieldMT,
//                   CInputFieldControl(Base)<IScreenMT>. Identical except Clear() = slot 9.
//
// Layout used (from gui-common.u15.h, names completed here):
//   +24 std::string m_texto            +36 shared_ptr<IInputValidation> m_validacao
//   +44 size_t m_tamanhoMaximo         +50 bool m_aguardaConfirma  (full field waits for CONFIRMA)
//   +51 bool m_corrigeLimpaTudo        +52 bool m_confirmaFinaliza (CONFIRMA ends the input)
//   +53 bool m_aceitaBranco            +54 bool m_corrigeRetornaAposLimpar
//   +55 char m_ultimaTecla             (flag names inferred from the code paths below)
// The IScreenMT constructor (vota_f3673) sets +52 = 1, +50 = argument, the others 0.
#pragma once

#include "api/gui/gui-common.u15.h"
#include "api/hwil/iinput.h"

namespace api {

template <class MEDIA>
EInputResult IInputField<MEDIA>::Read(IInput& entrada)
{
    for (;;) {
        if (!entrada.HasKey())
            return EInputResult::Tecla;                          // 13: nothing (more) to read
        m_ultimaTecla = entrada.GetKey();                        // iinput.h:86 (inlined)

        // 1. printable key accepted by the validation -> append
        if (m_texto.size() != m_tamanhoMaximo && m_validacao->IsValidChar(m_ultimaTecla)) {   // slot 3
            m_texto.push_back(m_ultimaTecla);
            Invalidate();                                        // form->IsActive() ? dirty + Redraw
        }

        // 2. CONFIRMA
        if (m_confirmaFinaliza && m_ultimaTecla == 'C') {
            if (m_validacao->IsValid(m_texto))                   // slot 2 (default: every char valid;
                return EInputResult::Confirma;                   //  true for an EMPTY text)      9
            Clear();                                             // slot 11 (IScreen) / 9 (IScreenMT)
        }

        // 3. CORRIGE
        if (m_ultimaTecla == 'D') {
            if (m_texto.empty())
                return EInputResult::Corrige;                    // 5
            if (m_corrigeLimpaTudo) {
                Clear();
                if (m_corrigeRetornaAposLimpar)
                    return EInputResult::Corrige;
            } else {
                m_texto.pop_back();
                Invalidate();
            }
        }

        // 4. field full: finish unless it must wait for CONFIRMA
        if (m_texto.size() == m_tamanhoMaximo && !(m_aguardaConfirma && m_ultimaTecla != 'C')) {
            if (m_validacao->IsValid(m_texto))
                return EInputResult::Confirma;
            Clear();
        }

        // 5. BRANCO on an empty field
        if (m_texto.empty() && m_aceitaBranco && m_ultimaTecla == 'B')
            return EInputResult::Branco;                         // 3
    }
}

}  // namespace api
