// uenux2/src/app/comum/relatorios/cgeradorrelpu.cpp (path inferred) -- FRAGMENT written by unit u37.
// Class: cgeradorrelpu.h (units u35/u37); destructors: cgeradorrelpu.cpp (unit u35).
#include <string>

#include "api/util/cstringutils.h"           // api::CStringUtils::PadLeft (wasm 753)
#include "comum/relatorios/cgeradorrelpu.h"

namespace comum {

// wasm func 677 (tools: vota_f677)                                              name inferred (as in u25)
// One "<rótulo>  <valor>" line of the PU report: the value is right-aligned so that the line is 38
// characters wide (the width of a printer line in this font). With a label longer than 38 characters the
// width becomes negative and PadLeft adds nothing. Only caller: vota::CImpressaoPU::StartState (11902), for
// the voting-day time windows ("<rótulo>   hh:mm:ss -" and then "            dd/mm/aaaa", see
// FormataHorario 2686 / FormataData 2687).
void CGeradorRelPU::AdicionaLinha(const std::string& rotulo, const std::string& valor)
{
    std::string linha = rotulo;
    linha += api::CStringUtils::PadLeft(valor, ' ', 38 - static_cast<int>(rotulo.size()));   // wasm 753
    m_builder.AddText(linha, 1, 0);          // shared_f193: new CFixedText(0, linha) in a CTextFieldPaper(font 1)
}

} // namespace comum
