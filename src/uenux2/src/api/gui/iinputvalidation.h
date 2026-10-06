// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/gui/iinputvalidation.h (path inferred).
//
// api::IInputValidation (typeinfo 1538688, "class", vtable @1538704, 16 bytes): decides which keys an input
// field accepts and whether the finished text is valid.
//   +0 vptr   +4 std::string m_caracteres (the accepted characters)
// Subclasses (all share slot 0 = 12502 and slot 1 = ICF 2980):
//   CNumberValidation  @1538660  16 bytes, "0123456789" (título, candidate number, year of birth ...)
//   COptionValidation  @1550960  16 bytes, the digits of the options of a menu (comum::CMenuBase)
//   CControlValidation @1577484  16 bytes, "" - only the control keys (CONFIRMA/CORRIGE/BRANCO) end the input
//   (these three also share slot 2 = 4022 and slot 3 = 12473)
//   CMenuValidation    @1583308  20 bytes (make_shared block of 32 in func 3675): "1234567890" + back
//                                pointer to the menu at +16 (cinputmenufield.cpp); overrides slot 2 (10900)
//                                and slot 3 (10899)
#pragma once

#include <string>
#include <utility>

namespace api {

class IInputValidation {
public:
    explicit IInputValidation(std::string caracteres) : m_caracteres(std::move(caracteres)) {}

    // wasm func 12502 - slot 0 of IInputValidation and of all four subclasses: thunk
    // `return shared_f1723(this, vtable IInputValidation)` (store the vptr, free m_caracteres).
    // Slot 1: ICF 325 (abstract) / ICF 2980 in the subclasses (= 1722, the same + operator delete).
    virtual ~IInputValidation() = default;

    // slot 2 - pure here; CNumber/COption/CControlValidation share func 4022:
    //   every character of `texto` must pass IsValidChar (an empty text is valid).
    virtual bool IsValid(const std::string& texto) const = 0;

    // wasm func 12473 - slot 3 of IInputValidation, CNumberValidation, COptionValidation, CControlValidation
    virtual bool IsValidChar(char c) const
    {
        return m_caracteres.find(c) != std::string::npos;          // memchr over m_caracteres
    }

protected:
    std::string m_caracteres;   // +4
};

} // namespace api
