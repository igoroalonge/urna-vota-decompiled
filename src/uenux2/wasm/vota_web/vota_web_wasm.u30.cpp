// FRAGMENT of uenux2/wasm/vota_web/vota_web_wasm.cpp reconstructed by unit u30 from vota_web_wasm.wasm.
//
// Split of this file between units (see the other fragments in this directory):
//   u28  CWasmSavd, CPoliticaExecucaoEleitorWeb                         (vota_web_wasm.u28.cpp)
//   u29  CVotaWebEngine (layout, GetInst, Init = votaInit, BuildStateJson), EmitEvent, CurrentStateName,
//        EscapeJson                                                     (vota_web_wasm.u29.cpp)
//   u30  main, votaTick, votaPressKey, votaGetStateJson, votaSetAudioEnabled, ReportError, the three JSON
//        option readers, WriteFileIfMissing, WriteSimulatedSignatures, CSincronismoVotoEleitorWeb   (this file)
//
// Evidence: export names of the Emscripten glue (main = "Kb", votaTick = "Hb", votaPressKey = "Gb",
// votaGetStateJson = "Jb", votaSetAudioEnabled = "Ib") and the srcloc record
//   vota_web_wasm.cpp:592 col 9  void (anonymous namespace)::CVotaWebEngine::SetAudioEnabled(bool)
// (the only srcloc inside this unit's functions). Every other name is inferred and follows the English style
// of the attested ones (Init, SetAudioEnabled) and of unit u29's fragment.
//
// This file does not exist on the urna. It is the browser entry point of the TSE training simulator: it
// replaces the urna's process start-up (main) and turns the urna's multi-threaded, blocking event loops into a
// C API that JavaScript drives once per animation frame (votaTick) - see docs/03-js-wasm-interface.md.

#include <emscripten.h>

#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <memory>
#include <source_location>
#include <string>
#include <vector>

#include "api/audio/isound.h"
#include "api/gui/capplication.h"
#include "api/pattern/cpolysingleton.h"
#include "api/pattern/cpolysingletonlist.h"
#include "api/pattern/igenericfactory.h"
#include "api/ipc/posix/cposixmutex.h"
#include "api/ipc/posix/cposixrwmutex.h"
#include "api/ipc/posix/cposixsemaphore.h"
#include "api/util/iajustedatahora.h"
#include "comum/cpath.h"
#include "comum/iinterfacesavd.h"
#include "ecourna/api/security/cprng.hpp"
#include "ecourna/api/security/csymmetriccipherfactory.hpp"
#include "simulador/wasm/cfingerpreparesimulador.h"
#include "simulador/wasm/csimuladorwasm.h"
#include "simulador/wasm/cwasmthread.h"
#include "simulador/wasm/cwasmwebsound.h"
#include "vota/cexecucaovotacooperativa.h"
#include "vota/eleitor/caguardamensagem.h"
#include "vota/eleitor/comum/ctelasvota.h"
#include "vota/eleitor/csincronismovotoeleitor.h"   // vota::impl::ISincronismoVotoEleitor
#include "vota/eleitor/cthreadeleitor.h"
#include "vota/iexecucaovota.h"

extern "C" {
// TSE's own imports, implemented in upstream/site/wasm/vota_web_wasm.js.
void js_use_external_keyboard();                         // Module.uenuxExternalKeyboard = true
// "read screen dimension": indice 0 = width, 1 = height. JS takes, in this order, ?screenWidth= / ?screenHeight=,
// ?screen=|?resolution=|?resolucao=WxH from the page URL, Module.votaScreenWidth/Height, globalThis.*, else the
// default. Any positive finite number is accepted (no upper bound); the result is converted to i32 by the wasm
// ABI and then narrowed to 16 bits by main (see below).
int  js_ler_dimensao_tela(int indice, int valorPadrao);
}

