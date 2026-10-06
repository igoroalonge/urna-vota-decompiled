// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/fimvotacao/cimprimirbuoutrasobrigatorias.cpp
//
// srcloc evidence:
//   :50  Assert (appInfo.GetVota().GetEstadoVota() == EAVENCERRADA)                        (3470)
//   :53  Assert (appInfo.GetVota().GetEstadoEncerramento() == EAEIMPRIMIROBRIGATORIABU)    (3471)
//   :57  Assert (appInfo.GetVota().GetQtdBU() <= qtdBuObrigatorio)                        (3472)
//
// Not executed in the recorded sessions.

#include "vota/eleitor/fimvotacao/cimprimirbuoutrasobrigatorias.h"

#include <string>
#include <vector>

#include "api/gui/cformbuilder.h"
#include "comum/cappinfo.h"
#include "comum/dados/cconfiguracaoeleicao.h"
#include "comum/informacao/cinformacaoeleicao.h"
#include "vota/eleitor/comum/ctelasvota.h"
#include "vota/eleitor/fimvotacao/cimprimindobim.h"
#include "vota/eleitor/fimvotacao/cimprimindobu.h"
#include "vota/eleitor/fimvotacao/cimprimirbjust.h"
#include "vota/eleitor/fimvotacao/cretirarmr.h"
#include "vota/log/clogvota.h"

namespace vota {

using comum::md::estadoaplicacao::EEstadoVota;
using comum::md::estadoaplicacao::EEstadoEncerramento;

namespace {

// Screen "telaDestinoBUs", built (inlined, probably a CTelasVota factory) before every copy.
// Coordinates in the 640x480 logical screen.                                          name inferred
api::SharedForm CriaTelaDestinoBUs()
{
    CTelasVota::GetInst();                                              // (result unused)
    api::CFormBuilder b;
    b.AddStatusHeader(5);                                               // func 502
    b.AddLabel("Imprimindo vias obrigatórias", {320, 60}, FONTE_TITULO /*474888*/, 2, 2, 1);   // api_f202
    b.AddLabel("do Boletim de Urna", {320, 110}, FONTE_TITULO, 2, 2, 1);
    // "Via nº {}" with GetVota().GetQtdBU() + 1, evaluated at draw time (data source func 13023, slot 1101)
    b.AddDataLabel(&DS_ViaAtualBU, {320, 170}, FONTE_TEXTO /*474992*/, 2);                    // api_f1191
    b.AddLabel("Por favor, aguarde...", {320, 200}, FONTE_TEXTO, 2, 2, 1);
    b.AddLine({0, 250}, {639, 250}, 3);                                                        // api_f2244
    // "destino das vias": configuration text split at '\n' (func 1880); one line every 25 px
    api::TPosition y = 275;
    for (const std::string& linha : comum::util::Split(comum::CConfiguracaoEleicao::GetInst().GetTextoDestinoBUs(), '\n')) {
        if (linha.empty())                                              // cfg +260  name inferred
            continue;
        b.AddLabel(linha, {10, y}, FONTE_PEQUENA /*474896*/, 0, 2, 1);
        y += 25;
    }
    return api::CriaForm(b, "telaDestinoBUs");                          // comum_f886
}

}  // namespace

// wasm func 12065 — vtable slot 2
void CImprimirBUOutrasObrigatorias::StartState()
{
    const comum::CInformacaoEleicao info(comum::CConfiguracaoEleicao::GetInst());   // vota_f603: {&cfg +88}
    auto& appInfo = comum::CAppInfo::GetInst();
    UE_ASSERT(appInfo.GetVota().GetEstadoVota() == EEstadoVota::EAVENCERRADA);                        // :50
    UE_ASSERT(appInfo.GetVota().GetEstadoEncerramento() == EEstadoEncerramento::EAEIMPRIMIROBRIGATORIABU); // :53

    // number of mandatory copies: 1 in demonstration mode, else parâmetro +12 (func 3847, whose
    // analyzer name "EhModoDemonstracao" comes from the inlined srcloc cinformacaoeleicao.cpp:40)
    const uebyte qtdBuObrigatorio = info.GetQtdViasObrigatoriasBU();                                   // name inferred
    UE_ASSERT(appInfo.GetVota().GetQtdBU() <= qtdBuObrigatorio);                                       // :57

    while (appInfo.GetVota().GetQtdBU() < qtdBuObrigatorio) {
        if (comum::UrnaDesligando())                     // @1832936: power-off requested
            return;                                      // m_proximoEstado unchanged
        CriaTelaDestinoBUs()->Exibe();
        const uebyte via = appInfo.GetVota().GetQtdBU() + 1;
        CLogVota::GetInst().LogaImpressaoRelatorio(comum::ERelatoriosUE::BU, via);   // func 1127
        CImprimindoBU::ImprimeBU(via, PrintMessageMode::SemMensagem);               // func 2890
        appInfo.GetVota().IncrementaQtdBU();                                         // func 3701
        comum::SalvaEstado();                                                        // func 491
    }

    if (info.ImprimeBoletimJustificativa())              // func 5918: parâmetro +394 (cfg +482)  name inferred
        m_proximoEstado = &CImprimirBJust::GetInst();    // inline singleton @1833736, CAppState(0)
    else if (info.IdentificaMesarios())                  // func 1950: !demo && parâmetro +400 (cfg +488)
        m_proximoEstado = &CImprimindoBim::GetInst();    // func 5985
    else
        m_proximoEstado = &CRetirarMR::GetInst();        // func 2291
}

}  // namespace vota
