// uenux2/src/app/comum/justificativa/cjustificador.cpp  --  FRAGMENT written by unit u36 (file owned by u24).
// Reconstructed from vota_web_wasm.wasm.
#include "comum/justificativa/cjustificador.h"

#include <format>
#include <string>

namespace comum {

// wasm func 11474 (table slot 2948)                                    // class and method names inferred
// Text source of the total line of the printed "Boletim de Justificativa" (BUJ): vota::CGeraRelatorios
// (12105) builds CDataTextFmt(slot 2948, "{:05} Justificativas"). The count is the size of the
// CJustificador map (+8), i.e. the justifications recorded in this urna (SQLite registro_justificativa).
// Pluralisation is done by chopping the LAST CHARACTER of the format when the count is 1:
//     0 -> "00000 Justificativas", 1 -> "00001 Justificativa", 7 -> "00007 Justificativas".
// CJustificador::GetInst() (1391: lazy singleton @1839012, 40-byte object, mutex residue @1838988) is
// inlined; its sibling CJustificadorDadoTitulo::Text is wasm 11475 (srcloc :104).
// Wrapper type: api::CDataTextFmt<std::string (*)(const std::string&)> (vtable @1539168).
std::string CJustificadorDadoQuantidade::Text(const std::string& formato)
{
    const CJustificador& justificador = CJustificador::GetInst();
    std::string texto = formato;
    if (justificador.size() == 1)
        texto.pop_back();        // no emptiness check: an empty format would write before the buffer (UB)
    return std::vformat(texto, std::make_format_args(justificador.size()));   // size_t -> unsigned
}

}  // namespace comum
