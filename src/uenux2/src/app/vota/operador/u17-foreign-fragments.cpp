// FRAGMENTS reconstructed by unit u17 from vota_web_wasm.wasm.
//
// Poll-worker (mesário) terminal states of the operator thread (vota::CThreadOperador). The analysis
// tools attributed them to u17's api files because an api inline function is compiled into each:
// the CTextSource constructor (ctextsource.h:37), CInteractiveForm::Read (cinteractiveform.h:57,
// which inlines IInput::GetKey) or a form-builder helper. Their real homes are given per block
// ("path inferred" = directory of the closest relatives: operador/{aguardaeleitor, confirmaidentidade,
// justificativa, leidentidade, outrasopcoes}). Merge each block into its file.
//
// WEB BUILD: dead code. The web entry point only steps CThreadEleitor; CThreadOperador::Run (func
// 10204) never runs (unit u10 §2), so none of these states is ever created in the simulator. It is
// the real urna logic of the microterminal.
//
// Vocabulary (same as u10, see confirmaidentidade/cpededigital.cpp):
//   MT LCD = 4 lines x 40 columns; api::SPoint{coluna, linha}, 1-based; alignment 0 left, 1 right,
//   2 centred. For 1 and 2 the web CWasmScreenMT::Write (func 8799) ignores x: right = ends at
//   column 40, centred = (40 - len) / 2; only y is used. A point written as one int32 in the binary
//   is shown decoded (65569 -> {33,1}).
//   Form helpers (template instances of the MT form builder api::CFormBuilderMT, a
//   std::vector<shared_ptr<IFormField<IScreenMT>>>):
//     func 435  Add<CLedFieldMT>(estado)          (0 off, 1 on, 2 used by the failure screen)
//     func 941  Add<CBuzzFieldMT>(51, 10)         func 1072 Add<CBeepFieldMT>(n)
//     func 728  Add<CClockFieldMT>(pos)           (hh:mm:ss at {33,1} on most screens)
//     func 180  Add<CTextFieldMT>(pos, make_shared<CFixedText>(align, texto))
//               (binary: func 180 takes (texto, pos, align), builds the CFixedText itself with
//               `new` + shared_ptr<IText> (__shared_ptr_pointer, not make_shared) and returns the
//               field's shared_ptr; the call form above is the vocabulary used in this file)
//     func 619  Add<CTextFieldMT>(pos, make_shared<CDataText<std::string (*)()>>(align, fonte))
//     func 651  Add<CTextFieldMT>(pos, make_shared<CDataTextFmt<std::string (*)(const std::string&)>>(align, fonte, fmt))
//     func 1152 Add<CTextFieldMT>(pos, make_shared<CDataTextFmt<CTextSource>>(align, fonte, fmt))
//     func 5409 Add<CTextFieldMT>(pos, make_shared<CDataText<CTextSource>>(align, fonte))
//     func 1151 Add<CInputFieldMT>(tamanho, aguardaConfirma, pos)  (CNumberValidation "0123456789")
//     func 395  AddInputControl()                 (keys without a visible field)
//     func 301  CriaFormInterativo(nome, limpaTeclado) -> CInteractiveForm<IScreenMT, IInputMT>
//     func 1694 CriaForm(nome)                    -> non-interactive IForm<IScreenMT>
//   Text sources (table slots): 3906 comum::CEleitorDadoNomeParaUrna::Text, 3907 TextoIdentidadeDigitada
//   (func 10586), 3860 IdentidadeDigitada (func 10670), 3781 TextoTituloEncerramento (func 10728:
//   format("{:<12s}", CThreadOperador +120)), 4053 IInformacaoThreadOperador::GetTextoQtdVotaram
//   (10583), 4054 ::GetTextoAudio (10584), 4055/4056/4113 CEleitorDadoSequencial / Secao / TTE::Text,
//   4057 TextoAudioEleitor (func 10539: "ÁUDIO ATIVADO" or " ").
//   func 807 = CThreadVota::CriaTick(ms) (shared by CThreadEleitor/CThreadOperador: tick table +20).
//   func 270 = CThreadOperador::GetInst(), 316 = CThreadEleitor::GetInst(), 383 = instance<IInputMT>.
//   State slots (comum::CAppState): 2 StartState, 3 NeedChangeState, 4 GetNextState, 5 FinishState,
//   6 ProcessMessage, 7 ProcessInput, 8 ProcessTick. m_proximoEstado (+4) = this means "stay";
//   nullptr means "no next state": the operator's CAppStateContext is left without a state.
//   Every state is a lazy singleton (static mutex + static unique_ptr; only the mutex unlock stub
//   remains in the single-threaded build). Strings are Latin-1 in the binary.
#include <format>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "api/gui/cinteractiveform.h"
#include "api/gui/ctextsource.h"
#include "api/util/cdatetime.h"
#include "comum/cappstate.h"
#include "comum/cconfiguracaoeleicao.h"
#include "comum/dados/celeitores.h"
#include "comum/justificativa/cjustificador.h"
#include "comum/md/cvalidadoridentidade.h"
#include "vota/comum/csincronizavota.h"
#include "vota/eleitor/cthreadeleitor.h"
#include "vota/eleitor/comum/cinformacaoeleitor.h"
#include "vota/log/clogvota.h"
#include "vota/operador/cthreadoperador.h"
#include "vota/operador/comum/iinformacaothreadoperador.h"

