// ecourna-lib/ecourna/api/security/caescipher.cpp
// (/home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/api/security/caescipher.cpp in the srcloc records)
//
// Reconstructed from vota_web_wasm.wasm. Unit u01. AES through the OpenSSL 3.0.17 EVP API.
//
// Summary of the behaviour (all confirmed from the wasm, see the doc for the evidence):
//  * (mode, keySize) selects a legacy EVP_aes_*() object from a 9-entry table; anything else throws 1330.
//  * SetKey only checks that key.key.size() == EVP key length; the IV length is NOT checked. Encrypt/Decrypt
//    copy the key IV into a local 16-byte zero-filled vector, so a shorter IV is silently zero-padded to 16
//    bytes and a longer one is truncated to its first 16 bytes (no out-of-bounds read).
//  * Encrypt: if the key has no IV, 16 random bytes come from IRng (CTrng -> std::random_device) and are
//    APPENDED to the ciphertext; if the key carries an IV, that fixed IV is used and nothing is appended.
//  * Decrypt mirrors it: without a key IV, the last 16 bytes of the input are the IV.
//  * OpenSSL padding is left at its default (PKCS#7 on for ECB/CBC).
//  * Update/Final failures only throw when ERR_get_error() returns a non-zero code.
#include "ecourna/api/security/caescipher.hpp"

#include <openssl/err.h>
#include <openssl/evp.h>

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace ecourna::api::security {

namespace {

// Global std::map at 0x1D2BDC (header @1911772, root @1911776, size @1911780), filled by the TU's static
// initializer (inlined into __wasm_call_ctors, func 14478) from a 9-entry initializer_list of
// {mode, bits, getter} triples (12 bytes each), in this order.                        // name inferred
using TCipherGetter = const EVP_CIPHER* (*)();
const std::map<std::pair<EOperationModes, ESymmetricCipherKeySize>, TCipherGetter> s_cipherTable = {
    {{EOperationModes::CBC, ESymmetricCipherKeySize::AES128}, &EVP_aes_128_cbc},   // table slot 6278 -> func 7422
    {{EOperationModes::CBC, ESymmetricCipherKeySize::AES192}, &EVP_aes_192_cbc},   // 6277 -> 7419
    {{EOperationModes::CBC, ESymmetricCipherKeySize::AES256}, &EVP_aes_256_cbc},   // 6276 -> 7415
    {{EOperationModes::ECB, ESymmetricCipherKeySize::AES128}, &EVP_aes_128_ecb},   // 6275 -> 7421
    {{EOperationModes::ECB, ESymmetricCipherKeySize::AES192}, &EVP_aes_192_ecb},   // 6274 -> 7418
    {{EOperationModes::ECB, ESymmetricCipherKeySize::AES256}, &EVP_aes_256_ecb},   // 6273 -> 7414
    {{EOperationModes::CTR, ESymmetricCipherKeySize::AES128}, &EVP_aes_128_ctr},   // 6272 -> 7420
    {{EOperationModes::CTR, ESymmetricCipherKeySize::AES192}, &EVP_aes_192_ctr},   // 6271 -> 7417
    {{EOperationModes::CTR, ESymmetricCipherKeySize::AES256}, &EVP_aes_256_ctr},   // 6270 -> 7413
};

// EVP_CIPHER_CTX_free is called through invoke_vi with a __clang_call_terminate landing pad, i.e. from a
// noexcept destructor: an RAII holder. unique_ptr with a function-pointer deleter matches that shape.
using TCipherCtx = std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)>;   // name inferred

} // namespace

