// ecourna-lib/ecourna/api/security/chkdfseed.hpp  (path inferred: the library uses .hpp headers,
// e.g. cblockcipher.hpp, iconversorasn.hpp)
//
// Reconstructed from vota_web_wasm.wasm (unit u02). The class has RTTI
// (typeinfo 1560104, kind "class": no base class) and a 2-slot vtable @1560096
// ([0] ~CHKDFSeed() D1 = wasm func 11498, [1] deleting dtor D0 = wasm func 11497).
//
// Its only user in the simulator is comum::(anonymous)::GetCifradorCryptoTable (crdv.cpp), which
// derives the AES-256 key/IV of the RDV cipher. The constructor and GetSeed() were inlined there
// (inside wasm func 7787), so they have no function index of their own.

#pragma once

#include <vector>

#include "ecourna/api/types.hpp"            // uebyte  (?)

namespace ecourna::api::security {

/// HKDF (RFC 5869) with SHA-512, implemented with OpenSSL's EVP_PKEY HKDF interface.
/// Object layout (wasm32, 40 bytes):
///   +0   vptr (vtable @1560096)
///   +4   std::vector<uebyte> m_salt
///   +16  std::vector<uebyte> m_chave      (input keying material, "IKM")
///   +28  std::vector<uebyte> m_info
class CHKDFSeed
{
public:
    /// Number of bytes produced by GetSeed() (the output vector is created with 128 zero bytes).
    static constexpr std::size_t TAMANHO_SEMENTE = 128;   // name inferred

    CHKDFSeed(const std::vector<uebyte>& salt,
              const std::vector<uebyte>& chave,
              const std::vector<uebyte>& info);           // inlined into wasm func 7787

    virtual ~CHKDFSeed();                                 // wasm func 11498 (D1) / 11497 (D0)

    /// Derives TAMANHO_SEMENTE bytes: HKDF-SHA512(salt = m_salt, IKM = m_chave, info = m_info).
    /// Throws CBaseError<ESecurityError> "Falha ao gerar as sementes criptográficas HKDF."
    std::vector<uebyte> GetSeed();                        // srcloc chkdfseed.cpp:47..67, inlined into 7787

private:
    std::vector<uebyte> m_salt;    // +4
    std::vector<uebyte> m_chave;   // +16
    std::vector<uebyte> m_info;    // +28
};

} // namespace ecourna::api::security
