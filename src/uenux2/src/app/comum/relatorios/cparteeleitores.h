// uenux2/src/app/comum/relatorios/cparteeleitores.h   (path inferred, as in unit u24)
// Reconstructed from vota_web_wasm.wasm (unit u35). ~CParteEleitores (slot 0, func 11205) is in
// src/uenux2/src/app/comum/u24-foreign-fragments.cpp (unit u24).
//
// Report part that prints one sub-part per voter of the section roll (CEleitores), positioning the roll's cursor
// on each voter so that the sub-part's data sources (name, título, sequencial...) read that voter.
// Only user: the voter-list report ("lista de eleitores", vota::CImpressaoListaEleitores::StartState, func 11908).
// RTTI: comum::CParteEleitores : api::IReportPart, vtable @1576596: [0] 11205 [1] 11204 [2] 11206 Imprime. 12 bytes.
#pragma once

#include <memory>

#include "api/gui/reports/ireportpart.h"

namespace comum {

class CParteEleitores : public api::IReportPart
{
public:
    explicit CParteEleitores(std::shared_ptr<api::IReportPart> parteEleitor)
        : m_parteEleitor(std::move(parteEleitor)) {}            // inlined
    ~CParteEleitores() override;                                // wasm func 11205 (u24) + deleting 11204

    void Imprime() const override;                              // wasm func 11206

private:
    std::shared_ptr<api::IReportPart> m_parteEleitor;           // +4 / +8
};

} // namespace comum
