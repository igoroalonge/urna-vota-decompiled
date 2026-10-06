// Reconstructed from vota_web_wasm.wasm (unit u04). Original: uenux2/src/app/comum/dados/clocal.h
//
// CLocal = where this urna is installed: either a regular polling section ("seção eleitoral": UF,
// município, zona, seção, local de votação, seções agregadas...) or a contingency urna ("urna de
// contingência", identified by UF/município/zona only). Wraps md::CLocal, read from the "locais"
// file (<zona><secao>-lo.dat, ASN.1 ModuloLocal) by asn::CConversorLocal (unit u03).
//
// Lazily created singleton (comum_f401, 4-byte object, @1838900; never throws). Every accessor
// first calls VerificaLido(<own name>) so that using it before the file is loaded is reported with
// the name of the accessor ("GetZona", "GetSecaoID", "GetUF", ...).
// Accessors outside u04: GetUF (comum_f1702), GetTodasSecoes (5741), comum_f820/1004/1077/1933/
// 2816/3752/3753/5738/5743.
#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "md/clocal.h"

namespace comum {

using TZonaID  = std::uint16_t;
using TSecaoID = std::uint16_t;

class CLocal {
public:
    static CLocal& GetInst();                                   // comum_f401 (lazy, no srcloc)

    TZonaID  GetZonaID() const;                                 // line 86  (wasm 1003)
    TSecaoID GetSecaoID() const;                                // line 110 (wasm 1078)
    const std::string& GetUF() const;                           // comum_f1702 (not in u04)
    void VerificaLido(const std::string& funcao) const;         // line 345 (wasm 782)
    void VerificaEhSecao(const std::string& funcao) const;      // line 354 (wasm 5742)

private:
    // +0. md::CLocal (137+ bytes): país +4 (string), UF +16 (string), nome da UF +28, município +40,
    //     optional<CSecaoEleitoral> at +84 (flag +120): municipio +92, zona +96, seção +104;
    //     optional<CIdentificacaoUrnaContingencia> at +124 (flag +136): municipio +128, zona +132.
    std::unique_ptr<md::CLocal> m_local;
};

}  // namespace comum
