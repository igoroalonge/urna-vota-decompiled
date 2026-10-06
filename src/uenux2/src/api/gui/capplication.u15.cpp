// FRAGMENT reconstructed by unit u15 from vota_web_wasm.wasm.
// Original file: uenux2/src/api/gui/capplication.cpp (srclocs 478 and 503 are in u15; QRCodeSRC::build
// (303), ShowExceptionMsgScreen (377), LogExceptionMsg (432) and ShowExceptionMsg belong to other units).
// Merge into capplication.cpp.
#include <chrono>
#include <locale>
#include <string>
#include <thread>

#include "api/gui/capplicationcontextstack.u15.h"
#include "api/pattern/cpolysingletonlist.h"

enum ELogAplicativos : int;   // which log (logd.dat record type) the application writes

namespace api {

class IBeep;     // vtable slot 1 = Beep(frequencia Hz, duracao in units of 10 ms): the web mock
                 // simulador::CWasmBeep::vf1 (func 8538) calls js_wasm_beep_queue(frequency, durationMs)
                 // with duracao * 10. Slots 7/8 are the destructors.
class IScreen;

// Static members (addresses in linear memory):
//   std::string CApplication::ms_nome        @1839168   e.g. "VOTA"
//   std::string CApplication::ms_descricao   @1839180   e.g. "Software de Votação"
//   std::string CApplication::ms_versao      @1839192   "10.23.0.1 - DESENVOLVIMENTO" (read by the "Versão: {}"
//                                                        line of the urna-state screen, func 3059)
//   ELogAplicativos CApplication::ms_log     @1839204
//   bool CApplication::ms_demonstracao       @1839208   read by the status header (func 5564); NEVER written
//                                                        in this binary -> "DEMONSTRAÇÃO" never shown  // name inferred
//   bool CApplication::ms_encerrar           @1839209   NEVER written in this binary                    // name inferred
//   int  CApplication::ms_codigoSaida        @1577212   initial value -1 (data segment), reset to -1 here // name inferred
class CApplication {
public:
    static void InitApplication(const std::string& nome, const std::string& descricao,
                                const std::string& versao, ELogAplicativos log);
    static void EnterLoopBeeping();
    static void EnterLoopDoingNothing();

    static inline std::string ms_nome, ms_descricao, ms_versao;
    static inline ELogAplicativos ms_log;
    static inline bool ms_demonstracao = false;
    static inline bool ms_encerrar = false;
    static inline int ms_codigoSaida = -1;
};

// wasm func 11159 (observed executing) - called once by main (func 10307)       // name: curated/high
// (the name is the one of the local class's mangled vtable name: InitApplication(const string&, const
// string&, const string&, ELogAplicativos)::commaAsDecimalSeparator)
void CApplication::InitApplication(const std::string& nome, const std::string& descricao,
                                   const std::string& versao, ELogAplicativos log)
{
    ms_nome = nome;
    ms_descricao = descricao;
    ms_versao = versao;
    ms_codigoSaida = -1;
    ms_log = log;

    // Brazilian decimal separator for every stream formatting a number (a local facet class).
    struct commaAsDecimalSeparator : std::numpunct<char> {
        // wasm func 11156: vtable slot 3 of the local class (@1577272) = do_decimal_point
        char do_decimal_point() const override { return ','; }
    };
    // std::locale::global() is inlined: name() != "*"  ->  setlocale(LC_ALL, name().c_str())
    std::locale::global(std::locale(std::locale(), new commaAsDecimalSeparator));

    // Fresh context stack with the generic fallback context (func 5558 + func 3684).
    CApplicationContextStack::GetInst().Clear();
    CApplicationContextStack::GetInst().Push(
        CApplicationContext("Não é possível continuar a execução", "Erro inesperado"));
}

// wasm func 3683 (not observed) - srcloc capplication.cpp:478
// The lambda of EnterLoopBeeping. Binaryen removed its first int parameter (the frequency, always 800 Hz)
// and replaced the closure object by a pointer to the captured `pausa`.
// EnterLoopBeeping itself is inlined into the std::async task of ShowExceptionMsg
// (__async_assoc_state<..., ShowExceptionMsg(...)::$_0>::__execute, func 11151).
// Pattern at 800 Hz: 3 x Beep(800, 10), 3 x Beep(800, 30), 3 x Beep(800, 10) = Morse "S O S". IBeep's
// duration unit is 10 ms (see IBeep above), so these are 100 ms dots and 300 ms dashes (the usual 1:3
// Morse ratio). The waits are raw milliseconds: duracao + 120 ms after each beep (130 / 150 ms), 360 ms
// after each letter and 1160 ms between repetitions.
void CApplication::EnterLoopBeeping()
{
    std::chrono::milliseconds pausa{360};
    auto bipa = [&pausa](int frequencia, int duracao) {                    // func 3683
        for (int i = 0; i < 3; ++i) {
            CPolySingletonList::instance<IBeep>().Beep(frequencia, duracao); // slot 1, srcloc :478
            std::this_thread::sleep_for(std::chrono::milliseconds(duracao + 120));
        }
        std::this_thread::sleep_for(pausa);
    };
    // Body inlined in func 11151:
    do {
        if (CPolySingletonList::exists<IBeep>()) {     // func 3386; with no IBeep this loop spins forever
            bipa(800, 10);
            bipa(800, 30);
            bipa(800, 10);
            std::this_thread::sleep_for(std::chrono::milliseconds(1160));
        }
    } while (!ms_encerrar && ms_codigoSaida <= 0);
}

// wasm func 5566 (not observed) - srcloc capplication.cpp:503
// Where the application parks after a fatal error: keeps the screen alive once per second, forever
// (nothing sets ms_encerrar, and ms_codigoSaida is only ever set to -1, by InitApplication). Callers: vota::CThreadMonitor
// vf3/vf4 (funcs 7710/7709). In the web build every std::this_thread::sleep_for of the application
// became `if (byte@1584624 == 1) emscripten_sleep(ms)`, and the glue's emscripten_sleep aborts
// (no Asyncify): the first iteration of this loop aborts the module.
void CApplication::EnterLoopDoingNothing()
{
    if (ms_encerrar)
        return;
    do {
        if (CPolySingletonList::exists<IScreen>())                         // func 3387
            CPolySingletonList::instance<IScreen>().vf37();                 // IScreen slot 37 (no-op in
                                                                            // CWasmScreen); srcloc :503  ?
        std::this_thread::sleep_for(std::chrono::seconds(1));
    } while (!ms_encerrar && ms_codigoSaida <= 0);
}

} // namespace api
