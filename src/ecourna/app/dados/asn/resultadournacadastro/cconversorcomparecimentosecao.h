// ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorcomparecimentosecao.h   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u11).
#pragma once

#include "ecourna/api/asn/iconversorasn.hpp"
#include "ecourna/app/dados/resultadournacadastro/ccomparecimentosecao.h"
#include "ModuloResultadoUrnaCadastro.h"

namespace ecourna::app::dados::asn {

// RTTI: CConversorComparecimentoSecao
//         : IConversorASN<ModuloResultadoUrnaCadastro::ComparecimentoSecao, CComparecimentoSecao>
// vtable @1131064: [0] 174  [1] 144  [2] 9126 DoConverte (unit u40)  [3] 9120 DoDeconverte
//
//   ComparecimentoSecao ::= SEQUENCE { identificacao IdentificacaoSecaoEleitoral,
//                                      eleitores     SEQUENCE OF EstadoComparecimento }
//   CComparecimentoSecao (28 bytes): +0 CIdentificacaoSecaoEleitoral (16, trivially copyable),
//                                    +16 std::vector<CEstadoComparecimento> (96-byte items)
//   constructor func 5092 (const&, const&): copies the vector (func 2278 + exception guard 5096)
//
//   CEstadoComparecimento (96 bytes, from its implicit destructor func 1006 and the move in func 9118):
//     +0  CRegistroIdentificacaoEleitor identificacao (shared_ptr +0/+4, optional<shared_ptr> +8/+12, flag +16)
//     +20 .. +41  situação de comparecimento, optional apresentação da foto, optional habilitação por áudio
//     +44 std::optional<CHabilitacaoBiometrica> (flag +92):
//           +44..+59 tentativas, dedo, erro de leitura, último score
//           +60 std::optional<CEstadoHabilitacaoPorCodigo> (flag +88):
//                 +60 situação do reconhecimento do mesário,
//                 +64 std::optional<CRegistroIdentificacaoEleitor> mesário (flag +84; inner optional flag +80)
class CConversorComparecimentoSecao
    : public api::asn::IConversorASN<ModuloResultadoUrnaCadastro::ComparecimentoSecao, CComparecimentoSecao>
{
protected:
    TEntidade DoConverte(const TDado& dado) const override;          // wasm func 9126 (unit u40)
    TDado DoDeconverte(const TEntidade& entidade) const override;    // wasm func 9120
};

}  // namespace ecourna::app::dados::asn
