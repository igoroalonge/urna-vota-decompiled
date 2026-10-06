// FRAGMENT reconstructed by unit u15 from vota_web_wasm.wasm.
// Original file: uenux2/src/api/gui/primitives.cpp (path inferred: the other SRect helpers,
// SRect::Left/Right (5488/5487), carry srclocs of primitives.cpp). Merge into primitives.cpp.
#include "api/gui/gui-common.u15.h"

#include <algorithm>

namespace api {

// wasm func 1915: SRect::MoveTo(const SPoint&) - defined inline in gui-common.u15.h.        // name inferred
// Callers: CInputMenuField::Move / UpdateLayout (u15), CProgressBar/CStepsProgressBar::Move.

// wasm func 1916 (tools: api_f1916)                                                        // name inferred
// Bounding box of two rectangles. Both results are normalised with min/max, exactly as the body does
// (it first takes min(left)/max(right), then orders the pair again).
SRect Union(const SRect& a, const SRect& b)
{
    const TPosition l = std::min(a.left, b.left);
    const TPosition r = std::max(a.right, b.right);
    const TPosition t = std::min(a.top, b.top);
    const TPosition btm = std::max(a.bottom, b.bottom);
    return SRect{std::min(l, r), std::min(t, btm), std::max(l, r), std::max(t, btm)};
}

} // namespace api
