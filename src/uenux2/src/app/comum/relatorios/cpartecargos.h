// uenux2/src/app/comum/relatorios/cpartecargos.h   (path inferred: RTTI only; sibling report parts live in the
// attested comum/relatorios/cpartecandidatos.cpp)
// Reconstructed from vota_web_wasm.wasm (unit u35: destructors). Imprime (slot 2, func 11209) is reconstructed in
// src/uenux2/src/app/comum/dados/u04-foreign-fragments.cpp (unit u04).
//
// Report part that repeats a per-cargo part for every cargo of the election (zerésima: "LISTA DE CANDIDATOS"),
// printing the eleição banner whenever the eleição changes.
// RTTI: comum::CParteCargos : api::IReportPart, vtable @1576564: [0] 11208 [1] 11207 [2] 11209 Imprime. 20 bytes.
#pragma once

#include <memory>

#include "api/gui/reports/ireportpart.h"

namespace comum {

class CParteCargos : public api::IReportPart
{
public:
    CParteCargos(std::shared_ptr<api::IReportPart> cabecalhoEleicao, std::shared_ptr<api::IReportPart> parteCargo)
        : m_cabecalhoEleicao(std::move(cabecalhoEleicao)), m_parteCargo(std::move(parteCargo)) {}   // inlined
    ~CParteCargos() override;                                   // wasm func 11208 (+ deleting 11207)

    void Imprime() const override;                              // wasm func 11209 (unit u04)

private:
    std::shared_ptr<api::IReportPart> m_cabecalhoEleicao;       // +4  (may be null)
    std::shared_ptr<api::IReportPart> m_parteCargo;             // +12
};

} // namespace comum
