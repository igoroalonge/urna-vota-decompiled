// ecourna-lib/ecourna/api/security/cprng.cpp   (path inferred)
//
// Reconstructed from vota_web_wasm.wasm. Unit u40.
//
// Library code inlined here (not reconstructed): boost::random::mt19937::seed(value) (the 623-step
// "x[i] = i + 1812433253 * (x[i-1] ^ x[i-1] >> 30)" loop), normalize_state() (the x[396] ^ x[623] fix-up of
// x[0] with constant 0x321161BF = (0x9908B0DF << 1) | 1, and the all-zero check that sets x[0] = 0x80000000),
// the tempering (>> 11, << 7 & 0x9D2C5680, << 15 & 0xEFC60000, >> 18) and boost's generate_uniform_int.
// wasm func 5179 is the out-of-line mersenne_twister_engine::twist() (624-word regeneration, unrolled);
// Gera(buffer, n) inlines its own copy of twist().
#include "ecourna/api/security/cprng.hpp"

#include <limits>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

namespace ecourna::api::security {

// wasm func 9503. Called only by main through invoke_iii(114, obj, 0).
CPrng::CPrng(std::uint32_t semente)
    : m_semente(semente)
{
}

// wasm func 9502 (vtable slot 2). The seed is read with i32.load8_u: only the low byte of m_semente seeds
// the engine here (and in Gera(buffer, n)), while Gera() uses all 32 bits. Seeds that differ only above
// bit 7 therefore give identical byte streams.                                             // ? source form
uebyte CPrng::GeraByte()
{
    boost::random::mt19937 gerador(static_cast<uebyte>(m_semente));
    boost::random::uniform_int_distribution<unsigned> distribuicao(0, 255);   // result = output >> 24
    return static_cast<uebyte>(distribuicao(gerador));
}

// wasm func 9500 (vtable slot 3). Rejection loop `while (z > 0xFFFFFFFD)` then `z - 0x7FFFFFFF`:
// boost uniform_int over [INT_MIN + 1, INT_MAX - 1] (bucket size 1).                      // ? bounds spelling
int CPrng::Gera()
{
    boost::random::mt19937 gerador(m_semente);
    boost::random::uniform_int_distribution<int> distribuicao(std::numeric_limits<int>::min() + 1,
                                                              std::numeric_limits<int>::max() - 1);
    return distribuicao(gerador);
}

// wasm func 9499 (vtable slot 4): direct call of slot 5.
int CPrng::Gera(std::vector<uebyte>& buffer)
{
    return Gera(buffer, buffer.size());
}

// wasm func 5178 (vtable slot 5). One engine for the whole buffer (so the bytes differ from each other),
// but the same bytes on every call. No try/catch (nothing here throws).
int CPrng::Gera(std::vector<uebyte>& buffer, std::size_t quantidade)
{
    boost::random::mt19937 gerador(static_cast<uebyte>(m_semente));             // load8_u, see GeraByte
    if (quantidade == 0)
        return 1;
    if (buffer.size() < quantidade)
        return 2;
    boost::random::uniform_int_distribution<unsigned> distribuicao(0, 255);
    for (std::size_t i = 0; i < quantidade; ++i)
        buffer[i] = static_cast<uebyte>(distribuicao(gerador));
    return 0;
}

} // namespace ecourna::api::security
