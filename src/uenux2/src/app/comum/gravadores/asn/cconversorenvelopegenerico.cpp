// uenux2/src/app/comum/gravadores/asn/cconversorenvelopegenerico.cpp
// Reconstructed from vota_web_wasm.wasm (unit u23).
//
// comum::asn::CConversorEnvelopeGenerico : IConversorASN<ModuloEnvelopeGenerico::EntidadeEnvelopeGenerico,
// md::CEnvelopeGenerico>. vtable: [2] DoConverte = func 10271, [3] DoDesconverte = func 10270.
// Used by CGravadorRDV / IGravadorEnvelope (writing) and by the code that re-reads envelopes (DoDesconverte).
#include "comum/gravadores/asn/cconversorenvelopegenerico.h"

#include <format>

#include "comum/asn/cconversorcabecalhoentidade.h"
#include "comum/asn/util.h"
#include "comum/gravadores/asn/cconversorurna.h"
#include "comum/gravadores/iresultado.h"
#include "comum/gravadores/md/cenvelopegenerico.h"
#include "ModuloEnvelopeGenerico.h"

namespace comum::asn {

// ConverteTipoEnvelope (srcloc :168): md Tipo 0..4 -> TipoEnvelope {1 envelopeBoletimUrna, 2 envelopeRegistroDigitalVoto,
// 4 envelopeBoletimUrnaImpresso, 5 envelopeImagemBiometria (?), 6 envelopeZeresimaImpressa}   (table @546580)
ModuloEnvelopeGenerico::TipoEnvelope::NamedNumber CConversorEnvelopeGenerico::ConverteTipoEnvelope(md::CEnvelopeGenerico::Tipo tipo)
{
    static constexpr int tabela[5] = {1, 2, 4, 5, 6};
    if (static_cast<int>(tipo) >= 5)
        throw CUeComumGravadoresError(8614, std::format("Tipo de envelope inválido: {}", static_cast<int>(tipo)));   // :168
    return ModuloEnvelopeGenerico::TipoEnvelope::NamedNumber(tabela[static_cast<int>(tipo)]);
}

// DesconverteTipoEnvelope (srcloc :191): values 1, 2, 4, 5, 6 accepted (bitmask 0b111011 over value-1)
md::CEnvelopeGenerico::Tipo CConversorEnvelopeGenerico::DesconverteTipoEnvelope(
    ModuloEnvelopeGenerico::TipoEnvelope::NamedNumber tipo)
{
    static constexpr int tabela[6] = {0, 1, 0, 2, 3, 4};                               // @546600
    const int indice = static_cast<int>(tipo) - 1;
    if (!(indice >= 0 && indice <= 5 && ((0b111011 >> indice) & 1)))
        throw CUeComumGravadoresError(8615, std::format("Tipo de envelope inválido: {}", static_cast<int>(tipo)));   // :191
    return md::CEnvelopeGenerico::Tipo(tabela[indice]);
}

// wasm func 10271 (vtable slot 2; srcloc :72, plus the inlined CEnvelopeGenerico getters :96/:106/:117)
CConversorEnvelopeGenerico::TEntidade CConversorEnvelopeGenerico::DoConverte(const TDado& envelope) const
{
    TEntidade entidade;
    entidade.set_cabecalho(CConversorCabecalhoEntidade().Converte(envelope.GetCabecalho()));
    entidade.set_fase(Utils::ConverteFase(envelope.GetFase()));
    if (envelope.PossuiUrna())
        entidade.set_urna(CConversorUrna().Converte(envelope.GetUrna()));             // "Não há informação de urna" :96
    else
        entidade.omit_urna();

    // Identification: seção 0 means a contingency urna (município + zona only).
    if (envelope.GetSecao() == 0) {
        entidade.identificacao().set_identificacaoContingencia({envelope.GetMunicipio(), envelope.GetZona()});
    } else {
        switch (static_cast<int>(envelope.GetTipoUrna())) {
        case '0':
            throw CUeComumGravadoresError(8611, std::format("Tipo inválido de urna: {}",
                                                            static_cast<int>(envelope.GetTipoUrna())));   // :72
        case '2':                                                                        // contingência
            entidade.identificacao().set_identificacaoContingencia({envelope.GetMunicipio(), envelope.GetZona()});
            break;
        case '1': case '3': case '4':
            entidade.identificacao().set_identificacaoSecao(
                {{envelope.GetMunicipio(), envelope.GetZona()}, envelope.GetLocal() /* :106 */, envelope.GetSecao()});
            break;
        default:                                                                         // left unset (?)
            break;
        }
    }

    entidade.set_tipoEnvelope(ConverteTipoEnvelope(envelope.GetTipo()));
    if (envelope.PossuiSeguranca())
        entidade.set_seguranca(CConversorSeguranca().Converte(envelope.GetSeguranca()));   // :117
    else
        entidade.omit_seguranca();
    entidade.set_conteudo(envelope.GetConteudo());
    return entidade;
}

// wasm func 10270 (vtable slot 3; srcloc :126, :139, :191)
md::CEnvelopeGenerico CConversorEnvelopeGenerico::DoDesconverte(const TEntidade& entidade) const
{
    const md::CCabecalhoEntidade cabecalho = CConversorCabecalhoEntidade().Desconverte(entidade.get_cabecalho());
    const EUrnaFase fase = Utils::DesconverteFase(entidade.get_fase());
    std::unique_ptr<md::CUrna> urna;
    if (entidade.hasOptionalField(0))                                                   // urna
        urna = std::make_unique<md::CUrna>(CConversorUrna().Desconverte(entidade.get_urna()));   // iconversorasn.h:71

    TMunicipioID municipio; TZonaID zona; std::optional<TLocalID> local; TSecaoID secao = 0;
    md::EUrnaTipo tipoUrna;
    const auto& id = entidade.get_identificacao();
    switch (id.choiceIndex()) {
    case 0:                                                                              // identificacaoSecao
        municipio = id.secao().municipioZona().municipio(); zona = id.secao().municipioZona().zona();
        local = id.secao().local(); secao = id.secao().secao();
        tipoUrna = md::EUrnaTipo('1');
        break;
    case 1:                                                                              // identificacaoContingencia
        municipio = id.contingencia().municipio(); zona = id.contingencia().zona();
        tipoUrna = md::EUrnaTipo('2');
        break;
    default:
        throw CUeComumGravadoresError(8612, "Entidade inválida. Tipo de identificação incorreto");   // :126
    }
    const auto tipo = DesconverteTipoEnvelope(entidade.get_tipoEnvelope());

    std::unique_ptr<md::CSeguranca> seguranca;
    if (entidade.hasOptionalField(1))
        seguranca = std::make_unique<md::CSeguranca>(CConversorSeguranca().Desconverte(entidade.get_seguranca()));
    const std::vector<uebyte> conteudo = entidade.get_conteudo();

    if (urna && seguranca)
        throw CUeComumGravadoresError(8613, "Entidade inválida com informação de urna e criptografia");   // :139
    if (urna)
        return md::CEnvelopeGenerico(cabecalho, fase, *urna, municipio, zona, local, secao, tipo, conteudo);   // func 5860
    if (seguranca)
        return md::CEnvelopeGenerico(cabecalho, fase, municipio, zona, local, secao, tipo, *seguranca, conteudo,
                                     tipoUrna);                                                            // comum_f3821
    return md::CEnvelopeGenerico(cabecalho, fase, municipio, zona, local, secao, tipo, conteudo, tipoUrna);  // comum_f5861
}

}  // namespace comum::asn

namespace comum::md {
// wasm func 5860: CEnvelopeGenerico constructor with an urna (the urna's tipoUrna is copied to +220), then
// ValidaCriacao (func 3822). Callers: DoDesconverte (10270) and IGravadorEnvelope::GravaResultado (11626).
// (Its trailing code tests tipoUrna in {'1','3','4'} and the local flag without effect: a dead assert.)   // ?
CEnvelopeGenerico::CEnvelopeGenerico(const CCabecalhoEntidade& cabecalho, EUrnaFase fase, const CUrna& urna,
                                     TMunicipioID municipio, TZonaID zona, std::optional<TLocalID> local,
                                     TSecaoID secao, Tipo tipo, const std::vector<uebyte>& conteudo)
    : m_cabecalho(cabecalho), m_fase(fase), m_urna(urna), m_municipio(municipio), m_zona(zona), m_local(local),
      m_secao(secao), m_tipo(tipo), m_conteudo(conteudo), m_tipoUrna(urna.GetTipoUrna())
{
    ValidaCriacao();
}
}  // namespace comum::md
