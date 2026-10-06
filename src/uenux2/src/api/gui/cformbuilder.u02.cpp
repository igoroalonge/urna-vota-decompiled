// uenux2/src/api/gui/cformbuilder.cpp + cinteractiveformbuilder.cpp + iform.h + iinputfield.h
// FRAGMENT written by unit u02 (these files belong to units u15/u17).
//
// Small GUI helpers that the tools attributed to chkdfseed.cpp because wasm func 7787 (the inlined
// vota::CTelasVota constructor) is their most frequent caller. All names are inferred from what the
// bodies construct (vtables stored) and from the srcloc-named siblings
// api::CFormBuilder::AddStatusHeader (502), GetLabeledKeyPos (5544) and
// api::CInteractiveFormBuilder::AddLabeledInputControl (653).
//
// CFormBuilder is a std::vector<std::shared_ptr<IFormField<IScreen>>> (12 bytes). Every Add*
// returns the new field as shared_ptr and appends it with CFormBuilder::Add (wasm func 426, which
// also gives the field a unique name).

#include "api/gui/cformbuilder.h"
#include "api/gui/cinteractiveformbuilder.h"

namespace api {

// wasm func 202                                                        // name inferred
// make_shared<CFixedText>(texto) + make_shared<CTextField>(pos, fonte, alinhamento, ...)
std::shared_ptr<CTextField> CFormBuilder::AddText(const std::string& texto, const SPoint& pos,
                                                  const SFonte& fonte, int alinhamento,
                                                  int ancora, int cor)
{
    auto dado = std::make_shared<CFixedText>(texto);                  // vtable CFixedText @1532648
    auto campo = std::make_shared<CTextField>(pos, dado, fonte, ancora, cor);   // wasm 1918
    // `alinhamento` is stored in the CFixedText (+16 of the control block, i.e. before the text)
    Add(campo);
    return campo;
}

// wasm func 3680                                                       // name inferred
// Rectangle outline given by two corners (normalised with min/max).
std::shared_ptr<CRectField> CFormBuilder::AddRect(const SPoint& a, const SPoint& b)
{
    const SRect rect{{std::min(a.x, b.x), std::min(a.y, b.y)}, {std::max(a.x, b.x), std::max(a.y, b.y)}};
    auto campo = std::make_shared<CRectField>(rect);
    Add(campo);
    return campo;
}

// wasm func 5501                                                       // name inferred
// CRectField(const SRect&): IFormField base zeroed, vtable CRectField @1582180, rect at +24,
// line width 2 at +32.
CRectField::CRectField(const SRect& rect) : m_rect(rect), m_espessura(2) {}

// wasm func 2245                                                       // name inferred
std::shared_ptr<CFillField> CFormBuilder::AddFill(const SPoint& a, const SPoint& b, int cor)
{
    const SRect rect{{std::min(a.x, b.x), std::min(a.y, b.y)}, {std::max(a.x, b.x), std::max(a.y, b.y)}};
    auto campo = std::make_shared<CFillField>(rect, cor);             // vtable CFillField @1577788
    Add(campo);
    return campo;
}

// wasm func 6689                                                       // name inferred
// Image field bound to the dynamic image data source 1086 (both callers use this id; the id is a
// constant in the body, so this may be a helper of ctelasvota.cpp rather than a builder method).
std::shared_ptr<CImageField> CFormBuilder::AddDataImage1086(const SPoint& pos)
{
    auto dado = std::make_shared<CDataImage<std::vector<unsigned char>>>(1086);
    auto campo = std::make_shared<CImageField>(pos, dado, 0);          // wasm 2873 (CImageField ctor)
    Add(campo);
    return campo;
}

// wasm func 3678                                                       // name inferred
// Key label pair ("CONFIRMA" + action text) at the positions computed by GetLabeledKeyPos
// (rect @1577200), row 0 and row 21. The SECOND field (the action text) is renamed
// "labelAcao" + tecla: the binary drops the shared_ptr returned by the first AddText right away
// and assigns the name (string at +12 of the field) through the pointer returned by the second one.
void CFormBuilder::AddLabeledKey(const std::string& tecla, const std::string& rotulo,
                                 ETextAlignment alinhamento)
{
    AddText(tecla, GetLabeledKeyPos(ms_areaTeclas, alinhamento, 0),
            FONTE_ROTULO /*@520920*/, alinhamento, 2, 1);
    auto campoRotulo = AddText(rotulo, GetLabeledKeyPos(ms_areaTeclas, alinhamento, 21),
                               FONTE_ROTULO, alinhamento, 2, 1);
    campoRotulo->SetName("labelAcao" + tecla);
}

// wasm func 2243                                                       // name inferred
void CFormBuilder::AddLabeledKey(char tecla, const std::string& rotulo, ETextAlignment alinhamento)
{
    AddLabeledKey(KeyName(tecla), rotulo, alinhamento);                 // KeyName = wasm 744
}

// ---------------------------------------------------------------------------------------------
// Interactive forms

// wasm func 2024                                                       // name inferred
// IInputField<IScreen> constructor (vtable @1538852). Layout: validation shared_ptr at +36/+40,
// maximum length at +44, short 256 at +48, five flags at +50..+54, typed text (std::string) at +24,
// blink timer at +56. Registers a 600 ms periodic callback with ITimerScheduler (wasm 1259) and
// reserves maxLen characters in the text.
template <>
IInputField<IScreen>::IInputField(std::shared_ptr<IInputValidation> validacao, std::size_t tamanhoMaximo,
                                  bool f1, bool f2, bool f3, bool f4, bool f5)
    : m_validacao(std::move(validacao)), m_tamanhoMaximo(tamanhoMaximo),
      m_f1(f1), m_f2(f2), m_f3(f3), m_f4(f4), m_f5(f5)
{
    ITimerScheduler::GetInst().Agenda(m_timer, [this] { PiscaCursor(); }, std::chrono::milliseconds{600});
    m_texto.reserve(tamanhoMaximo);
}

// wasm func 901                                                        // name inferred
// Invisible input that only accepts control keys (CONFIRMA/CORRIGE/BRANCO).
std::shared_ptr<CInputFieldControl<IScreen>> CInteractiveFormBuilder::AddControlInput(CFormBuilder& builder)
{
    auto campo = std::make_shared<CInputFieldControl<IScreen>>(
        std::shared_ptr<IInputValidation>(new CControlValidation()), 0, true, false, true, false, false);
    builder.Add(campo);
    return campo;
}

// wasm func 2383                                                       // name inferred
// Numeric input drawn in a CFramedText (digits validated by CNumberValidation("0123456789")).
std::shared_ptr<CInputField<CFramedText>> CInteractiveFormBuilder::AddNumberInput(
    CFormBuilder& builder, std::size_t tamanho, bool f1, bool f2, bool f3, bool f4, bool f5,
    const SPoint& pos, const SFonte& fonte, int ancora)
{
    std::shared_ptr<IInputValidation> validacao(new CNumberValidation("0123456789"));
    std::shared_ptr<CInputField<CFramedText>> campo(
        new CInputField<CFramedText>(validacao, tamanho, f1, f2, f3, f4, f5,
                                     CFramedText(tamanho, pos, fonte, ancora)));
    builder.Add(campo);
    return campo;
}

// wasm func 5548: thunk `return IFormBase(this, campos, preShow, vtable IForm<IScreen> @1578040)`
// (merged body wasm 6024, shared with other IForm instantiations): copies the field vector, the
// pre-show shared_ptr and creates the 128-byte FormControlBlock.
template <>
IForm<IScreen>::IForm(const CFormBuilder& campos, std::shared_ptr<IPreShow> preShow);

// wasm func 554                                                        // name inferred
// Builds a CInteractiveForm<IScreen, IInputKbd> (92 bytes, vtable @1579176) from the builder: every
// field that dynamic_casts from IFormField<IScreen> to IInputField<IScreen> is also pushed into the
// input list (+72). `nome` (if not empty) becomes the form name (+60). The builder is emptied.
std::shared_ptr<CInteractiveForm<IScreen, IInputKbd>>
CriaFormInterativo(CFormBuilder& builder, std::shared_ptr<IPreShow> preShow,
                   const std::string& nome, bool flag /* +88 */)
{
    auto* form = new CInteractiveForm<IScreen, IInputKbd>(builder, preShow);
    form->m_flag = flag;
    for (auto& campo : builder)
        if (auto entrada = std::dynamic_pointer_cast<IInputField<IScreen>>(campo))
            form->m_entradas.push_back(entrada.get());
    std::shared_ptr<CInteractiveForm<IScreen, IInputKbd>> resultado(form);
    if (!nome.empty())
        form->SetName(nome);
    builder.clear();
    return resultado;
}

// wasm func 5550                                                       // name inferred
// Same for a plain (non-interactive) IForm<IScreen> (72 bytes).
std::shared_ptr<IForm<IScreen>> CriaForm(CFormBuilder& builder, std::shared_ptr<IPreShow> preShow,
                                         const std::string& nome)
{
    std::shared_ptr<IForm<IScreen>> resultado(new IForm<IScreen>(builder, preShow));
    if (!nome.empty())
        resultado->SetName(nome);
    builder.clear();
    return resultado;
}

} // namespace api

// ---------------------------------------------------------------------------------------------
// VOTA wrappers (path inferred: uenux2/src/app/vota/comum/ ... ; they only exist in VOTA code)
namespace vota {

// wasm func 576 (curated earlier): interactive form with a new vota::CPreShowFormVota.
std::shared_ptr<api::CInteractiveForm<api::IScreen, api::IInputKbd>>
CriaFormInterativoVota(api::CFormBuilder& builder, const std::string& nome)
{
    return api::CriaFormInterativo(builder, std::shared_ptr<api::IPreShow>(new CPreShowFormVota()),
                                   nome, true);
}

// wasm func 6117: merged body (wasm-opt merge-similar-functions) of
//   template <class PRESHOW> shared_ptr<IForm<IScreen>> CriaForm(builder, nome)
// taking the control-block vtable and the PRESHOW vtable as extra parameters.
// wasm func 886 is its CPreShowFormVota instance (the other one is wasm 11140).
std::shared_ptr<api::IForm<api::IScreen>> CriaFormVota(api::CFormBuilder& builder, const std::string& nome)
{
    return api::CriaForm(builder, std::shared_ptr<api::IPreShow>(new CPreShowFormVota()), nome);
}

} // namespace vota
