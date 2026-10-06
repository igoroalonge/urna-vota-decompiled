// uenux2/src/app/comum/dados/asn/processoeleitoral/cconversorsituacoeseleicoes.h   (path inferred: RTTI only; the file
// "<fase><pleito:05><uf>-ste.dat" is read next to the pleito by the processo-eleitoral loader)
// Reconstructed from vota_web_wasm.wasm (unit u35).
//
// ModuloSituacoesEleicoes:
//   EntidadeSituacoesEleicoes ::= SEQUENCE { cabecalho CabecalhoEntidade, situacoesEleicoes SEQUENCE OF
//                                            SituacaoEleicao OPTIONAL }
//   SituacaoEleicao ::= SEQUENCE { idEleicao INTEGER (0..99999), tipo TipoSituacaoEleicao {ativa 0, suspensa 1,
//                                  cancelada 2, excluida 3}, ordemAquisicao, ordemImpressao, ordemApuracao (1..99) }
// md::CSituacoesEleicoes (8 bytes): +0 TEleicaoID, +4 uint8 ordemAquisicao, +5 ordemImpressao, +6 ordemApuracao.
// The orders decide in which order the eleições (and their cargos) are voted, printed and counted.
//
// RTTI: comum::asn::CConversorSituacoesEleicoes : IConversorASN<ModuloSituacoesEleicoes::EntidadeSituacoesEleicoes,
//       std::vector<comum::md::CSituacoesEleicoes>>, vtable @1570524: [2] 11360 default DoConverte (throws 7655,
//       read-only file) [3] 11361 DoDesconverte. sizeof 4.
#pragma once

#include <vector>

#include "comum/asn/iconversorasn.h"
#include "comum/dados/md/processoeleitoral/csituacoeseleicoes.h"   // (header name ?)
#include "ModuloSituacoesEleicoes.h"

namespace comum::asn {

class CConversorSituacoesEleicoes
    : public IConversorASN<ModuloSituacoesEleicoes::EntidadeSituacoesEleicoes, std::vector<md::CSituacoesEleicoes>>
{
protected:
    TDado DoDesconverte(const TEntidade& entidade) const override;   // wasm func 11361
};

} // namespace comum::asn
