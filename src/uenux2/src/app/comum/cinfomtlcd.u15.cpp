// FRAGMENT reconstructed by unit u15 from vota_web_wasm.wasm.
// Original file: uenux2/src/app/comum/cinfomtlcd.cpp (srclocs :33 CreateInst, :50/:51 Update,
// :84 MostrarImagemNoLCD are in other units). Wasm func 5904 was named by the tools after the
// BatteryIconDataSource<Vertical> constructor inlined into it; the function itself is the constructor
// of comum::CInfoMTLCD (it stores vtable comum::CInfoMTLCD @1551828 into `this`). Merge into cinfomtlcd.cpp.
#include "api/gui/cpowerinformation.h"   // BatteryIconDataSource (see cpowerinformation.u15.cpp)

namespace comum {

// CInfoMTLCD (typeinfo @1551888, vtable @1551828: 0/1 dtor, 2 Update(const SharedIImage&)) -
// shows the battery icon on the microterminal LCD:
//   +0 vptr (IObserver<SharedIImage>)
//   +4 api::BatteryIconDataSource<Vertical> m_bateria    (60 bytes)
class CInfoMTLCD : public api::IObserver<api::SharedIImage> {
public:
    CInfoMTLCD();
    void Update(const api::SharedIImage& icone) override;     // func 11653 (other unit), :50/:51
private:
    api::BatteryIconDataSource<api::CPowerInformation::IconOrientation::Vertical> m_bateria;   // +4
};

// wasm func 5904 (observed executing; callers func 2284 and func 7787, vota::CInformacaoEleitor::Inicializar)
//                                                                              // name inferred
CInfoMTLCD::CInfoMTLCD()
    : m_bateria()                // srcloc cpowerinformation.cpp:119 (inlined)
{
    m_bateria.Attach(this);      // inlined: lock; push_back(this) if absent; Update(current icon); unlock
}

} // namespace comum
