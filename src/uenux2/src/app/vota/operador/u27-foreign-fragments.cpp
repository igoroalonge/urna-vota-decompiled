// FRAGMENTS reconstructed by unit u27 from vota_web_wasm.wasm.
//
// Functions that the analysis tools put in unit u27 (uenux2/src/app/vota/operador) but whose original
// file is NOT one of the u27 files (cthreadoperador.cpp, confirmaidentidade/cregistradigitaloperador.cpp,
// justificativa/i{inicia,confirma}justificativa.cpp, leidentidade/{celeitorencontrado,cpedeidentidade}.cpp,
// outrasopcoes/{caguardaeleitoresvotarem,cencerramentohorarioinvalido,cescolheopcao}.cpp). Each block
// names its real (usually inferred) home; merge it there when that file is reconstructed.
//
// WEB BUILD: everything here runs on the operator thread (CThreadOperador::Run), which the web build never
// starts - dead code in the simulator (except the helpers marked "shared").
//
// Vocabulary: see src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp (MT LCD 4x40, SPoint{coluna,
// linha}, alignment 0 left / 1 right / 2 centre, form-builder template instances, lazy singletons).
#include <filesystem>
#include <format>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>

#include "api/gui/cformbuildermt.h"
#include "api/util/csystem.h"
#include "comum/appinfo/cappinfo.h"
#include "comum/cappstate.h"
#include "comum/dados/cconfiguracaoeleicao.h"
#include "comum/dados/celeitores.h"
#include "vota/log/clogvota.h"
#include "vota/operador/comum/iinformacaothreadoperador.h"
#include "vota/operador/cthreadoperador.h"
#include "vota/operador/leidentidade/celeitorencontrado.h"
#include "vota/operador/leidentidade/cpedeidentidade.h"
#include "vota/operador/leidentidade/ieleitorimpedidovotar.h"
#include "vota/operador/justificativa/iconfirmajustificativa.h"
#include "vota/operador/justificativa/iiniciajustificativa.h"
#include "vota/operador/outrasopcoes/cencerramentohorarioinvalido.h"
#include "vota/operador/outrasopcoes/cescolheopcao.h"

