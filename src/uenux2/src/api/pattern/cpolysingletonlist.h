// Reconstructed from vota_web_wasm.wasm (unit u19).
// Original: uenux2/src/api/pattern/cpolysingletonlist.h
//   (attested by std::source_location records: line 99 and 105 in
//    "static INTERFACE &api::CPolySingletonList::instance(TPolySingletonsInfo &) [INTERFACE = ...]"
//    and line 129 in
//    "static void api::CPolySingletonList::push(std::unique_ptr<INTERFACE> &&, TPolySingletonsInfo &) [INTERFACE = ...]")
//
// "Poly-singleton" = a singleton looked up by *interface*: the application asks for "the" api::IScreen,
// api::IInputKbd, comum::IInterfaceSavd, vota::IExecucaoVota ... and gets whatever implementation was
// registered for it. This is the seam that lets the same voting application run on the urna (real drivers)
// and in the browser (simulador::CWasm* mocks registered by main/votaInit, see csimuladorwasm.u19.cpp).
//
// The registry is ONE vector of {mangled type name, &typeid, shared_ptr<void>} protected by a
// writer-preferring read/write lock with an "upgrade" operation. Lookups are linear (std::find_if over
// 24-byte entries comparing std::string keys). There are ~28 entries after start-up (verified with
// DEBUG_UENUX=1, see docs/modules/u19-*.md §4).
//
// Everything here is header-only template code. In the binary it exists as:
//   * CPolySingleton<T>::instance(info, loc) bodies with CPolySingletonList::instance<T> inlined
//     (named "api::CPolySingletonList::instance@N" by the tools; they are listed in cpolysingleton.h),
//   * 18 push-related bodies in u19 (push<T> itself, the register-or-replace wrapper with push inlined,
//     one by-value helper), exists<T> bodies/thunks (37), find/erase/log helpers,
//   * the lock primitives of CUpgradeMutex (out-of-line copies) and libc++ vector/shared_ptr helpers.
// See cpolysingletonlist.u19.cpp for the function-index map of every instantiation.
//
// Error types:
//   CUePatternError = ecourna::api::exception::CBaseError<api::EUePatternError, SErrorLimits{6750, 6800}>
//     (typeinfo @1526364, vtable @1526448, constructor thunk = wasm func 235)
//       6754  "{}: solicitada uma instância não criada de {}"                  (instance, line 99)
//       6755  "{}: instância {} não corresponde a interface solicitada de {}"   (instance, line 105)
//       6756  "{}: instância já criada de {}"                                   (push, line 129)
//   std::runtime_error from the lock:  "unlock_shared() called with no shared owner",
//     "unlock() called when no writer is active", "unlock_shared_and_lock() requires an active shared owner"
#pragma once

#include <algorithm>
#include <condition_variable>
#include <cstddef>
#include <format>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <source_location>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <utility>
#include <vector>

#include "ecourna/api/exception/cbaseerror.hpp"

namespace api {

enum class EUePatternError : int {                       // names inferred, values attested
    InstanciaNaoCriada        = 6754,                    // ?
    InstanciaIncompativel     = 6755,                    // ?
    InstanciaJaCriada         = 6756,                    // ?
};
using CUePatternError = ecourna::api::exception::CBaseError<EUePatternError /*, SErrorLimits{6750, 6800}*/>;

// -------------------------------------------------------------------------------------------------------
// Read/write lock used by the registry (132 bytes on wasm32).                            // name inferred
// Writer-preferring: readers wait while a writer holds the lock OR is waiting for it.
// In this build (no pthreads) std::condition_variable::wait returns immediately (func 251), so the
// "while (!pred) wait()" loops below spin forever if the predicate is false: the lock is only correct
// as long as nobody re-enters it (see docs, "weird code").
class CUpgradeMutex {
public:
    // wasm func 1126 (out-of-line copy; inlined everywhere else)
    void lock()
    {
        std::unique_lock trava(m_mutex);
        ++m_escritoresEsperando;
        m_portaEscritores.wait(trava, [this] { return !m_escritorAtivo && m_leitores == 0; });   // func 6700
        m_escritorAtivo = true;
        --m_escritoresEsperando;
    }

    // wasm func 624 (shared_f624; not in u19)
    void unlock()
    {
        std::unique_lock trava(m_mutex);
        if (!m_escritorAtivo)
            throw std::runtime_error("unlock() called when no writer is active");
        m_escritorAtivo = false;
        if (m_escritoresEsperando > 0)
            m_portaEscritores.notify_one();
        else
            m_portaLeitores.notify_all();
    }

