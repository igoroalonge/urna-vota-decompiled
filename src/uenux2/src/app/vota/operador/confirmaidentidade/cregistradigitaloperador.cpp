// Reconstructed from vota_web_wasm.wasm (unit u27; the constructor/GetInst block is unit u17's, merged here).
// Original: uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp
// (attested srclocs: StartState :95 :98, FinishState :104 :105, ProcessTick :143 :144 :145,
//  BiometriaMesarioPresenteNosEleitores :328 :371).
//
// Functions of this file (wasm index -> symbol):
//   5396   GetInst + constructor (unit u17)            10454 StartState      10453 FinishState
//   10452  ProcessInput                                10451 ProcessTick     3614  IdentificaMesario
//          (with BiometriaMesarioPresenteNosEleitores inlined; the tools used the inlined srcloc name)
//   3258   CLogVota "Mesário {} habilitou o eleitor"   4522  CLogVota "Habilitação cancelada durante a captura..."
//   5374   ListaDigitaisNaoRegistradas                 3793  DiretorioWsq(INTERNA, "nao-registrado/")
//   5815   DiretorioWsq(INTERNA, "registrado/")        2298  DiretorioWsq(INTERNA, subdir) (shared)
//   310 / 2726 / 5370 / 5373   std::sort internals for vector<string> with the by-value "a > b" lambda
//          (comparator, __sort4 (four iterators, __sort3 inlined), __insertion_sort_incomplete, __introsort)
//   5376   std::map<CEleitorIdentidade, CComparecimentoMesario>::insert(hint, value) (__emplace_hint_unique)
//   2805   __find_equal(hint, ...) keyed by CEleitorIdentidade: used for that map AND for the copy of the
//          voter roll (map<CEleitorIdentidade, CEleitorDetalhe>) in 3614 (identical code, one body)
//   1902   map<CEleitorIdentidade, CComparecimentoMesario>: __tree::destroy
//   1055   std::filesystem::status (libc++; the only function of this unit seen running, from start-up code)
//
// WEB BUILD: dead code - the operator thread never runs, and the fingerprint hardware interfaces
// (IFingerScanner, IFingerDetection, IFingerMatcher) have no registered implementation, so the first
// CPolySingletonList::instance<> lookup would throw "solicitada uma instancia nao criada".
#include "vota/operador/confirmaidentidade/cregistradigitaloperador.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <format>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "api/gui/cbmpconversor.h"
#include "api/gui/cformbuildermt.h"
#include "api/hwil/ifingerdetection.h"
#include "api/hwil/ifingermatcher.h"
#include "api/hwil/ifingerscanner.h"
#include "api/hwil/iurna.h"
#include "comum/appinfo/cappinfo.h"
#include "comum/comparecimentomesario/ccontroladorreconhecimetomesario.h"
#include "comum/comparecimentomesario/cregistradormesario.h"
#include "comum/cpath.h"
#include "comum/dados/celeitores.h"
#include "ecourna/api/io/cfile.h"
#include "vota/log/clogvota.h"
#include "vota/operador/comum/iinformacaothreadoperador.h"
#include "vota/operador/confirmaidentidade/ccontrolareconhecimento.h"
#include "vota/operador/confirmaidentidade/cinformaeleitorpodevotar.h"
#include "vota/operador/leidentidade/cpedeidentidade.h"

