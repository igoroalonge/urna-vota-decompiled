// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/gui/cinputfieldmt.h (path inferred from its screen sibling cinputfield.h).
//
// api::CInputFieldMT : api::IInputField<api::IScreenMT>   (typeinfo 1587536, vtable @1587492, 72 bytes)
// The numeric input of the poll worker's microterminal: the 12-digit título (voter registration number)
// typed by the mesário to release a voter ("habilitação"), the year of birth check, the 1-digit menu choice.
//
// Layout: IInputField<IScreenMT> (64 bytes, constructor func 3673 - see iinputfield.u17.h / gui-common.u15.h)
//   +24 std::string m_texto   +36 shared_ptr<IInputValidation> m_validacao   +44 size_t m_tamanhoMaximo
//   +48 bool m_foco           +49 bool m_cursorVisivel (600 ms blink timer at +56)
//   +50..+55 behaviour flags / last key
// then
//   +64 SPoint m_pos          +68 bool m_mascarado (always false: the only constructor clears it)
//
// Vtable: 0 IInputField<IScreenMT> dtor (5517)  1 deleting (3672)  2 Draw (10726)  3/4 cursor timer
//         start/stop (6327/6324)  5 RedrawIfDirty  6 SetForm  7 GetClassName ("IFormField<IScreenMT>")
//         8 Read (11024)  9 Clear (6317)  10 SetLength (6312)
#pragma once

#include "api/gui/iinputfield.u17.h"

namespace api {

class CInputFieldMT : public IInputField<IScreenMT> {
public:
    // Inlined in func 1151 (CFormBuilderMT::Add<CInputFieldMT>(tamanho, aguardaConfirma, pos)), which always
    // passes a CNumberValidation("0123456789").                                          name inferred
    CInputFieldMT(std::shared_ptr<IInputValidation> validacao, std::size_t tamanho, bool aguardaConfirma,
                  const SPoint& pos)
        : IInputField<IScreenMT>(std::move(validacao), tamanho, aguardaConfirma),   // func 3673
          m_pos(pos)
    {
    }

    void Draw(IScreenMT& tela) const override;      // wasm 10726

private:
    SPoint m_pos;                  // +64
    bool   m_mascarado = false;    // +68 show '*' instead of the digits (never set in this build)
};

} // namespace api
