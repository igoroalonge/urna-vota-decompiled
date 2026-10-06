// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/audio/crhvoicetexttospeech.h (path inferred; the WAV container it produces is
// api::CWavFile, uenux2/src/api/audio/alsa/cwavfile.cpp, attested by srcloc).
//
// Speech synthesis for the accessible vote (voto com áudio / áudio do eleitor): the urna reads every screen
// to a visually impaired voter through headphones. The engine is RHVoice 1.14 with the Brazilian-Portuguese
// voice "Letícia-F123" (docs/libraries/rhvoice.md). In the web build it is installed by votaInit only when
// the page enables accessibility; otherwise simulador::CWasmNullTextToSpeech is used.
//
//   api::CRHVoiceTextToSpeech : api::ITextToSpeech   typeinfo 1585604, vtable @1585556, 112 bytes
//   api::AudioCollector : RHVoice::client            typeinfo 1586088, vtable @1586036, 24 bytes
//
// ITextToSpeech (vtable @1526528; slots 0..8 implemented in the base, see itexttospeech.u02/u19.cpp):
//   [0] GetAudio(texto) - cache front end   [1] SetPerfil   [2]/[3] set +80/+76   [4] SetVelocidade(nivel)
//   [5] GetVelocidade  [6]/[7] faster/slower   [8] set +88   [9] dtor  [10] deleting dtor
//   [11] Sintetiza(texto, parametros) = 0     <- implemented here (func 10841)
//
// Layout of CRHVoiceTextToSpeech:
//   +0..+95  api::ITextToSpeech (LRU cache +4, permanent cache +40, m_perfilVoz +64, levels/rates +60..+95)
//   +96/+100 std::shared_ptr<RHVoice::engine>          m_engine
//   +104     std::unique_ptr<RHVoice::voice_profile>   m_perfil   (24 bytes: voice vector + name string)
#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "api/audio/itexttospeech.h"
#include "core/client.hpp"          // RHVoice::client
#include "core/engine.hpp"          // RHVoice::engine, RHVoice::voice_profile

namespace api {

// Collects the 16 kHz 16-bit mono samples produced by RHVoice for one request.
// Layout: +0 vptr, +4 int (always 0, purpose unknown) ?, +8 std::vector<short> m_amostras,
//         +20 int m_taxaAmostragem (16000).
// Vtable (RHVoice::client 1.14 order, names from client.hpp):
//   [0] ~AudioCollector (10840)  [1] deleting (10822)  [2] get_audio_buffer_size (10821)
//   [3] play_speech (10820)      [4] get_supported_events (ICF 340, (this)->i32: event_none)
//   [5] process_mark  [6] play_audio  [7] set_sample_rate   (all ICF 371, (this, x)->i32: return true)
//   [8] sentence_starts  [9] sentence_ends  [10] word_starts  [11] word_ends
//                                                            (all func 1908, (this, x, y)->i32: return true)
//   [12] done (ICF 218, (this)->void: no-op)
// The wasm signatures separate the groups {4}, {5,6,7}, {8..11}, {12}; only the order inside {5,6,7} and
// {8..11} comes from client.hpp. Func 1908 is an ICF body: the same `return 1` is also OpenSSL's null
// digest update (legacy EVP_md_null struct @1647304, table slot 9271; provider nullmd_update, dispatch entry
// OSSL_FUNC_DIGEST_UPDATE @1752056, slot 10379).
// Only 2, 3 and the destructors are written by TSE; the others are RHVoice::client's inline defaults.
class AudioCollector : public RHVoice::client {
public:
    explicit AudioCollector(int taxaAmostragem) : m_taxaAmostragem(taxaAmostragem) {}
    ~AudioCollector() override;                                                   // 10840 / 10822

    int get_audio_buffer_size() const override;                                   // 10821
    bool play_speech(const short* amostras, std::size_t quantidade) override;     // 10820

    const std::vector<short>& GetAmostras() const { return m_amostras; }          // inlined; name inferred
    int GetTaxaAmostragem() const { return m_taxaAmostragem; }                    // inlined; name inferred

private:
    int                m_reservado = 0;       // +4  ?
    std::vector<short> m_amostras;            // +8
    int                m_taxaAmostragem;      // +20
};

class CRHVoiceTextToSpeech : public ITextToSpeech {
public:
    // wasm func 10842 (23.8 KB, other unit): paths raiz/"share"/"RHVoice" and raiz/"etc"/"RHVoice",
    // make_shared<RHVoice::engine>, then SelecionaPerfil("Letícia-F123") (Latin-1 literal @348663).
    explicit CRHVoiceTextToSpeech(const std::filesystem::path& raiz);
    ~CRHVoiceTextToSpeech() override;                                             // 5441 / 10838

protected:
    SharedWav Sintetiza(const std::string& texto, const SParametrosFala& parametros) override;   // 10841

private:
    // wasm func 5442 (tools: rhvoice_f5442): converts `nome` (Latin-1) to UTF-8 and, unless m_perfil already
    // has that name, replaces it with m_engine->create_voice_profile(utf8) (func 5337).        name inferred
    void SelecionaPerfil(const std::string& nome);

    std::shared_ptr<RHVoice::engine>         m_engine;   // +96/+100
    std::unique_ptr<RHVoice::voice_profile>  m_perfil;   // +104
};

} // namespace api
