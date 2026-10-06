// uenux2/src/app/comum/relatorios/cgeradorrelpu.h (path inferred: RTTI only; class comum::CGeradorRelPU,
// next to its sibling report generators such as comum::CGeradorRelVersaoPacoteDados in comum/relatorios/)
// Reconstructed from vota_web_wasm.wasm by units u35 (destructors, cgeradorrelpu.cpp) and u37 (AdicionaLinha,
// cgeradorrelpu.u37.cpp).
// NOTE: unit u37 accidentally overwrote u35's first version of this header while both units ran in
// parallel; this is the merged version (u35's .cpp defines the destructor out of line, as declared here).
//
// "Relatório de parâmetros de urna (PU)": a line-oriented thermal-printer report. The report body that uses
// it is vota::CImpressaoPU::StartState (wasm 11902, summarised by unit u25 in
// src/uenux2/src/app/vota/u25-foreign-fragments.cpp).
//
// RTTI: comum::CGeradorRelPU (class, no base; typeinfo @1545296, vtable @1545288):
//   [0] ~CGeradorRelPU  wasm 11901 (= shared_f6055(this, vtable): vptr + destroy the vector at +4)
//   [1] deleting dtor   wasm 11900 (= unknown_f6054(this, vtable): the same + operator delete)
// Layout (16 bytes): +0 vptr, +4 api::CPaperFormBuilder m_builder
// (a std::vector<std::shared_ptr<api::IFormField<api::IPaper>>>, 12 bytes).
#pragma once

#include <string>

#include "api/gui/cpaperformbuilder.h"       // api::CPaperFormBuilder (path inferred by other units)

namespace comum {

class CGeradorRelPU {
public:
    CGeradorRelPU() = default;                                        // inlined in 11902
    virtual ~CGeradorRelPU();                                         // wasm 11901 / 11900 (cgeradorrelpu.cpp, u35)

    api::CPaperFormBuilder& Builder() { return m_builder; }

    /// wasm func 677: one "<rótulo>  <valor>" line, value right-aligned to column 38.   (cgeradorrelpu.u37.cpp)
    void AdicionaLinha(const std::string& rotulo, const std::string& valor);

private:
    api::CPaperFormBuilder m_builder;                                 // +4
};

} // namespace comum
