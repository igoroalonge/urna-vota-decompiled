// FRAGMENT of uenux2/mock/app/simulador/wasm/cwasminit.cpp (path inferred: simulador::CWasmInit, vtable
// @1528772, the web implementation of comum::IInterfaceInit) reconstructed by unit u29 from vota_web_wasm.wasm.
// The class belongs to unit u31; its command handler EnviarMensagem (slot 5, func 9405) answers the urna's
// "init" service commands with fixed values (e.g. command 39 -> 512000, "= 512000") and logs each one through
// the helper below.

#include <format>
#include <string>

#include "simulador/wasm/cwasmlogbus.h"     // CWasmLogBus (cwasmlogbus.u29.cpp)                    path inferred

namespace simulador {
namespace {

// ------------------------------------------------------------------------------------------------
// wasm func 1524. Callers: CWasmInit::EnviarMensagem (func 9405) only, e.g. Log(39, "= 512000"),
// Log(6, "= no"), Log(7, "").                                                               name inferred
// The accessor of the bus (zero-initialisation of the static on first use) is inlined here.
// The command is formatted as a signed int: the packed argument-type word of the std::format call is 419 =
// __int (3) | __string_view (13) << 5. A std::uint16_t argument would have been __unsigned (6), so the value
// reaches std::format as int (the caller passes the 16-bit command code zero-extended: `c & 0xFFFF`).
// ------------------------------------------------------------------------------------------------
void LogComando(int comando, const std::string& detalhe)
{
    CWasmLogBus::GetInst().Publica(CWasmLogBus::INIT,                                   // func 5152
                                   std::format("CWasmInit::{} {}", comando, detalhe));  // format string @1752
}

}  // namespace
}  // namespace simulador
