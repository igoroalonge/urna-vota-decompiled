// uenux2/src/app/comum/gravadores/md/centidadehashes.cpp
// Reconstructed from vota_web_wasm.wasm (unit u23).
//
// md::CEntidadeHashes = ModuloHashes::EntidadeHashes (hash.dat): the SHA hashes of the files of the urna's
// installation at the end of the day. Layout (built in CGravadorHashes::GravaResultado, func 11620):
//   +0  CCabecalhoEntidade (20)   +20 EUrnaFase m_fase   +24 std::string m_uf (2 letters)
//   +36 identificação: CIdentificacaoUrnaContingencia (município, zona) or CIdentificacaoSecao (12 bytes)
//   +76 std::string m_versao      +88 std::vector<CHashArquivo> m_hashes (24-byte {nome, hash} pairs)
#include <format>

#include "comum/gravadores/iresultado.h"
#include "comum/gravadores/md/centidadehashes.h"

namespace comum::md {

// wasm func 5855 (srcloc centidadehashes.cpp:59, :62)
void CEntidadeHashes::ValidaCriacao() const
{
    if (m_fase == EUrnaFase('0') || static_cast<int>(m_fase) >= '4')
        throw CUeComumGravadoresError(8672, "Fase inválida");                                  // :59
    if (m_uf.size() != 2)
        throw CUeComumGravadoresError(8673, std::format("UF inválida: {}", m_uf));             // :62
}

}  // namespace comum::md
