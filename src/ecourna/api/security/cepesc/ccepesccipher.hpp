// ecourna-lib/ecourna/api/security/cepesc/ccepesccipher.hpp   (path inferred: the CEPESC data classes
//   cplaintext.cpp / ccipheredout.cpp / ccipheredin.cpp / cinfosalt.cpp are srcloc-attested in this directory;
//   the class itself is in namespace ecourna::api::cepesc)
//
// Reconstructed from vota_web_wasm.wasm. Unit u40.
//
// CEPESC = the TSE's hybrid encryption library for result files (named after CEPESC, the government's
// communications-security research centre). The RTTI of this build:
//   IAsymmetricCipher<CPlainText, CCipheredOut, CCipheredIn, std::vector<uebyte>> : pattern::NonCopyable
//       (typeinfo @1115072, "vmi", abstract, template parameters = input, output, ciphered input, plain output)
//   CCepescCipher : IAsymmetricCipher<...>   (typeinfo @1115020, vtable @1115004, sizeof 4)
//       [0] ~ (ICF 174) [1] deleting (144) [2] Encrypt func 2681 [3] Decrypt func 5171
// In THIS BUILD CCepescCipher is a stub that does not encrypt (see ccepesccipher.cpp). Whether the urna
// links another implementation under the same name cannot be told from the wasm.
#pragma once

#include <vector>

#include "ecourna/api/pattern/noncopyable.hpp"
#include "ecourna/api/security/cepesc/ccipheredout.hpp"   // CCipheredOut (unit u01)
#include "ecourna/api/security/cepesc/cplaintext.hpp"     // CPlainText (unit u01)
#include "ecourna/types.hpp"

namespace ecourna::api::cepesc {

class CCipheredIn;   // ccipheredin.hpp (u01/u03): +28 std::vector<uebyte> conteudo

} // namespace ecourna::api::cepesc

namespace ecourna::api::security {

// iasymmetriccipher.hpp (path inferred) - kept here because CCepescCipher is its only implementation.
template <typename IN, typename OUT, typename CIPHERED_IN, typename PLAIN_OUT>
class IAsymmetricCipher : public pattern::NonCopyable {
public:
    virtual ~IAsymmetricCipher() = default;
    virtual OUT Encrypt(const IN& claro) const = 0;                 // slot 2   (other units: "Cifra")
    virtual PLAIN_OUT Decrypt(const CIPHERED_IN& cifrado) const = 0; // slot 3  (other units: "Decifra")
};

} // namespace ecourna::api::security

namespace ecourna::api::cepesc {

class CCepescCipher
    : public security::IAsymmetricCipher<CPlainText, CCipheredOut, CCipheredIn, std::vector<uebyte>> {
public:
    CCipheredOut Encrypt(const CPlainText& claro) const override;                  // wasm func 2681
    std::vector<uebyte> Decrypt(const CCipheredIn& cifrado) const override;        // wasm func 5171 (u03)
};

} // namespace ecourna::api::cepesc
