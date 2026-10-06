// uenux2/src/app/comum/gravadores/md/cenvelopegenerico.cpp  (+ .h, layout from funcs 3822 / 5860 / 10271)
// Reconstructed from vota_web_wasm.wasm (unit u23).
//
// md::CEnvelopeGenerico = C++ model of ModuloEnvelopeGenerico::EntidadeEnvelopeGenerico, the wrapper written
// around every result file that goes to the TSE (bu.dat, rdv.dat, imgbu.dat, imgze.dat, ...):
//   { cabecalho, fase, urna OPTIONAL, identificacao (seção or contingência), tipoEnvelope,
//     seguranca OPTIONAL, conteudo OCTET STRING }
//
// Layout (224 bytes):
//   +0   CCabecalhoEntidade m_cabecalho (20)      +20  EUrnaFase m_fase ('1'..'3')
//   +24  std::optional<CUrna> m_urna (flag +160)   +164 TMunicipioID m_municipio   +168 TZonaID m_zona
//   +172 std::optional<TLocalID> m_local (flag +176)                               +180 TSecaoID m_secao
//   +184 Tipo m_tipo                               +188 std::optional<CSeguranca> m_seguranca
//   +208 std::vector<uebyte> m_conteudo            +220 EUrnaTipo m_tipoUrna ('1'..'4')
// Constructors (outside this file's unit list): comum_f5860 (with CUrna, unit u23 cconversorenvelopegenerico),
// comum_f5861 (plain), comum_f3821 (with CSeguranca = encrypted content).
#include "comum/gravadores/md/cenvelopegenerico.h"

#include "comum/gravadores/iresultado.h"

namespace comum::md {

// Tipo (0-based) -> ASN.1 TipoEnvelope: 0 BoletimUrna -> 1, 1 RDV -> 2, 2 BoletimUrnaImpresso -> 4,
// 3 ImagemBiometria -> 5 (?), 4 ZeresimaImpressa -> 6   (tables @546600 and in DoConverte, func 10271)

// wasm func 3822 (srcloc cenvelopegenerico.cpp:127..156)
void CEnvelopeGenerico::ValidaCriacao() const
{
    if (m_fase == EUrnaFase('0') || static_cast<int>(m_fase) >= '4')
        throw CUeComumGravadoresError(8678, "Fase inválida");                          // :127
    if (m_municipio >= 100000)
        throw CUeComumGravadoresError(8679, "Código de município inválido");           // :130
    if (m_zona >= 10000)
        throw CUeComumGravadoresError(8680, "Número de zona inválido");                // :135
    if (m_local.has_value() && *m_local >= 10000)
        throw CUeComumGravadoresError(8681, "Número de local inválido");               // :140
    if (static_cast<int>(m_tipoUrna) <= '0')                                           // compiled as (t - 53) <= -5
        throw CUeComumGravadoresError(8682, "Tipo de urna inválido");                  // :145
    if (m_secao >= 10000)
        throw CUeComumGravadoresError(8683, "Número de seção inválido");               // :151
    if (static_cast<int>(m_tipo) >= 5)
        throw CUeComumGravadoresError(8684, "Tipo de envelope inválido");              // :156
}

// Inlined into CConversorEnvelopeGenerico::DoConverte (func 10271): srcloc :96 / :106 / :117.
const CUrna& CEnvelopeGenerico::GetUrna() const
{
    if (!m_urna) throw CUeComumGravadoresError(8675, "Não há informação de urna");     // :96
    return *m_urna;
}
// Also inlined into 10271, same pattern:
TLocalID CEnvelopeGenerico::GetLocal() const
{
    if (!m_local) throw CUeComumGravadoresError(8676, "Não há informação de local");            // :106
    return *m_local;
}
const CSeguranca& CEnvelopeGenerico::GetSeguranca() const
{
    if (!m_seguranca) throw CUeComumGravadoresError(8677, "Não há informação de criptografia"); // :117
    return *m_seguranca;
}

}  // namespace comum::md
