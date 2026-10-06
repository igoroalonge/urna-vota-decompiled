// uenux2/src/app/comum/gravadores/iresultado.cpp
// Reconstructed from vota_web_wasm.wasm (unit u23).
#include "comum/gravadores/iresultado.h"

#include <format>

#include "comum/gravadores/cgravadorutil.h"

namespace comum {

// wasm func 1396 (srcloc iresultado.cpp:40 for the inlined IResultado constructor).
// The tools named it IResultado::IResultado; the function ends by storing the IGravador vptr (@1553656),
// so the out-of-line symbol is IGravador's constructor with IResultado's inlined into it.
// Callers: CGravaResultado::StartState (12098, every IGravador subclass constructor is inlined there),
// the CGravadorWSQ constructor (3796) and the CGravadorEnvelopeArquivo constructor (5856).
IResultado::IResultado(TMunicipioID municipio, TZonaID zona, TLocalID local, TSecaoID secao, char fase,
                       EExtensaoArquivoResultado extensao, ESavdArquivoUE arquivoSavd)
    : m_municipio(municipio),
      m_zona(zona),
      m_local(local),
      m_secao(secao),
      m_fase(fase),
      // NOTE: the name is computed BEFORE the fase check below (member initialiser order); an invalid fase
      // character is first formatted into the name with "{:c}".
      m_nome(CGravadorUtil::DeterminaNomeArquivo(municipio, zona, secao, fase, extensao)),   // func 3798
      m_extensao(extensao),
      m_arquivoSavd(arquivoSavd)
{
    if (!(fase == 'o' || fase == 's' || fase == 't'))
        throw CUeComumGravadoresError(8657, std::format("Fase inválida: {}", fase));          // line 40
}

IGravador::IGravador(TMunicipioID municipio, TZonaID zona, TLocalID local, TSecaoID secao, char fase,
                     EExtensaoArquivoResultado extensao, ESavdArquivoUE arquivoSavd)
    : IResultado(municipio, zona, local, secao, fase, extensao, arquivoSavd)
{
}

// wasm func 12090 (vtable slot 0 of IResultado, IGravador and CGravadorRCSecao: complete-object destructor).
// CGravadorRCSecao adds only trivially destructible members, so its destructor folded into this one.
IResultado::~IResultado() = default;     // destroys m_nome (+20)

}  // namespace comum
