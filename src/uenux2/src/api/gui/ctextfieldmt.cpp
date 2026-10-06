// Reconstructed from vota_web_wasm.wasm (unit u16). Original: uenux2/src/api/gui/ctextfieldmt.cpp
// (srcloc record :25).
#include "api/gui/ctextfieldmt.h"

#include <string>

namespace api {

// CUeGuiError = ecourna::api::exception::CBaseError<EUeGuiError> (SErrorLimits{4900, 5100}), gui-common.u15.h

// wasm func 1262 (srcloc line 25). Callers: the mesário screens (api_f180, 619, 651, 1152, 2753, 5409...).
CTextFieldMT::CTextFieldMT(const SPoint& pos, const SharedIText& texto)
    : m_pos(pos), m_texto(texto)
{
    if (!m_texto)                                                                         // :25
        throw CUeGuiError(EUeGuiError{4970}, "Campo estava com o texto nulo");
}

// wasm func 10933 (D1) / 10932 (D0): releases m_textoAnterior, m_texto, m_nome.
CTextFieldMT::~CTextFieldMT() = default;

// wasm func 10934 (slot 2). The micro-terminal has no "erase rectangle": if the previous text was longer,
// it is overwritten with as many blanks, at the previous position and with the previous alignment.
void CTextFieldMT::Draw(IScreenMT& tela) const
{
    if (m_textoAnterior && m_textoAnterior->GetText().size() > m_texto->GetText().size()) {
        const std::string brancos(m_textoAnterior->GetText().size(), ' ');
        tela.Write(m_posAnterior, CFixedText(m_textoAnterior->GetAlignment(), brancos));  // IScreenMT slot 3
    }
    tela.Write(m_pos, *m_texto);
    m_textoAnterior = m_texto;
    m_posAnterior = m_pos;
}

// No GetClassName here: slot 7 is inherited from IFormField<IScreenMT> (func 11051, not in this unit), which
// returns the literal "IFormField<IScreenMT>" for every MT field (CBeepFieldMT, CLedFieldMT, CClockFieldMT...).

} // namespace api
