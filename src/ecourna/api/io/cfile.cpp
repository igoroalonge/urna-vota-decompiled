// ecourna-lib/ecourna/api/io/cfile.cpp
// (/home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/api/io/cfile.cpp in the srcloc records)
//
// Reconstructed from vota_web_wasm.wasm. Unit u12.
//
// Behaviour summary (all read from the wasm; see the unit doc for the evidence):
//  * Every method first checks m_file and throws CIoError(<code>, EFileOperation::None, "arquivo nao estava
//    aberto", 0) when the file is closed. The texts differ slightly: some append the file name, with a
//    trailing space in the literal ("arquivo nao estava aberto "). Exception: SetFileMode (1178) passes the
//    current errno instead of 0.
//  * Open() applies the FileMode through fcntl(F_GETFD)/fcntl(F_SETFD). Those are the *descriptor* flags
//    (FD_CLOEXEC), not the file-status flags (F_GETFL/F_SETFL), so O_SYNC / O_NOATIME are never applied.
//    See the "suspicious" list of the unit doc.
//  * A file opened with 'w', 'a' or '+' in its mode is flushed and fsync'ed when it is closed (Sync()).
//    Sync() only calls fsync() when ecourna::api::util::CSynchronizer is enabled. It is created on demand
//    with `true`.
//  * RawWrite() flushes after every fwrite(). A partial fwrite() (0 < n < size) is not an error.
#include "ecourna/api/io/cfile.hpp"

#include <cerrno>
#include <cstring>
#include <format>
#include <string>
#include <vector>

#include <fcntl.h>
#include <unistd.h>

#include "ecourna/api/util/csynchronizer.hpp"   // ecourna::api::util::CSynchronizer (unit u13)