namespace {

// ---------------------------------------------------------------------------------------------------------
// Declared/defined by unit u29 (vota_web_wasm.u29.cpp), used here:
//   class CVotaWebEngine;                    // 40 bytes, singleton; GetInst() = wasm func 2094
//     bool m_initialized (+0), m_done (+1), m_audioEnabled (+2), m_audioEleitorHabilitado (+3),
//          m_recordingStarted (+4), m_recordingFinished (+5); int m_recordingStep (+8);
//     double m_recordingDeadline (+16); std::string m_lastJson (+24)
//     std::string BuildStateJson() const;    // wasm func 5500
//   void EmitEvent(const char* nome, const std::string& json);        // wasm func 3533 -> js_emit_event
//   std::string CurrentStateName();                                    // wasm func 5408 (RTTI name, demangled)
//   std::string EscapeJson(const std::string& texto);                  // wasm func 9640
// Simulator helpers (tools: simulador_f9646 / simulador_f9649, unit u29):
//   void ConsoleError(const std::string& texto);   // js_console_error(texto.c_str())  -> console.error
//   void PushKey(const std::string& tecla);        // js_push_key(tecla.c_str())       -> Module.uenuxKeys.push
// ---------------------------------------------------------------------------------------------------------

// =========================================================================================================
// CSincronismoVotoEleitorWeb - the web "vote synchronisation" policy (name attested by RTTI).
// RTTI: vota::impl::ISincronismoVotoEleitor <- (anonymous namespace)::CSincronismoVotoEleitorWeb
//       (typeinfo @1527264, vtable @1527252): [0] trivial destructor (ICF 174 "return this"),
//       [1] deleting destructor (ICF 144 = operator delete), [2] SincronizaVoto (ICF 434 "return 1").
// 4 bytes (vptr only): main (func 10307) does operator new(4), stores the vtable and registers it with the
// replace wrapper 9437 (-> push 5398), before IExecucaoVota. It is the only place that stores the vtable.
//
// On the urna, vota::impl::CSincronismoVotoEleitor::SincronizaVoto (func 7174) writes and signs the RDV and the
// state on both flash memories when CSincronismoEleitor receives message 5. This replacement writes nothing
// and reports success, so no vote reaches rdv.dat in the simulator (analysis/runtime/README.md).
// =========================================================================================================
class CSincronismoVotoEleitorWeb final : public vota::impl::ISincronismoVotoEleitor {
public:
    bool SincronizaVoto() override { return true; }      // slot 2 (table slot 342) -> ICF body func 434
};

// =========================================================================================================
// Option JSON readers used by CVotaWebEngine::Init (votaInit). Not a JSON parser: each reader looks for the
// FIRST occurrence of the text "\"<chave>\"" anywhere in the document (inside string values and nested
// objects too), then for the next ':' after it, skips " \t\r\n" (literal @450169) and reads the value there.
// The quote literal @439304 is "\"". The key search is std::string::find(const std::string&) (func 3355),
// the ':' search std::string::find(char, pos) (func 3325, constant-propagated to ':').
// =========================================================================================================

// wasm func 14477 (table slot 4) - observed executing                                    name inferred
bool JsonBool(const char* json, const std::string& chave, bool padrao)
{
    if (json == nullptr)
        return padrao;
    const std::string texto(json);
    auto pos = texto.find("\"" + chave + "\"");
    if (pos == std::string::npos)
        return padrao;
    pos = texto.find(':', pos);
    if (pos == std::string::npos)
        return padrao;
    pos = texto.find_first_not_of(" \t\r\n", pos + 1);
    if (pos == std::string::npos)
        return padrao;
    if (texto.compare(pos, 4, "true") == 0)
        return true;
    if (texto.compare(pos, 5, "false") == 0)
        return false;
    return padrao;                                  // any other token (e.g. 1, "yes") keeps the default
}

// wasm func 11571 (table slot 22) - observed executing                                   name inferred
// strtol base 10. The default 1 is a constant in the body: every caller (pe, municipio, zona, secao, turno)
// passes 1, so a `padrao` parameter may have been constant-propagated away.                           // ?
int JsonInt(const char* json, const std::string& chave /*, int padrao = 1 */)
{
    constexpr int padrao = 1;
    if (json == nullptr)
        return padrao;
    const std::string texto(json);
    auto pos = texto.find("\"" + chave + "\"");
    if (pos == std::string::npos)
        return padrao;
    pos = texto.find(':', pos);
    if (pos == std::string::npos)
        return padrao;
    pos = texto.find_first_not_of(" \t\r\n", pos + 1);
    if (pos == std::string::npos)
        return padrao;
    const char* inicio = texto.c_str() + pos;
    char* fim = nullptr;
    const long valor = std::strtol(inicio, &fim, 10);   // long is 32 bits on wasm32: saturates at +-2^31
    return fim == inicio ? padrao : static_cast<int>(valor);    // no range check; negative numbers accepted
}

// wasm func 11546 (table slot 23)                                                         name inferred
// Returns the text between the quotes. A backslash makes the next character literal (so \" and \\ work) but
// JSON escapes are NOT decoded: "\n" gives 'n', "\u00e9" gives "u00e9". An unterminated string, a missing
// key or a non-string value gives the default.
std::string JsonString(const char* json, const std::string& chave, const std::string& padrao)
{
    if (json == nullptr)
        return padrao;
    const std::string texto(json);
    auto pos = texto.find("\"" + chave + "\"");
    if (pos == std::string::npos)
        return padrao;
    pos = texto.find(':', pos);
    if (pos == std::string::npos)
        return padrao;
    pos = texto.find_first_not_of(" \t\r\n", pos + 1);
    if (pos == std::string::npos || texto[pos] != '"')
        return padrao;

    std::string valor;
    bool escapado = false;
    for (std::size_t i = pos + 1; i < texto.size(); ++i) {
        const char c = texto[i];
        if (!escapado) {
            if (c == '\\') {
                escapado = true;
                continue;
            }
            if (c == '"')
                return valor;                        // moved out
        }
        escapado = false;
        valor.push_back(c);
    }
    return padrao;
}

// =========================================================================================================
// wasm func 11818 (table slot 17) - observed executing                                   name inferred
// Creates the file with this content only if nothing exists at that path. Used for serialv.dat ("ABCDDCBA",
// by Init) and for the simulated signature files below. std::filesystem::exists = func 5836,
// parent_path = func 3706, create_directories = func 12121 (inline wrapper of __create_directories).
// The std::ofstream is opened with mode 16 = ios::out (libc++ values: out 0x10, trunc 0x20, binary 0x04).
// =========================================================================================================
void WriteFileIfMissing(const std::filesystem::path& arquivo, const std::string& conteudo)
{
    if (std::filesystem::exists(arquivo))
        return;
    std::filesystem::create_directories(arquivo.parent_path());
    std::ofstream saida(arquivo.c_str());             // basic_filebuf::open(const char*, ios::out)
    saida << conteudo;                                // errors are not checked (no exceptions() mask)
}

// =========================================================================================================
// wasm func 11733 (table slot 18) - observed executing                                   name inferred
// Called 4 times by votaInit (after the storage tree, after the fixture, after the voter start-up and after
// SalvaEstado). Writes the literal text below as the "signature" of the state files - the urna's signature
// packages (.vsu, produced by the SAVD/HSM on a real urna) are simulated:
//   <MI|MV>/dinamico/eg.vsu
//   <MI|MV>/dinamico/trab1|trab2/{eg,gap,sa,vota,rdv,uenux}.vsu
// Existing files are left alone (WriteFileIfMissing), so these are write-once placeholders; nothing ever
// verifies them in this build (CWasmSavd answers "OK" to every ValidarUE, unit u28).
// Loop constants: {0, 1} = EFlashOrigem INTERNA/EXTERNA (i64 0x1_00000000 at sp+48), {'1', '2'} = turnos
// (i64 0x32_00000031 at sp+16). The file names are built with std::vector<std::string>(initializer_list)
// (func 11332) and std::filesystem::path(const char(&)[N]) (func 11637, "eg.vsu") / path(const std::string&)
// (func 5641); '/' is func 5968.
// =========================================================================================================
void WriteSimulatedSignatures()
{
    const std::string conteudo = "assinatura simulada para vota_web_wasm";          // @149594
    const std::vector<std::string> assinaturas = {"eg.vsu", "gap.vsu", "sa.vsu",
                                                  "vota.vsu", "rdv.vsu", "uenux.vsu"};

    for (const auto flash : {comum::EFlashOrigem::INTERNA, comum::EFlashOrigem::EXTERNA}) {
        WriteFileIfMissing(comum::CPath::GetPathDinamico(flash) / "eg.vsu", conteudo);           // func 1082
        for (const auto turno : {comum::EUrnaTurno::PRIMEIRO, comum::EUrnaTurno::SEGUNDO}) {
            const std::filesystem::path trab = comum::CPath::GetPathTrab(flash, turno);          // func 358
            for (const auto& nome : assinaturas)
                WriteFileIfMissing(trab / nome, conteudo);
        }
    }
}

// =========================================================================================================
// wasm func 10857 (table slot 60)                                                         name inferred
// Error path of votaInit and votaTick: the text goes to the browser console and to the page as a DOM event
// "vota:error" with detail {"message":"<escaped text>"} (the adapter then stops ticking and shows "Erro: ...").
// =========================================================================================================
void ReportError(const std::string& texto)
{
    const std::string json = "{\"message\":\"" + EscapeJson(texto) + "\"}";   // @439251, EscapeJson 9640, @9024
    ConsoleError(texto);                                                       // func 9646 -> js_console_error
    EmitEvent("vota:error", json);                                             // func 3533
}

// =========================================================================================================
// wasm func 11661 (table slot 19)                                                         name inferred
// One-line out-of-line thunk: CPath::GetPathDinamico(func 1082) with the flash constant 0. Only votaInit
// calls it, to test "<MI>/dinamico/eg.bin exists?". wasm-opt's merge-similar-functions leaves exactly this
// shape for a call site whose constant argument was specialised by LLVM, so in the source it is most likely
// just the expression below written inside Init.                                                  // ?
// =========================================================================================================
std::filesystem::path PathDinamicoInterno()
{
    return comum::CPath::GetPathDinamico(comum::EFlashOrigem::INTERNA);
}

// =========================================================================================================
// Messages to the voter thread's queue. api_f3903(fila, codigo) builds {.., uint16 codigo} and pushes it with
// priority 1 (func 7708 -> CPriorityMessageQueue push 501). The binary has one 9-byte thunk per constant,
// same shape as PathDinamicoInterno above (call-site specialisation, then merged back):         // ?
//   wasm func 11026 (slot 57)  code 1: "Eleitor foi habilitado" - releases the urna for one voter (Init)
//   wasm func 11100 (slot 55)  code 9: voter audio mode (Init, when audioEleitorHabilitado)
//   wasm func 10376 (slot 110) code 5: "vote synchronised" - ends CSincronismoEleitor (votaTick, below)
// Names of the codes inferred (u29 calls 1 and 9 MSG_INICIA_ELEITOR and MSG_AUDIO_HABILITADO; on the urna
// code 5 comes from CSincronismoOperador, u07 §4.1). Written as one template so that each wasm thunk has a
// source counterpart; in the original they are probably plain `fila.Envia(N)` calls.                   // ?
// =========================================================================================================
template <int MENSAGEM>                                                                  // name inferred
void EnviaMensagemEleitor(vota::CFilaMensagensEleitor& fila)   // = IExecucaoVota::GetFilaEleitor() (slot 8)
{
    fila.Envia(MENSAGEM);                                       // api_f3903(fila, MENSAGEM): priority 1
}
// wasm func 11026: EnviaMensagemEleitor<1>   (votaInit, after the voter-side start-up)
// wasm func 11100: EnviaMensagemEleitor<9>   (votaInit, only when audioEleitorHabilitado)
// wasm func 10376: EnviaMensagemEleitor<5>   (votaTick, end of the "Gravando" animation)

}  // namespace