// wasm func 9495 (srcloc line 54)
// The shared_ptr parameter is destroyed by the callee: libc++ ABI v2 marks shared_ptr [[clang::trivial_abi]].
CAesCipher::CAesCipher(const EOperationModes mode, const ESymmetricCipherKeySize keySize,
                       std::shared_ptr<IRng> rng)
    : m_mode(mode), m_rng(rng)   // copy (the refcount is incremented, then the parameter is released)
{
    const auto it = s_cipherTable.find({mode, keySize});
    if (it == s_cipherTable.end()) {
        // Also thrown for an unsupported *mode*: the message only talks about the key size.
        throw CSecurityError(ESecurityError::AesTamanhoChaveInvalido /* 1330 */,   // name inferred
                             "Chave AES de tamanho inválido.");
    }
    m_cipher    = it->second();
    m_blockSize = EVP_CIPHER_get_block_size(m_cipher);   // inlined: m_cipher->block_size (+4)
}

// wasm func 9494 (srcloc line 70)
void CAesCipher::SetKey(const CAesKey& key)
{
    // EVP_CIPHER_get_key_length inlined as m_cipher->key_len (+8): 16/24/32 bytes.
    if (static_cast<std::size_t>(EVP_CIPHER_get_key_length(m_cipher)) != key.key.size()) {
        throw CSecurityError(ESecurityError::AesTamanhoChaveDivergente /* 1331 */,  // name inferred
                             "Chave AES de tamanho inválido.");
    }
    // Defaulted CAesKey::operator=: vector::operator= on key (func 1681 = __assign_with_size<uebyte*, uebyte*>),
    // keySize copy, vector::operator= on iv (func 1681 again). The self-assignment test is inlined.
    // Note: key.iv may have any length. Encrypt/Decrypt copy it into a local vector that already holds 16 zero
    // bytes, so OpenSSL always reads 16 in-bounds bytes: short IV = zero-padded, long IV = truncated.
    m_key = key;
}

// wasm func 9493 (srclocs lines 79, 96, 100, 105)
void CAesCipher::Encrypt(const std::vector<uebyte>& in, std::vector<uebyte>& out)
{
    TCipherCtx ctx(EVP_CIPHER_CTX_new(), &EVP_CIPHER_CTX_free);
    if (!ctx) {
        throw CSecurityError(ESecurityError::EncryptAlocacaoContexto /* 1332 */,   // line 79, name inferred
                             "Erro ao alocar contexto de cifra OpenSSL.");
    }

    // Worst case output: input + one block of padding. NOT a std::vector: the allocation goes through
    // func 7750 = operator new[](size_t) (unconditionally, even for n == 0), followed by a memset (value
    // initialisation), and the pointer is released with a null-checked free on every path, including the
    // landing pads (RAII). The std::vector<uebyte> allocations of this function (iv) call operator new
    // (func 137) directly instead.
    auto buffer = std::make_unique<uebyte[]>(in.size() + m_blockSize);
    std::vector<uebyte> iv(IV_SIZE);                    // 16 zero bytes

    if (m_key.iv.empty()) {
        // IRng vtable slot 5 (CTrng::vf5, func 5173): fills 16 bytes from std::random_device.
        // Its int status (0 ok, 1 n==0, 2 buffer too small, -1 std::exception) is IGNORED: on failure the IV
        // silently stays all-zero (or partly filled if random_device throws in the middle of the loop).
        m_rng->Fill(iv, IV_SIZE);                       // name inferred
    } else {
        // shared_f1158 = vector<uebyte>::__assign_with_size for CONST iterators. `iv = m_key.iv` would have
        // produced vector::operator=, i.e. func 1681 (the instantiation SetKey uses), so the source used an
        // iterator-range assign. The existing 16-byte capacity is reused when key.iv.size() <= 16.
        iv.assign(m_key.iv.cbegin(), m_key.iv.cend());
    }

    if (!EVP_EncryptInit_ex(ctx.get(), m_cipher, nullptr, m_key.key.data(), iv.data())) {
        throw CSecurityError(ESecurityError::EncryptInit /* 1333 */,               // line 96, name inferred
                             "EncryptInit_ex falhou.");
    }

    int outLen   = 0;   // sp+56
    int finalLen = 0;   // sp+52
    if (!EVP_EncryptUpdate(ctx.get(), buffer.get(), &outLen, in.data(), static_cast<int>(in.size()))) {
        if (const auto erro = ERR_get_error(); erro != 0) {
            throw CSecurityError(ESecurityError::EncryptUpdate /* 1334 */,         // line 100, name inferred
                                 ERR_error_string(erro, nullptr));
        }
        // no error queued: carries on as if it had worked
    }
    if (!EVP_EncryptFinal_ex(ctx.get(), buffer.get() + outLen, &finalLen)) {
        if (const auto erro = ERR_get_error(); erro != 0) {
            throw CSecurityError(ESecurityError::EncryptFinal /* 1335 */,          // line 105, name inferred
                                 ERR_error_string(erro, nullptr));
        }
    }

    out.assign(buffer.get(), buffer.get() + outLen + finalLen);                    // func 1681 (uebyte* range)
    if (m_key.iv.empty()) {
        out.insert(out.end(), iv.begin(), iv.end());    // func 9492 = vector::insert(pos, first, last)
    }
}   // ~iv, ~buffer, ~ctx (EVP_CIPHER_CTX_free)

