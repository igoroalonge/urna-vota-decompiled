// FRAGMENT reconstructed by unit u07 from vota_web_wasm.wasm.
// Original file: uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.cpp (or inline in ccargo.h) - attested
// by srcloc records :81 (GetDetalheCandidato), :89 (GetDetalheConsulta), :112, :212-240 (Valida).
// Two small accessors that stayed out of line and were assigned to u07. Merge into ccargo.cpp/.h.
//
// comum::md::CCargo layout (from Valida, func 5657, and the accessors):
//   +0  TCargoID m_codigo (uebyte)          +4  ETipo m_tipo (0 majoritário, 1 proporcional, 2 consulta)
//   +8  abrangência (< 3)                    +12 uebyte m_numeroDigitos (1..5)
//   +13 uebyte m_qtdEscolhas                 +18 uebyte (1..5) ?
//   +20 std::optional<CDetalheCandidato>  (engaged flag +84): +20 bool m_possuiFoto, +24/+36 strings (nome neutro /
//                                          nome), +72 std::vector<CDetalheSuplente> (52-byte elements)
//   +88 std::optional<CDetalheConsulta>   (engaged flag +136): +88 nome, +100 pergunta
#include "comum/dados/md/processoeleitoral/ccargo.h"

namespace comum::md {

// wasm func 1546                                                                     // name inferred
// true when the cargo is a candidate cargo whose screens show photos (CDetalheCandidato first byte).
bool CCargo::PossuiFoto() const
{
    return m_detalheCandidato.has_value() && m_detalheCandidato->PossuiFoto();
}

// wasm func 1157                                                                     // name inferred
// Number of running mates (vice / suplentes) of a candidate cargo, 0 for consultas.
uebyte CCargo::GetQtdSuplentes() const
{
    return m_detalheCandidato.has_value() ? static_cast<uebyte>(m_detalheCandidato->GetSuplentes().size()) : 0;
}

} // namespace comum::md
