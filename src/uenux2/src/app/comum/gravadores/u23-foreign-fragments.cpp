// FRAGMENTS reconstructed by unit u23 for functions that the analyzer put in this unit but that belong to
// other original files. Each block names its real (or inferred) source file.
// Reconstructed from vota_web_wasm.wasm.
#include <memory>
#include <mutex>
#include <string>

namespace comum {

// =================================================================================================================
// Error-class constructor thunks (merge-similar-functions: one body, ecourna_f710, takes the vtable).
// ecourna-lib/ecourna/api/exception/cbaseerror.hpp instantiations.
// =================================================================================================================
// wasm func 283: CBaseError<comum::EUeComumGravadoresError, SErrorLimits{8600, 8800}>(exc, code, msg, srcloc)
//   -> ecourna_f710(exc, code, msg, srcloc, vtable @1552984). Thrown with typeinfo @1552964.
// wasm func 480: CBaseError<comum::EUeComumError, SErrorLimits{7200, 7600}>(exc, code, msg, srcloc)
//   -> ecourna_f710(..., vtable @1551408). Thrown with typeinfo @1551372.

// =================================================================================================================
// wasm func 276 (libc++): std::string::assign(const char* s) = assign(s, strlen(s)) (ecourna_f986 is the sized
// assign with __grow_by_and_replace). Library code; attributed here only because callers are in this unit.
// =================================================================================================================

// =================================================================================================================
// uenux2/src/app/comum/cpath.cpp (unit u02/u21)
// =================================================================================================================
// wasm func 1274 (name inferred): result directory of a flash for the CURRENT turn.
std::string CPath::GetPathResult(EFlashOrigem origem)
{
    return GetPathResult(origem, CAppInfo::GetInst().GetEstadoGeral().GetTurno());    // func 1399, turno = eg +32
}

// =================================================================================================================
// uenux2/src/app/comum/dados/cpartidos.cpp (path inferred; see u05 / cdadosestaticos.u02.cpp)
// =================================================================================================================
// wasm func 819: CPartidos::GetInst() - lazily created singleton @1838928 (mutex @1838904, 28-byte object;
// the previous instance, if any, is destroyed with ecourna_f2815). Observed executing (voter screens, BU).
CPartidos& CPartidos::GetInst()
{
    static std::mutex mutex;                 // @1838904 (pthread stub)
    static std::unique_ptr<CPartidos> s;     // @1838928
    std::lock_guard lock(mutex);
    if (!s)
        s.reset(new CPartidos());
    return *s;
}

// wasm func 3751: CPartidos::CPartidos() = api::CDataMap<TPartidoID, md::CPartido>("CPartidos"):
//   +0 std::map<TPartidoID, md::CPartido> (begin/root/size), +12 cursor = end(), +16 std::string nome = "CPartidos".
CPartidos::CPartidos() : CDataMap("CPartidos") {}

}  // namespace comum

// =================================================================================================================
// ecourna-lib/ecourna/app/dados/resultadournacadastro/cestadocomparecimento.cpp and ccomparecimentosecao.cpp
// (paths inferred; classes documented by units u11/u14, see src/ecourna/app/dados/resultadournacadastro/)
// =================================================================================================================
namespace ecourna::app::dados {

// wasm func 2193: CEstadoComparecimento(id, situacao, foto, audio) - no biometric enabling
CEstadoComparecimento::CEstadoComparecimento(const CRegistroIdentificacaoEleitor& id, ESituacaoComparecimento situacao,
                                             const CApresentacaoFotoEleitor& foto, ESituacaoHabilitacaoAudio audio)
    : m_identificacao(id), m_situacao(situacao), m_apresentacaoFoto(foto), m_habilitacaoAudio(audio)
{
}

// wasm func 2192: CEstadoComparecimento(id, situacao, foto, audio, biometria)
CEstadoComparecimento::CEstadoComparecimento(const CRegistroIdentificacaoEleitor& id, ESituacaoComparecimento situacao,
                                             const CApresentacaoFotoEleitor& foto, ESituacaoHabilitacaoAudio audio,
                                             const CHabilitacaoBiometrica& biometria)
    : m_identificacao(id), m_situacao(situacao), m_apresentacaoFoto(foto), m_habilitacaoAudio(audio),
      m_habilitacaoBiometrica(biometria)
{
}

// wasm func 5092: CComparecimentoSecao(identificacao, eleitores) - copies the 16-byte identification and the
// vector<CEstadoComparecimento> (element copy = comum_f2278 under an exception guard, ecourna_f5096).
CComparecimentoSecao::CComparecimentoSecao(const CIdentificacaoSecaoEleitoral& identificacao,
                                           const std::vector<CEstadoComparecimento>& eleitores)
    : m_identificacao(identificacao), m_eleitores(eleitores)
{
}

}  // namespace ecourna::app::dados

// =================================================================================================================
// uenux2/src/app/vota/comum/cassinadorvota.h (path inferred)
// =================================================================================================================
// wasm func 7766: vota::CAssinadorVota::~CAssinadorVota() - 7-byte thunk to comum::CAssinador::~CAssinador (1007);
// slot 1 of its vtable (@1532312) is the base's deleting destructor (4694).

// =================================================================================================================
// uenux2/src/app/vota/operador/aguardaeleitor/celeitorvotounaovotou.cpp (path inferred; see u10 fragments)
// =================================================================================================================
namespace vota {
// wasm func 10431: CEleitorVotouNaoVotou::StartState (vtable slot 2). MT screen after a voter leaves the booth.
void CEleitorVotouNaoVotou::StartState()
{
    *m_textoSituacao = m_votou ? "VOTOU. Pode entregar o comprovante"                   // +11 flag, +12 text
                               : "NÃO VOTOU. Não entregar o comprovante";
    *m_textoComplemento = " ";                                                          // +20
    m_form->Exibe();                                                                    // +28, slot 2
    m_proximoEstado = this;                                                             // +4: stay
}
}  // namespace vota

// =================================================================================================================
// uenux2/wasm/vota_web/... (anonymous namespace)::CWasmSavd (web build mock; file inferred)
// =================================================================================================================
// wasm func 10965: CWasmSavd deleting destructor = comum::IInterfaceSavd::~IInterfaceSavd (5503) + operator delete.

// =================================================================================================================
// uenux2/src/api/audio/crhvoicetexttospeech.cpp (api::CRHVoiceTextToSpeech, see docs/libraries/rhvoice.md §2.2)
// =================================================================================================================
// wasm func 10842 (23.8 KB): api::CRHVoiceTextToSpeech::CRHVoiceTextToSpeech(root) - the RHVoice engine
// constructor (paths root/share/RHVoice, root/etc/RHVoice; make_shared<RHVoice::engine>) with the whole RHVoice
// init inlined. Called by votaInit through invoke slot 45 after operator new(112). Not TSE logic of this unit.
