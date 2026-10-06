// FRAGMENTS reconstructed by unit u37 from vota_web_wasm.wasm: functions of uenux2/src/app/vota/** files owned
// by other units (or of classes without an attested file) that the tools filed in unit u37. Each block names
// the file it belongs to ("path inferred" = no std::source_location record names the file).
//
// Contents
//   1. lazy singletons of states (merged body 764 and its thunks, and the inlined constructors)
//   2. other constructors / small members (testeteclado::CBase, CVisualizarCandidatos::GetCargo,
//      CTelasVota::CriaTelaAguarde, the WSQ directories, the extractor configuration)
//   3. compiler-generated code: merged destructor body 2902, at-exit destructors of function statics,
//      std::map / std::vector instantiations
#include <cstdint>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "api/gui/capplicationcontextstack.u15.h"
#include "comum/cappstate.u37.h"
#include "comum/dados/cconfiguracaoeleicao.h"
#include "vota/eleitor/comum/ctelasvota.h"
#include "vota/eleitor/cthreadeleitor.h"

namespace vota {

// =============================================================================================================
// 1. LAZY SINGLETONS
// =============================================================================================================
// Every state is a singleton created on first use:
//     static std::mutex s_mutex;  static std::unique_ptr<T> s_inst;
//     std::lock_guard trava(s_mutex);  if (!s_inst) s_inst.reset(new T(...));  return *s_inst;
// Emscripten builds without pthreads, so lock() vanished and only the unlock "noexcept stub" (func 150,
// std::mutex::unlock) is left; reset() still frees a previous object, which can never exist.

// wasm func 764 (tools: vota_f764) - merged body (merge-similar-functions) of the GetInst() of every 12-byte
// state whose constructor is just CAppState(flags) + its vptr; the thunk passes the two statics, the vtable
// and the flags. Ran at start-up: the profiler sampled its thunk CAguardaMensagem::GetInst (1337, called by
// main), which calls 764 unconditionally; 764 itself has no sample.                        name inferred
template <class ESTADO>
ESTADO& ObtemEstadoSimples(std::mutex& mutex, std::unique_ptr<ESTADO>& instancia)
{
    std::lock_guard trava(mutex);
    if (!instancia)
        instancia.reset(new ESTADO());          // operator new(12); CAppState(flags) = shared_f224; vptr
    return *instancia;
}

// Thunks of 764 in this unit (class, file, mutex / unique_ptr statics, CAppState flags):
//   wasm 3864  CReinicioComparecimentoMesario::GetInst()  eleitor/creiniciocomparecimentomesario.cpp (inferred)
//              @1834328 / @1834352, flags 0 (vtable @1544920)
//   wasm 3879  CEmitirMaisBU::GetInst()                   eleitor/fimvotacao/cemitirmaisbu.cpp
//              @1833600 / @1833624, flags 2 = keys (vtable @1541180)
//   wasm 5342  CFinalizaOperador::GetInst()               operador/cfinalizaoperador.cpp (inferred)
//              @1911628 / @1911652, flags 0 (vtable @1601096)
//   wasm 5983  CInicioBU::GetInst()                       eleitor/fimvotacao/ciniciobu.cpp (inferred)
//              @1833768 / @1833792, flags 0 (vtable @1541852)
//   wasm 5984  CImprimirBUOutrasObrigatorias::GetInst()   eleitor/fimvotacao/cimprimirbuoutrasobrigatorias.cpp
//              @1833740 / @1833764, flags 0 (vtable @1541748)
//   wasm 5985  CImprimindoBim::GetInst()                  eleitor/fimvotacao/cimprimindobim.cpp
//              @1833684 / @1833708, flags 0 (vtable @1541540)
//   wasm 6037  CGravaResultado::GetInst()                 eleitor/fimvotacao/cgravaresultado.cpp
//              @1833572 / @1833596, flags 0 (vtable @1540636)
CReinicioComparecimentoMesario& CReinicioComparecimentoMesario::GetInst()         // wasm 3864
{
    static std::mutex s_mutex;                                                      // @1834328
    static std::unique_ptr<CReinicioComparecimentoMesario> s_inst;                  // @1834352
    return ObtemEstadoSimples(s_mutex, s_inst);
}
CEmitirMaisBU& CEmitirMaisBU::GetInst()                                             // wasm 3879
{
    static std::mutex s_mutex;                                                      // @1833600
    static std::unique_ptr<CEmitirMaisBU> s_inst;                                   // @1833624
    return ObtemEstadoSimples(s_mutex, s_inst);
}
CFinalizaOperador& CFinalizaOperador::GetInst()                                     // wasm 5342
{
    static std::mutex s_mutex;                                                      // @1911628
    static std::unique_ptr<CFinalizaOperador> s_inst;                               // @1911652
    return ObtemEstadoSimples(s_mutex, s_inst);
}
CInicioBU& CInicioBU::GetInst()                                                     // wasm 5983
{
    static std::mutex s_mutex;                                                      // @1833768
    static std::unique_ptr<CInicioBU> s_inst;                                       // @1833792
    return ObtemEstadoSimples(s_mutex, s_inst);
}
CImprimirBUOutrasObrigatorias& CImprimirBUOutrasObrigatorias::GetInst()             // wasm 5984
{
    static std::mutex s_mutex;                                                      // @1833740
    static std::unique_ptr<CImprimirBUOutrasObrigatorias> s_inst;                   // @1833764
    return ObtemEstadoSimples(s_mutex, s_inst);
}
CImprimindoBim& CImprimindoBim::GetInst()                                           // wasm 5985
{
    static std::mutex s_mutex;                                                      // @1833684
    static std::unique_ptr<CImprimindoBim> s_inst;                                  // @1833708
    return ObtemEstadoSimples(s_mutex, s_inst);
}
CGravaResultado& CGravaResultado::GetInst()                                         // wasm 6037
{
    static std::mutex s_mutex;                                                      // @1833572
    static std::unique_ptr<CGravaResultado> s_inst;                                 // @1833596
    return ObtemEstadoSimples(s_mutex, s_inst);
}

// ------------------------------------------------------------------------------------------------------------
// Singletons whose constructor is inlined in GetInst (one function each).
// ------------------------------------------------------------------------------------------------------------

// uenux2/src/app/vota/eleitor/celeitorvotando.cpp (attested). wasm func 3229 - observed executing
// (CAguardaMensagem::ProcessMessage(1) = "Eleitor foi habilitado", every voter).
// 72 bytes. Class and member names: celeitorvotando.h (unit u08).
CEleitorVotando& CEleitorVotando::GetInst()
{
    static std::mutex s_mutex;                                                      // @1832996
    static std::unique_ptr<CEleitorVotando> s_inst;                                 // @1833020
    std::lock_guard trava(s_mutex);
    if (!s_inst)
        s_inst.reset(new CEleitorVotando());
    return *s_inst;
}
CEleitorVotando::CEleitorVotando()                                                  // inlined in 3229
    : comum::CAppState(MENSAGENS | TECLADO | TICKS)                                 // 7
    , m_estadoCargo(nullptr)                                                        // +12
    , m_eleitorDemorando(false)                                                     // +17
    , m_vemDaInstrucaoAcessibilidade(false)                                         // +18
    , m_contexto(api::Actions::ReinicieOuFotografeQRCode /*11*/,                    // +20, wasm 3682
                 "Erro inesperado durante a votação",
                 "O voto do eleitor NÃO foi registrado",
                 "Ocorreu um erro enquanto o eleitor registrava suas escolhas.")
{
    // Inactivity alarm ("eleitor demorando"): ParametrosUrna.tempoDispararSuspensaoTE (+84) in voter
    // training, tempoDispararSuspensao (+80) otherwise, in seconds (CConfiguracaoEleicao +172 / +168).
    const auto& pu = comum::CConfiguracaoEleicao::GetInst().GetParametrosUrna();
    const int segundos = comum::EhTreinamentoEleitor() ? pu.GetTempoDispararSuspensaoTE()        // wasm 697
                                                       : pu.GetTempoDispararSuspensao();
    m_tickEleitorDemorando = CThreadEleitor::GetInst().CriaTick(segundos * 1000);                // +16, 316+807
}

// uenux2/src/app/vota/eleitor/fimvotacao/cmostraqrcodebu.cpp (attested). wasm func 2888.
// 36 bytes: CEstadoComDesligamentoAutomatico(2 = keys) (unknown_f1285: +12 battery deadline, +24 1-s tick),
// +28 std::shared_ptr form (empty until StartState). Old instance: dtor icf 785 (releases +32) + free.
// Callers: CLimiteCopiasBUAtingido slot 7, CMostraQRCodeCertificado::ProcessInput, CEmitirMaisBU::ProcessInput.
CMostraQRCodeBU& CMostraQRCodeBU::GetInst()
{
    static std::mutex s_mutex;                                                      // @1833824
    static std::unique_ptr<CMostraQRCodeBU> s_inst;                                 // @1833848
    std::lock_guard trava(s_mutex);
    if (!s_inst)
        s_inst.reset(new CMostraQRCodeBU());          // : CEstadoComDesligamentoAutomatico(TECLADO), m_tela()
    return *s_inst;
}

// uenux2/src/app/vota/eleitor/iniciovotacao/cverificahorariozeresima.cpp. wasm func 5947.
// 52 bytes (layout: cverificahorariozeresima.h, unit u26). Callers: CGeraDadosDinamicos::StartState (11865),
// testeteclado::CPreZeresima::GetEstadoPassouNoTeste (5945).
CVerificaHorarioZeresima& CVerificaHorarioZeresima::GetInst()
{
    static std::mutex s_mutex;                                                      // @1834664
    static std::unique_ptr<CVerificaHorarioZeresima> s_inst;                        // @1834688
    std::lock_guard trava(s_mutex);
    if (!s_inst)
        s_inst.reset(new CVerificaHorarioZeresima());
    return *s_inst;
}
CVerificaHorarioZeresima::CVerificaHorarioZeresima()                                // inlined in 5947
    : CEstadoComDesligamentoAutomatico(TECLADO | TICKS)                             // 6 (| 4 added by the base)
    , m_tela(CTelasVota::GetInst().m_telaAntesHorarioZeresima)                     // CTelasVota +20/+24 (copy):
                                                                                    // "telaAntesHorarioZeresima",
                                                                                    // built by func 6595
    , m_horarioZeresima(comum::CConfiguracaoEleicao::GetInst().GetDataHoraZeresima()) // cfg +544..+555
    , m_tick(CThreadEleitor::GetInst().CriaTick(2000))                              // +48, stopped 2-s tick
{
}

// uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cvisualizarcandidatos.cpp (attested). wasm func 1279.
// 40 bytes: CAppState(2 = keys), m_candidaturas empty (+12), the three optional filters disengaged
// (+24/+25, +28/+32, +36/+38). Old instance: ~CVisualizarCandidatos (2871) + free.
CVisualizarCandidatos& CVisualizarCandidatos::GetInst()
{
    static std::mutex s_mutex;                                                      // @1834636
    static std::unique_ptr<CVisualizarCandidatos> s_inst;                           // @1834660
    std::lock_guard trava(s_mutex);
    if (!s_inst)
        s_inst.reset(new CVisualizarCandidatos());
    return *s_inst;
}

// cmenufiltrarcandidatospornumero.cpp (path inferred: eleitor/iniciovotacao/auxiliares/). wasm func 5954.
// 20 bytes: CAppState(2), +12 std::shared_ptr form (empty). Old instance: dtor icf 244 + free.
// Callers: CMenuVisualizarCandidatos::StartState (11881), CMenuFiltrarCandidatosPorCargo::StartState (11888).
CMenuFiltrarCandidatosPorNumero& CMenuFiltrarCandidatosPorNumero::GetInst()
{
    static std::mutex s_mutex;                                                      // @1834524
    static std::unique_ptr<CMenuFiltrarCandidatosPorNumero> s_inst;                 // @1834548
    std::lock_guard trava(s_mutex);
    if (!s_inst)
        s_inst.reset(new CMenuFiltrarCandidatosPorNumero());
    return *s_inst;
}

// uenux2/src/app/vota/operador/csincronismooperador.cpp (path inferred; u17 proposed operador/aguardaeleitor/:
// the statics lie between those of CFinalizaOperador and CThreadOperador, both in operador/). wasm func 1897.
// 12 bytes: CAppState(1 = messages), m_suspensaoAutomatica (+11) = false.
// Callers: CSuspensaoAutomaticaEleitor slot 6, CPerguntaCodigoSuspensao slot 6,
//          CMostraEleitorVotando::SalvaHabilitacaoEleitor.
CSincronismoOperador& CSincronismoOperador::GetInst()
{
    static std::mutex s_mutex;                                                      // @1911656
    static std::unique_ptr<CSincronismoOperador> s_inst;                            // @1911680
    std::lock_guard trava(s_mutex);
    if (!s_inst)
        s_inst.reset(new CSincronismoOperador());       // : CAppState(MENSAGENS), m_suspensaoAutomatica(false)
    return *s_inst;
}

// uenux2/src/app/vota/operador/confirmaidentidade/ccontrolareconhecimento.cpp (attested). wasm func 1256.
// 20 bytes: CAppState(2), +12 the "ELEITOR(A) PODE VOTAR" form. Old instance: dtor icf 244 + free.
// Callers: CMostraEleitorVotando::SalvaHabilitacaoEleitor, CDigitalNaoReconhecida::StartState,
//          CDigitalNaoReconhecidaPorTempo slot 2, CNomeEleitor::NavegaBiometrica.
CControlaReconhecimento& CControlaReconhecimento::GetInst()
{
    static std::mutex s_mutex;                                                      // @1908732
    static std::unique_ptr<CControlaReconhecimento> s_inst;                         // @1908756
    std::lock_guard trava(s_mutex);
    if (!s_inst)
        s_inst.reset(new CControlaReconhecimento());
    return *s_inst;
}
CControlaReconhecimento::CControlaReconhecimento()                                  // inlined in 1256
    : comum::CAppState(TECLADO)
    , m_form(CriaFormEleitorPodeVotar())                                            // wasm 5410 (unit u27)
{
    (void)ConfiguracaoExtrator::GetInst();       // wasm 1903: result unused (only its creation remains)   ?
}

// =============================================================================================================
// 2. OTHER CONSTRUCTORS AND SMALL MEMBERS
// =============================================================================================================

// Fingerprint-template extractor configuration (name from unit u22; class, namespace and file unknown -
// its statics come right before CControlaReconhecimento's, so ccontrolareconhecimento.cpp or a header it
// includes is the best guess).                                                                 path inferred
// wasm func 1903 (tools: vota_f1903). 16 bytes, never modified after construction:
//   +0 int 8 (bits per pixel)  +4 int 500 (resolution, ppi)  +8 double 196.8503937007874 (= 500 / 2.54,
//   pixels per centimetre, the unit of the ISO/ANSI template headers).                   field names inferred
// Callers: CControlaReconhecimento::GetInst (1256), CRegistraDigitalOperador::BiometriaMesarioPresenteNosEleitores
// (3614), comum::CControladorReconhecimentoMesario::ComparaDigitais (5372), comum::CPedeDigitalMesario::
// ProcessTick (10316), CPedeDigital::ProcessTick (10465) - always just before the (stubbed) extraction.
struct ConfiguracaoExtrator {
    int bitsPorPixel = 8;                                                           // +0
    int resolucaoPpi = 500;                                                         // +4
    double pixelsPorCm = 500 / 2.54;                                                // +8

