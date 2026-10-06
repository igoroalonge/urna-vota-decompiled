// Reconstructed from vota_web_wasm.wasm (unit u20).
// Original: uenux2/src/api/util/cgenerictags.h (class attested by srclocs in cgenerictags.cpp).
//
// api::CGenericTags - a textual TLV (tag / length / value) codec. Each registered tag has a fixed name
// length (m_tamanhoTag - 1 characters) and a "length of the length" n in 1..4. One element is encoded as
//     <tag><L><len as 2n lower-case hex digits><value>
// where L = ';' + n  ('<' for n=1, '=' n=2, '>' n=3, '?' n=4). Elements can be nested: the value of a
// tag may itself be a sequence of TLVs.
//
// The only instance in the binary is built by the SAVD client (comum::IInterfaceSavd, the urna's
// signing/validation service): CGenericTags(4, 3) with the tags
//     "pkg" (n=1)  package name           "key" (n=1)  key index (decimal text)
//     "fil" (n=1)  file name              "sup" (n=3)  envelope that contains the other three
#pragma once

#include <cstddef>
#include <map>
#include <string>

#include "ecourna/types.hpp"   // uebyte

namespace api {

class CGenericTags {
public:
    // Constructor inlined in funcs 3831 / 5892: {m_tamanhoTag = 4, m_tamanhoNome = 3}      // ?
    CGenericTags(std::size_t tamanhoTag, std::size_t tamanhoNome)
        : m_tamanhoTag(tamanhoTag), m_tamanhoNome(tamanhoNome) {}

    void insert(const std::string& tag, std::size_t tamanho);                          // func 1071
    std::size_t TagSize(const std::string& tag) const;                                 // func 5468
    std::string EncodeTLV(const std::string& tag, const std::string& valor, std::size_t tamanho) const; // 5467
    void AppendTLV(std::string& saida, const std::string& tag, const std::string& valor,
                   std::size_t tamanho = 0) const;                                      // func 5466
    std::size_t WalkTreeTLV(const std::string& mensagem, const std::string& tag) const; // inlined (5892)
    std::string DecodeTLV(std::string& mensagem, const std::string& tag) const;         // inlined (5892)

    // wasm func 3647 (tools: api_f3647)                                                 name inferred
    std::string EncodeTLV(const std::string& tag, const std::string& valor) const
    {
        return EncodeTLV(tag, valor, TagSize(tag));
    }

private:
    std::size_t m_tamanhoTag;                        // +0  tag field width incl. the length marker (4)
    std::size_t m_tamanhoNome;                       // +4  tag name width (3)
    // +8  tag -> n (bytes of the length). The mapped type is ONE byte: insert() stores it with i32.store8
    // at node+28 (from a pair<string, size_t> temporary) and TagSize() reads it with i32.load8_u offset=28.
    std::map<std::string, uebyte> m_tags;
    // ~map: wasm func 2283 (tools: api_f2283) = std::__tree<__value_type<string, uebyte>>::destroy(node)
};

} // namespace api