namespace vota {

using api::SPoint;

namespace {
constexpr auto ESQUERDA = api::ETextAlignment(0);
constexpr auto DIREITA  = api::ETextAlignment(1);
constexpr auto CENTRO   = api::ETextAlignment(2);

#define VOTA_LAZY_SINGLETON(Classe)                                                     \
    Classe& Classe::GetInst()                                                          \
    {                                                                                  \
        static std::mutex mutex;                                                       \
        static std::unique_ptr<Classe> s_inst;                                         \
        std::lock_guard lock(mutex);                                                   \
        if (!s_inst)                                                                   \
            s_inst.reset(new Classe());                                                \
        return *s_inst;                                                                \
    }
}  // namespace

// =================================================================================================
// uenux2/src/app/vota/operador/caguardainicio.cpp  (path inferred; methods 10217 ProcessMessage and
// 10220 StartState belong to unit u39). The tools filed wasm func 5343 under cthreadoperador.cpp.
// "Aguarda início": first state of the operator thread, until the voter thread reports that voting
// started (message 8, posted by CInicioVotacao -> func 5972). Harness screen:
//     VOTA: 10.23.0.1 / DESENVOLVIMENTO / Siga as instruções na tela do eleitor / 04/10/2026 16:50:00
// =================================================================================================

// Constructor inlined into GetInst (wasm func 5343). 24 bytes: CAppState(5 = messages + ticks),
// +12 m_form (non-interactive), +20 m_tick (500 ms: clock refresh).
CAguardaInicio::CAguardaInicio()
    : comum::CAppState(5)
{
    // "<nome>: <versão>" from the statics of api::CApplication::InitApplication (func 11159):
    // nome @1839168 ("VOTA"), versão @1839192 ("10.23.0.1 - DESENVOLVIMENTO"). Split at the first '-',
    // each half trimmed (func 1374): "VOTA: 10.23.0.1" / "DESENVOLVIMENTO".
    const std::string versao = std::format("{}: {}", api::CApplication::GetNome(), api::CApplication::GetVersao());
    std::string linha1 = versao, linha2;
    if (const auto traco = versao.find('-'); traco != std::string::npos) {
        linha1 = comum::Trim(versao.substr(0, traco));                    // func 1374
        linha2 = comum::Trim(versao.substr(traco + 1));                   // substr throws out_of_range if needed
    }
    api::CFormBuilderMT campos;
    if (!linha1.empty())
        campos.Add<api::CTextFieldMT>(SPoint{1, 1}, std::make_shared<api::CFixedText>(CENTRO, linha1));
    if (!linha2.empty())
        campos.Add<api::CTextFieldMT>(SPoint{1, 2}, std::make_shared<api::CFixedText>(CENTRO, linha2));
    campos.Add<api::CTextFieldMT>(SPoint{1, 3}, std::make_shared<api::CFixedText>(CENTRO, "Siga as instruções na tela do eleitor"));
    campos.Add<api::CTextFieldMT>(SPoint{11, 4}, std::make_shared<api::CDataTextFmt<std::string (*)(const std::string&)>>(
                                                     ESQUERDA, &TextoData, "DD/MM/YYYY"));   // func 651, slot 1103 (5472)
    campos.Add<api::CTextFieldMT>(SPoint{22, 4}, std::make_shared<api::CDataTextFmt<std::string (*)(const std::string&)>>(
                                                     ESQUERDA, &TextoHora, "hh:mm:ss"));     // slot 1104 (5473)
    m_form = campos.CriaForm("");                                                           // func 1694
    m_tick = CThreadOperador::GetInst().CriaTick(500);                                     // func 807
}
// wasm func 5343 (tools: vota_f5343): lazy singleton @1911624, mutex @1911600. Callers:
// CThreadOperador::Run (10204) and CControladorRegistraMesariosVota slot 9 (10794).
VOTA_LAZY_SINGLETON(CAguardaInicio)

// =================================================================================================
// uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp  (path inferred, as in u20)
// =================================================================================================

// wasm func 10794 - vtable slot 9 of comum::IControladorRegistraMesarios (u22 table: where the operator
// goes after the INITIAL registration of mesários).                                name from u22's slot table
comum::CAppState* CControladorRegistraMesariosVota::GetEstadoAposRegistroInicial()
{
    return &CAguardaInicio::GetInst();                                               // func 5343
}

// =================================================================================================
// uenux2/src/app/vota/operador/leidentidade/cprocuraeleitor.cpp  (path inferred, as in u17)
// =================================================================================================

// wasm func 10631 - vtable slot 2 (StartState). "Procura eleitor": look the typed identity up in the
// section roll (positions CEleitores' current voter).
void CProcuraEleitor::StartState()
{
    const comum::md::CEleitorIdentidade identidade =
        impl::IInformacaoThreadOperador::GetInst().GetIdentidadeEleitor();            // func 1535 (slot 25)
    const bool encontrado = comum::CEleitores::GetInst().Procura(                      // func 2264: find + make current
        comum::md::CEleitorIdentidade::Formata(identidade.GetIdentidade(), identidade.GetTipo()),   // func 2798
        identidade.GetTipo());

    if (encontrado) {
        m_proximoEstado = &CEleitorEncontrado::GetInst();                              // @1905616, ctor inlined
        return;
    }
    // Not in this section. With justification accepted and a TÍTULO typed, the voter may justify:
    // "não pertence à seção" (CConfirmaJustificativa, label from TraduzLabel).
    if (!comum::CInformacaoEleicao(comum::CConfiguracaoEleicao::GetInst()).ImprimeBoletimJustificativa()  // 4578
        || identidade.GetTipo() != comum::md::ETipoIdentidade::TITULO) {               // tipo 1
        m_proximoEstado = &CEleitorNaoEncontrado::GetInst();                           // func 5424
        return;
    }
    m_proximoEstado = &CIniciaJustificativa::GetInst();                                // @1905728, ctor inlined
}

// =================================================================================================
// uenux2/src/app/vota/operador/leidentidade/cvalidaidentidade.cpp  (path inferred, as in u17; the tools
// filed these under cpedeidentidade.cpp because CPedeIdentidade::ProcessInput inlines the constructor)
// Layout (32 bytes): CAppState(2), +12 m_form (shared_ptr), +20 std::map<int, SOpcaoIdentidade> m_opcoes.
// =================================================================================================

// wasm func 2749 (slot 0): ~CValidaIdentidade = m_opcoes tree destroyed (func 1906: recursive
// __tree::destroy; each node frees the SOpcaoIdentidade::texto string at +20) + m_form.reset().
// wasm func 10628 (slot 1): + operator delete.   wasm func 10630: atexit reset of the instance @1905672.
CValidaIdentidade::~CValidaIdentidade() = default;

// uenux2/src/app/vota/operador/leidentidade/cidentidadeinvalida.cpp  (path inferred, as in u17)
// wasm func 10667 - vtable slot 7: "Identidade: 000000000123 / Número errado / CORRIGE: retornar"
// (seen in the harness after typing 123).
void CIdentidadeInvalida::ProcessInput()
{
    RetornaParaPedeIdentidadeSeTecla(*this, *m_form, api::EInputResult::CORRIGE);    // shared body 2295
}

// =================================================================================================
// uenux2/src/app/vota/operador/justificativa/cjustificativaefetuada.cpp  (path inferred; the tools filed
// func 2747 under iiniciajustificativa.cpp). ProcessInput = func 10593 (u19).
// =================================================================================================

// Constructor inlined into GetInst (wasm func 2747). 32 bytes: CAppState(2), +12 m_texto
// (shared_ptr<string>, placeholder "justificou/ja justificou" - replaced by StartState, other unit ?),
// +20 m_form, +28 m_novaJustificativa = true (IIniciaJustificativa sets false: already justified).
CJustificativaEfetuada::CJustificativaEfetuada()
    : comum::CAppState(2)
    , m_texto(std::make_shared<std::string>("justificou/ja justificou"))
{
    api::CFormBuilderMT campos;
    campos.Add<api::CLedFieldMT>(false);
    campos.Add<api::CTextFieldMT>(SPoint{1, 1}, std::make_shared<api::CDataText<std::string (*)()>>(ESQUERDA, &TextoIdentidadeDigitada));
    campos.Add<api::CTextFieldMT>(SPoint{1, 2}, std::make_shared<api::CDataTextFmt<api::CTextSource>>(
                                                    ESQUERDA, api::CTextSource(m_texto), "%s"));   // func 1152
    campos.Add<api::CTextFieldMT>(SPoint{1, 4}, std::make_shared<api::CFixedText>(DIREITA, "CONFIRMA: continuar"));   // (1,4) right, as in the binary
    campos.AddInputControl();
    m_form = campos.CriaFormInterativo("", true);
    m_novaJustificativa = true;
}
// wasm func 2747: @1905952, mutex @1905928. Callers: IIniciaJustificativa::StartState (10587),
// CPedeAnoNascimento::ProcessInput (10590).
VOTA_LAZY_SINGLETON(CJustificativaEfetuada)

// uenux2/src/app/vota/operador/justificativa/celeitorimpedidojustificarvotocpf.cpp  (path inferred)
// wasm func 10604 - vtable slot 7 (the class is created in IConfirmaJustificativa::ProcessInput, 10588,
// when the typed identity is a CPF).
void CEleitorImpedidoJustificarVotoCPF::ProcessInput()
{
    RetornaParaPedeIdentidadeSeTecla(*this, *m_form, api::EInputResult::CORRIGE);    // shared body 2295
}

// =================================================================================================
// uenux2/src/app/vota/operador/outrasopcoes/ciniciafinalizacao.cpp  (path inferred)
// "Inicia finalização": transit state after "2-Encerrar votação" (CEscolheOpcao). 12 bytes, CAppState(0).
// =================================================================================================

// wasm func 10706 - vtable slot 2 (StartState)
void CIniciaFinalizacao::StartState()
{
    const api::CDateTime agora;                                                      // func 479
    const auto& cfg = comum::CConfiguracaoEleicao::GetInst();

    if (comum::EhTreinamentoEleitor()) {                                             // func 697 (voter training)
        m_proximoEstado = &CPerguntaFilaEleitorVazia::GetInst();                     // func 5426
        return;
    }
    if (agora >= cfg.GetDataHoraEncerramentoVotacao()) {                              // func 759 (<=>), cfg +568
        // After terminoVotacao with nobody voting, the "is anybody still in line?" question is skipped.
        if (impl::IInformacaoThreadOperador::GetInst().VotacaoBloqueadaPorHorario())  // func 2226
            m_proximoEstado = &CPedeTituloEncerramento::GetInst();                    // func 3631
        else
            m_proximoEstado = &CPerguntaFilaEleitorVazia::GetInst();                 // func 5426
        return;
    }
    CLogVota::GetInst().Loga(2, std::format("Encerramento só pode ser solicitado após as {} horas",
                                            cfg.GetDataHoraEncerramentoVotacao().GetTime().Format("hh:mm:ss")));   // func 779
    m_proximoEstado = &CEncerramentoHorarioInvalido::GetInst();                      // ctor inlined (see its file)
}

// uenux2/src/app/vota/operador/outrasopcoes/cperguntafilaeleitorvazia.cpp  (path inferred; methods 10713
// ProcessInput / 10715 StartState: other units). Constructor inlined into GetInst (wasm func 5426).
CPerguntaFilaEleitorVazia::CPerguntaFilaEleitorVazia()
    : comum::CAppState(2)
{
    api::CFormBuilderMT campos;
    campos.Add<api::CClockFieldMT>(SPoint{33, 1});
    campos.Add<api::CTextFieldMT>(SPoint{1, 2}, std::make_shared<api::CFixedText>(ESQUERDA, "Todas as pessoas presentes já votaram?"));
    campos.Add<api::CTextFieldMT>(SPoint{1, 4}, std::make_shared<api::CFixedText>(ESQUERDA, "CORRIGE: não"));
    campos.Add<api::CTextFieldMT>(SPoint{40, 4}, std::make_shared<api::CFixedText>(DIREITA, "CONFIRMA: sim"));
    campos.AddInputControl();
    m_form = campos.CriaFormInterativo("", true);                                    // +12
}
VOTA_LAZY_SINGLETON(CPerguntaFilaEleitorVazia)       // wasm func 5426: @1905196, mutex @1905172

// uenux2/src/app/vota/operador/outrasopcoes/cpedetituloencerramento.cpp  (path inferred; StartState 10722
// and ProcessInput 10721 are in u17's fragments). Constructor inlined into GetInst (wasm func 3631).
// 32 bytes: +12 m_form, +20 m_horarioEncerramento (CDateTime, 12 bytes).
CPedeTituloEncerramento::CPedeTituloEncerramento()
    : comum::CAppState(2)
{
    api::CFormBuilderMT campos;
    campos.Add<api::CClockFieldMT>(SPoint{33, 1});
    campos.Add<api::CTextFieldMT>(SPoint{1, 1}, std::make_shared<api::CFixedText>(ESQUERDA, "Informe seu título para"));
    campos.Add<api::CTextFieldMT>(SPoint{1, 2}, std::make_shared<api::CFixedText>(ESQUERDA, "encerrar a votação"));
    campos.Add<api::CTextFieldMT>(SPoint{1, 4}, std::make_shared<api::CFixedText>(ESQUERDA, "CORRIGE: retornar"));
    campos.Add<api::CTextFieldMT>(SPoint{40, 4}, std::make_shared<api::CFixedText>(DIREITA, "CONFIRMA: prosseguir"));
    campos.Add<api::CInputFieldMT>(12, /*aguardaConfirma*/ true, SPoint{1, 3});     // func 1151
    m_form = campos.CriaFormInterativo("", true);
    m_horarioEncerramento = comum::CConfiguracaoEleicao::GetInst().GetDataHoraEncerramentoVotacao();   // cfg +568
}
VOTA_LAZY_SINGLETON(CPedeTituloEncerramento)         // wasm func 3631: @1905140, mutex @1905116

// =================================================================================================
// uenux2/src/app/vota/operador/outrasopcoes/chabilitacaoaudionaopermitida.cpp  (path inferred)
// (constructor inlined into CEscolheOpcao::ProcessInput, see cescolheopcao.cpp)
// =================================================================================================

// wasm func 10754 - vtable slot 2 (StartState)
void CHabilitacaoAudioNaoPermitida::StartState()
{
    m_form->Show();                                  // "Ativação de áudio não permitida."
    api::CSystem::Sleep(1000);                       // `if (byte @1584624 == 1) emscripten_sleep(1000)`:
                                                     //  would ABORT in the browser (no ASYNCIFY)
    m_proximoEstado = &CEscolheOpcao::GetInst();     // func 2753
}

// uenux2/src/app/vota/operador/outrasopcoes/cconfirmaaudio.cpp  (path inferred)
// wasm func 10742 (table slot 3757): line 2 of CConfirmaAudio ("CORRIGE: não  CONFIRMA: sim"); the
// function is defined in cescolheopcao.cpp as TextoConfirmaAudio() because only that file's inlined
// constructor uses it.   ProcessInput = func 10740 (other unit).

// uenux2/src/app/vota/operador/outrasopcoes/chorariovotacaoterminou.cpp  (path inferred)
// GetInst + ctor = func 5430 (other unit): Beep(2); (1,1) "Horario de votacao terminou!";
// (1,2) "Favor encerrar a urna!"; (1,4) "CORRIGE"; input control.
// wasm func 10757 - vtable slot 7
void CHorarioVotacaoTerminou::ProcessInput()
{
    RetornaParaPedeIdentidadeSeTecla(*this, *m_form, api::EInputResult::CORRIGE);    // shared body 2295
}

// =================================================================================================
// uenux2/src/app/vota/operador/confirmaidentidade/ctentativacapturadigitalesgotada.cpp  (path inferred)
// (constructor inlined into CRegistraDigitalOperador::ProcessTick)
// =================================================================================================

// wasm func 10461 - vtable slot 7: CONFIRMA ("Eleitor não habilitado para votação") -> CPedeIdentidade.
void CTentativaCapturaDigitalEsgotada::ProcessInput()
{
    RetornaParaPedeIdentidadeSeTecla(*this, *m_form, api::EInputResult::CONFIRMA);   // shared body 2295 (key 9)
}

// =================================================================================================
// uenux2/src/app/vota/operador/confirmaidentidade/cpedeanonascimentosembiometria.cpp  (path inferred;
// class described by u10: birth-year check of a voter without usable biometrics)
// Layout: +12 m_formPedeAno ("Digite o ANO de nascimento:"), +20 m_formAnoIncorreto
// ("ANO DE NASCIMENTO INCORRETO / CONFIRMA: tentar novamente"), +28 int m_erros, +32 bool m_pedindoAno.
// =================================================================================================

// wasm func 10489 - vtable slot 7 (ProcessInput)
void CPedeAnoNascimentoSemBiometria::ProcessInput()
{
    if (!m_pedindoAno) {                                     // "ano incorreto" screen shown
        if (m_formAnoIncorreto->Read() == api::EInputResult::CONFIRMA) {
            m_pedindoAno = true;
            m_formPedeAno->Show();
        }
        return;
    }

    switch (m_formPedeAno->Read()) {
    case api::EInputResult::CORRIGE:
        CLogVota::GetInst().Loga("Habilitação cancelada durante confirmação de dado do eleitor");   // func 2097
        m_proximoEstado = &CCancelaHabilitacaoEleitor::GetInst();                                    // func 1536
        return;
    case api::EInputResult::CONFIRMA: {
        const std::string digitado = m_formPedeAno->GetEntrada(0).GetTexto();
        if (digitado.size() < 4)
            return;                                                                    // incomplete: stay
        const unsigned long ano = std::stoul(digitado);                                // func 4661
        const auto anoCadastro = comum::CEleitores::GetInst().GetCurrent().GetEleitor().GetAnoNascimento();   // 5665
        if (anoCadastro == ano) {
            CLogVota::GetInst().Loga(1, "Ano de nascimento digitado é igual ao do cadastro");
            m_proximoEstado = &CInformaEleitorPodeVotar::GetInst();                    // func 5399
            return;
        }
        CLogVota::GetInst().Loga(2, "Ano de nascimento digitado é diferente do cadastro");
        if (++m_erros <= 1) {                                                          // one retry
            m_pedindoAno = false;
            m_formAnoIncorreto->Show();
            return;
        }
        m_proximoEstado = &CInformaAnoNascimentoErrado::GetInst();
        //   ^ @1908896 (mutex @1908872), ctor inlined here: CAppState(2); (1,1) "Por favor oriente o eleitor a
        //     procurar o"; (1,2) "cartório eleitoral para consultar a data"; (1,3) "de nascimento dele no
        //     cadastro da urna"; (40,4) right "CONFIRMA: cancelar a habilitação"; input control;
        //     CriaFormInterativo("telaInformaAnoNascimentoErrado", false). ProcessInput = 10493 (u19).
        return;
    }
    default:
        return;
    }
}

// =================================================================================================
// uenux2/src/app/vota/operador/confirmaidentidade/cinformaeleitorpodevotar.cpp  (path inferred;
// ProcessInput = func 10496, other unit: releases the voter)
// =================================================================================================

// wasm func 5410 (tools: vota_f5410). Builds the "ELEITOR(A) PODE VOTAR" form. Shared by
// CInformaEleitorPodeVotar (5399) and CControlaReconhecimento::GetInst (func 1256, u10) - probably a
// free helper in a common header/file.                                                    name inferred
std::shared_ptr<api::CInteractiveForm<api::IScreenMT, api::IInputMT>> CriaFormEleitorPodeVotar()
{
    api::CFormBuilderMT campos;
    campos.Add<api::CLedFieldMT>(false);
    campos.Add<api::CBeepFieldMT>(1);
    campos.Add<api::CTextFieldMT>(SPoint{1, 1}, std::make_shared<api::CFixedText>(ESQUERDA, "ELEITOR(A) PODE VOTAR"));
    campos.Add<api::CTextFieldMT>(SPoint{1, 2}, std::make_shared<api::CFixedText>(ESQUERDA, "Assinar o caderno de votação antes de"));
    campos.Add<api::CTextFieldMT>(SPoint{1, 3}, std::make_shared<api::CFixedText>(ESQUERDA, "votar"));
    // Position (1,4) right-aligned (262145 in the binary), NOT (40,4) as in most other MT forms: the
    // alignment apparently ignores the column, so the text still ends at the right edge.
    campos.Add<api::CTextFieldMT>(SPoint{1, 4}, std::make_shared<api::CFixedText>(DIREITA, "CONFIRMA: prosseguir"));
    campos.AddInputControl();                        // inlined copy of func 395
    return campos.CriaFormInterativo("", true);
}

CInformaEleitorPodeVotar::CInformaEleitorPodeVotar()
    : comum::CAppState(2)
    , m_form(CriaFormEleitorPodeVotar())             // +12
{
}
// wasm func 5399 (tools: vota_f5399): @1908868, mutex @1908844. Callers: CRegistraDigitalOperador::ProcessTick,
// CPedeAnoNascimentoSemBiometria::ProcessInput.
VOTA_LAZY_SINGLETON(CInformaEleitorPodeVotar)

}  // namespace vota

