// FRAGMENT reconstructed by unit u19 from vota_web_wasm.wasm.
// Original files: uenux2/src/api/util/isystemdatetime.cpp (srcloc :23, ISystemDateTime::GetInst())
//                 uenux2/src/api/util/isystemdatetime.h   (srcloc :46, ISystemDateTime::CreateInst<T>())
//
// The clock of the urna. The first GetInst() of the process registers the POSIX default
// api::CSystemDateTime (vtable @1585244); the simulator then REPLACES it with simulador::CWasmSystemDateTime
// (browser clock + offset) in its start-up (func 8302). Verified with DEBUG_UENUX=1: ISystemDateTime is the
// very first push (sz[0], before main registers anything) and is pushed again at sz[26].
#include "api/util/isystemdatetime.h"

#include <memory>
#include <source_location>

#include "api/pattern/cpolysingleton.h"
#include "api/util/csystemdatetime.h"

namespace api {

// isystemdatetime.h:46 - inlined into func 1155.
template <typename T>
void ISystemDateTime::CreateInst()
{
    if (CPolySingleton<ISystemDateTime>::exists())                          // func 2160
        throw CUeUtilError(static_cast<EUeUtilError>(7084), "Tentativa de recriar o singleton",
                           std::source_location::current());                // :46 (thunk func 346)
    CPolySingletonList::replace<ISystemDateTime>(std::make_unique<T>(), GetPolySingletonsInfo());   // func 4890
        // (by-value helper with replace + push inlined: exists/erase prefix, then the push body)
}

// wasm func 1155 (tools name "api::CPolySingletonList::instance@1155"; executed at start-up)
ISystemDateTime& ISystemDateTime::GetInst()
{
    if (!CPolySingleton<ISystemDateTime>::exists())
        CreateInst<CSystemDateTime>();
    return CPolySingleton<ISystemDateTime>::instance(GetPolySingletonsInfo(),
                                                     std::source_location::current());   // :23
}

}  // namespace api
