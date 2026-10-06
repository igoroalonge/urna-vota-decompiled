// Reconstructed from vota_web_wasm.wasm (unit u06). Original: uenux2/src/app/vota/eleitor/cajusteinicial.cpp
//
// CAjusteInicial ("initial adjustment") is the FIRST state of the voter thread on a real urna:
// CThreadEleitor::Run (func 7061) takes IAjusteInicial::GetInst() (cajusteinicial.cpp:57, pushes a
// CAjusteInicial if nobody registered another implementation) as initial state. Its StartState
//   1. adjusts the clock in demonstration / training mode         (AjustaDataHora,           :220 :245)
//   2. validates the battery power-off times of the configuration (ValidaTemposDesligamento,  :74 :79 :84)
//   3. wipes the result media (MR, pen drive) of a training urna  (LimpaMidiaResultado,       :191)
//   4. registers the mesário-attendance controller in demo mode   (inlined CPolySingletonList::push)
//   5. routes to the state that corresponds to the persisted EstadoGeralVota (InicioVota, :170 :178):
//      this is how the urna resumes after a power failure / reboot at any point of the election day.
//
// The web build does NOT use this state: main() of vota_web_wasm starts the cooperative executor
// with CAguardaMensagem (see docs/modules/u06-...). None of these functions ran in the recorded votes.
//
// RTTI: comum::CAppState <- vota::IAjusteInicial <- vota::CAjusteInicial (typeinfo @1534424, vtable @1534244)
//   [2] StartState = func 7160 (the analyzer named it after one of the inlined methods)
//
// EstadoVota C++ values = ASN.1 EstadoGeralVota.estadoVota + 49 ('1'...), EstadoEncerramento idem.
// EUrnaFase: '1' oficial, '2' simulado, '3' treinamento (CEstadoGeral +48).

#include "comum/cappstate.h"

#include <filesystem>
#include <format>
#include <memory>
#include <string>

#include "api/pattern/cpolysingletonlist.h"
#include "api/util/cdatetime.h"
#include "api/util/cdirreader.h"
#include "api/util/csystem.h"
#include "comum/cappinfo.h"
#include "comum/cpath.h"
#include "comum/dados/cconfiguracaoeleicao.h"
#include "comum/iinterfaceinit.h"
#include "vota/log/clogvota.h"

