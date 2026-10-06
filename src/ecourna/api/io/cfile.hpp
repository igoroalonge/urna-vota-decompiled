// ecourna-lib/ecourna/api/io/cfile.hpp   (path inferred: the .cpp path is known from srclocs, ecourna headers use .hpp)
//
// Reconstructed from vota_web_wasm.wasm (VOTA "10.23.0.1 - DESENVOLVIMENTO", web simulator build).
// Unit u12, see docs/modules/u12-ecourna-lib-ecourna-api-compression-ecourna-lib-ecourna-api-.md
//
// ecourna::api::io::CFile is the urna's thin RAII wrapper over a C stdio FILE*. Almost every file the
// voting application reads or writes goes through it: the ASN.1 data files (CFileASN / CPartialFileASN),
// the encrypted RDV (api::CEncryptedFile), the state files eg.bin/vota.bin/gap.bin/sa.bin, WSQ images,
// the files added to .jez (ZIP) archives, key envelopes, ...
//
// It is NOT polymorphic (no vtable; offset 0 is the FILE*). sizeof(CFile) == 20.
// Every failure throws CIoError (cioerror.hpp) with an EFileOperation and errno.
#pragma once

#include <cstdio>
#include <filesystem>
#include <string>
#include <vector>

#include "ecourna/api/io/cioerror.hpp"
#include "ecourna/api/types.hpp"          // uebyte / uedword (TSE fixed-width typedefs, not in this unit)

namespace ecourna::api::io {

class CFile {
public:
    // Bit flags given to Open(); applied by SetFileMode(). Values from the code, names inferred:
    //   bit 0 -> O_SYNC (0x101000),  bit 1 -> O_NOATIME (0x40000)   (musl values)
    enum FileMode : int {
        FM_NORMAL  = 0,   // used by every "rb" reader
        FM_SYNC    = 1,
        FM_NOATIME = 2,   // used by the mesário WSQ writer (func 5371, mode "w+b")
    };

    CFile() = default;                                                          // ? (the object is zero-filled by func 517)

    // wasm func 517 (assigned to unit u18): m_file = nullptr, m_name = "", m_syncOnClose = false;
    // then Open() only if `name` is not empty.
    CFile(const std::string& name, const std::string& mode, FileMode fileMode = FM_NORMAL);

    // wasm func 3564: closes the file if it is still open and swallows any exception.
    ~CFile();

    bool    Open(const std::string& name, const std::string& mode, FileMode fileMode);   // wasm func 9508 (srcloc 60, 74)
    void    Close();                                                          // wasm func 336  (srcloc 111)
    int     Flush() const;                                                    // inlined in func 2767 (srcloc 145)
    void    Sync() const;                                                     // wasm func 5181 (srcloc 160..175)
    uedword RawWrite(const void* buffer, uedword size) const;                 // wasm func 1886 (srcloc 184..195)
    uedword RawRead(void* buffer, uedword size) const;                        // wasm func 598  (srcloc 222..240)
    uedword ReadLine(std::string& line) const;                                // inlined in func 5480 (srcloc 336, 356)
    bool    Seek(int offset, int origin) const;                               // wasm func 419  (srcloc 363, 366)
    bool    Eof() const;                                                      // wasm func 1885 (srcloc 374)
    long    Position() const;                                                 // wasm func 418  (srcloc 385, 389)

    // wasm func 5180 (name inferred): convenience overload, a no-op for an empty vector.
    void RawWrite(const std::vector<uebyte>& data) const
    {
        if (data.empty())
            return;
        RawWrite(data.data(), static_cast<uedword>(data.size()));
    }

    bool               IsOpen() const  { return m_file != nullptr; }          // name inferred (inlined everywhere)
    const std::string& GetName() const { return m_name; }                     // name inferred (CFileASN: "WriteToFile de " + name)

    static std::string         ReadFileContent(const std::string& fileName);             // wasm func 3522 (srcloc 420)
    static std::vector<uebyte> ReadFileBinary(const std::filesystem::path& fileName);    // wasm func 3521 (srcloc 435, 447)

private:
    bool SetFileMode(FileMode fileMode);                                      // inlined in func 9508 (srcloc 117)

    FILE*       m_file{nullptr};        // +0
    std::string m_name;                 // +4   (12-byte libc++ string)
    bool        m_syncOnClose{false};   // +16  set by Open() when the mode string contains 'w', 'a' or '+';
                                        //      never cleared again (not even by Close())
};                                      // sizeof 20

} // namespace ecourna::api::io
