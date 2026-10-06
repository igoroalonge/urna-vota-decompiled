// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/fimvotacao/cverificaqtdbusadicionais.cpp
//
// srcloc evidence:
//   :36  Assert (egVota.GetEstadoVota() == EAVENCERRADA)                (3461)
//   :37  Assert (egVota.GetEstadoEncerramento() == EAEFIMDOSTRABALHOS)  (3462)

#include "vota/eleitor/fimvotacao/cverificaqtdbusadicionais.h"

#include <memory>
#include <mutex>

#include "comum/cappinfo.h"
#include "comum/dados/cconfiguracaoeleicao.h"
#include "comum/informacao/cinformacaoeleicao.h"
#include "vota/eleitor/comum/ctelasvota.h"
#include "vota/eleitor/fimvotacao/cemitirmaisbu.h"
#include "vota/eleitor/fimvotacao/climitecopiasbuatingido.h"

namespace vota {

using comum::md::estadoaplicacao::EEstadoVota;
using comum::md::estadoaplicacao::EEstadoEncerramento;

// wasm func 12023 — vtable slot 2
void CVerificaQtdBUsAdicionais::StartState()
{
    auto& egVota = comum::CAppInfo::GetInst().GetVota();
    UE_ASSERT(egVota.GetEstadoVota() == EEstadoVota::EAVENCERRADA);                       // :36 (64)
    UE_ASSERT(egVota.GetEstadoEncerramento() == EEstadoEncerramento::EAEFIMDOSTRABALHOS); // :37 (52)

    const comum::CInformacaoEleicao info(comum::CConfiguracaoEleicao::GetInst());         // vota_f603
    // funcs 3847 / 5914 (both named "EhModoDemonstracao" by the tools after the inlined srcloc):
    // demonstration mode -> 1 and 1, otherwise parâmetro +12 / +16.                       names inferred
    if (info.GetQtdViasObrigatoriasBU() + info.GetQtdMaximaViasAdicionaisBU() <= egVota.GetQtdBU())
        m_proximoEstado = &CLimiteCopiasBUAtingido::GetInst();   // created inline: @1833960, CAppState(2),
                                                                 // m_tela = CTelasVota +108/+112
    else
        m_proximoEstado = &CEmitirMaisBU::GetInst();             // func 3879
}

}  // namespace vota
