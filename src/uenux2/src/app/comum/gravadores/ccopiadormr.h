// uenux2/src/app/comum/gravadores/ccopiadormr.h   (path inferred: RTTI only; included under this name by
// vota/eleitor/fimvotacao/ccopiaresultadoparamr.cpp, unit u08)
// Reconstructed from vota_web_wasm.wasm (unit u35).
//
// "Copiador MR" copies ONE result file to the MR ("mídia de resultado", the USB stick taken to the electoral
// office: /dsk/mr/, CPath::GetPathMR = func 949). It is the last step of the encerramento
// (vota::CCopiaResultadoParaMR::StartState, func 12134): one copier per result file (bu.dat, rdv.dat, jufa.dat,
// imgbu.dat, imgze.dat, hash.dat, log.jez, vota.vsc, mr.ver, and on biometric urnas the three wsq*.jez through
// CCopiadorWSQMR).
//
// RTTI:
//   comum::CCopiadorMR (class, vtable @1553348, 16 bytes)
//     [0] 11638 ~CCopiadorMR   [1] 5872 deleting dtor   [2] 5874 Copia()   [3] 5873 CopiaDaMV()
//   comum::CCopiadorWSQMR : CCopiadorMR (vtable @1553380): [0] 11638 through the SAME table slot 2175 (complete
//     destructor emitted as an alias of the base one), [1..3] = the same functions 5872/5874/5873 reached through
//     DIFFERENT table slots (2179..2181 instead of 2176..2178). At link time those were therefore distinct functions:
//     the per-class deleting destructor (always emitted, no evidence of user code) and CCopiadorWSQMR's overrides of
//     Copia and CopiaDaMV, whose bodies became byte-identical to the base ones and were folded by wasm-opt: in this
//     build the WSQ copier behaves exactly like the base class.
#pragma once

#include <string>

#include "comum/carquivosresultado.h"     // EExtensaoArquivoResultado
#include "comum/comumdefs.h"              // TMunicipioID, TZonaID, TSecaoID

namespace comum {

class CCopiadorMR
{
public:
    // wasm func 1276 (tools: vota_f1276, reconstructed in ccopiaresultadoparamr.cpp by unit u08)
    CCopiadorMR(TMunicipioID municipio, TZonaID zona, TSecaoID secao, char fase, EExtensaoArquivoResultado tipo);
    virtual ~CCopiadorMR();                          // slot 0 func 11638, slot 1 func 5872

    virtual void Copia() const;                      // slot 2 func 5874: resultado(MI)/nome -> MR/nome   name inferred
    virtual void CopiaDaMV() const;                  // slot 3 func 5873: resultado(MV)/nome -> MR/nome   name inferred

protected:
    std::string m_nome;                              // +4  "<fase><pleito:05><uf><mun:05><zona:04><seção:04>-<sufixo>"
};

class CCopiadorWSQMR : public CCopiadorMR
{
public:
    using CCopiadorMR::CCopiadorMR;                  // wasm func 3827 (unit u08): base ctor + vptr @1553380
    // Overrides folded into the base bodies (see above).
    void Copia() const override;                     // table slot 2180 -> func 5874
    void CopiaDaMV() const override;                 // table slot 2181 -> func 5873
};

} // namespace comum
