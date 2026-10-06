// FRAGMENT of uenux2/src/app/comum/iinterfaceinit.cpp reconstructed by unit u28 from vota_web_wasm.wasm.
// The class (comum::IInterfaceInit, client of the urna's "init" service) is declared in iinterfaceinit.h and its
// other members are reconstructed in iinterfaceinit.cpp (unit u23).
#include "comum/iinterfaceinit.h"

namespace comum {

// wasm func 5898 (tools: comum::IInterfaceInit::vf0) - complete-object destructor, vtable slot 0 of
// comum::IInterfaceInit (table 535, vtable @1552160). Compiler-generated member destruction in reverse order:
//   +12 std::string m_serialMR          (freed when long: SSO flag byte at +23)
//   +4  std::shared_ptr<bool> m_demoMode (control block at +8: --shared_owners, __on_zero_shared, __release_weak)
// The same function is slot 0 of simulador::CWasmInit (vtable @1528772): the web mock adds no member, so clang
// emits ~CWasmInit as an alias of this destructor; CWasmInit's deleting destructor (func 9388) is
// `free(~IInterfaceInit(this))`. The base's own slot 1 is the abstract-class trap (ICF 325).
// Runs only if the CPolySingletonList registry is destroyed (at exit): never seen in the recorded votes.
IInterfaceInit::~IInterfaceInit() = default;

}  // namespace comum
