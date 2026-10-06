// FRAGMENT reconstructed by unit u07 from vota_web_wasm.wasm.
// Original file: uenux2/src/api/gui/cgrayedframedtext.cpp (attested by the file of CGrayedFramedText::MaskText,
// wasm func 5534). Merge into cgrayedframedtext.cpp.
#include "api/gui/cgrayedframedtext.h"

namespace api {

// wasm func 5535 (observed executing)                                                // name inferred
// Grey variant of the framed digit boxes (vtable api::CGrayedFramedText @1578752): n boxes at `pos`,
// fixed font @474880 {40, normal}, alignment 0.
CGrayedFramedText::CGrayedFramedText(size_t digitos, const SPoint& pos)
    : CFramedText(digitos, pos, SFont{40, 0}, ETextAlignment(0))             // func 1385 (cframedtext.cpp:28)
{
}

} // namespace api
