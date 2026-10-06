// FRAGMENT reconstructed by unit u15 from vota_web_wasm.wasm.
// Original file: uenux2/src/api/gui/cimagefield.cpp (srcloc cimagefield.cpp:52 = Rect). Merge into it.
#include <memory>
#include <string>

#include "api/gui/gui-common.u15.h"
#include "api/pattern/cpolysingletonlist.h"

namespace api {

class IImage;
using SharedIImage = std::shared_ptr<IImage>;

// CImageField (typeinfo @1578848, vtable @1578792) - 44 bytes:
//   +24 SPoint m_pos
//   +28 SharedIImage m_imagem          (+28 ptr, +32 control block)
//   +36 std::unique_ptr<SRect> m_recorte   (optional clip/sub-rectangle; freed in the destructor,
//                                        passed to IScreen slot 24 when set)         name inferred
//   +40 EAnchorPoint m_ancora
class CImageField : public IFormField<IScreen> {
public:
    CImageField(const SPoint& pos, const SharedIImage& imagem, EAnchorPoint ancora);   // 2242
    ~CImageField() override;                                                            // 11099 / 11098
    void Draw(IScreen& tela) const override;                                            // 11102
    std::string GetClassName() const override { return "CImageField"; }                 // 11097
    SRect Rect() const override;                                                        // 11101
    void SetImage(SharedIImage imagem);                                                 // 5533
protected:
    SPoint m_pos;                               // +24
    SharedIImage m_imagem;                      // +28
    std::unique_ptr<SRect> m_recorte;           // +36
    EAnchorPoint m_ancora;                      // +40
};

// wasm func 2242 (not observed; callers AddStatusHeader (inlined CDSImageField), photo frames
// adicionaFotoEmoldurada, func 6689 ...)                                   // name confirmed by vtable store
CImageField::CImageField(const SPoint& pos, const SharedIImage& imagem, EAnchorPoint ancora)
    : m_pos(pos), m_imagem(imagem), m_recorte(nullptr), m_ancora(ancora)
{
}

// wasm func 11099 (slot 0) / 11098 (slot 1, deleting): release m_recorte, m_imagem, then ~IFormFieldBase.
CImageField::~CImageField() = default;

// wasm func 11102 (observed executing) - slot 2
void CImageField::Draw(IScreen& tela) const
{
    if (!m_imagem)
        return;
    if (!m_recorte)
        tela.DrawImage(m_pos, *m_imagem, m_ancora);                   // IScreen slot 22
    else
        tela.DrawImage(m_pos, *m_imagem, *m_recorte, m_ancora);       // IScreen slot 24
}

// wasm func 11101 (not observed) - srcloc cimagefield.cpp:52, slot 8
SRect CImageField::Rect() const
{
    if (!m_imagem)
        return SRect{};
    auto& tela = CPolySingletonList::instance<IScreen>();                               // srcloc :52
    return tela.GetImageSurfaceOps().GetImageRect(*m_imagem, m_pos, m_ancora);          // slot 34 -> slot 16
}

// wasm func 5533 (tools: api_f5533; attributed to cdsimagefield.cpp)                 // name inferred
// Replaces the image and asks the form for a redraw (the Invalidate idiom).
// The body copy-assigns (increments the new control block, releases the old one) and then releases the
// by-value parameter at the end (callers 11092/11093 pass a fresh copy), so it is a copy, not a move.
void CImageField::SetImage(SharedIImage imagem)
{
    m_imagem = imagem;
    Invalidate();
}

} // namespace api
