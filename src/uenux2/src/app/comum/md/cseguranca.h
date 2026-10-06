// uenux2/src/app/comum/md/cseguranca.h
// Reconstructed from vota_web_wasm.wasm (unit u24).
//
// md::CSeguranca = the "seguranca" block of an encrypted envelope (EntidadeEnvelopeGenerico.seguranca):
//   Seguranca ::= SEQUENCE { idTipoArquivo INTEGER (0..2), idCriptografia INTEGER (1..3),
//                            idArquivoCD INTEGER (0..255), idArquivoChave OCTET STRING }
// The C++ class has no idArquivoCD: the converter (CConversorSeguranca, 11412/11413) ignores that field
// on reading and leaves it at its default (0) on writing. Not polymorphic. sizeof 16.
#pragma once

#include <vector>

#include "comum/comumtypes.h"          // uebyte   (header name ?)

namespace comum::md {

class CSeguranca {
public:
    CSeguranca(uebyte idTipoArquivo, uebyte idCriptografia, const std::vector<uebyte>& chave);   // wasm 3823

    uebyte GetIdTipoArquivo() const { return m_idTipoArquivo; }
    uebyte GetIdCriptografia() const { return m_idCriptografia; }
    const std::vector<uebyte>& GetChave() const { return m_chave; }

private:
    uebyte              m_idTipoArquivo;    // +0  0..2
    uebyte              m_idCriptografia;   // +1  1..3
    std::vector<uebyte> m_chave;            // +4  idArquivoChave: the CEPESC-ciphered session key (not empty)
};

} // namespace comum::md
