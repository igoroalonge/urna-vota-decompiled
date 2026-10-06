// uenux2/src/app/comum/comparecimentomesario/estados/estadosregistromesarios.h  (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u22).
//
// Declarations of the mesário-registration states, collected in one header for brevity (the original
// has one .h per class, e.g. cimprimiridentificacaomesariosfinal.h is attested by a srcloc at line 41).
// All are comum::CAppState singletons run by the OPERATOR thread (micro-terminal, "MT"); each 28-byte
// state owns two forms:
//   +12 std::shared_ptr<TFormMT>       m_formMT       (mesário terminal, 4 lines x 40 columns)
//   +20 std::shared_ptr<TFormEleitor>  m_formEleitor  (voter screen: "REGISTRO DE MESÁRIOS / Siga as
//                                                      instruções no terminal do mesário.", func 1255)
// CAppState slots: 2 StartState, 3 NeedChangeState, 4 GetNextState, 5 FinishState, 6 ProcessMessage,
// 7 ProcessInput, 8 ProcessTick (see u06). Constructors are inlined into the lazy GetInst accessors
// (static std::unique_ptr + mutex; exit-time resets are listed per class).
#pragma once

#include <memory>
#include <string>

#include "api/gui/cinteractiveform.h"
#include "comum/cappstate.h"
#include "comum/comparecimentomesario/icontroladorregistramesarios.h"
#include "comum/comparecimentomesario/md/ccomparecimentomesario.h"

