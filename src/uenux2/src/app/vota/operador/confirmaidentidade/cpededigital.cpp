// Reconstructed from vota_web_wasm.wasm (unit u10).
// Original: uenux2/src/app/vota/operador/confirmaidentidade/cpededigital.cpp
// (attested by std::source_location records: lines 115, 120, 125, 131, 132, 162-164, 234, 262, 279).
//
// Operator-terminal (MT) screen vocabulary used below: the MT LCD has 4 lines x 40 columns;
// api::SPoint{coluna, linha} is 1-based; the CFixedText/CDataText alignment is 0 left, 1 right
// (x = right edge, usually 40), 2 centred (x = centre, usually 20).
// Form helpers (template instantiations of the MT form builder, see the u10 doc):
//   func 435  Add<CLedFieldMT>(bool)         func 941  Add<CBuzzFieldMT>(a, b)   func 1072 Add<CBeepFieldMT>(n)
//   func 180  Add<CTextFieldMT>(pos, CFixedText(align, texto))
//   func 619  Add<CTextFieldMT>(pos, CDataText<std::string(*)()>(align, fonte))
//   func 651  Add<CTextFieldMT>(pos, CDataTextFmt<std::string(*)(const std::string&)>(align, fonte, fmt))
//   func 1152 Add<CTextFieldMT>(pos, CDataTextFmt<CTextSource>(align, shared_ptr<string>, fmt))
//   func 395  Add<CInputFieldControl<IScreenMT>>(CControlValidation)
//   func 301  make_shared<CInteractiveForm<IScreenMT, IInputMT>>(campos, CPreShowClearMT, nome, flag)
//
// Web build: dead code (operator thread not run; no IFingerScanner/IFingerMatcher/IFingerDetection is
// registered, only simulador::CFingerPrepareSimulador for IFingerPrepare). Two things would abort the
// simulator if it ever got here: emscripten_sleep in ObtemEstadoPosReconhecimentoBiometrico, and the
// PolySingleton lookups of the fingerprint interfaces ("solicitada uma instancia nao criada").
#include "vota/operador/confirmaidentidade/cpededigital.h"

#include <cmath>
#include <format>
#include <map>

#include "api/gui/cbmpconversor.h"
#include "api/hwil/ifingerdetection.h"
#include "api/hwil/ifingermatcher.h"
#include "api/hwil/iurna.h"
#include "api/util/cwait.h"
#include "comum/cconfiguracaoeleicao.h"
#include "comum/dados/celeitores.h"
#include "vota/log/clogvota.h"
#include "vota/operador/cthreadoperador.h"
#include "vota/operador/comum/ccancelahabilitacaoeleitor.h"
#include "vota/operador/confirmaidentidade/ccontrolareconhecimento.h"
#include "vota/operador/confirmaidentidade/cdigitalnaoreconhecida.h"
#include "vota/operador/confirmaidentidade/cdigitalnaoreconhecidaportempo.h"
#include "vota/operador/confirmaidentidade/cdigitalreconhecida.h"
#include "vota/comum/votadefs.h"

