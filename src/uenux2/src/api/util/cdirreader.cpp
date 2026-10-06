// Reconstructed from vota_web_wasm.wasm (unit u20).
// Original: uenux2/src/api/util/cdirreader.cpp (srclocs cdirreader.cpp:30, :43).
#include "api/util/cdirreader.h"

#include <string>
#include <sys/stat.h>

namespace api {

// wasm func 1914                                                             srcloc cdirreader.cpp:30
CDirReader::CDirReader(const std::string& diretorio)
{
    if (diretorio.empty())
        return;                                            // an empty reader: NextEntry() will throw
    Open(diretorio);                                       // func 5470
    if (!m_dir)
        throw CUeUtilError(EUeUtilError{7009}, "Não foi possível ler o diretório " + diretorio);
}

// wasm func 1913 (destructor; Close() inlined)
CDirReader::~CDirReader()
{
    Close();
}

// wasm func 2232                                                                    name inferred
void CDirReader::Close()
{
    if (m_dir) {
        ::closedir(m_dir);                                 // func 957 (musl)
        m_diretorio.clear();
        m_dir = nullptr;
        m_entrada = nullptr;
    }
}

// wasm func 5470 (other unit, shown for context)                                    name inferred
bool CDirReader::Open(const std::string& diretorio)
{
    DIR* dir = ::opendir(diretorio.c_str());
    if (dir) {
        Close();
        m_diretorio = diretorio;
        m_entrada = nullptr;
        m_dir = dir;
    }
    return dir != nullptr;
}

// wasm func 1260                                                             srcloc cdirreader.cpp:43
bool CDirReader::NextEntry()
{
    if (!m_dir)
        throw CUeUtilError(EUeUtilError{7010}, "O diretório não está aberto");
    m_entrada = ::readdir(m_dir);
    return m_entrada != nullptr;
}

// wasm func 6020 (tools: api_f6020). Thunks: 1912 = IsType(S_IFDIR), 2231 = IsType(S_IFREG).  name inferred
bool CDirReader::IsType(mode_t tipo) const
{
    if (!m_entrada)
        return false;
    const std::string caminho = m_diretorio + "/" + m_entrada->d_name;
    struct stat info;
    return ::stat(caminho.c_str(), &info) != -1 && (info.st_mode & S_IFMT) == tipo;
}

} // namespace api
