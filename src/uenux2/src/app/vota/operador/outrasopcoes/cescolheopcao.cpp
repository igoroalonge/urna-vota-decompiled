// Reconstructed from vota_web_wasm.wasm (unit u27). Original: uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp
// (attested: srclocs cescolheopcao.cpp:150, :151, :152 in StartState).
// The three "contadores de biometria" text providers (funcs 10699-10701) were reconstructed by unit u02:
// see cescolheopcao.u02.cpp next to this file.
//
// WEB BUILD: dead code (the operator thread never runs). Verified with the patched harness
// tools/bu/operator_harness.mjs: the menu, option 4 (counters), option 2 before 17:00, option 2 after
// 17:00 and option 1 all behave as written below (u27 doc §2).
//
// Functions:
//   2753   CEscolheOpcao::GetInst + constructor (the tools filed it under cpedeidentidade.cpp)
//   2751   COpcaoDS::COpcaoDS(int, std::function&&)     2750  SOpcao::SOpcao(EAcao, COpcaoDS&&)
//   2752   ~CEscolheOpcao (slot 0)   10691 deleting dtor (slot 1)   10697 atexit reset of the instance
//   3630   std::map<int, SOpcao> node destruction (__tree::destroy)
//   10690  StartState     10689 ProcessInput
//   10695  menu text of option 1 (table slot 3827)
//   2502   CLogVota "Operador selecionou: {}" (merged log body 6113)
//   1687   thunk IInformacaoThreadOperador::GetInst().GetAudioHabilitadoManualmente() (slot 6)
// Constructors of other states inlined into ProcessInput (their other methods live in their own
// files, see u27-foreign-fragments.cpp): CHabilitacaoAudioNaoPermitida, CConfirmaAudio,
// CIniciaFinalizacao, CContadoresBiometria.
#include "vota/operador/outrasopcoes/cescolheopcao.h"

#include <algorithm>
#include <format>
#include <string>
#include <syslog.h>

#include "api/gui/cformbuildermt.h"
#include "api/hwil/iurna.h"
#include "comum/appinfo/cappinfo.h"                         // EhTreinamentoEleitor (697), DeveRegistrarMesarios (2520)
#include "comum/clocal.h"                                   // CLocal::UrnaBiometrica (401 + 820)
#include "comum/comparecimentomesario/estados/cregistrarmesarios.h"
#include "comum/dados/cconfiguracaoeleicao.h"
#include "ecourna/api/util/cstringutils.h"
#include "vota/log/clogvota.h"
#include "vota/operador/comum/iinformacaothreadoperador.h"
#include "vota/operador/leidentidade/cpedeidentidade.h"
#include "vota/operador/outrasopcoes/cconfirmaaudio.h"
#include "vota/operador/outrasopcoes/ccontadoresbiometria.h"
#include "vota/operador/outrasopcoes/chabilitacaoaudionaopermitida.h"
#include "vota/operador/outrasopcoes/chorariovotacaoterminou.h"
#include "vota/operador/outrasopcoes/ciniciafinalizacao.h"

