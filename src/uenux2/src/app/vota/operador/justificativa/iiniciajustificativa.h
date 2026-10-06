// Reconstructed from vota_web_wasm.wasm (unit u27).
// Original (path inferred from iiniciajustificativa.cpp, attested by srcloc :27):
// uenux2/src/app/vota/operador/justificativa/iiniciajustificativa.h
//
// "Inicia justificativa": entry of the justification (justificativa de ausência) flow of a voter who is
// impeded from voting in this section but may justify his absence here (voter in transit, "voto na
// seção original", TTE ...). It checks whether the voter already justified and then shows the
// confirmation screen (IConfirmaJustificativa) chosen by the concrete class.
//
// RTTI: comum::CAppState <- vota::IIniciaJustificativa (vtable @1590044; slot 0 icf "return this",
//   slot 1 operator delete, slot 2 StartState 10587). Subclasses (16 bytes, only the vptr differs):
//   CIniciaJustificativa (@1589316), CIniciaJustificativaTemporario (@1589428),
//   CIniciaJustificativaTransito (@1589540).
#pragma once

#include "comum/cappstate.h"

namespace vota {

class IConfirmaJustificativa;

class IIniciaJustificativa : public comum::CAppState {
public:
    /// wasm func 3624 (srcloc iiniciajustificativa.cpp:27)
    explicit IIniciaJustificativa(IConfirmaJustificativa* confirma);

    void StartState() override;                    // slot 2 (func 10587)

private:
    IConfirmaJustificativa* m_confirma;            // +12 (not owned: another lazy singleton)
};

class CIniciaJustificativa final : public IIniciaJustificativa {           // @1905728 (in 10631)
public:  static CIniciaJustificativa& GetInst();
private: CIniciaJustificativa();
};
class CIniciaJustificativaTemporario final : public IIniciaJustificativa { // @1905784 (in 10635)
public:  static CIniciaJustificativaTemporario& GetInst();
private: CIniciaJustificativaTemporario();
};
class CIniciaJustificativaTransito final : public IIniciaJustificativa {   // @1905840 (in 10635)
public:  static CIniciaJustificativaTransito& GetInst();
private: CIniciaJustificativaTransito();
};

}  // namespace vota
