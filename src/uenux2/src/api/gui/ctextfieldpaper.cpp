// Reconstructed from vota_web_wasm.wasm (unit u16). Original: uenux2/src/api/gui/ctextfieldpaper.cpp
// (srcloc record :23).
#include "api/gui/ctextfieldpaper.h"

namespace api {

// CUeGuiError = ecourna::api::exception::CBaseError<EUeGuiError> (SErrorLimits{4900, 5100}), gui-common.u15.h

// wasm func 2771 (srcloc line 23)
CTextFieldPaper::CTextFieldPaper(const SharedIText& texto, IPaper::EStyle estilo)
    : m_texto(texto), m_estilo(estilo)
{
    if (!m_texto)                                                                         // :23
        throw CUeGuiError(EUeGuiError{4973}, "Campo estava com o texto nulo");
}

// wasm func 10919 (D1) / 10918 (D0)
CTextFieldPaper::~CTextFieldPaper() = default;

// wasm func 10920 (slot 2). In the web build the paper is simulador::CWasmNullPaper, whose slot 2 is an
// empty function (func 1528): nothing is printed.
void CTextFieldPaper::Draw(IPaper& papel) const
{
    papel.Print(*m_texto, m_estilo);                                                     // IPaper slot 2
}

} // namespace api
