// FRAGMENT reconstructed by unit u32 from vota_web_wasm.wasm.
// Original file: uenux2/src/api/gui/cinputmenufield.cpp (attested by the srclocs :198/:210 of
// CMenuValidation::matchesExactly/matchesPartially). The class and the rest of the file are in
// cinputmenufield.u15.h / cinputmenufield.u15.cpp (unit u15). Merge into cinputmenufield.cpp.
#include "api/gui/cinputmenufield.u15.h"

namespace api {

// wasm func 10899 - CMenuValidation vtable slot 3 (overrides IInputValidation::IsValidChar)
// A key typed in a menu is accepted only if it is a digit AND the text it would produce still selects a
// visible item (IsValid = slot 2, func 10900: exact match when the field is full, prefix match otherwise;
// both beep on failure).
bool CMenuValidation::IsValidChar(char c) const
{
    if (m_caracteres.find(c) == std::string::npos)                     // "1234567890"
        return false;
    return IsValid(m_menu->m_texto + c);                               // virtual call, slot 2
}

} // namespace api
