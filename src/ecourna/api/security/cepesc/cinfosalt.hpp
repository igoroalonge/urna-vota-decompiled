// ecourna-lib/ecourna/api/security/cepesc/cinfosalt.hpp   (path inferred)
//
// Reconstructed from vota_web_wasm.wasm. Unit u01.
//
// CEPESC = the TSE's encryption scheme for result files, named after the Centro de Pesquisa e
// Desenvolvimento para a Segurança das Comunicações. The ecourna::api::cepesc classes of this unit are its
// DATA objects: CPlainText (input), CCipheredOut (output), CInfoSalt (optional salt + "info" pair).
// The cipher itself is ecourna::api::cepesc::CCepescCipher (funcs 2681/5171, not in this unit), which in the
// web build does NOT encrypt (see the doc).
//
// CInfoSalt maps to the ASN.1 pairs  DadosCifracao { chave, salt, informacaoAdicional }
// (ModuloResultadoUrnaCadastro) and BiometriaEleitorCifrada { ..., salt } (ModuloEleitores).
#pragma once

#include <vector>

namespace ecourna::api::cepesc {

class CInfoSalt {
public:
    CInfoSalt(const std::vector<uebyte>& salt, const std::vector<uebyte>& info);   // wasm func 5169
    CInfoSalt(const CInfoSalt&) = default;                                           // func 9470 (not in unit)

    const std::vector<uebyte>& GetSalt() const { return m_salt; }   // inlined, name inferred
    const std::vector<uebyte>& GetInfo() const { return m_info; }   // inlined, name inferred

private:
    static constexpr std::size_t TAMANHO_MINIMO_SALT = 16;   // name inferred

    std::vector<uebyte> m_salt;   // +0
    std::vector<uebyte> m_info;   // +12
};                                // sizeof 24 (operator new(24) in func 9466)

} // namespace ecourna::api::cepesc
