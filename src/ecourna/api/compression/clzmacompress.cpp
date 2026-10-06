// ecourna-lib/ecourna/api/compression/clzmacompress.cpp
// (/home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/api/compression/clzmacompress.cpp in the srcloc records)
//
// Reconstructed from vota_web_wasm.wasm. Unit u12.
//
// DEAD CODE (see clzmacompress.hpp): the only entry point, CLzmaCompress::DoAdd (func 9586), can never be
// called because no CLzmaCompress object is ever built. It is reconstructed here in less detail than the
// live code: 7-Zip's own structures are named after the 7-Zip 19.00 sources (CPP/7zip/UI/Common) and only
// described where the TSE code touches them.
//
// What the TSE code does: it runs the equivalent of the 7-Zip command lines
//     7za a  -m0=LZMA -mf=off <pacote.7z> <arquivo>          (level != store)
//     7za a  -m0=Copy         <pacote.7z> <arquivo>          (level == store)
//     7za rn                  <pacote.7z> <nome> <novo nome> (when the name inside the archive must change)
// through UpdateArchive() (func 8434), with a lazily built, process-wide 7-Zip context.
#include "ecourna/api/compression/clzmacompress.hpp"

#include <format>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "ecourna/api/compression/ecompressionerror.hpp"   // CCompressionError (not reconstructed)
// 7-Zip 19.00 + p7zip headers: UI/Common/Update.h, UI/Common/LoadCodecs.h, UI/Console/UpdateCallbackConsole.h,
// UI/Console/OpenCallbackConsole.h, Common/Wildcard.h ...

