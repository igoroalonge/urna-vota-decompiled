// Reconstructed from vota_web_wasm.wasm (unit u27; the constructor/GetInst block is unit u17's,
// merged here so that the file is complete). Original: uenux2/src/app/vota/operador/leidentidade/cpedeidentidade.cpp
// (attested: srcloc cpedeidentidade.cpp:62 and :63, both in StartState).
//
// WEB BUILD: dead code (the operator thread never runs in the simulator). Checked with the patched
// harness tools/bu/operator_harness.mjs (docs/modules/u27-uenux2-src-app-vota-operador.md §2).
//
// Functions of this file (and small outlined thunks the tools attributed to it):
//   652    CPedeIdentidade::GetInst + constructor        (unit u17)
//   10680  StartState      10678 FinishState     10677 ProcessInput     10676 ProcessTick
//   5414   thunk IInformacaoThreadOperador::GetInst().SetIdentidadeDigitada(s)       (slot 21)
//   3619   thunk IInformacaoThreadOperador::GetInst().SetTipoIdentidadeDigitada(t)   (slot 22)
//   2226   thunk IInformacaoThreadOperador::GetInst().VotacaoBloqueadaPorHorario()   (slot 19; no file)
// Attributed here by the tools but belonging to other classes (see u27-foreign-fragments.cpp and
// outrasopcoes/cescolheopcao.cpp): 2749/1906 (CValidaIdentidade), 2753/2750/2751 (CEscolheOpcao).
#include "vota/operador/leidentidade/cpedeidentidade.h"

#include <format>
#include <string>
#include <vector>

#include "api/gui/cformbuildermt.h"
#include "api/hwil/iurna.h"
#include "api/util/cdatetime.h"
#include "api/util/cstringutils.h"
#include "comum/appinfo/cappinfo.h"                     // EhTreinamentoEleitor (697), EhFaseTreinamento (1485)
#include "comum/cinfomtlcd.h"
#include "comum/dados/cconfiguracaoeleicao.h"
#include "comum/dados/celeitores.h"
#include "vota/eleitor/cthreadeleitor.h"
#include "vota/log/clogvota.h"
#include "vota/operador/aguardaeleitor/caguardainspecao.h"
#include "vota/operador/comum/iinformacaothreadoperador.h"
#include "vota/operador/confirmaidentidade/cnomeeleitor.h"
#include "vota/operador/cthreadoperador.h"
#include "vota/operador/leidentidade/cvalidaidentidade.h"
#include "vota/operador/outrasopcoes/cescolheopcao.h"
#include "vota/operador/outrasopcoes/chorariovotacaoterminou.h"

