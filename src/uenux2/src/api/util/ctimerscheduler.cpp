// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/util/ctimerscheduler.cpp (path inferred). See the header for the web-build caveat.
#include "api/util/ctimerscheduler.h"

namespace api {

// ------------------------------------------------------------------------------------------------------------
// CTimerScheduler
// ------------------------------------------------------------------------------------------------------------

// wasm func 10846 (vtable slot 0; not observed)
// The callback is copied (std::function copy: __clone into a local, then moved into the CTimer).
std::shared_ptr<ITimer> CTimerScheduler::CreateTimer(const std::chrono::milliseconds& periodo,
                                                     const std::function<void()>& callback)
{
    return std::make_shared<CTimer>(periodo, callback);
}

// ------------------------------------------------------------------------------------------------------------
// CTimer
// ------------------------------------------------------------------------------------------------------------

// wasm func 5445 (slot 0) / 10852 (slot 1 = 5445 + operator delete)
// Stop() is inlined; then the members are destroyed: ~mutex(+120), ~thread(+116: std::terminate if the thread
// were still joinable), ~mutex(+92), ~condition_variable(+44), ~function(+16).
CTimer::~CTimer()
{
    Stop();
}

// wasm func 10850 (slot 2, other unit) - shown for completeness. In this build the std::thread constructor is
// constant-folded to its failure path: the function is `Stop(); m_ativo = true; new __thread_struct (func
// 2541); new tuple{ts, this}; throw std::system_error(138, "thread constructor failed")`. It has no invoke_*,
// so the two allocations leak and m_ativo stays true after the throw.
void CTimer::Start()
{
    Stop();                                                   // virtual call, slot 3
    m_ativo = true;
    m_thread = std::thread([this] { Executa(); });            // web build: throws "thread constructor failed"
}

// wasm func 10847 (slot 3)
void CTimer::Stop()
{
    m_ativo = false;
    m_cv.notify_all();                                        // noexcept pthread stub (func 150) on +44
    std::lock_guard trava(m_mutexThread);                     // lock() vanished; unlock = func 150 on +120
    if (m_thread.joinable())
        m_thread.join();                                      // libc++ func 4653 ("thread::join failed")
}

// wasm func 10851 (slot 4)
bool CTimer::IsRunning() const
{
    return m_ativo;
}

// wasm func 10848 (slot 5) - takes effect at the next wait of the worker
void CTimer::SetInterval(std::int64_t ms)
{
    m_intervalo = ms;
}

// Worker loop. NOT in the binary (unreachable once std::thread cannot be constructed); reconstructed from the
// members only.                                                                                        ?
void CTimer::Executa()
{
    std::unique_lock trava(m_mutex);
    while (m_ativo) {
        if (m_cv.wait_for(trava, std::chrono::milliseconds{m_intervalo}, [this] { return !m_ativo; }))
            break;
        m_callback();
    }
}

} // namespace api
