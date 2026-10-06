// uenux2/src/app/vota/eleitor/votamajoritario/cpedemajoritario.cpp
// Reconstructed from vota_web_wasm.wasm (unit u26). std::source_location records:
//   :44  CFormInterativoTelaVota CPedeMajoritario::GetTelaCargoAtual()           (func 5920 -> 6050)
//   :64  virtual void CPedeMajoritario::ProcessInputAudio()                       (func 11683)
//   :125 bool CPedeMajoritario::ExisteCandidatoValidoResposta(const std::string&) const   (inlined in 11683)
// Observed executing in the recorded votes: 11683 and 11684 (Prefeito "12" -> null vote).
#include "vota/eleitor/votamajoritario/cpedemajoritario.h"

#include "comum/dados/ccandidaturas.h"
#include "comum/dados/ccargos.h"
#include "comum/dados/cpartidos.h"
#include "comum/dados/crespostas.h"
#include "ecourna/api/util/cstringutils.h"
#include "vota/comum/votadefs.h"                        // CUeVotaError
#include "vota/eleitor/celeitorvotando.h"               // g_votoDigitado, g_votosEleitor, g_numeroEscolha
#include "vota/eleitor/cconferevotoemcargo.h"           // CConfereVotoEmCargo<>, CMajoritario* (unit u06)
#include "vota/log/clogvota.h"

namespace vota {

// cpedemajoritario.cpp:44 - wasm func 5920. Reconstructed by unit u07 (cpedemajoritario.u07.cpp):
//   if (CCargos::GetInst().IsEnd()) throw CUeVotaError(9380, "O cargo atual nao esta posicionado");
//   return CTelasVota::GetInst().GetTelaCargo(cargo atual, ETelaVotacao(0));

// wasm func 11682 (vtable slot 15). Audio template; the {tags} are replaced by FormataMensagem (slot 14).
std::string CPedeMajoritario::GetMensagemAudio() const
{
    if (comum::CCargos::GetInst().GetCurrent().EhConsulta())                 // CCargo +136 (optional engaged)
        return "{cargo-atual} {quantidade-digitos}. Voto {progresso}.";
    return "Você está votando para {cargo-atual}. {quantidade-digitos}. Voto {progresso}.";
}

// wasm func 11684 (vtable slot 10)
void CPedeMajoritario::StartStateAudio()
{
    GetTelaCargoAtual()->Exibe();                                            // form slot 2
    m_proximoEstado = this;
}

// cpedemajoritario.cpp:125 - inlined into 11683.
// A candidate that exists but is flagged at CCandidatura +52 ("inapto") counts as nonexistent, so the vote
// becomes a null vote (majoritarian cargos have no "candidato inapto" screen).
bool CPedeMajoritario::ExisteCandidatoValidoResposta(const std::string& numero) const
{
    const auto& cargo = comum::CCargos::GetInst().GetCurrent();
    const auto codigo = cargo.GetCodigo();
    const auto valor = ecourna::api::util::CStringUtils::ToDWord(numero);
    if (cargo.EhConsulta())
        return comum::CRespostas::GetInst().Busca(codigo, valor) != nullptr;          // shared_f1273 (func 2812)

    const auto* candidatura = comum::CCandidaturas::GetInst().Busca(codigo, valor);  // func 521 + shared_f1273
    if (candidatura == nullptr || candidatura->EhInapto())                           // +52 != 0     name inferred
        return false;

    auto& partidos = comum::CPartidos::GetInst();                                     // func 819
    const auto it = partidos.find(candidatura->GetPartido());                         // +2
    if (it == partidos.end()) {
        CLogVota::GetInst().LogaErroPartidoNaoEncontrado();                           // func 4550
        throw CUeVotaError(9382, "Nao foi encontrado o partido do candidato");        // :125
    }
    partidos.SetCurrent(it);                                                          // cursor at +12
    return true;
}

// Inlined into 11683 (name inferred). Scans the votes already confirmed in this session for the same office:
// only for offices with several vagas (Senador with 2 seats) and not when CCargo +14 is set.
bool CPedeMajoritario::VotoRepetido(const std::string& numero) const
{
    const auto& cargo = comum::CCargos::GetInst().GetCurrent();
    if (cargo.GetQtdEscolhas() == 1 || cargo.PermiteRepeticao() /* +14 ? */ || g_numeroEscolha == 1)
        return false;
    const std::size_t n = g_votosEleitor.size();
    for (unsigned escolha = 1; escolha < g_numeroEscolha; ++escolha) {
        const auto& [cargoVoto, voto] = g_votosEleitor.at(n - escolha);             // at(): out_of_range if short
        if (cargoVoto == cargo.GetCodigo() && voto.GetTipo() == comum::md::CVoto::NOMINAL /* 2 */
            && voto.GetNumero() == numero)
            return true;
    }
    return false;
}

// wasm func 11683 (vtable slot 9, srcloc :64)
void CPedeMajoritario::ProcessInputAudio()
{
    if (comum::CCargos::GetInst().IsEnd())
        throw CUeVotaError(9381, "O cargo atual nao esta posicionado");              // :64

    const auto [resultado, digitado] = EmiteEcoComInputField(GetTelaCargoAtual());   // slot 13 (func 3139)
    switch (resultado) {
    case api::EInputResult::Branco:                                                    // 3
        g_votoDigitado = "";
        m_proximoEstado = &CConfereVotoEmCargo<CMajoritarioBranco, ETelaVotacao(4)>::GetInst();     // @1838372
        break;
    case api::EInputResult::Confirma:                                                  // 9: field complete
        g_votoDigitado = digitado;
        if (!ExisteCandidatoValidoResposta(digitado))
            m_proximoEstado = &CConfereVotoEmCargo<CMajoritarioNulo, ETelaVotacao(7)>::GetInst();   // @1838408
        else if (VotoRepetido(digitado))
            m_proximoEstado = &CConfereVotoEmCargo<CMajoritarioRepetido, ETelaVotacao(16)>::GetInst();  // @1838444
        else
            m_proximoEstado = &CConfereVotoEmCargo<CMajoritarioValido, ETelaVotacao(2)>::GetInst();  // @1838480
        break;
    default:                                                                           // digit typed, CORRIGE...
        break;
    }
    // Each GetInst above is an inlined lazy singleton: new(40) + IConfereVotoEmCargo ctor (func 1165)
    // with the screen id, then the CConfereVotoEmCargo<> vptr.
}

} // namespace vota
