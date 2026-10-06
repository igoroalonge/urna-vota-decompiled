// uenux2/src/api/util/cdatetime.cpp  --  FRAGMENT written by unit u02 (owner u20)

#include "api/util/cdatetime.h"

namespace api {

// wasm func 6155                                                       // name inferred
// Formats a microsecond time point (int64 at *b) with a 6-field format string given as
// [begin, end). The civil date is computed inline with the "days from civil" algorithm
// (4800-year shift, /100, /400 corrections) after dividing by 86 400 000 000 µs. Three special
// values (k = t - (INT64_MAX-1), tested with i64.le_u 2) bypass the conversion and give fixed day
// numbers: INT64_MIN (k = 2) -> 0, INT64_MAX (k = 1) -> -1, INT64_MAX-1 (k = 0) -> -2 (the usual
// -inf / +inf / not-a-date-time sentinels of an int64 time adapter). Also used by ecourna (wasm 9220).
std::string FormataDataHora(const CDateTime& dataHora, std::string_view formato);

// wasm func 2685                                                       // name inferred
// "AAAAMMDDhhmmss": used four times by the inlined md::CEleicaoDataHora constructor
// (celeicaodatahora.cpp:30, "Datas inválidas") while CConfiguracaoEleicao::CreateInst loads the
// election dates.
std::string FormataAAAAMMDDhhmmss(const CDateTime& dataHora)
{
    return FormataDataHora(dataHora, "{:04}{:02}{:02}{:02}{:02}{:02}");
}

} // namespace api
