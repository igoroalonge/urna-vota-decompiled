// uenux2/src/app/comum/gravadores/iresultado.h + igravador.h (path of IGravador inferred)
// Reconstructed from vota_web_wasm.wasm (unit u23).
//
// IResultado = "one result file of the urna" (name, extension, SAVD file id).
// IGravador  = "a writer (gravador) of one result file": writes it in the MI work area, copies it to the result
//              directories of the MI (memória interna, /dsk/fi) and of the MV (memória de votação, /dsk/fe).
//
// RTTI:  comum::IResultado  (vtable @1541028)
//          └ comum::IGravador (vtable @1553656)
//              ├ CGravadorBU           bu.dat    (this unit)
//              ├ CGravadorRDV          rdv.dat   (unit u21/u05)
//              ├ CGravadorRCSecao      jufa.dat  (this unit)
//              ├ CGravadorHashes       hash.dat  (this unit)
//              ├ CGravadorWSQ          wsq*.jez  (this unit + u12 fragment)
//              ├ CGravadorLog          log.jez   (u12)
//              ├ CGravadorVersoesArquivos mr.ver (u07/u21)
//              └ IGravadorEnvelope ─ CGravadorEnvelopeArquivo  imgbu.dat / imgze.dat
#pragma once

#include <string>

#include "comum/carquivosresultado.h"     // EExtensaoArquivoResultado
#include "comum/carquivossavd.h"          // ESavdArquivoUE
#include "comum/comumdefs.h"              // TMunicipioID, TZonaID, TLocalID, TSecaoID, CUeComumError
#include "ecourna/api/exception/cbaseerror.h"

namespace api { class CFile; }

namespace comum {

// Error class of the whole comum/gravadores subsystem (codes 8601..8698 are used; vtable @1552984,
// typeinfo @1552964). comum_f283 (wasm func 283) is its merged constructor thunk:
// ecourna_f710(exc, code, msg, srcloc, vtable).
enum class EUeComumGravadoresError : int;
using CUeComumGravadoresError =
    ecourna::api::exception::CBaseError<EUeComumGravadoresError, ecourna::api::exception::SErrorLimits{8600, 8800}>;

class IResultado {
public:
    // The only out-of-line constructor is IGravador's (wasm func 1396), which inlines this one.
    IResultado(TMunicipioID municipio, TZonaID zona, TLocalID local, TSecaoID secao, char fase,
               EExtensaoArquivoResultado extensao, ESavdArquivoUE arquivoSavd);          // srcloc iresultado.cpp:40
    virtual ~IResultado();                                                               // wasm func 12090 (slot 0)

    const std::string& GetNome() const { return m_nome; }
    EExtensaoArquivoResultado GetExtensao() const { return m_extensao; }
    ESavdArquivoUE GetArquivoSavd() const { return m_arquivoSavd; }

    virtual void CopiaParaResultado() const = 0;     // slot 2   (pure here, implemented by IGravador)
    virtual void CopiaParaMV() const = 0;            // slot 3

protected:
    TMunicipioID m_municipio;                // +4
    TZonaID m_zona;                          // +8
    TLocalID m_local;                        // +12
    TSecaoID m_secao;                        // +16
    char m_fase;                             // +18   'o' oficial / 's' simulado / 't' treinamento
    std::string m_nome;                      // +20   "<fase><pleito:05><uf><mun:05><zona:04><secao:04>-<sufixo>"
    EExtensaoArquivoResultado m_extensao;    // +32
    ESavdArquivoUE m_arquivoSavd;            // +36
};

// Slot numbers are the wasm vtable slots (0/1 = destructors).
class IGravador : public IResultado {
public:
    IGravador(TMunicipioID municipio, TZonaID zona, TLocalID local, TSecaoID secao, char fase,
              EExtensaoArquivoResultado extensao, ESavdArquivoUE arquivoSavd);          // wasm func 1396

    void CopiaParaResultado() const override;        // slot 2  func 11632: trab(MI)/nome -> resultado(MI)/nome
    void CopiaParaMV() const override;               // slot 3  func 11630: trab(MI)/nome -> trab(MV)/nome
    virtual void Grava() const;                      // slot 4  func 11634: open trab(MI)/nome "wb", GravaResultado()
    virtual void GravaMV() const;                    // slot 5  func 11633: open trab(MV)/nome "wb", GravaResultado()
    virtual void CopiaResultadoParaMV() const;       // slot 6  func 11631: trab(MV)/nome -> resultado(MV)/nome
    virtual void GravaResultado(api::CFile& arquivo) const = 0;   // slot 7
    // (funcs 11630..11634 belong to igravador.cpp, outside this unit; names from behaviour, see u07)
};

}  // namespace comum