namespace vota {

namespace fs = std::filesystem;
using api::SPoint;
using comum::md::CComparecimentoMesario;
using comum::md::CEleitorIdentidade;

namespace {

constexpr auto ESQUERDA = api::ETextAlignment(0);
constexpr auto DIREITA  = api::ETextAlignment(1);
constexpr auto CENTRO   = api::ETextAlignment(2);

// IFingerScanner LED modes (slot 7), as in cpededigital.cpp (u10).                               ?
enum class ELedLeitor : int { APAGADO = 0, RECONHECIDO = 1, CAPTURANDO = 2 };

// 999999 = "no image id" (CComparecimentoMesario::m_idArquivo not set; also CControlaArmazenamentoDeImagens'
// failure value).
constexpr std::uint32_t SEM_ID_ARQUIVO = 999999;

// Only the 6 newest unregistered captures are compared.                                  name inferred
constexpr std::size_t MAX_DIGITAIS_NAO_REGISTRADAS = 6;

using TMapaMesarios = std::map<CEleitorIdentidade, CComparecimentoMesario>;   // node 84 bytes (funcs 2805/5376/1902)

// wasm func 2298 (tools: vota_f2298) and its two thunks 3793 / 5815 (the string pair is passed as
// (end, begin)). <CPath::GetPathTrab(INTERNA, turno)>/wsq/<subdiretorio> (func 2864 = .../wsq/).   name inferred
fs::path DiretorioWsq(const char* subdiretorio)
{
    const auto turno = comum::GetEstadoGeral(comum::CAppInfo::GetInst()).GetTurno();   // GetEstado@457 +8
    return comum::CPath::GetPathWsq(comum::EFlashOrigem::INTERNA, turno) / subdiretorio;   // func 2864
}
fs::path DiretorioNaoRegistrado() { return DiretorioWsq("nao-registrado/"); }       // wasm func 3793
fs::path DiretorioRegistrado()    { return DiretorioWsq("registrado/"); }           // wasm func 5815

// Shannon entropy of the grey-level histogram (inlined; double precision here, float in CPedeDigital).
double Entropia(const std::vector<uebyte>& imagem)
{
    std::map<uebyte, int> histograma;
    for (const uebyte pixel : imagem)
        ++histograma[pixel];
    double soma = 0.0;
    const double total = static_cast<double>(imagem.size());
    for (const auto& [valor, qtd] : histograma) {
        const double p = qtd / total;
        soma += p * std::log2(p);
    }
    return -soma;
}

// Stubs compiled away in this build (u10 §6.4): WSQ encoder (func 2742 singleton) and minutiae extractor
// (func 1903 parameters + IFingerScanner slots 2/1, output cleared by func 2224).                   ?
std::vector<uebyte> CodificaWsq(const std::vector<uebyte>& bruta, long largura, long altura);
void ExtraiTemplate(api::IFingerScanner& leitor, const std::vector<uebyte>& digital, std::vector<uebyte>& modelo);

// wasm func 3258: thunk into the merged "log with one {} string" body (func 6113)
void LogaMesarioHabilitou(CLogVota& log, const std::string& titulo)
{
    log.Loga(std::format("Mesário {} habilitou o eleitor", titulo));
}

// wasm func 4522                                                                          name inferred
void LogaHabilitacaoCanceladaCapturaMesario(CLogVota& log)
{
    log.Loga("Habilitação cancelada durante a captura de digital do mesário");
}

// wasm func 5374 (tools: vota_f5374). Names of the files in <trab>/wsq/nao-registrado/ (internal flash),
// newest first. Also called by CControladorReconhecimentoMesario's constructor (func 1149) to seed its
// file counter. directory_iterator is built WITHOUT an error_code: a missing directory throws
// std::filesystem::filesystem_error.                                                     name inferred
std::vector<std::string> ListaDigitaisNaoRegistradas()
{
    std::vector<std::string> nomes;
    for (const fs::directory_entry& entrada : fs::directory_iterator(DiretorioNaoRegistrado())) {
        if (entrada.is_directory())                        // cached type, refreshed with fs::status (func 1055)
            continue;
        nomes.push_back(entrada.path().filename());        // func 1224
    }
    // Names are "me%06u.wsq" with a sequential id (++m_qtdArquivos), so descending order = newest first.
    std::sort(nomes.begin(), nomes.end(),
              [](std::string a, std::string b) { return a > b; });   // funcs 5373/5370/2726/310; strings by VALUE
    return nomes;
}

}  // namespace

// ---------------------------------------------------------------------------------------------------
// Constructor (unit u17's reconstruction, inlined into GetInst, wasm func 5396). 36 bytes.
CRegistraDigitalOperador::CRegistraDigitalOperador()
    : comum::CAppState(6)                                                    // keys + ticks
    , m_tickTempoEsgotado(CThreadOperador::GetInst().CriaTick(15000))        // func 807
    , m_tickLeitura(CThreadOperador::GetInst().CriaTick(50))
{
    {
        api::CFormBuilderMT campos;
        campos.Add<api::CLedFieldMT>(false);
        campos.Add<api::CTextFieldMT>(SPoint{1, 1}, std::make_shared<api::CFixedText>(ESQUERDA, "MESÁRIO:"));
        campos.Add<api::CTextFieldMT>(SPoint{1, 2}, std::make_shared<api::CFixedText>(ESQUERDA, "Posicione seu dedo POLEGAR ou INDICADOR"));
        campos.Add<api::CTextFieldMT>(SPoint{1, 3}, std::make_shared<api::CFixedText>(ESQUERDA, "sobre o sensor"));
        campos.Add<api::CTextFieldMT>(SPoint{1, 4}, std::make_shared<api::CFixedText>(ESQUERDA, "CORRIGE: não habilitar"));
        campos.AddInputControl();
        m_formCaptura = campos.CriaFormInterativo("", true);
    }
    {
        api::CFormBuilderMT campos;
        campos.Add<api::CLedFieldMT>(false);
        campos.Add<api::CBeepFieldMT>(1);
        campos.Add<api::CTextFieldMT>(SPoint{20, 1}, std::make_shared<api::CFixedText>(CENTRO, "Digital capturada"));
        campos.Add<api::CTextFieldMT>(SPoint{20, 3}, std::make_shared<api::CFixedText>(CENTRO, "Por favor aguarde"));
        m_formCapturada = campos.CriaFormInterativo("", true);
    }
    m_tentativasRestantes = 3;
}

// wasm func 5396: lazy singleton @1909148 (mutex @1909124). Callers: CVerificaDadoEleitor (10443),
// CDigitalNaoCapturada (10458).
CRegistraDigitalOperador& CRegistraDigitalOperador::GetInst()
{
    static std::mutex mutex;
    static std::unique_ptr<CRegistraDigitalOperador> s_inst;
    std::lock_guard lock(mutex);
    if (!s_inst)
        s_inst.reset(new CRegistraDigitalOperador());
    return *s_inst;
}

// ---------------------------------------------------------------------------------------------------
// wasm func 10454 (vtable slot 2, srclocs :95, :98)
void CRegistraDigitalOperador::StartState()
{
    CLogVota::GetInst().Loga("Solicita digital do mesário");                                // api_f233, level 1
    m_formCaptura->Show();
    api::CPolySingletonList::instance<api::IFingerScanner>().SetLed(ELedLeitor::CAPTURANDO);  // :95 slot 7 (2)
    CThreadOperador& operador = CThreadOperador::GetInst();
    operador.StartTick(m_tickTempoEsgotado);
    operador.StartTick(m_tickLeitura);
    api::CPolySingletonList::instance<api::IFingerScanner>().IniciaCaptura();               // :98 slot 3
    m_proximoEstado = this;
}

// ---------------------------------------------------------------------------------------------------
// wasm func 10453 (vtable slot 5, srclocs :104, :105) -> merged body 6012 (same code as another
// state's FinishState, srcloc records passed as parameters).
void CRegistraDigitalOperador::FinishState()
{
    api::CPolySingletonList::instance<api::IFingerScanner>().SetLed(ELedLeitor::APAGADO);   // :104 slot 7 (0)
    api::CPolySingletonList::instance<api::IFingerScanner>().FinalizaCaptura();            // :105 slot 6
}

// ---------------------------------------------------------------------------------------------------
// wasm func 10452 (vtable slot 7). CInteractiveForm::Read inlined (cinteractiveform.h:57 @1592624).
void CRegistraDigitalOperador::ProcessInput()
{
    if (m_formCaptura->Read() != api::EInputResult::CORRIGE)                                // "CORRIGE: não habilitar"
        return;
    CThreadOperador& operador = CThreadOperador::GetInst();
    operador.StopTick(m_tickTempoEsgotado);
    operador.StopTick(m_tickLeitura);
    LogaHabilitacaoCanceladaCapturaMesario(CLogVota::GetInst());                            // func 4522
    m_proximoEstado = &CPedeIdentidade::GetInst();                                          // func 652
}

// ---------------------------------------------------------------------------------------------------
// wasm func 10451 (vtable slot 8, srclocs :143, :144, :145)
void CRegistraDigitalOperador::ProcessTick(const uebyte tick)
{
    if (tick != m_tickTempoEsgotado && tick != m_tickLeitura)
        return;

    CThreadOperador& operador = CThreadOperador::GetInst();

    // ---- 15 s without a finger -----------------------------------------------------------------------
    if (tick == m_tickTempoEsgotado) {
        operador.StopTick(m_tickTempoEsgotado);
        operador.StopTick(m_tickLeitura);
        if (--m_tentativasRestantes == 0) {
            m_tentativasRestantes = 3;
            m_proximoEstado = &CTentativaCapturaDigitalEsgotada::GetInst();
            //   ^ @1909092 (mutex @1909068), 20 bytes, ctor inlined: CAppState(2); LED off; Beep(1);
            //     (20,1) centred "Digital do mesário não capturada"; (20,2) centred "Eleitor não habilitado
            //     para votação"; (40,4) right "CONFIRMA: prosseguir"; input control. ProcessInput = 10461
            //     (CONFIRMA -> CPedeIdentidade, shared body 2295).
            LogaHabilitacaoCanceladaCapturaMesario(CLogVota::GetInst());                    // func 4522
        } else {
            m_proximoEstado = &CDigitalNaoCapturada::GetInst();
            //   ^ @1909120 (mutex @1909096), 20 bytes, ctor inlined: CAppState(2); LED off; Beep(1);
            //     (20,2) centred "Digital não capturada"; (40,4) right "CONFIRMA: tentar novamente";
            //     input control. ProcessInput = 10458 (other unit): back to CRegistraDigitalOperador.
        }
        return;
    }

    // ---- 50 ms: poll the sensor ----------------------------------------------------------------------
    api::IFingerDetection& detector = api::CPolySingletonList::instance<api::IFingerDetection>();   // :143 (3615)
    api::IFingerScanner&   leitor   = api::CPolySingletonList::instance<api::IFingerScanner>();     // :144 (816)
    api::IUrna&            urna     = api::CPolySingletonList::instance<api::IUrna>();              // :145 (923)

    // Frame selection, identical to CPedeDigital::ProcessTick (u10): models 2009/2010/2020/2022 use the
    // first complete frame; other models need 3 frames with entropy > 4 bits and keep the best one.
    api::TSharedImageVector melhor;
    double melhorEntropia = 0.0;
    for (int quadros = 0; quadros <= 2;) {
        api::TSharedImageVector imagem = leitor.CapturaImagem();                              // slot 4
        if (imagem->size() != leitor.GetTamanhoImagem())                                      // slot 0
            return;
        const int modelo = urna.GetModelo();
        if (modelo == 2009 || modelo == 2010 || modelo == 2020 || modelo == 2022) {
            melhor = imagem;
            break;
        }
        const double entropia = Entropia(*imagem);
        if (!(entropia > 4.0))
            return;
        if (entropia > melhorEntropia) {
            melhor = imagem;
            melhorEntropia = entropia;
        }
        ++quadros;
    }
    if (!detector.DedoPresente(*melhor))                                                      // slot 2
        return;

    m_formCapturada->Show();                                          // "Digital capturada / Por favor aguarde"
    leitor.SetLed(ELedLeitor::RECONHECIDO);                                                   // slot 7 (1)
    if (urna.GetModelo() <= 2019)
        api::CBmpConversor::InvertColors(melhor->data(), leitor.GetLargura(), leitor.GetAltura());   // slots 2, 1
    api::CBmpConversor::VerticalFlip(melhor->data(), leitor.GetLargura(), leitor.GetAltura());
    const auto wsq = std::make_shared<std::vector<uebyte>>(
        CodificaWsq(*melhor, leitor.GetLargura(), leitor.GetAltura()));                       // stub -> empty
    CControlaReconhecimento::s_digitalMesario = *wsq;                                         // @1908684

    // Who is this mesário? Only used for the log and for s_tituloMesario.
    if (!ProcuraMesarioRegistrado() && !ProcuraEmDigitaisNaoRegistradas()) {
        CLogVota& log = CLogVota::GetInst();
        log.Loga("Capturada a digital do mesário");
        log.Loga("Não encontrou digital coletada em nenhum dos arquivos, vai salvar a digital em novo arquivo.");
        SalvaDigitalNaoRegistrada();
    }

    // The voter is released in every case.
    operador.StopTick(m_tickTempoEsgotado);
    operador.StopTick(m_tickLeitura);
    impl::IInformacaoThreadOperador::GetInst().SetHabilitacaoCodigoMesario();                 // slot 12 (tipo 2)
    m_proximoEstado = &CInformaEleitorPodeVotar::GetInst();                                   // func 5399
}

// Inlined into 10451. Three passes over the mesários of CRegistradorMesario (comparecimento_mesario),
// each one copied into a temporary std::map keyed by identity (funcs 5376/2805, destroyed by 1902):
//   1. registered mesários whose fingerprint was matched at registration (dedo != 0),
//   2. registered mesários without a matched finger (dedo == 0),
//   3. rows of mesários that are NOT voters of this section (pertenceSecao != true), all periods.
// The first mesário for whom IdentificaMesario() succeeds is logged; the search stops.   name inferred
bool CRegistraDigitalOperador::ProcuraMesarioRegistrado()
{
    auto& registrador = comum::CRegistradorMesario::GetInst();                                // func 815
    const auto verifica = [](const TMapaMesarios& mesarios) {
        for (const auto& [identidade, comparecimento] : mesarios) {
            const auto idArquivo = comparecimento.GetIdArquivo().value_or(SEM_ID_ARQUIVO);
            if (IdentificaMesario(identidade, idArquivo)) {                                   // func 3614
                LogaMesarioHabilitou(CLogVota::GetInst(), identidade.GetIdentidade());        // func 3258
                return true;
            }
        }
        return false;
    };

    TMapaMesarios comDedo, semDedo, naoEleitores;
    // (in the binary each pass builds its map right before walking it - a filtered range copied into a
    //  std::map with insert-at-end hints, func 5376 - so pass 2 is only built if pass 1 found nobody)
    for (const auto& [identidade, c] : registrador.PorIdentidade()) {                         // map at +28
        if (c.GetDedo() != 0)
            comDedo.emplace(identidade, c);
        else
            semDedo.emplace(identidade, c);
    }
    if (verifica(comDedo) || verifica(semDedo))
        return true;
    for (const auto& [pk, c] : registrador.Registros())                                       // CDataMap at +0
        if (!c.PertenceSecao())
            naoEleitores.emplace(c.GetIdentidade(), c);
    return verifica(naoEleitores);
}

// Inlined into 10451: compare with the 6 newest captures of mesários that could not be identified
// earlier (<trab>/wsq/nao-registrado/me%06u.wsq). A match means "a mesário who released a voter
// before" - still unidentified.                                                          name inferred
bool CRegistraDigitalOperador::ProcuraEmDigitaisNaoRegistradas()
{
    auto& controlador = comum::CControladorReconhecimentoMesario::GetInst();                  // func 1149
    const std::vector<uebyte> digital = CControlaReconhecimento::s_digitalMesario;
    std::vector<std::string> arquivos = ListaDigitaisNaoRegistradas();                        // func 5374
    arquivos.resize(std::min(arquivos.size(), MAX_DIGITAIS_NAO_REGISTRADAS));
    for (const std::string& nome : arquivos) {
        const fs::path caminho = DiretorioNaoRegistrado() / nome;                             // func 3793
        const fs::file_status status = fs::status(caminho);                                   // func 1055
        if (status.type() == fs::file_type::none || status.type() == fs::file_type::not_found)
            continue;
        const std::vector<uebyte> gravada = ecourna::api::io::CFile::ReadFileBinary(caminho);
        if (controlador.ComparaDigitais(digital, gravada)) {                                  // func 5372
            CLogVota::GetInst().Loga("Digital coletada bate com uma digital gravada após o registro inicial de "
                                     "mesário. Não é possível associar a habilitação a um mesário.");
            return true;
        }
    }
    return false;
}

// Inlined CControladorReconhecimentoMesario::SalvarBiometriaMesarioNaoRegistrado(digital)
// (lambda $_0, std::function vtable @1595908): writes "me{:06}.wsq" with id = ++m_qtdArquivos into both
// <trab fi>/wsq/nao-registrado/ and <trab fe>/wsq/nao-registrado/ (func 5371: needs >= 5 MiB free,
// skips ids whose file exists, "w+b"). The image is NOT encrypted on this path (compare with
// CControlaArmazenamentoDeImagens, u24).                                                  name inferred
void CRegistraDigitalOperador::SalvaDigitalNaoRegistrada()
{
    auto& controlador = comum::CControladorReconhecimentoMesario::GetInst();
    const std::vector<uebyte> digital = CControlaReconhecimento::s_digitalMesario;
    const auto turno = comum::GetEstadoGeral(comum::CAppInfo::GetInst()).GetTurno();
    controlador.GravaWsq(digital,
                         DiretorioNaoRegistrado(),                                                   // func 3793
                         comum::CPath::GetPathWsq(comum::EFlashOrigem::EXTERNA, turno) / "nao-registrado/",   // 2864
                         [&controlador] { return ++controlador.m_qtdArquivos; });                    // func 10296
}

// ---------------------------------------------------------------------------------------------------
// wasm func 3614 (tools: "CRegistraDigitalOperador::BiometriaMesarioPresenteNosEleitores" - the srclocs
// :328/:371 belong to that function, inlined here). Arguments: the mesário's identity and the id of the
// "me%06u.wsq" image stored when he registered (999999 = none).                        name inferred
bool CRegistraDigitalOperador::IdentificaMesario(const CEleitorIdentidade& identidade, const std::uint32_t idArquivo)
{
    const bool nosEleitores = BiometriaMesarioPresenteNosEleitores(identidade);
    CControlaReconhecimento::s_tituloMesario.clear();                                   // @1908696
    CLogVota& log = CLogVota::GetInst();

    if (nosEleitores) {
        log.Loga(idArquivo != SEM_ID_ARQUIVO
                     ? std::format("Biometria do mesário {} encontrada em eleitores ({})", identidade.GetIdentidade(), idArquivo)
                     : std::format("Biometria do mesário {} encontrada em eleitores", identidade.GetIdentidade()));
        CControlaReconhecimento::s_tituloMesario = identidade.GetIdentidade();
        return true;
    }

    // Fingerprint captured when this mesário registered: <trab fi>/wsq/registrado/me<id>.wsq
    // (also tried with id 999999, i.e. "me999999.wsq").
    const std::vector<uebyte> digital = CControlaReconhecimento::s_digitalMesario;
    auto& controlador = comum::CControladorReconhecimentoMesario::GetInst();                 // func 1149
    const bool nosArquivos = [&] {
        const fs::path caminho = DiretorioRegistrado() / std::format("me{:06}.wsq", static_cast<int>(idArquivo));   // 5815
        const fs::file_status status = fs::status(caminho);                                  // func 1055
        if (status.type() == fs::file_type::none || status.type() == fs::file_type::not_found)
            return false;
        return controlador.ComparaDigitais(digital, ecourna::api::io::CFile::ReadFileBinary(caminho));
    }();
    if (nosArquivos) {
        log.Loga(std::format("Biometria do mesário {} encontrada nos arquivos coletados ({})",
                             identidade.GetIdentidade(), idArquivo));
        CControlaReconhecimento::s_tituloMesario = identidade.GetIdentidade();
        return true;
    }
    log.Loga(idArquivo != SEM_ID_ARQUIVO
                 ? std::format("Biometria coletada não é do mesário {} ({})", identidade.GetIdentidade(), idArquivo)
                 : std::format("Biometria coletada não é do mesário {}", identidade.GetIdentidade()));
    return false;
}

// srclocs :328 and :371 - inlined into func 3614.
bool CRegistraDigitalOperador::BiometriaMesarioPresenteNosEleitores(const CEleitorIdentidade& identidade)
{
    api::IFingerScanner& leitor = api::CPolySingletonList::instance<api::IFingerScanner>();   // line 328
    std::vector<uebyte> amostra;
    ExtraiTemplate(leitor, CControlaReconhecimento::s_digitalMesario, amostra);                // stub -> empty

    // Finger to compare: the one matched when the mesário registered at the opening (period 1);
    // if he never registered, all four fingers of the 1x4 order; registered without a match -> no.
    std::vector<comum::md::CDedo::TipoDedo> dedos;
    const CComparecimentoMesario* registro = comum::CRegistradorMesario::GetInst().Localiza(
        comum::md::CComparecimentoMesarioPK(identidade, comum::md::EPeriodoPresente::ABERTURA));   // funcs 2738, 2222
    if (registro == nullptr)
        dedos = {1, 6, 2, 7};                          // right thumb, left thumb, right index, left index
    else if (registro->GetDedo() == 0)
        return false;
    else
        dedos = {registro->GetDedo()};

    // NOTE: the whole section roll (map<CEleitorIdentidade, CEleitorDetalhe>, 236-byte nodes with the
    // biometric templates) is COPIED here before the lookup (funcs 2805/946/5760, freed by 1270).
    const auto eleitores = comum::CEleitores::GetInst().GetMapa();
    const auto it = eleitores.find(identidade);                                                // func 1703
    if (it == eleitores.end())
        return false;

    for (const auto dedo : dedos) {
        if (!it->second.GetBiometria().PossuiDedo(dedo))                                       // funcs 1937, 3719
            continue;
        const std::vector<uebyte> modelo = it->second.GetBiometria().GetDedo(dedo).GetTemplate();   // func 3720
        const int limiar = comum::CControladorReconhecimentoMesario::GetInst().m_limiarScore;  // +4 (20)
        api::IFingerMatcher& comparador = api::CPolySingletonList::instance<api::IFingerMatcher>();   // line 371
        if (comparador.Compara(amostra, modelo, limiar))                                        // slot 2
            return true;
    }
    return false;
}

}  // namespace vota
