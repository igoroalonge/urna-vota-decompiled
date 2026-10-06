// ecourna-lib/ecourna/api/security/cepesc/cplaintext.hpp   (path inferred)
//
// Reconstructed from vota_web_wasm.wasm. Unit u01.
//
// CPlainText is the input of CCepescCipher::Encrypt (CCepescCipher vtable slot 2, func 2681). Its first two
// fields become the ASN.1 ModuloTiposEleitorais.Seguranca { idTipoArquivo INTEGER (0..2),
// idCriptografia INTEGER (1..3), idArquivoCD, idArquivoChave } through comum::md::CSeguranca.
#pragma once

#include <memory>
#include <vector>

#include "ecourna/api/security/cepesc/cinfosalt.hpp"

namespace ecourna::api::cepesc {

class CPlainText {
public:
    // wasm func 9465 (srclocs lines 89..104). Vectors and the shared_ptr are taken by value and moved in.
    CPlainText(const uebyte tipoArquivo, const uebyte idCriptografia,
               const ueword zona, const ueword secao,                       // ? names inferred, see below
               std::vector<uebyte> tabelaCriptografia, std::vector<uebyte> numeroAleatorio,
               std::vector<uebyte> chave, std::vector<uebyte> conteudo,
               std::shared_ptr<CInfoSalt> infoSalt);

    // ? Probably a delegating overload (func 5168, no srcloc, outside u01): passes tipoArquivo 0,
    // idCriptografia 1 and no CInfoSalt. See cplaintext.cpp.
    // CPlainText(const ueword zona, const ueword secao, std::vector<uebyte> tabelaCriptografia,
    //            std::vector<uebyte> numeroAleatorio, std::vector<uebyte> chave, std::vector<uebyte> conteudo);

private:
    uebyte                     m_tipoArquivo;          // +0   < 3   (0..2, Seguranca.idTipoArquivo)
    uebyte                     m_idCriptografia;       // +1   < 4   (ASN.1 says 1..3; 0 is accepted here)
    ueword                     m_zona;                 // +2   ? both callers pass two 16-bit fields that they
    ueword                     m_secao;                // +4   ? also wrap in CBaseType<0,9999> (zona, seção)
    std::vector<uebyte>        m_tabelaCriptografia;   // +8   "tabela de criptografia da UE" (1024 bytes in CGravadorBU)
    std::vector<uebyte>        m_numeroAleatorio;      // +20  "número aleatório da UE"
    std::vector<uebyte>        m_chave;                // +32  key (the public key read by ...::LeChavePublica)
    std::vector<uebyte>        m_conteudo;             // +44  data to protect
    std::shared_ptr<CInfoSalt> m_infoSalt;             // +56  optional
};                                                     // sizeof 64

} // namespace ecourna::api::cepesc
