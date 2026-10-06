// ecourna-lib/ecourna/app/dados/resultadournacadastro/ccomparecimentomesario.cpp  (path inferred)
// Reconstructed from vota_web_wasm.wasm, unit u14.
//
// The constructor has no std::source_location (it cannot throw), so its file is inferred from the
// class name. The tool attributed it to cconversorcomparecimentomesario.cpp (one of its two callers;
// the other is comum_f5841, the mesário registration code of the VOTA application).
#include "ecourna/app/dados/resultadournacadastro/cdadoscomparecimento.h"

namespace ecourna::app::dados {

// wasm func 3486 (name inferred): copies the identification (two shared_ptr, one optional),
// the ptime (+24) and the state (+32).
CComparecimentoMesario::CComparecimentoMesario(const CRegistroIdentificacaoEleitor& identificacao,
                                               const boost::posix_time::ptime& dataHoraColeta,
                                               EEstadoColetaDigital estado)
    : m_identificacao(identificacao)
    , m_dataHoraColeta(dataHoraColeta)
    , m_estadoColetaDigital(estado)
{
}

} // namespace ecourna::app::dados
