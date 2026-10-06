// uenux2/src/app/comum/md/cidentificacaournacontingencia.cpp
// Reconstructed from vota_web_wasm.wasm (unit u36).
//
// Evidence: the srcloc record @1552708 names this file, line 32 column 19, function
//   comum::md::CIdentificacaoUrnaContingencia::CIdentificacaoUrnaContingencia(const EUrnaTipo,
//                                                                          const TMunicipioID, const TZonaID)
// but NO code references that record, and the message "Tipo inválido para urna de contingência: {}"
// (@6328) is not referenced either. Both belong to the type check below, which the optimiser deleted:
// the two callers always pass EUrnaTipo '2', so after inlining/IPSCCP the condition is constant-false.
// What is left of it in wasm func 5888 is a 480-byte stack frame that is never used (the std::format
// buffer of the dead throw). (Tool note: the analyzer's "str 6328" annotations in comum_f5168 and
// CGravadorRCSecao are false positives - there 6328 is the table slot of CPlainText's constructor.)
#include "comum/md/cidentificacaournacontingencia.h"

#include <format>

#include "comum/md/cabrangencia.h"      // CUeComumMdError (error family of comum/md)   ?

namespace comum::md {

// wasm func 5888 (srcloc line 32, dead)                                          // name from the srcloc
// Callers: comum::asn::CConversorLocal::DoDesconverte (11452; ModuloLocal "identificacaoContingencia"
// branch of the -lo.dat file) and vota::CGravaResultado::StartState (12098; a contingency urna identifies
// its result files by município/zona only).
CIdentificacaoUrnaContingencia::CIdentificacaoUrnaContingencia(const EUrnaTipo tipo, const TMunicipioID municipio,
                                                               const TZonaID zona)
    : CIdentificacaoUrna(tipo, municipio, zona)                    // wasm 5886: validates tipo/município/zona
{
    if (tipo != EUrnaTipo{'2'})                                                                 // ? exact test
        throw CUeComumMdError(EUeComumMdError{} /* code not recoverable */,                     // ?
                              std::format("Tipo inválido para urna de contingência: {}",
                                          static_cast<char>(tipo)));                            // line 32
}

}  // namespace comum::md
