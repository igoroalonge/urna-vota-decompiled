// FRAGMENTS reconstructed from vota_web_wasm.wasm by unit u38 ("app:vota: classes without known file").
// Original files: the operator-side (mesário) files of uenux2/src/app/vota/operador/..., one section each.
// Paths marked "(path inferred)" follow the TSE convention (class CFooBar in cfoobar.cpp, next to its
// closest relatives); "(attested)" means the path appears in a std::source_location record.
//
// Contents:
//   * 70 exit-time destructor stubs ("__dtor_<variable>", never executed in this build) of the static
//     objects behind the operator states' lazy singletons, and of three static data members of
//     CControlaReconhecimento (the fingerprint buffers). The pattern and the evidence are explained at the
//     top of src/uenux2/src/app/vota/u38-foreign-fragments.cpp.
//   * 5 text sources (functions `std::string f()` passed as std::function<std::string()> to the
//     microterminal screens): funcs 10660, 10664 (written out below) and 10692/10693/10694 (already
//     written out by unit u27 in outrasopcoes/cescolheopcao.cpp; checked here).
//
// Each state's GetInst() and methods are reconstructed by other units (u10, u17, u19, u26, u27, u33); the
// comments name the accessor function so the pair "static <-> accessor <-> stub" can be followed.
//
// WEB BUILD: the operator thread never runs in the public simulator (docs/modules/u27 §2), so even the
// accessors are dead code there. The text source 10664 was checked with tools/bu/operator_harness.mjs:
// typing the unknown CPF XXX.XXX.XXX-XX on the operator terminal shows
//     "CPF: XXX.XXX.XXX-XX / CPF não encontrado. Digite o Título. / CONFIRMA: retornar".

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "comum/dados/cconfiguracaoeleicao.h"                    // GetTipoIdentificadorPrincipal (+668)
#include "comum/dados/md/eleitor/celeitoridentidade.h"           // ETipoIdentificadorEleitor
#include "vota/operador/comum/iinformacaothreadoperador.h"       // GetTipoIdentidadeDigitada (slot 24)
#include "vota/operador/confirmaidentidade/ccontrolareconhecimento.h"
#include "vota/operador/cthreadoperador.h"
#include "vota/operador/leidentidade/ieleitorimpedidovotar.h"
#include "vota/operador/outrasopcoes/cescolheopcao.h"
// (+ the headers of the other state classes named below)

