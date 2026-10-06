// uenux2/src/app/comum/comparecimentomesario/estados/climitemesariosregistradosatingido.cpp
// Reconstructed from vota_web_wasm.wasm (unit u22). Attested by the std::source_location record :53
// (GetControlador).
#include "comum/comparecimentomesario/estados/estadosregistromesarios.h"

#include <format>
#include <mutex>

#include "api/gui/cformbuildermt.h"
#include "api/pattern/cpolysingletonlist.h"

namespace comum {

using api::SPoint;

namespace {
IControladorRegistraMesarios& GetControlador()                                     // srcloc :53
{
    return api::CPolySingletonList::instance<IControladorRegistraMesarios>();
}
} // namespace

CLimiteMesariosRegistradosAtingido::CLimiteMesariosRegistradosAtingido()
    : CEstadoRegistroMesarioComForms(2)
{
    m_formEleitor = CriaFormEleitorRegistroMesarios();
    // The controller is looked up (:53) but only the constant 6 is used: the maximum is a
    // compile-time value in this build.                                                       ?
    (void)GetControlador();
    api::CFormBuilderMT campos;
    campos.Add<api::CTextFieldMT>(SPoint{1, 1}, std::make_shared<api::CFixedText>(ESQUERDA, "Limite de mesários registrados atingido"));
    campos.Add<api::CTextFieldMT>(SPoint{1, 2}, std::make_shared<api::CFixedText>(ESQUERDA,
                                                std::format("Quantidade máxima permitida: {}", QTD_MAXIMA_MESARIOS)));
    campos.Add<api::CTextFieldMT>(SPoint{40, 4}, std::make_shared<api::CFixedText>(DIREITA, "CONFIRMA: Finalizar"));
    campos.AddInputControl();
    m_formMT = campos.CriaFormInterativo("", true);
}

// wasm func 5386 (tools: "CLimiteMesariosRegistradosAtingido::GetControlador@5386")
CLimiteMesariosRegistradosAtingido& CLimiteMesariosRegistradosAtingido::GetInst()
{
    static std::mutex mutex;                                              // @1909600
    static std::unique_ptr<CLimiteMesariosRegistradosAtingido> s_inst;    // @1909624
    std::lock_guard trava(mutex);
    if (!s_inst)
        s_inst.reset(new CLimiteMesariosRegistradosAtingido());
    return *s_inst;
}

// wasm func 10358 - vtable slot 2
void CLimiteMesariosRegistradosAtingido::StartState()
{
    m_proximoEstado = this;
    m_formMT->Show();
    m_formEleitor->Show();
    GetControlador().LogaLimiteMesariosAtingido();                                  // slot 28
}

// wasm func 10357 - vtable slot 7
void CLimiteMesariosRegistradosAtingido::ProcessInput()
{
    if (m_formMT->Read() == api::EInputResult::CONFIRMA)
        m_proximoEstado = &CEncerraRegistroMesarios::GetInst();                     // func 5388
}

} // namespace comum
