// uenux2/src/app/vota/eleitor/comum/cinformacaoeleitor.cpp
// (original build path: /home/rubio/tse/uenux2/src/app/vota/eleitor/comum/cinformacaoeleitor.cpp)
//
// Reconstructed from vota_web_wasm.wasm by unit u02.
//
// This file holds the reconstruction of wasm func 7787 (148 912 bytes, 65 253 instructions, the
// largest application function of the module). The analysis tools first named it
// "ecourna::api::security::CHKDFSeed::GetSeed" (the name it was formerly shown under) because the first
// std::source_location record found in its body is chkdfseed.cpp:47; the database now calls it
// vota::CInformacaoEleitor::Inicializar. It is in fact the start-up routine of the voter side of VOTA:
// every call below that is marked "inlined" has no wasm function of its own and was expanded
// in place by LTO. Its real name is not in the binary; `CInformacaoEleitor::Inicializar` is an
// inference (it manipulates the CInformacaoEleitor flags, contains the only source_location of
// this file, and is called once by votaInit right before CInformacaoEleitor::GerarDadosDinamicos).
//
// dcmp = the decompiled pseudo-code of func 7787. The "dcmp N" line numbers in this file (and in crdv.cpp,
// crdvvota.u02.cpp, cconfiguracaoeleicao.u02.cpp, cestadosvota.u02.cpp, chkdfseed.cpp) were taken from an
// earlier layout, decompiled/app-api/ecourna-lib/ecourna/api/security/chkdfseed.cpp.dcmp, where func 7787
// spanned lines 6645-25944. That file no longer exists: the function is now in
// decompiled/app-vota/uenux2/src/app/vota/eleitor/cestadosvota.cpp.dcmp (lines 18-19761; the database files
// it under cestadosvota.cpp), and the old numbers do not map onto the new file. To find a spot, search
// the new file for the srcloc or string quoted next to the number (e.g. "srcloc crdvvota.cpp:92").

#include "vota/eleitor/comum/cinformacaoeleitor.h"

#include <filesystem>
#include <string>
#include <vector>

#include "api/gui/cformbuilder.h"
#include "api/hwil/ifingerprepare.h"
#include "api/pattern/cpolysingleton.h"
#include "api/persistencia/cdaorepositorio.hpp"
#include "comum/appinfo/cappinfo.h"
#include "comum/cinfomtlcd.h"
#include "comum/cpacotearquivos.h"
#include "comum/cpath.h"
#include "comum/dados/ccandidaturas.h"
#include "comum/dados/ccargos.h"
#include "comum/dados/cconfiguracaoeleicao.h"
#include "comum/dados/celeitores.h"
#include "comum/dados/cfederacoes.h"
#include "comum/dados/cfotos.h"
#include "comum/dados/chv.h"
#include "comum/dados/cintegridadereferencial.h"
#include "comum/dados/clocal.h"
#include "comum/dados/cpartidos.h"
#include "comum/dados/crdvvota.h"
#include "comum/comparecimentomesario/ccomparecimentomesariodao.h"
#include "comum/justificativa/cjustificadordao.h"
#include "vota/comum/csincronizavota.h"
#include "vota/eleitor/cinstrucaovotacaoacessibilidade.h"
#include "vota/eleitor/comum/ctelasvota.h"