namespace vota {

using uebyte = std::uint8_t;
using ETipoIdentificadorEleitor = ecourna::app::dados::ETipoIdentificadorEleitor;

// ======================================================================================================
// uenux2/src/app/vota/operador/cthreadoperador.cpp   (attested)
// ======================================================================================================
// CThreadOperador: CThreadOperador::GetInst (func 270).
std::mutex CThreadOperador::s_mutex;   // @1911684
std::unique_ptr<CThreadOperador> CThreadOperador::s_instancia;   // @1911708   (its stub: func 10208, not u38)
// wasm func 10207 (table slot 4593) - __dtor_CThreadOperador::s_mutex -> s_mutex.~mutex()   [body = func 150]

// ======================================================================================================
// uenux2/src/app/vota/operador/cfinalizaoperador.cpp   (path inferred)
// ======================================================================================================
// CFinalizaOperador: CFinalizaOperador::GetInst (func 5342 -> merged body 764).
std::mutex CFinalizaOperador::s_mutex;   // @1911628   (its stub: func 10215, not u38)
std::unique_ptr<CFinalizaOperador> CFinalizaOperador::s_instancia;   // @1911652
// wasm func 10216 (table slot 4581) - __dtor_CFinalizaOperador::s_instancia -> s_instancia.~unique_ptr()   [thunk -> func 349: free only (empty destructor body)]

// ======================================================================================================
// uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp   (attested)
// ======================================================================================================
// impl::IInformacaoThreadOperador: IInformacaoThreadOperador::GetInst (func 599, srcloc :554).
std::mutex impl::IInformacaoThreadOperador::s_mutex;   // @1908504
// wasm func 10553 (table slot 4004) - __dtor_IInformacaoThreadOperador::s_mutex -> s_mutex.~mutex()   [body = func 150]

// ======================================================================================================
// uenux2/src/app/vota/operador/aguardaeleitor/csuspensaoautomaticaeleitor.cpp   (attested)
// ======================================================================================================
// CSuspensaoAutomaticaEleitor: GetInst inlined in CEleitorDemorando::StartState (func 10440).
std::mutex CSuspensaoAutomaticaEleitor::s_mutex;   // @1909376
std::unique_ptr<CSuspensaoAutomaticaEleitor> CSuspensaoAutomaticaEleitor::s_instancia;   // @1909400   (its stub: func 10414, not u38)
// wasm func 10412 (table slot 4244) - __dtor_CSuspensaoAutomaticaEleitor::s_mutex -> s_mutex.~mutex()   [body = func 150]

// ======================================================================================================
// uenux2/src/app/vota/operador/aguardaeleitor/cperguntaeleitorvotando.cpp   (path inferred)
// ======================================================================================================
// CPerguntaEleitorVotando: GetInst inlined in CEleitorDemorando::ProcessInput (func 10439).
std::mutex CPerguntaEleitorVotando::s_mutex;   // @1909348
std::unique_ptr<CPerguntaEleitorVotando> CPerguntaEleitorVotando::s_instancia;   // @1909372
// wasm func 10417 (table slot 4236) - __dtor_CPerguntaEleitorVotando::s_mutex -> s_mutex.~mutex()   [body = func 150]
// wasm func 10418 (table slot 4235) - __dtor_CPerguntaEleitorVotando::s_instancia -> s_instancia.~unique_ptr()   [thunk -> func 389: ~ (ICF 244) + free]

// ======================================================================================================
// uenux2/src/app/vota/operador/aguardaeleitor/cmostraeleitorvotando.cpp   (attested)
// ======================================================================================================
// CMostraEleitorVotando: CMostraEleitorVotando::GetInst (func 1150).
std::mutex CMostraEleitorVotando::s_mutex;   // @1909292
std::unique_ptr<CMostraEleitorVotando> CMostraEleitorVotando::s_instancia;   // @1909316
// wasm func 10428 (table slot 4222) - __dtor_CMostraEleitorVotando::s_mutex -> s_mutex.~mutex()   [body = func 150]
// wasm func 10429 (table slot 4221) - __dtor_CMostraEleitorVotando::s_instancia -> s_instancia.~unique_ptr()   [thunk -> func 763: ~ (ICF 448) + free]

// ======================================================================================================
// uenux2/src/app/vota/operador/aguardaeleitor/celeitorvotounaovotou.cpp   (path inferred)
// ======================================================================================================
// CEleitorVotouNaoVotou: CEleitorVotouNaoVotou::GetInst (func 1901).
std::mutex CEleitorVotouNaoVotou::s_mutex;   // @1909264
std::unique_ptr<CEleitorVotouNaoVotou> CEleitorVotouNaoVotou::s_instancia;   // @1909288
// wasm func 10433 (table slot 4216) - __dtor_CEleitorVotouNaoVotou::s_mutex -> s_mutex.~mutex()   [body = func 150]
// wasm func 10434 (table slot 4215) - __dtor_CEleitorVotouNaoVotou::s_instancia -> s_instancia.~unique_ptr()   [thunk -> func 2903: ~ (ICF 1284) + free]

// ======================================================================================================
// uenux2/src/app/vota/operador/aguardaeleitor/celeitordemorando.cpp   (path inferred)
// ======================================================================================================
// CEleitorDemorando: CEleitorDemorando::GetInst (func 2734).
std::mutex CEleitorDemorando::s_mutex;   // @1909208
std::unique_ptr<CEleitorDemorando> CEleitorDemorando::s_instancia;   // @1909232
// wasm func 10441 (table slot 4203) - __dtor_CEleitorDemorando::s_mutex -> s_mutex.~mutex()   [body = func 150]
// wasm func 10442 (table slot 4202) - __dtor_CEleitorDemorando::s_instancia -> s_instancia.~unique_ptr()   [thunk -> func 2903: ~ (ICF 1284) + free]

// ======================================================================================================
// uenux2/src/app/vota/operador/confirmaidentidade/cverificadadoeleitor.cpp   (path inferred)
// ======================================================================================================
// CVerificaDadoEleitor: CVerificaDadoEleitor::GetInst (func 2735).
std::mutex CVerificaDadoEleitor::s_mutex;   // @1909180
std::unique_ptr<CVerificaDadoEleitor> CVerificaDadoEleitor::s_instancia;   // @1909204
// wasm func 10445 (table slot 4197) - __dtor_CVerificaDadoEleitor::s_mutex -> s_mutex.~mutex()   [body = func 150]
// wasm func 10446 (table slot 4196) - __dtor_CVerificaDadoEleitor::s_instancia -> s_instancia.~unique_ptr()   [thunk -> func 389: ~ (ICF 244) + free]

// ======================================================================================================
// uenux2/src/app/vota/operador/confirmaidentidade/cdadoeleitornaoconfere.cpp   (path inferred)
// ======================================================================================================
// CDadoEleitorNaoConfere: GetInst inlined in CVerificaDadoEleitor::ProcessInput (func 10443).
std::mutex CDadoEleitorNaoConfere::s_mutex;   // @1909152
std::unique_ptr<CDadoEleitorNaoConfere> CDadoEleitorNaoConfere::s_instancia;   // @1909176
// wasm func 10449 (table slot 4191) - __dtor_CDadoEleitorNaoConfere::s_mutex -> s_mutex.~mutex()   [body = func 150]
// wasm func 10450 (table slot 4190) - __dtor_CDadoEleitorNaoConfere::s_instancia -> s_instancia.~unique_ptr()   [thunk -> func 763: ~ (ICF 448) + free]

// ======================================================================================================
// uenux2/src/app/vota/operador/confirmaidentidade/ctentativacapturadigitalesgotada.cpp   (path inferred)
// ======================================================================================================
// CTentativaCapturaDigitalEsgotada: GetInst inlined in CRegistraDigitalOperador::ProcessTick (func 10451).
std::mutex CTentativaCapturaDigitalEsgotada::s_mutex;   // @1909068   (its stub: func 10462, not u38)
std::unique_ptr<CTentativaCapturaDigitalEsgotada> CTentativaCapturaDigitalEsgotada::s_instancia;   // @1909092
// wasm func 10463 (table slot 4170) - __dtor_CTentativaCapturaDigitalEsgotada::s_instancia -> s_instancia.~unique_ptr()   [thunk -> func 389: ~ (ICF 244) + free]

// ======================================================================================================
// uenux2/src/app/vota/operador/confirmaidentidade/cpededigital.cpp   (attested)
// ======================================================================================================
// CPedeDigital: CPedeDigital::GetInst (func 2740).
std::mutex CPedeDigital::s_mutex;   // @1909040
std::unique_ptr<CPedeDigital> CPedeDigital::s_instancia;   // @1909064   (its stub: func 10472, not u38)
// wasm func 10471 (table slot 4163) - __dtor_CPedeDigital::s_mutex -> s_mutex.~mutex()   [body = func 150]

// ======================================================================================================
// uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaoreconhecida.cpp   (attested)
// ======================================================================================================
// CDigitalNaoReconhecida: GetInst inlined in CPedeDigital::ObtemEstadoPosReconhecimentoBiometrico (func 5397).
std::mutex CDigitalNaoReconhecida::s_mutex;   // @1909012
std::unique_ptr<CDigitalNaoReconhecida> CDigitalNaoReconhecida::s_instancia;   // @1909036
// wasm func 10475 (table slot 4157) - __dtor_CDigitalNaoReconhecida::s_mutex -> s_mutex.~mutex()   [body = func 150]
// wasm func 10476 (table slot 4156) - __dtor_CDigitalNaoReconhecida::s_instancia -> s_instancia.~unique_ptr()   [thunk -> func 2903: ~ (ICF 1284) + free]

// ======================================================================================================
// uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaoreconhecidadecbiometria.cpp   (path inferred)
// ======================================================================================================
// CDigitalNaoReconhecidaDecBiometria: GetInst inlined in CDigitalNaoReconhecida::StartState (func 10474).
std::mutex CDigitalNaoReconhecidaDecBiometria::s_mutex;   // @1908984
std::unique_ptr<CDigitalNaoReconhecidaDecBiometria> CDigitalNaoReconhecidaDecBiometria::s_instancia;   // @1909008
// wasm func 10479 (table slot 4151) - __dtor_CDigitalNaoReconhecidaDecBiometria::s_mutex -> s_mutex.~mutex()   [body = func 150]
// wasm func 10480 (table slot 4150) - __dtor_CDigitalNaoReconhecidaDecBiometria::s_instancia -> s_instancia.~unique_ptr()   [thunk -> func 389: ~ (ICF 244) + free]

// ======================================================================================================
// uenux2/src/app/vota/operador/confirmaidentidade/cdigitalreconhecida.cpp   (attested)
// ======================================================================================================
// CDigitalReconhecida: GetInst inlined in CPedeDigital::ObtemEstadoPosReconhecimentoBiometrico (func 5397).
std::mutex CDigitalReconhecida::s_mutex;   // @1908956
std::unique_ptr<CDigitalReconhecida> CDigitalReconhecida::s_instancia;   // @1908980
// wasm func 10483 (table slot 4145) - __dtor_CDigitalReconhecida::s_mutex -> s_mutex.~mutex()   [body = func 150]
// wasm func 10484 (table slot 4144) - __dtor_CDigitalReconhecida::s_instancia -> s_instancia.~unique_ptr()   [thunk -> func 763: ~ (ICF 448) + free]

// ======================================================================================================
// uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaoreconhecidaportempo.cpp   (path inferred)
// ======================================================================================================
// CDigitalNaoReconhecidaPorTempo: GetInst inlined in CPedeDigital::ProcessTick (func 10465).
std::mutex CDigitalNaoReconhecidaPorTempo::s_mutex;   // @1908928
std::unique_ptr<CDigitalNaoReconhecidaPorTempo> CDigitalNaoReconhecidaPorTempo::s_instancia;   // @1908952
// wasm func 10487 (table slot 4139) - __dtor_CDigitalNaoReconhecidaPorTempo::s_mutex -> s_mutex.~mutex()   [body = func 150]
// wasm func 10488 (table slot 4138) - __dtor_CDigitalNaoReconhecidaPorTempo::s_instancia -> s_instancia.~unique_ptr()   [thunk -> func 763: ~ (ICF 448) + free]

// ======================================================================================================
// uenux2/src/app/vota/operador/confirmaidentidade/cnomeeleitor.cpp   (attested)
// ======================================================================================================
// CNomeEleitor: CNomeEleitor::GetInst (func 5401).
std::mutex CNomeEleitor::s_mutex;   // @1908816
std::unique_ptr<CNomeEleitor> CNomeEleitor::s_instancia;   // @1908840
// wasm func 10502 (table slot 4115) - __dtor_CNomeEleitor::s_mutex -> s_mutex.~mutex()   [body = func 150]
// wasm func 10503 (table slot 4114) - __dtor_CNomeEleitor::s_instancia -> s_instancia.~unique_ptr()   [thunk -> func 389: ~ (ICF 244) + free]

// ======================================================================================================
// uenux2/src/app/vota/operador/confirmaidentidade/cinformabiodesabilitadademo.cpp   (path inferred)
// ======================================================================================================
// CInformaBioDesabilitadaDemo: CInformaBioDesabilitadaDemo::GetInst (func 5402).
std::mutex CInformaBioDesabilitadaDemo::s_mutex;   // @1908760
std::unique_ptr<CInformaBioDesabilitadaDemo> CInformaBioDesabilitadaDemo::s_instancia;   // @1908784
// wasm func 10509 (table slot 4102) - __dtor_CInformaBioDesabilitadaDemo::s_mutex -> s_mutex.~mutex()   [body = func 150]
// wasm func 10510 (table slot 4101) - __dtor_CInformaBioDesabilitadaDemo::s_instancia -> s_instancia.~unique_ptr()   [thunk -> func 389: ~ (ICF 244) + free]

// ======================================================================================================
// uenux2/src/app/vota/operador/confirmaidentidade/ccontrolareconhecimento.cpp   (attested)
// ======================================================================================================
// CControlaReconhecimento: CControlaReconhecimento::GetInst (func 1256); static data members used by funcs 3614, 10425, 10451, 10465, 10512.
std::mutex CControlaReconhecimento::s_mutex;   // @1908732
std::unique_ptr<CControlaReconhecimento> CControlaReconhecimento::s_instancia;   // @1908756
// wasm func 10514 (table slot 4096) - __dtor_CControlaReconhecimento::s_mutex -> s_mutex.~mutex()   [body = func 150]
// wasm func 10515 (table slot 4095) - __dtor_CControlaReconhecimento::s_instancia -> s_instancia.~unique_ptr()   [thunk -> func 389: ~ (ICF 244) + free]

// Static data members of CControlaReconhecimento (declared in ccontrolareconhecimento.h by unit u10/u33,
// which also lists s_indiceDedo @1908668, s_score @1908708, s_qtdEleitores @1908712). Only the three with
// a non-trivial destructor get an exit-time stub:
std::vector<uebyte> CControlaReconhecimento::s_digitalCapturada;   // @1908672 voter's last capture (WSQ)
std::vector<uebyte> CControlaReconhecimento::s_digitalMesario;     // @1908684 mesário's capture (CRegistraDigitalOperador)
std::string         CControlaReconhecimento::s_tituloMesario;      // @1908696 título of the mesário who released the voter
// wasm func 10516 (table slot 4094) - __dtor_CControlaReconhecimento::s_tituloMesario
//                   -> s_tituloMesario.~basic_string()   [inline: if long mode (byte @1908707 < 0) free(data @1908696)]
// wasm func 10517 (table slot 4093) - __dtor_CControlaReconhecimento::s_digitalMesario
//                   -> s_digitalMesario.~vector()        [inline: if (begin) { end = begin; free(begin); }]
// wasm func 10518 (table slot 4092) - __dtor_CControlaReconhecimento::s_digitalCapturada
//                   -> s_digitalCapturada.~vector()      [same body, vector @1908672]
// Users of these members: funcs 3614 (BiometriaMesarioPresenteNosEleitores), 10425 (SalvaHabilitacaoEleitor),
// 10451 (CRegistraDigitalOperador::ProcessTick), 10465 (CPedeDigital::ProcessTick), 10512
// (CControlaReconhecimento::StartState, which only clear()s both vectors: end = begin, bytes kept). Note: the
// buffers are only freed at exit and never wiped (see "Weird or risky code" in the u38 doc).

// ======================================================================================================
// uenux2/src/app/vota/operador/confirmaidentidade/chabilitaaudioeleitor.cpp   (path inferred)
// ======================================================================================================
// CHabilitaAudioEleitor: CHabilitaAudioEleitor::GetInst (func 2743).
std::mutex CHabilitaAudioEleitor::s_mutex;   // @1908584
std::unique_ptr<CHabilitaAudioEleitor> CHabilitaAudioEleitor::s_instancia;   // @1908608
// wasm func 10525 (table slot 4082) - __dtor_CHabilitaAudioEleitor::s_mutex -> s_mutex.~mutex()   [body = func 150]
// wasm func 10526 (table slot 4081) - __dtor_CHabilitaAudioEleitor::s_instancia -> s_instancia.~unique_ptr()   [thunk -> func 389: ~ (ICF 244) + free]

// ======================================================================================================
// uenux2/src/app/vota/operador/leidentidade/cconfirmainspecionada.cpp   (path inferred)
// ======================================================================================================
// CConfirmaInspecionada: GetInst inlined in CAguardaInspecao::ProcessMessage (func 10530).
std::mutex CConfirmaInspecionada::s_mutex;   // @1908556
std::unique_ptr<CConfirmaInspecionada> CConfirmaInspecionada::s_instancia;   // @1908580
// wasm func 10528 (table slot 4076) - __dtor_CConfirmaInspecionada::s_mutex -> s_mutex.~mutex()   [body = func 150]
// wasm func 10529 (table slot 4075) - __dtor_CConfirmaInspecionada::s_instancia -> s_instancia.~unique_ptr()   [thunk -> func 389: ~ (ICF 244) + free]

// ======================================================================================================
// uenux2/src/app/vota/operador/leidentidade/caguardainspecao.cpp   (path inferred)
// ======================================================================================================
// CAguardaInspecao: GetInst inlined in CPedeIdentidade::ProcessTick (func 10676).
std::mutex CAguardaInspecao::s_mutex;   // @1908528
std::unique_ptr<CAguardaInspecao> CAguardaInspecao::s_instancia;   // @1908552
// wasm func 10531 (table slot 4070) - __dtor_CAguardaInspecao::s_mutex -> s_mutex.~mutex()   [body = func 150]
// wasm func 10532 (table slot 4069) - __dtor_CAguardaInspecao::s_instancia -> s_instancia.~unique_ptr()   [thunk -> func 389: ~ (ICF 244) + free]

// ======================================================================================================
// uenux2/src/app/vota/operador/leidentidade/cvalidaidentidade.cpp   (path inferred)
// ======================================================================================================
// CValidaIdentidade: GetInst inlined in CPedeIdentidade::ProcessInput (func 10677).
std::mutex CValidaIdentidade::s_mutex;   // @1905648
std::unique_ptr<CValidaIdentidade> CValidaIdentidade::s_instancia;   // @1905672   (its stub: func 10630, not u38)
// wasm func 10629 (table slot 3931) - __dtor_CValidaIdentidade::s_mutex -> s_mutex.~mutex()   [body = func 150]

// ======================================================================================================
// uenux2/src/app/vota/operador/justificativa/iiniciajustificativa.cpp   (attested)
// ======================================================================================================
// CIniciaJustificativa: GetInst inlined in CProcuraEleitor::StartState (func 10631).
std::mutex CIniciaJustificativa::s_mutex;   // @1905704
std::unique_ptr<CIniciaJustificativa> CIniciaJustificativa::s_instancia;   // @1905728   (its stub: func 10617, not u38)
// wasm func 10616 (table slot 3949) - __dtor_CIniciaJustificativa::s_mutex -> s_mutex.~mutex()   [body = func 150]

// ======================================================================================================
// uenux2/src/app/vota/operador/justificativa/iconfirmajustificativa.cpp   (attested)
// ======================================================================================================
// CConfirmaJustificativa: GetInst inlined in CProcuraEleitor::StartState (func 10631).
std::mutex CConfirmaJustificativa::s_mutex;   // @1905676
std::unique_ptr<CConfirmaJustificativa> CConfirmaJustificativa::s_instancia;   // @1905700   (its stub: func 10622, not u38)
// wasm func 10621 (table slot 3943) - __dtor_CConfirmaJustificativa::s_mutex -> s_mutex.~mutex()   [body = func 150]

// ======================================================================================================
// uenux2/src/app/vota/operador/confirmaidentidade/celeitorjavotou.cpp   (path inferred)
// ======================================================================================================
// CEleitorJaVotou: GetInst inlined in CEleitorEncontrado::StartState (func 10635).
std::mutex CEleitorJaVotou::s_mutex;   // @1905536
std::unique_ptr<CEleitorJaVotou> CEleitorJaVotou::s_instancia;   // @1905560
// wasm func 10644 (table slot 3909) - __dtor_CEleitorJaVotou::s_mutex -> s_mutex.~mutex()   [body = func 150]
// wasm func 10645 (table slot 3908) - __dtor_CEleitorJaVotou::s_instancia -> s_instancia.~unique_ptr()   [thunk -> func 389: ~ (ICF 244) + free]

// ======================================================================================================
// uenux2/src/app/vota/operador/leidentidade/ieleitorimpedidovotar.cpp   (path inferred)
// ======================================================================================================
// CEleitorOptouPorVotarEmTransito: GetInst inlined in CEleitorEncontrado::StartState (func 10635).
std::mutex CEleitorOptouPorVotarEmTransito::s_mutex;   // @1905396
std::unique_ptr<CEleitorOptouPorVotarEmTransito> CEleitorOptouPorVotarEmTransito::s_instancia;   // @1905420
// wasm func 10661 (table slot 3879) - __dtor_CEleitorOptouPorVotarEmTransito::s_mutex -> s_mutex.~mutex()   [body = func 150]
// wasm func 10662 (table slot 3878) - __dtor_CEleitorOptouPorVotarEmTransito::s_instancia -> s_instancia.~unique_ptr()   [thunk -> func 1959: ~IEleitorImpedidoVotar (1257) + free]

// CEleitorNaoEncontrado: CEleitorNaoEncontrado::GetInst (func 5424).
std::mutex CEleitorNaoEncontrado::s_mutex;   // @1905368
std::unique_ptr<CEleitorNaoEncontrado> CEleitorNaoEncontrado::s_instancia;   // @1905392
// wasm func 10665 (table slot 3870) - __dtor_CEleitorNaoEncontrado::s_mutex -> s_mutex.~mutex()   [body = func 150]
// wasm func 10666 (table slot 3869) - __dtor_CEleitorNaoEncontrado::s_instancia -> s_instancia.~unique_ptr()   [thunk -> func 1959: ~IEleitorImpedidoVotar (1257) + free]

// ------------------------------------------------------------------------------------------------------
// Text sources of the IEleitorImpedidoVotar screens (MT line 2). They are plain functions: the
// constructors wrap them in std::function<std::string()> whose target type is `std::string (*)()`
// (vtable @1542376 = __func<std::string(*)(), std::string()>), so the table slot of the function is stored
// as the callable. In the original they sit in the anonymous namespace of this file (u27 declares them in
// ieleitorimpedidovotar.cpp with these names).
// ------------------------------------------------------------------------------------------------------
namespace {

// wasm func 10660 (table slot 3876) - line 2 of CEleitorOptouPorVotarEmTransito. Used by the constructor
// inlined in CEleitorEncontrado::StartState (func 10635): IEleitorImpedidoVotar(&TextoOptouTransito,
// &TextoVazio /* slot 3877 = func 3628, " " */, RETORNA_CONFIRMA).
std::string TextoOptouTransito()                                   // name inferred (u27)
{
    return "Optou por votar em trânsito";                          // 27 chars (Latin-1), @126161
}

// wasm func 10664 (table slot 3867) - line 2 of CEleitorNaoEncontrado (constructor in func 5424).
// Observed with the operator harness (CPF branch). Both getters are called before the test, in this order.
std::string TextoNaoEncontrado()                                   // name inferred (u27)
{
    const ETipoIdentificadorEleitor digitado =
        impl::IInformacaoThreadOperador::GetInst().GetTipoIdentidadeDigitada();          // func 5416 -> 599, slot 24
    const comum::CConfiguracaoEleicao& configuracao = comum::CConfiguracaoEleicao::GetInst();   // func 187

    if (digitado == ETipoIdentificadorEleitor::CPF &&
        configuracao.GetTipoIdentificadorPrincipal() == ETipoIdentificadorEleitor::TITULO)      // cfg +668 == 1
        return "CPF não encontrado. Digite o Título.";              // 36 chars, @364898
    return "NÃO CADASTRADO nesta urna";                            // 25 chars, @226251
}

}  // namespace

// ======================================================================================================
// uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp   (attested)
// ======================================================================================================
// CEscolheOpcao: CEscolheOpcao::GetInst (func 2753).
std::mutex CEscolheOpcao::s_mutex;   // @1905284
std::unique_ptr<CEscolheOpcao> CEscolheOpcao::s_instancia;   // @1905308   (its stub: func 10697, not u38)
// wasm func 10696 (table slot 3832) - __dtor_CEscolheOpcao::s_mutex -> s_mutex.~mutex()   [body = func 150]

// Menu texts of CEscolheOpcao ("outras opções"): std::function<std::string()> text sources of COpcaoDS
// (built in func 2753, slots 3827..3830). u27 already wrote them out in cescolheopcao.cpp (lines 64-66,
// anonymous namespace); the bodies were re-checked here byte for byte:
// wasm func 10692 (table slot 3830) - std::string TextoOpcaoContadores()        { return "Exibir contadores"; }   // 17 chars, @74231
// wasm func 10693 (table slot 3829) - std::string TextoOpcaoRegistrarMesarios() { return "Registrar mesários"; }  // 18 chars, @66558
// wasm func 10694 (table slot 3828) - std::string TextoOpcaoEncerrar()          { return "Encerrar votação"; }    // 16 chars, @122964
// (The same literals are referenced again by CEscolheOpcao::ProcessInput, func 10689, for the log line
// "Operador selecionou: {}".)

// ======================================================================================================
// uenux2/src/app/vota/operador/outrasopcoes/ccontadoresbiometria.cpp   (path inferred)
// ======================================================================================================
// CContadoresBiometria: GetInst inlined in CEscolheOpcao::ProcessInput (func 10689).
std::mutex CContadoresBiometria::s_mutex;   // @1905256
std::unique_ptr<CContadoresBiometria> CContadoresBiometria::s_instancia;   // @1905280
// wasm func 10704 (table slot 3822) - __dtor_CContadoresBiometria::s_mutex -> s_mutex.~mutex()   [body = func 150]
// wasm func 10705 (table slot 3821) - __dtor_CContadoresBiometria::s_instancia -> s_instancia.~unique_ptr()   [thunk -> func 389: ~ (ICF 244) + free]

// ======================================================================================================
// uenux2/src/app/vota/operador/outrasopcoes/ciniciafinalizacao.cpp   (path inferred)
// ======================================================================================================
// CIniciaFinalizacao: GetInst inlined in CEscolheOpcao::ProcessInput (func 10689).
std::mutex CIniciaFinalizacao::s_mutex;   // @1905228
std::unique_ptr<CIniciaFinalizacao> CIniciaFinalizacao::s_instancia;   // @1905252
// wasm func 10707 (table slot 3814) - __dtor_CIniciaFinalizacao::s_mutex -> s_mutex.~mutex()   [body = func 150]
// wasm func 10708 (table slot 3813) - __dtor_CIniciaFinalizacao::s_instancia -> s_instancia.~unique_ptr()   [thunk -> func 349: free only (empty destructor body)]

// ======================================================================================================
// uenux2/src/app/vota/operador/outrasopcoes/chabilitacaoaudionaopermitida.cpp   (path inferred)
// ======================================================================================================
// CHabilitacaoAudioNaoPermitida: GetInst inlined in CEscolheOpcao::ProcessInput (func 10689).
std::mutex CHabilitacaoAudioNaoPermitida::s_mutex;   // @1904920
std::unique_ptr<CHabilitacaoAudioNaoPermitida> CHabilitacaoAudioNaoPermitida::s_instancia;   // @1904944
// wasm func 10755 (table slot 3737) - __dtor_CHabilitacaoAudioNaoPermitida::s_mutex -> s_mutex.~mutex()   [body = func 150]
// wasm func 10756 (table slot 3736) - __dtor_CHabilitacaoAudioNaoPermitida::s_instancia -> s_instancia.~unique_ptr()   [thunk -> func 389: ~ (ICF 244) + free]

// ======================================================================================================
// uenux2/src/app/vota/operador/outrasopcoes/cencerramentohorarioinvalido.cpp   (attested)
// ======================================================================================================
// CEncerramentoHorarioInvalido: GetInst inlined in CIniciaFinalizacao::StartState (func 10706).
std::mutex CEncerramentoHorarioInvalido::s_mutex;   // @1905200
std::unique_ptr<CEncerramentoHorarioInvalido> CEncerramentoHorarioInvalido::s_instancia;   // @1905224
// wasm func 10711 (table slot 3808) - __dtor_CEncerramentoHorarioInvalido::s_mutex -> s_mutex.~mutex()   [body = func 150]
// wasm func 10712 (table slot 3807) - __dtor_CEncerramentoHorarioInvalido::s_instancia -> s_instancia.~unique_ptr()   [thunk -> func 389: ~ (ICF 244) + free]

// ======================================================================================================
// uenux2/src/app/vota/operador/outrasopcoes/cperguntafilaeleitorvazia.cpp   (path inferred)
// ======================================================================================================
// CPerguntaFilaEleitorVazia: CPerguntaFilaEleitorVazia::GetInst (func 5426).
std::mutex CPerguntaFilaEleitorVazia::s_mutex;   // @1905172
std::unique_ptr<CPerguntaFilaEleitorVazia> CPerguntaFilaEleitorVazia::s_instancia;   // @1905196   (its stub: func 10717, not u38)
// wasm func 10716 (table slot 3802) - __dtor_CPerguntaFilaEleitorVazia::s_mutex -> s_mutex.~mutex()   [body = func 150]

// ======================================================================================================
// uenux2/src/app/vota/operador/outrasopcoes/caguardaeleitoresvotarem.cpp   (attested)
// ======================================================================================================
// CAguardaEleitoresVotarem: GetInst inlined in CPerguntaFilaEleitorVazia::ProcessInput (func 10713).
std::mutex CAguardaEleitoresVotarem::s_mutex;   // @1905144
std::unique_ptr<CAguardaEleitoresVotarem> CAguardaEleitoresVotarem::s_instancia;   // @1905168
// wasm func 10719 (table slot 3797) - __dtor_CAguardaEleitoresVotarem::s_mutex -> s_mutex.~mutex()   [body = func 150]
// wasm func 10720 (table slot 3796) - __dtor_CAguardaEleitoresVotarem::s_instancia -> s_instancia.~unique_ptr()   [thunk -> func 389: ~ (ICF 244) + free]

// ======================================================================================================
// uenux2/src/app/vota/operador/outrasopcoes/cpedetituloencerramento.cpp   (path inferred)
// ======================================================================================================
// CPedeTituloEncerramento: CPedeTituloEncerramento::GetInst (func 3631).
std::mutex CPedeTituloEncerramento::s_mutex;   // @1905116
std::unique_ptr<CPedeTituloEncerramento> CPedeTituloEncerramento::s_instancia;   // @1905140
// wasm func 10724 (table slot 3791) - __dtor_CPedeTituloEncerramento::s_mutex -> s_mutex.~mutex()   [body = func 150]
// wasm func 10725 (table slot 3790) - __dtor_CPedeTituloEncerramento::s_instancia -> s_instancia.~unique_ptr()   [thunk -> func 389: ~ (ICF 244) + free]

// ======================================================================================================
// uenux2/src/app/vota/operador/outrasopcoes/ctituloencerramentoinvalido.cpp   (path inferred)
// ======================================================================================================
// CTituloEncerramentoInvalido: GetInst inlined in CPedeTituloEncerramento::ProcessInput (func 10721).
std::mutex CTituloEncerramentoInvalido::s_mutex;   // @1905088
std::unique_ptr<CTituloEncerramentoInvalido> CTituloEncerramentoInvalido::s_instancia;   // @1905112   (its stub: func 10730, not u38)
// wasm func 10729 (table slot 3783) - __dtor_CTituloEncerramentoInvalido::s_mutex -> s_mutex.~mutex()   [body = func 150]

// ======================================================================================================
// uenux2/src/app/vota/operador/outrasopcoes/cencerramentoantecipado.cpp   (path inferred)
// ======================================================================================================
// CEncerramentoAntecipado: GetInst inlined in CPedeTituloEncerramento::StartState (func 10722).
std::mutex CEncerramentoAntecipado::s_mutex;   // @1905060   (its stub: func 10732, not u38)
std::unique_ptr<CEncerramentoAntecipado> CEncerramentoAntecipado::s_instancia;   // @1905084
// wasm func 10733 (table slot 3775) - __dtor_CEncerramentoAntecipado::s_instancia -> s_instancia.~unique_ptr()   [thunk -> func 389: ~ (ICF 244) + free]

// ======================================================================================================
// uenux2/src/app/vota/operador/outrasopcoes/cconfirmaencerramento.cpp   (path inferred)
// ======================================================================================================
// CConfirmaEncerramento: CConfirmaEncerramento::GetInst (func 3632).
std::mutex CConfirmaEncerramento::s_mutex;   // @1905032
std::unique_ptr<CConfirmaEncerramento> CConfirmaEncerramento::s_instancia;   // @1905056   (its stub: func 10736, not u38)
// wasm func 10735 (table slot 3770) - __dtor_CConfirmaEncerramento::s_mutex -> s_mutex.~mutex()   [body = func 150]

// ======================================================================================================
// uenux2/src/app/vota/operador/outrasopcoes/chorariovotacaoterminou.cpp   (path inferred)
// ======================================================================================================
// CHorarioVotacaoTerminou: CHorarioVotacaoTerminou::GetInst (func 5430).
std::mutex CHorarioVotacaoTerminou::s_mutex;   // @1904892
std::unique_ptr<CHorarioVotacaoTerminou> CHorarioVotacaoTerminou::s_instancia;   // @1904916
// wasm func 10760 (table slot 3731) - __dtor_CHorarioVotacaoTerminou::s_mutex -> s_mutex.~mutex()   [body = func 150]
// wasm func 10761 (table slot 3730) - __dtor_CHorarioVotacaoTerminou::s_instancia -> s_instancia.~unique_ptr()   [thunk -> func 389: ~ (ICF 244) + free]

// ======================================================================================================
// uenux2/src/app/vota/operador/outrasopcoes/cregistromesarioencerrado.cpp   (path inferred)
// ======================================================================================================
// CRegistroMesarioEncerrado: GetInst inlined in CControladorRegistraMesariosVota::GetEstadoAposRegistroVotacao (vf11, func 10793).
std::mutex CRegistroMesarioEncerrado::s_mutex;   // @1904864   (its stub: func 10764, not u38)
std::unique_ptr<CRegistroMesarioEncerrado> CRegistroMesarioEncerrado::s_instancia;   // @1904888
// wasm func 10765 (table slot 3724) - __dtor_CRegistroMesarioEncerrado::s_instancia -> s_instancia.~unique_ptr()   [thunk -> func 349: free only (empty destructor body)]

}  // namespace vota
