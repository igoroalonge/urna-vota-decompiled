// uenux2/src/api/io/asn/cpartialfileasn.h  -- FRAGMENT reconstructed by unit u12
// (api::CFileSeeder lives in this header: its decodeByte/skip srclocs are cpartialfileasn.h:52/:106.
//  u12 owns only the three one-line virtuals below)
//
// Reconstructed from vota_web_wasm.wasm.
//
// api::CFileSeeder (typeinfo @1564452) : ASN1::ISeeder. The streaming ("partial") BER decoder reads the
// voter file EntidadeEleitores through it (docs/libraries/asn1-runtime.md §6). Vtable @1564408:
//   [2] 11434 fim()   [3] 11433 decodeByte   [4] 11432 decodeBlock   [5] 11431 seek   [6] 11430 skip
//   [7] 2587 (ICF)    [8] 11429 restante()   [9] 11428 posicao()     [10] 3037 (ICF)
// Slot names 2/8/9 are inferred from the bodies and from ASN1::BufferSeeder's slots.
#pragma once

#include <cstddef>

#include "asn1/seeder.h"                  // ASN1::ISeeder (asn1-runtime)
#include "ecourna/api/io/cfile.hpp"

namespace api {

class CFileSeeder : public ASN1::ISeeder {
public:
    // slot 2, wasm func 11434 (name inferred)
    bool fim() override { return m_arquivo->Eof(); }

    // slot 8, wasm func 11429 (name inferred): bytes left before the end of the data
    std::size_t restante() override { return m_tamanho - static_cast<std::size_t>(m_arquivo->Position()); }

    // slot 9, wasm func 11428 (name inferred)
    std::size_t posicao() override { return static_cast<std::size_t>(m_arquivo->Position()); }

private:
    const ecourna::api::io::CFile* m_arquivo;   // +4
    std::size_t                    m_tamanho;   // +8 total size (file size or limit)
};

} // namespace api
