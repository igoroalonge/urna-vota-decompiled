// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/audio/crhvoicetexttospeech.cpp (path inferred).
//
// Only the TSE-level logic is reconstructed. Func 10841 (75 KB) contains the whole RHVoice request pipeline
// inlined (document, tokenisation, text analysis, G2P, HTS synthesis, output chain); those parts are
// upstream RHVoice 1.14 code and are described in docs/libraries/rhvoice.md §2.3, not rewritten here.
#include "api/audio/crhvoicetexttospeech.h"

#include <memory>
#include <string>

#include "api/audio/alsa/cwavfile.h"   // api::CWavFile, api::CWavAudio (cwavfile.cpp:47)
#include "core/document.hpp"           // RHVoice::document

namespace api {

namespace {

// Inlined twice (10841 and 5442). The application strings are ISO-8859-1 (the TSE sources are Latin-1):
// every byte >= 0x80 becomes the two-byte UTF-8 sequence of the same code point.            name inferred
std::string Latin1ParaUtf8(const std::string& latin1)
{
    std::string utf8;
    for (const unsigned char c : latin1) {
        if (c < 0x80) {
            utf8.push_back(static_cast<char>(c));
        } else {
            utf8.push_back(static_cast<char>(0xC0 | (c >> 6)));      // (c & 0xC0) >> 6 | 0xC0
            utf8.push_back(static_cast<char>(c & 0xBF));
        }
    }
    return utf8;
}

constexpr int TAXA_AMOSTRAGEM = 16000;     // Hz: only the 16000/ model of Letícia-F123 is shipped

} // namespace

// =========================================================================================================
// AudioCollector
// =========================================================================================================

// wasm func 10840 (slot 0) / 10822 (slot 1 = the same + operator delete): frees m_amostras.
AudioCollector::~AudioCollector() = default;

// wasm func 10821 (slot 2)
int AudioCollector::get_audio_buffer_size() const
{
    return 100;
}

// wasm func 10820 (slot 3). The body is libc++'s vector::insert(end(), first, last) inlined (in-place and
// reallocating paths); RHVoice keeps synthesising as long as this returns true.
bool AudioCollector::play_speech(const short* amostras, std::size_t quantidade)
{
    m_amostras.insert(m_amostras.end(), amostras, amostras + quantidade);
    return true;
}

// wasm func 1908 - slots 8..11 (sentence_starts, sentence_ends, word_starts, word_ends): the inline defaults of
// RHVoice::client, `bool f(std::size_t position, std::size_t length) { return true; }`, merged into one body
// (ICF; wasm-opt also folded OpenSSL's null-digest update functions into it, see the header).

// =========================================================================================================
// CRHVoiceTextToSpeech
// =========================================================================================================

// wasm func 5441 (slot 9) / 10838 (slot 10 = 5441 + operator delete)
//   m_perfil.reset() (voice_profile: name string +12, voice vector +0), m_engine.reset(), then
//   ~ITextToSpeech: m_perfilVoz (+64), the permanent cache (+40, libc++ 3746 = ~unordered_map) and the
//   LRU cache (+4, libc++ 3764 = ~list + ~unordered_map).
CRHVoiceTextToSpeech::~CRHVoiceTextToSpeech() = default;

// wasm func 10841 (slot 11; the tools first named it api::CWavFile::CWavFile because of the inlined srcloc
// cwavfile.cpp:47). Called by ITextToSpeech::GetAudio (func 11530) / CacheMessage (func 921) on a cache miss.
//
// parametros = { std::string perfil (+0) = m_perfilVoz, int taxa (+12) = speech rate in % (60..140),
//                int tom (+16) = field +80 (default 100), int (+20) = field +76, int64 (+24) = field +88 }
ITextToSpeech::SharedWav CRHVoiceTextToSpeech::Sintetiza(const std::string& texto, const SParametrosFala& parametros)
{
    // 1. ISO-8859-1 -> UTF-8 (RHVoice works on UTF-8; utf8::next of utfcpp is inlined further down and would
    //    throw utf8::invalid_utf8 / not_enough_room / invalid_code_point on malformed input).
    const std::string textoUtf8 = Latin1ParaUtf8(texto);

    // 2. Voice profile of the request. parametros.perfil is m_perfilVoz (ITextToSpeech slot 1), which nothing
    //    sets in the recorded sessions: the empty name replaces the "Letícia-F123" profile chosen by the
    //    constructor with an empty profile on the first call, and RHVoice falls back to its default voice
    //    (the only one installed). The comparison inside is between the stored (UTF-8) profile name and the
    //    Latin-1 argument.
    SelecionaPerfil(parametros.perfil);                                          // func 5442

    // 3. The document (608-byte RHVoice::document, built inline) with the speech settings of the request.
    //    Its bool property "enable_bilingual", verbosity_params (func 5287) and quality_setting (func 5433)
    //    keep their defaults.
    std::unique_ptr<RHVoice::document> documento =
        RHVoice::document::create_from_plain_text(m_engine, textoUtf8.begin(), textoUtf8.end(),
                                                  RHVoice::content_text, *m_perfil);
    documento->speech_settings.relative.rate   = parametros.taxa / 100.0;        // u32 -> double
    documento->speech_settings.relative.pitch  = parametros.tom / 100.0;
    documento->speech_settings.relative.volume = 1.0;     // volume is applied by the sound device instead

    // 4. Synthesis into memory.
    AudioCollector coletor(TAXA_AMOSTRAGEM);
    documento->set_owner(coletor);
    documento->synthesize();          // sentences -> utterances -> HTS -> limiter/volume -> coletor.play_speech
                                      // ends with `if (owner.get_supported_events() & 64) owner.done()`

    // 5. Canonical 44-byte RIFF/WAVE header (assembled from immediates) + a malloc'ed copy of the samples.
    //    Verified on the WAVs the headless simulator saves (--audio --save-audio):
    //    "RIFF" <data+36> "WAVE" "fmt " 16, PCM 1, mono 1, 16000 Hz, 32000 B/s, align 2, 16 bits, "data" <n>.
    const std::uint32_t bytes = static_cast<std::uint32_t>(coletor.GetAmostras().size() * sizeof(short));
    const CWavAudio cabecalho{
        .riff = {'R', 'I', 'F', 'F'}, .tamanhoRiff = bytes + 36, .wave = {'W', 'A', 'V', 'E'},
        .fmt = {'f', 'm', 't', ' '}, .tamanhoFmt = 16, .formato = 1 /* PCM */, .canais = 1,
        .taxaAmostragem = static_cast<std::uint32_t>(coletor.GetTaxaAmostragem()),
        .bytesPorSegundo = static_cast<std::uint32_t>(coletor.GetTaxaAmostragem()) * 2,
        .alinhamentoBloco = 2, .bitsPorAmostra = 16, .data = {'d', 'a', 't', 'a'}, .tamanhoDados = bytes};

    // CWavFile(const CWavAudio&, const void*) - cwavfile.cpp:47, inlined: copies the header, malloc(bytes),
    // memcpy; on allocation failure throws CBaseError<EUeAudioError>(4851,
    //   "Falha ao alocar memória " + std::to_string(bytes) + " bytes").
    return std::make_shared<CWavFile>(cabecalho, coletor.GetAmostras().data());
}

} // namespace api
