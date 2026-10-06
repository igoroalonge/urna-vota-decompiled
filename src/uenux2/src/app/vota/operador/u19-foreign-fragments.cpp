// FRAGMENTS reconstructed by unit u19 from vota_web_wasm.wasm.
// Operator (mesário, microterminal "MT") state methods that the tools filed under cpolysingletonlist.h
// because CPolySingleton<api::IInputMT>::instance (func 383) or CPolySingletonList code is inlined into them.
// Each belongs to the .cpp of its class (paths inferred from the class names / u10's layout):
//   vota/operador/aguardaeleitor/cperguntacodigosuspensao.cpp, .../confirmaidentidade/cdigitalnaoreconhecidadecbiometria.cpp,
//   .../confirmaidentidade/cinformaanonascimentoerrado.cpp, .../confirmaidentidade/chabilitaaudioeleitor.cpp,
//   .../justificativa/cjustificativaefetuada.cpp, .../confirmaidentidade/celeitorjavotou.cpp,
//   .../outrasopcoes/ctituloencerramentoinvalido.cpp, .../comum/ccancelahabilitacaoeleitor.cpp   (all paths inferred)
// None of them runs in the web build: the operator thread (CThreadOperador::Run) is never started there.
//
// Common pattern: slot 7 of comum::CAppState is ProcessInput(). The focused field of the MT form reads a key
// through api::CInteractiveForm<IScreenMT, IInputMT>::Read() (cinteractiveform.h:57, inlined:
// m_campos.at(m_foco)->Read(CPolySingleton<IInputMT>::instance(...))). EInputResult 5 = CORRIGE, 9 = CONFIRMA.
// m_form is a pointer to the form (+12; +20 in CJustificativaEfetuada), as in the u10/u34 reconstructions.
// m_proximoEstado (+4) is the transition read by the thread loop.
#include <format>
#include <string>

#include "api/pattern/cpolysingleton.h"
#include "vota/log/clogvota.h"
#include "vota/operador/comum/cinformacaothreadoperador.h"

