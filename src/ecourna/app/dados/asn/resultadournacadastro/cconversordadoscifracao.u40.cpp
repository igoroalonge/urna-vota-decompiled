// FRAGMENT of ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversordadoscifracao.cpp (path inferred;
// class declared by unit u11 in cconversordadoscifracao.h). Reconstructed by unit u40 from vota_web_wasm.wasm.
#include "ecourna/app/dados/asn/resultadournacadastro/cconversordadoscifracao.h"

#include <vector>

namespace ecourna::app::dados::asn {

// wasm func 9063 (table slot 7007). NOT an OCTET_STRING constructor: its type is (i32, i32) -> void and both
// callers reach it through invoke_vii, whereas every C++ constructor of this build returns `this` (wasm C++ ABI)
// and would be (i32, i32) -> i32 / invoke_iii (a table function's type cannot be changed by wasm-opt). It is a
// function that RETURNS an ASN1::OCTET_STRING by value (sret in the first argument), built in place from the byte
// vector: AbstractData(&OCTET_STRING::theInfo @1148948), vptr @1149080, empty std::vector<char> at +8, then
// insert(begin, first, last) (shared_f1927). Its only callers are 9062 (below) and 9060
// (CConversorDadosComparecimentoCifrado::DoConverte), so it is a helper shared by the two resultadournacadastro
// converters (probably declared in a common header).                                   // name and home inferred
ASN1::OCTET_STRING ConverteOctetString(const std::vector<uebyte>& bytes)
{
    return ASN1::OCTET_STRING(bytes.begin(), bytes.end());
}

// wasm func 9062 (vtable @1136812 slot 2)
//   DadosCifracao ::= SEQUENCE { chave OCTET STRING, salt OCTET STRING, informacaoAdicional OCTET STRING }
// Each vector is turned into a temporary ASN1::OCTET_STRING by the helper above (func 9063) whose bytes are then
// assigned into the field (shared_f1158 = vector<char>::assign(first, last, n)); no size check here (the >= 16
// bytes rule for salt/info is enforced by the CDadosCifracao constructor, func 5091).
ModuloResultadoUrnaCadastro::DadosCifracao CConversorDadosCifracao::DoConverte(const CDadosCifracao& dado) const
{
    ModuloResultadoUrnaCadastro::DadosCifracao entidade;
    entidade.set_chave(ConverteOctetString(dado.GetChave()));                                // +0
    entidade.set_salt(ConverteOctetString(dado.GetSalt()));                                  // +12
    entidade.set_informacaoAdicional(ConverteOctetString(dado.GetInformacaoAdicional()));    // +24
    return entidade;
}

}  // namespace ecourna::app::dados::asn
