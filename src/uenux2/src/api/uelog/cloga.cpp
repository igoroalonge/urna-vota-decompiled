// Reconstructed from vota_web_wasm.wasm (unit u20).
// Original: uenux2/src/api/uelog/cloga.cpp (srcloc cloga.cpp:26).
//
// api::CLoga is the static entry point every module uses to write to the urna log ("logar").
// It resolves the registered CEscritorLog (simulador::CWasmLogd in the web build) and forwards.
// Observed executing: every voter/operator action that is logged ("Voto confirmado para [...]",
// "Tecla indevida pressionada", ...) passes here.
#include "api/uelog/cloga.h"

#include "api/pattern/cpolysingletonlist.h"
#include "api/uelog/cescritorlog.h"

namespace api {

// wasm func 433 - 32 callers / 41 call sites, no table slot (comum::IEventosLog helpers api_f233 /
// vota_f2282, CThreadMonitor, ...)
void CLoga::loga(ELogAplicativos aplicativo, ESeveridade severidade, const std::string& mensagem)
{
    CPolySingletonList::instance<CEscritorLog>()        // srcloc cloga.cpp:26 (passed to instance())
        .loga(aplicativo, severidade, mensagem);        // virtual slot 3 (func 10260)
}

} // namespace api
