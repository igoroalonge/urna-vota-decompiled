// Reconstructed from vota_web_wasm.wasm (unit u04). Original: uenux2/src/app/comum/dados/celeitordetalhe.cpp
//
// See ccargos.cpp for the error types. CErroAsn = CBaseError<comum::EUeComumAsnError> (typeinfo
// @1528108), CErroIo = CBaseError<api::EUeIoError> (typeinfo @1528172).
#include "celeitordetalhe.h"

#include <filesystem>
#include <format>

#include "cconfiguracaoeleicao.h"
#include "../asn/legravaentidade.h"                         // (anonymous)::LeEntidadeBiometria<>
#include "asn/eleitor/cconversorbiometriaeleitorcifrada.h"
#include "../cpath.h"
#include "../nomearquivo/cnomearquivo.h"

namespace comum {

using CErroDados = ecourna::api::exception::CBaseError<EUeComumDadosError>;

// celeitordetalhe.cpp:49 - only exists inlined into CEleitores::LoadEleitorDetalhe (wasm 5764).
CEleitorDetalhe::CEleitorDetalhe(const md::CEleitorDecorator& eleitor, const md::CEleitorDinamico& dinamico,
                                 const md::ETipoImpedimento impedimentoP1,
                                 const md::ETipoImpedimento impedimentoP2)
    : m_eleitor(eleitor)
    , m_dinamico(dinamico)
    , m_impedimentoP1(impedimentoP1)
    , m_impedimentoP2(impedimentoP2)
{
    // The dynamic row (SQLite) must belong to the same voter as the static record (roll file).
    if (m_eleitor.GetIdentidadePorTipo(m_eleitor.GetTipoIdentificadorPrincipal()) != dinamico.GetTitulo())
        throw CErroDados(EUeComumDadosError{7829}, "CEleitorDetalhe - identidades divergentes");
}

// Variant without dynamic data (optional left empty) - the path taken by LoadEleitorDetalhe when it
// receives a null CEleitorDinamico*.                                                     // ?
CEleitorDetalhe::CEleitorDetalhe(const md::CEleitorDecorator& eleitor,
                                 const md::ETipoImpedimento impedimentoP1,
                                 const md::ETipoImpedimento impedimentoP2)
    : m_eleitor(eleitor), m_dinamico(std::nullopt), m_impedimentoP1(impedimentoP1), m_impedimentoP2(impedimentoP2)
{
}

// wasm func 1271 - curated name (medium); the function passes its own name to the check.
const md::CEleitorDinamico& CEleitorDetalhe::GetDinamico() const
{
    ConfereDadosDinamicos("GetDinamico");
    return *m_dinamico;
}

md::CEleitorDinamico& CEleitorDetalhe::GetDinamico()
{
    ConfereDadosDinamicos("GetDinamico");
    return *m_dinamico;
}

// wasm func 3770 (srcloc line 224)
void CEleitorDetalhe::ConfereDadosDinamicos(const std::string& funcao) const
{
    if (!m_dinamico.has_value())
        throw CErroDados(EUeComumDadosError{7832}, std::format("{}: Dados dinâmicos não carregados", funcao));
}

// Only exists inlined into CEleitores::MarcaEleitorFoiHabilitado (wasm 2825, after its checks
// "Item inexistente" / "Dados dinâmicos não carregados" / "Eleitor já votou"). Records how the poll
// worker enabled the voter; the row is later written to eleitor_dinamico by the synchronisation code.
void CEleitorDetalhe::MarcaHabilitado(const CEleitorDadosHabilitacao& dados)
{
    ConfereDadosDinamicos("MarcaHabilitado");
    md::CEleitorDinamico& din = *m_dinamico;
    const auto& bio = dados.GetDadosBiometricos();          // md::CEleitorDadosHabilitacaoBiometrica, +24

    // wasm: br_table [case0, case1, case2, default -> common tail] on dados +16.
    switch (dados.GetTipoHabilitacao()) {                   // +16
    case md::ETipoHabilitacao::SEM_BIOMETRIA:
        din.SetEstadoComparecimento(md::EEstadoComparecimento::NAO_VOTOU);
        din.SetTipoHabilitacao(md::ETipoHabilitacao::SEM_BIOMETRIA);
        din.SetDedo(0);
        din.SetTentativas(0);
        din.SetErroDecifrarBiometria(0);                    // score left untouched
        break;
    case md::ETipoHabilitacao::BIOMETRIA:
        din.SetEstadoComparecimento(md::EEstadoComparecimento::NAO_VOTOU);
        din.SetTipoHabilitacao(md::ETipoHabilitacao::BIOMETRIA);
        din.SetDedo(bio.GetDedo());
        din.SetScore(bio.GetScore());
        din.SetTentativas(bio.GetTentativas());
        din.SetErroDecifrarBiometria(0);
        break;
    case md::ETipoHabilitacao::CODIGO_MESARIO:
        din.SetErroDecifrarBiometria(bio.GetErroDecifrar());
        din.SetTentativas(bio.GetTentativas());
        din.SetDedo(0);
        din.SetEstadoComparecimento(md::EEstadoComparecimento::NAO_VOTOU);
        din.SetTipoHabilitacao(md::ETipoHabilitacao::CODIGO_MESARIO);
        if (bio.TemTituloMesario()) {                       // bio +28 (dados +52)
            // celeitordadoshabilitacaobiometrica.cpp:49: GetTituloMesario() throws 8067
            // "Eleitor não habilitado manualmente" when empty (cannot happen on this path)
            din.SetTituloMesario(bio.GetTituloMesario());
        } else {
            din.ResetTituloMesario();
        }
        break;
    default:
        // Any other value (>= 3) changes NOTHING of the enabling state: estado, tipo, dedo, score,
        // tentativas, erro and título do mesário keep their previous values (e.g. estado stays
        // FALTOU). Only the three assignments below run.
        break;
    }
    din.SetTipoAtivacaoAudio(dados.GetTipoAtivacaoAudio());  // +20 -> dinâmico +48
    din.SetIdentidadeHabilitacao(dados.GetIdentidade());      // +0  -> dinâmico +16
    din.SetApresentacaoFoto(dados.GetApresentacaoFoto());     // +56 -> dinâmico +76
}

// wasm func 1937 (srcloc lines 94 and 101), 5175 bytes: LeEntidadeBiometria (legravaentidade.h:119/125),
// CFileASN decoding (cfileasn.h:135/143) and IConversorBiometriaASN (iconversorbiometriaasn.h:62)
// are inlined.
//
// The voter roll file keeps, for each voter, an encrypted ModuloEleitores::BiometriaEleitorCifrada
// blob; the decorator only stores where it is (offset, size) plus a byte vector used by the
// decryption. The biometrics are therefore read lazily, when the fingerprint reader needs them.
md::CBiometriaEleitor CEleitorDetalhe::GetBiometria() const
{
    if (!m_eleitor.TemBiometria())                                         // decorator +100
        throw CErroDados(EUeComumDadosError{7830}, "Eleitor corrente não tem biometria.");

    // Voters with no temporary transfer (0) or with TTE "acessibilidade" (4) are in the section's
    // own roll (<fase><pe><uf><mun><zona><secao>-el.dat); the others are in the -tte.dat roll.
    const auto& id = CConfiguracaoEleicao::GetInst().GetIdentificacaoSecao();   // config +620
    std::string suffix;
    switch (m_eleitor.GetTipoTransferenciaTemporaria()) {                   // decorator +48
    case 0:
    case 4:  suffix = "el";  break;                                          // api_f3745
    default: suffix = "tte"; break;                                          // api_f5727
    }
    // CNomeArquivo (api_f3772): FormataFase + FormataNumero(pe,5) + FormataUF + FormataNumero(mun,5)
    //   + FormataNumero(zona,4) + FormataNumero(secao,4) + "-" + suffix + "." + "dat"
    const std::string arquivo =
        (CPath::Estatico() /                                                 // wasm_entry_f762 "estatico/"
         CNomeArquivo::Monta(id.fase, id.processo, id.uf, id.municipio, id.zona,
                             m_eleitor.GetSecao() /* decorator +4 */, suffix, "dat")).string();

    if (!std::filesystem::exists(arquivo))
        throw CErroDados(EUeComumDadosError{7831}, std::format("Arquivo {} não existe", arquivo));

    // celeitor.cpp:79 (inlined): md::CEleitor::GetBiometria() throws 8063
    // "Não há informação de biometria" when the optional is empty.
    const auto& localizacao = m_eleitor.GetBiometria();
    const md::CParametroBiometria parametro{                                 // name inferred
        m_eleitor.GetIdentidadePorTipo(m_eleitor.GetTipoIdentificadorPrincipal()),
        localizacao.GetDados(),      // vector<uebyte> (decorator +80) - salt/key info      // ?
        localizacao.GetOffset(),     // decorator +92
        localizacao.GetTamanho()};   // decorator +96

    // LeEntidadeBiometria<asn::CConversorBiometriaEleitorCifrada>(arquivo, parametro, ...):
    //   CFile f(arquivo, "rb");
    //   if (!f.Seek(offset, SEEK_SET)) throw CErroAsn(7669,
    //        "CLeitorASN::{} - não foi possível fazer o seek em {}", "LeBiometriaEleitor(<id>)", arquivo);
    //   vector<uebyte> buf(tamanho);
    //   if (f.RawRead(buf.data(), tamanho) != tamanho) throw CErroAsn(7670,
    //        "CLeitorASN::{} - não foi possível ler a entidade de {}", ...);
    //   BER-decode ModuloEleitores::BiometriaEleitorCifrada ("Conteúdo não foi decodificado para {}: {}"
    //        5953 / "Conteúdo inválido para {}: {}" 5954, CErroIo);
    //   check isValid/isStrictlyValid ("Entidade está inválida: {}" 7658, CErroAsn);
    //   return CConversorBiometriaEleitorCifrada{}.Desconverte(entidade, parametro);  (vtable slot 3:
    //        decrypts "conteudo" with the "salt" and a key derived from the voter's identity)
    return asn::LeEntidadeBiometria<asn::CConversorBiometriaEleitorCifrada>(
        arquivo, parametro, std::format("LeBiometriaEleitor({})", parametro.identidade.GetNumero()));
}

}  // namespace comum
