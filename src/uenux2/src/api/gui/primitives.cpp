// Reconstructed from vota_web_wasm.wasm (unit u17).
// Original: uenux2/src/api/gui/primitives.cpp (srclocs primitives.cpp:20, 26, 32, 38).
// Unit u15 wrote the other functions of this file (SRect::MoveTo, Union) in primitives.u15.cpp.
//
// Checked setters of the four edges of an SRect. Each is a UE_ASSERT (CBaseError<EUeAssertError>,
// built by the ecourna thunk func 463) that keeps the rectangle well formed (edges inclusive).
// Only Left and Right survive as functions; Top and Bottom are always inlined - both are inside
// func 5537 (CFramedText::DesenhaCaracter, reconstructed in cframedtext.u17.cpp), which is why the
// tools named that function "api::SRect::Top".
#include "api/gui/gui-common.u15.h"

#include "ecourna/api/exception/cbaseerror.hpp"

namespace api {

// UE_ASSERT(expr): if (!(expr)) throw CBaseError<EUeAssertError>(code, "Assert (" #expr ")",
// std::source_location::current()). The codes are consecutive, one per assert of this file.
#define UE_ASSERT_CODE(expr, codigo)                                                              \
    do {                                                                                          \
        if (!(expr))                                                                              \
            throw ecourna::api::exception::CBaseError<EUeAssertError>(                            \
                static_cast<EUeAssertError>(codigo), "Assert (" #expr ")");                       \
    } while (false)

// wasm func 5488 (observed executing) - primitives.cpp:20
// Callers: CFramedText::DesenhaCaracter (5537), CProgressBar slot 2 (10977).
void SRect::Left(TPosition l)
{
    UE_ASSERT_CODE(right >= l, 3400);
    left = l;
}

// primitives.cpp:26 - inlined only (func 5537)
void SRect::Top(TPosition t)
{
    UE_ASSERT_CODE(bottom >= t, 3401);
    top = t;
}

// wasm func 5487 - primitives.cpp:32
void SRect::Right(TPosition r)
{
    UE_ASSERT_CODE(r >= left, 3402);
    right = r;
}

// primitives.cpp:38 - inlined only (func 5537)
void SRect::Bottom(TPosition b)
{
    UE_ASSERT_CODE(b >= top, 3403);
    bottom = b;
}

}  // namespace api
