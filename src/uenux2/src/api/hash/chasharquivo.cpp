// Reconstructed from vota_web_wasm.wasm (unit u17).
// Original: uenux2/src/api/hash/chasharquivo.cpp (srclocs chasharquivo.cpp:26 and :30).
#include "api/hash/chasharquivo.h"

#include "ecourna/api/exception/cbaseerror.hpp"

namespace api::hash {

using CUeHashError = ecourna::api::exception::CBaseError<EUeHashError>;   // thunk func 2723

// wasm func 5364 (caller: the constructor, func 3600)
void CHashArquivo::ValidaObjeto() const
{
    if (m_nome.empty())
        throw CUeHashError(EUeHashError::NOME_ARQUIVO_INVALIDO, "CHashArquivo: nome inválido.");   // :26
    if (m_hash.empty())
        throw CUeHashError(EUeHashError::HASH_ARQUIVO_INVALIDO, "CHashArquivo: hash inválido.");   // :30
}

}  // namespace api::hash
