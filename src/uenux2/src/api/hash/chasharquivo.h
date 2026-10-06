// Reconstructed from vota_web_wasm.wasm (unit u17).
// Original: uenux2/src/api/hash/chasharquivo.h (path inferred from chasharquivo.cpp)
//           + the CHashDiretorio declaration of uenux2/src/api/hash/chashdiretorio.h (path from the
//             srcloc chashdiretorio.cpp:41; that .cpp is not part of this unit, its ValidaObjeto is
//             inlined into CMontadorHash::CriaHashesDiretorio, func 5360).
//
// The "hash tree" written to the result file hash.dat (comum::CGravadorHashes, ASN.1
// ModuloHashes::EntidadeHashes): a directory node with the hashes of its files and its
// sub-directories. Each hash is the Base64 text of the SHA-512 of the file contents.
#pragma once

#include <string>
#include <vector>

namespace api::hash {

// api::EUeHashError, CBaseError limits {5100, ...}; typeinfo @1599692; thunk func 2723 builds it.
enum class EUeHashError : int {
    NOME_ARQUIVO_INVALIDO = 5100,   // "CHashArquivo: nome inválido."            names inferred
    HASH_ARQUIVO_INVALIDO = 5101,   // "CHashArquivo: hash inválido."
    NOME_DIRETORIO_INVALIDO = 5102, // "CHashDiretorio: nome inválido."
    DIRETORIO_NAO_ABERTO = 5103,    // "CMontadorHash::CriaHashesDiretorio - diretório [..] não pode ser aberto"
};

// 24 bytes. ASN.1 counterpart: ModuloTiposEleitorais::ArquivoAssinatura {nome, hash}
// (converter comum::asn::CConversorHashArquivo).
class CHashArquivo {
public:
    // wasm func 3600 - copies both strings, then validates.                       name inferred
    // Callers: CMontadorHash::CriaHashesDiretorio (5360), comum_f5358 (CGravadorHashes path),
    // CConversorHashArquivo::DoDesconverte (10269).
    CHashArquivo(const std::string& nome, const std::string& hash)
        : m_nome(nome), m_hash(hash)
    {
        ValidaObjeto();
    }

    void ValidaObjeto() const;          // wasm func 5364 (chasharquivo.cpp:26/:30)

    const std::string& GetNome() const { return m_nome; }
    const std::string& GetHash() const { return m_hash; }

private:
    std::string m_nome;                 // +0   file name without directory
    std::string m_hash;                 // +12  Base64(SHA-512(contents))
};

// 36 bytes.
class CHashDiretorio {
public:
    CHashDiretorio(const std::string& nome, const std::vector<CHashArquivo>& arquivos,
                   const std::vector<CHashDiretorio>& subdiretorios)
        : m_nome(nome), m_arquivos(arquivos), m_subdiretorios(subdiretorios)   // funcs 1300, 5363
    {
        ValidaObjeto();
    }
    // Copy constructor: implicit (inlined into func 5363, std::vector<CHashDiretorio> copy).
    // wasm func 2220: the implicit destructor (recursive through m_subdiretorios).
    ~CHashDiretorio() = default;

    // chashdiretorio.cpp:41 - inlined into func 5360
    void ValidaObjeto() const;          // if (m_nome.empty()) throw (5102) "CHashDiretorio: nome inválido."

private:
    std::string m_nome;                         // +0   the directory path as passed (ends with '/')
    std::vector<CHashArquivo> m_arquivos;       // +12  sorted by full path
    std::vector<CHashDiretorio> m_subdiretorios;// +24  sorted by full path
};

}  // namespace api::hash
