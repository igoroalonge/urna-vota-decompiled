// uenux2/src/app/comum/comparecimentomesario/estados/cmesarioregistrado.cpp
// Reconstructed from vota_web_wasm.wasm (unit u22). Attested by the std::source_location record :72
// (GetControlador).
#include "comum/comparecimentomesario/estados/estadosregistromesarios.h"

#include <mutex>

#include "api/gui/cformbuildermt.h"
#include "api/pattern/cpolysingletonlist.h"

namespace comum {

using api::SPoint;

namespace {
IControladorRegistraMesarios& GetControlador()                                     // srcloc :72
{
    return api::CPolySingletonList::instance<IControladorRegistraMesarios>();
}
} // namespace

// Constructor + GetInst: inlined into IGestorDadoMesario::StartState (func 10337).
CMesarioRegistrado::CMesarioRegistrado()
    : CEstadoRegistroMesarioComForms(2)
{
    m_formEleitor = CriaFormEleitorRegistroMesarios();                               // func 1255
    api::CFormBuilderMT campos;
    campos.Add<api::CBuzzFieldMT>(51, 10);
    campos.Add<api::CTextFieldMT>(SPoint{1, 1}, std::make_shared<api::CFixedText>(ESQUERDA, "Mesário registrado com sucesso."));
    campos.Add<api::CTextFieldMT>(SPoint{1, 2}, std::make_shared<api::CFixedText>(ESQUERDA, "Continuar registrando mesários?"));
    campos.Add<api::CTextFieldMT>(SPoint{1, 4}, std::make_shared<api::CFixedText>(ESQUERDA,
                                                "CORRIGE: Encerrar  CONFIRMA: Prosseguir"));
    campos.AddInputControl();
    m_formMT = campos.CriaFormInterativo("", true);
}

CMesarioRegistrado& CMesarioRegistrado::GetInst()
{
    static std::mutex mutex;                                        // @1909712
    static std::unique_ptr<CMesarioRegistrado> s_inst;              // @1909736
    std::lock_guard trava(mutex);
    if (!s_inst)
        s_inst.reset(new CMesarioRegistrado());
    return *s_inst;
}

// wasm func 10339 - vtable slot 2 (tools: "GetControlador@10339")
void CMesarioRegistrado::StartState()
{
    m_proximoEstado = this;
    GetControlador().LogaMesarioRegistrado(GetControlador().GetTituloMesario());   // slots 19, 17
    if (GetControlador().LimiteMesariosAtingido()) {                              // slot 7
        m_proximoEstado = &CLimiteMesariosRegistradosAtingido::GetInst();         // func 5386
        return;
    }
    GetControlador().LogaIndagadoContinuarRegistro();                             // slot 35
    m_formEleitor->Show();
    m_formMT->Show();
}

// wasm func 10338 - vtable slot 7: same shared body as CRegistrarMesarios::ProcessInput (func 6011,
// with srcloc :72): CONFIRMA -> LogaConfirmouRegistro + CPedeTituloMesario;
// CORRIGE -> LogaCancelouRegistro + CConfirmaFimRegistroMesarios.
void CMesarioRegistrado::ProcessInput()
{
    switch (m_formMT->Read()) {
    case api::EInputResult::CONFIRMA:
        GetControlador().LogaConfirmouRegistro();                                 // slot 24
        m_proximoEstado = &CPedeTituloMesario::GetInst();
        break;
    case api::EInputResult::CORRIGE:
        GetControlador().LogaCancelouRegistro();                                  // slot 25
        m_proximoEstado = &CConfirmaFimRegistroMesarios::GetInst();
        break;
    default:
        break;
    }
}

} // namespace comum
