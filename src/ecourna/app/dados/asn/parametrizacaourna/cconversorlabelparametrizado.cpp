// ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversorlabelparametrizado.cpp
// (original build path: /home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/app/dados/asn/parametrizacaourna/cconversorlabelparametrizado.cpp)
// Reconstructed from vota_web_wasm.wasm, unit u14.
//
//   LabelParametrizado ::= SEQUENCE { genero GeneroLabel {neutro(1), masculino(2), feminino(3)},
//        longo (1..20), curto (1..11), longoPlural (1..24), curtoPlural (1..13) }  <->  CLabelParametrizado
// The label texts are used on screens and reports and decide the grammatical gender of the words
// around them ("o município"/"a seção"). Values in the scenarios: Município (masculino),
// Zona Eleitoral/Zona (feminino), Seção Eleitoral/Seção (feminino), Partido (masculino).
#include "ecourna/app/dados/asn/cconversores.h"
#include "ecourna/app/dados/dadoserros.h"

namespace ecourna::app::dados::asn {

namespace {
const std::string MSG_GENERO_INVALIDO = "Gênero do label inválido.";   // @1912232 (name inferred)
}

// wasm func 9168 (srcloc line 55): thunk into the shared body func 6148 (count 3): 0-based -> 1-based.
ModuloParametrizacaoUrna::GeneroLabel::NamedNumber
CConversorLabelParametrizado::ConverterGeneroLabel(CLabelParametrizado::EGeneroLabel genero) const
{
    if (static_cast<unsigned>(genero) >= 3) {
        throw CAsnParametrizacaoUrnaError(2560, MSG_GENERO_INVALIDO);   // line 55
    }
    return static_cast<ModuloParametrizacaoUrna::GeneroLabel::NamedNumber>(genero + 1);
}

// inlined into func 9167 (srcloc lines 71, 75)
CLabelParametrizado::EGeneroLabel CConversorLabelParametrizado::DesconverterGeneroLabel(const TEntidade& label) const
{
    switch (label.get_genero().asInt()) {
    case ModuloParametrizacaoUrna::GeneroLabel::neutro:    return CLabelParametrizado::Neutro;
    case ModuloParametrizacaoUrna::GeneroLabel::masculino: return CLabelParametrizado::Masculino;
    case ModuloParametrizacaoUrna::GeneroLabel::feminino:  return CLabelParametrizado::Feminino;
    case -1:   // ? explicit "invalid" enumerator
        throw CAsnParametrizacaoUrnaError(2561, MSG_GENERO_INVALIDO);   // line 71
    }
    throw CAsnParametrizacaoUrnaError(2562, MSG_GENERO_INVALIDO);       // line 75
}

// wasm func 9169 (vtable slot 2). Field order as in the binary: curto, curtoPlural, longo,
// longoPlural, then the gender.
CConversorLabelParametrizado::TEntidade CConversorLabelParametrizado::DoConverte(const TDado& label) const
{
    ModuloParametrizacaoUrna::LabelParametrizado entidade;
    entidade.set_curto(label.GetCurto());
    entidade.set_curtoPlural(label.GetCurtoPlural());
    entidade.set_longo(label.GetLongo());
    entidade.set_longoPlural(label.GetLongoPlural());
    entidade.set_genero(ConverterGeneroLabel(label.GetGenero()));
    return entidade;
}

// wasm func 9167 (vtable slot 3; the tool named it after the inlined DesconverterGeneroLabel)
CConversorLabelParametrizado::TDado CConversorLabelParametrizado::DoDeconverte(const TEntidade& label) const
{
    return CLabelParametrizado(DesconverterGeneroLabel(label), label.get_longo(), label.get_curto(),
                               label.get_longoPlural(), label.get_curtoPlural());
}

} // namespace ecourna::app::dados::asn
