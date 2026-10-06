// uenux2/src/app/vota/comum/votadefs.h   (path inferred from votadefs.cpp, attested by std::source_location
// records :103, :110, :135)
// Reconstructed from vota_web_wasm.wasm (unit u26).
//
// Shared definitions of the VOTA application: its error class and a few free helpers.
#pragma once

#include <cstdint>
#include <string>

#include "comum/comumtypes.h"                        // comum::TQtdImpresso (uebyte)
#include "ecourna/api/exception/cbaseerror.hpp"

namespace vota {

/// ecourna::api::exception::CBaseError<vota::EUeVotaError, SErrorLimits{9300, 9500}> (typeinfo @1532388).
/// Every throw site uses the merged constructor thunk wasm func 253 (tools: vota_f253). The header that
/// declares it is a guess (every VOTA unit uses the name CUeVotaError).
enum class EUeVotaError : int {};
using CUeVotaError =
    ecourna::api::exception::CBaseError<EUeVotaError, ecourna::api::exception::SErrorLimits{9300, 9500}>;

/// votadefs.cpp:103 - wasm func 3290. Asks SAVD to validate the signature package `pacote` (e.g. "vota.vsu")
/// of the current turno's work directory of the internal memory (MI).
void VerificaAssinaturaMI(const std::string& pacote);

/// votadefs.cpp:110 - wasm func 3288. Same for the external memory (MV).
void VerificaAssinaturaMV(const std::string& pacote);

/// votadefs.cpp:135 - wasm func 4582. How many more BU copies ("vias adicionais") may still be printed.
comum::TQtdImpresso GetQuantidadeMaximaBUsAdicionais();

} // namespace vota
