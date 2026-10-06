// FRAGMENT of ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrizacaourna.cpp (path
// inferred; class + DoDeconverte by unit u11). Reconstructed by unit u40 from vota_web_wasm.wasm.
#include "ecourna/app/dados/asn/cconversorcabecalhoentidade.h"
#include "ecourna/app/dados/asn/parametrizacaourna/cconversorparametrosurna.h"
#include "ecourna/app/dados/parametrizacaourna/cparametrizacaourna.h"

namespace ecourna::app::dados::asn {

// wasm func 9173 (vtable @1127436 slot 2). Not used by the voting application (it only reads -pu.dat); present
// because the converter class is instantiated.
//   EntidadeParametrizacaoUrna ::= SEQUENCE { cabecalho CabecalhoEntidade, parametros ParametrosUrna }
ModuloParametrizacaoUrna::EntidadeParametrizacaoUrna
CConversorParametrizacaoUrna::DoConverte(const CParametrizacaoUrna& dado) const
{
    ModuloParametrizacaoUrna::EntidadeParametrizacaoUrna entidade;
    const CConversorCabecalhoEntidade conversorCabecalho;                               // vptr @1123740
    entidade.set_cabecalho(conversorCabecalho.Converte(dado.GetCabecalho()));          // 9211 -> 9219
    const CConversorParametrosUrna conversorParametros;                                 // vptr @1128320
    entidade.set_parametros(conversorParametros.Converte(dado.GetParametros()));       // +16; 6795 -> 9164
    return entidade;
}

}  // namespace ecourna::app::dados::asn
