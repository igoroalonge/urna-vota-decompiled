// Reconstructed from vota_web_wasm.wasm (unit u06). Original: uenux2/src/app/vota/eleitor/cdefinerotaposreinicio.cpp
//
// CDefineRotaPosReinicio ("define the route after a restart") decides, when the application
// restarts in the middle of the election day, whether it must go through the mesário attendance
// registration again or straight to the "aguardando início da votação" screen.
//
// RTTI: comum::CAppState <- vota::CDefineRotaPosReinicio (typeinfo @1546184, vtable @1546132)
//   [2] StartState = func 3243 (shared tiny body: m_proximoEstado = this)   [3] NeedChangeState = 11858
//   [7] ProcessInput = func 5974 (shared tiny body)
// Singleton accessor: func 5943 (not in this unit), used by CReinicioVotacao / CReimprimindoResumoZeresima.
//
// srcloc: :81 virtual bool vota::CDefineRotaPosReinicio::NeedChangeState()
//
// EstadoVota values: the C++ enum is the ASN.1 EstadoGeralVota.estadoVota value + 49
// (54 = zeresimaimpressa, 55 = registromesarioinicial, 56 = votar).

#include "comum/cappstate.h"

#include <format>

#include "comum/cappinfo.h"
#include "vota/log/clogvota.h"

namespace vota {

class CDefineRotaPosReinicio final : public comum::CAppState {
public:
    bool NeedChangeState() override;
private:
    int m_campo12;          // +12, set to 0 before routing (meaning unknown) ?
};

// wasm func 11858 — vtable slot 3, srcloc :81. Always returns true (it is a pure routing state).
bool CDefineRotaPosReinicio::NeedChangeState()
{
    const auto estado = comum::GetEstadoVota(comum::CAppInfo::GetInst()).estadoVota;   // GetEstado<CEstadoGeralVota>(turno atual)

    switch (estado) {
    case EEstadoVota::ZeresimaImpressa:          // 54
    case EEstadoVota::RegistroMesarioInicial:    // 55
        m_campo12 = 0;
        // func 2520 = EhModoDemonstracao() && !(fase == treinamento && treinamentoEleitor):
        // a demonstration urna (unless it is a training-phase urna in voter training) registers the
        // mesários again; a non-demo urna never does (name inferred)
        m_proximoEstado = EhDemonstracaoTreinamentoSemTreinamentoEleitor()      // func 2520 name inferred
                              ? static_cast<CAppState*>(&CReinicioComparecimentoMesario::GetInst())  // func 3864
                              : &CInicioVotacao::GetInst();                                          // func 2880
        break;
    case EEstadoVota::Votar:                     // 56
        m_campo12 = 0;
        m_proximoEstado = &CInicioVotacao::GetInst();     // "Aguarde o horário de início da votação"
        break;
    default:
        CLogVota::GetInst().LogaErro("Erro estado do aplicativo não conhecido");   // func 3279
        // enum passed as is (format arg type 15 = handle: user std::formatter, func 536 prints the
        // unsigned value)
        throw CUeVotaError(9309, std::format("Estado inválido = {}", estado),
                           std::source_location::current());
    }
    return true;
}

}  // namespace vota
