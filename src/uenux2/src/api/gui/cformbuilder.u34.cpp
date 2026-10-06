// FRAGMENT reconstructed by unit u34 from vota_web_wasm.wasm.
// Original file: uenux2/src/api/gui/cformbuilder.cpp (attested by srclocs :292/:305 of AddStatusHeader; the
// file is shared by units u02, u07, u15, u16, u33 - see cformbuilder.u*.cpp).
#include <string>

#include "api/pattern/cpolysingletonlist.h"
#include "api/util/cdatetime.h"
#include "api/util/isystemdatetime.h"

namespace api {
namespace {

// wasm func 10870 (table slot 3117) - observed executing (twice a second: the clock of the status header of
// every voter screen).                                                                  name inferred
// Data source of the CDataTextFmt<std::string (*)(const std::string&)> that CFormBuilder::AddStatusHeader
// (func 502) creates with the format "A DD/MM/YYYY hh:mm:ss" (refreshed every 500 ms by CTextFieldUpdate).
// The time placeholders are expanded first (CTime::Format, shared_f779), then the date ones (CDate::Format,
// func 706) on the result.
std::string DataHoraAtual(const std::string& formato)
{
    // CPolySingletonList::instance<ISystemDateTime>() (func 1155) slot 0 = seconds since the epoch; in the
    // web build simulador::CWasmSystemDateTime (browser clock). CDateTime(time_t) = func 1000 (local time).
    const CDateTime agora(CPolySingletonList::instance<ISystemDateTime>().GetDataHora());
    return agora.GetDate().Format(agora.GetTime().Format(formato));
}

}  // namespace
}  // namespace api
