// FRAGMENTS of several ecourna-lib/ecourna/api/security/ files, reconstructed by unit u40 from
// vota_web_wasm.wasm. The classes are declared by units u01/u13 (caescipher.hpp, cblockcipher.hpp, csha.hpp,
// cepesc/cinfosalt.hpp, isymmetriccipher.hpp); these are functions the compiler emitted out of line for them.
#include <map>
#include <memory>
#include <vector>

#include "ecourna/api/security/cblockcipher.hpp"
#include "ecourna/api/security/cepesc/cinfosalt.hpp"
#include "ecourna/api/security/csha.hpp"
#include "ecourna/api/security/isymmetriccipher.hpp"

namespace ecourna::api::security {

// ---- csha.hpp (or csha512.hpp; file of CSha512 unknown) ----------------------------------------------------
// wasm func 2684 (table slot 6280). Observed executing (CHKDFSeed::GetSeed, inlined into func 7787, at votaInit,
// RDV key).
// CSha512::CSha512() : CSha("SHA2-512", 64) {} - the inline CSha constructor is expanded here: the name is
// stored as an 8-char SSO string, m_tamanho = 64, EVP_MD_CTX_new() (result not checked), the deleter slot
// 6235 (EVP_MD_CTX_free), Reset() (func 5177), then the vptr is switched from CSha (@1113348) to CSha512
// (@1113316). Users: CSymmetricCipherFactory::Create(string) 1884, comum::asn CalculaHash (3602),
// vota::CInformacaoEleitor::Inicializar (7787, inlined CHKDFSeed::GetSeed) and
// comum::CGeradorBUQRCode::GeraQRCodes (5604: the SHA-512 chain of the BU QR codes).
//   (definition: see csha.hpp, `CSha512() : CSha("SHA2-512", 64) {}`)

// ---- isymmetriccipher.hpp ------------------------------------------------------------------------------------
// wasm func 3517: CAesKey::~CAesKey() (implicit): frees iv (+16) then key (+0). Called on the unwinding paths of
// CSymmetricCipherFactory::Create(string) 1884, ISymmetricCipher::ISymmetricCipher 9474 and CAesCipher 9495.

// ---- cblockcipher.hpp ----------------------------------------------------------------------------------------
// wasm func 9488: CBlockCipher<CAesCipher, CTrng>::~CBlockCipher() deleting (vtable slot 1): stores the
// ISymmetricCipher vptr (@1114100), frees m_key.iv (+20) and m_key.key (+4), then operator delete(this).

// ---- caescipher.cpp ------------------------------------------------------------------------------------------
// wasm func 9496: atexit destructor of the file-static
//   std::map<std::pair<EOperationModes, ESymmetricCipherKeySize>, TCipherGetter> s_cipherTable  (@1911772)
// = __tree::destroy(root) (libcxx_f2682). See caescipher.cpp (u01) for the table.
// wasm func 9492: std::vector<uebyte>::insert(const_iterator, first, last) (libc++ __insert_with_size), used by
// CAesCipher::Encrypt to append the IV (out.insert(out.end(), iv.begin(), iv.end())). Library instantiation.

} // namespace ecourna::api::security

namespace ecourna::api::cepesc {

// ---- cepesc/cinfosalt.hpp ------------------------------------------------------------------------------------
// wasm func 9470 (table slot 6323): CInfoSalt::CInfoSalt(const CInfoSalt&) = default - copies m_salt (+0) and
// m_info (+12). Callers: CCipheredOut(chave, conteudo, infoSalt) 9466, CConversorBiometriaEleitorCifrada 11419,
// CGravadorRCSecao 11616.
// wasm func 5170: std::unique_ptr<CInfoSalt>::reset() / ~unique_ptr (frees m_info, m_salt, then the 24-byte
// object). Used by 9466 while the new CInfoSalt is handed to the std::shared_ptr, and by 11419.

} // namespace ecourna::api::cepesc
