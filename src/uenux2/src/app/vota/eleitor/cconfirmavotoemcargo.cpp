// Reconstructed from vota_web_wasm.wasm (unit u06). Original: uenux2/src/app/vota/eleitor/cconfirmavotoemcargo.cpp
//
// srcloc evidence:
//   :71  CFormInterativoTelaVota vota::CConfirmaVotoEmCargo::GetTelaCargoAtual(const std::string &)
// __PRETTY_FUNCTION__ strings passed to GetTelaCargoAtual name the callers:
//   "virtual void vota::CConfirmaVotoEmCargo::StartStateAudio()"     (func 11755)
//   "virtual void vota::CConfirmaVotoEmCargo::ProcessInputAudio()"   (func 11754)

#include "vota/eleitor/cconfirmavotoemcargo.h"

#include "comum/dados/ccargos.h"
#include "vota/eleitor/celeitorvotando.h"
#include "vota/eleitor/comum/ctelasvota.h"
#include "vota/eleitor/votamajoritario/cpedemajoritario.h"
#include "vota/eleitor/votaproporcional/cpedeproporcional.h"

namespace vota {

// wasm func 5929 (attributed to cconferevotoemcargo.cpp by the analyzer). Observed executing.
CConfirmaVotoEmCargo::CConfirmaVotoEmCargo(uebyte flags, ETelaVotacao tela, comum::md::CVoto::ETipo tipo)
    : CVotacaoStateAudio(flags)                 // func 1785
    , m_tela(tela)
    , m_tipoVoto(tipo)
{
}

// wasm func 11752 (observed) / 11700
CConfirmaProporcional::CConfirmaProporcional(ETelaVotacao tela, comum::md::CVoto::ETipo tipo)
    : CConfirmaVotoEmCargo(6, tela, tipo)
{
}

CConfirmaMajoritario::CConfirmaMajoritario(uebyte flags, ETelaVotacao tela, comum::md::CVoto::ETipo tipo)
    : CConfirmaVotoEmCargo(flags, tela, tipo)
{
}

// ---------------------------------------------------------------------------------------------
// wasm func 5928, srcloc :71. The body is a wasm-opt merged helper (func 3921, shared with
// CConfirmaVotoSemCandidato and CCompletaProporcional); the thunk passes the srcloc and code 9306.
CFormInterativoTelaVota CConfirmaVotoEmCargo::GetTelaCargoAtual(const std::string& funcao)
{
    auto& cargos = comum::CCargos::GetInst();
    if (cargos.IsEnd())
        throw CUeVotaError(9306, funcao + " - o cargo atual nao esta posicionado",
                           std::source_location::current());
    return CTelasVota::GetInst().GetTelaCargo(cargos.GetCurrent().GetId(), m_tela);
}

// ---------------------------------------------------------------------------------------------
// wasm func 11755 — vtable slot 10 (shared by every CConfirmaVotoEmCargo subclass). Observed executing.
void CConfirmaVotoEmCargo::StartStateAudio()
{
    GetTelaCargoAtual(__PRETTY_FUNCTION__)->Exibe();
    m_proximoEstado = this;
}

// wasm func 11754 — vtable slot 9 (unit u08; EmiteEcoCorrigeConfirma inlined). For the flow:
//   void CConfirmaVotoEmCargo::ProcessInputAudio() {
//       auto tela = GetTelaCargoAtual(__PRETTY_FUNCTION__);
//       const auto resultado = EmiteEcoCorrigeConfirma(tela);   // Read(); speaks 'C'/'D' keys
//       switch (resultado) {
//       case EInputResult::Corrige:  TrataResultado(Corrige); m_proximoEstado = GetEstadoCorrige(); break;
//       case EInputResult::Confirma: {
//           // order in the binary: cargo id, then a COPY of g_votoDigitado, then GetInst(); the copy
//           // is freed only after TrataResultado/m_proximoEstado, so it is a local, not a temporary
//           const auto cargo = CCargos::GetInst().GetCurrent().GetId();
//           const std::string numero = g_votoDigitado;
//           CEleitorVotando::GetInst().RegistraVoto(cargo, m_tipoVoto, numero);
//           TrataResultado(Confirma); m_proximoEstado = nullptr; break;
//       }
//       default: break;
//       }
//   }

// ---------------------------------------------------------------------------------------------
// wasm func 11751 — CConfirmaProporcional vtable slot 16 (also every proportional confirm state).
// Observed executing.
comum::CAppState* CConfirmaProporcional::GetEstadoCorrige()
{
    return &CPedeProporcional::GetInst();       // func 3849 -> merged singleton body func 6051
}

// wasm func 11699 — CConfirmaMajoritario vtable slot 16.
comum::CAppState* CConfirmaMajoritario::GetEstadoCorrige()
{
    return &CPedeMajoritario::GetInst();        // func 5921 -> func 6051
}

}  // namespace vota
