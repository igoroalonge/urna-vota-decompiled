// ecourna-lib/ecourna/app/dados/resultadournacadastro/cdadoscifracao.h and
// cdadoscomparecimentocifrado.h  (paths inferred)
// Reconstructed from vota_web_wasm.wasm, unit u14.
//
// Encrypted attendance data. When the parameter criptografarJUFA (-pu.dat) is TRUE the urna does
// not write the DadosComparecimento in clear in the attendance file; it encrypts its BER encoding
// with the CEPESC library and the public key "jufa.pk1" (comum::CGravadorRCSecao, unit u23) and
// stores ModuloResultadoUrnaCadastro::DadosComparecimentoCifrado { dadosCifracao, conteudo }.
#pragma once

#include <vector>

#include "ecourna/app/dados/tiposbasicos.h"

namespace ecourna::app::dados {

// = DadosCifracao { chave, salt, informacaoAdicional OCTET STRING }
class CDadosCifracao {                                   // 36 bytes
public:
    CDadosCifracao(std::vector<uebyte> chave, std::vector<uebyte> salt,
                   std::vector<uebyte> informacaoAdicional);          // func 5091
    // implicit copy = func 9054, implicit dtor = func 2659

    const std::vector<uebyte>& GetChave() const { return m_chave; }
    const std::vector<uebyte>& GetSalt() const { return m_salt; }
    const std::vector<uebyte>& GetInformacaoAdicional() const { return m_informacaoAdicional; }

private:
    std::vector<uebyte> m_chave;                 // +0  (encrypted content key; no size check)
    std::vector<uebyte> m_salt;                  // +12 (>= 16 bytes)
    std::vector<uebyte> m_informacaoAdicional;   // +24 (>= 16 bytes)
};

// = DadosComparecimentoCifrado { dadosCifracao DadosCifracao, conteudo OCTET STRING }
class CDadosComparecimentoCifrado {                       // 48 bytes; implicit dtor = func 3487
public:
    const CDadosCifracao& GetDadosCifracao() const { return m_dadosCifracao; }
    const std::vector<uebyte>& GetConteudo() const { return m_conteudo; }
private:
    CDadosCifracao m_dadosCifracao;              // +0
    std::vector<uebyte> m_conteudo;              // +36
};

} // namespace ecourna::app::dados
