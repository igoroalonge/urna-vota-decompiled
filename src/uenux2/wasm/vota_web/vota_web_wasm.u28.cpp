// FRAGMENT of uenux2/wasm/vota_web/vota_web_wasm.cpp reconstructed by unit u28 from vota_web_wasm.wasm.
// (The rest of this file - CVotaWebEngine, main, votaInit/votaTick/..., CSincronismoVotoEleitorWeb - belongs to
//  units u29 and u30: vota_web_wasm.u29.cpp and vota_web_wasm.u30.cpp.)
//
// vota_web_wasm.cpp is the web entry point of the simulator; the urna has no such file. Besides the C API it
// defines, in an anonymous namespace, the web replacements of three "policy" singletons of the voting
// application. main() (wasm func 10307) registers them in api::CPolySingletonList before anything else runs:
//
//     api::CPolySingletonList::replace<comum::IInterfaceSavd>(std::make_unique<CWasmSavd>(), info);    // 10090
//     ... IRng, ISymmetricCipherFactory, IFingerPrepare, IAjusteDataHora, thread/mutex factories ...
//     replace<vota::impl::IPoliticaExecucaoEleitor>(std::make_unique<CPoliticaExecucaoEleitorWeb>(), info);  // 9507
//     replace<vota::impl::ISincronismoVotoEleitor>(std::make_unique<CSincronismoVotoEleitorWeb>(), info);     // 9437
// (replace = the register-or-replace wrapper "if (exists) erase; push", see main in vota_web_wasm.u30.cpp)
//
// Anonymous-namespace classes have internal linkage, so the classes below live in the same translation unit
// as main(), which stores their vtables: that is why CWasmSavd is placed in this file (path inferred for it;
// CPoliticaExecucaoEleitorWeb is attested by srcloc vota_web_wasm.cpp:361).
//
// RTTI:
//   comum::IInterfaceSavd (vtable @1526736)
//     └ (anonymous namespace)::CWasmSavd (typeinfo @1526708, vtable @1526688; 20 bytes = the base's members only)
//   vota::impl::IPoliticaExecucaoEleitor
//     ├ vota::impl::CPoliticaExecucaoEleitor              (urna implementation, cpoliticaexecucaoeleitor.cpp)
//     └ (anonymous namespace)::CPoliticaExecucaoEleitorWeb (typeinfo @1527168, vtable @1527156; 4 bytes, no data)

#include <cstddef>
#include <source_location>
#include <stdexcept>
#include <vector>

#include "api/hwil/iinputkbd.h"                                // api::IInputKbd : api::IInput (slot 4 = Flush)
#include "api/pattern/cpolysingleton.h"
#include "comum/iinterfacesavd.h"
#include "vota/eleitor/comum/cpoliticaexecucaoeleitor.h"      // vota::impl::IPoliticaExecucaoEleitor

namespace {

// =====================================================================================================
// CWasmSavd - the SAVD (signing / signature-validation service of the urna) of the web build.
//
// comum::IInterfaceSavd talks to the service with a binary request/answer protocol (see
// src/uenux2/src/app/comum/iinterfacesavd.h): the client sends an 8-byte header + payload through
// EnviaMensagem (slot 2) and reads a 12-byte answer header through RecebeMensagem (slot 3); only when the
// header says "erro" does it read a second, variable-size message with the error text.
//
// This mock never looks at the request. It answers every request with the 12-byte header
// {0xFE, erro = 0, 0, 0, codigo = 0, tamanho = 0}: "OK, no message". Hence every AssinarUE / AssinarEcourna
// (sign a file), EnviarAcaoHSM (open/close the HSM session) and ValidarUE (check a package signature) of the
// application succeeds without any cryptography, and no signature file is produced by the service (the .vsu
// files of the simulator are the fixed text "assinatura simulada para vota_web_wasm" written by votaInit).
// =====================================================================================================
class CWasmSavd final : public comum::IInterfaceSavd {
public:
    // slot 0: inherited ~IInterfaceSavd (func 5503); slot 1: deleting dtor (func 10965, unit u23).

    // slot 2 - ICF body 425 (empty function shared with many other no-op virtuals): the request is dropped.
    void EnviaMensagem(const std::vector<uebyte>& /*dados*/) override {}

    void RecebeMensagem(ueint32& estado, std::vector<uebyte>& dados, std::size_t tamanho) override;

