// ecourna-lib/ecourna/api/security/isymmetriccipherfactory.hpp   (path inferred; included by crdv.cpp)
//
// Reconstructed from vota_web_wasm.wasm. Unit u40.
//
// RTTI: ISymmetricCipherFactory : pattern::NonCopyable   (typeinfo @1526756, "vmi", abstract).
// Only implementation: CSymmetricCipherFactory. main (func 10307) registers one as the application's
// ISymmetricCipherFactory (CPolySingletonList::push<ISymmetricCipherFactory>); several classes also build
// a local CSymmetricCipherFactory on the stack (CGeraBU, CGravadorBU, CGravadorRCSecao,
// CControlaArmazenamentoDeImagens, CConversorBiometriaEleitorCifrada).
#pragma once

#include <string>

#include "ecourna/api/pattern/noncopyable.hpp"
#include "ecourna/api/security/isymmetriccipher.hpp"   // ISymmetricCipher, CAesKey, TSharedSymmetricCipher

namespace ecourna::api::security {

class ISymmetricCipherFactory : public pattern::NonCopyable {
public:
    virtual ~ISymmetricCipherFactory() = default;

    // Slot 2: cipher keyed by a secret string (the key is derived from it). Other units call it "Cria".
    virtual TSharedSymmetricCipher Create(const std::string& segredo) const = 0;   // name inferred
    // Slot 3: cipher for an explicit key (+ optional IV).
    virtual TSharedSymmetricCipher Create(const CAesKey& chave) const = 0;         // name inferred
};

} // namespace ecourna::api::security
