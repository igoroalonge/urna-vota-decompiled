// uenux2/src/api/ipc/posix/cposixrwmutex.h   (path inferred)
// Reconstructed from vota_web_wasm.wasm. Unit u18.
#pragma once

#include <pthread.h>

#include "api/ipc/irwsyncctl.h"   // api::IRWSyncCtl: [0]/[1] dtor, [2] ReadLock, [3] WriteLock, [4] Unlock

namespace api {

class CPosixRWMutex : public IRWSyncCtl {
public:
    CPosixRWMutex();                      // wasm 10245 (srcloc records :29, :35, :42 of its dead error paths)
    ~CPosixRWMutex() override;            // wasm 5353 / 10244
    void ReadLock() override;             // slot 2 (wasm 2219, ICF)   name attested (srcloc :58); slot order inferred
    void WriteLock() override;            // slot 3 (wasm 2219, ICF)   name attested (srcloc :69); slot order inferred
    void Unlock() override;               // slot 4 (wasm 2219, ICF)   name attested (srcloc :80); slot order inferred
private:
    pthread_rwlock_t m_lock;              // +4
};

} // namespace api
