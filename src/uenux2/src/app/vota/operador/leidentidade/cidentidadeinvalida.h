// Reconstructed from vota_web_wasm.wasm (unit u39; constructor by u17, ProcessInput by u27).
// Original (path inferred by u17): uenux2/src/app/vota/operador/leidentidade/cidentidadeinvalida.h
//
// "Identidade inválida": the identifier typed by the mesário (título, CPF...) failed validation
// (CValidaIdentidade). MT: "Identidade: <typed digits> / Número errado / CORRIGE: retornar".
//
// RTTI: comum::CAppState <- vota::CIdentidadeInvalida (typeinfo @1588420, vtable @1588384, 20 bytes)
//   [0] ICF 244  [1] ICF 387  [2] StartState 10669  [7] ProcessInput 10667 (u27)
// Lazy singleton @1905364 (mutex @1905340), inlined into CValidaIdentidade (10627).
//
// WEB BUILD: dead code (operator thread not run).
#pragma once

#include <memory>

#include "api/gui/cinteractiveform.h"
#include "comum/cappstate.h"

namespace vota {

class CIdentidadeInvalida final : public comum::CAppState {
public:
    static CIdentidadeInvalida& GetInst();

    void StartState() override;                        // [2] wasm func 10669
    void ProcessInput() override;                      // [7] wasm func 10667 (u27)

private:
    CIdentidadeInvalida();                             // CAppState(2), u17-foreign-fragments.cpp

    std::shared_ptr<api::CInteractiveForm<api::IScreenMT, api::IInputMT>> m_form;   // +12
};

}  // namespace vota
