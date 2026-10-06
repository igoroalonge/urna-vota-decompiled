// ecourna-lib/ecourna/api/security/cepesc/ccipheredout.hpp   (path inferred)
//
// Reconstructed from vota_web_wasm.wasm. Unit u01.
//
// CCipheredOut is the output of CCepescCipher::Encrypt (func 2681): the (ciphered) key that the receiver
// needs, the (ciphered) content and, optionally, the salt/info pair. comum::md::CSeguranca takes m_chave as
// Seguranca.idArquivoChave; the content becomes the envelope's "conteudo".
#pragma once

#include <memory>
#include <vector>

#include "ecourna/api/security/cepesc/cinfosalt.hpp"

namespace ecourna::api::cepesc {

class CCipheredOut {
public:
    CCipheredOut(const std::vector<uebyte>& chave, const std::vector<uebyte>& conteudo);   // wasm func 9467
    CCipheredOut(const std::vector<uebyte>& chave, const std::vector<uebyte>& conteudo,
                 const CInfoSalt& infoSalt);                                               // wasm func 9466

private:
    std::vector<uebyte>        m_chave;      // +0
    std::vector<uebyte>        m_conteudo;   // +12
    std::shared_ptr<CInfoSalt> m_infoSalt;   // +24  null for the 2-argument constructor
};                                           // sizeof 32

} // namespace ecourna::api::cepesc
