// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/gui/cformpart.h (path inferred; unit u24 and cgeradorresumozeresima.cpp already
// use this name).
//
// api::CFormPart : api::IReportPart   (typeinfo 1583708, vtable @1583696, 12 bytes)
// Adapter that makes a printable form (IForm<IPaper>, built with CPaperFormBuilder) one "part" of a
// composite report (api::CReport, reports/creport.cpp). Composite reports - the extract of the RDV
// (Registro Digital do Voto), the zerésima summary - are vectors of parts printed in order.
//   +4/+8 std::shared_ptr<IForm<IPaper>> m_form
// IReportPart (typeinfo 1576512, "class"): [0] dtor [1] deleting [2] Imprime() (unit u17 calls the
// interface IFormPart). Other implementations: comum::CParteEleitores, CParteRdv, CParteCargos (u24).
#pragma once

#include <memory>

#include "api/gui/iform.h"

namespace api {

class IReportPart {
public:
    virtual ~IReportPart() = default;
    virtual void Imprime() const = 0;                                 // slot 2
};

class CFormPart : public IReportPart {
public:
    explicit CFormPart(std::shared_ptr<IForm<IPaper>> form) : m_form(std::move(form)) {}   // inlined in func 601
    ~CFormPart() override;                                            // wasm 10888 (u24) / 10887

    void Imprime() const override;                                    // wasm 10889

private:
    std::shared_ptr<IForm<IPaper>> m_form;    // +4/+8
};

} // namespace api
