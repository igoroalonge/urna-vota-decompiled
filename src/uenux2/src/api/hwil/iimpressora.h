// Reconstructed from vota_web_wasm.wasm (unit u17) - partial.
// Original: uenux2/src/api/hwil/iimpressora.h (srclocs iimpressora.h:103 DisableSensors,
// iimpressora.h:116 EnableSensors).
//
// Hardware interface of the urna's thermal printer ("impressora"). The web build registers
// simulador::CWasmNullPrinter (vtable @1530652, : IImpressoraRelatorios : IImpressora), which prints
// nothing. Its vtable, in declaration order (the destructor is NOT first in this interface):
//   [0] ret 0 (ICF 340)   [1] nop   [2] nop   [3] nop   [4] SetStyle (10881)   [5] GetColumns (10880:
//   style 3 -> 19 columns, else 38)   [6] DisableSensors (8360)   [7] EnableSensors (8341)
//   [8] nop  [9] nop  [10] dtor (ICF 174)  [11] deleting dtor  [12] DoSetStyle (8365: stores the
//   style)  [13] ret 0  [14] nop
#pragma once

#include <cstdint>
#include <format>
#include <source_location>
#include <string>

#include "ecourna/api/exception/cbaseerror.hpp"

namespace api {

using uebyte = std::uint8_t;

// api::EUePrinterError, SErrorLimits{4450, 4650}; typeinfo @1530740.
enum class EUePrinterError : int {
    DISABLE_SENSORS_NAO_SUPORTADO = 4466,   // names inferred
    ENABLE_SENSORS_NAO_SUPORTADO = 4467,
    ESTILO_INVALIDO = 4468,
};

// vtable @1530772. Two extra std::string members, both empty unless set (+40, +52).
class CUePrinterError
    : public ecourna::api::exception::CBaseError<EUePrinterError /*, SErrorLimits{4450, 4650}*/> {
public:
    // wasm func 10257 (tools: api_f10257): base constructor (ecourna_f1143) + the two empty
    // strings, then the CUePrinterError vptr.                                      name inferred
    CUePrinterError(EUePrinterError codigo, std::string mensagem,
                    const std::source_location& local = std::source_location::current())
        : CBaseError(codigo, std::move(mensagem), local)
    {
    }

protected:
    std::string m_detalhe;       // +40  ?
    std::string m_complemento;   // +52  ?
};

// vtable @1583872: CUePrinterError + int code at +64 (see IImpressoraRelatorios::SetStyle).
class CUeCodedPrinterError : public CUePrinterError {
public:
    CUeCodedPrinterError(EUePrinterError codigo, std::string mensagem, std::string detalhe, int codigoImpressora,
                         const std::source_location& local = std::source_location::current())
        : CUePrinterError(codigo, std::move(mensagem), local), m_codigoImpressora(codigoImpressora)
    {
        m_detalhe = std::move(detalhe);
    }

private:
    int m_codigoImpressora;      // +64
};

// wasm func 6133 (tools: api_f6133) is NOT a source function. Its shape is Binaryen's
// merge-similar-functions (docs/02-wasm-binary-anatomy.md §5.4): 8360 and 8341 are constant thunks
// `api_f6133(this, sensores, <srcloc>, <code>)` - their own two parameters in order, then two
// constants (srcloc iimpressora.h:103 + 4466 / iimpressora.h:116 + 4467). The srcloc comes before
// the error code, an order a default `std::source_location` argument could not produce. So each
// default method throws directly, as written below, and the optimiser merged the two bodies.
class IImpressora {
public:
    // slot 6 - wasm func 8360 (iimpressora.h:103). Default: the printer has no paper sensors.
    virtual void DisableSensors(uebyte sensores)
    {
        throw CUePrinterError(EUePrinterError::DISABLE_SENSORS_NAO_SUPORTADO,
                              std::format("Not supported ({})", unsigned{sensores}));   // :103
    }

    // slot 7 - wasm func 8341 (iimpressora.h:116)
    virtual void EnableSensors(uebyte sensores)
    {
        throw CUePrinterError(EUePrinterError::ENABLE_SENSORS_NAO_SUPORTADO,
                              std::format("Not supported ({})", unsigned{sensores}));   // :116
    }

    virtual ~IImpressora() = default;    // slots 10/11
};

}  // namespace api
