// FRAGMENT header written by unit u34 (reconstructed from vota_web_wasm.wasm).
// Original file: uenux2/src/api/ipc/clockguard.h  (path inferred: the guard is used by
// api::CPriorityMessageQueue<T> in cmessagequeue.h, next to api::ISyncCtl in isyncctl.h).
//
// RAII lock over the application's own lock interface api::ISyncCtl (vtable slot 2 = Lock, slot 3 = Unlock;
// in the web build a CPosixMutex whose pthread calls are stubs).
#pragma once

#include "api/ipc/isyncctl.h"

namespace api {

class CLockGuard {                                             // 4 bytes: +0 ISyncCtl*    name from u18
public:
    explicit CLockGuard(ISyncCtl& trava) : m_trava(&trava) { m_trava->Lock(); }   // inlined (slot 2)

    // wasm func 5528 (tools: api_f5528) - observed executing (every message posted to a thread queue:
    // its only caller is CPriorityMessageQueue<SMessage>::Add, rhvoice_f501).
    // A destructor is noexcept: the virtual Unlock() is called through invoke_vi and an exception escaping
    // it goes to __clang_call_terminate (std::terminate).
    ~CLockGuard() noexcept { m_trava->Unlock(); }                                  // slot 3

    CLockGuard(const CLockGuard&) = delete;
    CLockGuard& operator=(const CLockGuard&) = delete;

private:
    ISyncCtl* m_trava;
};

}  // namespace api
