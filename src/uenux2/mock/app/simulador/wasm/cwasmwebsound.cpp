// Reconstructed from vota_web_wasm.wasm (unit u31).
// Original file UNKNOWN - path inferred: uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp
//
// simulador::CWasmWebSound : api::ISound - the speech output of the accessible vote in the browser
// (typeinfo @1528524, vtable @1528476). Installed by the lambda of main() (func 10384): new CWasmWebSound
// (16 bytes: m_volume = 9, status 0, not muted), push<api::ISound> (replacing CWasmNullSound), then the
// event "vota:ready".
//
// The WAV synthesised by api::ITextToSpeech (RHVoice, docs/libraries/rhvoice.md) is handed to JavaScript
// by address: js_wasm_web_sound_play_wav copies header + samples out of HEAPU8 into a Blob("audio/wav") and
// plays it with an <audio> element from a queue (modo 0 = stop and clear the queue first, 1 = append).
// Because wasm cannot block, waiting for the end of a sound is either a status poll (Wait) or a JavaScript
// setInterval of 20 ms that calls back the exports uenux_wasm_web_sound_wait_cancel_requested (func 9614)
// and uenux_wasm_web_sound_wait_finished (func 9604) of unit u29 (WaitAsync).
// Observed executing: 9523 (Wait), 9541 (Mute; votaInit / votaSetAudioEnabled).
#include <functional>
#include <map>
#include <memory>
#include <string>

#include "api/audio/alsa/cwavfile.h"
#include "api/hwil/isound.h"
#include "simulador/wasm/cwasmjs.h"   // js_wasm_web_sound_*

namespace simulador {

namespace {

// Declared in the u29/u30 fragments of this same file (cwasmwebsound.u29.cpp / cwasmwebsound.u30.cpp):
//   struct SEspera { std::function<bool()> cancelado; std::function<void(bool)> terminado; };   // node +24 / +48
//   std::map<int, SEspera> s_esperas;          // @1832664..@1832672, read by the exports 9604 / 9614 (u29)
//   class CEsperaAudioWasm : api::IEsperaAudio  // typeinfo @1528684, vtable @1528668 (9459/9448/2680/9445, u30):
//                                               // Cancela() erases the entry and calls js_wasm_web_sound_cancel_wait
int s_proximoIdEspera = 1;                     // @1528464 (initialised data; post-incremented: the first id is 1)

}  // namespace

class CWasmWebSound : public api::ISound {
public:
    void Pause() override;                                                               // slot 0 (9589)
    void Play(const std::string& arquivo) override;                                      // slot 1 (9580)
    void Play(const std::string& arquivo, bool enfileira) override;                      // slot 2 (9573)
    void Play(std::shared_ptr<api::CWavFile> audio) override;                            // slot 3 (9565)
    void Play(std::shared_ptr<api::CWavFile> audio, bool enfileira) override;            // slot 4 (9559)
    void Stop() override;                                                                // slot 5 (9550)
    void Mute(bool mudo) override;                                                       // slot 6 (9541)
    EStatus GetStatus() override;                                                        // slot 7 (9533)
    bool Wait(std::function<bool()> interromper) override;                               // slot 8 (9523)
    std::shared_ptr<api::IEsperaAudio> WaitAsync(std::function<bool()> interromper,
                                                 std::function<void(bool)> aoTerminar) override;   // slot 9 (9515)

private:
    // +4 m_volume (ISound)
    int m_status = Parado;    // +8   written by the play/stop/pause calls, never read (GetStatus asks JS)
    bool m_mudo = false;      // +12
};

// wasm func 9589 - slot 0
void CWasmWebSound::Pause()
{
    m_status = Pausado;
    js_wasm_web_sound_pause();
}

// wasm func 9580 - slot 1: sound FILES are not supported (the key click ":/resource/sounds/tecE.wav" of
// CVotacaoStateAudio::PlayKey is never heard in the simulator).
void CWasmWebSound::Play(const std::string& /*arquivo*/)
{
    if (!m_mudo)
        m_status = Parado;
}

// wasm func 9573 - slot 2 (same)
void CWasmWebSound::Play(const std::string& /*arquivo*/, bool /*enfileira*/)
{
    if (!m_mudo)
        m_status = Parado;
}

// wasm func 9565 - slot 3 = slot 4 with enfileira = false (inlined)
void CWasmWebSound::Play(std::shared_ptr<api::CWavFile> audio)
{
    Play(audio, false);
}

// wasm func 9559 - slot 4. Volume sent to JS: 50 % + 5 % per level (level 9 -> 95 %).
void CWasmWebSound::Play(std::shared_ptr<api::CWavFile> audio, bool enfileira)
{
    if (m_mudo) {
        m_status = Parado;
        return;
    }
    if (!audio || audio->GetData() == nullptr || audio->GetDataSize() == 0) {
        m_status = Parado;
        return;
    }
    m_status = Tocando;
    js_wasm_web_sound_play_wav(audio->GetHeader(), 44, audio->GetData(), audio->GetDataSize(),
                               m_volume * 5 + 50, enfileira ? 1 : 0);
}

// wasm func 9550 - slot 5
void CWasmWebSound::Stop()
{
    m_status = Parado;
    js_wasm_web_sound_stop();
}

// wasm func 9541 - slot 6. Observed executing (votaInit / votaSetAudioEnabled).
void CWasmWebSound::Mute(bool mudo)
{
    m_mudo = mudo;
    js_wasm_web_sound_mute(mudo);
}

// wasm func 9533 - slot 7
api::ISound::EStatus CWasmWebSound::GetStatus()
{
    return static_cast<EStatus>(js_wasm_web_sound_get_status());
}

// wasm func 9523 - slot 8. Observed executing. Does NOT wait: it answers "has the sound ended?" once, so
// CVotacaoStateAudio::PlayMessage (CONFIRMA) returns while the message is still playing.
bool CWasmWebSound::Wait(std::function<bool()> interromper)
{
    if (interromper && interromper())
        return false;
    return js_wasm_web_sound_get_status() != Tocando;
}

// wasm func 9515 - slot 9 (vota::CVotacaoStateAudio::IniciarEsperaFimAudio: arm the 2 s repetition tick when
// the spoken instructions end).
std::shared_ptr<api::IEsperaAudio> CWasmWebSound::WaitAsync(std::function<bool()> interromper,
                                                            std::function<void(bool)> aoTerminar)
{
    if (interromper && interromper()) {
        return std::make_shared<api::CEsperaAudio>(true);   // built already cancelled; aoTerminar is NOT called
    }
    if (js_wasm_web_sound_get_status() != Tocando) {
        auto espera = std::make_shared<api::CEsperaAudio>();
        if (aoTerminar)
            aoTerminar(true);                    // synchronously, before the caller gets the handle
        return espera;
    }
    const int id = s_proximoIdEspera++;
    s_esperas[id] = SEspera{std::move(interromper), std::move(aoTerminar)};   // map::operator[] + move-assign
    js_wasm_web_sound_wait_async(id);            // JS polls every 20 ms, then calls export 9604 (finished)
    return std::make_shared<CEsperaAudioWasm>(id);   // {vptr, int id +4, bool cancelada +8}
}

}  // namespace simulador
