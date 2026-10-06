// Reconstructed from vota_web_wasm.wasm (unit u08).
// Original: uenux2/src/app/vota/eleitor/fimvotacao/ccopiaresultadoparamr.h (path inferred)
//
// "Copia resultado para MR" = encerramento step that copies the result files of the section (BU,
// RDV, logs, hashes, images of the reports, signatures, biometric WSQ packages) from the internal
// result directory to the MR ("mídia de resultado", the USB result stick mounted at /dsk/mr/),
// then checks the copy of the BU byte by byte. EstadoVota must be EAVCOPIARESULTADOSMR (63); on
// success the state becomes EAVENCERRADA (64) / EAEIMPRIMIROBRIGATORIABU (50) and the machine goes
// on to print the remaining mandatory BU copies (CImprimirBUOutrasObrigatorias).
//
// RTTI: comum::CAppState <- vota::CCopiaResultadoParaMR (typeinfo @1540212, vtable @1540080)
//   [0] 244 dtor (ICF)  [1] 387 deleting dtor (ICF)  [2] StartState 12134  [3] NeedChangeState 7480
//   [4] GetNextState 1661  [5..8] nops
#pragma once

#include <memory>

#include "comum/cappstate.h"

namespace vota {

class CPreShowFormVota;   // vota/eleitor/comum (unit u07)

class CCopiaResultadoParaMR final : public comum::CAppState {
public:
    /// Lazy singleton, wasm func 6174 (not in this unit):
    ///   CAppState(4 = ticks);
    ///   m_tela = CPreShowFormVota(CFormBuilder{StatusHeader(5), "Gravando o resultado na mídia",
    ///                              "Por favor, aguarde..."}, "telaCopiaResultadoParaMR")
    static CCopiaResultadoParaMR& GetInst();

    void StartState() override;       // wasm func 12134 (srcloc 59; CopiaResultado inlined)

private:
    void CopiaResultado();            // srcloc 85/94/112/126/174, inlined into StartState

    std::shared_ptr<CPreShowFormVota> m_tela;   // +12 (+16 control block); size 20
};

}  // namespace vota
