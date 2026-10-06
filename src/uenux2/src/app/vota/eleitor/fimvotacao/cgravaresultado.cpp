// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/fimvotacao/cgravaresultado.cpp
//
// srcloc evidence (func 12098, 26.7 KB: LTO inlined a large part of comum/gravadores into it):
//   cgravaresultado.cpp:63            Assert (vota.GetEstadoVota() == EAVGRAVARRESULTADOS)          (3456)
//   cgravaresultado.cpp:79            Assert (eleitores.GetNumComparecimentos() == rdv.Comparecimento()) (3457)
//   cestadogeralvota.h:90 / 106 / 159 GetDtHrInicioAquisicao / GetDtHrFimAquisicao / GetDtHrEmissaoBU
//                                     "Início da aquisição não marcado" (8090), "Fim da aquisição não
//                                     marcado" (8091), "A data/hora da emissão do BU não foi registrada" (8093)
//   cgravadorutil.cpp:113             CGravadorUtil::ConverteFaseEcourna(char)  "Fase inválida: {:#x}" (8698)
//   cdependenciascontratos.cpp:27     CDependenciasContratos(path)  tag check (8662)
//   cversoescontratos.cpp:29          CVersoesContratos(path)       tag check (8696)
//   cgeracaoversoescontratos.cpp:38   CGeracaoVersoesContratos::GetInst()  + cpolysingleton(list).h
//   cassinador.cpp:141/146/149        CAssinador::AssinaArquivosResultado(const std::vector<ESavdArquivoUE>&)
//   iinterfacesavd.cpp:1022           comum::AssinarEcourna(IInterfaceSavd&, ESavdAplic, ueint32, ueint32) (7338)
//   clogcomum.cpp:44/50/56/62/166/172/178  CLogComum::LogaIniciaSessaoMSD / FinalizaSessaoMSD / ...MSE,
//                                     LogaInicio/TerminoProcedimentoAssinatura, LogaPreparandoAssinatura...
//
// Nothing here ran in the recorded sessions (the web page only drives the voter terminal), except
// func 1249 (CDadoCorrespondencia copy constructor, observed during votaInit through other callers).

#include "vota/eleitor/fimvotacao/cgravaresultado.h"

#include <filesystem>
#include <format>
#include <map>
#include <memory>
#include <set>
#include <source_location>
#include <string>
#include <vector>

#include "api/gui/capplicationcontextguard.h"
#include "api/hwil/iurna.h"
#include "api/pattern/cpolysingletonlist.h"
#include "api/util/cdatetime.h"
#include "api/util/csynchronizer.h"
#include "api/util/csystem.h"
#include "comum/cappinfo.h"
#include "comum/carquivosresultado.h"            // EExtensaoArquivoResultado
#include "comum/cpath.h"
#include "comum/dados/ccargos.h"
#include "comum/dados/cconfiguracaoeleicao.h"
#include "comum/dados/celeitores.h"
#include "comum/dados/clocal.h"
#include "comum/dados/crdvvota.h"
#include "comum/gravadores/cgeracaoversoescontratos.h"
#include "comum/gravadores/cgravadorbu.h"
#include "comum/gravadores/cgravadorenvelopearquivo.h"
#include "comum/gravadores/cgravadorhashes.h"
#include "comum/gravadores/cgravadorlog.h"
#include "comum/gravadores/cgravadorrcsecao.h"
#include "comum/gravadores/cgravadorrdv.h"
#include "comum/gravadores/cgravadorutil.h"
#include "comum/gravadores/cgravadorversoesarquivos.h"
#include "comum/gravadores/cgravadorwsq.h"
#include "comum/gravadores/md/cversoesarquivos.h"
#include "comum/iinterfacesavd.h"
#include "comum/log/clogcomum.h"
#include "comum/md/cidentificacaosecao.h"
#include "vota/comum/cassinadorvota.h"
#include "vota/eleitor/comum/cinformacaoeleitor.h"
#include "vota/eleitor/fimvotacao/ccopiaresultadoparamr.h"
#include "vota/eleitor/fimvotacao/cprogressoencerramento.h"

