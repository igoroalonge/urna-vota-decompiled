// Reconstructed from vota_web_wasm.wasm (unit u20).
// Original: uenux2/src/api/util/itimerscheduler.h (srcloc itimerscheduler.h:40).
//
// api::ITimerScheduler - the GUI's timer service (clock of the status header, blinking fields, battery
// icon updates, CTextFieldUpdate / CImageFieldUpdate refreshes). Registered in api::CPolySingletonList;
// the only implementation is api::CTimerScheduler (4-byte object = vptr, vtable @1585404).
#pragma once

#include "api/pattern/cpolysingletonlist.h"

namespace api {

class ITimerScheduler {
public:
    virtual ~ITimerScheduler() = default;

    static ITimerScheduler& GetInst();                  // itimerscheduler.cpp:23 (func 1259)

    // wasm func 5444 (T = api::CTimerScheduler)                          srcloc itimerscheduler.h:40
    // GetInst() is inlined at the end, which makes the function (harmlessly) self-recursive in wasm.
    template <typename T>
    static ITimerScheduler& CreateInst()
    {
        if (CPolySingletonList::exists<ITimerScheduler>())                       // api_f1654
            throw CUeUtilError(EUeUtilError{7085}, "Tentativa de recriar o singleton");
        CPolySingletonList::push<ITimerScheduler>(new T());                      // func 4879
        return GetInst();
    }
};

} // namespace api
