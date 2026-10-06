// uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesarioinvalido.cpp
// Reconstructed from vota_web_wasm.wasm (unit u22). Attested by the std::source_location records :75
// (GetControlador) and :28 (CTituloMesarioInvalidoDS::Text).
#include "comum/comparecimentomesario/estados/estadosregistromesarios.h"

#include <format>
#include <mutex>

#include "api/gui/cformbuildermt.h"
#include "api/pattern/cpolysingletonlist.h"

namespace comum {

using api::SPoint;

namespace {
// wasm func 10349 (table slot 4338) - data source of the typed título. Body shared with
// CTituloMesarioJaRegistradoDS::Text (func 6009, srcloc passed as a parameter: :28 here).
struct CTituloMesarioInvalidoDS {
    static std::string Text(const std::string& formato)
    {
        const std::string titulo = api::CPolySingletonList::instance<IControladorRegistraMesarios>()   // :28
                                       .GetTituloMesario();                                            // slot 17
        return std::vformat(formato, std::make_format_args(titulo));
    }
};
} // namespace

// Constructor + GetInst: inlined into IPedeTituloMesario::ProcessInput (func 10313).
CTituloMesarioInvalido::CTituloMesarioInvalido()
    : CEstadoRegistroMesarioComForms(2)
{
    m_formEleitor = CriaFormEleitorRegistroMesarios();
    api::CFormBuilderMT campos;
    campos.Add<api::CTextFieldMT>(SPoint{1, 1}, std::make_shared<api::CFixedText>(ESQUERDA, "Título inválido:"));
    campos.Add<api::CTextFieldMT>(SPoint{18, 1}, std::make_shared<api::CDataTextFmt<std::string (*)(const std::string&)>>(
                                                     ESQUERDA, &CTituloMesarioInvalidoDS::Text, "{}"));   // func 651
    campos.Add<api::CTextFieldMT>(SPoint{1, 4}, std::make_shared<api::CFixedText>(ESQUERDA, "CORRIGE: Retornar"));
    campos.AddInputControl();
    m_formMT = campos.CriaFormInterativo("", true);
}

CTituloMesarioInvalido& CTituloMesarioInvalido::GetInst()
{
    static std::mutex mutex;                                        // @1909656
    static std::unique_ptr<CTituloMesarioInvalido> s_inst;          // @1909680
    std::lock_guard trava(mutex);
    if (!s_inst)
        s_inst.reset(new CTituloMesarioInvalido());
    return *s_inst;
}

// wasm func 10348 - vtable slot 2 (tools: "GetControlador"; body func 6010 with srcloc :75)
void CTituloMesarioInvalido::StartState()
{
    m_proximoEstado = this;
    m_formMT->Show();
    m_formEleitor->Show();
    api::CPolySingletonList::instance<IControladorRegistraMesarios>().LogaTituloInvalido();   // :75, slot 26
}

// wasm func 10347 - vtable slot 7 (body func 2895 with CORRIGE)
void CTituloMesarioInvalido::ProcessInput()
{
    if (m_formMT->Read() == api::EInputResult::CORRIGE)
        m_proximoEstado = &CPedeTituloMesario::GetInst();
}

} // namespace comum
