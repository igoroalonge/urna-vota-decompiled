// uenux2/src/app/comum/comparecimentomesario/estados/cconfirmafimregistromesarios.cpp
// Reconstructed from vota_web_wasm.wasm (unit u22). Attested by the std::source_location record :60
// (GetControlador).
#include "comum/comparecimentomesario/estados/estadosregistromesarios.h"

#include <mutex>

#include "api/gui/cformbuildermt.h"
#include "api/pattern/cpolysingletonlist.h"

namespace comum {

using api::SPoint;

namespace {
IControladorRegistraMesarios& GetControlador()                                     // srcloc :60
{
    return api::CPolySingletonList::instance<IControladorRegistraMesarios>();
}
} // namespace

CConfirmaFimRegistroMesarios::CConfirmaFimRegistroMesarios()
    : CEstadoRegistroMesarioComForms(2)
{
    m_formEleitor = CriaFormEleitorRegistroMesarios();
    api::CFormBuilderMT campos;
    campos.Add<api::CBuzzFieldMT>(51, 10);
    campos.Add<api::CTextFieldMT>(SPoint{1, 1}, std::make_shared<api::CFixedText>(ESQUERDA, "Finalizar registro de mesários?"));
    campos.Add<api::CTextFieldMT>(SPoint{1, 4}, std::make_shared<api::CFixedText>(ESQUERDA,
                                                "CORRIGE: Voltar  CONFIRMA: Finalizar"));
    campos.AddInputControl();
    m_formMT = campos.CriaFormInterativo("", true);
}

// wasm func 5389 (tools: comum_f5389)
CConfirmaFimRegistroMesarios& CConfirmaFimRegistroMesarios::GetInst()
{
    static std::mutex mutex;                                        // @1909460
    static std::unique_ptr<CConfirmaFimRegistroMesarios> s_inst;    // @1909484
    std::lock_guard trava(mutex);
    if (!s_inst)
        s_inst.reset(new CConfirmaFimRegistroMesarios());
    return *s_inst;
}

// wasm func 10385 - vtable slot 2
void CConfirmaFimRegistroMesarios::StartState()
{
    m_proximoEstado = this;
    GetControlador().LogaIndagadoFinalizarRegistro();                             // slot 36
    m_formEleitor->Show();
    m_formMT->Show();
}

// wasm func 10383 - vtable slot 7
void CConfirmaFimRegistroMesarios::ProcessInput()
{
    switch (m_formMT->Read()) {                                                    // cinteractiveform.h:57
    case api::EInputResult::CORRIGE:                                               // "Voltar": keep registering
        GetControlador().LogaConfirmouRegistro();                                  // slot 24 ("Operador confirmou o registro de mesários")
        m_proximoEstado = &CPedeTituloMesario::GetInst();
        break;
    case api::EInputResult::CONFIRMA:                                              // "Finalizar"
        m_proximoEstado = &CEncerraRegistroMesarios::GetInst();                    // func 5388
        break;
    default:
        break;
    }
}

} // namespace comum