// =================================================================================================
// comum (other directories)
// =================================================================================================
namespace comum {

// uenux2/src/app/comum/comparecimentomesario/estados/cregistrarmesarios.cpp  (path inferred; the u22 flow
// "Registrar mesário?"). Constructor inlined into GetInst (wasm func 3610). 28 bytes: CAppState(2),
// +12 m_form (MT), +20 m_formEleitor (voter screen, func 1255: "REGISTRO DE MESÁRIOS" /
// "Siga as instruções no terminal do mesário.").
CRegistrarMesarios::CRegistrarMesarios()
    : CAppState(2)
    , m_formEleitor(CriaFormEleitorRegistroMesarios())                               // func 1255
{
    api::CFormBuilderMT campos;
    campos.Add<api::CBuzzFieldMT>(51, 10);                                           // func 941
    campos.Add<api::CTextFieldMT>(api::SPoint{1, 1}, std::make_shared<api::CFixedText>(api::ETextAlignment(0), "Registrar mesário?"));
    campos.Add<api::CTextFieldMT>(api::SPoint{1, 4}, std::make_shared<api::CFixedText>(api::ETextAlignment(0),
                                                                                      "CORRIGE: Cancelar  CONFIRMA: Prosseguir"));
    campos.AddInputControl();
    m_form = campos.CriaFormInterativo("", true);
}
// wasm func 3610 (tools: comum_f3610): @1909428, mutex @1909404. Callers: CAguardaInicio::ProcessMessage
// (10217), CEscolheOpcao::ProcessInput (10689), CFimAquisicaoVotos::StartState (10737).
CRegistrarMesarios& CRegistrarMesarios::GetInst()
{
    static std::mutex mutex;
    static std::unique_ptr<CRegistrarMesarios> s_inst;
    std::lock_guard lock(mutex);
    if (!s_inst)
        s_inst.reset(new CRegistrarMesarios());
    return *s_inst;
}

// uenux2/src/app/comum/comparecimentomesario/ccontroladorreconhecimetomesario.cpp (u22 owns the file)
// wasm func 1149 (tools: comum_f1149): lazy singleton @1909960 (mutex @1909936), 24 bytes, constructor inlined:
//   m_dedo = 0, m_limiarScore = 20, m_estado = 1 (NAO_COLETADA), m_idArquivo = nullopt,
//   m_qtdArquivos = ListaDigitaisNaoRegistradas().size()   (vota_f5374: throws if the directory is missing)
CControladorReconhecimentoMesario& CControladorReconhecimentoMesario::GetInst()
{
    static std::mutex mutex;
    static std::unique_ptr<CControladorReconhecimentoMesario> s_inst;
    std::lock_guard lock(mutex);
    if (!s_inst) {
        auto novo = std::make_unique<CControladorReconhecimentoMesario>();
        novo->m_qtdArquivos = vota::ListaDigitaisNaoRegistradas().size();
        s_inst = std::move(novo);
    }
    return *s_inst;
}

// uenux2/src/app/comum/appinfo/cappinfo.cpp (declared in cappinfo.h by another unit)
// wasm func 697 (tools: comum_f697). Voter-training mode ("treinamento de eleitores") = training phase
// (CEstadoGeral fase '3') AND CEstadoGeralVota.treinamentoEleitor (+72). The web simulator runs in it.
bool EhTreinamentoEleitor()
{
    auto& app = CAppInfo::GetInst();                                                 // func 185
    if (GetEstadoGeral(app).GetFase() != '3')                                        // GetEstado@291 +12
        return false;
    return GetEstadoVota(app).treinamentoEleitor;                                    // GetEstado@261(51) +72
}

// uenux2/src/app/comum/cpath.cpp ? (curated note: "builds '<CPath::GetPathTrab>/wsq/' paths")
// wasm func 2864 (tools: comum_f2864)                                                   name inferred
std::filesystem::path CPath::GetPathWsq(const EFlashOrigem flash, const int turno)
{
    return GetPathTrab(flash, turno) / "wsq/";                                       // func 358
}

}  // namespace comum

