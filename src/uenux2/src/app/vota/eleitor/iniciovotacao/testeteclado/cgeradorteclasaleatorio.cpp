// Reconstructed from vota_web_wasm.wasm (unit u39).
// Original (path inferred): uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cgeradorteclasaleatorio.cpp
// (could also be part of ctesteteclado.cpp, whose :121 srcloc registers it as the default IGeradorTeclas).
// Class declaration: ctesteteclado.h (unit u26):
//   vota::testeteclado::impl::IGeradorTeclas (typeinfo @1547320) <- impl::CGeradorTeclasAleatorio
//   (typeinfo @1547308, vtable @1547296, 4 bytes: [0] 174 [1] 144 [2] GeraSequencia 11803)
//
// Order in which the mesário must press the 13 keys of the voter keypad during the keypad test
// (CTesteTeclado::StartState): a uniform random permutation.
//
// WEB BUILD: unreachable (no keypad test in the simulator's start-up).
#include <algorithm>
#include <random>
#include <string>
#include <vector>

#include "vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.h"

namespace vota::testeteclado::impl {

// wasm func 11803 - vtable slot 2
// Everything is inlined:
//   * the function-local static engine: state 624 words @1835152, index @1837648, init flag @1837652 (a plain
//     byte, not a __cxa_guard: single-threaded build). Seeded ONCE with one 32-bit value of
//     std::random_device (func 5935 = random_device("/dev/urandom" token), func 3331 = operator(); in the
//     browser Emscripten serves it from crypto.getRandomValues) and the standard MT19937 recurrence
//     x[i] = 1812433253 * (x[i-1] ^ (x[i-1] >> 30)) + i;
//   * std::shuffle (libc++): uniform_int_distribution<ptrdiff_t> over [0, n-1], [0, n-2], ...; swap only when
//     the drawn index is not 0; tempering constants 0x9D2C5680 / 0xEFC60000, rejection sampling with a mask.
// The vector is taken by value and returned by move.
std::vector<std::string> CGeradorTeclasAleatorio::GeraSequencia(std::vector<std::string> teclas)
{
    static std::mt19937 gerador(std::random_device{}());
    std::shuffle(teclas.begin(), teclas.end(), gerador);
    return teclas;
}

}  // namespace vota::testeteclado::impl
