// uenux2/src/app/comum/dados/asn/eleitor/cconversorentidadeeleitores.cpp
// Reconstructed from vota_web_wasm.wasm (unit u03).
//
// The voter roll of the section (cadastro de eleitores), file <...>-el.dat (and -tte.dat):
//   EntidadeEleitores ::= SEQUENCE { cabecalho, identificacao IdentificacaoSecaoEleitoral,
//                                    seguranca [1] Seguranca OPTIONAL, eleitores SEQUENCE OF EleitorSequencia OPTIONAL }
//   EleitorSequencia ::= SEQUENCE { sequencial INTEGER (0..9999), eleitor EleitorUrna,
//                                   tipoTransferenciaTemporaria [1] TipoTransferenciaTemporaria OPTIONAL }
//   EleitorUrna ::= SEQUENCE { identificacaoEleitor SEQUENCE OF IdentificadorEleitor, nome, nomeSocial [1] OPTIONAL,
//                              nomeMae, dataNascimento DataJE, situacao, domicilio [2] DomicilioEleitoral OPTIONAL,
//                              necessidade NecessidadeEspecial, biometria BiometriaEleitorCifrada OPTIONAL }
// The file is decoded by the streaming partial decoder; the biometria blobs are skipped and only their file
// offsets (collected by CVisitanteEleitor) are stored in each md::CEleitor.
// Not read: nomeMae, situacao, DomicilioEleitoral other than uf/codigoMunicipio, the identificacao's "local".
#include "comum/dados/asn/eleitor/cconversorentidadeeleitores.h"

#include <algorithm>
#include <format>
#include <string>

#include "comum/dados/asn/ccabecalhoentidade.h"
#include "comum/dados/asn/cconversorseguranca.h"
#include "comum/dados/md/cdataje.h"
#include "comum/dados/md/cvalidadoridentidade.h"

