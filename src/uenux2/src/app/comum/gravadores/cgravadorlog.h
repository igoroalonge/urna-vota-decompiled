// uenux2/src/app/comum/gravadores/cgravadorlog.h   (path inferred: class known from RTTI only)
// Reconstructed from vota_web_wasm.wasm (unit u35).
//
// CGravadorLog writes "<prefixo>-log.jez" (EExtensaoArquivoResultado 13, SAVD file 60): a ZIP (".jez") of the
// application log dinamico/log/logd.dat plus every archived log of dinamico/log/arquivados/. GravaResultado
// (slot 7, func 11584) is reconstructed in src/uenux2/src/app/comum/gravadores/u12-foreign-fragments.cpp (unit u12):
// it ignores the CFile opened by IGravador::Grava, writes <trab>/temp.jez with CZip and renames it.
//
// RTTI: comum::CGravadorLog : comum::IGravador, vtable @1557196, 76 bytes
//   [0] 5823 ~CGravadorLog   [1] 11583 deleting dtor   [2..6] IGravador (igravador.cpp)   [7] 11584 GravaResultado
#pragma once

#include <string>

#include "comum/gravadores/iresultado.h"

namespace comum {

class CGravadorLog : public IGravador
{
public:
    CGravadorLog(TMunicipioID municipio, TZonaID zona, TLocalID local, TSecaoID secao, char fase,
                 const std::string& diretorioArquivados, const std::string& arquivoLog,
                 const std::string& diretorioTrabalho)                              // inlined in 12098 (u07)
        : IGravador(municipio, zona, local, secao, fase, EExtensaoArquivoResultado(13) /*log.jez*/,
                    ESavdArquivoUE(60)),
          m_diretorioArquivados(diretorioArquivados),
          m_arquivoLog(arquivoLog),
          m_diretorioTrabalho(diretorioTrabalho)
    {
    }
    ~CGravadorLog() override;                                                     // func 5823 (+ 11583)

    void GravaResultado(ecourna::api::io::CFile& arquivo) const override;         // func 11584 (unit u12)

private:
    std::string m_diretorioArquivados;   // +40  "<root>dinamico/log/arquivados/"
    std::string m_arquivoLog;            // +52  "<root>dinamico/log/logd.dat"
    std::string m_diretorioTrabalho;     // +64  CPath::GetPathTrab(MI)
};

} // namespace comum
