// uenux2/src/app/comum/iinterfaceinit.cpp (attested file: srclocs :220, :285 ...) -- FRAGMENT written by
// unit u37. comum::IInterfaceInit is the client of the urna's "init" daemon (commands with a numeric id and
// a text, EnviarMensagemThrowVoid); in the web build it is simulador::CWasmInit.
#include <cstdarg>
#include <string>
#include <syslog.h>

#include "api/util/csystem.h"
#include "comum/iinterfaceinit.h"

namespace comum {

namespace {

// Cache "is this a real urna (/dev/urna exists)?" of this translation unit, -1 = not tested yet.
int s_urnaReal = -1;                                                          // @1552472

// wasm func 5895 (tools: vota_f5895). Header helper instantiated per translation unit; wasm-opt merged all
// copies into func 2921 (LogSistema: vsyslog on a real urna, vprintf when DEBUG_UENUX is set, silent
// otherwise) with the priority and the cache as extra arguments. This copy uses LOG_INFO.     name inferred
// Callers: IInterfaceInit::MontarMRSemHabilitar (5896: "Serial da MR: %s") and IInterfaceInit::LogDiskInfo
// (iinterfaceinit.cpp:285, inlined into vota::CThreadMonitor::Run 10226: "%s: erro [%d] ao tentar logar
// informações sobre as partições").
void LogInfo(const char* formato, ...)
{
    va_list args;
    va_start(args, formato);
    LogSistema(formato, args, LOG_INFO /* 6 */, &s_urnaReal);                       // func 2921
    va_end(args);
}

} // namespace

// wasm func 2863 (tools: vota_f2863)                              name inferred (u06/u09 call it HabilitaMR)
// Powers the result-media slot (MR = mídia de resultado, the USB stick that carries the BU and the other
// result files to the Junta Eleitoral) and waits half a second for it to settle.
// Callers: vota::CAjusteInicial::StartState (7160, clean-up of the MR in training) and
//          vota::CCopiaResultadoParaMR::CopiaResultado (12134, encerramento: copy of the result files).
// The same two statements open IInterfaceInit::DispositivoMR, which is inlined into MontarMRSemHabilitar
// (5896, unit u23). That supports a member of IInterfaceInit that DispositivoMR calls and that stayed out of
// line for the two VOTA callers. The callers' units write it as a free function HabilitaMR(IInterfaceInit&).
//
// WEB BUILD: the sleep is compiled as `if (byte @1584624 == 1) emscripten_sleep(500)`. The byte is 1 and
// the module has no Asyncify, so reaching this line aborts the program ("Please compile your program with
// async support..."). Neither caller is reachable from the simulator page (see the doc, §suspicious).
void IInterfaceInit::HabilitarMR()
{
    EnviarMensagemThrowVoid(10, "habilitando MR");                                  // wasm 3833
    api::CSystem::Sleep(500);                                                         // emscripten_sleep(500)
}

} // namespace comum
