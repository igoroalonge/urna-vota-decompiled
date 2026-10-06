// uenux2/src/app/comum/comparecimentomesario/estados/cpededigitalmesario.cpp
// Reconstructed from vota_web_wasm.wasm (unit u22). Attested by the std::source_location records
// :47 (CNomeMesariosUrnaDS::Text), :106 / :109 (StartState), :141..:143 / :182 (ProcessTick),
// :262 (ExecutaBatimentoDigitalMesario), :298 / :299 (FinishState), :304 (GetControlador).
//
// Same capture logic as vota::CPedeDigital (u10 §4.2), but for a mesário: 15 s time-out, 50 ms polling
// tick, up to 3 frames, entropy test, then a 1:N comparison against the mesário's own fingerprints
// taken from the voter roll (fingers 1, 6, 2, 7), and a record of the outcome in
// CControladorReconhecimentoMesario (read later by IGestorDadoMesario).
//
// Web build: dead code - the operator thread is not run, and IFingerScanner / IFingerMatcher /
// IFingerDetection have no implementation (CPolySingletonList would throw). ProcessTick also contains
// an `emscripten_sleep(1000)` (guarded by the byte @1584624, which is 1) that aborts in this build
// (no Asyncify).
#include "comum/comparecimentomesario/estados/estadosregistromesarios.h"

#include <cmath>
#include <format>
#include <map>
#include <mutex>

#include "api/gui/cbmpconversor.h"
#include "api/gui/cformbuildermt.h"
#include "api/hwil/ifingerdetection.h"
#include "api/hwil/ifingermatcher.h"
#include "api/hwil/ifingerscanner.h"
#include "api/hwil/iurna.h"
#include "api/pattern/cpolysingletonlist.h"
#include "comum/comparecimentomesario/ccontroladorreconhecimetomesario.h"
#include "comum/reconhecimentobiometrico/ccontrolaarmazenamentodeimagens.h"
#include "comum/cpath.h"
#include "comum/dados/celeitordetalhe.h"

