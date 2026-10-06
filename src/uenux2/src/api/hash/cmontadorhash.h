// Reconstructed from vota_web_wasm.wasm (unit u17).
// Original: uenux2/src/api/hash/cmontadorhash.h (path inferred from cmontadorhash.cpp).
//
// "Montador de hash" (hash builder): computes the hash tree of a directory (CHashDiretorio) for the
// result file hash.dat, and chains every file hash into one "hash geral" (overall hash) with the
// scheme also used in the TSE's published hash tables (docs/libraries/openssl.md §10):
//     G1 = H1,  Gk = Base64(SHA-512(G(k-1) || Hk))      with Hk = Base64(SHA-512(file k)) as text.
//
// The hash and text-encoding algorithms are injected through the poly-singleton factories
// api::IGenericFactory<ecourna::api::security::IHash> (SHA-512 = ecourna::api::security::CSha512) and
// api::IGenericFactory<ecourna::api::security::ITextEncoding> (Base64). Neither factory is registered
// in the web build (see the u17 doc): the first file makes CPolySingletonList throw.
#pragma once

#include <cstdint>
#include <set>
#include <string>
#include <vector>

#include "api/hash/chasharquivo.h"

namespace api::hash {

using uebyte = std::uint8_t;

// cmontadorhash.cpp:49 - free function (srcloc signature: "const std::vector<uebyte>
// api::hash::CalculaHashArquivo(const std::string &)"). Inlined into func 5360.
const std::vector<uebyte> CalculaHashArquivo(const std::string& arquivo);

class CMontadorHash {
public:
    // cmontadorhash.cpp:78 - Base64 of CalculaHashArquivo. Inlined into func 5360.
    static std::string CalculaHash(const std::string& arquivo);

    // cmontadorhash.cpp:91/:98 - chains `hash` into `hashAnterior`. Inlined into func 5360.
    static std::string CalculaHashGeral(const std::string& hashAnterior, const std::string& hash);

    // wasm func 5360 (tools: "CalculaHashGeral") - cmontadorhash.cpp:121. Recursive.
    // Called by comum_f5359 (CGravadorHashes::GravaResultado, func 11620).
    static CHashDiretorio CriaHashesDiretorio(const std::string& diretorio,
                                              const std::set<std::string>& ignorados);

    // Overall hash of every file hashed so far (static std::string @1910016; destroyed at exit by
    // func 10259). No accessor survives in this build: nothing reads it.          name inferred
    static inline std::string ms_hashGeral;
};

}  // namespace api::hash