namespace ecourna::api::io {

// -------------------------------------------------------------------------------------------------------
// wasm func 517 (assigned to unit u18, reproduced here so that the class is complete)
CFile::CFile(const std::string& name, const std::string& mode, const FileMode fileMode)
{
    if (!name.empty())
        Open(name, mode, fileMode);            // invoke_iiiii(slot 6209): ~string(m_name) on unwind
}

// wasm func 3564
CFile::~CFile()
{
    if (m_file != nullptr) {
        try {
            Close();
        } catch (...) {
            // swallowed (__cxa_begin_catch / __cxa_end_catch, no rethrow)
        }
    }
}

// wasm func 9508 (srcloc lines 60, 74; SetFileMode inlined, line 117)
bool CFile::Open(const std::string& name, const std::string& mode, const FileMode fileMode)
{
    FILE* const file = std::fopen(name.c_str(), mode.c_str());
    if (file == nullptr) {
        throw CIoError(EIoError::ErroAbrir /*1175*/, EFileOperation::Open,
                       "nao foi possivel abrir " + name, errno);                       // line 60
    }

    if (m_file != nullptr)
        Close();

    m_name = name;
    m_file = file;
    // Any writing mode: fflush + fsync when the file is closed.  (find_first_of over the three letters;
    // the order of the characters in the set is not recoverable)
    if (mode.find_first_of("wa+") != std::string::npos)
        m_syncOnClose = true;                  // note: never reset to false for a read-only mode

    if (!SetFileMode(fileMode)) {
        m_file = nullptr;
        std::fclose(file);
        throw CIoError(EIoError::ErroAlterarModo /*1176*/, EFileOperation::Mode,
                       "nao foi possivel alterar modo " + name, errno);                // line 74
    }
    return true;
}

// inlined into func 9508 (srcloc line 117)
bool CFile::SetFileMode(const FileMode fileMode)
{
    if (m_file == nullptr) {
        // Unlike the other "not open" checks, this one passes errno (the binary loads errno @1931660).
        // Unreachable from Open(), which has just stored a non-null FILE*.
        throw CIoError(EIoError::ModoArquivoNaoAberto /*1178*/, EFileOperation::None,
                       "arquivo nao estava aberto", errno);                            // line 117
    }

    const int fd    = fileno(m_file);
    const int flags = fcntl(fd, F_GETFD, 0);   // BUG? cmd 1 = F_GETFD (descriptor flags). F_GETFL is 3.
    if (flags == -1 || fd < 0)
        return false;

    int novo = flags & ~(O_SYNC | O_NOATIME);                // & ~0x141000
    if (fileMode & FM_SYNC)
        novo |= O_SYNC;                                      // 0x101000
    if (fileMode & FM_NOATIME)
        novo |= O_NOATIME;                                   // (mode << 17) & 0x40000
    return fcntl(fd, F_SETFD, novo) != -1;     // cmd 2 = F_SETFD: only FD_CLOEXEC is honoured by the kernel
}

// wasm func 336 (srcloc line 111)
void CFile::Close()
{
    if (m_file == nullptr)
        return;

    if (m_syncOnClose)
        Sync();          // if this throws, fclose() is skipped and m_file stays open. ~CFile() calls Close()
                         // again; a second Sync() failure is swallowed there and the FILE* / fd leaks.

    if (std::fclose(m_file) != 0) {
        throw CIoError(EIoError::ErroFechar /*1179*/, EFileOperation::Close,
                       "nao foi possivel fechar " + m_name, errno);                    // line 111
        // (m_file is left dangling on this path: the FILE* has already been released by fclose)
    }
    m_name.clear();
    m_file = nullptr;
}

// inlined into func 2767 (api::CEncryptedFile::Save), srcloc line 145
int CFile::Flush() const
{
    if (m_file == nullptr) {
        throw CIoError(EIoError::FlushArquivoNaoAberto /*1180*/, EFileOperation::None,
                       "arquivo nao estava aberto", 0);                                // line 145
    }
    return std::fflush(m_file);
}

// wasm func 5181 (srcloc lines 160, 165, 170, 175; CSynchronizer::GetInst/CreateInst inlined)
void CFile::Sync() const
{
    if (m_file == nullptr) {
        throw CIoError(EIoError::SyncArquivoNaoAberto /*1181*/, EFileOperation::None,
                       "arquivo nao estava aberto " + m_name, 0);                      // line 160
    }

    if (std::fflush(m_file) != 0) {
        throw CIoError(EIoError::SyncErroFflush /*1184*/, EFileOperation::Sync,
                       "nao sincronizou " + m_name + " - erro = " + std::strerror(errno), errno);   // line 165
    }

    const int fd = fileno(m_file);
    if (fd == -1) {
        // EFileOperation::Read (3) although the step is a sync: copied as found in the binary.
        throw CIoError(EIoError::SyncErroDescritor /*1182*/, EFileOperation::Read,
                       "nao recuperou o descritor de " + m_name + ", erro = " + std::strerror(errno),
                       errno);                                                           // line 170
    }

    // util::CSynchronizer::GetInst(): singleton @0x1D2CB4 (1911988); created on first use by the inlined
    // CreateInst(true) (csynchronizer.cpp:59 throws 1889 "Instância já criada" if it already exists).
    if (util::CSynchronizer::GetInst().IsEnabled() && fsync(fd) != 0) {                // names inferred
        throw CIoError(EIoError::SyncErroFsync /*1183*/, EFileOperation::Sync,
                       "nao sincronizou " + m_name + " - erro = " + std::strerror(errno), errno);   // line 175
    }
}

// wasm func 1886 (srcloc lines 184, 187, 190, 195)
uedword CFile::RawWrite(const void* const buffer, const uedword size) const
{
    if (m_file == nullptr) {
        throw CIoError(EIoError::EscritaArquivoNaoAberto /*1185*/, EFileOperation::None,
                       "arquivo nao estava aberto " + m_name, 0);                      // line 184
    }
    if (buffer == nullptr) {
        throw CIoError(EIoError::EscritaBufferInvalido /*1186*/, EFileOperation::None, "buffer invalido", 0);   // line 187
    }
    if (size == 0) {
        throw CIoError(EIoError::EscritaTamanhoZero /*1187*/, EFileOperation::None,
                       "tamanho do buffer deve ser maior que zero", 0);                // line 190
    }

    const uedword written = static_cast<uedword>(std::fwrite(buffer, 1, size, m_file));
    if (written != 0 && std::fflush(m_file) == 0)
        return written;                        // partial writes are returned as success

    throw CIoError(EIoError::ErroEscrita /*1188*/, EFileOperation::Write,
                   m_name + " " + std::format("size = {}", size), errno);             // line 195
}

// wasm func 598 (srcloc lines 222, 225, 228, 234, 240)
uedword CFile::RawRead(void* const buffer, const uedword size) const
{
    if (m_file == nullptr) {
        throw CIoError(EIoError::LeituraArquivoNaoAberto /*1191*/, EFileOperation::None,
                       "arquivo nao estava aberto", 0);                                // line 222
    }
    if (buffer == nullptr) {
        throw CIoError(EIoError::LeituraBufferInvalido /*1192*/, EFileOperation::None, "buffer invalido", 0);   // line 225
    }
    if (size == 0) {
        throw CIoError(EIoError::LeituraTamanhoZero /*1193*/, EFileOperation::None,
                       "tamanho do buffer nao pode ser zero", 0);                      // line 228
    }

    const uedword lidos = static_cast<uedword>(std::fread(buffer, 1, size, m_file));
    if (lidos != 0)
        return lidos;

    if (std::ferror(m_file)) {
        throw CIoError(EIoError::ErroLeitura /*1194*/, EFileOperation::Read,
                       m_name + " " + std::format("size = {}", size), errno);         // line 234
    }
    if (std::feof(m_file))
        return 0;

    throw CIoError(EIoError::ErroLeituraSemEof /*1195*/, EFileOperation::Read,
                   m_name + " " + std::format("size = {}", size), errno);             // line 240
}

// inlined into func 5480 (api::CIniStrings loader), srcloc lines 336, 356
// Reads one line (any length) in 128-byte fgets() chunks and always returns it '\n'-terminated.
uedword CFile::ReadLine(std::string& line) const
{
    if (m_file == nullptr) {
        throw CIoError(EIoError::ReadLineArquivoNaoAberto /*1205*/, EFileOperation::None,
                       "arquivo nao estava aberto", 0);                                // line 336
    }

    constexpr std::size_t TAM = 128;                 // name inferred
    char buffer[TAM];
    buffer[0]       = '\0';
    buffer[TAM - 2] = '\0';                          // sentinel: non-zero after fgets() filled the buffer
    std::string resultado;

    if (std::fgets(buffer, TAM, m_file) != nullptr) {
        // A full chunk whose last character is not '\n' means the line continues.
        while (buffer[TAM - 2] != '\0' && buffer[TAM - 2] != '\n') {
            resultado.append(buffer);
            buffer[TAM - 2] = '\0';
            if (std::fgets(buffer, TAM, m_file) == nullptr)
                break;   // BUG: fgets() leaves `buffer` untouched at EOF, so it still holds the previous
                         // chunk with buffer[126] zeroed: its first 126 bytes are appended again below
                         // (last line of 127*k bytes without a final '\n', or a read error)
        }
    }
    resultado.append(buffer);

    if (resultado.empty()) {
        if (std::feof(m_file)) {
            line.clear();
            return 0;
        }
        throw CIoError(EIoError::ErroReadLine /*1206*/, EFileOperation::Read, m_name, errno);   // line 356
    }

    if (resultado.back() != '\n')
        resultado.append("\n");
    line.swap(resultado);
    return static_cast<uedword>(line.size());        // ? the only caller ignores the result (inlined)
}

// wasm func 419 (srcloc lines 363, 366)
bool CFile::Seek(const int offset, const int origin) const
{
    if (m_file == nullptr) {
        throw CIoError(EIoError::SeekArquivoNaoAberto /*1207*/, EFileOperation::None,
                       "arquivo nao estava aberto", 0);                                // line 363
    }
    if (fseeko(m_file, offset, origin) != 0) {      // shared_f1982 = fseek() -> fseeko(off_t)
        throw CIoError(EIoError::ErroSeek /*1208*/, EFileOperation::Seek, m_name, errno);   // line 366
    }
    return true;
}

// wasm func 1885 (srcloc line 374)
bool CFile::Eof() const
{
    if (m_file == nullptr) {
        throw CIoError(EIoError::EofArquivoNaoAberto /*1209*/, EFileOperation::None,
                       "arquivo nao estava aberto", 0);                                // line 374
    }
    return std::feof(m_file) != 0;
}

// wasm func 418 (srcloc lines 385, 389)
long CFile::Position() const
{
    if (m_file == nullptr) {
        throw CIoError(EIoError::PositionArquivoNaoAberto /*1210*/, EFileOperation::None,
                       "arquivo nao estava aberto", 0);                                // line 385
    }
    const long posicao = std::ftell(m_file);
    if (posicao < 0) {
        throw CIoError(EIoError::ErroPosition /*1211*/, EFileOperation::Tell, m_name, errno);   // line 389
    }
    return posicao;
}

// wasm func 3522 (srcloc line 420). Text files only: fgets() + strlen() stop at an embedded NUL.
std::string CFile::ReadFileContent(const std::string& fileName)
{
    FILE* const file = std::fopen(fileName.c_str(), "rb");
    if (file == nullptr) {
        throw CIoError(EIoError::ReadFileContentErroAbrir /*1213*/, EFileOperation::Open,
                       "nao foi possivel abrir " + fileName, errno);                   // line 420
    }

    std::string conteudo;
    char linha[256];
    while (std::fgets(linha, sizeof linha, file) != nullptr)
        conteudo.append(linha);                      // (if append throws, `file` leaks: no RAII here)
    std::fclose(file);
    return conteudo;
}

// wasm func 3521 (srcloc lines 435, 447)
std::vector<uebyte> CFile::ReadFileBinary(const std::filesystem::path& fileName)
{
    FILE* const file = std::fopen(fileName.string().c_str(), "rb");
    if (file == nullptr) {
        throw CIoError(EIoError::ReadFileBinaryErroAbrir /*1214*/, EFileOperation::Open,
                       "nao foi possível abrir " + fileName.string(), errno);           // line 435
    }

    std::fseek(file, 0, SEEK_END);
    const long tamanho = std::ftell(file);
    std::vector<uebyte> conteudo(tamanho);           // ftell() == -1 -> std::length_error, `file` leaks
    std::fseek(file, 0, SEEK_SET);
    const std::size_t lidos = std::fread(conteudo.data(), 1, conteudo.size(), file);
    std::fclose(file);

    if (lidos != conteudo.size()) {
        throw CIoError(EIoError::ReadFileBinaryErroLeitura /*1215*/, EFileOperation::Read,
                       "não foi possível ler conteúdo de " + fileName.string(), 0);    // line 447
    }
    return conteudo;
}

} // namespace ecourna::api::io
