// uenux2/src/app/comum/comparecimentomesario/icontroladorregistramesarios.h  (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u22).
//
// The comum mesário-registration states (estados/*.cpp) are application-independent: everything that
// depends on the host application (VOTA) goes through this interface, looked up with
// api::CPolySingletonList::instance<IControladorRegistraMesarios>() (func 356). The only
// implementation is vota::CControladorRegistraMesariosVota (vtable @1586608, funcs 10766..10800),
// pushed by vota::CAjusteInicial (u06) - never in the web build, where the lookup would throw.
//
// RTTI: comum::IControladorRegistraMesarios (typeinfo @1534552, no vtable of its own). No srcloc or
// RTTI gives the method names: every name below is inferred from the VOTA implementation (which
// strings it logs, which singletons it returns) and from the call sites.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace comum {

using uebyte = std::uint8_t;
class CAppState;
class CEleitorDetalhe;

// Which registration round the urna is in, derived by VOTA from EstadoGeralVota.estadoVota:
//   '7' REGISTROMESARIOINICIAL -> INICIAL, '8' VOTAR -> VOTACAO, ':' REGISTROMESARIOFINAL -> FINAL,
//   anything else -> NENHUM (table @534720 = {1, 2, 0, 3} indexed by estado - '7').
enum class EPeriodoRegistro : int { NENHUM = 0, INICIAL = 1, VOTACAO = 2, FINAL = 3 };   // names inferred

// Maximum number of mesários per round (hard-coded in both the comum screen and VOTA's log).
inline constexpr int QTD_MAXIMA_MESARIOS = 6;                                            // name inferred

class IControladorRegistraMesarios
{
public:
    virtual ~IControladorRegistraMesarios() = default;                            // slots 0/1
    virtual uebyte CriaTick(std::uint32_t ms) = 0;                                // 2  CThreadOperador tick
    virtual void StopTick(uebyte tick) = 0;                                       // 3
    virtual void StartTick(uebyte tick) = 0;                                      // 4
    virtual void AtualizaEstadoRegistro() = 0;                                    // 5  '6'->'7', '9'->':' ...
    virtual EPeriodoRegistro GetPeriodoRegistro() const = 0;                      // 6
    virtual bool LimiteMesariosAtingido() const = 0;                              // 7  > 5 rows in the period
    virtual bool Slot8() const = 0;                                               // 8  VOTA: return true; unused here
    virtual CAppState* GetEstadoAposRegistroInicial() = 0;                        // 9  VOTA: CAguardaInicio
    virtual void NotificaFimRegistroInicial() = 0;                                // 10 VOTA: message 11 -> voter thread
    virtual CAppState* GetEstadoAposRegistroVotacao() = 0;                        // 11 VOTA: CRegistroMesarioEncerrado
    virtual CAppState* GetEstadoAposRegistroFinal() = 0;                          // 12 VOTA: CFinalizaOperador
    virtual bool MesarioEhEleitorDaSecao() const = 0;                             // 13 CEleitores lookup != null
    virtual const CEleitorDetalhe* GetEleitorMesario() const = 0;                 // 14
    virtual void NotificaFimRegistroFinal() = 0;                                  // 15 VOTA: message 7 (-> BU)
    virtual void SincronizaBancoDados() = 0;                                      // 16 (name used by u02)
    virtual const std::string& GetTituloMesario() const = 0;                      // 17 CThreadOperador +96
    virtual void SetTituloMesario(const std::string& titulo) = 0;                 // 18
    virtual void LogaMesarioRegistrado(const std::string& titulo) = 0;            // 19 "Mesário {} registrado"
    virtual void LogaRegistroAntesVotacao() = 0;                                  // 20 "Registrando mesários antes da votação"
    virtual void LogaRegistroDuranteVotacao() = 0;                                // 21 "... durante a votação"
    virtual void LogaRegistroAposVotacao() = 0;                                   // 22 "... após a votação"
    virtual void LogaIndagadoRegistro() = 0;                                      // 23 "Operador indagado se ocorrerá registro de mesários"
    virtual void LogaConfirmouRegistro() = 0;                                     // 24 "Operador confirmou o registro de mesários"
    virtual void LogaCancelouRegistro() = 0;                                      // 25 "Operador cancelou o registro de mesários"
    virtual void LogaTituloInvalido() = 0;                                        // 26 "Digitado título inválido para o registro de mesário"
    virtual void LogaMesarioJaRegistrado(const std::string& titulo) = 0;          // 27 "Mesário {} já registrado"
    virtual void LogaLimiteMesariosAtingido() = 0;                                // 28 "Limite ... Limite máximo: {}" (6)
    virtual void LogaEncerrouRegistro() = 0;                                      // 29 "Operador encerrou ciclo de registro de mesários"
    virtual void LogaDigitalNaoCorresponde(const std::vector<int>& scores) = 0;   // 30 "Digital capturada não corresponde ..."
    virtual void LogaPedidoLeituraBiometria() = 0;                                // 31 "Pedido de leitura da biometria do mesário {}"
    virtual void LogaMesarioEhEleitor() = 0;                                      // 32 "Mesário {} é eleitor da seção"
    virtual void LogaMesarioNaoEhEleitor() = 0;                                   // 33 "Mesário {} não é eleitor da seção"
    virtual void LogaConferenciaBiometria() = 0;                                  // 34 "Realizada a conferência da biometria do mesário"
    virtual void LogaIndagadoContinuarRegistro() = 0;                             // 35 "Operador indagado se continua registrando mesários"
    virtual void LogaIndagadoFinalizarRegistro() = 0;                             // 36 "Operador indagado se finaliza registro mesários"
};

} // namespace comum
