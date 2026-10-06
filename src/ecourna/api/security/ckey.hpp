// ecourna-lib/ecourna/api/security/ckey.hpp   (path inferred from the .cpp in the srcloc records)
//
// Reconstructed from vota_web_wasm.wasm. Unit u13.
//
// Non-polymorphic value classes (no RTTI). In this binary a CKey is only built by vota::CGeraBU::vf2
// (func 12110) for the key of the BU's "Código Verificador" (file /dsk/fi/estatico/chave/cv.ber.pri,
// ASN.1 ModuloEnvelopeChave::EntidadeChave), with
//     CKey(descritor.nomeUsuario, descritor.serial, tipo (0 -> 0, 1 -> 1, other -> -1), chave decifrada, tagChaves)
// and immediately turned into a CKeyData whose first 16 bytes become the SipHash key of comum::CCalculaCV.
#pragma once

#include <string>
#include <vector>

#include "ecourna/types.hpp"

namespace ecourna::api::security {

// Values from the conversion done by the caller (func 12110) from ASN.1 TipoChave
// {chaveSecreta(0), chavePublica(1)}.                                           names inferred
enum class KeyType : int {
    Invalida = -1,
    Secreta  = 0,
    Publica  = 1,
};

// ckey.cpp line 31 - exists only inlined into func 12110.
class CKeyData {
public:
    explicit CKeyData(const std::vector<uebyte>& bytes);
    ~CKeyData();                                    // wipes the bytes (inlined memset in 12110)
    const std::vector<uebyte>& GetBytes() const { return m_bytes; }   // name inferred
private:
    std::vector<uebyte> m_bytes;                    // +0
};                                                  // sizeof 12

class CKey {
public:
    // wasm func 9504 (srcloc lines 55, 58, 61)
    CKey(const std::string& titular, uedword idParChaves, KeyType tipo,
         const std::vector<uebyte>& bytes, const std::string& tag);
    ~CKey();                                        // wipes the bytes (inlined memset in 12110)

    CKeyData GetKeyData() const { return CKeyData(m_bytes); }   // name inferred (inlined in 12110)

private:
    std::string         m_titular;                  // +0   descritor.nomeUsuario
    uedword             m_idParChaves;              // +12  descritor.serial (must not be 0)
    KeyType             m_tipo;                     // +16
    std::vector<uebyte> m_bytes;                    // +20  key material
    std::string         m_tag;                      // +32  tagChaves (NumericString SIZE(12))
    uedword             m_reservado{0};             // +44  ? always initialised to 0, never read here
};                                                  // sizeof 48

} // namespace ecourna::api::security
