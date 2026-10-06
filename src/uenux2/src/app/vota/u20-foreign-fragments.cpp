// FRAGMENTS reconstructed by unit u20 from vota_web_wasm.wasm.
// vota:: state-machine methods that the tools attributed to unit u20 (they inline CAppInfo getters of
// comum/appinfo/cappinfo.cpp). Each section names the original file (paths inferred unless a srcloc is
// quoted) and the unit(s) that own the rest of it.
//
// comum::CAppState slot protocol (unit u06): 2 StartState, 3 NeedChangeState, 4 GetNextState,
// 5 FinishState, 6 ProcessMessage(uebyte), 7 ProcessInput, 8 ProcessTick(uebyte); +4 m_proximoEstado.
// EstadoVota values are chars ('1' + ASN.1 value): '3' AGUARDAHORAZERESIMA, '4' GERARZE,
// '5' ZERESIMAGERADA, '6' ZERESIMAIMPRESSA, '7' REGISTROMESARIOINICIAL, '8' VOTAR, '9' FIMAQUISICAOVOTOS,
// ':' REGISTROMESARIOFINAL.
#include <memory>
#include <mutex>
#include <string>

#include "api/gui/cinteractiveform.h"
#include "api/util/cwait.h"
#include "comum/appinfo/cappinfo.h"
#include "comum/dados/celeitores.h"
#include "comum/dados/crdvvota.h"
#include "vota/eleitor/cthreadeleitor.h"
#include "vota/eleitor/comum/ctelasvota.h"
#include "vota/log/clogvota.h"

using comum::CAppInfo;
using comum::EUrnaTurno;
using EstadoVota = comum::md::estadoaplicacao::EEstadoVota;

// =====================================================================================================
// uenux2/src/app/vota/operador/csincronismooperador.cpp  (path inferred; ProcessMessage = func 10209, u17)
// =====================================================================================================
namespace comum {
// Inlined into func 10210.                                        srclocs celeitores.cpp:456 / :460
void CEleitores::MarcaVotou()
{
    CEleitorDetalhe* atual = m_dados.GetCurrent();                                     // func 656, +4
    if (!atual)
        throw CUeComumDadosError(EUeComumDadosError{7845}, "Item inexistente");
    if (!atual->DinamicoCarregado())                                                    // +192
        throw CUeComumDadosError(EUeComumDadosError{7846}, "Dados dinâmicos não carregados");
    if (atual->GetDinamico().GetSituacao() != md::ESituacaoEleitor::Votou) {            // +8 != 3
        atual->ConfereDadosDinamicos("MarcaVotou");                                     // func 3770
        atual->SetSituacao(md::ESituacaoEleitor::Votou);                                // +140 = 3
        ++m_qtdVotaram;                                                                 // +104
    }
}
} // namespace comum

namespace vota {
// wasm func 10210 (slot 2; tools: vota::CSincronismoOperador::vf2)                     name inferred
// Operator side of "voto registrado": marks the current voter as VOTOU (except in voter-training mode,
// where the voter database is not used) and posts message 5 back to the voter thread's queue.
void CSincronismoOperador::StartState()
{
    if (!comum::EhTreinamentoEleitor())                                                 // func 697 inlined
        comum::CEleitores::GetInst().MarcaVotou();
    // message 5 to the VOTER thread: CThreadEleitor::GetInst() (func 316) +36 = its
    // CPriorityMessageQueue<SMessage>; push = rhvoice_f501 (misnamed), priority 1
    CThreadEleitor::GetInst().GetFila().Push(api::SMessage{5}, 1);
    m_proximoEstado = this;
}
} // namespace vota

// =====================================================================================================
// uenux2/src/app/vota/operador/outrasopcoes/cconfirmaencerramento.cpp  (path inferred; u10 owns the class)
// =====================================================================================================
namespace comum::md::estadoaplicacao {
// Inlined into func 10734.                              srclocs cestadogeralvota.cpp:108 / :113
void CEstadoGeralVota::MarcaFimAquisicao()
{
    if (m_dhFimAquisicao)                                                               // flag +48
        throw CUeComumDadosError(EUeComumDadosError{8084}, "O fim da aquisição já havia sido registrado");
    if (!m_dhIniAquisicao)                                                              // flag +32
        throw CUeComumDadosError(EUeComumDadosError{8085}, "O início da aquisição não havia sido registrado");
    m_dhFimAquisicao = api::CDateTime::Agora();                                         // api_f479, +36
}
} // namespace comum::md::estadoaplicacao

