// Reconstructed from vota_web_wasm.wasm (unit u17).
// Original: uenux2/src/api/gui/ctextsource.h (srcloc ctextsource.h:37, the constructor).
//
// A text source that reads a shared, mutable std::string: the owner keeps the shared_ptr and changes
// the text; the field that displays it (CDataText<CTextSource>, CDataTextFmt<CTextSource>,
// CTextFieldUpdateMT) re-reads it at every redraw. Used by the poll worker's microterminal screens
// ("VOTANDO PARA: <cargo>", the inspection status line, "Votou parcialmente"/"Não votou", ...).
//
// The class has no out-of-line function: its constructor is inlined into the (lazy singleton)
// constructors of the vota states that own such a text. That is why the tools named two of those
// singleton accessors "api::CTextSource::CTextSource":
//   func  652 = vota::CPedeIdentidade::GetInst()        (see vota/operador/u17-foreign-fragments.cpp)
//   func 1150 = vota::CMostraEleitorVotando::GetInst()
//   (also func 5397 CPedeDigital::ObtemEstadoPosReconhecimentoBiometrico and 10740, other units)
// Users: CDataText<CTextSource> (vtable @1590656, text = *m_texto),
//        CDataTextFmt<CTextSource> (vtable @1587072, snprintf(buf, 512, fmt, m_texto->c_str())).
#pragma once

#include <memory>
#include <string>

#include "api/gui/gui-common.u15.h"

namespace api {

class CTextSource {
public:
    // ctextsource.h:37 - always inlined
    explicit CTextSource(const std::shared_ptr<std::string>& texto)
        : m_texto(texto)
    {
        if (!m_texto)
            throw CUeGuiError(static_cast<EUeGuiError>(4977), "Texto nulo");
    }

    const std::string& operator()() const { return *m_texto; }

private:
    std::shared_ptr<std::string> m_texto;    // +0/+4 (8 bytes; stored at +8 of CDataText<CTextSource>)
};

}  // namespace api
