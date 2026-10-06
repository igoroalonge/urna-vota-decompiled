// ecourna-lib/ecourna/api/util/cbitarray.cpp
// (/home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/api/util/cbitarray.cpp in the srcloc records)
//
// Reconstructed from vota_web_wasm.wasm. Unit u13.
//
// wasm func 1375 is GetValor(uebyte) with GetValor(ueqword, uebyte) inlined (it carries the srclocs of both:
// lines 44, 56 and 64).
#include "ecourna/api/util/cbitarray.hpp"

#include <memory>

#include "ecourna/api/util/cstringutils.hpp"   // EUtilError, CUtilError

namespace ecourna::api::util {

// Constructor: only exists inlined into comum::asn::CConversorDedo (func 5702).                        // ?
CBitArray::CBitArray()
    : m_bits(std::make_shared<boost::dynamic_bitset<uebyte>>())
{
}

// wasm func 1375 (srcloc line 44): reads `quantidade` bits at the cursor and advances it.
// The message talks about WRITE permission although the flag guards reading (as found in the binary).
ueword CBitArray::GetValor(const uebyte quantidade)
{
    if (!m_leitura)
        throw CUtilError(EUtilError::BitArraySemLeitura, "sem permissao de escrita no array de bits");   // line 44
    const ueword valor = GetValor(m_posicao, quantidade);
    m_posicao += quantidade;
    return valor;
}

// Inlined into wasm func 1375 (srcloc lines 56 and 64). Bits are read MSB-first:
// bit (posicao + i) becomes bit (quantidade - 1 - i) of the result.
// quantidade > 16 loses the high bits (ueword result); quantidade > 32 would shift by >= 32 (undefined in
// C++; wasm's i32.shl masks the count).
ueword CBitArray::GetValor(const ueqword posicao, const uebyte quantidade) const
{
    if (!m_bits)
        throw CUtilError(EUtilError::BitArrayNaoAlocado, "array de bits nao alocado corretamente");      // line 56
    if (quantidade == 0)
        return 0;
    // Compiled as "quantidade - 1 >= (size > posicao ? size - posicao : 0)", i.e. posicao + quantidade > size.
    if (posicao + quantidade > m_bits->size())
        throw CUtilError(EUtilError::BitArrayPosicaoInvalida, "busca no vetor em posicao invalida");     // line 64

    ueword valor = 0;
    for (uebyte i = 0; i < quantidade; ++i) {
        if ((*m_bits)[posicao + i])
            valor |= static_cast<ueword>(1u << (quantidade - 1 - i));
    }
    return valor;
}

} // namespace ecourna::api::util
