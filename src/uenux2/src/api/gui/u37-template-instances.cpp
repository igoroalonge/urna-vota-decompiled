// Template instantiations of uenux2/src/api/gui headers (iform.h, cformbuilder.h) that the tools filed in
// unit u37 (app:vota, no source file). Reconstructed from vota_web_wasm.wasm by unit u37.
// They are written in their generic source form; the wasm bodies are shared by several instantiations
// because wasm-opt's merge-similar-functions turned the differing constants (vtables) into parameters.
#include <memory>
#include <vector>

#include "api/gui/cformbuilder.h"
#include "api/gui/iform.h"

namespace api {

// ------------------------------------------------------------------------------------------------------
// wasm func 6024 (tools: vota_f6024) - observed executing (every screen the voter sees is an IForm).
// Merged body of   IForm<MEDIA>::IForm(const TCampos& campos, const std::shared_ptr<IPreShow<MEDIA>>& preShow)
// with the vtable as 4th parameter; thunks: 5548 (MEDIA = IScreen, observed), 5522 (IScreenMT).
// (The IPaper instance is inlined elsewhere.) Layout and destructor: iform.h (unit u17).
// ------------------------------------------------------------------------------------------------------
template <class MEDIA>
IForm<MEDIA>::IForm(const TCampos& campos, const std::shared_ptr<IPreShow<MEDIA>>& preShow)
    : m_ativo(false)                                   // +4
    , m_campos(campos)                                 // +8   copy: every shared_ptr use count +1
    , m_preShow(preShow)                               // +20
    , m_mutex()                                        // +28  (24 bytes zeroed)
    , m_controle(std::make_shared<FormControlBlock>()) // +52  128-byte block, m_vivo = true (+12 in the
                                                       //      emplace), shared_mutex zeroed (shared_f4670)
    , m_nome()                                         // +60
{
    for (auto& campo : m_campos)
        campo->SetForm(this);                          // IFormFieldBase slot 6
}
template class IForm<IScreen>;
template class IForm<IScreenMT>;

// ------------------------------------------------------------------------------------------------------
// wasm func 3890 (tools: vota_f3890). Merged body (field vtable and shared_ptr control-block vtable as
// parameters) of the builders' one-argument "add a field" helpers:
//   198  CPaperFormBuilder::AddNewLine(int linhas)     -> new CNewLineFieldPaper(linhas)
//   435  CFormBuilderMT::Add<CLedFieldMT>(arg)         -> new CLedFieldMT(arg)
//   1072 CFormBuilderMT::Add<CBeepFieldMT>(int)        -> new CBeepFieldMT(arg)
// Each field is 28 bytes: IFormFieldBase (+4 dirty flag, +8 form, +12 empty name) + the argument at +24.
// The object is wrapped with shared_ptr(new T) (a __shared_ptr_pointer control block, not make_shared).
// ------------------------------------------------------------------------------------------------------
template <class MEDIA>
template <class CAMPO, class ARG>
void CFormBuilderBase<MEDIA>::AddCampo(ARG argumento)                                  // name inferred
{
    std::shared_ptr<IFormField<MEDIA>> campo(new CAMPO(argumento));
    m_campos.push_back(campo);                                                         // grows x2
}

// ------------------------------------------------------------------------------------------------------
// wasm func 5547 (tools: vota_f5547). A CFormBuilder helper that adds a CImageField for the QR-code images of
// the end of the day. Its wasm signature is (sret shared_ptr<CImageField>, this, imagem, posicao):
//   * the IMAGE comes first and the POSITION second (callers: vota_f5547(ret, builder, &img, &pos));
//   * the image is a std::shared_ptr passed BY VALUE: the callee releases it at the end. libc++ ABI v2
//     marks shared_ptr [[clang::trivial_abi]], so the callee destroys by-value shared_ptr parameters;
//   * the anchor 1 is a constant in the body, not a parameter.
// So this is not a plain instance of the variadic forwarding Add<FIELD>(ARGS&&...) of cformbuilder.u07.cpp.
// Such an instance would take the arguments by reference, in the order of CImageField's constructor
// (posicao, imagem, 1). This one is either a dedicated overload or a by-value instance whose constant anchor
// argument LTO removed.
// Callers: vota::CMostraQRCodeBU::StartState (12055: the BU QR codes, "QR code i/n") and
// vota::CMostraQRCodeCertificado slot 2 (12040: the certificate QR code). CImageField ctor = wasm 2242
// (posicao, imagem [copied: refcount+1], anchor), CFormBuilder::Add(shared_ptr<IFormField<IScreen>>) = wasm 426.
// ------------------------------------------------------------------------------------------------------
//   std::shared_ptr<CImageField> CFormBuilder::Add<CImageField>(std::shared_ptr<CQRCodeImage> imagem,
//                                                               const SPoint& posicao)      // name inferred
//   {
//       auto campo = std::make_shared<CImageField>(posicao, imagem, 1);   // 56-byte emplace block
//       Add(campo);                                                       // by-value copy (refcount+1)
//       return campo;
//   }                                                                     // ~imagem (callee-destroyed)

} // namespace api
