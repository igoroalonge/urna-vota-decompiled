// FRAGMENT of ecourna-lib/ecourna/app/dados/resultadournacadastro/chabilitacaobiometrica.cpp (srcloc-attested;
// class declared by unit u14 in chabilitacaobiometrica.h). Reconstructed by unit u40 from vota_web_wasm.wasm.
// Neither constructor validates anything (no srcloc). Callers: CConversorHabilitacaoBiometrica::DoDeconverte
// (func 9066) and comum::CGravadorRCSecao (func 11616, the attendance-file writer). Not observed executing.
#include "ecourna/app/dados/resultadournacadastro/chabilitacaobiometrica.h"

namespace ecourna::app::dados {

// wasm func 5087. Biometric enabling without "habilitação por código": the optional stays empty (+44 = 0).
CHabilitacaoBiometrica::CHabilitacaoBiometrica(int tentativas, TScoreHabilitacao ultimoScore, CDedo::ETipoDedo dedo,
                                               TErroLeituraBiometria erro)
    : m_tentativas(tentativas)          // +0
    , m_ultimoScore(ultimoScore)        // +4 (16-bit store)
    , m_dedo(dedo)                      // +8
    , m_erroLeitura(erro)               // +12
    , m_porCodigo(std::nullopt)         // +16, engaged flag +44
{
}

// wasm func 2657 (table slot 6993). Same, plus a copy of the CEstadoHabilitacaoPorCodigo: situação (+16) and the
// optional mesário identification (shared_ptr copies with use_count + 1; inner optional flags +36 / +40).
CHabilitacaoBiometrica::CHabilitacaoBiometrica(int tentativas, TScoreHabilitacao ultimoScore, CDedo::ETipoDedo dedo,
                                               TErroLeituraBiometria erro,
                                               const CEstadoHabilitacaoPorCodigo& porCodigo)
    : m_tentativas(tentativas)
    , m_ultimoScore(ultimoScore)
    , m_dedo(dedo)
    , m_erroLeitura(erro)
    , m_porCodigo(porCodigo)            // engaged: +44 = 1
{
}

} // namespace ecourna::app::dados
