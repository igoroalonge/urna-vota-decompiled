// uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cesperaretestar.cpp (path inferred)  --  FRAGMENT
// written by unit u26 (the class is described by unit u02 in ctesteteclado.u02.cpp; GetInst = func 5936).
//
// vota::testeteclado::CEsperaRetestar (typeinfo @1546820, vtable @1546780): after a failed keyboard test the
// mesário must wait before repeating it. Screen "telaEsperaRepetirTesteTeclado": "Por favor, espere {}s para a"
// / "realização de uma nova tentativa" / "de execução do teste.", the {} refreshed every 200 ms.
//   [2] StartState (11825, other unit)  [5] FinishState (11824)  [9] ProcessTickNaoDesligamento (11823)
#include "vota/eleitor/iniciovotacao/testeteclado/cesperaretestar.h"

#include <format>
#include <string>

#include "api/util/cdatetime.h"
#include "vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.h"

namespace vota::testeteclado {

// @1835000: moment from which the test may be repeated (static CDateTime, initialised to 0 by
// __wasm_call_ctors via api_f1000; set by CEsperaRetestar::StartState).                name inferred
extern api::CDateTime g_horaRetestar;

// wasm func 11823 (vtable slot 9)
void CEsperaRetestar::ProcessTickNaoDesligamento(uebyte tick)
{
    if (tick != m_tick)                                  // +36
        return;
    const api::CDateTime agora;                          // func 479
    if (agora.Compare(g_horaRetestar) < 0)               // func 759
        return;
    m_proximoEstado = &CTesteTeclado::GetInst();         // func 3855
}

// wasm func 11826 (table slot 1862, used by func 5936 as the text source of the countdown). name inferred
// Remaining seconds (0 once the moment has passed) in the screen's format string.
std::string TextoEsperaRetestar(const std::string& formato)
{
    const api::CDateTime agora;
    unsigned segundos = 0;
    if (agora.Compare(g_horaRetestar) < 0)
        segundos = static_cast<unsigned>(api::CDateTime::DiferencaSegundos(g_horaRetestar, agora));   // func 5471
    return std::vformat(formato, std::make_format_args(segundos));
}

} // namespace vota::testeteclado
