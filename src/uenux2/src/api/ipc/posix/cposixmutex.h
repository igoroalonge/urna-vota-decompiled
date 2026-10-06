// uenux2/src/api/ipc/posix/cposixmutex.h   (path inferred)
// Reconstructed from vota_web_wasm.wasm. Unit u18.
#pragma once

#include <pthread.h>

#include "api/ipc/isyncctl.h"     // api::ISyncCtl: [0]/[1] dtor, [2] Lock, [3] Unlock

namespace api {

class CPosixMutex : public ISyncCtl {
public:
    CPosixMutex();                        // wasm 10254 (cposixmutex.cpp:30)
    ~CPosixMutex() override;              // wasm 5355 / 10253
    void Lock() override;                 // wasm 10252 (name attested by the srcloc record cposixmutex.cpp:47)
    void Unlock() override;               // wasm 10251 (name attested by the srcloc record cposixmutex.cpp:56)
private:
    pthread_mutex_t m_mutex;              // +4
    int m_travamentos = 0;                // +28 lock depth (name inferred)
};

} // namespace api
