// ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorresultadournacadastro.cpp
// (original build path: /home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/app/dados/asn/resultadournacadastro/cconversorresultadournacadastro.cpp)
// Reconstructed from vota_web_wasm.wasm, unit u14.
//
//   EntidadeResultadoUrnaCadastro ::= SEQUENCE { cabecalho CabecalhoEntidade, fase Fase,
//        versaoVotacao GeneralString, situacao SituacaoArquivo {arquivoFinal(1), arquivoParcial(2)},
//        infoDadosComparecimento CHOICE { dadosComparecimento [0], dadosComparecimentoCifrado [1] } }
//   <-> CResultadoUrnaCadastro
// The only user in this build is comum::CGravadorRCSecao::GravaResultado (func 11616, misnamed
// "LeChavePublica" by the tool), which writes the section's attendance file at the end of voting,
// and api::CFileASN::CodeObjectFunction<EntidadeResultadoUrnaCadastro> (func 5366).
#include "ecourna/app/dados/asn/cconversores.h"
#include "ecourna/app/dados/asn/resultadournacadastro/cconversordadoscomparecimento.h"   // unit u40
#include "ecourna/app/dados/dadoserros.h"

namespace ecourna::app::dados::asn {

// wasm func 9055 (srcloc line 102)
ModuloTiposEcoUrna::SituacaoArquivo
CConversorResultadoUrnaCadastro::ConverteSituacaoArquivo(CResultadoUrnaCadastro::ESituacaoArquivo situacao) const
{
    switch (situacao) {
    case CResultadoUrnaCadastro::ArquivoFinal:   return ModuloTiposEcoUrna::SituacaoArquivo(ModuloTiposEcoUrna::SituacaoArquivo::arquivoFinal);
    case CResultadoUrnaCadastro::ArquivoParcial: return ModuloTiposEcoUrna::SituacaoArquivo(ModuloTiposEcoUrna::SituacaoArquivo::arquivoParcial);
    }
    throw CAsnResultadoUrnaCadastroError(2651, "Situação do arquivo inválida.");   // line 102
}

// wasm func 9051 (srcloc lines 116, 120). (Wording differs from line 102: "Situação arquivo".)
CResultadoUrnaCadastro::ESituacaoArquivo
CConversorResultadoUrnaCadastro::DeconverteSituacaoArquivo(ModuloTiposEcoUrna::SituacaoArquivo situacao) const
{
    switch (situacao.asInt()) {
    case ModuloTiposEcoUrna::SituacaoArquivo::arquivoFinal:   return CResultadoUrnaCadastro::ArquivoFinal;
    case ModuloTiposEcoUrna::SituacaoArquivo::arquivoParcial: return CResultadoUrnaCadastro::ArquivoParcial;
    case -1:   // ? explicit "invalid" enumerator
        throw CAsnResultadoUrnaCadastroError(2652, "Situação arquivo inválida.");   // line 116
    }
    throw CAsnResultadoUrnaCadastroError(2653, "Situação arquivo inválida.");       // line 120
}

// wasm func 9056 (vtable slot 2; srcloc line 48)
CConversorResultadoUrnaCadastro::TEntidade CConversorResultadoUrnaCadastro::DoConverte(const TDado& resultado) const
{
    const CConversorCabecalhoEntidade conversorCabecalho;
    const CConversorDadosComparecimento conversorDados;
    const CConversorDadosComparecimentoCifrado conversorCifrado;
    const CConversorFase conversorFase;

    ModuloResultadoUrnaCadastro::EntidadeResultadoUrnaCadastro entidade;
    entidade.set_cabecalho(conversorCabecalho.Converte(resultado.GetCabecalho()));
    entidade.set_fase(conversorFase.Converte(resultado.GetFase()));
    entidade.set_situacao(ConverteSituacaoArquivo(resultado.GetSituacao()));
    entidade.set_versaoVotacao(resultado.GetVersaoVotacao());

    if (resultado.PossuiDadosComparecimento()) {
        const auto dados = resultado.GetDadosComparecimento();          // copied (func 5846), then freed (1162)
        entidade.ref_infoDadosComparecimento().select_dadosComparecimento() = conversorDados.Converte(dados);
    } else if (resultado.PossuiDadosComparecimentoCifrado()) {
        const auto dados = resultado.GetDadosComparecimentoCifrado();   // copied (func 9054 + vector), freed (3487)
        entidade.ref_infoDadosComparecimento().select_dadosComparecimentoCifrado() = conversorCifrado.Converte(dados);
    } else {
        // BUG in the original: the error object is constructed and immediately destroyed; there is
        // no `throw` (the binary calls the CError constructor func 1143 on a stack object, never
        // __cxa_allocate_exception/__cxa_throw, then runs its destructor and returns normally).
        // The CHOICE stays unselected and the error only surfaces later as a generic
        // "entity left in an invalid state" from IConversorASN::Converte.
        CAsnResultadoUrnaCadastroError(2658, "Classe de dados de comparecimento mal formada.");   // line 48
    }
    return entidade;
}

// wasm func 9052 (vtable slot 3; srcloc line 88)
CConversorResultadoUrnaCadastro::TDado CConversorResultadoUrnaCadastro::DoDeconverte(const TEntidade& resultado) const
{
    const CConversorCabecalhoEntidade conversorCabecalho;
    const CConversorDadosComparecimento conversorDados;
    const CConversorDadosComparecimentoCifrado conversorCifrado;
    const CConversorFase conversorFase;

    const auto cabecalho = conversorCabecalho.Deconverte(resultado.get_cabecalho());
    const auto fase = conversorFase.Deconverte(resultado.get_fase());
    const std::string versaoVotacao = resultado.get_versaoVotacao();
    const auto situacao = DeconverteSituacaoArquivo(resultado.get_situacao());

    const auto& info = resultado.get_infoDadosComparecimento();
    switch (info.currentSelection()) {
    case 0: {   // dadosComparecimento
        const auto dados = conversorDados.Deconverte(info.get_dadosComparecimento());
        return CResultadoUrnaCadastro(cabecalho, fase, versaoVotacao, situacao,
                                      std::optional<CDadosComparecimento>(dados));          // func 3483
    }
    case 1: {   // dadosComparecimentoCifrado
        const auto dados = conversorCifrado.Deconverte(info.get_dadosComparecimentoCifrado());
        return CResultadoUrnaCadastro(cabecalho, fase, versaoVotacao, situacao,
                                      std::optional<CDadosComparecimentoCifrado>(dados));   // func 3482
    }
    }
    throw CAsnResultadoUrnaCadastroError(2660, "Não foram definidos os dados de comparecimento.");   // line 88
}

} // namespace ecourna::app::dados::asn
