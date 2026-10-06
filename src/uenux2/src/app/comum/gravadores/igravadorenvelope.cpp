// uenux2/src/app/comum/gravadores/igravadorenvelope.cpp   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u35: only the destructor; GravaResultado = func 11626 is in
// src/uenux2/src/app/comum/u18-foreign-fragments.cpp).
#include "comum/gravadores/igravadorenvelope.h"

namespace comum {

// wasm func 11625 - vtable slot 0 (complete-object destructor). Not observed executing.
// comum_f6043(this, 76, vtable) is a wasm-opt merged body shared with ~CGravadorRDV (func 11586, offset 56):
//     this->vptr = <vtable>; reinterpret_cast<CDadoCorrespondencia*>(this + offset)->~CDadoCorrespondencia();
//     this->vptr = IResultado; m_nome.~string();
// i.e. the compiler-generated destructor:
//     IGravadorEnvelope::~IGravadorEnvelope() = default;   (declared in the header)

IGravadorEnvelope::IGravadorEnvelope(TMunicipioID municipio, TZonaID zona, TLocalID local, TSecaoID secao, char fase,
                                     EExtensaoArquivoResultado extensao, ESavdArquivoUE arquivoSavd,
                                     const api::CDateTime& dataGeracao, EUrnaFase faseEcourna,
                                     md::CEnvelopeGenerico::Tipo tipo,
                                     const md::estadoaplicacao::CDadoCorrespondencia& correspondencia)
    : IGravador(municipio, zona, local, secao, fase, extensao, arquivoSavd),      // func 1396
      m_dataGeracao(dataGeracao),
      m_fase(faseEcourna),
      m_tipoEnvelope(tipo),
      m_correspondencia(correspondencia)                                          // copy ctor func 1249
{
}   // inlined into CGravadorEnvelopeArquivo::CGravadorEnvelopeArquivo (func 5856, unit u07)

} // namespace comum