namespace vota {
// wasm func 10734 (slot 7; tools: vota::CConfirmaEncerramento::vf7)                    name inferred
// Micro-terminal question "confirm the end of voting?".
void CConfirmaEncerramento::ProcessInput()
{
    switch (m_tela->Read()) {             // CInteractiveForm<IScreenMT,IInputMT>::Read (cinteractiveform.h:57)
    case api::EInputResult::Corrige:                                                    // 5
        CLogVota::GetInst().LogaAviso("Procedimento de encerramento abortado");          // CLoga level 2
        m_proximoEstado = &CPedeIdentidade::GetInst();                                  // func 652 (misnamed)
        break;
    case api::EInputResult::Confirma: {                                                 // 9
        CLogVota::GetInst().Loga("Procedimento de encerramento confirmado");             // level 1
        auto& vota = CAppInfo::GetInst().GetVota(EUrnaTurno::Atual);
        vota.MarcaFimAquisicao();
        vota.SetEstadoVota(EstadoVota::FimAquisicaoVotos);                              // '9'
        comum::SalvaEstado();                                                           // func 491
        m_proximoEstado = &CFimAquisicaoVotos::GetInst();                               // func 5428
        break;
    }
    default:
        break;
    }
}

// wasm func 5428 (tools: vota_f5428): CFimAquisicaoVotos::GetInst() = merged lazy-singleton body
// vota_f764(mutex @1905004, &instance @1905028, vtable @1587220, flags 0).              name inferred
} // namespace vota

// =====================================================================================================
// uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp  (path inferred; other slots: u35 ?)
// =====================================================================================================
namespace vota {
// wasm func 10797 (slot 5 of comum::IControladorRegistraMesarios)                        name inferred
// Adjusts EstadoVota when the mesário-attendance registration is entered.
void CControladorRegistraMesariosVota::IniciaRegistro()
{
    auto& app = CAppInfo::GetInst();
    switch (app.GetVota(EUrnaTurno::Atual).GetEstadoVota()) {
    case EstadoVota::ZeresimaImpressa:                                           // '6'
        break;                                                                   // -> REGISTROMESARIOINICIAL
    case EstadoVota::Votar: {                                                    // '8'
        // back to the opening registration only while nothing happened yet (no vote, no justification)
        // and outside training. Both singletons are obtained BEFORE the tests (the binary calls
        // CRdvVota::GetInst and CJustificador::GetInst = func 1391, which may construct the justification
        // cache through CDAORepositorio::Entregar, even when the RDV already has attendance); the same
        // sequence is inlined in CReinicioVotacao::StartState (func 11835).
        const auto fase = app.GetGeral().GetFase();
        auto& rdv = comum::CRdvVota::GetInst();
        auto& justificador = comum::CJustificador::GetInst();                    // func 1391
        if (rdv.Comparecimento() != 0)                                           // shared_f1269
            return;
        if (justificador.Quantidade() != 0)                                      // +8 (map size)
            return;
        if (fase == comum::EFase::Treinamento)                                   // '3'
            return;
        break;                                                                   // -> REGISTROMESARIOINICIAL
    }
    case EstadoVota::FimAquisicaoVotos:                                          // '9'
        app.GetVota(EUrnaTurno::Atual).SetEstadoVota(EstadoVota::RegistroMesarioFinal);   // ':'
        comum::SalvaEstado();
        return;
    default:                                                                     // '7' and others
        return;
    }
    app.GetVota(EUrnaTurno::Atual).SetEstadoVota(EstadoVota::RegistroMesarioInicial);     // '7'
    comum::SalvaEstado();
}
} // namespace vota

// =====================================================================================================
// uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cretomada.cpp  (unit u07 owns the file)
// =====================================================================================================
namespace vota::testeteclado {

// wasm func 11831 (slot 10)                                             name as in unit u07 (CriaTela)
// "Quer testar o teclado?" with or without the "Não testar" option.
std::shared_ptr<api::CInteractiveForm<api::IScreen, api::IInputKbd>> CRetomada::CriaTela()
{
    auto& app = CAppInfo::GetInst();
    const auto& vota = app.GetVota(EUrnaTurno::Atual);
    if (vota.GetUrnaIdGerouZeresima() && *vota.GetUrnaIdGerouZeresima() == app.GetGeral().GetIdUrna())
        m_podePularTeste = true;                           // zerésima produced by this urna  (+12/+16 vs +60)
    else
        m_podePularTeste = vota.TecladoTestadoPosConversao();                            // +96
    return m_podePularTeste ? CTelasVota::GetInst().CriaTelaTesteTecladoOpcional()      // func 6581
                            : CTelasVota::GetInst().CriaTelaTesteTeclado();             // func 6580
}

// wasm func 11830 (slot 11)                                 name as in unit u07 (GetEstadoPassouNoTeste)
comum::CAppState* CRetomada::GetEstadoPassouNoTeste()
{
    auto& app = CAppInfo::GetInst();
    const auto& geral = app.GetGeral();
    const auto tipoUrna = geral.GetTurno() == EUrnaTurno::Primeiro ? geral.GetTipoUrnaT1()   // +36
                                                                   : geral.GetTipoUrnaT2();  // +40
    auto& vota = app.GetVota(EUrnaTurno::Atual);
    if (tipoUrna == comum::ETipoUrnaOperacao::ContingenciaVota && !vota.TecladoTestadoPosConversao()) {  // '3'
        vota.SetTecladoTestadoPosConversao(true);
        comum::SalvaEstado();
    }
    return &CReinicioVotacao::GetInst();                                                 // func 5938
}

} // namespace vota::testeteclado

