// FRAGMENT reconstructed by unit u15 from vota_web_wasm.wasm.
// Original file: uenux2/src/api/gui/cpowerinformation.cpp (srclocs :119 BatteryIconDataSource<O>() and
// :131 UpdateBatteryIcon). The two constructor instantiations only exist inlined (Horizontal in
// CFormBuilder::AddStatusHeader, func 502; Vertical in comum::CInfoMTLCD's constructor, func 5904).
// The icon table initialiser (func 2773), the status decoder (func 2774) and the timer lambdas
// (10981/10987) belong to other units. Merge into cpowerinformation.cpp.
#include <chrono>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "api/gui/cfixedimage.h"
#include "api/pattern/cpolysingletonlist.h"
#include "api/util/itimerscheduler.h"

namespace api {

class IImage;
class IPower;
using SharedIImage = std::shared_ptr<IImage>;

struct CPowerInformation {
    enum class IconOrientation : int { Horizontal = 0, Vertical = 1 };
    // func 2774: power/battery state index from IPower (0..10: AC, full, partial, critical, external ...)
    static int GetEstadoIcone(IPower& energia);                                        // name inferred
    // Static table (std::map root @1839236, size @1839240), filled on first use by func 2773:
    //   estado -> { horizontal icon, vertical icon }, e.g.
    //   ":/resource/images/bateria/img-bateria-full-hor.jpg", "...-parcial-hor.jpg", "...-ext-crit-hor.jpg",
    //   "...-ext-full-hor.jpg", "img-ac-bateria-full-hor.jpg" and the matching vertical images.
    static std::map<int, std::vector<SharedIImage>> ms_icones;                         // name inferred
};

// wasm func 1263 (tools: api_f1263; observed executing; only caller func 2773)      // name inferred
// One entry of the battery icon table: both orientations of the same state.
std::vector<SharedIImage> CriaParIcones(const std::string& horizontal, const std::string& vertical)
{
    return {std::make_shared<CFixedImage>(horizontal), std::make_shared<CFixedImage>(vertical)};
}

// IObservable<SharedIImage> data source of the battery icon - 60 bytes:
//   +4  SharedIImage m_valor           (current icon)
//   +12 std::vector<IObserver<SharedIImage>*> m_observadores
//   +24 std::mutex
//   +48 std::shared_ptr<ITimer> m_timer (1 s)
//   +56 int m_estado                   (last CPowerInformation::GetEstadoIcone value)
template <CPowerInformation::IconOrientation O>
class BatteryIconDataSource : public IObservable<SharedIImage> {
public:
    BatteryIconDataSource();
private:
    void UpdateBatteryIcon();                      // srcloc :131 (timer lambda, funcs 10981/10987)
    std::shared_ptr<ITimer> m_timer;
    int m_estado = 0;
};

// Constructor - srcloc cpowerinformation.cpp:119 (inlined in funcs 502 and 5904).
template <CPowerInformation::IconOrientation O>
BatteryIconDataSource<O>::BatteryIconDataSource()
{
    m_timer = ITimerScheduler::GetInst().Agenda(std::chrono::milliseconds{1000},
                                                [this] { UpdateBatteryIcon(); });
    m_estado = CPowerInformation::GetEstadoIcone(CPolySingletonList::instance<IPower>());   // :119
    SharedIImage icone;                                         // stays null if the state is not in the table
    if (auto it = CPowerInformation::ms_icones.find(m_estado); it != CPowerInformation::ms_icones.end())
        icone = it->second[static_cast<int>(O)];               // [0] horizontal, [1] vertical
    {
        std::lock_guard trava(m_mutex);
        m_valor = icone;
    }
    m_timer->Start();
}

} // namespace api