namespace vota {

using api::SPoint;

namespace {

constexpr auto ESQUERDA = api::ETextAlignment(0);
constexpr auto DIREITA  = api::ETextAlignment(1);
constexpr auto CENTRO   = api::ETextAlignment(2);

// wasm func 1687 (thunk kept out of line; callers 10539, 10646, 10689, 10695, 10742, 10751)
bool AudioHabilitadoManualmente()
{
    return impl::IInformacaoThreadOperador::GetInst().GetAudioHabilitadoManualmente();   // slot 6
}

// Menu texts (table slots 3827..3830 -> std::function<std::string()>)
// wasm func 10695 (slot 3827)
std::string TextoOpcaoAudio() { return AudioHabilitadoManualmente() ? "Desativar áudio" : "Ativar áudio"; }
std::string TextoOpcaoEncerrar()        { return "Encerrar votação"; }      // func 10694 (slot 3828)
std::string TextoOpcaoRegistrarMesarios() { return "Registrar mesários"; }  // func 10693 (slot 3829)
std::string TextoOpcaoContadores()      { return "Exibir contadores"; }     // func 10692 (slot 3830)

// wasm func 2502 (thunk into the merged "log one {} argument" body, func 6113)
void LogaOpcaoSelecionada(CLogVota& log, const std::string& opcao)
{
    log.Loga(std::format("Operador selecionou: {}", opcao));             // level 1
}

// Confirmation text of CConfirmaAudio (wasm func 10742, table slot 3757)
std::string TextoConfirmaAudio()
{
    return AudioHabilitadoManualmente() ? "Deseja realmente desativar o áudio?"
                                        : "Deseja realmente ativar o áudio?";
}

}  // namespace

// ---------------------------------------------------------------------------------------------------
// Constructor (inlined into GetInst, wasm func 2753). 32 bytes: CAppState(2 = keys).
CEscolheOpcao::CEscolheOpcao()
    : comum::CAppState(2)
{
    // Menu entries. try_emplace: an existing key is kept (the lookup loops in the binary).
    m_opcoes.try_emplace(1, EAcao::AUDIO, COpcaoDS(1, &TextoOpcaoAudio));
    m_opcoes.try_emplace(2, EAcao::ENCERRAR_VOTACAO, COpcaoDS(2, &TextoOpcaoEncerrar));
    int proximo = 3;
    if (comum::DeveRegistrarMesarios()) {       // func 2520: CInformacaoEleicao::IdentificaMesarios()
                                                //   (!demo && ParametrosUrna.registrarMesarios) && !EhTreinamentoEleitor()
        m_opcoes.try_emplace(3, EAcao::REGISTRAR_MESARIOS, COpcaoDS(3, &TextoOpcaoRegistrarMesarios));
        proximo = 4;
    }
    if (comum::CLocal::GetInst().UrnaBiometrica() && !comum::EhTreinamentoEleitor())   // funcs 401/820, 697
        m_opcoes.try_emplace(proximo, EAcao::CONTADORES_BIOMETRIA, COpcaoDS(proximo, &TextoOpcaoContadores));

    api::CFormBuilderMT campos;
    campos.Add<api::CClockFieldMT>(SPoint{33, 1});                                  // func 728
    campos.Add<api::CTextFieldMT>(SPoint{1, 1}, std::make_shared<api::CFixedText>(ESQUERDA, "Selecione a opção: "));
    // Two columns: 1 -> (1,2), 2 -> (1,3), 3 -> (20,2), 4 -> (20,3)
    SPoint pos{1, 2};
    for (const auto& [numero, opcao] : m_opcoes) {
        campos.Add<api::CTextFieldMT>(pos, std::make_shared<api::CDataText<COpcaoDS>>(ESQUERDA, opcao.ds));
        const bool fimColuna = pos.linha == 3;
        pos = fimColuna ? SPoint{20, 2} : SPoint{pos.coluna, 3};
    }
    campos.Add<api::CTextFieldMT>(SPoint{1, 4}, std::make_shared<api::CFixedText>(ESQUERDA, "CORRIGE: retornar"));
    campos.Add<api::CTextFieldMT>(SPoint{40, 4}, std::make_shared<api::CFixedText>(DIREITA, "CONFIRMA: prosseguir"));
    campos.Add<api::CInputFieldMT>(1, /*aguardaConfirma*/ true, SPoint{20, 1});     // func 1151: 1 digit
    m_form = campos.CriaFormInterativo("", true);                                    // func 301 -> +24
}

// wasm func 2753: lazy singleton, static unique_ptr @1905308, mutex @1905284.
// wasm func 10697: its atexit handler (reset -> ~CEscolheOpcao (2752) + operator delete).
CEscolheOpcao& CEscolheOpcao::GetInst()
{
    static std::mutex mutex;
    static std::unique_ptr<CEscolheOpcao> s_inst;
    std::lock_guard lock(mutex);
    if (!s_inst)
        s_inst.reset(new CEscolheOpcao());
    return *s_inst;
}

// wasm func 2752 (slot 0): m_form.reset(); m_opcoes tree destroyed (func 3630). 10691 = + delete.
CEscolheOpcao::~CEscolheOpcao() = default;

// ---------------------------------------------------------------------------------------------------
// wasm func 10690 (vtable slot 2, srclocs :150, :151, :152)
void CEscolheOpcao::StartState()
{
    m_proximoEstado = this;
    m_form->Show();

    // Debug trace left in the build: syslog(LOG_INFO, "%s:%d> %d", __func__, __LINE__, modelo > 2019).
    // It goes straight to vsyslog (musl; no /dev/log in the browser) - not through CLogVota.
    syslog(LOG_INFO /* 6 */, "%s:%d> %d", "StartState", 150,
           api::CPolySingletonList::instance<api::IUrna>().GetModelo() > 2019);   // :150 (func 923)

    if (api::CPolySingletonList::instance<api::IUrna>().GetModelo() >= 2020)      // :151
        api::CPolySingletonList::instance<api::IScreenMT>().SetModoMenu(true);    // :152 slot 19, name unknown ?
}

// ---------------------------------------------------------------------------------------------------
// wasm func 10689 (vtable slot 7). CInteractiveForm::Read (cinteractiveform.h:57) inlined.
void CEscolheOpcao::ProcessInput()
{
    comum::CAppState* proximo = nullptr;
    switch (m_form->Read()) {
    case api::EInputResult::CORRIGE:
        proximo = &CPedeIdentidade::GetInst();                                     // func 652
        break;
    case api::EInputResult::CONFIRMA:
        proximo = ExecutaOpcao();
        break;
    default:
        break;
    }
    if (proximo != nullptr)                  // an invalid or unavailable choice leaves the menu as it is
        m_proximoEstado = proximo;
}

// Inlined into 10689.                                                                  name inferred
comum::CAppState* CEscolheOpcao::ExecutaOpcao()
{
    const std::string digitado = m_form->GetEntrada(0).GetTexto();
    if (digitado.empty() || !std::ranges::all_of(digitado, [](char c) { return c >= '0' && c <= '9'; }))
        return nullptr;
    const auto it = m_opcoes.find(ecourna::api::util::CStringUtils::ToInt32(digitado));   // func 2200
    if (it == m_opcoes.end())
        return nullptr;

    CLogVota& log = CLogVota::GetInst();
    switch (it->second.acao) {
    case EAcao::AUDIO: {
        LogaOpcaoSelecionada(log, TextoOpcaoAudio());                   // "Ativar áudio" / "Desativar áudio"
        if (impl::IInformacaoThreadOperador::GetInst().VotacaoBloqueadaPorHorario()) {   // func 2226
            m_form->ResetEntradas();          // every input field cleared (slot 9), focus 0 (func 1401)
            return &CHorarioVotacaoTerminou::GetInst();                                  // func 5430
        }
        if (!comum::CConfiguracaoEleicao::GetInst().GetPermitirHabManualAudio())         // cfg +485
            return &CHabilitacaoAudioNaoPermitida::GetInst();
            //   ^ lazy singleton @1904944 (mutex @1904920), 20 bytes, ctor inlined here:
            //     CAppState(2); Beep(2); (1,2) centred "Ativação de áudio não permitida."; interactive form.
        return &CConfirmaAudio::GetInst();
            //   ^ lazy singleton @1905000 (mutex @1904976), 20 bytes, ctor inlined here:
            //     CAppState(2); clock (33,1); (1,2) CDataText<fn>(TextoConfirmaAudio, slot 3757);
            //     (1,4) "CORRIGE: não"; (40,4) right "CONFIRMA: sim"; input control; interactive form.
    }
    case EAcao::ENCERRAR_VOTACAO:
        LogaOpcaoSelecionada(log, "Encerrar votação");
        return &CIniciaFinalizacao::GetInst();
            //   ^ lazy singleton @1905252 (mutex @1905228), 12 bytes: CAppState(0), no members.
    case EAcao::REGISTRAR_MESARIOS:
        LogaOpcaoSelecionada(log, "Registrar mesários");
        return &comum::CRegistrarMesarios::GetInst();                                   // func 3610 (u22 flow)
    case EAcao::CONTADORES_BIOMETRIA:
        LogaOpcaoSelecionada(log, "Exibir contadores");
        if (!comum::CLocal::GetInst().UrnaBiometrica())
            return nullptr;
        return &CContadoresBiometria::GetInst();
            //   ^ lazy singleton @1905280 (mutex @1905256), 20 bytes, ctor inlined here: CAppState(2);
            //     right-aligned at column 35 of lines 1-3 the three counters of cescolheopcao.u02.cpp
            //     (table slots 3818/3819/3820: "Habilitação biométrica: {:04}" / "biográfica" / "sem
            //     biometria", or "n.a."); (1,4) "CORRIGE: retornar"; input control; interactive form.
    }
    return nullptr;
}

}  // namespace vota
