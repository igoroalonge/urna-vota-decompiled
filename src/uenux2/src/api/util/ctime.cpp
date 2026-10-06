// Reconstructed from vota_web_wasm.wasm (unit u20, owner of this file).
// Original: uenux2/src/api/util/ctime.cpp (srclocs ctime.cpp:48, :63, :148, :161).
//
// api::CTime = time of day as a single int: seconds since midnight (0 .. 86399). Being a one-int struct,
// the WebAssembly C ABI passes and returns it by value as a plain i32.
#include "api/util/ctime.h"

#include <string>

#include "ecourna/api/util/cstringutils.hpp"

namespace api {

namespace {
constexpr int SEGUNDOS_POR_DIA = 86400;

// srcloc ctime.cpp:48 (inlined into func 3642)
std::size_t EncodeTime(uebyte hora, uebyte minuto, uebyte segundo)
{
    if (hora > 23 || minuto > 59 || segundo >= 60)
        throw CUeUtilError(EUeUtilError{7079}, "Hora inválida");
    return hora * 3600 + minuto * 60 + segundo;
}

// srcloc ctime.cpp:63 (inlined into func 3643). "hhmm" or "hhmmss" (validated by CTime::IsValid,
// func 5449, other unit).
std::size_t EncodeTime(const std::string& hora)
{
    if (!CTime::IsValid(hora))
        throw CUeUtilError(EUeUtilError{7080}, "Hora inválida");
    using ecourna::api::util::CStringUtils;
    const uebyte h = CStringUtils::ToByte(hora.substr(0, 2));               // func 1142
    const uebyte m = CStringUtils::ToByte(hora.substr(2, 2));
    const uebyte s = hora.size() == 6 ? CStringUtils::ToByte(hora.substr(4, 2)) : 0;
    return h * 3600 + m * 60 + s;
}
} // namespace

// wasm func 3642 (tools: api::EncodeTime@3642; `this` is the first parameter)          name inferred
CTime::CTime(uebyte hora, uebyte minuto, uebyte segundo)
    : m_segundos(static_cast<int>(EncodeTime(hora, minuto, segundo)))
{
}

// wasm func 3643 (tools: api::EncodeTime@3643) - observed executing                     name inferred
CTime::CTime(const std::string& hora)
    : m_segundos(static_cast<int>(EncodeTime(hora)))
{
}

// srcloc ctime.cpp:148 (inlined)
void CTime::operator+=(int segundos)
{
    if (segundos < 0) {
        *this -= -segundos;
        return;
    }
    if (segundos > SEGUNDOS_POR_DIA - 1 || m_segundos + segundos >= SEGUNDOS_POR_DIA)
        throw CUeUtilError(EUeUtilError{7081}, "Overflow");
    m_segundos += segundos;
}

// srcloc ctime.cpp:161 (inlined)
void CTime::operator-=(int segundos)
{
    if (segundos < 0) {
        *this += -segundos;
        return;
    }
    if (m_segundos < segundos)
        throw CUeUtilError(EUeUtilError{7082}, "Underflow");
    m_segundos -= segundos;
}

// wasm func 3641 (tools: api::CTime::operator+=) - a by-value subtraction: the copy's operator-= is
// inlined (and, one level deep, the operator+= it calls for negative arguments). The result is
// returned as an i32.                                                                   name inferred
// Callers: CAjusteInicial::ValidaTemposDesligamento, CIniciodeCiclo::AjustaDataHora,
//          CEncerramentoHorarioInvalido::StartState.
CTime CTime::operator-(int segundos) const
{
    CTime r = *this;
    r -= segundos;
    return r;
}

} // namespace api
