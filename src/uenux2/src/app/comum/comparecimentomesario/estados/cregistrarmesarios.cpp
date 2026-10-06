// uenux2/src/app/comum/comparecimentomesario/estados/cregistrarmesarios.cpp
// Reconstructed from vota_web_wasm.wasm (unit u22). Attested by the std::source_location record :68
// (GetControlador, inlined).
//
// MT form vocabulary (see vota/operador/confirmaidentidade/cpededigital.cpp of u10): SPoint{coluna,
// linha} 1-based on the 4x40 MT display; alignment 0 left, 1 right (x = right edge), 2 centred.
//   func 180  Add<CTextFieldMT>(pos, CFixedText(align, texto))    func 941  Add<CBuzzFieldMT>(a, b)
//   func 395  AddInputControl()                                    func 301  CriaFormInterativo("", true)
#include "comum/comparecimentomesario/estados/estadosregistromesarios.h"

#include "api/gui/cformbuilder.h"
#include "api/gui/cformbuildermt.h"
#include "api/pattern/cpolysingletonlist.h"

namespace comum {

using api::SPoint;

namespace {
// srcloc :68 - every call site re-fetches the controller (the lookup is not cached).
IControladorRegistraMesarios& GetControlador()
{
    return api::CPolySingletonList::instance<IControladorRegistraMesarios>();     // func 356
}
} // namespace

// wasm func 1255 (tools: comum_f1255; called by every state constructor). File inferred.  name inferred
// What the VOTER screen shows while the mesários are being registered at the MT.
std::shared_ptr<TFormEleitor> CriaFormEleitorRegistroMesarios()
{
    api::CFormBuilder b;
    b.AddStatusHeader(7);                                                           // func 502
    b.AddLabel("REGISTRO DE MESÁRIOS", SPoint{320, 150}, FONTE_TITULO /*@540572*/, 2, 2, 1);                       // func 202
    b.AddLabel("Siga as instruções no terminal do mesário.", SPoint{320, 250}, FONTE_TEXTO /*@540580*/, 2, 2, 1);
    return b.CriaFormInterativo("", std::make_shared<api::CPreShowClearScreen>());   // func 11140 -> 6117
}

// Constructor - inlined into CRegistrarMesarios::GetInst (func 3610, listed in another unit).
CRegistrarMesarios::CRegistrarMesarios()
    : CEstadoRegistroMesarioComForms(2)                                            // keys
{
    m_formEleitor = CriaFormEleitorRegistroMesarios();
    api::CFormBuilderMT campos;
    campos.Add<api::CBuzzFieldMT>(51, 10);                                          // func 941
    campos.Add<api::CTextFieldMT>(SPoint{1, 1}, std::make_shared<api::CFixedText>(ESQUERDA, "Registrar mesário?"));
    campos.Add<api::CTextFieldMT>(SPoint{1, 4}, std::make_shared<api::CFixedText>(ESQUERDA,
                                                "CORRIGE: Cancelar  CONFIRMA: Prosseguir"));
    campos.AddInputControl();                                                       // func 395
    m_formMT = campos.CriaFormInterativo("", true);                                 // func 301
}

// wasm func 10392 - vtable slot 2 (the tools named it GetControlador after the inlined srcloc :68)
void CRegistrarMesarios::StartState()
{
    m_proximoEstado = this;
    GetControlador().LogaIndagadoRegistro();                                        // slot 23
    GetControlador().AtualizaEstadoRegistro();                                      // slot 5
    if (GetControlador().LimiteMesariosAtingido()) {                                // slot 7
        m_proximoEstado = &CLimiteMesariosRegistradosAtingido::GetInst();           // func 5386
        return;
    }
    m_formEleitor->Show();
    m_formMT->Show();
}

// wasm func 10391 - vtable slot 7. Body shared with CMesarioRegistrado (func 6011, which receives the
// two srclocs as parameters: :68 and cinteractiveform.h:57).
void CRegistrarMesarios::ProcessInput()
{
    switch (m_formMT->Read()) {                                                     // cinteractiveform.h:57 inlined
    case api::EInputResult::CONFIRMA:
        GetControlador().LogaConfirmouRegistro();                                   // slot 24
        m_proximoEstado = &CPedeTituloMesario::GetInst();                           // func 2729
        break;
    case api::EInputResult::CORRIGE:
        GetControlador().LogaCancelouRegistro();                                    // slot 25
        m_proximoEstado = &CConfirmaFimRegistroMesarios::GetInst();                 // func 5389
        break;
    default:
        break;
    }
}

} // namespace comum
