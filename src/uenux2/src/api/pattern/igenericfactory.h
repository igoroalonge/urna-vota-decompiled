// Reconstructed from vota_web_wasm.wasm (unit u20).
// Original: uenux2/src/api/pattern/igenericfactory.h (srclocs igenericfactory.h:44 and :50).
//
// A tiny abstract-factory used by the urna's portability layer to create OS primitives (semaphores,
// mutexes, read/write locks, thread implementations). CDefaultGenericFactory<AP, C> overrides only the
// parameterless Create(); the two other overloads keep the base-class default, which throws.
//
// Instances present in the binary (RTTI, vtables @1526856..@1527076, 5 slots each):
//   CDefaultGenericFactory<ISemaphore,  CPosixSemaphore>      Create(string) = 10845, Create(size_t) = 10839
//   CDefaultGenericFactory<IRWSyncCtl,  CPosixRWMutex>        Create(string) = 10863, Create(size_t) = 10856
//   CDefaultGenericFactory<ISyncCtl,    CPosixMutex>          Create(string) = 10868, Create(size_t) = 10865
//   CDefaultGenericFactory<IThreadImpl, simulador::CWasmThread>  Create(string) = 10872, Create(size_t) = 10871
// The web build swaps the POSIX thread implementation for simulador::CWasmThread (cooperative threads:
// the module is compiled without pthreads). The factories are registered in api::CPolySingletonList as
// shared_ptr<IGenericFactory<...>> (default_delete deleters seen in RTTI).
#pragma once

#include <cstddef>
#include <memory>
#include <source_location>
#include <string>

#include "ecourna/api/exception/cbaseerror.hpp"

namespace api {

// Error type of this header: ecourna::api::exception::CBaseError<api::EUePatternError, ...>
// (typeinfo @1526364; constructor thunk = api_f235).
enum class EUePatternError : int;
using CUePatternError = ecourna::api::exception::CBaseError<EUePatternError /*, SErrorLimits{...}*/>;

namespace detail {
// wasm func 1561 (tools: api_f1561) - body shared by all eight default Create overloads
// (wasm-opt merge-similar-functions: srcloc record, error code and message are passed as parameters).
// The first two parameters (this, argument) are unused. It is a pure "throw" helper:
//     throw CUePatternError(codigo, std::string(mensagem), local);
// Emscripten builds the exception inside invoke_* trampolines, so the helper also contains the
// landing pads that free the half-built exception if the std::string constructor throws.
[[noreturn]] void LancaNaoSobrecarregado(int codigo, const char* mensagem,
                                         const std::source_location& local);   // name inferred
} // namespace detail

// vtable layout (all instances): [0] ~IGenericFactory  [1] deleting dtor  [2] Create()
//                                [3] Create(const std::string&)  [4] Create(size_t)
template <typename AP>
class IGenericFactory {
public:
    virtual ~IGenericFactory() = default;                      // slot 0 = icf_ret_this, slot 1 = operator delete

    virtual std::unique_ptr<AP> Create() = 0;                  // slot 2 (CDefaultGenericFactory::vf2)

    // wasm funcs 10845 / 10863 / 10868 / 10872 (slot 3)              srcloc igenericfactory.h:44
    virtual std::unique_ptr<AP> Create(const std::string& /*nome*/)
    {
        // body = detail::LancaNaoSobrecarregado(6761, "...", srcloc :44)   (func 1561)
        throw CUePatternError(EUePatternError{6761}, "Create(const std::string &) não sobrecarregado.");
    }

    // wasm funcs 10839 / 10856 / 10865 / 10871 (slot 4)              srcloc igenericfactory.h:50
    virtual std::unique_ptr<AP> Create(size_t /*valor*/)
    {
        // body = detail::LancaNaoSobrecarregado(6762, "...", srcloc :50)   (func 1561)
        throw CUePatternError(EUePatternError{6762}, "Create(size_t) não sobrecarregado.");
    }
};

template <typename AP, typename C>
class CDefaultGenericFactory : public IGenericFactory<AP> {
public:
    // slot 2 (funcs 10854 etc., not in this unit)
    std::unique_ptr<AP> Create() override { return std::make_unique<C>(); }
};

} // namespace api
