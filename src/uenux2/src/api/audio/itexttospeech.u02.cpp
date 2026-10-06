// uenux2/src/api/audio/itexttospeech.cpp (path inferred)  --  FRAGMENT written by unit u02
//
// Non-virtual members of api::ITextToSpeech (vtable @1526528, see docs/libraries/rhvoice.md §2)
// that the tools attributed to chkdfseed.cpp. They implement the synthesis cache used by the
// accessible-voting audio (voto com áudio, for visually impaired voters):
//
//   ITextToSpeech layout (only the members used here):
//     +4   CLruCache<std::string, SharedWav> m_cacheRecente   (bounded, least-recently-used)
//            +4  capacity (size_t)
//            +8  std::list<std::pair<std::string, SharedWav>>   (sentinel +8/+12, size +16)
//            +20 std::unordered_map<std::string, list::iterator> (buckets, count, first, size +32, max_load)
//     +40  std::unordered_map<std::string, SharedWav> m_cachePermanente   (never evicted)
//     +64  std::string m_perfilVoz      +76 int m_p76 (default 50)   +80 int m_p80 (default 100)
//     +84  int m_taxa (60..140 %)       +88 int64 m_p88
//
// SharedWav = std::shared_ptr<api::CWavFile>. The vtable front end ITextToSpeech::vf0 (wasm 11530,
// not in this unit) is  key = MontaChave(); wav = Busca(key); if (!wav) { wav = vf11(...);
// Armazena(key, wav, false); }  and CacheMessage below does the same into the permanent cache.

#include "api/audio/itexttospeech.h"

#include <format>

namespace api {

struct SParametrosFala                                  // 32 bytes, built on the stack
{
    std::string perfil;   // +0  copy of m_perfilVoz
    int taxa;             // +12 m_taxa
    int p80;              // +16
    int p76;              // +20
    std::int64_t p88;     // +24
};

// wasm func 921                                                        // name inferred
// Synthesises `texto` with the current voice parameters and keeps it in the permanent cache.
// Called 14 times per speech rate by vota::CInstrucaoVotacaoAcessibilidade::CacheInstructionAudio
// (inlined in wasm func 7787). NOTE (web build): at that moment the registered ITextToSpeech is
// the simulador::CWasmNullTextToSpeech that votaInit pushes first; votaInit creates the
// CRHVoiceTextToSpeech only after 7787 returns. Its vf11 (wasm 9436) returns an empty shared_ptr,
// so these 70 calls synthesise nothing and fill the permanent cache of the null object only.
void ITextToSpeech::CacheMessage(const std::string& texto)
{
    const SParametrosFala parametros{m_perfilVoz, m_taxa, m_p80, m_p76, m_p88};
    const std::string chave = MontaChave(texto, parametros);                     // wasm 5758
    if (SharedWav wav = Busca(chave)) {                                         // wasm 5754
        m_cachePermanente[chave] = wav;                                         // wasm 5685 (try_emplace)
        return;
    }
    SharedWav wav = Sintetiza(texto, parametros);                               // vtable slot 11
    Armazena(chave, wav, true);                                                 // wasm 5750
}

// wasm func 5758 (table slot 207, called through invoke_viiii)          // name inferred
// Cache key: "{}:{}:{}:{}:{}" (string @1147, 14 bytes). The five arguments are packed in this order
// (string, int, string, int, int64: __create_packed_storage calls on c, d+12, d, d+16 and the
// int64 copy of d+24). p76 (+20) is not part of the key.
std::string ITextToSpeech::MontaChave(const std::string& texto, const SParametrosFala& p) const
{
    return std::vformat("{}:{}:{}:{}:{}", std::make_format_args(texto, p.taxa, p.perfil, p.p80, p.p88));
}

// wasm func 5754 (table slot 208)                                      // name inferred
ITextToSpeech::SharedWav ITextToSpeech::Busca(const std::string& chave)
{
    if (auto it = m_cachePermanente.find(chave); it != m_cachePermanente.end())    // wasm 2808
        return it->second;
    if (!m_cacheRecente.m_indice.contains(chave))                                  // wasm 2808
        return {};
    auto posicao = m_cacheRecente.m_indice[chave];                                 // wasm 3732 (operator[])
    m_cacheRecente.m_lista.splice(m_cacheRecente.m_lista.begin(),
                                  m_cacheRecente.m_lista, posicao);               // wasm 5701
    return posicao->second;
}

// wasm func 5750 (table slot 209)                                      // name inferred
void ITextToSpeech::Armazena(const std::string& chave, SharedWav wav, bool permanente)
{
    if (permanente)
        m_cachePermanente[chave] = wav;              // wasm 11447 + 5707 (copy-assign; `wav` is destroyed after)
    else
        m_cacheRecente.Insere(chave, wav);                                         // wasm 11436
}

// wasm func 11436 (table slot 211)                                     // name inferred
template <class K, class V>
void CLruCache<K, V>::Insere(const K& chave, V valor)
{
    if (m_indice.contains(chave)) {                                                 // wasm 2808
        auto posicao = m_indice[chave];                                             // wasm 3732
        posicao->second = valor;                                                   // wasm 5707 (copy-assign)
        m_lista.splice(m_lista.begin(), m_lista, posicao);                         // wasm 5701
        return;
    }
    if (m_indice.size() == m_capacidade) {                                          // evict the oldest
        const auto antiga = m_lista.back();       // copy of the whole pair (string + shared_ptr, wasm 1928 frees it)
        m_indice.erase(antiga.first);                                               // wasm 11353
        m_lista.pop_back();                                                         // libc++ 11346
    }
    m_lista.emplace_front(chave, std::move(valor));
    m_indice[chave] = m_lista.begin();
}

// ---------------------------------------------------------------------------------------------
// libc++ instantiations used by the two caches (listed for the mapping table; not reconstructed):
//   wasm 3729  std::hash<std::string>{}(key) (__murmur2 via libc++ 4773) wrapped in a noexcept call
//   wasm 2808  unordered_map<string, ...>::find(key)            (returns the node or nullptr)
//   wasm 3732  unordered_map<string, list::iterator>::operator[](key)  (emplace + rehash)
//   wasm 11353 unordered_map<string, list::iterator>::erase(key) -> size_t
//   wasm 2804  __hash_node_destructor for that map (unique_ptr<node>::reset)
//   wasm 11447 unordered_map<string, SharedWav>::operator[](key) -> &node->value (node + 20)
//   wasm 5685  unordered_map<string, SharedWav>::__emplace_unique_key_args (try_emplace)
//   wasm 3722  __hash_node_destructor for that map
//   wasm 5701  std::list::splice(pos, list, it)  (move one node to the front)
//   wasm 5707  std::shared_ptr<CWavFile>::operator=(const shared_ptr&)

} // namespace api
