// Reconstructed from vota_web_wasm.wasm (unit u39; constructor and ProcessInput by u10).
// Original (path inferred by u10): uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaoreconhecidaportempo.h
//
// "Digital não reconhecida por tempo": a fingerprint capture attempt timed out (30 s for the first attempt,
// 15 s for the next ones; CPedeDigital::ProcessTick). MT: <voter name> / "Eleitor(a) não reconhecido(a)" /
// "Tentativa x de y" / "CORRIGE: cancelar  CONFIRMA: retornar". CONFIRMA -> next attempt (CPedeDigital) or,
// after the last one, the birth-year check (CVerificaDadoEleitor); CORRIGE -> CCancelaHabilitacaoEleitor.
//
// RTTI: comum::CAppState <- vota::CDigitalNaoReconhecidaPorTempo (typeinfo @1591612, vtable @1591576)
//   [0] ICF 448  [1] ICF 765  [2] StartState 10486  [7] ProcessInput 10485 (u10)
// Lazy singleton @1908952; GetInst + ctor inlined into CPedeDigital::ProcessTick (func 10465).
//
// WEB BUILD: dead code (operator thread not run).
#pragma once

#include <memory>
#include <string>

#include "api/gui/cinteractiveform.h"
#include "comum/cappstate.h"

namespace vota {

class CDigitalNaoReconhecidaPorTempo final : public comum::CAppState {
public:
    static CDigitalNaoReconhecidaPorTempo& GetInst();

    void StartState() override;                        // [2] wasm func 10486
    void ProcessInput() override;                      // [7] wasm func 10485 (u10)

private:
    CDigitalNaoReconhecidaPorTempo();                  // CAppState(2), see u10-foreign-fragments.cpp

    std::shared_ptr<std::string> m_textoTentativa;     // +12 (+16) "Tentativa x de x", shown through CTextSource
    std::shared_ptr<api::CInteractiveForm<api::IScreenMT, api::IInputMT>> m_form;   // +20 (+24); sizeof 28
};

}  // namespace vota
