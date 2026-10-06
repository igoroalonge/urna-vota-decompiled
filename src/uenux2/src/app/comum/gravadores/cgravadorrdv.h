// uenux2/src/app/comum/gravadores/cgravadorrdv.h   (path inferred: class known from RTTI only; every sibling writer
// has a srcloc in uenux2/src/app/comum/gravadores/)
// Reconstructed from vota_web_wasm.wasm (unit u35).
//
// CGravadorRDV writes "<prefixo>-rdv.dat" (EExtensaoArquivoResultado 6, SAVD file 37): the RDV ("registro digital
// do voto", the table of every vote cast, kept sorted so that the voting order is lost) wrapped in
// ModuloRegistroDigitalVoto::EntidadeResultadoRDV {cabecalho, urna, rdv}. GravaResultado (slot 7, func 11587) is
// reconstructed in src/uenux2/src/app/comum/u18-foreign-fragments.cpp (unit u18); the constructor is inlined into
// vota::CGravaResultado::StartState (func 12098, unit u07).
//
// RTTI: comum::CGravadorRDV : comum::IGravador, vtable @1556856, 168 bytes
//   [0] 11586 ~CGravadorRDV   [1] 11585 deleting dtor   [2..6] IGravador (igravador.cpp)   [7] 11587 GravaResultado
#pragma once

#include <optional>

#include "api/util/cdatetime.h"
#include "comum/dados/crdvvota.h"
#include "comum/dados/md/estadoaplicacao/cdadocorrespondencia.h"
#include "comum/gravadores/iresultado.h"

namespace comum {

class CGravadorRDV : public IGravador
{
public:
    CGravadorRDV(TMunicipioID municipio, TZonaID zona, TLocalID local, TSecaoID secao, char fase,
                 const api::CDateTime& dataGeracao,
                 const md::estadoaplicacao::CDadoCorrespondencia& correspondencia,
                 const CRdvVota& rdv)                                            // inlined in 12098
        : IGravador(municipio, zona, local, secao, fase, EExtensaoArquivoResultado(6) /*rdv.dat*/,
                    ESavdArquivoUE(37)),
          m_dataGeracao(dataGeracao),
          m_correspondencia(correspondencia),
          m_rdv(&rdv)
    {
    }
    ~CGravadorRDV() override;                                                    // func 11586 (+ 11585)

    void GravaResultado(ecourna::api::io::CFile& arquivo) const override;        // func 11587 (unit u18)

private:
    // IResultado / IGravador: +0 .. +39
    api::CDateTime m_dataGeracao;                                  // +40
    char m_tipoArquivo = '1';                                      // +52
    md::estadoaplicacao::CDadoCorrespondencia m_correspondencia;   // +56 (96 bytes)
    std::optional<int> m_motivoUtilizacaoSA;                       // +152 (engaged flag +160; empty for VOTA) ?
    const CRdvVota* m_rdv;                                         // +164
};

} // namespace comum
