// uenux2/src/app/comum/relatorios/cgeradorrelpu.cpp   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u35: the two destructors of vtable @1545288). The header
// cgeradorrelpu.h is shared with unit u37 (AdicionaLinha, func 677, in cgeradorrelpu.u37.cpp).
#include "comum/relatorios/cgeradorrelpu.h"

namespace comum {

// wasm func 11901 - vtable slot 0. Not observed executing.
// Body = shared_f6055(this, vtable @1545288): vptr = CGeradorRelPU, then the builder's
// std::vector<std::shared_ptr<field>> at +4 is destroyed (each shared_ptr released from the back, buffer freed).
// shared_f6055 is a merged body also used by RHVoice::dtree::in_list.
CGeradorRelPU::~CGeradorRelPU() = default;

// wasm func 11900 - vtable slot 1 (deleting destructor): unknown_f6054(this, vtable) = the same + operator delete.
// The object normally lives on the stack of vota::CImpressaoPU::StartState (func 11902), so slot 1 is not used there.

} // namespace comum
