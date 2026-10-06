// FRAGMENTS reconstructed from vota_web_wasm.wasm by unit u39 ("app:vota: classes without known file").
//
// 31 functions of unit u39 are compiler-generated exit-time destructors ("atexit stubs") of static objects of
// the voter side of VOTA; with the two destructors of the message queue this file lists 33. They are filed under the
// original file that DEFINES each static (the stub is emitted in that translation unit), not under the
// function where the lazy accessor was inlined.
//
// Background (see the long note at the top of src/uenux2/src/app/vota/u38-foreign-fragments.cpp):
//   * every state is a lazy singleton "static std::mutex s_mutex; static std::unique_ptr<T> s_instancia;"
//     (the mutex is 24 bytes, the pointer lives at mutex + 24);
//   * clang emits one "__dtor_<variable>" stub per static with a non-trivial destructor, registered with
//     __cxa_atexit. With EXIT_RUNTIME=0 the registrations were optimised away; the stubs survive only because
//     their function-table slots do. They are DEAD CODE: never called in the simulator (and never on an urna
//     that is switched off without running exit handlers);
//   * ~mutex() = pthread_mutex_destroy, a no-op here: what is left is the ICF stub func 150 (tools name
//     "std::mutex::unlock()");
//   * ~unique_ptr<T>() = `if (p) delete p;` with the destructor devirtualised and merged by wasm-opt into
//     shared bodies that take the address as 2nd parameter:
//       func 349  delete of a class with a trivial destructor (free only)
//       func 389  destructor ICF 244 (one shared_ptr at +12, e.g. the screen) + free
//       func 763  destructor ICF 448 (shared_ptrs at +12 and +20) + free
//       func 1564 destructor ICF 785 (shared_ptr at +28, CEstadoComDesligamentoAutomatico subclasses) + free
//
// In the code below, `static void __dtor_x(void*)` stands for the compiler-generated stub; the variables are
// shown with their addresses. Where another unit already declared the static, only the stub is listed.
#include <memory>
#include <mutex>
#include <regex>
#include <string>
#include <utility>
#include <vector>