    static const ConfiguracaoExtrator& GetInst()
    {
        static std::mutex s_mutex;                                                  // @1908640
        static std::unique_ptr<ConfiguracaoExtrator> s_inst;                        // @1908664
        std::lock_guard trava(s_mutex);
        if (!s_inst)
            s_inst.reset(new ConfiguracaoExtrator());
        return *s_inst;
    }
};

// uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cbase.cpp (path inferred: cprezeresima.h includes
// "testeteclado/cbase.h"). wasm func 5948 (tools: vota_f5948) - the protected constructor of the abstract
// keypad-test base (slots 10-12 pure). The caller stores the concrete vptr afterwards: CPreZeresima::GetInst
// inlined in CVerificaEleicaoPassou::StartState (11914) and in CAjusteInicial::StartState (7160).
namespace testeteclado {
CBase::CBase()
    : CEstadoComDesligamentoAutomatico(TECLADO)                                     // unknown_f1285(this, 2)
    , m_tela()                                                                       // +28/+32 empty shared_ptr
{
}
} // namespace testeteclado

// uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cvisualizarcandidatos.h (attested .cpp). wasm func 5951.
// Out-of-line inline accessor (name as used by unit u02): the cargo filter, 0 = "Todos".
// Callers: CMenuFiltrarCandidatosPorNumero slots 2 and 7 (11892, 11891).
std::uint8_t CVisualizarCandidatos::GetCargo() const
{
    return m_cargo.value_or(0);                                                     // +24 value, +25 flag
}

// uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp (attested). wasm func 6587 (tools: vota_f6587).
// Callers: CImprimindoZeresima::StartState (11991), CReimprimindoResumoZeresima::ImprimeResumoZeresima (11855).
// name as used by units u06/u09 (name inferred)
std::shared_ptr<api::IForm<api::IScreen>> CTelasVota::CriaTelaAguarde()
{
    return CriaTelaNeutra("Por favor, aguarde...", 35, false);                      // wasm 2380
}

// WSQ directories. The fingerprint images captured by the operator terminal are kept, encrypted, under
// <flash>/dinamico/trab<turno>/wsq/<subdir>/ until CGravaResultado packs them into wsqbio.jez / wsqman.jez /
// wsqmes.jez. Body shared by the INTERNAL-flash variants: wasm 2298 = DiretorioWsq(subdir) (unit u27,
// cregistradigitaloperador.cpp) = CPath::GetPathWsq(INTERNA, turno of eg.bin) / subdir; the EXTERNAL-flash
// twins are comum 5817 / 5816 / 3794 over body 3898. Placed with 2298 (path inferred).        names inferred
std::filesystem::path DiretorioWsqHabilitado()    { return DiretorioWsq("habilitado/"); }      // wasm 5819
std::filesystem::path DiretorioWsqNaoHabilitado() { return DiretorioWsq("nao-habilitado/"); }  // wasm 5818
std::filesystem::path DiretorioWsqOperador()      { return DiretorioWsq("operador/"); }        // wasm 3795
// Users: CMostraEleitorVotando::SalvaHabilitacaoEleitor (10425: all three), comum::CPedeDigitalMesario::
// GetControlador (5382: operador), comum::CGravadorWSQ::GetCaminhoCorretoInternal (u12: tipo 0/1/2; the tools
// show it as ecourna::...::globToFileList because of an inlining artefact).

// =============================================================================================================
// 3. COMPILER-GENERATED CODE
// =============================================================================================================

// wasm func 2902 (tools: vota_f2902). Merged destructor body (merge-similar-functions: the vtable is the 2nd
// parameter): store the vptr, release the std::shared_ptr at +12 (control block +16), return this.
// Thunks: IEleitorImpedidoVotar::~ (1257), IConfirmaJustificativa::~ (1688), CGeraZeresimaBase::~ (1720),
// CGeraResumoZeresimaBase::~ (1952) - four CAppState subclasses whose only non-trivial member is the
// screen `std::shared_ptr<...> m_tela` at +12. Source form: `~Classe() override = default;`.

// At-exit destructors of function-local statics (`static std::mutex s_mutex; static std::unique_ptr<T>
// s_inst;` of the GetInst functions). Each is `void f(void*)` registered with __cxa_atexit; the registration
// calls themselves are gone (Emscripten builds without EXIT_RUNTIME, atexit is a no-op) but the function-table
// entries remain, so these bodies are dead code. ~mutex is the noexcept pthread stub (func 150);
// ~unique_ptr is shared_f349 (free) or shared_f389 (icf 244 dtor + free).
//   wasm | static                        | owner
//   -----+-------------------------------+---------------------------------------------------------------
//   7116 | ~mutex      @1833188          | CThreadEleitor::GetInst (316)
//   7175 | ~mutex      @1833164          | impl::ISincronismoVotoEleitor::GetInst (inlined in 7181,
//        |                               |   csincronismovotoeleitor.cpp:67)
//   7187 | ~mutex      @1833136          | CSincronismoEleitor::GetInst (inlined in 4454; the tools name 4454
//        |                               |   comum::md::CVotosCargos::ConfereCedula)
//   7195 | ~unique_ptr @1833160 (349)    |   same
//   7202 | ~mutex      @1833108          | CFimVotoEleitor::GetInst (inlined in 7181)
//   7212 | ~unique_ptr @1833132 (349)    |   same
//   7232 | ~mutex      @1833080          | CMostraTelaContinuaVotacao::GetInst (inlined in 7481)
//   7239 | ~unique_ptr @1833104 (389)    |   same
//   7284 | ~mutex      @1833052          | CInstrucaoVotacaoAcessibilidade::GetInst (4420)
//   7323 | ~mutex      @1833024          | CIniciodeCiclo::GetInst (4433)
//   7333 | ~unique_ptr @1833048 (349)    |   same
//   7405 | ~mutex      @1832996          | CEleitorVotando::GetInst (3229)  (its unique_ptr: 7416, other unit)
//   7470 | ~mutex      @1832968          | CConfirmaVotoSemCandidato::GetInst (inlined in 4459)
//   7482 | ~mutex      @1832940          | CAguardaMensagem::GetInst (1337)
//   7485 | ~unique_ptr @1832964 (349)    |   same
//   7790 | ~mutex      @1832908          | UNKNOWN: no code references these two statics any more (the
//   7798 | ~unique_ptr @1832932 (349)    |   accessor did not survive optimisation). Table slots 866 (7798) and
//        |                               |   867 (7790) come right BEFORE slot 868, the first vtable slot of
//        |                               |   vota::CAssinadorVota. Every other pair in this table sits right
//        |                               |   before the first vtable slot of the class of its own TU (924->925
//        |                               |   CEleitorVotando, 896/897->898 CAguardaMensagem, 908->909
//        |                               |   CConfirmaVotoSemCandidato, 933/934->935 CIniciodeCiclo, 939->940
//        |                               |   CInstrucaoVotacaoAcessibilidade, 947/948->949 CMostraTelaContinua-
//        |                               |   Votacao, 952/953->954 CFimVotoEleitor, 957/958->959
//        |                               |   CSincronismoEleitor, 980->981 CThreadEleitor). The data agree: the next static,
//        |                               |   @1832936, is CSincronizaVota's "urna desligando" flag. So the
//        |                               |   owner is most likely a singleton with a trivial destructor in the
//        |                               |   TU that emits CAssinadorVota's vtable and holds that flag
//        |                               |   (vota/comum/csincronizavota.cpp or cassinadorvota.cpp). It is not
//        |                               |   CExecucaoVotaCooperativa: its slots 856-865 are followed, not
//        |                               |   preceded, by 866/867                                          ?

// wasm func 2125 (tools: vota_f2125): std::__tree<std::__value_type<uebyte, api::STick>, ...>::destroy(node*)
// - the recursive node deleter of api::CTickManager's std::map (trivially destructible values: left, right,
// free). Callers: ~CThreadVota (2126), ~CThreadEleitor (2438), ~CThreadOperador (2717).

// wasm func 2875 (tools: vota_f2875): std::vector<std::shared_ptr<T>>::__insert_with_size(pos, first, last, n)
// - libc++ range insert (in place when the capacity allows, otherwise split-buffer reallocation with 2x growth,
// max 536870911 elements). Identical-code-folded for every T: the calls from CGeraZeresimaBase::StartState
// (11946) and CGeraResumoZeresimaBase::StartState (11943) insert the "extrato do RDV" parts between the
// header and the trailer of the zerésima (`partes.insert(partes.begin() + 1, extrato.begin(), extrato.end())`);
// the RHVoice callers (813, 1245) are other instantiations folded into the same body.

} // namespace vota
