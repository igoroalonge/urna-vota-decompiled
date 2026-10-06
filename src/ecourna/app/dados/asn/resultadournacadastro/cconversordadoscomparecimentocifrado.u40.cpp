// FRAGMENT of ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversordadoscomparecimentocifrado.cpp
// (path inferred; class + DoDeconverte by unit u11). Reconstructed by unit u40 from vota_web_wasm.wasm.
#include "ecourna/app/dados/asn/resultadournacadastro/cconversordadoscifracao.h"
// class CConversorDadosComparecimentoCifrado is declared at the top of u11's cconversordadoscomparecimentocifrado.cpp

namespace ecourna::app::dados::asn {

// wasm func 9063: by-value helper std::vector<uebyte> -> ASN1::OCTET_STRING (not a constructor), defined in
// cconversordadoscifracao.u40.cpp; its two callers are 9060 and 9062.                  // name inferred
ASN1::OCTET_STRING ConverteOctetString(const std::vector<uebyte>& bytes);

// wasm func 9060 (vtable @1137068 slot 2). Used when criptografarJUFA is set: comum::CGravadorRCSecao encrypts
// the BER of DadosComparecimento with CEPESC and stores the result as DadosComparecimentoCifrado.
//   DadosComparecimentoCifrado ::= SEQUENCE { dadosCifracao DadosCifracao, conteudo OCTET STRING }
ModuloResultadoUrnaCadastro::DadosComparecimentoCifrado
CConversorDadosComparecimentoCifrado::DoConverte(const CDadosComparecimentoCifrado& dado) const
{
    const CConversorDadosCifracao conversorDadosCifracao;                                   // vptr @1136812
    ModuloResultadoUrnaCadastro::DadosComparecimentoCifrado entidade;
    entidade.set_dadosCifracao(conversorDadosCifracao.Converte(dado.GetDadosCifracao()));   // 9059 -> 9062
    entidade.set_conteudo(ConverteOctetString(dado.GetConteudo()));                        // 9063, assign 1158
    return entidade;
}

}  // namespace ecourna::app::dados::asn
