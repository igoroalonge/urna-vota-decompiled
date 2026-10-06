// ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversordadoscifracao.h   (path inferred:
//   sibling converters of ModuloResultadoUrnaCadastro live in asn/resultadournacadastro/, cf. srcloc of
//   cconversorhabilitacaobiometrica.cpp; the data class is resultadournacadastro/cdadoscifracao.cpp)
// Reconstructed from vota_web_wasm.wasm (unit u11).
#pragma once

#include <vector>

#include "ecourna/api/asn/iconversorasn.hpp"
#include "ecourna/app/dados/resultadournacadastro/cdadoscifracao.h"
#include "ModuloResultadoUrnaCadastro.h"

namespace ecourna::app::dados::asn {

// wasm func 5095 - name and location inferred. OCTET STRING -> byte vector. It exists out of line and
// is called from two converters (9057, 9061) that are probably different translation units, so it is
// an inline function of a shared header. It receives the OCTET_STRING itself (the std::vector<char> of
// the III runtime sits at +8) and builds a new vector with one allocation + memcpy.
inline std::vector<uebyte> ConverteOctetString(const ASN1::OCTET_STRING& octetos)   // name inferred
{
    return std::vector<uebyte>(octetos.begin(), octetos.end());
}

// RTTI: CConversorDadosCifracao : IConversorASN<ModuloResultadoUrnaCadastro::DadosCifracao, CDadosCifracao>
// vtable @1136812: [0] 174  [1] 144  [2] 9062 DoConverte (unit u40)  [3] 9061 DoDeconverte
//
//   DadosCifracao ::= SEQUENCE { chave OCTET STRING, salt OCTET STRING, informacaoAdicional OCTET STRING }
//   CDadosCifracao (36 bytes): +0 vector<uebyte> chave, +12 salt, +24 informacaoAdicional
//   (constructor func 5091, cdadoscifracao.cpp:27/32: salt and informacaoAdicional must have >= 16 bytes)
class CConversorDadosCifracao
    : public api::asn::IConversorASN<ModuloResultadoUrnaCadastro::DadosCifracao, CDadosCifracao>
{
protected:
    TEntidade DoConverte(const TDado& dado) const override;          // wasm func 9062 (unit u40)
    TDado DoDeconverte(const TEntidade& entidade) const override;    // wasm func 9061
};

}  // namespace ecourna::app::dados::asn
