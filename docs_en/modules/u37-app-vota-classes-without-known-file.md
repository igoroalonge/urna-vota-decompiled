# u37: `app:vota` functions without a known source file (state-machine plumbing, lazy singletons, the "Mais informações" menu, log records, BU printing helpers)

Unit u37 collects **89 wasm functions** that the tools placed in component `app:vota` but could not attach to an
original file: no `std::source_location` record, and often no RTTI either. They are not one subsystem. They are the
glue that the rest of the voting application (VOTA) is built on. About a third are tiny functions whose bodies
wasm-opt **merged** with others ("merge-similar-functions": the differing constants became parameters) or
**folded** (identical-code folding). The remaining functions are small members of classes that other units already
reconstructed. Every function has been named, and each one was written out in the file it most likely comes
from. Some paths are **(path inferred)**: no record names those files, so they come from TSE's naming convention
(class `CFooBar` → `cfoobar.cpp`, in the directory of its relatives). Each reconstructed file says so in its first
lines.

**13 of the 89 functions ran** during the recorded votes (`analysis/runtime/*.functions.tsv`):
`CAppState::NeedChangeState` (7480), the state-context helpers `ProcessMessage`/`ProcessInput`/`ProcessTick`/
`AceitaTeclado` (3843, 5911, 5910, 5909), the tick poller `CTickManager::GetTicksExpirados` (5450, on every
`votaTick`), `CEleitorVotando::GetInst` (3229), the `IConfereVotoEmCargo` constructor (1165, on every vote), the
application-context copy/remove pair (1841, 5553), the `IForm` constructor body (6024, every screen), and two
functions of the "Mais informações" menu that `CTelasVota` builds at start-up (`CItemMenu::~CItemMenu` 2937,
`CItemImprimeEstadoUrna::Disponivel` 11669). A 14th function certainly ran too, although the profiler never
sampled it: the merged singleton body 764. Its thunk `CAguardaMensagem::GetInst` (1337, called by `main`) was
sampled, and that thunk calls 764 unconditionally.

| topic | functions | § |
|---|---|---|
| state machine plumbing: `comum::CAppState`, `comum::CAppStateContext`, the tick poller | 7480, 3842, 3843, 5908-5911, 5450, 2125 | 3 |
| lazy singletons of states (merged body 764 + 7 thunks, 8 inlined constructors) | 764, 3864, 3879, 5342, 5983-5985, 6037, 1256, 1279, 1897, 2888, 3229, 5947, 5954, 1903 | 4 |
| at-exit destructors of the singletons' statics (dead code) | 7116 … 7798 (17 functions), 1286 | 4.3 |
| "Mais informações" menu: `CMenuBase`, `CItemMenu` and four report items | 12235, 6188, 2937, 11666-11669 | 5 |
| voting screens: conferência timing, voter cycle context, screens | 1165, 3229, 1841, 5553, 6024, 5547, 3890, 6587, 2902 | 6 |
| event-log records (`CLogVota`, `CControladorRegistraMesariosVota`) | 2282, 3279, 4550, 4556, 5881, 5885, 3887, 3888, 6017, 5895 | 7 |
| persistence: `eg.bin`, `vota.bin` counters, the "imprimindo" marker, MR, WSQ directories | 3333, 3705, 3701, 1487, 1488, 2863, 3795, 5818, 5819, 1381 | 8 |
| BU: pieces of the encerramento flow | 3879, 5983-5985, 6037, 2888, 3701, 1487/1488, 1540, 4556, 5885, 2863, 5547, 5918 | 9 |
| misc. members of other classes | 677, 2687, 2803, 1393, 5903, 5918, 5948, 5951, 3667, 2875 | 10 |

**Reconstructed sources** (new files are marked *new*; `*.u37.*` files are fragments to merge into the named file):

```
src/uenux2/src/app/comum/cappstate.u37.h                        new   comum::CAppState (class, layout, slots)
src/uenux2/src/app/comum/cappstate.u37.cpp                      new   NeedChangeState + 6 CAppStateContext helpers
src/uenux2/src/app/comum/citemmenu.h                            new   comum::CItemMenu
src/uenux2/src/app/comum/citemimprimeestadourna.{h,cpp}         new   comum::CItemImprimeEstadoUrna
src/uenux2/src/app/comum/citemimprimelistaeleitores.{h,cpp}     new   comum::CItemImprimeListaEleitores
src/uenux2/src/app/comum/citemparametrosurna.{h,cpp}            new   comum::CItemParametrosUrna
src/uenux2/src/app/comum/citemversoespacotes.{h,cpp}            new   comum::CItemVersoesPacotes
src/uenux2/src/app/comum/cmenubase.h                            new   comum::CMenuBase (declaration)
src/uenux2/src/app/comum/cmenubase.u37.cpp                            ~CMenuBase (u02 has AdicionaItem/Monta)
src/uenux2/src/app/comum/appinfo/cappinfo.u37.cpp                     comum::GravaEstadoGeral (eg.bin)
src/uenux2/src/app/comum/cinfomtlcd.u37.cpp                           CInfoMTLCD::ExibeBateria
src/uenux2/src/app/comum/iinterfaceinit.u37.cpp                       LogInfo, IInterfaceInit::HabilitarMR
src/uenux2/src/app/comum/informacao/cinformacaoeleicao.u37.cpp        ImprimeBoletimJustificativa
src/uenux2/src/app/comum/dados/celeitores.u37.cpp                     CEleitores::GetBiometriaCorrente
src/uenux2/src/app/comum/dados/md/cvalidadoridentidade.u37.cpp        CValidadorIdentidade::EhValida
src/uenux2/src/app/comum/dados/md/estadoaplicacao/estadoaplicacao.u37.cpp   IncrementaQtdBU, SetAjusteDataHora
src/uenux2/src/app/comum/relatorios/cgeradorrelpu.h                   shared with unit u35 (see note in the file)
src/uenux2/src/app/comum/relatorios/cgeradorrelpu.u37.cpp             CGeradorRelPU::AdicionaLinha
src/uenux2/src/api/util/cdatetime.u37.cpp                             CDateTime::ToTimeT
src/uenux2/src/api/util/ctickmanager.u37.cpp                          CTickManager::GetTicksExpirados
src/uenux2/src/api/gui/capplicationcontextstack.u37.cpp               CApplicationContext copy ctor, Stack::Remove
src/uenux2/src/api/gui/u37-template-instances.cpp                     IForm ctor body, builder Add bodies
src/uenux2/src/app/vota/u37-foreign-fragments.cpp                     singletons, small members, compiler-generated code
src/uenux2/src/app/vota/log/clogvota.u37.cpp                          5 CLogVota records + IEventosLog::LogaErro
src/uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.u37.cpp  8 mesário-registration log slots
src/uenux2/src/app/vota/comum/crelvotautil.u37.cpp                    the "dinamico/imprimindo" marker
src/uenux2/src/app/vota/eleitor/cconferevotoemcargo.u37.cpp           IConfereVotoEmCargo constructor
src/uenux2/src/app/vota/operador/outrasopcoes/cimpressaopu.u37.cpp    FormataData (PU report)
```

Glossary: *urna* voting machine; *eleitor* voter; *mesário* poll worker; *terminal do mesário* / MT = the poll
worker's microterminal; *título (de eleitor)* voter registration number; *habilitação* releasing a voter to vote;
*conferência* the short "check your vote" screen after the last digit; *zerésima* the zero report printed before
voting; *BU (boletim de urna)* the per-urna result report and file; *via* printed copy; *encerramento* closing the
vote; *MI / MV* internal flash (`/dsk/fi`) / removable voting-media flash (`/dsk/fe`); *MR (mídia de resultado)*
the USB stick that carries the result files to the Junta Eleitoral; *PU (parâmetros de urna)* the per-election
urna parameters (`-pu.dat`, `ecourna::app::dados::CParametrosUrna` at `CConfiguracaoEleicao +88`); *WSQ* the
fingerprint-image format; *treinamento* training phase; *modo demonstração* demonstration mode.

---

## 1. Where this code sits in the voting process

