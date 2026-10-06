// uenux2/src/app/comum/gravadores/md/cenvelopegenerico.cpp  --  FRAGMENT written by unit u36
// (file owned by u23; the layout comment at the top of cenvelopegenerico.cpp applies).
// Reconstructed from vota_web_wasm.wasm.
//
// The two md::CEnvelopeGenerico constructors WITHOUT a CUrna (the third one, with CUrna, is comum_f5860).
// They build the envelope ("envelope genérico") that wraps every result file sent to the TSE - in
// particular the BU file <fase><pleito><uf><mun><zona><secao>-bu.dat written by CGravadorBU::GravaResultado
// (11629) - and the envelope decoded back by CConversorEnvelopeGenerico::DoDesconverte (10270).
//
// Layout (224 bytes): +0 CCabecalhoEntidade (20) | +20 fase | +24 optional<CUrna> (flag +160) |
//   +164 município | +168 zona | +172 optional<TLocalID> (value +172, flag +176) | +180 seção | +184 tipo |
//   +188 optional<CSeguranca> (flag +204; CSeguranca = {ueword +188, vector<uebyte> +192}) |
//   +208 vector<uebyte> conteúdo | +220 EUrnaTipo
// EUrnaTipo ('1'..'4', see cconversorurna.cpp): '1' seção, '2' contingência, '3' contingência de seção,
// '4' contingência encerrando seção. ValidaCriacao (3822) rejects every other value: its test is
// `(tipoUrna - '5') <= -5` compiled as i32.le_u, i.e. an unsigned range test (not `tipoUrna <= '0'`).
#include "comum/gravadores/md/cenvelopegenerico.h"

#include <optional>
#include <vector>

namespace comum::md {

namespace {
// Written as a helper for readability: in the binary this code sits inline at the end of both constructors
// (the original may simply repeat it, or call an inlined helper).                     // ? name invented
// A pure contingency urna ('2') has no local de votação: whatever local was given is dropped. Note that
// ValidaCriacao() runs BEFORE the reset, so an out-of-range local (>= 10000) still throws 8681
// "Número de local inválido" for such an urna.
void DescartaLocalSeContingencia(std::optional<TLocalID>& local, EUrnaTipo tipoUrna)
{
    // compiled as: (unsigned)(tipoUrna - '1') > 3 || tipoUrna == '2'; after ValidaCriacao only '2' gets here
    const bool temLocal = tipoUrna == EUrnaTipo{'1'} || tipoUrna == EUrnaTipo{'3'} || tipoUrna == EUrnaTipo{'4'};
    if (!temLocal && local.has_value())
        local.reset();
}
}  // namespace

// wasm func 5861                                                            // name from the class + callers
// Plain envelope: the content is stored as is (no "seguranca"). Callers: CGravadorBU::GravaResultado (11629,
// BU not encrypted: parameter criptografarBU off - the simulator's case), CConversorEnvelopeGenerico::
// DoDesconverte (10270, decoded envelope without urna and without seguranca).
CEnvelopeGenerico::CEnvelopeGenerico(const CCabecalhoEntidade& cabecalho, const EUrnaFase fase,
                                     const TMunicipioID municipio, const TZonaID zona,
                                     const std::optional<TLocalID>& local, const TSecaoID secao, const Tipo tipo,
                                     const std::vector<uebyte>& conteudo, const EUrnaTipo tipoUrna)
    : m_cabecalho(cabecalho)
    , m_fase(fase)
    , m_urna()                                   // disengaged
    , m_municipio(municipio)
    , m_zona(zona)
    , m_local(local)
    , m_secao(secao)
    , m_tipo(tipo)
    , m_seguranca()                              // disengaged
    , m_conteudo(conteudo)
    , m_tipoUrna(tipoUrna)
{
    ValidaCriacao();                             // wasm 3822 (cenvelopegenerico.cpp:127..156)
    DescartaLocalSeContingencia(m_local, m_tipoUrna);
}

// wasm func 3821                                                            // name from the class + callers
// Encrypted envelope: `seguranca` describes the CEPESC encryption and `conteudo` is the ciphertext.
// Callers: CGravadorBU::GravaResultado (11629, parameter criptografarBU on: CSeguranca(0, 1, cifrado)),
// CControlaArmazenamentoDeImagens::LeChavePublica (2725, fingerprint images, tipoUrna '1') and
// CConversorEnvelopeGenerico::DoDesconverte (10270).
CEnvelopeGenerico::CEnvelopeGenerico(const CCabecalhoEntidade& cabecalho, const EUrnaFase fase,
                                     const TMunicipioID municipio, const TZonaID zona,
                                     const std::optional<TLocalID>& local, const TSecaoID secao, const Tipo tipo,
                                     const CSeguranca& seguranca, const std::vector<uebyte>& conteudo,
                                     const EUrnaTipo tipoUrna)
    : m_cabecalho(cabecalho)
    , m_fase(fase)
    , m_urna()
    , m_municipio(municipio)
    , m_zona(zona)
    , m_local(local)
    , m_secao(secao)
    , m_tipo(tipo)
    , m_seguranca(seguranca)                     // engaged: ueword +188 copied, vector copied
    , m_conteudo(conteudo)
    , m_tipoUrna(tipoUrna)
{
    ValidaCriacao();
    DescartaLocalSeContingencia(m_local, m_tipoUrna);
}

}  // namespace comum::md