// =========================================================================================================
// CVotaWebEngine::Tick, inlined into the export votaTick = wasm func 10619 (export "Hb").   name inferred
// Observed executing (thousands of calls per vote: the page calls it on every requestAnimationFrame and after
// every key).
//
// Returns 1 while the session runs, 0 once it is over (m_done) or before votaInit succeeded, and 0 after an
// exception (reported as "vota:error").
//
// The web build replaces the urna's operator thread for one thing only: while the voter thread shows the
// "Gravando" (recording) screen of vota::CSincronismoEleitor, this function advances the 4-step progress bar
// on a timer and then posts message 5, as CSincronismoOperador would after writing the RDV on a real urna.
// Nothing is written: SincronizaVoto is CSincronismoVotoEleitorWeb (returns true). The bar is a pure
// animation: +120 ms, +280, +440, +600, then message 5 at +760 ms (f64 constants 120.0 and 160.0).
// =========================================================================================================
int CVotaWebEngine::Tick()
{
    if (!m_initialized || m_done)
        return 0;
    try {
        const std::string antes = CurrentStateName();                  // func 5408
        vota::IExecucaoVota::GetInst().Processa();                     // func 3594, slot 7 (7823): one step of
                                                                       // CThreadEleitor (messages, keys, ticks)
        const std::string depois = CurrentStateName();

        if (depois.find("CSincronismoEleitor") != std::string::npos) {                 // @83021
            const double agora = emscripten_get_now();                 // invoke_d(107): slot 107 IS the import
            if (!m_recordingStarted) {
                m_recordingStarted = true;                             // one i16 store of 1 at +4
                m_recordingFinished = false;
                m_recordingStep = 0;
                m_recordingDeadline = agora + 120.0;
            } else if (!m_recordingFinished && agora >= m_recordingDeadline) {
                if (m_recordingStep <= 3) {
                    vota::CTelasVota::GetInst().AvancaBarraProgresso();                 // funcs 407, 2369
                    m_recordingDeadline = agora + 160.0;
                    ++m_recordingStep;
                } else {
                    EnviaMensagemEleitor<5>(vota::IExecucaoVota::GetInst().GetFilaEleitor());   // 10376
                    m_recordingFinished = true;
                }
            }
        } else {
            m_recordingDeadline = 0.0;
            m_recordingStep = 0;
            m_recordingStarted = m_recordingFinished = false;
        }

        // Back to the idle state = the voter's session is over. Any path back counts: a completed vote
        // (CFimVotoEleitor -> CAguardaMensagem) or a discarded one (DescartaVotos -> CAguardaMensagem).
        if (depois.find("CAguardaMensagem") != std::string::npos && antes != depois)   // @151396
            m_done = true;

        const std::string json = BuildStateJson();                      // func 5500
        if (antes != depois || json != m_lastJson) {
            m_lastJson = json;                                          // copy-assign (func 1700)
            EmitEvent("vota:state", m_lastJson);                        // @175093
        }
        if (m_done && depois.find("CAguardaMensagem") != std::string::npos && antes != depois)
            EmitEvent("vota:done", BuildStateJson());                   // @181476 (same text as the state)
        return 1;
    } catch (const std::exception& e) {
        ReportError(e.what());
        return 0;
    } catch (...) {
        ReportError("erro desconhecido em votaTick");                   // @158360
        return 0;
    }
}

