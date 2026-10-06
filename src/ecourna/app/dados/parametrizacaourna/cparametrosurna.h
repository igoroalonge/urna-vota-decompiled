// ecourna-lib/ecourna/app/dados/parametrizacaourna/cparametrosurna.h, ctitulorelatorio.h,
// clabelparametrizado.h  (paths inferred)
// Reconstructed from vota_web_wasm.wasm, unit u14 (complements the u02 fragment cparametrosurna.u02.cpp).
//
// "Parametrização da urna": the national (t00000br-pu.dat) or state (e.g. t02400ac-pu.dat) urna
// parameters = ModuloParametrizacaoUrna::ParametrosUrna. The object is embedded at +88 of
// comum::CConfiguracaoEleicao (so CConfiguracaoEleicao+88/+92 are the two suspension rules, +484
// apresentarPartido, +486 imprimirQrCodeNoBU, as seen in units u06 and docs/bu/qrcode.md).
// Correction to u02: the three "vector<SRotulo>" it saw at +100/+112/+124 are six
// vector<CTituloRelatorio> at +100..+160 (20-byte items {alinhamento, estilo, texto}).
#pragma once

#include <string>
#include <vector>

namespace ecourna::app::dados {

// = TituloRelatorio { alinhamento, estilo, texto }: one header/footer line of a printed report
class CTituloRelatorio {                          // 20 bytes
public:
    enum EAlinhamentoTitulo : int { Esquerdo = 0, Direito = 1, Centro = 2 };   // ASN value - 1
    enum EEstiloTitulo : int { Normal = 0, Expandido = 1 };                      // ASN value - 1
    CTituloRelatorio(EAlinhamentoTitulo alinhamento, EEstiloTitulo estilo, const std::string& texto)
        : m_alinhamento(alinhamento), m_estilo(estilo), m_texto(texto) {}        // inlined into func 9145
    EAlinhamentoTitulo GetAlinhamento() const { return m_alinhamento; }
    EEstiloTitulo GetEstilo() const { return m_estilo; }
    const std::string& GetTexto() const { return m_texto; }
private:
    EAlinhamentoTitulo m_alinhamento;   // +0
    EEstiloTitulo m_estilo;             // +4
    std::string m_texto;                // +8
};
using TVectorTituloRelatorio = std::vector<CTituloRelatorio>;   // dtor = func 1673, grow = func 9152

// = LabelParametrizado { genero, longo, curto, longoPlural, curtoPlural }: how to call
// município / zona / seção / partido on screens and reports (e.g. "Seção Eleitoral"/"Seção")
class CLabelParametrizado {                       // 52 bytes; implicit dtor = func 2663
public:
    enum EGeneroLabel : int { Neutro = 0, Masculino = 1, Feminino = 2 };         // ASN value - 1
    CLabelParametrizado(EGeneroLabel genero, const std::string& longo, const std::string& curto,
                        const std::string& longoPlural, const std::string& curtoPlural)
        : m_genero(genero), m_longo(longo), m_curto(curto), m_longoPlural(longoPlural),
          m_curtoPlural(curtoPlural) {}                                          // inlined into func 9167
    EGeneroLabel GetGenero() const { return m_genero; }
    const std::string& GetLongo() const { return m_longo; }
    const std::string& GetCurto() const { return m_curto; }
    const std::string& GetLongoPlural() const { return m_longoPlural; }
    const std::string& GetCurtoPlural() const { return m_curtoPlural; }
private:
    EGeneroLabel m_genero;              // +0
    std::string m_longo;                // +4
    std::string m_curto;                // +16
    std::string m_longoPlural;          // +28
    std::string m_curtoPlural;          // +40
};

class CParametrosUrna {                           // 402 bytes (+2 padding)
public:
    enum EFormaSuspenderSemVoto : int { TornaNaoVotou = 0, TornaTodosBranco = 1, TornaTodosNulo = 2 };
    enum EFormaSuspenderComVoto : int { TornaOutrosBranco = 0, TornaOutrosNulo = 1, TornaNaoVotouParcial = 2 };

