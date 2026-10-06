// uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversorestadogeralvota.cpp
// Reconstructed from vota_web_wasm.wasm (unit u03).
//
// Persistent state of the VOTA application (the election-day state machine), file trab1|trab2/vota.bin:
//   EstadoGeralVota ::= SEQUENCE {
//     estadoVota EstadoVota, estadoEncerramento EstadoEncerramento, qtdBU INTEGER (0..999),
//     comparecimento INTEGER (0..99999), qtdJustificativa INTEGER (0..99999), treinamentoEleitor BOOLEAN,
//     numViasImpressasRelatorios NumViasImpressasRelatorios, tecladoTestadoPosConversao BOOLEAN,
//     urnaIdGerouZeresima [1] INTEGER OPTIONAL, dhIniAquisicao [2] DataHoraJE OPTIONAL,
//     dhFimAquisicao [3] ..., dhUltimoVoto [4] ..., dhEmissao [5] ... }
//   EstadoVota ::= ENUMERATED { inicial(0), gerabasedinamica(1), aguardahorazeresima(2), gerarze(3),
//     zeresimagerada(4), zeresimaimpressa(5), registromesarioinicial(6), votar(7), fimaquisicaovotos(8),
//     registromesariofinal(9), gerarbu(10), gerarrelatorios(11), imprimirbu(12), gravarresultados(13),
//     copiaresultadosmr(14), encerrada(15), exibealertadesligamento(16), ultimoid(17) }
//   EstadoEncerramento ::= ENUMERATED { inicial(0), imprimirobrigatoriabu(1), retirarmr(2),
//     fimdostrabalhos(3), ultimoid(4) }
// md EEstadoVota: char '1' + v for v = 0..15 ('1'..'@'); 'A' is refused. exibealertadesligamento(16) exists in
// the file format but "must not be used" by this version. md EEstadoEncerramento: '1'..'4'; '5' refused.
// Observed at run time: after votaInit, vota.bin = {votar, inicial, qtdBU 0, comparecimento 0, ...,
// treinamentoEleitor TRUE, dhIniAquisicao "2026...T..."}; it is NOT rewritten after a vote in the web build.
#include "comum/dados/asn/estadoaplicacao/cconversorestadogeralvota.h"

#include <format>
#include <optional>

#include "comum/asn/util.h"
#include "comum/dados/asn/estadoaplicacao/cconversornumviasimpressasrelatorios.h"

