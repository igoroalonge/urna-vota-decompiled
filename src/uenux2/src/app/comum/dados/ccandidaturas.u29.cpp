// FRAGMENT of uenux2/src/app/comum/dados/ccandidaturas.cpp reconstructed by unit u29 from vota_web_wasm.wasm.
// The class is reconstructed by unit u03 (ccandidaturas.h / .cpp); u29 only owns the out-of-line accessor.
#include "comum/dados/ccandidaturas.h"

#include <memory>
#include <mutex>

namespace comum {

namespace {
std::unique_ptr<CCandidaturas> s_instancia;   // @1838692
std::mutex s_mutex;                           // @1838668 (lock vanished: single-threaded build; unlock residue)
}  // namespace

// wasm func 521 (table slot 99). 26 callers: the voter states, the BU writer (CGravadorBU::AcrescentaVotoVotavel),
// the BU/zerésima printers (CParteCandidatos*), the key-derivation start-up (7787) and the web state JSON (5500).
CCandidaturas& CCandidaturas::GetInst()
{
    std::lock_guard lock(s_mutex);
    if (!s_instancia)
        s_instancia.reset(new CCandidaturas());   // 44 bytes; constructor func 5811, old instance ~ func 3786
    return *s_instancia;
}

}  // namespace comum
