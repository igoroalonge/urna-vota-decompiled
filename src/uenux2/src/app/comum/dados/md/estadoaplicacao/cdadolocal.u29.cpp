// FRAGMENT of uenux2/src/app/comum/dados/md/estadoaplicacao/cdadolocal.cpp (path inferred from the class name
// in the srcloc IConversorASN<ModuloEstadoGeralUrna::DadoLocal, comum::md::estadoaplicacao::CDadoLocal>)
// reconstructed by unit u29 from vota_web_wasm.wasm.
//
// md side of ModuloEstadoGeralUrna::DadoLocal {uf SiglaUF, secaoCarga DadoSecao, tipoLocalVotacao}. 24 bytes:
//   +0  std::string m_uf     +12 CLocalidadeEleitoral m_secao {município +12, zona +16, seção +18}
//   +20 ETipoLocalVotacao m_tipo
#include <string>

#include "comum/dados/md/estadoaplicacao/clocalidadeeleitoral.h"

namespace comum::md::estadoaplicacao {

// wasm func 5632 (table slot 448). Member-wise constructor (the string is moved). Callers:
// CConversorDadoLocal::DoDesconverte (func 11404) and the builder's copy of CEstadoGeral (func 10205).
CDadoLocal::CDadoLocal(std::string uf, const CLocalidadeEleitoral& secao, ETipoLocalVotacao tipo)
    : m_uf(std::move(uf)), m_secao(secao), m_tipo(tipo)
{
}

}  // namespace comum::md::estadoaplicacao
