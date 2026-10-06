// ecourna-lib/ecourna/api/security/asn1/ckeyloader.hpp   (path inferred)
//
// Reconstructed from vota_web_wasm.wasm. Unit u01.
//
// CKeyLoader turns a key envelope read from disk (ASN.1 ModuloEnvelopeChave::EntidadeChave, see
// src/asn1/ModuloEnvelopeChave.asn) into raw key bytes, deciphering them when the envelope says
// "cifrado = TRUE". Not polymorphic; sizeof 8.
#pragma once

#include <memory>
#include <vector>

#include "ecourna/api/security/isymmetriccipher.hpp"   // TSharedSymmetricCipher

namespace ecourna::api::security {

class CKeyLoader {
public:
    // Inlined everywhere. The out-of-line body that callers use (func 5172) is an ICF body shared by any
    // "one shared_ptr member, initialised from a by-value shared_ptr" constructor, e.g. also used by
    // CConversorRegistroIdentificacaoEleitor::vf2 (func 9114).                        // name inferred
    explicit CKeyLoader(TSharedSymmetricCipher cifrador = nullptr) : m_cifrador(cifrador) {}

    // Explicitly instantiated for T = ModuloEnvelopeChave::EntidadeChave (wasm func 9473, srcloc line 78).
    // Non-const (the srcloc signature has no trailing const).
    template <typename T>
    std::vector<uebyte> DecipherKeyIfNeeded(const T& entidade);

private:
    TSharedSymmetricCipher m_cifrador;   // +0 (+4 control block)                        // name inferred
};

} // namespace ecourna::api::security
