// ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversordadoscomparecimento.h   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u11).
#pragma once

#include "ecourna/api/asn/iconversorasn.hpp"
#include "ecourna/app/dados/resultadournacadastro/cdadoscomparecimento.h"
#include "ModuloResultadoUrnaCadastro.h"

namespace ecourna::app::dados::asn {

// RTTI: CConversorDadosComparecimento
//         : IConversorASN<ModuloResultadoUrnaCadastro::DadosComparecimento, CDadosComparecimento>
// vtable @1132868: [0] 174  [1] 144  [2] 9102 DoConverte  [3] 9095 DoDeconverte
//
//   DadosComparecimento ::= SEQUENCE {
//       justificativas               SEQUENCE OF IdentificacaoJustificativa,
//       identificacaoComparecimento  ComparecimentoSecao,
//       mesariosAbertura         [1] SEQUENCE OF ComparecimentoMesario OPTIONAL,
//       mesariosEncerramento     [2] SEQUENCE OF ComparecimentoMesario OPTIONAL }
//
//   CDadosComparecimento (72 bytes, layout from 9102):
//     +0  std::vector<CIdentificacaoJustificativa>                     justificativas   (24-byte items)
//     +12 CComparecimentoSecao                                          secao            (28 bytes)
//     +40 std::optional<std::vector<CComparecimentoMesario>>           mesariosAbertura (flag +52; 40-byte items)
//     +56 std::optional<std::vector<CComparecimentoMesario>>           mesariosEncerramento (flag +68)
//   GetMesariosAbertura() (func 9024) / GetMesariosEncerramento() (func 9023) are srcloc-named and return
//   const TVectorComparecimentoMesario& (= std::vector<CComparecimentoMesario>), throwing when the optional
//   is empty; the other getters and the "has value" tests are inlined (names inferred).
//   Constructor func 3485 (4 parameters, all BY VALUE).
class CConversorDadosComparecimento
    : public api::asn::IConversorASN<ModuloResultadoUrnaCadastro::DadosComparecimento, CDadosComparecimento>
{
protected:
    TEntidade DoConverte(const TDado& dado) const override;          // wasm func 9102
    TDado DoDeconverte(const TEntidade& entidade) const override;    // wasm func 9095
};

}  // namespace ecourna::app::dados::asn
