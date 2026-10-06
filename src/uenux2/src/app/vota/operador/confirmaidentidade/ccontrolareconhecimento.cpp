// Reconstructed from vota_web_wasm.wasm (unit u10).
// Original: uenux2/src/app/vota/operador/confirmaidentidade/ccontrolareconhecimento.cpp
// (attested by std::source_location records: AvancaTentativa line 167, AvancaProximoDedo1x4 line 187).
#include "vota/operador/confirmaidentidade/ccontrolareconhecimento.h"

#include "comum/appinfo/cappinfo.h"
#include "comum/cconfiguracaoeleicao.h"
#include "comum/dados/celeitores.h"
#include "vota/eleitor/cthreadeleitor.h"
#include "vota/log/clogvota.h"
#include "vota/operador/aguardaeleitor/cmostraeleitorvotando.h"
#include "vota/operador/comum/chabilitaaudioeleitor.h"
#include "vota/operador/comum/iinformacaothreadoperador.h"
#include "vota/operador/confirmaidentidade/cpededigital.h"
#include "vota/comum/votadefs.h"

namespace vota {

int                 CControlaReconhecimento::s_tentativa = 1;              // data segment: 1
int                 CControlaReconhecimento::s_verificacoesDadoEleitor = 1;
int                 CControlaReconhecimento::s_tentativaDigitalSalva = 1;
uebyte              CControlaReconhecimento::s_indiceDedo = 0;
std::vector<uebyte> CControlaReconhecimento::s_digitalCapturada;
std::vector<uebyte> CControlaReconhecimento::s_digitalMesario;
std::string         CControlaReconhecimento::s_tituloMesario;
std::uint16_t       CControlaReconhecimento::s_score = 0;
int                 CControlaReconhecimento::s_qtdEleitores = 0;
int                 CControlaReconhecimento::s_faixaTentativa[4] = {};

// ---------------------------------------------------------------------------------------------
// wasm func 5406 (srcloc line 167)
void CControlaReconhecimento::AvancaTentativa()
{
    if (comum::CConfiguracaoEleicao::GetInst().GetNumTentativasHabilitacao() == s_tentativa)   // cfg +136 (uebyte)
        throw CUeVotaError(EUeVotaError{9397}, "Limite de tentativas atingido.");
    s_indiceDedo = 0;               // every new attempt restarts the 1x4 comparison at the right thumb
    ++s_tentativa;
}

// srcloc line 187 - only exists inlined into CPedeDigital::ProcessTick (func 10465).
void CControlaReconhecimento::AvancaProximoDedo1x4()
{
    if (s_indiceDedo == 4)
        throw CUeVotaError(EUeVotaError{9399}, "Fim da verificação 1x4 atingido.");
    ++s_indiceDedo;
}

// wasm func 5405 (tools: "api_f5405", attributed to cdigitalnaoreconhecida.cpp)   name inferred
bool CControlaReconhecimento::EhUltimaTentativa()
{
    return comum::CConfiguracaoEleicao::GetInst().GetNumTentativasHabilitacao() == s_tentativa;
}

// wasm func 5403 = EhPrimeiraTentativa() (inline in the header, emitted out of line).

// Only exists inlined (CDigitalNaoReconhecida::StartState, func 10474).       name inferred
uebyte CControlaReconhecimento::GetNumTentativas() const
{
    return comum::CConfiguracaoEleicao::GetInst().GetNumTentativasHabilitacao();         // cfg +136
}

// ---------------------------------------------------------------------------------------------
// wasm func 10512 - vtable slot 2                                             name from slot order
void CControlaReconhecimento::StartState()
{
    s_verificacoesDadoEleitor = 1;
    s_tentativa = 1;
    s_indiceDedo = 0;
    s_tentativaDigitalSalva = 1;
    s_digitalCapturada.clear();
    s_digitalMesario.clear();

    comum::CEleitores& eleitores = comum::CEleitores::GetInst();
    if (EhTreinamentoSemTreinamentoEleitor()) {                              // func 1823: training of mesários
        // Simulated biometrics: the roll (size N) is cut into sixths by sequential number. Voters of
        // the last sixth (5N/6 < seq <= N) behave as voters WITHOUT biometrics.
        s_qtdEleitores = static_cast<int>(eleitores.size());                 // CDataMap size (+12)
        const int sexto = s_qtdEleitores / 6;
        for (int& faixa : s_faixaTentativa)
            faixa = sexto;
        const int sequencial = eleitores.GetCurrent().GetEleitor().GetSequencial();   // +0
        if (sequencial > sexto * 5 && sequencial <= s_qtdEleitores) {
            m_form->Show();                                                  // "ELEITOR(A) PODE VOTAR"
            m_proximoEstado = this;
        } else {
            m_proximoEstado = &CPedeDigital::GetInst();                      // func 2740
        }
        return;
    }

    const comum::CEleitorDetalhe& eleitor = eleitores.GetCurrent();
    if (eleitor.GetEleitor().PossuiBiometria()                               // CEleitor +100
        && eleitor.GetBiometria().GetDedos().has_value()) {                  // func 1937; optional<map> flag +32
        m_proximoEstado = &CPedeDigital::GetInst();
        return;
    }
    CLogVota::GetInst().Loga(2, "O eleitor não possui biometria");            // api_f1398 = CLoga::loga(level 2)
    m_form->Show();
    m_proximoEstado = this;
}

// ---------------------------------------------------------------------------------------------
// wasm func 10511 - vtable slot 7                                             name from slot order
// Voter without (usable) biometrics: CONFIRMA releases him without fingerprint.
void CControlaReconhecimento::ProcessInput()
{
    if (m_form->Read() != api::EInputResult::CONFIRMA)                       // cinteractiveform.h:57 inlined
        return;
    auto& info = impl::IInformacaoThreadOperador::GetInst();
    info.SetHabilitacaoSemBiometria();                                        // slot 14 (api_f3621)
    if (info.DeveHabilitarAudio()) {                                          // slot 3 (api_f2746)
        m_proximoEstado = &CHabilitaAudioEleitor::GetInst();                  // func 2743
    } else {
        CThreadEleitor::GetInst().EnviaMensagem({CThreadEleitor::MSG_INICIA_ELEITOR}, 1);   // 1: release the urna
        m_proximoEstado = &CMostraEleitorVotando::GetInst();                  // func 1150
    }
}

}  // namespace vota
