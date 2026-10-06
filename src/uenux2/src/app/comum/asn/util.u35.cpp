// FRAGMENT of uenux2/src/app/comum/asn/util.cpp (attested by srclocs util.cpp:41..581; file owned by unit u21,
// declarations in util.h). Reconstructed from vota_web_wasm.wasm (unit u35):
//   * Utils::ConverteDataHoraJE        wasm func 1080 (out of line, no srcloc: it cannot throw)
//   * Utils::DesconverteIdEleitoral    srcloc util.cpp:89   - exists only inlined into func 11437
//   * Utils::DesconverteTipoPacote     srcloc util.cpp:329  - exists only inlined into func 11437
//   * Utils::DesconverteIdSistema      srcloc util.cpp:451  - exists only inlined into func 11437
//   * Utils::DesconverteSexo           srcloc util.cpp:483  - exists only inlined into func 11450
// (func 11437 = CConversorCabecalhoPacote::DoDesconverte, func 11450 = CConversorDadosCandidato::DoDesconverte.)
#include "comum/asn/util.h"

#include <array>
#include <format>

#include "api/util/cdate.h"
#include "api/util/ctime.h"
#include "comum/md/cideleitoral.h"

namespace comum::asn {

// wasm func 1080 - name inferred (unit u21 uses the same name). Observed executing: every CabecalhoEntidade written
// at start-up (eg.bin / vota.bin, through CConversorCabecalhoEntidade 11589, CConversorDadoCorrespondencia 11399,
// CConversorEstadoGeralVota 11390); also CConversorEntidadeBU (10273), CConversorCarga (10291) and
// CConversorHistoricoVotoImpresso (10276).
// DataHoraJE ::= GeneralString "YYYYMMDDThhmmss" (local time of the urna; in the web build the browser's clock).
ModuloTiposEleitorais::DataHoraJE Utils::ConverteDataHoraJE(const api::CDateTime& dataHora)
{
    const std::string data = api::CDate::Format(dataHora.GetDate(), "YYYYMMDD");     // api::CDate::Format (706)
    const std::string hora = api::CTime::Format(dataHora.GetTime(), "hhmmss");       // shared_f779 (CDateTime +8)
    return ModuloTiposEleitorais::DataHoraJE((data + "T" + hora).c_str());           // AbstractString(info @1913292)
}

// ---------------------------------------------------------------------------------------------------------------
// util.cpp:89 - inlined into func 11437. IDEleitoral ::= CHOICE { idProcessoEleitoral [1], idPleito [2],
// idEleicao [3] } -> md::CIDEleitoral {id, tipo 1|2|3}. The md factories validate the id (cideleitoral.cpp):
//   :21 CriaProcessoEleitoral / :29 CriaPleito / :37 CriaEleicao throw CUeComumMdError 8911 / 8912 / 8913
//   "ID inválido." when id >= 100000.
md::CIDEleitoral Utils::DesconverteIdEleitoral(const ModuloTiposEleitorais::IDEleitoral& id)
{
    switch (id.currentSelection()) {
    case 0:  return md::CIDEleitoral::CriaProcessoEleitoral(id.get_idProcessoEleitoral());
    case 1:  return md::CIDEleitoral::CriaPleito(id.get_idPleito());
    case 2:  return md::CIDEleitoral::CriaEleicao(id.get_idEleicao());
    default:
        throw CUeComumAsnError(EUeComumAsnError(7673), "Tipo de id inválido.");                      // line 89
    }
}

// util.cpp:329 - inlined into func 11437. Exact inverse of ConverteTipoPacote (util.cpp:231): the 33 ASN.1 values
// that md::ETipoPacote can express (validity bitmask 0x80DF_FFF3_F3D3 over value-1, table @498004).
md::ETipoPacote Utils::DesconverteTipoPacote(const ModuloTiposPacotes::TipoPacote& tipo)
{
    using T = ModuloTiposPacotes::TipoPacote;
    switch (tipo.asInt()) {
    case T::pacoteEleitores:                          return md::ETipoPacote(0);
    case T::pacoteMunicipiosZonas:                    return md::ETipoPacote(1);
    case T::pacoteImpedidos:                          return md::ETipoPacote(2);
    case T::pacoteCandidatosTRE:                      return md::ETipoPacote(3);
    case T::pacoteFotosCandidatosTRE:                 return md::ETipoPacote(4);
    case T::pacoteCorrespondencia:                    return md::ETipoPacote(5);
    case T::pacoteSecoes:                             return md::ETipoPacote(6);
    case T::pacoteNotificacaoPacote:                  return md::ETipoPacote(7);
    case T::pacoteConfiguracaoProcessoEleitoral:      return md::ETipoPacote(8);
    case T::pacoteValidacaoCandidatos:                return md::ETipoPacote(9);
    case T::pacoteAlteracaoCandidaturaTRE:            return md::ETipoPacote(10);
    case T::pacoteResultadoTotalizacao:               return md::ETipoPacote(11);
    case T::pacoteRespostaVersaoPacote:               return md::ETipoPacote(12);
    case T::pacoteJuntas:                             return md::ETipoPacote(13);
    case T::pacoteEleicoesSegundoTurno:               return md::ETipoPacote(14);
    case T::pacoteParametrosUrna:                     return md::ETipoPacote(15);
    case T::pacoteNotificacaoImportacao:              return md::ETipoPacote(16);
    case T::pacotePrestacaoContas:                    return md::ETipoPacote(17);
    case T::pacoteSolicitacaoEstadoSistema:           return md::ETipoPacote(18);
    case T::pacoteRespostaEstadoSistema:              return md::ETipoPacote(19);
    case T::pacoteSolicitacaoVersaoPacote:            return md::ETipoPacote(20);
    case T::pacoteProcessamentoResultadoUrnaCadastro: return md::ETipoPacote(21);
    case T::pacoteNotificacaoDisponibilidadeSistema:  return md::ETipoPacote(22);
    case T::pacoteCandidatosTSE:                      return md::ETipoPacote(23);
    case T::pacoteFotosCandidatosTSE:                 return md::ETipoPacote(24);
    case T::pacoteAlteracaoCandidaturaTSE:            return md::ETipoPacote(25);
    case T::pacoteSolicitacaoDadosCadastro:           return md::ETipoPacote(26);
    case T::pacoteDadosEleitor:                       return md::ETipoPacote(27);
    case T::pacoteDadosDocumentacaoCandidatura:       return md::ETipoPacote(28);
    case T::pacoteComplementosMunicipios:             return md::ETipoPacote(29);
    case T::pacoteEleicao:                            return md::ETipoPacote(30);
    case T::pacoteSituacoesEleicoes:                  return md::ETipoPacote(31);
    case T::pacoteTransferenciaTemporariaEleitores:   return md::ETipoPacote(32);
    }
    throw CUeComumAsnError(EUeComumAsnError(7679),
                           std::format("Tipo de pacote inválido: {}", tipo.asInt()));             // line 329
}

// util.cpp:451 - inlined into func 11437. Inverse of ConverteIdSistema (table @498244, mask 0x3FE7 over value-1):
//   configurador 1->1, intercad 2->2, candidaturas 3->3, gedai 6->6, urnaEletronica 7->7, simulador 8->8,
//   parametrizador 9->9, geradorDeBases 10->10, sistot 11->11, seweb 12->5, simon 13->4, padaUE 14->12.
ESistemaJE Utils::DesconverteIdSistema(const ModuloTiposEleitorais::Sistema& sistema)
{
    static constexpr std::array<int, 14> kTabela = {1, 2, 3, 0, 0, 6, 7, 8, 9, 10, 11, 5, 4, 12};
    const unsigned indice = static_cast<unsigned>(sistema.asInt()) - 1u;
    if (indice < kTabela.size() && kTabela[indice] != 0)
        return ESistemaJE(kTabela[indice]);
    throw CUeComumAsnError(EUeComumAsnError(7682),
                           std::format("Sistema inválido: {}", sistema.asInt()));                  // line 451
}

// util.cpp:483 - inlined into func 11450. CodigoSexo ::= ENUMERATED { sexoNaoInformado (0), sexoMasculino (2),
// sexoFeminino (4) } -> md::CSexo::ESexo 0 / 1 / 2.
md::CSexo::ESexo Utils::DesconverteSexo(ModuloTiposEleitorais::CodigoSexo::NamedNumber sexo)
{
    using S = ModuloTiposEleitorais::CodigoSexo;
    switch (sexo) {
    case S::sexoNaoInformado: return md::CSexo::ESexo(0);
    case S::sexoMasculino:    return md::CSexo::ESexo(1);
    case S::sexoFeminino:     return md::CSexo::ESexo(2);
    }
    throw CUeComumAsnError(EUeComumAsnError(7684), std::format("Sexo inválido: {}", static_cast<int>(sexo)));  // :483
}

} // namespace comum::asn