// =================================================================================================
// api/gui template instances of the MT form builder (api::CFormBuilderMT = vector<shared_ptr<
// IFormField<IScreenMT>>>) that the tools placed in this unit. Library-like code, summarised:
//   395   AddInputControl(): push make_shared<CInputFieldControl<IScreenMT>>(make_shared<CControlValidation>())
//         (IInputField ctor 3673 with tamanho 0, flag 1)
//   728   Add<CClockFieldMT>(pos)                         (28-byte field, vtable @1579812)
//   1151  Add<CInputFieldMT>(tamanho, aguardaConfirma, pos) -> shared_ptr<CInputFieldMT>; validation
//         CNumberValidation("0123456789") (vtable @1538660); field 72 bytes (vtable @1587492)
//   1694  CriaForm(nome): make_shared<IForm<IScreenMT>>(campos, make_shared<CPreShowClearMT>()), name set
//         when not empty, then the builder's vector is cleared (non-interactive form)
//   5522  IForm<IScreenMT>::IForm(campos, preShow) -> merged body 6024 with the vtable as parameter
//   3673  IInputField<IScreenMT>::IInputField(shared_ptr<IValidation>, int tamanhoMax, bool flag): +36/+40
//         validation, +44 tamanho, +48 foco = false, +49 cursorVisivel = true, +50 flag, +52 = 1,
//         +56 ITimerScheduler timer of 600 ms (cursor blink), text string reserved to tamanhoMax
//   3674  std::vector<std::shared_ptr<IFormField<IScreenMT>>>::push_back(&&) (growth inlined)
//   5419  Add<CTextFieldMT>(pos, make_shared<CDataText<std::function<std::string()>>>(ESQUERDA, fonte))
// Other helpers of the unit:
//   1055  std::filesystem::status(const path&, error_code*) wrapper (libc++; the only u27 function seen
//         executing in the recorded votes - reached from start-up code: std::filesystem::create_directories
//         (func 2548), CPacoteArquivos::ValidarChaveEAplicacaoValida (4625), unknown_f5836; runtime edges)
//   1687 / 2226 / 3619 / 5414  one-line outlined calls IInformacaoThreadOperador::GetInst().X()
//         (slots 6 / 19 / 22 / 21), see cpedeidentidade.cpp / cescolheopcao.cpp
// =================================================================================================
