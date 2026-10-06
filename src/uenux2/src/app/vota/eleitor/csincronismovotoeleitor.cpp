// Reconstructed from vota_web_wasm.wasm (unit u07). Original: uenux2/src/app/vota/eleitor/csincronismovotoeleitor.cpp
// (attested: std::source_location record @1534132, csincronismovotoeleitor.cpp:67, ISincronismoVotoEleitor::GetInst).
//
// Functions of this file found in the binary:
//   7174  CSincronismoVotoEleitor::SincronizaVoto          vtable slot 2 (name inferred)
//         (the bodies of csincronizavota.cpp's SincronizaRDVInternoSeguro / SincronizaRDVExternoSeguro are
//          inlined into it: see src/uenux2/src/app/vota/comum/csincronizavota.u07.cpp)
//   ----  ISincronismoVotoEleitor::GetInst                 inlined into CSincronismoEleitor::ProcessMessage (func 7181)
//
// The web adapter replaces this implementation (see the header): nothing below runs in the simulator.
#include "vota/eleitor/csincronismovotoeleitor.h"

#include <memory>

#include "api/pattern/cpolysingleton.h"          // api::CPolySingleton / api::CPolySingletonList
#include "vota/comum/csincronizavota.h"      // vota::CSincronizaVota
#include "vota/eleitor/comum/ctelasvota.h"   // vota::CTelasVota
#include "vota/log/clogvota.h"               // vota::CLogVota

namespace vota::impl {

// csincronismovotoeleitor.cpp:67 - inlined into vota::CSincronismoEleitor::ProcessMessage (wasm func 7181,
// another unit), which calls it when it receives message 5 and then moves to CFimVotoEleitor on success.
ISincronismoVotoEleitor& ISincronismoVotoEleitor::GetInst()
{
    auto& lista = api::GetPolySingletonsInfo();                        // function pointer @1526320 -> func 11265
    if (!api::CPolySingletonList::contains<ISincronismoVotoEleitor>(lista))     // func 2741
        api::CPolySingletonList::push<ISincronismoVotoEleitor>(std::make_unique<CSincronismoVotoEleitor>(), lista);
                                                                        // func 5398 (cpolysingletonlist.h:129)
    return api::CPolySingleton<ISincronismoVotoEleitor>::instance(lista, std::source_location::current());
                                                                        // cpolysingleton.h:78, cpolysingletonlist.h:99/105
}

// ---------------------------------------------------------------------------------------------------
// wasm func 7174 (vtable slot 2)                                                    // name inferred
//
// The four AvancaBarraProgresso() calls drive the 4-step progress bar of the "Gravando" screen that
// CSincronismoEleitor::StartState (func 7178) displayed (CTelasVota +172 form, +180 CProgressBar, max 4).
// The web adapter imitates exactly these four steps in votaTick: while the state name contains
// "CSincronismoEleitor" it calls CTelasVota::GetInst().AvancaBarraProgresso() (func 2369) at t+120 ms and
// then every 160 ms, four times, and finally posts message 5 (func 10376) to the voter queue.
bool CSincronismoVotoEleitor::SincronizaVoto()
{
    CTelasVota::GetInst().AvancaBarraProgresso();                       // func 2369 (1/4)

    // csincronizavota.cpp:290 (inlined): write + verify + sign the RDV and the state on the MI
    // (memória interna = internal flash /dsk/fi). Throws 9300 "Falha na gravação do RDV na MI".
    CSincronizaVota::SincronizaRDVInterno();                            // name inferred (wrapper of SincronizaRDVInternoSeguro)

    CTelasVota::GetInst().AvancaBarraProgresso();                       // (2/4)
    CLogVota::GetInst().Loga("O voto do eleitor foi computado");       // api_f233 -> CLoga::loga(level 1)
    CTelasVota::GetInst().AvancaBarraProgresso();                       // (3/4)

    // csincronizavota.cpp:303 (inlined): same on the MV (memória de votação = removable flash /dsk/fe),
    // then copies the MI signature files to the MV. Throws 9301 "Falha na gravação do RDV na MV".
    CSincronizaVota::SincronizaRDVExterno();                            // name inferred (wrapper of SincronizaRDVExternoSeguro)

    CTelasVota::GetInst().AvancaBarraProgresso();                       // (4/4)
    return true;
}

} // namespace vota::impl

// For reference, the web replacement registered by main() (uenux2/wasm/vota_web/vota_web_wasm.cpp; declared in
// src/uenux2/wasm/vota_web/vota_web_wasm.u30.cpp):
//
//   namespace {
//   class CSincronismoVotoEleitorWeb final : public vota::impl::ISincronismoVotoEleitor {
//   public:
//       bool SincronizaVoto() override { return true; }      // ICF body func 434 ("return 1")
//   };
//   }

// ---------------------------------------------------------------------------------------------------
// Added by unit u41. The registry query that GetInst() above makes (called `contains` there, `exists` in
// cpolysingletonlist.h). In the binary it is its own function:
// wasm func 2741 = api::CPolySingletonList::exists<vota::impl::ISincronismoVotoEleitor>(info)   (name inferred)
//   return <merged exists body, func 3884>(info, &typeid(ISincronismoVotoEleitor).__type_name /* @1534152 */);
// Observed executing (from __wasm_call_ctors, from main via replace<> 9437, and from 7181).
template bool api::CPolySingletonList::exists<vota::impl::ISincronismoVotoEleitor>(api::TPolySingletonsInfo&);
