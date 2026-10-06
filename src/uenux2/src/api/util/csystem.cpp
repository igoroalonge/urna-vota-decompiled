// Reconstructed from vota_web_wasm.wasm (unit u20, owner of this file).
// Original: uenux2/src/api/util/csystem.cpp
//   srclocs: :64 ExistResource, :236/:243 TemporaryPath, :255 IsEmptyDir, :334 GetFileSize,
//            :397/:414/:448/:456/:465/:471/:476 CopyFile, :916/:922/:928 PrepareReplace,
//            :954/:962 ReplaceResource, :1024 truncate, :1039 ZeroFill.
// Other fragments of this file: csystem.u12-fragment.cpp (IsDirectory / IsRegularFile, funcs 5459/412/6019).
//
// api::CSystem = static file-system helpers of the urna (copy with O_SYNC, atomic replace with backup,
// "zero-fill" before deletion, emptiness checks). All errors are
// ecourna::api::exception::CBaseError<api::EUeUtilError, ...> (typeinfo @1531252); the constructor thunk
// is wasm func 346 (tools: api_f346) = ecourna_f710(exc, codigo, msg, srcloc, vtable @1531280).
//
// In the web build every path lives in Emscripten's MEMFS: fsync/O_SYNC, chown and the physical effect
// of ZeroFill are no-ops there.
#include "api/util/csystem.h"

#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <format>
#include <functional>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>
#include <vector>

#include "api/util/cdirreader.h"
#include "api/util/csynchronizer.h"

