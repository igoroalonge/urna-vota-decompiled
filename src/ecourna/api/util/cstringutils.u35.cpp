// FRAGMENT of ecourna-lib/ecourna/api/util/cstringutils.cpp (file of unit u13; declarations in cstringutils.hpp).
// Reconstructed from vota_web_wasm.wasm (unit u35). Both functions were attributed to app:comum by the tools because
// their callers are in comum/vota; the attribution to ecourna's CStringUtils follows u13 (Trim) and the fact that
// Split's emplace_back slow path (func 9404) is emitted among the ecourna cstringutils functions (9398..9410).
// Other units refer to Split as comum::Split / util::Split / api::CStringUtils::Split (same function).   (class ?)
#include "ecourna/api/util/cstringutils.hpp"

#include <string>
#include <vector>

namespace ecourna::api::util {

// wasm func 1374 - copy + in-place trim: an instance of the merged "copy then transform" body func 3942, with the
// transform passed as table slot 6416 = Trim(std::string&) (func 9406). Observed executing (start-up: CVoto,
// CMunicipio, CEleitor::ValidaCriacao, CCargos::GetCurrentEleicaoVersaoPacote, IInterfaceInit::MontarMRSemHabilitar,
// CConversorDetalheConsulta, the operator's version screen).
std::string CStringUtils::Trim(const std::string& texto)
{
    std::string copia = texto;
    Trim(copia);            // removes every byte <= ' ' at both ends
    return copia;
}

// wasm func 1880 - name inferred. Observed executing (CTradutorFrase::TraduzLabel 654 at start-up, 2380, 5869,
// CSubstituidorTitulo, CConversorDetalheConsulta, CImprimirBUOutrasObrigatorias).
// Splits at every occurrence of `separador`. Always returns at least one element: "" -> {""}, "a|" -> {"a", ""},
// "|a" -> {"", "a"}. The text is copied, then consumed from the front with erase(0, pos + 1).
std::vector<std::string> CStringUtils::Split(const std::string& texto, char separador)
{
    std::vector<std::string> partes;
    std::string resto = texto;
    for (std::size_t pos = resto.find(separador); pos != std::string::npos; pos = resto.find(separador)) {
        partes.emplace_back(resto.data(), pos);          // pos == 0 -> push_back(std::string{})
        resto.erase(0, pos + 1);
    }
    partes.push_back(resto);
    return partes;
}

} // namespace ecourna::api::util
