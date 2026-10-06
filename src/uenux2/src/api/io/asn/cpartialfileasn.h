// uenux2/src/api/io/asn/cpartialfileasn.h
//
// Reconstructed from vota_web_wasm.wasm. Unit u18 (the three one-line slots fim/restante/posicao were
// written by u12 in cpartialfileasn.u12-fragment.h and are repeated here so that the class is complete).
//
// api::CFileSeeder is the ASN1::ISeeder that lets the TSE "partial" BER decoder (CoderEnv rule 2,
// BERDecoderPartial - docs/libraries/asn1-runtime.md §6) pull bytes straight from a file instead of from a
// memory buffer. The voter roll (ModuloEleitores::EntidadeEleitores, "-el.dat") is read this way, one
// voter at a time, so the whole file never has to be in memory. seek/decodeBlock/decodeByte were observed
// executing during votaInit (loading the scenario's voter roll).
//
// RTTI: api::CFileSeeder : ASN1::ISeeder (typeinfo @1564452), vtable @1564408:
//   [0]/[1] destructors  [2] 11434 fim()  [3] 11433 decodeByte()  [4] 11432 decodeBlock(char*, size_t)
//   [5] 11431 seek(size_t)  [6] 11430 skip(size_t)  [7] 2587 (ICF)  [8] 11429 restante()  [9] 11428 posicao()
//   [10] 3037 (ICF)
// Errors: api::EUeIoError (CBaseError limits {5950, 6150}); the file name comes from CFile::GetName().
#pragma once

#include <cstddef>
#include <cstdio>
#include <format>

#include "asn1/seeder.h"                  // ASN1::ISeeder (asn1-runtime)
#include "api/io/euioerror.h"             // api::CUeIoError
#include "ecourna/api/io/cfile.hpp"

namespace api {

class CFileSeeder : public ASN1::ISeeder {
public:
    CFileSeeder(const ecourna::api::io::CFile& arquivo, std::size_t tamanho)       // inlined; name inferred
        : m_arquivo(&arquivo), m_tamanho(tamanho) {}

    // slot 2, wasm func 11434 (u12; name inferred)
    bool fim() override { return m_arquivo->Eof(); }

    // slot 3, wasm func 11433 (srcloc line 52). Observed executing.
    unsigned char decodeByte() override
    {
        if (fim())
            throw CUeIoError(EUeIoError(5957),
                             std::format("Não há mais bytes disponíveis em {}", m_arquivo->GetName()));   // line 52
        unsigned char byte;
        m_arquivo->RawRead(&byte, 1);                  // result not checked
        return byte;
    }

    // slot 4, wasm func 11432 (srcloc line 75). Reads at most up to m_tamanho. Observed executing.
    std::size_t decodeBlock(char* destino, std::size_t quantidade) override
    {
        const auto posicao = static_cast<std::size_t>(m_arquivo->Position());
        const std::size_t aLer = (posicao + quantidade > m_tamanho) ? m_tamanho - posicao : quantidade;
        if (m_arquivo->RawRead(destino, static_cast<uedword>(aLer)) != aLer)
            throw CUeIoError(EUeIoError(5958),
                             std::format("Não foi possível ler {} bytes em {}", aLer, m_arquivo->GetName()));  // line 75
        return aLer;
    }

    // slot 5, wasm func 11431 (srcloc lines 88, 94). Observed executing.
    void seek(std::size_t posicao) override
    {
        if (posicao > m_tamanho)
            throw CUeIoError(EUeIoError(5959),
                             std::format("Não é possível reposicionar para {} em {}", posicao, m_arquivo->GetName()));  // line 88
        if (!m_arquivo->Seek(static_cast<int>(posicao), SEEK_SET))
            throw CUeIoError(EUeIoError(5960),
                             std::format("Não é possível reposicionar para {} em {}", posicao, m_arquivo->GetName()));  // line 94
    }

    // slot 6, wasm func 11430 (srcloc line 106)
    void skip(std::size_t quantidade) override
    {
        const std::size_t destino = static_cast<std::size_t>(m_arquivo->Position()) + quantidade;
        if (destino > m_tamanho)
            throw CUeIoError(EUeIoError(5961),
                             std::format("Não é possível saltar {} bytes em {}", quantidade, m_arquivo->GetName()));  // line 106
        seek(destino);                                 // virtual call (slot 5)
    }

    // slot 8, wasm func 11429 (u12; name inferred): bytes left before the end of the data
    std::size_t restante() override { return m_tamanho - static_cast<std::size_t>(m_arquivo->Position()); }

    // slot 9, wasm func 11428 (u12; name inferred)
    std::size_t posicao() override { return static_cast<std::size_t>(m_arquivo->Position()); }

private:
    const ecourna::api::io::CFile* m_arquivo;   // +4
    std::size_t                    m_tamanho;   // +8  end of the readable data (file size or a limit)
};

} // namespace api
