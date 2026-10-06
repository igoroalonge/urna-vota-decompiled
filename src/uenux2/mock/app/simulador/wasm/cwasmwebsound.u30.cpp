// FRAGMENT of uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp (path inferred: the anonymous-namespace class
// below is created only by simulador::CWasmWebSound::vf9, func 9515, so it lives in the same translation unit).
// Reconstructed by unit u30 from vota_web_wasm.wasm; CWasmWebSound itself is unit u31's.
//
// simulador::(anonymous namespace)::CEsperaAudioWasm : api::IEsperaAudio
//   typeinfo @1528684, vtable @1528668; object = 12 bytes inside a std::make_shared control block of 24 bytes
//   (vtable of the block @1528628: __shared_ptr_emplace<CEsperaAudioWasm>).
//
// What it is: the handle returned by ISound's asynchronous "wait for the end of the audio" (slot 9). The urna
// can block its audio thread; the browser cannot. So CWasmWebSound::vf9 stores the caller's two callbacks in a
// static std::map<int, SEspera> keyed by a fresh id and asks JavaScript to poll (js_wasm_web_sound_wait_async):
// every 20 ms JS calls the exports uenux_wasm_web_sound_wait_cancel_requested(id) (func 9614) and, when the
// sound has ended or a cancel was requested, uenux_wasm_web_sound_wait_finished(id, ok) (func 9604), which
// erases the entry and runs `terminado(ok)`. This handle lets the owner cancel a pending wait.
//
// api::IEsperaAudio slots (from api::CEsperaAudio, the "already finished" handle, and this class):
//   [0] ~IEsperaAudio()   [1] deleting dtor   [2] void Cancela()   [3] bool Cancelada() const      names inferred
// vota::CVotacaoStateAudio keeps the handle (+20/+24 shared_ptr) and calls Cancela() before starting the next
// wait (func 7010).
#include <functional>
#include <map>
#include <memory>

#include "api/audio/iesperaaudio.h"      // path inferred

extern "C" void js_wasm_web_sound_cancel_wait(int id);    // clearInterval(Module.uenuxWebSoundWaits[id])

namespace simulador {
namespace {

// Pending waits (std::map header @1832664: begin @1832664, root @1832668, size @1832672). Node layout: +16 key,
// +24 `cancelado` (std::function, __f_ at +40), +48 `terminado` (std::function, __f_ at +64).
struct SEspera {                                                       // name inferred
    std::function<bool()>     cancelado;   // polled by JS through export func 9614
    std::function<void(bool)> terminado;   // called once by export func 9604 (true = played to the end)
};
std::map<int, SEspera> s_esperas;
int s_proximoId = 0;                       // @(data)+536, incremented by CWasmWebSound::vf9

class CEsperaAudioWasm final : public api::IEsperaAudio {
public:
    explicit CEsperaAudioWasm(int id) : m_id(id) {}          // inlined in func 9515

    // wasm func 9459 (slot 0): destroying the handle cancels the wait.
    ~CEsperaAudioWasm() override { Cancela(); }
    // wasm func 9448 (slot 1): deleting destructor = Cancela(); operator delete(this).

    // wasm func 2680 (slot 2). Idempotent. Drops the stored callbacks WITHOUT calling `terminado`, and stops the
    // JavaScript poll. The id is sent to JS even when the entry had already been removed by wait_finished.
    void Cancela() override
    {
        if (m_cancelada)
            return;
        m_cancelada = true;
        s_esperas.erase(m_id);                                  // __tree_remove (1527) + ~function x2 + free
        js_wasm_web_sound_cancel_wait(m_id);
    }

    // wasm func 9445 (slot 3).
    bool Cancelada() const override { return m_cancelada; }

private:
    int  m_id;                   // +4
    bool m_cancelada = false;    // +8
};

}  // namespace
}  // namespace simulador