namespace vota {

using comum::EExtensaoArquivoResultado;          // index of CArquivosResultado::operator[] (func 347)
using comum::md::estadoaplicacao::EEstadoVota;   // ASN.1 EstadoGeralVota.estadoVota + '1'

// Result files written here (EExtensaoArquivoResultado -> suffix, ESavdArquivoUE = SAVD file id):
//    4 bu.dat      35   ASN.1 EntidadeBoletimUrna (CGravadorBU)
//    6 rdv.dat     37   Registro Digital do Voto (CGravadorRDV)
//    8 jufa.dat    39   ModuloResultadoUrnaCadastro: justificativas/faltosos (CGravadorRCSecao)
//    9 imgbu.dat   43   EntidadeEnvelopeGenerico around trab/bu.dat, the printed BU image
//   11 imgze.dat   44   EntidadeEnvelopeGenerico around trab/ze.dat, the printed zerésima image
//   12 hash.dat    46   ModuloHashes::EntidadeHashes (CGravadorHashes)
//   16/17/18 wsqbio/wsqman/wsqmes.jez  48/49/50  fingerprints (biometric urnas only)
//   19 mr.ver      70   ModuloVersaoArquivos (versions of the ASN.1 "contratos")
//   13 log.jez     60   packed application log (CGravadorLog)
// The file names are "<fase><pleito:05><UF><município:05><zona:04><seção:04>-<suffix>"
// (comum::CGravadorUtil::DeterminaNomeArquivoSemLetra, called by IResultado's constructor).

namespace {

constexpr const char* TAG_CONTRATOS = "20260601173148";   // @341442: expected "tag" of both .properties

bool TurnoUm()                                                     // inlined; name inferred
{
    return comum::CAppInfo::GetInst().GetEstadoGeral().GetTurno() == '1';   // CEstadoGeral +32
}

}  // namespace

// =================================================================================================
// wasm func 12098 — vtable slot 2 (analyzer name vota::CGravaResultado::vf2)
// =================================================================================================
void CGravaResultado::StartState()
{
    auto& appInfo   = comum::CAppInfo::GetInst();                          // func 185
    auto& vota      = appInfo.GetVota();                                   // GetEstado<CEstadoGeralVota>
    auto& eleitores = comum::CEleitores::GetInst();                        // func 326

    UE_ASSERT(vota.GetEstadoVota() == EEstadoVota::EAVGRAVARRESULTADOS);  // :63 (62)

    const char fase = appInfo.GetEstadoGeral().GetDadoCarga().GetFaseChar();   // 'o'/'s'/'t' (func 2253)
    const auto qtdAptos = eleitores.GetQtdAptos();                         // func 2824 (SQtdeAptos, map)
    auto& rdv   = comum::CRdvVota::GetInst();                              // func 555
    auto& local = comum::CLocal::GetInst();                                // func 401
    const std::string& uf       = local.GetUF();                           // func 1702
    const auto municipio        = local.GetMunicipio();                    // func 1004
    const auto zona             = local.GetZonaID();                       // func 1003
    const auto localId          = local.GetLocalID();                      // func 1933
    const auto secao            = local.GetSecaoID();                      // func 1078
    const api::CDateTime agora;                                            // func 479: "data/hora de geração"

    const api::CDateTime dhIniAquisicao = vota.GetDtHrInicioAquisicao();  // h:90  8090 if unset
    const api::CDateTime dhFimAquisicao = vota.GetDtHrFimAquisicao();     // h:106 8091 if unset
    UE_ASSERT(eleitores.GetNumComparecimentos() == rdv.Comparecimento()); // :79 (CEleitores +104 vs func 1269)

    CInformacaoEleitor::GetInst();                                         // func 509, result unused (?)
    comum::VerificaIntegridadeReferencial();                               // func 2543 (CRdvVota/CCargos/CRespostas)

    std::vector<std::string> modulosAsn;                                   // ASN.1 modules whose versions go to mr.ver
    std::vector<std::shared_ptr<comum::IGravador>> gravadores;

    // ---- 1. Boletim de Urna (bu.dat, ASN.1 EntidadeBoletimUrna) ----------------------------------
    modulosAsn.push_back("ModuloBoletimUrna");
    {
        const auto& estadoGeral = appInfo.GetEstadoGeral();
        const api::CDateTime& dhEmissao = vota.GetDtHrEmissaoBU();         // h:159 8093 if unset (set by CGeraBU)
        const std::vector<std::string> historicoCargas = appInfo.GetGap().GetCodigosCarga();  // func 3788
        const bool urnaBiometrica = local.UrnaBiometrica();                // func 820
        const uint16_t comparecimento = eleitores.GetNumComparecimentos(); // CEleitores +104
        // habilitações: each count is range-checked by CBaseType<uint16_t, 0, 9999> (func 1960)
        const comum::SQtdHabilitacoes habilitacoes{                        // func 5094 (3 x u16) name inferred
            CBaseType<uint16_t, 0, 9999>(eleitores.QtdSemBiometria()),        // func 2822
            CBaseType<uint16_t, 0, 9999>(eleitores.QtdHabilitados(1)),        // func 2821 biométrica
            CBaseType<uint16_t, 0, 9999>(eleitores.QtdHabilitados(2))};       // func 1935 biográfica

        // Order of the cargos in the BU: first position of each cargo id in CCargos (1-based).
        std::map<comum::TCargoID, uebyte> ordemCargos;                     // CGravadorBU +284
        auto& cargos = comum::CCargos::GetInst();
        cargos.Inicio();                                                   // funcs 3784, 3782, 2269
        uebyte n = 0;
        while (!cargos.Fim()) {                                            // shared_f602
            ++n;
            ordemCargos.emplace(cargos.GetCurrent().GetID(), n);           // only the first occurrence
            cargos.Next();                                                 // func 1708
        }

        // new CGravadorBU (304 bytes, constructor inlined; class in comum/gravadores/cgravadorbu.cpp):
        //   IResultado(municipio, zona, localId, secao, fase, bu.dat (4), SAVD 35)
        //   +40 dhGeracao = agora, +52 dhEmissao, +64 CGravadorUtil::ConverteFase(fase) (func 2274),
        //   +68 '1' (49), +72 CDadoCorrespondencia (copy of estadoGeral +60, func 1249),
        //   +168 historicoCargas, +180 qtdAptos, +196 &rdv, +194 comparecimento, +192 urnaBiometrica,
        //   +200 arquivo de chave "bu.pk1", +212 true, +216.. habilitações, dhIni/dhFimAquisicao,
        //   +284 ordemCargos
        gravadores.push_back(std::make_shared<comum::CGravadorBU>(
            municipio, zona, localId, secao, fase, agora, dhEmissao, estadoGeral.GetCorrespondencia(),
            historicoCargas, qtdAptos, "bu.pk1", rdv, comparecimento, urnaBiometrica, habilitacoes,
            dhIniAquisicao, dhFimAquisicao, std::move(ordemCargos)));
    }

    // ---- 2. RDV (rdv.dat) -----------------------------------------------------------------------
    modulosAsn.push_back("ModuloEnvelopeGenerico");
    // new CGravadorRDV (168 bytes): IResultado(..., rdv.dat (6), SAVD 37), +40 agora, +52 '1',
    //   +56 correspondência, +164 &rdv
    gravadores.push_back(std::make_shared<comum::CGravadorRDV>(
        municipio, zona, localId, secao, fase, agora, appInfo.GetEstadoGeral().GetCorrespondencia(), rdv));

    // ---- 3. Registro de comparecimento da seção (jufa.dat) --------------------------------------
    modulosAsn.push_back("ModuloResultadoUrnaCadastro");
    // new CGravadorRCSecao (60 bytes): IResultado(..., jufa.dat (8), SAVD 39), +40 agora,
    //   +52 CGravadorUtil::ConverteFaseEcourna(fase) (inlined, cgravadorutil.cpp:113):
    //        'o' -> 2 (oficial), 's' -> 1 (simulado), 't' -> 3 (treinamento),
    //        otherwise throw CBaseError<EUeComumGravadoresError>(8698, "Fase inválida: {:#x}")
    gravadores.push_back(std::make_shared<comum::CGravadorRCSecao>(
        municipio, zona, localId, secao, fase, agora));

    // ---- 4. Envelopes of the printed images (func 5856 = CGravadorEnvelopeArquivo ctor) ---------
    const auto trab = comum::CPath::GetPathTrab(comum::EFlashOrigem::INTERNA);        // func 436
    gravadores.push_back(std::make_shared<comum::CGravadorEnvelopeArquivo>(
        agora, municipio, zona, localId, secao, fase, EExtensaoArquivoResultado(9) /*imgbu.dat*/, 43,
        comum::md::CEnvelopeGenerico::Tipo::BoletimUrnaImpresso /*2 -> ASN.1 4*/,
        appInfo.GetEstadoGeral().GetCorrespondencia(), trab / "bu.dat"));
    gravadores.push_back(std::make_shared<comum::CGravadorEnvelopeArquivo>(
        agora, municipio, zona, localId, secao, fase, EExtensaoArquivoResultado(11) /*imgze.dat*/, 44,
        comum::md::CEnvelopeGenerico::Tipo::ZeresimaImpressa /*4 -> ASN.1 6*/,
        appInfo.GetEstadoGeral().GetCorrespondencia(), trab / "ze.dat"));

    // ---- 5. Hashes of the installation (hash.dat, ModuloHashes::EntidadeHashes) -----------------
    modulosAsn.push_back("ModuloHashes");
    const std::string versao = "10.23.0.1 - DESENVOLVIMENTO";                          // @326597
    if (secao == 0) {
        // urna de contingência: identified by município/zona only (EUrnaTipo '2', func 5888)
        const comum::md::CIdentificacaoUrnaContingencia id(municipio, zona);
        // new CGravadorHashes (96 bytes): IResultado(mun, zona, 0, 0, fase, hash.dat (12), SAVD 46),
        //   +40 agora, +52 ConverteFase(fase), +56 uf, +76 shared_ptr<identificação>, +84 versão
        gravadores.push_back(std::make_shared<comum::CGravadorHashes>(
            fase, agora, uf, std::make_shared<comum::md::CIdentificacaoUrnaContingencia>(id), versao));
    } else {
        // NOTE: the "local" of the identification is the constant 1, not localId (see the u09 doc,
        // suspicious item "EntidadeHashes local = 1").
        const comum::md::CIdentificacaoSecao id(municipio, zona, /*local*/ 1, secao);   // func 5887 ('1')
        gravadores.push_back(std::make_shared<comum::CGravadorHashes>(
            fase, agora, uf, std::make_shared<comum::md::CIdentificacaoSecao>(id), versao));
    }

    // ---- 6. Biometrics (WSQ packages), biometric urnas only ------------------------------------
    if (local.UrnaBiometrica()) {
        const bool cifra = !comum::EhFaseTreinamento();                   // func 1485: fase != '3'
        // CGravadorWSQ ctor (88 bytes; analyzer name "ValidaTipoBiometria", which it inlines)
        gravadores.push_back(std::make_shared<comum::CGravadorWSQ>(agora, municipio, zona, localId, secao, fase,
            EExtensaoArquivoResultado(16) /*wsqbio.jez*/, 48, /*tipo*/ 0, cifra));
        gravadores.push_back(std::make_shared<comum::CGravadorWSQ>(agora, municipio, zona, localId, secao, fase,
            EExtensaoArquivoResultado(17) /*wsqman.jez*/, 49, /*tipo*/ 1, cifra));
        gravadores.push_back(std::make_shared<comum::CGravadorWSQ>(agora, municipio, zona, localId, secao, fase,
            EExtensaoArquivoResultado(18) /*wsqmes.jez*/, 50, /*tipo*/ 2, cifra));
    }

    modulosAsn.push_back("ModuloAssinaturaEcourna");
    modulosAsn.push_back("ModuloVersaoArquivos");

    // ---- 7. Versions of the ASN.1 contracts (mr.ver) --------------------------------------------
    if (!api::CPolySingletonList::Existe<comum::CGeracaoVersoesContratos>()) {        // func 2860
        // CGeracaoVersoesContratos (48 bytes) = { CDependenciasContratos (+0), CVersoesContratos (+24) },
        // both "key=value" files read with the api::CIniStrings loader (func 5480, analyzer name
        // "CFile::ReadLine", see unit u12):
        //   CDependenciasContratos(root / "etc/dependencias.properties")   (cdependenciascontratos.cpp:27)
        //   CVersoesContratos     (root / "etc/versoes.properties")        (cversoescontratos.cpp:29)
        // Each checks its "tag" entry == "20260601173148", otherwise it throws
        // CBaseError<EUeComumGravadoresError> 8662 "Tag do arquivo de dependências de contratos
        // inválida: [{}]. Tag atual: [{}]" / 8696 "Tag do arquivo de versões de contratos inválida: ...".
        // (root = the global CPath root string @1838600 = "/", func 3834.)  In the simulator both
        // files are 0-byte stubs: the parse succeeds with no key, and the tag lookup itself,
        // CDependenciasContratos::GetValor("tag") (func 3826 -> 6044, cdependenciascontratos.cpp:59),
        // throws 8663 "Propriedade inexistente: tag" before the 8662 comparison is reached.
        api::CPolySingletonList::push(std::make_unique<comum::CGeracaoVersoesContratos>(
            comum::CPath::GetRoot() / "etc/dependencias.properties",
            comum::CPath::GetRoot() / "etc/versoes.properties"));                     // cpolysingletonlist.h:129
    }
    const auto& geracao = comum::CGeracaoVersoesContratos::GetInst();                // cgeracaoversoescontratos.cpp:38

    std::set<std::string> modulos;                     // the modules and all their (transitive) dependencies
    for (const auto& m : modulosAsn)
        ColetaDependencias(geracao.GetDependencias(), m, modulos);                     // func 5869 (copy discarded)
    modulos.insert(modulosAsn.begin(), modulosAsn.end());                              // unknown_f2281
    std::map<std::string, std::string> versoes;
    for (const auto& m : modulos)
        versoes.emplace(m, geracao.GetVersoes().GetValor(m));                          // func 3825
    const comum::md::CVersoesArquivos versoesArquivos =
        comum::md::CVersoesArquivos::ValidaCriacao(TAG_CONTRATOS, versoes);            // func 5867
    // new CGravadorVersoesArquivos (64 bytes): IResultado(..., mr.ver (19), SAVD 70), +40 tag,
    //   +52 map<string,string> versões
    gravadores.push_back(std::make_shared<comum::CGravadorVersoesArquivos>(
        municipio, zona, localId, secao, fase, versoesArquivos));

    // ---- 8. Application log (log.jez) ----------------------------------------------------------
    const std::string dirLog = comum::CPath::GetPathLog();                             // "<root>dinamico/log/" (func 5899)
    // new CGravadorLog (76 bytes): IResultado(..., log.jez (13), SAVD 60),
    //   +40 diretório de arquivados, +52 arquivo de log, +64 diretório de trabalho
    gravadores.push_back(std::make_shared<comum::CGravadorLog>(
        municipio, zona, localId, secao, fase,
        std::filesystem::path(dirLog) / "arquivados/",
        std::filesystem::path(dirLog) / "logd.dat",
        comum::CPath::GetPathTrab(comum::EFlashOrigem::INTERNA)));

    // ---- 9. Signer, progress screen, and the actual work ----------------------------------------
    // vota::CAssinadorVota(pacote): CAssinador constructor inlined:
    //   +4 aplicação = 1 (VOTA?), +8 pacote = 158 (1º turno) / 159 (2º turno),
    //   +36 local = std::format("{}{:05}{:04}{:04}", uf, município, zona, seção)   (15 characters),
    //   nome = std::vformat(CArquivosSavd::GetInst()[pacote],                       (func 1164 / 275)
    //                       fase, cfg.GetPleito() /*+28*/, uf, município, zona, seção)
    //        = GetPathResult(MI, turno) / "{:c}{:05}{}{:05}{:04}{:04}-" + "vota.vsc"
    //   +24 diretório = Diretorio(nome) + "/"   (func 5462),  +12 arquivo = nome do arquivo (api_f2763)
    const CAssinadorVota assinador(TurnoUm() ? 158 : 159);

    // std::make_shared<CProgressoEncerramento>() (constructor inlined, class of unit u07/u39):
    //   CFormBuilder: StatusHeader(5), "Preparando dados para encerramento" (320,90 font 474888),
    //   "O processo pode levar alguns minutos" (320,220), "Por favor, aguarde..." (320,260),
    //   CProgressBar(30 passos, {70,350}-{570,385}, "%p") (api 5545), name "telaProgressoEncerramento"
    auto progresso = std::make_shared<CProgressoEncerramento>();

    comum::CGravacaoResultados gravacao(gravadores, {}, assinador,
                                        !comum::EhFaseTreinamento(), progresso);   // func 5824
    gravacao.Executa();

    vota.SetEstadoVota(EEstadoVota::EAVCOPIARESULTADOSMR);                           // 63
    comum::SalvaEstado();                                                              // func 491
    m_proximoEstado = &CCopiaResultadoParaMR::GetInst();                               // func 6174
}

}  // namespace vota

