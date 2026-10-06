// ecourna-lib/ecourna/api/security/ckey.cpp
// (/home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/api/security/ckey.cpp in the srcloc records)
//
// Reconstructed from vota_web_wasm.wasm. Unit u13.
#include "ecourna/api/security/ckey.hpp"

#include <algorithm>
#include <string>
#include <vector>

#include "ecourna/api/security/esecurityerror.hpp"   // ESecurityError; CSecurityError

namespace ecourna::api::security {

// srcloc line 31 - inlined into vota::CGeraBU::vf2 (func 12110), not in u13's function list.
CKeyData::CKeyData(const std::vector<uebyte>& bytes)
    : m_bytes(bytes)
{
    if (m_bytes.empty())
        throw CSecurityError(ESecurityError::BytesChaveVazio /* 1411 */, "Bytes da chave vazio.");   // name inferred
}

// Inlined in func 12110: memset(begin, 0, size) before the vector is freed.
CKeyData::~CKeyData()
{
    std::fill(m_bytes.begin(), m_bytes.end(), uebyte{0});
}

// wasm func 9504 (srcloc lines 55, 58 and 61). Members are initialised first, then validated in order.
CKey::CKey(const std::string& titular, uedword idParChaves, KeyType tipo,
           const std::vector<uebyte>& bytes, const std::string& tag)
    : m_titular(titular), m_idParChaves(idParChaves), m_tipo(tipo), m_bytes(bytes), m_tag(tag), m_reservado(0)
{
    if (m_titular.empty())
        throw CSecurityError(ESecurityError::TitularChaveVazio /* 1412 */, "Titular da chave vazio.");                     // line 55
    if (m_idParChaves == 0)
        throw CSecurityError(ESecurityError::IdParChavesZero /* 1413 */, "Identificador do par de chaves igual a zero.");  // line 58
    if (m_bytes.empty())
        throw CSecurityError(ESecurityError::BytesChaveVazio2 /* 1414 */, "Bytes da chave vazio.");                        // line 61
    // m_tipo is not validated (KeyType::Invalida is accepted).
}

// Inlined in func 12110 (memset of the key bytes, then the members' destructors).
CKey::~CKey()
{
    std::fill(m_bytes.begin(), m_bytes.end(), uebyte{0});
}

} // namespace ecourna::api::security
