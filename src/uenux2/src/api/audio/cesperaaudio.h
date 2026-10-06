// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/audio/cesperaaudio.h (path inferred). Unit u31 declares the same two classes in
// its isound.h (api/hwil, path inferred) with the same slot names (Cancela / Cancelada); merge with it.
//
// api::CEsperaAudio : api::IEsperaAudio   (typeinfo 1528600, vtable @1528584, 8 bytes)
// "Audio wait" handle returned by ISound slot 9 (asynchronous wait for the end of the voter's audio,
// accessible vote). The voting state keeps it (vota::CVotacaoStateAudio, CConfereVotoEmCargo) and cancels
// it when the voter presses a key, so that the next message does not wait for the previous one.
// Created with make_shared by simulador::CWasmWebSound::vf9 (func 9515) and CWasmNullSound::vf9 (8450).
//   +4 bool m_cancelada
// Vtable: [0] dtor (ICF 174)  [1] deleting (144)  [2] Cancela (9509)  [3] Cancelada (ICF 2683: return +4)
#pragma once

namespace api {

class IEsperaAudio {
public:
    virtual ~IEsperaAudio() = default;
    virtual void Cancela() = 0;                                       // slot 2   name inferred (u07)
    virtual bool Cancelada() const = 0;                             // slot 3   name inferred
};

class CEsperaAudio : public IEsperaAudio {
public:
    explicit CEsperaAudio(bool cancelada = false) : m_cancelada(cancelada) {}

    // wasm func 9509 (slot 2)
    void Cancela() override { m_cancelada = true; }

    // ICF 2683 (slot 3; the same body is used by vota::impl::CInformacaoThreadOperador)
    bool Cancelada() const override { return m_cancelada; }

private:
    bool m_cancelada;   // +4
};

} // namespace api
