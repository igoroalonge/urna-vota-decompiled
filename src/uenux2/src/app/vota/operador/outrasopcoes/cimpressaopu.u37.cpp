// uenux2/src/app/vota/operador/outrasopcoes/cimpressaopu.cpp (path inferred, as in unit u25) -- FRAGMENT
// written by unit u37. CImpressaoPU::StartState (wasm 11902) and FormataHorario (wasm 2686) are in
// src/uenux2/src/app/vota/u25-foreign-fragments.cpp.
//
// The "Parâmetros de urna" report prints the four voting-day instants of the configuration (4 x int64 at
// CConfiguracaoEleicao +504/+512/+520/+528). The special values handled below (INT64_MAX-1, INT64_MAX,
// INT64_MIN) and the tick unit (microseconds, 86 400 000 000 per day, days counted with the Julian-day-number
// formula of boost::gregorian) show that they are boost::posix_time::ptime values; each is printed on two
// lines: "<rótulo>  hh:mm:ss -" (FormataHorario) and "dd/mm/aaaa" (FormataData, below), both through
// comum::CGeradorRelPU::AdicionaLinha (wasm 677).
#include <format>
#include <string>

#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace vota {

namespace {

// wasm func 2687 (tools: vota_f2687)                                                    name inferred
// ptime::date() is inlined: a special ptime (not_a_date_time / ±infinity) gives the special day numbers
// -2 / -1 / 0, which boost::gregorian::gregorian_calendar::from_day_number (ecourna_f1376) rejects by
// throwing boost::gregorian::bad_year ("Year is out of valid range: 1400..9999") - uncaught here.
// The year goes through std::to_string (ecourna_f296) and std::stoi (base 10; called through invoke_iiii
// only so that the temporary string is destroyed if it throws) before being formatted: a pointless round
// trip that does not change the value.
std::string FormataData(const boost::posix_time::ptime& instante)
{
    const boost::gregorian::date data = instante.date();
    const int dia = data.day();                                                       // from_day_number
    const int mes = data.month();                                                     // from_day_number
    const int ano = std::stoi(std::to_string(data.year()));                           // from_day_number
    return std::format("{:02}/{:02}/{:04}", dia, mes, ano);
}

} // namespace

} // namespace vota
