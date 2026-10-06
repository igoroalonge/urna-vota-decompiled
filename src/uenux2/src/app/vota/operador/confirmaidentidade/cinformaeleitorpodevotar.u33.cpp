// uenux2/src/app/vota/operador/confirmaidentidade/{cinformaeleitorpodevotar,cinformaanodesabilitadodemo,
// cinformabiodesabilitadademo}.cpp  -- FRAGMENT written by unit u33 (paths inferred; the constructors and
// GetInst of these states are in operador/u27-foreign-fragments.cpp and cnomeeleitor.cpp).
//
// The three microterminal states that end the voter's identification with "press CONFIRMA to release":
//   vota::CInformaEleitorPodeVotar    "ELEITOR(A) PODE VOTAR / Assinar o caderno de votação antes de votar"
//   vota::CInformaAnoDesabilitadoDemo birth-year check disabled (modo demonstração)
//   vota::CInformaBioDesabilitadaDemo fingerprint check disabled (modo demonstração)
// Their ProcessInput() methods (slot 7: 10496, 10504, 10508) are 1-line thunks into one merged body; the
// only difference is the std::source_location of the IInputMT lookup inside CInteractiveForm::Read
// (cinteractiveform.h:57, records @1591408 / @1591232 / @1591160).
// Dead code in the web build (the operator thread never runs; votaInit posts message 1 itself).
#include "api/gui/cinteractiveform.h"
#include "api/ipc/cmessagequeue.h"
#include "vota/eleitor/cthreadeleitor.h"
#include "vota/operador/aguardaeleitor/cmostraeleitorvotando.h"
#include "vota/operador/comum/iinformacaothreadoperador.h"
#include "vota/operador/confirmaidentidade/chabilitaaudioeleitor.h"

namespace vota {

// wasm func 3883 - merged body of the three ProcessInput() methods                  // name inferred
// "Release the voter" (liberar o eleitor): on CONFIRMA either ask the mesário to enable audio, or tell the
// voter thread to start (MSG_INICIA_ELEITOR = 1) and switch the MT to "eleitor votando".
// The same sequence is inlined in CNomeEleitor::HabilitaEleitorSemBiometria (5400),
// CDigitalReconhecida::ProcessInput (10481) and CControlaReconhecimento::ProcessInput (10511).
template <class ESTADO>
static void LiberaEleitorSeConfirmado(ESTADO& estado)
{
    if (estado.m_form->Read() != api::EInputResult::Confirma)                  // 9; form at +12 (enum of gui-common.u15.h)
        return;
    if (impl::IInformacaoThreadOperador::GetInst().DeveHabilitarAudio()) {     // func 2746 (slot 3)
        estado.m_proximoEstado = &CHabilitaAudioEleitor::GetInst();            // func 2743
        return;
    }
    auto& fila = CThreadEleitor::GetInst().Fila();                            // func 316, +36
    fila.Add(api::SMessage{CThreadEleitor::MSG_INICIA_ELEITOR, &fila}, 1);     // rhvoice_f501 (Add)
    estado.m_proximoEstado = &CMostraEleitorVotando::GetInst();                // func 1150
}

void CInformaEleitorPodeVotar::ProcessInput()    { LiberaEleitorSeConfirmado(*this); }   // 10496
void CInformaAnoDesabilitadoDemo::ProcessInput() { LiberaEleitorSeConfirmado(*this); }   // 10504
void CInformaBioDesabilitadaDemo::ProcessInput() { LiberaEleitorSeConfirmado(*this); }   // 10508

} // namespace vota
