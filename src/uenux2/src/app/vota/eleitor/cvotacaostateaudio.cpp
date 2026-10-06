// Reconstructed from vota_web_wasm.wasm (unit u08).
// Original: uenux2/src/app/vota/eleitor/cvotacaostateaudio.cpp
//
// srcloc evidence (std::source_location records, file:line = original line):
//   :69   virtual void ProcessInput()                      api::ISound lookup
//   :120  virtual void ProcessTick(uebyte)                 api::ISound lookup
//   :136  void PlayKey(const char) const                   api::IBeep lookup
//   :172/:173 static void PlayInterruptibleMessage(const std::string&)   ITextToSpeech / ISound
//   :178/:179 static void PlayMessage(const std::string&)              ITextToSpeech / ISound
//   :188  EmiteEcoComInputField(...)                       throw EUeVotaError 9318
//   :209  EmiteEcoCorrigeConfirma(...)                     throw EUeVotaError 9319
//   :226  static void PlayFile(const std::string&)         api::ISound lookup (inlined into PlayKey)
//   :297  std::string FormatPartyName(const std::string&)  throw EUeVotaError 9321
//   :388  void IniciarEsperaFimAudio()                     api::ISound lookup (inlined into ProcessTick)
// Lambda names from RTTI: IniciarEsperaFimAudio()::$_0 (bool()), IniciarEsperaFimAudio()::$_1 (void(bool)).
//
// Every spoken text goes through api::ITextToSpeech (RHVoice, see docs/libraries/rhvoice.md) and is
// played by api::ISound. In the web build ISound is simulador::CWasmWebSound (JS Web Audio through the
// js_wasm_web_sound_* imports) and IBeep is simulador::CWasmBeep (js_wasm_beep_*).
//
// 41 of the 53 wasm functions that the tools attributed to this file are libc++ <regex> instances
// (FormataMensagem uses std::regex); they are listed at the end of this file.

#include "vota/eleitor/cvotacaostateaudio.h"

#include <cstdlib>
#include <format>
#include <regex>
#include <source_location>

#include "api/hwil/ibeep.h"
#include "api/hwil/iinputkbd.h"
#include "api/hwil/isound.h"
#include "api/hwil/itexttospeech.h"
#include "api/gui/cinteractiveform.h"
#include "comum/dados/ccargos.h"
#include "comum/dados/ccandidaturas.h"
#include "comum/dados/cconfiguracaoeleicao.h"
#include "comum/dados/cpartidos.h"
#include "comum/dados/crespostas.h"
#include "comum/dados/ccargods.h"                 // comum::CCargoDSNomeSexoCandidato
#include "vota/eleitor/celeitorvotando.h"         // g_votoDigitado, g_votosEleitor, g_numeroEscolha
#include "vota/eleitor/cthreadeleitor.h"          // CThreadEleitor: ticks
#include "vota/eleitor/comum/cinformacaoeleitor.h"
#include "vota/eleitor/comum/ctelasvota.h"        // CFormInterativoTelaVota
#include "vota/log/clogvota.h"

