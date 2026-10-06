// uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesariojaregistrado.cpp
// Reconstructed from vota_web_wasm.wasm (unit u22). Attested by the std::source_location records :53
// (StartState), :76 (GetControlador) and :29 (CTituloMesarioJaRegistradoDS::Text).
#include "comum/comparecimentomesario/estados/estadosregistromesarios.h"

#include <format>
#include <mutex>

#include "api/gui/cformbuildermt.h"
#include "api/pattern/cpolysingletonlist.h"

namespace comum {

using api::SPoint;

namespace {
// wasm func 10344 (table slot 4345) - body func 6009 with srcloc :29.
struct CTituloMesarioJaRegistradoDS {
    static std::string Text(const std::string& formato)
    {
        const std::string titulo = api::CPolySingletonList::instance<IControladorRegistraMesarios>()   // :29
                                       .GetTituloMesario();
        return std::vformat(formato, std::make_format_args(titulo));
    }
};
} // namespace

// Constructor + GetInst: inlined into IPedeTituloMesario::ProcessInput (func 10313).
CTituloMesarioJaRegistrado::CTituloMesarioJaRegistrado()
    : CEstadoRegistroMesarioComForms(2)
{
    m_formEleitor = CriaFormEleitorRegistroMesarios();
    api::CFormBuilderMT campos;
    campos.Add<api::CTextFieldMT>(SPoint{1, 1}, std::make_shared<api::CFixedText>(ESQUERDA, "Título:"));
    campos.Add<api::CTextFieldMT>(SPoint{9, 1}, std::make_shared<api::CDataTextFmt<std::string (*)(const std::string&)>>(
                                                    ESQUERDA, &CTituloMesarioJaRegistradoDS::Text, "{}"));
    campos.Add<api::CTextFieldMT>(SPoint{1, 2}, std::make_shared<api::CFixedText>(ESQUERDA, "Já registrado"));
    campos.Add<api::CTextFieldMT>(SPoint{1, 4}, std::make_shared<api::CFixedText>(ESQUERDA, "CORRIGE: Retornar"));
    campos.AddInputControl();
    m_formMT = campos.CriaFormInterativo("", true);
}

CTituloMesarioJaRegistrado& CTituloMesarioJaRegistrado::GetInst()
{
    static std::mutex mutex;                                        // @1909684
    static std::unique_ptr<CTituloMesarioJaRegistrado> s_inst;      // @1909708
    std::lock_guard trava(mutex);
    if (!s_inst)
        s_inst.reset(new CTituloMesarioJaRegistrado());
    return *s_inst;
}

// wasm func 10343 - vtable slot 2 (srcloc :53, :76)
void CTituloMesarioJaRegistrado::StartState()
{
    m_proximoEstado = this;
    auto& controlador = api::CPolySingletonList::instance<IControladorRegistraMesarios>();   // :53
    m_formMT->Show();
    m_formEleitor->Show();
    api::CPolySingletonList::instance<IControladorRegistraMesarios>()                       // :76
        .LogaMesarioJaRegistrado(controlador.GetTituloMesario());                            // slots 27, 17
}

// wasm func 10342 - vtable slot 7 (body func 2895 with CORRIGE)
void CTituloMesarioJaRegistrado::ProcessInput()
{
    if (m_formMT->Read() == api::EInputResult::CORRIGE)
        m_proximoEstado = &CPedeTituloMesario::GetInst();
}

} // namespace comum
