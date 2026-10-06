// Reconstructed from vota_web_wasm.wasm (unit u27; constructor by unit u17).
// Original (path inferred from cregistradigitaloperador.cpp, attested by srclocs :95-:371):
// uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.h
//
// "Registra digital do operador": last resort of the voter identification. The voter's fingerprint was
// not recognised in any attempt and the birth-year check passed (CVerificaDadoEleitor), or the capture
// failed (CDigitalNaoCapturada "tentar novamente"): the MESÁRIO now places HIS OWN finger on the sensor to
// take responsibility for releasing the voter (habilitação "por código do mesário",
// ETipoHabilitacao::CODIGO_MESARIO = 2, QR "HBBG" in the BU).
//
// The mesário's capture is kept in CControlaReconhecimento::s_digitalMesario (stored later as encrypted
// WSQ under <trab>/wsq/operador/ by CMostraEleitorVotando::SalvaHabilitacaoEleitor). This state tries to
// find WHICH registered mesário it is (CRegistradorMesario = comparecimento_mesario rows) to fill
// CControlaReconhecimento::s_tituloMesario; if nobody matches, the capture is saved as a new
// "me{:06}.wsq" under <trab>/wsq/nao-registrado/. The voter is released in every case once a finger
// is detected.
//
// RTTI: comum::CAppState <- vota::CRegistraDigitalOperador (typeinfo @1592612, vtable @1592432)
//   [0] icf 1537 (dtor)  [1] icf 3629 (deleting)  [2] StartState 10454  [3]/[4] CAppState defaults
//   [5] FinishState 10453  [6] no-op  [7] ProcessInput 10452  [8] ProcessTick 10451
#pragma once

#include <cstdint>
#include <memory>

#include "api/gui/cinteractiveform.h"
#include "comum/cappstate.h"
#include "comum/dados/md/eleitor/celeitoridentidade.h"

namespace vota {

class CRegistraDigitalOperador final : public comum::CAppState {
public:
    /// wasm func 5396 (unit u17). Lazy singleton @1909148 (mutex @1909124), 36 bytes.
    static CRegistraDigitalOperador& GetInst();

    void StartState() override;                      // slot 2 (func 10454, srcloc :95, :98)
    void FinishState() override;                     // slot 5 (func 10453, srcloc :104, :105)
    void ProcessInput() override;                    // slot 7 (func 10452)
    void ProcessTick(uebyte tick) override;          // slot 8 (func 10451, srcloc :143, :144, :145)

private:
    CRegistraDigitalOperador();

    /// srcloc :328 / :371 - only exists inlined into func 3614: is the captured fingerprint one of the
    /// fingers of this mesário's record in the section roll?
    static bool BiometriaMesarioPresenteNosEleitores(const comum::md::CEleitorIdentidade& identidade);
    /// wasm func 3614 (the tools named it after the inlined function above)                name inferred
    static bool IdentificaMesario(const comum::md::CEleitorIdentidade& identidade, std::uint32_t idArquivo);
    static bool ProcuraMesarioRegistrado();          // three passes over CRegistradorMesario, inlined   name inferred
    static bool ProcuraEmDigitaisNaoRegistradas();   // inlined, name inferred
    static void SalvaDigitalNaoRegistrada();         // inlined CControladorReconhecimentoMesario::
                                                     //   SalvarBiometriaMesarioNaoRegistrado      name inferred

    // +0 vptr, +4 m_proximoEstado, +8..+10 CAppState flags (6 = keys + ticks)
    uebyte m_tickTempoEsgotado;                      // +11  15 s without a finger
    uebyte m_tickLeitura;                            // +12  50 ms sensor polling
    std::shared_ptr<api::CInteractiveForm<api::IScreenMT, api::IInputMT>> m_formCaptura;    // +16 "MESÁRIO: Posicione seu dedo..."
    std::shared_ptr<api::CInteractiveForm<api::IScreenMT, api::IInputMT>> m_formCapturada;  // +24 "Digital capturada / Por favor aguarde"
    uebyte m_tentativasRestantes = 3;                // +32  time-outs left before giving up
};

}  // namespace vota