namespace ecourna::api::compression {

namespace {

// 568-byte object, created once (singleton pointer @1923240, guarded by the std::mutex @1923244).
// Its layout follows 7-Zip's CArcCmdLineOptions (censor, CUpdateOptions, rename pairs, ...) with TSE fields
// appended. Name and member names inferred.
struct CContexto7z {
    // +36   NCommandType::EEnum command   (0 = kAdd, 10 = kRename)
    // +40   UString  arcPath
    // +48   NWildcard::CCensor censor     (+48 Pairs, +60 CensorPaths)   ~CCensor = func 4936
    // +184  CUpdateOptions  (+240 Commands: CObjectVector<CUpdateArchiveCommand>, ~elem = func 3413;
    //                        +224 MethodMode.Properties: CObjectVector<CProperty>, Add = func 2170)
    // +352  NWildcard::CCensor arcCensor  (~ = func 4936)
    // +436  CObjectVector<CRenamePair> renamePairs
    // +528  CCodecs* codecs               (new CCodecs, vtable @1158580; CCodecs::Load() inlined in DoAdd)
    // +532  HRESULT  erro                 last UpdateArchive() result
    // +536  std::string mensagem          error text reported by 7-Zip
    // +560  uint8_t  estado[2]            set to {1,0} / {2,0} / {2,0x32} by Adiciona7z (func 8376, 16-bit
    //                                     stores 1 / 2 / 0x3202; meaning unknown)
};

std::unique_ptr<CContexto7z> s_contexto;   // @1923240; deleter = func 4937 (~CContexto7z + free), atexit func 8379
std::mutex                   s_mutex;      // @1923244

// Builds the context the first time (inlined into func 9586):
//  * registers the codecs LZMA (0x030101), LZMA2 (0x21), BCJ (0x03030103, filter) and Copy (0x00) in
//    g_Codecs (descriptors @1153184..1153280) and the archive format "7z" (signature 7z\xBC\xAF\x27\x1C,
//    descriptor @1153320) in g_Arcs, unless already present;
//  * builds the CRC-32 tables (polynomial 0xEDB88320) and selects CrcUpdateT8;
//  * new CCodecs, CCodecs::Load() (CArcInfoEx::AddExts with SplitString = func 5009 and
//    CObjectVector<CArcExtInfo>::Add = func 2170 inlined).
std::unique_ptr<CContexto7z> CriaContexto7z();                          // name inferred (inlined)

// wasm func 3414 (name inferred). One "7za" command:
//   renomear == false : command kAdd,    censor = every entry of `arquivos` (Include, not recursive)
//   renomear == true  : command kRename, censor = "*" (@378616), one CRenamePair{arquivos[i] -> novosNomes[i]}
//                       per pair whose names differ; nothing to rename -> returns true immediately
//   properties: comprimir ? { "0" = "LZMA", "f" = "off" } : { "0" = "Copy" }   (strings @358471 "0",
//               @335052 "LZMA", @169675 "f", @169421 "off", @10885 "Copy")
//   HRESULT hr = UpdateArchive(ctx.codecs, ..., ctx.arcPath, ctx.censor, ctx.updateOptions, errorInfo,
//                              &openCallback /*COpenCallbackConsole*/, &updateCallback /*CUpdateCallbackConsole*/);
//   success = hr == S_OK && !errorInfo.ThereIsError() && no failed / missing files;
//   on failure: ctx.erro = hr; ctx.mensagem = errorInfo message.
// The original signature also took the progress std::function by value; wasm-opt removed that unused
// parameter (the callers still build and destroy the copy around the call). The 7z path therefore
// never reports progress.
bool Executa7z(CContexto7z& ctx, const std::vector<std::string>& arquivos, const std::string& pacote,
               const std::vector<std::string>& novosNomes, bool renomear, bool comprimir
               /*, std::function<void(std::uint64_t, const std::string&)> progresso  -- removed by DAE */);

// wasm func 8376 (name inferred)
bool Adiciona7z(CContexto7z& ctx, const std::vector<std::string>& arquivos, const std::string& pacote,
                const std::vector<std::string>& novosNomes, const bool comprimir,
                const std::function<void(std::uint64_t, const std::string&)>& progresso)
{
    if (novosNomes.empty()) {
        // ctx.estado = {1, 0}
        return Executa7z(ctx, arquivos, pacote, novosNomes, false, comprimir /*, progresso*/);
    }
    if (novosNomes.size() != arquivos.size())
        return false;

    // Name each existing file will get inside the archive: the text after the last '/' or '\'.
    std::vector<std::string> nomesBase = arquivos;
    for (auto& nome : nomesBase) {
        if (!DoesFileOrDirExist(UString(nome.c_str())))          // func 4834 (7-Zip NFind)
            continue;
        const auto barra = nome.find_last_of("/\\");
        if (barra != std::string::npos && barra != 0 && barra + 1 < nome.size())   // a leading '/' alone is kept
            nome = nome.substr(barra + 1);
    }
    if (nomesBase.size() != novosNomes.size())
        return false;
    const bool precisaRenomear = nomesBase != novosNomes;        // ctx.estado = {2, 0} when true

    if (!Executa7z(ctx, arquivos, pacote, {}, false, comprimir /*, progresso*/))
        return false;
    if (precisaRenomear) {
        // ctx.estado = {2, 0x32}
    }
    return Executa7z(ctx, nomesBase, pacote, novosNomes, true, false /*, progresso*/);
}

} // namespace

// wasm func 9586 (srcloc line 114)
void CLzmaCompress::DoAdd(const std::filesystem::path& origem, const std::filesystem::path& destino)
{
    {
        std::lock_guard<std::mutex> trava(s_mutex);          // only the unlock survives (single-threaded build)
        if (!s_contexto)
            s_contexto = CriaContexto7z();
    }

    const std::vector<std::string> arquivos = {origem.string()};             // func 5585
    std::vector<std::string> novosNomes;
    // Keep the file's own name when `destino` is exactly the file name of an absolute `origem`;
    // otherwise rename the entry to `destino` after adding it.
    if (!(origem.filename() == destino && origem.has_root_directory()))
        novosNomes = {destino.string()};

    const std::function<void(std::uint64_t, const std::string&)> progresso =
        std::bind(&CLzmaCompress::OnProgress, this, std::placeholders::_1, std::placeholders::_2);

    if (!Adiciona7z(*s_contexto, arquivos, m_arquivo.string(), novosNomes,
                    m_nivel != ECompressLevel::Armazenar, progresso)) {
        throw CCompressionError(ECompressionError(1013),
                                std::format("(0x{:X} - {}) Não foi possível incluir o arquivo {} no pacote {}.",
                                            s_contexto->erro, s_contexto->mensagem, origem.string(),
                                            m_arquivo.string()));                    // line 114
    }
}

// wasm func 9582 / 9581: implicit destructors (~m_arquivo, then ~ICompressor = func 1526).

} // namespace ecourna::api::compression

// -------------------------------------------------------------------------------------------------------
// 7-Zip / libc++ functions that the analyzer attributed to this file (not TSE code):
//   func 3413  CUpdateArchiveCommand::~CUpdateArchiveCommand()        (8 UStrings freed)
//   func 4936  NWildcard::CCensor::~CCensor()                         (Pairs + CensorPaths)
//   func 4937  delete CContexto7z  (destroys the CArcCmdLineOptions-like members, deletes the CCodecs)
//   func 4949  CPercentPrinter::~CPercentPrinter()                    (ClosePrint inlined: "\b"*n, " "*n, "\b"*n)
//   func 5009  SplitString(const UString&, UStringVector&)            (7-Zip LoadCodecs.cpp, splits on ' ')
//   func 2170  CObjectVector<CArcExtInfo / CProperty>::Add(const T&)  (24-byte {UString, UString} element)
//   func 5585  std::vector<std::string>::__init_with_size(first, last, n)    (libc++; also used elsewhere)
//   func 5573  its exception guard (__destroy_vector on unwind)
