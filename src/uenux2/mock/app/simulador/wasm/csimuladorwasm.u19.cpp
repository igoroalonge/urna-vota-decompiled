// FRAGMENT reconstructed by unit u19 from vota_web_wasm.wasm.
// Original file: UNKNOWN - path inferred: uenux2/mock/app/simulador/wasm/csimuladorwasm.cpp
// (the constructor, func 8311, is attributed to app:simulador; every object it creates is a
//  simulador::CWasm* mock or an api::teste::* mock).
//
// The browser "hardware": main() (func 10307, vota_web_wasm.cpp) builds this object with
//   CSimuladorWasm simulador(10, "VOTA Web", js_ler_dimensao_tela(0, 1280), js_ler_dimensao_tela(1, 800));
// and calls simulador.Executa(<main lambda>) (table slot 128). Executa registers the platform singletons
// and then runs the lambda (which installs CWasmWebSound and emits "vota:ready").
//
// Func 8302 is simulador::CSimuladorWasm::Executa (name inferred). It was formerly shown by the tools as
// "api::CPolySingletonList::push@8302", because 12 replace<T>/push<T> bodies are inlined into it (28 435 bytes,
// the largest function of unit u19). Every registration below goes through the
// register-or-replace wrapper CPolySingletonList::replace (cpolysingletonlist.h): "if (exists) erase", then
// push. ISound calls the out-of-line push 5384 after the inlined prefix; ITextToSpeech goes through the by-value
// helper 4162 (-> replace 7667); ISystemDateTime through 4890 (helper + replace + push inlined).
#include <cstdlib>
#include <filesystem>
#include <functional>
#include <memory>
#include <string>

#include "api/pattern/cpolysingletonlist.h"
#include "api/teste/cpowermock.h"
#include "api/teste/curnamock.h"
#include "api/util/itimerscheduler.h"
#include "comum/cpath.h"
#include "simulador/wasm/cwasm_all.h"   // CWasmInit, CWasmScreen, CWasmScreenMT, CWasmInputKbd, ... (?)

namespace simulador {

class CSimuladorWasm {                                                       // name inferred
public:
    // wasm func 8311
    CSimuladorWasm(int parametroInit, std::string titulo, short largura, short altura)
        : m_parametroInit(parametroInit), m_titulo(std::move(titulo)), m_largura(largura), m_altura(altura) {}

    void Executa(const std::function<void()>& principal);                   // wasm func 8302  name inferred

private:
    int         m_parametroInit;   // +0  = 10; handed to CWasmInit (+24), returned by IInterfaceInit slot 4  (?)
    std::string m_titulo;          // +4  "VOTA Web" (not used by Executa)
    short       m_largura;         // +16 canvas width  (1280 by default)
    short       m_altura;          // +18 canvas height (800 by default)
};

// wasm func 8302 (table slot 128). Executed once, inside main().
void CSimuladorWasm::Executa(const std::function<void()>& principal)
{
    using api::CPolySingletonList;
    // Every registration below re-reads the registry accessor right after constructing its object, i.e. the
    // `info` argument is the default GetPolySingletonsInfo() of replace().

    // 1. Storage layout of the urna inside MEMFS (comum::CPath statics @1838600 / @1838576 / @1838588):
    comum::CPath::SetRaiz("/");                                    // name inferred
    //    internal flash = raiz / "dsk/fi/", external flash (memory card) = raiz / "dsk/fe/"
    std::filesystem::create_directories("/uenux");
    ::setenv("PROJECT_DIR", ".", 1);
    ::setenv("BUILD_DIR", "/", 1);

    // 2. Platform singletons (the order below is the order of the DEBUG_UENUX trace, sz[13] .. sz[27]).
    api::ITimerScheduler::CreateInst<CWasmTimerScheduler>();       // itimerscheduler.h:40 "Tentativa de recriar
                                                                   // o singleton" (7085); then func 4879
                                                                   // (by-value helper -> replace -> push)
    CPolySingletonList::replace<comum::IInterfaceInit>(std::make_unique<CWasmInit>(m_parametroInit));

    auto tela = std::make_unique<CWasmScreen>(m_largura, m_altura);   // 48 bytes: logical 640x480 (+32/+34),
                                                                      // sx = w/640.0 (+16), sy = h/480.0 (+24)
    //   its constructor calls js_init(w, h), Clear(1) (vtable slot 4, func 9254) and logs through js_log:
    //   "CWasmScreen this=" + std::to_string(reinterpret_cast<std::uintptr_t>(this))   (a wasm address)
    CPolySingletonList::replace<api::IScreen>(std::move(tela));
    CPolySingletonList::replace<api::IScreenMT>(std::make_unique<CWasmScreenMT>());   // 4 lines = std::string(40, ' ')
    CPolySingletonList::replace<api::IInputKbd>(std::make_unique<CWasmInputKbd>());
    CPolySingletonList::replace<api::IInputMT>(std::make_unique<CWasmInputMT>());
    CPolySingletonList::replace<api::IResource>(std::make_unique<CWasmResource>());
    CPolySingletonList::replace<api::IUrna>(std::make_unique<api::teste::CUrnaMock>());
    //   CUrnaMock (28 bytes): +4 = 2020 (model year, IUrna slot 0; callers test "<= 2019"),
    //   +8 = 87654321 (serial?), +12 = 255; its slot 6 fills the 128-byte RDV key table with 0x03 (u01/u02)
    CPolySingletonList::replace<api::IPower>(std::make_unique<api::teste::CPowerMock>());
    //   CPowerMock (48 bytes): status {0x840, 2, 0, 100} at +20 (mains, battery full -> icon key 0),
    //   +40 = std::chrono::steady_clock::now()
    CPolySingletonList::replace<api::IBeep>(std::make_unique<CWasmBeep>());   // the CWasmBeep constructor
                                                                                   // calls js_wasm_beep_init()
    CPolySingletonList::replace<api::ISound>(std::make_unique<CWasmNullSound>(9));   // replaced later by
                                                                                        // CWasmWebSound (main lambda)
    CPolySingletonList::replace<api::IPaperRelatorios>(std::make_unique<CWasmNullPaper>());
    CPolySingletonList::replace<api::IImpressoraRelatorios>(std::make_unique<CWasmNullPrinter>());
    CPolySingletonList::replace<api::ITextToSpeech>(std::make_unique<CWasmNullTextToSpeech>());   // func 4162;
                                                                   // rate 100 %, level 2, params 50/100; replaced
                                                                   // again by votaInit (RHVoice or Null)
    CPolySingletonList::replace<api::ISystemDateTime>(std::make_unique<CWasmSystemDateTime>());   // func 4890:
                                                                   // REPLACES the api::CSystemDateTime created by
                                                                   // the first ISystemDateTime::GetInst()
    CPolySingletonList::replace<api::CEscritorLog>(
        std::make_unique<CWasmLogd>(comum::CPath::GetPathLog() / "logd.dat"));   // wasm_entry_f5899

    // 3. Run the application's main lambda.
    principal();                                                   // std::bad_function_call if empty
}

}  // namespace simulador
