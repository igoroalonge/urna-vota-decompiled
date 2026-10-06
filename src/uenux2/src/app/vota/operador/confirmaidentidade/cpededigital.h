// Reconstructed from vota_web_wasm.wasm (unit u10).
// Original (path inferred): uenux2/src/app/vota/operador/confirmaidentidade/cpededigital.h
//
// "Pede digital": the mesário's terminal asks the voter to put the thumb or index finger on the
// fingerprint sensor ("Solicite que o(a) eleitor(a) / <nome> / posicione POLEGAR ou INDICADOR no sensor").
// The state polls the scanner every 50 ms, picks a usable frame, compares it with the templates of the
// voter's fingers stored (encrypted) in the roll - "1x4": right thumb, left thumb, right index, left
// index - and moves to CDigitalReconhecida / CDigitalNaoReconhecida / CDigitalNaoReconhecidaPorTempo.
//
// RTTI: comum::CAppState <- vota::CPedeDigital (typeinfo @1592108, vtable @1591896)
//   slot 0 dtor (2739)  1 deleting (10469)  2 StartState (10468)  3/4 CAppState defaults
//   5 FinishState (10467)  6 no-op  7 ProcessInput (10466)  8 ProcessTick (10465)
#pragma once

#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

#include "comum/cappstate.h"
#include "api/gui/capplicationcontextstack.h"     // api::CApplicationContext
#include "api/gui/cinteractiveform.h"
#include "api/hwil/ifingerscanner.h"               // api::TSharedImageVector = std::shared_ptr<std::vector<uebyte>>
#include "comum/dados/celeitordetalhe.h"           // comum::TScoreHabilitacao (uint16)

namespace vota {

using uebyte = std::uint8_t;

class CPedeDigital final : public comum::CAppState {
public:
    static CPedeDigital& GetInst();                          // func 2740 (constructor inlined)
    ~CPedeDigital() override;                                // slot 0 (2739) / slot 1 (10469)

    void StartState() override;                              // slot 2 (10468) (srcloc 115, 120, 125)
    void FinishState() override;                             // slot 5 (10467) (srcloc 131, 132)
    void ProcessInput() override;                            // slot 7 (10466)
    void ProcessTick(uebyte tick) override;                  // slot 8 (10465) (srcloc 162, 163, 164)

private:
    CPedeDigital();                                          // inlined into 2740

    void ObtemEstadoPosReconhecimentoBiometrico(const bool reconhecido);          // func 5397 (srcloc 234)
    std::pair<bool, comum::TScoreHabilitacao>
        VerificaDigital(const api::TSharedImageVector& imagem, const long largura, const long altura);   // srcloc 262, inlined
    std::pair<bool, comum::TScoreHabilitacao> VerificaDigitalTreinamento();       // srcloc 279, inlined
    void ParaTicks();                                                            // inlined 3x, name inferred

    uebyte m_tickPrimeiraTentativa;                          // +11  30 000 ms: timeout of attempt 1
    uebyte m_tickDemaisTentativas;                           // +12  15 000 ms: timeout of attempts 2..n
    uebyte m_tickLeitura;                                    // +13  50 ms: scanner polling
    std::shared_ptr<api::CInteractiveForm<api::IScreenMT, api::IInputMT>> m_formCaptura;   // +16/+20
    std::shared_ptr<api::CInteractiveForm<api::IScreenMT, api::IInputMT>> m_formAguarde;   // +24/+28
    api::CApplicationContext m_contexto;                     // +32 (52 bytes) -> sizeof 84
};

}  // namespace vota
