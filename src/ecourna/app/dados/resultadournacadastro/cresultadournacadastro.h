// ecourna-lib/ecourna/app/dados/resultadournacadastro/cresultadournacadastro.h  (path inferred)
// Reconstructed from vota_web_wasm.wasm, unit u14.
//
// = ModuloResultadoUrnaCadastro::EntidadeResultadoUrnaCadastro: the section's attendance result
// file written at the end of voting by comum::CGravadorRCSecao (unit u23), either with the
// DadosComparecimento in clear or encrypted (DadosComparecimentoCifrado, parameter criptografarJUFA).
#pragma once

#include <optional>
#include <string>

#include "ecourna/app/dados/ccabecalhoentidade.h"   // CCabecalhoEntidade (16 bytes, unit u40)
#include "ecourna/app/dados/midias/cinformacaomidia.h"   // TFaseID
#include "ecourna/app/dados/resultadournacadastro/cdadoscifracao.h"
#include "ecourna/app/dados/resultadournacadastro/cdadoscomparecimento.h"

namespace ecourna::app::dados {

class CResultadoUrnaCadastro {                                              // 164 bytes
public:
    enum ESituacaoArquivo : int {   // ASN SituacaoArquivo - 1
        ArquivoFinal = 0,           // arquivoFinal (1)
        ArquivoParcial = 1,         // arquivoParcial (2)
    };

    // Constructors in unit u40 (func 3483 clear data, func 3482 encrypted data).
    CResultadoUrnaCadastro(CCabecalhoEntidade cabecalho, TFaseID fase, std::string versaoVotacao,
                           ESituacaoArquivo situacao, std::optional<CDadosComparecimento> dados);
    CResultadoUrnaCadastro(CCabecalhoEntidade cabecalho, TFaseID fase, std::string versaoVotacao,
                           ESituacaoArquivo situacao, std::optional<CDadosComparecimentoCifrado> dados);

    const CCabecalhoEntidade& GetCabecalho() const { return m_cabecalho; }
    TFaseID GetFase() const { return m_fase; }
    const std::string& GetVersaoVotacao() const { return m_versaoVotacao; }
    ESituacaoArquivo GetSituacao() const { return m_situacao; }
    bool PossuiDadosComparecimento() const { return m_dadosComparecimento.has_value(); }
    bool PossuiDadosComparecimentoCifrado() const { return m_dadosComparecimentoCifrado.has_value(); }
    const CDadosComparecimento& GetDadosComparecimento() const;                 // func 9015
    const CDadosComparecimentoCifrado& GetDadosComparecimentoCifrado() const;   // func 9014

private:
    CCabecalhoEntidade m_cabecalho;                                         // +0
    TFaseID m_fase;                                                         // +16
    std::string m_versaoVotacao;                                            // +20
    ESituacaoArquivo m_situacao;                                            // +32
    std::optional<CDadosComparecimento> m_dadosComparecimento;              // +36  (flag +108)
    std::optional<CDadosComparecimentoCifrado> m_dadosComparecimentoCifrado;// +112 (flag +160)
};

} // namespace ecourna::app::dados