* **Every screen of the urna is a state** (`comum::CAppState`). The voter terminal (`vota::CThreadEleitor`) and
  the operator terminal (`vota::CThreadOperador`) each drive one state machine through a
  `comum::CAppStateContext`. On each loop iteration a thread (1) passes queued inter-thread messages to the
  current state, (2) passes one key press, (3) passes the expired timer ticks, and (4) switches state when
  `NeedChangeState()` says so. In the web build the voter thread has no thread of its own: `votaTick` calls
  `CThreadEleitor::Processar` (4349, formerly shown by the tools as `ProcessarEntrada`), and that is where 3843,
  5909-5911, 5450 and 7480 were seen running.
* **States are lazy singletons.** Section 4 lists the accessors of this unit: the voter cycle `CEleitorVotando`,
  the encerramento/BU chain (`CInicioBU`, `CGravaResultado`, `CImprimirBUOutrasObrigatorias`, `CImprimindoBim`,
  `CEmitirMaisBU`, `CMostraQRCodeBU`), the start-of-day states (`CVerificaHorarioZeresima`, the candidate viewer),
  and operator states (`CControlaReconhecimento`, `CSincronismoOperador`, `CFinalizaOperador`,
  `CReinicioComparecimentoMesario`).
* **Before the zerésima**, the "Mais informações" menu lets the mesário print a limited number of extra copies of
  four reports (§5).
* **During the vote**, the `IConfereVotoEmCargo` constructor sets how long the conferência screen stays up (§6.1).
  `CEleitorVotando` pushes an error context that names the vote as *not recorded* if anything fails while the voter
  is choosing (§6.2).
* **At the end of the day** the unit provides the pieces that print BU copies and count them (§9).

---

## 2. Classes and hierarchy (RTTI)

```
api::CState (typeinfo @1551296, no vtable)
 └─ comum::CAppState                          vtable @1551248 (9 slots, table below)            cappstate.u37.h
     ├─ every vota:: and comum:: state (CAguardaMensagem, CEleitorVotando, CGeraBU, CInicioBU, ...)
     ├─ vota::CVotacaoStateAudio ─ vota::IConfereVotoEmCargo (@1547584) ─ CConfereVotoEmCargo<TConfirma, TELA> (10)
     └─ vota::CEstadoComDesligamentoAutomatico (@1541908) ─ testeteclado::CBase (@1545776, abstract) ─ CPreZeresima, CRetomada
                                                         ─ CVerificaHorarioZeresima, CMostraQRCodeBU, ...
api::CStateContext<comum::CAppState> (typeinfo @1551332) ─ comum::CAppStateContext (vtable @1551312)   (u35)

comum::CItemMenu (typeinfo @1539904, vtable @1539892: [0] 2937, [1] 325, [2] pure Disponivel)   citemmenu.h
 ├─ comum::CItemImprimeEstadoUrna     @1550788  [2] 11669 [3] pure ─ vota::CItemImprimeEstadoUrnaVota     @1539644 [3] 12267
 ├─ comum::CItemImprimeListaEleitores @1550824  [2] 11668 [3] pure ─ vota::CItemImprimeListaEleitoresVota @1539680 [3] 12266
 ├─ comum::CItemParametrosUrna        @1550860  [2] 11667 [3] pure ─ vota::CItemParametrosUrnaVota        @1539752 [3] 12255
 ├─ comum::CItemVersoesPacotes        @1550896  [2] 11666 [3] pure ─ vota::CItemVersoesPacotesVota        @1539716 [3] 12259
 └─ vota::CItemVisualizarCandidatosVota @1539788 [2] 12244 (u02)
comum::CMenuBase (typeinfo @1550944, vtable @1550932: [0] 12235 [1] 6188 [2] Monta 5913) ─ vota::CMenuMaisInformacoesVota @1539820
comum::CGeradorRelPU (typeinfo @1545296, vtable @1545288: [0] 11901 [1] 11900)     (u35 + u37)
```

The four comum item vtables, `CMenuBase` and `api::COptionValidation` (used by `CMenuBase::Monta`) are contiguous in
the data segment (@1550788 … @1550996). That fits one translation unit per class compiled in alphabetical order
(`citemimprimeestadourna` < `citemimprimelistaeleitores` < `citemparametrosurna` < `citemversoespacotes` <
`cmenubase`). The same layout would also appear if everything were in `cmenubase.cpp`.

`comum::CAppState` slots (names: 3 and 7 from srcloc-named overrides, the rest from their use by the threads):

| slot | function | default body |
|---:|---|---|
| 0 / 1 | `~CAppState()` / deleting | 174 (trivial) / 325 (unreachable, abstract) |
| 2 | `StartState()` | pure |
| 3 | `NeedChangeState()` | **7480** `return GetNextState() != this` |
| 4 | `GetNextState()` | 1661 `return m_proximoEstado` (+4) |
| 5 | `FinishState()` | no-op |
| 6 | `ProcessMessage(uebyte)` | no-op |
| 7 | `ProcessInput()` | no-op |
| 8 | `ProcessTick(uebyte)` | no-op |

Layout (12 bytes, constructor `shared_f224(this, flags)`): `+4 m_proximoEstado = this`, and the three input flags
from the constructor's bit mask: `+8` messages (1), `+9` keys (2), `+10` ticks (4).
`CEstadoComDesligamentoAutomatico` always adds ticks (`flags | 4`).

---

## 3. State-machine plumbing

### 3.1 `CAppStateContext` helpers (3842, 3843, 5908, 5909, 5910, 5911)

Six out-of-line one-liners on the context (`+4 CAppState* m_estado`). Each one tolerates an empty context:
`AceitaMensagens/AceitaTeclado/AceitaTicks` return the state's flag (+8/+9/+10) or false.
`ProcessMessage/ProcessInput/ProcessTick` forward to slots 6/7/8 when there is a state. Both thread loops use them
(CThreadEleitor::Processar 4349, CThreadOperador::Run 10204). They could also be members of the template
base `api::CStateContext<CAppState>`, because they only touch public members of the state.

### 3.2 Ticks: `api::CTickManager::GetTicksExpirados` (5450)

Each VOTA thread owns a `std::map<uebyte, STick>` (unit u20: `AddTick`, `AddStoppedTick`, `StartTick`,
`StopTick`). The poller:

1. reads the wall clock with `gettimeofday` (the JS clock in the web build);
2. walks the map in id order and skips stopped ticks (`+24 == 1`) and ticks whose `proximo` is still in the future;
3. for each expired tick, appends its id to the result and **re-arms** it. The new expiry is `proximo + k·intervalo`,
   where k is the number of whole periods elapsed plus one. The binary computes this in mixed double/32-bit integer
   arithmetic (reproduced in `ctickmanager.u37.cpp`). **Missed periods are collapsed**: a thread that was busy for
   several periods receives the tick once.

The thread then calls `ProcessTick(id)` for each id (5910), provided that the state accepts ticks.

### 3.3 `NeedChangeState` (7480)

A state asks to be left by writing another state into `m_proximoEstado`. `NeedChangeState` compares
`GetNextState()` with `this`. States that override slot 3 (`CEleitorVotando::NeedChangeState` 7352,
`CDefineRotaPosReinicio::NeedChangeState` 11858) add their own conditions.

---

## 4. Lazy singletons

### 4.1 The merged body 764 and its thunks

Every state is created on first use. The source pattern is
`static std::mutex m; static std::unique_ptr<T> p; lock_guard; if (!p) p.reset(new T); return *p;`. The build has no
pthreads, so `lock()` disappeared and only the unlock stub (func 150) remains. For the 12-byte states whose
constructor is just `CAppState(flags)` plus a vptr, wasm-opt merged all the GetInst bodies into
**764** `(mutex, &unique_ptr, vtable, flags)`. It has 19 callers. The ones in this unit:

| wasm | accessor | statics (mutex / unique_ptr) | flags |
|---:|---|---|:-:|
| 3864 | `CReinicioComparecimentoMesario::GetInst` | @1834328 / @1834352 | 0 |
| 3879 | `CEmitirMaisBU::GetInst` | @1833600 / @1833624 | 2 |
| 5342 | `CFinalizaOperador::GetInst` | @1911628 / @1911652 | 0 |
| 5983 | `CInicioBU::GetInst` | @1833768 / @1833792 | 0 |
| 5984 | `CImprimirBUOutrasObrigatorias::GetInst` | @1833740 / @1833764 | 0 |
| 5985 | `CImprimindoBim::GetInst` | @1833684 / @1833708 | 0 |
| 6037 | `CGravaResultado::GetInst` | @1833572 / @1833596 | 0 |

### 4.2 Accessors with an inlined constructor

