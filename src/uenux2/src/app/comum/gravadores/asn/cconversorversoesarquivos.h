// uenux2/src/app/comum/gravadores/asn/cconversorversoesarquivos.h   (path inferred: md class in
// gravadores/md/cversoesarquivos.cpp)
// Reconstructed from vota_web_wasm.wasm (unit u21).
#pragma once

#include "comum/asn/iconversorasn.h"
#include "comum/gravadores/md/cversoesarquivos.h"   // md::CVersoesArquivos: +0 std::string versaoTag,
                                                    //   +12 std::map<std::string, std::string> arquivos (nome -> assinatura)
#include "ModuloVersaoArquivos.h"

namespace comum::asn {

// EntidadeVersaoArquivos ::= SEQUENCE { versaoTag GeneralString, arquivos SEQUENCE OF ArquivoAssinatura }
// Version manifest of the files of a medium; the only user is comum::CGravadorVersoesArquivos (vtable slot 7,
// wasm 11582), which builds this converter on its stack and writes the entity.
// RTTI: CConversorVersoesArquivos
//         : IConversorASN<ModuloVersaoArquivos::EntidadeVersaoArquivos, md::CVersoesArquivos>
// vtable @1599296: [0] 174 [1] 144 [2] 10264 DoConverte [3] 10263 DoDesconverte (other unit). sizeof 4.
class CConversorVersoesArquivos
    : public IConversorASN<ModuloVersaoArquivos::EntidadeVersaoArquivos, md::CVersoesArquivos>
{
protected:
    TEntidade DoConverte(const TDado& versoes) const override;      // wasm func 10264
    TDado DoDesconverte(const TEntidade& versoes) const override;   // wasm func 10263 (other unit)
};

} // namespace comum::asn
