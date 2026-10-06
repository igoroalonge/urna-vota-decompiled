// ecourna-lib/ecourna/api/security/cepesc/cplaintext.cpp
// (/home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/api/security/cepesc/cplaintext.cpp in the srclocs)
//
// Reconstructed from vota_web_wasm.wasm. Unit u01.
#include "ecourna/api/security/cepesc/cplaintext.hpp"

#include <utility>

#include "ecourna/api/exception/cbaseerror.hpp"
#include "ecourna/api/security/esecurityerror.hpp"   // ESecurityError; CSecurityError = exception::CBaseError<ESecurityError, SErrorLimits{1325, 1725}> (alias name inferred)

namespace ecourna::api::cepesc {

using security::CSecurityError;
using security::ESecurityError;

namespace {
constexpr uebyte QUANTIDADE_TIPOS_ARQUIVO   = 3;   // name inferred (Seguranca.idTipoArquivo 0..2)
constexpr uebyte QUANTIDADE_ID_CRIPTOGRAFIA = 4;   // name inferred (Seguranca.idCriptografia 1..3)
} // namespace

// wasm func 9465 (srclocs lines 89, 92, 95, 98, 101, 104)
// Callers: func 5168 and comum::CGravadorRCSecao (func 11616, with a CInfoSalt).
// Func 5168 (no srcloc, classified app:comum) has the shape of a DELEGATING CPlainText constructor: it takes
// (this, ueword, ueword, 4 vectors by value), moves the vectors into temporaries, invokes this constructor
// (table slot 6328) with tipoArquivo 0, idCriptografia 1 and an empty shared_ptr<CInfoSalt>, and returns
// `this`. Probably:                                                                        // ? name/signature inferred
//   CPlainText(ueword zona, ueword secao, std::vector<uebyte> tabela, std::vector<uebyte> numero,
//              std::vector<uebyte> chave, std::vector<uebyte> conteudo)
//       : CPlainText(0, 1, zona, secao, std::move(tabela), std::move(numero), std::move(chave),
//                    std::move(conteudo), nullptr) {}
// Its callers are comum::CGravadorBU::GravaResultado (func 11629) and
// CControlaArmazenamentoDeImagens::CifrarWsq (srcloc ccontrolaarmazenamentodeimagens.cpp:149/152, inlined into
// func 2725, which the database names after another inlined function, LeChavePublica).
// All members are initialised (moved) before the checks; the four emptiness checks read the members.
CPlainText::CPlainText(const uebyte tipoArquivo, const uebyte idCriptografia,
                       const ueword zona, const ueword secao,
                       std::vector<uebyte> tabelaCriptografia, std::vector<uebyte> numeroAleatorio,
                       std::vector<uebyte> chave, std::vector<uebyte> conteudo,
                       std::shared_ptr<CInfoSalt> infoSalt)
    : m_tipoArquivo(tipoArquivo),
      m_idCriptografia(idCriptografia),
      m_zona(zona),
      m_secao(secao),
      m_tabelaCriptografia(std::move(tabelaCriptografia)),
      m_numeroAleatorio(std::move(numeroAleatorio)),
      m_chave(std::move(chave)),
      m_conteudo(std::move(conteudo)),
      m_infoSalt(std::move(infoSalt))
{
    if (tipoArquivo >= QUANTIDADE_TIPOS_ARQUIVO) {
        throw CSecurityError(ESecurityError::TipoArquivoInvalido /* 1517 */,       // line 89
                             "tipo do arquivo inválido.");
    }
    if (idCriptografia >= QUANTIDADE_ID_CRIPTOGRAFIA) {
        throw CSecurityError(ESecurityError::IdCriptografiaInvalido /* 1518 */,    // line 92
                             "id da criptografia inválido.");
    }
    if (m_tabelaCriptografia.empty()) {
        throw CSecurityError(ESecurityError::TabelaCriptografiaVazia /* 1519 */,   // line 95
                             "tabela de criptografia da UE vazia.");
    }
    if (m_numeroAleatorio.empty()) {
        throw CSecurityError(ESecurityError::NumeroAleatorioVazio /* 1520 */,      // line 98
                             "número aleatório da UE vazio.");
    }
    if (m_chave.empty()) {
        throw CSecurityError(ESecurityError::ChaveVazia /* 1521 */,                // line 101
                             "chave vazia.");
    }
    if (m_conteudo.empty()) {
        throw CSecurityError(ESecurityError::ConteudoVazio /* 1522 */,             // line 104
                             "conteúdo vazio.");
    }
}

} // namespace ecourna::api::cepesc