namespace api {

// wasm func 1690                                                                srcloc csystem.cpp:64
// Observed executing. "Does the resource exist?" - a missing path (ENOENT) or a non-directory
// component (ENOTDIR) is a normal "no"; any other stat() error is fatal.
bool ExistResource(struct stat& info, const std::string& caminho)
{
    if (::stat(caminho.c_str(), &info) != -1)
        return true;
    if (errno == ENOENT || errno == ENOTDIR)       // WASI errno 44 / 54 in this build
        return false;
    throw CUeUtilError(EUeUtilError{7031},
        std::format("Falha ao validar a existência do recurso [{}]: {}", caminho, std::strerror(errno)));
}

namespace {

// ---------------------------------------------------------------------------------------------------
// Helpers of CopyFile (inlined): change owner / mode of the destination, preferring the fd-based call.
// A directory is opened with opendir() and changed through dirfd(); a file with the S_IFREG bit is
// opened read-only (falling back to the path-based call on EPERM/EACCES); anything else uses the path.
// Emscripten's MEMFS accepts chown but ignores ownership.                         names inferred
template <typename PorFd, typename PorCaminho>
int AlteraAtributo(const std::string& caminho, PorFd porFd, PorCaminho porCaminho)
{
    struct stat info;
    if (::stat(caminho.c_str(), &info) != 0)
        return -1;
    if (S_ISDIR(info.st_mode)) {
        DIR* dir = ::opendir(caminho.c_str());
        if (!dir)
            return -1;
        const int r = porFd(::dirfd(dir));
        ::closedir(dir);                       // wasm func 957 = musl closedir (close + free)
        return r < 0 ? -1 : 0;
    }
    if (info.st_mode & S_IFREG) {              // literal bit test (0x8000), not S_ISREG
        const int fd = ::open(caminho.c_str(), O_RDONLY);
        if (fd == -1) {
            if (errno != EPERM && errno != EACCES)
                return -1;
            return porCaminho();
        }
        const int r = porFd(fd);
        ::close(fd);
        return r < 0 ? -1 : 0;
    }
    return porCaminho();
}

int AlteraDono(const std::string& caminho, uid_t dono, gid_t grupo)
{
    return AlteraAtributo(caminho, [&](int fd) { return ::fchown(fd, dono, grupo); },
                          [&] { return ::chown(caminho.c_str(), dono, grupo); });
}

int AlteraPermissoes(const std::string& caminho, mode_t modo)
{
    return AlteraAtributo(caminho, [&](int fd) { return ::fchmod(fd, modo); },
                          [&] { return ::chmod(caminho.c_str(), modo); });
}

// wasm func 3645 (tools: api_f3645)                                                  name from literal
// Returns "" on success, otherwise "RenameResource(<de>, <para>) - <strerror>".
std::string RenameResource(const std::string& de, const std::string& para)
{
    if (::rename(de.c_str(), para.c_str()) == 0)
        return {};
    return "RenameResource(" + de + ", " + para + ") - " + std::strerror(errno);
}

// Secure-ish deletion used twice in ReplaceFile and in RemoveConteudoDiretorio (inlined):
// a symbolic link is removed as is; anything else is "zero-filled" first.        name inferred
// The same body exists out of line as wasm func 2760 (tools: vota_f2760, not in u20; observed executing),
// called by CAjusteInicial::ValidaTemposDesligamento, vota_f1487 and func 6737 to delete rdv.dat /
// uenux.db when the dynamic data is regenerated - so it is probably a public CSystem method.   // ?
void ApagaArquivo(const std::string& caminho)
{
    struct stat info;
    if (::lstat(caminho.c_str(), &info) == -1 || !S_ISLNK(info.st_mode))
        CSystem::ZeroFill(caminho);
    ::remove(caminho.c_str());
}

// wasm func 5458 (tools: api_f5458) - recursive helper of ReplaceFile when the backup is a directory.
// Collects the entries first, then removes sub-directories (recursively + rmdir) and files
// (ApagaArquivo). Returns 0 on success, -1 at the first failure.                  name inferred
int RemoveConteudoDiretorio(const std::string& diretorio)
{
    std::vector<std::string> arquivos;
    std::vector<std::string> subdiretorios;
    CDirReader leitor(diretorio);                                     // func 1914
    while (leitor.NextEntry()) {                                      // func 1260
        const std::string nome = leitor.GetEntryName();               // d_name (+19), "" if none
        if (leitor.IsLink() || leitor.IsFile())                       // funcs 5469 / 2231
            arquivos.push_back(diretorio + "/" + nome);
        else if (leitor.IsDirectory() && nome != "." && nome != "..") // func 1912
            subdiretorios.push_back(diretorio + "/" + nome);
    }
    leitor.Close();                                                   // func 2232
    // (~CDirReader, func 1913, runs at the end of the function, after both loops)
    for (const std::string& sub : subdiretorios)
        if (RemoveConteudoDiretorio(sub) != 0 || ::rmdir(sub.c_str()) != 0)
            return -1;
    for (const std::string& arquivo : arquivos) {
        struct stat info;
        if (::lstat(arquivo.c_str(), &info) == -1 || !S_ISLNK(info.st_mode))
            CSystem::ZeroFill(arquivo);
        if (::remove(arquivo.c_str()) != 0)
            return -1;
    }
    return 0;
}

// Inlined into func 5455.                                         srclocs csystem.cpp:916/:922/:928
// tipo = "arquivo" (the literal at @124102), funcao = "CSystem::ReplaceFile".
std::string PrepareReplace(const std::string& novo, const std::string& destino, const std::string& backup,
                           bool (*existe)(const std::string&), const std::string& funcao,
                           const std::string& tipo)
{
    const std::string caminhoBackup = backup.empty() ? CSystem::TemporaryPath(novo) : backup;
    if (backup.empty())
        ApagaArquivo(caminhoBackup);   // TemporaryPath created it (mkstemp): delete it, keep only the name

    if (!existe(novo))                                                       // CSystem::IsRegularFile (412)
        throw CUeUtilError(EUeUtilError{7064}, std::format("{} - {} [{}] não encontrado", funcao, tipo, novo));
    if (novo == destino)
        throw CUeUtilError(EUeUtilError{7065}, std::format("{} - os {}s têm o mesmo nome [{}]", funcao, tipo, destino));
    struct stat info;
    if (::stat(caminhoBackup.c_str(), &info) == 0)
        throw CUeUtilError(EUeUtilError{7066}, std::format("{} - {} [{}] existe", funcao, tipo, caminhoBackup));
    return caminhoBackup;
}

// Inlined into func 5455.                                                srclocs csystem.cpp:954/:962
void ReplaceResource(const std::string& novo, const std::string& destino, const std::string& backup,
                     const std::string& funcao)
{
    struct stat info;
    std::string erro;
    if (::stat(destino.c_str(), &info) == 0) {
        erro = RenameResource(destino, backup);                      // 1) destino -> backup
        if (!erro.empty())
            throw CUeUtilError(EUeUtilError{7067}, funcao + " - " + erro);
    }
    erro = RenameResource(novo, destino);                            // 2) novo -> destino
    if (!erro.empty()) {
        if (::stat(backup.c_str(), &info) == 0)
            RenameResource(backup, destino);                         //    roll back (result ignored)
        throw CUeUtilError(EUeUtilError{7068}, funcao + " - " + erro);
    }
    // 3) drop the backup
    if (ExistResource(info, backup) && S_ISDIR(info.st_mode)) {
        if (RemoveConteudoDiretorio(backup) == 0)
            ::rmdir(backup.c_str());
    } else if (ExistResource(info, backup) && S_ISREG(info.st_mode)) {
        ApagaArquivo(backup);
    }
}

} // namespace

// wasm func 5457                                                              srcloc csystem.cpp:1024
// The srcloc prints "void api::truncate(int, off_t, const std::string &)": a file-static function of
// namespace api (not in the anonymous namespace like PrepareReplace / ReplaceResource).
static void truncate(int fd, off_t tamanho, const std::string& caminho)
{
    if (::ftruncate(fd, tamanho) == -1)
        throw CUeUtilError(EUeUtilError{7070},
                           std::format("Falha ao truncar arquivo: {}-{}", caminho, std::strerror(errno)));
}

// Inlined into func 5455 (the only caller of TemporaryPath in the binary).  srclocs csystem.cpp:236/:243
// mkstemp() is inlined too (musl __randname: 6 letters from clock_gettime + a counter, 100 attempts,
// O_RDWR|O_CREAT|O_EXCL, mode 0600).
std::string CSystem::TemporaryPath(const std::string& base)
{
    std::string modelo = base + ".XXXXXX";
    const int fd = ::mkstemp(&modelo.at(0));
    if (fd == -1)
        throw CUeUtilError(EUeUtilError{7034},
            std::format("Falha ao criar caminho temporário para [{}]: {}", base, std::strerror(errno)));
    ::close(fd);
    if (modelo.at(0) == '\0')
        throw CUeUtilError(EUeUtilError{7035},
            std::format("Falha ao criar caminho temporário para [{}]: {}", base, std::strerror(errno)));
    return modelo;
}

// wasm func 5455 (tools: api::(anonymous namespace)::PrepareReplace)                name inferred
// Atomically replaces `destino` by `novo`, keeping the old file under `backup` (or a mkstemp name)
// until the rename succeeded. Only caller: vota::impl::CSincronismoVotoEleitor::vf2 (func 7174), which
// replaces rdv.dat by rdv.dat.tmp with an empty backup name after every vote on the real urna
// (never executed in the web build: CSincronismoVotoEleitorWeb short-circuits the vote persistence).
// The "CSystem::ReplaceFile" / "arquivo" strings are temporaries of each call: the binary frees both after
// PrepareReplace and builds "CSystem::ReplaceFile" again for ReplaceResource.
void CSystem::ReplaceFile(const std::string& novo, const std::string& destino, const std::string& backup)
{
    const std::string caminhoBackup =
        PrepareReplace(novo, destino, backup, &CSystem::IsRegularFile, "CSystem::ReplaceFile", "arquivo");
    ReplaceResource(novo, destino, caminhoBackup, "CSystem::ReplaceFile");
}

// wasm func 2762                                                               srcloc csystem.cpp:255
bool CSystem::IsEmptyDir(const std::string& diretorio)
{
    struct stat info;
    if (!ExistResource(info, diretorio) || !S_ISDIR(info.st_mode))
        // note: for an existing non-directory errno is stale, so the "{}" reason is meaningless
        throw CUeUtilError(EUeUtilError{7036},
            std::format("O diretório [{}] não existe: {}", diretorio, std::strerror(errno)));

    CDirReader leitor(diretorio);
    bool vazio = true;
    while (leitor.NextEntry()) {
        const std::string nome = leitor.GetEntryName();
        if (leitor.IsDirectory() && nome != "." && nome != "..") { vazio = false; break; }
        if (leitor.IsFile())                                  { vazio = false; break; }
        // symbolic links, devices, "." and ".." are ignored
    }
    leitor.Close();
    return vazio;
}

// wasm func 2759                                                               srcloc csystem.cpp:334
uedword CSystem::GetFileSize(const std::string& arquivo)
{
    struct stat info;
    if (::stat(arquivo.c_str(), &info) != 0)
        throw CUeUtilError(EUeUtilError{7038},
            std::format("Falha ao obter tamanho do arquivo [{}]: {}", arquivo, std::strerror(errno)));
    return static_cast<uedword>(info.st_size);
}

// wasm func 378 - observed executing (signature copies, CCopiadorMR, IGravador, CSincronizaVota ...).
// srclocs csystem.cpp:397 .. :476. The 4th parameter was removed by dead-argument elimination
// (every caller passes the same value); the sync at the end is unconditional in the binary.
void CSystem::CopyFile(const std::string& origem, const std::string& destino, bool preservaAtributos,
                       bool /*sincroniza ?*/)
{
    if (origem == destino)
        return;

    const int entrada = ::open(origem.c_str(), O_RDONLY);
    if (entrada < 0)
        throw CUeUtilError(EUeUtilError{7042},                                          // :397
            std::format("O arquivo de origem [{}] não pode ser lido: {}", origem, std::strerror(errno)));

    const int flags = (CSynchronizer::GetInst().IsSincrono() ? O_SYNC : 0) | O_WRONLY | O_CREAT | O_TRUNC;
    const int saida = ::open(destino.c_str(), flags, 0644);
    if (saida < 0) {
        const int erro = errno;
        ::close(entrada);
        errno = erro;
        throw CUeUtilError(EUeUtilError{7043},                                          // :414
            std::format("O arquivo de destino [{}] não pôde ser criado: {}", destino, std::strerror(erro)));
    }

    std::vector<char> buffer(32768);
    int erro = 0;
    for (;;) {
        const ssize_t lidos = ::read(entrada, buffer.data(), buffer.size());
        if (lidos == 0)
            break;
        if (lidos < 0) { erro = errno; break; }
        ssize_t escritos = 0;
        while (escritos < lidos) {
            const ssize_t n = ::write(saida, buffer.data() + escritos, lidos - escritos);
            if (n <= 0) { erro = errno; break; }
            escritos += n;
        }
        if (escritos != lidos)
            break;
    }
    ::close(entrada);
    if (::close(saida) != 0)
        erro = errno;
    if (erro != 0)
        throw CUeUtilError(EUeUtilError{7044},                                          // :448
            std::format("Falha ao copiar o arquivo [{}] para [{}]: {}", origem, destino, std::strerror(erro)));

    if (preservaAtributos) {
        struct stat info;
        if (::lstat(origem.c_str(), &info) < 0)
            throw CUeUtilError(EUeUtilError{7045},                                      // :456
                std::format("Falha ao obter informações do arquivo [{}]: {}", origem, std::strerror(errno)));
        const utimbuf tempos{info.st_atime, info.st_mtime};
        if (::utime(destino.c_str(), &tempos) < 0)                     // wasm func 6238 = musl utime
            throw CUeUtilError(EUeUtilError{7046},                                      // :465
                std::format("Falha ao alterar a hora do arquivo [{}]: {}", destino, std::strerror(errno)));
        if (AlteraDono(destino, info.st_uid, info.st_gid) < 0)
            throw CUeUtilError(EUeUtilError{7047},                                      // :471
                std::format("Falha ao alterar o dono do arquivo [{}]: {}", destino, std::strerror(errno)));
        if (AlteraPermissoes(destino, info.st_mode) < 0)
            throw CUeUtilError(EUeUtilError{7048},                                      // :476
                std::format("Falha ao alterar as permissões do arquivo [{}]: {}", destino, std::strerror(errno)));
    }
    CSynchronizer::GetInst().Sync();                                   // shared_f620 (empty in this build)
}

// wasm func 2761 - observed executing                                         srcloc csystem.cpp:1039
// Despite the name, nothing is written: the file is truncated to 0 and extended back to its size,
// which on Linux leaves a sparse file of zeros and merely FREES the old data blocks (they are not
// overwritten on the medium). Only regular files are touched; the descriptor is closed by an on-exit
// guard holding std::function<void()> (lambda $_0, vtable @1585036, operator() = func 10860 = close(fd)).
void CSystem::ZeroFill(const std::string& arquivo)
{
    struct stat info;
    if (!ExistResource(info, arquivo) || !S_ISREG(info.st_mode))
        return;

    int fd = ::open(arquivo.c_str(), O_WRONLY);
    if (fd == -1)
        throw CUeUtilError(EUeUtilError{7069},
                           std::format("Falha ao abrir arquivo: {}-{}", arquivo, std::strerror(errno)));
    const CScopeExit fecha{std::function<void()>([&fd] { ::close(fd); })};   // guard type name unknown

    const off_t tamanho = ::lseek(fd, 0, SEEK_END);
    truncate(fd, 0, arquivo);
    truncate(fd, tamanho, arquivo);
}

} // namespace api

// -----------------------------------------------------------------------------------------------------
// Library code that the tools attributed to this file:
//   wasm func 957  musl closedir(DIR*): { int r = close(d->fd); free(d); return r; }  (11 callers)
//   wasm func 6238 musl utime(path, const utimbuf*) -> utimensat(AT_FDCWD, path, {{actime,0},{modtime,0}}, 0)
//                  (also used by 7-Zip's CFileBase)
//   wasm func 5456 std::make_format_args(string, string, const char*) for "[{}] para [{}]: {}"
//                  (packed arg types 12717 = string_view, string_view, const char*)
//   wasm func 5460 std::operator+(std::string&&, const std::string&) (funcao + " - " + erro)
// -----------------------------------------------------------------------------------------------------
