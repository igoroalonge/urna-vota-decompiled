// FRAGMENT reconstructed by unit u15 from vota_web_wasm.wasm.
// Original file: uenux2/src/api/gui/cmoviefield.cpp (srcloc cmoviefield.cpp:53 = Rect; the constructor
// is in another unit). Merge into cmoviefield.cpp.
#include <memory>
#include <string>

#include "api/gui/gui-common.u15.h"
#include "api/pattern/cpolysingletonlist.h"

namespace api {

class CMovie;

// CMovieField (typeinfo @1579496, vtable @1579440):
//   +24 std::shared_ptr<CMovie> m_filme     (+24/+28)
//   +32 SPoint m_pos
//   +36 EAnchorPoint m_ancora
//   +40 bool m_tocando                       (playing; an int-sized store)      name inferred
//   +44 bool                                 (zeroed by the constructor, func 5543; not used here) ?
//   +48 std::shared_ptr<ITimer> m_timer      (+48/+52: frame timer)
// The constructor (func 5543, other unit) schedules m_timer with period 0 and does not start it; the
// timer callback (func 11056) advances the frame, invalidates the field and re-arms the timer with the
// remaining frame time through ITimer slot 5. Start() below does not touch the timer.
class CMovieField : public IFormField<IScreen> {
public:
    ~CMovieField() override;                                                  // 5524 / 11065
    void Draw(IScreen& tela) const override;                                  // 11064
    void Start() override;                                                    // 11061
    void Stop() override;                                                     // 11060
    std::string GetClassName() const override { return "CMovieField"; }       // 11059
    SRect Rect() const override;                                              // 11063
    void Move(const SPoint& pos) override;                                    // 11062
private:
    std::shared_ptr<CMovie> m_filme;
    SPoint m_pos;
    EAnchorPoint m_ancora;
    bool m_tocando = false;
    bool m_flag44 = false;               // +44  ?
    std::shared_ptr<ITimer> m_timer;
};

// wasm func 5524 (slot 0) / 11065 (slot 1)
CMovieField::~CMovieField()
{
    m_timer->Stop();          // slot 3
    m_tocando = false;
    // m_timer and m_filme released implicitly, then ~IFormFieldBase
}

// wasm func 11064 (observed executing) - slot 2
void CMovieField::Draw(IScreen& tela) const
{
    tela.DrawMovie(m_pos, *m_filme, m_ancora);                                // IScreen slot 26
}

// wasm func 11061 (observed executing) - slot 3: rewind the animation to its first frame (no timer call)
void CMovieField::Start()
{
    m_tocando = true;
    m_filme->m_frameAtual = 0;
    Invalidate();
}

// wasm func 11060 (observed executing) - slot 4
void CMovieField::Stop()
{
    m_timer->Stop();
    m_tocando = false;
}

// wasm func 11063 (not observed) - srcloc cmoviefield.cpp:53
SRect CMovieField::Rect() const
{
    auto& tela = CPolySingletonList::instance<IScreen>();                                  // srcloc :53
    return tela.GetImageSurfaceOps().GetRect(m_filme->m_tamanho.x, m_filme->m_tamanho.y,   // slot 34 -> 14
                                             m_pos, m_ancora);
}

// wasm func 11062 (not observed) - slot 9 (same as IFormFieldBase's func 2241, with m_pos at +32)
void CMovieField::Move(const SPoint& pos)
{
    if (pos == m_pos)
        return;
    m_pos = pos;
    Invalidate();
}

} // namespace api
