// Reconstructed from vota_web_wasm.wasm (unit u39).
// Original (path inferred by u10): uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaoreconhecidaportempo.cpp
// Constructor and ProcessInput (10485): operador/u10-foreign-fragments.cpp.
//
// WEB BUILD: dead code (operator thread not run).
#include "vota/operador/confirmaidentidade/cdigitalnaoreconhecidaportempo.h"

#include <format>

#include "comum/dados/cconfiguracaoeleicao.h"
#include "vota/log/clogvota.h"
#include "vota/operador/confirmaidentidade/ccontrolareconhecimento.h"

namespace vota {

// wasm func 10486 - vtable slot 2
void CDigitalNaoReconhecidaPorTempo::StartState()
{
    m_proximoEstado = this;

    // CControlaReconhecimento::GetInst() (func 1256) is called twice with its result discarded: before the
    // first read of the static attempt counter (byte @1590924) and before the first read of CConfiguracaoEleicao
    // +136 = número de tentativas de habilitação (ParametrosUrna.numTentativasHabilitacao). The log record
    // re-reads both values without calling it. Both are uebyte -> formatted as unsigned (arg types 198).
    CControlaReconhecimento::GetInst();
    const uebyte tentativa = CControlaReconhecimento::GetTentativa();
    CControlaReconhecimento::GetInst();
    const uebyte numTentativas = comum::CConfiguracaoEleicao::GetInst().GetNumTentativasHabilitacao();
    *m_textoTentativa = std::format("Tentativa {} de {}", tentativa, numTentativas);

    CLogVota::GetInst().Loga(std::format("Timeout de reconhecimento do dedo. Tentativa [{}] de [{}]",   // api_f233
                                         CControlaReconhecimento::GetTentativa(),
                                         comum::CConfiguracaoEleicao::GetInst().GetNumTentativasHabilitacao()));
    m_form->Show();                                                                   // +20
}

}  // namespace vota
