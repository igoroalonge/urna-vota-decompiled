// Reconstructed from vota_web_wasm.wasm (unit u27).
// Original file UNKNOWN - path inferred: uenux2/src/app/vota/operador/leidentidade/ieleitorimpedidovotar.cpp
// (the analysis tools filed funcs 1905, 5419, 5424, 1257 under celeitorencontrado.cpp by their callers).
//
// WEB BUILD: dead code (operator thread not run).
#include "vota/operador/leidentidade/ieleitorimpedidovotar.h"

#include "api/gui/cformbuildermt.h"

namespace vota {

using api::SPoint;

namespace {

constexpr auto ESQUERDA = api::ETextAlignment(0);
constexpr auto DIREITA  = api::ETextAlignment(1);

// Typed identity "Título: XXXX XXXX XXXX" (table slot 3907 = func 10586, unit u10).
std::string TextoIdentidadeDigitada();

// Texts of the concrete screens (table slots; bodies in other units, strings decoded here):
std::string TextoNaoEncontrado();   // slot 3867 (10664): "CPF não encontrado. Digite o Título." when the typed type
                                    //   is CPF (func 5416 == 2) and the principal identity is the título (cfg +668 == 1);
                                    //   otherwise "NÃO CADASTRADO nesta urna"
std::string TextoVazio();           // slot 3868 / 3877 / 3896 (3628): " "
std::string TextoOptouTransito();   // slot 3876 (10660): "Optou por votar em trânsito"
std::string TextoImpedido();        // slot 3883 / 3914 (5422): "está impedido de votar ou justificar."
std::string TextoProcureCartorio(); // slot 3884 (10657): "Eleitor deve procurar cartório eleitoral"
std::string TextoImpedido2();       // slot 3889 (10654): "está impedido de votar ou justificar"
std::string TextoIdadeMinima();     // slot 3890 (10653): "por não ter idade mínima"
std::string TextoNaoApto();         // slot 3895 (10649): "não está apto a votar nesta eleição."
std::string TextoSolicitouTransito();   // slot 3915 (10638): "Eleitor solicitou voto em trânsito"

}  // namespace

// ---------------------------------------------------------------------------------------------------
// wasm func 5419 (tools: vota_f5419). Template instance of the MT form builder:
//   Add<CTextFieldMT>(pos, make_shared<CDataText<std::function<std::string()>>>(ESQUERDA, fonte))
// (CDataText<std::function<...>> vtable @1539324, 32 bytes). Summarised - see api/gui.

// ---------------------------------------------------------------------------------------------------
// wasm func 1905 (tools: vota_f1905). 24-byte objects: CAppState(2 = keys), +12 m_form, +20 m_tipo.
IEleitorImpedidoVotar::IEleitorImpedidoVotar(std::function<std::string()> linha2,
                                             std::function<std::string()> linha3, const ETipoRetorno tipo)
    : comum::CAppState(2)
    , m_tipo(tipo)
{
    api::CFormBuilderMT campos;
    campos.Add<api::CLedFieldMT>(false);                                                   // func 435
    campos.Add<api::CTextFieldMT>(SPoint{1, 1}, std::make_shared<api::CDataText<std::string (*)()>>(
                                                    ESQUERDA, &TextoIdentidadeDigitada));  // func 619, slot 3907
    campos.Add<api::CTextFieldMT>(SPoint{1, 2}, std::make_shared<api::CDataText<std::function<std::string()>>>(
                                                    ESQUERDA, std::move(linha2)));         // func 5419
    campos.Add<api::CTextFieldMT>(SPoint{1, 3}, std::make_shared<api::CDataText<std::function<std::string()>>>(
                                                    ESQUERDA, std::move(linha3)));
    switch (m_tipo) {
    case RETORNA_CONFIRMA:
        campos.Add<api::CTextFieldMT>(SPoint{40, 4}, std::make_shared<api::CFixedText>(DIREITA, "CONFIRMA: retornar"));
        break;
    case RETORNA_CORRIGE:
        campos.Add<api::CTextFieldMT>(SPoint{1, 4}, std::make_shared<api::CFixedText>(ESQUERDA, "CORRIGE: retornar"));
        break;
    default:                                   // any other value: no key hint on line 4
        break;
    }
    campos.AddInputControl();                                                              // func 395
    m_form = campos.CriaFormInterativo("", true);                                          // func 301
}

// wasm func 1257 (slot 0): body merged by wasm-opt with other "vptr + one shared_ptr at +12" destructors
// (func 2902: store vptr, release m_form's control block). wasm func 1689 (slot 1) = dtor + delete; it is
// the deleting destructor of all seven classes (they have no own members).
IEleitorImpedidoVotar::~IEleitorImpedidoVotar() = default;

// ---------------------------------------------------------------------------------------------------
// Concrete screens: lazy singletons (static unique_ptr + mutex). Only CEleitorNaoEncontrado::GetInst has
// its own function (5424); the others are inlined into CEleitorEncontrado::StartState (10635).

// wasm func 5424 (@1905392, mutex @1905368). Callers: CProcuraEleitor::StartState, CEleitorEncontrado::StartState.
CEleitorNaoEncontrado::CEleitorNaoEncontrado()
    : IEleitorImpedidoVotar(&TextoNaoEncontrado, &TextoVazio, RETORNA_CONFIRMA) {}

CEleitorOptouPorVotarEmTransito::CEleitorOptouPorVotarEmTransito()          // @1905420
    : IEleitorImpedidoVotar(&TextoOptouTransito, &TextoVazio, RETORNA_CONFIRMA) {}

CEleitorImpedidoJustificar::CEleitorImpedidoJustificar()                    // @1905448
    : IEleitorImpedidoVotar(&TextoImpedido, &TextoProcureCartorio, RETORNA_CORRIGE) {}

CEleitorNaoTemIdadeMinima::CEleitorNaoTemIdadeMinima()                      // @1905476
    : IEleitorImpedidoVotar(&TextoImpedido2, &TextoIdadeMinima, RETORNA_CORRIGE) {}

CEleitorNaoPossuiCargosParaVotar::CEleitorNaoPossuiCargosParaVotar()        // @1905504
    : IEleitorImpedidoVotar(&TextoNaoApto, &TextoVazio, RETORNA_CORRIGE) {}

CEleitorImpedidoJustificarVotoTransito::CEleitorImpedidoJustificarVotoTransito()   // @1905588
    : IEleitorImpedidoVotar(&TextoImpedido, &TextoSolicitouTransito, RETORNA_CORRIGE) {}

#define VOTA_LAZY_SINGLETON(Classe)                                             \
    Classe& Classe::GetInst()                                                  \
    {                                                                          \
        static std::mutex mutex;                                               \
        static std::unique_ptr<Classe> s_inst;                                 \
        std::lock_guard lock(mutex);                                           \
        if (!s_inst)                                                           \
            s_inst.reset(new Classe());                                        \
        return *s_inst;                                                        \
    }

VOTA_LAZY_SINGLETON(CEleitorNaoEncontrado)                   // wasm func 5424
VOTA_LAZY_SINGLETON(CEleitorOptouPorVotarEmTransito)
VOTA_LAZY_SINGLETON(CEleitorImpedidoJustificar)
VOTA_LAZY_SINGLETON(CEleitorNaoTemIdadeMinima)
VOTA_LAZY_SINGLETON(CEleitorNaoPossuiCargosParaVotar)
VOTA_LAZY_SINGLETON(CEleitorImpedidoJustificarVotoTransito)

}  // namespace vota
