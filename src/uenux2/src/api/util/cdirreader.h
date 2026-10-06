// Reconstructed from vota_web_wasm.wasm (unit u20).
// Original: uenux2/src/api/util/cdirreader.h (class attested by srclocs cdirreader.cpp:30 / :43).
// A thin RAII wrapper over opendir/readdir used by CSystem::IsEmptyDir, CMontadorHash::CalculaHashGeral,
// comum::impl::CValidaMidia and vota::CAjusteInicial::ValidaTemposDesligamento.
#pragma once

#include <dirent.h>
#include <string>
#include <sys/types.h>

namespace api {

class CDirReader {                                   // 20 bytes
public:
    explicit CDirReader(const std::string& diretorio);   // func 1914
    ~CDirReader();                                        // func 1913 (tools: api_f1913)

    bool Open(const std::string& diretorio);              // func 5470 (other unit)   name inferred
    void Close();                                         // func 2232 (tools: api_f2232) name inferred
    bool NextEntry();                                     // func 1260

    // Name of the current entry (d_name at dirent+19), "" when there is none (always inlined;
    // the "" is the literal @450187).                                                 name inferred
    std::string GetEntryName() const { return m_entrada ? m_entrada->d_name : ""; }

    bool IsDirectory() const { return IsType(S_IFDIR); }  // func 1912 (tools: ecourna_f1912) name inferred
    bool IsFile() const { return IsType(S_IFREG); }       // func 2231 (tools: ecourna_f2231) name inferred
    bool IsLink() const;                                  // func 5469 (other unit): lstat + S_IFLNK

private:
    bool IsType(mode_t tipo) const;                       // func 6020 (tools: api_f6020)  name inferred

    std::string m_diretorio;        // +0
    DIR* m_dir = nullptr;           // +12
    dirent* m_entrada = nullptr;    // +16
};

} // namespace api
