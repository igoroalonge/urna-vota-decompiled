// Reconstructed from vota_web_wasm.wasm (unit u06).
// Original: uenux2/src/app/vota/eleitor/cconfirmavotosemcandidato.h (path inferred)
//
// Sub-state used for a cargo that has no candidate in this urna (and is not a consulta): the voter
// only sees "Tela de Cargo sem Candidato" and must press CONFIRMA; the vote is stored as RDV
// TipoVoto nuloCargoSemCandidato (8). Created by CEleitorVotando::ChamaEstadoProximoCargo.
//
// RTTI: CVotacaoStateAudio <- CConfirmaVotoSemCandidato (typeinfo @1533116, vtable @1533020)
#pragma once

#include <string>
#include <utility>

#include "vota/eleitor/cvotacaostateaudio.h"
#include "vota/eleitor/comum/ctelascargo.h"

namespace vota {

class CConfirmaVotoSemCandidato final : public CVotacaoStateAudio {
public:
    /// Inline singleton inside CEleitorVotando::ChamaEstadoProximoCargo (func 4459):
    ///   new (32 bytes) CConfirmaVotoSemCandidato : CVotacaoStateAudio(6), m_tela = CargoSemCandidato (17)
    static CConfirmaVotoSemCandidato& GetInst();

    void ProcessInputAudio() override;                  // slot 9  func 7428
    void StartStateAudio() override;                    // slot 10 func 7441
    /// slot 12 (pure in CVotacaoStateAudio): no-op override (func 425, own table slot 919)
    void ProcessTickAudio(uebyte) override {}
    std::pair<api::EInputResult, std::string>
        EmiteEcoComInputField(const CFormInterativoTelaVota& tela) const override;   // slot 13 func 4477
    std::string GetMensagemAudio() const override;      // slot 15 func 7425 name inferred

private:
    CConfirmaVotoSemCandidato();
    CFormInterativoTelaVota GetTelaCargoAtual(const std::string& funcao);   // func 4483 (srcloc :64)

    ETelaVotacao m_tela = ETelaVotacao::CargoSemCandidato;   // +28
};

}  // namespace vota
