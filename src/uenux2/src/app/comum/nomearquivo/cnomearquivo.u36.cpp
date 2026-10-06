// uenux2/src/app/comum/nomearquivo/cnomearquivo.cpp (+ .h)  --  FRAGMENT written by unit u36 (file owned by
// u02/u24). Reconstructed from vota_web_wasm.wasm.                                   (placement inferred)
//
// The 24-byte identification record used to build the names of the election data / result files
// ("<fase><id:05><uf>..."). cnomearquivo.h declares its first three members; the binary shows a fourth:
//   +0 EUrnaFase fase | +4 uedword id do processo eleitoral | +8 std::string uf (lower case) |
//   +20 const md::CPleito* pleito (the pleito of the current turno)
// It is embedded in CPE (+180, built by CPE::CreateInst 2787 with pleito = turno '1' ? pleito1 :
// GetPleito2()) and at the head of the section identification of CConfiguracaoEleicao (+620, built in the
// start-up function 7787 with pleito = &m_pleito; município +644, zona +648 and the aggregated sections
// +652 follow it there).
#include "comum/nomearquivo/cnomearquivo.h"

#include "ecourna/api/util/cstringutils.h"

namespace comum {

// wasm func 5726 - executed once in votaInit (entry counter)            // name inferred (struct name from u24)
SIdentificacaoCarga::SIdentificacaoCarga(const EUrnaFase fase_, const uedword id_, const std::string& uf_,
                                         const md::CPleito* pleito_)
    : fase(fase_)
    , pleito(id_)                                                   // named `pleito` in cnomearquivo.h   ?
    , uf(ecourna::api::util::CStringUtils::ToLower(uf_))             // wasm 1879 (copy + Latin-1 tolower)
    , dadosPleito(pleito_)
{
}

}  // namespace comum