    ueint32 Slot4() override;                                      // name unknown (see below)
};

// Value written by RecebeMensagem into its out-parameter and returned by slot 4. Reads as hex-speak
// "0CABECA0" ("cabeça", Portuguese for "head"): a recognisable fake identifier.                       // ?
constexpr ueint32 IDENTIFICADOR_SIMULADO = 0x0CABECA0;                                       // name inferred

// wasm func 10949 - vtable slot 3 (table 311). Observed executing, but only inside votaInit: the SAVD exchanges
// of the start-up are SalvaEstado (func 491) -> AssinarUE (1277) -> 5891 -> 3829 (signing) and the package check
// CPacoteArquivos::ValidarChaveEAplicacaoValida (4625) -> 5892 -> 3830. A CPU-profile timeline of a complete
// municipal vote shows no SAVD call after votaInit: the web CSincronismoVotoEleitorWeb writes neither vota.bin
// nor rdv.dat, so nothing is "signed" when a vote is confirmed.
//   estado  : out-parameter of the interface; the client (IInterfaceSavd::EnviaRequisicao, func 3830, and
//             EnviaComando, func 3829) passes an uninitialised local and never reads it. What it carries on the
//             real urna (message type / sequence / channel id) is not visible in this build.            // ?
//   dados   : receives the answer bytes (vector<uebyte>::assign, func 5496)
//   tamanho : number of bytes the client wants: 12 for the header; anything else is rejected.
void CWasmSavd::RecebeMensagem(ueint32& estado, std::vector<uebyte>& dados, std::size_t tamanho)
{
    estado = IDENTIFICADOR_SIMULADO;                          // written before the size check
    if (tamanho != sizeof(comum::ueRespostaSavd))             // 12
        throw std::invalid_argument("CWasmSavd::RecebeMensagem - mensagem de tamanho invalido");

    comum::ueRespostaSavd resposta{};                         // 12 zero bytes: erro 0, codigo 0, tamanho 0
    resposta.marca = 0xFE;
    const auto* bytes = reinterpret_cast<const uebyte*>(&resposta);
    dados.assign(bytes, bytes + sizeof(resposta));
}

// wasm func 10930 - vtable slot 4 (table 312), pure virtual in comum::IInterfaceSavd. No call through slot 4
// of an IInterfaceSavd was found in the binary (the SAVD client code reads slots 2 and 3 only), so the
// meaning of this method is unknown; it returns the same constant that RecebeMensagem stores in `estado`,
// which suggests an identifier of the client / channel (e.g. the message type the client expects).  // ?
ueint32 CWasmSavd::Slot4()
{
    return IDENTIFICADOR_SIMULADO;
}

// =====================================================================================================
// CPoliticaExecucaoEleitorWeb - execution policy of the voter thread in the web build.
//
// The only policy method is LimpaBufferInput(): flush the voter keypad. The voting application calls it
// through IPoliticaExecucaoEleitor::GetInst() when the voter presses CORRIGE on a confirmation screen
// (vota::CConfirmaVotoNominal slot 17 <- CVotacaoStateAudio::EmiteEcoCorrigeConfirma; verified with a stack
// trace of the import wasm_input_clear under tools/run/headless.mjs --named, keys "91001  D  ").
//
// The urna implementation (vota::impl::CPoliticaExecucaoEleitor::LimpaBufferInput, func 13564, unit u06)
// draws two IRng::Gera() values: n = Gera() % 4 + 3 flushes and ONE delay (Gera() % 100 + 50) & 0xFF ms, then
// does "flush; sleep(delay)" n times. With non-negative random numbers that is 3..6 flushes of 50..149 ms each,
// i.e. the voter thread pauses ~0.15-0.9 s after CORRIGE (the remainders are i32.rem_s: a negative Gera() gives
// 0..2 passes). Its std::this_thread::sleep_for compiles to `if (byte@1584624 == 1) emscripten_sleep(ms)`; the
// byte is 1 and emscripten_sleep aborts the module without ASYNCIFY - hence this replacement: a single flush,
// no wait (the next screen's CInteractiveForm::ClearKeyboardInput flushes once more when it starts).
// =====================================================================================================
class CPoliticaExecucaoEleitorWeb final : public vota::impl::IPoliticaExecucaoEleitor {
public:
    // slot 0: trivial destructor (ICF 174 "return this"); slot 1: operator delete (ICF 144).

    // wasm func 10835 - vtable slot 2 (table 339). srcloc vota_web_wasm.cpp:361 (column 46: the lookup is
    // on the method's own line, consistent with an in-class one-line definition).                      // ?
    void LimpaBufferInput() const override
    {
        // CPolySingleton<IInputKbd>::instance (func 455) with the caller's source_location (:361) for the
        // "instância não criada" error message; the registry accessor is the function pointer @1526320.
        api::CPolySingleton<api::IInputKbd>::instance(api::GetPolySingletonsInfo(),
                                                      std::source_location::current())      // :361
            .Flush();   // IInput slot 4 -> simulador::CWasmInputKbd (ICF func 5013) -> JS wasm_input_clear():
                        // Module.uenuxKeys = []  (drops every key queued by votaPressKey)
    }
};

}  // namespace
