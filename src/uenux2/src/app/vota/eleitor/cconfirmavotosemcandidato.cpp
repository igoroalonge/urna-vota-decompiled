// Reconstructed from vota_web_wasm.wasm (unit u06). Original: uenux2/src/app/vota/eleitor/cconfirmavotosemcandidato.cpp
//
// srcloc evidence:
//   :64  CFormInterativoTelaVota vota::CConfirmaVotoSemCandidato::GetTelaCargoAtual(const std::string &)
//   :77  virtual std::pair<api::EInputResult, std::string>
//        vota::CConfirmaVotoSemCandidato::EmiteEcoComInputField(const CFormInterativoTelaVota &) const
// __PRETTY_FUNCTION__ strings: "virtual void vota::CConfirmaVotoSemCandidato::ProcessInputAudio()"
//                              "virtual void vota::CConfirmaVotoSemCandidato::StartStateAudio()"

#include "vota/eleitor/cconfirmavotosemcandidato.h"

#include "comum/dados/ccargos.h"
#include "vota/eleitor/celeitorvotando.h"
#include "vota/eleitor/comum/ctelasvota.h"

namespace vota {

CConfirmaVotoSemCandidato::CConfirmaVotoSemCandidato() : CVotacaoStateAudio(6) {}

// wasm func 4483 — thunk to the merged body func 3921 (error code 9307, srcloc :64)
CFormInterativoTelaVota CConfirmaVotoSemCandidato::GetTelaCargoAtual(const std::string& funcao)
{
    auto& cargos = comum::CCargos::GetInst();
    if (cargos.IsEnd())
        throw CUeVotaError(9307, funcao + " - o cargo atual nao esta posicionado",
                           std::source_location::current());
    return CTelasVota::GetInst().GetTelaCargo(cargos.GetCurrent().GetId(), m_tela);
}

// wasm func 7441 — vtable slot 10
void CConfirmaVotoSemCandidato::StartStateAudio()
{
    GetTelaCargoAtual(__PRETTY_FUNCTION__)->Exibe();
    m_proximoEstado = this;
}

// wasm func 7428 — vtable slot 9
void CConfirmaVotoSemCandidato::ProcessInputAudio()
{
    // The screen is a temporary: the binary releases the shared_ptr (and the __PRETTY_FUNCTION__
    // string) right after EmiteEcoComInputField (direct call: the class is final), before testing
    // the result; the returned pair lives until the end of the function.
    const auto [resultado, texto] = EmiteEcoComInputField(GetTelaCargoAtual(__PRETTY_FUNCTION__));
    if (resultado == api::EInputResult::Confirma) {
        const auto cargo = comum::CCargos::GetInst().GetCurrent().GetId();
        CEleitorVotando::GetInst().RegistraVoto(cargo, comum::md::CVoto::ETipo::NuloCargoSemCandidato, "");
        m_proximoEstado = nullptr;                          // cargo done
    }
}

// wasm func 4477 — vtable slot 13, srcloc :77
std::pair<api::EInputResult, std::string>
CConfirmaVotoSemCandidato::EmiteEcoComInputField(const CFormInterativoTelaVota& tela) const
{
    if (tela->GetInputs().size() != 1)
        throw CUeVotaError(9308, "Nao havia campos de input no formulario sem nada digitado",
                           std::source_location::current());

    const auto resultado = tela->Read();                    // CInteractiveForm::Read (cinteractiveform.h:57)
    // CORRIGE (5) and digits (13) are not accepted on this screen
    const bool indevida = resultado == api::EInputResult::Corrige || resultado == api::EInputResult::Tecla;
    PlayKey(indevida ? 0 : tela->GetInputs().at(0)->GetUltimaTecla());
    return {resultado, std::string{}};
}

// wasm func 7425 — vtable slot 15 (template later expanded by CVotacaoStateAudio slot 14)
std::string CConfirmaVotoSemCandidato::GetMensagemAudio() const
{
    return "Você está votando para {cargo-atual}, mas não há candidato para o cargo";
}

}  // namespace vota
