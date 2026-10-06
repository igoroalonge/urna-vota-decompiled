// ecourna-lib/ecourna/api/security/isymmetriccipher.cpp
// (/home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/api/security/isymmetriccipher.cpp in the srcloc records)
//
// Reconstructed from vota_web_wasm.wasm. Unit u13.
#include "ecourna/api/security/isymmetriccipher.hpp"

#include <format>

#include "ecourna/api/security/esecurityerror.hpp"   // ESecurityError; CSecurityError

namespace ecourna::api::security {

// wasm func 9474 (srcloc lines 26 and 29). Called by CSymmetricCipherFactory::Create(const CAesKey&)
// (func 9489: the RDV cipher); CSymmetricCipherFactory::Create(const std::string&) (func 1884) has the
// same checks inlined. Both checks use code 1445. The size check comes FIRST, so an empty key with
// keySize 0 passes it and is caught by the second check.
ISymmetricCipher::ISymmetricCipher(const CAesKey& chave)
    : m_key(chave)
{
    const auto bits = static_cast<int>(m_key.keySize);
    if (static_cast<std::size_t>(bits) != m_key.key.size() * 8)
        throw CSecurityError(ESecurityError::TamanhoChaveSimetrica /* 1445 */,                       // line 26, name inferred
                             std::format("Incompatibilidade entre o tamanho da chave e a quantidade de bits [{}/{}].",
                                         m_key.key.size(), static_cast<ueword>(bits)));
    if (bits == 0)
        throw CSecurityError(ESecurityError::TamanhoChaveSimetrica /* 1445 */, "Quantidade de bits nula.");   // line 29
}

// wasm func 9484: the defaulted destructor (vtable reset, then ~vector for iv and key).

} // namespace ecourna::api::security
