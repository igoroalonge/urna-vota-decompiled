// uenux2/src/app/comum/dados/ccandidaturas.cpp  --  FRAGMENT written by unit u13 (path inferred; owner u03)
#include "comum/dados/ccandidaturas.h"

namespace comum {

// wasm func 1939 - name inferred (no srcloc; table slot 101). Observed executing.
// Key of the candidacy / answer maps (CCandidaturas, CRespostas): cargo * 1000000 + number.
// Users: the voting-state JSON of the web build (votaGetStateJson host, wasm_entry_f5500, which looks up every
// apt candidate of the current cargo to emit {"number":...}) and the RDV integrity check (func 11514).
int CCandidaturas::Chave(TCargoID cargo, TCandidatoID numero)
{
    return static_cast<int>(cargo) * 1000000 + static_cast<int>(numero);
}

} // namespace comum
