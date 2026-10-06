// uenux2/src/api/util/cdatetime.cpp (attested by srclocs :31, :61, :206) -- FRAGMENT written by unit u37.
// Class and conventions: cdatetime.cpp (unit u20). api::CDateTime = +0 CDate m_data {+0 dia (uebyte),
// +2 mês (uebyte), +4 ano (short)} + 8 CTime m_hora {+0 seconds since midnight (uint32)}.
#include <ctime>

#include "api/util/cdatetime.h"

namespace api {

// wasm func 1381 (tools: vota_f1381)                                   name as in cajustedatahora.cpp (u20)
// The civil date/time as a time_t, interpreted as UTC (timegm): the urna keeps "local time expressed as if
// it were UTC", the inverse of ConvertFromLocalTime (gmtime_r, wasm 5476).
// Callers: api::CAjusteDataHora slot 0 (10875: sets the system clock), vota::CAjusteInicial::StartState
// (7160) and vota::CIniciodeCiclo::AjustaDataHora (7306) (delta for EstadoGeralUrna.ajusteDataHora),
// vota::CEncerramentoHorarioInvalido::StartState (10710), comum::dao::CComparecimentoMesarioDAO slot 3
// (10403: timestamps stored in uenux.db), comum_f5841.
// The same tm conversion is inlined in CDateTime::DiferencaSegundos (wasm 5471).
std::time_t CDateTime::ToTimeT() const
{
    std::tm tm;                                   // tm_wday / tm_yday left unset: timegm ignores them
    tm.tm_sec = m_hora.GetSegundo();              // segundos % 60
    tm.tm_min = m_hora.GetMinuto();               // uint16(segundos - hora * 3600) / 60
    tm.tm_hour = m_hora.GetHora();                // uebyte(segundos / 3600)
    tm.tm_mday = m_data.GetDia();                 // byte +0
    tm.tm_mon = m_data.GetMes() - 1;              // byte +2
    tm.tm_year = m_data.GetAno() - 1900;          // short +4
    tm.tm_isdst = 0;
    return ::timegm(&tm);
}

} // namespace api