// srcloc vota_web_wasm.cpp:592 (the source_location passed to CPolySingleton<ISound>::instance).
// Inlined into the export votaSetAudioEnabled (wasm func 10240).
void CVotaWebEngine::SetAudioEnabled(bool habilitado)
{
    m_audioEnabled = habilitado;
    api::CPolySingleton<api::ISound>::instance(api::GetPolySingletonsInfo(),        // func 1091
                                               std::source_location::current())      // :592
        .Mute(!habilitado);                    // ISound slot 6; CWasmWebSound::vf6 (9541) -> js_wasm_web_sound_mute
}

// =========================================================================================================
// The C API. EMSCRIPTEN_KEEPALIVE exports; JavaScript calls them through Module.ccall.
// =========================================================================================================

// wasm func 10619 - export "Hb" (Module._votaTick)
extern "C" EMSCRIPTEN_KEEPALIVE int votaTick()
{
    return CVotaWebEngine::GetInst().Tick();
}

// wasm func 10703 - export "Gb" (Module._votaPressKey). Observed executing.
// The key does not go into the wasm keyboard directly: it is pushed back to JavaScript (Module.uenuxKeys) and
// simulador::CWasmInputKbd pops it on a later votaTick (docs/03 §10.2). No check of `initialized` and no
// validation of the text: any string is queued. The JS import wasm_input_get_key hands the wasm only the first
// character of each queued string (Module.uenuxKeys.shift().charCodeAt(0)); CWasmInputKbd decides what it means.
extern "C" EMSCRIPTEN_KEEPALIVE void votaPressKey(const char* tecla)
{
    CVotaWebEngine::GetInst();                   // result unused: only makes sure the engine exists
    if (tecla != nullptr)
        PushKey(std::string(tecla));             // func 9649 -> js_push_key
}

