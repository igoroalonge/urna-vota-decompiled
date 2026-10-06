// FRAGMENT of uenux2/wasm/vota_web/vota_web_wasm.cpp reconstructed by unit u29 from vota_web_wasm.wasm.
// Attested by three std::source_location records of this file, two of them in this class:
//   :515 col 13  int (anonymous namespace)::CVotaWebEngine::Init(const char *)        (inside wasm func 7840)
//   :592 col 9   void (anonymous namespace)::CVotaWebEngine::SetAudioEnabled(bool)    (inside wasm func 10240, u30)
//   (:361 col 46 CPoliticaExecucaoEleitorWeb::LimpaBufferInput() const, func 10835, is unit u28's)
// and by the export names of the Emscripten glue (votaInit, votaTick, votaPressKey, votaGetStateJson,
// votaSetAudioEnabled). The file does not exist on the urna: it is the browser entry point of the simulator.
//
// Split of this file between units:
//   u28  CWasmSavd, CPoliticaExecucaoEleitorWeb (vota_web_wasm.u28.cpp)
//   u29  CVotaWebEngine (layout, GetInst, Init = votaInit, BuildStateJson) and its private helpers (this file)
//   u30  main, votaTick, votaPressKey, votaGetStateJson, votaSetAudioEnabled, ReportError, the JSON option
//        readers, WriteSimulatedSignatures, CSincronismoVotoEleitorWeb
//
// The English method names Init / SetAudioEnabled are attested; the other names in this file follow that style
// and are inferred ("// name inferred").
//
// Everything here runs on the browser's only thread. JavaScript calls the exports through Module.ccall; a C++
// exception never crosses the boundary: every export catches, reports through the "vota:error" DOM event and
// returns 0.

#include <cxxabi.h>
#include <emscripten.h>

#include <cstdint>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iomanip>
#include <memory>
#include <source_location>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <typeinfo>
#include <vector>

#include "api/audio/crhvoicetexttospeech.h"
#include "api/audio/isound.h"
#include "api/audio/itexttospeech.h"
#include "api/pattern/cpolysingletonlist.h"
#include "comum/appinfo/cappinfo.h"
#include "comum/cpath.h"
#include "comum/dados/ccandidaturas.h"
#include "comum/dados/ccargos.h"
#include "comum/dados/md/processoeleitoral/ccargo.h"
#include "comum/salvaestado.h"                                   // comum::SalvaEstado (func 491)    ?
#include "ecourna/api/util/cstringutils.hpp"
#include "mock/app/comum/cappinfobuilder.h"
#include "simulador/wasm/cwasmnulltexttospeech.h"
#include "vota/eleitor/celeitorvotando.h"
#include "vota/eleitor/comum/cinformacaoeleitor.h"
#include "vota/eleitor/cthreadeleitor.h"
#include "vota/eleitor/votaproporcional/cpedeproporcional.h"   // vota::LegendaValida (func 5922)
#include "vota/iexecucaovota.h"
#include "vota/log/clogvota.h"

extern "C" {
// TSE's JavaScript side of the boundary (upstream/site/wasm/vota_web_wasm.js). docs/03-js-wasm-interface.md §12.
void js_emit_event(const char* nome, const char* detalheJson);   // import 96: window.dispatchEvent(CustomEvent)
void js_console_error(const char* texto);                         // import 94: console.error("[vota_web_wasm]", ...)
void js_push_key(const char* tecla);                              // import 95: Module.uenuxKeys.push(...)
}

namespace {

// ================================================================================================
// CVotaWebEngine - the state the C API keeps between calls (40 bytes, operator new(40) in func 2094).
// Offsets from the loads/stores of funcs 2094, 7840, 10240, 10619 and 5500.
// ================================================================================================
class CVotaWebEngine
{
public:
    static CVotaWebEngine& GetInst();                             // wasm func 2094           name inferred

    int  Init(const char* opcoesJson);                            // srcloc :515, inlined in votaInit (7840)
    void SetAudioEnabled(bool habilitado);                        // srcloc :592, votaSetAudioEnabled (10240, u30)
    int  Tick();                                                  // inlined in votaTick (10619, u30)  name inferred
    std::string BuildStateJson() const;                           // wasm func 5500           name inferred