namespace vota {

// CBaseError<vota::EUeVotaError, ...> (typeinfo @1532388); built by the merged thunk func 253.
using CUeVotaError = ecourna::api::exception::CBaseError<EUeVotaError /*, SErrorLimits{...} */>;

// ================================================================================================
// construction / destruction
// ================================================================================================

// wasm func 1785 (inline constructor; the tools attributed it to celeitorvotando.cpp)
CVotacaoStateAudio::CVotacaoStateAudio(uebyte flags)
    : comum::CAppState(flags)
    , m_audioHabilitado(false)
    , m_tickRepeticao(CThreadEleitor::GetInst().CriaTick(2000))   // func 807
    , m_tickInicio(CThreadEleitor::GetInst().CriaTick(1500))
{
}

// wasm func 1035 (vtable slot 0). Shared through ICF by the derived classes that add no member
// (CCompletaProporcional, CConfirmaVotoEmCargo, CConfirmaProporcional, CConfirmaMajoritario...).
CVotacaoStateAudio::~CVotacaoStateAudio()
{
    if (m_esperaFimAudio) {
        m_esperaFimAudio->Cancela();              // IEsperaAudio slot 2 (name inferred)
        m_esperaFimAudio.reset();
    }
    // ~shared_ptr (+20), then ~unique_ptr (+16): virtual deleting destructor of the pointee
}

// always inlined (FinishState, ProcessInput, ProcessTick, destructor)
void CVotacaoStateAudio::CancelaEsperaFimAudio()
{
    if (m_esperaFimAudio) {
        m_esperaFimAudio->Cancela();
        m_esperaFimAudio.reset();
    }
}

// always inlined: vota_f422 = CThreadEleitor::StopTick (api::CTickManager::StopTick at +20)
void CVotacaoStateAudio::PararTicks()
{
    if (m_audioHabilitado) {
        CThreadEleitor::GetInst().StopTick(m_tickRepeticao);
        CThreadEleitor::GetInst().StopTick(m_tickInicio);
    }
}

// ================================================================================================
// api::CState interface
// ================================================================================================

// wasm func 7028 (vtable slot 2). Observed executing.
void CVotacaoStateAudio::StartState()
{
    // CInformacaoEleitor::m_modoAudio (+4): 0 = not chosen, 1 = enabled, 2 = no audio (initial value)
    m_audioHabilitado = CInformacaoEleitor::GetInst().GetModoAudio() != CInformacaoEleitor::SemAudio;

    StartStateAudio();                                            // slot 10

    if (m_audioHabilitado)
        CThreadEleitor::GetInst().StartTick(m_tickInicio);        // speak after 1.5 s
}

// wasm func 7022 (vtable slot 5). Observed executing.
void CVotacaoStateAudio::FinishState()
{
    CancelaEsperaFimAudio();
    PararTicks();
    FinishStateAudio();                                           // slot 11
}

// wasm func 7029 (vtable slot 7), srcloc :69. Observed executing (every key press).
void CVotacaoStateAudio::ProcessInput()
{
    if (m_audioHabilitado) {
        // a key interrupts whatever the urna is saying
        CancelaEsperaFimAudio();
        PararTicks();
        api::ISound::GetInst().Stop();                            // :69  ISound slot 5
        PararTicks();
    }

    ProcessInputAudio();                                          // slot 9

    if (GetNextState() == this && m_audioHabilitado)
        CThreadEleitor::GetInst().StartTick(m_tickRepeticao);    // repeat the message 2 s later
}

// wasm func 7010 (vtable slot 8), srcloc :120 and :388. Observed executing.
void CVotacaoStateAudio::ProcessTick(uebyte tick)
{
    if (tick != m_tickRepeticao && tick != m_tickInicio) {
        ProcessTickAudio(tick);                                   // slot 12
        return;
    }

    if (api::ISound::GetInst().GetStatus() == api::ISound::Tocando)   // :120, slot 7 (1 = playing)
        return;                                                   // still talking: wait for next tick

    CancelaEsperaFimAudio();
    PararTicks();
    if (m_audioHabilitado)
        PlayInterruptibleMessage(FormataMensagem(GetMensagemAudio()));   // slots 15 -> 14

    IniciarEsperaFimAudio();
}

// srcloc :388 — inlined into ProcessTick (func 7010)
void CVotacaoStateAudio::IniciarEsperaFimAudio()
{
    if (!m_audioHabilitado)
        return;

    // ISound slot 9: wait (asynchronously) for the end of the current audio.
    //   $_0 (func 340, ICF "return 0"): the wait is never interrupted
    //   $_1 (func 13721): when the audio really ended, arm the 2 s repetition tick
    m_esperaFimAudio = api::ISound::GetInst().EsperaFimAudio(         // :388
        [] { return false; },
        [this](bool terminou) {
            if (terminou && m_audioHabilitado)
                CThreadEleitor::GetInst().StartTick(m_tickRepeticao);
        });
}

// ================================================================================================
// key echo
// ================================================================================================

// wasm func 1455, srcloc :136 (IBeep) and :226 (PlayFile, inlined). Observed executing.
// `tecla` is the key code returned by the input field ('0'..'9', 'B' branco, 'C' confirma,
// 'D' corrige) or 0 when the key was refused.
void CVotacaoStateAudio::PlayKey(const char tecla) const
{
    if (tecla == 0) {
        api::IBeep::GetInst().BeepErro();                         // :136  IBeep slot 6 (554 Hz, 200 ms)
        CLogVota::GetInst().Loga("Tecla indevida pressionada");   // api_f233 = CLoga::loga(level 1)
    }

    if (!m_audioHabilitado)
        return;

    if ((tecla >= '0' && tecla <= '9') || tecla == 'B' || tecla == 'D') {
        PlayInterruptibleMessage(api::KeyName(tecla));            // api::KeyName (func 744)
    } else if (tecla == 'C') {
        PlayMessage(api::KeyName('C'));                           // CONFIRMA: waits for the end
    } else if (tecla == 0) {
        PlayFile(":/resource/sounds/tecE.wav");                   // error sound
    }
}

// srcloc :226 — inlined into PlayKey
void CVotacaoStateAudio::PlayFile(const std::string& arquivo)
{
    if (CInformacaoEleitor::GetInst().GetModoAudio() == CInformacaoEleitor::SemAudio)
        return;
    api::ISound::GetInst().Play(arquivo);                         // :226  ISound slot 1
    // web build: CWasmWebSound slot 1 (func 9580) ignores the file, so tecE.wav is never heard
}

// wasm func 3143, srcloc :172/:173. The message interrupts the current audio (JS "modo" 0 = stop the
// queue and play now) and the caller does not wait for its end.
void CVotacaoStateAudio::PlayInterruptibleMessage(const std::string& mensagem)
{
    const auto audio = api::ITextToSpeech::GetInst().Sintetiza(mensagem);   // :172  TTS slot 0
    api::ISound::GetInst().Play(audio, false);                              // :173  ISound slot 4
}

// wasm func 1204, srcloc :178/:179. Queued after the current audio (JS "modo" 1) and waited for.
void CVotacaoStateAudio::PlayMessage(const std::string& mensagem)
{
    const auto audio = api::ITextToSpeech::GetInst().Sintetiza(mensagem);   // :178
    auto& som = api::ISound::GetInst();                                     // :179
    som.Play(audio, true);                                                  // slot 4
    som.Wait();   // non-virtual ISound::Wait() -> slot 8 Wait(std::function<bool()>{ISound::Wait()::lambda})
    // web build: CWasmWebSound::Wait (func 9523) does not block, it only returns status != playing
}

// ================================================================================================
// input echo
// ================================================================================================

// wasm func 3139 (vtable slot 13), srcloc :188 (+ api::CInteractiveForm::Read, cinteractiveform.h:57,
// inlined). Observed executing (CPedeMajoritario / CPedeProporcional::ProcessInputAudio).
std::pair<api::EInputResult, std::string>
CVotacaoStateAudio::EmiteEcoComInputField(const CFormInterativoTelaVota& tela) const
{
    const auto& campos = tela.GetInputs();
    if (campos.size() != 1)
        throw CUeVotaError(9318, "Nao havia campos de input no formulario sem nada digitado",
                           std::source_location::current());                   // :188

    auto* campo = campos[0];
    const std::string antes = campo->GetTexto();              // input field +24
    const api::EInputResult resultado = tela.Read();          // CInteractiveForm<IScreen,IInputKbd>::Read:
                                                              //   GetInputs().at(m_atual)->Read(IInputKbd::GetInst())
    const std::string depois = campo->GetTexto();

    // A digit that did not fit (Tecla) or a CORRIGE with nothing to erase leaves the text unchanged:
    // that key is refused (beep + log). Everything else is echoed.
    char tecla = campo->GetUltimaTecla();                     // input field +55
    if ((resultado == api::EInputResult::Corrige || resultado == api::EInputResult::Tecla) && antes == depois)
        tecla = 0;

    PlayKey(tecla);
    return {resultado, depois};
}

// srcloc :209 — inlined into wasm func 11754 (= CConfirmaVotoEmCargo::ProcessInputAudio, see below).
// Observed executing. On the CONFIRMA/CORRIGE screen only those two keys are echoed.
api::EInputResult CVotacaoStateAudio::EmiteEcoCorrigeConfirma(const CFormInterativoTelaVota& tela) const
{
    const api::EInputResult resultado = tela.Read();          // cinteractiveform.h:57
    const auto& campos = tela.GetInputs();
    if (campos.size() != 1)
        throw CUeVotaError(9319, "Nao havia campos de input no formulario sem nada digitado",
                           std::source_location::current());                   // :209

    const char tecla = campos[0]->GetUltimaTecla();
    PlayKey((tecla == 'C' || tecla == 'D') ? tecla : 0);
    return resultado;
}

// ================================================================================================
// audio templates
// ================================================================================================

// wasm func 6954, srcloc :297. `this` is not used (removed by the optimiser).
std::string CVotacaoStateAudio::FormatPartyName(const std::string& numeroPartido)
{
    auto& partidos = comum::CPartidos::GetInst();             // func 819: map<TPartidoID, CPartido> + cursor
    const auto numero = static_cast<comum::TPartidoID>(std::atoi(numeroPartido.c_str()));
    const auto it = partidos.find(numero);
    if (it == partidos.end()) {
        CLogVota::GetInst().LogaErroPartidoNaoEncontrado();  // func 4550 "Erro partido não encontrado"
        throw CUeVotaError(9321, "Nao foi encontrado o partido", std::source_location::current()); // :297
    }
    partidos.SetCurrent(it);                                  // cursor (+12) = it
    return it->second.GetNome();                              // CPartido +16
}

// wasm func 6999 (vtable slot 14). Not observed in the recorded votes (audio was off); it runs when
// the simulator is started with the voter audio enabled (checked with headless.mjs --audio).
//
// The template is split with std::sregex_token_iterator(modelo, "\\{.+?\\}", {-1, 0}): the text
// between tags (-1) and the tags themselves (0) are visited in order; each known tag is replaced.
std::string CVotacaoStateAudio::FormataMensagem(const std::string& modelo) const
{
    if (modelo.empty())
        return {};

    std::string resultado;
    const std::regex tag(R"(\{.+?\})");                      // ECMAScript (flags 512 = ABI-v2 value)

    // "\B\d" + ", $&" inserts ", " before every digit that is not the first one: "123" -> "1, 2, 3",
    // so that the TTS reads the digits one by one.
    static const std::regex s_digito(R"(\B\d)");            // @1833216, guard @1833256
    const auto soletra = [](const std::string& numero) {
        return std::regex_replace(numero, s_digito, ", $&");  // func 6958
    };

    const std::sregex_token_iterator fim;
    for (std::sregex_token_iterator it(modelo.begin(), modelo.end(), tag, {-1, 0}); it != fim; ++it) {
        const std::string token = *it;
        std::string trecho;

        if (token == "{cargo-atual}") {
            const auto& cargo = comum::CCargos::GetInst().GetCurrent();
            std::string nome;
            if (cargo.EhConsulta()) {                          // CCargo +136
                const auto& detalhe = cargo.GetDetalheConsulta();          // func 1923 (+88)
                nome = !detalhe.m_nomeAudio.empty() ? detalhe.m_nomeAudio   // +36
                                                    : detalhe.m_nome;       // +12
            } else {
                nome = cargo.GetNome();                        // func 1547 (named GetDetalheConsulta by the tools)
            }
            std::string escolha;
            if (cargo.GetQtdEscolhas() >= 2)                   // CCargo +13
                escolha = std::format(" {}ª {}", g_numeroEscolha, cargo.EhConsulta() ? "escolha" : "vaga");
            trecho = nome + escolha;                           // e.g. "Senador 2ª vaga"
        } else if (token == "{voto}") {
            trecho = soletra(g_votoDigitado);
        } else if (token == "{legenda}") {
            trecho = soletra(g_votoDigitado.substr(0, 2));
        } else if (token == "{nome-partido}") {
            trecho = const_cast<CVotacaoStateAudio*>(this)->FormatPartyName(g_votoDigitado.substr(0, 2));
        } else if (token == "{escolha-referendo}") {
            const auto& resposta = comum::CRespostas::GetInst().GetCurrent();
            trecho = !resposta.m_nomeAudio.empty() ? resposta.m_nomeAudio    // +16
                                                   : resposta.m_descricao;   // +4
        } else if (token == "{o/a}") {
            auto& candidaturas = comum::CCandidaturas::GetInst();            // func 521
            trecho = (!candidaturas.IsEnd() && candidaturas.GetCurrent().GetSexo() == 2) ? "a" : "o";
        } else if (token == "{candidato}") {
            const auto& candidato = comum::CCandidaturas::GetInst().GetCurrent();
            trecho = candidato.m_nomeAudio ? *candidato.m_nomeAudio          // optional<string> +32/+44
                                           : candidato.m_nomeUrna;           // +20
        } else if (token == "{candidato-com-suplentes}") {
            const auto& candidato = comum::CCandidaturas::GetInst().GetCurrent();
            trecho = candidato.m_nomeAudio ? *candidato.m_nomeAudio : candidato.m_nomeUrna;
            uebyte ordem = 0;
            for (const auto& suplente : candidato.m_suplentes) {             // vector +60, 52-byte items
                const std::string titulo = comum::CCargoDSNomeSexoCandidato{++ordem, true}();   // func 2268
                trecho += ", " + titulo + " " +
                          (suplente.m_nomeAudio ? *suplente.m_nomeAudio : suplente.m_nomeUrna); // +24/+36, +12
            }
        } else if (token == "{quantidade-digitos}") {
            const uebyte digitos = comum::CCargos::GetInst().GetCurrent().GetQtdDigitos();   // CCargo +12
            trecho = std::format("{} {}", digitos, digitos > 1 ? "dígitos" : "dígito");
        } else if (token == "{progresso}") {
            // "<n> de <total>": n = votes already confirmed + 1, total = sum of the escolhas of all cargos
            uebyte total = 0;
            for (const auto& item : comum::CCargos::GetInst().GetLista())     // 8-byte entries, id at +4
                total += comum::CConfiguracaoEleicao::GetInst().GetCargo(item.m_cargo).GetQtdEscolhas();
            trecho = std::format("{} de {}", g_votosEleitor.size() + 1, total);
        } else {
            trecho = token;                                    // plain text between the tags
        }
        resultado += trecho;
    }
    return resultado;
}

// ================================================================================================
// wasm func 11754 — the tools named it "CVotacaoStateAudio::EmiteEcoCorrigeConfirma" because of the
// srcloc :209 it contains, but it is vtable slot 9 of CConfirmaVotoEmCargo (and of CConfirmaProporcional,
// CCandidatoInapto, CConfirmaVotoLegenda, CCandidatoInexistente, CConfirmaVotoNominal, CProporcionalBranco,
// CProporcionalNulo, ...): vota::CConfirmaVotoEmCargo::ProcessInputAudio(), whose body lives in
// cconfirmavotoemcargo.cpp. The string literal passed to GetTelaCargoAtual is its __PRETTY_FUNCTION__
// "virtual void vota::CConfirmaVotoEmCargo::ProcessInputAudio()". Shown here because the unit owns it:
//
//   void CConfirmaVotoEmCargo::ProcessInputAudio() {
//       const auto tela = GetTelaCargoAtual(__PRETTY_FUNCTION__);        // func 5928, shared_ptr
//       switch (EmiteEcoCorrigeConfirma(*tela)) {
//       case api::EInputResult::Corrige:                                 // 5
//           NotificaOperador(5);                                         // slot 17 (name inferred)
//           m_proximoEstado = GetEstadoCorrige();                        // slot 16 (name inferred)
//           break;
//       case api::EInputResult::Confirma: {                              // 9
//           const auto cargo = comum::CCargos::GetInst().GetCurrent().GetID();
//           const std::string voto = g_votoDigitado;
//           CEleitorVotando::GetInst();                                  // func 3229 (result unused)
//           CEleitorVotando::RegistraVoto(cargo, m_tipoVoto /* +32 */, voto);   // func 4471
//           NotificaOperador(9);                                         // slot 17
//           m_proximoEstado = nullptr;                                   // cargo done
//           break;
//       }
//       default: break;
//       }
//   }
// ================================================================================================

// ================================================================================================
// Other functions that the tools placed in this unit
// ================================================================================================

// wasm func 509 — vota::CInformacaoEleitor::GetInst() (original file:
// uenux2/src/app/vota/eleitor/comum/cinformacaoeleitor.cpp, path inferred; see unit u02). Lazily
// creates the 12-byte object {0, 0, m_modoAudio = 2 (SemAudio), 0, 0, 0}; the no-op call to
// shared_f150 is the residue of std::lock_guard<std::mutex> on @1833260. Destroyed at exit by
// func 13695 (unique_ptr @1833284). Its m_modoAudio (+4) is written by CThreadEleitor (messages
// 8/9/10 -> 0/1/2: funcs 6745/6743/4195) and by votaInit (2 when the page disables the voter audio).
//
//   CInformacaoEleitor& CInformacaoEleitor::GetInst() {
//       static std::mutex s_mutex;                                   // @1833260
//       static std::unique_ptr<CInformacaoEleitor> s_inst;           // @1833284
//       std::lock_guard lock(s_mutex);
//       if (!s_inst) s_inst.reset(new CInformacaoEleitor);           // operator new(12)
//       return *s_inst;
//   }

// wasm func 13424 — std::string(*)() stored as table slot 1087 and handed by the voter-screen builder
// (api_f2384, inlined into the start-up function 7787) to a CDataText of the status bar. Probably a
// static function of vota/eleitor/comum/ctelasvota.cpp (path inferred). name inferred
std::string TextoStatusAudio()
{
    if (CInformacaoEleitor::GetInst().GetModoAudio() != CInformacaoEleitor::SemAudio)
        return "Áudio ativado";
    return {};
}

// wasm func 906 — body of the at-exit destructor of the function-local singletons
// `static std::unique_ptr<X> s_inst` whose X derives from CVotacaoStateAudio without adding members
// (their destructor folds into ~CVotacaoStateAudio, func 1035):
//   void ResetaSingletonAudio(std::unique_ptr<CVotacaoStateAudio>& p) { p.reset(); }   // name inferred
// Thunks (one per singleton, registered with __cxa_atexit; each passes the address of the unique_ptr):
//   func 7479  -> @1832992  (CEleitorVotando::ChamaEstadoProximoCargo's per-cargo state)
//   func 11689 -> @1838312  CConfereVotoEmCargo<CMajoritarioValido, 2>
//   func 11692 -> @1838284  CConfereVotoEmCargo<CMajoritarioRepetido, 16>
//   func 11695 -> @1838256  CConfereVotoEmCargo<CMajoritarioNulo, 7>
//   func 11698 -> @1838228  CConfereVotoEmCargo<CMajoritarioBranco, 4>
//   func 11703 -> @1838200  CConfereVotoEmCargo<CProporcionalNulo, 7>
//   func 11706 -> @1838172  CConfereVotoEmCargo<CProporcionalBranco, 4>
//   func 11714 -> @1838108  (singleton of vota_f3849)
//   func 11730 -> @1837944  CConfereVotoEmCargo<CConfirmaVotoNominal, 2>
//   func 11734 -> @1837916  CConfereVotoEmCargo<CCandidatoInexistente, 12>
//   func 11737 -> @1837888  CConfereVotoEmCargo<CConfirmaVotoLegenda, 10>

// ================================================================================================
// libc++ <regex> instances used by FormataMensagem (not TSE code; listed so the map is complete).
// libc++ 21/22, ABI v2 (_LIBCPP_ABI_REGEX_CONSTANTS_NONZERO: ECMAScript == 1 << 9).
//
//   950   std::__throw_regex_error(error_type)  shared body (throws regex_error, typeinfo @1305144)
//   4671  std::regex_error::regex_error(error_type)   ("Unknown error type" fallback, table @326295)
//   2391 / 6770 / 1599 / 1597 / 3087 / 3086 / 3107 / 6879 / 3135
//         __throw_regex_error<error_collate(1) / error_ctype(2) / error_escape(3) / error_brack(5) /
//                             error_brace(7) / error_badbrace(8) / __re_err_empty(15) /
//                             __re_err_grammar(14) / __re_err_parse(17)>()
//   2416  basic_regex<char>::basic_regex(const char*, flag_type)  (== __init + __parse)
//   2062  basic_regex::__parse(first, last)          (grammar switch; ECMAScript branch inlines __parse_ecma_exp)
//   6891  __parse_alternative (top level, __parse_term inlined: loop of assertion | atom + quantifier)
//   6869  __parse_term (one __parse_assertion, or __parse_atom + __parse_ERE_dupl_symbol; no loop)
//   6871  __parse_ecma_exp (groups; __parse_alternative inlined around 6869: __alternate / __empty_non_own_state)
//   6833  __parse_assertion (^ $ \b \B (?= (?!, builds a nested basic_regex)
//   6826  __parse_atom      /  4242 __parse_ERE_dupl_symbol (* + ? {m,n} and the ECMAScript lazy '?';
//         __loop, __repeat_one_loop; there is no "__parse_QUANTIFIER" in libc++)
//   6751  __test_back_ref(char) (digit 1..9 -> __push_back_ref, error_backref above mark_count)
//         /  2393 __push_back_ref (icase / collate / plain)
//   4237  __parse_bracket_expression  /  6765, 9511 __parse_class_escape  /  6788, 5186, 5187
//         __parse_character_escape / __parse_awk_escape variants  /  4203 __parse_awk_escape
//   9512  __parse_collating_symbol  /  9513 __parse_character_class  /  5185 __lookup_collatename
//   3329  __get_classname (regex_traits::__lookup_classname)
//   965   __bracket_expression::__add_char  /  6763 __add_digraph  /  1321 vector<pair<char,char>>::push_back
//   6764  __bracket_expression::__add_range (uses comum_f625 = vector<pair<string,string>>::push_back)
//   6762  __bracket_expression::__add_neg_char('_') (constant-propagated: pushes '_' into __neg_chars_
//         (+32), translated when icase/collate; called by __parse_class_escape for \W)
// These instances are shared: api::CInputMenuField (func 5492), CThreadMonitor (7709), ecourna
// CSerialMidia (9258) and CRelatorioTesteImpressora (5782) also use std::regex.
//   4257  __parse_basic_reg_exp  /  6865 __parse_simple_RE / __parse_RE_expression  /  6862 __parse_RE_dupl_symbol
//   3081  __parse_QUOTED_CHAR  /  3108 __parse_extended_reg_exp  /  3102 __parse_ERE_expression
//   4307  regex_token_iterator(first, last, re, initializer_list<int>{-1, 0})
//   1202  regex_token_iterator copy constructor  /  3137 regex_token_iterator::operator==
//   6970  regex_token_iterator::operator++       /  6942 regex_iterator::operator==
//   6939  regex_iterator::operator++ (regex_search)  /  6958 regex_replace(back_inserter, first, last, re, fmt)
// ================================================================================================

}  // namespace vota
