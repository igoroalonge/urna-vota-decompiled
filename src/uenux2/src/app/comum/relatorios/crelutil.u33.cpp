// uenux2/src/app/comum/relatorios/crelutil.cpp  -- FRAGMENT written by unit u33 (srcloc crelutil.cpp exists;
// the file is reconstructed by unit u25). Class membership inferred from the caller
// CRelUtil::IncluiResumoCorrespondencia (func 1541) and from GetIDCargaFormatado (func 2786), which formats
// the whole code the same way.                                                             // ?
#include <string>

#include "comum/md/estadoaplicacao/cdadocorrespondencia.h"
#include "comum/relatorios/crelutil.h"

namespace comum {

// wasm func 2792 (observed executing: one profiler sample in the general-election session, called from the
// CTelasVota blob 7787, where it feeds the "RESUMO DA CORRESPONDÊNCIA: " line of the "O horário de emissão da
// zerésima passou" screen, ctelasvota.cpp:2382)                                      // name inferred (u15)
// "RESUMO DA CORRESPONDÊNCIA": the last six digits of the 24-digit código de carga (codigoCarga, the load
// identifier of the urna's media, correspondência +28), grouped as "DDD.DDD".
//   codigoCarga "537864991559016480376254" -> "376.254"   (the example BU of docs/10-boletim-de-urna.md)
//   simulator fixture "123456789012345678901234" -> "901.234"
// Unlike GetIDCargaFormatado there is no explicit length check: a code shorter than 21 characters makes
// std::string::substr throw std::out_of_range (CCarga::ValidaCriacao already enforces 24 digits, u05).
// Callers: CRelUtil::IncluiResumoCorrespondencia (1541: BU, zerésima and other printed reports),
// vota's "Estado da urna" report (5591), the zerésima-time screen block (3059, "RESUMO DA CORRESPONDÊNCIA: ")
// and the CTelasVota constructor blob (7787).
std::string CRelUtil::FormataCorrespondencia(const md::estadoaplicacao::CDadoCorrespondencia& correspondencia)
{
    const std::string& codigo = correspondencia.GetCodigoCarga();      // +28
    return codigo.substr(18, 3) + "." + codigo.substr(21);
}

} // namespace comum
