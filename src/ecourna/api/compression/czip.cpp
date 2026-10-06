// ecourna-lib/ecourna/api/compression/czip.cpp
// (/home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/api/compression/czip.cpp in the srcloc records)
//
// Reconstructed from vota_web_wasm.wasm. Unit u12 (5193 CZip::CZip and 9543 CreateZipFile belong to unit
// u15 and are reproduced here, marked as such, so that the file reads as a whole).
//
// minizip functions that appear below (see docs/libraries/compression-7zip-lzma-zlib.md):
//   zipOpenNewFileInZip2_64 = func 8118, zipWriteInFileInZip = 8117, zipCloseFileInZip = 8119,
//   zipClose = inlined into CZip::Close (func 2691), zipOpen3 + fill_fopen64_filefunc inlined into 9543.
#include "ecourna/api/compression/czip.hpp"

#include <chrono>
#include <ctime>
#include <filesystem>
#include <format>
#include <string>
#include <vector>

#include "ecourna/api/compression/ecompressionerror.hpp"   // CCompressionError (not reconstructed)
#include "ecourna/api/io/cfile.hpp"
#include "minizip/ioapi.h"
#include "minizip/zip.h"

namespace ecourna::api::compression {

// -------------------------------------------------------------------------------------------------------
// wasm func 5193 (unit u15)
CZip::CZip(const std::filesystem::path& arquivo, const ECompressLevel nivel)
    : ICompressor(), m_arquivo(arquivo), m_zip(nullptr), m_nivel(nivel), m_modo(EOpenMode::Criar)
{
    CreateZipFile();                              // invoke_vi(slot 6119); on throw: ~path, ~ICompressor
}

// inlined into func 11584 (comum::CGravadorLog). Delegating constructor: once the target constructor has
// finished, the destructor also runs on unwind (the landing pad destroys m_arquivo and the base).
CZip::CZip(const std::filesystem::path& arquivo, const ECompressLevel nivel,
           const std::map<std::filesystem::path, std::filesystem::path>& arquivos)
    : CZip(arquivo, nivel)
{
    try {
        Add(arquivos);                            // func 9529
    } catch (...) {
        FechaSemExcecao();                        // func 9542 through invoke_vi(slot 6122)
        throw;
    }
}

// wasm func 2690 (complete) / func 9540 (deleting = 2690 + free)
CZip::~CZip()
{
    try {
        Close();
    } catch (...) {
    }
    // ~m_arquivo, ~ICompressor (= ~IObservableProgressWithDescription, func 1526)
}

// wasm func 9542 (name inferred). Only caller: the catch(...) of the file-list constructor above.
void CZip::FechaSemExcecao()
{
    try {
        Close();
    } catch (...) {
    }
}

// wasm func 9543 (unit u15; srcloc line 170). zipOpen3() and ConvertToOpen() are inlined.
void CZip::CreateZipFile()
{
    zlib_filefunc64_def funcoes;
    fill_fopen64_filefunc(&funcoes);              // slots 8259..8253 (fopen64/read/write/tell/seek/close/error)
    m_zip = zipOpen2_64(m_arquivo.c_str(), ConvertToOpen(), nullptr, &funcoes);
    if (m_zip == nullptr) {
        throw CCompressionError(ECompressionError(1041),
                                std::format("O arquivo {} não pode ser criado.", m_arquivo.string()));   // line 170
    }
}

// inlined into func 9543 (srcloc line 155)
int CZip::ConvertToOpen() const
{
    switch (m_modo) {
    case EOpenMode::Criar:     return APPEND_STATUS_CREATE;      // 0
    case EOpenMode::Adicionar: return APPEND_STATUS_ADDINZIP;    // 2
    }
    throw CCompressionError(ECompressionError(1040), "Modo de abertura do zip incorreto.");   // line 155
}

// wasm func 9539 (srcloc line 207). Table @1110872 = { 0, 1, -1, 9 }.
int CZip::ConvertToCompressLevel() const
{
    static constexpr int NIVEIS[] = {0, 1, Z_DEFAULT_COMPRESSION, 9};     // name inferred
    const auto i = static_cast<unsigned>(m_nivel);
    if (i >= 4)
        throw CCompressionError(ECompressionError(1043), "Modo de compressão incorreto.");   // line 207
    return NIVEIS[i];
}

// inlined into func 9538 (srcloc line 188)
void CZip::AssertZipIsOpened() const
{
    if (m_zip == nullptr)
        throw CCompressionError(ECompressionError(1042), std::format("O arquivo {} está fechado.", m_arquivo.string()));   // line 188
}

// wasm func 2691 (srcloc line 139). minizip's zipClose() is inlined (central directory, zip64 end record).
void CZip::Close()
{
    if (m_zip == nullptr)
        return;

    const int erro = zipClose(m_zip, nullptr);    // global comment = zi->globalcomment (NULL here)
    m_zip = nullptr;
    if (erro != ZIP_OK) {
        throw CCompressionError(ECompressionError(1039),
                                std::format("O arquivo {} não pode ser fechado. {}", m_arquivo.string(),
                                            ErrorToString(erro)));                      // line 139
    }
}

// wasm func 9538 (srcloc lines 244, 264, 281; AssertZipIsOpened inlined)
void CZip::DoAdd(const std::filesystem::path& origem, const std::filesystem::path& destino)
{
    AssertZipIsOpened();

    // Entry time = the file's modification time, converted to local time.
    zip_fileinfo info{};                          // dosDate = internal_fa = external_fa = 0
    const std::time_t modificado = std::chrono::system_clock::to_time_t(
        std::chrono::file_clock::to_sys(std::filesystem::last_write_time(origem)));    // ns -> us -> s
    const std::tm* const local = std::localtime(&modificado);
    info.tmz_date.tm_sec  = local->tm_sec;
    info.tmz_date.tm_min  = local->tm_min;
    info.tmz_date.tm_hour = local->tm_hour;
    info.tmz_date.tm_mday = local->tm_mday;
    info.tmz_date.tm_mon  = local->tm_mon;
    info.tmz_date.tm_year = local->tm_year;

    const std::uintmax_t tamanho = std::filesystem::file_size(origem);   // throws filesystem_error on failure

    int erro = zipOpenNewFileInZip2_64(m_zip, destino.c_str(), &info,
                                       nullptr, 0, nullptr, 0, nullptr,          // no extra fields, no comment
                                       m_nivel != ECompressLevel::Armazenar ? Z_DEFLATED : 0,
                                       ConvertToCompressLevel(),
                                       0,                                        // raw = 0
                                       tamanho > 0xFFFFFFFEu);                   // zip64 only when needed
    if (erro != ZIP_OK) {
        throw CCompressionError(ECompressionError(1044),
                                std::format("O arquivo {} não pode ser adicionado. {}", origem.string(),
                                            ErrorToString(erro)));              // line 244
    }

    io::CFile arquivo(origem.string(), "rb");
    std::uint64_t processados = 0;
    while (!arquivo.Eof()) {
        std::vector<uebyte> bloco(TAMANHO_BLOCO);                              // new zeroed 1 KiB per iteration
        const uedword lidos = arquivo.RawRead(bloco.data(), TAMANHO_BLOCO);
        if (lidos == 0)
            continue;                                                            // Eof() is now true
        erro = zipWriteInFileInZip(m_zip, bloco.data(), lidos);
        if (erro < 0) {
            throw CCompressionError(ECompressionError(1045),
                                    std::format("Falha ao adicionar o arquivo {}. {}", origem.string(),
                                                ErrorToString(erro)));          // line 264
        }
        processados += lidos;
        NotificaProgresso(processados, tamanho, destino.string());
    }
    arquivo.Close();

    erro = zipCloseFileInZip(m_zip);
    if (erro != ZIP_OK) {
        throw CCompressionError(ECompressionError(1046),
                                std::format("A adição do arquivo {} não pode ser concluída. {}", origem.string(),
                                            ErrorToString(erro)));              // line 281
    }
}   // ~CFile

// wasm func 9537 (name inferred). Signals progress once per MiB, as a fraction of 1024.
void CZip::NotificaProgresso(const std::uint64_t processados, const std::uint64_t total, const std::string& nome)
{
    if ((processados & 0xFFFFF) != 0)             // only on exact multiples of 1 MiB
        return;
    const std::string descricao = std::format("Compactando arquivo {}...", nome);
    const std::uint64_t fracao = (processados << 10) / total;   // i64 division: traps if total == 0
    m_progresso(static_cast<unsigned long>(fracao), 1024, descricao);   // signal_impl::operator() (func 9583)
}

// wasm func 9510 (name inferred): minizip/zlib error code -> Portuguese text.
std::string CZip::ErrorToString(const int erro)
{
    switch (erro) {
    case ZIP_OK:                  return "";                               // 0
    case ZIP_ERRNO:               return "Erro de arquivo.";               // -1  (Z_ERRNO)
    case Z_STREAM_ERROR:          return "Erro de leitura/escrita.";       // -2
    case Z_DATA_ERROR:            return "Erro de dados.";                 // -3
    case Z_MEM_ERROR:             return "Memória insuficiente.";          // -4
    case Z_BUF_ERROR:             return "Erro de buffer.";                // -5
    case Z_VERSION_ERROR:         return "Versão incompatível.";           // -6
    case -100 /*UNZ_END_OF_LIST_OF_FILE*/: return "Arquivo não encontrado.";
    case ZIP_PARAMERROR:          return "Parâmetro incorreto.";           // -102
    case ZIP_BADZIPFILE:          return "Arquivo inválido.";              // -103
    case ZIP_INTERNALERROR:       return "Erro interno.";                  // -104
    case -105 /*UNZ_CRCERROR*/:   return "Erro de CRC.";
    default:                      return "Erro desconhecido.";             // includes -101
    }
}

} // namespace ecourna::api::compression
