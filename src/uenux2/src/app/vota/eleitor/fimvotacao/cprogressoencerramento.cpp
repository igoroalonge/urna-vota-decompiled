// Reconstructed from vota_web_wasm.wasm (unit u39).
// Original (path inferred by u07): uenux2/src/app/vota/eleitor/fimvotacao/cprogressoencerramento.cpp
// Avanca (slot 4, func 12101): cprogressoencerramento.u07.cpp. The constructor (inlined into
// CGravaResultado::StartState, func 12098) is described in cgravaresultado.cpp.
//
// Every method takes m_mutex; in this single-threaded build std::mutex::lock() compiles to nothing and the
// unlock (or the ~mutex) to the no-op ICF stub func 150, which is all that is left of the lock_guard.
//
// WEB BUILD: unreachable (the closing never starts in the page).
#include "vota/eleitor/fimvotacao/cprogressoencerramento.h"

namespace vota {

// wasm func 3907 - vtable slot 0 (complete-object destructor; returns `this` on wasm).
// Members in reverse order: m_mutex (stub 150), m_barra, m_tela. Also called by
// std::shared_ptr<CProgressoEncerramento>'s control block (__on_zero_shared, func 12085).
CProgressoEncerramento::~CProgressoEncerramento() = default;

// wasm func 12104 - vtable slot 1: deleting destructor = func 3907 + free().

// wasm func 12103 - vtable slot 2
void CProgressoEncerramento::Inicia()
{
    std::lock_guard trava(m_mutex);
    m_barra->SetValor(m_barra->GetMinimo());            // func 3667; GetMinimo = inline read of CProgressBar +28 (name inferred)
    m_tela->Show();                                     // IForm slot 2
}

// wasm func 12102 - vtable slot 3
void CProgressoEncerramento::Finaliza()
{
    std::lock_guard trava(m_mutex);
    m_barra->SetValor(m_barra->GetMaximo());            // func 3667; GetMaximo = inline read of CProgressBar +32 (name inferred)
}

}  // namespace vota
