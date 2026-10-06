// ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorhabilitacaobiometrica.cpp
// (original build path: /home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/app/dados/asn/resultadournacadastro/cconversorhabilitacaobiometrica.cpp)
// Reconstructed from vota_web_wasm.wasm, unit u14.
//
//   HabilitacaoBiometrica ::= SEQUENCE { tentativasHabilitacaoBiometrica INTEGER, dedoHabilitado TipoDedo,
//        erroLeituraBiometria ErroLeituraBiometria, habilitacaoPorCodigo [1] EstadoHabilitacaoPorCodigo OPTIONAL,
//        ultimoScore [2] INTEGER (0..999) OPTIONAL }  <->  CHabilitacaoBiometrica
#include "ecourna/app/dados/asn/cconversores.h"
#include "ecourna/app/dados/dadoserros.h"

namespace ecourna::app::dados::asn {

// wasm func 9069 (srcloc line 111). ("biomoetria" [sic])
ModuloResultadoUrnaCadastro::ErroLeituraBiometria
CConversorHabilitacaoBiometrica::ConverteErroLeituraBiometria(TErroLeituraBiometria erro) const
{
    if (static_cast<unsigned>(erro) >= 13) {
        throw CAsnResultadoUrnaCadastroError(2647, "Tipo de erro de biomoetria inválido.");   // line 111
    }
    return ModuloResultadoUrnaCadastro::ErroLeituraBiometria(
        static_cast<ModuloResultadoUrnaCadastro::ErroLeituraBiometria::NamedNumber>(static_cast<int>(erro)));
}

// inlined into func 9066 (srcloc line 150). CErroLeituraBiometria (func 2668) re-checks the range.
TErroLeituraBiometria
CConversorHabilitacaoBiometrica::DeconverteErroLeituraBiometria(const ModuloResultadoUrnaCadastro::ErroLeituraBiometria& erro) const
{
    if (static_cast<unsigned>(erro.asInt()) >= 13) {
        throw CAsnResultadoUrnaCadastroError(2648, "Tipo de erro de biomoetria inválido.");   // line 150
    }
    return CErroLeituraBiometria(static_cast<EErroLeituraBiometria>(erro.asInt()));
}

// wasm func 9070 (srcloc line 182). Note the different wording from line 216 ("Tipo dedo" vs "Tipo de dedo").
ModuloTiposEleitorais::TipoDedo CConversorHabilitacaoBiometrica::ConverteTipoDedo(const CDedo::ETipoDedo& dedo) const
{
    if (static_cast<unsigned>(dedo) >= 11) {
        throw CAsnResultadoUrnaCadastroError(2649, "Tipo dedo inválido.");      // line 182
    }
    return ModuloTiposEleitorais::TipoDedo(static_cast<ModuloTiposEleitorais::TipoDedo::NamedNumber>(dedo));
}

// inlined into func 9066 (srcloc line 216)
CDedo::ETipoDedo CConversorHabilitacaoBiometrica::DeconverteTipoDedo(const ModuloTiposEleitorais::TipoDedo& dedo) const
{
    if (static_cast<unsigned>(dedo.asInt()) >= 11) {
        throw CAsnResultadoUrnaCadastroError(2650, "Tipo de dedo inválido.");   // line 216
    }
    return static_cast<CDedo::ETipoDedo>(dedo.asInt());
}

// wasm func 9071 (vtable slot 2)
// A score of 0 is treated as "no score" and the OPTIONAL field is omitted (so a genuine score of
// 0 does not survive a round trip).
CConversorHabilitacaoBiometrica::TEntidade CConversorHabilitacaoBiometrica::DoConverte(const TDado& hab) const
{
    ModuloResultadoUrnaCadastro::HabilitacaoBiometrica entidade;
    entidade.set_tentativasHabilitacaoBiometrica(hab.GetTentativas());
    entidade.set_dedoHabilitado(ConverteTipoDedo(hab.GetDedo()));
    entidade.set_erroLeituraBiometria(ConverteErroLeituraBiometria(hab.GetErroLeitura()));
    if (static_cast<unsigned short>(hab.GetUltimoScore()) != 0) {
        entidade.set_ultimoScore(static_cast<unsigned short>(hab.GetUltimoScore()));   // includeOptionalField(1, 4)
    } else {
        entidade.omit_ultimoScore();
    }
    if (hab.PossuiEstadoHabilitacaoPorCodigo()) {
        const CConversorEstadoHabilitacaoPorCodigo conversor;
        entidade.set_habilitacaoPorCodigo(conversor.Converte(hab.GetEstadoHabilitacaoPorCodigo()));   // includeOptionalField(0, 3)
    } else {
        entidade.omit_habilitacaoPorCodigo();
    }
    return entidade;
}

// wasm func 9066 (vtable slot 3; srcloc lines 53, 61 + the inlined helpers 150, 216)
// Consistency rule: if a finger was identified the voter was enabled by fingerprint, so there must
// be neither a code-enabling record nor a reading error; if no finger was identified the voter must
// have been enabled by code.
CConversorHabilitacaoBiometrica::TDado CConversorHabilitacaoBiometrica::DoDeconverte(const TEntidade& hab) const
{
    const int tentativas = hab.get_tentativasHabilitacaoBiometrica();
    const auto dedo = DeconverteTipoDedo(hab.get_dedoHabilitado());
    const auto erro = DeconverteErroLeituraBiometria(hab.get_erroLeituraBiometria());
    const TScoreHabilitacao ultimoScore =
        hab.ultimoScore_isPresent() ? TScoreHabilitacao(hab.get_ultimoScore()) : TScoreHabilitacao(0);   // func 2850

    if (hab.get_dedoHabilitado().asInt() != CDedo::NaoIdentificado) {
        if (hab.habilitacaoPorCodigo_isPresent() || hab.get_erroLeituraBiometria().asInt() != ErroBioPrimeiro) {
            throw CAsnResultadoUrnaCadastroError(2645, "Estrutura de dados mal formada.");                          // line 53
        }
        return CHabilitacaoBiometrica(tentativas, ultimoScore, dedo, erro);                                          // func 5087
    }
    if (!hab.habilitacaoPorCodigo_isPresent()) {
        throw CAsnResultadoUrnaCadastroError(2646, "Informação habilitacaoPorCodigo era esperada neste ponto.");     // line 61
    }
    const CConversorEstadoHabilitacaoPorCodigo conversor;
    return CHabilitacaoBiometrica(tentativas, ultimoScore, dedo, erro,
                                  conversor.Deconverte(hab.get_habilitacaoPorCodigo()));                             // func 2657
}

} // namespace ecourna::app::dados::asn
