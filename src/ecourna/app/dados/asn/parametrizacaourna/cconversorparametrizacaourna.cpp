// ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrizacaourna.cpp   (path inferred:
//   siblings cconversorparametrosurna.cpp / cconversorlabelparametrizado.cpp / cconversortitulorelatorio.cpp
//   are srcloc-attested in the same directory)
// Reconstructed from vota_web_wasm.wasm (unit u11). Only DoDeconverte is in this unit.
//
// "Parametrização da urna" (<..>-pu.dat; every scenario has a national t00000br-pu.dat and a UF file such as
// t02400ac-pu.dat): report titles, labels and behaviour switches of the voting application.
#include "ecourna/api/asn/iconversorasn.hpp"
#include "ecourna/app/dados/asn/cconversorcabecalhoentidade.h"
#include "ecourna/app/dados/asn/parametrizacaourna/cconversorparametrosurna.h"   // unit u14
#include "ecourna/app/dados/parametrizacaourna/cparametrizacaourna.h"
#include "ModuloParametrizacaoUrna.h"

namespace ecourna::app::dados::asn {

// RTTI: CConversorParametrizacaoUrna
//         : IConversorASN<ModuloParametrizacaoUrna::EntidadeParametrizacaoUrna, CParametrizacaoUrna>
// vtable @1127436: [0] 174  [1] 144  [2] 9173 DoConverte (unit u40)  [3] 9171 DoDeconverte
//
//   EntidadeParametrizacaoUrna ::= SEQUENCE { cabecalho CabecalhoEntidade, parametros ParametrosUrna }
class CConversorParametrizacaoUrna
    : public api::asn::IConversorASN<ModuloParametrizacaoUrna::EntidadeParametrizacaoUrna, CParametrizacaoUrna>
{
protected:
    TEntidade DoConverte(const TDado& dado) const override;          // wasm func 9173 (unit u40)
    TDado DoDeconverte(const TEntidade& entidade) const override;    // wasm func 9171
};

// wasm func 9171 (vtable slot 3). Named "IConversorASN<ParametrosUrna, CParametrosUrna>::Deconverte" by the
// tool (inlined call, srcloc :66 record @1127716). Observed at run time: the heaviest function of this unit
// in the profiles (46/54 inclusive samples), because CConversorParametrosUrna::DoDeconverte (func 9156,
// ~3.5 KB) runs inside it.
CParametrizacaoUrna CConversorParametrizacaoUrna::DoDeconverte(const TEntidade& entidade) const
{
    const CConversorCabecalhoEntidade conversorCabecalho;   // vptr @1123740
    const CConversorParametrosUrna conversorParametros;     // vptr @1128320

    const CCabecalhoEntidade cabecalho = conversorCabecalho.Deconverte(entidade.get_cabecalho());   // func 2665
    const CParametrosUrna parametros = conversorParametros.Deconverte(entidade.get_parametros());   // inlined

    return CParametrizacaoUrna(cabecalho, parametros);     // func 9029; then ~CParametrosUrna (func 2267)
}

}  // namespace ecourna::app::dados::asn
