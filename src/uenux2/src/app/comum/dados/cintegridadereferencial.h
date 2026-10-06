// Reconstructed from vota_web_wasm.wasm (unit u04). Original: uenux2/src/app/comum/dados/cintegridadereferencial.h
//
// CIntegridadeReferencial = referential-integrity checks between the election data loaded at
// start-up (cargos x candidaturas x partidos x respostas x eleitores...). The checks themselves
// (lines < 344) are inlined into their callers (comum_f2543 and the start-up function 7787); each
// check returns a TResultado and Lanca() turns a failed one into an exception.
#pragma once

#include <string>
#include <utility>

namespace comum {

class CIntegridadeReferencial {
public:
    using TResultado = std::pair<bool, std::string>;   // {ok, description of the inconsistency}  (+0, +4)

    static void Lanca(const TResultado& resultado);    // line 344 (wasm 2263)
};

}  // namespace comum
