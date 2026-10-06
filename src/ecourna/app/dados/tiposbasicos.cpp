// ecourna-lib/ecourna/app/dados/tiposbasicos.cpp
// (original build path: /home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/app/dados/tiposbasicos.cpp)
// Reconstructed from vota_web_wasm.wasm, unit u14.
//
// The file is long in the original (the only std::source_location record is at line 1149). Only
// CErroLeituraBiometria's constructor survives as a function of its own; the type-name strings of
// the CBaseTypes listed in tiposbasicos.h are initialised by the global constructor (func 14478).
#include "ecourna/app/dados/tiposbasicos.h"

#include <format>

#include "ecourna/app/dados/dadoserros.h"

namespace ecourna::app::dados {

// wasm func 2668 (srcloc line 1149).
// The base-class constructor CBaseType<EErroLeituraBiometria, 0, 13, 34>(erro) is inlined first:
// erro >= 14 throws CBaseError<EPatternError>(1300,
//     std::format("O tipo '{}' deve ter valores no intervalo [{},{}].", "ErroLeituraBiometria", 0, 13))
// with srcloc cbasetype.hpp:39. Then the sentinel ErroBioUltimo is rejected with the same text but
// the range printed as [ErroBioPrimeiro, ErroBioUltimo - 1]; the first bound goes through the
// enum's std::formatter (format handle, slot 6637 -> func 536), the second is the int 12.
CErroLeituraBiometria::CErroLeituraBiometria(EErroLeituraBiometria erro)
    : TErroLeituraBiometria(erro)
{
    if (erro == ErroBioUltimo) {
        throw CDadosError(2014, std::format("O tipo '{}' deve ter valores no intervalo [{},{}].",
                                            TErroLeituraBiometria::NomeTipo(),   // "ErroLeituraBiometria" (static @1912136; ? accessor name)
                                            ErroBioPrimeiro,
                                            ErroBioUltimo - 1));                   // line 1149
    }
}

} // namespace ecourna::app::dados