    // wasm func 3095 (out-of-line copy)
    void lock_shared()
    {
        std::unique_lock trava(m_mutex);
        m_portaLeitores.wait(trava, [this] { return !m_escritorAtivo && m_escritoresEsperando == 0; });  // func 13876
        ++m_leitores;
    }

    // wasm func 13974 (unknown_f13974; not in u19)
    void unlock_shared()
    {
        std::unique_lock trava(m_mutex);
        if (m_leitores <= 0)
            throw std::runtime_error("unlock_shared() called with no shared owner");
        if (--m_leitores == 0 && m_escritoresEsperando > 0)
            m_portaEscritores.notify_one();
    }

    // "Upgrade" a shared (reader) ownership into exclusive ownership.
    // wasm func 13621 (sret, this, &shared_lock): the body of unlock_shared_and_lock() plus the lock
    // transfer from the std::shared_lock to a std::unique_lock. The shared_lock is emptied only AFTER the
    // upgrade succeeded (if unlock_shared_and_lock throws, it still owns the shared lock).
    std::unique_lock<CUpgradeMutex> upgrade(std::shared_lock<CUpgradeMutex>& leitura)   // name inferred
    {
        unlock_shared_and_lock();
        std::unique_lock<CUpgradeMutex> escrita(*this, std::adopt_lock);
        leitura.release();                                           // no unlock_shared()
        return escrita;
    }

    void unlock_shared_and_lock()
    {
        std::unique_lock trava(m_mutex);
        if (m_leitores <= 0)
            throw std::runtime_error("unlock_shared_and_lock() requires an active shared owner");
        --m_leitores;
        ++m_escritoresEsperando;
        m_portaEscritores.wait(trava, [this] { return !m_escritorAtivo && m_leitores == 0; });   // func 6700
        m_escritorAtivo = true;
        --m_escritoresEsperando;
    }

private:
    std::mutex              m_mutex;                     // +0   (24 bytes, pthread stubs in this build)
    std::condition_variable m_portaLeitores;             // +24  "gate 1"
    std::condition_variable m_portaEscritores;           // +72  "gate 2"
    int                     m_leitores = 0;              // +120 active readers
    int                     m_escritoresEsperando = 0;   // +124 writers waiting
    bool                    m_escritorAtivo = false;     // +128
};
// std::unique_lock<CUpgradeMutex>::~unique_lock  = wasm func 642  (-> merged body 6147, unlock = slot 160)
// std::shared_lock<CUpgradeMutex>::~shared_lock  = wasm func 1322 (-> merged body 6147, unlock_shared = slot 163)

// -------------------------------------------------------------------------------------------------------
// One registry entry (24 bytes).                                                          // name inferred
struct SPolySingleton {
    std::string           nome;        // +0   typeid(INTERFACE).name(), the mangled name ("N3api7IScreenE")
    const std::type_info* tipo;        // +12  &typeid(INTERFACE)
    std::shared_ptr<void> instancia;   // +16  (+20 control block: __shared_ptr_pointer<INTERFACE*, default_delete>)
};
// SPolySingleton::~SPolySingleton = wasm func 2584 (shared_ptr then string).
// Element constructor used by emplace_back = func 1005 (string(nome), tipo, copy of the shared_ptr).

// The registry ("TPolySingletonsInfo" is attested by the srcloc signatures). 148 bytes.
// The single instance lives at 0x1BF600 = 1832448 (static local of the accessor below).
struct TPolySingletonsInfo {
    std::vector<SPolySingleton> lista;     // +0
    bool                        debug = false;   // +12 tested by trace(); the accessor sets it to true  // name inferred
    CUpgradeMutex               mutex;     // +16
};
// TPolySingletonsInfo::~TPolySingletonsInfo = wasm func 10620 (table slot 2; destroys the cv/mutexes, then
// the vector through func 4852). Only runs at exit, i.e. never in the browser (noExitRuntime).

// Accessor. All code reaches it through a *function pointer* stored in data (@1526320 = table slot 284 ->
// func 11265), so every "GetPolySingletonsInfo()" in the binary is a call_indirect.            // name inferred
inline TPolySingletonsInfo& DefaultPolySingletonsInfo()                  // wasm func 11265 (not in u19)
{
    static TPolySingletonsInfo info;       // guard byte @1832596 (dtor registration folded away)
    info.debug = true;                     // stored on every call, outside the guard            // ?
    return info;
}
inline TPolySingletonsInfo& (*GetPolySingletonsInfo)() = &DefaultPolySingletonsInfo;   // @1526320  // name inferred

// -------------------------------------------------------------------------------------------------------
class CPolySingletonList {
public:
    using TIterador = std::vector<SPolySingleton>::iterator;

