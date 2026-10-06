// Reconstructed from vota_web_wasm.wasm (unit u06). Original: uenux2/src/app/vota/eleitor/cconferevotoemcargo.cpp
//
// srcloc evidence:
//   :41  comum::TCargoID vota::(anonymous namespace)::GetCargoID()   "O cargo atual nao esta posicionado" (9305)
//   :80  virtual void vota::IConfereVotoEmCargo::ProcessInputAudio()                (IInputKbd lookup)
//   :95  void vota::IConfereVotoEmCargo::ExecutarAposAudioAtual(std::function<void ()>)   (ISound lookup)
// Lambda names from the std::function vtables: IConfereVotoEmCargo::StartStateAudio()::$_0/$_1,
// IConfereVotoEmCargo::ProcessTickAudio(unsigned char)::$_0, ExecutarAposAudioAtual::$_0/$_1/$_2.

#include "vota/eleitor/cconferevotoemcargo.h"

#include "api/hwil/iinputkbd.h"
#include "api/hwil/isound.h"
#include "comum/dados/ccargos.h"
#include "vota/eleitor/cthreadeleitor.h"
#include "vota/eleitor/comum/ctelasvota.h"
#include "vota/eleitor/cconfirmavotoemcargo.h"

namespace vota {

namespace {

// srcloc :41 — inlined into StartStateAudio (func 11793)
comum::TCargoID GetCargoID()
{
    auto& cargos = comum::CCargos::GetInst();
    if (cargos.IsEnd())
        throw CUeVotaError(9305, "O cargo atual nao esta posicionado", std::source_location::current());
    return cargos.GetCurrentCargoID();
}

}  // namespace

// ---------------------------------------------------------------------------------------------
// wasm func 11793 — vtable slot 10 (the analyzer named it after the inlined GetCargoID).
// Observed executing.
void IConfereVotoEmCargo::StartStateAudio()
{
    m_proximoEstado = this;
    if (m_comTick) {
        CThreadEleitor::GetInst().StartTick(m_tick);
        const auto tela = CTelasVota::GetInst().GetTelaCargo(GetCargoID(), m_tela);  // func 4135
        tela->Exibe();
        ExecutarAposAudioAtual([this] {                                          // $_0 (func 11786)
            if (m_audioHabilitado)
                PlayInterruptibleMessage(FormataMensagem(GetMensagemAudio()));  // slots 15 -> 14
        });
    } else {
        // no conferência screen: go to the confirmation state as soon as the audio ends
        ExecutarAposAudioAtual([this] { m_proximoEstado = GetProximoEstado(); }); // $_1 (func 5934)
    }
}

// wasm func 11790 — vtable slot 12 (not in this unit, shown for the flow)
//   void IConfereVotoEmCargo::ProcessTickAudio(uebyte tick) {
//       if (m_comTick && tick == m_tick) {
//           CThreadEleitor::GetInst().StopTick(m_tick);
//           ExecutarAposAudioAtual([this] { m_proximoEstado = GetProximoEstado(); });   // $_0
//       }
//   }

// ---------------------------------------------------------------------------------------------
// wasm func 11791 — vtable slot 9, srcloc :80. Observed executing: every key pressed while the
// conferência is on screen is refused.
void IConfereVotoEmCargo::ProcessInputAudio()
{
    auto& teclado = api::IInputKbd::GetInst();              // :80
    PlayKey(0);                                             // "Tecla indevida pressionada" + beep
    teclado.Clear();                                        // IInputKbd slot 4
}

// ---------------------------------------------------------------------------------------------
// wasm func 3851 — srcloc :95. Observed executing.
void IConfereVotoEmCargo::ExecutarAposAudioAtual(std::function<void()> acao)
{
    auto& som = api::ISound::GetInst();                     // :95

    if (m_espera) {                                         // cancel a previous wait
        m_espera->Cancela();                                // CEsperaAudio slot 2 (name inferred)
        m_espera.reset();
    }

    // ISound slot 8 (name inferred): true when nothing is playing (and the predicate is false).
    if (som.AudioLivre([] { return false; })) {             // $_0 (func 340: return false)
        acao();
        return;
    }

    // ISound slot 9 (name inferred): calls the 2nd callback when the audio ends or is interrupted.
    m_espera = som.EsperaFimAudio(
        [this] { return GetNextState() != this; },          // $_1 (func 11768): abort if we left
        [this, acao = std::move(acao)](bool terminou) {     // $_2 (func 11760)
            m_espera.reset();
            if (terminou && GetNextState() == this)
                acao();
        });
}

// ---------------------------------------------------------------------------------------------
// Confirmation-state singletons returned by CConfereVotoEmCargo<T, TELA>::GetProximoEstado().
//
// wasm funcs 1961 / 2901 are wasm-opt *merged* bodies of the lazy singletons T::GetInst() for the
// proportional (1961) and majoritarian (2901) confirmation states; the constants are the mutex,
// the unique_ptr storage, the vtable and the constructor arguments:
//
//   T& T::GetInst() { if (!s) s.reset(new T); s_mutex.unlock(); return *s; }
//
// wasm func 11752  CConfirmaProporcional::CConfirmaProporcional(ETelaVotacao tela, CVoto::ETipo tipo)
//                      : CConfirmaVotoEmCargo(6 /*keys|ticks*/, tela, tipo) {}
// wasm func 11700  CConfirmaMajoritario::CConfirmaMajoritario(uebyte flags, ETelaVotacao tela, CVoto::ETipo tipo)
//                      : CConfirmaVotoEmCargo(flags, tela, tipo) {}
//
// template instance (slot 16)                       func   -> state                 flags tela (ETelaVotacao)      tipo (RDV TipoVoto)
// CConfereVotoEmCargo<CConfirmaVotoNominal, 2>      11715  CConfirmaVotoNominal       6   1 Completa                2 nominal
// CConfereVotoEmCargo<CConfirmaVotoLegenda, 10>     11717  CConfirmaVotoLegenda       6   9 VotoLegendaIncompleto   1 legenda
// CConfereVotoEmCargo<CCandidatoInexistente, 12>    11716  CCandidatoInexistente      6  11 CandidatoInexistente    1 legenda (!)
// CConfereVotoEmCargo<CCandidatoInapto, 14>         11739  CCandidatoInapto           6  13 CandidatoInapto         4 nulo
// CConfereVotoEmCargo<CProporcionalBranco, 4>       11707  CProporcionalBranco        6   3 VotoBranco              3 branco
// CConfereVotoEmCargo<CProporcionalNulo, 7>         11738  CProporcionalNulo          6   6 VotoNulo                4 nulo
// CConfereVotoEmCargo<CMajoritarioValido, 2>        11670  CMajoritarioValido         2   1 Completa                2 nominal
// CConfereVotoEmCargo<CMajoritarioRepetido, 16>     11671  CMajoritarioRepetido       6  15 CandidatoRepetido       7 nuloPorRepeticao
// CConfereVotoEmCargo<CMajoritarioNulo, 7>          11672  CMajoritarioNulo           6   6 VotoNulo                4 nulo
// CConfereVotoEmCargo<CMajoritarioBranco, 4>        11673  CMajoritarioBranco         6   3 VotoBranco              3 branco
//
// e.g. func 11715:
//   comum::CAppState* CConfereVotoEmCargo<CConfirmaVotoNominal, ETelaVotacao::ConferenciaVotoNominal>::GetProximoEstado()
//   { return &CConfirmaVotoNominal::GetInst(); }   // new CConfirmaVotoNominal : CConfirmaProporcional(Completa, nominal)
//
// Also in this unit (no file assigned): func 11699 CConfirmaMajoritario::GetEstadoCorrige() and
// func 11751 CConfirmaProporcional::GetEstadoCorrige() -> see cconfirmavotoemcargo.cpp.

}  // namespace vota
