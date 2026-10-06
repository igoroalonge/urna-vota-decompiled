// uenux2/src/api/audio/itexttospeech.cpp   (path inferred)
//
// Reconstructed from vota_web_wasm.wasm (unit u33): the virtual members 1..9 of api::ITextToSpeech.
// Slot 0 (GetAudio) is in itexttospeech.u19.cpp, the cache helpers (CacheMessage, MontaChave, Busca,
// Armazena) in itexttospeech.u02.cpp. None of the functions below was seen executing in the recorded
// sessions (the recordings ran with accessibility off, i.e. with CWasmNullTextToSpeech).
//
// Speech-rate levels: 0..4 -> 60, 80, 100, 120, 140 % (TaxaDoNivel = nivel * 20 + 60). The constructor
// starts at level 2 (100 %); CInstrucaoVotacaoAcessibilidade::StartStateAudio resets it to 2 and the voter
// changes it with keys 4 (slower, slot 7) and 6 (faster, slot 6) on the audio-instruction screen.
#include "api/audio/itexttospeech.h"

namespace api {

// wasm func 11518 - slot 1                                                          // name inferred
// Voice profile name (RHVoice profile, e.g. "Letícia-F123"). Never called: the profile stays "".
void ITextToSpeech::SetPerfilVoz(const std::string& perfil)
{
    m_perfilVoz = perfil;                          // std::string::operator= (func 1700)
}

// wasm func 11511 - slot 2                                                          // name inferred
// Stored at +80 and passed to RHVoice as pitch = valor / 100. Never called (stays 100).
void ITextToSpeech::SetTom(int percentual)
{
    m_p80 = percentual;
}

// wasm func 11507 - slot 3                                                          // name inferred
// Stored at +76 (default 50). Dead data in this binary: the cache key (func 5758 formats texto, taxa,
// perfil, tom, p88 - in that order) and the RHVoice request (func 10841 reads taxa and tom) ignore it. ?
void ITextToSpeech::SetP76(int valor)
{
    m_p76 = valor;
}

// wasm func 11505 - slot 4                                                          // name inferred
// No range check: any level is accepted and turned into a rate (the only caller passes 2).
void ITextToSpeech::SetVelocidade(int nivel)
{
    m_nivelVelocidade = nivel;
    m_taxa = TaxaDoNivel(nivel);
}

// wasm func 11504 - slot 5                                                          // name inferred
int ITextToSpeech::GetVelocidade() const
{
    return m_nivelVelocidade;
}

// wasm func 11500 - slot 6: one step faster, saturating at level 4 (140 %)          // name inferred
void ITextToSpeech::AumentaVelocidade()
{
    m_nivelVelocidade = (m_nivelVelocidade == 4) ? 4 : m_nivelVelocidade + 1;
    m_taxa = TaxaDoNivel(m_nivelVelocidade);
}

// wasm func 11494 - slot 7: one step slower, saturating at level 0 (60 %)           // name inferred
void ITextToSpeech::DiminuiVelocidade()
{
    m_nivelVelocidade -= (m_nivelVelocidade != 0) ? 1 : 0;
    m_taxa = TaxaDoNivel(m_nivelVelocidade);
}

// wasm func 11491 - slot 8 (i64 argument)                                           // name inferred, ?
void ITextToSpeech::SetP88(std::int64_t valor)
{
    m_p88 = valor;
}

// wasm func 11489 - slot 9 (complete-object destructor). Also slot 9 of simulador::CWasmNullTextToSpeech,
// whose own destructor adds nothing. Members in reverse order: m_perfilVoz (func 149), m_cachePermanente
// (libc++ func 3746, ~unordered_map), m_cacheRecente (libc++ func 3764, ~list + ~unordered_map).
// The deleting destructor of the abstract base (slot 10) is func 325, a bare `unreachable`.
ITextToSpeech::~ITextToSpeech() = default;

} // namespace api
