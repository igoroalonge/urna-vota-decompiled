// Reconstructed from vota_web_wasm.wasm (unit u20).
// Original: uenux2/src/api/persistencia/iuenuxgenericdao.h (srclocs iuenuxgenericdao.h:41, :49, :57).
//
// Generic DAO interface of the urna's persistence layer (SQLite database trab/uenux.db). Concrete DAOs
// (comum::dao::CComparecimentoMesarioDAO, CJustificadorDAO, CEleitorDinamicoDAO) override the operations
// they support; the others keep these defaults, which throw.
//
// IDAO / IUenuxGenericDAO vtable (identical in the three concrete DAOs):
//   [0] ~dtor (icf 1704)  [1] deleting dtor (icf 3767)  [2] Clonar() (func 3894 family)
//   [3] Inserir?  [4] Excluir(const TDado&)  [5] ExcluirID(const TDadoID&)  [6] Atualizar(const TDado&)
//   [7] ?  [8] ?                                                         (slots 3, 7, 8: other units)
//
// Which defaults survive in the binary:
//   CComparecimentoMesarioDAO: Excluir (10398), ExcluirID (10397), Atualizar (10396) -> attendance rows
//                              of mesários can only be inserted/read, never deleted or updated.
//   CJustificadorDAO:          Excluir (11467)          (ExcluirID and Atualizar are implemented)
//   CEleitorDinamicoDAO:       Excluir (11540)          (ExcluirID is implemented)
#pragma once

#include <string>
#include <typeinfo>

#include "api/persistencia/cdaorepositorio.hpp"   // CUePersistenciaError
#include "api/persistencia/idao.h"

namespace api::persistencia {

namespace detail {
// wasm func 2297 (tools: api_f2297) - merged body of every default below: builds
//   std::string(prefixo) + std::string(typeid(DADO).name())
// and throws CUePersistenciaError(codigo, mensagem) with the caller's source_location.   name inferred
[[noreturn]] inline void NaoImplementada(int codigo, const char* prefixo, const char* nomeTipo)
{
    throw CUePersistenciaError(EUePersistenciaError{codigo}, prefixo + std::string(nomeTipo));
}
} // namespace detail

template <typename DADO, typename ID>
class IUenuxGenericDAO : public IDAO {
public:
    using TDado = DADO;
    using TDadoID = ID;

    // slot 4 - funcs 10398 (<CComparecimentoMesario, CComparecimentoMesarioPK>),
    //          11467 (<ecourna::app::dados::CIdentificacaoJustificativa, std::string>),
    //          11540 (<comum::md::CEleitorDinamico, std::string>)            srcloc iuenuxgenericdao.h:41
    virtual void Excluir(const TDado&) const
    {
        detail::NaoImplementada(6804, "Excluir não implementada para entidade ", typeid(DADO).name());
    }

    // slot 5 - func 10397 (<CComparecimentoMesario, CComparecimentoMesarioPK>) srcloc iuenuxgenericdao.h:49
    virtual void ExcluirID(const TDadoID&) const
    {
        detail::NaoImplementada(6805, "ExcluirID não implementada para entidade ", typeid(DADO).name());
    }

    // slot 6 - func 10396 (<CComparecimentoMesario, CComparecimentoMesarioPK>) srcloc iuenuxgenericdao.h:57
    virtual void Atualizar(const TDado&) const
    {
        detail::NaoImplementada(6806, "Atualizar não implementada para entidade ", typeid(DADO).name());
    }
};

} // namespace api::persistencia
