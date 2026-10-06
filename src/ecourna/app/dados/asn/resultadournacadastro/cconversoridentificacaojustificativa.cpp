// ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversoridentificacaojustificativa.cpp  (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u11). Only DoDeconverte is in this unit.
//
// A "justificativa" is the form of a voter who is outside his electoral domicile and justifies his
// absence at this urna instead of voting (the mesário types the título and the birth year).
//
// RTTI: CConversorIdentificacaoJustificativa
//         : IConversorASN<ModuloResultadoUrnaCadastro::IdentificacaoJustificativa, CIdentificacaoJustificativa>
// vtable @1136564: [0] 174  [1] 144  [2] 9065 DoConverte (unit u40)  [3] 9064 DoDeconverte
//
//   IdentificacaoJustificativa ::= SEQUENCE { identificacaoEleitor RegistroIdentificacaoEleitor,
//                                             anoNascimentoEleitor INTEGER (0..9999) }
//   CIdentificacaoJustificativa (24 bytes): +0 CRegistroIdentificacaoEleitor (20), +20 ushort ano
//   constructor func 1875 ("shared_f1875"; also used by vota::CPedeAnoNascimento, CJustificadorDAO and
//   CGravadorRCSecao): copies the registro (two shared_ptr add-refs) and stores the year.
#include "ecourna/app/dados/asn/resultadournacadastro/cconversoridentificacaojustificativa.h"

namespace ecourna::app::dados::asn {

// wasm func 9064 (vtable slot 3)
CIdentificacaoJustificativa CConversorIdentificacaoJustificativa::DoDeconverte(const TEntidade& entidade) const
{
    const CConversorRegistroIdentificacaoEleitor conversorRegistro;   // vptr @1131924
    // func 9105 = IConversorASN<RegistroIdentificacaoEleitor>::Deconverte (validates, then func 9112)
    const CRegistroIdentificacaoEleitor registro = conversorRegistro.Deconverte(entidade.get_identificacaoEleitor());

    // CBaseType range check 0..9999 (the INTEGER is read as 16 bits: ((field1)+8) as ushort)
    return CIdentificacaoJustificativa(registro, TAnoNascimento(entidade.get_anoNascimentoEleitor()));
}

}  // namespace ecourna::app::dados::asn
