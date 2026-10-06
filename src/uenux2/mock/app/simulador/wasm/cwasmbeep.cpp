// Reconstructed from vota_web_wasm.wasm (unit u30).
// Original: uenux2/mock/app/simulador/wasm/cwasmbeep.cpp (path inferred, see cwasmbeep.h).
//
// All tone tables below are decoded from the i64 constants the compiler stored into the vectors
// ({frequency, duration} pairs, little-endian). Durations are in hundredths of a second: Beep() multiplies by 10
// before calling JavaScript. Every melody plays its tones through the VIRTUAL Beep (vtable slot 1), not the
// import directly.
//
// Observed executing in the recorded votes: Beep (8538), BeepConfirma (8533: 1x per confirmed cargo, 4x at the
// end - the urna's long "FIM" beep) and BeepErro (8494: key refused while a confirmation screen settles).
#include "simulador/wasm/cwasmbeep.h"

namespace simulador {

// Inlined into CSimuladorWasm::Executa (func 8302): operator new(4), vptr @1530312, then the import.
CWasmBeep::CWasmBeep()
{
    js_wasm_beep_init();
}

// wasm func 8541 (vtable slot 0). Never called in the recorded sessions.
void CWasmBeep::SetVolume(int volume)
{
    js_wasm_beep_set_volume(volume);
}

// wasm func 8538 (vtable slot 1). E.g. Beep(2000, 100) = 2 kHz for 1 s (CExibeAlertaDesligamento::StartState),
// Beep(800, n) in CApplication::EnterLoopBeeping.
void CWasmBeep::Beep(int frequenciaHz, int duracao)
{
    js_wasm_beep_queue(frequenciaHz, duracao * 10);          // 1/100 s -> ms
}

// Helper inlined in every melody (vector iteration + virtual call).                        name inferred
void CWasmBeep::Toca(const std::vector<STom>& tons)
{
    for (const STom& tom : tons)
        Beep(tom.frequencia, tom.duracao);                    // virtual: (this->vptr)[1]
}

// wasm func 8533 (vtable slot 2). The confirmation "pi-pi": 2300 Hz then 2200 Hz, 100 ms each, `vezes` times.
// vezes <= 0 plays nothing.
void CWasmBeep::BeepConfirma(int vezes)
{
    const std::vector<STom> tons = {{2300, 10}, {2200, 10}};  // operator new(16)
    for (int i = 0; i < vezes; ++i)
        Toca(tons);
}

// wasm func 8523 (vtable slot 3). A falling "sad trombone": 392, 370, 349 Hz (0.5 s each; G4 F#4 F4) then a
// 329 Hz tone with a +-5 Hz vibrato in 100 ms steps. Built with push_back (func 760, shared with RHVoice code),
// the vibrato loop fully unrolled by the compiler.
void CWasmBeep::BeepAlerta()
{
    std::vector<STom> tons;
    tons.push_back({392, 50});
    tons.push_back({370, 50});
    tons.push_back({349, 50});
    for (int i = 0; i < 3; ++i) {                                                           // unrolled x3
        tons.push_back({329, 10});
        tons.push_back({334, 10});
        tons.push_back({329, 10});
        tons.push_back({324, 10});
    }
    tons.push_back({329, 10});
    Toca(tons);
}

// wasm func 8514 (vtable slot 4). 17-tone tune (operator new(136)); the 5 Hz / 50 ms entries are rests.
void CWasmBeep::BeepFinalizacao()
{
    const std::vector<STom> tons = {
        {1244, 20}, {830, 20},  {1046, 20}, {1244, 20},                   // D#6 G#5 C6 D#6
        {1661, 10}, {1568, 10}, {1661, 15}, {5, 5},                       // G#6 G6 G#6 (rest)
        {1864, 10}, {1661, 10}, {1864, 15}, {5, 5},                       // A#6 G#6 A#6 (rest)
        {2489, 10}, {2349, 10}, {2489, 10}, {2349, 10}, {2489, 40},       // D#7 D7 D#7 D7 D#7
    };
    Toca(tons);
}

// wasm func 8505 (vtable slot 5). Descending run ending on a long note (operator new(56)).
void CWasmBeep::BeepSuspensao()
{
    const std::vector<STom> tons = {
        {2489, 11}, {2349, 11}, {1864, 11}, {1568, 11}, {1244, 11}, {1174, 11}, {1244, 69},
    };
    Toca(tons);
}

// wasm func 8494 (vtable slot 6) - observed executing. 554 Hz (C#5) for 200 ms: the "invalid key" beep.
void CWasmBeep::BeepErro()
{
    Beep(554, 20);                                            // virtual call through slot 1
}

}  // namespace simulador
