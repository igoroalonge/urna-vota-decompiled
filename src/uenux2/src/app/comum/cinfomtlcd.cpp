// uenux2/src/app/comum/cinfomtlcd.cpp
// Reconstructed from vota_web_wasm.wasm (unit u22). Attested by std::source_location records
// :33 (CreateInst), :50 and :51 (Update), :84 (MostrarImagemNoLCD).
// The constructor (func 5904) was reconstructed by unit u15 (cinfomtlcd.u15.cpp); it is repeated here
// so that the file is complete.
#include "comum/cinfomtlcd.h"

#include <algorithm>
#include <mutex>
#include <vector>

#include "api/gui/iimage.h"
#include "api/gui/iscreen.h"
#include "api/gui/iscreenmt.h"
#include "api/pattern/cpolysingletonlist.h"

namespace comum {

using api::CPolySingletonList;

// wasm func 5904 (unit u15) - inlined BatteryIconDataSource<Vertical>() (cpowerinformation.cpp:119).
CInfoMTLCD::CInfoMTLCD()
{
    m_bateria.Attach(this);          // lock; push_back(this) if absent; Update(current icon); unlock
}

// wasm func 5902 (vtable slot 0) and 11654 (slot 1 = this + operator delete).
CInfoMTLCD::~CInfoMTLCD()
{
    m_bateria.Detach(this);          // inlined IObservable::Detach: lock; erase(remove(obs, this)); unlock
    // ~BatteryIconDataSource<Vertical>() = func 3837 (see the note at the end of this file)
}

// wasm func 3836 (srcloc :84). The `bool` parameter was removed by LTO (dead-argument elimination):
// both callers (CBiometriaEleitor::GetFoto 3616 and func 3617) pass `true`, which is folded into the
// code below. Update() inlines this function with `false`, and its inlined copy contains ONLY the
// IScreenMT call (with 0): no Detach and no GetDados() test. Since neither can be optimised away for
// an unknown object, both are guarded by the same flag in the source; the flag is also forwarded to
// IScreenMT slot 4. (Parameter name inferred.)
void CInfoMTLCD::MostrarImagemNoLCD(const api::IImage& imagem, const api::SPoint& posicao, bool imagemExterna)
{
    if (imagemExterna) {
        m_bateria.Detach(this);                                          // stop drawing the battery
        if (imagem.GetDados().empty())                                   // IImage slot 2 (returns a copy)
            return;
    }
    auto& lcd = CPolySingletonList::instance<api::IScreenMT>();          // :84
    lcd.DesenhaImagem(posicao, imagem, imagemExterna);                   // IScreenMT slot 4   name inferred
    // Web build: simulador::CWasmScreenMT slot 4 is an ICF no-op (func 1870) - nothing is drawn.
}

// wasm func 11653 (vtable slot 2, srcloc :50, :51, :84). Observed executing (the 1 s battery timer of
// the web build calls it; IPower never reports a change, so the icon is the same every time). The
// final IScreenMT slot 4 call is a no-op in the web build (CWasmScreenMT), so nothing is drawn.
void CInfoMTLCD::Update(const api::SharedIImage& icone)
{
    if (!icone)
        return;
    // the icon size is measured by the voter screen's image renderer
    const api::SSize tamanho =
        CPolySingletonList::instance<api::IScreen>().GetRenderer().GetTamanho(*icone);  // :50 slot 34 -> slot 13
    auto& lcd = CPolySingletonList::instance<api::IScreenMT>();                          // :51
    const api::SPoint centro{static_cast<api::TPosition>(lcd.GetLarguraLCD() / 2 - tamanho.largura / 2),   // slot 17
                             static_cast<api::TPosition>(lcd.GetAlturaLCD() / 2 - tamanho.altura / 2)};    // slot 18
    MostrarImagemNoLCD(*icone, centro, false);                   // inlined (:84): only the slot 4 call remains
}

} // namespace comum

// ---------------------------------------------------------------------------------------------------
// Template code listed in this unit (class template declared in api/gui/cpowerinformation.h and
// defined in cpowerinformation.cpp - srclocs :119 ctor, :131 UpdateBatteryIcon):
//   func 3837  BatteryIconDataSource<Vertical>::~BatteryIconDataSource()   = func 6049(this, vtable @1551916)
//   func 5510  BatteryIconDataSource<Horizontal>::~BatteryIconDataSource() = func 6049(this, vtable @1581580)
//   func 11652 / 10990  the deleting destructors (dtor + operator delete) of the two instantiations
//   func 6049  body shared by both destructors (wasm-opt merged the two, passing the vtable as a
//              parameter): release m_timer (+48), set the IObservable<SharedIImage> vtable (@1551952),
//              destroy the mutex (+24, unlock residue), free the observer vector (+12) and release the
//              current icon (+4).
// In C++ these are simply:
//   template <api::CPowerInformation::IconOrientation O>
//   api::BatteryIconDataSource<O>::~BatteryIconDataSource() = default;
