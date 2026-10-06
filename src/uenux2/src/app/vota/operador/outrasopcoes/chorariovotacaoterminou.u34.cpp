// FRAGMENT reconstructed by unit u34 from vota_web_wasm.wasm.
// Original file: uenux2/src/app/vota/operador/outrasopcoes/chorariovotacaoterminou.cpp  (path inferred by u27)
// Class declaration: ../estadosoperador.u34.h. StartState (10758) and ProcessInput (10757, CORRIGE ->
// CPedeIdentidade) are in u27-foreign-fragments.cpp.
//
// Shown when the mesário tries to identify a voter, or to switch the audio, after voting was blocked by the
// clock (IInformacaoThreadOperador::VotacaoBloqueadaPorHorario). Note: "Horario de votacao terminou!" is
// stored without its accents (Horário, votação); "Favor encerrar a urna!" needs none. It is not the only
// unaccented MT text: CVerificaDadoEleitor (10443) shows "CONFIRMA: cancelar a habilitacao".
//
// WEB BUILD: dead code (operator thread not run).
#include <memory>
#include <mutex>

#include "api/gui/cformbuildermt.h"
#include "vota/operador/estadosoperador.u34.h"

namespace vota {

namespace {
constexpr auto ESQUERDA = api::ETextAlignment(0);
}

// Constructor - inlined into wasm func 5430.
CHorarioVotacaoTerminou::CHorarioVotacaoTerminou()
    : comum::CAppState(2)                                                                      // keys
{
    api::CFormBuilderMT campos;
    campos.Add<api::CBeepFieldMT>(2);                                                          // func 1072
    campos.Add<api::CTextFieldMT>(api::SPoint{1, 1},
                                  std::make_shared<api::CFixedText>(ESQUERDA, "Horario de votacao terminou!"));   // @439306
    campos.Add<api::CTextFieldMT>(api::SPoint{1, 2},
                                  std::make_shared<api::CFixedText>(ESQUERDA, "Favor encerrar a urna!"));         // @439933
    campos.Add<api::CTextFieldMT>(api::SPoint{1, 4}, std::make_shared<api::CFixedText>(ESQUERDA, "CORRIGE"));      // @331259
    campos.AddInputControl();                                                                  // func 395
    m_form = campos.CriaFormInterativo("", true);                                              // func 301
}

// wasm func 5430 (tools: api_f5430). Lazy singleton: static unique_ptr @1904916, mutex @1904892.
// Callers: CPedeIdentidade::ProcessInput (10677), CEscolheOpcao::ProcessInput (10689).
CHorarioVotacaoTerminou& CHorarioVotacaoTerminou::GetInst()
{
    static std::mutex mutex;
    static std::unique_ptr<CHorarioVotacaoTerminou> s_inst;
    std::lock_guard lock(mutex);
    if (!s_inst)
        s_inst.reset(new CHorarioVotacaoTerminou());
    return *s_inst;
}

}  // namespace vota
