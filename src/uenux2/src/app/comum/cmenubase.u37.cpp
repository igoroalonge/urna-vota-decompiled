// uenux2/src/app/comum/cmenubase.cpp (path inferred) -- FRAGMENT written by unit u37.
// The other two members (AdicionaItem wasm 2285, Monta wasm 5913) are in cmenubase.u02.cpp.
#include "comum/cmenubase.h"

namespace comum {

// wasm func 12235 (vtable slot 0, complete-object destructor; also slot 0 of vota::CMenuMaisInformacoesVota)
// and wasm func 6188 (slot 1, deleting destructor = the same body + operator delete).
// Member-wise, in reverse order: ~m_opcoes (+16), then every shared_ptr of m_itens from the back
// (__shared_weak_count release: use count at +4, __on_zero_shared = slot 2, then __release_weak), then the
// vector storage.
CMenuBase::~CMenuBase() = default;

} // namespace comum
