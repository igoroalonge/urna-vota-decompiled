// ecourna-lib/ecourna/app/dados/parametrizacaourna/cparametrosurna.cpp  (path inferred)
// Reconstructed from vota_web_wasm.wasm, unit u14. See also cparametrosurna.u02.cpp (copy
// constructor func 3777 and destructor func 2267, written by unit u02).
//
// The constructor cannot throw, has no std::source_location, and is only called by
// CConversorParametrosUrna::DoDeconverte (func 9156); the tool therefore attributed it to
// cconversorparametrosurna.cpp. It does not validate anything: all range checks come from the
// ASN.1 constraints (numZeresimas 1..50, tempoDispararSuspensao 15..360, ...), which
// IConversorASN::Deconverte enforces before DoDeconverte runs.
#include "ecourna/app/dados/parametrizacaourna/cparametrosurna.h"

namespace ecourna::app::dados {

// wasm func 5093 (name inferred)
CParametrosUrna::CParametrosUrna(
    EFormaSuspenderSemVoto formaSuspensaoSemVoto, EFormaSuspenderComVoto formaSuspensaoComVoto, int numZeresimas,
    int numBUVotaObrigatorios, int numBUVotaAdicionais, int numBUFinalREDObrigat, int numBUFinalREDAdic,
    int numBUParcialREDObrigat, int numBUParcialREDAdic, int numBUFinalSAObrigat, int numBUFinalSAAdic,
    int numBUParcialSAObrigat, int numTentativasHabilitacao, int numTentativasVerificacao, int numRelatorioEstado,
    int numRelatorioEleitores, int numRelatorioVersoesDados, int numRelatorioPU, bool criptografarBU,
    bool criptografarJUFA, int numDigitosPartido, int tempoDispararSuspensao, int tempoDispararSuspensaoTE,
    int tempoConfirmacaoVoto, int tempoDesligamentoAutomatico, int tempoAvisoDesligamentoAutomatico,
    TVectorTituloRelatorio cabecalho, TVectorTituloRelatorio rodapeZEVOTA, TVectorTituloRelatorio rodapeBUVOTA,
    TVectorTituloRelatorio rodapeZESA, TVectorTituloRelatorio rodapeBUSA, TVectorTituloRelatorio rodapeBUParcial,
    std::string telaEmissaoBU, CLabelParametrizado labelMunicipio, CLabelParametrizado labelZona,
    CLabelParametrizado labelSecao, CLabelParametrizado labelPartido, bool imprimirZeradosBU, bool gravarZeradosBU,
    bool aceitarJustificativa, bool aceitarBrancoNulo, bool apresentarPartido, bool permitirHabManualAudio,
    bool imprimirQrCodeNoBU, bool exibirScoreBiometria, bool registrarMesarios, bool pedeAnoNascimentoEleitor)
    : m_formaSuspensaoSemVoto(formaSuspensaoSemVoto)
    , m_formaSuspensaoComVoto(formaSuspensaoComVoto)
    , m_numZeresimas(numZeresimas)
    , m_numBUVotaObrigatorios(numBUVotaObrigatorios)
    , m_numBUVotaAdicionais(numBUVotaAdicionais)
    , m_numBUFinalREDObrigat(numBUFinalREDObrigat)
    , m_numBUFinalREDAdic(numBUFinalREDAdic)
    , m_numBUParcialREDObrigat(numBUParcialREDObrigat)
    , m_numBUParcialREDAdic(numBUParcialREDAdic)
    , m_numBUFinalSAObrigat(numBUFinalSAObrigat)
    , m_numBUFinalSAAdic(numBUFinalSAAdic)
    , m_numBUParcialSAObrigat(numBUParcialSAObrigat)
    , m_numTentativasHabilitacao(numTentativasHabilitacao)
    , m_numTentativasVerificacao(numTentativasVerificacao)
    , m_numRelatorioEstado(numRelatorioEstado)
    , m_numRelatorioEleitores(numRelatorioEleitores)
    , m_numRelatorioVersoesDados(numRelatorioVersoesDados)
    , m_numRelatorioPU(numRelatorioPU)
    , m_criptografarBU(criptografarBU)
    , m_criptografarJUFA(criptografarJUFA)
    , m_numDigitosPartido(numDigitosPartido)
    , m_tempoDispararSuspensao(tempoDispararSuspensao)
    , m_tempoDispararSuspensaoTE(tempoDispararSuspensaoTE)
    , m_tempoConfirmacaoVoto(tempoConfirmacaoVoto)
    , m_tempoDesligamentoAutomatico(tempoDesligamentoAutomatico)
    , m_tempoAvisoDesligamentoAutomatico(tempoAvisoDesligamentoAutomatico)
    , m_cabecalho(std::move(cabecalho))
    , m_rodapeZEVOTA(std::move(rodapeZEVOTA))
    , m_rodapeBUVOTA(std::move(rodapeBUVOTA))
    , m_rodapeZESA(std::move(rodapeZESA))
    , m_rodapeBUSA(std::move(rodapeBUSA))
    , m_rodapeBUParcial(std::move(rodapeBUParcial))
    , m_telaEmissaoBU(std::move(telaEmissaoBU))
    , m_labelMunicipio(std::move(labelMunicipio))
    , m_labelZona(std::move(labelZona))
    , m_labelSecao(std::move(labelSecao))
    , m_labelPartido(std::move(labelPartido))
    , m_imprimirZeradosBU(imprimirZeradosBU)
    , m_gravarZeradosBU(gravarZeradosBU)
    , m_aceitarJustificativa(aceitarJustificativa)
    , m_aceitarBrancoNulo(aceitarBrancoNulo)
    , m_apresentarPartido(apresentarPartido)
    , m_permitirHabManualAudio(permitirHabManualAudio)
    , m_imprimirQrCodeNoBU(imprimirQrCodeNoBU)
    , m_exibirScoreBiometria(exibirScoreBiometria)
    , m_registrarMesarios(registrarMesarios)
    , m_pedeAnoNascimentoEleitor(pedeAnoNascimentoEleitor)
{
}

// Library instantiations compiled with this class (called from the converter):
//   wasm func 1673  ~std::vector<CTituloRelatorio>()
//   wasm func 9152  std::vector<CTituloRelatorio>::__push_back_slow_path(CTituloRelatorio&&) (growth 2x, max 214748364)
//   wasm func 2663  ~CLabelParametrizado() (four strings)

} // namespace ecourna::app::dados