namespace comum {

using TFormMT      = api::CInteractiveForm<api::IScreenMT, api::IInputMT>;
using TFormEleitor = api::CInteractiveForm<api::IScreen, api::IInputKbd>;

// Voter-screen form shared by every state (func 1255, file inferred: cregistrarmesarios.cpp).
std::shared_ptr<TFormEleitor> CriaFormEleitorRegistroMesarios();                          // name inferred

// Base of the simple "two forms" states                                                   // name inferred
class CEstadoRegistroMesarioComForms : public CAppState {
protected:
    explicit CEstadoRegistroMesarioComForms(uebyte flags) : CAppState(flags) {}
    std::shared_ptr<TFormMT>      m_formMT;        // +12
    std::shared_ptr<TFormEleitor> m_formEleitor;   // +20
};
// (the destructors of all these 28-byte states are the ICF bodies 448 / 765)

// "Registrar mesário?" - entry point, called by VOTA (CAguardaInicio, CEscolheOpcao, CFimAquisicaoVotos).
class CRegistrarMesarios final : public CEstadoRegistroMesarioComForms {                 // vtable @1593592
public:
    static CRegistrarMesarios& GetInst();          // func 3610 (@1909428)
    void StartState() override;                    // func 10392
    void ProcessInput() override;                  // func 10391 (body func 6011)
private:
    CRegistrarMesarios();
};

// Dispatcher: picks the title screen of the current period.                              vtable @1593728
class CPedeTituloMesario final : public CAppState {
public:
    static CPedeTituloMesario& GetInst();          // func 2729 (= vota_f764(@1909432, @1909456, vtable, 0))
    void StartState() override;                    // func 10388
private:
    CPedeTituloMesario() : CAppState(0) {}
};

// "Informe o título do mesário:" (abstract; slot 9 = period stored in the row)            vtable @1595584
class IPedeTituloMesario : public CAppState {
public:
    IPedeTituloMesario(std::shared_ptr<TFormMT> formMT, std::shared_ptr<TFormEleitor> formEleitor);  // func 3608
    ~IPedeTituloMesario() override;                // func 1378 (slot 0 of all four vtables)
    void StartState() override;                    // func 10314
    void ProcessInput() override;                  // func 10313
    virtual md::EPeriodoPresente GetPeriodoPresente() const = 0;   // slot 9
protected:
    std::shared_ptr<TFormMT>      m_formMT;        // +12
    std::shared_ptr<TFormEleitor> m_formEleitor;   // +20
};
class CPedeTituloMesarioInicial final : public IPedeTituloMesario {   // vtable @1593960, @1909540 (exit: 10379)
public:
    static CPedeTituloMesarioInicial& GetInst();   // inlined into 10388
    CPedeTituloMesarioInicial();                   // srcloc cpedetitulomesarioinicial.cpp:44
    md::EPeriodoPresente GetPeriodoPresente() const override { return md::EPeriodoPresente::ABERTURA; }   // 434
};
class CPedeTituloMesarioVotacao final : public IPedeTituloMesario {   // vtable @1594264, @1909596 (exit: 10366)
public:
    static CPedeTituloMesarioVotacao& GetInst();
    CPedeTituloMesarioVotacao();                   // srcloc cpedetitulomesariovotacao.cpp:44
    md::EPeriodoPresente GetPeriodoPresente() const override { return md::EPeriodoPresente::ABERTURA; }   // 434
};
class CPedeTituloMesarioFinal final : public IPedeTituloMesario {     // vtable @1594112, @1909568 (exit: 10372)
public:
    static CPedeTituloMesarioFinal& GetInst();
    CPedeTituloMesarioFinal();                     // srcloc cpedetitulomesariofinal.cpp:45
    md::EPeriodoPresente GetPeriodoPresente() const override { return md::EPeriodoPresente::ENCERRAMENTO; } // 2254
};

// Error screens after the title was typed (all return to CPedeTituloMesario with CORRIGE).
class CTituloMesarioVazio final : public CEstadoRegistroMesarioComForms {                // vtable @1594568, @1909652
public:
    static CTituloMesarioVazio& GetInst();         // inlined into 10313
    void StartState() override;                    // func 10353 (body func 6010)
    void ProcessInput() override;                  // func 10352 (body func 2895)
private:
    CTituloMesarioVazio();
};
class CTituloMesarioInvalido final : public CEstadoRegistroMesarioComForms {             // vtable @1594656, @1909680
public:
    static CTituloMesarioInvalido& GetInst();
    void StartState() override;                    // func 10348 (body func 6010)
    void ProcessInput() override;                  // func 10347 (body func 2895)
private:
    CTituloMesarioInvalido();
};
class CTituloMesarioJaRegistrado final : public CEstadoRegistroMesarioComForms {         // vtable @1594760, @1909708
public:
    static CTituloMesarioJaRegistrado& GetInst();
    void StartState() override;                    // func 10343
    void ProcessInput() override;                  // func 10342 (body func 2895)
private:
    CTituloMesarioJaRegistrado();
};

// Fingerprint capture of the mesário (only when the election uses biometrics and the urna is not
// abroad).                                                                   vtable @1595336, @1909904 (32 bytes)
class CPedeDigitalMesario final : public CAppState {
public:
    static CPedeDigitalMesario& GetInst();         // inlined into 10313
    void StartState() override;                    // func 10318
    void FinishState() override;                   // func 10315 (body func 6012)
    void ProcessInput() override;                  // func 10317
    void ProcessTick(uebyte tick) override;        // func 10316
private:
    CPedeDigitalMesario();
    bool ExecutaBatimentoDigitalMesario(const api::TSharedImageVector& digital,
                                        const long largura, const long altura);   // srcloc :262 (inlined)
    void SalvaDigitalMesario(const api::TSharedImageVector& digital);              // func 5382  name inferred
    uebyte m_tickTempoEsgotado;                    // +11 (15 000 ms)
    uebyte m_tickLeitura;                          // +12 (50 ms)
    std::shared_ptr<TFormMT> m_formCaptura;        // +16
    std::shared_ptr<TFormMT> m_formAguarde;        // +24
};
class CDigitalMesarioNaoReconhecida final : public CAppState {                           // vtable @1595248, @1909876 (20 bytes)
public:
    static CDigitalMesarioNaoReconhecida& GetInst();   // inlined into 10316
    void StartState() override;                    // ICF func 1070
    void ProcessInput() override;                  // func 10322 (body func 2895)
private:
    CDigitalMesarioNaoReconhecida();
    std::shared_ptr<TFormMT> m_form;               // +12
};

// Saves the row. Dispatcher + one subclass per period (slot 9 = period stored).
class CGestorDadoMesario final : public CAppState {                                     // vtable @1595176, @1909848
public:
    static CGestorDadoMesario& GetInst();          // func 5383 (= vota_f764(@1909824, @1909848, vtable, 0))
    void StartState() override;                    // func 10327
private:
    CGestorDadoMesario() : CAppState(0) {}
};
class IGestorDadoMesario : public CAppState {                                            // typeinfo @1594976
public:
    void StartState() override;                    // func 10337 (slot 2 of the three subclasses)
    virtual md::EPeriodoPresente GetPeriodoPresente() const = 0;   // slot 9
protected:
    IGestorDadoMesario() : CAppState(0) {}
};
class CGestorDadoMesarioInicial final : public IGestorDadoMesario {   // vtable @1594996, @1909764 (12 bytes)
public:
    static CGestorDadoMesarioInicial& GetInst();   // inlined into 10327
    md::EPeriodoPresente GetPeriodoPresente() const override { return md::EPeriodoPresente::ABERTURA; }
};
class CGestorDadoMesarioVotacao final : public IGestorDadoMesario {   // vtable @1595056, @1909792
public:
    static CGestorDadoMesarioVotacao& GetInst();
    md::EPeriodoPresente GetPeriodoPresente() const override { return md::EPeriodoPresente::ABERTURA; }
};
class CGestorDadoMesarioFinal final : public IGestorDadoMesario {     // vtable @1595116, @1909820
public:
    static CGestorDadoMesarioFinal& GetInst();
    md::EPeriodoPresente GetPeriodoPresente() const override { return md::EPeriodoPresente::ENCERRAMENTO; }
};

// "Mesário registrado com sucesso. Continuar registrando mesários?"                     vtable @1594880, @1909736
class CMesarioRegistrado final : public CEstadoRegistroMesarioComForms {
public:
    static CMesarioRegistrado& GetInst();          // inlined into 10337
    void StartState() override;                    // func 10339
    void ProcessInput() override;                  // func 10338 (body func 6011)
private:
    CMesarioRegistrado();
};

// "Limite de mesários registrados atingido"                                              vtable @1594416, @1909624
class CLimiteMesariosRegistradosAtingido final : public CEstadoRegistroMesarioComForms {
public:
    static CLimiteMesariosRegistradosAtingido& GetInst();   // func 5386
    void StartState() override;                    // func 10358
    void ProcessInput() override;                  // func 10357
private:
    CLimiteMesariosRegistradosAtingido();
};

// "Finalizar registro de mesários?"                                                      vtable @1593800, @1909484
class CConfirmaFimRegistroMesarios final : public CEstadoRegistroMesarioComForms {
public:
    static CConfirmaFimRegistroMesarios& GetInst();   // func 5389
    void StartState() override;                    // func 10385
    void ProcessInput() override;                  // func 10383
private:
    CConfirmaFimRegistroMesarios();
};

// End of the cycle: hands control back to VOTA.                                          vtable @1593888, @1909512
class CEncerraRegistroMesarios final : public CEstadoRegistroMesarioComForms {
public:
    static CEncerraRegistroMesarios& GetInst();    // func 5388 (forms stay empty)
    void StartState() override;                    // func 10380
private:
    CEncerraRegistroMesarios() : CEstadoRegistroMesarioComForms(0) {}
};

// Final printout of the mesários' identification (BIM report helper used by VOTA's CGeraRelatorios).
class CImprimirIdentificacaoMesariosFinal {
public:
    static std::string GetNomeMesario(const std::string& titulo);   // func 5385 (srcloc :229)
    api::IPaper& GetPrinter() const;                                // srcloc cimprimiridentificacaomesariosfinal.h:41 (func 12105)
};

} // namespace comum
