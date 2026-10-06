// uenux2/src/app/comum/gravadores/{cgravadorwsq,cgravadorlog,cgravadorenvelopearquivo}.cpp
//   -- FRAGMENTS reconstructed by unit u12
//
// These three result writers ("gravadores") contain inlined ecourna compression / io code, which is why the
// analyzer put them in unit u12. The classes belong to the comum/gravadores unit (u23). Only the
// functions listed here were reconstructed.
//
// Reconstructed from vota_web_wasm.wasm. See docs/modules/u12-...md §3.
#include <filesystem>
#include <format>
#include <map>
#include <string>
#include <vector>

#include "api/util/csystem.h"
#include "comum/gravadores/cgravadorenvelopearquivo.h"
#include "comum/gravadores/cgravadorlog.h"
#include "comum/gravadores/cgravadorwsq.h"
#include "ecourna/api/compression/czip.hpp"
#include "ecourna/api/io/cfile.hpp"

namespace comum {

using ecourna::api::compression::CZip;
using ecourna::api::compression::ECompressLevel;

// =======================================================================================================
// CGravadorWSQ (vtable @1557300). m_tipo (+68): 0 = eleitor habilitado (wsqbio.jez), 1 = não habilitado
// (wsqman.jez), 2 = operador/mesário (wsqmes.jez)  (see cgravaresultado.cpp, unit u07).
// -------------------------------------------------------------------------------------------------------

// Inlined helpers (srclocs cgravadorwsq.cpp:175 and :192; codes 8652/8653 of EUeComumGravadoresError)
std::vector<std::string> CGravadorWSQ::GetCaminhoCorretoInternal() const
{
    switch (m_tipo) {
    case 0: return {CaminhoWsqInterno("habilitado/")};         // vota_f5819 -> vota_f2298(<wsq dir int>, ...)
    case 1: return {CaminhoWsqInterno("nao-habilitado/")};     // vota_f5818
    case 2: return {CaminhoWsqInterno("operador/")};           // vota_f3795
    }
    throw CUeComumGravadoresError(EUeComumGravadoresError(8652), std::format("Tipo de biometria inválido: {}", m_tipo));   // line 175
}

std::vector<std::string> CGravadorWSQ::GetCaminhoCorretoExternal() const
{
    switch (m_tipo) {
    case 0: return {CaminhoWsqExterno("habilitado/")};         // comum_f5817 -> comum_f3898
    case 1: return {CaminhoWsqExterno("nao-habilitado/")};     // comum_f5816
    case 2: return {CaminhoWsqExterno("operador/")};           // comum_f3794
    }
    throw CUeComumGravadoresError(EUeComumGravadoresError(8653), std::format("Tipo de biometria inválido: {}", m_tipo));   // line 192
}

// wasm func 5821 (name inferred; the analyzer called it "globToFileList" after the inlined ecourna srcloc
// icompressor.cpp:61). Called by CGravadorWSQ::vf2 (func 11579) with externo = false and
// arquivoJez = <result dir> / <result file name>, and by comum_f6041.
void CGravadorWSQ::CompactaWsq(const bool externo, const std::string& arquivoJez) const
{
    const std::vector<std::string> diretorios = externo ? GetCaminhoCorretoExternal() : GetCaminhoCorretoInternal();

    bool todosVazios = true;
    for (const auto& diretorio : diretorios) {
        if (!api::CSystem::IsDirectory(diretorio))              // func 5459
            CriaDiretorios(diretorio);                           // inlined "mkdir -p" (below)
        if (todosVazios)
            todosVazios = api::CSystem::IsEmptyDir(diretorio);   // func 2762
    }

    if (todosVazios) {
        // Nothing to pack: an EMPTY archive is still produced (level Padrao), so the result file always exists.
        CZip zip(arquivoJez, ECompressLevel::Padrao);
        zip.Close();
        return;
    }

    CZip zip(arquivoJez, ECompressLevel::Armazenar);             // WSQ is already compressed: "store"
    for (const auto& diretorio : diretorios)
        zip.Add(std::filesystem::path(diretorio + "*.wsq"));   // ICompressor::Add(path) -> globToFileList (inlined)
    zip.Close();
}

// Inlined into func 5821 (probably api::CSystem::CreateDirectories or similar; name inferred).
// Creates every prefix of `caminho` that ends before a '/' and does not yet exist as a directory, mode 0755.
// A failing mkdir() stops the loop SILENTLY (no error is reported; the zip step fails later).
void CGravadorWSQ::CriaDiretorios(const std::string& caminho)
{
    std::string prefixo;
    for (std::size_t i = 0; i < caminho.size();) {
        const std::size_t barra = std::min(caminho.find('/', i), caminho.size());   // memchr
        if (barra > i) {                                     // empty component ("/x", "a//b"): no mkdir
            prefixo.append(caminho, i, barra - i);
            struct stat info;
            const bool ehDir = api::ExistResource(info, prefixo) && S_ISDIR(info.st_mode);
            if (!ehDir && mkdir(prefixo.c_str(), 0755) < 0)
                return;
        }
        prefixo.push_back('/');
        i = (barra == caminho.size()) ? barra : barra + 1;
    }
}

// =======================================================================================================
// CGravadorLog (vtable @1557196). Layout used here: +40 archived-logs directory, +52 current log file,
// +64 working directory, +20 result file name.
// -------------------------------------------------------------------------------------------------------

// wasm func 11584 = vtable slot 7, the pure virtual of IGravador. Its signature is known from CGravadorBU's
// override (func 11629, srcloc cgravadorbu.cpp:484): `virtual void GravaResultado(api::CFile&) const`
// (api::CFile = ecourna::api::io::CFile). CGravadorLog IGNORES the CFile it receives (the parameter register
// is reused at once). It writes its own archive and renames it over the result name.
void CGravadorLog::GravaResultado(api::CFile& /*arquivo: unused*/) const
{
    // 1. Files to pack: {path -> file name inside the archive}
    std::map<std::filesystem::path, std::filesystem::path> arquivos;
    arquivos.emplace(m_arquivoLog, std::filesystem::path(m_arquivoLog).filename());   // the current log (logd.dat)
    for (const auto& entrada : std::filesystem::directory_iterator(m_diretorioArquivados))
        if (entrada.is_regular_file())
            arquivos.emplace(entrada.path(), entrada.path().filename());

    // 2. Write <trabalho>/temp.jez (the literal is the i64 0x7A656A2E706D6574) with the default level ...
    const std::string temporario = m_diretorioTrabalho + "temp.jez";
    CZip zip(temporario, ECompressLevel::Padrao, arquivos);   // CZip file-list constructor (czip.cpp)
    zip.Close();

    // 3. ... then rename it to the final result name (api::CSystem::Rename inlined, csystem.cpp:692):
    //    throws EUeUtilError 7058 "Falha ao renomear [{}] para [{}]: {}" (strerror(errno)).
    api::CSystem::Rename(temporario, m_diretorioTrabalho + m_nomeArquivo);
}

// =======================================================================================================
// CGravadorEnvelopeArquivo (vtable @1554512)
// -------------------------------------------------------------------------------------------------------

// wasm func 11623 = vtable slot 8 (name inferred): the raw bytes of the file to wrap in the envelope.
// Signature (sret, this) only: IGravadorEnvelope::vf7 (func 11626) calls it as call_indirect(out, this), so it
// takes no parameter. The file name is the member m_arquivo (+172), as declared in cgravadorenvelopearquivo.h.
std::vector<uebyte> CGravadorEnvelopeArquivo::LeConteudo() const
{
    const auto tamanho = api::CSystem::GetFileSize(m_arquivo);         // func 2759
    std::vector<uebyte> conteudo(tamanho);                             // "negative" size -> length_error
    if (tamanho == 0)
        return conteudo;

    ecourna::api::io::CFile file(m_arquivo, "rb");
    file.RawRead(conteudo.data(), static_cast<uedword>(tamanho));      // result ignored: a short read leaves
    return conteudo;                                                     // zero bytes at the end (unchecked)
}   // ~CFile

} // namespace comum