// wasm func 9491 (srclocs lines 119, 141, 145, 150)
void CAesCipher::Decrypt(const std::vector<uebyte>& in, std::vector<uebyte>& out)
{
    TCipherCtx ctx(EVP_CIPHER_CTX_new(), &EVP_CIPHER_CTX_free);
    if (!ctx) {
        throw CSecurityError(ESecurityError::DecryptAlocacaoContexto /* 1336 */,   // line 119, name inferred
                             "Erro ao alocar contexto de cifra OpenSSL.");
    }

    auto buffer = std::make_unique<uebyte[]>(in.size() + m_blockSize);   // operator new[] (func 7750) + memset
    std::vector<uebyte> iv(IV_SIZE);
    std::vector<uebyte> cifrado(in.size());             // name inferred; sized then re-assigned below

    if (m_key.iv.empty()) {
        // The IV travels at the end of the ciphertext (see Encrypt). There is NO check that
        // in.size() >= IV_SIZE: CBlockCipher only rejects an empty input. With 1..15 bytes the copy reads
        // before in.data() and the assign below gets a negative length -> std::length_error("vector").
        iv.assign(in.end() - IV_SIZE, in.end());        // inlined as a 16-byte copy
        cifrado.assign(in.begin(), in.end() - IV_SIZE); // shared_f1158
    } else {
        iv.assign(m_key.iv.cbegin(), m_key.iv.cend());  // shared_f1158 (const iterators, see Encrypt)
        cifrado.assign(in.begin(), in.end());           // shared_f1158
    }

    if (!EVP_DecryptInit_ex(ctx.get(), m_cipher, nullptr, m_key.key.data(), iv.data())) {
        throw CSecurityError(ESecurityError::DecryptInit /* 1337 */,               // line 141, name inferred
                             "DecryptInit_ex falhou.");
    }

    int outLen   = 0;   // sp+72
    int finalLen = 0;   // sp+68
    if (!EVP_DecryptUpdate(ctx.get(), buffer.get(), &outLen, cifrado.data(),
                           static_cast<int>(cifrado.size()))) {
        if (const auto erro = ERR_get_error(); erro != 0) {
            throw CSecurityError(ESecurityError::DecryptUpdate /* 1338 */,         // line 145, name inferred
                                 ERR_error_string(erro, nullptr));
        }
    }
    if (!EVP_DecryptFinal_ex(ctx.get(), buffer.get() + outLen, &finalLen)) {
        if (const auto erro = ERR_get_error(); erro != 0) {
            throw CSecurityError(ESecurityError::DecryptFinal /* 1339 */,          // line 150, name inferred
                                 ERR_error_string(erro, nullptr));
        }
    }

    out.assign(buffer.get(), buffer.get() + outLen + finalLen);                    // func 1681 (uebyte* range)
}

// wasm func 9486: CAesCipher::~CAesCipher() — implicit destructor, emitted out of line only for the landing
// pads of func 6154 (the success path inlines it): frees m_key.iv (+32), m_key.key (+16), releases m_rng (+12).

} // namespace ecourna::api::security
