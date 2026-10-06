// ecourna-lib/ecourna/api/security/cepesc/ccipheredout.cpp
// (/home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/api/security/cepesc/ccipheredout.cpp in the srclocs)
//
// Reconstructed from vota_web_wasm.wasm. Unit u01.
// Both constructors are called only from CCepescCipher::Encrypt (func 2681).
#include "ecourna/api/security/cepesc/ccipheredout.hpp"

#include <memory>

#include "ecourna/api/exception/cbaseerror.hpp"
#include "ecourna/api/security/esecurityerror.hpp"   // ESecurityError; CSecurityError = exception::CBaseError<ESecurityError, SErrorLimits{1325, 1725}> (alias name inferred)

namespace ecourna::api::cepesc {

using security::CSecurityError;
using security::ESecurityError;

// wasm func 9467 (srclocs lines 26, 29)
// The members are copied first; the two emptiness checks read the PARAMETERS (b/c in the wasm), not the members.
CCipheredOut::CCipheredOut(const std::vector<uebyte>& chave, const std::vector<uebyte>& conteudo)
    : m_chave(chave), m_conteudo(conteudo)
{
    if (chave.empty()) {
        throw CSecurityError(ESecurityError::CipheredOutChaveVazia /* 1504 */, "chave vazia.");       // line 26
    }
    if (conteudo.empty()) {
        throw CSecurityError(ESecurityError::CipheredOutConteudoVazio /* 1505 */, "conteúdo vazio."); // line 29
    }
}

// wasm func 9466 (srclocs lines 44, 47)
// The CInfoSalt is COPIED into a new heap object (operator new(24) + copy ctor func 9470) owned by a plain
// shared_ptr<CInfoSalt>(T*) (control block __shared_ptr_pointer<CInfoSalt*,...> @1115328), not make_shared.
CCipheredOut::CCipheredOut(const std::vector<uebyte>& chave, const std::vector<uebyte>& conteudo,
                           const CInfoSalt& infoSalt)
    : m_chave(chave), m_conteudo(conteudo), m_infoSalt(new CInfoSalt(infoSalt))
{
    if (chave.empty()) {
        throw CSecurityError(ESecurityError::CipheredOutSaltChaveVazia /* 1506 */, "chave vazia.");       // line 44
    }
    if (conteudo.empty()) {
        throw CSecurityError(ESecurityError::CipheredOutSaltConteudoVazio /* 1507 */, "conteúdo vazio."); // line 47
    }
}

} // namespace ecourna::api::cepesc
