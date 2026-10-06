// ecourna-lib/ecourna/api/security/cblockcipher.hpp
// (/home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/api/security/cblockcipher.hpp in the srcloc records)
//
// Reconstructed from vota_web_wasm.wasm. Unit u01.
//
// The only instantiation in the binary is CBlockCipher<CAesCipher, CTrng>, typeinfo @1113944, vtable @1113928:
//   [0] ~CBlockCipher()          -> func 9484 (ICF with ~ISymmetricCipher: nothing of its own to destroy)
//   [1] ~CBlockCipher() deleting -> func 9488
//   [2] Encrypt                  -> func 9487 (srcloc line 57)
//   [3] Decrypt                  -> func 9485 (srcloc line 71)
// Objects are created only by CSymmetricCipherFactory (func 9489 = factory vtable slot 3, which
// allocates 36 bytes, runs ISymmetricCipher(const CAesKey&) (func 9474), stores mode 1 = CBC at +32 and
// wraps the pointer in a std::shared_ptr<ISymmetricCipher>).
//
// Encrypt and Decrypt had identical bodies except for constants; wasm-opt's merge-similar-functions folded
// them into ONE body, func 6154, which takes (this, in, out, &ALGO::Encrypt|&ALGO::Decrypt as a table slot,
// srcloc, error code, message). 9487/9485 are 26-byte thunks that pass those constants.
#pragma once

#include <memory>
#include <vector>

#include "ecourna/api/security/isymmetriccipher.hpp"   // ISymmetricCipher, CAesKey, EOperationModes
#include "ecourna/api/exception/cbaseerror.hpp"
#include "ecourna/api/security/esecurityerror.hpp"   // ESecurityError; CSecurityError = exception::CBaseError<ESecurityError, SErrorLimits{1325, 1725}> (alias name inferred)

namespace ecourna::api::security {

// --- Reference: ISymmetricCipher (isymmetriccipher.cpp, not part of this unit) ----------------------------
// class ISymmetricCipher : public pattern::NonCopyable {        // typeinfo @1114024, vtable @1114100
// public:
//     explicit ISymmetricCipher(const CAesKey& key);             // func 9474: copies the key, checks
//                                                                //   keySize != 0 ("Quantidade de bits nula.")
//                                                                //   and keySize == key.size()*8 (code 1445)
//     virtual ~ISymmetricCipher();                               // func 9484
//     virtual void Encrypt(const std::vector<uebyte>& in, std::vector<uebyte>& out) const = 0;
//     virtual void Decrypt(const std::vector<uebyte>& in, std::vector<uebyte>& out) const = 0;
// protected:
//     CAesKey m_key;                                             // +4 (key +4, keySize +16, iv +20..+32)
// };
// using TSharedSymmetricCipher = std::shared_ptr<ISymmetricCipher>;
// ---------------------------------------------------------------------------------------------------------

template <typename ALGO, typename RANDOM>
class CBlockCipher : public ISymmetricCipher {
public:
    // name inferred; inlined into CSymmetricCipherFactory::vf3 (func 9489), which always passes CBC.
    explicit CBlockCipher(const CAesKey& key, const EOperationModes mode = EOperationModes::CBC)   // ?
        : ISymmetricCipher(key), m_mode(mode)
    {
    }

    ~CBlockCipher() override = default;   // wasm func 9484 (complete, ICF) / 9488 (deleting)

    // wasm func 9487 -> body func 6154 (srcloc line 57)
    void Encrypt(const std::vector<uebyte>& in, std::vector<uebyte>& out) const override
    {
        // A fresh algorithm object (and a fresh CTrng) is built for every call, BEFORE the argument check.
        ALGO algoritmo(m_mode, m_key.keySize, std::make_shared<RANDOM>());   // name inferred
        if (in.empty()) {
            throw CSecurityError(ESecurityError::SemDadosParaCifrar /* 1344 */,   // name inferred
                                 "Sem dados para cifrar.");
        }
        algoritmo.SetKey(m_key);
        algoritmo.Encrypt(in, out);
    }   // ~ALGO (func 9486 on the exception paths)

    // wasm func 9485 -> body func 6154 (srcloc line 71)
    void Decrypt(const std::vector<uebyte>& in, std::vector<uebyte>& out) const override
    {
        ALGO algoritmo(m_mode, m_key.keySize, std::make_shared<RANDOM>());
        if (in.empty()) {
            throw CSecurityError(ESecurityError::SemDadosParaDecifrar /* 1345 */, // name inferred
                                 "Sem dados para decifrar.");
        }
        algoritmo.SetKey(m_key);
        algoritmo.Decrypt(in, out);
    }

private:
    EOperationModes m_mode;   // +32   (object size 36)
};

// wasm func 6154: the merged body of the two methods above, as emitted by wasm-opt.
// Signature (this, in, out, memfn_slot, srcloc, code, message):
//   Encrypt: memfn_slot 6284 = &CAesCipher::Encrypt (func 9493), srcloc cblockcipher.hpp:57, 1344, "Sem dados para cifrar."
//   Decrypt: memfn_slot 6285 = &CAesCipher::Decrypt (func 9491), srcloc cblockcipher.hpp:71, 1345, "Sem dados para decifrar."
// Order inside: operator new(16) for std::make_shared<CTrng> (vtable __shared_ptr_emplace<CTrng> @1114140,
// CTrng vtable @1114668), CAesCipher ctor (func 9495) with (mode = this+32, keySize = this+16), empty check,
// SetKey(this+4) (func 9494), call_indirect memfn, inlined ~CAesCipher.

} // namespace ecourna::api::security
