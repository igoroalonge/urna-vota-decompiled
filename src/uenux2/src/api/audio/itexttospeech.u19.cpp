// FRAGMENT reconstructed by unit u19 from vota_web_wasm.wasm.
// Original file: uenux2/src/api/audio/itexttospeech.cpp (path inferred). The helpers it calls
// (MontaChave 5758, Busca 5754, Armazena 5750) are in itexttospeech.u02.cpp (unit u02).
#include "api/audio/itexttospeech.h"

namespace api {

// wasm func 11530 = vtable slot 0 of api::ITextToSpeech, shared by CRHVoiceTextToSpeech and
// simulador::CWasmNullTextToSpeech (see docs/libraries/rhvoice.md §2).                      // name inferred
// Speech synthesis with the recent-text LRU cache: key = text + voice parameters.
SharedWav ITextToSpeech::GetAudio(const std::string& texto)
{
    const SParametrosFala parametros{m_perfilVoz, m_taxa, m_p80, m_p76, m_p88};   // +64, +84, +80, +76, +88
    const std::string chave = MontaChave(texto, parametros);                      // func 5758 (slot 207)
    SharedWav wav = Busca(chave);                                                 // func 5754 (slot 208)
    if (!wav) {
        wav = Sintetiza(texto, parametros);                                       // vtable slot 11 (pure)
        Armazena(chave, wav, false);                                              // func 5750 (slot 209)
    }
    return wav;
}

}  // namespace api
