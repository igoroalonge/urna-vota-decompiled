// FRAGMENT of ecourna-lib/ecourna/app/dados/resultadournacadastro/cresultadournacadastro.cpp (srcloc-attested;
// class declared by unit u14). Reconstructed by unit u40 from vota_web_wasm.wasm.
// Callers of both constructors: CConversorResultadoUrnaCadastro::DoDeconverte (9052) and comum::CGravadorRCSecao
// (11616), which writes the section's attendance result ("resultado da urna para o cadastro") at the end of
// voting. Not observed executing.
#include "ecourna/app/dados/resultadournacadastro/cresultadournacadastro.h"

namespace ecourna::app::dados {

// wasm func 3483 (table slot 7032): attendance data in clear. Header (16 bytes) and fase copied; the version
// string and the optional CDadosComparecimento MOVED (buffers stolen, source zeroed); the encrypted optional
// (+112, flag +160) left empty.
CResultadoUrnaCadastro::CResultadoUrnaCadastro(CCabecalhoEntidade cabecalho, TFaseID fase, std::string versaoVotacao,
                                               ESituacaoArquivo situacao,
                                               std::optional<CDadosComparecimento> dados)
    : m_cabecalho(cabecalho)                           // +0
    , m_fase(fase)                                     // +16
    , m_versaoVotacao(std::move(versaoVotacao))        // +20
    , m_situacao(situacao)                             // +32
    , m_dadosComparecimento(std::move(dados))          // +36, flag +108
    , m_dadosComparecimentoCifrado(std::nullopt)       // +112, flag +160
{
}

// wasm func 3482 (table slot 7034): the encrypted variant (parameter criptografarJUFA); the clear optional stays empty.
CResultadoUrnaCadastro::CResultadoUrnaCadastro(CCabecalhoEntidade cabecalho, TFaseID fase, std::string versaoVotacao,
                                               ESituacaoArquivo situacao,
                                               std::optional<CDadosComparecimentoCifrado> dados)
    : m_cabecalho(cabecalho)
    , m_fase(fase)
    , m_versaoVotacao(std::move(versaoVotacao))
    , m_situacao(situacao)
    , m_dadosComparecimento(std::nullopt)
    , m_dadosComparecimentoCifrado(std::move(dados))   // four vectors moved (chave, salt, informação, conteúdo)
{
}

} // namespace ecourna::app::dados
