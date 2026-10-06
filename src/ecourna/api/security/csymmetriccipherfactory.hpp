// ecourna-lib/ecourna/api/security/csymmetriccipherfactory.hpp   (path inferred)
//
// Reconstructed from vota_web_wasm.wasm. Unit u40.
//
// RTTI: CSymmetricCipherFactory : ISymmetricCipherFactory   (typeinfo @1113856, vtable @1113840)
//   [0] ~ (ICF 174)  [1] deleting (144)  [2] Create(const std::string&) func 1884  [3] Create(const CAesKey&) 9489
// Stateless (sizeof 4).
#pragma once

#include "ecourna/api/security/isymmetriccipherfactory.hpp"

namespace ecourna::api::security {

class CSymmetricCipherFactory : public ISymmetricCipherFactory {
public:
    TSharedSymmetricCipher Create(const std::string& segredo) const override;   // wasm func 1884
    TSharedSymmetricCipher Create(const CAesKey& chave) const override;         // wasm func 9489 (observed)
};

} // namespace ecourna::api::security
