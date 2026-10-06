// Reconstructed from vota_web_wasm.wasm (unit u31).
// Original file UNKNOWN - path inferred: uenux2/mock/app/simulador/wasm/cwasmnullsound.cpp
//
// simulador::CWasmNullSound : api::ISound - a silent sound device (typeinfo @1530416, vtable @1530368).
// func 8302 registers it as api::ISound (8-byte object: vptr + m_volume = 9); the lambda of main()
// (func 10384) REPLACES it with simulador::CWasmWebSound before the page can start a vote.
//
// Its vtable is also where the inline defaults of api::ISound were emitted (see api/hwil/isound.h):
//   [0] Pause         nop (ICF 218)          [1] Play(arquivo)        nop (ICF 425)
//   [2] Play(arquivo, enfileira)  ISound default (8479)
//   [3] Play(audio)   8487 (this file)       [4] Play(audio, enfileira)  ISound default (8472)
//   [5] Stop          nop (ICF 218)          [6] Mute(bool)           nop (ICF 425)
//   [7] GetStatus     -> Parado (ICF 340)    [8] Wait(...)            ISound default (8461)
//   [9] WaitAsync     ISound default (8450)  [10]/[11] ~ (ICF 174 / 144)
// Because GetStatus() is always Parado, the polling loop of ISound::Wait never runs here.
#include "api/hwil/isound.h"

namespace simulador {

class CWasmNullSound : public api::ISound {
public:
    void Pause() override {}
    void Play(const std::string&) override {}
    void Play(std::shared_ptr<api::CWavFile> audio) override;   // 8487
    void Stop() override {}
    void Mute(bool) override {}
    EStatus GetStatus() override { return Parado; }
};

// wasm func 8487 - slot 3: nothing to play; only the by-value shared_ptr is released (callee-destroyed,
// libc++ ABI v2 trivial_abi shared_ptr).
void CWasmNullSound::Play(std::shared_ptr<api::CWavFile> /*audio*/)
{
}

}  // namespace simulador
