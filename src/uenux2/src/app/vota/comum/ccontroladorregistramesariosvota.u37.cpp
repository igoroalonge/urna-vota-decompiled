// uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp (path inferred, as in units u20/u27)
// -- FRAGMENT written by unit u37.
//
// vota::CControladorRegistraMesariosVota implements comum::IControladorRegistraMesarios (interface and slot
// names: comum/comparecimentomesario/icontroladorregistramesarios.h, unit u22) - the VOTA side of the
// "registro de mesários" (attendance of the poll workers, identified by título and fingerprint). Its log
// slots are one-liners; wasm-opt merged the bodies of equal shape:
//   func 3888  body of slots 29, 34, 36 (47-byte literal)          -> vf29 10773, vf34 10768, vf36 10766
//   func 3887  body of slots 31, 32, 33 (format with the título being registered)  -> 10771, 10770, 10769
//   func 6017  body of slots 19, 27 (format with the título given as argument)    -> 10784, 10775
// Only CAjusteInicial::StartState (7160) pushes the controller, and only when
// CInformacaoEleicao::IdentificaMesarios() (func 1950: NOT demonstration mode AND ParametrosUrna.registrarMesarios,
// PU +400, TRUE in the published scenarios) is true and no IControladorRegistraMesarios exists yet. So it is
// active on REAL urnas and never on a demonstration urna. It never runs in the web build either, because
// CAjusteInicial never runs there (the first state is CAguardaMensagem).
//
// PRIVACY NOTE: the título (voter registration number) of the mesário is written in clear into the event
// log (logd.dat) by slots 19, 27, 31, 32 and 33.
#include <format>
#include <string>

#include "vota/comum/ccontroladorregistramesariosvota.h"
#include "vota/log/clogvota.h"
#include "vota/operador/cthreadoperador.h"

namespace vota {

// ---- merged body 3888 --------------------------------------------------------------------------------
void CControladorRegistraMesariosVota::LogaEncerrouRegistro()              // slot 29, wasm 10773
{
    CLogVota::GetInst().Loga("Operador encerrou ciclo de registro de mesários");
}
void CControladorRegistraMesariosVota::LogaConferenciaBiometria()          // slot 34, wasm 10768
{
    CLogVota::GetInst().Loga("Realizada a conferência da biometria do mesário");
}
void CControladorRegistraMesariosVota::LogaIndagadoFinalizarRegistro()     // slot 36, wasm 10766
{
    CLogVota::GetInst().Loga("Operador indagado se finaliza registro mesários");
}

// ---- merged body 3887: std::format(<literal>, título in CThreadOperador +96) ------------------------------
void CControladorRegistraMesariosVota::LogaPedidoLeituraBiometria()        // slot 31, wasm 10771
{
    CLogVota::GetInst().Loga(std::format("Pedido de leitura da biometria do mesário {}",
                                         CThreadOperador::GetInst().GetTituloMesario()));   // wasm 270, +96
}
void CControladorRegistraMesariosVota::LogaMesarioEhEleitor()              // slot 32, wasm 10770
{
    CLogVota::GetInst().Loga(std::format("Mesário {} é eleitor da seção",
                                         CThreadOperador::GetInst().GetTituloMesario()));
}
void CControladorRegistraMesariosVota::LogaMesarioNaoEhEleitor()           // slot 33, wasm 10769
{
    CLogVota::GetInst().Loga(std::format("Mesário {} não é eleitor da seção",
                                         CThreadOperador::GetInst().GetTituloMesario()));
}

// ---- merged body 6017: severity 1 through api::CLoga::loga directly --------------------------------------
void CControladorRegistraMesariosVota::LogaMesarioRegistrado(const std::string& titulo)      // slot 19, 10784
{
    CLogVota::GetInst().Loga(std::format("Mesário {} registrado", titulo));
}
void CControladorRegistraMesariosVota::LogaMesarioJaRegistrado(const std::string& titulo)    // slot 27, 10775
{
    CLogVota::GetInst().Loga(std::format("Mesário {} já registrado", titulo));
}

} // namespace vota
