// ecourna-lib/ecourna/api/security/imacalgorithm.cpp
// (/home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/api/security/imacalgorithm.cpp in the srcloc records)
//
// Reconstructed from vota_web_wasm.wasm. Unit u13.
//
// IMacAlgorithm::Mac has no out-of-line copy. It is inlined, with CSiphashMac's constructor and
// CStringUtils::HexToQWord, into wasm func 3700, which analysis/functions.tsv names after it but which is
// really comum::CCalculaCV's "compute the next code" method (see
// src/uenux2/src/app/comum/relatorios/ccalculacv.u13.cpp and docs/bu/codigo-verificador.md).
#include "ecourna/api/security/imacalgorithm.hpp"

#include <vector>

#include "ecourna/api/security/esecurityerror.hpp"   // ESecurityError; CSecurityError

namespace ecourna::api::security {

// srcloc lines 32 and 35 (inlined into func 3700). The key LENGTH is not checked here: CSiphashMac::DoMac
// relies on OpenSSL (EVP_MAC_init fails for a key that is not 16 bytes).
void IMacAlgorithm::Mac(const std::vector<uebyte>& dados, std::vector<uebyte>& mac, const std::vector<uebyte>& chave) const
{
    if (dados.empty())
        throw CSecurityError(ESecurityError::MacDadosVazios /* 1432 */, "Vetor de dados vazio.");   // line 32, name inferred
    if (chave.empty())
        throw CSecurityError(ESecurityError::MacChaveVazia /* 1433 */, "Chave vazia.");             // line 35, name inferred
    DoMac(dados, mac, chave);   // virtual call, vtable slot 2
}

} // namespace ecourna::api::security