namespace vota {

using api::SPoint;

namespace {

constexpr auto ESQUERDA = api::ETextAlignment(0);
constexpr auto DIREITA  = api::ETextAlignment(1);

// Operator -> voter thread message: show the "inspeção" screen on the voter terminal. Name inferred.
constexpr short MSG_ELEITOR_INSPECAO = 12;

// Text providers of the idle screen (table slots 4053 / 4054; bodies in cinformacaothreadoperador.cpp, u10).
std::string TextoQtdVotaram() { return impl::IInformacaoThreadOperador::GetInst().GetTextoQtdVotaram(); }
std::string TextoAudio()      { return impl::IInformacaoThreadOperador::GetInst().GetTextoAudio(); }

// "o Título", "o CPF", "o Identificador" (u17)
std::string NomeTipoIdentidadeComArtigo(comum::md::ETipoIdentidade tipo);

}  // namespace

// ---------------------------------------------------------------------------------------------------
// Constructor (unit u17's reconstruction, inlined into GetInst, wasm func 652). 32 bytes.
CPedeIdentidade::CPedeIdentidade()
    : comum::CAppState(6)                                                    // keys + ticks
    , m_tickBloqueio(CThreadOperador::GetInst().CriaTick(60000))             // func 807
    , m_tickInspecao(CThreadOperador::GetInst().CriaTick(5000))
    , m_tickStatus(CThreadOperador::GetInst().CriaTick(1000))
    , m_textoStatus(std::make_shared<std::string>(" "))
{
    api::CFormBuilderMT campos;
    if (comum::EhTreinamentoEleitor()) {                                     // func 697
        // Voter-training mode ("treinamento de eleitores"): no identification, just count the votes.
        campos.Add<api::CBuzzFieldMT>(51, 10);
        campos.Add<api::CClockFieldMT>(SPoint{33, 1});
        campos.Add<api::CTextFieldMT>(SPoint{1, 1}, std::make_shared<api::CFixedText>(ESQUERDA, "TREINAMENTO DE ELEITORES"));
        campos.Add<api::CTextFieldMT>(SPoint{30, 2}, std::make_shared<api::CFixedText>(ESQUERDA, "Votos:"));
        campos.Add<api::CTextFieldMT>(SPoint{37, 2}, std::make_shared<api::CDataText<std::string (*)()>>(ESQUERDA, &TextoQtdVotaram));
        campos.Add<api::CTextFieldMT>(SPoint{40, 3}, std::make_shared<api::CDataText<std::string (*)()>>(DIREITA, &TextoAudio));
        campos.Add<api::CTextFieldMT>(SPoint{1, 4}, std::make_shared<api::CFixedText>(ESQUERDA, "CORRIGE: outras opções"));
        campos.Add<api::CTextFieldMT>(SPoint{1, 4}, std::make_shared<api::CFixedText>(DIREITA, "CONFIRMA: votar"));   // (1,4) right (262145), not (40,4)
        campos.Add<api::CLedFieldMT>(false);
        campos.AddInputControl();
    } else {
        const auto aptos = comum::CEleitores::GetInst().GetQtdAptosSecao();   // comum_f2823
        const std::string totalAptos = std::format("/{:04}", aptos.qtd1 + aptos.qtd2);
        std::vector<std::string> tipos;
        for (const auto tipo : comum::CConfiguracaoEleicao::GetInst().GetTiposIdentidadePermitidos())
            tipos.push_back(NomeTipoIdentidadeComArtigo(tipo));
        const std::string digite = std::format("Digite {}", Junta(tipos, " ou "));   // "Digite o Título ou o CPF"

        campos.Add<api::CBuzzFieldMT>(51, 10);
        campos.Add<api::CClockFieldMT>(SPoint{33, 1});
        campos.Add<api::CTextFieldMT>(SPoint{1, 1}, std::make_shared<api::CFixedText>(ESQUERDA, digite));
        campos.Add<api::CTextFieldMT>(SPoint{32, 2}, std::make_shared<api::CDataText<std::string (*)()>>(ESQUERDA, &TextoQtdVotaram));
        campos.Add<api::CTextFieldMT>(SPoint{36, 2}, std::make_shared<api::CFixedText>(ESQUERDA, totalAptos));
        campos.Add<api::CTextFieldMT>(SPoint{40, 4}, std::make_shared<api::CDataText<std::string (*)()>>(DIREITA, &TextoAudio));
        campos.Add<api::CTextFieldUpdateMT>(SPoint{1, 4},
            std::make_shared<api::CDataTextFmt<api::CTextSource>>(ESQUERDA, api::CTextSource(m_textoStatus), "%s"),
            std::chrono::milliseconds{300});
        campos.Add<api::CLedFieldMT>(false);
        campos.Add<api::CInputFieldMT>(12, /*aguardaConfirma*/ true, SPoint{1, 2});   // func 1151: 12 digits
    }
    m_form = campos.CriaFormInterativo("", true);                            // func 301

    impl::IInformacaoThreadOperador::GetInst().SorteiaProximaInspecao();     // func 3620 (slot 17)
}

// wasm func 652 (unit u17): lazy singleton, static unique_ptr @1905336, mutex @1905312.
CPedeIdentidade& CPedeIdentidade::GetInst()
{
    static std::mutex mutex;
    static std::unique_ptr<CPedeIdentidade> s_inst;
    std::lock_guard lock(mutex);
    if (!s_inst)
        s_inst.reset(new CPedeIdentidade());
    return *s_inst;
}

// ---------------------------------------------------------------------------------------------------
// wasm func 10680 (vtable slot 2, srclocs :62 and :63)
void CPedeIdentidade::StartState()
{
    CLogVota::GetInst().Loga("Aguardando digitação do identificador do eleitor");     // CLoga::loga(level 1)
    impl::IInformacaoThreadOperador::GetInst().LimpaIdentidades();                     // slot 20
    m_form->Show();                                                                    // IForm slot 2
    m_proximoEstado = this;

    if (api::CPolySingletonList::instance<api::IUrna>().GetModelo() >= 2020)          // :62 (func 923)
        api::CPolySingletonList::instance<api::IScreenMT>().SetModoMenu(false);       // :63 slot 19 ?
                                                                                       //  (name unknown; the web mock only refreshes)
    comum::CInfoMTLCD::GetInst().ExibeBateria();   // funcs 2284 + 5903: re-attach the MT LCD to the battery
                                                   // icon source (replaces the photo of the last voter) ?

    CThreadOperador& operador = CThreadOperador::GetInst();
    // The "end of voting" check is not armed in the training phase nor once it already fired.
    if (!comum::EhFaseTreinamento() &&                                                 // func 1485: fase == '3'
        !impl::IInformacaoThreadOperador::GetInst().VotacaoBloqueadaPorHorario())      // func 2226 (slot 19)
        operador.StartTick(m_tickBloqueio);                                            // func 700
    if (!comum::EhTreinamentoEleitor()) {                                              // func 697
        operador.StartTick(m_tickInspecao);
        operador.StartTick(m_tickStatus);
    }
}

// ---------------------------------------------------------------------------------------------------
// wasm func 10678 (vtable slot 5)                                                  name from slot order
void CPedeIdentidade::FinishState()
{
    CThreadOperador& operador = CThreadOperador::GetInst();
    operador.StopTick(m_tickBloqueio);                                                 // func 422
    operador.StopTick(m_tickInspecao);
    operador.StopTick(m_tickStatus);
}

// ---------------------------------------------------------------------------------------------------
// wasm func 10677 (vtable slot 7). CInteractiveForm::Read (cinteractiveform.h:57) is inlined.
void CPedeIdentidade::ProcessInput()
{
    comum::CAppState* proximo = nullptr;
    switch (m_form->Read()) {
    case api::EInputResult::CORRIGE:                                                   // 5
        proximo = &CEscolheOpcao::GetInst();                                           // func 2753: "outras opções"
        break;

    case api::EInputResult::CONFIRMA:                                                  // 9
        if (comum::EhTreinamentoEleitor()) {
            // Voter-training mode: the typed number is ignored; the roll is rewound to its FIRST voter
            // and that voter is "identified" every time (CEleitores current := begin).
            auto& eleitores = comum::CEleitores::GetInst();
            eleitores.PosicionaPrimeiro();                                             // +16 = +4 (begin)   name inferred
            const auto& eleitor = eleitores.GetCurrent().GetEleitor();                 // func 656
            const comum::md::CEleitorIdentidade identidade =
                eleitor.GetIdentidadePorTipo(eleitor.GetTipoIdentificadorPrincipal()); // func 708, +104
            auto& info = impl::IInformacaoThreadOperador::GetInst();
            info.SetIdentidadeDigitada(identidade.GetIdentidade());                    // func 5414 (slot 21)
            info.SetTipoIdentidadeDigitada(identidade.GetTipo());                      // func 3619 (slot 22)
            proximo = &CNomeEleitor::GetInst();                                        // func 5401
        } else {
            proximo = ValidaEntrada();
        }
        break;

    default:
        break;
    }
    if (proximo != nullptr)
        m_proximoEstado = proximo;
}

// Inlined into 10677.                                                                  name inferred
comum::CAppState* CPedeIdentidade::ValidaEntrada()
{
    const std::string digitado = m_form->GetEntrada(0).GetTexto();                     // field 0, string at +24
    if (digitado.empty())
        return nullptr;                                                                // CONFIRMA on an empty field: nothing

    CLogVota::GetInst().Loga("Identificador do eleitor digitado pelo mesário");       // level 1
    if (impl::IInformacaoThreadOperador::GetInst().VotacaoBloqueadaPorHorario())       // func 2226
        return &CHorarioVotacaoTerminou::GetInst();                                    // func 5430

    // Left-pad to 12 digits with '0' (func 753): "123" -> "000000000123".
    impl::IInformacaoThreadOperador::GetInst().SetIdentidadeDigitada(
        api::CStringUtils::PadLeft(digitado, '0', 12));                                // func 5414
    return &CValidaIdentidade::GetInst();   // lazy singleton @1905672 (32 bytes, ctor inlined: CAppState(2),
                                            // m_form = nullptr, m_opcoes = {}); see u27-foreign-fragments.cpp
}

// ---------------------------------------------------------------------------------------------------
// wasm func 10676 (vtable slot 8)
void CPedeIdentidade::ProcessTick(const uebyte tick)
{
    if (tick == m_tickBloqueio) {
        // End of voting: the tick is stopped once voting is blocked; the next CONFIRMA shows
        // CHorarioVotacaoTerminou.
        if (impl::IInformacaoThreadOperador::GetInst().VotacaoBloqueadaPorHorario()) {
            CThreadOperador::GetInst().StopTick(m_tickBloqueio);
            CLogVota::GetInst().Loga(2, "Votacao foi bloqueada por horario");          // api_f1398 (level 2)
        }
        return;
    }

    if (tick == m_tickInspecao) {
        auto& info = impl::IInformacaoThreadOperador::GetInst();
        const api::CDateTime agora;                                                    // func 479
        if (agora < info.GetDataHoraProximaInspecao())                                 // slot 18, func 759 (<=>)
            return;
        info.SorteiaProximaInspecao();                                                 // func 3620: now + 60..90 min
        CThreadEleitor::GetInst().GetFila().Push(api::SMessage{MSG_ELEITOR_INSPECAO}, 1);   // func 501
        m_proximoEstado = &CAguardaInspecao::GetInst();
        //   ^ lazy singleton @1908552 (mutex @1908528), constructor inlined here:
        //     CAppState(1 = messages only); non-interactive form (func 1694) with
        //     Buzz(51, 10), clock at (33,1), (1,2) "Inspecione cabina e urna",
        //     (1,3) "Instruções no terminal do eleitor".
        return;
    }

    if (tick == m_tickStatus) {
        // Hint shown on line 4 only while the input field is empty.
        const bool vazio = m_form->GetEntrada(m_form->GetFoco()).GetTexto().empty();   // +84 focus, vector.at
        *m_textoStatus = vazio ? "CORRIGE: outras opções" : " ";
    }
}

}  // namespace vota
