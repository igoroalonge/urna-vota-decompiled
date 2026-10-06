// Reconstructed from vota_web_wasm.wasm (unit u20, owner of this file).
// Original: uenux2/src/api/util/csynchronizer.cpp (srcloc csynchronizer.cpp:37).
//
// api::CSynchronizer decides whether writes to the flash memories are synchronous. It is a 1-byte
// object (bool m_sincrono at +0) kept in a static unique_ptr (@1839324).
//   * CSystem::CopyFile opens destinations with O_SYNC when m_sincrono is true;
//   * Sync() (wasm func 620, tools: shared_f620) is called after every group of writes
//     (SalvaEstado, CopyFile, BU/RDV/report generation ...). In this build its body is a single load of
//     m_sincrono with no effect: the ::sync() it guards compiles to nothing under Emscripten/MEMFS.
#include "api/util/csynchronizer.h"

#include <memory>
#include <unistd.h>

namespace api {

namespace {
std::unique_ptr<CSynchronizer> s_instancia;               // @1839324
}

// Inlined into func 600.                                                  srcloc csynchronizer.cpp:37
void CSynchronizer::CreateInst(bool sincrono)
{
    if (s_instancia)
        throw CUeUtilError(EUeUtilError{7030}, "Instância já criada");
    s_instancia.reset(new CSynchronizer(sincrono));       // ctor = shared_f5729(obj, 1)
}

// wasm func 600 (tools: api::CSynchronizer::CreateInst) - observed executing.             name inferred
// The function returns the instance; CreateInst(true) is inlined on first use.
CSynchronizer& CSynchronizer::GetInst()
{
    if (!s_instancia)
        CreateInst(true);
    return *s_instancia;
}

// wasm func 620 (tools: shared_f620)                                                     name inferred
void CSynchronizer::Sync() const
{
    if (m_sincrono)
        ::sync();                                         // no-op in the web build
}

} // namespace api
