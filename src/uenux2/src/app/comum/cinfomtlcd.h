// uenux2/src/app/comum/cinfomtlcd.h  (path inferred from cinfomtlcd.cpp)
// Reconstructed from vota_web_wasm.wasm (unit u22).
//
// CInfoMTLCD = "informação no LCD do micro-terminal": the mesário's micro-terminal (MT) has a small
// graphic LCD besides the 4x40 text area. This singleton shows the battery icon there (it observes
// api::BatteryIconDataSource<Vertical>) and, on request, an arbitrary image (the voter's photo, see
// comum::md::CBiometriaEleitor::GetFoto / func 3616).
//
// RTTI: comum::CInfoMTLCD : api::IObserver<api::SharedIImage>   typeinfo @1551888, vtable @1551828
//   slot 0 ~CInfoMTLCD (func 5902)   slot 1 deleting dtor (func 11654)   slot 2 Update (func 11653)
// Singleton: std::unique_ptr @1838572 (mutex @1838548), created by CreateInst (cinfomtlcd.cpp:33, inlined
// into the voter start-up routine, func 7787) and returned by GetInst (func 2284, unit u05 fragment).
#pragma once

#include <memory>

#include "api/gui/cpowerinformation.h"   // api::BatteryIconDataSource, api::CPowerInformation
#include "api/pattern/iobserver.h"

namespace api {
class IImage;
struct SPoint;
using SharedIImage = std::shared_ptr<IImage>;
}

namespace comum {

class CInfoMTLCD : public api::IObserver<api::SharedIImage>
{
public:
    static void CreateInst();                                    // srcloc :33 (inlined into func 7787)
    static CInfoMTLCD& GetInst();                                // func 2284

    ~CInfoMTLCD() override;                                      // func 5902 / 11654

    // Observer callback of the battery data source: centre the new icon on the MT LCD.
    void Update(const api::SharedIImage& icone) override;        // func 11653 (srcloc :50, :51)

    // Draws an image on the MT LCD. For an external picture (flag true, the only value the out-of-line
    // callers use) the battery observer is detached first, so that the next battery tick does not
    // overwrite it, and an empty image is ignored; Update() calls it with false.
    void MostrarImagemNoLCD(const api::IImage& imagem, const api::SPoint& posicao,
                            bool imagemExterna = true);          // func 3836 (srcloc :84)

private:
    CInfoMTLCD();                                                // func 5904 (unit u15)

    // layout (64 bytes)
    //   +0  vptr (IObserver<SharedIImage>)
    //   +4  battery data source, 60 bytes:
    //         +8  SharedIImage icone atual         +16 std::vector<IObserver*> observadores (begin/end/cap)
    //         +28 std::mutex (single-threaded build: only unlock stubs remain)
    //         +52 std::shared_ptr<ITimer> (1 s)    +60 int estado do ícone
    api::BatteryIconDataSource<api::CPowerInformation::IconOrientation::Vertical> m_bateria;   // +4
};

} // namespace comum
