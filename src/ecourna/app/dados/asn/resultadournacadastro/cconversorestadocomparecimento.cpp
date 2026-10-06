// ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorestadocomparecimento.cpp
// (original build path: /home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/app/dados/asn/resultadournacadastro/cconversorestadocomparecimento.cpp)
// Reconstructed from vota_web_wasm.wasm, unit u14.
//
//   EstadoComparecimento ::= SEQUENCE { identificacaoEleitor RegistroIdentificacaoEleitor,
//        situacaoComparecimentoEleitor {faltou(1), naoVotou(2), votou(3), semCargoParaVotar(4)},
//        apresentacaoFoto [1] OPTIONAL, situacaoHabilitacaoAudio [2] OPTIONAL, habilitacaoBiometrica [3] OPTIONAL }
//   <-> CEstadoComparecimento
#include "ecourna/app/dados/asn/cconversores.h"
#include "ecourna/app/dados/asn/resultadournacadastro/cconversorregistroidentificacaoeleitor.h"   // unit u40
#include "ecourna/app/dados/dadoserros.h"

namespace ecourna::app::dados::asn {

namespace {
// switch tables emitted by the compiler (data @1135428 and @1135444)
constexpr int ASN_DE_SITUACAO[4] = {1, 4, 2, 3};   // C++ 0..3 -> faltou, semCargoParaVotar, naoVotou, votou
constexpr CEstadoComparecimento::ESituacaoComparecimento SITUACAO_DE_ASN[4] = {
    CEstadoComparecimento::Faltou, CEstadoComparecimento::NaoVotou,
    CEstadoComparecimento::Votou, CEstadoComparecimento::SemCargoParaVotar};
}

// wasm func 9085 (srcloc line 113). In the source a 4-case switch; compiled to a lookup table.
ModuloResultadoUrnaCadastro::SituacaoComparecimentoEleitor
CConversorEstadoComparecimento::ConverteEstadoComparecimento(TDado::ESituacaoComparecimento situacao) const
{
    if (static_cast<unsigned>(situacao) >= 4) {
        throw CAsnResultadoUrnaCadastroError(2639, "Situação de comparecimento inválida.");   // line 113
    }
    return ModuloResultadoUrnaCadastro::SituacaoComparecimentoEleitor(
        static_cast<ModuloResultadoUrnaCadastro::SituacaoComparecimentoEleitor::NamedNumber>(ASN_DE_SITUACAO[situacao]));
}

// wasm func 9080 (srcloc line 135)
CConversorEstadoComparecimento::TDado::ESituacaoComparecimento
CConversorEstadoComparecimento::DeconverteEstadoComparecimento(ModuloResultadoUrnaCadastro::SituacaoComparecimentoEleitor situacao) const
{
    const unsigned indice = static_cast<unsigned>(situacao.asInt() - 1);
    if (indice >= 4) {
        throw CAsnResultadoUrnaCadastroError(2640, "Situação de comparecimento inválida.");   // line 135
    }
    return SITUACAO_DE_ASN[indice];
}

// wasm func 9083 (srcloc line 152): thunk into func 2306 (count 3): 0-based -> 1-based.
ModuloResultadoUrnaCadastro::SituacaoHabilitacaoAudio
CConversorEstadoComparecimento::ConverteSituacaoHabilitacaoAudio(TDado::ESituacaoHabilitacaoAudio situacao) const
{
    if (static_cast<unsigned>(situacao) >= 3) {
        throw CAsnResultadoUrnaCadastroError(2641, "Situação de habilitação do áudio inválida.");   // line 152
    }
    return ModuloResultadoUrnaCadastro::SituacaoHabilitacaoAudio(
        static_cast<ModuloResultadoUrnaCadastro::SituacaoHabilitacaoAudio::NamedNumber>(situacao + 1));
}

// wasm func 9077 (srcloc line 172): thunk into func 6144: 1-based -> 0-based.
CConversorEstadoComparecimento::TDado::ESituacaoHabilitacaoAudio
CConversorEstadoComparecimento::DeconverteSituacaoHabilitacaoAudio(ModuloResultadoUrnaCadastro::SituacaoHabilitacaoAudio situacao) const
{
    const unsigned valor = static_cast<unsigned>(situacao.asInt() - 1);
    if (valor >= 3) {
        throw CAsnResultadoUrnaCadastroError(2642, "Situação de habilitação do áudio inválida.");   // line 172
    }
    return static_cast<TDado::ESituacaoHabilitacaoAudio>(valor);
}

// wasm func 9086 (vtable slot 2)
CConversorEstadoComparecimento::TEntidade CConversorEstadoComparecimento::DoConverte(const TDado& estado) const
{
    const CConversorHabilitacaoBiometrica conversorBiometria;
    const CConversorRegistroIdentificacaoEleitor conversorRegistro;
    const CConversorApresentacaoFotoEleitor conversorFoto;

    ModuloResultadoUrnaCadastro::EstadoComparecimento entidade;
    entidade.set_situacaoComparecimentoEleitor(ConverteEstadoComparecimento(estado.GetSituacao()));
    entidade.set_identificacaoEleitor(conversorRegistro.Converte(estado.GetIdentificacaoEleitor()));
    if (estado.PossuiApresentacaoFoto()) {
        entidade.set_apresentacaoFoto(conversorFoto.Converte(estado.GetApresentacaoFoto()));            // opt 0
    } else {
        entidade.omit_apresentacaoFoto();
    }
    if (estado.PossuiSituacaoHabilitacaoAudio()) {
        entidade.set_situacaoHabilitacaoAudio(ConverteSituacaoHabilitacaoAudio(estado.GetSituacaoHabilitacaoAudio()));   // opt 1
    } else {
        entidade.omit_situacaoHabilitacaoAudio();
    }
    if (estado.PossuiHabilitacaoBiometrica()) {
        entidade.set_habilitacaoBiometrica(conversorBiometria.Converte(estado.GetHabilitacaoBiometrica()));  // opt 2
    } else {
        entidade.omit_habilitacaoBiometrica();
    }
    return entidade;
}

// wasm func 9081 (vtable slot 3; srcloc lines 70, 75)
// Consistency rules: audio or biometric data require the photo data; photo or biometric data require
// the audio data. So the valid shapes are: nothing, photo+audio, photo+audio+biometrics.
CConversorEstadoComparecimento::TDado CConversorEstadoComparecimento::DoDeconverte(const TEntidade& estado) const
{
    const CConversorRegistroIdentificacaoEleitor conversorRegistro;
    const CConversorApresentacaoFotoEleitor conversorFoto;

    const auto identificacao = conversorRegistro.Deconverte(estado.get_identificacaoEleitor());
    const auto situacao = DeconverteEstadoComparecimento(estado.get_situacaoComparecimentoEleitor());
    const bool temAudio = estado.situacaoHabilitacaoAudio_isPresent();
    const bool temFoto = estado.apresentacaoFoto_isPresent();
    const bool temBiometria = estado.habilitacaoBiometrica_isPresent();

    if (!temFoto && (temAudio || temBiometria)) {
        throw CAsnResultadoUrnaCadastroError(
            2637, "Eleitor que votou não possui informações sobre a apresentacao de fotos do eleitor.");   // line 70
    }
    if (!temAudio && (temFoto || temBiometria)) {
        throw CAsnResultadoUrnaCadastroError(2638, "Eleitor que votou não possui informações sobre habilitação por áudio.");   // line 75
    }

    if (estado.habilitacaoBiometrica_isPresent()) {
        const CConversorHabilitacaoBiometrica conversorBiometria;
        const auto biometria = conversorBiometria.Deconverte(estado.get_habilitacaoBiometrica());
        const auto audio = DeconverteSituacaoHabilitacaoAudio(estado.get_situacaoHabilitacaoAudio());
        const auto foto = conversorFoto.Deconverte(estado.get_apresentacaoFoto());
        return CEstadoComparecimento(identificacao, situacao, foto, audio, biometria);   // func 2192
    }
    if (temAudio) {
        const auto audio = DeconverteSituacaoHabilitacaoAudio(estado.get_situacaoHabilitacaoAudio());
        const auto foto = conversorFoto.Deconverte(estado.get_apresentacaoFoto());
        return CEstadoComparecimento(identificacao, situacao, foto, audio);              // func 2193
    }
    return CEstadoComparecimento(identificacao, situacao);                               // func 2658
}

} // namespace ecourna::app::dados::asn