// wasm func 10171 - export "Jb" (Module._votaGetStateJson). Observed executing (headless runner only; the page
// never calls it). SIDE EFFECT: it overwrites m_lastJson, the cache votaTick compares against, so a caller
// that polls between ticks can make votaTick skip the "vota:state" event of a change it has already seen.
// The returned pointer is into the engine's std::string and stays valid until the next call.
extern "C" EMSCRIPTEN_KEEPALIVE const char* votaGetStateJson()
{
    auto& engine = CVotaWebEngine::GetInst();
    engine.m_lastJson = engine.BuildStateJson();          // move-assign (func 707)
    return engine.m_lastJson.c_str();
}

// wasm func 10240 - export "Ib" (Module._votaSetAudioEnabled). Called by the adapter right after votaInit.
extern "C" EMSCRIPTEN_KEEPALIVE void votaSetAudioEnabled(int habilitado)
{
    CVotaWebEngine::GetInst().SetAudioEnabled(habilitado != 0);
}

// =========================================================================================================
// wasm func 10307 - export "Kb" (_main), 2,480 bytes. Observed executing. Called once by the glue's callMain()
// after the main data package is mounted. It builds the process-wide services of the urna application
// (api::CPolySingletonList registry, unit u19) with their WEB implementations, then hands control to the
// simulator object, which registers the mocked hardware and runs the lambda. main returns 0 but the runtime
// stays alive (noExitRuntime): from then on the wasm is driven only by the vota* exports and by timers.
//
// Registration order (verified with DEBUG_UENUX traces by unit u19, §4): IInterfaceSavd, IRng,
// ISymmetricCipherFactory, IFingerPrepare, IAjusteDataHora, the four IGenericFactory, IPoliticaExecucaoEleitor,
// ISincronismoVotoEleitor, IExecucaoVota; then (inside CSimuladorWasm::Executa, func 8302) the hardware mocks;
// then (in the lambda) ISound = CWasmWebSound. Registering the web policies FIRST is what keeps the urna
// defaults (CExecucaoVota with real threads, CPoliticaExecucaoEleitor with emscripten_sleep,
// CSincronismoVotoEleitor that writes the RDV) from ever being created: their GetInst() only create a default
// when nothing is registered.
//
// Any exception escapes main (the landing pads only destroy locals and resume): the glue then aborts.
// =========================================================================================================
int main(int /*argc*/, char** /*argv*/)
{
    js_use_external_keyboard();
    // NB: narrowed to short (i32.extend16_s) - a URL value >= 32768 becomes negative, >= 65536 wraps.
    const auto largura = static_cast<short>(js_ler_dimensao_tela(0, 1280));
    const auto altura  = static_cast<short>(js_ler_dimensao_tela(1, 800));

    // {int 10, std::string "VOTA Web" (moved in), short, short}; ctor = func 8311 (unit u19). The 10 is handed
    // to CWasmInit (IInterfaceInit slot 4).
    simulador::CSimuladorWasm simulador(10, "VOTA Web", largura, altura);                    // @221349

    // Application identity: name, description, version (static strings @0x1C1040.., func 11159).
    api::CApplication::InitApplication("VOTA", "Software de Votação", "10.23.0.1 - DESENVOLVIMENTO", true);

    using api::CPolySingletonList;
    using api::GetPolySingletonsInfo;       // registry accessor, called through the function pointer @1526320
                                            // before every registration (it is re-read each time)

    // Every registration below goes through the register-OR-REPLACE wrapper that unit u19 reconstructs as
    // CPolySingletonList::replace (cpolysingletonlist.h): if (exists<I>) erase(name); push(p). The bodies prove
    // it: 10090, 9507, 9437 and 8728 start with the exists (2744/2736/5434) + erase (func 640) prefix. A bare
    // push<I> would instead THROW 6756 "instância já criada" if I were already registered (u19 §3.4).
    //
    // SAVD (file signing/validation service): answers OK to everything (unit u28). unique_ptr kept on main's
    // stack and passed by reference.
    CPolySingletonList::replace<comum::IInterfaceSavd>(std::make_unique<CWasmSavd>(),            // 10090 (push
                                                       GetPolySingletonsInfo());                 //   inlined)
    // The next eight registrations (IRng ... IGenericFactory<ISemaphore>, and IExecucaoVota below) go through
    // u19's by-value helper f(std::unique_ptr<I> p, info) { replace(std::move(p), info); } - merged body 1562,
    // thunks 9986, 9897, 9831, 9731, 9685, 9650, 9574, 9372 (unique_ptr is [[clang::trivial_abi]], so main
    // passes the raw pointer). Written here as direct replace calls.                                        // ?
    CPolySingletonList::replace<ecourna::api::security::IRng>(                                   // 9986 -> 9233
        std::make_unique<ecourna::api::security::CPrng>(nullptr), GetPolySingletonsInfo());      // ctor 9503(this, 0) ?
    CPolySingletonList::replace<ecourna::api::security::ISymmetricCipherFactory>(                // 9897 -> 9135
        std::make_unique<ecourna::api::security::CSymmetricCipherFactory>(), GetPolySingletonsInfo());
    CPolySingletonList::replace<api::IFingerPrepare>(                                            // 9831 -> 9039
        std::make_unique<simulador::CFingerPrepareSimulador>(), GetPolySingletonsInfo());
    api::IAjusteDataHora::CreateInst();                                                          // func 10876

    // No pthreads in this build: "threads" are cooperative objects, the locks are the POSIX wrappers whose
    // pthread calls are stubs.
    CPolySingletonList::replace<api::IGenericFactory<api::IThreadImpl>>(                         // 9731 -> 8986
        std::make_unique<api::CDefaultGenericFactory<api::IThreadImpl, simulador::CWasmThread>>(),
        GetPolySingletonsInfo());
    CPolySingletonList::replace<api::IGenericFactory<api::ISyncCtl>>(                            // 9685 -> 8911
        std::make_unique<api::CDefaultGenericFactory<api::ISyncCtl, api::CPosixMutex>>(), GetPolySingletonsInfo());
    CPolySingletonList::replace<api::IGenericFactory<api::IRWSyncCtl>>(                          // 9650 -> 8834
        std::make_unique<api::CDefaultGenericFactory<api::IRWSyncCtl, api::CPosixRWMutex>>(),
        GetPolySingletonsInfo());
    CPolySingletonList::replace<api::IGenericFactory<api::ISemaphore>>(                          // 9574 -> 8758
        std::make_unique<api::CDefaultGenericFactory<api::ISemaphore, api::CPosixSemaphore>>(),
        GetPolySingletonsInfo());

    // Web policies of the voting application (anonymous namespace of this file, units u28/u29). unique_ptr kept
    // on main's stack, as for the SAVD.
    CPolySingletonList::replace<vota::impl::IPoliticaExecucaoEleitor>(                           // 9507: exists/
        std::make_unique<CPoliticaExecucaoEleitorWeb>(), GetPolySingletonsInfo());               // erase + push 5407
    CPolySingletonList::replace<vota::impl::ISincronismoVotoEleitor>(                            // 9437: exists/
        std::make_unique<CSincronismoVotoEleitorWeb>(), GetPolySingletonsInfo());                // erase + push 5398
    // The voting "threads" run cooperatively; the voter thread starts in CAguardaMensagem (the urna starts in
    // CAjusteInicial: clock adjustment, result-media checks... none of that runs here).
    CPolySingletonList::replace<vota::IExecucaoVota>(                                            // 9372 -> 1562
        std::make_unique<vota::CExecucaoVotaCooperativa>(vota::CAguardaMensagem::GetInst()),     // 7828, 1337
        GetPolySingletonsInfo());                                                                // -> 8728

    // Hardware mocks + the lambda (func 8302, unit u19). std::function<void()> = __func<main::$_0>
    // (vtable @1527908), built on the stack and destroyed after the call (func 5120).
    simulador.Executa([] {
        // wasm func 10384 = main::$_0::operator() (not in u30). Runs synchronously inside main.
        CPolySingletonList::replace<api::ISound>(std::make_unique<simulador::CWasmWebSound>(),  // {vptr, 9, 0, 0};
                                                 GetPolySingletonsInfo());   // libcxx_f10356 -> push 5384;
                                                                             // replaces CWasmNullSound
        EmitEvent("vota:ready", "{}");       // fires before the page adapter exists (docs/03 §3.3)
    });
    return 0;
}                                                  // ~CSimuladorWasm: its std::string only
