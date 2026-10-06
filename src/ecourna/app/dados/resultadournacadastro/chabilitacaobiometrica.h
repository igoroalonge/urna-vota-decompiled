// ecourna-lib/ecourna/app/dados/resultadournacadastro/chabilitacaobiometrica.h  (path inferred)
// Reconstructed from vota_web_wasm.wasm, unit u14.
//
// How a voter was enabled biometrically (fingerprint) at the mesário's terminal.
// = ModuloResultadoUrnaCadastro::HabilitacaoBiometrica
#pragma once

#include <optional>

#include "ecourna/app/dados/resultadournacadastro/cestadohabilitacaoporcodigo.h"
#include "ecourna/app/dados/tiposbasicos.h"

namespace ecourna::app::dados {

struct CDedo {   // (unit u40/u03) only the enum is used here
    enum ETipoDedo : int {   // same numbers as ModuloTiposEleitorais::TipoDedo
        NaoIdentificado = 0, PolegarDireito = 1, IndicadorDireito = 2, MedioDireito = 3, AnelarDireito = 4,
        MinimoDireito = 5, PolegarEsquerdo = 6, IndicadorEsquerdo = 7, MedioEsquerdo = 8, AnelarEsquerdo = 9,
        MinimoEsquerdo = 10,
    };
};

class CHabilitacaoBiometrica {                                                   // 48 bytes
public:
    // Constructors in unit u40: func 5087 (without code enabling), func 2657 (with it).
    CHabilitacaoBiometrica(int tentativas, TScoreHabilitacao ultimoScore, CDedo::ETipoDedo dedo,
                           TErroLeituraBiometria erro);
    CHabilitacaoBiometrica(int tentativas, TScoreHabilitacao ultimoScore, CDedo::ETipoDedo dedo,
                           TErroLeituraBiometria erro, const CEstadoHabilitacaoPorCodigo& porCodigo);

    int GetTentativas() const { return m_tentativas; }
    TScoreHabilitacao GetUltimoScore() const { return m_ultimoScore; }
    CDedo::ETipoDedo GetDedo() const { return m_dedo; }
    TErroLeituraBiometria GetErroLeitura() const { return m_erroLeitura; }
    bool PossuiEstadoHabilitacaoPorCodigo() const { return m_porCodigo.has_value(); }
    const CEstadoHabilitacaoPorCodigo& GetEstadoHabilitacaoPorCodigo() const;   // func 9016

private:
    int m_tentativas;                                        // +0  tentativasHabilitacaoBiometrica
    TScoreHabilitacao m_ultimoScore;                         // +4  (16-bit; 0 = absent)
    CDedo::ETipoDedo m_dedo;                                 // +8  dedoHabilitado
    TErroLeituraBiometria m_erroLeitura;                     // +12 erroLeituraBiometria
    std::optional<CEstadoHabilitacaoPorCodigo> m_porCodigo;  // +16 (flag +44)
};

} // namespace ecourna::app::dados
