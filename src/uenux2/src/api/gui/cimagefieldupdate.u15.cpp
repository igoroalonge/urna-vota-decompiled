// FRAGMENT reconstructed by unit u15 from vota_web_wasm.wasm.
// Original file: uenux2/src/api/gui/cimagefieldupdate.cpp (srclocs cimagefieldupdate.cpp:32 constructor,
// :80 Rect). The constructor only exists inlined in func 3059 (a screen helper of vota, see
// src/uenux2/src/app/vota/u15-foreign-fragments.cpp), which is why the tools gave 3059 its name.
#include <chrono>
#include <functional>
#include <memory>
#include <string>

#include "api/gui/gui-common.u15.h"
#include "api/pattern/cpolysingletonlist.h"
#include "api/util/itimerscheduler.h"

namespace api {

class IImage;
using SharedIImage = std::shared_ptr<IImage>;

// CImageFieldUpdate (typeinfo @1579060, vtable @1578988) - 52 bytes. An image field whose image is
// re-read periodically (the QR codes that rotate through several pages).
//   +24 SPoint m_pos
//   +28 SharedIImage m_imagem               (+28/+32)
//   +36 std::unique_ptr<SRect> m_recorte     (always null here)
//   +40 EAnchorPoint m_ancora
//   +44 std::shared_ptr<ITimer> m_timer     (+44/+48)
class CImageFieldUpdate : public IFormField<IScreen> {
public:
    CImageFieldUpdate(const SPoint& pos, const SharedIImage& imagem,
                      const std::chrono::milliseconds& periodo, EAnchorPoint ancora);
    ~CImageFieldUpdate() override;                                            // 5531 / 11086
    void Draw(IScreen& tela) const override;                                  // 11090
    void Start() override { m_timer->Start(); }                               // 11089 (slot 3)
    void Stop() override { m_timer->Stop(); }                                 // 11088 (slot 4)
    std::string GetClassName() const override { return "CImageFieldUpdate"; } // 11085
    SRect Rect() const override;                                              // 11087
private:
    SPoint m_pos;
    SharedIImage m_imagem;
    std::unique_ptr<SRect> m_recorte;
    EAnchorPoint m_ancora;
    std::shared_ptr<ITimer> m_timer;
};

// Constructor - srcloc cimagefieldupdate.cpp:32 (inlined in func 3059).
CImageFieldUpdate::CImageFieldUpdate(const SPoint& pos, const SharedIImage& imagem,
                                     const std::chrono::milliseconds& periodo, EAnchorPoint ancora)
    : m_pos(pos), m_imagem(imagem), m_recorte(nullptr), m_ancora(ancora)
{
    // ITimerScheduler slot 0: Schedule(out timer, period, callback). The callback (vtable @1579080)
    // invalidates the field so the image (which changes page on every read) is drawn again.
    m_timer = ITimerScheduler::GetInst().Agenda(periodo, [this] { Invalidate(); });   // lambda = func 11082
    if (!m_imagem)
        throw CUeGuiError(static_cast<EUeGuiError>(4921), "Campo estava com a imagem nula");
}

// wasm func 5531 (slot 0) / 11086 (slot 1): release m_timer, m_recorte, m_imagem, then the base.
CImageFieldUpdate::~CImageFieldUpdate() = default;

// wasm func 11090 (slot 2) - identical to CImageField::Draw but without the null test.
void CImageFieldUpdate::Draw(IScreen& tela) const
{
    if (!m_recorte)
        tela.DrawImage(m_pos, *m_imagem, m_ancora);                    // IScreen slot 22
    else
        tela.DrawImage(m_pos, *m_imagem, *m_recorte, m_ancora);        // IScreen slot 24
}

// wasm func 11087 - srcloc cimagefieldupdate.cpp:80
SRect CImageFieldUpdate::Rect() const
{
    auto& tela = CPolySingletonList::instance<IScreen>();                                 // srcloc :80
    return tela.GetImageSurfaceOps().GetImageRect(*m_imagem, m_pos, m_ancora);            // slot 34 -> 16
}

} // namespace api