namespace comum::asn {

// Inlined into wasm func 11390 (srcloc lines 152, 156)
ModuloEstadoGeralVota::EstadoVota CConversorEstadoGeralVota::ConverteEstadoVota(const EEstadoVota estado) const
{
    const char c = static_cast<char>(estado);
    if (c >= '1' && c <= '@') {   // 16 states, compiled as a br_table on c - '1'
        return ModuloEstadoGeralVota::EstadoVota(c - '1');
    }
    if (c == 'A') {               // md-only value after encerrada (probably "ultimo id")
        throw CDadosError(7929, std::format("Valor inválido para EEstadoVota: {}", estado));   // line 152
    }
    throw CDadosError(7930, std::format("Valor inválido para EEstadoVota: {}", estado));       // line 156
}

// Inlined into wasm func 11391 (srcloc lines 198, 203, 207)
EEstadoVota CConversorEstadoGeralVota::DesconverteEstadoVota(const ModuloEstadoGeralVota::EstadoVota& estado) const
{
    const int valor = estado.asInt();
    if (valor >= ModuloEstadoGeralVota::EstadoVota::inicial && valor <= ModuloEstadoGeralVota::EstadoVota::encerrada) {
        return static_cast<EEstadoVota>('1' + valor);
    }
    switch (valor) {
    case ModuloEstadoGeralVota::EstadoVota::exibealertadesligamento:
        throw CDadosError(7931, std::format("Estado não deve ser usado: {}", estado));             // line 198
    case -1:   // ? generated "invalid" enumerator
    case ModuloEstadoGeralVota::EstadoVota::ultimoid:
        throw CDadosError(7932, std::format("Valor inválido para EstadoVota: {}", estado));         // line 203
    }
    throw CDadosError(7933, std::format("Valor inválido para EstadoVota: {}", estado));             // line 207
}

// Inlined into wasm func 11390 (srcloc lines 225, 229)
ModuloEstadoGeralVota::EstadoEncerramento
CConversorEstadoGeralVota::ConverteEstadoEncerramento(const EEstadoEncerramento estado) const
{
    switch (static_cast<char>(estado)) {
    case '1': return ModuloEstadoGeralVota::EstadoEncerramento::inicial;
    case '2': return ModuloEstadoGeralVota::EstadoEncerramento::imprimirobrigatoriabu;
    case '3': return ModuloEstadoGeralVota::EstadoEncerramento::retirarmr;
    case '4': return ModuloEstadoGeralVota::EstadoEncerramento::fimdostrabalhos;
    case '5':   // md "ultimo id"
        throw CDadosError(7934, std::format("Valor não suportado para EEstadoEncerramento: {}", estado));   // line 225
    }
    throw CDadosError(7935, std::format("Valor inválido para EEstadoEncerramento: {}", estado));           // line 229
}

// Inlined into wasm func 11391 (srcloc lines 248, 252)
EEstadoEncerramento
CConversorEstadoGeralVota::DesconverteEstadoEncerramento(const ModuloEstadoGeralVota::EstadoEncerramento& estado) const
{
    switch (estado.asInt()) {
    case ModuloEstadoGeralVota::EstadoEncerramento::inicial:               return EEstadoEncerramento{'1'};
    case ModuloEstadoGeralVota::EstadoEncerramento::imprimirobrigatoriabu: return EEstadoEncerramento{'2'};
    case ModuloEstadoGeralVota::EstadoEncerramento::retirarmr:             return EEstadoEncerramento{'3'};
    case ModuloEstadoGeralVota::EstadoEncerramento::fimdostrabalhos:       return EEstadoEncerramento{'4'};
    case -1:   // ? generated "invalid" enumerator
    case ModuloEstadoGeralVota::EstadoEncerramento::ultimoid:
        throw CDadosError(7936, std::format("Valor não suportado para EstadoEncerramento: {}", estado));   // line 248
    }
    throw CDadosError(7937, std::format("Valor inválido para EstadoEncerramento: {}", estado));           // line 252
}

// wasm func 11390 (vtable slot 2; named after the inlined ConverteEstadoVota)
ModuloEstadoGeralVota::EstadoGeralVota CConversorEstadoGeralVota::DoConverte(const TDado& estado) const
{
    ModuloEstadoGeralVota::EstadoGeralVota entidade;
    entidade.set_estadoVota(ConverteEstadoVota(estado.GetEstado()));
    entidade.set_estadoEncerramento(ConverteEstadoEncerramento(estado.GetEncerramento()));
    entidade.set_qtdBU(estado.GetQtdBU());

    if (const auto& urna = estado.GetUrnaIdGerouZeresima(); urna.has_value()) {
        entidade.set_urnaIdGerouZeresima(*urna);                                 // includeOptionalField(0, 8)
    } else {
        entidade.omit_urnaIdGerouZeresima();
    }
    if (const auto& dh = estado.GetDhIniAquisicao(); dh.has_value()) {
        entidade.set_dhIniAquisicao(Utils::ConverteDataHoraJE(*dh));             // func 1080; (1, 9)
    } else {
        entidade.omit_dhIniAquisicao();
    }
    if (const auto& dh = estado.GetDhFimAquisicao(); dh.has_value()) {
        entidade.set_dhFimAquisicao(Utils::ConverteDataHoraJE(*dh));             // (2, 10)
    } else {
        entidade.omit_dhFimAquisicao();
    }
    entidade.set_comparecimento(estado.GetComparecimento());
    entidade.set_qtdJustificativa(estado.GetQtdJustificativa());
    if (const auto& dh = estado.GetDhUltimoVoto(); dh.has_value()) {
        entidade.set_dhUltimoVoto(Utils::ConverteDataHoraJE(*dh));               // (3, 11)
    } else {
        entidade.omit_dhUltimoVoto();
    }
    entidade.set_treinamentoEleitor(estado.GetTreinamentoEleitor());
    entidade.set_numViasImpressasRelatorios(CConversorNumViasImpressasRelatorios().Converte(estado.GetNumVias()));
    if (const auto& dh = estado.GetDhEmissao(); dh.has_value()) {
        entidade.set_dhEmissao(Utils::ConverteDataHoraJE(*dh));                  // (4, 12)
    } else {
        entidade.omit_dhEmissao();
    }
    entidade.set_tecladoTestadoPosConversao(estado.GetTecladoTestadoPosConversao());
    return entidade;
}

// wasm func 11391 (vtable slot 3; named after the inlined DesconverteEstadoVota)
md::estadoaplicacao::CEstadoGeralVota CConversorEstadoGeralVota::DoDesconverte(const TEntidade& estado) const
{
    const EEstadoVota estadoVota = DesconverteEstadoVota(estado.get_estadoVota());
    const EEstadoEncerramento encerramento = DesconverteEstadoEncerramento(estado.get_estadoEncerramento());
    const auto qtdBU = static_cast<uebyte>(estado.get_qtdBU());   // INTEGER (0..999) truncated to 8 bits

    std::optional<uedword> urnaIdGerouZeresima;
    if (estado.urnaIdGerouZeresima_isPresent()) {
        urnaIdGerouZeresima = static_cast<uedword>(estado.get_urnaIdGerouZeresima());
    }
    std::optional<api::CDateTime> dhIniAquisicao, dhFimAquisicao, dhUltimoVoto, dhEmissao;
    if (estado.dhIniAquisicao_isPresent()) {
        dhIniAquisicao = Utils::DesconverteDataHoraJE(estado.get_dhIniAquisicao());   // func 1713, util.cpp:41
    }
    if (estado.dhFimAquisicao_isPresent()) {
        dhFimAquisicao = Utils::DesconverteDataHoraJE(estado.get_dhFimAquisicao());
    }
    const auto comparecimento = static_cast<ueword>(estado.get_comparecimento());       // INTEGER (0..99999) -> 16 bits
    const auto qtdJustificativa = static_cast<ueword>(estado.get_qtdJustificativa());   // idem
    if (estado.dhUltimoVoto_isPresent()) {
        dhUltimoVoto = Utils::DesconverteDataHoraJE(estado.get_dhUltimoVoto());
    }
    const bool treinamentoEleitor = estado.get_treinamentoEleitor();
    const auto numVias = CConversorNumViasImpressasRelatorios().Desconverte(estado.get_numViasImpressasRelatorios());
    if (estado.dhEmissao_isPresent()) {
        dhEmissao = Utils::DesconverteDataHoraJE(estado.get_dhEmissao());
    }
    const bool tecladoTestado = estado.get_tecladoTestadoPosConversao();

    return md::estadoaplicacao::CEstadoGeralVota(estadoVota, encerramento, qtdBU, urnaIdGerouZeresima,
                                                 dhIniAquisicao, dhFimAquisicao, comparecimento,
                                                 qtdJustificativa, dhUltimoVoto, treinamentoEleitor, numVias,
                                                 dhEmissao, tecladoTestado);
}

} // namespace comum::asn

// ----------------------------------------------------------------------------------------------------------
// Emitted in this TU: wasm func 5628 — md::estadoaplicacao::CEstadoGeralVota::CEstadoGeralVota(13 arguments,
// in the order used above), a member-wise constructor without validation (layout in the header). Also used by
// the simulator fixture mock_f10067. Real home: md/estadoaplicacao/cestadogeralvota.{h,cpp} (path inferred).
