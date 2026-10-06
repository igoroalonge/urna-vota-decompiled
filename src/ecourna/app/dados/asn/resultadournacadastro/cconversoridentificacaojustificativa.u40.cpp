// FRAGMENT of ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversoridentificacaojustificativa.cpp
// (path inferred; class declared by unit u11). Reconstructed by unit u40 from vota_web_wasm.wasm.
#include "ecourna/app/dados/asn/resultadournacadastro/cconversoridentificacaojustificativa.h"

namespace ecourna::app::dados::asn {

// wasm func 9065 (vtable @1136564 slot 2)
//   IdentificacaoJustificativa ::= SEQUENCE { identificacaoEleitor RegistroIdentificacaoEleitor,
//                                             anoNascimentoEleitor INTEGER (0..9999) }
// A "justificativa" is the declaration of a voter who is away from his polling place and cannot vote; the
// urna records who justified (and the year of birth typed by the mesário).
ModuloResultadoUrnaCadastro::IdentificacaoJustificativa
CConversorIdentificacaoJustificativa::DoConverte(const CIdentificacaoJustificativa& dado) const
{
    const CConversorRegistroIdentificacaoEleitor conversorRegistro;                               // vptr @1131924
    ModuloResultadoUrnaCadastro::IdentificacaoJustificativa entidade;
    entidade.set_identificacaoEleitor(conversorRegistro.Converte(dado.m_identificacao));          // func 9113 -> 9114
    entidade.set_anoNascimentoEleitor(dado.m_anoNascimento);                                     // u16 at +20
    return entidade;
}

}  // namespace ecourna::app::dados::asn
