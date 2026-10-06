// uenux2/src/app/comum/comparecimentomesario/estados/cdigitalmesarionaoreconhecida.cpp
// Reconstructed from vota_web_wasm.wasm (unit u22). Attested by the std::source_location record :27
// (CNomeMesariosUrnaDS::Text).
// Shown when no finger was presented within 15 s (CPedeDigitalMesario::ProcessTick).
#include "comum/comparecimentomesario/estados/estadosregistromesarios.h"

#include <format>
#include <mutex>

#include "api/gui/cformbuildermt.h"
#include "api/pattern/cpolysingletonlist.h"
#include "comum/dados/celeitordetalhe.h"

namespace comum {

using api::SPoint;

namespace {
// wasm func 10323 (table slot 4379) - identical to the one in cpededigitalmesario.cpp (func 10319);
// both call the shared body func 6008 with their own srcloc (:27 here).
struct CNomeMesariosUrnaDS {
    static std::string Text(const std::string& formato)
    {
        auto& controlador = api::CPolySingletonList::instance<IControladorRegistraMesarios>();   // :27
        if (!controlador.MesarioEhEleitorDaSecao())
            return controlador.GetTituloMesario();
        const md::CEleitorDecorator eleitor = controlador.GetEleitorMesario()->GetEleitor();
        const std::string& nome = eleitor.GetNomeSocial().empty() ? eleitor.GetNome() : eleitor.GetNomeSocial();
        const std::string nomeUrna = nome.substr(0, 40);
        return std::vformat(formato, std::make_format_args(nomeUrna));
    }
};
} // namespace

// Constructor + GetInst: inlined into CPedeDigitalMesario::ProcessTick (func 10316).
CDigitalMesarioNaoReconhecida::CDigitalMesarioNaoReconhecida()
    : CAppState(2)
{
    api::CFormBuilderMT campos;
    campos.Add<api::CLedFieldMT>(false);                                          // func 435
    campos.Add<api::CBeepFieldMT>(1);                                             // func 1072
    campos.Add<api::CTextFieldMT>(SPoint{1, 1}, std::make_shared<api::CDataTextFmt<std::string (*)(const std::string&)>>(
                                                    ESQUERDA, &CNomeMesariosUrnaDS::Text, "{:2}"));
    campos.Add<api::CTextFieldMT>(SPoint{20, 2}, std::make_shared<api::CFixedText>(CENTRO, "Mesário(a) não reconhecido(a)."));
    campos.Add<api::CTextFieldMT>(SPoint{40, 4}, std::make_shared<api::CFixedText>(DIREITA, "CONFIRMA: Retornar"));
    campos.AddInputControl();
    m_form = campos.CriaFormInterativo("", true);
}

CDigitalMesarioNaoReconhecida& CDigitalMesarioNaoReconhecida::GetInst()
{
    static std::mutex mutex;                                        // @1909852
    static std::unique_ptr<CDigitalMesarioNaoReconhecida> s_inst;   // @1909876 (dtor: ICF 244 / 387)
    std::lock_guard trava(mutex);
    if (!s_inst)
        s_inst.reset(new CDigitalMesarioNaoReconhecida());
    return *s_inst;
}

// ICF func 1070 (vtable slot 2): shared by every 20-byte state with one form at +12.
void CDigitalMesarioNaoReconhecida::StartState()
{
    m_proximoEstado = this;
    m_form->Show();
}

// wasm func 10322 - vtable slot 7 (body func 2895 with 9 = CONFIRMA)
void CDigitalMesarioNaoReconhecida::ProcessInput()
{
    if (m_form->Read() == api::EInputResult::CONFIRMA)
        m_proximoEstado = &CPedeTituloMesario::GetInst();                          // back to the título screen
}

} // namespace comum
