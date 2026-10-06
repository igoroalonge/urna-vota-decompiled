// uenux2/src/app/comum/gravadores/igravadorenvelope.h   (path inferred: class known from RTTI only; siblings of
// comum/gravadores/)
// Reconstructed from vota_web_wasm.wasm (unit u35). Constructor: inlined into CGravadorEnvelopeArquivo's (func 5856,
// unit u07). GravaResultado (slot 7, func 11626): src/uenux2/src/app/comum/u18-foreign-fragments.cpp (unit u18).
//
// IGravadorEnvelope writes a result file that is an ASN.1 ModuloEnvelopeGenerico::EntidadeEnvelopeGenerico: a
// header (cabeçalho + urna identification + correspondência) wrapping an opaque byte blob returned by slot 8.
// The only subclass, CGravadorEnvelopeArquivo, wraps the bytes of a file: the printer image of the BU
// ("<prefixo>-imgbu.dat", from trab/bu.dat) and of the zerésima ("<prefixo>-imgze.dat", from trab/ze.dat).
//
// RTTI: comum::IGravadorEnvelope : comum::IGravador, vtable @1554456
//   [0] 11625 ~IGravadorEnvelope   [1] icf_tiny_vf1@325 (deleting, shared)   [2..6] IGravador (igravador.cpp)
//   [7] 11626 GravaResultado       [8] pure LeConteudo
#pragma once

#include <optional>
#include <vector>

#include "api/util/cdatetime.h"
#include "comum/dados/md/estadoaplicacao/cdadocorrespondencia.h"
#include "comum/gravadores/iresultado.h"
#include "comum/gravadores/md/cenvelopegenerico.h"   // md::CEnvelopeGenerico::Tipo

namespace comum {

class IGravadorEnvelope : public IGravador
{
public:
    // wasm func 11625 - vtable slot 0. Body = merged helper comum_f6043(this, 76, vtable @1554456):
    // vptr = IGravadorEnvelope; ~CDadoCorrespondencia (func 857) on this+76; vptr = IResultado; ~m_nome.
    ~IGravadorEnvelope() override = default;

    void GravaResultado(ecourna::api::io::CFile& arquivo) const override;   // slot 7, func 11626 (unit u18)

protected:
    IGravadorEnvelope(TMunicipioID municipio, TZonaID zona, TLocalID local, TSecaoID secao, char fase,
                      EExtensaoArquivoResultado extensao, ESavdArquivoUE arquivoSavd,
                      const api::CDateTime& dataGeracao, EUrnaFase faseEcourna, md::CEnvelopeGenerico::Tipo tipo,
                      const md::estadoaplicacao::CDadoCorrespondencia& correspondencia);   // inlined in 5856

    virtual std::vector<uebyte> LeConteudo() const = 0;                     // slot 8

    // IResultado / IGravador: +0 .. +39
    api::CDateTime m_dataGeracao;                                           // +40
    EUrnaFase m_fase;                                                       // +52 CGravadorUtil::ConverteFase(fase)
    char m_tipoArquivo = '1';                                               // +56 (md::CUrna tipo de arquivo)
    std::optional<int> m_motivoUtilizacaoSA;                                // +60 (engaged +68; always empty) ?
    md::CEnvelopeGenerico::Tipo m_tipoEnvelope;                             // +72 2 = BU impresso, 4 = zerésima
    md::estadoaplicacao::CDadoCorrespondencia m_correspondencia;            // +76 (96 bytes)
};

} // namespace comum
