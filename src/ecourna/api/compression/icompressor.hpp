// ecourna-lib/ecourna/api/compression/icompressor.hpp   (path inferred; icompressor.cpp is known from srcloc)
//
// Reconstructed from vota_web_wasm.wasm. Unit u12 (functions 5192, 9530 and the inlined Add(path) /
// globToFileList; Add(map) = func 9529 belongs to unit u15).
//
// RTTI: ecourna::api::compression::ICompressor (typeinfo @1110916, __vmi_class_type_info)
//         : pattern::NonCopyable (empty, typeinfo @1526780), pattern::IObservableProgressWithDescription
// Vtable @1110896:
//   [0] ~ICompressor (D1)      -> func 1526 (= ~IObservableProgressWithDescription: nothing else to destroy)
//   [1] ~ICompressor (D0)      -> func 325  (`unreachable`: the deleting destructor of an abstract class)
//   [2] Close()                 pure
//   [3] ExpandeGlob() const     pure        name inferred: CZip returns true (func 434), CLzmaCompress false (func 340)
//   [4] DoAdd(src, dst)         pure        (protected)
//
// Implementations: CZip (minizip/zlib, used by the application) and CLzmaCompress (7-Zip, dead code).
#pragma once

#include <filesystem>
#include <map>
#include <vector>

#include "ecourna/api/pattern/iobservableprogresswithdescription.hpp"
#include "ecourna/api/pattern/noncopyable.hpp"   // pattern::NonCopyable (empty base, not in this unit)

namespace ecourna::api::compression {

// Compression level shared by the implementations. Values from CZip::ConvertToCompressLevel (table @1110872);
// enumerator names inferred.
enum class ECompressLevel : int {
    Armazenar = 0,   // "store": zlib level 0 and method 0 (no deflate)
    Rapido    = 1,   // zlib level 1
    Padrao    = 2,   // Z_DEFAULT_COMPRESSION (-1 = zlib level 6)
    Maximo    = 3,   // zlib level 9
};

class ICompressor : public pattern::NonCopyable, public pattern::IObservableProgressWithDescription {
public:
    ICompressor();                                               // wasm func 5192
    ~ICompressor() override = default;                           // slot 0 = func 1526

    virtual void Close() = 0;                                    // slot 2
    virtual bool ExpandeGlob() const = 0;                        // slot 3  (name inferred)

    // Adds one file, or every file matched by a glob ('*', '?') when the implementation asks for it.
    void Add(const std::filesystem::path& arquivo);                                    // inlined in func 5821
    // Adds every file under its own file name (the directory part is dropped inside the archive).
    void Add(const std::vector<std::filesystem::path>& arquivos);                      // wasm func 9530
    // Adds every {source -> name inside the archive} pair. func 9529 returns `this` (sig (i32,i32)->i32),
    // hence the reference return type (the other overloads return void).
    ICompressor& Add(const std::map<std::filesystem::path, std::filesystem::path>& arquivos);  // wasm func 9529 (unit u15)

protected:
    virtual void DoAdd(const std::filesystem::path& origem, const std::filesystem::path& destino) = 0;   // slot 4
};                                                               // sizeof 16

} // namespace ecourna::api::compression
