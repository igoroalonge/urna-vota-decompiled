// uenux2/src/app/comum/carquivosresultado.cpp
// Reconstructed from vota_web_wasm.wasm (unit u21).
#include "comum/carquivosresultado.h"

#include <format>
#include <utility>

#include "comum/comumdefs.h"      // CUeComumError = CBaseError<comum::EUeComumError, SErrorLimits{7200, 7600}>

namespace comum {

// wasm func 347 (srcloc line 99). Callers: CArquivosSavd construction (func 1164), CGravadorUtil::
// DeterminaNomeArquivoSemLetra, CLogComum::LogaGerandoResultados, func 6045. Compiled to a br_table; every string
// is built in place (SSO) except "imgbusa.dat" (11 chars, heap).
const std::string CArquivosResultado::operator[](EExtensaoArquivoResultado extensao) const
{
    using E = EExtensaoArquivoResultado;
    switch (extensao) {
    case E::AssinaturaVota: return "vota.vsc";
    case E::AssinaturaSA:   return "sa.vsc";
    case E::AssinaturaRed:  return "red.vsc";
    case E::BU:             return "bu.dat";
    case E::BUSA:           return "busa.dat";
    case E::RDV:            return "rdv.dat";
    case E::RDVRed:         return "rdvred.dat";
    case E::Justificativas: return "jufa.dat";
    case E::ImagemBU:       return "imgbu.dat";
    case E::ImagemBUSA:     return "imgbusa.dat";
    case E::ImagemZeresima: return "imgze.dat";
    case E::Hashes:         return "hash.dat";
    case E::Log:
    case E::Log2:           return "log.jez";
    case E::LogSA:          return "logsa.jez";
    case E::WsqBiometria:   return "wsqbio.jez";
    case E::WsqManual:      return "wsqman.jez";
    case E::WsqMesarios:    return "wsqmes.jez";
    case E::VersaoMR:       return "mr.ver";
    case E::AssinaturaSW:   return "asw.vsc";
    case E::AssinaturaHW:   return "ahw.vsc";
    }
    throw CUeComumError(EUeComumError(7200),
        std::format("Extensão de arquivo de resultado associada ao identificador {} não encontrada",
                    std::to_underlying(extensao)));                                                   // line 99
}

} // namespace comum
