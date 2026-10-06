// ecourna-lib/ecourna/app/dados/parametrizacaourna/cparametrizacaourna.cpp   (path inferred: next to
//   cparametrosurna.cpp; the class name comes from the RTTI of its converter
//   IConversorASN<ModuloParametrizacaoUrna::EntidadeParametrizacaoUrna, ecourna::app::dados::CParametrizacaoUrna>)
// Reconstructed from vota_web_wasm.wasm (unit u11).
//
// CParametrizacaoUrna (non-polymorphic):
//   +0  CCabecalhoEntidade cabecalho   (16 bytes, copied as two 64-bit words: data de geração + id eleitoral)
//   +16 CParametrosUrna    parametros  (400+ bytes, see cparametrosurna.u02.cpp)
#include "ecourna/app/dados/parametrizacaourna/cparametrizacaourna.h"

namespace ecourna::app::dados {

// wasm func 9029 - name inferred (only caller: CConversorParametrizacaoUrna::DoDeconverte, func 9171).
// Unit u02 described the same function as "std::pair<const K, CParametrosUrna> constructor": the machine
// code is identical (16-byte trivially copyable first member + CParametrosUrna copy constructor, func 3777).
// It always ran during the recorded votes (votaInit reads the -pu.dat files).
CParametrizacaoUrna::CParametrizacaoUrna(const CCabecalhoEntidade& cabecalho, const CParametrosUrna& parametros)
    : m_cabecalho(cabecalho)      // +0
    , m_parametros(parametros)    // +16, func 3777
{
}

}  // namespace ecourna::app::dados
