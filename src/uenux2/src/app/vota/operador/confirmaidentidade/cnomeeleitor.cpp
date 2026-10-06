// Reconstructed from vota_web_wasm.wasm (unit u10).
// Original: uenux2/src/app/vota/operador/confirmaidentidade/cnomeeleitor.cpp
// (attested by std::source_location records: NavegaBiometrica line 145, NavegaAnoNascimento line 156).
#include "vota/operador/confirmaidentidade/cnomeeleitor.h"

#include "comum/appinfo/cappinfo.h"
#include "comum/cconfiguracaoeleicao.h"
#include "comum/clocal.h"
#include "comum/dados/celeitores.h"
#include "comum/iinterfaceinit.h"
#include "vota/eleitor/cthreadeleitor.h"
#include "vota/log/clogvota.h"
#include "vota/operador/aguardaeleitor/cmostraeleitorvotando.h"
#include "vota/operador/comum/capresentacaofotoeleitor.h"            // vota::ApresentaFotoEleitor (func 3616, u05)
#include "vota/operador/comum/ccancelahabilitacaoeleitor.h"
#include "vota/operador/comum/chabilitaaudioeleitor.h"
#include "vota/operador/comum/iinformacaothreadoperador.h"
#include "vota/operador/confirmaidentidade/ccontrolareconhecimento.h"
#include "vota/operador/confirmaidentidade/cinformaanodesabilitadodemo.h"
#include "vota/operador/confirmaidentidade/cinformabiodesabilitadademo.h"
#include "vota/operador/confirmaidentidade/cpedeanonascimentosembiometria.h"

namespace vota {

namespace {

// Voter has usable biometrics in a biometric section: the urna is biometric (CLocal, funcs 401/820),
// the roll marks the voter with biometrics (CEleitor +100) and the decrypted record has fingers.
bool PossuiBiometriaUtilizavel()                                                  // inlined, name inferred
{
    if (!comum::CLocal::GetInst().UrnaBiometrica())
        return false;
    const comum::CEleitorDetalhe& eleitor = comum::CEleitores::GetInst().GetCurrent();
    if (!eleitor.GetEleitor().PossuiBiometria())
        return false;
    return eleitor.GetBiometria().GetDedos().has_value();                        // func 1937
}

}  // namespace

// ---------------------------------------------------------------------------------------------
// wasm func 5400 (tools: "vota_f5400")                                         name inferred
// Releases the voter without fingerprint: marks the habilitação type, then either asks the mesário
// to enable audio (voter with a special need or audio switched on manually) or tells the voter thread
// to start the vote (message 1) and shows CMostraEleitorVotando.
void CNomeEleitor::HabilitaEleitorSemBiometria()
{
    auto& info = impl::IInformacaoThreadOperador::GetInst();
    info.SetHabilitacaoSemBiometria();                                            // slot 14 (api_f3621)
    if (info.DeveHabilitarAudio()) {                                              // slot 3 (api_f2746)
        m_proximoEstado = &CHabilitaAudioEleitor::GetInst();                      // func 2743
        return;
    }
    CThreadEleitor::GetInst().EnviaMensagem({CThreadEleitor::MSG_INICIA_ELEITOR}, 1);
    m_proximoEstado = &CMostraEleitorVotando::GetInst();                          // func 1150
}

// srcloc line 145 - inlined into ProcessInput.
void CNomeEleitor::NavegaBiometrica()
{
    if (comum::IInterfaceInit::GetInst().GetDemoMode())                           // line 145 (PolySingleton 611)
        m_proximoEstado = &CInformaBioDesabilitadaDemo::GetInst();                // func 5402
    else
        m_proximoEstado = &CControlaReconhecimento::GetInst();                    // func 1256
}

// srcloc line 156 - inlined into ProcessInput.
void CNomeEleitor::NavegaAnoNascimento()
{
    if (comum::IInterfaceInit::GetInst().GetDemoMode()) {                         // line 156
        m_proximoEstado = &CInformaAnoDesabilitadoDemo::GetInst();                // GetInst + ctor inlined (@1908812)
        return;
    }
    impl::IInformacaoThreadOperador::GetInst().SetHabilitacaoSemBiometria();     // slot 14
    m_proximoEstado = &CPedeAnoNascimentoSemBiometria::GetInst();                 // GetInst + ctor inlined (@1908924)
}

// ---------------------------------------------------------------------------------------------
// wasm func 10501 - vtable slot 2                                             name from slot order
void CNomeEleitor::StartState()
{
    if (comum::EhTreinamentoEleitor()) {                                          // func 697
        HabilitaEleitorSemBiometria();                                            // voter training: no checks
        return;
    }
    ApresentaFotoEleitor();                                                       // func 3616 (u05): photo on the MT LCD
    m_form->Show();
    m_proximoEstado = this;
}

// ---------------------------------------------------------------------------------------------
// wasm func 10500 - vtable slot 7. The tools named it NavegaBiometrica after the first inlined srcloc.
void CNomeEleitor::ProcessInput()
{
    switch (m_form->Read()) {                                                     // cinteractiveform.h:57 inlined
    case api::EInputResult::CORRIGE:
        CLogVota::GetInst().Loga("Habilitação cancelada durante confirmação de dado do eleitor");   // func 2097
        m_proximoEstado = &CCancelaHabilitacaoEleitor::GetInst();                 // func 1536
        break;

    case api::EInputResult::CONFIRMA: {
        const auto& cfg = comum::CConfiguracaoEleicao::GetInst();
        if (cfg.GetPedeAnoNascimentoEleitor()) {                                  // ParametrosUrna.pedeAnoNascimentoEleitor (+489)
            bool biometria = PossuiBiometriaUtilizavel();
            if (biometria && EhTreinamentoSemTreinamentoEleitor()) {             // func 1823
                // training of mesários: the last sixth of the roll plays "voters without biometrics"
                const auto& eleitores = comum::CEleitores::GetInst();
                const int total = static_cast<int>(eleitores.size());
                const int seq = eleitores.GetCurrent().GetEleitor().GetSequencial();
                biometria = !(seq > total / 6 * 5 && seq <= total);
            }
            if (biometria && cfg.GetUtilizaBiometria())                           // cfg +665
                NavegaBiometrica();
            else
                NavegaAnoNascimento();
        } else if (cfg.GetUtilizaBiometria() && comum::CLocal::GetInst().UrnaBiometrica()) {
            NavegaBiometrica();
        } else {
            HabilitaEleitorSemBiometria();
        }
        break;
    }
    default:
        break;
    }
}

// wasm func 2097 (tools: "vota_f2097")                                         name inferred
// CLogVota helper: builds the 60-byte literal "Habilitação cancelada durante confirmação de dado do
// eleitor" and logs it with level 1 (api_f233). Shared by CVerificaDadoEleitor, CDadoEleitorNaoConfere,
// CPedeAnoNascimentoSemBiometria, CValidaIdentidade and this file.

}  // namespace vota
