// Reconstructed from vota_web_wasm.wasm (unit u20).
// Original: uenux2/src/api/util/cgenerictags.cpp
//   srclocs :31/:34/:37 insert, :46 TagSize, :79/:85 EncodeTLV, :102 AppendTLV, :110 WalkTreeTLV,
//           :136 DecodeTLV.
// WalkTreeTLV and DecodeTLV only exist inlined into their single caller, func 5892 (an
// comum::IInterfaceSavd routine, see src/uenux2/src/app/comum/u20-foreign-fragments.cpp).
#include "api/util/cgenerictags.h"

#include <cmath>
#include <format>
#include <string>

#include "ecourna/api/util/cstringutils.hpp"

namespace api {

namespace {
constexpr char MARCADOR_BASE = ';';                 // length marker = ';' + n
} // namespace

// wasm func 1071                                                        srclocs cgenerictags.cpp:31/34/37
void CGenericTags::insert(const std::string& tag, std::size_t tamanho)
{
    if (tag.size() != m_tamanhoTag - 1)
        throw CUeUtilError(EUeUtilError{7013}, std::format("Tag [{}] com tamanho inválido", tag));
    if (m_tags.find(tag) != m_tags.end())
        throw CUeUtilError(EUeUtilError{7014}, std::format("Tag [{}] já foi mapeada", tag));
    if (tamanho < 1 || tamanho > 4)            // wasm: (tamanho - 5) <=u 0xFFFFFFFB (unsigned compare)
        throw CUeUtilError(EUeUtilError{7015}, std::format("Tag [{}] com tamanho inválido", tag));
    m_tags.insert(std::make_pair(tag, tamanho));   // pair<string, size_t> -> value_type<string, uebyte>
}

// wasm func 5468                                                            srcloc cgenerictags.cpp:46
std::size_t CGenericTags::TagSize(const std::string& tag) const
{
    auto it = m_tags.find(tag);
    if (it == m_tags.end())
        throw CUeUtilError(EUeUtilError{7016}, std::format("Tag [{}] não mapeada", tag));
    return it->second;                         // uebyte widened to size_t (i32.load8_u offset=28)
}

// wasm func 5467 - observed executing                             srclocs cgenerictags.cpp:79 / :85
std::string CGenericTags::EncodeTLV(const std::string& tag, const std::string& valor, std::size_t tamanho) const
{
    if (tag.size() != m_tamanhoTag - 1)
        throw CUeUtilError(EUeUtilError{7017}, std::format("Tag [{}] com tamanho inválido", tag));

    const double limite = std::exp2(static_cast<double>(tamanho * 8)) - 1.0;      // exp2 inlined (musl)
    std::string cabecalho = tag;
    cabecalho.append(m_tamanhoTag - tag.size(), static_cast<char>(MARCADOR_BASE + tamanho));   // exactly 1 char
    if (valor.size() > static_cast<std::size_t>(limite))
        throw CUeUtilError(EUeUtilError{7018}, std::format("Tag [{}] excedeu o limite de codificação", tag));

    return std::format("{}{:0{}x}{}", cabecalho, valor.size(), tamanho * 2, valor);
}

// wasm func 5466 - observed executing                                      srcloc cgenerictags.cpp:102
// The size_t parameter of the recorded signature is not used by the binary: the element size comes
// from TagSize(tag) (evaluated before the check).                                         // ?
void CGenericTags::AppendTLV(std::string& saida, const std::string& tag, const std::string& valor,
                             std::size_t /*tamanho*/) const
{
    const std::size_t tamanho = TagSize(tag);
    if (tag.size() != m_tamanhoTag - 1)
        throw CUeUtilError(EUeUtilError{7019}, std::format("Tag [{}] com tamanho inválido", tag));
    saida += EncodeTLV(tag, valor, tamanho);
}

// Inlined into func 5892                                                   srcloc cgenerictags.cpp:110
// Walks the top-level elements of `mensagem` and returns the offset of the first element named `tag`,
// or npos if it is absent or if a malformed length marker is met.
std::size_t CGenericTags::WalkTreeTLV(const std::string& mensagem, const std::string& tag) const
{
    if (tag.size() != m_tamanhoTag - 1)
        throw CUeUtilError(EUeUtilError{7020}, std::format("Tag [{}] com tamanho inválido", tag));

    std::size_t pos = 0;
    while (pos < mensagem.size()) {
        const std::string nome = mensagem.substr(pos, m_tamanhoNome);
        const std::string campo = mensagem.substr(pos, m_tamanhoTag);
        if (m_tamanhoNome > campo.size())
            throw std::out_of_range("basic_string");                 // substr(m_tamanhoNome, ...) check
        const std::string marcador = campo.substr(m_tamanhoNome, 1);
        for (char c : marcador)
            if ((c & 0xFC) != 0x3C)                                  // only '<' '=' '>' '?' are valid
                return std::string::npos;
        const std::size_t digitos = static_cast<std::size_t>((campo.back() - MARCADOR_BASE) * 2);   // 2n
        // the length is parsed before the name is compared (HexToInt, func 3507, may throw first)
        const std::size_t tamanhoValor = static_cast<std::size_t>(
            ecourna::api::util::CStringUtils::HexToInt(mensagem.substr(pos + m_tamanhoTag, digitos)));
        if (nome == tag)                   // `tag` is a static std::string ("sup") in the only caller
            return pos;
        pos += m_tamanhoTag + digitos + tamanhoValor;
    }
    return std::string::npos;
}

// Inlined into func 5892                                                   srcloc cgenerictags.cpp:136
// Removes the element `tag` from `mensagem` and returns its value.
std::string CGenericTags::DecodeTLV(std::string& mensagem, const std::string& tag) const
{
    const std::size_t pos = WalkTreeTLV(mensagem, tag);
    if (pos == std::string::npos)
        throw CUeUtilError(EUeUtilError{7021}, std::format("Tag [{}] não codificada", tag));
    const std::string campo = mensagem.substr(pos, m_tamanhoTag);
    const std::size_t digitos = static_cast<std::size_t>((campo.back() - MARCADOR_BASE) * 2);
    const std::size_t tamanhoValor = static_cast<std::size_t>(
        ecourna::api::util::CStringUtils::HexToInt(mensagem.substr(pos + m_tamanhoTag, digitos)));
    std::string valor = mensagem.substr(pos + m_tamanhoTag + digitos, tamanhoValor);
    mensagem.erase(pos, m_tamanhoTag + tamanhoValor + digitos);
    return valor;
}

} // namespace api