| wasm | accessor | object | notes |
|---:|---|---|---|
| 3229 | `CEleitorVotando::GetInst` ✓ | 72 B, CAppState(7) | context `(11, "Erro inesperado durante a votação", "O voto do eleitor NÃO foi registrado", "Ocorreu um erro enquanto o eleitor registrava suas escolhas.")`, inactivity tick = PU `tempoDispararSuspensaoTE` (+84) in voter training, else `tempoDispararSuspensao` (+80), × 1000 ms |
| 5947 | `CVerificaHorarioZeresima::GetInst` | 52 B | form `CTelasVota +20` ("telaAntesHorarioZeresima"), zerésima date/time `cfg +544`, stopped 2 s tick |
| 2888 | `CMostraQRCodeBU::GetInst` | 36 B, `CEstadoComDesligamentoAutomatico(2)` | +28 form, built in StartState |
| 1279 | `CVisualizarCandidatos::GetInst` | 40 B, CAppState(2) | three optional filters (cargo +24, número +28, partido +36) |
| 5954 | `CMenuFiltrarCandidatosPorNumero::GetInst` | 20 B, CAppState(2) | +12 form |
| 1897 | `CSincronismoOperador::GetInst` | 12 B, CAppState(1) | +11 `m_suspensaoAutomatica = false` |
| 1256 | `CControlaReconhecimento::GetInst` | 20 B, CAppState(2) | +12 = `CriaFormEleitorPodeVotar()` (5410), then touches 1903 |
| 1903 | `ConfiguracaoExtrator::GetInst` (name from u22) | 16 B | `{8, 500, 500/2.54}` = bits per pixel, ppi, pixels per cm. Constant configuration of the fingerprint-template extractor. It is created just before each (stubbed) extraction |

### 4.3 At-exit destructors (dead code) and the unknown singleton

The 17 tiny functions 7116 … 7798 are the `__cxa_atexit` destructors of these statics: `~mutex` is the pthread
stub 150, and `~unique_ptr` is `shared_f349` (plain free) or `shared_f389` (dtor 244 + free). Emscripten builds
without `EXIT_RUNTIME`, so atexit is a no-op, the registration calls are gone and **no code calls these functions**.
They survive only because the function table still references them. The owner of each static appears in the
mapping table and in `u37-foreign-fragments.cpp`. **7790/7798 (@1832908/@1832932)** belong to a singleton whose
accessor no longer exists: no instruction loads either address. The table layout points to its translation
unit. Every other at-exit pair of this unit sits **right before the first vtable slot of the class of its own
file**:

| at-exit slots | next slot |
|---|---|
| 924 | 925 `CEleitorVotando` |
| 896/897 | 898 `CAguardaMensagem` |
| 908 | 909 `CConfirmaVotoSemCandidato` |
| 933/934 | 935 `CIniciodeCiclo` |
| 939 | 940 `CInstrucaoVotacaoAcessibilidade` |
| 947/948 | 949 `CMostraTelaContinuaVotacao` |
| 952/953 | 954 `CFimVotoEleitor` |
| 957/958 | 959 `CSincronismoEleitor` |
| 963 | 964 `impl::CSincronismoVotoEleitor` (its slot [1]; [0] reuses slot 340) |
| 980 | 981 `CThreadEleitor` |

Slots 866/867 are followed by slot 868, the first vtable slot of **`vota::CAssinadorVota`**. The data agree: the
next static, @1832936, is `CSincronizaVota`'s "urna desligando" flag. The unknown singleton therefore most likely
lives in the file that emits `CAssinadorVota`'s vtable and holds that flag (`vota/comum/csincronizavota.cpp` or
`cassinadorvota.cpp`). It does not come from `vota::CExecucaoVotaCooperativa`: that class's vtable slots
(856-865) come *before* 866/867, not after them, which is the opposite of the pattern.

`vota_f1286` is a merged at-exit body of the same kind, `(atexit argument, &unique_ptr)`. The 10 at-exit stubs of
the `s_inst` of the `CConfereVotoEmCargo<…>` instantiations in unit u30 use it. It deletes through 1717, the
`~IConfereVotoEmCargo` that all 10 template classes inherit in vtable slot 0.

---

## 5. The "Mais informações" menu (`CMenuBase`, `CItemMenu`, four report items)

`CTelasVota::CriaTelaMaisInformacoes` (6599) builds, once, at start-up:

```
Mais informações
[1] - Estado da urna (n/max)          comum::CItemImprimeEstadoUrna     vias: EstadoGeralVota +73
[2] - Lista de eleitores (n/max)      comum::CItemImprimeListaEleitores vias: +74
[3] - Versões de pacotes (n/max)      comum::CItemVersoesPacotes        vias: +75
[4] - Parâmetros de urna (n/max)      comum::CItemParametrosUrna        vias: +76
[5] - Visualizar candidatos           vota::CItemVisualizarCandidatosVota
Escolha a sua opção: [_]
```

* `Disponivel()` (slot 2) returns `GetNumViasImpressas() < max`, **unsigned**. `GetNumViasImpressas()` (slot 3) is
  the vota subclass reading `vota.bin` (`EstadoGeralVota.numViasImpressasRelatorios`). `max` is the PU value
  `numRelatorioEstado / numRelatorioEleitores / numRelatorioVersoesDados / numRelatorioPU`, read through
  `CInformacaoEleicao`, and forced to **1 in demonstration mode**.
* "Lista de eleitores" also requires an urna type for the current turno of vota ('1'), contingenciavota ('3') or
  contingenciavotarecupera ('4') (`EstadoGeralUrna.dadoCarga.tipoUrnaT1/T2`, CEstadoGeral +36/+40). A plain
  contingência urna ('2') or an urna without a type ('0') never offers the voter list.
* **Correction to unit u02** (`cmenubase.u02.cpp`): `AdicionaItem` (2285) adds the id to the accepted options only
  when `item->Disponivel()`. `Monta` (5913) draws unavailable items in colour 5 (grey) instead of 2. An exhausted
  report is therefore still listed, but it cannot be selected. The runtime edges confirm that `2285 → 11669` and
  `5913 → 11669` ran.
* Destructors: `~CMenuBase` 12235/6188 (string `m_opcoes` +16, then the `vector<shared_ptr<CItemMenu>>` +4), and
  `~CItemMenu` 2937 (a merged body that destroys the string at +8).

Selecting an item prints the report and increments the counter in `vota.bin`. For "Parâmetros de urna" this is
`vota::CImpressaoPU` (11902, unit u25). It prints the four voting-day instants of the configuration
(`boost::posix_time::ptime` at cfg +504…+528) through `CGeradorRelPU::AdicionaLinha` (677: label + value
right-aligned to column 38), `FormataHorario` (2686, u25) and `FormataData` (2687, `"{:02}/{:02}/{:04}"`).

---

## 6. Voting screens

### 6.1 Conferência timing: `IConfereVotoEmCargo(ETelaVotacao)` (1165) ✓

`CPede*` builds the state for every conferência (`CConfereVotoEmCargo<TConfirma, TELA>`, 10 instantiations) with
`new(40)` + this constructor + the template vptr:

* `CVotacaoStateAudio(6)`: keys and ticks, no messages;
* `m_tela` (+30) = the screen id (the only parameter). The header of unit u06 declared a second `bool comTick`
  parameter; **it does not exist**;
* `m_tick` (+28) = a *stopped* tick of `ParametrosUrna.tempoConfirmacaoVoto` ms (`CParametrosUrna +88`, cfg +176;
  1000 in the published scenarios), created on the voter thread only when the value is > 0. `m_comTick` (+29)
  records whether it exists;
* `m_espera` (+32) empty.

StartStateAudio starts the tick. When it fires, the CONFIRMA/CORRIGE screen follows. Keys pressed in between are
rejected and logged as *"Tecla indevida pressionada"*: the voter has to press CONFIRMA again once the confirmation
screen appears. This is the 1.5 s pause that `tools/run/headless.mjs` needs.

### 6.2 The voter cycle's error context (3229, 1841, 5553) ✓

`CEleitorVotando` owns an `api::CApplicationContext` (+20). `IniciaCiclo` pushes a copy of it (1841 = copy
constructor), and `FinishState` removes it (5553, `CApplicationContextStack::Remove`: reverse search for an equal
context, then erase). While it is on the stack, any fatal error during the vote shows *"O voto do eleitor NÃO foi
registrado / Ocorreu um erro enquanto o eleitor registrava suas escolhas."* with action 11 (restart the urna; if
the error persists, photograph the QR code). `~CApplicationContextGuard` (675) contains the same removal code, but
skips it when `std::uncaught_exceptions() != 0`, so that the context of an operation that is failing stays on top
for the fatal-error screen.

