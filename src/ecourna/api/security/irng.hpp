// ecourna-lib/ecourna/api/security/irng.hpp   (path inferred: interface IRng -> irng.hpp, next to the other
//                                               ecourna/api/security interfaces; 3 reconstructed files include it)
//
// Reconstructed from vota_web_wasm.wasm. Unit u40.
//
// RTTI: ecourna::api::security::IRng (typeinfo @1527332, polymorphic, no base). Two implementations:
//   CTrng  (ctrng.hpp)  std::random_device("/dev/urandom")  -> in the browser: WASI random_get ->
//                       crypto.getRandomValues. Used only inside CBlockCipher: CAesCipher::Encrypt (9493)
//                       draws a 16-byte IV with Gera(iv, 16) ONLY when its CAesKey carries no IV. The RDV
//                       cipher has a fixed IV (CHKDFSeed seed[32..48), u01 3.1), so the recorded votes never
//                       drew from CTrng (5173 was not observed executing).
//   CPrng  (cprng.hpp)  Mersenne Twister with a fixed seed. main (func 10307) registers CPrng(0) as THE
//                       application-wide IRng (api::CPolySingletonList::push<IRng>), so every
//                       CPolySingletonList::instance<IRng>() of the web build is deterministic.
// Slot order (identical in both vtables @1114668 / @1113176):
//   [0] ~T (ICF 174)  [1] deleting (144)  [2] GeraByte  [3] Gera  [4] Gera(buffer)  [5] Gera(buffer, n)
// Method names are inferred (other units already call slot 3 "Gera()" and slot 4 "Gera(vector&)").
#pragma once

#include <cstddef>
#include <vector>

#include "ecourna/types.hpp"   // uebyte

namespace ecourna::api::security {

class IRng {
public:
    virtual ~IRng() = default;

    // Slot 2: one random byte.
    virtual uebyte GeraByte() = 0;                                                    // name inferred

    // Slot 3: one random int, uniform over [INT_MIN + 1, INT_MAX - 1] (both implementations). Callers
    // take remainders of it (CPoliticaExecucaoEleitor: % 4, % 100 with signed rem; CControlaArmazenamento
    // DeImagens: unsigned % 999999).
    virtual int Gera() = 0;                                                           // name inferred

    // Slot 4: fills the whole buffer = Gera(buffer, buffer.size()).
    virtual int Gera(std::vector<uebyte>& buffer) = 0;                                // name inferred

    // Slot 5: writes `quantidade` random bytes at the start of `buffer`.
    // Returns 0 = ok, 1 = quantidade == 0, 2 = buffer smaller than quantidade, -1 = std::exception caught
    // (CTrng only). No caller of the binary checks the value (CAesCipher::Encrypt 9493 drops it and keeps
    // the 16 zero bytes it pre-filled if the call fails).
    virtual int Gera(std::vector<uebyte>& buffer, std::size_t quantidade) = 0;        // name inferred
};

} // namespace ecourna::api::security