namespace vota {

using api::SPoint;
using TFormMT = api::CInteractiveForm<api::IScreenMT, api::IInputMT>;

namespace {

constexpr api::ETextAlignment ESQUERDA = api::ETextAlignment(0);
constexpr api::ETextAlignment CENTRO   = api::ETextAlignment(2);

// IFingerScanner LED modes (slot 7). Values from the calls; colours are a guess.        ?
enum class ELedLeitor : int { APAGADO = 0, RECONHECIDO = 1, CAPTURANDO = 2, NAO_RECONHECIDO = 3 };

// Shannon entropy (bits) of the grey-level histogram of a frame; a frame with <= 4 bits has no finger.
float Entropia(const std::vector<uebyte>& imagem)                          // inlined, name inferred
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

// Stateless helpers whose calls were compiled away in this build (their bodies are empty stubs:
// the WSQ encoder (singleton of a 1-byte class @1908636, func 2742) and the minutiae extractor
// (16-byte parameter singleton @1908664 = {8, 500, ...}, func 1903)). Both return an empty vector.
// Names inferred.                                                                           ?
std::vector<uebyte> CodificaWsq(const std::vector<uebyte>& bruta, long largura, long altura);
std::vector<uebyte> ExtraiTemplate(const std::vector<uebyte>& wsq, long largura, long altura);

}  // namespace

// ---------------------------------------------------------------------------------------------
// Constructor (inlined into GetInst, wasm func 2740; lazy singleton @1909064, 84 bytes).
CPedeDigital::CPedeDigital()
    : comum::CAppState(6)                                                     // keys + ticks
    , m_tickPrimeiraTentativa(CThreadOperador::GetInst().CriaTick(30000))
    , m_tickDemaisTentativas(CThreadOperador::GetInst().CriaTick(15000))
    , m_tickLeitura(CThreadOperador::GetInst().CriaTick(50))
    , m_contexto(2, "Captura da digital do eleitor", "Falha na captura da digital",
                 "Ocorreu um erro durante captura da digital do eleitor.")   // func 3682
{
    api::CFormBuilderMT captura;
    captura.Add<api::CLedFieldMT>(false);
    captura.Add<api::CBuzzFieldMT>(52, 5);
    captura.Add<api::CTextFieldMT>(SPoint{20, 1}, std::make_shared<api::CFixedText>(CENTRO, "Solicite que o(a) eleitor(a)"));
    captura.Add<api::CTextFieldMT>(SPoint{20, 2}, std::make_shared<api::CDataTextFmt<std::string (*)(const std::string&)>>(
                                                      CENTRO, &comum::CEleitorDadoNomeParaUrna::Text, "{:2}"));   // slot 3906
    captura.Add<api::CTextFieldMT>(SPoint{1, 3}, std::make_shared<api::CFixedText>(ESQUERDA, "posicione POLEGAR ou INDICADOR no sensor"));
    captura.Add<api::CTextFieldMT>(SPoint{1, 4}, std::make_shared<api::CFixedText>(ESQUERDA, "CORRIGE: cancelar"));
    captura.AddInputControl();                                               // func 395
    m_formCaptura = captura.CriaFormInterativo("", true);                    // func 301

    api::CFormBuilderMT aguarde;
    aguarde.Add<api::CLedFieldMT>(false);
    aguarde.Add<api::CTextFieldMT>(SPoint{1, 2}, std::make_shared<api::CFixedText>(CENTRO, "Por favor, aguarde."));   // x = 1, centred
    m_formAguarde = aguarde.CriaFormInterativo("", true);
}

// wasm func 2740 (tools: "vota_f2740"; curated note "lazily constructs vota::CPedeDigital")
CPedeDigital& CPedeDigital::GetInst()
{
    static std::mutex mutex;                                  // @1909040 (only the unlock stub remains)
    static std::unique_ptr<CPedeDigital> s_inst;              // @1909064; reset at exit by func 10472
    std::lock_guard lock(mutex);
    if (!s_inst)
        s_inst.reset(new CPedeDigital());
    return *s_inst;
}

// wasm func 2739 (slot 0) / 10469 (slot 1, deleting): m_contexto (func 1468) and the two forms.
CPedeDigital::~CPedeDigital() = default;

void CPedeDigital::ParaTicks()
{
    CThreadOperador& operador = CThreadOperador::GetInst();
    operador.StopTick(m_tickPrimeiraTentativa);
    operador.StopTick(m_tickDemaisTentativas);
    operador.StopTick(m_tickLeitura);
}

// ---------------------------------------------------------------------------------------------
// wasm func 10468 - vtable slot 2 (srcloc lines 115, 120, 125)
void CPedeDigital::StartState()
{
    api::CApplicationContextStack::Push(api::CApplicationContext(m_contexto));      // funcs 1841, 3684
    CLogVota::GetInst().Loga(std::format("Solicita digital. Tentativa [{}] de [{}]",
                                         CControlaReconhecimento::GetTentativa(),
                                         comum::CConfiguracaoEleicao::GetInst().GetNumTentativasHabilitacao()));
    if (!comum::CConfiguracaoEleicao::GetInst().GetUtilizaBiometria())             // cfg +665
        throw CUeVotaError(EUeVotaError{9400},
                           "Biometria desativada na configuração da eleição, fluxo indevido.");   // line 115
    m_formCaptura->Show();
    api::CPolySingletonList::instance<api::IFingerScanner>().SetLed(ELedLeitor::CAPTURANDO);  // line 120, slot 7
    CThreadOperador& operador = CThreadOperador::GetInst();
    operador.StartTick(m_tickPrimeiraTentativa);
    operador.StartTick(m_tickDemaisTentativas);
    operador.StartTick(m_tickLeitura);
    api::CPolySingletonList::instance<api::IFingerScanner>().IniciaCaptura();      // line 125, slot 3
    m_proximoEstado = this;
}

// ---------------------------------------------------------------------------------------------
// wasm func 10467 - vtable slot 5 (srcloc lines 131, 132)
void CPedeDigital::FinishState()
{
    api::CPolySingletonList::instance<api::IFingerScanner>().SetLed(ELedLeitor::APAGADO);   // line 131
    api::CPolySingletonList::instance<api::IFingerScanner>().FinalizaCaptura();            // line 132, slot 6
    api::CApplicationContextStack::Remove(m_contexto);                                     // func 5553
}

// ---------------------------------------------------------------------------------------------
// wasm func 10466 - vtable slot 7                                             name from slot order
void CPedeDigital::ProcessInput()
{
    if (m_formCaptura->Read() == api::EInputResult::CORRIGE) {
        CLogVota::GetInst().Loga(2, "Habilitação cancelada durante reconhecimento biométrico");   // func 2495
        m_proximoEstado = &CCancelaHabilitacaoEleitor::GetInst();                                 // func 1536
    }
}

// ---------------------------------------------------------------------------------------------
// wasm func 10465 - vtable slot 8 (srcloc lines 162, 163, 164; plus inlined VerificaDigital :262,
// VerificaDigitalTreinamento :279 and CControlaReconhecimento::AvancaProximoDedo1x4 :187)
void CPedeDigital::ProcessTick(uebyte tick)
{
    if (tick != m_tickPrimeiraTentativa && tick != m_tickDemaisTentativas && tick != m_tickLeitura)
        return;

    // Timeout: 30 s for the first attempt, 15 s for the following ones.
    const bool primeira = CControlaReconhecimento::EhPrimeiraTentativa();   // func 5403
    if ((primeira && tick == m_tickPrimeiraTentativa) || (!primeira && tick == m_tickDemaisTentativas)) {
        ParaTicks();
        m_proximoEstado = &CDigitalNaoReconhecidaPorTempo::GetInst();       // GetInst + ctor inlined (@1908952)
        return;
    }
    if (tick != m_tickLeitura)
        return;

    api::IFingerDetection& detector = api::CPolySingletonList::instance<api::IFingerDetection>();   // line 162
    api::IFingerScanner&   leitor   = api::CPolySingletonList::instance<api::IFingerScanner>();     // line 163
    api::IUrna&            urna     = api::CPolySingletonList::instance<api::IUrna>();              // line 164

    // Frame selection. Urnas 2009/2010/2020/2022: the first complete frame is used. Other models:
    // three consecutive frames must each have entropy > 4 bits; the one with the highest entropy wins.
    api::TSharedImageVector melhor;
    float melhorEntropia = 0.0f;
    for (int quadros = 0; quadros <= 2;) {
        api::TSharedImageVector imagem = leitor.CapturaImagem();                    // slot 4
        if (imagem->size() != leitor.GetTamanhoImagem())                            // slot 0
            return;                                                                 // no frame yet
        // IUrna slot 0, called again for each comparison (up to 4 virtual calls in the binary)
        if (urna.GetModelo() == 2009 || urna.GetModelo() == 2010 ||
            urna.GetModelo() == 2020 || urna.GetModelo() == 2022) {
            melhor = imagem;
            break;
        }
        const float entropia = Entropia(*imagem);
        if (!(entropia > 4.0f))
            return;                                                                 // empty sensor
        if (entropia > melhorEntropia) {
            melhor = imagem;
            melhorEntropia = entropia;
        }
        ++quadros;
    }
    if (!detector.DedoPresente(*melhor))                                           // slot 2
        return;

    m_formAguarde->Show();
    CLogVota::GetInst().Loga(std::format("Capturada a digital. Tentativa [{}] de [{}]",
                                         CControlaReconhecimento::GetTentativa(),
                                         comum::CConfiguracaoEleicao::GetInst().GetNumTentativasHabilitacao()));

    std::pair<bool, comum::TScoreHabilitacao> resultado;
    if (EhTreinamentoSemTreinamentoEleitor()) {                                    // func 1823
        resultado = VerificaDigitalTreinamento();
    } else {
        // Image preparation uses the IUrna / IFingerScanner references of lines 163/164: the binary
        // has no other IUrna lookup in this function, so the model test cannot sit inside
        // VerificaDigital (every CPolySingletonList::instance call carries its own srcloc record).
        // Width / height (scanner slots 2 / 1) are re-read at every use.
        if (urna.GetModelo() <= 2019)
            api::CBmpConversor::InvertColors(melhor->data(), leitor.GetLargura(), leitor.GetAltura());
        api::CBmpConversor::VerticalFlip(melhor->data(), leitor.GetLargura(), leitor.GetAltura());

        // Keep the capture (WSQ) for CMostraEleitorVotando::SalvaHabilitacaoEleitor. The make_shared copy
        // is the TSharedImageVector handed to VerificaDigital.
        const api::TSharedImageVector wsq = std::make_shared<std::vector<uebyte>>(
            CodificaWsq(*melhor, leitor.GetLargura(), leitor.GetAltura()));      // func 2742 + empty result
        CControlaReconhecimento::s_digitalCapturada = *wsq;                          // @1908672
        CControlaReconhecimento::s_tentativaDigitalSalva = CControlaReconhecimento::s_tentativa;
        resultado = VerificaDigital(wsq, leitor.GetLargura(), leitor.GetAltura());  // slots 2 / 1
    }
    ObtemEstadoPosReconhecimentoBiometrico(resultado.first);
    CControlaReconhecimento::s_score = resultado.second;                           // @1908708
}

// srcloc line 262 - inlined into ProcessTick. (Where exactly the WSQ step sits - here or in the caller -
// is not visible after inlining; the IUrna model test is certainly in the caller, see above.)
std::pair<bool, comum::TScoreHabilitacao>
CPedeDigital::VerificaDigital(const api::TSharedImageVector& imagem, const long largura, const long altura)
{
    const std::vector<uebyte> amostra = ExtraiTemplate(*imagem, largura, altura);   // func 1903 + clear()
    ParaTicks();

    // 1x4: compare with the stored template of each finger in turn, from the current index on.
    while (CControlaReconhecimento::s_indiceDedo != 4) {
        const int dedo = CControlaReconhecimento::DEDOS_1X4[CControlaReconhecimento::s_indiceDedo];
        const comum::md::CBiometriaEleitor bio = comum::CEleitores::GetInst().GetCurrent().GetBiometria();   // func 1937
        const std::vector<uebyte> modelo =
            bio.PossuiDedo(dedo) ? bio.GetDedo(dedo).GetTemplate() : std::vector<uebyte>{};   // funcs 3719, 3720
        api::IFingerMatcher& comparador = api::CPolySingletonList::instance<api::IFingerMatcher>();   // line 262
        if (comparador.Compara(amostra, modelo, 20))                                // slot 2
            return {true, comparador.GetScore()};                                  // slot 4
        CControlaReconhecimento::AvancaProximoDedo1x4();                            // ccontrolareconhecimento.cpp:187
    }
    return {false, 0};
}

// srcloc line 279 - inlined into ProcessTick. Training of mesários: whether the fingerprint is
// "recognised" depends only on the voter's sequential number and on the attempt number:
//   seq in (0, N/6]      -> recognised at attempt 1      seq in (N/6, 2N/6]  -> at attempt 2
//   seq in (2N/6, 3N/6]  -> at attempt 3                 seq in (3N/6, 4N/6] -> at attempt 4
//   seq in (4N/6, 5N/6]  -> never                        seq in (5N/6, N]    -> "no biometrics"
//                                                          (CControlaReconhecimento / CNomeEleitor)
std::pair<bool, comum::TScoreHabilitacao> CPedeDigital::VerificaDigitalTreinamento()
{
    ParaTicks();
    api::IFingerMatcher& comparador = api::CPolySingletonList::instance<api::IFingerMatcher>();   // line 279
    const unsigned seq = static_cast<std::uint16_t>(
        comum::CEleitores::GetInst().GetCurrent().GetEleitor().GetSequencial());
    const int tentativa = CControlaReconhecimento::s_tentativa;
    const int* faixa = CControlaReconhecimento::s_faixaTentativa;

    bool reconhecido;
    unsigned limite = faixa[0];
    if (seq <= limite && tentativa == 1) {
        reconhecido = true;
    } else if (seq > limite && seq <= (limite += faixa[1]) && tentativa == 2) {
        reconhecido = true;
    } else if (seq > limite && seq <= (limite += faixa[2]) && tentativa == 3) {
        reconhecido = true;
    } else if (seq <= limite) {
        reconhecido = false;
    } else {
        reconhecido = tentativa == 4 && seq <= limite + faixa[3];
    }
    comparador.SimulaResultado(reconhecido, 20);                                   // slot 3
    return {reconhecido, comparador.GetScore()};                                   // slot 4
}

// ---------------------------------------------------------------------------------------------
// wasm func 5397 (srcloc line 234)
void CPedeDigital::ObtemEstadoPosReconhecimentoBiometrico(const bool reconhecido)
{
    api::IFingerScanner& leitor = api::CPolySingletonList::instance<api::IFingerScanner>();   // line 234
    if (reconhecido) {
        leitor.SetLed(ELedLeitor::RECONHECIDO);
        m_proximoEstado = &CDigitalReconhecida::GetInst();       // GetInst + ctor inlined (@1908980)
    } else {
        leitor.SetLed(ELedLeitor::NAO_RECONHECIDO);
        m_proximoEstado = &CDigitalNaoReconhecida::GetInst();    // GetInst + ctor inlined (@1909036)
    }
    api::CWait::Sleep(1000);   // compiled as: if (byte @1584624 == 1) emscripten_sleep(1000) - aborts in this build
}

}  // namespace vota
