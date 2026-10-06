// uenux2/src/app/comum/dados/asn/eleitor/cvisitanteeleitor.cpp
// Reconstructed from vota_web_wasm.wasm (unit u03).
#include "comum/dados/asn/eleitor/cvisitanteeleitor.h"

#include "comum/dados/md/cvalidadoridentidade.h"

namespace comum::asn {

namespace {
constexpr auto CAMINHO_IDENTIFICACAO = "EntidadeEleitores.eleitores.eleitor.identificacaoEleitor.OF";   // @329740 (59 chars)
constexpr auto CAMINHO_BIOMETRIA     = "EntidadeEleitores.eleitores.eleitor.biometria";                  // @228809 (45 chars)
} // namespace

// wasm func 11407 (vtable slot 2; srcloc line 91)
// The two path comparisons are inlined memcmp's (XOR of 8-byte words against the literal).
ASN1::IAbstractVisitor::EVisitorResult
CVisitanteEleitor::visit(int /*nivel*/, const std::string& caminho, ASN1::AbstractData& dado,
                         const std::size_t inicio, const std::size_t fim)
{
    if (caminho == CAMINHO_IDENTIFICACAO) {
        // One element of EleitorUrna.identificacaoEleitor: remember it until the voter's biometria is reached.
        auto& identificador = static_cast<ModuloTiposEleitorais::IdentificadorEleitor&>(dado);
        const std::string& numero = identificador.getSelection<ASN1::AbstractString>().getValue();
        if (identificador.currentSelection() == 0) {   // numeroInscricao
            m_inscricao.assign(numero);
        }
        if (identificador.currentSelection() == 1) {   // numeroCPF
            m_cpf.assign(numero);
        }
        if (identificador.currentSelection() == 2) {   // identificacaoLivre
            m_livre.assign(numero);
        }
        return EVisitorResult{0};   // keep the decoded value   // enumerator name unknown
    }

    if (caminho == CAMINHO_BIOMETRIA) {
        m_posicaoBiometria = {inicio, fim};
        // Every identity seen so far is mapped to this biometria. NOTE: the three strings are never cleared,
        // so a voter without (say) a CPF inherits the CPF of the previous voter here. emplace() does not
        // overwrite, so an identity keeps its FIRST position: the only bogus entries are for identities of
        // earlier voters that had no biometria of their own, and CConversorEntidadeEleitores (func 11411) uses a
        // position only when the voter's own `biometria` field is present, so the stale entries are never used.
        // CValidadorIdentidade::Valida(numero, tipo) (func 566) checks the number and builds the md identity;
        // it throws on a malformed number, aborting the whole voter-file load.
        if (!m_inscricao.empty()) {
            m_posicoes.emplace(md::CValidadorIdentidade::Valida(m_inscricao, 1), m_posicaoBiometria);
        }
        if (!m_cpf.empty()) {
            m_posicoes.emplace(md::CValidadorIdentidade::Valida(m_cpf, 2), m_posicaoBiometria);
        }
        if (!m_livre.empty()) {
            m_posicoes.emplace(md::CValidadorIdentidade::Valida(m_livre, 3), m_posicaoBiometria);
        }
        return EVisitorResult{1};   // free the decoded biometria (ASN1::DataFree)   // enumerator name unknown
    }

    throw CDadosError(7905, "Entidade visitada inválida: " + caminho);   // line 91
}

} // namespace comum::asn

// ----------------------------------------------------------------------------------------------------------
// Library code emitted in this TU: wasm func 3728 —
// std::map<md::CEleitorIdentidade, TPosicao>::__emplace_unique_key_args (the emplace above; 40-byte node:
// key {std::string numero; int tipo} at +16, value {begin, end} at +32).
