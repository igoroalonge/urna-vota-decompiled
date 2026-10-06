// uenux2/src/api/pattern/cpolysingletonlist.h  -- FRAGMENT written by unit u33 (the header belongs to u19,
// see cpolysingletonlist.h / cpolysingletonlist.u19.cpp for the templates themselves).
//
// Four more instances / merged bodies of the poly-singleton registry that the tools left without a file.
#include <memory>
#include <string>
#include <typeinfo>

#include "api/pattern/cpolysingletonlist.h"
#include "comum/gravadores/cgeracaoversoescontratos.h"
#include "vota/eleitor/iniciovotacao/testeteclado/impl/igeradorteclas.h"   // path inferred

namespace api {

// -------------------------------------------------------------------------------------------------------
// wasm func 1005 (observed executing) - table slots 170, 248, 251 ... 278 (12 slots)
// SPolySingleton's constructor as used by lista.emplace_back(nome, &typeid(I), instancia): the element
// constructor that every vector<SPolySingleton>::emplace_back instantiation passes by table slot to the
// shared slow path (func 1009). One body for all interfaces (identical code folding), hence 12 slots.
//     this->nome = std::string(nome); this->tipo = tipo; this->instancia = instancia (use count + 1)
// -------------------------------------------------------------------------------------------------------
//   SPolySingleton(const char* const& nome, const std::type_info* tipo, const std::shared_ptr<void>& instancia)
//       : nome(nome), tipo(tipo), instancia(instancia) {}

// -------------------------------------------------------------------------------------------------------
// wasm func 1562 (observed executing: every registration made by main)                   // name inferred
// Raw-pointer overload of push. Merged body (merge-similar-functions): the push<INTERFACE> to call is the
// third parameter (a table slot), passed by eight 1-line wrappers that main (func 10307) invokes:
//     9372 -> slot 136 push<vota::IExecucaoVota> (8728)
//     9574 -> slot 135 push<IGenericFactory<ISemaphore>> (8758)
//     9650 -> slot 134 push<IGenericFactory<IRWSyncCtl>> (8834)
//     9685 -> slot 133 push<IGenericFactory<ISyncCtl>> (8911)
//     9731 -> slot 132 push<IGenericFactory<IThreadImpl>> (8986)  9831 -> slot 131 push<IFingerPrepare> (9039)
//     9897 -> slot 130 push<ISymmetricCipherFactory> (9135)      9986 -> slot 129 push<IRng> (9233)
// If push throws, the unique_ptr still owns the object and deletes it (vtable slot 1) before rethrowing;
// on success it is empty and nothing happens.
// -------------------------------------------------------------------------------------------------------
template <typename INTERFACE>
void CPolySingletonList::push(INTERFACE* instancia, TPolySingletonsInfo& info)
{
    push(std::unique_ptr<INTERFACE>(instancia), info);
}

// -------------------------------------------------------------------------------------------------------
// wasm func 2860 - exists<comum::CGeracaoVersoesContratos>(info)                           (srcloc-free)
// wasm func 3853 - exists<vota::testeteclado::impl::IGeradorTeclas>(info)
// Two more instantiations of the template exists<INTERFACE> (cpolysingletonlist.h, unit u19) that did not
// fold with the others: the mangled name is built inline from 8-byte pieces, so bodies only merge when
// the names have the same length (34 and 42 characters here, no partner).
//   * std::string nome = "N5comum24CGeracaoVersoesContratosE" / "N4vota12testeteclado4impl14IGeradorTeclasE"
//   * std::shared_lock on info.mutex (TSE's CUpgradeMutex inlined: while (writer active || writers
//     waiting) cv.wait(); ++readers)  - NOTE: without pthreads cv.wait returns at once, so a blocked
//     request would spin forever (unit u19, suspicious #1)
//   * find(nome, info).first (func 608)
//   * unlock_shared: if (readers <= 0) throw std::runtime_error("unlock_shared() called with no shared owner");
//     --readers; if it reaches 0 and a writer waits, notify the writers' condition variable.
// Callers: 2860 <- vota::CGravaResultado::StartState (func 12098: register CGeracaoVersoesContratos once
//          before writing mr.ver) and __wasm_call_ctors; 3853 <- vota::testeteclado::CTesteTeclado::StartState
//          (func 11805, the keyboard test) and __wasm_call_ctors.
// -------------------------------------------------------------------------------------------------------
template bool CPolySingletonList::exists<comum::CGeracaoVersoesContratos>(TPolySingletonsInfo&);
template bool CPolySingletonList::exists<vota::testeteclado::impl::IGeradorTeclas>(TPolySingletonsInfo&);

} // namespace api