namespace vota {

class IAjusteInicial : public comum::CAppState {
public:
    static IAjusteInicial& GetInst();                       // srcloc :57, inlined into func 7061
};

class CAjusteInicial final : public IAjusteInicial {
public:
    void StartState() override;                             // slot 2, func 7160
private:
    void AjustaDataHora();                                  // :220, :245
    void ValidaTemposDesligamento();                        // :74, :79, :84
    void LimpaMidiaResultado();                             // :191
    void InicioVota();                                      // :170, :178
};

// ---------------------------------------------------------------------------------------------
// wasm func 7160 — vtable slot 2 (all five methods inlined)
void CAjusteInicial::StartState()
{
    AjustaDataHora();
    ValidaTemposDesligamento();
    LimpaMidiaResultado();

    // mesário registration enabled (real urna, PU registrarMesarios): the VOTA application records the
    // mesários' attendance itself. Correction by the u37 review: the test is func 1950 = IdentificaMesarios()
    // = !demo && PU +400, not EhModoDemonstracao(), so this never happens on a demonstration urna.
    if (comum::CConfiguracaoEleicao::GetInst().GetInformacaoEleicao().IdentificaMesarios() &&   // cfg +88, 1950
        !api::CPolySingletonList::exists<comum::IControladorRegistraMesarios>()) {              // func 2446
        api::CPolySingletonList::push<comum::IControladorRegistraMesarios>(
            std::make_unique<CControladorRegistraMesariosVota>());   // inlined push: "sz[{}] ptr[{}]",
                                                                    // "{}: instância já criada de {}"
    }

    InicioVota();
}

// inlined — srcloc :220 (IInterfaceInit), :245 (IAjusteDataHora)
void CAjusteInicial::AjustaDataHora()
{
    auto& app = comum::CAppInfo::GetInst();
    const bool demo = comum::IInterfaceInit::GetInst().GetDemoMode();                  // :220
    if (!demo && comum::GetEstadoGeral(app).GetFase() != EUrnaFase::Treinamento)       // CEstadoGeral +48 != '3'
        return;

    auto& estadoGeral = comum::GetEstadoGeral(app);
    const auto estadoVota = comum::GetEstadoVota(app).estadoVota;
    const auto& cfg = comum::CConfiguracaoEleicao::GetInst();

    const api::CDateTime agora;
    const auto antes = agora.ToTimeT();
    if (estadoVota > EEstadoVota::RegistroMesarioInicial)   // > 55: the vote already started
        return;

    api::CDateTime novo;
    if (comum::GetEstadoVota(app).treinamentoEleitor)       // EstadoGeralVota.treinamentoEleitor (+72)
        novo = cfg.GetDataHoraInicioVotacao();              // cfg +556 (date) / +564 (time)   ?name
    else
        novo = api::CDateTime(cfg.GetDataZeresima(), cfg.GetHoraZeresima() + 30);   // cfg +544/+552, +30 s ?name

    auto ajuste = estadoGeral.GetAjusteDataHora();          // CEstadoGeral +52
    ajuste.AdicionaDeltaT(static_cast<int>(novo.ToTimeT() - antes));
    estadoGeral.SetAjusteDataHora(ajuste);
    api::IAjusteDataHora::GetInst().AjustaDataHora(novo);   // :245
    comum::GravaEstadoGeral();                              // func 3333
}

// inlined — srcloc :74 / :79 / :84
void CAjusteInicial::ValidaTemposDesligamento()
{
    const auto& cfg = comum::CConfiguracaoEleicao::GetInst();
    const int limite = cfg.GetTempoLimiteBateria();         // +180 (seconds on internal battery)
    const int aviso = cfg.GetTempoAvisoDesligamento();      // +184
    if (limite <= 0)
        throw CUeVotaError(9405, "Tempo limite de uso de bateria interna inválido",
                           std::source_location::current());                                   // :74
    if (aviso <= 0)
        throw CUeVotaError(9406, "Tempo de aviso de desligamento por uso da bateria interna inválido",
                           std::source_location::current());                                   // :79
    if (limite < aviso)
        throw CUeVotaError(9407, "Tempo de aviso de desligamento não pode ser maior que o tempo para desligar a urna",
                           std::source_location::current());                                   // :84
}

// inlined — srcloc :191 (IInterfaceInit)
void CAjusteInicial::LimpaMidiaResultado()
{
    auto& app = comum::CAppInfo::GetInst();
    if (comum::GetEstadoGeral(app).GetFase() != EUrnaFase::Treinamento)
        return;
    if (comum::GetEstadoVota(app).treinamentoEleitor)
        return;

    auto& init = comum::IInterfaceInit::GetInst();          // :191
    HabilitaMR(init);                                       // func 2863 (below)
    if (!init.IsMRPresenteSemHabilitar()) {                 // SAVD request 18
        CLogVota::GetInst().LogaErro("Mídia de resultado não estava presente");   // func 5885 (severity 3)
        return;
    }
    HabilitaMR(init);
    if (init.IsMRMontadoSemHabilitar()) {
        init.DesmontarMRSemDesabilitar();
        init.EnviarMensagemThrowVoid(11, "desabilitando MR");
        api::Sleep(std::chrono::milliseconds(100));         // emscripten_sleep(100) if flag @1584624
    }
    HabilitaMR(init);
    init.MontarMRSemHabilitar();

    // delete everything on the MR except "infomidia*" and "turno2*"
    const std::string raiz = comum::CPath::GetPathMR();     // func 949: "/dsk/mr/"
    auto* dir = new api::CDirReader(raiz);
    if (!dir)
        return;
    while (dir->NextEntry()) {
        const std::string nome = dir->GetEntry()->d_name;
        if (nome.starts_with("infomidia") || nome.starts_with("turno2"))
            continue;
        api::CSystem::SecureRemove(std::filesystem::path(comum::CPath::GetPathMR()) / nome);   // func 2760
    }
    delete dir;                                             // func 1913 + free
}

// inlined — srcloc :170 / :178
void CAjusteInicial::InicioVota()
{
    const auto& estado = comum::GetEstadoVota(comum::CAppInfo::GetInst());
    switch (estado.estadoVota) {
    case EEstadoVota::Inicial:                  // 49
    case EEstadoVota::GeraBaseDinamica:         // 50
    case EEstadoVota::AguardaHoraZeresima:      // 51
        m_proximoEstado = &CVerificaEleicaoPassou::GetInst();           break;
    case EEstadoVota::GerarZe:                  // 52
        m_proximoEstado = &CInicioZeresima::GetInst();                  break;   // func 3865
    case EEstadoVota::ZeresimaGerada:           // 53
        m_proximoEstado = &CImprimindoZeresima::GetInst();              break;   // func 5973
    case EEstadoVota::ZeresimaImpressa:         // 54
    case EEstadoVota::RegistroMesarioInicial:   // 55
    case EEstadoVota::Votar:                    // 56
        m_proximoEstado = &testeteclado::CRetomada::GetInst();          break;   // keypad test, then resume
    case EEstadoVota::FimAquisicaoVotos:        // 57
        m_proximoEstado = &CFinalizaAquisicao::GetInst();               break;
    case EEstadoVota::RegistroMesarioFinal:     // 58
        m_proximoEstado = &CReinicioComparecimentoMesario::GetInst();   break;   // func 3864
    case EEstadoVota::GerarBU:                  // 59
        m_proximoEstado = &CGeraBU::GetInst();                          break;   // func 6129
    case EEstadoVota::GerarRelatorios:          // 60
        m_proximoEstado = &CGeraRelatorios::GetInst();                  break;   // func 6084
    case EEstadoVota::ImprimirBU:               // 61
        m_proximoEstado = &CInicioBU::GetInst();                        break;   // func 5983
    case EEstadoVota::GravarResultados:         // 62
        m_proximoEstado = &CGravaResultado::GetInst();                  break;   // func 6037
    case EEstadoVota::CopiaResultadosMR:        // 63
        m_proximoEstado = &CCopiaResultadoParaMR::GetInst();            break;   // func 6174
    case EEstadoVota::Encerrada:                // 64
        switch (estado.estadoEncerramento) {
        case EEstadoEncerramento::Inicial:                  // 49
        case EEstadoEncerramento::ImprimirObrigatoriaBU:    // 50
            m_proximoEstado = &CImprimirBUOutrasObrigatorias::GetInst(); break;  // func 5984
        case EEstadoEncerramento::RetirarMR:                // 51
            m_proximoEstado = &CRetirarMR::GetInst();                    break;  // func 2291
        case EEstadoEncerramento::FimDosTrabalhos:          // 52
            m_proximoEstado = &CVerificaQtdBUsAdicionais::GetInst();     break;
        default:
            CLogVota::GetInst().LogaErro("Erro estado do aplicativo não conhecido");   // func 3279
            // The enum is passed as is: format arg type 15 (handle) = a user std::formatter
            // (ICF body func 536 formats it as an unsigned integer, e.g. "... = 53").
            throw CUeVotaError(9303, std::format("Estado encerramento nao conhecido = {}",
                                                 estado.estadoEncerramento),
                               std::source_location::current());                      // :170
        }
        break;
    default:                                    // 65 = exibealertadesligamento is not handled
        CLogVota::GetInst().LogaErro("Erro estado do aplicativo não conhecido");
        throw CUeVotaError(9304, std::format("Estado nao conhecido = {}", estado.estadoVota),   // handle arg
                           std::source_location::current());                          // :178
    }
}

// =============================================================================================
// Functions of unit u06 attributed to this file by caller evidence, with their probable owner
// =============================================================================================

// wasm func 2760 — probably api::CSystem::SecureRemove (uenux2/src/api/util/csystem.cpp, path
// inferred: it sits between CSystem::GetFileSize (2759) and CSystem::ZeroFill (2761)). Also called
// by func 1487 (removes "dinamico/imprimindo") and func 6737 (votaInit / CGeraDadosDinamicos).
// Observed executing (from votaInit). name inferred
namespace api {
void CSystem::SecureRemove(const std::string& caminho)
{
    struct stat st;
    // never follow a symbolic link when overwriting
    if (::lstat(caminho.c_str(), &st) == -1 || !S_ISLNK(st.st_mode))
        ZeroFill(caminho);                  // overwrites regular files with zeros (func 2761)
    ::remove(caminho.c_str());
}
}  // namespace api

// wasm func 2863 (not in this unit) — HabilitaMR(IInterfaceInit&):
//   init.EnviarMensagemThrowVoid(10, "habilitando MR"); api::Sleep(500ms) /* emscripten_sleep */

// wasm func 4152 — builds the body of a "please wait" status screen (name inferred):
//   void CriaTelaStatusAguarde(api::CFormBuilder& fb, const std::string& titulo) {
//       fb.AddStatusHeader(5);
//       fb.AddLabel(titulo, ...);                       // api_f202, font 474888
//       fb.AddLabel("Por favor, aguarde...", ...);      // font 474992
//   }
// wasm func 6583 — builds "telaPreparandoDadosEncerramento" (name inferred):
//   CFormInterativoTelaVota CriaTelaPreparandoDadosEncerramento() {
//       api::CFormBuilder fb; CriaTelaStatusAguarde(fb, "Preparando dados para encerramento");
//       return CFormVota(fb, "telaPreparandoDadosEncerramento");       // comum_f886
//   }
//
// Lazy singletons of end-of-day states (constructor inlined; classes in eleitor/fimvotacao/, units u08/u09):
// wasm func 6129 — CGeraBU& CGeraBU::GetInst():  CAppState(0);
//                    +12 = CFormVota(fb{CriaTelaStatusAguarde("Votação encerrada")}, "telaVotacaoEncerrada")
//                    +20 = CriaTelaPreparandoDadosEncerramento()
// wasm func 6084 — CGeraRelatorios& CGeraRelatorios::GetInst():  CAppState(0);
//                    +12 = CriaTelaPreparandoDadosEncerramento()
// wasm func 6174 — CCopiaResultadoParaMR& CCopiaResultadoParaMR::GetInst():  CAppState(4 = ticks);
//                    +12 = CFormVota(fb{CriaTelaStatusAguarde("Gravando o resultado na mídia")},
//                                    "telaCopiaResultadoParaMR")
// wasm func 5973 — CImprimindoZeresima& CImprimindoZeresima::GetInst()  (func 764 generic 12-byte
//                    CAppState singleton, flags 0)
// wasm func 11939 — vota::CGeraResumoZeresima vtable slot 10 (iniciovotacao/cgeradorresumozeresima.cpp):
//   comum::CAppState* CGeraResumoZeresima::GetProximoEstado() { return &CImprimindoZeresima::GetInst(); }
//   (slot 9, func 11940, sets estadoVota = 53 "zeresimagerada" and saves the state)

}  // namespace vota