    const std::string& GetLastJson() const { return m_lastJson; }

private:
    bool   m_initialized = false;                // +0  set by Init (i32.store16 of 1 at +0 also clears +1)
    bool   m_done = false;                       // +1  the voter finished (set by votaTick)
    bool   m_audioEnabled = true;                // +2  "reproduzirAudio"; ISound muted when false
    bool   m_audioEleitorHabilitado = false;     // +3  "audioEleitorHabilitado": voter audio (RHVoice)
    bool   m_recordingStarted = false;           // +4  web-only "Gravando..." simulation (votaTick)
    bool   m_recordingFinished = false;          // +5
    int    m_recordingStep = 0;                  // +8  0..4 progress-bar steps
    double m_recordingDeadline = 0.0;            // +16 performance.now() deadline, ms
    std::string m_lastJson = "{}";               // +24 last state JSON sent to the page (literal "{}" @8150)
};                                               // 40 bytes

// Static singleton: std::unique_ptr @1832600. Its out-of-line template members are
//   wasm func 4954  std::unique_ptr<CVotaWebEngine>::reset(CVotaWebEngine*)   (~string at +24, then free)
//   wasm func 5101  std::unique_ptr<CVotaWebEngine>::~unique_ptr()            (= reset(nullptr))
//   wasm func 9832  atexit destructor of s_engine                              (table slot 283)
std::unique_ptr<CVotaWebEngine> s_engine;

// wasm func 2094. Called first by every vota* export (votaPressKey even before checking its argument).
// No mutex is involved (no lock or unlock residue, unlike e.g. CCandidaturas::GetInst, func 521).
CVotaWebEngine& CVotaWebEngine::GetInst()                                         // name inferred
{
    if (!s_engine)
        s_engine = std::make_unique<CVotaWebEngine>();    // new(40), memset 0, +2 = true, +24 = "{}"
    return *s_engine;
}

// ------------------------------------------------------------------------------------------------
// Helpers of this file owned by unit u30 (declared here so that Init reads naturally).    names inferred
// ------------------------------------------------------------------------------------------------
// Hand-written readers of the option JSON (docs/libraries/boost-fmt-and-small-libs.md §7): each one searches
// for "\"" + chave + "\"" (std::string::find, func 3355), then ':', then skips " \t\r\n".
bool        JsonBool(const char* json, const std::string& chave, bool padrao);                 // func 14477
int         JsonInt(const char* json, const std::string& chave);    // func 11571, strtol base 10, default 1
std::string JsonString(const char* json, const std::string& chave, const std::string& padrao); // func 11546
// Creates the file with this content only if it does not exist yet (std::ofstream).             func 11818
void WriteFileIfMissing(const std::filesystem::path& arquivo, const std::string& conteudo);
// Writes uenux.vsu, vota.vsu, rdv.vsu, eg.vsu, gap.vsu and sa.vsu with the literal text
// "assinatura simulada para vota_web_wasm" next to the state files (func 11733, invoked through slot 18).
void WriteSimulatedSignatures();
// std::string("{\"message\":\"") + EscapeJson(texto) + "\"}"; js_console_error(texto) (func 9646);
// EmitEvent("vota:error", ...).                                                                  func 10857
void ReportError(const std::string& texto);
// Not a helper of this file, and not web code: vota::LegendaValida (func 5922, reconstructed in
// vota/eleitor/votaproporcional/cpedeproporcional.cpp, also called by CPedeProporcional::ProcessInputAudio) is
// true if party `partido` exists (comum::CPartidos, func 819) and it or its federação has an apt candidacy for
// `cargo`. BuildStateJson below uses it for "legendaValida".

// ------------------------------------------------------------------------------------------------
// wasm func 3533 (table slot 59). The only caller of the js_emit_event import.            name inferred
// Callers: Init, votaTick, ReportError, main's lambda ("vota:ready", "{}"), CWasmScreen::Refresh (vf27/vf28,
// "vota:screen").
// ------------------------------------------------------------------------------------------------
void EmitEvent(const char* nome, const std::string& json)
{
    js_emit_event(nome, json.c_str());
}

// ------------------------------------------------------------------------------------------------
// Two more one-line forwarders to TSE imports. They exist out of line only because they are called through
// invoke_vi (inside try blocks). The tools put them in the "simulador" component; their only callers are in
// this file, so they are placed here (they could equally be inline helpers of a simulador/wasm header).  ?
//
// wasm func 9646 (table slot 104). Caller: ReportError (func 10857). Prints the RAW text with
// console.error("[vota_web_wasm]", ...): Latin-1 bytes are decoded as UTF-8 by the glue and show as U+FFFD.
// wasm func 9649 (table slot 105). Caller: votaPressKey (func 10703). Module.uenuxKeys.push(tecla); the key
// comes back into the wasm when the application polls the keyboard (CWasmInputKbd: wasm_input_has_key/get_key).
//                                                                                          names inferred
// ------------------------------------------------------------------------------------------------
void ConsoleError(const std::string& texto)
{
    js_console_error(texto.c_str());                  // import 94
}

void PushKey(const std::string& tecla)
{
    js_push_key(tecla.c_str());                       // import 95
}

// ------------------------------------------------------------------------------------------------
// wasm func 5521 (table slot 83). Readable C++ name of a type, e.g. "vota::CPedeNominal" for
// "N4vota12CPedeNominalE". Falls back to the mangled name if the demangler fails.         name inferred
// (__cxa_demangle is called in a specialised form with buf = nullptr, len = nullptr pruned by wasm-opt.)
// ------------------------------------------------------------------------------------------------
std::string DemangleTypeName(const char* nomeMangled)
{
    int status = 0;
    char* legivel = abi::__cxa_demangle(nomeMangled, nullptr, nullptr, &status);
    const char* bruto = nomeMangled ? nomeMangled : "";
    std::string resultado = (status == 0 && legivel != nullptr) ? legivel : bruto;
    std::free(legivel);
    return resultado;
}

// ------------------------------------------------------------------------------------------------
// wasm func 5408 (table slot 106). Name of the current state of the voter thread ("" if none).  name inferred
// IExecucaoVota slot 9 (CExecucaoVotaCooperativa: func 4705) = CThreadEleitor::GetInst().m_pContexto->estado.
// Callers: BuildStateJson and votaTick (before and after each step, to detect a state change).
// ------------------------------------------------------------------------------------------------
std::string CurrentStateName()
{
    const comum::CAppState* estado = vota::IExecucaoVota::GetInst().GetEstadoAtual();   // func 3594, slot 9
    if (estado == nullptr)
        return "";
    return DemangleTypeName(typeid(*estado).name());         // vtable[-1] -> type_info -> +4 name
}

// ------------------------------------------------------------------------------------------------
// wasm func 9640 (table slot 94). JSON string escaper for the state JSON and the vota:error detail.
//                                                                                          name inferred
// Short escapes for " \ \b \t \n \f \r; every other byte < 0x20 as \u00XX (4 hex digits); every byte >= 0x80
// as \u00XX too, i.e. the input is taken as Latin-1 (ISO-8859-1), which is what the election files and the
// application strings are. The output is therefore pure ASCII. (0x7F is copied as is, which JSON allows.)
// ------------------------------------------------------------------------------------------------
std::string EscapeJson(const std::string& texto)
{
    std::ostringstream saida;
    for (const char c : texto) {
        switch (c) {
        case '"':  saida << "\\\""; break;
        case '\\': saida << "\\\\"; break;
        case '\b': saida << "\\b";  break;
        case '\t': saida << "\\t";  break;
        case '\n': saida << "\\n";  break;
        case '\f': saida << "\\f";  break;
        case '\r': saida << "\\r";  break;
        default: {
            const auto byte = static_cast<unsigned char>(c);
            if (byte <= 0x1F)
                saida << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<long>(byte);
            else if (byte >= 0x80)
                saida << "\\u00" << std::hex << std::setw(2) << std::setfill('0') << static_cast<long>(byte);
            else
                saida << c;
        }
        }
    }
    return saida.str();
}

}  // namespace

