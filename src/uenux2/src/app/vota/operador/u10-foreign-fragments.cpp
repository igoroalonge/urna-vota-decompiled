// FRAGMENTS reconstructed by unit u10 from vota_web_wasm.wasm.
//
// Functions (and inlined constructors) that the analysis tools attributed to u10's files
// (uenux2/src/app/vota/operador/...) but whose real home is another file: singleton accessors of
// neighbouring operator states, one state method, form-builder template instantiations, small data
// constructors and thunks. Each block says where it belongs. Merge them into those files.
//
// MT form vocabulary: see the header comment of confirmaidentidade/cpededigital.cpp.
#include <chrono>
#include <ctime>
#include <memory>
#include <mutex>
#include <string>

#include "api/gui/cinteractiveform.h"
#include "api/util/cdatetime.h"
#include "comum/dados/celeitores.h"
#include "comum/dados/md/eleitor/celeitordadoshabilitacao.h"
#include "vota/eleitor/comum/cinformacaoeleitor.h"
#include "vota/log/clogvota.h"
#include "vota/operador/cthreadoperador.h"
#include "vota/operador/comum/ccancelahabilitacaoeleitor.h"
#include "vota/operador/comum/cinformacaothreadoperador.h"
#include "vota/operador/confirmaidentidade/ccontrolareconhecimento.h"
#include "vota/operador/confirmaidentidade/cpededigital.h"

