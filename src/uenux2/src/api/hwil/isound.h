// Reconstructed from vota_web_wasm.wasm (unit u31).
// Original file UNKNOWN - path inferred: uenux2/src/api/hwil/isound.h
// (api/hwil = hardware interface layer, like the attested iimpressora.h; unit u08 already includes it.)
//
// api::ISound - the loudspeaker/headphone device used by the accessible vote ("voto com áudio": every screen
// and key is spoken, for visually impaired voters). ISound is abstract and has no vtable of its own in the
// binary, but four of its methods are INLINE DEFAULT implementations defined in this header; their bodies
// were emitted with the only class that does not override them, simulador::CWasmNullSound (vtable @1530368),
// which is why the tools named them "CWasmNullSound::vf2/vf4/vf8/vf9". Evidence: the RTTI of their lambdas,
// "api::ISound::Wait()::'lambda'()" (typeinfo @1530492) and
// "api::ISound::WaitAsync(std::function<bool ()>, std::function<void (bool)>)::'lambda'()" (@1530556) -
// the 'lambda' (not $_N) spelling is clang's for lambdas of inline functions.
//
// Slot order (vtables of CWasmNullSound @1530368 and CWasmWebSound @1528476; names from the callers in
// vota::CVotacaoStateAudio, unit u08, and from the web implementation):
//   0 Pause  1 Play(arquivo)  2 Play(arquivo, enfileira)  3 Play(audio)  4 Play(audio, enfileira)  5 Stop
//   6 Mute   7 GetStatus      8 Wait(interromper)          9 WaitAsync(interromper, aoTerminar)   10/11 ~
#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <thread>

#include "api/audio/cesperaaudio.h"   // api::IEsperaAudio, api::CEsperaAudio (unit u32)

namespace api {

class CWavFile;   // api/audio/alsa/cwavfile.cpp: +4 the 44-byte RIFF/WAVE header, +48 samples*, +60 size

// api::IEsperaAudio / api::CEsperaAudio (the handles returned by slot 9: Cancela() slot 2, Cancelada() slot 3)
// are declared in api/audio/cesperaaudio.h (unit u32, path inferred).

class ISound {
public:
    enum EStatus : int { Parado = 0, Tocando = 1, Pausado = 2 };   // values of js_wasm_web_sound_get_status

    virtual void Pause() = 0;                                                            // slot 0
    virtual void Play(const std::string& arquivo) = 0;                                   // slot 1

    // slot 2 - wasm func 8479 (default). `enfileira` = play after the current sound: the default waits.
    virtual void Play(const std::string& arquivo, bool enfileira)
    {
        if (enfileira)
            Wait();
        Play(arquivo);                                                                   // slot 1
    }

    virtual void Play(std::shared_ptr<CWavFile> audio) = 0;                              // slot 3

    // slot 4 - wasm func 8472 (default)
    virtual void Play(std::shared_ptr<CWavFile> audio, bool enfileira)
    {
        if (enfileira)
            Wait();
        Play(audio);                                                                     // slot 3 (copy of the shared_ptr)
    }

    virtual void Stop() = 0;                                                             // slot 5
    virtual void Mute(bool mudo) = 0;                                                    // slot 6
    virtual EStatus GetStatus() = 0;                                                     // slot 7

    // slot 8 - wasm func 8461 (default): polls every 20 ms until the sound ends. Returns false when
    // `interromper` asked to stop waiting. std::this_thread::sleep_for compiles here to
    // "if (byte@1584624 == 1) emscripten_sleep(20)" - the byte is 1 and the build has no Asyncify, so this
    // loop ABORTS the module if it ever runs with a sound playing (unreachable in the page, see the doc).
    virtual bool Wait(std::function<bool()> interromper)
    {
        while (GetStatus() == Tocando) {
            if (interromper && interromper())
                return false;
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
        return true;
    }

    // slot 9 - wasm func 8450 (default): NOT asynchronous - it waits in place, then reports.
    virtual std::shared_ptr<IEsperaAudio> WaitAsync(std::function<bool()> interromper,
                                                    std::function<void(bool)> aoTerminar)
    {
        auto espera = std::make_shared<CEsperaAudio>();
        const bool terminou = Wait([espera, interromper] {                               // lambda: func 8382
            return espera->Cancelada() || (interromper && interromper());
        });
        if (!espera->Cancelada() && aoTerminar)
            aoTerminar(terminou);
        return espera;
    }

    virtual ~ISound() = default;                                                         // slots 10/11

    // Non-virtual helper (inlined into slots 2 and 4 and into vota::CVotacaoStateAudio::PlayMessage).
    bool Wait()
    {
        return Wait([] { return false; });                                               // lambda: func 340 (ICF ret 0)
    }

    // +4  level 0..10, accessed directly by vota::CInstrucaoVotacaoAcessibilidade (keys 3/9, u08; set to 7
    //     at :129). The web device plays at m_volume * 5 + 50 percent. Both mocks start at 9 (ISound default ?).
    int m_volume = 9;
};

}  // namespace api
