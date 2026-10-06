// uenux2/src/api/hash/chasharquivo.h (+ cmontadorhash.cpp)  -- FRAGMENT written by unit u33.
// The hash classes are reconstructed by unit u17 (chasharquivo.*, cmontadorhash.*).
#include <source_location>
#include <string>

#include "api/hash/chasharquivo.h"
#include "ecourna/api/exception/cbaseerror.h"

namespace api::hash {

// wasm func 2723                                                                    // name inferred
// Constructor thunk of the api/hash error class:
//     CBaseError<api::EUeHashError, SErrorLimits{5100, 5150}>(EUeHashError codigo, std::string mensagem,
//                                                           const std::source_location& onde)
// = ecourna_f710(this, codigo, mensagem, onde, vtable @1599728): the shared CError constructor (func 1143)
// followed by the store of this instantiation's vtable. Each CBaseError<E> instantiation has such a thunk
// (comum_f283, comum_f480, api_f1229 ...). Callers: CMontadorHash::CalculaHashGeral /
// CriaHashesDiretorio (5360, with CHashDiretorio inlined: 5102 "CHashDiretorio: nome inválido."
// chashdiretorio.cpp:41, 5103 "diretório [..] não pode ser aberto" cmontadorhash.cpp:121) and
// CHashArquivo::ValidaObjeto (5364: 5100 "CHashArquivo: nome inválido." chasharquivo.cpp:26,
// 5101 "CHashArquivo: hash inválido." chasharquivo.cpp:30).
using CUeHashError = ecourna::api::exception::CBaseError<EUeHashError>;
//   CUeHashError::CUeHashError(EUeHashError codigo, std::string mensagem, const std::source_location& onde)
//       : CError(static_cast<int>(codigo), std::move(mensagem), onde) {}

} // namespace api::hash
