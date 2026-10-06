// uenux2/src/app/comum/dados/celeitores.cpp (attested file) -- FRAGMENT written by unit u37.
#include "comum/dados/celeitores.h"

namespace comum {

// wasm func 1393 (tools: vota_f1393)                                                  name inferred
// Out-of-line copy of an inline accessor: the biometric record (encrypted fingerprint templates, WSQ
// state, decryption status) of the voter the roll is currently positioned on, returned BY VALUE.
// `GetCurrent()` is api::CDataMap<CEleitorIdentidade, CEleitorDetalhe>::GetCurrent (wasm 656) on the data
// map at +4; CEleitorDetalhe::GetBiometria is wasm 1937.
// Callers (operator terminal, fingerprint identification): CMostraEleitorVotando::SalvaHabilitacaoEleitor,
// CPedeDigital::ProcessTick, CDigitalNaoReconhecida::StartState, CDigitalNaoReconhecidaDecBiometria slot 2,
// CNomeEleitor::NavegaBiometrica, CControlaReconhecimento::StartState.
md::CBiometriaEleitor CEleitores::GetBiometriaCorrente() const
{
    return m_dados.GetCurrent()->GetBiometria();
}

} // namespace comum
