// uenux2/src/app/vota/log/clogvota.cpp (attested by srclocs :37, :432) -- FRAGMENT written by unit u37.
// Class: clogvota.h (unit u26); other fragments: clogvota.u09.cpp.
//
// vota::CLogVota has one method per fixed record of the urna's event log (/dsk/fi/dinamico/log/logd.dat,
// Latin-1 "<aplicativo>|<severidade>|<texto>", aplicativo 1 = VOTA). Severity: 1 informação, 2 aviso,
// 3 erro. The methods below were kept out of line; wasm-opt then merged the bodies of the messages of equal
// length (the 8-byte chunks of the literal became parameters):
//   func 3902 = body of the 38-byte-message methods (severity as parameter)   - 5880, 5881, 5885
//   func 6115 = body of the 55-byte-message warnings (IEventosLog::LogaAviso)    - 4556, api_f2495
#include <string>

#include "comum/log/ieventoslog.h"
#include "vota/log/clogvota.h"

namespace comum {
// wasm func 2282 (tools: vota_f2282). Out-of-line copy of the inline severity-3 helper of the base class
// (like LogaAviso = api_f1398 for severity 2). Callers: 3279, 4550 (below), CEleitorVotando::
// ChamaEstadoProximoCargo (4459), CDigitalNaoReconhecidaDecBiometria slot 2 (10478),
// CQuerReimprimirZeresima::StartState (11849).                                     name inferred
void IEventosLog::LogaErro(const std::string& mensagem) const
{
    api::CLoga::loga(m_aplicativo, api::ESeveridade{3}, mensagem);                   // wasm 433
}
} // namespace comum

namespace vota {

// wasm func 3279 (tools: vota_f3279). Callers: CAjusteInicial::StartState (7160, default branch of the
// restart router on EstadoGeralVota.estadoVota) and CDefineRotaPosReinicio::NeedChangeState (11858).
void CLogVota::LogaErroEstadoDesconhecido()                                         // name inferred
{
    LogaErro("Erro estado do aplicativo não conhecido");                            // severity 3
}

// wasm func 4550 (tools: vota_f4550). Callers: CVotacaoStateAudio::FormatPartyName (6954) and
// CPedeMajoritario::ProcessInputAudio (11683) - a party number typed by the voter that is not in the
// party table while the audio (accessible) vote is on.                     name as used by units u08/u26
void CLogVota::LogaErroPartidoNaoEncontrado()
{
    LogaErro("Erro partido não encontrado");                                         // severity 3
}

// wasm func 4556 (tools: vota_f4556 -> merged body api_f6115 -> IEventosLog::LogaAviso, api_f1398).
// Callers: CLimiteCopiasBUAtingido::StartState (12027) and CEmitirMaisBU::ProcessInput (12081): the mesário
// asked for more BU copies than ParametrosUrna.numBUVotaAdicionais allows.    name as used by unit u09
void CLogVota::LogaQtdViasExcedeMaximo()
{
    LogaAviso("Quantidade de vias adicionais excede o máximo permitido");            // severity 2
}

// wasm func 5881 (tools: vota_f5881 -> merged body 3902 with severity 3). Callers:
// testeteclado::CPreZeresima::GetEstadoPassouNoTeste (5945, then throws CUeVotaError 9377 "Estado nao esperado
// <valor>", cprezeresima.cpp:56) and CVerificaEleicaoPassou::StartState (11914, then throws 9376 "Estado nao
// esperado", cverificaeleicaopassou.cpp:70).                                 name as used by unit u09
void CLogVota::LogaEstadoNaoEsperado()
{
    Loga(api::ESeveridade{3}, "Erro estado do aplicativo não esperado");
}

// wasm func 5885 (tools: vota_f5885 -> merged body 3902 with severity 3). Callers: CAjusteInicial::StartState
// (7160) and CCopiaResultadoParaMR::CopiaResultado (12134): the result medium (MR, USB stick of the
// encerramento) was not detected after IInterfaceInit::HabilitarMR (2863).    name as used by unit u09
void CLogVota::LogaMRNaoPresente()
{
    Loga(api::ESeveridade{3}, "Mídia de resultado não estava presente");
}

} // namespace vota