    // Linear search by mangled name.
    // wasm func 305 and func 608 (two identical thunks of the merged body 6161; the lambda is func 6779,
    // std::find_if over 24-byte elements, reached through table slots 156 / 162).       // name inferred
    static std::pair<bool, TIterador> find(const std::string& nome, TPolySingletonsInfo& info)
    {
        const auto it = std::find_if(info.lista.begin(), info.lista.end(),
                                     [nome](const SPolySingleton& e) { return e.nome == nome; });
        return {it != info.lista.end(), it};
    }

    // Diagnostic line, printed only on a real urna (syslog) or with DEBUG_UENUX set (stdout).
    // wasm func 212 (always prints) and func 14476 (prints only when info.debug).      // names inferred
    static void log(const std::string& funcao, const std::string& tipo, const std::string& detalhe);
    static void trace(const std::string& funcao, const std::string& tipo, const std::string& detalhe)
    {
        if (GetPolySingletonsInfo().debug)
            log(funcao, tipo, detalhe);
    }

    // Is there an entry for INTERFACE?   (37 functions: see cpolysingletonlist.u19.cpp)  // name inferred
    // Other unit docs call this "contains".
    //
    // wasm func 3884 (unit u41): one of the merged bodies of this template, `bool body(info, const char* const*
    // pNome)`. It reads the name through a pointer to the type_info's name field, and the thunks pass
    // &typeid(INTERFACE) + 4 (the libc++ type_info::__type_name field) as a constant:
    //     2736 exists<vota::IExecucaoVota>                   (&__type_name = 1600568)
    //     2741 exists<vota::impl::ISincronismoVotoEleitor>   (1534152 -> "N4vota4impl23ISincronismoVotoEleitorE")
    //     2744 exists<vota::impl::IPoliticaExecucaoEleitor>  (1536364 -> "N4vota4impl24IPoliticaExecucaoEleitorE")
    // Its code is func 1166 (the body that takes the name string itself) plus one extra i32.load. In these
    // instantiations typeid(INTERFACE).name() was not folded to a string constant. It stayed a load from the
    // type_info object (`i32.const ti; i32.load offset=4`, which wasm-opt rewrote to `i32.const ti+4; i32.load`).
    // wasm-opt's merge-similar-functions then turned that constant into the parameter. Step by step:
    //   1. std::string nome(*pNome)                 func 148, before the lock: nothing to undo if it throws
    //   2. std::shared_lock leitura(info.mutex)     {&info+16, owns=true}; lock_shared = func 3095 (invoke).
    //                                               If it throws, only `nome` is destroyed (the shared_lock
    //                                               was never constructed)
    //   3. find(nome, info).first                   func 608 (invoke, sret pair at sp+4; .first = byte +0)
    //   4. ~shared_lock (func 1322), ~string (149)  also on the landing pad of 3, then __resumeException
    // Observed executing. exists<IExecucaoVota> runs on every IExecucaoVota::GetInst() (func 3594). The web
    // adapter calls that on every votaTick (CurrentStateName 5408, CVotaWebEngine::BuildStateJson 5500). The
    // edge 3594 -> 2736 has 1450 profiler samples in vote_geral_t1, about 1% of votaTick, most of them in
    // lock_shared/find/string. exists<ISincronismoVotoEleitor> ran from
    // __wasm_call_ctors, main (9437) and CSincronismoEleitor::ProcessMessage (7181).
    // exists<IPoliticaExecucaoEleitor> ran from main (9507).
    template <typename INTERFACE>
    static bool exists(TPolySingletonsInfo& info)
    {
        const std::string nome = typeid(INTERFACE).name();
        std::shared_lock leitura(info.mutex);
        return find(nome, info).first;
    }

