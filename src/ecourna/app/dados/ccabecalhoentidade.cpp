// ecourna-lib/ecourna/app/dados/ccabecalhoentidade.cpp   (path inferred)
//
// Reconstructed from vota_web_wasm.wasm. Unit u40.
#include "ecourna/app/dados/ccabecalhoentidade.h"

namespace ecourna::app::dados {

// wasm func 5112 - member-wise copies (one i64 load/store and two i32 stores), no checks.
CCabecalhoEntidade::CCabecalhoEntidade(const boost::posix_time::ptime& dataGeracao, int id, ETipoId tipo)
    : m_dataGeracao(dataGeracao)
    , m_id(id)
    , m_tipoId(tipo)
{
}

} // namespace ecourna::app::dados
