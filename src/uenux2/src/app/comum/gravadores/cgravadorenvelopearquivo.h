// uenux2/src/app/comum/gravadores/cgravadorenvelopearquivo.h   (path inferred: class known from RTTI only)
// Reconstructed from vota_web_wasm.wasm (unit u35).
//
// RTTI: comum::CGravadorEnvelopeArquivo : comum::IGravadorEnvelope, vtable @1554512, 184 bytes
//   [0] 11622 ~CGravadorEnvelopeArquivo   [1] 11621 deleting dtor   [2..6] IGravador   [7] 11626 (IGravadorEnvelope)
//   [8] 11623 LeConteudo  (src/uenux2/src/app/comum/gravadores/u12-foreign-fragments.cpp, unit u12)
// Constructor: func 5856 (reconstructed at the end of vota/eleitor/fimvotacao/cgravaresultado.cpp, unit u07).
// Built twice by vota::CGravaResultado: imgbu.dat (extensão 9, SAVD 43, trab/bu.dat) and imgze.dat (extensão 11,
// SAVD 44, trab/ze.dat).
#pragma once

#include <string>
#include <vector>

#include "comum/gravadores/igravadorenvelope.h"

namespace comum {

class CGravadorEnvelopeArquivo : public IGravadorEnvelope
{
public:
    CGravadorEnvelopeArquivo(const api::CDateTime& dhGeracao, TMunicipioID municipio, TZonaID zona, TLocalID local,
                             TSecaoID secao, char fase, EExtensaoArquivoResultado extensao, ESavdArquivoUE savd,
                             md::CEnvelopeGenerico::Tipo tipo,
                             const md::estadoaplicacao::CDadoCorrespondencia& correspondencia,
                             const std::string& arquivo);                              // func 5856 (u07)
    ~CGravadorEnvelopeArquivo() override;                                              // func 11622 (+ 11621)

protected:
    std::vector<uebyte> LeConteudo() const override;                                   // func 11623 (u12)

private:
    std::string m_arquivo;                                                             // +172 file to wrap
};

} // namespace comum
