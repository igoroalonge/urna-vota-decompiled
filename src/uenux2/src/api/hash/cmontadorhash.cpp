// Reconstructed from vota_web_wasm.wasm (unit u17).
// Original: uenux2/src/api/hash/cmontadorhash.cpp (srclocs cmontadorhash.cpp:49, :78, :91, :98, :121;
// chashdiretorio.cpp:41 is inlined too).
//
// Only one function of this file survives as a wasm function: CriaHashesDiretorio (func 5360, 9.6 KB).
// CalculaHashArquivo, CalculaHash, CalculaHashGeral, CHashDiretorio's constructor and
// std::filesystem::read_symlink / weakly_canonical are all inlined into it. The libc++ filesystem
// helpers that stayed out of line were attributed to this file by the tools (mapping table: 2547,
// 3335, 3370, 4675, 4676, 4686, 7735, 7741-7744).
#include "api/hash/cmontadorhash.h"

#include <algorithm>
#include <filesystem>
#include <memory>

#include "api/pattern/cpolysingletonlist.h"
#include "api/pattern/igenericfactory.h"
#include "api/util/cdirreader.h"
#include "ecourna/api/exception/cbaseerror.hpp"
#include "ecourna/api/io/cfile.hpp"
#include "ecourna/api/security/ihash.hpp"
#include "ecourna/api/security/itextencoding.hpp"

