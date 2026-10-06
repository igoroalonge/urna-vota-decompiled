// ecourna-lib/ecourna/api/security/ctrng.hpp   (path inferred)
//
// Reconstructed from vota_web_wasm.wasm. Unit u40.
//
// RTTI: CTrng : IRng   (typeinfo @1114692, vtable @1114668). "True" RNG = std::random_device. Stateless
// (sizeof 4, vptr only): every call builds a new std::random_device from the token "/dev/urandom".
// Only user in the binary: CBlockCipher<CAesCipher, CTrng> (std::make_shared<CTrng>() for each
// Encrypt/Decrypt, merged body 6154). CAesCipher::Encrypt (9493) draws a random IV from it only when the key has
// no IV of its own; the RDV key has one (CHKDFSeed), so the web page never reaches CTrng::Gera.
#pragma once

#include "ecourna/api/security/irng.hpp"

namespace ecourna::api::security {

class CTrng final : public IRng {   // "final": CTrng::Gera(buffer) calls Gera(buffer, n) directly   // ?
public:
    uebyte GeraByte() override;                                                  // wasm func 9477
    int Gera() override;                                                         // wasm func 9476
    int Gera(std::vector<uebyte>& buffer) override;                              // wasm func 9475
    int Gera(std::vector<uebyte>& buffer, std::size_t quantidade) override;      // wasm func 5173
};

} // namespace ecourna::api::security