### 6.3 Screens

* `IForm<MEDIA>::IForm` (6024, merged for IScreen 5548 and IScreenMT 5522) ✓: copies the fields and the pre-show
  hook, allocates the 128-byte `FormControlBlock`, and calls `SetForm(this)` on every field.
* `CFormBuilder` "add a field" bodies: 3890 (a 28-byte field with one argument: new-line on paper, LED, beep) and
  5547, which adds a `CImageField` for the QR-code images of `CMostraQRCodeBU` / `CMostraQRCodeCertificado`. The
  wasm parameters of 5547 are `(sret, builder, image, position)`:
  * the image comes before the position;
  * the image is a `shared_ptr` passed **by value**, and the callee releases it (libc++ ABI v2 makes `shared_ptr`
    trivial_abi);
  * the anchor `1` is a constant in the body.

  So 5547 is not a plain instance of the variadic forwarding `Add<FIELD>(ARGS&&...)` of `cformbuilder.u07.cpp`,
  which would forward `(position, image, 1)` by reference. It is a dedicated overload, or a by-value instance
  whose constant argument LTO removed.
* `CTelasVota::CriaTelaAguarde` (6587): `CriaTelaNeutra("Por favor, aguarde...", 35, false)`, shown while the
  zerésima prints.
* 2902: merged destructor (vptr + release of the `shared_ptr` screen at +12) of `IEleitorImpedidoVotar`,
  `IConfirmaJustificativa`, `CGeraZeresimaBase` and `CGeraResumoZeresimaBase`.
* `testeteclado::CBase::CBase` (5948): base of the keypad test before the zerésima,
  `CEstadoComDesligamentoAutomatico(2)` plus an empty screen.

---

## 7. Event-log records

All records go to `/dsk/fi/dinamico/log/logd.dat` (`"<app>|<severity>|<text>"`, Latin-1, app 1 = VOTA, severity
1 info / 2 warning / 3 error).

| wasm | method | severity | text |
|---:|---|:-:|---|
| 2282 | `IEventosLog::LogaErro(msg)` | 3 | (argument) |
| 3279 | `CLogVota::LogaErroEstadoDesconhecido` | 3 | Erro estado do aplicativo não conhecido |
| 4550 | `CLogVota::LogaErroPartidoNaoEncontrado` | 3 | Erro partido não encontrado |
| 4556 | `CLogVota::LogaQtdViasExcedeMaximo` (body 6115) | 2 | Quantidade de vias adicionais excede o máximo permitido |
| 5881 | `CLogVota::LogaEstadoNaoEsperado` (body 3902) | 3 | Erro estado do aplicativo não esperado |
| 5885 | `CLogVota::LogaMRNaoPresente` (body 3902) | 3 | Mídia de resultado não estava presente |
| 3888 | slots 29/34/36 of `CControladorRegistraMesariosVota` | 1 | Operador encerrou ciclo de registro de mesários / Realizada a conferência da biometria do mesário / Operador indagado se finaliza registro mesários |
| 3887 | slots 31/32/33 | 1 | Pedido de leitura da biometria do mesário {título} / Mesário {título} é eleitor da seção / Mesário {título} não é eleitor da seção |
| 6017 | slots 19/27 | 1 | Mesário {título} registrado / Mesário {título} já registrado |

The bodies with the same message length were merged by wasm-opt: the 8-byte chunks of the literal became
parameters. That explains why 3888, 3902 and 6115 take 5-7 pointers into one string. 5895 is not an event-log
record: it is the `LOG_INFO` copy of the system-log helper `LogSistema` (2921) in `iinterfaceinit.cpp`. It writes
to syslog on a real urna (`/dev/urna` exists) and to stdout when `DEBUG_UENUX` is set.

---

## 8. Persistence and files

| what | function | details |
|---|---|---|
| `eg.bin` (EstadoGeralUrna) on MI and MV | `comum::GravaEstadoGeral` 3333 | checks the shutdown flag @1832936 (throws `CUeDesligandoError`). **MI**: under the guard (2, "Gravando o estado geral da urna na MI", "Ocorreu um erro durante a sincronização do estado geral da urna na MI.") it writes `dinamico/eg.bin` (CServicoEstadoGeral(0).Salva), `vota::CAssinadorVota(120).Assina(25)` signs it into `/dsk/fi/dinamico/eg.vsu` (1501 stores `CAssinadorVota`'s vptr), then sync. **MV**: under the guard (4, "... na MV", "... na MV.") it writes eg.bin, copies `eg.vsu` MI → MV, then sync. Called after the clock is moved: `CAjusteInicial`, `CIniciodeCiclo::AjustaDataHora`, `CEncerramentoHorarioInvalido`. The body uses VOTA-only classes (`CAssinadorVota`, `CSincronizaVota`'s flag), as its twin `SalvaEstado` (491) does, so `comum`/`cappinfo.cpp` is uncertain: `vota/comum/csincronizavota.cpp` is an equally good home |
| clock adjustment record | `CEstadoGeral::SetAjusteDataHora` 3705 | `EstadoGeralUrna.ajusteDataHora` (+52: tipo, valor) |
| local time → time_t | `CDateTime::ToTimeT` 1381 | `timegm` of the civil date/time (the urna keeps local time "as UTC") |
| number of BU copies | `CEstadoGeralVota::IncrementaQtdBU` 3701 | `vota.bin` `qtdBU` (+8, one byte although ASN.1 allows 0..999) |
| printing marker | `CRelVotaUtil::MarcaImpressaoEmAndamento` 1488 / `RemoveMarcaImpressaoEmAndamento` 1487 | creates (`CFile "wb"`) / securely removes `/dsk/fi/dinamico/imprimindo` around every print job, each followed by `sync`. Only 1487/1488 reference the path, and the only read is 1487's own existence test before the removal |
| result medium | `IInterfaceInit::HabilitarMR` 2863 | init-daemon command 10 "habilitando MR", then `sleep 500 ms` |
| WSQ directories | 5819 / 5818 / 3795 | `<MI>/dinamico/trab<turno>/wsq/{habilitado,nao-habilitado,operador}/`: fingerprint images of enabled voters, voters not enabled, and mesários, packed later into `wsqbio.jez`, `wsqman.jez`, `wsqmes.jez` |
| PU parameters read | 1165, 3229, 5918, 11666-11669 | `tempoConfirmacaoVoto` (+88), `tempoDispararSuspensao(TE)` (+80/+84), `aceitarJustificativa` (+394), `numRelatorio*` (+56…+68) |

---

## 9. BOLETIM DE URNA: the parts of the encerramento that live in this unit

The BU flow itself (generation `CGeraBU`, printing `CImprimindoBU::ImprimeBU`, result files `CGravaResultado`, copy
to the MR, QR codes) is documented step by step in units u08/u09 (`docs/modules/u09-…md` §4-5) and
`docs/10-boletim-de-urna.md`. This unit contributes the following pieces, in flow order:

1. **Restart routing.** After a reboot, `CAjusteInicial::InicioVota` maps `EstadoGeralVota.estadoVota` to a state:
   60 → `CGeraRelatorios`, **61 → `CInicioBU::GetInst` (5983)**, **62 → `CGravaResultado::GetInst` (6037)**,
   63 → `CCopiaResultadoParaMR`. For 64 (encerrada) it looks at `estadoEncerramento`: '1'/'2' →
   **`CImprimirBUOutrasObrigatorias::GetInst` (5984)**, '3' → `CRetirarMR`, '4' → `CVerificaQtdBUsAdicionais`.
   Any other value logs **"Erro estado do aplicativo não conhecido" (3279)** and throws 9303/9304.
2. **First copy.** `CGeraRelatorios` → `CInicioBU` → `CImprimindoBU`. Around each print job:
   `MarcaImpressaoEmAndamento` (1488) creates `dinamico/imprimindo`, `CSigVerifier(trab, "bu.dat", "bu.vsu")`
   (1540) lets the printer service check the image's signature before printing, then
   `RemoveMarcaImpressaoEmAndamento` (1487). When the mesário accepts the quality, `qtdBU` is incremented
   (**3701**) and `vota.bin` is saved. `CImprimindoBU` then goes to `CEmitirMaisBU::GetInst` (3879, voter
   training) or `CGravaResultado::GetInst` (6037).