// Note on two small out-of-line functions the tools attributed to this file: wasm func 2737
// (std::string::find(const char*, size_type = 0), callers BuildStateJson and votaTick) and wasm func 3355
// (std::string::find(const std::string&, size_type = 0), callers the three JSON readers) are libc++
// instantiations; below they are written as plain .find() calls.

namespace {

// ================================================================================================
// wasm func 5500 (table slot 58), 6,871 bytes. The state JSON returned by votaGetStateJson and sent in the
// "vota:state" / "vota:done" events. Callers: Init, votaTick, votaGetStateJson.            name inferred
//
// Every condition below is a substring test on RTTI class names of the voting state machine: the page's
// guide depends on internal C++ class names (see docs/modules/u29-...md, "state JSON").
// ================================================================================================
std::string CVotaWebEngine::BuildStateJson() const
{
    const std::string estado = CurrentStateName();                                  // func 5408

    // Sub-state: only when the voter thread is in vota::CEleitorVotando (a final class: clang turned the
    // dynamic_cast into a compare with vtable @1533152). Its +12 is the per-cargo sub-state.
    std::string subestado;
    const comum::CAppState* atual = vota::IExecucaoVota::GetInst().GetEstadoAtual();  // slot 9
    const auto* votando = dynamic_cast<const vota::CEleitorVotando*>(atual);
    if (votando != nullptr && votando->GetEstadoCargo() != nullptr)                // +12, accessor inlined ?
        subestado = DemangleTypeName(typeid(*votando->GetEstadoCargo()).name());   // func 5521
    else
        subestado = "";

    const std::string tela =
        subestado.find("CInstrucaoVotacaoAcessibilidade") != std::string::npos ? "accessibility" : "";

    // Is the current cargo a party-list (proportional) office? Errors are swallowed.
    bool proporcional = false;
    try {
        auto& cargos = comum::CCargos::GetInst();                                    // func 273
        if (!cargos.IsEnd()) {                                                       // shared_f602
            const comum::md::CCargo& cargo = cargos.GetCurrent();                   // func 332
            proporcional = cargo.TemDetalheCandidato()                               // optional flag +84 ?
                        && cargo.GetTipo() == comum::md::CCargo::ETipo::PROPORCIONAL;   // +4 == 1
        }
    } catch (...) {
    }

    // "legendaValida": the first two typed digits are the number of a party that has candidates for this
    // office. The digits come from the voting code's global vota::g_votoDigitado (@1833288).
    bool legendaValida = false;
    try {
        auto& cargos = comum::CCargos::GetInst();
        if (proporcional && !cargos.IsEnd() && vota::g_votoDigitado.size() >= 2) {
            const auto partido = ecourna::api::util::CStringUtils::ToWord(vota::g_votoDigitado.substr(0, 2));
            legendaValida = vota::LegendaValida(cargos.GetCurrentCargoID(), partido);   // func 5922  name inferred
        }
    } catch (...) {
        legendaValida = false;
    }

    // guide.voteMode - first matching rule wins.
    std::string_view modo;
    if (!m_initialized)
        modo = "inicio";
    else if (m_done)
        modo = "fim";
    else if (m_recordingStarted || estado.find("CSincronismo") != std::string::npos)
        modo = "gravando";
    else if (tela == "accessibility")
        modo = "accessibility";
    else if (subestado.find("Branco") != std::string::npos)
        modo = "branco";
    else if (subestado.find("CPedeNominal") != std::string::npos)
        modo = (legendaValida && proporcional) ? "legendaOuNominal" : "nulo";
    else if (subestado.find("VotoLegenda") != std::string::npos ||
             subestado.find("CandidatoInexistente") != std::string::npos)
        modo = (legendaValida && proporcional) ? "legenda" : "nulo";
    else if (subestado.find("Nulo") != std::string::npos || subestado.find("Inexistente") != std::string::npos ||
             subestado.find("Inapto") != std::string::npos || subestado.find("Repetido") != std::string::npos)
        modo = "nulo";
    else
        modo = "nominal";

    const bool confirmacao = subestado.find("VotoNominal") != std::string::npos ||
                             subestado.find("MajoritarioValido") != std::string::npos;

    auto booleano = [](bool b) { return b ? "true" : "false"; };

    std::ostringstream json;
    json << "{\"initialized\":" << booleano(m_initialized) << ','
         << "\"done\":" << booleano(m_done) << ','
         << "\"audioEnabled\":" << booleano(m_audioEnabled) << ','
         << "\"audioEleitorHabilitado\":" << booleano(m_audioEleitorHabilitado) << ','
         << "\"state\":\"" << EscapeJson(estado) << "\""
         << ",\"guide\":{\"voteMode\":\"" << EscapeJson(std::string(modo)) << "\""
         << ",\"legendaValida\":" << booleano(legendaValida)
         << ",\"confirmacao\":" << booleano(confirmacao) << '}';
    if (!subestado.empty())
        json << ",\"substate\":\"" << EscapeJson(subestado) << "\"";
    if (!tela.empty())
        json << ",\"screen\":\"" << EscapeJson(tela) << "\"";

    try {
        if (!tela.empty()) {
            json << ",\"candidates\":[]";                                  // accessibility screen: no cargo
        } else if (!comum::CCargos::GetInst().IsEnd()) {
            const comum::md::CCargo& cargo = comum::CCargos::GetInst().GetCurrent();
            json << ",\"cargo\":{\"id\":" << static_cast<unsigned>(cargo.GetCodigo()) << ','
                 << "\"name\":\"" << EscapeJson(cargo.GetNome()) << "\","                  // func 1547
                 << "\"digits\":" << static_cast<unsigned>(cargo.GetNumeroDigitos()) << ','   // +12
                 << "\"tipo\":\""
                 << (cargo.TemDetalheCandidato() && cargo.GetTipo() == comum::md::CCargo::ETipo::PROPORCIONAL
                         ? "proporcional" : "majoritario")
                 << "\"";
            if (cargo.TemDetalheCandidato() && cargo.GetTipo() == comum::md::CCargo::ETipo::PROPORCIONAL)
                json << ",\"legendaDigits\":2";                            // constant text @350200
            json << '}';

            json << ",\"candidates\":";
            std::ostringstream lista;
            lista << '[';
            try {
                auto& cargos = comum::CCargos::GetInst();
                if (!cargos.IsEnd()) {
                    const comum::TCargoID idCargo = cargos.GetCurrentCargoID();          // func 1938
                    auto& candidaturas = comum::CCandidaturas::GetInst();               // func 521
                    // Numbers of this cargo's candidaturas whose +72 field is 0 (func 2840) - so every entry
                    // printed below has "apt":true.
                    bool primeiro = true;
                    for (const auto numero : comum::CCandidaturas::GetInst().GetNumerosCandidatos(idCargo)) {
                        const auto it = candidaturas.Find(comum::CCandidaturas::Chave(idCargo, numero));  // 1939,
                        if (it == candidaturas.End())                                                     // 11011
                            continue;
                        const comum::md::CCandidatura& c = it->second;
                        if (!primeiro)
                            lista << ',';
                        lista << "{\"number\":" << c.GetNumero() << ','                  // +24 in the map node
                              << "\"name\":\"" << EscapeJson(c.GetNome()) << "\","      // +40
                              << "\"party\":" << c.GetPartido() << ','                  // u16 +22
                              << "\"apt\":" << booleano(c.GetSituacao() == 0) << '}';   // +72 == 0     ?
                        primeiro = false;
                    }
                }
            } catch (...) {
            }
            lista << ']';
            json << lista.str();
        }
    } catch (...) {
        // NB: if something throws after ",\"cargo\":{" was written, the result is not valid JSON.
        json << ",\"candidates\":[]";
    }
    json << '}';
    return json.str();
}

// ================================================================================================
// CVotaWebEngine::Init (srcloc vota_web_wasm.cpp:515), inlined into the export votaInit (wasm func 7840,
// 6,669 bytes). Returns 1 on success, 0 after reporting the error to the page.
//
// What it does, in order:
//   1. read the two audio flags of the option JSON;
//   2. install the null text-to-speech engine and mute/unmute api::ISound;
//   3. create the urna's storage tree on both flash memories (MI /dsk/fi, MV /dsk/fe) and serialv.dat;
//   4. if dinamico/eg.bin does not exist yet: fabricate the whole persistent state from the options with the
//      test fixture comum::teste::CAppInfoBuilder (eg.bin, trab1|2/{gap,sa,vota}.bin on both flashes);
//   5. load that state, log the start, run the voter-side initialisation of the voting application;
//   6. install the RHVoice engine (voter audio) or the null engine again;
//   7. put the urna in "votar" state (as the mesário's opening of the vote would), save it;
//   8. start the voter thread's state machine and post MSG_INICIA_ELEITOR - on the urna the mesário sends this
//      message after identifying the voter (CNomeEleitor / CDigitalReconhecida);
//   9. build and emit the first state JSON.
// ================================================================================================
int CVotaWebEngine::Init(const char* opcoes)
{
    try {
        m_audioEleitorHabilitado = JsonBool(opcoes, "audioEleitorHabilitado", false);   // func 14477
        m_audioEnabled           = JsonBool(opcoes, "reproduzirAudio", true);

        // 2. A text-to-speech engine must exist before anything speaks. Registered through the register-or-replace
        //    wrapper (by-value helper 4162 -> replace 7667: "if (exists) erase; push"), which replaces the one
        //    main's CSimuladorWasm::Executa (8302) registered; a plain push<> would throw 6756 (u19 §3.4).
        api::CPolySingletonList::replace<api::ITextToSpeech>(                           // 4162 -> 7667
            std::make_unique<simulador::CWasmNullTextToSpeech>(), api::GetPolySingletonsInfo());
        api::CPolySingleton<api::ISound>::instance(api::GetPolySingletonsInfo(),        // func 1091
                                                   std::source_location::current())      // :515
            .Mute(!m_audioEnabled);                                                      // ISound slot 6

        // 3. Storage tree. comum::EFlashOrigem: 0 = flash interna (MI), 1 = flash externa (MV, memory card).
        for (const auto flash : {comum::EFlashOrigem::INTERNA, comum::EFlashOrigem::EXTERNA}) {
            std::filesystem::create_directories(comum::CPath::GetPathEstatico(flash));            // 762
            std::filesystem::create_directories(comum::CPath::GetPathDinamico(flash));            // 1082
            std::filesystem::create_directories(comum::CPath::GetPathDinamico(flash) / "tmp");    // 5968
            std::filesystem::create_directories(comum::CPath::GetPathTrab(flash, comum::EUrnaTurno::Primeiro));
            std::filesystem::create_directories(comum::CPath::GetPathTrab(flash, comum::EUrnaTurno::Segundo));
            WriteFileIfMissing(comum::CPath::GetPathRootSemSA(flash) / "serialv.dat", "ABCDDCBA");   // 634
        }
        WriteSimulatedSignatures();                                                      // 11733

        // 4. Fabricated persistent state. Only when eg.bin is absent: the options below are ignored if the
        //    state already exists (e.g. a second votaInit in the same page).
        if (!std::filesystem::exists(comum::CPath::GetPathDinamico(comum::EFlashOrigem::INTERNA) / "eg.bin")) {
            const int pe        = JsonInt(opcoes, "pe");          // processo eleitoral (2400 municipal ...)
            const int municipio = JsonInt(opcoes, "municipio");
            const int zona      = JsonInt(opcoes, "zona");
            const int secao     = JsonInt(opcoes, "secao");
            const int turno     = JsonInt(opcoes, "turno");

            // fase: '1' oficial, '2' simulado, '3' anything else (the page sends "te", treinamento)
            char fase;
            {
                const std::string texto = JsonString(opcoes, "fase", "te");               // func 11546
                if (texto == "oficial" || texto == "o")
                    fase = '1';
                else if (texto == "simulado" || texto == "s")
                    fase = '2';
                else
                    fase = '3';
            }
            const std::string uf = JsonString(opcoes, "uf", "ee");

            comum::teste::CAppInfoBuilder builder(pe);                                   // func 10268 (u30)
            builder.SetFase(fase)                                                         // 10197: geral +48
                   .SetTurno(turno == 2 ? '2' : '1')                                      // 10188: geral +32
                   .SetTipoUrna('1')                                                      // 10181: +36/+40 ?
                   .SetLocal(municipio, static_cast<std::uint16_t>(zona),                 // 10176: +20/+24/+26
                             static_cast<std::uint16_t>(secao));      // no range check: zona 70000 -> 4464
            builder.GetGeral().SetUF(ecourna::api::util::CStringUtils::ToUpper(uf));     // 5156; geral +8   ?
            for (auto& vota : builder.GetVota()) {                                        // m_vota[0], m_vota[1]
                vota.SetEstadoVota(comum::EEstadoVota::EAVINICIAL);                       // +0 = '1'
                vota.SetTreinamentoEleitor(true);                                         // +72 = true
            }
            // SalvaGeral(false, {MI, MV}); SalvaApps(false, {MI, MV}, '1'|'2', {GAP, SA, VOTA}) - func 10256
            builder.Salva(false, {comum::teste::EMidia::FlashInterna, comum::teste::EMidia::FlashExterna});
            WriteSimulatedSignatures();
        }                                                                                  // ~builder = func 5644

        // 5. Load the state files into comum::CAppInfo, log, voter-side start-up of VOTA.
        comum::teste::CarregaAppInfo(comum::CAppInfo::GetInst());                       // mock_f11572 (u30) ?
        const auto& geral = comum::CAppInfo::GetInst().GetGeral();                      // func 291
        auto& log = vota::CLogVota::GetInst();                                          // func 184
        log.LogaInicioAplicacao(geral.GetTurno());        // func 11641 "Iniciando aplicação - {}"      ?
        log.LogaVersaoAplicacao();                        // func 11642 "Versão da aplicação: {}"       ?
        vota::CInformacaoEleitor::Inicializar();                                        // func 7787 (u02)
        vota::CInformacaoEleitor::GetInst().GerarDadosDinamicos();                      // 509 -> 6737
        WriteSimulatedSignatures();

        // 6. Voter audio needs the RHVoice package mounted by the page (rhvoice-leticia.data).
        if (m_audioEleitorHabilitado) {
            if (!std::filesystem::exists("/etc/RHVoice") || !std::filesystem::exists("/share/RHVoice"))
                throw std::runtime_error("dados do RHVoice nao foram carregados para habilitar o audio do eleitor");
            api::CPolySingletonList::replace<api::ITextToSpeech>(                       // 4162 -> 7667
                std::make_unique<api::CRHVoiceTextToSpeech>(std::filesystem::path("/")),   // 112 bytes
                api::GetPolySingletonsInfo());
        } else {
            api::CPolySingletonList::replace<api::ITextToSpeech>(                       // 4162 -> 7667
                std::make_unique<simulador::CWasmNullTextToSpeech>(), api::GetPolySingletonsInfo());
        }

        // 7. Voting open ("votar"): what the zerésima + mesário registration would have produced on an urna.
        comum::CAppInfo::GetInst().GetVota(comum::EUrnaTurno::Atual)                     // func 261, turno '3'
            .SetEstadoVota(comum::EEstadoVota::EAVVOTAR);                                // func 11266: 56 = '8'
        comum::CAppInfo::GetInst().GetVota(comum::EUrnaTurno::Atual).MarcaInicioAquisicao();   // func 5627
        comum::SalvaEstado();                                                            // func 491: vota.bin
        WriteSimulatedSignatures();

        // 8. Start the (cooperative) voter thread and release the urna for one voter.
        auto& execucao = vota::IExecucaoVota::GetInst();                                // func 3594
        execucao.Inicia();                                                               // slot 3 (func 4713)
        if (m_audioEleitorHabilitado)
            execucao.GetFilaEleitor().Envia(vota::CThreadEleitor::MSG_AUDIO_HABILITADO); // slot 8; func 11100
        else
            vota::CInformacaoEleitor::GetInst().DesabilitaAudio();                      // func 4195
        execucao.GetFilaEleitor().Envia(vota::CThreadEleitor::MSG_INICIA_ELEITOR);      // func 11026

        // 9. First state (the voter thread is still in vota::CAguardaMensagem: the message is handled by
        //    the first votaTick).
        m_initialized = true;
        m_done = false;
        m_lastJson = BuildStateJson();                                                   // func 5500
        EmitEvent("vota:state", m_lastJson);                                             // func 3533
        return 1;
    } catch (const std::exception& e) {
        ReportError(e.what());                                                           // func 10857
        return 0;
    } catch (...) {
        ReportError("erro desconhecido em votaInit");
        return 0;
    }
}

}  // namespace