// =================================================================================================
// comum::CGravacaoResultados (name inferred). Constructor = wasm func 5824; Executa() and
// CAssinador::AssinaArquivosResultado() are inlined into func 12098.
// =================================================================================================
namespace comum {

// wasm func 5824
CGravacaoResultados::CGravacaoResultados(const std::vector<std::shared_ptr<IGravador>>& gravadores,
                                         const std::vector<std::shared_ptr<IGravador>>& complementares,
                                         const CAssinador& assinador, bool copiaParaMV,
                                         std::shared_ptr<CAbstractTelaProgresso> progresso)
    : m_gravadores(gravadores),
      m_gravadoresComplementares(complementares),
      m_assinador(assinador),          // copied as a plain comum::CAssinador (vptr @1553260)
      m_copiaParaMV(copiaParaMV),
      m_progresso(std::move(progresso))
{
}

// Inlined into wasm func 12098. IGravador slots used (unit u23):
//   [4] Grava(): opens GetPathTrab(MI)/<nome> "wb" and calls [7] GravaResultado(CFile&)   (func 11634)
//   [2] CopiaParaResultado(): trab MI -> result dir MI                                     (func 11632)
//   [3] CopiaParaMV(): -> MV                                                               (func 11630)
//   GetExtensao() = IResultado +32,  GetArquivoSavd() = IResultado +36
void CGravacaoResultados::Executa()
{
    m_progresso->Inicia();                                             // slot 2
    std::vector<ESavdArquivoUE> arquivosSavd;

    {
        api::CApplicationContextGuard contexto(2, "", "Gerando os resultados na MI",
            "Ocorreu um erro gerando os resultados no diretório de trabalho na MI.");
        for (const auto& g : m_gravadores) {
            CLogComum::GetInst().LogaGerandoResultados(g->GetExtensao(), false);   // func 5876 [INÍCIO]
            g->Grava();
            CLogComum::GetInst().LogaGerandoResultados(g->GetExtensao(), true);    // [TÉRMINO]
            m_progresso->Avanca();                                                 // slot 4
        }
    }                                                                  // ~CApplicationContextGuard (func 675)
    {
        api::CApplicationContextGuard contexto(2, "", "Copiando arquivos para o dir. de resultados",
            "Ocorreu um erro copiando os arquivos de resultado para o diretório de definitivo da MI.");
        for (const auto& g : m_gravadores) {
            g->CopiaParaResultado();
            arquivosSavd.push_back(g->GetArquivoSavd());
            CLogComum::GetInst().LogaCopiandoArqResParaFI(g->GetExtensao());       // func 5878
            m_progresso->Avanca();
        }
    }
    {
        api::CApplicationContextGuard contexto(2, "", "Copiando resultados da MI",
            "Ocorreu um erro copiando os resultados para o diretório de resultados da MI.");
        for (const auto& g : m_gravadoresComplementares) {
            g->CopiaParaResultado();
            arquivosSavd.push_back(g->GetArquivoSavd());
            CLogComum::GetInst().LogaCopiandoArqResParaFI(g->GetExtensao());
            m_progresso->Avanca();
        }
    }

    CLogComum::GetInst().LogaPreparandoAssinaturaArquivosResultado();              // clogcomum.cpp:178
    {
        api::CApplicationContextGuard contexto(2, "", "Assinando resultados da MI",
            "Ocorreu um erro assinando os resultados da MI.");
        CLogComum::GetInst().LogaInicioProcedimentoAssinatura();                    // clogcomum.cpp:166
        m_assinador.AssinaArquivosResultado(arquivosSavd);                          // cassinador.cpp:141..149
        CLogComum::GetInst().LogaTerminoProcedimentoAssinatura();                   // clogcomum.cpp:172
        m_progresso->Avanca();
    }
    api::CSynchronizer::CreateInst()->Sincroniza();                                 // shared_f620

    if (m_copiaParaMV) {
        {
            api::CApplicationContextGuard contexto(4, "", "Copiando resultados para a MV",
                "Ocorreu um erro copiando arquivos gerados para o diretório de trabalho da MV");
            for (const auto& g : m_gravadores) {
                g->CopiaParaMV();
                CLogComum::GetInst().LogaResultadoCopiadoResFE(g->GetExtensao());     // func 3828
                m_progresso->Avanca();
            }
        }
        {
            api::CApplicationContextGuard contexto(4, "", "Copiando resultados para a MV",
                "Ocorreu um erro copiando arquivos de resultado para o diretório de resultados da MV");
            for (const auto& g : m_gravadoresComplementares) {
                g->CopiaParaMV();
                CLogComum::GetInst().LogaResultadoCopiadoResFE(g->GetExtensao());
                m_progresso->Avanca();
            }
        }
        {
            api::CApplicationContextGuard contexto(4, "", "Assinando resultados para a MV",
                "Ocorreu um erro copiando arquivos de resultados e assinatura  para a MV");
            // The signature package (.vsc) is not re-created on the MV: it is copied.
            const std::string destino = CPath::GetPathResult(EFlashOrigem::EXTERNA)   // func 1274(1)
                                        + m_assinador.GetArquivo();
            api::CSystem::CopyFile(m_assinador.GetDiretorio() + m_assinador.GetArquivo(), destino, false);
            // SAVD packages 158..169 map to vota.vsc (1), sa.vsc (2), red.vsc (3) (table @495280)
            if (static_cast<unsigned>(m_assinador.GetPacote() - 158) <= 11)
                CLogComum::GetInst().LogaResultadoCopiadoResFE(
                    ExtensaoDoPacoteAssinatura(m_assinador.GetPacote()));
            m_progresso->Avanca();
        }
        api::CSynchronizer::CreateInst()->Sincroniza();
    }
    m_progresso->Finaliza();                                                       // slot 3
}

// comum::CAssinador::AssinaArquivosResultado (cassinador.cpp:141-149), inlined into func 12098.
// CAssinador layout: +4 ESavdAplic m_aplicacao, +8 ESavdPacote m_pacote, +12 std::string m_arquivo,
//                    +24 std::string m_diretorio, +36 std::string m_local (15 chars)
void CAssinador::AssinaArquivosResultado(const std::vector<ESavdArquivoUE>& arquivos) const
{
    if (arquivos.empty())
        throw CUeComumGravadoresError(8636, "Não foi passado nenhum arquivo de resultado",
                                      std::source_location::current());               // :141

    auto& savd = IInterfaceSavd::GetInst();                                          // :146 (func 1822)
    EnviarAcaoHSM(savd, m_aplicacao, ESavdCmdHSM::ABRE_SESSAO /*0*/);                // func 5889
    auto& urna = api::IUrna::GetInst();                                              // :149 (func 923)
    if (urna.GetModelo() >= 2020)                                                    // IUrna slot 0
        CLogComum::GetInst().LogaIniciaSessaoMSE();                                  // "Inicia uma sessão no MSE"
    else
        CLogComum::GetInst().LogaIniciaSessaoMSD();                                  // "Inicia uma sessão no MSD"

    // Select the "local" (urna identification) in the SAVD. Inlined IInterfaceSavd request:
    //   aplicação >= 256        -> last error = "Argumento inválido para local."
    //   m_local.size() != 15    -> last error = "Argumento inválido, tamanho do local."
    //   else message {0xFE, m_local[0..14]} + u16 0x0404 + u8 aplicação -> comum_f3829
    // The result is NOT checked.
    savd.DefineLocal(m_aplicacao, m_local);                                          // name inferred

    for (const ESavdArquivoUE arquivo : arquivos) {
        api::CApplicationContextGuard contexto(11, "", "Erro na assinatura dos arquivos de resultado",
            "Ocorreu um erro durante a assinatura dos arquivos: " + NomeArquivoSavd(arquivo));  // func 5905
        AssinarEcourna(savd, m_aplicacao, 128, arquivo);                              // iinterfacesavd.cpp:1022
    }

    EnviarAcaoHSM(savd, m_aplicacao, ESavdCmdHSM::FECHA_SESSAO /*1*/);
    if (urna.GetModelo() >= 2020)
        CLogComum::GetInst().LogaFinalizaSessaoMSE();                                // :62
    else
        CLogComum::GetInst().LogaFinalizaSessaoMSD();                                // :50
}

// iinterfacesavd.cpp:1022, inlined.
void AssinarEcourna(IInterfaceSavd& savd, ESavdAplic aplicacao, ueint32 opcoes, ueint32 arquivo)
{
    // func 5891 (name inferred): validates (opcoes <= 65535, aplicação <= 255, arquivo < 256, else
    // "Argumento inválido validar assinar."), sends {0xFE, '=', arquivo} + u16 opcoes + u8 aplicação,
    // returns true when the service answered without error text.
    if (!savd.AssinaArquivo(aplicacao, opcoes, arquivo))
        throw CUeComumError(7338, std::format("Falha ao assinar ({}-Ecourna[{}])", savd.GetUltimoErro(), arquivo),
                            std::source_location::current());
}

}  // namespace comum

