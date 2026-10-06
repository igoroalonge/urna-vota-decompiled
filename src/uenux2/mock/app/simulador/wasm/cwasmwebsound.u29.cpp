// FRAGMENT of uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp (path inferred: simulador::CWasmWebSound, the
// web api::ISound, and its file-local simulador::(anonymous namespace)::CEsperaAudioWasm) reconstructed by unit
// u29 from vota_web_wasm.wasm. The class and CEsperaAudioWasm belong to unit u31; this file holds the two
// extern "C" callbacks that JavaScript calls and the static they share (docs/03-js-wasm-interface.md §14).
//
// Why: the urna application waits for the end of a spoken sentence (api::ISound "espera"). The browser cannot
// block, so CWasmWebSound's wait method (vtable slot 9, func 9515) registers a waiter under a fresh id in the
// map below and calls js_wasm_web_sound_wait_async(id). The glue then polls every 20 ms:
//     if (Module.ccall("uenux_wasm_web_sound_wait_cancel_requested", ..., [id]) || status != playing)
//         Module.ccall("uenux_wasm_web_sound_wait_finished", ..., [id, cancelado ? 0 : 1]);
// CEsperaAudioWasm (the object handed back to the application) removes its entry and calls
// js_wasm_web_sound_cancel_wait(id) when it is cancelled (slot 2, func 2680).

#include <emscripten.h>

#include <functional>
#include <map>

namespace simulador {
namespace {

struct SEsperaWasm {                                   // map node value, 48 bytes                name inferred
    std::function<bool()>     cancelado;               // +0 (node +24): "has the application given up?"
    std::function<void(bool)> fim;                     // +24 (node +48): completion callback (true = played)
};

// @1832664 (begin node), @1832668 (root), @1832672 (size). Destroyed at exit by wasm func 9631
// (__tree<...>::destroy(root), table slot 502). The id counter is a separate static, post-incremented by the
// wait method (id = s_proximoId++: func 9515 keys the new node with the value read before the increment).
std::map<int, SEsperaWasm> s_esperas;                                                         // name inferred

}  // namespace
}  // namespace simulador

extern "C" {

// ------------------------------------------------------------------------------------------------
// wasm func 9614 = export "Lb". Polled by the glue every 20 ms while a wait is active.
// Unknown id, or no cancel predicate: 0.
// ------------------------------------------------------------------------------------------------
EMSCRIPTEN_KEEPALIVE int uenux_wasm_web_sound_wait_cancel_requested(int id)
{
    const auto it = simulador::s_esperas.find(id);
    if (it == simulador::s_esperas.end() || !it->second.cancelado)
        return 0;
    return it->second.cancelado() ? 1 : 0;
}

// ------------------------------------------------------------------------------------------------
// wasm func 9604 = export "Mb". Called once by the glue when the sound ended (concluido = 1) or the wait was
// cancelled (0). The entry is removed BEFORE the callback runs, so the callback may start a new wait.
// Unknown id (already cancelled by CEsperaAudioWasm): nothing.
// ------------------------------------------------------------------------------------------------
EMSCRIPTEN_KEEPALIVE void uenux_wasm_web_sound_wait_finished(int id, int concluido)
{
    const auto it = simulador::s_esperas.find(id);
    if (it == simulador::s_esperas.end())
        return;
    std::function<void(bool)> fim = std::move(it->second.fim);
    simulador::s_esperas.erase(it);
    if (fim)
        fim(concluido != 0);
}

}  // extern "C"
