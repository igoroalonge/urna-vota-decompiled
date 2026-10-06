// FRAGMENT reconstructed by unit u19 from vota_web_wasm.wasm.
// Original file: uenux2/src/api/gui/cpowerinformation.cpp (srcloc :131 UpdateBatteryIcon, twice).
// Declarations and the BatteryIconDataSource constructor are in cpowerinformation.u15.cpp (unit u15);
// names (GetEstadoIcone, ms_icones, CriaParIcones) follow that file. Merge both into cpowerinformation.cpp.
//
// The battery/mains icon of the status header of every screen (horizontal: urna screen, vertical: the
// microterminal LCD). A 1-second timer re-reads IPower and swaps the icon when the state changes.
// In the web build IPower is api::teste::CPowerMock (its slot 15, func 8071, copies a fixed 16-byte status).
#include <map>
#include <memory>
#include <mutex>
#include <source_location>
#include <string>
#include <vector>

#include "api/hwil/ipower.h"
#include "api/pattern/cpolysingleton.h"

namespace api {

// Accessors of IPower used below (inlined; each one refreshes IPower::m_status (+4) through vtable
// slot 15 first, so every test sees a fresh reading):                                    names inferred
//   Fonte()          = m_status & 0x06 : 0 rede elétrica (AC), 2 bateria interna, 4 bateria externa   (?)
//   BateriaInterna() = m_status & 0x18 : 0 cheia, 0x08 parcial, 0x10 crítica, 0x18 ausente            (?)
//   BateriaExterna() = m_status & 0x60 : 0 cheia, 0x20 parcial, 0x40 crítica, 0x60 ausente            (?)

// wasm func 2774 (observed executing: every status header).  Icon key 0..10 (10 = no icon).
int CPowerInformation::GetEstadoIcone(IPower& energia)
{
    if (energia.Fonte() == 4) {                                   // on the external battery
        if (energia.BateriaExterna() == 0x60) return 3;
        if (energia.BateriaExterna() == 0x40) return 9;
        return energia.BateriaExterna() == 0x20 ? 8 : 7;
    }
    if (energia.BateriaInterna() == 0x18) return 3;               // no internal battery
    if (energia.BateriaInterna() == 0x10) return energia.Fonte() ? 6 : 2;
    if (energia.BateriaInterna() == 0x08) return energia.Fonte() ? 5 : 1;
    if (energia.BateriaInterna() != 0)    return 10;              // only reachable if the status changed
                                                                  // between two readings
    return energia.Fonte() ? 4 : 0;
}

// wasm func 2773 (observed executing): (re)fills the static table; called when it is empty.   // name inferred
// It is an initializer-list assignment to the static std::map (node-reusing __assign_unique; the value
// assignment is func 3668 = vector<shared_ptr<IImage>>::assign, a libc++ helper shared with RHVoice by ICF).
void CPowerInformation::CarregaIcones()
{
    const std::string d = ":/resource/images/bateria/";
    ms_icones = {
        {0, CriaParIcones(d + "img-ac-bateria-full-hor.jpg",     d + "img-ac-bateria-full.jpg")},
        {1, CriaParIcones(d + "img-ac-bateria-parcial-hor.jpg",  d + "img-ac-bateria-parcial.jpg")},
        {2, CriaParIcones(d + "img-ac-bateria-critical-h.jpg",   d + "img-ac-bateria-critical.jpg")},
        {3, CriaParIcones(d + "img-ac-sem-bateria-hor.jpg",      d + "img-ac-sem-bateria.jpg")},
        {4, CriaParIcones(d + "img-bateria-full-hor.jpg",        d + "img-bateria-full.jpg")},
        {5, CriaParIcones(d + "img-bateria-parcial-hor.jpg",     d + "img-bateria-parcial.jpg")},
        {6, CriaParIcones(d + "img-bateria-critical-hor.jpg",    d + "img-bateria-critical.jpg")},
        {7, CriaParIcones(d + "img-bateria-ext-full-hor.jpg",    d + "img-bateria-ext-full.jpg")},
        {8, CriaParIcones(d + "img-bateria-ext-parc-hor.jpg",    d + "img-bateria-ext-parcial.jpg")},
        {9, CriaParIcones(d + "img-bateria-ext-crit-hor.jpg",    d + "img-bateria-ext-critical.jpg")},
    };   // (the literals are complete strings in the binary; the prefix is factored here for legibility)
}
// wasm func 2238: std::__tree<int, vector<shared_ptr<IImage>>>::destroy(node) (recursive, library helper).
// wasm func 10991: exit-time destructor of ms_icones (@1839232): ms_icones.~map() -> 2238.

// wasm funcs 10981 (Horizontal, table slot 3433) and 10987 (Vertical, slot 3425): operator() of the timer
// lambda [this] { UpdateBatteryIcon(); } created by the constructor, with UpdateBatteryIcon inlined.
template <CPowerInformation::IconOrientation O>
void BatteryIconDataSource<O>::UpdateBatteryIcon()
{
    const int estado = CPowerInformation::GetEstadoIcone(
        CPolySingleton<IPower>::instance(GetPolySingletonsInfo(), std::source_location::current()));  // :131 (862)
    if (estado == m_estado)
        return;
    m_estado = estado;
    if (CPowerInformation::ms_icones.empty())
        CPowerInformation::CarregaIcones();
    SharedIImage icone;                                            // stays empty for key 10
    if (auto it = CPowerInformation::ms_icones.find(estado); it != CPowerInformation::ms_icones.end())
        icone = it->second[static_cast<int>(O)];                   // [0] horizontal, [1] vertical
    std::lock_guard trava(m_mutex);                                // +24
    m_valor = icone;                                               // +4
    for (IObserver<SharedIImage>* obs : m_observadores)            // +12
        obs->Update(m_valor);                                      // observer slot 2
}

}  // namespace api
