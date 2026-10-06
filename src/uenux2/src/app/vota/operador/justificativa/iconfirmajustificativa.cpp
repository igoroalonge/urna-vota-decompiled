// Reconstructed from vota_web_wasm.wasm (unit u27). Original: uenux2/src/app/vota/operador/justificativa/iconfirmajustificativa.cpp
// (attested: srcloc iconfirmajustificativa.cpp:34 in the constructor).
//
// Functions: 3625 constructor, 1688 destructor (slot 0), 2748 deleting destructor (slot 1),
//            10589 StartState (slot 2), 3885 unique_ptr<IConfirmaJustificativa>::reset helper and its
//            three atexit thunks 10622 (@1905700 CConfirmaJustificativa), 10615 (@1905756 ...Temporario),
//            10611 (@1905812 ...Transito).
//
// WEB BUILD: dead code (operator thread not run).
#include "vota/operador/justificativa/iconfirmajustificativa.h"

#include "api/gui/cformbuildermt.h"
#include "comum/md/ctradutorfrase.h"                        // comum::md::CTradutorFrase::TraduzLabel (654)
#include "vota/operador/comum/capresentacaofotoeleitor.h"   // vota::ApresentaFotoEleitor (func 3616, u05)
#include "vota/votaerrors.h"

namespace vota {

using api::SPoint;

namespace {
constexpr auto ESQUERDA = api::ETextAlignment(0);
constexpr auto DIREITA  = api::ETextAlignment(1);
constexpr std::size_t LARGURA_MT = 40;                      // characters per MT line

std::string TextoIdentidadeDigitada();                      // table slot 3907 = func 10586 (u10)
}  // namespace

// ---------------------------------------------------------------------------------------------------
// wasm func 3625 (srcloc :34). CAppState(2 = keys).
IConfirmaJustificativa::IConfirmaJustificativa(const std::string& frase)
    : comum::CAppState(2)
{
    if (frase.size() > LARGURA_MT)                           // size < 41 accepted
        throw CUeVotaError(EUeVotaError{9395}, "Frase muito grande [" + frase + "]");   // line 34
                                                            //  (the annotator shows 9395 as "%z%s%z": it is the code)
    api::CFormBuilderMT campos;
    campos.Add<api::CLedFieldMT>(false);
    campos.Add<api::CTextFieldMT>(SPoint{1, 1}, std::make_shared<api::CDataText<std::string (*)()>>(ESQUERDA, &TextoIdentidadeDigitada));
    campos.Add<api::CTextFieldMT>(SPoint{1, 2}, std::make_shared<api::CFixedText>(ESQUERDA, frase));
    campos.Add<api::CTextFieldMT>(SPoint{1, 4}, std::make_shared<api::CFixedText>(ESQUERDA, "CORRIGE: retornar"));
    campos.Add<api::CTextFieldMT>(SPoint{40, 4}, std::make_shared<api::CFixedText>(DIREITA, "CONFIRMA: justificar"));
    campos.AddInputControl();
    m_form = campos.CriaFormInterativo("", true);
}

// wasm func 1688 (slot 0; merged body 2902: vptr + m_form.reset()), 2748 (slot 1 = dtor + delete).
IConfirmaJustificativa::~IConfirmaJustificativa() = default;

// ---------------------------------------------------------------------------------------------------
// wasm func 10589 (vtable slot 2)
void IConfirmaJustificativa::StartState()
{
    m_form->Show();
    ApresentaFotoEleitor();      // func 3616 (u05): the voter's photo on the MT LCD + IInformacaoThreadOperador
    m_proximoEstado = this;
}

// Concrete classes (constructors inlined into CProcuraEleitor::StartState / CEleitorEncontrado::StartState).
CConfirmaJustificativa::CConfirmaJustificativa()
    : IConfirmaJustificativa(comum::md::CTradutorFrase::TraduzLabel("não pertence <S|a|ao|à> <SCSN>")) {}
        // TraduzLabel expands the parametrised label of the seção, e.g. "não pertence à seção" ?
CConfirmaJustificativaTemporario::CConfirmaJustificativaTemporario()
    : IConfirmaJustificativa("está impedido de votar nesta seção") {}
CConfirmaJustificativaTransito::CConfirmaJustificativaTransito()
    : IConfirmaJustificativa("optou por votar EM TRÂNSITO") {}

// wasm func 3885: std::unique_ptr<IConfirmaJustificativa>::reset() on a static slot (virtual dtor via 1688
// + operator delete). wasm funcs 10622 / 10615 / 10611: the atexit destructors of the three static
// instances (@1905700, @1905756, @1905812), each a one-line call of 3885.

}  // namespace vota
