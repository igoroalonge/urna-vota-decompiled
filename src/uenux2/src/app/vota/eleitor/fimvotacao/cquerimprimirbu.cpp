// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/fimvotacao/cquerimprimirbu.cpp
//
// "Quer imprimir BU?" = in voter-training mode (treinamento do eleitor) the operator may skip printing
// the BU. CInicioBU::StartState (func 12062, other unit) goes here when EhTreinamentoEleitor(),
// otherwise straight to CImprimindoBU.
//
// srcloc evidence:
//   :40  StartState  Assert (poInfo.GetVota().GetEstadoVota() == EAVIMPRIMIRBU)   (3474)
//   :41  StartState  Assert (poInfo.GetVota().GetQtdBU() == 0)                    (3475)
//   (the assert texts say "poInfo": the variable of the original source has that name/typo)
//
// RTTI: comum::CAppState <- vota::CQuerImprimirBU (typeinfo @1542648, vtable @1542580)
//   [0] 244 dtor (ICF)  [1] 387 deleting (ICF)  [2] StartState 12035  [7] ProcessInput 12034

#include "vota/eleitor/fimvotacao/cquerimprimirbu.h"

#include <memory>
#include <mutex>

#include "comum/cappinfo.h"
#include "vota/eleitor/fimvotacao/caplicacaoencerrada.h"
#include "vota/eleitor/fimvotacao/cimprimindobu.h"

namespace vota {

using comum::md::estadoaplicacao::EEstadoVota;

// wasm func 12035 — vtable slot 2
void CQuerImprimirBU::StartState()
{
    auto& poInfo = comum::CAppInfo::GetInst();
    UE_ASSERT(poInfo.GetVota().GetEstadoVota() == EEstadoVota::EAVIMPRIMIRBU);   // :40 (61)
    UE_ASSERT(poInfo.GetVota().GetQtdBU() == 0);                                  // :41
    m_tela->Exibe();
    m_proximoEstado = this;
}

// wasm func 12034 — vtable slot 7 (analyzer name vota::CQuerImprimirBU::vf7)
void CQuerImprimirBU::ProcessInput()
{
    switch (m_tela->Read()) {                                            // cinteractiveform.h:57
    case api::EInputResult::Corrige:                                     // 5: do not print
        comum::CAppInfo::GetInst().GetVota().SetEstadoVota(EEstadoVota::EAVENCERRADA);   // 64
        comum::SalvaEstado();                                            // func 491
        m_proximoEstado = &CAplicacaoEncerrada::GetInst();               // func 3877
        break;
    case api::EInputResult::Confirma:                                    // 9: print the BU
        m_proximoEstado = &CImprimindoBU::GetInst();                     // func 5986
        break;
    default:
        break;
    }
}

// wasm func 5986 — CImprimindoBU::GetInst() (lazy singleton, constructor inlined; the tools put it
// in this file because its callers are CQuerImprimirBU / CInicioBU).
CImprimindoBU& CImprimindoBU::GetInst()
{
    static std::unique_ptr<CImprimindoBU> s_inst;       // @1833652 (mutex residue @1833628;
    static std::mutex s_mutex;                          //  atexit reset: func 12079)
    std::lock_guard<std::mutex> lock(s_mutex);
    if (!s_inst)
        s_inst.reset(new CImprimindoBU());              // 20 bytes
    return *s_inst;
}

CImprimindoBU::CImprimindoBU()
    : comum::CAppState(2),                              // shared_f224: keyboard
      m_tela(CTelasVota::GetInst().m_telaQualidadeBU)   // CTelasVota +100/+104  name inferred
{
}

}  // namespace vota
