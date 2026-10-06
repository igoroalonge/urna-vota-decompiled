// ecourna-lib/ecourna/api/security/cprng.hpp   (path inferred)
//
// Reconstructed from vota_web_wasm.wasm. Unit u40.
//
// RTTI: CPrng : IRng   (typeinfo @1113200, vtable @1113176). Pseudo-random generator: Boost.Random
// mt19937 (the seeding code contains boost's normalize_state(), absent from std::mersenne_twister_engine)
// plus boost::random::uniform_int_distribution (the byte is the TOP 8 bits of the output, boost's bucket
// division; std:: would take the low 8 bits).
//
// Every method builds a NEW engine seeded with m_semente on the stack (2.5 KB) and discards it afterwards,
// so the object has no evolving state: GeraByte() and Gera() return the same value on every call, and
// Gera(buffer, n) always writes the same n bytes. main (func 10307) registers CPrng(0) as the application's
// IRng. With seed 0 (values computed with a reference MT19937):
//   GeraByte()            = 140 (0x8C)
//   Gera()                = 209652397   (first output 2357136044 - 0x7FFFFFFF)
//   Gera(buffer, 32)      = 8c97b7d89adb8bd86c9fa562704ce40ef645627acacf877a9164ecd6125616a5
// (all three confirmed by calling the functions in the running simulator; the 32 bytes appear as salt +
// informacaoAdicional of the attendance file written by the BU harness, samples/bu-real/run-full).
#pragma once

#include <cstdint>

#include "ecourna/api/security/irng.hpp"

namespace ecourna::api::security {

class CPrng final : public IRng {
public:
    explicit CPrng(std::uint32_t semente);                                       // wasm func 9503

    uebyte GeraByte() override;                                                  // wasm func 9502
    int Gera() override;                                                         // wasm func 9500
    int Gera(std::vector<uebyte>& buffer) override;                              // wasm func 9499
    int Gera(std::vector<uebyte>& buffer, std::size_t quantidade) override;      // wasm func 5178

private:
    std::uint32_t m_semente;   // +4  (Gera() reads the 32-bit word; GeraByte/Gera(buffer, n) read only
                               //      its low byte with i32.load8_u - see cprng.cpp)
};                             // sizeof 8

} // namespace ecourna::api::security