3. **Result files → MR.** `CCopiaResultadoParaMR::CopiaResultado` calls **`HabilitarMR` (2863)**: command 10
   "habilitando MR" to the init daemon, then a 500 ms sleep. If the stick is missing it logs **"Mídia de resultado
   não estava presente" (5885)**. It continues to **`CImprimirBUOutrasObrigatorias` (5984)**.
4. **Mandatory copies.** For each remaining copy up to `numBUVotaObrigatorios` (1 in demo mode) it runs 1488 →
   print → 1487 and **3701**. Then **`ImprimeBoletimJustificativa` (5918)** (PU `aceitarJustificativa`, *no* demo
   override) decides whether the "boletim de justificativa" (`buj.dat`) is printed. After that comes the mesário
   report `CImprimindoBim::GetInst` (**5985**, when mesários were identified), then `CRetirarMR`.
5. **Extra copies.** `CEmitirMaisBU` prints further copies up to `numBUVotaObrigatorios + numBUVotaAdicionais`
   (**3701** for each). Beyond that, `CLimiteCopiasBUAtingido` / `CEmitirMaisBU` log **"Quantidade de vias
   adicionais excede o máximo permitido" (4556, warning)**.
6. **QR codes on screen.** **`CMostraQRCodeBU::GetInst` (2888)** shows the BU QR codes as `CImageField`s created
   by **5547** (wasm parameters `(image by value, position)`, anchor 1 constant). `CMostraQRCodeCertificado` uses the same builder for
   the certificate QR code.

Exact data: `qtdBU` is `EstadoGeralVota.qtdBU` (`vota.bin`, byte +8 in memory, INTEGER 0..999 in ASN.1). The
marker is `/dsk/fi/dinamico/imprimindo` (empty file). The signature pair is `trab/bu.dat` + `trab/bu.vsu`
(`CSigVerifier` +4 ESavdAplic 1, +8 directory, +20 file, +32 signature). The MR command id is 10 with the text
"habilitando MR" (11 "desabilitando MR" is inlined in `CAjusteInicial`).

---

## 10. Other members reconstructed here

* `comum::md::CValidadorIdentidade::EhValida(tipo, identidade)` (2803): the non-throwing twin of `Valida`. It uses
  the same rule search (the first rule in the list that is either the "free identifier" rule, tipo 3, or of the
  requested type) and returns false when no
  rule exists. Used to check the títulos typed to suspend a vote, to close the vote, and to register a mesário.
* `comum::CEleitores::GetBiometriaCorrente()` (1393): an out-of-line copy of `GetCurrent().GetBiometria()`, used by
  six operator states.
* `comum::CInfoMTLCD::ExibeBateria()` (5903): re-attaches the MT LCD to the battery-icon observable only if it is
  not attached yet, and pushes the current icon at once.
* `api::CProgressBar::SetValor` (3667): reconstructed by unit u16 (`cprogressbar.cpp`), listed here for the map.
* Library code: 2125 (`std::map` node deleter of the tick table), 2875 (`vector<shared_ptr<T>>` range insert,
  identical-code-folded with RHVoice instantiations), 1286 (`unique_ptr<IConfereVotoEmCargo>::reset`).

---

## 11. Web build specifics

* The web page never leaves the voter's voting loop, so none of the encerramento, zerésima, menu, mesário or
  fingerprint functions above run in the simulator. The two exceptions are the start-up construction of the "Mais
  informações" form and the voting-loop plumbing (§3, §6).
* `HabilitarMR` (2863) is compiled as `if (byte @1584624 == 1) emscripten_sleep(500)`. The byte is 1 and the module
  has no Asyncify, so reaching it **aborts** the program. It is unreachable from the page.
* `GravaEstadoGeral` would call the web SAVD mock: the "signatures" are the text `assinatura simulada para
  vota_web_wasm`.
* Time comes from the browser: `gettimeofday` in the tick poller, `timegm` in `ToTimeT`. The web
  `CWasmSystemDateTime` cannot move the JS clock, so the clock jumps of the real urna do not happen.

## 12. Wasm / Emscripten observations

* **merge-similar-functions** created the parameterised bodies 764 (vtable + flags), 2902 / 6024 / 3890 (vtables),
  3887 / 3888 / 6017 / 3902 / 6115 (string chunks or format strings), and 2921 (priority + per-TU cache).
* **Identical code folding**: 2875 is shared with RHVoice instantiations, which explains the "rhvoice" callers.
  2937's body `shared_f1727` is shared by every class whose destructor only destroys a string at +8.
* **Orphaned table entries**: the 17 at-exit destructors are unreachable. Their registrations disappeared with the
  no-op `__cxa_atexit` of a build without `EXIT_RUNTIME`, but the table slots were kept. Their table order is
  still useful: each file's at-exit stubs sit right before the first vtable slot of that file's class. This is
  how §4.3 places 7790/7798. The at-exit bodies themselves are merged, with the static's address as a parameter:
  `shared_f349` / `shared_f389` / `vota_f1286`.
* **`shared_ptr` is trivial_abi** (libc++ ABI v2): a `shared_ptr` passed by value goes by pointer to the caller's
  copy and is **released by the callee**. Examples: 5547, which releases its image argument, and 426
  (`CFormBuilder::Add`).
* The **function-local static mutexes** survive only as the unlock stub `std::mutex::unlock` (func 150), called on
  the static's address.
* Three instances of **dead computation left by the source**: the unused detail string of `GravaEstadoGeral`, the
  unused result of `ConfiguracaoExtrator::GetInst()` in `CControlaReconhecimento`, and the
  `std::to_string`/`std::stoi` round trip of the year in `FormataData`.

---

## 13. Corrections to other units' reconstructions

