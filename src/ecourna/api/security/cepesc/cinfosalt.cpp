// ecourna-lib/ecourna/api/security/cepesc/cinfosalt.cpp
// (/home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/api/security/cepesc/cinfosalt.cpp in the srclocs)
//
// Reconstructed from vota_web_wasm.wasm. Unit u01.
#include "ecourna/api/security/cepesc/cinfosalt.hpp"

#include "ecourna/api/exception/cbaseerror.hpp"
#include "ecourna/api/security/esecurityerror.hpp"   // ESecurityError; CSecurityError = exception::CBaseError<ESecurityError, SErrorLimits{1325, 1725}> (alias name inferred)

namespace ecourna::api::cepesc {

using security::CSecurityError;
using security::ESecurityError;

// wasm func 5169 (srclocs lines 27, 31, 35)
// Callers: comum::asn::CConversorBiometriaEleitorCifrada::vf3 (func 11419), comum::CGravadorRCSecao (func 11616).
// The members are copied first; the checks read the parameters.
CInfoSalt::CInfoSalt(const std::vector<uebyte>& salt, const std::vector<uebyte>& info)
    : m_salt(salt), m_info(info)
{
    if (salt.empty()) {
        throw CSecurityError(ESecurityError::SaltVazio /* 1513 */, "salt vazio.");            // line 27
    }
    if (salt.size() < TAMANHO_MINIMO_SALT) {   // compiled as (end - begin) <= 15
        throw CSecurityError(ESecurityError::SaltCurto /* 1514 */,                           // line 31
                             "tamanho minimo do salt deve ser 16 bytes.");
    }
    if (info.empty()) {
        throw CSecurityError(ESecurityError::InfoVazio /* 1515 */, "conteúdo de info vazio."); // line 35
    }
}

} // namespace ecourna::api::cepesc