namespace vota {

using api::SPoint;
using TFormMT = api::CInteractiveForm<api::IScreenMT, api::IInputMT>;

namespace {
constexpr auto ESQUERDA = api::ETextAlignment(0);
constexpr auto DIREITA  = api::ETextAlignment(1);
constexpr auto CENTRO   = api::ETextAlignment(2);

constexpr api::EInputResult CORRIGE  = api::EInputResult::Corrige;    // 5
constexpr api::EInputResult CONFIRMA = api::EInputResult::Confirma;   // 9

// Lazy singleton accessor pattern shared by every state below (written out once).
#define VOTA_LAZY_SINGLETON(Classe, mutexAddr, instAddr)                                 \
    Classe& Classe::GetInst()                                                           \
    {                                                                                   \
        static std::mutex mutex;                    /* mutexAddr */                     \
        static std::unique_ptr<Classe> s_inst;      /* instAddr  */                     \
        std::lock_guard lock(mutex);                                                    \
        if (!s_inst)                                                                    \
            s_inst.reset(new Classe());                                                 \
        return *s_inst;                                                                 \
    }

// "Identidade" types accepted to identify a voter (CConfiguracaoEleicao +672, vector<ETipoIdentidade>):
// 1 título de eleitor, 2 CPF, 3 identificador; anything else is shown as "a Identidade".
std::string NomeTipoIdentidadeComArtigo(comum::md::ETipoIdentidade tipo)          // inlined; name inferred
{
    switch (static_cast<int>(tipo)) {
    case 1: return "o Título";
    case 2: return "o CPF";
    case 3: return "o Identificador";
    default: return "a Identidade";
    }
}
}  // namespace

// =================================================================================================
// uenux2/src/app/vota/operador/leidentidade/cpedeidentidade.cpp  (srclocs of StartState 10680)
// "Pede identidade": the idle screen of the microterminal, waiting for the next voter's título/CPF.
// =================================================================================================

// Constructor (inlined into GetInst, func 652). 32 bytes:
//   +11 m_tickMinuto (60 s: VotacaoBloqueadaPorHorario / next inspection, see u10)  +12 m_tick5s (?)
//   +13 m_tick1s (?)  +16 m_textoStatus (shared_ptr<string>, " ")  +24 m_form
CPedeIdentidade::CPedeIdentidade()
    : comum::CAppState(6)                                                    // keys + ticks
    , m_tickMinuto(CThreadOperador::GetInst().CriaTick(60000))               // func 807
    , m_tick5s(CThreadOperador::GetInst().CriaTick(5000))
    , m_tick1s(CThreadOperador::GetInst().CriaTick(1000))
    , m_textoStatus(std::make_shared<std::string>(" "))
{
    api::CFormBuilderMT campos;
    if (comum::EhTreinamentoEleitor()) {                                     // func 697
        // Voter-training mode ("treinamento de eleitores"): no identification, just count the votes.
        campos.Add<api::CBuzzFieldMT>(51, 10);
        campos.Add<api::CClockFieldMT>(SPoint{33, 1});
        campos.Add<api::CTextFieldMT>(SPoint{1, 1}, std::make_shared<api::CFixedText>(ESQUERDA, "TREINAMENTO DE ELEITORES"));
        campos.Add<api::CTextFieldMT>(SPoint{30, 2}, std::make_shared<api::CFixedText>(ESQUERDA, "Votos:"));
        campos.Add<api::CTextFieldMT>(SPoint{37, 2}, std::make_shared<api::CDataText<std::string (*)()>>(
                                                         ESQUERDA, &TextoQtdVotaram));           // slot 4053
        campos.Add<api::CTextFieldMT>(SPoint{40, 3}, std::make_shared<api::CDataText<std::string (*)()>>(
                                                         DIREITA, &TextoAudio));                 // slot 4054
        campos.Add<api::CTextFieldMT>(SPoint{1, 4}, std::make_shared<api::CFixedText>(ESQUERDA, "CORRIGE: outras opções"));
        // The binary really passes {1, 4} here (not {40, 4} as on the other screens); with DIREITA
        // CWasmScreenMT ignores x and right-aligns to column 40, so the result is the same.
        campos.Add<api::CTextFieldMT>(SPoint{1, 4}, std::make_shared<api::CFixedText>(DIREITA, "CONFIRMA: votar"));
        campos.Add<api::CLedFieldMT>(false);
        campos.AddInputControl();
    } else {
        // "0003/0150": votes so far (live) / voters able to vote in the section (fixed at construction).
        const auto aptos = comum::CEleitores::GetInst().GetQtdAptosSecao();   // comum_f2823
        const std::string totalAptos = std::format("/{:04}", aptos.qtd1 + aptos.qtd2);

        // "Digite o Título ou o CPF" - one entry per identity type accepted by the election.
        std::vector<std::string> tipos;
        for (const auto tipo : comum::CConfiguracaoEleicao::GetInst().GetTiposIdentidadePermitidos())
            tipos.push_back(NomeTipoIdentidadeComArtigo(tipo));
        const std::string digite = std::format("Digite {}", Junta(tipos, " ou "));   // shared_f1696

        campos.Add<api::CBuzzFieldMT>(51, 10);
        campos.Add<api::CClockFieldMT>(SPoint{33, 1});
        campos.Add<api::CTextFieldMT>(SPoint{1, 1}, std::make_shared<api::CFixedText>(ESQUERDA, digite));
        campos.Add<api::CTextFieldMT>(SPoint{32, 2}, std::make_shared<api::CDataText<std::string (*)()>>(
                                                         ESQUERDA, &TextoQtdVotaram));           // slot 4053
        campos.Add<api::CTextFieldMT>(SPoint{36, 2}, std::make_shared<api::CFixedText>(ESQUERDA, totalAptos));
        campos.Add<api::CTextFieldMT>(SPoint{40, 4}, std::make_shared<api::CDataText<std::string (*)()>>(
                                                         DIREITA, &TextoAudio));                 // slot 4054
        // Status line refreshed every 300 ms from m_textoStatus (CTextFieldUpdateMT, 56 bytes;
        // its constructor throws std::invalid_argument("CTextFieldUpdate - campo estava com o texto
        // nulo") on a null text).
        campos.Add<api::CTextFieldUpdateMT>(SPoint{1, 4},
                                            std::make_shared<api::CDataTextFmt<api::CTextSource>>(
                                                ESQUERDA, api::CTextSource(m_textoStatus), "%s"),   // ctextsource.h:37
                                            std::chrono::milliseconds{300});
        campos.Add<api::CLedFieldMT>(false);
        campos.Add<api::CInputFieldMT>(12, /*aguardaConfirma*/ true, SPoint{1, 2});   // func 1151: 12 digits
    }
    m_form = campos.CriaFormInterativo("", true);                            // func 301 -> +24

    impl::IInformacaoThreadOperador::GetInst().SorteiaProximaInspecao();     // func 3620 (slot 17)
}

// wasm func 652 (tools: "api::CTextSource::CTextSource@652"). @1905336, mutex @1905312.
// Callers: CSincronismoOperador (10209), CEleitorVotouNaoVotou (10430), CVerificaDadoEleitor (10443),
// CPedeAnoNascimento (10590), CValidaIdentidade (10626), CPedeTituloEncerramento (10721),
// CEncerramentoAntecipado (10731), CRegistroMesarioEncerrado (10762), and u10/u15 states.
VOTA_LAZY_SINGLETON(CPedeIdentidade, @1905312, @1905336)

// =================================================================================================
// uenux2/src/app/vota/operador/aguardaeleitor/cmostraeleitorvotando.cpp  (path from u10)
// =================================================================================================

// Constructor (inlined into GetInst, func 1150). 28 bytes: +12 m_textoCargo, +20 m_form. See
// cmostraeleitorvotando.h (u10). Line 3 is the voter-side audio indicator (slot 4057 = func 10539),
// not the TTE as the u10 header says.
CMostraEleitorVotando::CMostraEleitorVotando()
    : comum::CAppState(3)                                                    // messages + keys
    , m_textoCargo(std::make_shared<std::string>())
{
    api::CFormBuilderMT campos;
    campos.Add<api::CLedFieldMT>(true);
    if (comum::EhTreinamentoEleitor()) {
        campos.Add<api::CTextFieldMT>(SPoint{1, 1}, std::make_shared<api::CFixedText>(ESQUERDA, "TREINAMENTO DE ELEITORES"));
    } else {
        campos.Add<api::CTextFieldMT>(SPoint{1, 1}, std::make_shared<api::CDataTextFmt<std::string (*)(const std::string&)>>(
                                                        ESQUERDA, &comum::CEleitorDadoNomeParaUrna::Text, "{:35}"));
        campos.Add<api::CTextFieldMT>(SPoint{1, 2}, std::make_shared<api::CDataText<std::string (*)()>>(
                                                        ESQUERDA, &TextoIdentidadeDigitada));      // slot 3907
        campos.Add<api::CTextFieldMT>(SPoint{32, 2}, std::make_shared<api::CFixedText>(ESQUERDA, "Seq:"));
        campos.Add<api::CTextFieldMT>(SPoint{40, 2}, std::make_shared<api::CDataTextFmt<std::string (*)(const std::string&)>>(
                                                         DIREITA, &comum::CEleitorDadoSequencial::Text, "{:04}"));
        campos.Add<api::CTextFieldMT>(SPoint{30, 3}, std::make_shared<api::CFixedText>(ESQUERDA, "Seção:"));
        campos.Add<api::CTextFieldMT>(SPoint{40, 3}, std::make_shared<api::CDataTextFmt<std::string (*)(const std::string&)>>(
                                                         DIREITA, &comum::CEleitorDadoSecao::Text, "{:04}"));
    }
    campos.Add<api::CTextFieldMT>(SPoint{1, 3}, std::make_shared<api::CDataText<std::string (*)()>>(
                                                    ESQUERDA, &TextoAudioEleitor));                // slot 4057
    campos.Add<api::CTextFieldMT>(SPoint{1, 4}, std::make_shared<api::CDataText<api::CTextSource>>(
                                                    ESQUERDA, api::CTextSource(m_textoCargo)));    // func 5409
    m_form = campos.CriaForm("");                                            // func 1694 -> +20
}

// wasm func 1150 (tools: "api::CTextSource::CTextSource@1150"). @1909316, mutex @1909292.
VOTA_LAZY_SINGLETON(CMostraEleitorVotando, @1909292, @1909316)

// =================================================================================================
// uenux2/src/app/vota/operador/confirmaidentidade/cnomeeleitor.cpp  (path from u10)
// =================================================================================================

// Constructor (inlined into GetInst, func 5401). 20 bytes: +12 m_form.
CNomeEleitor::CNomeEleitor()
    : comum::CAppState(2)                                                    // keys
{
    api::CFormBuilderMT campos;
    campos.Add<api::CLedFieldMT>(false);
    campos.Add<api::CTextFieldMT>(SPoint{1, 1}, std::make_shared<api::CDataTextFmt<std::string (*)(const std::string&)>>(
                                                    ESQUERDA, &comum::CEleitorDadoNomeParaUrna::Text, "{:2}"));
    campos.Add<api::CTextFieldMT>(SPoint{1, 2}, std::make_shared<api::CDataText<std::string (*)()>>(
                                                    ESQUERDA, &TextoIdentidadeDigitada));
    campos.Add<api::CTextFieldMT>(SPoint{32, 2}, std::make_shared<api::CFixedText>(ESQUERDA, "Seq:"));
    campos.Add<api::CTextFieldMT>(SPoint{40, 2}, std::make_shared<api::CDataTextFmt<std::string (*)(const std::string&)>>(
                                                     DIREITA, &comum::CEleitorDadoSequencial::Text, "{:04}"));
    campos.Add<api::CTextFieldMT>(SPoint{30, 3}, std::make_shared<api::CFixedText>(ESQUERDA, "Seção:"));
    campos.Add<api::CTextFieldMT>(SPoint{40, 3}, std::make_shared<api::CDataTextFmt<std::string (*)(const std::string&)>>(
                                                     DIREITA, &comum::CEleitorDadoSecao::Text, "{:04}"));
    campos.Add<api::CTextFieldMT>(SPoint{1, 3}, std::make_shared<api::CDataText<std::string (*)()>>(
                                                    ESQUERDA, &comum::CEleitorDadoTTE::Text));   // slot 4113
    campos.Add<api::CTextFieldMT>(SPoint{1, 4}, std::make_shared<api::CFixedText>(ESQUERDA, "CORRIGE: cancelar"));
    campos.Add<api::CTextFieldMT>(SPoint{40, 4}, std::make_shared<api::CFixedText>(DIREITA, "CONFIRMA: prosseguir"));
    campos.AddInputControl();
    m_form = campos.CriaFormInterativo("telaNomeEleitor", false);            // keyboard NOT flushed
}

// wasm func 5401. @1908840, mutex @1908816. Callers: CEleitorEncontrado::StartState (10635),
// CPedeIdentidade::ProcessInput (10677).
VOTA_LAZY_SINGLETON(CNomeEleitor, @1908816, @1908840)

// =================================================================================================
// uenux2/src/app/vota/operador/aguardaeleitor/csincronismooperador.cpp  (path inferred)
// "Sincronismo operador": waits for the voter thread to finish recording the vote.
// =================================================================================================

// wasm func 5341 (tools: vota_f5341). Shows a recording-failure screen on the MT.   name inferred
// tipo 1 = the external medium failed but the vote was recorded; 2 = the internal one failed.
// The wasm function has a single i32 parameter (tipo) and no `this`: a static member or a
// file-local function (declare it `static` in the class if it is kept as a member).
void CSincronismoOperador::MostraFalhaGravacao(int tipo)
{
    api::CFormBuilderMT campos;
    campos.Add<api::CLedFieldMT>(2);                                          // func 435 with 2 (?)
    if (tipo == 1) {
        campos.Add<api::CTextFieldMT>(SPoint{20, 1}, std::make_shared<api::CFixedText>(CENTRO, "FALHA NA ME: SUBSTITUA A ME"));
        campos.Add<api::CTextFieldMT>(SPoint{20, 3}, std::make_shared<api::CFixedText>(CENTRO, "ÚLTIMO VOTO FOI COMPUTADO"));
    } else {
        campos.Add<api::CTextFieldMT>(SPoint{20, 1}, std::make_shared<api::CFixedText>(CENTRO, "FALHA NA MI: SUBSTITUA A URNA"));
        campos.Add<api::CTextFieldMT>(SPoint{20, 3}, std::make_shared<api::CFixedText>(CENTRO, "ELEITOR DEVE VOTAR NOVAMENTE"));
    }
    campos.CriaForm("")->Show();                                             // func 1694, form slot 2
}

// wasm func 10209 - vtable slot 6 (ProcessMessage). +11 = m_suspensaoAutomatica (set by
// CMostraEleitorVotando / CSuspensaoAutomaticaEleitor, see u10).
void CSincronismoOperador::ProcessMessage(uebyte mensagem)
{
    switch (mensagem) {
    case 1:                                                                  // FimVotoEleitor
        if (m_suspensaoAutomatica) {
            if (!comum::EhTreinamentoEleitor()) {
                auto& tela = CEleitorVotouNaoVotou::GetInst();               // func 1901
                tela.m_votou = true;
                m_proximoEstado = &tela;
            } else {
                m_proximoEstado = &CPedeIdentidade::GetInst();
            }
        } else if (CInformacaoEleitor::GetInst().m_modoAudio != 2) {        // func 509: 2 = sem áudio
            m_proximoEstado = &CDesabilitaAudioEleitor::GetInst();           // "Retire o fone ..."
        } else {
            m_proximoEstado = &CPedeIdentidade::GetInst();
        }
        break;
    case 14:                                                                 // failure writing the MI
        MostraFalhaGravacao(2);
        m_proximoEstado = this;
        break;
    case 15:                                                                 // failure writing the ME
        MostraFalhaGravacao(1);
        m_proximoEstado = this;
        break;
    default:
        break;
    }
}

// =================================================================================================
// uenux2/src/app/vota/operador/aguardaeleitor/cdesabilitaaudioeleitor.cpp  (path inferred)
// =================================================================================================

// Constructor (inlined into GetInst, func 5393). 20 bytes: +12 m_form.
CDesabilitaAudioEleitor::CDesabilitaAudioEleitor()
    : comum::CAppState(2)
{
    api::CFormBuilderMT campos;
    campos.Add<api::CBuzzFieldMT>(51, 10);
    campos.Add<api::CClockFieldMT>(SPoint{33, 1});
    campos.Add<api::CTextFieldMT>(SPoint{1, 2}, std::make_shared<api::CFixedText>(ESQUERDA, "Retire o fone de ouvido da urna"));
    campos.Add<api::CTextFieldMT>(SPoint{40, 4}, std::make_shared<api::CFixedText>(DIREITA, "CONFIRMA"));
    campos.AddInputControl();
    m_form = campos.CriaFormInterativo("", true);
}

// wasm func 5393. @1909260, mutex @1909236. Callers: 10209, 10430.
VOTA_LAZY_SINGLETON(CDesabilitaAudioEleitor, @1909236, @1909260)

// =================================================================================================
// uenux2/src/app/vota/operador/aguardaeleitor/celeitorvotounaovotou.cpp  (path from u10)
// =================================================================================================

// wasm func 10430 - vtable slot 7 (ProcessInput). m_form at +28.
void CEleitorVotouNaoVotou::ProcessInput()
{
    if (m_form->Read() != CONFIRMA)                                          // cinteractiveform.h:57
        return;
    if (CInformacaoEleitor::GetInst().m_modoAudio != 2)
        m_proximoEstado = &CDesabilitaAudioEleitor::GetInst();
    else
        m_proximoEstado = &CPedeIdentidade::GetInst();
}

// =================================================================================================
// uenux2/src/app/vota/operador/aguardaeleitor/celeitordemorando.cpp  (path from u10)
// "O eleitor está demorando": the voter thread reported inactivity (message 3/4).
// =================================================================================================

// wasm func 10440 - vtable slot 2 (StartState). +11 m_votouParcialmente, +12 m_textoSituacao,
// +20 m_textoNome (u10's name; it receives CThreadOperador +108, the "VOTANDO PARA" cargo text),
// +28 m_form.
void CEleitorDemorando::StartState()
{
    if (comum::EhTreinamentoEleitor()) {
        // Voter training: suspend the voter automatically (constructor inlined here, see
        // csuspensaoautomaticaeleitor.h of u10; @1909400, mutex @1909376).
        auto& suspensao = CSuspensaoAutomaticaEleitor::GetInst();
        suspensao.m_votouParcialmente = m_votouParcialmente;
        m_proximoEstado = &suspensao;
        return;
    }
    *m_textoSituacao = m_votouParcialmente ? "Votou parcialmente" : "Não votou";
    CLogVota::GetInst().Loga(1, std::format("Eleitor sem atividade por {} segundos",
                                         unsigned{comum::CConfiguracaoEleicao::GetInst().m_tempoInatividade}));  // +168 byte
    m_proximoEstado = this;
    *m_textoNome = CThreadOperador::GetInst().m_textoCargo;                  // +108
    m_form->Show();                                                          // form slot 2
}

// wasm func 10439 - vtable slot 7 (ProcessInput)
void CEleitorDemorando::ProcessInput()
{
    if (m_form->Read() == CONFIRMA)
        m_proximoEstado = &CPerguntaEleitorVotando::GetInst();
}

// =================================================================================================
// uenux2/src/app/vota/operador/aguardaeleitor/cperguntaeleitorvotando.cpp  (path inferred)
// "O eleitor ainda está votando?"
// =================================================================================================

// Constructor (inlined into GetInst, inside func 10439). 20 bytes: +12 m_form.
CPerguntaEleitorVotando::CPerguntaEleitorVotando()
    : comum::CAppState(3)
{
    api::CFormBuilderMT campos;
    campos.Add<api::CLedFieldMT>(true);
    campos.Add<api::CBuzzFieldMT>(51, 10);
    campos.Add<api::CTextFieldMT>(SPoint{1, 1}, std::make_shared<api::CDataText<std::string (*)()>>(
                                                    ESQUERDA, &TextoIdentidadeDigitada));
    campos.Add<api::CTextFieldMT>(SPoint{1, 2}, std::make_shared<api::CFixedText>(ESQUERDA, "O eleitor ainda está votando?"));
    campos.Add<api::CTextFieldMT>(SPoint{1, 4}, std::make_shared<api::CFixedText>(ESQUERDA, "CORRIGE: não"));
    campos.Add<api::CTextFieldMT>(SPoint{40, 4}, std::make_shared<api::CFixedText>(DIREITA, "CONFIRMA: sim"));
    campos.AddInputControl();
    m_form = campos.CriaFormInterativo("", true);
}
VOTA_LAZY_SINGLETON(CPerguntaEleitorVotando, @1909348, @1909372)   // inlined in 10439

// wasm func 10415 - vtable slot 7 (ProcessInput)
void CPerguntaEleitorVotando::ProcessInput()
{
    switch (m_form->Read()) {
    case CORRIGE:                                                            // "no": suspend
        CLogVota::GetInst().Loga(2, "Eleitor não está votando");            // api_f1398 (level 2)
        m_proximoEstado = &CPerguntaCodigoSuspensao::GetInst();              // inlined, below
        break;
    case CONFIRMA:                                                           // "yes": back to voting
        CLogVota::GetInst().Loga(1, "Eleitor está votando");                // api_f233 (level 1)
        CThreadEleitor::GetInst().EnviaMensagem({EMensagemEleitor::ContinuaVotacao}, 1);   // 4, +36 queue
        m_proximoEstado = &CMostraEleitorVotando::GetInst();                 // func 1150
        break;
    default:
        break;
    }
}

// uenux2/src/app/vota/operador/aguardaeleitor/cperguntacodigosuspensao.cpp  (path inferred)
// Constructor (inlined into GetInst, inside func 10415). 32 bytes: +12 m_form (título), +20
// m_formInvalido, +28 m_tentativas (?) = 0. StartState/ProcessInput: funcs 10419/10420 (other unit).
CPerguntaCodigoSuspensao::CPerguntaCodigoSuspensao()
    : comum::CAppState(3)
    , m_tentativas(0)
{
    {
        api::CFormBuilderMT campos;
        campos.Add<api::CClockFieldMT>(SPoint{33, 1});
        campos.Add<api::CTextFieldMT>(SPoint{1, 1}, std::make_shared<api::CFixedText>(ESQUERDA, "Informe seu título para"));
        campos.Add<api::CTextFieldMT>(SPoint{1, 2}, std::make_shared<api::CFixedText>(ESQUERDA, "suspender a votação"));
        campos.Add<api::CTextFieldMT>(SPoint{1, 4}, std::make_shared<api::CFixedText>(ESQUERDA, "CORRIGE: retornar"));
        campos.Add<api::CTextFieldMT>(SPoint{40, 4}, std::make_shared<api::CFixedText>(DIREITA, "CONFIRMA: suspender"));
        campos.Add<api::CInputFieldMT>(12, true, SPoint{1, 3});
        m_form = campos.CriaFormInterativo("", true);
    }
    {
        api::CFormBuilderMT campos;
        campos.Add<api::CClockFieldMT>(SPoint{33, 1});
        campos.Add<api::CTextFieldMT>(SPoint{1, 1}, std::make_shared<api::CFixedText>(ESQUERDA, "Título inválido para"));
        campos.Add<api::CTextFieldMT>(SPoint{1, 2}, std::make_shared<api::CFixedText>(ESQUERDA, "suspender a votação"));
        m_formInvalido = campos.CriaFormInterativo("", true);
    }
}
VOTA_LAZY_SINGLETON(CPerguntaCodigoSuspensao, @1909320, @1909344)   // inlined in 10415

// =================================================================================================
// uenux2/src/app/vota/operador/confirmaidentidade/cverificadadoeleitor.cpp  (path from u10)
// Birth-year check after the last failed fingerprint attempt (constructor: func 2735, u10).
// =================================================================================================

// wasm func 10443 - vtable slot 7 (ProcessInput). m_form at +12.
void CVerificaDadoEleitor::ProcessInput()
{
    switch (m_form->Read()) {
    case CORRIGE:
        CLogVota::GetInst().Loga("Habilitação cancelada durante confirmação de dado do eleitor");   // func 2097
        m_proximoEstado = &CPedeIdentidade::GetInst();
        break;
    case CONFIRMA: {
        const std::string ano = m_form->GetEntrada(0).GetTexto();            // m_entradas.at(0) +24
        if (ano.size() <= 3) {
            CLogVota::GetInst().Loga(1, "Dado digitado está incompleto");
            break;                                                           // stay
        }
        const unsigned long anoDigitado = std::stoul(ano);
        const auto anoCadastro =
            comum::CEleitores::GetInst().GetCurrent().GetEleitor().GetAnoNascimento();   // func 5665
        if (anoCadastro == anoDigitado) {
            CLogVota::GetInst().Loga(1, "Dado digitado confere");
            m_proximoEstado = &CRegistraDigitalOperador::GetInst();          // func 5396
        } else {
            CLogVota::GetInst().Loga(1, "Dado digitado não confere");
            m_proximoEstado = &CDadoEleitorNaoConfere::GetInst();            // inlined, below
        }
        break;
    }
    default:
        break;
    }
}

// uenux2/src/app/vota/operador/confirmaidentidade/cdadoeleitornaoconfere.cpp  (path inferred)
// Constructor (inlined into GetInst, inside func 10443). 28 bytes: +12 m_formTentarNovamente,
// +20 m_formCancelar. Methods: 10447 (other unit).
CDadoEleitorNaoConfere::CDadoEleitorNaoConfere()
    : comum::CAppState(2)
{
    {
        api::CFormBuilderMT campos;
        campos.Add<api::CLedFieldMT>(false);
        campos.Add<api::CTextFieldMT>(SPoint{1, 1}, std::make_shared<api::CFixedText>(ESQUERDA, "Ano de nascimento não confere. Por favor"));
        campos.Add<api::CTextFieldMT>(SPoint{1, 2}, std::make_shared<api::CFixedText>(ESQUERDA, "pergunte novamente ao eleitor o ano de"));
        campos.Add<api::CTextFieldMT>(SPoint{1, 3}, std::make_shared<api::CFixedText>(ESQUERDA, "nascimento dele"));
        campos.Add<api::CTextFieldMT>(SPoint{40, 4}, std::make_shared<api::CFixedText>(DIREITA, "CONFIRMA: tentar novamente"));
        campos.AddInputControl();
        m_formTentarNovamente = campos.CriaFormInterativo("", true);
    }
    {
        api::CFormBuilderMT campos;
        campos.Add<api::CLedFieldMT>(false);
        campos.Add<api::CTextFieldMT>(SPoint{1, 1}, std::make_shared<api::CFixedText>(ESQUERDA, "Por favor oriente o eleitor a procurar o"));
        campos.Add<api::CTextFieldMT>(SPoint{1, 2}, std::make_shared<api::CFixedText>(ESQUERDA, "cartorio eleitoral para consultar a data"));
        campos.Add<api::CTextFieldMT>(SPoint{1, 3}, std::make_shared<api::CFixedText>(ESQUERDA, "de nascimento dele no cadastro da urna"));
        campos.Add<api::CTextFieldMT>(SPoint{40, 4}, std::make_shared<api::CFixedText>(DIREITA, "CONFIRMA: cancelar a habilitacao"));
        campos.AddInputControl();
        m_formCancelar = campos.CriaFormInterativo("", true);
    }
}
VOTA_LAZY_SINGLETON(CDadoEleitorNaoConfere, @1909152, @1909176)   // inlined in 10443

// =================================================================================================
// uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp  (srclocs of 5 methods)
// The mesário registers his own fingerprint to release a voter who could not be identified by
// fingerprint (habilitação "por código do mesário").
// =================================================================================================

// Constructor (inlined into GetInst, func 5396). 36 bytes: +11 m_tickTimeout (15 s), +12 m_tickLeitura
// (50 ms), +16 m_formCaptura, +24 m_formCapturada, +32 m_tentativasRestantes = 3.  names inferred
CRegistraDigitalOperador::CRegistraDigitalOperador()
    : comum::CAppState(6)
    , m_tickTimeout(CThreadOperador::GetInst().CriaTick(15000))
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

// wasm func 5396 (tools: api_f5396). @1909148, mutex @1909124. Callers: 10443, CDigitalNaoCapturada (10458).
VOTA_LAZY_SINGLETON(CRegistraDigitalOperador, @1909124, @1909148)

// =================================================================================================
// uenux2/src/app/vota/operador/justificativa/cpedeanonascimento.cpp  (path inferred; the siblings
// iiniciajustificativa.cpp / iconfirmajustificativa.cpp are attested)
// Justification of absence ("justificativa"): a voter of another section declares he cannot vote.
// The mesário typed the título before (IInformacaoThreadOperador slot 25); here he types the birth year.
// =================================================================================================

// wasm func 10590 - vtable slot 7 (ProcessInput). m_form at +12.
void CPedeAnoNascimento::ProcessInput()
{
    switch (m_form->Read()) {
    case CORRIGE:
        CLogVota::GetInst().Loga(1, "Mesário cancelou entrada dos dados");
        m_proximoEstado = &CPedeIdentidade::GetInst();
        return;
    case CONFIRMA:
        break;
    default:
        return;
    }

    const std::string ano = m_form->GetEntrada(0).GetTexto();
    if (ano.size() < 4)
        return;                                                              // stay
    CThreadOperador::GetInst().m_anoNascimentoDigitado = ano;                // +84

    const short anoNascimento = ecourna::api::util::CStringUtils::ToInt16(ano);
    // ushort at CConfiguracaoEleicao +48 (i32.load16_u offset=48): the year of the pleito date
    // (CPleito at +28, its CDate at +44). Only years are compared (no day/month).
    const short idade = static_cast<short>(comum::CConfiguracaoEleicao::GetInst().m_anoEleicao - anoNascimento);  // +48
    if (anoNascimento < 1900 || idade <= 0) {
        CLogVota::GetInst().Loga(2, "Ano de nascimento inválido");
        m_proximoEstado = &CAnoInformadoInvalido::GetInst();                 // inlined, below
        return;
    }
    if (static_cast<unsigned short>(idade) <= 15) {                          // minimum age 16 (by year)
        CLogVota::GetInst().Loga(2, "Eleitor não tem idade mínima");
        m_proximoEstado = &CEleitorMenor16Anos::GetInst();                   // inlined, below
        return;
    }

    // --- record the justification -------------------------------------------------------------
    auto& justificador = comum::CJustificador::GetInst();                      // func 1391
    // DANGLING REFERENCE (as compiled): slot 25 returns a comum::md::CEleitorIdentidade temporary
    // (16 bytes: std::string +0, tipo +12). The binary destroys it (frees the 12-digit string,
    // which is heap-allocated with libc++'s 10-char SSO) right after the call, and only then passes
    // the same object to CNumeroInscricaoEleitoral(const std::string&) (func 1241). That is the code
    // of a reference bound to a member of a temporary, e.g.:
    const std::string& titulo =
        impl::IInformacaoThreadOperador::GetInst().GetIdentidadeEleitor().GetIdentidade();   // api_f1535
    // IIniciaJustificativa::StartState (10587) makes a copy instead (no dangling there).
    justificador.Justifica(ecourna::app::dados::CNumeroInscricaoEleitoral(titulo),       // reads freed memory
                           static_cast<ueword>(anoNascimento));
    //   ^ inlined, cjustificador.cpp:55: if the título is already in the map -> make it current and
    //     throw CBaseError<EUeComumJustificativaError>(8800, "Já havia justificativa para o título");
    //     else CDataMap<CNumeroInscricaoEleitoral, CJustificadorDetalhe>::Add({titulo, ano}).
    comum::GetEstadoGeralVota().m_qtdJustificativas++;                         // GetEstado<51> +54 (short)

    SincronizaJustificativa(justificador);   // inlined, below: the shutdown check comes first, before
                                             // any MI/MV write (but after Justifica and the counter)

    auto& efetuada = CJustificativaEfetuada::GetInst();                       // func 2747 ("justificou/ja justificou")
    efetuada.m_novaJustificativa = true;                                      // +28          name inferred
    m_proximoEstado = &efetuada;
}

// Inlined into 10590 (probably vota::CSincronizaVota, csincronizavota.cpp).        name inferred
// Same MI -> MV discipline as the vote (u07 csincronizavota.u07.cpp).
void CPedeAnoNascimento::SincronizaJustificativa(comum::CJustificador& justificador)
{
    if (CSincronizaVota::ms_desligando)                                       // byte @1832936
        throw CSincronizaVota::VerificaUrnaDesligando();                      // api::CUeDesligandoError
    {
        api::CApplicationContextGuard contexto(2, "", "Gravando a justificativa na MI",
                                               "Ocorreu um erro durante a sincronização da justificativa na MI.");
        comum::CAppInfo::GetInst().SalvaVotaInterno();                        // func 3790 (vota.bin, MI)
        justificador.SaveCurrentInternal();
        //   ^ inlined, cjustificador.cpp:83: if there is no current entry throw (8801) "Não há dados a
        //     serem salvos na MI"; builds CRegistroIdentificacaoEleitor(make_shared<CNumeroInscricao
        //     Eleitoral>(titulo)) + CBaseType<0,9999>(ano) (shared_f1875) and inserts it through the DAO
        //     (+32, slot 3) unless the DAO already has it (slot 7): table registro_justificativa of uenux.db.
        comum::CAssinador assinador(TurnoUm() ? 122 : 123);                   // func 1501
        comum::AssinarUE(assinador, 31);                                      // vota.bin signature
        api::CSynchronizer::CreateInst()->Sincroniza();                       // shared_f620
    }                                                                          // func 675
    comum::GravaBancoDadosNaMI();                                              // func 4657 (u02 name)
    {
        api::CApplicationContextGuard contexto(4, "", "Gravando a justificativa na MV",
                                               "Ocorreu um erro durante a sincronização da justificativa na MV.");
        comum::CAppInfo::GetInst().SalvaVotaExterno();                        // func 3789 (vota.bin, MV)
        comum::CopiaAssinaturaVotaParaMV();   // func 4687: CArquivosSavd 122 -> 124 (turno 1) or 123 -> 125
        api::CSynchronizer::CreateInst()->Sincroniza();                       //   (vota.vsu MI -> MV)
    }
}

// uenux2/src/app/vota/operador/justificativa/canoinformadoinvalido.cpp  (path inferred)
CAnoInformadoInvalido::CAnoInformadoInvalido()                               // 20 bytes: +12 m_form
    : comum::CAppState(2)
{
    api::CFormBuilderMT campos;
    campos.Add<api::CLedFieldMT>(false);
    campos.Add<api::CTextFieldMT>(SPoint{1, 1}, std::make_shared<api::CFixedText>(ESQUERDA, "Ano de nascimento inválido."));
    campos.Add<api::CTextFieldMT>(SPoint{40, 4}, std::make_shared<api::CFixedText>(DIREITA, "CONFIRMA: tentar novamente"));
    campos.AddInputControl();
    m_form = campos.CriaFormInterativo("", true);
}
VOTA_LAZY_SINGLETON(CAnoInformadoInvalido, @1905872, @1905896)   // inlined in 10590

// uenux2/src/app/vota/operador/justificativa/celeitormenor16anos.cpp  (path inferred)
CEleitorMenor16Anos::CEleitorMenor16Anos()                                   // 20 bytes: +12 m_form
    : comum::CAppState(2)
{
    api::CFormBuilderMT campos;
    campos.Add<api::CLedFieldMT>(false);
    campos.Add<api::CTextFieldMT>(SPoint{1, 1}, std::make_shared<api::CFixedText>(ESQUERDA, "Eleitor não pode votar ou justificar"));
    campos.Add<api::CTextFieldMT>(SPoint{1, 2}, std::make_shared<api::CFixedText>(ESQUERDA, "por não ter idade mínima"));
    campos.Add<api::CTextFieldMT>(SPoint{40, 4}, std::make_shared<api::CFixedText>(DIREITA, "CONFIRMA: tentar novamente"));
    campos.AddInputControl();
    m_form = campos.CriaFormInterativo("", true);
}
VOTA_LAZY_SINGLETON(CEleitorMenor16Anos, @1905900, @1905924)   // inlined in 10590

// =================================================================================================
// uenux2/src/app/vota/operador/leidentidade/cvalidaidentidade.cpp  (path inferred)
// The typed number is valid for several identity types: the mesário chooses the type from a menu.
// =================================================================================================

struct SOpcaoIdentidade {                                                    // map value, name inferred
    std::string texto;                                                        // "Título de eleitor" ...
    comum::md::ETipoIdentidade tipo;
};
using TOpcoesIdentidade = std::map<int, SOpcaoIdentidade>;                    // key = menu number 1..n

// wasm func 5420 (tools: vota_f5420)                                                name inferred
// Numbers the accepted types 1, 2, 3 ... in the order given; unknown types are skipped (and do not
// consume a number). Wasm signature (sret map, const vector&): no `this`, so static.
TOpcoesIdentidade CValidaIdentidade::MontaOpcoes(const std::vector<comum::md::ETipoIdentidade>& tipos)
{
    TOpcoesIdentidade opcoes;
    int numero = 1;
    for (const auto tipo : tipos) {
        switch (static_cast<int>(tipo)) {
        case 1: opcoes.try_emplace(numero++, SOpcaoIdentidade{"Título de eleitor", tipo}); break;
        case 2: opcoes.try_emplace(numero++, SOpcaoIdentidade{"CPF", tipo}); break;
        case 3: opcoes.try_emplace(numero++, SOpcaoIdentidade{"Identificador", tipo}); break;
        default: break;
        }
    }
    return opcoes;
}

// wasm func 10627 - vtable slot 2 (StartState). +12 m_form, +20 m_opcoes (TOpcoesIdentidade).
void CValidaIdentidade::StartState()
{
    const std::string identidade = impl::IInformacaoThreadOperador::GetInst().GetIdentidadeDigitada();   // slot 23
    std::vector<comum::md::ETipoIdentidade> tipos;                           // types that accept the number
    for (const auto* validador : comum::md::CValidadorIdentidade::GetInst().GetValidadores())
        if (validador->Valida(identidade))                                   // vota_f3723
            tipos.push_back(validador->GetTipo());                           // slot 2

    switch (tipos.size()) {
    case 0:
        m_proximoEstado = &CIdentidadeInvalida::GetInst();                   // inlined, below
        return;
    case 1: {
        auto opcoes = MontaOpcoes(tipos);
        CLogVota::GetInst().Loga(1, std::format("Identificador digitado pelo mesário foi: ({})", opcoes[1].texto));
        impl::IInformacaoThreadOperador::GetInst().SetTipoIdentidadeDigitada(tipos.front());   // slot 22 (api_f3619)
        m_proximoEstado = &CProcuraEleitor::GetInst();                       // func 5421
        return;
    }
    default:
        break;
    }

    // Several types: menu "1-Título de eleitor / 2-CPF / 3-Identificador".
    m_proximoEstado = this;
    m_opcoes = MontaOpcoes(tipos);
    api::CFormBuilderMT campos;
    campos.Add<api::CBuzzFieldMT>(51, 10);
    campos.Add<api::CClockFieldMT>(SPoint{33, 1});
    campos.Add<api::CTextFieldMT>(SPoint{1, 1}, std::make_shared<api::CFixedText>(ESQUERDA, "Tipo de identidade digitada"));
    // Line numero + 1: a 3rd option ("3-Identificador", 15 chars) would be at {1,4}, the same point
    // as "CORRIGE: retornar" (17 chars), which is added later and so drawn over it.
    for (const auto& [numero, opcao] : m_opcoes)
        campos.Add<api::CTextFieldMT>(SPoint{1, static_cast<api::TPosition>(numero + 1)},
                                      std::make_shared<api::CFixedText>(ESQUERDA, std::format("{}-{}", numero, opcao.texto)));
    campos.Add<api::CLedFieldMT>(false);
    campos.Add<api::CTextFieldMT>(SPoint{24, 4}, std::make_shared<api::CFixedText>(ESQUERDA, "ESCOLHA O TIPO:"));
    campos.Add<api::CInputFieldMT>(1, false, SPoint{40, 4});
    campos.Add<api::CTextFieldMT>(SPoint{1, 4}, std::make_shared<api::CFixedText>(ESQUERDA, "CORRIGE: retornar"));
    m_form = campos.CriaFormInterativo("", true);
    m_form->Show();
}

// wasm func 10626 - vtable slot 7 (ProcessInput)
void CValidaIdentidade::ProcessInput()
{
    switch (m_form->Read()) {
    case CORRIGE:
        CLogVota::GetInst().Loga("Habilitação cancelada durante confirmação de dado do eleitor");   // func 2097
        m_proximoEstado = &CPedeIdentidade::GetInst();
        break;
    case CONFIRMA:
        // Only a non-null result is stored (func 10626: `local.tee; i32.eqz; br_if` skips the store):
        // an empty or unknown choice leaves m_proximoEstado = this (set by StartState), i.e. the
        // menu stays on the MT. Same pattern as CPedeTituloEncerramento::ProcessInput.
        if (auto* proximo = ProcessaEscolha())
            m_proximoEstado = proximo;
        break;
    default:
        break;
    }
}

// Inlined into 10626.                                                                name inferred
comum::CAppState* CValidaIdentidade::ProcessaEscolha()
{
    const std::string escolha = m_form->GetEntrada(0).GetTexto();
    if (escolha.size() != 1)
        return nullptr;
    const auto it = m_opcoes.find(std::stoi(escolha));
    if (it == m_opcoes.end()) {
        CLogVota::GetInst().Loga(1, "Identificador inválido para o tipo informado");
        return nullptr;
    }
    CLogVota::GetInst().Loga(1, std::format("Identificador digitado pelo mesário foi: ({})", it->second.texto));
    impl::IInformacaoThreadOperador::GetInst().SetTipoIdentidadeDigitada(it->second.tipo);
    return &CProcuraEleitor::GetInst();
}

// uenux2/src/app/vota/operador/leidentidade/cidentidadeinvalida.cpp  (path inferred)
CIdentidadeInvalida::CIdentidadeInvalida()                                   // 20 bytes: +12 m_form
    : comum::CAppState(2)
{
    api::CFormBuilderMT campos;
    campos.Add<api::CBeepFieldMT>(2);
    campos.Add<api::CTextFieldMT>(SPoint{1, 1}, std::make_shared<api::CFixedText>(ESQUERDA, "Identidade:"));
    campos.Add<api::CTextFieldMT>(SPoint{13, 1}, std::make_shared<api::CDataText<std::string (*)()>>(
                                                     ESQUERDA, &IdentidadeDigitada));             // slot 3860
    campos.Add<api::CTextFieldMT>(SPoint{1, 2}, std::make_shared<api::CFixedText>(ESQUERDA, "Número errado"));
    campos.Add<api::CTextFieldMT>(SPoint{1, 4}, std::make_shared<api::CFixedText>(ESQUERDA, "CORRIGE: retornar"));
    campos.AddInputControl();
    m_form = campos.CriaFormInterativo("", true);
}
VOTA_LAZY_SINGLETON(CIdentidadeInvalida, @1905340, @1905364)   // inlined in 10627

// uenux2/src/app/vota/operador/leidentidade/cprocuraeleitor.cpp  (path inferred)
// wasm func 5421 (tools: api_f5421): CProcuraEleitor has no own members; its accessor uses the merged
// singleton body vota_f764(mutex @1905620, instance @1905644, vtable @1589016, flags 6).
// CProcuraEleitor& CProcuraEleitor::GetInst();

// =================================================================================================
// uenux2/src/app/vota/operador/outrasopcoes/cpedetituloencerramento.cpp  (path inferred; sibling
// cencerramentohorarioinvalido.cpp is attested). Operator's encerramento ("closing the vote").
// Constructor + GetInst: func 3631 (other unit): "Informe seu título para / encerrar a votação",
// 12-digit input, CORRIGE: retornar, CONFIRMA: prosseguir; +20 = the scheduled closing time.
// =================================================================================================

// wasm func 10722 - vtable slot 2 (StartState). +12 m_form, +20 m_horarioEncerramento (CDateTime).
void CPedeTituloEncerramento::StartState()
{
    m_proximoEstado = this;
    if (comum::EhTreinamentoEleitor()) {
        // Voter training: no título is asked.
        const api::CDateTime agora = api::CDateTime::Now();                  // api_f479
        if ((m_horarioEncerramento <=> agora) > 0)                           // vota_f759
            m_proximoEstado = &CEncerramentoAntecipado::GetInst();           // inlined, below
        else
            m_proximoEstado = &CConfirmaEncerramento::GetInst();             // func 3632
        return;
    }
    m_form->Show();
}

// wasm func 10721 - vtable slot 7 (ProcessInput)
void CPedeTituloEncerramento::ProcessInput()
{
    switch (m_form->Read()) {
    case CORRIGE:
        CLogVota::GetInst().Loga(1, "Operador cancelou o encerramento da votação");
        m_proximoEstado = &CPedeIdentidade::GetInst();
        break;
    case CONFIRMA:
        if (auto* proximo = ValidaTitulo())                                  // inlined
            m_proximoEstado = proximo;
        break;
    default:
        break;
    }
}

// Inlined into 10721.                                                                name inferred
comum::CAppState* CPedeTituloEncerramento::ValidaTitulo()
{
    const std::string titulo = m_form->GetEntrada(0).GetTexto();
    if (titulo.empty())
        return nullptr;
    CThreadOperador::GetInst().m_tituloEncerramento = titulo;                // +120
    const bool soDigitos = std::all_of(titulo.begin(), titulo.end(), [](char c) { return c >= '0' && c <= '9'; });
    if (!soDigitos ||
        !comum::md::CValidadorIdentidade::GetInst().Valida(comum::md::ETipoIdentidade(1),   // vota_f2803
                                                           std::format("{:0>{}}", titulo, 12)))
        return &CTituloEncerramentoInvalido::GetInst();                      // inlined, below
    CLogVota::GetInst().Loga(1, std::format("Título digitado para encerramento: {}", titulo));
    return &CConfirmaEncerramento::GetInst();
}

// uenux2/src/app/vota/operador/outrasopcoes/ctituloencerramentoinvalido.cpp  (path inferred)
CTituloEncerramentoInvalido::CTituloEncerramentoInvalido()                   // 20 bytes: +12 m_form
    : comum::CAppState(2)
{
    api::CFormBuilderMT campos;
    campos.Add<api::CBeepFieldMT>(2);
    campos.Add<api::CTextFieldMT>(SPoint{1, 1}, std::make_shared<api::CFixedText>(ESQUERDA, "Título inválido: "));
    campos.Add<api::CTextFieldMT>(SPoint{18, 1}, std::make_shared<api::CDataText<std::string (*)()>>(
                                                     ESQUERDA, &TextoTituloEncerramento));        // slot 3781
    campos.Add<api::CTextFieldMT>(SPoint{1, 4}, std::make_shared<api::CFixedText>(ESQUERDA, "CORRIGE: tentar novamente"));
    campos.AddInputControl();
    m_form = campos.CriaFormInterativo("", true);
}
VOTA_LAZY_SINGLETON(CTituloEncerramentoInvalido, @1905088, @1905112)   // inlined in 10721

// uenux2/src/app/vota/operador/outrasopcoes/cencerramentoantecipado.cpp  (path inferred)
CEncerramentoAntecipado::CEncerramentoAntecipado()                           // 20 bytes: +12 m_form
    : comum::CAppState(2)
{
    api::CFormBuilderMT campos;
    campos.Add<api::CLedFieldMT>(false);
    campos.Add<api::CClockFieldMT>(SPoint{33, 1});
    campos.Add<api::CTextFieldMT>(SPoint{1, 2}, std::make_shared<api::CFixedText>(CENTRO, "Encerramento de votação antecipado?"));
    campos.Add<api::CTextFieldMT>(SPoint{1, 4}, std::make_shared<api::CFixedText>(ESQUERDA, "CORRIGE: retornar"));
    campos.Add<api::CTextFieldMT>(SPoint{40, 4}, std::make_shared<api::CFixedText>(DIREITA, "CONFIRMA: encerrar"));
    campos.AddInputControl();
    m_form = campos.CriaFormInterativo("", true);
}
VOTA_LAZY_SINGLETON(CEncerramentoAntecipado, @1905060, @1905084)   // inlined in 10722

// wasm func 10731 - vtable slot 7 (ProcessInput). StartState = ICF 1718 (show m_form).
void CEncerramentoAntecipado::ProcessInput()
{
    switch (m_form->Read()) {
    case CORRIGE:  m_proximoEstado = &CPedeIdentidade::GetInst(); break;
    case CONFIRMA: m_proximoEstado = &CConfirmaEncerramento::GetInst(); break;
    default: break;
    }
}

// uenux2/src/app/vota/operador/outrasopcoes/cconfirmaencerramento.cpp  (path inferred)
// Constructor (inlined into GetInst, func 3632). 20 bytes: +12 m_form. ProcessInput = 10734 (other
// unit: "Procedimento de encerramento confirmado" / "... abortado").
CConfirmaEncerramento::CConfirmaEncerramento()
    : comum::CAppState(2)
{
    api::CFormBuilderMT campos;
    campos.Add<api::CLedFieldMT>(false);
    campos.Add<api::CClockFieldMT>(SPoint{33, 1});
    campos.Add<api::CTextFieldMT>(SPoint{1, 2}, std::make_shared<api::CFixedText>(CENTRO, "Encerramento da Votação"));
    campos.Add<api::CTextFieldMT>(SPoint{1, 4}, std::make_shared<api::CFixedText>(ESQUERDA, "CORRIGE: cancelar"));
    campos.Add<api::CTextFieldMT>(SPoint{40, 4}, std::make_shared<api::CFixedText>(DIREITA, "CONFIRMA: encerrar"));
    campos.AddInputControl();
    m_form = campos.CriaFormInterativo("", true);
}

// wasm func 3632 (tools: api_f3632). @1905056, mutex @1905032. Callers: 10721, 10722, 10731.
VOTA_LAZY_SINGLETON(CConfirmaEncerramento, @1905032, @1905056)

// uenux2/src/app/vota/operador/outrasopcoes/cregistromesarioencerrado.cpp  (path inferred)
// wasm func 10762 - vtable slot 6 (ProcessMessage). StartState (10763, other unit) posts message 14
// to the voter thread.
void CRegistroMesarioEncerrado::ProcessMessage(uebyte mensagem)
{
    if (mensagem == 12)
        m_proximoEstado = &CPedeIdentidade::GetInst();
}

// uenux2/src/app/vota/operador/cfinalizaoperador.cpp  (path inferred)
// wasm func 10214 - vtable slot 2 (StartState). The last state of the operator thread: the rest of
// the encerramento (BU, reports, media) is driven from the voter screen.
void CFinalizaOperador::StartState()
{
    api::CFormBuilderMT campos;
    campos.Add<api::CLedFieldMT>(false);
    campos.Add<api::CTextFieldMT>(SPoint{20, 1}, std::make_shared<api::CFixedText>(CENTRO, "Procedimentos de Encerramento"));
    campos.Add<api::CTextFieldMT>(SPoint{20, 3}, std::make_shared<api::CFixedText>(CENTRO, "Siga as instruções na tela do eleitor"));
    campos.CriaForm("")->Show();
    m_proximoEstado = nullptr;                                                // ends the operator state machine
}

}  // namespace vota
