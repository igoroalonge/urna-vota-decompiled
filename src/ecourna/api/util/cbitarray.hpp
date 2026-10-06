// ecourna-lib/ecourna/api/util/cbitarray.hpp   (path inferred from the .cpp in the srcloc records)
//
// Reconstructed from vota_web_wasm.wasm. Unit u13.
//
// RTTI: ecourna::api::util::CBitArray (no base; typeinfo @1566540, vtable @1566532 with only the two
// destructors: [0] func 11417 complete, [1] func 11414 deleting).
// A bit reader over a boost::dynamic_bitset<uebyte>, MSB-first. Its only user in this binary is the
// biometric-template converter comum::asn::CConversorDedo (func 5702, called by CConversorDedo::vf3,
// func 11416), which builds the object on the stack (constructor inlined there):
//     vptr; m_bits = std::make_shared<boost::dynamic_bitset<uebyte>>() (emplace block, 28 bytes);
//     m_buffer = {}; m_leitura = true; m_escrita = false; m_posicao = 0.
#pragma once

#include <memory>
#include <vector>

#include <boost/dynamic_bitset.hpp>

#include "ecourna/types.hpp"

namespace ecourna::api::util {

class CBitArray {
public:
    CBitArray();                          // inlined into func 5702                          // ?
    virtual ~CBitArray() = default;       // wasm func 11417 (complete) / 11414 (deleting)

    ueword GetValor(const uebyte quantidade);                                // cbitarray.cpp:44 } both in
    ueword GetValor(const ueqword posicao, const uebyte quantidade) const;   // cbitarray.cpp:56, :64 } func 1375

private:
    std::shared_ptr<boost::dynamic_bitset<uebyte>> m_bits;   // +4  (+8 control block)
                                                             //     dynamic_bitset = { vector<uebyte> m_bits; size_t m_num_bits (+12) }
    std::vector<uebyte> m_buffer;                            // +12 ? (destroyed by the destructor; not used by GetValor)
    bool                m_leitura{true};                     // +24 read mode (checked by GetValor)       name inferred
    bool                m_escrita{false};                    // +25 ? (initialised with the same 16-bit store)
    ueqword             m_posicao{0};                        // +32 read cursor, in bits
};                                                           // sizeof 40

} // namespace ecourna::api::util
