// ecourna-lib/ecourna/api/security/csymmetriccipherfactory.cpp   (path inferred)
//
// Reconstructed from vota_web_wasm.wasm. Unit u40.
#include "ecourna/api/security/csymmetriccipherfactory.hpp"

#include <memory>
#include <vector>

#include "ecourna/api/security/caescipher.hpp"     // CAesCipher (unit u01)
#include "ecourna/api/security/cblockcipher.hpp"   // CBlockCipher<ALGO, RANDOM> (unit u01)
#include "ecourna/api/security/csha.hpp"           // CSha512 (unit u13)
#include "ecourna/api/security/ctrng.hpp"

namespace ecourna::api::security {

// wasm func 1884 (vtable slot 2). Key derivation from a secret string:
//   key = SHA-512(segredo)[16..48)   (32 bytes -> AES-256), IV empty (CAesCipher then draws a random IV and
//   appends it to the ciphertext, see u01).
// The byte range is read from the wasm: the four i64 copies take offsets 16, 24, 32 and 40 of the digest
// returned by CSha::Finish (the vector at frame+52). Earlier docs (u01, u13, openssl.md) say [0..32); that is
// not what the code does.
// Callers: vota::CGeraBU::StartState (CV key, func 12110), comum::CGravadorBU::GravaResultado (BU public
// key, 11629), comum::CGravadorRCSecao (11616), CControlaArmazenamentoDeImagens (2725),
// CConversorBiometriaEleitorCifrada::DoDeconverte (11419). In all of them `segredo` is the key-encryption
// secret returned by api::IKernelHSM, which does not exist in the web build (see the module doc).
TSharedSymmetricCipher CSymmetricCipherFactory::Create(const std::string& segredo) const
{
    const std::vector<uebyte> dados(segredo.begin(), segredo.end());

    CSha512 sha;                                                   // wasm func 2684
    sha.Update(dados);                                             // wasm func 3519
    const std::vector<uebyte> resumo = sha.Finish();               // wasm func 3518 (64 bytes)

    const std::vector<uebyte> chave(resumo.begin() + 16, resumo.begin() + 48);
    const CAesKey chaveAes{chave, ESymmetricCipherKeySize::AES256, {}};   // keySize stored as 256; iv empty
    return Create(chaveAes);                                       // virtual call through slot 3
}   // ~CAesKey = wasm func 3517 on the unwinding path

// wasm func 9489 (vtable slot 3). Observed executing: the RDV cipher is built here at votaInit
// (comum::CRdv via CPolySingleton<ISymmetricCipherFactory>, key/IV from CHKDFSeed, see crdv.cpp).
// operator new(36), ISymmetricCipher(const CAesKey&) (func 9474, checks the key size), mode CBC (1) stored
// at +32, vptr CBlockCipher<CAesCipher, CTrng> @1113928. The shared_ptr is built from the raw pointer
// (control block __shared_ptr_pointer @1114280, 16 bytes), not with make_shared.
TSharedSymmetricCipher CSymmetricCipherFactory::Create(const CAesKey& chave) const
{
    return TSharedSymmetricCipher(new CBlockCipher<CAesCipher, CTrng>(chave, EOperationModes::CBC));
}
// On a throw from the shared_ptr control-block allocation the landing pad runs the deleting destructor of
// CBlockCipher<CAesCipher, CTrng> inline; the same destructor out of line is wasm func 9488 (vtable slot 1:
// ISymmetricCipher vptr, ~iv (+20), ~key (+4), operator delete).

} // namespace ecourna::api::security
