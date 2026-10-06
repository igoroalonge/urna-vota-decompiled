// uenux2/src/api/ipc/posix/cposixrwmutex.cpp
//
// Reconstructed from vota_web_wasm.wasm. Unit u18.
//
// api::CPosixRWMutex : api::IRWSyncCtl - pthread read/write lock, created by
// CDefaultGenericFactory<IRWSyncCtl, CPosixRWMutex> (wasm 10864: operator new(36) + ctor slot 244).
// RTTI: typeinfo @1600456, vtable @1600340: [0] ~ (5353) [1] deleting ~ (10244) [2..4] 2219 (ICF no-op).
// Layout (36 bytes): +0 vptr, +4 pthread_rwlock_t m_lock (32 bytes, musl wasm32).
//
// Web build: every pthread_rwlock_* call is a stub, so the three lock/unlock slots collapsed into the shared
// empty body 2219 (which only keeps a dead 480-byte frame of the removed error path) and the constructor
// only zeroes the 8-byte pthread_rwlockattr_t local (musl's pthread_rwlockattr_init, inlined). Not observed
// executing.
//
// The removed error paths left unreferenced std::source_location records (analysis/srcloc.tsv):
//   @1600360/@1600376/@1600392  api::CPosixRWMutex::CPosixRWMutex()  lines 29, 35, 42
//   @1600408  virtual void api::CPosixRWMutex::ReadLock()   line 58
//   @1600424  virtual void api::CPosixRWMutex::WriteLock()  line 69
//   @1600440  virtual void api::CPosixRWMutex::Unlock()     line 80
// The messages are unreferenced strings of the same pool; which line uses which is inferred:
//   "Falha ao inicializar atributos de mutex: {}" @3679, "Falha ao criar mutex: {}" @3554,
//   "Falha ao destruir atributos de mutex: {}" @3638, "Falha ao bloquear mutex para leitura: {}" @6048,
//   "Falha ao bloquear mutex para escrita: {}" @6007, "Falha ao desbloquear mutex: {}" @3579.
// Codes 6153..6158 are inferred: EUeIpcError 6150..6152 are CPosixMutex's three sites and 6159 is
// CPosixSemaphore's first, which leaves exactly six codes for the six sites of this file.
#include "api/ipc/posix/cposixrwmutex.h"

#include <cstring>
#include <format>
#include <pthread.h>

#include "api/ipc/euipcerror.h"

namespace api {

// wasm func 10245 (all three checks folded away: the stubs return 0)
CPosixRWMutex::CPosixRWMutex()
{
    pthread_rwlockattr_t atributos;
    int erro = pthread_rwlockattr_init(&atributos);                // inlined: zeroes the 8-byte local
    if (erro != 0)
        throw CUeIpcError(EUeIpcError(6153),
                          std::format("Falha ao inicializar atributos de mutex: {}", std::strerror(erro)));  // line 29 (code/text ?)
    erro = pthread_rwlock_init(&m_lock, &atributos);               // stub -> 0
    if (erro != 0)
        throw CUeIpcError(EUeIpcError(6154), std::format("Falha ao criar mutex: {}", std::strerror(erro)));  // line 35 (code/text ?)
    erro = pthread_rwlockattr_destroy(&atributos);                 // no-op
    if (erro != 0)
        throw CUeIpcError(EUeIpcError(6155),
                          std::format("Falha ao destruir atributos de mutex: {}", std::strerror(erro)));     // line 42 (code/text ?)
}

// wasm func 5353 (slot 0) / 10244 (slot 1, deleting)
CPosixRWMutex::~CPosixRWMutex()
{
    pthread_rwlock_destroy(&m_lock);                               // stub                                     // ?
}

// slots 2, 3, 4 = wasm 2219 (ICF: empty body, shared with CPosixSemaphore slots 2/3).
void CPosixRWMutex::ReadLock()
{
    const int erro = pthread_rwlock_rdlock(&m_lock);               // stub -> 0
    if (erro != 0)
        throw CUeIpcError(EUeIpcError(6156),
                          std::format("Falha ao bloquear mutex para leitura: {}", std::strerror(erro)));     // line 58 (code/text ?)
}

void CPosixRWMutex::WriteLock()
{
    const int erro = pthread_rwlock_wrlock(&m_lock);               // stub -> 0
    if (erro != 0)
        throw CUeIpcError(EUeIpcError(6157),
                          std::format("Falha ao bloquear mutex para escrita: {}", std::strerror(erro)));     // line 69 (code/text ?)
}

void CPosixRWMutex::Unlock()
{
    const int erro = pthread_rwlock_unlock(&m_lock);               // stub -> 0
    if (erro != 0)
        throw CUeIpcError(EUeIpcError(6158), std::format("Falha ao desbloquear mutex: {}", std::strerror(erro)));  // line 80 (code/text ?)
}

} // namespace api
