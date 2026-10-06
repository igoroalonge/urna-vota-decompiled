// FRAGMENT reconstructed by unit u34 from vota_web_wasm.wasm.
// Original file: uenux2/src/api/util/cstringutils.cpp (attested; api::CStringUtils, owner unit u20; see also
// cstringutils.u33.cpp for NomeArquivo, func 2763).
#include <string>

#include "api/util/cstringutils.h"

namespace api {

// Inlined into 5461: the directory part of a path ("" when the path has no '/').      name inferred
static std::string Diretorio(const std::string& caminho)
{
    const std::string antes = caminho.substr(0, caminho.rfind('/'));   // npos -> the whole string
    return antes == caminho ? std::string() : caminho.substr(0, caminho.rfind('/'));
}

// wasm func 5461 (tools: api_f5461) - observed executing.                               name inferred
// Replaces the extension of the file name: "/dsk/fi/estatico/t02411ac-ce.dat" + "pid" -> "/dsk/fi/estatico/t02411ac-ce.pid". The dot is
// kept, `extensao` is appended after it. A name without '.' is returned unchanged.
// Callers: comum::asn::LePleito (func 5778, "-ce.dat" -> "-ce.pid": the package-version header of each
// election's candidate file) and the voter start-up routine (7787).
// NOTE: for a file directly under the root ("/x.dat") the directory part is "" and the result loses the
// leading '/' ("x.pid"); for every path used by VOTA (/dsk/fi/estatico/...) this does not happen.
std::string CStringUtils::TrocaExtensao(const std::string& caminho, const std::string& extensao)
{
    const std::string diretorio = Diretorio(caminho);
    const std::string nome = NomeArquivo(caminho);                     // func 2763
    const auto ponto = nome.rfind('.');
    if (ponto == std::string::npos)                                    // also when the name is empty
        return caminho;
    const std::string prefixo = diretorio.empty() ? std::string() : diretorio + '/';
    return prefixo + nome.substr(0, ponto + 1) + extensao;
}

}  // namespace api
