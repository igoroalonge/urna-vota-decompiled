// ecourna-lib/ecourna/api/security/base64.cpp
// (/home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/api/security/base64.cpp in the srcloc records)
//
// Reconstructed from vota_web_wasm.wasm. Unit u01.
//
// The file is at least 267 lines long (srcloc lines 247 and 267 are in EncodeBase64), so most of it is not
// in the binary: probably a decoder and helpers that the web build never links (error code 1326, between
// the two codes used here, never appears).
//
// The encoder core is the well-known public-domain C snippet ("encoding_table" + "mod_table = {0, 2, 1}",
// output_length = 4 * ((len + 2) / 3), malloc'ed result). Both tables are in the data segment:
//   @1112848  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
//   @1112912  int[3] {0, 2, 1}
#include "ecourna/api/security/base64.hpp"

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "ecourna/api/exception/cbaseerror.hpp"
#include "ecourna/api/security/esecurityerror.hpp"   // ESecurityError; CSecurityError = exception::CBaseError<ESecurityError, SErrorLimits{1325, 1725}> (alias name inferred)

namespace ecourna::api::security {

namespace {

const char s_encodingTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";   // @1112848
const int  s_modTable[]      = {0, 2, 1};                                                         // @1112912

// Size of the output buffer the caller allocates.                                          // name inferred
// Compiled as 4*n - 8*(n/3), i.e. 4 * (n/3 + n%3): a rounding-up attempt that over-estimates by 4 bytes when
// n % 3 == 2 (harmless: it is never smaller than the real 4 * ceil(n/3)).
std::size_t TamanhoCodificado(const std::size_t n)
{
    return ((n / 3) + (n % 3)) * 4;
}

// The public snippet: returns a malloc'ed, NOT NUL-terminated buffer of *outLen bytes, or nullptr.   // inlined
char* base64_encode(const unsigned char* data, const std::size_t inLen, std::size_t* outLen)
{
    *outLen = 4 * ((inLen + 2) / 3);
    char* encoded = static_cast<char*>(std::malloc(*outLen));
    if (encoded == nullptr) {
        return nullptr;
    }
    for (std::size_t i = 0, j = 0; i < inLen;) {
        const std::uint32_t a = i < inLen ? data[i++] : 0;
        const std::uint32_t b = i < inLen ? data[i++] : 0;
        const std::uint32_t c = i < inLen ? data[i++] : 0;
        const std::uint32_t triple = (a << 16) + (b << 8) + c;
        encoded[j++] = s_encodingTable[(triple >> 18) & 0x3F];
        encoded[j++] = s_encodingTable[(triple >> 12) & 0x3F];
        encoded[j++] = s_encodingTable[(triple >> 6) & 0x3F];
        encoded[j++] = s_encodingTable[triple & 0x3F];
    }
    for (int i = 0; i < s_modTable[inLen % 3]; i++) {   // compiled to a memset of '=' (61)
        encoded[*outLen - 1 - i] = '=';
    }
    return encoded;
}

// Copies the encoding into a caller buffer. Returns the length written, or -1.          // inlined, name inferred
// The four checks are compiled in exactly this order.
int CodificaBase64(const uebyte* origem, const std::size_t tamanho, char* destino, const std::size_t tamanhoDestino)
{
    if (destino == nullptr || origem == nullptr || tamanho == 0 ||
        tamanhoDestino < TamanhoCodificado(tamanho)) {
        return -1;
    }
    std::size_t tamanhoSaida = 0;
    char* codificado = base64_encode(origem, tamanho, &tamanhoSaida);
    if (tamanhoSaida > tamanhoDestino) {
        std::free(codificado);
        return -1;
    }
    // No nullptr check: if the malloc inside base64_encode failed, this copies tamanhoSaida bytes from
    // address 0 (valid linear memory in wasm, so no trap: the result is garbage instead of an error).
    std::memcpy(destino, codificado, tamanhoSaida);
    std::free(codificado);
    return static_cast<int>(tamanhoSaida);
}

} // namespace

// srcloc lines 247 and 267. There is no stand-alone copy of this function in the binary; see func 3703 below.
std::string EncodeBase64(const std::vector<uebyte>& dados)
{
    if (dados.empty()) {
        throw CSecurityError(ESecurityError::VetorDadosVazio /* 1325 */,          // line 247, name inferred
                             "Vetor de dados vazio.");
    }

    std::string resultado;
    const std::size_t tamanhoBuffer = TamanhoCodificado(dados.size());
    char* buffer = static_cast<char*>(std::malloc(tamanhoBuffer));
    const int tamanho = CodificaBase64(dados.data(), dados.size(), buffer, tamanhoBuffer);
    if (tamanho < 0) {
        std::free(buffer);
        throw CSecurityError(ESecurityError::FalhaConversaoBase64 /* 1327 */,     // line 267, name inferred
                             "Nao foi converter para base64.");                   // sic (missing "possível")
    }
    resultado.assign(buffer, tamanho);   // func 986 = std::string::assign(const char*, size_t)
    std::free(buffer);
    return resultado;
}

// ---------------------------------------------------------------------------------------------------------
// wasm func 3703 (named "EncodeBase64" by the srcloc heuristic) is really a small CALLER into which the whole
// EncodeBase64 above was inlined. Its only parameter is a comum::md::estadoaplicacao::CEstadoGeral (every
// caller passes comum::GetEstado<CEstadoGeral>(...), func 291), it reads the std::vector<uebyte> at +168 —
// the field CConversorEstadoGeral (func 11397) writes into EstadoGeralUrna.hashVersoesPacotes — and it
// returns only the FIRST 8 CHARACTERS of the Base64 text (min(size, 8) copied into an SSO string).
// Callers: CGeradorRelVersaoPacoteDados::vf4 (func 11222, report line "Dados:" right-aligned to 38 columns),
// func 3059 ("Versão: {}" / "Dados: {}") and func 7787. Real name and file unknown; it behaves like:
//
//   std::string ?(const comum::md::estadoaplicacao::CEstadoGeral& estado)   // name/file unknown
//   {
//       return EncodeBase64(estado.GetHashVersoesPacotes()).substr(0, 8);
//   }
// ---------------------------------------------------------------------------------------------------------

} // namespace ecourna::api::security
