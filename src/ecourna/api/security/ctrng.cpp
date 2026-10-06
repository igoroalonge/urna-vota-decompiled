// ecourna-lib/ecourna/api/security/ctrng.cpp   (path inferred)
//
// Reconstructed from vota_web_wasm.wasm. Unit u40.
//
// std::random_device in this libc++ (Emscripten, _LIBCPP_USING_GETENTROPY): the constructor (func 2544)
// accepts ONLY the token "/dev/urandom" (else throws "random device not supported <token>"), and
// operator() (func 3331) is getentropy(&r, 4) = WASI random_get -> crypto.getRandomValues in the browser.
#include "ecourna/api/security/ctrng.hpp"

#include <exception>
#include <limits>
#include <random>
#include <string>

namespace ecourna::api::security {

namespace {
// Static std::string built by __wasm_call_ctors (func 14478) at @1911784 ("/dev/urandom", 12 chars, heap
// buffer of 16). wasm func 9478 = its atexit destructor.                                  // name inferred
const std::string TOKEN_DISPOSITIVO = "/dev/urandom";
} // namespace

// wasm func 9477 (vtable slot 2). No try: an exception of the device propagates.
uebyte CTrng::GeraByte()
{
    std::random_device dispositivo(TOKEN_DISPOSITIVO);
    return static_cast<uebyte>(dispositivo());          // low 8 bits (`& 255`)
}

// wasm func 9476 (vtable slot 3). The loop `do r = rd(); while (r > 0xFFFFFFFD)` followed by
// `return r - 0x7FFFFFFF` is libc++'s uniform_int_distribution<int> for a range of 2^32 - 2 values starting at
// -2147483647, i.e. [INT_MIN + 1, INT_MAX - 1].                                            // ? exact bounds spelling
int CTrng::Gera()
{
    std::random_device dispositivo(TOKEN_DISPOSITIVO);
    std::uniform_int_distribution<int> distribuicao(std::numeric_limits<int>::min() + 1,
                                                    std::numeric_limits<int>::max() - 1);
    return distribuicao(dispositivo);
}

// wasm func 9475 (vtable slot 4): direct (devirtualised) call of slot 5.
int CTrng::Gera(std::vector<uebyte>& buffer)
{
    return Gera(buffer, buffer.size());
}

// wasm func 5173 (vtable slot 5). The device is created inside the try, so a failing constructor also
// returns -1. Only std::exception is caught (typeid test with typeinfo @1525848).
int CTrng::Gera(std::vector<uebyte>& buffer, std::size_t quantidade)
{
    try {
        std::random_device dispositivo(TOKEN_DISPOSITIVO);
        if (quantidade == 0)
            return 1;
        if (buffer.size() < quantidade)
            return 2;
        for (std::size_t i = 0; i < quantidade; ++i)
            buffer[i] = static_cast<uebyte>(dispositivo());
        return 0;
    } catch (const std::exception&) {
        return -1;
    }
}

} // namespace ecourna::api::security
