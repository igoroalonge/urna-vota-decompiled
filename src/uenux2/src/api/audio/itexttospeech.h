// uenux2/src/api/audio/itexttospeech.h   (path inferred: the only other audio sources with a known path are
//                                          uenux2/src/api/audio/alsa/cwavfile.cpp; crhvoicetexttospeech.* were
//                                          placed in api/audio/ by unit u32)
//
// Reconstructed from vota_web_wasm.wasm (unit u33, owner of the class). The non-virtual cache members are
// reconstructed in itexttospeech.u02.cpp (unit u02) and slot 0 in itexttospeech.u19.cpp (unit u19); the
// virtual setters/getters (slots 1..9) are in itexttospeech.cpp (this unit).
//
// api::ITextToSpeech: text-to-speech for the accessible vote ("voto com áudio": the urna reads every
// screen to a visually impaired voter through headphones). RTTI: class without base, typeinfo @1526340,
// vtable @1526528 (12 slots). Implementations:
//   api::CRHVoiceTextToSpeech        vtable @1585556, 112 bytes (RHVoice 1.14 + voice Letícia-F123)
//   simulador::CWasmNullTextToSpeech vtable @1528704,  96 bytes (silent; slot 11 = empty stub 9436)
// votaInit (func 7840) registers the RHVoice one only when the page enables accessibility.
//
// Slots 0..9 are written once, here, and shared by both implementations:
//   [0] GetAudio(texto)          11530  cache front end (u19)      [6] AumentaVelocidade()  11500
//   [1] SetPerfilVoz(nome)       11518                              [7] DiminuiVelocidade()  11494
//   [2] SetTom(percentual)       11511                              [8] SetP88(valor)        11491
//   [3] SetP76(valor)            11507                              [9] ~ITextToSpeech()     11489
//   [4] SetVelocidade(nivel)     11505                              [10] deleting dtor: 325 (unreachable:
//   [5] GetVelocidade()          11504                                   clang's D0 of an abstract class)
//   [11] Sintetiza(texto, parametros) = 0   (CRHVoiceTextToSpeech: func 10841; Null: 9436)
// Callers: CInstrucaoVotacaoAcessibilidade uses 4, 5, 6, 7 (keys 4/6 = slower/faster); CVotacaoStateAudio
// uses 0 (its reconstruction, cvotacaostateaudio.cpp, writes that call as "Sintetiza(mensagem)"; in this header
// Sintetiza is the pure slot 11 and slot 0 is GetAudio, as in units u19/u32). Slots 1, 2, 3 and 8 are never
// called, so the RHVoice request always uses the constructor defaults (perfil "", tom 100 %, 50, 0).
#pragma once

#include <cstdint>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

namespace api {

class CWavFile;                                  // uenux2/src/api/audio/alsa/cwavfile.cpp
using SharedWav = std::shared_ptr<CWavFile>;

// Speech parameters of one request (32 bytes, built on the stack by GetAudio / CacheMessage).
struct SParametrosFala {
    std::string   perfil;    // +0   m_perfilVoz
    int           taxa;      // +12  m_taxa: speech rate in % (60, 80, 100, 120, 140) -> RHVoice "rate" / 100
    int           tom;       // +16  m_p80: default 100                               -> RHVoice "pitch" / 100
    int           p76;       // +20  m_p76: default 50; read by nobody (not in the cache key either)     ?
    std::int64_t  p88;       // +24  m_p88: default 0;  part of the cache key, not used by RHVoice       ?
};
// Cache key (func 5758, unit u02): std::format("{}:{}:{}:{}:{}", texto, taxa, perfil, tom, p88) - the format
// string is the 14-byte literal @1147 (string_view {1147, 14} in the i64 60129543291), the argument order is
// read from the packed format arguments; unit u02's MontaChave (itexttospeech.u02.cpp) has the same string and
// order (texto, p.taxa, p.perfil, p.p80, p.p88).

// Bounded least-recently-used cache (36 bytes; members from the constructor inlined in 13877/10842).
template <class K, class V>
class CLruCache {                                                                   // name inferred (u02)
public:
    explicit CLruCache(std::size_t capacidade) : m_capacidade(capacidade) {}
    // ~CLruCache = libc++ func 3764 (~list + ~unordered_map)
private:
    // offsets relative to the cache (the cache itself is at ITextToSpeech+4)
    std::size_t                                                     m_capacidade;   // +0  (256)
    std::list<std::pair<K, V>>                                      m_lista;        // +4  (sentinel +4/+8, size +12)
    std::unordered_map<K, typename std::list<std::pair<K, V>>::iterator> m_indice;  // +16 (max_load 1.0f at +32)
};

class ITextToSpeech {
public:
    // Constructor inlined into both implementations (13877 CWasmNullTextToSpeech, 10842 CRHVoiceTextToSpeech):
    //   LRU capacity 256, both hash maps max_load_factor 1.0, level 2 / rate 100 %, +76 = 50, +80 = 100,
    //   +88 = 0, empty profile name.
    ITextToSpeech() = default;

    // slot 0 - wasm func 11530 (unit u19)
    virtual SharedWav GetAudio(const std::string& texto);

    // slots 1..8 - this unit (itexttospeech.cpp)                                      // names inferred
    virtual void SetPerfilVoz(const std::string& perfil);       // 1
    virtual void SetTom(int percentual);                        // 2
    virtual void SetP76(int valor);                             // 3   ?
    virtual void SetVelocidade(int nivel);                      // 4   0..4
    virtual int  GetVelocidade() const;                         // 5
    virtual void AumentaVelocidade();                           // 6
    virtual void DiminuiVelocidade();                           // 7
    virtual void SetP88(std::int64_t valor);                    // 8   ?

    // slot 9 - wasm func 11489 (also slot 9 of CWasmNullTextToSpeech, which adds no member)
    virtual ~ITextToSpeech();

    // Non-virtual helpers (unit u02): CacheMessage 921, MontaChave 5758, Busca 5754, Armazena 5750.
    void CacheMessage(const std::string& texto);

protected:
    // slot 11 - pure
    virtual SharedWav Sintetiza(const std::string& texto, const SParametrosFala& parametros) = 0;

    static int TaxaDoNivel(int nivel) { return nivel * 20 + 60; }    // inlined in slots 4, 6, 7

    CLruCache<std::string, SharedWav>             m_cacheRecente{256};   // +4
    std::unordered_map<std::string, SharedWav>    m_cachePermanente;     // +40  (max_load_factor at +56)
    int                                           m_nivelVelocidade = 2; // +60
    std::string                                   m_perfilVoz;           // +64
    int                                           m_p76 = 50;            // +76  ?
    int                                           m_p80 = 100;           // +80  tom (pitch, %)
    int                                           m_taxa = 100;          // +84  = TaxaDoNivel(m_nivelVelocidade)
    std::int64_t                                  m_p88 = 0;             // +88  ?
};                                                                       // sizeof = 96

} // namespace api
