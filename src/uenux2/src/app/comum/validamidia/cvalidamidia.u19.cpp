// FRAGMENT reconstructed by unit u19 from vota_web_wasm.wasm.
// Original file: uenux2/src/app/comum/validamidia/cvalidamidia.cpp (srcloc :182, IValidaMidia::GetInst()).
//
// comum::impl::IValidaMidia validates the result medium (MR, memória de resultado) before the results
// are copied to it. Only caller: vota::CCopiaResultadoParaMR::CopiaResultado (func 12134), i.e. the
// encerramento, which the web build never reaches.
#include "comum/validamidia/cvalidamidia.h"

#include <memory>
#include <mutex>
#include <source_location>

#include "api/pattern/cpolysingleton.h"

namespace comum::impl {

// wasm func 5575 (tools name "api::CPolySingletonList::instance@5575"); push<IValidaMidia> inlined.
IValidaMidia& IValidaMidia::GetInst()
{
    static std::mutex s_mutex;                                              // @1839144 (unlock residue)
    std::lock_guard trava(s_mutex);
    if (!api::CPolySingleton<IValidaMidia>::exists())                       // func 3686
        api::CPolySingletonList::push<IValidaMidia>(std::make_unique<CValidaMidia>(),   // vtable @1577032, 4 bytes
                                                    api::GetPolySingletonsInfo());
    return api::CPolySingleton<IValidaMidia>::instance(api::GetPolySingletonsInfo(),
                                                       std::source_location::current());   // :182
}

}  // namespace comum::impl
