// uenux2/src/app/comum/comparecimentomesario/cregistradormesario.cpp  (path inferred by u20)
//   --  FRAGMENT written by unit u36. The class is written in comum/u20-foreign-fragments.cpp.
// Reconstructed from vota_web_wasm.wasm.
//
// comum::CRegistradorMesario caches the rows of the SQLite table comparecimento_mesario (uenux.db):
// which poll workers ("mesários") registered their attendance, and in which period.
//   +0 std::map<md::CComparecimentoMesarioPK, md::CComparecimentoMesario> m_registros
//      node: +16 key { std::string titulo (+16), int tipoIdentificador (+28), int periodo (+32) }
#include <cstddef>
#include <map>

namespace comum {

// wasm func 6007 - merge-similar body                                                  // name inferred
// Number of mesários registered in `periodo`. wasm-opt merged two source functions that differ only in
// the constant; the survivors are 3-instruction thunks:
//   shared_f3606 = QuantidadeRegistrados(1)   -> "registro inicial" (before the voting starts)      ?
//   shared_f2727 = QuantidadeRegistrados(2)   -> "registro final" (at the encerramento)            ?
// Callers of the thunks: the MT data sources CDataTextFmt<CComparecimentoMesariosDS> (5387 / 10368),
// vota::CControladorRegistraMesariosVota (10795) and vota::CGeraRelatorios::StartState (12105, BIM report).
std::size_t CRegistradorMesario::QuantidadeRegistrados(const int periodo) const
{
    std::size_t quantidade = 0;
    for (const auto& [chave, registro] : m_registros)
        quantidade += (chave.GetPeriodo() == periodo);
    return quantidade;
}

}  // namespace comum
