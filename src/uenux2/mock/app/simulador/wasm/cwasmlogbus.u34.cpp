// FRAGMENT reconstructed by unit u34 from vota_web_wasm.wasm.
// Original file: uenux2/mock/app/simulador/wasm/cwasmlogbus.cpp (path inferred by unit u29, which reconstructs
// the class in cwasmlogbus.u29.cpp: a mutex + 3 channels {deque<string> historico; unordered_map<int,
// function<void(const string&)>> ouvintes; int proximoId}).
#include "simulador/wasm/cwasmlogbus.h"

namespace simulador {

// wasm func 5154 (tools: api_f5154; table slots 534 and 741 = the atexit registration of the static object
// @1832676). Destructor of the function-local static `s_bus` of CWasmLogBus::GetInst(), members in reverse
// order: for channel 2, 1, 0: the listener map (each node's std::function destroyed through its __base
// vtable: slot 4 "destroy" for the small-buffer case, slot 5 "destroy_deallocate" otherwise; then the node
// and the bucket array freed) and the history deque (std::deque<std::string>::clear, func 3503, + blocks);
// last the mutex (only the unlock residue, func 150, remains in this single-threaded build).
CWasmLogBus::~CWasmLogBus() = default;

}  // namespace simulador