// Also in this unit, called only from main() (func 10307, unit u30), and not reconstructed here because they
// are template instantiations of other files:
//   wasm func 9507  api::CPolySingletonList::push<vota::impl::IPoliticaExecucaoEleitor> - head part
//                   (if exists<T>(info): erase(typeid(T).name(), info) via func 640; the tail is func 5407)
//   wasm func 9437  api::CPolySingletonList::push<vota::impl::ISincronismoVotoEleitor> - head part (tail 5398)
//   wasm func 5120  std::function<void()>::~function() for main's lambda (std::function<main::$_0>)
//   wasm func 8311  simulador::CSimuladorWasm::CSimuladorWasm(int, std::string, short, short) - reconstructed by
//                   unit u19 in src/uenux2/mock/app/simulador/wasm/csimuladorwasm.u19.cpp

// ================================================================================================
// The export. wasm func 7840 = export "Eb" (Module._votaInit). Called by the page adapter with
//   {"fase":"te","pe":2400,"turno":1,"uf":"ac","municipio":1,"zona":1,"secao":1,
//    "audioEleitorHabilitado":false,"reproduzirAudio":false}
// ================================================================================================
extern "C" EMSCRIPTEN_KEEPALIVE int votaInit(const char* opcoesJson)
{
    return CVotaWebEngine::GetInst().Init(opcoesJson);
}
