// Reconstructed from vota_web_wasm.wasm (unit u20).
// Original: uenux2/src/api/persistencia/cdaorepositorio.hpp (srcloc cdaorepositorio.hpp:79).
//
// CDAORepositorio = the process-wide registry of DAO prototypes ("Data Access Objects" over the
// urna's SQLite database trab/uenux.db). At start-up (inlined into vota::CInformacaoEleitor::Inicializar,
// func 7787, "InicializarPersistencia") one prototype of each DAO is registered under typeid(I).name():
//     "N5comum3dao25IComparecimentoMesarioDAOE"  -> comum::dao::CComparecimentoMesarioDAO
//     "N5comum3dao16IJustificadorDAOE"           -> comum::dao::CJustificadorDAO
// Entregar<I>() ("deliver") looks the prototype up, clones it (IDAO vtable slot 2) and hands the
// caller a fresh shared_ptr<I>. Each clone shares the prototype's database connection (the clone copies
// a shared_ptr at +4/+8, see func 3894).
//
// The two instantiations present in the binary are always inlined into the constructor of their only
// client (see src/uenux2/src/app/comum/u20-foreign-fragments.cpp):
//     func 815  = comum::CRegistradorMesario::GetInst()   -> Entregar<IComparecimentoMesarioDAO>()
//     func 3742 = comum::CJustificador::CJustificador()     -> Entregar<IJustificadorDAO>()
#pragma once

#include <format>
#include <map>
#include <memory>
#include <string>
#include <typeinfo>

#include "api/persistencia/idao.h"
#include "ecourna/api/exception/cbaseerror.hpp"

namespace api::persistencia {

// ecourna::api::exception::CBaseError<api::EUePersistenciaError, ...> (typeinfo @1551044; ctor thunk = api_f1715)
enum class EUePersistenciaError : int;
using CUePersistenciaError = ecourna::api::exception::CBaseError<EUePersistenciaError>;

class CDAORepositorio {
public:
    // Registration (inlined in func 7787): s_daos.emplace(Chave<I>(), std::shared_ptr<IDAO>(new DAO(...)))
    template <typename I_DOMINIO_DAO, typename I_ORIGEM = I_DOMINIO_DAO>
    static void Registrar(std::shared_ptr<I_ORIGEM> prototipo);                 // name inferred

    // srcloc cdaorepositorio.hpp:79 (inlined into funcs 815 and 3742)
    template <typename I_DOMINIO_DAO, typename I_ORIGEM = I_DOMINIO_DAO>
    static std::shared_ptr<I_DOMINIO_DAO> Entregar()
    {
        const std::string chave = Chave<I_DOMINIO_DAO, I_ORIGEM>();
        auto it = s_daos.find(chave);                                           // map @1909964
        if (it != s_daos.end()) {
            if (IDAO* copia = it->second->Clonar()) {                           // IDAO slot 2 (func 3894 family)
                if (auto* dao = dynamic_cast<I_DOMINIO_DAO*>(copia))
                    return std::shared_ptr<I_DOMINIO_DAO>(dao);
                delete copia;                                                   // slot 1 (deleting dtor)
            }
        }
        throw CUePersistenciaError(EUePersistenciaError{6802},
                                   std::format("Classe DAO {} não foi registrada.", chave));
    }

private:
    // The key is the mangled type name. The binary compares typeid(I_ORIGEM).name() with
    // typeid(I_DOMINIO_DAO).name() at run time (two identical 39/30-byte literals are built and
    // memcmp'ed) and would append the second name if they differed; for both instantiations they are
    // the same type, so that branch is dead code.                                      // ?
    template <typename I_DOMINIO_DAO, typename I_ORIGEM>
    static std::string Chave()
    {
        std::string chave = typeid(I_ORIGEM).name();
        if (std::string(typeid(I_DOMINIO_DAO).name()) != chave)
            chave += typeid(I_DOMINIO_DAO).name();
        return chave;
    }

    static inline std::map<std::string, std::shared_ptr<IDAO>> s_daos;          // @1909964 (header @1909968)
};

} // namespace api::persistencia
