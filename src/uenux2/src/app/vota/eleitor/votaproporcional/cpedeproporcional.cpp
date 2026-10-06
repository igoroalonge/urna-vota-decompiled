// uenux2/src/app/vota/eleitor/votaproporcional/cpedeproporcional.cpp
// Reconstructed from vota_web_wasm.wasm (unit u26). std::source_location records:
//   :42 CFormInterativoTelaVota CPedeProporcional::GetTelaCargoAtual()   (func 5923 -> merged 6050)
//   :84 virtual void CPedeProporcional::ProcessInputAudio()              (func 11711)
// Observed executing in the recorded votes: 5923, 11711, 11712 (Vereador 91001: party "91").
// Also here: wasm func 5922 = vota::LegendaValida (no srcloc; filed in this file by the database, next to
// its caller ProcessInputAudio; the web state JSON of vota_web_wasm.cpp calls it too).
#include "vota/eleitor/votaproporcional/cpedeproporcional.h"

#include "comum/dados/ccandidaturas.h"                  // comum::CCandidaturas (func 521), used by LegendaValida
#include "comum/dados/ccargos.h"
#include "comum/dados/cfederacoes.h"                    // comum::CFederacoes (func 2832), used by LegendaValida
#include "comum/dados/cpartidos.h"
#include "ecourna/api/util/cstringutils.h"
#include "vota/comum/votadefs.h"
#include "vota/eleitor/celeitorvotando.h"               // g_votoDigitado
#include "vota/eleitor/cconferevotoemcargo.h"           // CConfereVotoEmCargo<CProporcionalBranco, 4>
#include "vota/eleitor/votaproporcional/cpedenominal.h" // CPedeNominal (other unit)
#include "vota/eleitor/votaproporcional/cpedenulo.h"

namespace vota {

// cpedeproporcional.cpp:42 - wasm func 5923 (observed). Same merged body as CPedeMajoritario::GetTelaCargoAtual
// (func 6050, see cpedemajoritario.u07.cpp) with error 9386:
//   if (CCargos::GetInst().IsEnd()) throw CUeVotaError(9386, "O cargo atual nao esta posicionado");   // :42
//   return CTelasVota::GetInst().GetTelaCargo(cargo atual, ETelaVotacao(0));

// wasm func 5922 (table slot 90)                                                          name inferred
// Decides the voto de legenda: true when the two digits typed for a proportional cargo are a party that
// exists and can receive votes for this cargo. Callers: CPedeProporcional::ProcessInputAudio (11711, below)
// and the web state JSON (CVotaWebEngine::BuildStateJson, func 5500, "legendaValida").
bool LegendaValida(comum::TCargoID cargo, comum::TPartidoID partido)
{
    auto& partidos = comum::CPartidos::GetInst();                                // func 819
    if (partidos.find(partido) == partidos.end())                                // std::map lookup, inlined
        return false;

    // An apt candidacy of the party itself for this cargo: the same test as GetNumerosCandidatosAptos
    // (func 2272), written as an any_of over the map (situação +52 == 0, cargo +0, partido +2 of the CCandidatura).
    auto& candidaturas = comum::CCandidaturas::GetInst();                        // func 521
    for (const auto& [chave, c] : candidaturas)
        if (c.GetSituacao() == 0 && c.GetCargo() == cargo && c.GetPartido() == partido)
            return true;

    // Otherwise the party's federação may have one (the same two helpers as func 11976, PartidoTemCandidatos).
    const auto* federacao = comum::CFederacoes::GetInst().GetFederacaoDoPartido(partido);   // 2832, 5799  ?
    return federacao != nullptr && candidaturas.FederacaoTemCandidatos(cargo, *federacao);  // 5809       ?
}

// wasm func 11710 (vtable slot 15)
std::string CPedeProporcional::GetMensagemAudio() const
{
    return "Você está votando para {cargo-atual}. {quantidade-digitos}. Voto {progresso}.";
}

// wasm func 11712 (vtable slot 10, observed)
void CPedeProporcional::StartStateAudio()
{
    GetTelaCargoAtual()->Exibe();
    m_proximoEstado = this;
}

// wasm func 11711 (vtable slot 9, srcloc :84, observed)
void CPedeProporcional::ProcessInputAudio()
{
    auto& cargos = comum::CCargos::GetInst();                                    // func 273
    if (cargos.IsEnd())                                                          // shared_f602
        throw CUeVotaError(9387, "O cargo atual nao esta posicionado");         // :84
    const auto& cargo = cargos.GetCurrent();

    const auto [resultado, digitado] = EmiteEcoComInputField(GetTelaCargoAtual());   // slot 13 (func 3139)
    switch (resultado) {
    case api::EInputResult::Branco:                                               // 3
        g_votoDigitado = "";
        m_proximoEstado = &CConfereVotoEmCargo<CProporcionalBranco, ETelaVotacao(4)>::GetInst();   // @1838140
        break;

    case api::EInputResult::Confirma: {                                           // 9: the two party digits are in
        g_votoDigitado = digitado;
        const auto partido = ecourna::api::util::CStringUtils::ToWord(digitado);
        auto& partidos = comum::CPartidos::GetInst();                             // func 819
        const auto it = partidos.find(partido);
        bool legendaValida = false;
        if (it != partidos.end()) {
            partidos.SetCurrent(it);                                              // cursor +12
            legendaValida = LegendaValida(cargo.GetCodigo(), partido);            // func 5922 (party has candidates)
        }
        if (legendaValida)
            m_proximoEstado = &CPedeNominal::GetInst();      // lazy @1837972: new(32), CCompletaProporcional(ETelaVotacao 8)
        else
            m_proximoEstado = &CPedeNulo::GetInst();         // lazy @1837788: new(32), CCompletaProporcional(ETelaVotacao 5)
        break;
    }

    default:
        break;
    }
}

} // namespace vota
