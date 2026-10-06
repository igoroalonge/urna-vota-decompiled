// FRAGMENT of ecourna-lib/ecourna/api/util/cstringutils.cpp (srcloc-attested file; class reconstructed by unit
// u13 in cstringutils.hpp/.cpp). Reconstructed by unit u40 from vota_web_wasm.wasm.
#include "ecourna/api/util/cstringutils.hpp"

namespace ecourna::api::util {

// wasm func 9398 (table slot 6356). No srcloc (nothing here throws on purpose). Replaces EVERY occurrence of
// the character `de` by the string `para` (an empty `para` deletes the character); the search restarts after the
// inserted text, so `para` may contain `de`. Callers: sqlite::CSqlStatement (func 9422: 'T' -> " " when binding a
// date-time) and sqlite::CSqlResultSet (func 5162, reading one back: ' ' -> "T", then '-' -> "" and ':' -> "",
// then parse_iso_time).                                                                  // name inferred
void CStringUtils::Replace(std::string& texto, char de, const std::string& para)
{
    // para.size() is loaded ONCE, before the first find, and reused as the step, while para's data and size are
    // re-read for every replace() call: the source keeps the length in a local (the compiler could not hoist
    // it across replace(), since `para` may alias `texto`).
    const std::size_t tamanho = para.size();
    // (memchr + the inlined `pos > size()` test of find are all that remains of the two find calls)
    for (auto pos = texto.find(de); pos != std::string::npos; pos = texto.find(de, pos + tamanho))
        texto.replace(pos, 1, para);
}

} // namespace ecourna::api::util
