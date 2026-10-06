// ecourna-lib/ecourna/api/compression/czip.hpp   (path inferred; czip.cpp is known from srcloc)
//
// Reconstructed from vota_web_wasm.wasm. Unit u12 (constructor/CreateZipFile = unit u15).
//
// RTTI: ecourna::api::compression::CZip (typeinfo @1110824) : ICompressor.   Vtable @1110676:
//   [0] ~CZip (D1)        func 2690      [1] ~CZip (D0)   func 9540
//   [2] Close()           func 2691      [3] ExpandeGlob() func 434 (ICF body `return 1`)
//   [4] DoAdd(src, dst)   func 9538
//
// CZip writes ZIP archives through minizip 1.3.1 (zip.c + ioapi.c, deflate-only zlib). In this code base
// ZIP archives carry the extension ".jez" (log.jez, wsqbio.jez, wsqman.jez, wsqmes.jez, ...).
#pragma once

#include <cstdint>
#include <filesystem>
#include <map>
#include <string>

#include "ecourna/api/compression/icompressor.hpp"
#include "minizip/zip.h"   // zipFile

namespace ecourna::api::compression {

class CZip : public ICompressor {
public:
    // Open mode, converted by ConvertToOpen() into minizip's APPEND_STATUS_*; names inferred.
    enum class EOpenMode : int {
        Criar     = 0,   // APPEND_STATUS_CREATE   (the only value ever stored: the constructors write 0)
        Adicionar = 1,   // APPEND_STATUS_ADDINZIP
    };

    // wasm func 5193 (unit u15): m_arquivo = arquivo, m_nivel = nivel, m_modo = Criar, m_zip = nullptr,
    // then CreateZipFile() (func 9543, unit u15).
    CZip(const std::filesystem::path& arquivo, ECompressLevel nivel);

    // Inlined into comum::CGravadorLog's archive routine (func 11584): builds the archive and adds all the
    // files; if Add() throws, the archive is closed quietly (func 9542) and the exception is rethrown.
    // (constructor shape inferred from the inlined code)
    CZip(const std::filesystem::path& arquivo, ECompressLevel nivel,
         const std::map<std::filesystem::path, std::filesystem::path>& arquivos);

    ~CZip() override;                                              // wasm funcs 2690 (D1) / 9540 (D0)

    void Close() override;                                         // wasm func 2691 (srcloc line 139)
    bool ExpandeGlob() const override { return true; }             // wasm func 434 (shared `return 1`)

protected:
    void DoAdd(const std::filesystem::path& origem, const std::filesystem::path& destino) override;   // wasm func 9538

private:
    void        CreateZipFile();                                   // wasm func 9543 (unit u15; srcloc line 170)
    int         ConvertToOpen() const;                             // inlined in 9543 (srcloc line 155)
    int         ConvertToCompressLevel() const;                    // wasm func 9539 (srcloc line 207)
    void        AssertZipIsOpened() const;                         // inlined in 9538 (srcloc line 188)
    void        NotificaProgresso(std::uint64_t processados, std::uint64_t total,
                                  const std::string& nome);        // wasm func 9537 (name inferred)
    void        FechaSemExcecao();                                 // wasm func 9542 (name inferred): try { Close(); } catch (...) {}
    static std::string ErrorToString(int erro);                    // wasm func 9510 (name inferred)

    static constexpr std::size_t TAMANHO_BLOCO = 1024;             // read size used by DoAdd (name inferred)

    std::filesystem::path m_arquivo;                               // +16 archive path
    zipFile               m_zip{nullptr};                          // +28 minizip handle (zip64_internal*)
    ECompressLevel        m_nivel;                                 // +32
    EOpenMode             m_modo{EOpenMode::Criar};                // +36
};                                                                 // sizeof 40

} // namespace ecourna::api::compression
