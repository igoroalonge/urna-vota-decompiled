// FRAGMENT reconstructed by unit u15 from vota_web_wasm.wasm.
// Original file: uenux2/src/api/gui/cinteractiveformbuilder.cpp (srclocs :76 lambda, :86
// AddLabeledInputControl). See also cformbuilder.u02.cpp (AddControlInput 901, AddNumberInput 2383,
// CriaFormInterativo 554). Merge into cinteractiveformbuilder.cpp.
#include <array>
#include <format>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "api/gui/cformbuilder.h"
#include "api/gui/cinputmenufield.u15.h"
#include "api/gui/gui-common.u15.h"

namespace api {

using TTeclasRotuladas = std::vector<std::pair<char, std::string>>;

// Accepts only the three control keys; the valid-character string is empty (vtable @1577484).
class CControlValidation : public IInputValidation {
public:
    // slot 2 = func 4022 (ICF body shared with CNumberValidation / COptionValidation): every character
    // must pass IsValidChar (slot 3); an empty text is valid.
    bool IsValid(const std::string& texto) const override
    {
        for (char c : texto)
            if (!IsValidChar(c))
                return false;
        return true;
    }
};

// Bits of the control-key mask (table @524056 indexed by tecla - 'B'):
enum : unsigned { TeclaBranco = 1, TeclaCorrige = 2, TeclaConfirma = 4 };          // names inferred

// wasm func 5565 (tools: api_f5565; observed executing)                             // name inferred
// CInputFieldControlBase<IScreen>(teclas): an invisible input (maximum length 0) that finishes on the
// control keys present in the mask. Callers: AddLabeledInputControl (653) and func 5530.
template <>
CInputFieldControlBase<IScreen>::CInputFieldControlBase(unsigned teclas)
    : IInputField<IScreen>(std::shared_ptr<IInputValidation>(new CControlValidation()),
                           /*tamanhoMaximo*/ 0, true, false,
                           (teclas & TeclaConfirma) != 0,
                           (teclas & TeclaBranco) != 0,
                           (teclas & TeclaCorrige) != 0)        // IInputField ctor = func 2024
{
}

// wasm func 653 (observed executing) - srclocs cinteractiveformbuilder.cpp:76 and :86
// Draws up to three labelled control keys in the bottom band of the screen and returns the form's
// input field that will receive them (an existing input of the form, or a new invisible control input).
// The SRect and bool parameters of the original signature were constant-propagated away by Binaryen:
// the area is always ms_areaTeclas (@1577200 = {0, 0, 639, 479}); the bool is unused in the body.
CInteractiveFormBuilder::SharedInput
CInteractiveFormBuilder::AddLabeledInputControl(const TTeclasRotuladas& teclas,
                                                const SRect& area /* = ms_areaTeclas */,
                                                bool /* ? */)
{
    // Positions are consumed from the back: 1st key right, 2nd key left, 3rd key centred.
    std::vector<ETextAlignment> posicoes{ETextAlignment::Center, ETextAlignment::Left, ETextAlignment::Right};
    if (teclas.size() > 3)
        throw CUeGuiError(static_cast<EUeGuiError>(4937),
                          std::format("O número máximo de teclas suportado é {}", 3));        // :86

    unsigned mascara = 0;
    for (const auto& [tecla, rotulo] : teclas) {
        // lambda at :76: only BRANCO ('B'), CONFIRMA ('C') and CORRIGE ('D') are allowed
        if (tecla < 'B' || tecla > 'D')
            throw CUeGuiError(static_cast<EUeGuiError>(4936),
                              std::format("Tecla de controle não suportada '{}'", tecla));   // :76
        const ETextAlignment alinhamento = posicoes.back();
        posicoes.pop_back();
        AddLabeledKey(KeyName(tecla), rotulo, alinhamento);           // funcs 744 + 3678
        mascara |= std::array<unsigned, 3>{TeclaBranco, TeclaConfirma, TeclaCorrige}[tecla - 'B'];
    }

    // Separator line above the key band: (left, bottom-50) .. (right+1, bottom-50), 3 px.
    AddLine(SPoint{area.left, static_cast<TPosition>(area.bottom - 50)},
            SPoint{static_cast<TPosition>(area.right + 1), static_cast<TPosition>(area.bottom - 50)}, 3);  // 2244

    // Reuse the first input field already in the form, if any.
    for (auto& campo : m_campos)
        if (campo && dynamic_cast<IInputField<IScreen>*>(campo.get()))
            return std::dynamic_pointer_cast<IInputField<IScreen>>(campo);

    auto controle = std::make_shared<CInputFieldControl<IScreen>>(mascara);  // 76 bytes, vtable @1577340
    Add(controle);                                                            // func 426
    return controle;
}

// wasm func 3675 (observed: no; tools named it CInputMenuField::GetInstructionTextRect after an
// inlined srcloc)                                                                  // name inferred
// Creates a CInputMenuField (constructor and SetInstructionText inlined, see cinputmenufield.u15.cpp)
// and adds it to the builder. Callers: vota::CMenuVisualizarCandidatos::StartState and the two
// CMenuFiltrarCandidatosPor{Partido,Cargo}::StartState (the "view candidates" menus).
// Like AddLabeledInputControl, `this` is the builder (the field vector) and the result is returned
// through the sret pointer.
std::shared_ptr<CInputMenuField>
CInteractiveFormBuilder::AddInputMenu(const SPoint& pos, const SFont& fonte,
                                      const std::string& instrucao, TPosition alturaMaxima)
{
    auto menu = std::make_shared<CInputMenuField>(pos, fonte, alturaMaxima);   // 184-byte control block
    menu->SetInstructionText(instrucao);
    Add(menu);                                                                  // func 426
    return menu;
}

} // namespace api
