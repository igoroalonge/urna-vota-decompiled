// Reconstructed from vota_web_wasm.wasm (unit u27). Original: uenux2/src/app/vota/operador/outrasopcoes/cencerramentohorarioinvalido.cpp
// (attested: srclocs cencerramentohorarioinvalido.cpp:60 and :73, both in StartState).
//
// Functions: 10710 StartState (slot 2), 10709 ProcessInput (slot 7), 2295 shared "leave on key" body.
// The constructor + GetInst are inlined into CIniciaFinalizacao::StartState (func 10706,
// u27-foreign-fragments.cpp) and reproduced here.
//
// WEB BUILD: dead code (operator thread not run). Harness run (u27 doc §2): "Encerrar votação" at 16:50
// showed this screen and logged "Encerramento só pode ser solicitado após as 17:00:00 horas"; the
// clock jump of StartState then threw 8071 (CBaseError<comum::EUeComumDadosError>) "DeltaT pode ser
// adicionado apenas para eAlterarDataSistema 0" from CAjusteDataHora::AdicionaDeltaT (func 3702), after
// the clock had already been moved: back on CPedeIdentidade / CEscolheOpcao the MT clock read 16:59
// (= 17:00:00 - 10 s), see the doc (§6).
#include "vota/operador/outrasopcoes/cencerramentohorarioinvalido.h"

#include "api/gui/cformbuildermt.h"
#include "api/hwil/iinput.h"
#include "api/util/cdatetime.h"
#include "api/util/iajustedatahora.h"
#include "comum/appinfo/cappinfo.h"
#include "comum/dados/cconfiguracaoeleicao.h"
#include "comum/iinterfaceinit.h"
#include "vota/operador/leidentidade/cpedeidentidade.h"

namespace vota {

using api::SPoint;

// ---------------------------------------------------------------------------------------------------
// Constructor (inlined into func 10706).
CEncerramentoHorarioInvalido::CEncerramentoHorarioInvalido()
    : comum::CAppState(2)
{
    api::CFormBuilderMT campos;
    campos.Add<api::CLedFieldMT>(false);
    campos.Add<api::CClockFieldMT>(SPoint{33, 1});
    campos.Add<api::CTextFieldMT>(SPoint{1, 2}, std::make_shared<api::CFixedText>(api::ETextAlignment(2), "Encerramento de votação inválido"));
    campos.Add<api::CTextFieldMT>(SPoint{1, 3}, std::make_shared<api::CFixedText>(api::ETextAlignment(2), "Antes do horário"));
    campos.Add<api::CTextFieldMT>(SPoint{1, 4}, std::make_shared<api::CFixedText>(api::ETextAlignment(0), "CORRIGE: retornar"));
    campos.AddInputControl();
    m_form = campos.CriaFormInterativo("", true);
}

CEncerramentoHorarioInvalido& CEncerramentoHorarioInvalido::GetInst()    // @1905224, mutex @1905200
{
    static std::mutex mutex;
    static std::unique_ptr<CEncerramentoHorarioInvalido> s_inst;
    std::lock_guard lock(mutex);
    if (!s_inst)
        s_inst.reset(new CEncerramentoHorarioInvalido());
    return *s_inst;
}

// ---------------------------------------------------------------------------------------------------
// wasm func 10710 (vtable slot 2, srclocs :60 and :73)
void CEncerramentoHorarioInvalido::StartState()
{
    m_form->Show();

    // Same test as vota::EhTreinamentoSemTreinamentoEleitor (func 1823), inlined:
    const bool treinamentoMesarios = comum::EhFaseTreinamento() && !comum::EhTreinamentoEleitor();
    if (comum::IInterfaceInit::GetInst().GetDemoMode() || treinamentoMesarios)      // :60 (funcs 611 + 729)
        AvancaRelogioParaEncerramento();

    m_proximoEstado = this;
}

// Inlined into 10710 (same pattern as CIniciodeCiclo::AjustaDataHora, u06).               name inferred
void CEncerramentoHorarioInvalido::AvancaRelogioParaEncerramento()
{
    auto& estadoGeral = comum::GetEstadoGeral(comum::CAppInfo::GetInst());           // func 291
    const auto& cfg = comum::CConfiguracaoEleicao::GetInst();

    const api::CDateTime agora;                                                      // func 479 (ISystemDateTime)
    const auto antes = agora.ToTimeT();                                              // func 1381 (timegm)

    // HorariosUrna.encerramentoVotacao (cfg +568: CDate +568, CTime +576) MINUS 10 seconds: the clock is
    // put just before the earliest closing time, so the trainee can retry "Encerrar votação" 10 s later.
    // Func 3641 (tools: "CTime::operator+=") copies the CTime and applies CTime::operator-=(int) to the
    // copy (underflow check, srcloc ctime.cpp:161; a negative argument would go through operator+=,
    // ctime.cpp:148): with 10 it returns time - 10. The configuration itself is not modified.
    const api::CDateTime novo(cfg.GetDataHoraEncerramentoVotacao().GetDate(),
                              cfg.GetDataHoraEncerramentoVotacao().GetTime() - 10);  // CTime::operator- (func 3641)
    const auto depois = novo.ToTimeT();

    api::CPolySingletonList::instance<api::IAjusteDataHora>().AjustaDataHora(novo);  // :73 (func 3204), slot 0:
                                                                                     //   sets the clock FIRST
    auto ajuste = estadoGeral.GetAjusteDataHora();                                   // CEstadoGeral +52 (8 bytes)
    ajuste.AdicionaDeltaT(static_cast<std::uint32_t>(depois - antes));               // func 3702: throws 8071 unless
                                                                                     //   the mode is eAlterarDataSistema
    estadoGeral.SetAjusteDataHora(ajuste);                                           // func 3705
    comum::SalvaEstado();                                                            // func 491  "Gravando o estado da urna"
    comum::GravaEstadoGeral();                                                       // func 3333 eg.bin on MI/MV
}

// ---------------------------------------------------------------------------------------------------
// wasm func 10709 (vtable slot 7)
void CEncerramentoHorarioInvalido::ProcessInput()
{
    RetornaParaPedeIdentidadeSeTecla(*this, *m_form, api::EInputResult::CORRIGE);    // func 2295 (key 5)
}

// ---------------------------------------------------------------------------------------------------
// wasm func 2295 (shared body; m_form is at +12 in all five classes)                    name inferred
void RetornaParaPedeIdentidadeSeTecla(comum::CAppState& estado,
                                      api::CInteractiveForm<api::IScreenMT, api::IInputMT>& form,
                                      const api::EInputResult tecla)
{
    // CInteractiveForm::Read() inlined (cinteractiveform.h:57): m_campos.at(m_foco)->Read(IInputMT)
    if (form.Read() == tecla)
        estado.SetProximoEstado(&CPedeIdentidade::GetInst());                        // func 652
}

}  // namespace vota
