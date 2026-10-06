// Reconstructed from vota_web_wasm.wasm (unit u06). Original: uenux2/src/app/vota/eleitor/ciniciodeciclo.cpp
//
// CIniciodeCiclo ("start of cycle") runs when the operator starts the voting period (operator
// message handled by CAguardaMensagem -> func 4433 = CIniciodeCiclo::GetInst). If the vote has not
// started yet it (a) in demonstration / training mode moves the urna clock to the configured start
// of voting, (b) marks EstadoGeralVota.estadoVota = votar, and then goes to CInicioVotacao.
//
// RTTI: comum::CAppState <- vota::CIniciodeCiclo (typeinfo @1533500, vtable @1533432)
//   [2] StartState = func 7306 (the analyzer named it after the inlined AjustaDataHora)
//
// srcloc: :48 / :61  void vota::CIniciodeCiclo::AjustaDataHora()  (IInterfaceInit / IAjusteDataHora lookups)

#include "comum/cappstate.h"

#include "api/util/cdatetime.h"
#include "comum/cappinfo.h"
#include "comum/dados/cconfiguracaoeleicao.h"
#include "comum/iinterfaceinit.h"

namespace vota {

class CIniciodeCiclo final : public comum::CAppState {
public:
    void StartState() override;
private:
    void AjustaDataHora();
};

// wasm func 7306 — vtable slot 2
void CIniciodeCiclo::StartState()
{
    auto& app = comum::CAppInfo::GetInst();                                 // func 185
    if (comum::GetEstadoVota(app).estadoVota <= EEstadoVota::RegistroMesarioInicial) {  // <= 55
        AjustaDataHora();
        comum::GetEstadoVota(app).estadoVota = EEstadoVota::Votar;           // 56
        comum::SalvaEstado();                                                // func 491 "Gravando o estado da urna"
    }
    m_proximoEstado = &CInicioVotacao::GetInst();                           // func 2880
}

// inlined into func 7306 (srcloc :48, :61)
void CIniciodeCiclo::AjustaDataHora()
{
    auto& app = comum::CAppInfo::GetInst();
    const bool demo = comum::IInterfaceInit::GetInst().GetDemoMode();       // :48
    if (!demo && !EhTreinamentoSemTreinamentoEleitor())                     // func 1823:
        return;                                                             //  fase == treinamento && !treinamentoEleitor

    auto& estadoGeral = comum::GetEstadoGeral(app);                         // CEstadoGeral (eg.bin)
    const auto& cfg = comum::CConfiguracaoEleicao::GetInst();

    const api::CDateTime agora;                                             // func 479 (system clock)
    const auto antes = agora.ToTimeT();                                     // func 1381 (timegm)

    // new clock = configured start of voting (cfg +556 date, +564 time) plus 10 s   ?field names
    api::CDateTime novo(cfg.GetDataHoraInicioVotacao().GetDate(),
                        cfg.GetDataHoraInicioVotacao().GetTime() + 10);   // CTime::operator+ (func 3641)
    const auto depois = novo.ToTimeT();

    auto ajuste = estadoGeral.GetAjusteDataHora();                          // CEstadoGeral +52
    ajuste.AdicionaDeltaT(static_cast<int>(depois - antes));
    estadoGeral.SetAjusteDataHora(ajuste);                                  // func 3705

    api::IAjusteDataHora::GetInst().AjustaDataHora(novo);                   // :61  slot 0 (sets the clock)
    comum::GravaEstadoGeral();                                              // func 3333 "Gravando o estado geral da urna na MV/MI"
}

}  // namespace vota
