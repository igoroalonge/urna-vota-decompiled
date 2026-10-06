// FRAGMENT of uenux2/src/app/comum/relatorios/crelutil.cpp (attested; owner u25, declarations in crelutil.h).
// Reconstructed from vota_web_wasm.wasm (unit u35). Small layout helpers of the thermal-printer reports (BU, zerésima,
// lista de eleitores, BUJ...). A report line is 38 characters wide; api::CPaperFormBuilder::AddText(texto, fonte,
// alinhamento) = shared_f193 (fonte 1 normal / 2 expandida; alinhamento 0 esquerda, 1 direita, 2 centro).
#include "comum/relatorios/crelutil.h"

#include <format>
#include <functional>
#include <string>

namespace comum {

// wasm func 1387 - name inferred (other units: "IncluiTracejado", "AddSeparador"). Not observed executing.
// A line of 38 '-' in the given alignment. Callers: vota::CImpressaoListaEleitores (11908), CGeraZeresimaBase (11946),
// CGeraRelatorios (12105), CGeraBU (12110).
void CRelUtil::IncluiSeparador(api::CPaperFormBuilder& b, const uebyte alinhamento)
{
    b.AddText(std::string(38, '-'), 1, alinhamento);
}

// wasm func 1881 - name as declared by u25 (u05 calls the same function api::CStringUtils::PadRight).
// Not observed executing. Pads with spaces on the right up to `largura` (never truncates).
std::string CRelUtil::CompletaDireita(const std::string& texto, const std::size_t largura)
{
    std::string resultado = texto;
    if (resultado.size() < largura)
        resultado.append(largura - resultado.size(), ' ');       // shared_f1054, through invoke (landing pad frees)
    return resultado;
}

// wasm func 1921 - name as declared by u25. Not observed executing.
// The three "aptos" lines of the BU / zerésima / voter list. The std::function is taken BY VALUE (callers copy it
// with func 1922 = std::function copy constructor); calling an empty one throws std::bad_function_call.
//   "Eleitores aptos                   0123\n"
//   "          Originais da seção      0120\n"
//   "          Temporários na seção    0003"
// (the total is computed as uint16: (originais + temporários) & 0xFFFF)
std::string CRelUtil::FormataQtdAptos(std::function<SQtdeAptos()> fonte)
{
    const SQtdeAptos aptos = fonte();
    const std::uint16_t total = static_cast<std::uint16_t>(aptos.qtdAptosSecao + aptos.qtdAptosTTE);
    std::string texto;
    texto += std::format("Eleitores aptos                   {:04}\n", total);
    texto += std::format("          Originais da seção      {:04}\n", aptos.qtdAptosSecao);
    texto += std::format("          Temporários na seção    {:04}", aptos.qtdAptosTTE);
    return texto;
}

} // namespace comum
