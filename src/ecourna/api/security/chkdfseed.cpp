// ecourna-lib/ecourna/api/security/chkdfseed.cpp
// (original build path: /home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/api/security/chkdfseed.cpp)
//
// Reconstructed from vota_web_wasm.wasm, unit u02.
//
// Evidence:
//  * std::source_location records 1112992..1113088 name
//    "std::vector<uebyte> ecourna::api::security::CHKDFSeed::GetSeed()" at lines 47, 50, 53, 56, 59,
//    62 and 67 of this file. Each record is passed, with a distinct ESecurityError code (1397..1403),
//    to the CBaseError<ESecurityError, SErrorLimits{1325, 1725}> constructor (wasm func 9505).
//  * The OpenSSL calls are reached through invoke_* trampolines (table slots 6218..6226):
//    EVP_PKEY_CTX_new_id(1036 = NID_hkdf), EVP_PKEY_derive_init, EVP_PKEY_CTX_set_hkdf_md(EVP_sha512()),
//    EVP_PKEY_CTX_set1_hkdf_salt, EVP_PKEY_CTX_set1_hkdf_key (slot 6223; the pseudo-code annotation
//    "622 ... 3" in the .dcmp is a mis-parse of that slot), EVP_PKEY_CTX_add1_hkdf_info,
//    EVP_PKEY_derive, EVP_PKEY_CTX_free. (Slot 6219 is wasm 7373, whose ERR_raise strings name
//    EVP_PKEY_derive_init_ex: in OpenSSL 3 EVP_PKEY_derive_init(ctx) is derive_init_ex(ctx, NULL),
//    and the NULL was constant-propagated, which is why 7373 takes one argument.)
//  * EVP_sha512() was inlined to the constant 1646408 (&sha512_md); only the empty __THREW__
//    bracket of its call is left between derive_init and set_hkdf_md.
//
// The whole body of GetSeed() and of the constructor is inlined into wasm func 7787
// (vota::CInformacaoEleitor::Inicializar; the tools formerly showed 7787 under the name GetSeed). The "dcmp"
// line numbers below (15827-16260) are from an earlier layout, in which 7787 was in
// decompiled/app-api/ecourna-lib/ecourna/api/security/chkdfseed.cpp.dcmp (that file no longer exists). 7787 is
// now in decompiled/app-vota/uenux2/src/app/vota/eleitor/cestadosvota.cpp.dcmp: search it for
// "CBaseError<ecourna::api::security::ESecurityError" (the seven error sites, one per srcloc line).

#include "ecourna/api/security/chkdfseed.hpp"

#include <openssl/evp.h>
#include <openssl/kdf.h>

#include "ecourna/api/exception/cbaseerror.hpp"
#include "ecourna/api/security/esecurityerror.hpp"   // (?)

namespace ecourna::api::security {

namespace {
// ESecurityError codes used by this file (numeric values read from the binary, names inferred).
constexpr auto HKDF_CRIA_CONTEXTO   = ESecurityError{1397};   // line 47
constexpr auto HKDF_INICIA_DERIVACAO = ESecurityError{1398};  // line 50
constexpr auto HKDF_DEFINE_HASH     = ESecurityError{1399};   // line 53
constexpr auto HKDF_DEFINE_SALT     = ESecurityError{1400};   // line 56
constexpr auto HKDF_DEFINE_CHAVE    = ESecurityError{1401};   // line 59
constexpr auto HKDF_DEFINE_INFO     = ESecurityError{1402};   // line 62
constexpr auto HKDF_DERIVA          = ESecurityError{1403};   // line 67

// "Falha ao gerar as sementes criptográficas HKDF." (string @1112944, in the library's own
// read-only data next to the source_location records)
constexpr const char* MSG_FALHA_HKDF = "Falha ao gerar as sementes criptográficas HKDF.";

using CSecurityError = exception::CBaseError<ESecurityError, exception::SErrorLimits{1325, 1725}>;
} // namespace

// inlined into wasm func 7787 (dcmp 15827-15946). Copies the three vectors (each copy is a plain
// std::vector<uebyte> copy-construction; a length_error is raised through slot 6213 when size < 0).
CHKDFSeed::CHKDFSeed(const std::vector<uebyte>& salt,
                     const std::vector<uebyte>& chave,
                     const std::vector<uebyte>& info)
    : m_salt(salt)
    , m_chave(chave)
    , m_info(info)
{
}

// wasm func 11498 (vtable slot 0, complete-object destructor) and
// wasm func 11497 (vtable slot 1, deleting destructor: same body + free(this)).
// Members are released in reverse order: m_info (+28), m_chave (+16), m_salt (+4).
CHKDFSeed::~CHKDFSeed() = default;

// inlined into wasm func 7787 (dcmp 15947-16260)            (srcloc lines 47..67)
std::vector<uebyte> CHKDFSeed::GetSeed()
{
    std::vector<uebyte> semente(TAMANHO_SEMENTE);            // operator new(128) + memset 0
    std::size_t tamanho = semente.size();

    EVP_PKEY_CTX* contexto = EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF, nullptr);
    if (contexto == nullptr)
        throw CSecurityError(HKDF_CRIA_CONTEXTO, MSG_FALHA_HKDF);                         // line 47

    if (EVP_PKEY_derive_init(contexto) <= 0)
        throw CSecurityError(HKDF_INICIA_DERIVACAO, MSG_FALHA_HKDF);                      // line 50

    if (EVP_PKEY_CTX_set_hkdf_md(contexto, EVP_sha512()) <= 0)
        throw CSecurityError(HKDF_DEFINE_HASH, MSG_FALHA_HKDF);                           // line 53

    if (EVP_PKEY_CTX_set1_hkdf_salt(contexto, m_salt.data(), static_cast<int>(m_salt.size())) <= 0)
        throw CSecurityError(HKDF_DEFINE_SALT, MSG_FALHA_HKDF);                           // line 56

    if (EVP_PKEY_CTX_set1_hkdf_key(contexto, m_chave.data(), static_cast<int>(m_chave.size())) <= 0)
        throw CSecurityError(HKDF_DEFINE_CHAVE, MSG_FALHA_HKDF);                          // line 59

    if (EVP_PKEY_CTX_add1_hkdf_info(contexto, m_info.data(), static_cast<int>(m_info.size())) <= 0)
        throw CSecurityError(HKDF_DEFINE_INFO, MSG_FALHA_HKDF);                           // line 62

    // lines 63-66: nothing survives in the binary (a comment or the declaration of `tamanho`).
    if (EVP_PKEY_derive(contexto, semente.data(), &tamanho) <= 0)
        throw CSecurityError(HKDF_DERIVA, MSG_FALHA_HKDF);                                // line 67

    // NOTE (binary): EVP_PKEY_CTX_free is called only on the success path. Every throw above
    // leaves `contexto` allocated (no RAII guard, no free in the landing pads). See the u02 doc.
    EVP_PKEY_CTX_free(contexto);
    return semente;
}

} // namespace ecourna::api::security