namespace vota {

using api::SPoint;
namespace {
const auto ESQUERDA = api::ETextAlignment(0);
const auto DIREITA  = api::ETextAlignment(1);
const auto CENTRO   = api::ETextAlignment(2);

// Recurring MT fields (name, typed identity, sequential number) - written out at each use in the binary.
void AddNomeEleitor(api::CFormBuilderMT& campos, SPoint pos, api::ETextAlignment alinhamento)
{
    campos.Add<api::CTextFieldMT>(pos, std::make_shared<api::CDataTextFmt<std::string (*)(const std::string&)>>(
                                           alinhamento, &comum::CEleitorDadoNomeParaUrna::Text, "{:2}"));   // func 651, slot 3906
}
void AddIdentidadeESequencial(api::CFormBuilderMT& campos)
{
    campos.Add<api::CTextFieldMT>(SPoint{1, 2}, std::make_shared<api::CDataText<std::string (*)()>>(
                                                    ESQUERDA, &TextoIdentidadeDigitada));                     // func 619, slot 3907
    campos.Add<api::CTextFieldMT>(SPoint{32, 2}, std::make_shared<api::CFixedText>(ESQUERDA, "Seq:"));
    campos.Add<api::CTextFieldMT>(SPoint{40, 2}, std::make_shared<api::CDataTextFmt<std::string (*)(const std::string&)>>(
                                                     DIREITA, &comum::CEleitorDadoSequencial::Text, "{:04}")); // slot 4055
}
}  // namespace

// =============================================================================================
// uenux2/src/app/vota/operador/aguardaeleitor/celeitorvotounaovotou.cpp  (path inferred)
// "Eleitor votou / não votou": end-of-voter screen on the MT. Methods: StartState 10431 (u23),
// ProcessInput 10430 (u17).
// ---------------------------------------------------------------------------------------------
// wasm func 1901 (tools: "vota_f1901", attributed to cmostraeleitorvotando.cpp)      name inferred
CEleitorVotouNaoVotou& CEleitorVotouNaoVotou::GetInst()
{
    static std::mutex mutex;                                              // @1909264
    static std::unique_ptr<CEleitorVotouNaoVotou> s_inst;                 // @1909288 (36 bytes)
    std::lock_guard lock(mutex);
    if (!s_inst)
        s_inst.reset(new CEleitorVotouNaoVotou());
    return *s_inst;
}

CEleitorVotouNaoVotou::CEleitorVotouNaoVotou()                            // inlined into 1901
    : comum::CAppState(2)                                                 // keys
    , m_votou(false)                                                      // +11
    , m_textoSituacao(std::make_shared<std::string>("pode/nao pode entregar comp"))   // +12 placeholder
    , m_textoComplemento(std::make_shared<std::string>())                 // +20
{
    api::CFormBuilderMT campos;
    campos.Add<api::CLedFieldMT>(false);
    campos.Add<api::CTextFieldMT>(SPoint{1, 1}, std::make_shared<api::CDataText<std::string (*)()>>(
                                                    ESQUERDA, &TextoIdentidadeDigitada));
    campos.Add<api::CTextFieldMT>(SPoint{1, 2}, std::make_shared<api::CDataTextFmt<api::CTextSource>>(
                                                    ESQUERDA, api::CTextSource(m_textoSituacao), "%s"));
    campos.Add<api::CTextFieldMT>(SPoint{1, 3}, std::make_shared<api::CDataTextFmt<api::CTextSource>>(
                                                    ESQUERDA, api::CTextSource(m_textoComplemento), "%s"));
    campos.Add<api::CTextFieldMT>(SPoint{40, 4}, std::make_shared<api::CFixedText>(DIREITA, "CONFIRMA: prosseguir"));
    campos.AddInputControl();
    m_form = campos.CriaFormInterativo("", true);                          // +28
}

// =============================================================================================
// uenux2/src/app/vota/operador/aguardaeleitor/celeitordemorando.cpp  (path inferred)
// "O eleitor está demorando": inactivity alert. Methods: StartState 10440, ProcessInput 10439 (u17).
// ---------------------------------------------------------------------------------------------
// wasm func 2734 (tools: "vota_f2734", attributed to cmostraeleitorvotando.cpp)      name inferred
CEleitorDemorando& CEleitorDemorando::GetInst()
{
    static std::mutex mutex;                                              // @1909208
    static std::unique_ptr<CEleitorDemorando> s_inst;                     // @1909232 (36 bytes)
    std::lock_guard lock(mutex);
    if (!s_inst)
        s_inst.reset(new CEleitorDemorando());
    return *s_inst;
}

CEleitorDemorando::CEleitorDemorando()                                    // inlined into 2734
    : comum::CAppState(3)                                                 // messages + keys
    , m_votouParcialmente(false)                                          // +11
    , m_textoSituacao(std::make_shared<std::string>("votou/não votou"))   // +12 placeholder
    , m_textoNome(std::make_shared<std::string>("nome abreviado"))        // +20 placeholder
{
    api::CFormBuilderMT campos;
    campos.Add<api::CLedFieldMT>(true);                                   // LED lit: attention
    campos.Add<api::CBuzzFieldMT>(51, 10);
    campos.Add<api::CTextFieldMT>(SPoint{1, 1}, std::make_shared<api::CFixedText>(ESQUERDA, "O eleitor está demorando"));
    campos.Add<api::CTextFieldMT>(SPoint{1, 2}, std::make_shared<api::CDataTextFmt<api::CTextSource>>(
                                                    ESQUERDA, api::CTextSource(m_textoSituacao), "%s"));   // func 1152
    campos.Add<api::CTextFieldMT>(SPoint{1, 3}, std::make_shared<api::CDataText<api::CTextSource>>(
                                                    ESQUERDA, api::CTextSource(m_textoNome)));             // func 5409
    campos.Add<api::CTextFieldMT>(SPoint{40, 4}, std::make_shared<api::CFixedText>(DIREITA, "CONFIRMA"));
    campos.AddInputControl();
    m_form = campos.CriaFormInterativo("", true);                          // +28
}

// =============================================================================================
// uenux2/src/app/vota/operador/confirmaidentidade/cverificadadoeleitor.cpp  (path inferred)
// Birth-year check after the last failed fingerprint attempt. ProcessInput = 10443 (u17): compares the
// typed year with the roll ("Dado digitado confere / não confere"), limited by
// ParametrosUrna.numTentativasVerificacao (CControlaReconhecimento::s_verificacoesDadoEleitor).
// ---------------------------------------------------------------------------------------------
// wasm func 2735 (tools: "api_f2735", attributed to cdigitalnaoreconhecida.cpp)       name inferred
CVerificaDadoEleitor& CVerificaDadoEleitor::GetInst()
{
    static std::mutex mutex;                                              // @1909180
    static std::unique_ptr<CVerificaDadoEleitor> s_inst;                  // @1909204 (20 bytes)
    std::lock_guard lock(mutex);
    if (!s_inst)
        s_inst.reset(new CVerificaDadoEleitor());
    return *s_inst;
}

CVerificaDadoEleitor::CVerificaDadoEleitor()                              // inlined into 2735
    : comum::CAppState(2)
{
    api::CFormBuilderMT campos;
    AddNomeEleitor(campos, SPoint{1, 1}, ESQUERDA);
    AddIdentidadeESequencial(campos);
    campos.Add<api::CTextFieldMT>(SPoint{1, 3}, std::make_shared<api::CFixedText>(ESQUERDA, "Digite o ANO de nascimento:"));
    campos.AddNumberInput(4, 1, SPoint{29, 3});                            // func 1151: 4-digit CNumberValidation field
    campos.Add<api::CTextFieldMT>(SPoint{1, 4}, std::make_shared<api::CFixedText>(ESQUERDA, "CORRIGE: cancelar"));
    campos.Add<api::CTextFieldMT>(SPoint{40, 4}, std::make_shared<api::CFixedText>(DIREITA, "CONFIRMA: habilitar"));
    m_form = campos.CriaFormInterativo("", true);                          // +12
}

// =============================================================================================
// uenux2/src/app/vota/operador/confirmaidentidade/cinformabiodesabilitadademo.cpp  (path inferred)
// Demonstration mode (IInterfaceInit::GetDemoMode): the biometric flow is replaced by this notice.
// ---------------------------------------------------------------------------------------------
// wasm func 5402 (tools: "vota_f5402", attributed to cnomeeleitor.cpp)                name inferred
CInformaBioDesabilitadaDemo& CInformaBioDesabilitadaDemo::GetInst()
{
    static std::mutex mutex;                                              // @1908760
    static std::unique_ptr<CInformaBioDesabilitadaDemo> s_inst;           // @1908784 (20 bytes)
    std::lock_guard lock(mutex);
    if (!s_inst)
        s_inst.reset(new CInformaBioDesabilitadaDemo());
    return *s_inst;
}

CInformaBioDesabilitadaDemo::CInformaBioDesabilitadaDemo()
    : comum::CAppState(2)
{
    api::CFormBuilderMT campos;
    campos.Add<api::CLedFieldMT>(false);
    campos.Add<api::CTextFieldMT>(SPoint{1, 1}, std::make_shared<api::CFixedText>(ESQUERDA, "Biometria desabilitada em demonstração"));
    campos.Add<api::CTextFieldMT>(SPoint{1, 4}, std::make_shared<api::CFixedText>(ESQUERDA, "CONFIRMA: prosseguir"));
    campos.AddInputControl();
    m_form = campos.CriaFormInterativo("telaInformaBioDesabilitadaDemo", false);
}

// =============================================================================================
// uenux2/src/app/vota/operador/confirmaidentidade/cinformaanodesabilitadodemo.cpp  (path inferred)
// GetInst + constructor exist only inlined into CNomeEleitor::ProcessInput (func 10500), @1908812.
CInformaAnoDesabilitadoDemo::CInformaAnoDesabilitadoDemo()
    : comum::CAppState(2)
{
    api::CFormBuilderMT campos;
    campos.Add<api::CLedFieldMT>(false);
    campos.Add<api::CTextFieldMT>(SPoint{1, 1}, std::make_shared<api::CFixedText>(ESQUERDA, "A validação do ano de nascimento"));
    campos.Add<api::CTextFieldMT>(SPoint{1, 2}, std::make_shared<api::CFixedText>(ESQUERDA, "é desabilitada em demonstração"));
    campos.Add<api::CTextFieldMT>(SPoint{1, 4}, std::make_shared<api::CFixedText>(ESQUERDA, "CONFIRMA: prosseguir"));
    campos.AddInputControl();
    m_form = campos.CriaFormInterativo("telaInformaAnoDesabilitadoDemo", false);
}

// =============================================================================================
// uenux2/src/app/vota/operador/confirmaidentidade/cpedeanonascimentosembiometria.cpp  (path inferred)
// GetInst + constructor exist only inlined into CNomeEleitor::ProcessInput (func 10500), @1908924.
// ProcessInput = 10489 (u27): "Ano de nascimento digitado é igual/diferente do cadastro".
CPedeAnoNascimentoSemBiometria::CPedeAnoNascimentoSemBiometria()
    : comum::CAppState(2)
    , m_tentativas(0)                                                     // +28
    , m_primeiraVez(true)                                                 // +32 ?
{
    api::CFormBuilderMT pede;
    AddNomeEleitor(pede, SPoint{1, 1}, ESQUERDA);
    AddIdentidadeESequencial(pede);
    pede.Add<api::CTextFieldMT>(SPoint{1, 3}, std::make_shared<api::CFixedText>(ESQUERDA, "Digite o ANO de nascimento:"));
    pede.AddNumberInput(4, 1, SPoint{29, 3});
    pede.Add<api::CTextFieldMT>(SPoint{1, 4}, std::make_shared<api::CFixedText>(ESQUERDA, "CORRIGE: cancelar"));
    pede.Add<api::CTextFieldMT>(SPoint{40, 4}, std::make_shared<api::CFixedText>(DIREITA, "CONFIRMA: habilitar"));
    m_formPedeAno = pede.CriaFormInterativo("", true);                     // +12

    api::CFormBuilderMT erro;
    AddNomeEleitor(erro, SPoint{1, 1}, ESQUERDA);
    AddIdentidadeESequencial(erro);
    erro.Add<api::CTextFieldMT>(SPoint{1, 3}, std::make_shared<api::CFixedText>(ESQUERDA, "ANO DE NASCIMENTO INCORRETO"));
    erro.Add<api::CTextFieldMT>(SPoint{40, 4}, std::make_shared<api::CFixedText>(DIREITA, "CONFIRMA: tentar novamente"));
    erro.AddInputControl();
    m_formAnoIncorreto = erro.CriaFormInterativo("", true);                // +20
}

// =============================================================================================
// uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaoreconhecidadecbiometria.cpp (path inferred)
// GetInst + constructor exist only inlined into CDigitalNaoReconhecida::StartState (func 10474), @1909008.
// StartState 10478 (u39) logs "Erro ao decifrar a biometria do eleitor - Código ({})"; ProcessInput 10477 (u19).
CDigitalNaoReconhecidaDecBiometria::CDigitalNaoReconhecidaDecBiometria()
    : comum::CAppState(2)
{
    api::CFormBuilderMT campos;
    campos.Add<api::CLedFieldMT>(false);
    campos.Add<api::CBeepFieldMT>(1);
    AddNomeEleitor(campos, SPoint{1, 1}, ESQUERDA);
    campos.Add<api::CTextFieldMT>(SPoint{1, 2}, std::make_shared<api::CFixedText>(CENTRO, "Eleitor(a) não reconhecido(a)"));
    campos.Add<api::CTextFieldMT>(SPoint{1, 3}, std::make_shared<api::CFixedText>(CENTRO, "Dados biométricos inválidos"));
    campos.Add<api::CTextFieldMT>(SPoint{1, 4}, std::make_shared<api::CFixedText>(ESQUERDA, "CORRIGE: cancelar"));
    campos.Add<api::CTextFieldMT>(SPoint{40, 4}, std::make_shared<api::CFixedText>(DIREITA, "CONFIRMA: prosseguir"));
    campos.AddInputControl();
    m_form = campos.CriaFormInterativo("", true);                          // +12
}

// =============================================================================================
// uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaoreconhecidaportempo.cpp  (path inferred)
// Timeout of a capture attempt (30 s first attempt, 15 s later ones). StartState = 10486 (u39):
// "Tentativa {} de {}" + log "Timeout de reconhecimento do dedo. Tentativa [{}] de [{}]".
// GetInst + constructor exist only inlined into CPedeDigital::ProcessTick (func 10465), @1908952.
CDigitalNaoReconhecidaPorTempo::CDigitalNaoReconhecidaPorTempo()
    : comum::CAppState(2)
    , m_textoTentativa(std::make_shared<std::string>("Tentativa x de x"))    // +12
{
    api::CFormBuilderMT campos;
    campos.Add<api::CLedFieldMT>(false);
    campos.Add<api::CBeepFieldMT>(1);
    AddNomeEleitor(campos, SPoint{1, 1}, ESQUERDA);
    campos.Add<api::CTextFieldMT>(SPoint{20, 2}, std::make_shared<api::CFixedText>(CENTRO, "Eleitor(a) não reconhecido(a)"));
    campos.Add<api::CTextFieldMT>(SPoint{20, 3}, std::make_shared<api::CDataTextFmt<api::CTextSource>>(
                                                     CENTRO, api::CTextSource(m_textoTentativa), "%s"));
    campos.Add<api::CTextFieldMT>(SPoint{1, 4}, std::make_shared<api::CFixedText>(ESQUERDA, "CORRIGE: cancelar"));
    campos.Add<api::CTextFieldMT>(SPoint{40, 4}, std::make_shared<api::CFixedText>(DIREITA, "CONFIRMA: retornar"));
    campos.AddInputControl();
    m_form = campos.CriaFormInterativo("", true);                          // +20
}

// wasm func 10485 - CDigitalNaoReconhecidaPorTempo vtable slot 7 (identical logic to
// CDigitalNaoReconhecida::ProcessInput, func 10473, but the form is at +20)          name from slot order
void CDigitalNaoReconhecidaPorTempo::ProcessInput()
{
    switch (m_form->Read()) {
    case api::EInputResult::CORRIGE:
        CLogVota::GetInst().Loga(2, "Habilitação cancelada durante reconhecimento biométrico");   // func 2495
        m_proximoEstado = &CCancelaHabilitacaoEleitor::GetInst();                                 // func 1536
        break;
    case api::EInputResult::CONFIRMA:
        if (CControlaReconhecimento::EhUltimaTentativa()) {                  // func 5405
            m_proximoEstado = &CVerificaDadoEleitor::GetInst();             // func 2735
        } else {
            CControlaReconhecimento::AvancaTentativa();                     // func 5406
            m_proximoEstado = &CPedeDigital::GetInst();                     // func 2740
        }
        break;
    default:
        break;
    }
}

// =============================================================================================
// Data constructors of the habilitação record (uenux2/src/app/comum/dados/..., unit u05's model)
// ---------------------------------------------------------------------------------------------
// wasm func 2733 (tools: "vota_f2733")                                              name inferred
// comum::CEleitorDadosHabilitacao (64 bytes): +0 identidade, +16 tipo, +20 tipoAtivacaoAudio,
// +24 dados biométricos (32 bytes, optional título do mesário engaged flag +52), +56 apresentação da foto.
comum::CEleitorDadosHabilitacao::CEleitorDadosHabilitacao(const md::CEleitorIdentidade& identidade,
                                                          md::ETipoHabilitacao tipo, int tipoAtivacaoAudio,
                                                          const md::CEleitorDadosHabilitacaoBiometrica& biometria,
                                                          const ecourna::app::dados::CApresentacaoFotoEleitor& foto)
    : m_identidade(identidade), m_tipo(tipo), m_tipoAtivacaoAudio(tipoAtivacaoAudio),
      m_biometria(biometria), m_apresentacaoFoto(foto)
{
}

// wasm func 3718 (tools: "vota_f3718")                                              name inferred
// md::CEleitorDadosHabilitacaoBiometrica (32 bytes): +0 dedo, +4 score (uint16), +6 tentativas (uint8),
// +8 erroDecifrarBiometria, +12 std::optional<CEleitorIdentidade> tituloMesario (flag +28, reset here).
comum::md::CEleitorDadosHabilitacaoBiometrica::CEleitorDadosHabilitacaoBiometrica(
    int dedo, TScoreHabilitacao score, uebyte tentativas, int erroDecifrar)
    : m_dedo(dedo), m_score(score), m_tentativas(tentativas), m_erroDecifrar(erroDecifrar), m_tituloMesario()
{
}

// =============================================================================================
// Thunks, inline getters and template instantiations (bodies kept out of line by the optimizer)
// ---------------------------------------------------------------------------------------------
// wasm func 422  vota::CThreadVota::StopTick(uebyte id) { m_ticks.StopTick(id); }  -> api::CTickManager::StopTick
//                (func 5451) on the tick manager at thread +20. 23 callers (voter and operator states).
// wasm func 2745 `impl::IInformacaoThreadOperador::GetInst().GetApresentacaoFoto()` (slot 30) - outlined call.
// wasm func 3621 `impl::IInformacaoThreadOperador::GetInst().SetHabilitacaoSemBiometria()` (slot 14) - outlined call.
// wasm func 3078 `bool CInformacaoEleitor::AudioHabilitado() const { return m_modoAudio == 1; }` (+4).
// wasm func 1072 `CFormBuilderMT::Add<api::CBeepFieldMT>(int)` - thin wrapper over the shared body func 3890
//                (which also implements Add<CLedFieldMT>(bool), func 435): new field(28 bytes){..., +24 = arg},
//                wrapped in shared_ptr and pushed on the field vector.
// wasm func 651  `Add<api::CTextFieldMT>(SPoint, make_shared<api::CDataTextFmt<std::string(*)(const std::string&)>>(align, fn, fmt))`
// wasm func 1152 `Add<api::CTextFieldMT>(SPoint, make_shared<api::CDataTextFmt<api::CTextSource>>(align, src, fmt))`
//                (the tools attribute it to cpededigital.cpp; used by 7 states)
// wasm func 2499 std::__tree<std::__value_type<K, std::string>>::destroy (recursive node deleter; ICF-shared by
//                the local map of CDigitalReconhecida::ProcessInput and the static map of api::getResourceMovie).
// wasm func 13471 exit-time destructor (atexit, table slot 1085) of the static
//                std::map<int, std::string> in api::getResourceMovie (uenux2/src/api/gui/iresource.h:87):
//                `s_filmes.~map()` -> func 2499. Not operator code.

// wasm func 2233 (tools: "api_f2233") - uenux2/src/api/util/cdatetime.cpp               name inferred
// (u06 calls it AdicionaSegundos; func 5474 is the operator+=(std::chrono::seconds) wrapper.)
void api::CDateTime::AdicionaSegundos(int segundos)
{
    std::tm t{};
    t.tm_sec  = m_segundosDoDia % 60;
    t.tm_min  = (m_segundosDoDia % 3600) / 60;
    t.tm_hour = m_segundosDoDia / 3600;
    t.tm_mday = m_data.GetDia();
    t.tm_mon  = m_data.GetMes() - 1;
    t.tm_year = m_data.GetAno() - 1900;
    const std::time_t instante = timegm(&t) + segundos;      // calendar arithmetic in UTC (no DST effects)
    std::tm r;
    if (gmtime_r(&instante, &r) != nullptr) {
        m_data = CDate(r.tm_mday, r.tm_mon + 1, r.tm_year + 1900);           // func 2765
        m_segundosDoDia = EncodeTime(r.tm_hour, r.tm_min, r.tm_sec);        // func 3642
    }
}

}  // namespace vota
