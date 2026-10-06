// FRAGMENT reconstructed by unit u32 from vota_web_wasm.wasm.
// Original file: uenux2/src/api/pattern/igenericfactory.h (template, unit u20). This file lists the four
// instantiations of CDefaultGenericFactory<AP, C>::Create() (vtable slot 2) that the tools left in unit u32.
// Each is `return std::make_unique<C>();` - the object is new'ed, then constructed inside an invoke_* so that
// the memory is freed if the constructor throws.
//
// The factories are registered by main() (funcs 8986/8911/8834/8758) and used by api::CThread, the
// message queues and the locks of the application; in the web build the thread implementation is the
// cooperative simulador::CWasmThread.
#include "api/pattern/igenericfactory.h"

#include "api/ipc/posix/cposixmutex.h"
#include "api/ipc/posix/cposixrwmutex.h"
#include "api/ipc/posix/cposixsemaphore.h"
#include "simulador/wasm/cwasmthread.h"     // uenux2/mock/app/simulador/wasm (path inferred)

namespace api {

// wasm func 10854 (observed executing) - new(20) + invoke CPosixSemaphore::CPosixSemaphore(p, 0)
//   (table slot 245 = func 10250, cposixsemaphore.cpp:23): the initial count is 0.
template <>
std::unique_ptr<ISemaphore> CDefaultGenericFactory<ISemaphore, CPosixSemaphore>::Create()
{
    return std::make_unique<CPosixSemaphore>(0);
}

// wasm func 10864 - thunk into the merged body api_f6023(this, ctor slot 244, size 36)
template <>
std::unique_ptr<IRWSyncCtl> CDefaultGenericFactory<IRWSyncCtl, CPosixRWMutex>::Create()
{
    return std::make_unique<CPosixRWMutex>();
}

// wasm func 10869 (observed executing) - thunk into api_f6023(this, ctor slot 243, size 32)
template <>
std::unique_ptr<ISyncCtl> CDefaultGenericFactory<ISyncCtl, CPosixMutex>::Create()
{
    return std::make_unique<CPosixMutex>();
}

// wasm func 10878 - the CWasmThread constructor is inlined: 16 bytes, vptr @1528400, +4..+11 zero,
// +12 = 0x2000 as a 16-bit store (i.e. byte +12 = 0, byte +13 = 0x20)                                   ?
template <>
std::unique_ptr<IThreadImpl> CDefaultGenericFactory<IThreadImpl, simulador::CWasmThread>::Create()
{
    return std::make_unique<simulador::CWasmThread>();
}

} // namespace api
