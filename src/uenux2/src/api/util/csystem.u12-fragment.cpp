// uenux2/src/api/util/csystem.cpp  -- FRAGMENT reconstructed by unit u12
// (funcs 5459 and 6019 were attributed to ecourna's icompressor.cpp only because their caller func 5821
//  carries the icompressor.cpp srcloc; they are uenux2 CSystem helpers next to api::ExistResource)
//
// Reconstructed from vota_web_wasm.wasm.
#include <string>
#include <sys/stat.h>

#include "api/util/csystem.h"

namespace api {

bool ExistResource(struct stat& info, const std::string& caminho);   // func 1690 (csystem.cpp:64, not u12)

namespace {
// wasm func 6019 (name inferred): the resource exists and has type `tipo` (S_IFDIR / S_IFREG).
bool EhDoTipo(const std::string& caminho, const unsigned tipo)
{
    struct stat info;
    return ExistResource(info, caminho) && (info.st_mode & S_IFMT) == tipo;
}
} // namespace

// wasm func 5459 (name inferred). Callers: CGravadorWSQ archive routine (5821), comum_f5572.
bool CSystem::IsDirectory(const std::string& caminho)
{
    return EhDoTipo(caminho, S_IFDIR);   // 16384
}

// wasm func 412 (not in u12; same shape, S_IFREG = 32768). Name inferred (other units call it
// IsRegularFile / FileExists).
bool CSystem::IsRegularFile(const std::string& caminho)
{
    return EhDoTipo(caminho, S_IFREG);
}

} // namespace api
