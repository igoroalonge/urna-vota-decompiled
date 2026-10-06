// uenux2/src/app/comum/validamidia/cfabricaconteudomidiastart.h   (path inferred from
// cfabricaconteudomidiastart.cpp, attested by the std::source_location record :62)
// Reconstructed from vota_web_wasm.wasm (unit u26).
#pragma once

#include <string>
#include <vector>

#include "comum/comumtypes.h"                  // EUrnaTurno (header name ?)
#include "comum/validamidia/cvalidamidia.h"     // EAplicativosDeUrna

namespace comum {

class CFabricaConteudoMidiaStart {
public:
    /// cfabricaconteudomidiastart.cpp:62 - inlined into wasm func 11172. Names (or "#regex#" patterns, or
    /// literals with '#'/'@'/'?' wildcards) that an initialised result stick must contain.
    static std::vector<std::string> ConteudoIncializacao(const EAplicativosDeUrna aplicativo, const EUrnaTurno turno);
};

} // namespace comum