namespace api::hash {

namespace {

using CUeHashError = ecourna::api::exception::CBaseError<EUeHashError>;   // thunk func 2723

using ecourna::api::security::IHash;           // slot 3 Update(const vector<uebyte>&), slot 4 Final()
using ecourna::api::security::ITextEncoding;   // slot 2 Encode(const vector<uebyte>&) -> string

constexpr size_t TAMANHO_BLOCO = 4096;

// Name of a path without its directory: the text after the last '/', or the whole text
// (func 2763, shared with comum/vota code; name inferred).
std::string NomeArquivo(const std::string& caminho);

// Entries never hashed, whatever the directory.                                      name inferred
bool EntradaIgnorada(const std::string& nome)
{
    return nome == "." || nome == ".." || nome == "lost+found" || nome == "tmp";
}

// True when `caminho` (already canonical) or one of its parent directories is in `ignorados`
// (directories appear there with a trailing '/'). Inlined.                          name inferred
bool CaminhoIgnorado(std::filesystem::path caminho, bool ehDiretorio, const std::set<std::string>& ignorados)
{
    if (ignorados.contains(caminho.string() + (ehDiretorio ? "/" : "")))
        return true;
    for (;;) {
        if (caminho.parent_path().empty())            // func 1840 (path::__parent_path)
            return false;
        caminho = caminho.parent_path();
        if (caminho == caminho.root_path())           // libcxx_f808 = path::compare
            return false;
        if (ignorados.contains(caminho.string() + "/"))
            return true;
    }
}

}  // namespace

// cmontadorhash.cpp:49 (inlined)
const std::vector<uebyte> CalculaHashArquivo(const std::string& arquivo)
{
    std::unique_ptr<IHash> hash(
        CPolySingletonList::instance<IGenericFactory<IHash>>().Create());          // :49, factory slot 2
    ecourna::api::io::CFile entrada(arquivo, "rb");                                 // ecourna_f517
    uebyte bloco[TAMANHO_BLOCO];
    while (!entrada.Eof()) {
        const auto lidos = entrada.RawRead(bloco, TAMANHO_BLOCO);
        if (lidos)
            hash->Update(std::vector<uebyte>(bloco, bloco + lidos));               // IHash slot 3
    }
    return hash->Final();                                                           // IHash slot 4
}                                                                                   // ~CFile: Close()

// cmontadorhash.cpp:78 (inlined)
std::string CMontadorHash::CalculaHash(const std::string& arquivo)
{
    std::unique_ptr<ITextEncoding> codificador(
        CPolySingletonList::instance<IGenericFactory<ITextEncoding>>().Create());  // :78
    return codificador->Encode(CalculaHashArquivo(arquivo));                        // Base64
}

// cmontadorhash.cpp:91/:98 (inlined)
std::string CMontadorHash::CalculaHashGeral(const std::string& hashAnterior, const std::string& hash)
{
    if (hashAnterior.empty())
        return hash;
    std::unique_ptr<IHash> h(CPolySingletonList::instance<IGenericFactory<IHash>>().Create());   // :91
    std::vector<uebyte> dados(hashAnterior.begin(), hashAnterior.end());
    dados.insert(dados.end(), hash.begin(), hash.end());                           // shared_f1927
    h->Update(dados);
    const auto digest = h->Final();
    std::unique_ptr<ITextEncoding> codificador(
        CPolySingletonList::instance<IGenericFactory<ITextEncoding>>().Create());  // :98
    return codificador->Encode(digest);
}

// wasm func 5360 - cmontadorhash.cpp:121
// `diretorio` must end with '/': entry paths are built as diretorio + nome (+ "/" for directories).
// `ignorados` holds full paths (directories with a trailing '/') that are skipped, also when reached
// through a symbolic link.
CHashDiretorio CMontadorHash::CriaHashesDiretorio(const std::string& diretorio,
                                                  const std::set<std::string>& ignorados)
{
    std::vector<std::string> arquivos;
    std::vector<std::string> diretorios;

    CDirReader leitor("");                                                          // func 1914
    if (!leitor.Open(diretorio))                                                    // func 5470
        throw CUeHashError(EUeHashError::DIRETORIO_NAO_ABERTO,
                           "CMontadorHash::CriaHashesDiretorio - diretório [" + diretorio +
                               "] não pode ser aberto");                            // :121

    while (leitor.NextEntry()) {
        const std::string nome = leitor.GetEntryName();
        if (EntradaIgnorada(nome))
            continue;

        // stat() follows links: a link to a directory counts as a directory here.
        const std::string caminho = diretorio + nome + (leitor.IsDirectory() ? "/" : "");   // 1912
        if (ignorados.contains(caminho))
            continue;

        if (leitor.IsLink()) {                                                      // func 5469 (lstat)
            std::filesystem::path alvo = std::filesystem::read_symlink(diretorio + nome);
            if (!alvo.has_root_directory())
                alvo = diretorio + alvo.string();                                   // relative link
            const std::filesystem::path canonico = std::filesystem::weakly_canonical(alvo);
            if (CaminhoIgnorado(canonico, leitor.IsDirectory(), ignorados))
                continue;
        }

        if (leitor.IsFile())                                                        // func 2231 (S_IFREG)
            arquivos.push_back(caminho);
        else                                     // directories - and anything else that is not a
            diretorios.push_back(caminho);       // regular file (FIFO, device, dangling link ...)
    }
    leitor.Close();                                                                 // func 2232

    std::sort(arquivos.begin(), arquivos.end());                                    // byte order
    std::sort(diretorios.begin(), diretorios.end());

    // Hashed and listed, but kept out of the overall hash (they differ from urna to urna).
    const std::set<std::string> foraDoHashGeral{"uenux.cfg", "avbootcfg.vsu"};

    std::vector<CHashArquivo> hashesArquivos;
    for (const auto& arquivo : arquivos) {
        const std::string hash = CalculaHash(arquivo);
        const std::string nomeArquivo = NomeArquivo(arquivo);                       // func 2763
        if (!foraDoHashGeral.contains(nomeArquivo))
            ms_hashGeral = CalculaHashGeral(ms_hashGeral, hash);
        hashesArquivos.push_back(CHashArquivo(nomeArquivo, hash));                  // func 3600
    }

    std::vector<CHashDiretorio> hashesDiretorios;
    for (const auto& subdiretorio : diretorios)
        hashesDiretorios.push_back(CriaHashesDiretorio(subdiretorio, ignorados));   // recursion

    return CHashDiretorio(diretorio, hashesArquivos, hashesDiretorios);   // copies (1300, 5363);
                                                                          // ValidaObjeto (chashdiretorio.cpp:41)
}

}  // namespace api::hash