// =====================================================================================================
// uenux2/src/app/vota/eleitor/iniciovotacao/  - zerésima states (units u07 / u09 own the classes)
// =====================================================================================================
namespace vota {

// wasm func 11924 (slot 2 of CInicioZeresima; path inferred cinciozeresima.cpp)           name inferred
void CInicioZeresima::StartState()
{
    CAppInfo::GetInst().GetVota(EUrnaTurno::Atual).SetEstadoVota(EstadoVota::AguardaHoraZeresima);  // '3'
    comum::SalvaEstado();
    if (comum::EhTreinamentoEleitor())
        m_proximoEstado = &CQuerImprimirZeresima::GetInst();         // func 5957: printing is optional
    else
        m_proximoEstado = &CConfirmaImpressaoZeresima::GetInst();    // func 5959
}

// wasm func 5959 (tools: vota_f5959) - lazy singleton @1834268 (mutex residue @1834244)   name inferred
CConfirmaImpressaoZeresima& CConfirmaImpressaoZeresima::GetInst()
{
    static std::unique_ptr<CConfirmaImpressaoZeresima> s_instancia;
    if (!s_instancia)
        s_instancia.reset(new CConfirmaImpressaoZeresima());
    return *s_instancia;
}
// 32 bytes: CAppState(2 = keys), m_tela = CTelasVota +52/+56, +20 CDateTime copied from
// CConfiguracaoEleicao +544..+555, then "+= 10800 s" (func 2233, CDateTime += seconds): a deadline
// 3 hours after that configured date-time.                                                // ?
CConfirmaImpressaoZeresima::CConfirmaImpressaoZeresima()
    : comum::CAppState(2)
    , m_tela(CTelasVota::GetInst().m_telaConfirmaImpressaoZeresima)
    , m_limite(comum::CConfiguracaoEleicao::GetInst().GetDataHoraReferencia())
{
    m_limite += std::chrono::seconds(10800);
}

// wasm func 11920 (slot 7 of CQuerImprimirZeresima)                                      name inferred
// "Quer imprimir a zerésima?" - only reached in voter-training mode. Its singleton getter (func 5957) has
// two callers: CInicioZeresima::StartState (11924, above) and CReinicioVotacao::StartState (11835, restart
// with no attendance and no justification, when trab/ze.dat does not exist), both behind
// EhTreinamentoEleitor() (func 697).
void CQuerImprimirZeresima::ProcessInput()
{
    switch (m_tela->Read()) {             // CInteractiveForm<IScreen,IInputKbd>::Read (cinteractiveform.h:57)
    case api::EInputResult::Corrige:                                                    // 5: do not print
        CAppInfo::GetInst().GetVota(EUrnaTurno::Atual).SetEstadoVota(EstadoVota::Votar);  // '8'
        comum::SalvaEstado();
        m_proximoEstado = &CInicioVotacao::GetInst();                                   // func 2880
        break;
    case api::EInputResult::Confirma:                                                   // 9: print
        m_proximoEstado = &CConfirmaImpressaoZeresima::GetInst();                       // func 5959
        break;
    default:
        break;
    }
}

// wasm func 11931 (slot 7 of CImpressaoZeresimaTardia)                                   name inferred
// "Late zerésima": the mesário is asked whether the urna clock is right before generating it.
void CImpressaoZeresimaTardia::ProcessInput()
{
    switch (m_tela->Read()) {
    case api::EInputResult::Corrige:                                                    // 5: clock wrong
        CLogVota::GetInst().Loga("Mesário confirma que o horário da urna está errado");
        m_proximoEstado = &CInformacaoZeresimaTardia::GetInst();    // lazy @1834156: CAppState(2),
        break;                                                      // CWait(1000) +16, tela CTelasVota +68
    case api::EInputResult::Confirma:                                                   // 9: clock right
        CAppInfo::GetInst().GetVota(EUrnaTurno::Atual).SetEstadoVota(EstadoVota::GerarZe);   // '4'
        comum::SalvaEstado();
        CLogVota::GetInst().Loga("Mesário confirma que o horário da urna está correto");
        m_proximoEstado = &CGeraZeresima::GetInst();                                    // func 5962
        break;
    default:
        break;
    }
}

// wasm func 11940 (slot 9 of CGeraResumoZeresima; pure in CGeraResumoZeresimaBase, no-op in
// CRegeraResumoZeresima)                                                                 name inferred
void CGeraResumoZeresima::PosGeracao()
{
    CAppInfo::GetInst().GetVota(EUrnaTurno::Atual).SetEstadoVota(EstadoVota::ZeresimaGerada);   // '5'
    comum::SalvaEstado();
}

// wasm func 11943 (slot 2 of CGeraResumoZeresimaBase / CGeraResumoZeresima / CRegeraResumoZeresima).
// Generates trab/rze.dat, the "Resumo da Zerésima". Twin of CGeraZeresimaBase::StartState (func 11946,
// reconstructed by unit u09 in cgeradorresumozeresima.cpp, which also holds the helpers used here:
// CriaTituloExtratoRDV = 5971, CRelVotaUtil::CriaCabecalho = 5977, CriaTrailer = 5579).   name inferred
void CGeraResumoZeresimaBase::StartState()
{
    m_proximoEstado = this;
    m_tela->Exibe();                                                                     // screen slot 2

    auto& app = CAppInfo::GetInst();
    const std::string separadorFase = comum::CRelUtil::GetSeparadorFase(app.GetGeral().GetFase());  // 1919
    const auto& correspondencia = app.GetGeral().GetCorrespondencia();                 // +60

    std::vector<SharedFormPart> extrato = CriaTituloExtratoRDV(separadorFase);         // func 5971
    auto cabecalho = std::make_shared<api::CFormPart>(CRelVotaUtil::CriaCabecalho("Resumo da Zerésima"));
    auto trailer = std::make_shared<api::CFormPart>(
        CriaTrailer(separadorFase, comum::CConfiguracaoEleicao::GetInst().GetTextoRodape() /* +200 */,
                    correspondencia));                                                  // func 5579
    std::vector<SharedFormPart> partes{cabecalho, trailer};
    partes.insert(partes.begin() + 1, extrato.begin(), extrato.end());                 // vota_f2875
    api::CReport relatorio(partes);                                                     // shared_f2259

    CLogVota::GetInst().LogaGeracaoRelatorio(comum::ERelatoriosUE::RESUMO_ZERESIMA /*11*/, false);   // 1047
    {
        const std::filesystem::path arquivo = comum::CPath::GetPathTrab(comum::EFlashOrigem::INTERNA) / "rze.dat";
        api::CScopedReportFile saida(arquivo, relatorio);                               // func 3654
    }
    CSincronizaVota::SincronizaRelatorios("rze.dat");                                   // func 1836

    auto& vota = app.GetVota(EUrnaTurno::Atual);
    vota.SetUrnaIdGerouZeresima(app.GetGeral().GetIdUrna());                           // +12 = geral +60
    comum::SalvaEstado();
    CLogVota::GetInst().LogaGeracaoRelatorio(comum::ERelatoriosUE::RESUMO_ZERESIMA, true);

    PosGeracao();                          // slot 9 (CGeraResumoZeresima: estado ZERESIMAGERADA)
    m_proximoEstado = GetProximoEstado();  // slot 10
}

} // namespace vota

// =====================================================================================================
// uenux2/src/app/vota/eleitor/fimvotacao/ciniciobu.cpp  (path inferred; BU flow of units u08/u09)
// =====================================================================================================
namespace vota {

// wasm func 12062 (slot 2 of CInicioBU)                                                  name inferred
// First state of the BU printing: in voter-training mode the mesário is asked "Quer imprimir o BU?"
// (CQuerImprimirBU, lazy singleton @1833904 inlined: CAppState(2), tela CTelasVota +236/+240);
// otherwise the BU is printed right away (CImprimindoBU).
void CInicioBU::StartState()
{
    if (comum::EhTreinamentoEleitor())                                                  // func 697 inlined
        m_proximoEstado = &CQuerImprimirBU::GetInst();
    else
        m_proximoEstado = &CImprimindoBU::GetInst();                                    // func 5986
}

} // namespace vota
