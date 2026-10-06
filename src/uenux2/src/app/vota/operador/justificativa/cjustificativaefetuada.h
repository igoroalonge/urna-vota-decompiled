// Reconstructed from vota_web_wasm.wasm (unit u39; constructor by u27, ProcessInput by u19).
// Original (path inferred by u27): uenux2/src/app/vota/operador/justificativa/cjustificativaefetuada.h
//
// "Justificativa efetuada": end of the justification of absence ("justificativa eleitoral": a voter who is
// away from his own section declares it at any urna). The MT shows the typed identity and either
// "AUSÊNCIA JUSTIFICADA" (a new justification was recorded) or "JÁ JUSTIFICOU" (the voter had already
// justified at this urna), plus "CONFIRMA: continuar".
//
// RTTI: comum::CAppState <- vota::CJustificativaEfetuada (typeinfo @1589848, vtable @1589812)
//   [0] ICF 448  [1] ICF 765  [2] StartState 10594  [7] ProcessInput 10593 (u19)
// Lazy singleton func 2747 (u27): @1905952, mutex @1905928. IIniciaJustificativa::StartState sets
// m_novaJustificativa = false when the voter is already in the justification table.
//
// WEB BUILD: dead code (operator thread not run).
#pragma once

#include <memory>
#include <string>

#include "api/gui/cinteractiveform.h"
#include "comum/cappstate.h"

namespace vota {

class CJustificativaEfetuada final : public comum::CAppState {
public:
    static CJustificativaEfetuada& GetInst();          // wasm func 2747 (u27)

    void StartState() override;                        // [2] wasm func 10594
    void ProcessInput() override;                      // [7] wasm func 10593 (u19)

    bool m_novaJustificativa;                          // +28 true = recorded now, false = já justificou

private:
    CJustificativaEfetuada();                          // CAppState(2), u27-foreign-fragments.cpp

    std::shared_ptr<std::string> m_texto;              // +12 (+16) line 2, via CTextSource ("%s")
    std::shared_ptr<api::CInteractiveForm<api::IScreenMT, api::IInputMT>> m_form;   // +20 (+24); sizeof 32
};

}  // namespace vota
