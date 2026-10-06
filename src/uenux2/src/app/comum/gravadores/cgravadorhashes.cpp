// uenux2/src/app/comum/gravadores/cgravadorhashes.cpp
// Reconstructed from vota_web_wasm.wasm (unit u23).
//
// CGravadorHashes (96 bytes, vtable @1554600) writes "...-hash.dat" = ModuloHashes::EntidadeHashes: the hash of
// every file of the urna's installation (root file system + the key directory) at the end of the day, so that the
// TSE can check afterwards which software produced the results.
//   IResultado (+0..+39)   +40 api::CDateTime m_dhGeracao   +52 EUrnaFase m_fase   +56 std::string m_uf
//   +68 std::shared_ptr<md::CIdentificacaoUrnaContingencia> (urna de contingência: município/zona)
//   +76 std::shared_ptr<md::CIdentificacaoSecao>            (urna de seção: município/zona/local/seção)
//   +84 std::string m_versao ("10.23.0.1 - DESENVOLVIMENTO")
// In the harness run of docs/bu/codepath.md this is where the encerramento stops: the web build links no
// api::IGenericFactory<ecourna::api::security::IHash>, so CMontadorHash::CalculaHashGeral throws.
#include "comum/gravadores/cgravadorhashes.h"

#include <filesystem>
#include <set>

#include "api/hash/cmontadorhash.h"
#include "api/io/asn/cfileasn.h"
#include "comum/cpath.h"
#include "comum/dados/cconfiguracaoeleicao.h"
#include "comum/gravadores/md/centidadehashes.h"

namespace comum {

namespace {

// wasm func 5358 (name inferred): flattens the directory tree returned by CMontadorHash into
// (path, hash) pairs. Tree node = { std::string diretorio (+0); vector<{string nome, hash}> arquivos (+12, 24-byte);
// vector<node> subdiretorios (+24, 36-byte) }.
void Achata(std::vector<md::CHashArquivo>& saida, const api::hash::CNoHash& no)
{
    for (const auto& arquivo : no.GetArquivos())
        saida.push_back(md::CHashArquivo(no.GetDiretorio() + arquivo.GetNome(), arquivo.GetHash()));   // api_f3600
    for (const auto& sub : no.GetSubdiretorios()) {
        std::vector<md::CHashArquivo> parcial;
        Achata(parcial, sub);                                                          // recursive
        saida.insert(saida.end(), parcial.begin(), parcial.end());                     // func 5854 (+5852)
    }
}

// wasm func 5359 (name inferred)
std::vector<md::CHashArquivo> ListaHashes(const std::string& raiz, const std::set<std::string>& excluidos)
{
    const api::hash::CNoHash arvore = api::hash::CMontadorHash::CalculaHashGeral(raiz, excluidos);   // func 5360
    std::vector<md::CHashArquivo> hashes;
    Achata(hashes, arvore);
    return hashes;                                                                     // ~tree: api_f2220
}

}  // namespace

// wasm func 11620 (vtable slot 7; srcloc cgravadorhashes.cpp:112)
void CGravadorHashes::GravaResultado(api::CFile& arquivo) const
{
    const md::CCabecalhoEntidade cabecalho(m_dhGeracao, CConfiguracaoEleicao::GetInst().GetPleito(),
                                           md::CCabecalhoEntidade::ETipoId(1));

    // Directories that are NOT hashed: pseudo file systems, the flash areas, temporary files, and the dynamic
    // (per-election) data of both flashes.
    const std::string raiz = CPath::GetRoot();                                         // global @1838600
    const std::set<std::string> excluidos = {
        raiz + "dev/", raiz + "dsk/", raiz + "proc/", raiz + "sys/",
        std::filesystem::path(raiz) / std::filesystem::path("/tmp/").relative_path(),  // comum_f1651 -> "tmp/"
        std::filesystem::path(CPath::GetRootFlash(EFlashOrigem::INTERNA)) / "dinamico/",   // @1838576
        std::filesystem::path(CPath::GetRootFlash(EFlashOrigem::EXTERNA)) / "dinamico/",   // @1838588
        std::filesystem::path(CPath::GetRootFlash(EFlashOrigem::INTERNA)) / "dinamico/" / "tmp/",
    };

    std::vector<md::CHashArquivo> hashes =
        ListaHashes(std::filesystem::path(CPath::GetRoot()) / "", excluidos);            // comum_f3834, / "" (@450187)
    const std::vector<md::CHashArquivo> chaves = ListaHashes(CPath::GetPathChaves(), excluidos);        // "/dsk/fi/estatico/chave/"
    hashes.insert(hashes.end(), chaves.begin(), chaves.end());

    if (m_contingencia) {
        const md::CEntidadeHashes entidade(cabecalho, m_fase, m_uf, *m_contingencia, m_versao, hashes);
        entidade.ValidaCriacao();                                                       // func 5855
        api::CFileASN::CodeObjectFunction(arquivo, entidade);                           // func 5367
    } else if (m_secao) {
        const md::CEntidadeHashes entidade(cabecalho, m_fase, m_uf, *m_secao, m_versao, hashes);
        entidade.ValidaCriacao();
        api::CFileASN::CodeObjectFunction(arquivo, entidade);
    } else {
        throw CUeComumGravadoresError(8644, "Tipo de urna inválido");                  // :112
    }
}

// wasm func 5853 (slot 0) / 11619 (slot 1, deleting): destroys m_versao, both shared_ptrs, m_uf, IResultado.
CGravadorHashes::~CGravadorHashes() = default;

// wasm func 5852: std::copy of CHashArquivo (two std::string) used by the vector range-insert 5854.
// wasm func 5854: std::vector<md::CHashArquivo>::insert(pos, first, last, n)  (libc++ __insert_with_size).

}  // namespace comum