| where | correction |
|---|---|
| `src/uenux2/src/app/comum/cmenubase.u02.cpp` (2285) | the option id is added only when `item->Disponivel()` (slot 2), not whenever `item` is non-null; `Monta` greys unavailable items (colour 5) |
| `src/uenux2/src/app/vota/eleitor/cconferevotoemcargo.h` | the constructor takes only `ETelaVotacao`; `m_comTick` = `tempoConfirmacaoVoto > 0` |
| callers of `comum::GravaEstadoGeral` (u06/u10) | the guards of 3333 get an **empty** detail string (unlike `SalvaEstado`, 491) |
| `src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp` | `CSincronismoOperador` is more likely in `operador/` than in `operador/aguardaeleitor/` (static layout) |
| `src/uenux2/src/app/comum/relatorios/cgeradorrelpu.h` | u37 overwrote u35's first version of this header by accident, because the two units ran in parallel. The file now holds a merged version with u35's out-of-line destructor declaration kept |
| `src/uenux2/src/app/vota/eleitor/cajusteinicial.cpp` (7160) | (u37 review, applied) `CAjusteInicial::StartState` pushes `CControladorRegistraMesariosVota` when `CInformacaoEleicao::IdentificaMesarios()` (func 1950 = `!demo && PU.registrarMesarios`, +400) is true, not when `EhModoDemonstracao()` is true. The VOTA mesário-registration controller therefore runs on real urnas and never in demonstration mode |
| `src/uenux2/src/api/gui/capplicationcontextstack.u15.h` | (u37 review, not applied) `CApplicationContext +48` (`bool m_generico` there) is stored (5557), copied (1841) and compared (5556) as a 32-bit value: more likely an `int`/enum than a `bool` |
| `src/uenux2/src/app/comum/citemmenu.h` / `cmenubase.u02.cpp` | (u37 review, applied to citemmenu.h) `CItemMenu +24` is one `api::SFont {20, 0}` (8 bytes, as u02's "SFonte"), not two ints; the text is copied (const reference) |

---

## 14. Suspicious or weird code

| # | function | finding | who is affected | severity |
|---:|---|---|---|:-:|
| 1 | 2863 `IInterfaceInit::HabilitarMR` | `emscripten_sleep(500)` behind the always-true flag @1584624 in a build without Asyncify: reaching it aborts the wasm. Callers: the MR clean-up of `CAjusteInicial` (training phase) and the encerramento copy to the MR | simulator only, and only if a future page drove the encerramento (not reachable today); a real urna just waits 500 ms | low |
| 2 | 5450 `CTickManager::GetTicksExpirados` | the period is converted to microseconds in a 32-bit `int` (`intervalo * 1000`): a tick longer than 2 147 s (35.8 min) overflows (UB; wraps in wasm) and gets a wrong next expiry. Missed periods are collapsed into one `ProcessTick`, and the clock is the wall clock (`gettimeofday`), not a monotonic one, so a backwards clock change delays every running tick until the clock catches up | real urna (clock adjustments by `CAjusteInicial`/`CIniciodeCiclo` in training/demo). No valid parameter reaches the overflow. The ticks created are 1 s, 2 s, `tempoConfirmacaoVoto` (ASN.1 0..99 999 ms; 1 s in the scenarios) and `tempoDispararSuspensao(TE)` (15..360 s; 45 s in the scenarios). All of them are far below 2 147 s | low |
| 3 | 3333 `GravaEstadoGeral` | computes the detail text of the current error context (or "Erro de sincronização") and never uses it: both guards get an empty detail, unlike its twin `SalvaEstado` (491). An eg.bin write failure therefore shows a fatal-error screen without the detail line | real urna, error path only | info |
| 4 | 3887 / 6017 | the mesário's **título de eleitor** goes in clear into the event log `logd.dat` ("Mesário {} registrado", "... é eleitor da seção", "Pedido de leitura da biometria do mesário {}"). Urna logs are among the files that leave the urna (log.jez on the MR, `CGravadorLog`). Whether the TSE filters them before publishing was not checked | poll workers' privacy on **real** urnas. `CAjusteInicial` installs this controller only when `IdentificaMesarios()` is true (func 1950: not demonstration mode and PU `registrarMesarios`, which is TRUE in the published scenarios). It never runs in demonstration mode or in the web build | low |
| 5 | 3701 `IncrementaQtdBU` | `qtdBU` is `INTEGER (0..999)` in ASN.1 but one unchecked byte in C++ | nobody in practice (copies are capped by the PU parameters) | info |
| 6 | 5918 `ImprimeBoletimJustificativa` | the only `CInformacaoEleicao` getter without the demonstration-mode override: a demo urna prints the boletim de justificativa according to the real PU value | demonstration urnas; probably intentional | info |
| 7 | 2687 `FormataData` | a special `ptime` (not_a_date_time/±infinity, e.g. a missing time window in the configuration) reaches `from_day_number`, which throws `boost::gregorian::bad_year`, uncaught in `CImpressaoPU`. The year also takes a pointless `to_string`/`stoi` round trip | real urna "Parâmetros de urna" report with malformed configuration | info |
| 8 | 1165 `IConfereVotoEmCargo` | by design, CONFIRMA pressed during the conferência (PU `tempoConfirmacaoVoto`, 1 s) is discarded as "Tecla indevida pressionada". The discard is not silent: `PlayKey(0)` gives the error beep and writes the log record (unit u06). A voter who presses CONFIRMA once, too early, sees the confirmation screen and must press it again; nothing is recorded until then. Re-checked with `headless.mjs --keys "91001C  C  12  C  "`: the first C leaves the state at `CConfereVotoEmCargo<…>`, and the C after the pause confirms | voters (UX only: no vote is recorded without the second CONFIRMA) | info |
| 9 | 7790 / 7798 | at-exit destructors of a singleton whose accessor no longer exists (statics @1832908/@1832932 never referenced; by table and data layout probably in the `CAssinadorVota`/`CSincronizaVota` translation unit, see §4.3); all 17 at-exit destructors of the unit are unreachable | nobody (dead code) | info |
| 10 | 1487 / 1488 | the `dinamico/imprimindo` marker is written and removed by VOTA. No other code in this binary reads it: the only read is 1487's own existence test before the removal. Its consumer, presumably detection of a print interrupted by a power cut, lies outside this module | none in the simulator | info |

---

## 15. Mapping table (all 89 functions)

✓ = observed executing in the recorded votes; (✓) = not sampled, but it must have run because a sampled caller
calls it unconditionally. "library/inlined helper" = not TSE source code (compiler-generated or
a libc++ instantiation), with the reason.

| wasm | size | ran | reconstructed symbol | original file | reconstructed in |
|---:|---:|:-:|---|---|---|
| 677 | 223 |  | `comum::CGeradorRelPU::AdicionaLinha` | uenux2/src/app/comum/relatorios/cgeradorrelpu.cpp (path inferred) | src/uenux2/src/app/comum/relatorios/cgeradorrelpu.u37.cpp |
| 764 | 71 | (✓) | `vota::ObtemEstadoSimples<T> [merged GetInst body of 12-byte states]` | merged body: the GetInst() of each 12-byte state, one per state file under uenux2/src/app/vota/ | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 1165 | 91 | ✓ | `vota::IConfereVotoEmCargo::IConfereVotoEmCargo` | uenux2/src/app/vota/eleitor/cconferevotoemcargo.cpp | src/uenux2/src/app/vota/eleitor/cconferevotoemcargo.u37.cpp |
| 1256 | 109 |  | `vota::CControlaReconhecimento::GetInst` | uenux2/src/app/vota/operador/confirmaidentidade/ccontrolareconhecimento.cpp | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 1279 | 139 |  | `vota::CVisualizarCandidatos::GetInst` | uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cvisualizarcandidatos.cpp | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 1286 | 29 |  | `std::unique_ptr<vota::IConfereVotoEmCargo>::reset [at-exit body]` | library/inlined helper (libc++ unique_ptr, instantiated in uenux2/src/app/vota/eleitor/votamajoritario, votaproporcional) | src/uenux2/src/app/vota/eleitor/cconferevotoemcargo.u37.cpp (comment) |
| 1381 | 132 |  | `api::CDateTime::ToTimeT` | uenux2/src/api/util/cdatetime.cpp | src/uenux2/src/api/util/cdatetime.u37.cpp |
| 1393 | 15 |  | `comum::CEleitores::GetBiometriaCorrente` | uenux2/src/app/comum/dados/celeitores.cpp | src/uenux2/src/app/comum/dados/celeitores.u37.cpp |
| 1487 | 587 |  | `vota::CRelVotaUtil::RemoveMarcaImpressaoEmAndamento` | uenux2/src/app/vota/comum/crelvotautil.cpp (path inferred) | src/uenux2/src/app/vota/comum/crelvotautil.u37.cpp |
| 1488 | 405 |  | `vota::CRelVotaUtil::MarcaImpressaoEmAndamento` | uenux2/src/app/vota/comum/crelvotautil.cpp (path inferred) | src/uenux2/src/app/vota/comum/crelvotautil.u37.cpp |
| 1540 | 123 |  | `comum::CSigVerifier::CSigVerifier` | uenux2/src/app/comum/relatorios/csigverifier.cpp | src/uenux2/src/app/comum/relatorios/csigverifier.h (unit u25, inline) |
| 1841 | 351 | ✓ | `api::CApplicationContext::CApplicationContext(const CApplicationContext&)` | uenux2/src/api/gui/capplicationcontextstack.cpp | src/uenux2/src/api/gui/capplicationcontextstack.u37.cpp |
| 1897 | 98 |  | `vota::CSincronismoOperador::GetInst` | uenux2/src/app/vota/operador/csincronismooperador.cpp (path inferred) | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 1903 | 102 |  | `ConfiguracaoExtrator::GetInst` | uenux2/src/app/vota/operador/confirmaidentidade/ccontrolareconhecimento.cpp (path inferred) | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 2125 | 32 |  | `std::__tree<std::__value_type<uebyte, api::STick>>::destroy` | library/inlined helper (libc++ <map> for api::CTickManager) | src/uenux2/src/app/vota/u37-foreign-fragments.cpp (comment) |
| 2282 | 14 |  | `comum::IEventosLog::LogaErro` | uenux2/src/app/comum/log/ieventoslog.h | src/uenux2/src/app/vota/log/clogvota.u37.cpp |
| 2687 | 983 |  | `vota::(anonymous namespace)::FormataData` | uenux2/src/app/vota/operador/outrasopcoes/cimpressaopu.cpp (path inferred) | src/uenux2/src/app/vota/operador/outrasopcoes/cimpressaopu.u37.cpp |
| 2803 | 114 |  | `comum::md::CValidadorIdentidade::EhValida` | uenux2/src/app/comum/dados/md/cvalidadoridentidade.cpp | src/uenux2/src/app/comum/dados/md/cvalidadoridentidade.u37.cpp |
| 2863 | 135 |  | `comum::IInterfaceInit::HabilitarMR` | uenux2/src/app/comum/iinterfaceinit.cpp (path inferred) | src/uenux2/src/app/comum/iinterfaceinit.u37.cpp |
| 2875 | 1093 |  | `std::vector<std::shared_ptr<T>>::__insert_with_size` | library/inlined helper (libc++ <vector>, identical-code-folded for several T) | src/uenux2/src/app/vota/u37-foreign-fragments.cpp (comment) |
| 2888 | 104 |  | `vota::CMostraQRCodeBU::GetInst` | uenux2/src/app/vota/eleitor/fimvotacao/cmostraqrcodebu.cpp | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 2902 | 63 |  | `[merged destructor body: vptr + shared_ptr<+12> release]` | library/inlined helper (compiler-generated ~X() of IEleitorImpedidoVotar, IConfirmaJustificativa, CGeraZeresimaBase, CGeraResumoZeresimaBase) | src/uenux2/src/app/vota/u37-foreign-fragments.cpp (comment) |
| 2937 | 12 | ✓ | `comum::CItemMenu::~CItemMenu` | uenux2/src/app/comum/citemmenu.h (path inferred) | src/uenux2/src/app/comum/citemmenu.h |
| 3229 | 586 | ✓ | `vota::CEleitorVotando::GetInst` | uenux2/src/app/vota/eleitor/celeitorvotando.cpp | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 3279 | 150 |  | `vota::CLogVota::LogaErroEstadoDesconhecido` | uenux2/src/app/vota/log/clogvota.cpp | src/uenux2/src/app/vota/log/clogvota.u37.cpp |
| 3333 | 1260 |  | `comum::GravaEstadoGeral` | uenux2/src/app/comum/appinfo/cappinfo.cpp (path inferred) | src/uenux2/src/app/comum/appinfo/cappinfo.u37.cpp |
| 3667 | 112 |  | `api::CProgressBar::SetValor` | uenux2/src/api/gui/cprogressbar.cpp | src/uenux2/src/api/gui/cprogressbar.cpp (unit u16) |
| 3701 | 15 |  | `comum::md::estadoaplicacao::CEstadoGeralVota::IncrementaQtdBU` | uenux2/src/app/comum/dados/md/estadoaplicacao/cestadogeralvota.cpp | src/uenux2/src/app/comum/dados/md/estadoaplicacao/estadoaplicacao.u37.cpp |
| 3705 | 12 |  | `comum::md::estadoaplicacao::CEstadoGeral::SetAjusteDataHora` | uenux2/src/app/comum/dados/md/estadoaplicacao/cestadogeral.cpp | src/uenux2/src/app/comum/dados/md/estadoaplicacao/estadoaplicacao.u37.cpp |
| 3795 | 15 |  | `vota::DiretorioWsqOperador` | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp (path inferred) | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 3842 | 23 |  | `comum::CAppStateContext::AceitaMensagens` | uenux2/src/app/comum/cappstate.cpp (path inferred) | src/uenux2/src/app/comum/cappstate.u37.cpp |
| 3843 | 27 | ✓ | `comum::CAppStateContext::ProcessMessage` | uenux2/src/app/comum/cappstate.cpp (path inferred) | src/uenux2/src/app/comum/cappstate.u37.cpp |
| 3864 | 22 |  | `vota::CReinicioComparecimentoMesario::GetInst` | uenux2/src/app/vota/eleitor/creiniciocomparecimentomesario.cpp (path inferred) | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 3879 | 22 |  | `vota::CEmitirMaisBU::GetInst` | uenux2/src/app/vota/eleitor/fimvotacao/cemitirmaisbu.cpp | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 3887 | 458 |  | `vota::CControladorRegistraMesariosVota::LogaTituloMesario [merged body of slots 31-33]` | uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp (path inferred) | src/uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.u37.cpp |
| 3888 | 151 |  | `vota::CControladorRegistraMesariosVota::Loga47 [merged body of slots 29, 34, 36]` | uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp (path inferred) | src/uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.u37.cpp |
| 3890 | 372 |  | `api::CFormBuilderBase<MEDIA>::AddCampo<FIELD>(arg) [merged body]` | library/inlined helper (template of uenux2/src/api/gui/cformbuilder.h) | src/uenux2/src/api/gui/u37-template-instances.cpp |
| 4550 | 138 |  | `vota::CLogVota::LogaErroPartidoNaoEncontrado` | uenux2/src/app/vota/log/clogvota.cpp | src/uenux2/src/app/vota/log/clogvota.u37.cpp |
| 4556 | 35 |  | `vota::CLogVota::LogaQtdViasExcedeMaximo` | uenux2/src/app/vota/log/clogvota.cpp | src/uenux2/src/app/vota/log/clogvota.u37.cpp |
| 5342 | 22 |  | `vota::CFinalizaOperador::GetInst` | uenux2/src/app/vota/operador/cfinalizaoperador.cpp (path inferred) | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 5450 | 625 | ✓ | `api::CTickManager::GetTicksExpirados` | uenux2/src/api/util/ctickmanager.cpp | src/uenux2/src/api/util/ctickmanager.u37.cpp |
| 5547 | 166 |  | `api::CFormBuilder::Add<api::CImageField>(shared_ptr<CQRCodeImage> [by value], const SPoint&) [anchor 1 constant]` | library/inlined helper (template of uenux2/src/api/gui/cformbuilder.h) | src/uenux2/src/api/gui/u37-template-instances.cpp |
| 5553 | 204 | ✓ | `api::CApplicationContextStack::Remove` | uenux2/src/api/gui/capplicationcontextstack.cpp | src/uenux2/src/api/gui/capplicationcontextstack.u37.cpp |
| 5818 | 15 |  | `vota::DiretorioWsqNaoHabilitado` | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp (path inferred) | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 5819 | 15 |  | `vota::DiretorioWsqHabilitado` | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp (path inferred) | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 5881 | 29 |  | `vota::CLogVota::LogaEstadoNaoEsperado` | uenux2/src/app/vota/log/clogvota.cpp | src/uenux2/src/app/vota/log/clogvota.u37.cpp |
| 5885 | 29 |  | `vota::CLogVota::LogaMRNaoPresente` | uenux2/src/app/vota/log/clogvota.cpp | src/uenux2/src/app/vota/log/clogvota.u37.cpp |
| 5895 | 16 |  | `comum::(anonymous namespace)::LogInfo` | uenux2/src/app/comum/iinterfaceinit.cpp | src/uenux2/src/app/comum/iinterfaceinit.u37.cpp |
| 5903 | 112 |  | `comum::CInfoMTLCD::ExibeBateria` | uenux2/src/app/comum/cinfomtlcd.cpp | src/uenux2/src/app/comum/cinfomtlcd.u37.cpp |
| 5908 | 23 |  | `comum::CAppStateContext::AceitaTicks` | uenux2/src/app/comum/cappstate.cpp (path inferred) | src/uenux2/src/app/comum/cappstate.u37.cpp |
| 5909 | 23 | ✓ | `comum::CAppStateContext::AceitaTeclado` | uenux2/src/app/comum/cappstate.cpp (path inferred) | src/uenux2/src/app/comum/cappstate.u37.cpp |
| 5910 | 27 | ✓ | `comum::CAppStateContext::ProcessTick` | uenux2/src/app/comum/cappstate.cpp (path inferred) | src/uenux2/src/app/comum/cappstate.u37.cpp |
| 5911 | 25 | ✓ | `comum::CAppStateContext::ProcessInput` | uenux2/src/app/comum/cappstate.cpp (path inferred) | src/uenux2/src/app/comum/cappstate.u37.cpp |
| 5918 | 11 |  | `comum::CInformacaoEleicao::ImprimeBoletimJustificativa` | uenux2/src/app/comum/informacao/cinformacaoeleicao.cpp | src/uenux2/src/app/comum/informacao/cinformacaoeleicao.u37.cpp |
| 5947 | 212 |  | `vota::CVerificaHorarioZeresima::GetInst` | uenux2/src/app/vota/eleitor/iniciovotacao/cverificahorariozeresima.cpp | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 5948 | 28 |  | `vota::testeteclado::CBase::CBase` | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cbase.cpp (path inferred) | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 5951 | 19 |  | `vota::CVisualizarCandidatos::GetCargo` | uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cvisualizarcandidatos.h | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 5954 | 104 |  | `vota::CMenuFiltrarCandidatosPorNumero::GetInst` | uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatospornumero.cpp (path inferred) | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 5983 | 22 |  | `vota::CInicioBU::GetInst` | uenux2/src/app/vota/eleitor/fimvotacao/ciniciobu.cpp (path inferred) | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 5984 | 22 |  | `vota::CImprimirBUOutrasObrigatorias::GetInst` | uenux2/src/app/vota/eleitor/fimvotacao/cimprimirbuoutrasobrigatorias.cpp | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 5985 | 22 |  | `vota::CImprimindoBim::GetInst` | uenux2/src/app/vota/eleitor/fimvotacao/cimprimindobim.cpp | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 6017 | 460 |  | `vota::CControladorRegistraMesariosVota::LogaComTitulo [merged body of slots 19, 27]` | uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp (path inferred) | src/uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.u37.cpp |
| 6024 | 365 | ✓ | `api::IForm<MEDIA>::IForm(campos, preShow) [merged body]` | library/inlined helper (template of uenux2/src/api/gui/iform.h) | src/uenux2/src/api/gui/u37-template-instances.cpp |
| 6037 | 22 |  | `vota::CGravaResultado::GetInst` | uenux2/src/app/vota/eleitor/fimvotacao/cgravaresultado.cpp | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 6188 | 162 |  | `comum::CMenuBase::~CMenuBase (deleting)` | uenux2/src/app/comum/cmenubase.cpp (path inferred) | src/uenux2/src/app/comum/cmenubase.u37.cpp |
| 6587 | 130 |  | `vota::CTelasVota::CriaTelaAguarde` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 7116 | 10 |  | `vota::CThreadEleitor::GetInst()::s_mutex [atexit ~mutex]` | uenux2/src/app/vota/eleitor/cthreadeleitor.cpp | src/uenux2/src/app/vota/u37-foreign-fragments.cpp (table) |
| 7175 | 10 |  | `vota::impl::ISincronismoVotoEleitor::GetInst()::s_mutex [atexit ~mutex]` | uenux2/src/app/vota/eleitor/csincronismovotoeleitor.cpp | src/uenux2/src/app/vota/u37-foreign-fragments.cpp (table) |
| 7187 | 10 |  | `vota::CSincronismoEleitor::GetInst()::s_mutex [atexit ~mutex]` | uenux2/src/app/vota/eleitor/csincronismoeleitor.cpp (path inferred) | src/uenux2/src/app/vota/u37-foreign-fragments.cpp (table) |
| 7195 | 12 |  | `vota::CSincronismoEleitor::GetInst()::s_inst [atexit ~unique_ptr]` | uenux2/src/app/vota/eleitor/csincronismoeleitor.cpp (path inferred) | src/uenux2/src/app/vota/u37-foreign-fragments.cpp (table) |
| 7202 | 10 |  | `vota::CFimVotoEleitor::GetInst()::s_mutex [atexit ~mutex]` | uenux2/src/app/vota/eleitor/cfimvotoeleitor.cpp | src/uenux2/src/app/vota/u37-foreign-fragments.cpp (table) |
| 7212 | 12 |  | `vota::CFimVotoEleitor::GetInst()::s_inst [atexit ~unique_ptr]` | uenux2/src/app/vota/eleitor/cfimvotoeleitor.cpp | src/uenux2/src/app/vota/u37-foreign-fragments.cpp (table) |
| 7232 | 10 |  | `vota::CMostraTelaContinuaVotacao::GetInst()::s_mutex [atexit ~mutex]` | uenux2/src/app/vota/eleitor/cmostratelacontinuavotacao.cpp (path inferred) | src/uenux2/src/app/vota/u37-foreign-fragments.cpp (table) |
| 7239 | 12 |  | `vota::CMostraTelaContinuaVotacao::GetInst()::s_inst [atexit ~unique_ptr]` | uenux2/src/app/vota/eleitor/cmostratelacontinuavotacao.cpp (path inferred) | src/uenux2/src/app/vota/u37-foreign-fragments.cpp (table) |
| 7284 | 10 |  | `vota::CInstrucaoVotacaoAcessibilidade::GetInst()::s_mutex [atexit ~mutex]` | uenux2/src/app/vota/eleitor/cinstrucaovotacaoacessibilidade.cpp | src/uenux2/src/app/vota/u37-foreign-fragments.cpp (table) |
| 7323 | 10 |  | `vota::CIniciodeCiclo::GetInst()::s_mutex [atexit ~mutex]` | uenux2/src/app/vota/eleitor/ciniciodeciclo.cpp | src/uenux2/src/app/vota/u37-foreign-fragments.cpp (table) |
| 7333 | 12 |  | `vota::CIniciodeCiclo::GetInst()::s_inst [atexit ~unique_ptr]` | uenux2/src/app/vota/eleitor/ciniciodeciclo.cpp | src/uenux2/src/app/vota/u37-foreign-fragments.cpp (table) |
| 7405 | 10 |  | `vota::CEleitorVotando::GetInst()::s_mutex [atexit ~mutex]` | uenux2/src/app/vota/eleitor/celeitorvotando.cpp | src/uenux2/src/app/vota/u37-foreign-fragments.cpp (table) |
| 7470 | 10 |  | `vota::CConfirmaVotoSemCandidato::GetInst()::s_mutex [atexit ~mutex]` | uenux2/src/app/vota/eleitor/cconfirmavotosemcandidato.cpp | src/uenux2/src/app/vota/u37-foreign-fragments.cpp (table) |
| 7480 | 18 | ✓ | `comum::CAppState::NeedChangeState` | uenux2/src/app/comum/cappstate.cpp (path inferred) | src/uenux2/src/app/comum/cappstate.u37.cpp |
| 7482 | 10 |  | `vota::CAguardaMensagem::GetInst()::s_mutex [atexit ~mutex]` | uenux2/src/app/vota/eleitor/caguardamensagem.cpp (path inferred) | src/uenux2/src/app/vota/u37-foreign-fragments.cpp (table) |
| 7485 | 12 |  | `vota::CAguardaMensagem::GetInst()::s_inst [atexit ~unique_ptr]` | uenux2/src/app/vota/eleitor/caguardamensagem.cpp (path inferred) | src/uenux2/src/app/vota/u37-foreign-fragments.cpp (table) |
| 7790 | 10 |  | `vota::<unknown>::GetInst()::s_mutex [atexit ~mutex]` | unknown (TU emitting vota::CAssinadorVota's vtable and holding CSincronizaVota's flag @1832936, e.g. uenux2/src/app/vota/comum/csincronizavota.cpp?) | src/uenux2/src/app/vota/u37-foreign-fragments.cpp (table) |
| 7798 | 12 |  | `vota::<unknown>::GetInst()::s_inst [atexit ~unique_ptr]` | unknown (TU emitting vota::CAssinadorVota's vtable and holding CSincronizaVota's flag @1832936, e.g. uenux2/src/app/vota/comum/csincronizavota.cpp?) | src/uenux2/src/app/vota/u37-foreign-fragments.cpp (table) |
| 11666 | 56 |  | `comum::CItemVersoesPacotes::Disponivel` | uenux2/src/app/comum/citemversoespacotes.cpp (path inferred) | src/uenux2/src/app/comum/citemversoespacotes.cpp |
| 11667 | 56 |  | `comum::CItemParametrosUrna::Disponivel` | uenux2/src/app/comum/citemparametrosurna.cpp (path inferred) | src/uenux2/src/app/comum/citemparametrosurna.cpp |
| 11668 | 106 |  | `comum::CItemImprimeListaEleitores::Disponivel` | uenux2/src/app/comum/citemimprimelistaeleitores.cpp (path inferred) | src/uenux2/src/app/comum/citemimprimelistaeleitores.cpp |
| 11669 | 56 | ✓ | `comum::CItemImprimeEstadoUrna::Disponivel` | uenux2/src/app/comum/citemimprimeestadourna.cpp (path inferred) | src/uenux2/src/app/comum/citemimprimeestadourna.cpp |
| 12235 | 159 |  | `comum::CMenuBase::~CMenuBase` | uenux2/src/app/comum/cmenubase.cpp (path inferred) | src/uenux2/src/app/comum/cmenubase.u37.cpp |
