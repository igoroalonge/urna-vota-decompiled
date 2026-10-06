// Reconstructed from vota_web_wasm.wasm (unit u39 = slots 0-3; slot 4 by unit u07).
// Original (path inferred by u07): uenux2/src/app/vota/eleitor/fimvotacao/cprogressoencerramento.h
//
// Progress screen of the ENCERRAMENTO while CGravaResultado writes and signs the result files (BU, RDV,
// hashes, logs...): "Preparando dados para encerramento / O processo pode levar alguns minutos /
// Por favor, aguarde..." with a 30-step CProgressBar ({70,350}-{570,385}, text "%p" = percentage), form
// name "telaProgressoEncerramento". Created by CGravaResultado::StartState (func 12098,
// std::make_shared<CProgressoEncerramento>(), constructor inlined there) and handed to
// comum::CGravacaoResultados (func 5824), which calls Inicia, Avanca once per step and Finaliza.
//
// RTTI: comum::CAbstractTelaProgresso <- vota::CProgressoEncerramento (typeinfo @1540616, vtable @1540596)
//   [0] dtor 3907  [1] deleting dtor 12104  [2] Inicia 12103  [3] Finaliza 12102  [4] Avanca 12101 (u07)
// Slot names: u07 ("Inicia: bar = min, show", "Finaliza: bar = max", "Avanca").          names inferred
#pragma once

#include <memory>
#include <mutex>

#include "api/gui/cprogressbar.h"
#include "api/gui/iform.h"
#include "comum/cabstracttelaprogresso.h"   // comum::CAbstractTelaProgresso (typeinfo @1551232, vtable @1551212: trivial dtor + 3 no-op virtuals;
                                             // path inferred: emitted just before comum::CAppState)

namespace vota {

class CProgressoEncerramento final : public comum::CAbstractTelaProgresso {
public:
    CProgressoEncerramento();                          // inlined into CGravaResultado::StartState (12098)
    ~CProgressoEncerramento() override;                // [0] wasm func 3907, [1] 12104 (+ operator delete)

    void Inicia() override;                            // [2] wasm func 12103
    void Finaliza() override;                          // [3] wasm func 12102
    void Avanca() override;                            // [4] wasm func 12101 (u07)

private:
    // +0 vptr (the base has no data members)
    api::SharedForm m_tela;                            // +4  (+8 ctrl)  "telaProgressoEncerramento"
    std::shared_ptr<api::CProgressBar> m_barra;        // +12 (+16 ctrl) 30 steps
    std::mutex m_mutex;                                // +20 (24 bytes: sizeof 44) - the voter thread draws,
                                                       //     CGravacaoResultados may report from elsewhere
};

}  // namespace vota
