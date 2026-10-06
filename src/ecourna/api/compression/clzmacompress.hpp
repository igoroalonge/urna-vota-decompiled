// ecourna-lib/ecourna/api/compression/clzmacompress.hpp   (path inferred; clzmacompress.cpp is known from srcloc)
//
// Reconstructed from vota_web_wasm.wasm. Unit u12.
//
// RTTI: ecourna::api::compression::CLzmaCompress (typeinfo @1108612) : ICompressor.   Vtable @1108448:
//   [0] ~CLzmaCompress (D1)  func 9582      [1] ~CLzmaCompress (D0)  func 9581
//   [2] Close()              func 218 (ICF: empty body)
//   [3] ExpandeGlob()        func 340 (ICF: `return 0`; 7-Zip expands wildcards itself)
//   [4] DoAdd(src, dst)      func 9586
//
// DEAD CODE in this binary: no function stores the vtable @1108448 (0 "stored by"), so no CLzmaCompress can
// be constructed. The 7-Zip 19.00 code that only DoAdd reaches (about 1,000 functions, 344 KB) is linked but
// can never run. See docs/libraries/compression-7zip-lzma-zlib.md §5.
#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

#include "ecourna/api/compression/icompressor.hpp"

namespace ecourna::api::compression {

class CLzmaCompress : public ICompressor {
public:
    CLzmaCompress(const std::filesystem::path& arquivo, ECompressLevel nivel);   // ? not in the binary

    ~CLzmaCompress() override = default;          // funcs 9582 (D1: ~m_arquivo + func 1526) / 9581 (D0)

    void Close() override {}                      // func 218
    bool ExpandeGlob() const override { return false; }   // func 340

protected:
    void DoAdd(const std::filesystem::path& origem, const std::filesystem::path& destino) override;   // wasm func 9586 (srcloc line 114)

private:
    // Bound with std::bind(&CLzmaCompress::OnProgress, this, _1, _2) into a std::function (functor vtable
    // @1109952). Body = func 9585 (unit u40): emits m_progresso(..., "Compactando arquivo {}...").
    void OnProgress(std::uint64_t processados, const std::string& nome);         // name inferred

    std::filesystem::path m_arquivo;   // +16 the .7z package
    ECompressLevel        m_nivel;     // +32 0 = "Copy" method, anything else = LZMA
};

} // namespace ecourna::api::compression
