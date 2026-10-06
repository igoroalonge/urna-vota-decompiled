// uenux2/src/app/comum/gravadores/cgravadorutil.h
// Reconstructed from vota_web_wasm.wasm (unit u23).
#pragma once

#include <string>

#include "comum/carquivosresultado.h"
#include "comum/comumdefs.h"

namespace comum {

// ASCII-coded phase of the urna, as stored in the result entities (md::CEntidadeBU::m_fase, ...).
enum class EUrnaFase : int { OFICIAL = '1', SIMULADO = '2', TREINAMENTO = '3' };   // names inferred

class CGravadorUtil {
public:
    // wasm func 3798 (name inferred): DeterminaNomeArquivoSemLetra (srcloc :28/:33/:38) + the suffix.
    static std::string DeterminaNomeArquivo(TMunicipioID municipio, TZonaID zona, TSecaoID secao, char fase,
                                            EExtensaoArquivoResultado extensao);
    // inlined into 3798 (srcloc cgravadorutil.cpp:28..38)
    static std::string DeterminaNomeArquivoSemLetra(TMunicipioID municipio, TZonaID zona, TSecaoID secao, char fase);
    // wasm func 2274 (srcloc :97)
    static EUrnaFase ConverteFase(const char fase);
    // inlined in CGravaResultado (srcloc :113, see cgravaresultado.cpp of unit u07)
    static int ConverteFaseEcourna(const char fase);
};

}  // namespace comum
