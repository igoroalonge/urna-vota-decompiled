// FRAGMENT reconstructed by unit u32 from vota_web_wasm.wasm.
// Original file: uenux2/src/api/hwil/iimpressora.h (unit u17; CUePrinterError and CUeCodedPrinterError are
// declared there). Only the destructors were left in unit u32.
#include "api/hwil/iimpressora.h"

namespace api {

// wasm func 8351 - slot 0 of BOTH api::CUePrinterError (vtable @1530772) and api::CUeCodedPrinterError
//                  (vtable @1583872): CUeCodedPrinterError only adds an int (+64), so its destructor is
//                  identical and wasm-opt merged them.
//   store CUePrinterError vptr; free m_complemento (+52) and m_detalhe (+40);
//   store ecourna::api::exception::CError vptr; free the two strings of CError (+28, +8).
// wasm func 4910 - slot 1 of both: the same + operator delete.
CUePrinterError::~CUePrinterError() = default;

} // namespace api
// (In iimpressora.h the destructor is implicit; the out-of-line definition above only documents the two
//  wasm functions and must not be merged as a separate definition.)