namespace vota {

// C++ enum of CEstadoGeralVota: '1' + value of the ASN.1 ENUMERATED EstadoVota (see
// CConversorEstadoGeralVota::ConverteEstadoVota/DesconverteEstadoVota, wasm 11390/11391):
// INICIAL='1', GERABASEDINAMICA='2', AGUARDAHORAZERESIMA='3', GERARZE='4', ZERESIMAGERADA='5',
// ZERESIMAIMPRESSA='6', REGISTROMESARIOINICIAL='7', VOTAR='8', FIMAQUISICAOVOTOS='9',
// REGISTROMESARIOFINAL=':', GERARBU=';', GERARRELATORIOS='<', IMPRIMIRBU='=', GRAVARRESULTADOS='>',
// COPIARESULTADOSMR='?', ENCERRADA='@'.
using comum::md::estadoaplicacao::EEstadoVota;

namespace {
// wasm funcs 1551 / 1701 / 2826 / 5762 (not inlined; shared with CEleitores, CSincronismoVotoEleitor
// and wasm func 6737). Each is "<trab dir of the flash>/<file>"; see comum/cpath.u02.cpp.
std::filesystem::path ArquivoRdvInterno();     // wasm func 1551  -> trab (flash interna)/rdv.dat
std::filesystem::path ArquivoRdvExterno();     // wasm func 1701  -> trab (flash externa)/rdv.dat
std::filesystem::path ArquivoBancoInterno();   // wasm func 2826  -> trab (flash interna)/uenux.db
std::filesystem::path ArquivoBancoExterno();   // wasm func 5762  -> trab (flash externa)/uenux.db
} // namespace

// ---------------------------------------------------------------------------------------------
// wasm func 7787                                                          // name inferred
// Called only by votaInit (wasm func 7840) through invoke_v(39): any exception propagates to the
// "catch (const std::exception&)" of votaInit, which turns it into a failed initialisation.
void CInformacaoEleitor::Inicializar()
{
    auto& appInfo = comum::CAppInfo::GetInst();                                    // wasm func 185

    // dcmp 6674: global @1577208, read by the status header of every screen (wasm func 5564 in
    // cformbuilder.cpp): '2' -> "SIMULADO", '3' -> "TREINAMENTO", other -> no label.
    // CEstadoGeral +48 is the fase of the urna (u06/u20: eg.bin "+48 fase"; the same value is the
    // first field of the file-name identification, see wasm 5728, and the argument of the
    // report separator wasm 2785).
    api::CFormBuilder::ms_fase = appInfo.GetGeral().GetFase();                     // names inferred (+48)

    // dcmp 6675-7132, inlined comum::CAppInfo::CarregaVotaInternoEmCache() (cappinfo.cpp):
    //   IndiceTurno("CarregaVotaInternoEmCache", TURNO_ATUAL), path from
    //   CServicoEstadoGeralVota(EFlashOrigem::INTERNA, TURNO_ATUAL) (wasm 3787 -> 3897),
    //   api::CFileASN::ReadFromFile<ModuloEstadoGeralVota::EstadoGeralVota>(path) (vota.bin, BER,
    //   limit 5 MiB, "O arquivo [{}] não existe/não estava aberto/é muito grande para ser lido",
    //   "Conteúdo não foi decodificado/inválido para {}: {}") and
    //   CConversorEstadoGeralVota::Desconverte ("Entidade está inválida: {}"), then stores the
    //   result in the per-turn optional<CEstadoGeralVota> cache of CAppInfo (+280 + 104 * turno).
    appInfo.CarregaVotaInternoEmCache();

    // dcmp 7133-7200
    auto& estadoGeralVota = appInfo.GetVota(EUrnaTurno::ATUAL);                     // wasm func 261
    const EEstadoVota estado = estadoGeralVota.GetEstadoVota();
    // compiled as `unsigned(estado - '1') <= 1`
    if (estado == EEstadoVota::INICIAL || estado == EEstadoVota::GERABASEDINAMICA) {
        if (estado == EEstadoVota::GERABASEDINAMICA) {
            // an interrupted generation of the dynamic base: discard what was written
            CInformacaoEleitor::GetInst().ApagarDadosDinamicos();
        } else {
            estadoGeralVota.SetEstadoVota(EEstadoVota::GERABASEDINAMICA);
            comum::SalvaEstadoVota();                                               // wasm func 491
        }
    }

    // dcmp 7202-7215
    comum::CPacoteArquivos::ValidarChaveEAplicacaoValida(comum::CPath::GetPathTrab(EFlashOrigem::INTERNA));
    comum::CPacoteArquivos::ValidarChaveEAplicacaoValida(comum::CPath::GetPathTrab(EFlashOrigem::EXTERNA));

    // dcmp 7216-7607
    CInformacaoEleitor::GetInst().InicializarPersistencia();

    // dcmp 7608-18395: every known state, INICIAL ('1') .. ENCERRADA ('@')
    // (compiled as `unsigned(estado - '1') <= 15`, i32.gt_u 15 -> skip). `estado` is the value read
    // before the update above. The dynamic-data load is NESTED in this test (the `gt_u 15` branch
    // jumps past the CompleteLoad call too), so an out-of-range state loads neither.
    if (estado >= EEstadoVota::INICIAL && estado <= EEstadoVota::ENCERRADA) {
        CInformacaoEleitor::GetInst().CarregarDadosEstaticos();

        // dcmp 18394-18395: the dynamic data (rdv.dat, uenux.db, dynamic voter file) exist once the
        // dynamic base has been generated, i.e. from AGUARDAHORAZERESIMA ('3') on (i32.lt_u 51 -> skip).
        if (estado >= EEstadoVota::AGUARDAHORAZERESIMA)
            CInformacaoEleitor::GetInst().CarregarDadosDinamicos();                 // wasm func 6734
    }

    // dcmp 18396-25754, inlined: "CTelasVota - instancia ja criada" (ctelasvota.cpp:3435).
    // The CTelasVota constructor (ctelasvota.cpp:3502, 252-byte object stored @1833396) builds
    // every voter screen: see ctelasvota.u02.cpp for the out-of-line CriaTela* helpers it calls.
    CTelasVota::CreateInst();

    // dcmp 25755-25776, inlined: "CInfoMTLCD - instancia ja criada" (cinfomtlcd.cpp:33),
    // 64-byte object stored @1838572.
    comum::CInfoMTLCD::CreateInst();

    // dcmp 25777-25928, inlined (cinstrucaovotacaoacessibilidade.cpp:157/161): pre-synthesises the
    // key names and the accessibility instruction at the 5 speech rates. In the web build the TTS
    // singleton at this point is simulador::CWasmNullTextToSpeech (votaInit registers RHVoice only
    // after this function returns), so nothing is actually synthesised. See the u02 doc §3.3.
    CInstrucaoVotacaoAcessibilidade::GetInst().CacheInstructionAudio();             // GetInst = wasm 4420
}

// ---------------------------------------------------------------------------------------------
// inlined into wasm func 7787 (dcmp 7138-7196)                          // name inferred
// The object is fetched (wasm func 509) but not used: the body only deletes files.
// std::filesystem::remove(const path&) is the throwing overload (__remove(p, nullptr)).
void CInformacaoEleitor::ApagarDadosDinamicos()
{
    std::filesystem::remove(ArquivoRdvInterno());
    std::filesystem::remove(ArquivoRdvExterno());
    std::filesystem::remove(ArquivoBancoInterno());
    std::filesystem::remove(ArquivoBancoExterno());
}

// ---------------------------------------------------------------------------------------------
// inlined into wasm func 7787 (dcmp 7216-7607)                          // name inferred
// Opens trab/uenux.db of the internal flash (SQLite 3.50.4) and registers the two DAOs used by VOTA
// in api::persistencia::CDAORepositorio (a std::map<std::string, shared_ptr<...>> @1909964 keyed by
// typeid(I).name()).
void CInformacaoEleitor::InicializarPersistencia()
{
    if (m_persistenciaInicializada)
        return;

    const std::string arquivoBanco = comum::CPath::GetPathTrab(EFlashOrigem::INTERNA) / "uenux.db";

    // CComparecimentoMesarioDAO(arquivo): shared_ptr<CSqlConnection>(new CSqlConnection(arquivo)),
    // then Prepare("CREATE TABLE IF NOT EXISTS comparecimento_mesario ( ... )")->vf10(); ->Close().
    // CDAORepositorio::Registrar<comum::dao::IComparecimentoMesarioDAO>:
    //   null  -> "O ponteiro para a DAO não pode ser nulo."   (cdaorepositorio.hpp:35, code 6800)
    //   found -> "Classe DAO {} já foi registrada."           (cdaorepositorio.hpp:52, code 6801)
    //   key   "N5comum3dao25IComparecimentoMesarioDAOE"
    api::persistencia::CDAORepositorio::Registrar<comum::dao::IComparecimentoMesarioDAO>(
        new comum::dao::CComparecimentoMesarioDAO(arquivoBanco));

    // CJustificadorDAO(arquivo): same, with
    //   "CREATE TABLE IF NOT EXISTS registro_justificativa ( numero_titulo BIGINT PRIMARY KEY UNIQUE
    //    NOT NULL, ano_nascimento SMALLINT )"
    //   key   "N5comum3dao16IJustificadorDAOE"
    api::persistencia::CDAORepositorio::Registrar<comum::dao::IJustificadorDAO>(
        new comum::dao::CJustificadorDAO(arquivoBanco));

    api::persistencia::CDAORepositorio::ms_registrado = true;                       // global @1838488, name inferred

    // wasm func 4662: throws api::CUeDesligandoError if the shutdown flag (@1832936) is set,
    // otherwise signs uenux.db and copies it from the internal to the external flash
    // ("Gravando o banco de dados na MI", wasm func 4657), then runs the MV copy (wasm 4682).
    CSincronizaVota::SincronizaBancoDados();                                        // name inferred

    m_persistenciaInicializada = true;
}

// ---------------------------------------------------------------------------------------------
// inlined into wasm func 7787 (dcmp 7609-18392)                         (srcloc line 181)
// Loads every static data set of the election ("dados estáticos": the files under estatico/ of the
// scenario) into its singleton, then runs the referential-integrity checks.
void CInformacaoEleitor::CarregarDadosEstaticos()
{
    if (m_dadosEstaticosCarregados)
        return;

    auto& appInfo = comum::CAppInfo::GetInst();

    // dcmp 7613-8090, inlined comum::CAppInfo::CarregaGapInternoEmCache(): gap.bin of the internal
    // flash (CServicoEstadoGeralGap, wasm 5812) -> ModuloEstadoGeralGap::EstadoGeralGap ->
    // optional<CEstadoGeralGap> cache (+48 * turno).
    appInfo.CarregaGapInternoEmCache();

    // dcmp 8095: CLocal::GetInst() (wasm 401) + ReadFromFile<ModuloLocal::Local> (wasm 5740).
    auto& local = comum::CLocal::GetInst();
    local.Carrega();                                                                // name inferred

    // dcmp 8096-10270, inlined comum::CConfiguracaoEleicao::CreateInst(estadoGeral, secoes, origem)
    // (cconfiguracaoeleicao.cpp:103/117/130/239): reads the ProcessoEleitoral, ParametrizacaoUrna
    // ("<fase><pleito><uf>...-pu.dat") and ConfiguracaoMunicipios files, builds CPleito (wasm 3713)
    // and CEleicaoDataHora (celeicaodatahora.cpp:30, "Datas inválidas"), stores the object @1838752
    // and ends with the inlined comum::CCargos::CreateInst() ("CCargos - instancia ja criada",
    // ccargos.cpp:25, @1838720) and comum::CRespostas::CreateInst() ("Instância já criada",
    // crespostas.cpp:33, @1838984).
    comum::CConfiguracaoEleicao::CreateInst(appInfo.GetGeral(), local.GetTodasSecoes(),
                                            EFlashOrigem::INTERNA);

    // dcmp 10271-10978, inlined comum::CHV::CreateInst(estadoGeral, origem) (chv.cpp:32/47/54/68):
    // ComplementosMunicipios (horário de verão / time zone of the municipality), @1838872.
    comum::CHV::CreateInst(appInfo.GetGeral(), EFlashOrigem::INTERNA);

    // dcmp 10979-11760: parties. CPartidos::GetInst() (wasm 819, @1838928) is filled from the
    // "...-pa.dat" files of every abrangência (wasm 2811 builds the names) through
    // ReadFromFile<ModuloPartidos::EntidadePartidos>.
    comum::CPartidos::GetInst().Carrega(comum::CPath::GetPathEstatico(EFlashOrigem::INTERNA));   // name inferred

    // dcmp 11761-13091, inlined comum::CFederacoes::Load(const std::string&) (cfederacoes.cpp:141/149):
    // "...-fe.dat" files; "o partido {} fazia parte das federações [{}] e [{}]".
    comum::CFederacoes::GetInst().Load(comum::CPath::GetPathEstatico(EFlashOrigem::INTERNA));    // GetInst = wasm 2832

    // dcmp 13092-14028, inlined comum::CFotos::Load() (cfotos.cpp:111/121): candidate photos through
    // api::CPartialFileASN (ModuloFotosCandidatos, visitor CVisitanteFoto), "Foto duplicada [{}] em {}".
    comum::CFotos::GetInst().Load();                                                // GetInst = wasm 2819

    // dcmp 14029-15643: candidacies. CCandidaturas::GetInst() (wasm 521) filled by the inlined
    // static CCandidaturas::LoadFromFile(path) (ccandidaturas.cpp:201/208/245, "...-ca.dat",
    // "{}: Candidatura duplicada para candidato/cargo {}/{}") plus the CabecalhoPacote of each file.
    comum::CCandidaturas::GetInst().Carrega(comum::CPath::GetPathEstatico(EFlashOrigem::INTERNA)); // name inferred

    // dcmp 15644-16951, inlined comum::CRdvVota::CreateInst() (crdvvota.cpp:92, @1838956):
    //   new CRdvVota(100 bytes) = CRdv(std::make_unique<CRdvPosicionadorVota>(),
    //                                  sorted office codes (wasm 5790 + std::sort, wasm 4820),
    //                                  CConfiguracaoEleicao::GetInst() byte +164 = party digits)
    //   -> GetCifradorCryptoTable -> CHKDFSeed (see crdv.cpp and chkdfseed.cpp),
    //   then CVotosCargos (cvotoscargos.cpp:30/37) and CConversorRegistroDigitalVoto<CConversorEleicoesVota>.
    comum::CRdvVota::CreateInst();

    // dcmp 16953-17153: biometric urna -> prepare the fingerprint subsystem.
    // CConfiguracaoEleicao byte +665; IFingerPrepare vtable slot 2. In the simulator the singleton is
    // simulador::CFingerPrepareSimulador, whose slot 2 is a no-op (wasm func 218).
    if (comum::CConfiguracaoEleicao::GetInst().EhUrnaBiometrica())                  // name inferred
        api::CPolySingleton<api::IFingerPrepare>::instance().Prepara();             // line 181, name inferred

    // dcmp 17154-18039, inlined comum::CEleitores::StaticLoad(const std::string&)
    // (celeitores.cpp:395 "Eleitor {} com biometria em urna não biométrica",
    //  celeitores.cpp:407 "Identidade principal {} duplicada"): UF, município and the biometric flag
    // of CLocal, the "...-el.dat"/"...-tte.dat" file names of every section (wasm 3745/5727),
    // voters sorted and validated one by one.
    comum::CEleitores::StaticLoad(comum::CPath::GetPathEstatico(EFlashOrigem::INTERNA));

    // dcmp 18040-18392: referential integrity of the static data. Each check builds a
    // CIntegridadeReferencial::TResultado {bool ok; std::string erro} and
    // CIntegridadeReferencial::Lanca (wasm 2263) throws when ok == false.
    {
        const auto cargos = comum::CCargos::GetInst().GetMapaCargos();              // wasm 2834, name inferred
        auto& candidaturas = comum::CCandidaturas::GetInst();                       // wasm 521
        auto& partidos     = comum::CPartidos::GetInst();                           // wasm 819
        auto& fotos        = comum::CFotos::GetInst();                              // wasm 2819
        auto& federacoes   = comum::CFederacoes::GetInst();                         // wasm 2832

        using comum::CIntegridadeReferencial;
        // 1. office of every candidacy exists, is a candidate office and has the same number of
        //    alternates (wasm 5747)
        CIntegridadeReferencial::Lanca(CIntegridadeReferencial::VerificaCargosCandidaturas(candidaturas, cargos));
        // 2. "o partido ({}) da candidatura ({},{}) não foi encontrado"
        CIntegridadeReferencial::Lanca(CIntegridadeReferencial::VerificaPartidosCandidaturas(candidaturas, partidos));
        // 3. photo of the candidate and of each alternate: "a foto (<nome>) da candidatura ({}[{}]) não foi encontrada"
        //    (wasm 5746, called for index 0 and 1..qtdSuplentes)
        CIntegridadeReferencial::Lanca(CIntegridadeReferencial::VerificaFotosCandidaturas(candidaturas, cargos, fotos));
        // 4. "o partido ({}) da federação ({},{}) não foi encontrado"
        CIntegridadeReferencial::Lanca(CIntegridadeReferencial::VerificaPartidosFederacoes(federacoes, partidos));

        m_dadosEstaticosCarregados = true;
    }   // ~map (wasm func 1835)
}

} // namespace vota
