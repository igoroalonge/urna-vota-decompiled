// uenux2/src/api/ipc/posix/cposixsemaphore.h   (path inferred)
// Reconstructed from vota_web_wasm.wasm. Unit u18.
#pragma once

#include <cstddef>
#include <semaphore.h>

#include "api/ipc/isemaphore.h"   // api::ISemaphore : ISyncCtl, api::ESemaphoreResult (0 = signalled)

namespace api {

class CPosixSemaphore : public ISemaphore {
public:
    explicit CPosixSemaphore(unsigned int valorInicial);        // wasm 10250 (cposixsemaphore.cpp:23)
    ~CPosixSemaphore() override;                                // wasm 5354 / 10249
    void Lock() override;                                       // slot 2 (2219)  name attested (srcloc :90)
    void Unlock() override;                                     // slot 3 (2219)  name attested (srcloc :99)
    void Wait() override;                                       // slot 4 (10248) name inferred
    ESemaphoreResult Wait(std::size_t timeoutMs) override;      // slot 5 (10247, cposixsemaphore.cpp:48/:76)
    int GetValue() const override;                              // slot 6 (10246) name attested (srcloc :114)
private:
    sem_t m_semaforo;                                           // +4
};

} // namespace api