    // wasm func 5093 (no srcloc; name inferred). 47 arguments in field order; vectors, the string
    // and the labels are taken by value and moved in.
    CParametrosUrna(EFormaSuspenderSemVoto formaSuspensaoSemVoto, EFormaSuspenderComVoto formaSuspensaoComVoto,
                    int numZeresimas, int numBUVotaObrigatorios, int numBUVotaAdicionais,
                    int numBUFinalREDObrigat, int numBUFinalREDAdic, int numBUParcialREDObrigat,
                    int numBUParcialREDAdic, int numBUFinalSAObrigat, int numBUFinalSAAdic,
                    int numBUParcialSAObrigat, int numTentativasHabilitacao, int numTentativasVerificacao,
                    int numRelatorioEstado, int numRelatorioEleitores, int numRelatorioVersoesDados,
                    int numRelatorioPU, bool criptografarBU, bool criptografarJUFA, int numDigitosPartido,
                    int tempoDispararSuspensao, int tempoDispararSuspensaoTE, int tempoConfirmacaoVoto,
                    int tempoDesligamentoAutomatico, int tempoAvisoDesligamentoAutomatico,
                    TVectorTituloRelatorio cabecalho, TVectorTituloRelatorio rodapeZEVOTA,
                    TVectorTituloRelatorio rodapeBUVOTA, TVectorTituloRelatorio rodapeZESA,
                    TVectorTituloRelatorio rodapeBUSA, TVectorTituloRelatorio rodapeBUParcial,
                    std::string telaEmissaoBU, CLabelParametrizado labelMunicipio, CLabelParametrizado labelZona,
                    CLabelParametrizado labelSecao, CLabelParametrizado labelPartido, bool imprimirZeradosBU,
                    bool gravarZeradosBU, bool aceitarJustificativa, bool aceitarBrancoNulo,
                    bool apresentarPartido, bool permitirHabManualAudio, bool imprimirQrCodeNoBU,
                    bool exibirScoreBiometria, bool registrarMesarios, bool pedeAnoNascimentoEleitor);
    CParametrosUrna(const CParametrosUrna&);      // func 3777 (unit u02)
    ~CParametrosUrna();                           // func 2267 (unit u02)

    // Getters are inline (Get<Field>()); they are used by DoConverte (func 9164).
    EFormaSuspenderSemVoto GetFormaSuspenderSemVoto() const { return m_formaSuspensaoSemVoto; }
    EFormaSuspenderComVoto GetFormaSuspenderComVoto() const { return m_formaSuspensaoComVoto; }
    // ... one getter per member below

    EFormaSuspenderSemVoto m_formaSuspensaoSemVoto;   // +0
    EFormaSuspenderComVoto m_formaSuspensaoComVoto;   // +4
    int m_numZeresimas;                               // +8   zerésima copies to print
    int m_numBUVotaObrigatorios;                      // +12  BU copies printed at encerramento (VOTA)
    int m_numBUVotaAdicionais;                        // +16  ? extra BU copies the mesário may request
    int m_numBUFinalREDObrigat;                       // +20  RED = recuperação de dados
    int m_numBUFinalREDAdic;                          // +24
    int m_numBUParcialREDObrigat;                     // +28
    int m_numBUParcialREDAdic;                        // +32
    int m_numBUFinalSAObrigat;                        // +36  SA = sistema de apuração
    int m_numBUFinalSAAdic;                           // +40
    int m_numBUParcialSAObrigat;                      // +44
    int m_numTentativasHabilitacao;                   // +48  fingerprint attempts to enable a voter
    int m_numTentativasVerificacao;                   // +52
    int m_numRelatorioEstado;                         // +56
    int m_numRelatorioEleitores;                      // +60
    int m_numRelatorioVersoesDados;                   // +64
    int m_numRelatorioPU;                             // +68
    bool m_criptografarBU;                            // +72
    bool m_criptografarJUFA;                          // +73  encrypt the attendance (RC) data
    int m_numDigitosPartido;                          // +76
    int m_tempoDispararSuspensao;                     // +80  ? s (45 in the scenarios)
    int m_tempoDispararSuspensaoTE;                   // +84  ? s
    int m_tempoConfirmacaoVoto;                       // +88  ? ms (1000 in the scenarios)
    int m_tempoDesligamentoAutomatico;                // +92  ? s (1800)
    int m_tempoAvisoDesligamentoAutomatico;           // +96  ? s (60)
    TVectorTituloRelatorio m_cabecalho;               // +100 report header lines
    TVectorTituloRelatorio m_rodapeZEVOTA;            // +112 zerésima footer (VOTA)
    TVectorTituloRelatorio m_rodapeBUVOTA;            // +124 BU footer (VOTA)
    TVectorTituloRelatorio m_rodapeZESA;              // +136
    TVectorTituloRelatorio m_rodapeBUSA;              // +148
    TVectorTituloRelatorio m_rodapeBUParcial;         // +160
    std::string m_telaEmissaoBU;                      // +172 text shown when the BU is printed
    CLabelParametrizado m_labelMunicipio;             // +184
    CLabelParametrizado m_labelZona;                  // +236
    CLabelParametrizado m_labelSecao;                 // +288
    CLabelParametrizado m_labelPartido;               // +340
    bool m_imprimirZeradosBU;                         // +392
    bool m_gravarZeradosBU;                           // +393
    bool m_aceitarJustificativa;                      // +394
    bool m_aceitarBrancoNulo;                         // +395
    bool m_apresentarPartido;                         // +396
    bool m_permitirHabManualAudio;                    // +397
    bool m_imprimirQrCodeNoBU;                        // +398
    bool m_exibirScoreBiometria;                      // +399
    bool m_registrarMesarios;                         // +400
    bool m_pedeAnoNascimentoEleitor;                  // +401
};

// = EntidadeParametrizacaoUrna { cabecalho, parametros } (class and converter in units u40/u11)
class CParametrizacaoUrna;

} // namespace ecourna::app::dados