    // Removes the entry called `nome`, destroying the instance if nobody else owns it.
    // wasm func 640.                                                                        // name inferred
    static void erase(const std::string& nome, TPolySingletonsInfo& info)
    {
        std::shared_lock leitura(info.mutex);                        // 3095 ... 1322
        const auto [encontrado, it] = find(nome, info);
        if (encontrado) {
            std::unique_lock escrita = info.mutex.upgrade(leitura);  // 13621 ... 642
            std::shared_ptr<void> manter = it->instancia;           // released below, still under the lock
            info.lista.erase(it);                                    // func 13540 (vector::erase)
        }
    }

    // The registered implementation of INTERFACE.
    // Line 99: no entry; line 105: the entry was registered with another type_info.
    template <typename INTERFACE>
    static INTERFACE& instance(TPolySingletonsInfo& info = GetPolySingletonsInfo())
    {
        std::shared_lock leitura(info.mutex);
        const std::string nome = typeid(INTERFACE).name();
        const auto [encontrado, it] = find(nome, info);
        if (!encontrado) {
            log("instance", nome, "find");
            throw CUePatternError(EUePatternError::InstanciaNaoCriada,
                                  std::format("{}: solicitada uma instância não criada de {}", "instance", nome),
                                  std::source_location::current());                         // :99
        }
        if (*it->tipo != typeid(INTERFACE)) {                        // libc++: compares the name pointers
            log("instance", nome, "typeid");
            throw CUePatternError(EUePatternError::InstanciaIncompativel,
                                  std::format("{}: instância {} não corresponde a interface solicitada de {}",
                                              "instance", it->nome, nome),
                                  std::source_location::current());                         // :105
        }
        const std::shared_ptr<INTERFACE> p = std::static_pointer_cast<INTERFACE>(it->instancia);
        return *p;                                                   // a raw reference escapes the lock
    }

    // Registers the implementation of INTERFACE. Throws 6756 (line 129) if one is already registered.
    // push itself has NO "exists -> erase" prefix: the GetInst() functions 599, 3594, 5575, 5925 and 7181
    // test exists() once and then run this body directly (inlined, or the out-of-line copies 5384 ISound,
    // 5395/5398/5407 -> merged body 3882), with no second exists() call. Replacement is done by
    // replace() below.
    template <typename INTERFACE>
    static void push(std::unique_ptr<INTERFACE>&& instancia, TPolySingletonsInfo& info = GetPolySingletonsInfo())
    {
        const char* nome = typeid(INTERFACE).name();
        std::unique_lock escrita(info.mutex);                        // CUpgradeMutex::lock (1126)
        if (find(nome, info).first) {                                // unreachable after exists()/erase()
            log("push", nome, "find");                               //   in a single thread
            throw CUePatternError(EUePatternError::InstanciaJaCriada,
                                  std::format("{}: instância já criada de {}", "push", nome),
                                  std::source_location::current());                         // :129
        }
        trace("push", nome, std::format("sz[{}] ptr[{}]", info.lista.size(), static_cast<const void*>(&info)));
        const std::shared_ptr<INTERFACE> p(std::move(instancia));    // funcs 1010 / 10335 ...
        info.lista.emplace_back(nome, &typeid(INTERFACE), p);        // funcs 1009 / 10325 ... / 631
    }

    // Registers OR REPLACES the implementation of INTERFACE.                 // name inferred (no string/srcloc)
    // Replacing destroys the previous object (erase), even if callers still hold the INTERFACE&
    // that instance() returned for it.
    // In the binary: 8728 (IExecucaoVota; push not inlined, calls 5395), and with push inlined 7667, 8758,
    // 8834, 8911, 8986, 9039, 9135, 9233, 10090, the inline copies in 8302, 1956 and 10876. The start-up code
    // reaches it through a by-value helper, f(std::unique_ptr<INTERFACE> p, info) { replace(std::move(p), info); }
    // (func 4162 for ITextToSpeech, merged body 1562 for main's calls; 4879/4890 are that helper with replace
    // and push inlined): libc++ ABI v2 makes std::unique_ptr [[clang::trivial_abi]], so a by-value unique_ptr
    // travels as the raw pointer and the callee destroys it.
    template <typename INTERFACE>
    static void replace(std::unique_ptr<INTERFACE>&& instancia, TPolySingletonsInfo& info = GetPolySingletonsInfo())
    {
        if (exists<INTERFACE>(info))
            erase(typeid(INTERFACE).name(), info);
        push(std::move(instancia), info);
    }
};

}  // namespace api
