// uenux2/src/app/comum/comparecimentomesario/estados/*.cpp  -- FRAGMENT written by unit u33.
// The four ProcessInput() methods are reconstructed one per file by unit u22 (ctitulomesariovazio.cpp,
// ctitulomesarioinvalido.cpp, ctitulomesariojaregistrado.cpp, cdigitalmesarionaoreconhecida.cpp); this file
// documents the body they share in the binary.
//
// Registration of the poll workers (comparecimento de mesários): after a bad título or an unrecognised
// fingerprint the microterminal shows an error screen; one key brings the mesário back to "type your título".
#include <source_location>

#include "api/gui/cinteractiveform.h"
#include "comum/comparecimentomesario/estados/estadosregistromesarios.h"

namespace comum {

// wasm func 2895 - merge-similar-functions body of four ProcessInput() methods; the expected key and the
// std::source_location of the IInputMT lookup inside CInteractiveForm::Read (cinteractiveform.h:57) became
// parameters:
//     10352 CTituloMesarioVazio            (5 = CORRIGE)
//     10347 CTituloMesarioInvalido         (5 = CORRIGE)
//     10342 CTituloMesarioJaRegistrado     (5 = CORRIGE)
//     10322 CDigitalMesarioNaoReconhecida  (9 = CONFIRMA)
// Inlined Read(): IInputMT& teclado = CPolySingleton<IInputMT>::instance(info, onde) (func 383);
// campo = form.m_campos.at(form.m_foco) (vector at +72, index at +84 -> std::out_of_range if empty);
// resultado = campo->Read(teclado) (field slot 8).
template <class ESTADO>
static void VoltaParaPedeTitulo(ESTADO& estado, api::EInputResult tecla)             // name inferred
{
    if (estado.m_form->Read() == tecla)                     // form at this+12
        estado.m_proximoEstado = &CPedeTituloMesario::GetInst();   // comum_f2729 (lazy singleton @1909456)
}

} // namespace comum
