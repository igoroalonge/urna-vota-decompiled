// FRAGMENT reconstructed by unit u34 from vota_web_wasm.wasm.
// Original file: uenux2/src/api/util/cdirreader.cpp (attested by srcloc :30/:43; owner unit u20, see
// cdirreader.h/.cpp). Layout: +0 std::string m_diretorio, +12 DIR* m_dir, +16 dirent* m_entrada.
#include <dirent.h>
#include <string>
#include <sys/stat.h>

#include "api/util/cdirreader.h"

namespace api {

// wasm func 5470 (tools: api_f5470; the owner shows the same body "for context")         name inferred
// Callers: CDirReader::CDirReader (1914), CMontadorHash::CalculaHashGeral (5360: hash.dat of the media).
bool CDirReader::Open(const std::string& diretorio)
{
    DIR* dir = ::opendir(diretorio.c_str());                   // musl opendir (func 1737)
    if (dir) {
        Close();                                               // inlined: closedir (957), m_diretorio.clear()
        m_diretorio = diretorio;
        m_entrada = nullptr;
        m_dir = dir;
    }
    return dir != nullptr;
}

// wasm func 5469 (tools: api_f5469)                                                       name inferred
// Is the current entry a symbolic link? lstat (not stat) of "<diretorio>/<d_name>".
// Callers: CMontadorHash::CalculaHashGeral (5360: links are hashed as links, not followed) and
// api::(anonymous)::RemoveConteudoDiretorio (5458, csystem.cpp).
bool CDirReader::IsLink() const
{
    if (m_entrada == nullptr)
        return false;
    const std::string caminho = m_diretorio + "/" + m_entrada->d_name;   // d_name at dirent +19
    struct stat info {};
    return ::lstat(caminho.c_str(), &info) != -1 && S_ISLNK(info.st_mode);   // (st_mode & 0xF000) == 0xA000
}

}  // namespace api
