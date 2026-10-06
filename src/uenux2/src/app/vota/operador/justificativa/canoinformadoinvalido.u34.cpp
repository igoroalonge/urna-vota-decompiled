// FRAGMENT reconstructed by unit u34 from vota_web_wasm.wasm.
// Original files: uenux2/src/app/vota/operador/justificativa/canoinformadoinvalido.cpp and
//                 uenux2/src/app/vota/operador/justificativa/celeitormenor16anos.cpp   (paths inferred by u17)
// Class declarations: ../estadosoperador.u34.h. Constructors: u17-foreign-fragments.cpp.
//
// Justification of absence ("justificativa"): a voter registered elsewhere justifies not voting; the mesário
// types the voter's year of birth (CPedeAnoNascimento, func 10590). An impossible year (< 1900 or not in the
// past) gives CAnoInformadoInvalido; a voter younger than 16 (by year) gives CEleitorMenor16Anos. Both
// screens end with "CONFIRMA: tentar novamente".
//
// WEB BUILD: dead code (operator thread not run).
#include "vota/operador/estadosoperador.u34.h"
#include "vota/operador/justificativa/cpedeanonascimento.h"

namespace vota {
namespace {

// wasm func 6014 - merged body (wasm-opt merge-similar-functions) of the two ProcessInput methods below.
// The only difference between them, the std::source_location record of the IInputMT lookup inside the
// inlined CInteractiveForm::Read (cinteractiveform.h:57), became the second parameter.     name inferred
template <class ESTADO>
void VoltaParaAnoNascimentoSeConfirma(ESTADO& estado)
{
    if (estado.m_form->Read() == api::EInputResult::CONFIRMA)
        estado.m_proximoEstado = &CPedeAnoNascimento::GetInst();    // func 5418: type the year again
}

}  // namespace

// wasm func 10601 - vtable slot 7: api_f6014(this, srcloc @1589716)
void CAnoInformadoInvalido::ProcessInput() { VoltaParaAnoNascimentoSeConfirma(*this); }

// wasm func 10598 - vtable slot 7: api_f6014(this, srcloc @1589788)
void CEleitorMenor16Anos::ProcessInput() { VoltaParaAnoNascimentoSeConfirma(*this); }

}  // namespace vota