namespace comum::asn {

namespace {
constexpr std::size_t TAM_MAX_NOME = 40;   // names are cut to 40 characters (ASN.1 allows 70)   // name inferred
} // namespace

// Inlined into wasm func 11411 (srcloc lines 49, 76)
std::vector<md::CEleitorIdentidade> GetInscricoesEleitor(ModuloEleitores::EleitorSequencia eleitor)
{
    const auto identificadores = eleitor.get_eleitor().get_identificacaoEleitor();   // SEQUENCE OF copied again
    if (identificadores.empty()) {
        throw CDadosError(7899, "Inscrição não encontrada");   // line 49
    }

    std::vector<md::CEleitorIdentidade> identidades;
    for (const auto* identificador : identificadores) {
        const std::string numero = identificador->getSelection<ASN1::AbstractString>().getValue();
        switch (identificador->currentSelection()) {
        case 0:  identidades.push_back(md::CValidadorIdentidade::Valida(numero, 1)); break;   // numeroInscricao
        case 1:  identidades.push_back(md::CValidadorIdentidade::Valida(numero, 2)); break;   // numeroCPF
        default: identidades.push_back(md::CValidadorIdentidade::Valida(numero, 3)); break;   // identificacaoLivre
        }
    }

    // Only ADJACENT duplicates are detected (std::adjacent_find on {numero, tipo}; the list is not sorted).
    const auto duplicada = std::ranges::adjacent_find(identidades);
    if (duplicada != identidades.end()) {
        // Argument order from the packed format-arg types: 419 = 3 | 13 << 5 -> arg0 int (tipo, element +12),
        // arg1 string_view (numero).
        throw CDadosError(7900, std::format("Identidade duplicada ({}) para o eleitor ({})",
                                            duplicada->GetTipo(), duplicada->GetNumero()));   // line 76
    }
    return identidades;
}

// Inlined into wasm func 11411 (srcloc line 38)
md::CEleitor::ENecessidadeEspecial CConversorEntidadeEleitores::ConverteNecessidadeEspecial(ModuloEleitores::EleitorUrna eleitor)
{
    const auto necessidade = eleitor.get_necessidade();   // the whole EleitorUrna was copied for this call
    switch (necessidade.asInt()) {
    case ModuloEleitores::NecessidadeEspecial::semNecessidade: return md::CEleitor::ENecessidadeEspecial{0};
    case ModuloEleitores::NecessidadeEspecial::necessitaAudio: return md::CEleitor::ENecessidadeEspecial{1};
    }
    throw CDadosError(7898, std::format("Tipo de necessidade inválido: {}", necessidade.asInt()));   // line 38
}

// Inlined into wasm func 11411 (srcloc line 210)
md::CTransferenciaTemporaria::ETransferenciaTemporaria
CConversorEntidadeEleitores::DesconverteTipoTransferenciaTemporaria(const ModuloEleitores::EleitorSequencia& eleitor) const
{
    using E = md::CTransferenciaTemporaria::ETransferenciaTemporaria;   // numeric values; names unknown
    if (!eleitor.tipoTransferenciaTemporaria_isPresent()) {
        return E{0};   // no TTE
    }
    const int tipo = eleitor.get_tipoTransferenciaTemporaria().asInt();
    switch (tipo) {
    case 1:  // votoEmTransito
    case 3:  // servidorEmServico
    case 5:  // eleitorConvocado
    case 6:  // justicaEleitoral
        return E{1};   // these four collapse into ONE md value (a "de fato" transfer to another section)
    case 2:  return E{2};   // presoProvisorio
    case 4:  return E{3};   // acessibilidade
    case 7:  return E{4};   // oficio
    case 8:  return E{5};   // indigenaQuilombola
    case 9:  return E{6};   // situacaoRua
    case -1:                // ? generated "invalid" enumerator
        throw CDadosError(7901, std::format("Tipo de transferência inválido: {}", tipo));   // line 210
    }
    return E{0};   // any other value is silently treated as "no transfer"
}

// Inlined into wasm func 11411 (srcloc line 236; and the inlined md ctor ctransferenciatemporaria.cpp:35)
md::CTransferenciaTemporaria
CConversorEntidadeEleitores::DesconverteTransferenciaTemporaria(const ModuloEleitores::EleitorSequencia& eleitor) const
{
    const auto tipo = DesconverteTipoTransferenciaTemporaria(eleitor);
    if (tipo == md::CTransferenciaTemporaria::ETransferenciaTemporaria{0}) {
        return md::CTransferenciaTemporaria();   // {0, "", 0}
    }
    if (!eleitor.get_eleitor().domicilio_isPresent()) {
        throw CDadosError(7902, "Eleitor de voto em trânsito sem informações de domicílio eleitoral.");   // line 236
    }
    const auto& domicilio = eleitor.get_eleitor().get_domicilio();
    // The md constructor refuses tipo == 0: CDadosError(8070, "Construtor exclusivo para transferências de fato.").
    return md::CTransferenciaTemporaria(tipo, domicilio.get_uf(), domicilio.get_codigoMunicipio());
}

// wasm func 11411 (vtable slot 2; the tool named it "vf2"; runtime edges show it as "GetInscricoesEleitor")
md::CEntidadeEleitores CConversorEntidadeEleitores::DoDesconverte(const TEntidade& entidade, const TVisitor& visitante) const
{
    const auto cabecalho = CConversorCabecalhoEntidade().Desconverte(entidade.get_cabecalho());
    const auto& identificacao = entidade.get_identificacao();
    const TMunicipioID municipio = identificacao.get_municipioZona().get_municipio();
    const TZonaID zona = identificacao.get_municipioZona().get_zona();
    const TSecaoID secao = identificacao.get_secao();

    const CConversorSeguranca conversorSeguranca;
    std::vector<uebyte> chaveArquivo;   // Seguranca.idArquivoChave, copied into every voter that has biometrics
    if (entidade.seguranca_isPresent()) {
        chaveArquivo = conversorSeguranca.Desconverte(entidade.get_seguranca()).GetChave();
    }

    std::vector<md::CEleitor> eleitores;
    if (entidade.eleitores_isPresent()) {
        const CVisitanteEleitor::TMapaPosicoes posicoes = visitante.GetPosicoes();   // full copy of the map
        for (const auto* sequencia : entidade.get_eleitores()) {
            const TSequencialEleitor sequencial = sequencia->get_sequencial();
            const auto identidades = GetInscricoesEleitor(*sequencia);                     // copy of the SEQUENCE
            const auto necessidade = ConverteNecessidadeEspecial(sequencia->get_eleitor()); // copy of the SEQUENCE

            std::string nome = sequencia->get_eleitor().get_nome();
            nome.resize(std::min(nome.size(), TAM_MAX_NOME));
            std::string nomeSocial;
            if (sequencia->get_eleitor().nomeSocial_isPresent()) {
                nomeSocial = sequencia->get_eleitor().get_nomeSocial();
                nomeSocial.resize(std::min(nomeSocial.size(), TAM_MAX_NOME));
            }
            const auto transferencia = DesconverteTransferenciaTemporaria(*sequencia);
            // md::CDataJE ctor (inlined, cdataje.cpp:26): exactly 8 ASCII digits, else
            // CDadosError(8002, std::format("DataJE com formato inválido: {}", data)).
            const md::CDataJE dataNascimento(sequencia->get_eleitor().get_dataNascimento());

            // First identity of the voter that the visitor saw next to a biometria.
            auto posicao = posicoes.end();
            for (const auto& identidade : identidades) {
                if (posicao = posicoes.find(identidade); posicao != posicoes.end()) {
                    break;
                }
            }

            if (sequencia->get_eleitor().biometria_isPresent() && posicao != posicoes.end()) {
                // func 5668
                eleitores.emplace_back(sequencial, secao, identidades, necessidade, nome, nomeSocial, transferencia,
                                       dataNascimento, chaveArquivo, posicao->second);
            } else {
                // func 5666 (a voter whose biometria offset was not recorded silently loses it)
                eleitores.emplace_back(sequencial, secao, identidades, necessidade, nome, nomeSocial, transferencia,
                                       dataNascimento);
            }
        }
    }

    if (entidade.seguranca_isPresent()) {
        // The Seguranca record is converted a second time for the result.
        return md::CEntidadeEleitores(cabecalho, municipio, zona, secao,
                                      conversorSeguranca.Desconverte(entidade.get_seguranca()), eleitores);
    }
    return md::CEntidadeEleitores(cabecalho, municipio, zona, secao, std::nullopt, eleitores);   // vector copied (api_f946)
}

} // namespace comum::asn

// ----------------------------------------------------------------------------------------------------------
// Emitted in this TU (listed in the unit):
//  * wasm func 5666 — md::CEleitor::CEleitor(sequencial, secao, identidades, necessidade, nome, nomeSocial,
//    transferencia, dataNascimento): member-wise copy, chaveArquivo empty, posicaoBiometria = nullopt, then
//    md::CEleitor::ValidaCriacao() (func 5667). Observed executing (the training voter has no biometria).
//  * wasm func 5668 — the same plus chaveArquivo (vector copy) and posicaoBiometria = {begin, end}.
//    Real home of both: md/eleitor/celeitor.{h,cpp} (inline constructors).
//  * wasm func 5700 — std::vector<md::CEleitor>::__swap_out_circular_buffer (reallocation of the 104-byte
//    elements during emplace_back). Library helper.
