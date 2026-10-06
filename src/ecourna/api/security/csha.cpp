// ecourna-lib/ecourna/api/security/csha.cpp
// (/home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/api/security/csha.cpp in the srcloc records)
//
// Reconstructed from vota_web_wasm.wasm. Unit u13. OpenSSL 3.0.17 EVP digest API.
#include "ecourna/api/security/csha.hpp"

#include <openssl/evp.h>

#include <vector>

#include "ecourna/api/security/esecurityerror.hpp"   // ESecurityError; CSecurityError = exception::CBaseError<ESecurityError, SErrorLimits{1325, 1725}>

namespace ecourna::api::security {

// wasm func 5177 (srcloc line 23). Observed executing.
// The digest is looked up by name on every Reset (EVP_get_digestbyname), then the context is re-initialised.
void CSha::Reset()
{
    if (!EVP_DigestInit_ex2(m_contexto.get(), EVP_get_digestbyname(m_algoritmo.c_str()), nullptr))
        throw CSecurityError(ESecurityError::HashIniciarContexto /* 1417 */, "Falha ao iniciar o contexto.");   // name inferred
}

// wasm func 3519 (srcloc lines 30 and 35). Observed executing.
// Empty input is an error, so the hash of an empty message cannot be computed with this class.
void CSha::Update(const std::vector<uebyte>& dados)
{
    if (dados.empty())
        throw CSecurityError(ESecurityError::HashDadosVazios /* 1418 */,                                  // line 30
                             "Não é possível calcular o hash de dados vazios.");
    if (!EVP_DigestUpdate(m_contexto.get(), dados.data(), dados.size()))
        throw CSecurityError(ESecurityError::HashAtualizar /* 1419 */, "Falha ao atualizar o hash.");       // line 35
}

// wasm func 3518 (srcloc line 45). Observed executing.
// Returns m_tamanho bytes (64 for CSha512). The context is not reset afterwards.
const std::vector<uebyte> CSha::Finish()
{
    std::vector<uebyte> resumo(m_tamanho);
    if (!EVP_DigestFinal_ex(m_contexto.get(), resumo.data(), nullptr))
        throw CSecurityError(ESecurityError::HashFinalizar /* 1420 */, "Falha ao finalizar o hash.");
    return resumo;
}

} // namespace ecourna::api::security
