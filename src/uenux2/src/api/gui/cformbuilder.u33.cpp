// uenux2/src/api/gui/cformbuilder.cpp  -- FRAGMENT written by unit u33 (srcloc cformbuilder.cpp exists; the
// file is shared between units u02, u07, u15, u16 - see cformbuilder.u*.cpp for the other members).
//
// CFormBuilder is a std::vector<std::shared_ptr<IFormField<IScreen>>>. Every Add* builds a field with
// std::make_shared (one allocation: control block __shared_ptr_emplace + object), appends it with
// CFormBuilder::Add (wasm func 426, which also names the field "<ClassName><n>") and returns it.
#include <chrono>
#include <memory>
#include <string>

#include "api/gui/cdatatext.h"
#include "api/gui/cformbuilder.h"
#include "api/gui/clinefield.h"
#include "api/gui/ctextfieldupdate.h"

namespace api {

// wasm func 2244                                                                    // name inferred
// A 1-pixel line between two points: CLineField (vtable @1579364, 36 bytes: IFormField base zeroed,
// +24 inicio, +28 fim, +32 cor). Callers: CInteractiveFormBuilder::AddLabeledInputControl (653), func 3054,
// the CTelasVota constructor blob (7787), vota::CMostraQRCodeBU::StartState (12055: separators of the
// "BU digital" QR-code screen) and vota::CImprimirBUOutrasObrigatorias::StartState (12065).
std::shared_ptr<CLineField> CFormBuilder::AddLine(const SPoint& inicio, const SPoint& fim, TColor cor)
{
    auto campo = std::make_shared<CLineField>(inicio, fim, cor);
    Add(campo);                                                  // func 426
    return campo;
}

// wasm func 3058 (observed executing: status-header clock of every voter screen)    // name inferred
// A text field refreshed periodically from a formatting data source:
//   CDataTextFmt<std::string (*)(const std::string&)> (vtable @1539168; +4 alinhamento, +8 fonte,
//   +12 formato) wrapped in a CTextFieldUpdate(pos, texto, periodo, fonte) (func 3658, 92-byte block).
// Callers: CFormBuilder::AddStatusHeader (502: fonte = table slot 3117, the date/time source, formato
// "A DD/MM/YYYY hh:mm:ss", period 500 ms) and the CTelasVota constructor blob (7787).
std::shared_ptr<CTextFieldUpdate> CFormBuilder::AddDataTextFmt(std::string (*fonteDados)(const std::string&),
                                                               const SPoint& pos,
                                                               const std::chrono::milliseconds& periodo,
                                                               const SFont& fonte,
                                                               const std::string& formato,
                                                               ETextAlignment alinhamento)
{
    auto texto = std::make_shared<CDataTextFmt<std::string (*)(const std::string&)>>(alinhamento, fonteDados,
                                                                                     formato);
    auto campo = std::make_shared<CTextFieldUpdate>(pos, texto, periodo, fonte);
    Add(campo);                                                  // func 426
    return campo;
}

} // namespace api
