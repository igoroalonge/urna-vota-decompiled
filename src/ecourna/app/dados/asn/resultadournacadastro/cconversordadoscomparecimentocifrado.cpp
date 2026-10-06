// ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversordadoscomparecimentocifrado.cpp
//   (path inferred, next to its siblings in asn/resultadournacadastro/)
// Reconstructed from vota_web_wasm.wasm (unit u11). Only DoDeconverte is in this unit.
//
// RTTI: CConversorDadosComparecimentoCifrado
//         : IConversorASN<ModuloResultadoUrnaCadastro::DadosComparecimentoCifrado, CDadosComparecimentoCifrado>
// vtable @1137068: [0] 174  [1] 144  [2] 9060 DoConverte (unit u40)  [3] 9057 DoDeconverte
//
//   DadosComparecimentoCifrado ::= SEQUENCE { dadosCifracao DadosCifracao, conteudo OCTET STRING }
//   CDadosComparecimentoCifrado (48 bytes): +0 CDadosCifracao dados (36), +36 vector<uebyte> conteudo
//   constructor func 3484: CDadosComparecimentoCifrado(CDadosCifracao, std::vector<uebyte>) - by value, moved in
#include "ecourna/app/dados/asn/resultadournacadastro/cconversordadoscifracao.h"
#include "ecourna/app/dados/resultadournacadastro/cdadoscifracao.h"   // also declares CDadosComparecimentoCifrado (u14)

namespace ecourna::app::dados::asn {

class CConversorDadosComparecimentoCifrado
    : public api::asn::IConversorASN<ModuloResultadoUrnaCadastro::DadosComparecimentoCifrado,
                                     CDadosComparecimentoCifrado>
{
protected:
    TEntidade DoConverte(const TDado& dado) const override;          // wasm func 9060 (unit u40)
    TDado DoDeconverte(const TEntidade& entidade) const override;    // wasm func 9057
};

// wasm func 9057 (vtable slot 3). The tool named it "IConversorASN<DadosCifracao, CDadosCifracao>::
// Deconverte" because that call is inlined here and carries the srcloc (iconversorasn.hpp:66, record
// @1137420); the vtable slot decides the real name. Not observed at run time: it is only reached when an
// EntidadeResultadoUrnaCadastro holding the encrypted variant is READ back.
CDadosComparecimentoCifrado CConversorDadosComparecimentoCifrado::DoDeconverte(const TEntidade& entidade) const
{
    const CConversorDadosCifracao conversorDadosCifracao;   // stack object, vptr @1136812

    // Inlined IConversorASN<DadosCifracao>::Deconverte: validates field 0, throws 1902 otherwise,
    // then calls CConversorDadosCifracao::DoDeconverte (func 9061) through the vtable.
    const CDadosCifracao dadosCifracao = conversorDadosCifracao.Deconverte(entidade.get_dadosCifracao());
    const std::vector<uebyte> conteudo = ConverteOctetString(entidade.get_conteudo());   // func 5095

    // Both arguments are copied (func 9054 = CDadosCifracao copy constructor, vector copy inlined)
    // and moved into the object by func 3484.
    return CDadosComparecimentoCifrado(dadosCifracao, conteudo);
}

}  // namespace ecourna::app::dados::asn
