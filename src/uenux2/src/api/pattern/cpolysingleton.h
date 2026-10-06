// Reconstructed from vota_web_wasm.wasm (unit u19).
// Original: uenux2/src/api/pattern/cpolysingleton.h
//   (attested: std::source_location record at line 78 in
//    "static T &api::CPolySingleton<T>::instance(TPolySingletonsInfo &, const std::source_location &)")
//
// The typed front end of the registry in cpolysingletonlist.h. Interfaces "inherit" their GetInst() from it:
//     api::IScreen& tela = api::CPolySingleton<api::IScreen>::instance(api::GetPolySingletonsInfo());
// The caller's std::source_location is only used to build the error message, so a missing implementation
// is reported as "PolySingleton - solicitada uma instancia nao criada N3api7IScreenE [/home/.../x.cpp:123]".
//
// Error type: CPatternError = ecourna::api::exception::CBaseError<ecourna::api::pattern::EPatternError,
//             SErrorLimits{1300, 1325}> (typeinfo @1526600, vtable @1526620, constructor thunk = wasm func 331).
//             Code 1301 (the sibling code 1303 is "CRespostas - instancia nao criada").
#pragma once

#include <source_location>
#include <string>
#include <typeinfo>

#include "api/pattern/cpolysingletonlist.h"
#include "ecourna/api/pattern/epatternerror.hpp"     // ecourna::api::pattern::EPatternError, CPatternError  (?)

namespace api {

template <typename T>
class CPolySingleton {
public:
    static bool exists()                                                                   // name inferred
    {
        return CPolySingletonList::exists<T>(GetPolySingletonsInfo());
    }

    // Every instantiation below is one wasm function (tools name "api::CPolySingletonList::instance@N"),
    // with CPolySingletonList::instance<T>, exists<T> (or a call to its thunk) and the lock inlined.
    static T& instance(TPolySingletonsInfo& info, const std::source_location& loc)
    {
        if (!exists())
            throw ecourna::api::pattern::CPatternError(
                static_cast<ecourna::api::pattern::EPatternError>(1301),
                "PolySingleton - solicitada uma instancia nao criada " + std::string(typeid(T).name())
                    + std::string(" [") + loc.file_name() + ":" + std::to_string(loc.line()) + "]",
                std::source_location::current());                                          // :78
        return CPolySingletonList::instance<T>(info);
    }
};

// Instantiations present in the binary (wasm func: T  [exists<T> used]  {run = executed in recorded votes})
//   356  comum::IControladorRegistraMesarios          [2446]
//   383  api::IInputMT                                [4873]
//   455  api::IInputKbd                               [5436]   run  (out-of-line instance<T>: landing pads)
//   512  api::IScreen                                 [3387]   run
//   611  comum::IInterfaceInit                        [3394]   run
//   816  api::IFingerScanner                          thunk -> merged body 6013
//   837  comum::IEventosLog                           [2509]   run
//   862  api::IPower                                  [4867]   run
//   905  api::IPaperRelatorios                        [4853]
//   923  api::IUrna                                   [4871]   run
//   925  api::IBeep                                   [3386]   run
//  1091  api::ISound                                  [3343]   run  (out-of-line instance<T>: landing pads)
//  1282  comum::CCalculaCV                            [1954]
//  1686  api::IFingerMatcher                          thunk -> merged body 6013
//  1714  api::IScreenMT                               [4877]
//  1822  comum::IInterfaceSavd                        [5434]   run
//  2030  ecourna::api::security::IRng                 [5431]
//  2079  api::ITextToSpeech                           [3281]
//  2279  api::IKernelHSM                              [inlined]
//  3075  api::IResource                               [4872]   run
//  3164  api::IGenericFactory<api::ISyncCtl>          [3627]   run
//  3204  api::IAjusteDataHora                         [2469]
//  3615  api::IFingerDetection                        [inlined]
//  3704  api::pkcs11::IPkcs11                         [inlined]
//  4356  api::IGenericFactory<api::ISemaphore>        [3618]
//  5361  api::IGenericFactory<ecourna::api::security::ITextEncoding>  [inlined]
//  5362  api::IGenericFactory<ecourna::api::security::IHash>          [inlined]
//  5443  api::ITimerScheduler                         [1654]   run
// 11157  api::CEscritorLog                            [3391]
//  6013  merged body (wasm-opt merge-similar-functions) of the two 22-character names IFingerScanner /
//        IFingerMatcher: (info, loc, &srcloc:105, name, &srcloc:99, name+8, name+14, &srcloc:78)
//        (the 22-byte key is copied with three overlapping 8-byte loads at +0, +8 and +14)
// Also inlined (with a lazy default registration) into the GetInst() functions 599, 1155, 1956, 3594,
// 5575, 5925 and 7181, and (guarded by exists(), no default) into CLp::GetInst 3876; they are reconstructed
// with their own classes (see the u19 doc).

}  // namespace api