namespace vota {

// =========================================================================================================
// uenux2/src/app/vota/eleitor/celeitorvotando.cpp  (attested file; the globals are declared in
// celeitorvotando.h by u06: "g_votoDigitado @1833288, dtor func 13593", "g_votosEleitor @1833300, dtor 13588")
// =========================================================================================================
// wasm func 13593 (table slot 1078): __dtor_g_votoDigitado -> std::string::~string (long mode: free buffer)
// wasm func 13588 (table slot 1079): __dtor_g_votosEleitor -> ~vector<pair<TCargoID, comum::md::CVoto>>:
//   20-byte elements destroyed from the back (each holds a std::string at +8), then the buffer freed.

// =========================================================================================================
// uenux2/src/app/vota/eleitor/cvotacaostateaudio.cpp  (unit u08)
// =========================================================================================================
// wasm func 14464 (table slot 993): __dtor of the function-local
//     static const std::regex digitos("\\B\\d");            // @1833216, guard byte @1833256, flags 512 = ECMAScript
// used by CVotacaoStateAudio::FormataMensagem (slot 14, func 6999) to read "{voto}" digit by digit
// (regex_replace(s, digitos, ", $&") -> "1, 2, 3"). The body is the shared basic_regex destructor
// unknown_f6096(this, &__traits_.__loc_ @1833216, &__start_ @1833248): release the shared_ptr<__empty_state>
// __start_ (+32), then ~locale.

// =========================================================================================================
// uenux2/src/app/vota/eleitor/comum/cinformacaoeleitor.cpp  (attested by srcloc :181)
// =========================================================================================================
// CInformacaoEleitor::GetInst = wasm func 509: s_mutex @1833260, s_instancia @1833284 (12-byte object).
// wasm func 13685 (table slot 1077): __dtor_s_mutex      -> ~mutex (stub 150)
// wasm func 13695 (table slot 1076): __dtor_s_instancia  -> shared_f349 (trivial destructor: free)

// =========================================================================================================
// uenux2/src/app/vota/eleitor/comum/cpoliticaexecucaoeleitor.cpp  (attested by srcloc :45)
// =========================================================================================================
// IPoliticaExecucaoEleitor::GetInst() (inlined into func 5925) locks a static mutex @1833324 around the lazy
// registration of the urna default CPoliticaExecucaoEleitor.
// wasm func 13574 (table slot 1080): __dtor of that mutex -> ~mutex (stub 150)

// =========================================================================================================
// uenux2/src/app/vota/eleitor/iniciovotacao/cexibealertadesligamento.cpp  (unit u09)
// =========================================================================================================
// CExibeAlertaDesligamento::GetInst = wasm func 5979: s_mutex @1833992, s_instancia @1834016.
// wasm func 12021 (table slot 1560): __dtor_s_mutex -> ~mutex (stub 150)   (its partner 12022 is in u09)

// =========================================================================================================
// uenux2/src/app/vota/eleitor/fimvotacao/*.cpp - the ENCERRAMENTO (closing / BOLETIM DE URNA) states.
// One pair per state; accessor function and the vtable stored into the new object in brackets.
// =========================================================================================================

// fimvotacao/cgerabu.cpp - CGeraBU (generates the BU report + QR codes, u08), GetInst func 6129,
// 28-byte object (vtable @1540372)
//   wasm func 12111 (slot 1402): __dtor_CGeraBU::s_mutex      @1833472 -> stub 150
//   wasm func 12112 (slot 1401): __dtor_CGeraBU::s_instancia  @1833496 -> unknown_f763 (ICF 448 dtor + free)

// fimvotacao/cgerarelatorios.cpp - CGeraRelatorios (BUJ, BIM, BEHB), GetInst func 6084 (vtable @1540524)
//   wasm func 12106 (slot 1407): __dtor_CGeraRelatorios::s_mutex      @1833544 -> stub 150
//   wasm func 12107 (slot 1406): __dtor_CGeraRelatorios::s_instancia  @1833568 -> shared_f389

// fimvotacao/cimprimindobu.cpp - CImprimindoBU (prints via 1 of the BU), GetInst func 5986 (vtable @1541300)
//   wasm func 12079 (slot 1466): __dtor_CImprimindoBU::s_mutex      @1833628 -> stub 150
//   wasm func 12080 (slot 1465): __dtor_CImprimindoBU::s_instancia  @1833652 -> shared_f389

// fimvotacao/cquerimprimirbu.cpp - CQuerImprimirBU (voter training: print the BU?), inlined into
// CInicioBU::StartState (12062) (vtable @1542580). Its mutex stub (@1833880) is shared_f12036 (other unit).
//   wasm func 12037 (slot 1536): __dtor_CQuerImprimirBU::s_instancia  @1833904 -> shared_f389

// fimvotacao/cgravaresultado.cpp - CGravaResultado (writes and signs the result files, BU included),
// GetInst func 6037 = vota_f764 merged body (vtable @1540636). Mutex stub (@1833572) = shared_f12099.
//   wasm func 12100 (slot 1416): __dtor_CGravaResultado::s_instancia  @1833596 -> shared_f349

// fimvotacao/ccopiaresultadoparamr.cpp - CCopiaResultadoParaMR (copies the results to the MR = mídia de
// resultado), GetInst func 6174 (vtable @1540080)
//   wasm func 12145 (slot 1382): __dtor_CCopiaResultadoParaMR::s_mutex      @1833416 -> stub 150
//   wasm func 12152 (slot 1381): __dtor_CCopiaResultadoParaMR::s_instancia  @1833440 -> shared_f389

// fimvotacao/cimprimirbuoutrasobrigatorias.cpp - CImprimirBUOutrasObrigatorias (remaining mandatory BU
// copies), GetInst func 5984 = vota_f764 (vtable @1541748)
//   wasm func 12066 (slot 1487): __dtor_CImprimirBUOutrasObrigatorias::s_mutex      @1833740 -> stub 150
//   wasm func 12067 (slot 1486): __dtor_CImprimirBUOutrasObrigatorias::s_instancia  @1833764 -> shared_f349

// fimvotacao/cimprimirbjust.cpp - CImprimirBJust (boletim de justificativa), inlined into
// CImprimirBUOutrasObrigatorias::StartState (12065) (vtable @1541644)
//   wasm func 12069 (slot 1482): __dtor_CImprimirBJust::s_mutex      @1833712 -> stub 150
//   wasm func 12070 (slot 1481): __dtor_CImprimirBJust::s_instancia  @1833736 -> shared_f349

// fimvotacao/cimprimindobehb.cpp - CImprimindoBEHB (BEHB = report of the voters released BIOGRAPHICALLY, i.e. without
// fingerprint, in a biometric section: "ELEITORES HABILITADOS BIOGRAFICAMENTE"),
// inlined into CImprimindoBim::StartState (12071) (vtable @1541436)
//   wasm func 12075 (slot 1472): __dtor_CImprimindoBEHB::s_mutex      @1833656 -> stub 150
//   wasm func 12076 (slot 1471): __dtor_CImprimindoBEHB::s_instancia  @1833680 -> shared_f349

// fimvotacao/climitecopiasbuatingido.cpp - CLimiteCopiasBUAtingido ("Quantidade de vias adicionais excede o
// máximo permitido"), inlined into CVerificaQtdBUsAdicionais::StartState (12023) (vtable @1542836)
//   wasm func 12028 (slot 1549): __dtor_CLimiteCopiasBUAtingido::s_mutex      @1833936 -> stub 150
//   wasm func 12029 (slot 1548): __dtor_CLimiteCopiasBUAtingido::s_instancia  @1833960 -> shared_f389

// fimvotacao/cverificaqtdbusadicionais.cpp - CVerificaQtdBUsAdicionais (restart during the extra copies),
// inlined into CAjusteInicial::ValidaTemposDesligamento (7160) (vtable @1542924, 12 bytes)
//   wasm func 12024 (slot 1555): __dtor_CVerificaQtdBUsAdicionais::s_mutex      @1833964 -> stub 150
//   wasm func 12025 (slot 1554): __dtor_CVerificaQtdBUsAdicionais::s_instancia  @1833988 -> shared_f349

// fimvotacao/cmostraqrcodecertificado.cpp - CMostraQRCodeCertificado (QR code of the urna certificate,
// reached from the BU QR screen), inlined into CMostraQRCodeBU::ProcessInput (12051) (vtable @1542488, 36 bytes)
//   wasm func 12041 (slot 1531): __dtor_CMostraQRCodeCertificado::s_mutex      @1833852 -> stub 150
//   wasm func 12042 (slot 1530): __dtor_CMostraQRCodeCertificado::s_instancia  @1833876 -> unknown_f1564 (ICF 785)

// fimvotacao/cfinalizaaquisicao.cpp (path inferred: vtable @1540316 between CCopiaResultadoParaMR and
// CGeraBU) - CFinalizaAquisicao (after a restart in estadoVota FIMAQUISICAOVOTOS: posts operator message 10
// and waits in CAguardaMensagem, func 12113), inlined into CAjusteInicial::ValidaTemposDesligamento (7160)
//   wasm func 12114 (slot 1395): __dtor_CFinalizaAquisicao::s_mutex      @1833444 -> stub 150
//   wasm func 12115 (slot 1394): __dtor_CFinalizaAquisicao::s_instancia  @1833468 -> shared_f349

// =========================================================================================================
// uenux2/src/api/ipc/cmessagequeue.h (unit u18) - template instance api::CPriorityMessageQueue<api::SMessage>
// =========================================================================================================
// wasm func 3162 (vtable slot 0 of CPriorityMessageQueue<SMessage>, vota::CMessageEleitor and
// vota::CMessageOperador): ~CPriorityMessageQueue() - stores the base vptr (the derived classes' own vptr
// stores are dead and were removed, so the three destructors are one body), destroys m_pLock (+24,
// ISyncCtl, virtual deleting dtor), m_pSemaforo (+20, ISemaphore) and frees the heap vector (+4..+12).
// wasm func 3157 (slot 1 of the same three vtables): the same + operator delete.
// The tools named them "vota::CMessageEleitor::vf0/vf1" (first vtable seen). Declared in cmessagequeue.h:
//   virtual ~CPriorityMessageQueue() = default;

}  // namespace vota
