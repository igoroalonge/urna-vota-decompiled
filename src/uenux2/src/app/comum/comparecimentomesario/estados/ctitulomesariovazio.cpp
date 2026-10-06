// uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesariovazio.cpp
// Reconstructed from vota_web_wasm.wasm (unit u22). Attested by the std::source_location record :57
// (GetControlador, inlined).
#include "comum/comparecimentomesario/estados/estadosregistromesarios.h"

#include <mutex>

#include "api/gui/cformbuildermt.h"
#include "api/pattern/cpolysingletonlist.h"

namespace comum {

using api::SPoint;

// Constructor + GetInst: inlined into IPedeTituloMesario::ProcessInput (func 10313).
CTituloMesarioVazio::CTituloMesarioVazio()
    : CEstadoRegistroMesarioComForms(2)
{
    m_formEleitor = CriaFormEleitorRegistroMesarios();                               // func 1255
    api::CFormBuilderMT campos;
    campos.Add<api::CTextFieldMT>(SPoint{1, 1}, std::make_shared<api::CFixedText>(ESQUERDA, "Título inválido: número vazio"));
    campos.Add<api::CTextFieldMT>(SPoint{1, 4}, std::make_shared<api::CFixedText>(ESQUERDA, "CORRIGE: Retornar"));
    campos.AddInputControl();
    m_formMT = campos.CriaFormInterativo("", true);
}

CTituloMesarioVazio& CTituloMesarioVazio::GetInst()
{
    static std::mutex mutex;                                        // @1909628
    static std::unique_ptr<CTituloMesarioVazio> s_inst;             // @1909652
    std::lock_guard trava(mutex);
    if (!s_inst)
        s_inst.reset(new CTituloMesarioVazio());
    return *s_inst;
}

// wasm func 10353 - vtable slot 2. Body shared with CTituloMesarioInvalido::StartState (func 6010,
// srcloc passed as a parameter: :57 here).
void CTituloMesarioVazio::StartState()
{
    m_proximoEstado = this;
    m_formMT->Show();
    m_formEleitor->Show();
    api::CPolySingletonList::instance<IControladorRegistraMesarios>().LogaTituloInvalido();   // :57, slot 26
}

// wasm func 10352 - vtable slot 7. Shared body func 2895(this, 5 /*CORRIGE*/, srcloc).
void CTituloMesarioVazio::ProcessInput()
{
    if (m_formMT->Read() == api::EInputResult::CORRIGE)
        m_proximoEstado = &CPedeTituloMesario::GetInst();                             // func 2729
}

} // namespace comum