namespace comum {

using api::SPoint;

namespace {
// srcloc :304
IControladorRegistraMesarios& GetControlador()
{
    return api::CPolySingletonList::instance<IControladorRegistraMesarios>();
}

// IFingerScanner LED modes (slot 7), as in u10.                                            ?
enum class ELedLeitor : int { APAGADO = 0, RECONHECIDO = 1, CAPTURANDO = 2 };

float Entropia(const std::vector<uebyte>& imagem)                  // inlined; identical to u10's
{
    std::map<uebyte, int> histograma;
    for (const uebyte pixel : imagem)
        ++histograma[pixel];
    float soma = 0.0f;
    const float total = static_cast<float>(imagem.size());
    for (const auto& [valor, qtd] : histograma) {
        const float p = static_cast<float>(qtd) / total;
        soma += p * std::log2f(p);
    }
    return -soma;
}

// Fingers tried, in order: right thumb, left thumb, right index, left index (CDedo numbering).
constexpr md::CDedo::TipoDedo DEDOS[4] = {md::CDedo::TipoDedo{1}, md::CDedo::TipoDedo{6},
                                          md::CDedo::TipoDedo{2}, md::CDedo::TipoDedo{7}};
} // namespace

// wasm func 10319 (table slot 4386) - the mesário's name as printed on the MT (body func 6008 with
// srcloc :47; the same function exists in cdigitalmesarionaoreconhecida.cpp, func 10323).
namespace {
struct CNomeMesariosUrnaDS {
    static std::string Text(const std::string& formato)
    {
        auto& controlador = api::CPolySingletonList::instance<IControladorRegistraMesarios>();   // :47
        if (!controlador.MesarioEhEleitorDaSecao())                                           // slot 13
            return controlador.GetTituloMesario();          // not a voter here: the raw título (no format)
        const md::CEleitorDecorator eleitor = controlador.GetEleitorMesario()->GetEleitor();  // copy (func 946)
        const std::string& nome = eleitor.GetNomeSocial().empty() ? eleitor.GetNome() : eleitor.GetNomeSocial();
        const std::string nomeUrna = nome.substr(0, 40);
        return std::vformat(formato, std::make_format_args(nomeUrna));
    }
};
} // namespace

// Constructor + GetInst: inlined into IPedeTituloMesario::ProcessInput (func 10313).
CPedeDigitalMesario::CPedeDigitalMesario()
    : CAppState(6)                                                                // keys + ticks
    , m_tickTempoEsgotado(GetControlador().CriaTick(15000))                       // slot 2 (:304)
    , m_tickLeitura(GetControlador().CriaTick(50))
{
    api::CFormBuilderMT captura;
    captura.Add<api::CLedFieldMT>(false);                                         // func 435
    captura.Add<api::CBuzzFieldMT>(52, 5);                                        // func 941
    captura.Add<api::CTextFieldMT>(SPoint{1, 1}, std::make_shared<api::CFixedText>(CENTRO, "Solicite que o(a) mesário(a)"));   // x = 1, centred
    captura.Add<api::CTextFieldMT>(SPoint{1, 2}, std::make_shared<api::CDataTextFmt<std::string (*)(const std::string&)>>(
                                                      CENTRO, &CNomeMesariosUrnaDS::Text, "{:2}"));   // func 651
    captura.Add<api::CTextFieldMT>(SPoint{1, 3}, std::make_shared<api::CFixedText>(ESQUERDA, "posicione POLEGAR ou INDICADOR no sensor"));
    captura.Add<api::CTextFieldMT>(SPoint{1, 4}, std::make_shared<api::CFixedText>(ESQUERDA, "CORRIGE: Cancelar"));
    captura.AddInputControl();
    m_formCaptura = captura.CriaFormInterativo("", true);                         // +16

    api::CFormBuilderMT aguarde;
    aguarde.Add<api::CLedFieldMT>(false);
    aguarde.Add<api::CTextFieldMT>(SPoint{1, 2}, std::make_shared<api::CFixedText>(CENTRO, "Por favor, aguarde."));
    m_formAguarde = aguarde.CriaFormInterativo("", true);                         // +24
}

CPedeDigitalMesario& CPedeDigitalMesario::GetInst()
{
    static std::mutex mutex;                                        // @1909880
    static std::unique_ptr<CPedeDigitalMesario> s_inst;             // @1909904 (dtor: ICF 1537)
    std::lock_guard trava(mutex);
    if (!s_inst)
        s_inst.reset(new CPedeDigitalMesario());
    return *s_inst;
}

// wasm func 10318 - vtable slot 2 (srcloc :106, :109, :304)
void CPedeDigitalMesario::StartState()
{
    GetControlador().LogaPedidoLeituraBiometria();                                // slot 31
    m_formCaptura->Show();
    api::CPolySingletonList::instance<api::IFingerScanner>().SetLed(ELedLeitor::CAPTURANDO);   // :106, slot 7
    GetControlador().StartTick(m_tickTempoEsgotado);                              // slot 4
    GetControlador().StartTick(m_tickLeitura);
    api::CPolySingletonList::instance<api::IFingerScanner>().Start();            // :109, slot 3
    m_proximoEstado = this;
}

// wasm func 10315 - vtable slot 5. Body shared with vota::CRegistraDigitalOperador::FinishState
// (func 6012(this, srcloc :299, srcloc :298)).
void CPedeDigitalMesario::FinishState()
{
    api::CPolySingletonList::instance<api::IFingerScanner>().SetLed(ELedLeitor::APAGADO);      // :298, slot 7
    api::CPolySingletonList::instance<api::IFingerScanner>().Stop();                          // :299, slot 6
}

// wasm func 10317 - vtable slot 7
void CPedeDigitalMesario::ProcessInput()
{
    if (m_formCaptura->Read() == api::EInputResult::CORRIGE)                      // cinteractiveform.h:57
        m_proximoEstado = &CPedeTituloMesario::GetInst();                          // func 2729
}

// Inlined into ProcessTick (srcloc :262). Returns true when one of the four fingers matched.
bool CPedeDigitalMesario::ExecutaBatimentoDigitalMesario(const api::TSharedImageVector& digital,
                                                        const long largura, const long altura)
{
    std::vector<uebyte> amostra;
    ExtraiTemplate(*digital, largura, altura, amostra);   // stub in this build: func 1903 + clear   ?
    GetControlador().StopTick(m_tickTempoEsgotado);
    GetControlador().StopTick(m_tickLeitura);

    std::vector<int> scores(4);
    auto& comparador = api::CPolySingletonList::instance<api::IFingerMatcher>();                  // :262
    auto& reconhecimento = CControladorReconhecimentoMesario::GetInst();                          // func 1149
    std::size_t n = 0;
    for (const md::CDedo::TipoDedo dedo : DEDOS) {
        const CEleitorDetalhe& mesario = *GetControlador().GetEleitorMesario();                   // slot 14
        if (!mesario.GetBiometria().TemDedo(dedo))                                                // funcs 1937, 3719
            continue;
        const std::vector<uebyte> modelo = mesario.GetBiometria().GetDedo(dedo).GetMinucias();    // CBiometriaEleitor::GetDedo
        if (comparador.Compara(amostra, modelo, reconhecimento.m_limiarScore)) {                  // slot 2
            GetControlador().LogaConferenciaBiometria();                                          // slot 34
            reconhecimento.m_dedo = dedo;
            return true;
        }
        scores[n++] = comparador.GetScore();                                                      // slot 4
    }
    GetControlador().LogaDigitalNaoCorresponde(scores);                                           // slot 30
    return false;
}

// wasm func 5382 (tools: "CPedeDigitalMesario::GetControlador", after the inlined srcloc :304).
// Stops the ticks and stores the captured image: CControladorReconhecimentoMesario::
// SalvarBiometriaMesarioRegistrado(digital, título) is inlined here.                    name inferred
void CPedeDigitalMesario::SalvaDigitalMesario(const api::TSharedImageVector& digital)
{
    GetControlador().StopTick(m_tickTempoEsgotado);                               // slot 3
    GetControlador().StopTick(m_tickLeitura);
    const std::string titulo = GetControlador().GetTituloMesario();               // slot 17
    auto& reconhecimento = CControladorReconhecimentoMesario::GetInst();          // func 1149

    // -- inlined SalvarBiometriaMesarioRegistrado(*digital, titulo) --
    // A descriptive name is computed and then never used (same pattern as u10 §6.3):
    const std::string nomeDescritivo = api::CStringUtils::PadLeft(titulo, '0', 12) + "_Operador";   // func 753
    (void)nomeDescritivo;
    // Encrypt + store the image under <trab>/wsq/operador/ in both flashes, prefix "me" (random id).
    const std::uint32_t id = CControlaArmazenamentoDeImagens::Armazena(
        *digital, DiretorioWsq(EFlashOrigem::INTERNA, "operador/"),               // vota_f3795
        DiretorioWsq(EFlashOrigem::EXTERNA, "operador/"), "me");                  // comum_f3794, func 2725
    if (id != 999999) {                                                           // 999999 = failure
        const std::function<unsigned()> geraId = [&id] { return id; };            // lambda $_0 (@1595836), captures &id
        const std::string subdiretorio = "registrado/";                           // vota_f5815
        const std::filesystem::path diretorio =
            DiretorioWsqTrab(EFlashOrigem::EXTERNA, CAppInfo::GetInst().GetTurno()) / "registrado/";   // func 2864
        reconhecimento.GravaWsq(*digital, subdiretorio, diretorio, geraId);       // func 5371(this, img, "registrado/",
                                                                                  // dir, fn): "me{:06}.wsq", "w+b",
                                                                                  // sets m_idArquivo = id
    }
    // -- end of inlined code --
    // Executed on every path (also when Armazena failed with 999999, in which case m_idArquivo keeps
    // the value of an EARLIER mesário - it is never cleared).
    reconhecimento.m_dedo = md::CDedo::TipoDedo{0};
}

// wasm func 10316 - vtable slot 8 (srcloc :141, :142, :143, :182, :262, :304)
void CPedeDigitalMesario::ProcessTick(uebyte tick)
{
    if (tick == m_tickTempoEsgotado) {                                            // 15 s without a finger
        GetControlador().StopTick(m_tickTempoEsgotado);
        GetControlador().StopTick(m_tickLeitura);
        m_proximoEstado = &CDigitalMesarioNaoReconhecida::GetInst();              // ctor inlined here
        return;
    }
    if (tick != m_tickLeitura)
        return;

    auto& deteccao = api::CPolySingletonList::instance<api::IFingerDetection>();  // :141
    auto& leitor   = api::CPolySingletonList::instance<api::IFingerScanner>();    // :142
    auto& urna     = api::CPolySingletonList::instance<api::IUrna>();             // :143

    // Grab up to 3 frames; the 2009/2010/2020/2022 models use the first complete frame, the others
    // need 3 frames with grey-level entropy > 4 bits and keep the best one. Any incomplete or "empty"
    // frame ends this tick (the next 50 ms tick tries again).
    api::TSharedImageVector escolhida;
    float melhorEntropia = 0.0f;
    for (int quadros = 0; quadros < 3; ++quadros) {
        api::TSharedImageVector quadro = leitor.CapturaQuadro();                  // slot 4
        if (quadro->size() != leitor.GetTamanhoQuadro())                          // slot 0
            return;
        const int modelo = urna.GetModelo();                                      // IUrna slot 0
        if (modelo == 2009 || modelo == 2010 || modelo == 2020 || modelo == 2022) {
            escolhida = quadro;
            break;
        }
        const float entropia = Entropia(*quadro);
        if (entropia <= 4.0f)
            return;
        if (entropia > melhorEntropia) {
            escolhida = quadro;
            melhorEntropia = entropia;
        }
    }
    if (!deteccao.DedoPresente(*escolhida))                                       // slot 2
        return;

    m_formAguarde->Show();
    leitor.SetLed(ELedLeitor::RECONHECIDO);                                       // :182, slot 7
    if (g_aguardaAposCaptura)                                                     // byte @1584624 == 1
        api::CWait::Sleep(1000);                                                  // -> emscripten_sleep(1000): ABORTS in the web build
    // Width / height (scanner slots 2 / 1) are re-read at every use, as in u10's CPedeDigital.
    if (urna.GetModelo() <= 2019)
        api::CBmpConversor::InvertColors(*escolhida, leitor.GetLargura(), leitor.GetAltura());
    api::CBmpConversor::VerticalFlip(*escolhida, leitor.GetLargura(), leitor.GetAltura());
    // The shared image handed to the comparison and to SalvaDigitalMesario is a copy of the WSQ
    // ENCODER'S OUTPUT, not of the raw frame (same as vota::CPedeDigital, u10). The encoder is a stub
    // here: func 2742 (singleton), the scanner width/height calls, then an empty vector (func 1379) -
    // so `digital` is always empty in this build. An empty std::string temporary is built around the
    // call (probably a further argument of the encoder, dropped with the stub body).            ?
    const auto digital = std::make_shared<std::vector<uebyte>>(
        CodificaWsq(*escolhida, leitor.GetLargura(), leitor.GetAltura(), ""));    // __shared_ptr_emplace @1529416

    auto& reconhecimento = CControladorReconhecimentoMesario::GetInst();          // func 1149
    if (GetControlador().MesarioEhEleitorDaSecao()) {                             // slot 13
        if (GetControlador().GetEleitorMesario()->TemBiometria()) {               // slot 14, CEleitorDetalhe +100
            reconhecimento.m_estado = md::CComparecimentoMesario::EstadoReconhecimentoBiometrico::CONFERIDA_CADASTRO;
            if (!ExecutaBatimentoDigitalMesario(digital, leitor.GetLargura(), leitor.GetAltura()))
                SalvaDigitalMesario(digital);                                     // func 5382
        } else {
            reconhecimento.m_estado = md::CComparecimentoMesario::EstadoReconhecimentoBiometrico::COLETADA;
            SalvaDigitalMesario(digital);
        }
        GetControlador().LogaMesarioEhEleitor();                                  // slot 32
    } else {
        reconhecimento.m_estado = md::CComparecimentoMesario::EstadoReconhecimentoBiometrico::COLETADA;
        SalvaDigitalMesario(digital);
        GetControlador().LogaMesarioNaoEhEleitor();                               // slot 33
    }
    m_proximoEstado = &CGestorDadoMesario::GetInst();                             // func 5383
}

} // namespace comum
