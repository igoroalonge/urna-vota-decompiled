// FRAGMENT reconstructed by unit u34 from vota_web_wasm.wasm.
// Original file: uenux2/src/app/vota/log/clogvota.cpp (attested; owner unit u26, see clogvota.h).
//
// Two more bodies merged by wasm-opt ("merge similar functions") out of CLogVota's one-line log methods.
// Every such method is `Loga(<literal>)` or `Loga(std::format(<literal>, x))`; methods whose code differs
// only in the literal were folded into one body that receives the literal as parameters.
#include <format>
#include <string>
#include <string_view>

#include "api/uelog/cloga.h"
#include "vota/log/clogvota.h"

namespace vota {

// wasm func 6113 - merged body, severity 1 (IEventosLog::Loga = api_f233).                 name inferred
// std::vformat of a format string with one "{}" and one std::string argument (packed type 13 =
// string_view), 256-byte inline format buffer. The format string is passed as two pointers, END first:
// api_f6113(this, &argumento, end, begin), e.g. 2502 passes (3746, 3723) for the 23-byte literal @3723.
// Thunks (their names inferred from the literal):
//   func 2502  "Operador selecionou: {}"        (CEscolheOpcao::ProcessInput, cescolheopcao.cpp)
//   func 3258  "Mesário {} habilitou o eleitor"  (CRegistraDigitalOperador::ProcessTick)
void CLogVota::LogaFormatado(std::string_view formato, const std::string& argumento) const
{
    Loga(std::vformat(formato, std::make_format_args(argumento)));
}

// wasm func 6115 - merged body, severity 2 (IEventosLog::LogaAviso = api_f1398).           name inferred
// A 55-character literal passed as seven 8-byte pieces (texto+0, +8, ... +47).
// Thunks:
//   func 2495  "Habilitação cancelada durante reconhecimento biométrico"
//              (CPedeDigital, CDigitalNaoReconhecida, CDigitalNaoReconhecidaDecBiometria,
//               CDigitalNaoReconhecidaPorTempo ProcessInput: the mesário gave up the voter's fingerprint)
//   func 4556  "Quantidade de vias adicionais excede o máximo permitido"
//              (CLimiteCopiasBUAtingido::StartState, CEmitirMaisBU::ProcessInput: extra BU copies)
void CLogVota::LogaAviso55(const char (&texto)[56]) const
{
    api::CLoga::loga(m_aplicativo, api::ESeveridade{2}, std::string(texto, 55));        // func 433
}

}  // namespace vota
