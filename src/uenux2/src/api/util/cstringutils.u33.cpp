// uenux2/src/api/util/cstringutils.cpp  -- FRAGMENT written by unit u33 (the file belongs to unit u20; srcloc
// cstringutils.cpp:59 = CStringUtils::GetVersionNumber). File of the three helpers inferred: they are api-level
// string helpers called from api, comum and vota code alike.                                    // path inferred
#include <string>
#include <utility>

#include <boost/regex.hpp>

#include "api/util/cstringutils.h"

namespace api {

// wasm func 2199 (observed executing: only through CTradutorFrase::TraduzLabel, 654, called by 4160/4161;
// GetVersionNumber never shows up in the profiles)                                   // name inferred
// First match of a boost::regex in `texto`, as {start, end} offsets; {-1, -1} when either string is empty,
// when nothing matches, and when boost throws (the catch(...) swallows it: an invalid pattern looks like
// "no match"). Unit u20 declared it as {posicao, tamanho}: the second field is position(0) + length(0),
// i.e. the END offset (equal to the length only because GetVersionNumber requires position 0).
// Callers: CStringUtils::GetVersionNumber (1539, "^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+"),
//          comum::md::CTradutorFrase::TraduzLabel (654, three patterns per loop: "[CL][PS][ABN]>",
//          "<DT[+-][0-9]+>", "[^|>]*"), comum_f5465.
std::pair<int, int> CStringUtils::ProcuraRegex(const std::string& expressao, const std::string& texto)
{
    if (texto.empty() || expressao.empty())
        return {-1, -1};
    try {
        const boost::regex regex(expressao);                     // basic_regex::do_assign (9401), flags 0
        boost::smatch ocorrencia;
        if (!boost::regex_search(texto, ocorrencia, regex))      // 9400, match_default
            return {-1, -1};
        const int inicio = static_cast<int>(ocorrencia.position(0));
        return {inicio, inicio + static_cast<int>(ocorrencia.length(0))};
        // (position()/length() on a singular match_results throw std::logic_error("Attempt to access an
        //  uninitialized boost::match_results<> class."), inlined; unreachable after a search)
    } catch (...) {
        return {-1, -1};
    }
}

// wasm func 2677                                                                    // name inferred
// Copy of `texto` with every occurrence of `de` replaced by `para`. The in-place loop is shared_f9407
// (find via memchr/memcmp + std::string::replace). Callers: CIniStrings escape decoding (3651),
// comum::CSubstituidorTitulo (3689), api::CProgressBar::Draw (10977: "%p" -> "NN%").
// `texto` is taken by const reference: the callee itself copies it into the result slot (inline SSO copy or
// __init_copy_ctor_external, func 138), then runs the loop on the copy (NRVO); if the loop throws, the copy is
// freed and the exception resumed. (A by-value parameter would have been copied by the caller and moved here.)
std::string CStringUtils::ReplaceAll(const std::string& texto, const std::string& de, const std::string& para)
{
    std::string resultado(texto);
    SubstituiTodos(resultado, de, para);                         // shared_f9407 (in place)   // name inferred
    return resultado;
}

// wasm func 2763                                                                    // name inferred
// File-name part of a path: the text after the last '/', or the whole string when there is none.
// Callers: CMontadorHash::CalculaHashGeral (5360, names inside hash.dat), vota::CGravaResultado::StartState
// (12098, name of the "-vota.vsc" signature package), comum_f1501, api_f5461.
std::string CStringUtils::NomeArquivo(const std::string& caminho)
{
    const auto barra = caminho.rfind('/');
    if (barra == std::string::npos)
        return caminho;
    return caminho.substr(barra + 1);
}

} // namespace api
