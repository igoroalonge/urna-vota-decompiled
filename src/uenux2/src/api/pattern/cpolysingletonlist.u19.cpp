// Reconstructed from vota_web_wasm.wasm (unit u19).
// Original: uenux2/src/api/pattern/cpolysingletonlist.h  (header-only in the original; this file holds
// the out-of-line pieces as they appear in the binary and the map "template instantiation -> wasm func").
//
// Nothing of CPolySingletonList has its own .cpp in the source tree we know of: every function below is an
// instantiation or an inline function emitted into the translation units that used it. wasm-opt then merged
// bodies that differed only in constants (merge-similar-functions) and split a few (partial inlining).
#include "api/pattern/cpolysingletonlist.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cxxabi.h>
#include <memory>
#include <string>
#include <syslog.h>
#include <unistd.h>

namespace api {

namespace {

// wasm func 13156 (table slot 157)                                                    // name inferred
// Variadic logger. The priority (LOG_INFO) and the format string were constant-folded into the body
// because it has a single caller (func 212); the parameters stay because the function is address-taken
// (called through invoke_viii).
// In the binary: prioridade == LOG_INFO (6) and fmt == "CPolySingletonList::%s(%s[%s])[%s] %s%s" (@78443).
void LogUenux(int prioridade, const char* fmt, ...)
{
    static int s_naUrna = -1;                                        // @1526436, initial value -1
    if (s_naUrna == -1)
        s_naUrna = (access("/dev/urna", F_OK) == 0);                 // the urna has /dev/urna; MEMFS does not

    std::va_list args;
    va_start(args, fmt);
    if (s_naUrna) {
        vsyslog(prioridade, fmt, args);
    } else if (std::getenv("DEBUG_UENUX")) {
        char prefixo[140];                                           // ? filled by a call that the web build
                                                                     //   compiled to nothing (thread/process
                                                                     //   name?): printed UNINITIALISED
        std::printf("%s: ", prefixo);                                // (iprintf, format folded @444300)
        std::vprintf(fmt, args);                                     // no trailing '\n'
    }
    va_end(args);
}

// Call stack text for the log line. In this build it is always empty (the backtrace code, if any,
// is compiled out): the binary builds std::string("") from a literal and move-assigns it.   // name inferred ?
std::string PilhaDeChamadas() { return ""; }

}  // namespace

// wasm func 212 (table slot 146)                                                       // name inferred
// Output: "CPolySingletonList::<funcao>(<mangled>[<demangled>])[<detalhe>] {calls}<pilha>"
void CPolySingletonList::log(const std::string& funcao, const std::string& tipo, const std::string& detalhe)
{
    int status = 0;
    std::unique_ptr<char, void (*)(void*)> nomeReal(
        abi::__cxa_demangle(tipo.c_str(), nullptr, nullptr, &status), &std::free);   // func 6222, free = slot 167
    std::string demangled;
    if (status == 0)
        demangled = nomeReal.get();                                  // slot 168 -> func 12979
    else
        demangled = std::string("...");

    std::string pilha;
    if (funcao.substr(0, 4) != "list")
        pilha = PilhaDeChamadas();

    LogUenux(LOG_INFO, "CPolySingletonList::%s(%s[%s])[%s] %s%s",
             funcao.c_str(), tipo.c_str(), demangled.c_str(), detalhe.c_str(),
             pilha.empty() ? "" : "{calls}", pilha.c_str());
}

// wasm func 14476 (table slot 152): CPolySingletonList::trace() (inline in the header), out-of-line copy.

// =======================================================================================================
// Instantiation map (every function index of unit u19 that belongs to this header)
// =======================================================================================================
//
// ---- CPolySingletonList::push<INTERFACE>(std::unique_ptr<INTERFACE>&&, TPolySingletonsInfo&) ---------
// ---- and the register-or-replace wrapper replace<INTERFACE> (= "if (exists) erase" + push) ------------
//  (all contain the srcloc record cpolysingletonlist.h:129, which belongs to push)
//  push alone (no exists/erase prefix; called directly by GetInst() functions and by replace):
//   5384  push<api::ISound>                              out-of-line, landing pads (callers: 8302, where replace
//                                                        is inlined, and replace<api::ISound> = libcxx_f10356,
//                                                        not u19, called by the main lambda)
//   5395  push<vota::IExecucaoVota>                      -> merged body 3882 (callers: GetInst 3594, replace 8728)
//   5398  push<vota::impl::ISincronismoVotoEleitor>      -> 3882 (callers: 7181, replace wasm_entry_f9437)
//   5407  push<vota::impl::IPoliticaExecucaoEleitor>     -> 3882 (callers: 5925, replace wasm_entry_f9507)
//   3882  merged push body (unique_ptr&, info, &typeid, slot emplace, slot make_shared, &srcloc:129, &name)
//  replace with push inlined, unique_ptr by reference:
//   7667  replace<api::ITextToSpeech>                    (only caller: func 4162)
//   8758  replace<api::IGenericFactory<api::ISemaphore>> (table slot 135, from main through 1562)
//   8834  replace<api::IGenericFactory<api::IRWSyncCtl>>   (slot 134)
//   8911  replace<api::IGenericFactory<api::ISyncCtl>>     (slot 133)
//   8986  replace<api::IGenericFactory<api::IThreadImpl>>  (slot 132)
//   9039  replace<api::IFingerPrepare>                     (slot 131)
//   9135  replace<ecourna::api::security::ISymmetricCipherFactory>  (slot 130)
//   9233  replace<ecourna::api::security::IRng>            (slot 129)
//  10090  replace<comum::IInterfaceSavd>                   (slot 113, called directly by main)
//  replace without push inlined:
//   8728  replace<vota::IExecucaoVota>: "if (exists) erase", then push 5395 (slot 136, from main via 1562)
//         (the same shape outside u19: wasm_entry_f9437, wasm_entry_f9507, libcxx_f10356 for ISound)
//  by-value helper (unique_ptr passed as the raw pointer: libc++ ABI v2 trivial_abi, callee destroys it):
//   4162  f(std::unique_ptr<api::ITextToSpeech>, info) -> replace 7667  (table slots 7, 46; used by votaInit
//         and 8302; deleter = vtable slot 10) — same shape as the merged body 1562 (not u19) used by main
//   4879  the same helper for api::ITimerScheduler with replace+push inlined (callers: ITimerScheduler::
//         CreateInst 5444, 8302)
//   4890  the same helper for api::ISystemDateTime with replace+push inlined (callers: 1155, 8302)
//
// ---- CPolySingletonList::exists<INTERFACE>(TPolySingletonsInfo&) -------------------------------------
//  generic body with landing pads (string(name), lock_shared 3095, find 608):
//   1166  body(info, name)          thunks: 3281 ITextToSpeech, 3343 ISound, 3618 IGenericFactory<ISemaphore>,
//                                   3627 IGenericFactory<ISyncCtl>, 5417 IGenericFactory<IRWSyncCtl>,
//                                   5423 IGenericFactory<IThreadImpl>, 5425 IFingerPrepare,
//                                   5427 ISymmetricCipherFactory, 5431 IRng, 5434 IInterfaceSavd, 5436 IInputKbd
//   3928  body(info, &typeinfo.__name)   thunks: 3391 CEscritorLog, 3392 IImpressoraRelatorios, 3394 IInterfaceInit
//  inline-key bodies (the name is copied with 8-byte loads; merged by name length):
//   6131  12-char names             thunks: 3386 IBeep, 4871 IUrna
//   6132  16-char names             thunks: 4872 IResource, 4877 IScreenMT
//   6057  20-char names             thunks: 2886 IQRCodeBUDS (and 1954 CCalculaCV, not u19)
//   2924  23-char names             thunks: 1654 ITimerScheduler, 2160 ISystemDateTime, 2469 IAjusteDataHora
//                                   (and 2450 vota::IAjusteInicial, not u19)
//  single instantiations:
//   2446 comum::IControladorRegistraMesarios   2509 comum::IEventosLog   3387 api::IScreen
//   3622 vota::impl::IInformacaoThreadOperador 3686 comum::impl::IValidaMidia 4853 api::IPaperRelatorios
//   4867 api::IPower                           4873 api::IInputMT
//   2736 vota::IExecucaoVota -> body 3884 (not u19)
//  added by unit u41 (the tools had left them as api_fNNNN):
//   3884  body(info, &typeinfo.__type_name) with landing pads: func 1166 plus one load of the name pointer
//         (see the comment on exists<> in cpolysingletonlist.h). Thunks: 2736 vota::IExecucaoVota,
//   2741  vota::impl::ISincronismoVotoEleitor   (callers: 7181 CSincronismoEleitor::ProcessMessage =
//         inlined ISincronismoVotoEleitor::GetInst, 9437 replace<> from main, __wasm_call_ctors)
//   2744  vota::impl::IPoliticaExecucaoEleitor  (callers: 5925 CConfirmaVotoNominal::PosTecla = inlined
//         IPoliticaExecucaoEleitor::GetInst, 9507 replace<> from main, __wasm_call_ctors)
//  Many of these thunks are also called from __wasm_call_ctors with the result discarded (static
//  initialisers of the interfaces' translation units).
//
// ---- other members ------------------------------------------------------------------------------------
//    305 / 608 / 6161  find (two thunks of one merged body; lambda = func 6779 via slots 156/162)
//    640  erase                                         (table slot 151)
//    212  log                                           (table slot 146)
//  14476  trace                                         (table slot 152)
//  13156  LogUenux (printf/syslog backend)              (table slot 157)
//   1126  CUpgradeMutex::lock()   3095 lock_shared()   13621 upgrade()/unlock_shared_and_lock()
//    642  ~unique_lock<CUpgradeMutex>   1322 ~shared_lock<CUpgradeMutex>   6147 their merged body
//    235  CUePatternError(code, msg, loc) constructor thunk -> shared_f2294 with vtable @1526448
//   2584  SPolySingleton::~SPolySingleton
//  10620  TPolySingletonsInfo::~TPolySingletonsInfo (static object, exit-time)
//
// ---- libc++ helpers instantiated for SPolySingleton / shared_ptr / std::format -------------------------
//    631  vector<SPolySingleton>::__emplace_back_slow_path(const char*&, const type_info*, const shared_ptr&)
//   4852  vector<SPolySingleton>::__base_destruct_at_end
//   5786  vector<SPolySingleton>::__swap_out_circular_buffer   5779 __split_buffer<SPolySingleton>::~__split_buffer
//  11544  destroy range [first,last)   11533 _AllocatorDestroyRangeReverse::operator()   5774 / 3895 its
//         __exception_guard destructor (3895 = merged body; 5310 and 5562 are the same guard for two
//         unrelated vectors: rollback slots 473 -> comum_f9873, 235 -> libcxx_f11146)
//   1010  shared_ptr<T>(unique_ptr<T>&&) merged body (control-block vtable as parameter); thunks:
//         10335 ISound, 10432 IExecucaoVota, 10470 ISincronismoVotoEleitor, 10513 IPoliticaExecucaoEleitor,
//         10555 IGenericFactory<ISemaphore>, 10585 <IRWSyncCtl>, 10618 <ISyncCtl>, 10652 <IThreadImpl>,
//         10687 IFingerPrepare, 10723 ISymmetricCipherFactory, 10759 IRng, 10807 IInterfaceSavd,
//         14456 ITextToSpeech
//   emplace_back thunks -> func 1009 (element ctor func 1005 by table slot):
//         10325 ISound, 10547 <ISemaphore>, 10582 <IRWSyncCtl>, 10607 <ISyncCtl>, 10643 <IThreadImpl>,
//         10679 IFingerPrepare, 10714 ISymmetricCipherFactory, 10750 IRng, 10801 IInterfaceSavd,
//         14423 ITextToSpeech
//    388  shared_ptr<T>::~shared_ptr (ICF, 20+ callers)    4180  shared_ptr<T>::operator=(shared_ptr&&) (ICF)
//    327  std::to_string(unsigned) (loc.line())         321  __format::__allocating_buffer<char> destructor
//    429 / 430 / 5650 / 5656  std::make_format_args("instance", ...) for the two instance() messages
//   1113  make_format_args(size_t, const void*)  ("sz[{}] ptr[{}]")
//   1119  make_format_args("push", const char*)  ("{}: instância já criada de {}")
//   5646 / 1549  the __create_packed_storage lambdas for const char[9] and const std::string&

}  // namespace api
