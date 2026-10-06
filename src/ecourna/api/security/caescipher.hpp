// ecourna-lib/ecourna/api/security/caescipher.hpp   (path inferred: the .cpp path is known from srcloc,
//                                                     ecourna headers use the .hpp extension, cf. cblockcipher.hpp)
//
// Reconstructed from vota_web_wasm.wasm (VOTA "10.23.0.1 - DESENVOLVIMENTO", web simulator build).
// Unit u01, see docs/modules/u01-ecourna-lib-ecourna-api-security.md.
//
// CAesCipher is the "ALGO" policy of CBlockCipher<ALGO, RANDOM> (cblockcipher.hpp). It is NOT polymorphic
// (offset 0 holds the EVP_CIPHER pointer, not a vptr). One instance is built for every Encrypt/Decrypt call.
#pragma once

#include <openssl/evp.h>

#include <map>
#include <memory>
#include <utility>
#include <vector>

#include "ecourna/api/security/irng.hpp"             // IRng   (not part of this unit)
#include "ecourna/api/security/isymmetriccipher.hpp" // CAesKey, EOperationModes, ESymmetricCipherKeySize
#include "ecourna/api/exception/cbaseerror.hpp"
#include "ecourna/api/security/esecurityerror.hpp"   // ESecurityError; CSecurityError = exception::CBaseError<ESecurityError, SErrorLimits{1325, 1725}> (alias name inferred)

namespace ecourna::api::security {

// --- Reference: types declared elsewhere (isymmetriccipher.hpp), layouts recovered from this unit ---------
//
// enum class EOperationModes : int {          // values from the (mode, bits) table built by the static
//     ECB = 0,                                // initializer in __wasm_call_ctors (func 14478)
//     CBC = 1,                                // used by CBlockCipher/CSymmetricCipherFactory (func 9489 stores 1)
//     /* 2, 3 not in the table (CFB/OFB?) */
//     CTR = 4,
// };
// enum class ESymmetricCipherKeySize : int { AES128 = 128, AES192 = 192, AES256 = 256 };   // names inferred
//
// struct CAesKey {                                           // sizeof 28
//     std::vector<uebyte>     key;                           // +0   raw key bytes
//     ESymmetricCipherKeySize keySize = AES256;              // +12  default 256 (i64 store 0x100'00000000 @+24)
//     std::vector<uebyte>     iv;                            // +16  empty => random IV appended to the output
// };
// --------------------------------------------------------------------------------------------------------

class CAesCipher {
public:
    CAesCipher(const EOperationModes mode, const ESymmetricCipherKeySize keySize,
               std::shared_ptr<IRng> rng);                                   // wasm func 9495 (srcloc line 54)
    ~CAesCipher() = default;                                                 // wasm func 9486 (implicit)

    void SetKey(const CAesKey& key);                                         // wasm func 9494 (srcloc line 70)
    void Encrypt(const std::vector<uebyte>& in, std::vector<uebyte>& out);   // wasm func 9493 (lines 79..105)
    void Decrypt(const std::vector<uebyte>& in, std::vector<uebyte>& out);   // wasm func 9491 (lines 119..150)

private:
    // Size of the IV that Encrypt appends (random IV) and Decrypt strips from the end; also the size of the
    // local zero-filled IV vector a key IV is copied into.   name inferred (literal 16 in the code)
    static constexpr std::size_t IV_SIZE = 16;

    const EVP_CIPHER*     m_cipher{nullptr};   // +0   EVP_aes_<bits>_<mode>()
    const EOperationModes m_mode;              // +4   stored, never read again after the constructor
    std::shared_ptr<IRng> m_rng;               // +8   (+12 control block)  CTrng in practice
    CAesKey               m_key;               // +16  key +16, keySize +28, iv +32..+44
    int                   m_blockSize{0};      // +44  EVP_CIPHER_get_block_size(m_cipher) (16 for ECB/CBC, 1 for CTR)
};                                             // sizeof 48

} // namespace ecourna::api::security
