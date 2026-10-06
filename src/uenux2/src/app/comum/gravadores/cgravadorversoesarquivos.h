// uenux2/src/app/comum/gravadores/cgravadorversoesarquivos.h   (path inferred: every sibling writer -
// cgravadorbu, cgravadorhashes, cgravadorrcsecao, cgravadorwsq - has a srcloc in uenux2/src/app/comum/gravadores/)
//
// Reconstructed from vota_web_wasm.wasm (unit u33).
//
// comum::CGravadorVersoesArquivos : comum::IGravador    typeinfo @1557280, vtable @1557248, 64 bytes.
// The writer of the result file "mr.ver": the version of every ASN.1 contract (module) used by the result
// files that the urna writes at the encerramento (end of voting). Built by vota::CGravaResultado::StartState
// (func 12098, unit u07/u09) as the 10th writer:
//     IResultado(município, zona, local, seção, fase, EExtensaoArquivoResultado 19 ("mr.ver"), ESavdArquivoUE 70)
//     m_versoes = md::CVersoesArquivos(TAG_CONTRATOS "20260601173148", {módulo -> versão})
// Vtable: [0] 11581 ~CGravadorVersoesArquivos  [1] 11580 deleting  [2..6] IGravador (11632, 11630, 11634,
//         11633, 11631)  [7] 11582 GravaResultado (this unit).
#pragma once

#include "comum/gravadores/iresultado.h"          // comum::IGravador (unit u23)
#include "comum/gravadores/md/cversoesarquivos.h" // comum::md::CVersoesArquivos (units u21/u23)

namespace comum {

class CGravadorVersoesArquivos : public IGravador {
public:
    // Inlined into CGravaResultado::StartState (func 12098).
    CGravadorVersoesArquivos(TMunicipioID municipio, TZonaID zona, TLocalID local, TSecaoID secao, char fase,
                             const md::CVersoesArquivos& versoes)
        : IGravador(municipio, zona, local, secao, fase, EExtensaoArquivoResultado(19) /* mr.ver */,
                    ESavdArquivoUE(70)),
          m_versoes(versoes)
    {
    }
    ~CGravadorVersoesArquivos() override = default;   // wasm funcs 11581 / 11580 (map +52, tag +40, IResultado)

    // slot 7 - wasm func 11582
    void GravaResultado(api::CFile& arquivo) const override;

private:
    md::CVersoesArquivos m_versoes;   // +40  (+40 std::string m_versaoTag, +52 std::map<std::string, std::string>)
};

} // namespace comum
