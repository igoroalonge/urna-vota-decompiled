// Reconstructed from vota_web_wasm.wasm (unit u31).
// Original file UNKNOWN - path inferred: uenux2/mock/app/simulador/wasm/cwasmnulltexttospeech.cpp
//
// simulador::CWasmNullTextToSpeech : api::ITextToSpeech - speech synthesis that produces nothing
// (typeinfo @1528752, vtable @1528704, 96-byte object). Slots 0..8 are the api::ITextToSpeech front end
// (cache, speed levels; docs/libraries/rhvoice.md s2), slot 9 its destructor (11489).
// Registered twice: by func 8302 (start-up, through the push helper 4162) and by votaInit (func 7840, via
// invoke slot 6 -> this constructor) when the voter audio ("audioEleitorHabilitado") is off; with audio on,
// votaInit registers api::CRHVoiceTextToSpeech instead.
#include <memory>
#include <string>

#include "api/audio/itexttospeech.h"

namespace simulador {

class CWasmNullTextToSpeech : public api::ITextToSpeech {
public:
    CWasmNullTextToSpeech();                                                           // 13877
    ~CWasmNullTextToSpeech() override = default;                                       // slot 10 = deleting (9427)

protected:
    api::SharedWav Sintetiza(const std::string& texto, const api::SParametrosFala& parametros) override;   // slot 11 (9436)
};

// wasm func 13877 (called by votaInit; inlined into 8302). Only the base constructor runs:
// recent-cache capacity 256 (+4), both unordered_maps with max_load_factor 1.0, speed level 2 (+60),
// +76 = 50, +80 = 100, rate 100 % (+84).
CWasmNullTextToSpeech::CWasmNullTextToSpeech() = default;

// wasm func 9436 - slot 11: an empty WAV pointer. ISound::Play(nullptr) is then a no-op in CWasmWebSound.
api::SharedWav CWasmNullTextToSpeech::Sintetiza(const std::string& /*texto*/,
                                                 const api::SParametrosFala& /*parametros*/)
{
    return {};
}

// wasm func 9427 - slot 10: ~ITextToSpeech inlined (the +64 profile string, the permanent cache +40 via
// libcxx_f3746, the LRU cache +4 via libcxx_f3764), then operator delete. Observed executing: the start-up
// instance is released (shared_ptr<ITextToSpeech*>::__on_zero_shared, func 11557) when votaInit registers the
// next text-to-speech.

}  // namespace simulador
