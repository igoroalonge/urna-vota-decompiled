// FRAGMENT reconstructed by unit u15 from vota_web_wasm.wasm.
// Original file: ecourna-lib/ecourna/api/compression/icompressor.cpp (the constructor, globToFileList
// and the other Add overloads are reconstructed by unit u12). Merge into icompressor.cpp.
#include "ecourna/api/compression/czip.u15.hpp"

namespace ecourna::api::compression {

// wasm func 9529 (observed: no; table slot 6050)                                     // name inferred
// Adds every (source file -> name inside the archive) pair of the map, in key order, through the
// virtual DoAdd (vtable slot 4). Returns *this so calls can be chained.
// Only caller: comum::CGravadorLog::vf7 (func 11584), which zips the log files into "temp.jez".
ICompressor& ICompressor::Add(const std::map<std::filesystem::path, std::filesystem::path>& arquivos)
{
    for (const auto& [origem, destino] : arquivos)   // node +16 = key, node +28 = value
        DoAdd(origem, destino);
    return *this;
}

} // namespace ecourna::api::compression
