// uenux2/src/app/comum/asn/util.cpp
// Reconstructed from vota_web_wasm.wasm (unit u21). The srcloc records give the line of every throw (listed next to
// each function). Functions that only survive inlined into a caller are reconstructed from that caller and say so.
// Other fragments of this file: util.u13.cpp (DesconverteDataJE, wasm 2276).
//
// Enum formatting: the md/comum enums (EUrnaFase, md::ETipoPacote, ESistemaJE, ETipoLocalVotacao, the ASN.1
// TipoAbrangencia::NamedNumber) are formatted through a std::formatter specialisation (format-arg "handle", table
// slots 2285/2286/2287/2289/2290 -> bodies 536/1711). ASN.1 ENUMERATED values and md::ETipoAbrangencia are
// formatted as int.
#include "comum/asn/util.h"

#include <array>
#include <format>
#include <string>
#include <utility>

#include "api/util/ctime.h"                       // api::CTime (EncodeTime, ctime.cpp:63 "Hora inválida")
#include "comum/asn/iconversorasn.h"              // CUeComumAsnError

namespace comum::asn {

using ModuloTiposEleitorais::DataHoraJE;
using ModuloTiposEleitorais::DataJE;
using ModuloTiposEleitorais::HoraJE;

// ---------------------------------------------------------------------------------------------------------------
// wasm func 1713 (srcloc line 41). Observed executing: every CabecalhoEntidade read (static data load, eg.bin).
// "YYYYMMDDTHHMMSS" -> api::CDateTime (CDate at +0, 8 bytes; CTime at +8).
//   * text shorter than 8 characters: std::out_of_range from substr (libc++ __throw_out_of_range);
//   * character 8 missing or not 'T': "Formato de data inválido." (7671);
//   * the date part is parsed by DesconverteDataJE (wasm 2276), the time part by api::CTime (wasm 3643).
// Note: the length of the time part is not checked here (substr(9, 6) accepts less); DataHoraJE is SIZE(15) and
// is validated by IConversorASN::Desconverte before this is reached.
api::CDateTime Utils::DesconverteDataHoraJE(const DataHoraJE& dataHora)
{
    const std::string& texto = dataHora;                                    // std::string at +8 of the ASN.1 object
    const api::CDate data = DesconverteDataJE(DataJE(texto.substr(0, 8).c_str()));
    if (texto.substr(8, 1) != "T") {
        throw CUeComumAsnError(EUeComumAsnError(7671), "Formato de data inválido.");               // line 41
    }
    const HoraJE hora(texto.substr(9, 6).c_str());
    return api::CDateTime(data, api::CTime(hora));
}

// ---------------------------------------------------------------------------------------------------------------
// :89 DesconverteIdEleitoral - inlined into CConversorCabecalhoPacote::DoDesconverte (wasm 11437, other unit).

// ---------------------------------------------------------------------------------------------------------------
// wasm func 1712 (srcloc line 102). Callers: CConversorEntidadeHashes, CConversorEnvelopeGenerico,
// CConversorEntidadeBU, CConversorDadoCarga, CConversorDadosDisponiveisCarga, CConversorCabecalhoPacote,
// CConversorRegistroDigitalVoto. The application codes the phase as a character, the ASN.1 enum in another order:
//   '1' oficial -> Fase::oficial (2)    '2' simulado -> Fase::simulado (1)    '3' treinamento -> Fase::treinamento (3)
// (compiled into the table @497848 = {2, 1, 3} indexed by fase - '1').
ModuloTiposEleitorais::Fase Utils::ConverteFase(const EUrnaFase& fase)
{
    using ModuloTiposEleitorais::Fase;
    switch (fase) {
    case EUrnaFase::Oficial:     return Fase(Fase::oficial);
    case EUrnaFase::Simulado:    return Fase(Fase::simulado);
    case EUrnaFase::Treinamento: return Fase(Fase::treinamento);
    }
    throw CUeComumAsnError(EUeComumAsnError(7674), std::format("Fase inválida: {}", fase));         // line 102
}

// wasm func 2843 (srcloc line 116). Inverse table @497860 = {'2', '1', '3'} indexed by value - 1.
EUrnaFase Utils::DesconverteFase(const ModuloTiposEleitorais::Fase& fase)
{
    using ModuloTiposEleitorais::Fase;
    switch (fase.asInt()) {
    case Fase::simulado:    return EUrnaFase::Simulado;       // 1 -> '2'
    case Fase::oficial:     return EUrnaFase::Oficial;        // 2 -> '1'
    case Fase::treinamento: return EUrnaFase::Treinamento;    // 3 -> '3'
    }
    throw CUeComumAsnError(EUeComumAsnError(7675), std::format("Fase inválida: {}", fase.asInt()));  // line 116
}

// ---------------------------------------------------------------------------------------------------------------
// :231 ConverteTipoPacote - inlined into CConversorCabecalhoPacote::DoConverte (wasm 11438).
// md::ETipoPacote is dense (0..32); the ASN.1 enum (56 values between 1 and 65, with gaps) has 23 values that the
// md enum cannot express: 41..47, 49, 50 and 52..65 (table @497872; ModuloTiposPacotes.asn).
ModuloTiposPacotes::TipoPacote Utils::ConverteTipoPacote(const md::ETipoPacote& tipo)
{
    using T = ModuloTiposPacotes::TipoPacote;
    static constexpr std::array<int, 33> kTabela = {
        T::pacoteEleitores, T::pacoteMunicipiosZonas, T::pacoteImpedidos, T::pacoteCandidatosTRE,              //  1  2  5  7
        T::pacoteFotosCandidatosTRE, T::pacoteCorrespondencia, T::pacoteSecoes, T::pacoteNotificacaoPacote,     //  8  9 10 13
        T::pacoteConfiguracaoProcessoEleitoral, T::pacoteValidacaoCandidatos, T::pacoteAlteracaoCandidaturaTRE, // 14 15 16
        T::pacoteResultadoTotalizacao, T::pacoteRespostaVersaoPacote, T::pacoteJuntas,                          // 17 18 21
        T::pacoteEleicoesSegundoTurno, T::pacoteParametrosUrna, T::pacoteNotificacaoImportacao,                 // 22 23 24
        T::pacotePrestacaoContas, T::pacoteSolicitacaoEstadoSistema, T::pacoteRespostaEstadoSistema,            // 25 26 27
        T::pacoteSolicitacaoVersaoPacote, T::pacoteProcessamentoResultadoUrnaCadastro,                          // 28 29
        T::pacoteNotificacaoDisponibilidadeSistema, T::pacoteCandidatosTSE, T::pacoteFotosCandidatosTSE,        // 30 31 32
        T::pacoteAlteracaoCandidaturaTSE, T::pacoteSolicitacaoDadosCadastro, T::pacoteDadosEleitor,            // 33 34 35
        T::pacoteDadosDocumentacaoCandidatura, T::pacoteComplementosMunicipios, T::pacoteEleicao,               // 36 37 39
        T::pacoteSituacoesEleicoes, T::pacoteTransferenciaTemporariaEleitores,                                  // 40 48
    };
    const auto indice = static_cast<unsigned>(tipo);
    if (indice >= kTabela.size()) {
        throw CUeComumAsnError(EUeComumAsnError(7678), std::format("Tipo de pacote inválido: {}", tipo)); // line 231
    }
    return T(kTabela[indice]);
}

// :329 DesconverteTipoPacote - inlined into wasm 11437 (other unit).

// :344 ConverteIdPacote - inlined into CConversorCabecalhoPacote::DoConverte (wasm 11438).
// md::CIDPacote (+0 id, +4 tipo IDEleitoral 1..3, +8 EUrnaFase, +12 optional<string> UF (flag +24),
// +28 optional<TMunicipioID> (flag +32), +36 optional<TZonaID> (flag +38)).
// The accessors UF()/Municipio()/Zona() (cidpacote.cpp:67/75/83) throw EUeComumMdError 8922/8923/8924
// "Valor não disponível." when empty; they are only called after the presence test.
ModuloTiposEleitorais::IDPacote Utils::ConverteIdPacote(const md::CIDPacote& id)
{
    ModuloTiposEleitorais::IDPacote entidade;
    const int tipo = static_cast<int>(id.GetTipoIdEleitoral());          // 1 processo eleitoral, 2 pleito, 3 eleição
    if (tipo < 1 || tipo > 3) {
        throw CUeComumAsnError(EUeComumAsnError(7680), "Tipo IDEleitoral inválido.");                // line 344
    }
    entidade.ref_idPacoteEleitoral().select(tipo - 1, ASN1::INTEGER::create()).setValue(id.GetId());  // CHOICE alt 0..2
    entidade.set_fase(ConverteFase(id.GetFase()));

    if (id.PossuiUF())        entidade.set_siglaUF(id.UF());               // optional 0 / field 2
    else                      entidade.omit_siglaUF();
    if (id.PossuiMunicipio()) entidade.set_codigoMunicipio(id.Municipio()); // optional 1 / field 3
    else                      entidade.omit_codigoMunicipio();
    if (id.PossuiZona())      entidade.set_numeroZona(id.Zona());           // optional 2 / field 4
    else                      entidade.omit_numeroZona();
    return entidade;
}

// :419 ConverteIdSistema - inlined into CConversorCabecalhoPacote::DoConverte (wasm 11438). Table @498196.
ModuloTiposEleitorais::Sistema Utils::ConverteIdSistema(const ESistemaJE& sistema)
{
    using S = ModuloTiposEleitorais::Sistema;
    static constexpr std::array<int, 12> kTabela = {
        S::configurador, S::intercad, S::candidaturas, S::simon,          // ESistemaJE 1..4  -> 1 2 3 13
        S::seweb, S::gedai, S::urnaEletronica, S::simulador,              //            5..8  -> 12 6 7 8
        S::parametrizador, S::geradorDeBases, S::sistot, S::padaUE,       //            9..12 -> 9 10 11 14
    };
    const auto indice = static_cast<unsigned>(sistema) - 1u;
    if (indice >= kTabela.size()) {
        throw CUeComumAsnError(EUeComumAsnError(7681), std::format("Sistema inválido: {}", sistema));   // line 419
    }
    return S(kTabela[indice]);
}

// :451 DesconverteIdSistema - inlined into wasm 11437.   :483 DesconverteSexo - inlined into wasm 11450.

// ---------------------------------------------------------------------------------------------------------------
// wasm func 5827 (srcloc line 498). Callers: CConversorEleicaoPE::DoDesconverte, CConversorAbrangencia::DoDesconverte.
// Identity mapping municipal 0 / estadual 1 / federal 2.
md::ETipoAbrangencia Utils::DesconverteAbrangencia(ModuloTiposEleitorais::TipoAbrangencia::NamedNumber tipo)
{
    if (static_cast<unsigned>(tipo) >= 3) {
        throw CUeComumAsnError(EUeComumAsnError(7686), std::format("Abrangência inválida: {}", tipo));  // line 498
    }
    return static_cast<md::ETipoAbrangencia>(tipo);
}

// :513 - inlined into CConversorAbrangencia::DoConverte (wasm 11462).
ModuloTiposEleitorais::TipoAbrangencia::NamedNumber Utils::ConverteAbrangencia(md::ETipoAbrangencia tipo)
{
    if (static_cast<unsigned>(tipo) >= 3) {
        throw CUeComumAsnError(EUeComumAsnError(7685),
                               std::format("Abrangência inválida: {}", std::to_underlying(tipo)));    // line 513
    }
    return static_cast<ModuloTiposEleitorais::TipoAbrangencia::NamedNumber>(tipo);
}

// wasm func 3801 (srcloc line 530). normal 1 / emTransito 2 / presoProvisorio 3 / temporario 4 (identity).
ModuloTiposCadastro::TipoLocalVotacao Utils::ConverteTipoLocalVotacao(ETipoLocalVotacao tipo)
{
    const int valor = static_cast<int>(tipo);
    if (valor < 1 || valor > 4) {
        throw CUeComumAsnError(EUeComumAsnError(7687),
                               std::format("Tipo de local de votação inválido: {}", tipo));         // line 530
    }
    return ModuloTiposCadastro::TipoLocalVotacao(valor);
}

// wasm func 3800 (srcloc line 549). The ENUMERATED is taken BY VALUE (callers copy it with ENUMERATED's copy ctor,
// e.g. wasm 11455).
ETipoLocalVotacao Utils::DesconverteTipoLocalVotacao(ModuloTiposCadastro::TipoLocalVotacao tipo)
{
    const int valor = tipo.asInt();
    if (valor < 1 || valor > 4) {
        throw CUeComumAsnError(EUeComumAsnError(7688),
                               std::format("Tipo de local de votação inválido: {}", valor));        // line 549
    }
    return static_cast<ETipoLocalVotacao>(valor);
}

// :566 ConverteTurno - inlined into CConversorDadoCarga::DoConverte (wasm 11401, other unit).
// :581 DesconverteTurno - inlined into CConversorDadoCarga::DesconverteModelo (wasm 11402, other unit).

} // namespace comum::asn
