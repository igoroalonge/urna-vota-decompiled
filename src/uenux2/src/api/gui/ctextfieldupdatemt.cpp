// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/gui/ctextfieldupdatemt.cpp (path inferred). See the header for the layout.
#include "api/gui/ctextfieldupdatemt.h"

#include <stdexcept>

#include "api/gui/cfixedtext.h"
#include "api/gui/iscreen.h"

namespace api {

// Constructor - no out-of-line copy: inlined in vota::CPedeIdentidade::GetInst (func 652, the tools call it
// "CTextSource::CTextSource@652"). The timer lambda is func 10542 (std::function vtable @1590536).
CTextFieldUpdateMT::CTextFieldUpdateMT(const SPoint& pos, const SharedIText& texto,
                                       const std::chrono::milliseconds& periodo)
    : m_pos(pos),
      m_texto(texto),
      m_timer(ITimerScheduler::GetInst().CreateTimer(periodo, [this] {      // ITimerScheduler slot 0
          // wasm func 10542: redraw only when the text changed
          if (m_texto->GetText() != m_textoAnterior)
              Invalidate();
      }))
{
    // Unlike the screen version (CUeGuiError 4974 "Campo estava com o texto nulo"), the MT version throws a
    // plain std::invalid_argument - and only AFTER the timer was created with a callback that dereferences
    // m_texto (harmless: the timer is not started before Start()).
    if (!m_texto)
        throw std::invalid_argument("CTextFieldUpdate - campo estava com o texto nulo");
}

// wasm func 5411 (slot 0) / 10549 (slot 1 = 5411 + operator delete): releases m_timer (+52), m_textoAnterior
// (+36), m_texto (+32) and the base m_nome (+12).
CTextFieldUpdateMT::~CTextFieldUpdateMT() = default;

// wasm func 10548 (slot 2)
// The LCD has no "erase rectangle" primitive: when the new text is shorter than the previous one, the old
// text is first overwritten with the same number of spaces.
void CTextFieldUpdateMT::Draw(IScreenMT& tela) const
{
    const std::size_t tamanhoAnterior = m_textoAnterior.size();
    if (m_texto->GetText().size() < tamanhoAnterior)
        tela.Write(m_pos, CFixedText(ETextAlignment::Left, std::string(tamanhoAnterior, ' ')));   // IScreenMT slot 3
    tela.Write(m_pos, *m_texto);                                                               // IScreenMT slot 3
    m_textoAnterior = m_texto->GetText();
}

// wasm func 10546 (slot 3)
void CTextFieldUpdateMT::Start()
{
    m_timer->Start();      // ITimer slot 2
}

// wasm func 10545 (slot 4)
void CTextFieldUpdateMT::Stop()
{
    m_timer->Stop();       // ITimer slot 3
}

} // namespace api
