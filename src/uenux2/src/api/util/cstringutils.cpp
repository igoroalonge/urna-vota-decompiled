// Reconstructed from vota_web_wasm.wasm (unit u20, owner of this file).
// Original: uenux2/src/api/util/cstringutils.cpp (srcloc cstringutils.cpp:59).
// Other fragments: CStringUtils::PadLeft (func 753) in src/uenux2/src/app/comum/dados/u05-foreign-fragments.cpp.
// (Not to be confused with ecourna::api::util::CStringUtils, the ecourna library class.)
#include "api/util/cstringutils.h"

#include <format>
#include <string>

namespace api {

namespace {
// wasm func 2199 (tools: api_f2199, other unit): boost::regex search that returns
// {position, length} of the first match, or {-1, -1} when the pattern or text is empty, when there is
// no match, or when boost throws (exceptions are swallowed).                           name inferred
struct SOcorrencia { int posicao; int tamanho; };
SOcorrencia ProcuraRegex(const std::string& expressao, const std::string& texto);
} // namespace

// wasm func 1539                                                           srcloc cstringutils.cpp:59
// "10.23.0.1 - DESENVOLVIMENTO" -> "10.23.0.1". Used for the "Ver:" line of the BU / zerésima, the
// VERS field of the BU QR code, CGeradorRelVersaoPacoteDados and the voter list.
std::string CStringUtils::GetVersionNumber(const std::string& versao)
{
    const std::string expressao = R"(^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+)";
    const SOcorrencia ocorrencia = ProcuraRegex(expressao, versao);
    if (ocorrencia.posicao != 0)
        throw CUeUtilError(EUeUtilError{7025},
                           std::format("A string [{}] não representa uma versão válida", versao));
    return versao.substr(0, ocorrencia.tamanho);
}

} // namespace api