namespace vota {

enum EInputResult { Corrige = 5, Confirma = 9 };   // values attested, names inferred

// wasm func 1536                                                                   // name inferred
// Lazy singleton of the state CCancelaHabilitacaoEleitor (merged body vota_f764(&mutex @1905508,
// &instance @1905532, vtable @1588756, 0)).
CCancelaHabilitacaoEleitor& CCancelaHabilitacaoEleitor::GetInst();

// wasm func 6015: merged body of the two ProcessInput below (form member offset as parameter).
// wasm func 10493 (vtable slot 7)
void CInformaAnoNascimentoErrado::ProcessInput()
{
    if (m_form->Read() == Confirma)                       // srcloc cinteractiveform.h:57 @1591480
        m_proximoEstado = &CCancelaHabilitacaoEleitor::GetInst();
}

// wasm func 10641 (vtable slot 7) - same body (6015)
void CEleitorJaVotou::ProcessInput()
{
    if (m_form->Read() == Confirma)                       // @1588860
        m_proximoEstado = &CCancelaHabilitacaoEleitor::GetInst();
}

// wasm func 10593 (vtable slot 7); form at +20
void CJustificativaEfetuada::ProcessInput()
{
    if (m_form->Read() == Confirma)
        m_proximoEstado = &CCancelaHabilitacaoEleitor::GetInst();
}

// wasm func 10727 (vtable slot 7)
void CTituloEncerramentoInvalido::ProcessInput()
{
    if (m_form->Read() == Corrige)
        m_proximoEstado = &ProximoEstadoCorrige();        // vota_f3631 (state getter, not identified)  ?
}

// wasm func 10477 (vtable slot 7)
void CDigitalNaoReconhecidaDecBiometria::ProcessInput()
{
    switch (m_form->Read()) {
    case Corrige:
        CLogVota::GetInst().LogaHabilitacaoCanceladaReconhecimento();       // api_f2495 (u10 name)
        m_proximoEstado = &CCancelaHabilitacaoEleitor::GetInst();
        break;
    case Confirma:
        m_proximoEstado = &CVerificaDadoEleitor::GetInst();                 // api_f2735 (u10 name)
        break;
    default:
        break;
    }
}

// wasm func 10523 (vtable slot 7)
// "Habilitar áudio?" asked to the mesário for a voter registered as needing audio.
void CHabilitaAudioEleitor::ProcessInput()
{
    const EInputResult r = m_form->Read();                                  // @1590908
    const bool necessitaAudio = impl::IInformacaoThreadOperador::GetInst().EleitorNecessitaAudio();   // slot 5
    if (r != Confirma)
        return;
    SetAudioHabilitadoManualmente(false);                                  // func 5415 (slot 7)
    auto& filaEleitor = CThreadEleitor::GetInst().Fila();                  // func 316 + 36
    filaEleitor.Push(api::SMessage{static_cast<short>(necessitaAudio ? 8 : 9)}, 1);   // func 501
    filaEleitor.Push(api::SMessage{1}, 1);                                 // 1 = "eleitor habilitado"
    m_proximoEstado = &CMostraEleitorVotando::GetInst();                   // func 1150
    if (necessitaAudio)
        CLogVota::GetInst().Loga("Áudio ativado conforme cadastro");      // api_f233
}

// wasm func 10420 (vtable slot 7) - 1917 bytes
// The mesário types a título to SUSPEND the current voter's session ("suspender a votação"): it must be a
// valid título AND different from the identity typed for the voter (IInformacaoThreadOperador slot 23).
void CPerguntaCodigoSuspensao::ProcessInput()
{
    const EInputResult r = m_form->Read();                                  // @1593176
    if (m_suspensaoEnviada)                                                // +28
        return;
    if (r == Corrige) {
        CLogVota::GetInst().LogaMesarioAbortouSuspensao();                 // api_f4535 (u10 name)
        CThreadEleitor::GetInst().Fila().Push(api::SMessage{4}, 1);        // resume voting
        m_proximoEstado = &CMostraEleitorVotando::GetInst();
        return;
    }
    if (r != Confirma)
        return;

    const std::string digitado = m_form->Campo(0).Texto();                  // field 0, string at +24
    if (digitado.empty())
        return;
    CLogVota::GetInst().Loga("Mesário digitou o título para suspender o eleitor");
    // The next four lines are an inlined helper that returns {bool valido, std::string titulo} (the binary
    // builds that pair at sp+8/sp+12). EhValida is called unconditionally (the compiler computes the string
    // comparison before it, which it may do because memcmp has no side effects).
    const std::string titulo = comum::md::CEleitorIdentidade::Formata(digitado, 1);            // comum_f2798
    const std::string doEleitor = comum::md::CEleitorIdentidade::Formata(IdentidadeDigitada(), 1);  // 3623
    const bool valido = comum::md::CValidadorIdentidade::GetInst().EhValida(1 /*título*/, titulo)   // vota_f2803
                        && titulo != doEleitor;
    if (!valido) {
        CLogVota::GetInst().Loga(std::format("Título {} é inválido para suspender a votação", titulo));   // api_f1398
        m_telaErro->Show();                                                // +20, vtable slot 2
        api::CSystem::Sleep(3000);   // compiled as: if (byte @1584624 == 1) emscripten_sleep(3000)
                                     // -> ABORTS in this build (no Asyncify); unreachable in the simulator
        m_form->Show();                                                    // +12, IForm slot 2 (redraw)
        return;
    }
    CLogVota::GetInst().Loga(std::format("Título {} é válido para suspender a votação", titulo));
    CThreadEleitor::GetInst().Fila().Push(api::SMessage{2}, 1);            // suspend the voter's session
    m_suspensaoEnviada = true;
}

}  // namespace vota
