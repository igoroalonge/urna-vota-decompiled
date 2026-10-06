// uenux2/src/api/ipc/posix/cposixmutex.cpp
//
// Reconstructed from vota_web_wasm.wasm. Unit u18.
//
// api::CPosixMutex : api::ISyncCtl - recursive pthread mutex. It is the ISyncCtl every VOTA object gets
// from CDefaultGenericFactory<ISyncCtl, CPosixMutex> (wasm 10869: operator new(32) + ctor slot 243):
// CThread::m_pSync, the message queues' locks, ...
//
// RTTI: typeinfo @1600156, vtable @1600092: [0] ~ (5355) [1] deleting ~ (10253) [2] Lock (10252) [3] Unlock (10251)
// Layout (32 bytes): +0 vptr, +4 pthread_mutex_t m_mutex (24 bytes, musl wasm32), +28 int m_travamentos.
//
// Web build: the build has no pthreads, so pthread_mutex_lock/unlock are Emscripten stubs returning 0 and
// were inlined away; Lock/Unlock only update the counter. Their 480-byte stack frame is the leftover of the
// (now dead) std::format error path. Lock/Unlock were observed executing (queue and thread locks).
//
// The dead error paths still left their std::source_location records in the data segment (unreferenced):
//   @1600124 "virtual void api::CPosixMutex::Lock()"   line 47, column 15
//   @1600140 "virtual void api::CPosixMutex::Unlock()" line 56, column 15
// so the slot names are attested. The messages are unreferenced strings of the same pool
// ("Falha ao bloquear mutex: {}" @3610, "Falha ao desbloquear mutex: {}" @3579); the codes 6151/6152 are
// inferred from the EUeIpcError numbering: 6150 (line 30) .. 6158 are exactly the 3 CPosixMutex + 6
// CPosixRWMutex throw sites that precede CPosixSemaphore's first code 6159.
#include "api/ipc/posix/cposixmutex.h"

#include <cstring>
#include <format>
#include <pthread.h>

#include "api/ipc/euipcerror.h"

namespace api {

// wasm func 10254 (srcloc line 30). Table slot 243 (used by the factory).
CPosixMutex::CPosixMutex()
{
    pthread_mutexattr_t atributos;
    pthread_mutexattr_init(&atributos);                                                   // wasm 3977
    int erro = pthread_mutexattr_settype(&atributos, PTHREAD_MUTEX_RECURSIVE);            // wasm 3976 (type folded)
    if (erro == 0)
        erro = pthread_mutex_init(&m_mutex, &atributos);                                  // stub, folded to 0      // ?
    pthread_mutexattr_destroy(&atributos);                                                // no-op                  // ?
    if (erro != 0)
        throw CUeIpcError(EUeIpcError(6150), std::format("Falha ao criar mutex recursivo: {}", std::strerror(erro)));   // line 30
}

// wasm func 5355 (slot 0) / 10253 (slot 1, deleting). pthread_mutex_destroy is a no-op stub.
CPosixMutex::~CPosixMutex()
{
    pthread_mutex_destroy(&m_mutex);                                                                             // ?
}

// wasm func 10252 (slot 2; name attested by the unreferenced srcloc record of line 47)
void CPosixMutex::Lock()
{
    const int erro = pthread_mutex_lock(&m_mutex);   // stub -> 0, so the check below was compiled away
    if (erro != 0)
        throw CUeIpcError(EUeIpcError(6151), std::format("Falha ao bloquear mutex: {}", std::strerror(erro)));      // line 47 (code/text ?)
    ++m_travamentos;                                                                    // order vs. the lock ?
}

// wasm func 10251 (slot 3; name attested by the unreferenced srcloc record of line 56)
void CPosixMutex::Unlock()
{
    --m_travamentos;                                                                    // order vs. the unlock ?
    const int erro = pthread_mutex_unlock(&m_mutex); // stub -> 0
    if (erro != 0)
        throw CUeIpcError(EUeIpcError(6152), std::format("Falha ao desbloquear mutex: {}", std::strerror(erro)));   // line 56 (code/text ?)
}

} // namespace api
