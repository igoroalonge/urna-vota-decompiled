// FRAGMENTS reconstructed from vota_web_wasm.wasm by unit u38 ("app:vota: classes without known file").
//
// Unit u38 has 107 functions. 101 of them are compiler-generated exit-time destructors ("atexit stubs") of
// static objects of the VOTA application, and 6 are small text sources for screens and reports. This
// file has the stubs of the top-level vota/ files. The other stubs are in:
//   src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp   (mesário/operator states, 5 text sources)
//   src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp    (voter states, the BEHB header)
// Full mapping and evidence: docs/modules/u38-app-vota-classes-without-known-file.md
//
// ------------------------------------------------------------------------------------------------------
// What an "atexit stub" is
// ------------------------------------------------------------------------------------------------------
// Every state of the operator and voter state machines is a lazy singleton. The source pattern is
//
//     std::mutex             CFoo::s_mutex;        // 24 bytes (musl pthread_mutex_t on wasm32)
//     std::unique_ptr<CFoo>  CFoo::s_instancia;    // 4 bytes, directly after the mutex (X + 24)
//
//     CFoo& CFoo::GetInst()
//     {
//         std::lock_guard trava(s_mutex);
//         if (!s_instancia)
//             s_instancia.reset(new CFoo());
//         return *s_instancia;
//     }
//
// Each static object with a non-trivial destructor makes clang emit one registration
//     __cxa_atexit(&<helper>, nullptr, &__dso_handle);
// plus the registered helper. On WebAssembly, destructors return `this` (see docs/libraries/libcxx-core.md
// §1.1) and an indirect call with a mismatched signature traps (WebAssemblyCXXABI overrides
// canCallMismatchedFunctionType() to false; ARM, whose destructors also return `this`, does not), so `~T`
// cannot be passed to __cxa_atexit. clang therefore emits a helper through
// CodeGenFunction::generateDestroyHelper: an internal `void __cxx_global_array_dtor(void*)` (renamed
// __cxx_global_array_dtor.1, .2, ... inside one TU) that ignores its (null) argument and destroys the
// variable by address. The observed signature (i32)->void matches it; clang's other kind of stub,
// "__dtor_<mangled var>", is a void() function used only with -fno-use-cxa-atexit. The project names
// these functions "__dtor_<variable>" as a descriptive label (same convention as u39), not as the
// compiler's symbol:
//
//     static void __dtor_s_mutex(void*)     { CFoo::s_mutex.~mutex(); }            // -> func 150
//     static void __dtor_s_instancia(void*) { CFoo::s_instancia.~unique_ptr(); }   // -> "delete p"
//     (real symbol of both: __cxx_global_array_dtor[.N])
//
// In this build, __cxa_atexit is a no-op (EXIT_RUNTIME=0), so LTO/wasm-opt deleted every registering call.
// Only the stubs remain, because their table slots are never compacted. No i32.const of any stub slot is
// left in the code (checked for all 101 stubs). They are dead code: they never run in the simulator.
//
// Two different bodies:
//   * s_mutex:     `~mutex()` = pthread_mutex_destroy(), a no-op without pthreads. What remains is the empty
//                  noexcept __THREW__ check, the ICF body func 150 (tools name "std::mutex::unlock()").
//   * s_instancia: `~unique_ptr()` = `if (T* p = release()) delete p;`. The delete was devirtualised
//                  (the classes are presumably `final`, or whole-program devirtualisation was on; the
//                  binary cannot tell which), so the body calls the class's complete-object destructor
//                  directly and then free(). wasm-opt's merge-similar-functions then folded the stubs of
//                  classes with the same destructor into one body that takes the address as a 2nd
//                  argument, leaving 12-byte thunks:
//                      func 349   delete of a class whose (virtual) destructor has an empty body (free only)
//                      func 389   destructor ICF 244  (one shared_ptr member at +12, e.g. the MT form)
//                      func 763   destructor ICF 448  (shared_ptrs at +12 and +20)
//                      func 2903  destructor ICF 1284 (shared_ptrs at +12, +20, +28)
//                      func 1564  destructor ICF 785  (shared_ptr at +28)
//                      func 1959  IEleitorImpedidoVotar::~IEleitorImpedidoVotar (func 1257)
//                      func 1286  IConfereVotoEmCargo::~IConfereVotoEmCargo (func 1717)
//                  A few stubs kept the destructor call inline (11833, 11844, 11864, 11938).
//
// Where the statics live. The accessors (e.g. func 1901) contain NO guard-variable test. A function-local
// static with a non-trivial destructor would need one (first-pass __cxa_atexit registration). So the
// mutex/unique_ptr pairs are namespace-scope or static data members of the .cpp that defines the class.
// This fragment writes them as static data members. The class template CConfereVotoEmCargo<P, TELA> is
// different: its statics are template static data members. Those are linkonce and need a guard. The 20
// guards (one after each of the 10 mutexes and 10 unique_ptrs, 1837816 ... 1838484) are still set to 1 in
// __wasm_call_ctors (func 14478). See the eleitor fragment.
//
// The stub is emitted in the translation unit that DEFINES the variable. That is why each stub is filed
// below under the original file of its class, not under the function where the accessor was inlined.
// (Exception: the CConfereVotoEmCargo template statics are implicitly instantiated, so their stubs are
// emitted in the TUs that use them - cpedemajoritario.cpp, cpedeproporcional.cpp, cpedenominal.cpp.)
// ------------------------------------------------------------------------------------------------------

#include <memory>
#include <mutex>

#include "vota/iexecucaovota.h"
#include "vota/monitor/cthreadmonitor.h"

namespace vota {

// ======================================================================================================
// uenux2/src/app/vota/iexecucaovota.cpp   (attested: srcloc iexecucaovota.cpp:74 in IExecucaoVota::GetInst)
// ======================================================================================================
// IExecucaoVota::GetInst (wasm func 3594, reconstructed in iexecucaovota.u19.cpp) takes a lock on a static
// mutex around "register CExecucaoVota in the CPolySingletonList if no implementation was pushed, then
// return CPolySingleton<IExecucaoVota>::instance()". The instance itself lives in the CPolySingletonList,
// so there is only a mutex here. The unlock residue is at the end of func 3594: mutex_unlock(1911520).
std::mutex IExecucaoVota::s_mutex;                                      // @1911520   name inferred
//
// wasm func 10237 (table slot 4549) - __dtor_IExecucaoVota::s_mutex -> s_mutex.~mutex()   [body = func 150]

// ======================================================================================================
// uenux2/src/app/vota/monitor/cthreadmonitor.cpp   (attested)
// ======================================================================================================
// CThreadMonitor::GetInst = wasm func 1898 (formerly shown by the tools as
// "comum::util::CMonitoraAlimentacao::CreateInst", the srcloc inlined into it; statics declared in
// cthreadmonitor.h by unit u19: s_mutex @1911572, s_instancia @1911596).
std::mutex                      CThreadMonitor::s_mutex;                // @1911572
std::unique_ptr<CThreadMonitor> CThreadMonitor::s_instancia;            // @1911596 (its stub: func 10229, not u38)
//
// wasm func 10228 (table slot 4565) - __dtor_CThreadMonitor::s_mutex -> s_mutex.~mutex()   [body = func 150]

}  // namespace vota
