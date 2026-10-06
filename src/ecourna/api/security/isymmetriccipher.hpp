// ecourna-lib/ecourna/api/security/isymmetriccipher.hpp   (path inferred from the .cpp in the srcloc records;
//                                                          u01's caescipher.hpp/cblockcipher.hpp include it)
//
// Reconstructed from vota_web_wasm.wasm. Unit u13 (the AES implementation is unit u01).
//
// RTTI: ISymmetricCipher : pattern::NonCopyable   (typeinfo @1114024, vtable @1114100)
//   [0] ~ISymmetricCipher (func 9484, ICF-shared with ~CBlockCipher<CAesCipher, CTrng>)  [1] func 325
//   [2] Encrypt, [3] Decrypt = __cxa_pure_virtual
// Only implementation: CBlockCipher<CAesCipher, CTrng> (u01), created by CSymmetricCipherFactory.
#pragma once

#include <memory>
#include <vector>

#include "ecourna/api/pattern/noncopyable.hpp"
#include "ecourna/types.hpp"

namespace ecourna::api::security {

enum class EOperationModes : int { ECB = 0, CBC = 1, CTR = 4 };                        // names inferred (u01)
enum class ESymmetricCipherKeySize : int { AES128 = 128, AES192 = 192, AES256 = 256 };  // names inferred (u01)

struct CAesKey {                                               // sizeof 28 (layout from func 9474)
    std::vector<uebyte>     key;                               // +0
    ESymmetricCipherKeySize keySize{ESymmetricCipherKeySize::AES256};   // +12, in BITS
    std::vector<uebyte>     iv;                                // +16
};

class ISymmetricCipher : public pattern::NonCopyable {
public:
    explicit ISymmetricCipher(const CAesKey& chave);           // wasm func 9474 (srcloc lines 26, 29)
    virtual ~ISymmetricCipher() = default;                     // wasm func 9484: frees iv (+20) then key (+4)

    virtual void Encrypt(const std::vector<uebyte>& in, std::vector<uebyte>& out) const = 0;
    virtual void Decrypt(const std::vector<uebyte>& in, std::vector<uebyte>& out) const = 0;

protected:
    CAesKey m_key;                                             // +4 (key +4, keySize +16, iv +20)
};

using TSharedSymmetricCipher = std::shared_ptr<ISymmetricCipher>;   // name from u01's CKeyLoader error text

} // namespace ecourna::api::security
