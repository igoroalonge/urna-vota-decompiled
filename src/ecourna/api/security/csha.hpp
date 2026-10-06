// ecourna-lib/ecourna/api/security/csha.hpp   (path inferred from the .cpp in the srcloc records; also
//                                               declares CSha512, whose file is unknown)
//
// Reconstructed from vota_web_wasm.wasm. Unit u13.
//
// RTTI: CSha : IHash : pattern::NonCopyable   (CSha typeinfo @1113372, vtable @1113348; IHash @1113416)
//       CSha512 : CSha                          (typeinfo @1113472, vtable @1113316, same slots as CSha)
// vtable slots: [0] ~CSha (5175) [1] deleting (5176) [2] Reset (5177) [3] Update (3519) [4] Finish (3518)
//               [5] GetName (3874, ICF body "return a copy of the std::string at this+4", shared with
//                   api::CSubReport and a std::function)
//
// Users: comum::CGeradorBUQRCode::GeraQRCodes (func 5604: SHA-512 hash chain of the BU QR codes),
// vota::CInformacaoEleitor::Inicializar, which inlines CHKDFSeed::GetSeed (func 7787: RDV key derivation at
// votaInit),
// CSymmetricCipherFactory::vf2 (func 1884: AES key = SHA-512(secret)[0..32)), func 3602, comum::CRdv.
#pragma once

#include <openssl/evp.h>

#include <memory>
#include <string>
#include <vector>

#include "ecourna/api/pattern/noncopyable.hpp"
#include "ecourna/types.hpp"

namespace ecourna::api::security {

class IHash : public pattern::NonCopyable {
public:
    virtual ~IHash() = default;
    virtual void Reset() = 0;
    virtual void Update(const std::vector<uebyte>& dados) = 0;
    virtual const std::vector<uebyte> Finish() = 0;
    virtual std::string GetName() const = 0;                     // name inferred (slot 5)
};

class CSha : public IHash {
public:
    // Inlined into CSha512's constructor (func 2684). The EVP_MD_CTX_new() result is not checked.     // ?
    CSha(const std::string& algoritmo, std::size_t tamanho)
        : m_algoritmo(algoritmo), m_tamanho(tamanho), m_contexto(EVP_MD_CTX_new(), &EVP_MD_CTX_free)
    {
        Reset();
    }
    ~CSha() override = default;                                  // wasm func 5175 / 5176

    void Reset() override;                                       // wasm func 5177 (line 23)
    void Update(const std::vector<uebyte>& dados) override;      // wasm func 3519 (lines 30, 35)
    const std::vector<uebyte> Finish() override;                 // wasm func 3518 (line 45)
    std::string GetName() const override { return m_algoritmo; } // wasm func 3874 (ICF)

private:
    std::string m_algoritmo;                                                    // +4  OpenSSL digest name
    std::size_t m_tamanho;                                                      // +16 digest length in bytes
    std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> m_contexto;         // +20 ctx, +24 deleter (slot 6235)
};                                                                              // sizeof 28

// wasm func 2684 (not in u13): stores "SHA2-512" (inline SSO, 8 chars) and 64, EVP_MD_CTX_new, Reset(),
// then switches the vptr to CSha512.
class CSha512 : public CSha {
public:
    CSha512() : CSha("SHA2-512", 64) {}
};

} // namespace ecourna::api::security
