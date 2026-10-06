// FRAGMENT reconstructed by unit u16 from vota_web_wasm.wasm. Merge into uenux2/src/api/gui/primitives.cpp
// (attested by the srcloc records of SRect::Left :20, SRect::Top :26, SRect::Right :32, SRect::Bottom :38).
//
// api::SRect = {TPosition left, top, right, bottom} (4 x int16, 8 bytes). SRect(const SPoint&, const SPoint&)
// normalises the two corners with min/max (inlined everywhere).
#include "api/gui/primitives.h"

#include <algorithm>

namespace api {

// wasm func 2768 (no file in the tools' database; callers CProgressBar ctor/Draw, CInputMenuField,
// CTelasVota). Returns a copy with each side moved by the given amount (like Qt's QRect::adjusted),
// normalised.                                                                              // name inferred
SRect SRect::Adjusted(TPosition dl, TPosition dt, TPosition dr, TPosition db) const
{
    const TPosition l = left + dl, t = top + dt, r = right + dr, b = bottom + db;
    return SRect{std::min(l, r), std::min(t, b), std::max(l, r), std::max(t, b)};
}

} // namespace api