// =================================================================================================
// Other functions the tools attributed to cgravaresultado.cpp (callers only here).
// =================================================================================================
namespace comum {

// wasm func 5856 — comum::CGravadorEnvelopeArquivo::CGravadorEnvelopeArquivo (class of
// comum/gravadores, unit u23). Base IGravadorEnvelope ctor inlined.
//   IResultado(municipio, zona, local, secao, fase, extensao, savd);   vptr IGravadorEnvelope
//   +40 dhGeracao, +52 ConverteFase(fase) (func 2274), +56 '1', +60/+68 flags = false,
//   +72 md::CEnvelopeGenerico::Tipo (0-based: 0 BU, 1 RDV, 2 BU impresso, 3 imagem biometria,
//   4 zerésima impressa; CEnvelopeGenerico::ValidaCriacao rejects >= 5, the ASN.1 converter maps them
//   to TipoEnvelope 1,2,4,5,6), +76 CDadoCorrespondencia (func 1249);  vptr CGravadorEnvelopeArquivo;
//   +172 std::string arquivo (the file wrapped into the envelope, e.g. trab/bu.dat)
CGravadorEnvelopeArquivo::CGravadorEnvelopeArquivo(const api::CDateTime& dhGeracao, TMunicipioID municipio,
        TZonaID zona, TLocalID local, TSecaoID secao, char fase, EExtensaoArquivoResultado extensao,
        ESavdArquivoUE savd, md::CEnvelopeGenerico::Tipo tipo, const md::estadoaplicacao::CDadoCorrespondencia& corresp,
        const std::string& arquivo)
    : IGravadorEnvelope(municipio, zona, local, secao, fase, extensao, savd, dhGeracao,
                        CGravadorUtil::ConverteFase(fase), tipo, corresp),
      m_arquivo(arquivo)
{
}

// wasm func 5869 — collects, recursively, the dependencies of an ASN.1 module listed in
// etc/dependencias.properties ("Modulo=Dep1,Dep2,..."). Returns a copy of the accumulated set
// (the caller discards it). Probably a static helper of cgravaresultado.cpp.   name inferred
std::set<std::string> ColetaDependencias(const md::CDependenciasContratos& dependencias,
                                         const std::string& modulo, std::set<std::string>& coletados)
{
    const std::vector<std::string> lista = util::Split(dependencias.GetValor(modulo), ',');  // funcs 3826, 1880
    for (const auto& dep : lista) {
        if (dep.empty())
            continue;
        if (coletados.insert(dep).second) {          // func 5868 (called twice with the same key)
            coletados.insert(dep);
            ColetaDependencias(dependencias, dep, coletados);
        }
    }
    return std::set<std::string>(coletados.begin(), coletados.end());
}

// wasm func 5868 — std::set<std::string>::insert(const std::string&) -> pair<iterator,bool>
//   (libc++ __tree::__emplace_unique_key_args; string compare via memcmp). library instantiation.

// wasm func 5870 — builds the std::format arguments {tagLida, "20260601173148"} of the 8662/8696
//   messages ("... Tag atual: [{}]"): make_format_args instance specialised by LTO.

// wasm func 5462 — directory part of a path string: everything before the last '/', or the whole
//   string when there is no '/'. Also called by comum_f1501 (the out-of-line CAssinador ctor).
//   Probably CAssinador's (anonymous) helper in comum/gravadores/cassinador.cpp. name inferred
std::string Diretorio(const std::string& caminho)
{
    const auto pos = caminho.rfind('/');
    return pos == std::string::npos ? caminho : caminho.substr(0, pos);
}

// wasm func 5866 — ~CGeracaoVersoesContratos() body (also the __on_zero_shared of its shared_ptr,
//   func 11636): CDependenciasContratos (+0) and CVersoesContratos (+24) are each an api::CIniStrings
//   (unit u12): +0 std::map<std::string, api::CIniSection> (destroyed by func 2859) and
//   +12 std::map<std::string, api::CIniKey> (func 1397); 5866 destroys +36, +24, +12, +0 in that order.
// wasm func 2859 — recursive std::map<std::string, api::CIniSection>::__tree::destroy used by 5866
//   (node: key string +16; CIniSection value = name string +28 and its own map<string, CIniKey> +40,
//   destroyed by func 1397). library instantiation.
// wasm func 1249 — implicit copy constructor of comum::md::estadoaplicacao::CDadoCorrespondencia
//   (96 bytes: int, string, 8 bytes, string (codigoCarga, +28), 8 bytes, +48 member (func 10145),
//   +60 vector (func 9872)). Observed executing (votaInit: CConversorEstadoGeral / GAP).

}  // namespace comum
