// ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorcomparecimentosecao.cpp   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u11). Only DoDeconverte (+ its template instances) is in u11.
#include "ecourna/app/dados/asn/resultadournacadastro/cconversorcomparecimentosecao.h"

#include <algorithm>
#include <iterator>
#include <vector>

#include "ecourna/app/dados/asn/cconversoridentificacaosecaoeleitoral.h"
#include "ecourna/app/dados/asn/resultadournacadastro/cconversorestadocomparecimento.h"   // unit u14

namespace ecourna::app::dados::asn {

// wasm func 9120 (vtable slot 3). Named "IConversorASN<IdentificacaoSecaoEleitoral, ...>::Deconverte" by
// the tool: that call is inlined here (srcloc iconversorasn.hpp:66, record @1131644). Not observed at run time.
CComparecimentoSecao CConversorComparecimentoSecao::DoDeconverte(const TEntidade& entidade) const
{
    const CConversorIdentificacaoSecaoEleitoral conversorIdentificacao;   // vptr @1130788
    const CConversorEstadoComparecimento conversorEstado;                 // vptr @1134808

    // inlined Deconverte: validates field 0, then CConversorIdentificacaoSecaoEleitoral::DoDeconverte (9127)
    const CIdentificacaoSecaoEleitoral identificacao = conversorIdentificacao.Deconverte(entidade.get_identificacao());

    // One CEstadoComparecimento per voter of the section (no copy of the SEQUENCE OF here).
    std::vector<CEstadoComparecimento> eleitores;
    std::transform(entidade.get_eleitores().begin(), entidade.get_eleitores().end(), std::back_inserter(eleitores),
                   [&conversorEstado](const ModuloResultadoUrnaCadastro::EstadoComparecimento& estado) {
                       return conversorEstado.Deconverte(estado);   // srcloc :66 @1131660 -> DoDeconverte 9081
                   });

    return CComparecimentoSecao(identificacao, eleitores);            // func 5092
}

// -------------------------------------------------------------------------------------------------------
// Template instantiations emitted for this file (library code, summarised):
//   wasm func 9119  std::transform<SEQUENCE_OF<EstadoComparecimento>::const_iterator,
//                   std::back_insert_iterator<std::vector<CEstadoComparecimento>>, $lambda above>
//                   (the lambda, i.e. IConversorASN<EstadoComparecimento>::Deconverte, is inlined; the
//                   96-byte temporary is destroyed with func 1006)
//   wasm func 9118  std::vector<CEstadoComparecimento>::push_back(CEstadoComparecimento&&)
//                   (fast path: member-wise move of the 96-byte record; slow path: capacity max(2*cap, size+1), capped at
//                   max_size 44 739 242, relocation func 2845, destruction 1006 on failure)
//   wasm func 5100  std::vector<CEstadoComparecimento>::~vector()
//   wasm func 5096  std::__exception_guard_exceptions<std::vector<CEstadoComparecimento>::__destroy_vector>::
//                   ~__exception_guard_exceptions()   (also used by the CComparecimentoSecao ctor, func 5092)
//   wasm func 1006  CEstadoComparecimento::~CEstadoComparecimento() (implicit; releases the nested optional
//                   shared_ptrs described in the header, innermost first)
// -------------------------------------------------------------------------------------------------------

}  // namespace ecourna::app::dados::asn
