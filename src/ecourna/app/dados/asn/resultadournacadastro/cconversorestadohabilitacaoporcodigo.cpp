// ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorestadohabilitacaoporcodigo.cpp
// (original build path: /home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/app/dados/asn/resultadournacadastro/cconversorestadohabilitacaoporcodigo.cpp)
// Reconstructed from vota_web_wasm.wasm, unit u14.
//
//   EstadoHabilitacaoPorCodigo ::= SEQUENCE { situacaoReconhecimentoMesario (1..3),
//        identificacaoMesario RegistroIdentificacaoEleitor OPTIONAL }  <->  CEstadoHabilitacaoPorCodigo
#include "ecourna/app/dados/asn/cconversores.h"
#include "ecourna/app/dados/asn/resultadournacadastro/cconversorregistroidentificacaoeleitor.h"   // unit u40
#include "ecourna/app/dados/dadoserros.h"

namespace ecourna::app::dados::asn {

// wasm func 9074 (srcloc line 64): thunk into func 2306 (count 3): 0-based -> 1-based.
ModuloResultadoUrnaCadastro::SituacaoReconhecimentoMesario
CConversorEstadoHabilitacaoPorCodigo::ConverteSituacaoReconhecimentoMesario(
    CEstadoHabilitacaoPorCodigo::ESituacaoReconhecimentoMesario situacao) const
{
    if (static_cast<unsigned>(situacao) >= 3) {
        throw CAsnResultadoUrnaCadastroError(2643, "Situação de habilitação por código inválida.");   // line 64
    }
    return ModuloResultadoUrnaCadastro::SituacaoReconhecimentoMesario(
        static_cast<ModuloResultadoUrnaCadastro::SituacaoReconhecimentoMesario::NamedNumber>(situacao + 1));
}

// wasm func 9072 (srcloc line 85): thunk into the shared body func 6144: 1-based -> 0-based.
CEstadoHabilitacaoPorCodigo::ESituacaoReconhecimentoMesario
CConversorEstadoHabilitacaoPorCodigo::DeconverteSituacaoReconhecimentoMesario(
    ModuloResultadoUrnaCadastro::SituacaoReconhecimentoMesario situacao) const
{
    const unsigned valor = static_cast<unsigned>(situacao.asInt() - 1);
    if (valor >= 3) {
        throw CAsnResultadoUrnaCadastroError(2644, "Situação de habilitação por código inválida.");   // line 85
    }
    return static_cast<CEstadoHabilitacaoPorCodigo::ESituacaoReconhecimentoMesario>(valor);
}

// wasm func 9075 (vtable slot 2)
CConversorEstadoHabilitacaoPorCodigo::TEntidade CConversorEstadoHabilitacaoPorCodigo::DoConverte(const TDado& estado) const
{
    const CConversorRegistroIdentificacaoEleitor conversorRegistro;
    ModuloResultadoUrnaCadastro::EstadoHabilitacaoPorCodigo entidade;
    entidade.set_situacaoReconhecimentoMesario(ConverteSituacaoReconhecimentoMesario(estado.GetSituacao()));
    if (estado.PossuiIdentificacaoMesario()) {
        entidade.set_identificacaoMesario(conversorRegistro.Converte(estado.GetIdentificacaoMesario()));
    } else {
        entidade.omit_identificacaoMesario();
    }
    return entidade;
}

// wasm func 9073 (vtable slot 3; shown as "vf3" by the tool). The data-class constructors enforce
// "identification present <=> situation != NaoReconhecido" (codes 3270/3271).
CConversorEstadoHabilitacaoPorCodigo::TDado CConversorEstadoHabilitacaoPorCodigo::DoDeconverte(const TEntidade& estado) const
{
    const CConversorRegistroIdentificacaoEleitor conversorRegistro;
    const auto situacao = DeconverteSituacaoReconhecimentoMesario(estado.get_situacaoReconhecimentoMesario());
    if (estado.identificacaoMesario_isPresent()) {
        return CEstadoHabilitacaoPorCodigo(situacao, conversorRegistro.Deconverte(estado.get_identificacaoMesario()));   // func 5088
    }
    return CEstadoHabilitacaoPorCodigo(situacao);                                                                        // func 5090
}

} // namespace ecourna::app::dados::asn
