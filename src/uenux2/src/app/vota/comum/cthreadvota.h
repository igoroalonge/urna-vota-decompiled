// uenux2/src/app/vota/comum/cthreadvota.h   (path inferred: no std::source_location record names this file;
// the class is shared by eleitor/, operador/ and monitor/, and vota/comum/ holds the other shared VOTA code.
// Unit u07's cthreadeleitor.h includes it as "vota/cthreadvota.h": either location is a guess.)
// Reconstructed from vota_web_wasm.wasm (unit u26; the tick API is reconstructed by units u07/u10/u20).
//
// vota::CThreadVota = base of the three VOTA threads (CThreadEleitor, CThreadOperador, CThreadMonitor).
// It owns the tick table (api::CTickManager at +20) and the state-machine context, and implements the
// fatal-error handling shared by the three threads (slots 3 and 4).
//
// RTTI: api::CThread <- vota::CThreadVota (typeinfo @1532484, vtable @1532460):
//   [0] ~CThreadVota (func 2126)   [1] deleting dtor (ICF 325: unreachable, abstract)   [2] Run = 0
//   [3] TrataExcecao(const ecourna::api::exception::CError&) (func 7710)
//   [4] TrataExcecaoDesconhecida(const std::exception&)       (func 7709)
//   [5] FinalizaExecucao = 0
// Slot names 3/4 come from unit u07; the parameter types come from the bodies: 7710 dynamic_casts its
// argument from ecourna::api::exception::CError to api::CUeDesligandoError, 7709 only calls what().
// Slots 3 and 4 are NOT introduced here: api::CThread's own vtable (@1599960) already has slots 2, 3 and 4
// pure (__cxa_pure_virtual), so both handlers are overrides of pure virtuals declared in api::CThread; only
// slot 5 (FinalizaExecucao) is new in CThreadVota. (Unit u18's cthread.h declares the base slots as
// TrataExcecao(const std::exception&) / TrataExcecaoDesconhecida() - the bodies show (const CError&) /
// (const std::exception&): both functions take a second i32 argument.)
// No caller of slots 3/4 exists in this binary (CThread::ThreadProc, func 10255, has no try/catch), so both
// are unreachable in the web build.
#pragma once

#include <exception>
#include <memory>

#include "api/ipc/cthread.h"                     // api::CThread (unit u18)
#include "api/util/ctickmanager.h"               // api::CTickManager (unit u20)
#include "comum/cappstate.h"                     // comum::CAppStateContext
#include "ecourna/api/exception/cerror.hpp"      // ecourna::api::exception::CError

namespace vota {

class CThreadVota : public api::CThread {
public:
    ~CThreadVota() override;                                                        // [0] func 2126

    void Parar() { m_bParar = true; }            // inline: "GetInst()[8] = 1" in funcs 10225 / 10226   name inferred

protected:
    CThreadVota() = default;

    void Run() override = 0;                                                        // [2]
    void TrataExcecao(const ecourna::api::exception::CError& erro) override;       // [3] func 7710 (pure in CThread)
    void TrataExcecaoDesconhecida(const std::exception& erro) override;            // [4] func 7709 (pure in CThread)
    virtual void FinalizaExecucao() = 0;                                            // [5] new in CThreadVota

    api::CTickManager m_ticks;                                   // +20 (std::map, 12 bytes)
    std::unique_ptr<comum::CAppStateContext> m_pContexto;        // +32
};

} // namespace vota
