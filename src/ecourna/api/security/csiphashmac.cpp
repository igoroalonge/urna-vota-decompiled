// ecourna-lib/ecourna/api/security/csiphashmac.cpp
// (/home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/api/security/csiphashmac.cpp in the srcloc records)
//
// Reconstructed from vota_web_wasm.wasm. Unit u13. OpenSSL 3.0.17 EVP_MAC "SIPHASH" provider.
//
// SipHash with c-rounds = 4 and d-rounds = 6 ("SipHash-4-6", not the usual 2-4), 8-byte tag, 16-byte key.
// Checked in the running wasm against a Python implementation (docs/bu/codigo-verificador.md, 51/51 cases).
#include "ecourna/api/security/imacalgorithm.hpp"

#include <openssl/core_names.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/params.h>

#include <cstring>
#include <exception>
#include <format>
#include <memory>
#include <vector>

#include "ecourna/api/security/esecurityerror.hpp"   // ESecurityError; CSecurityError

namespace ecourna::api::security {

namespace {
constexpr std::size_t TAMANHO_CHAVE_SIPHASH = 16;   // literal in DoVerify                     name inferred
constexpr std::size_t TAMANHO_MAC_SIPHASH = 8;      // literal in DoMac / DoVerify             name inferred
} // namespace

// wasm func 9498 (srcloc lines 52, 59, 66). Not observed during the recorded votes (it only runs when a BU
// is printed). The three error messages carry the numeric ERR_get_error() code, not its text.
void CSiphashMac::DoMac(const std::vector<uebyte>& dados, std::vector<uebyte>& mac, const std::vector<uebyte>& chave) const
{
    mac = std::vector<uebyte>(TAMANHO_MAC_SIPHASH, 0);

    std::unique_ptr<EVP_MAC, decltype(&EVP_MAC_free)> algoritmo(EVP_MAC_fetch(nullptr, "siphash", nullptr), &EVP_MAC_free);
    unsigned int tamanho = TAMANHO_MAC_SIPHASH;   // stored at sp+252
    unsigned int cRounds = 4;                     // sp+248
    unsigned int dRounds = 6;                     // sp+244
    std::vector<OSSL_PARAM> parametros(4);        // 80 bytes on the heap, zero-filled, then assigned
    parametros[0] = OSSL_PARAM_construct_uint(OSSL_MAC_PARAM_SIZE, &tamanho);       // "size"     @170537
    parametros[1] = OSSL_PARAM_construct_uint(OSSL_MAC_PARAM_C_ROUNDS, &cRounds);   // "c-rounds" @77434
    parametros[2] = OSSL_PARAM_construct_uint(OSSL_MAC_PARAM_D_ROUNDS, &dRounds);   // "d-rounds" @77425
    parametros[3] = OSSL_PARAM_construct_end();

    std::unique_ptr<EVP_MAC_CTX, decltype(&EVP_MAC_CTX_free)> contexto(EVP_MAC_CTX_new(algoritmo.get()), &EVP_MAC_CTX_free);

    if (EVP_MAC_init(contexto.get(), chave.data(), chave.size(), parametros.data()) != 1)
        throw CSecurityError(ESecurityError::FalhaAutenticar /* 1423 */,                          // line 52, name inferred
                             std::format("Falha ao autenticar. - {}", ERR_get_error()));
    if (EVP_MAC_update(contexto.get(), dados.data(), dados.size()) != 1)
        throw CSecurityError(ESecurityError::FalhaAutenticarUpdate /* 1424 */,                    // line 59, name inferred
                             std::format("Falha ao autenticar. - {}", ERR_get_error()));
    if (EVP_MAC_final(contexto.get(), mac.data(), nullptr, mac.size()) != 1)
        throw CSecurityError(ESecurityError::FalhaAutenticarFinal /* 1425 */,                     // line 66, name inferred
                             std::format("Falha ao autenticar. - {}", ERR_get_error()));
}

// wasm func 9497 (srcloc lines 79 and 88). Not called anywhere in the binary (reachable only through the
// vtable): nothing in VOTA verifies a Código Verificador.
// NOTE: the comparison reads 8 bytes from mac.data() without checking mac.size() (out-of-bounds read for a
// shorter tag, address 0 for an empty one). It is not written as a constant-time compare (no CRYPTO_memcmp),
// but the fixed 8-byte length makes it compile to a single i64.ne, so there is no timing leak in this build.
// Both errors use code 1426.
bool CSiphashMac::DoVerify(const std::vector<uebyte>& dados, const std::vector<uebyte>& mac, const std::vector<uebyte>& chave) const
{
    if (chave.size() != TAMANHO_CHAVE_SIPHASH)
        throw CSecurityError(ESecurityError::VerificarMac /* 1426 */,                             // line 79, name inferred
                             "O tamanho da chave deve ser igual 16 bytes.");
    try {
        std::vector<uebyte> calculado(TAMANHO_MAC_SIPHASH, 0);
        DoMac(dados, calculado, chave);   // direct call (table slot 6253), not through the vtable
        // The length is the CONSTANT 8 (calculado.size() is not re-read after DoMac, and mac.data() is loaded
        // first): i64.load(mac.data()) != i64.load(calculado.data()). A range compare over calculado would not
        // compile to a fixed-size load.
        return std::memcmp(mac.data(), calculado.data(), TAMANHO_MAC_SIPHASH) == 0;
    } catch (const std::exception&) {
        throw CSecurityError(ESecurityError::VerificarMac /* 1426 */, "Falha ao autenticar.");  // line 88
    }
}

} // namespace ecourna::api::security
