// FRAGMENT reconstructed by unit u15 from vota_web_wasm.wasm.
// Original file: ecourna-lib/ecourna/api/compression/czip.cpp (Conan package libecea1da310e5107,
// srclocs czip.cpp:155 and :170). Only the constructor and CreateZipFile are here; Close, DoAdd,
// ConvertToCompressLevel and the destructors belong to unit u12. Merge into czip.cpp.
#include "ecourna/api/compression/czip.u15.hpp"

#include <format>

#include <minizip/ioapi.h>
#include <minizip/zip.h>

namespace ecourna::api::compression {

// wasm func 5193 (observed: no)  - czip.cpp, constructor                              // name confirmed by
// vtable store (CZip @1110676). Only direct caller: CGravadorWSQ (func 5821, level 0 = stored, or 2 for
// an archive that is closed without adding anything). CGravadorLog (func 11584, level 2 = default
// deflate, "temp.jez") does not call it: it has this constructor inlined (it stores the CZip vtable
// itself and runs CreateZipFile through invoke_vi, table slot 6119).
// The archive is created immediately: CreateZipFile() runs inside the constructor (through
// invoke_vi, i.e. inside a try region whose landing pad destroys m_arquivo and the ICompressor base).
CZip::CZip(const std::filesystem::path& arquivo, ECompressLevel nivel)
    : ICompressor(),                     // func 5192
      m_arquivo(arquivo),
      m_zip(nullptr),
      m_nivel(nivel),
      m_modoAbertura(0)                  // always "create": the ADDINZIP branch below is dead code here
{
    CreateZipFile();
}

// Inlined into CreateZipFile (srcloc czip.cpp:155).
int CZip::ConvertToOpen()
{
    switch (m_modoAbertura) {
    case 0: return APPEND_STATUS_CREATE;      // 0
    case 1: return APPEND_STATUS_ADDINZIP;    // 2
    default:
        throw CCompressionError(static_cast<ECompressionError>(1040),
                                "Modo de abertura do zip incorreto.");       // srcloc czip.cpp:155
    }
}

// wasm func 9543 (observed: no)  - srcloc czip.cpp:170 (+ inlined ConvertToOpen :155)
// 4.4 KB because minizip's zipOpen3() is inlined: fill_fopen64_filefunc (table slots 8253..8259),
// the 64 KiB zip64_internal on the stack, and - for APPEND_STATUS_ADDINZIP only - the backwards
// search for the (zip64) end-of-central-directory record (malloc(1028), scan of at most 0xFFFF bytes
// for "PK\5\6" / "PK\6\7") and the reload of the existing central directory in 4080-byte blocks.
// With m_modoAbertura always 0 in this binary, only the CREATE path is reachable.
void CZip::CreateZipFile()
{
    zlib_filefunc64_def funcoes;
    fill_fopen64_filefunc(&funcoes);
    m_zip = zipOpen2_64(m_arquivo.c_str(), ConvertToOpen(), /*globalcomment*/ nullptr, &funcoes);
    if (m_zip == nullptr)
        throw CCompressionError(static_cast<ECompressionError>(1041),
                                std::format("O arquivo {} não pode ser criado.", m_arquivo.string()));
                                                                               // srcloc czip.cpp:170
}

} // namespace ecourna::api::compression
