# u38: `app:vota` without a known file (exit-time destructors of the state singletons, and six text sources)

Unit u38 has **107 wasm functions** in component `app:vota`. The tools could not give them a source file: none
of them references a `std::source_location` record, none is in a vtable, and none has a direct caller. They
are in the function table only. Reading them gives two groups:

| group | functions | what they are |
|---|---|---|
| **exit-time destructor stubs** (named `__dtor_<variable>` here; clang's own symbol is `__cxx_global_array_dtor[.N]`, §2.2) | 101 | Compiler-generated. Clang emits one for each static object with a non-trivial destructor: 54 static `std::mutex`, 44 static `std::unique_ptr<State>` of the lazy **state singletons** of the operator (mesário) and voter state machines, and 3 static data members of `CControlaReconhecimento` (two fingerprint buffers and a título). **Dead code in this build**: nothing registers them. |
| **text sources** | 6 | `std::string f()` functions that the screens and reports call through a pointer: 5 lines of the microterminal (10660, 10664, 10692, 10693, 10694) and the column header of the "Eleitores com habilitação biográfica" report (11240). |

None of the 107 was seen by the sampling profiler during the recorded votes. The six text sources belong to the
operator thread and to the end-of-day reports, which the public web page never runs. The stubs can never
run. The CPF branch of func 10664 **was** executed and observed with the operator harness
(`tools/bu/operator_harness.mjs`, §5.2).

Reconstructed sources (new files, fragments per original file; every section names its original path):

```
src/uenux2/src/app/vota/u38-foreign-fragments.cpp            the atexit-stub pattern (read this first) +
                                                            iexecucaovota.cpp, monitor/cthreadmonitor.cpp
src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp   41 operator classes' statics (70 stubs), the
                                                            CControlaReconhecimento static buffers, funcs
                                                            10660/10664 (+ check of 10692-10694)
src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp    CConfereVotoEmCargo<> template statics, 18 voter
                                                            classes/instances' statics (29 stubs), func 11240 (BEHB header)
```

Funcs 10692/10693/10694 had already been written out by unit u27 in
`src/uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp` (lines 64-66). u38 checked them byte for byte
and did not duplicate them.

**Glossary**: *urna* voting machine; *eleitor* voter; *mesário* poll worker. The *presidente da mesa* is the
head poll worker. *Terminal do mesário* / MT is the poll worker's 4 x 40 character microterminal. *Título (de
eleitor)* is the 12-digit voter registration number and *CPF* the 11-digit taxpayer number. Both can identify
a voter. *Número livre* is a free identifier. *Habilitação* is releasing the voter to the booth. It can be
*biométrica* (a fingerprint match) or *biográfica* (identified by personal data after a failed or absent
fingerprint). *Justificativa* is the form a voter fills to justify not voting in their own section.
*Trânsito* is voting away from home, requested in advance. *Encerramento* is closing the vote at the end of
the day. *Zerésima* is the zero report printed before voting. *BU (boletim de urna)* is the per-machine result.
*Seção* is the polling section. *Estado* is a state of a state machine.

---

## 1. Where this code sits in the voting process

The VOTA application runs two state machines. Each runs in its own thread on the urna:

* the **operator thread** (`vota::CThreadOperador`, the mesário's microterminal). It covers identifying the
  voter (título/CPF typed, roll lookup, impediments, justificativa), biometrics (`CPedeDigital`,
  `CRegistraDigitalOperador`, `CControlaReconhecimento`), releasing the booth, waiting while the voter votes
  (`CMostraEleitorVotando`, `CEleitorDemorando`, `CSuspensaoAutomaticaEleitor`), the "outras opções" menu
  (`CEscolheOpcao`) and the start of the **encerramento** (`CIniciaFinalizacao` → … → `CConfirmaEncerramento`);
* the **voter thread** (`vota::CThreadEleitor`, the booth screen). It covers start-up (zerésima, keyboard
  test `testeteclado::*`, `CGeraDadosDinamicos`), the vote itself (`CPedeProporcional`, `CPedeMajoritario`,
  `CPedeNominal`, the confirmation states `CConfereVotoEmCargo<…>`) and the auxiliary menus (`CVisualizarCandidatos`,
  `CMaisInformacoes`, …).

Every state is a `comum::CAppState` and a **lazy singleton**. A transition is written as
`m_proximoEstado = &CFoo::GetInst();`. The instance is created on first use and lives until the process exits.
This unit contains the exit-time halves of those singletons: the functions that would destroy the mutex and
the instance at `exit()`. The web build never calls `exit()` (Emscripten `EXIT_RUNTIME=0`), so they never run.

The six text sources are the only functional code of the unit:

* func 10664 is line 2 of the MT screen **"eleitor não encontrado"**: `"CPF não encontrado. Digite o Título."`
  when a CPF was typed and the election's principal identifier is the título, else `"NÃO CADASTRADO nesta urna"`;
* func 10660 is line 2 of **"optou por votar em trânsito"** (voter who requested a transit vote, at a urna
  that does not accept justifications);
* funcs 10692-10694 are the "outras opções" menu entries **"Exibir contadores"**, **"Registrar mesários"** and
  **"Encerrar votação"**;
* func 11240 is the column header `"Sequencial                      Título"` of the end-of-day report
  **"Eleitores com habilitação biográfica"** (file `behb.dat`, printed before the BU).

---

## 2. The lazy-singleton pattern and its exit-time stubs

### 2.1 Source pattern

```cpp
// in cfoo.cpp
std::mutex             CFoo::s_mutex;        // @X       (24 bytes: musl pthread_mutex_t on wasm32)
std::unique_ptr<CFoo>  CFoo::s_instancia;    // @X + 24

CFoo& CFoo::GetInst()
{
    std::lock_guard trava(s_mutex);          // lock(): removed entirely (no pthreads)
    if (!s_instancia)
        s_instancia.reset(new CFoo());       // "old = s; s = novo; if (old) delete old;"
    return *s_instancia;                     // unlock(): only the empty func-150 residue remains
}
```

Most accessors are **inlined into the state that makes the transition** (LTO). That is why the tools found the
construction of, for example, `CPerguntaEleitorVotando` inside `CEleitorDemorando::ProcessInput` (func 10439).
Some accessors stay out of line (e.g. func 1901 = `CEleitorVotouNaoVotou::GetInst`). Several of those are thunks
into wasm-opt *merged* bodies that take the addresses and vtable as parameters: func 764
(`CFinalizaOperador`, `CReinicioComparecimentoMesario`) and 6051 (`CPedeProporcional`).

### 2.2 What clang emits for each static, and what is left of it

For every static with a non-trivial destructor, clang emits in the translation unit that defines (or, for a
template static, instantiates) it:

```cpp
static void __dtor_s_mutex(void*)     { CFoo::s_mutex.~mutex(); }          // real symbol: __cxx_global_array_dtor[.N]
static void __dtor_s_instancia(void*) { CFoo::s_instancia.~unique_ptr(); } // real symbol: __cxx_global_array_dtor[.N]
// and in the TU's global initialiser:
__cxa_atexit(&__dtor_s_mutex, nullptr, &__dso_handle);
__cxa_atexit(&__dtor_s_instancia, nullptr, &__dso_handle);
```

The helper exists because on WebAssembly destructors return `this` (docs/libraries/libcxx-core.md §1.1) **and**
an indirect call through a mismatched signature traps: clang's `WebAssemblyCXXABI` overrides
`canCallMismatchedFunctionType()` to `false`, so `EmitDeclDestroy` cannot register `&T::~T` and builds a helper
with `generateDestroyHelper`. That helper is an internal `void __cxx_global_array_dtor(void*)` (LLVM renames the
copies `.1`, `.2`, … inside one TU), registered with a **null** argument, which is why every stub ignores its
parameter and uses the variable's address as a constant. The observed wasm signature `(i32)->void` confirms this
kind of helper: clang's `__dtor_<mangled var>` stubs (`createAtExitStub`) are `void()` and are only used with
`-fno-use-cxa-atexit`. The names `__dtor_<variable>` used in this unit (and in u39) are therefore descriptive
labels, not the compiler's symbols. In this build:

* `__cxa_atexit` does nothing (`EXIT_RUNTIME=0`, docs/libraries/libc-and-emscripten-runtime.md). LTO and wasm-opt
  deleted every registration, and the global initialisers became empty (§8);
* the stubs survive only as **function-table entries**. The table is not compacted. No code takes their slot
  number: all 101 slots were searched as `i32.const`, and the few hits are unrelated numbers such as string
  addresses, OpenSSL control codes and arithmetic;
* the stub **ignores its argument** and uses the variable's address as a constant. This is how each stub was
  matched to its accessor: the same address appears in the accessor (`if (X+24[0]) …; X+24[0] = new …;
  mutex_unlock(X)`), next to the vtable of the class it instantiates.

Bodies:

| stub of | body | meaning |
|---|---|---|
| `s_mutex` | `mutex_unlock(X)` = func 150 | `~mutex()` → `pthread_mutex_destroy`, a no-op without pthreads. What remains is the empty `noexcept` `__THREW__` check that ICF shares with the unlock residue. |
| `s_instancia` | `thunk(a, X+24)` → merged body | `~unique_ptr()`: `p = s; s = nullptr; if (p) { p->~T(); free(p); }`. The `delete` was devirtualised (the classes are presumably `final`, or whole-program devirtualisation was on; the binary cannot tell), so it calls the concrete destructor directly. wasm-opt merge-similar-functions then folded the stubs with the same destructor into one body that takes the address as a 2nd argument (below). |

| merged `~unique_ptr` body | destructor it calls | classes (in u38) |
|---|---|---|
| 349 | none (the virtual destructor has an empty body: vtable slot 0 = `icf_ret_this_vf0` 174; `free` only) | CFinalizaOperador, CIniciaFinalizacao, CRegistroMesarioEncerrado, CGeraDadosDinamicos, CMenuFiltrarCandidatosPorCargo, CReinicioComparecimentoMesario |
| 389 | ICF 244 (releases the `shared_ptr` at +12/+16, the state's MT form) | 20 operator/voter states |
| 763 | ICF 448 (two `shared_ptr`s: +12, +20) | CMostraEleitorVotando, CDadoEleitorNaoConfere, CDigitalReconhecida, CDigitalNaoReconhecidaPorTempo |
| 2903 | ICF 1284 (three `shared_ptr`s: +12, +20, +28) | CEleitorVotouNaoVotou, CEleitorDemorando, CDigitalNaoReconhecida |
| 1564 | ICF 785 (`shared_ptr` at +28) | testeteclado::CEsperaRetestar, CVerificaHorarioZeresima |
| 1959 | `IEleitorImpedidoVotar::~` (1257) | CEleitorOptouPorVotarEmTransito, CEleitorNaoEncontrado |
| 1286 | `IConfereVotoEmCargo::~` (1717) | 3 `CConfereVotoEmCargo<…>` instances |
| (inline) | `testeteclado::CBase::~` (1559) / `CGeraZeresimaBase::~` (1720) | CRetomada, CPreZeresima / CRegerarZeresima, CGeraZeresima (11833, 11864, 11844, 11938: 38-byte stubs that were not merged) |

### 2.3 Where the statics are defined (evidence from guard variables)

* **Plain classes.** The accessors have **no guard-variable test**. A function-local static with a non-trivial
  destructor needs one (`__cxa_guard_acquire` + `__cxa_atexit` on first pass). The mutex/unique_ptr pairs are
  therefore **namespace-scope or static data members** of the `.cpp` that defines the class. The binary cannot
  tell which of the two. The fragments write them as static data members `s_mutex` / `s_instancia`, the
  style u19 used for `CThreadMonitor`. Several reconstructions by other units show them as function-local
  statics: they behave the same, but that is not what was compiled.
* **`CConfereVotoEmCargo<PROXIMO, TELA>`** (the confirmation state of the vote) is different. Its mutex and
  instance are **template static data members**. Those are linkonce, and each has a 4-byte guard next to it
  (mutex @M, guard @M+24, unique_ptr @M+28, guard @M+32). `__wasm_call_ctors` (func 14478) still sets all 20
  guards of the 10 instantiations to 1: `if (!(guard & 1)) guard = 1;` is what remains of the guarded
  `__cxa_atexit` registration. That is why the unique_ptr of these instances is at mutex **+28**, not +24.
  Being implicit instantiations, their destroy helpers are emitted in the TUs that use them, not in the header.
  The function order agrees: 11674/11675 sit next to the `CConfereVotoEmCargo<CMajoritario*>::vf16` bodies and
  `CPedeMajoritario` (cpedemajoritario.cpp), 11708/11709 among the `CPedeProporcional` functions, 11722 next to
  `CPedeNominal::vf15/vf16`.
* **`IExecucaoVota`, `impl::IInformacaoThreadOperador` and `testeteclado::impl::IGeradorTeclas`** have a static mutex
  only. Their `GetInst()` (srcloc-attested: iexecucaovota.cpp:74, cinformacaothreadoperador.cpp:554,
  ctesteteclado.cpp:121) locks it around "push the default implementation into the `CPolySingletonList` if none
  was registered, then return `CPolySingleton<I>::instance()`". The instance is owned by the registry. Only the
  unlock residue is left: `mutex_unlock(1911520)` at the end of func 3594, `mutex_unlock(1908504)` in func 599,
  `mutex_unlock(1837656)` in func 11805. The web build pushes `CExecucaoVotaCooperativa` from `main`, so the lazy
  default of `IExecucaoVota` is never created there.

---

## 3. Classes and hierarchy (RTTI)

```
api::CState
└─ comum::CAppState                                  (vptr, +4 m_proximoEstado, +8/+9/+10 accepts msgs/keys/ticks)
   ├─ operator states (vota::, operador/…): CFinalizaOperador, CSuspensaoAutomaticaEleitor, CPerguntaEleitorVotando,
   │  CMostraEleitorVotando, CEleitorVotouNaoVotou, CEleitorDemorando, CVerificaDadoEleitor, CDadoEleitorNaoConfere,
   │  CTentativaCapturaDigitalEsgotada, CPedeDigital, CDigitalNaoReconhecida, CDigitalNaoReconhecidaDecBiometria,
   │  CDigitalReconhecida, CDigitalNaoReconhecidaPorTempo, CNomeEleitor, CInformaBioDesabilitadaDemo,
   │  CControlaReconhecimento, CHabilitaAudioEleitor, CConfirmaInspecionada, CAguardaInspecao, CValidaIdentidade,
   │  CEleitorJaVotou, CEscolheOpcao, CContadoresBiometria, CIniciaFinalizacao, CHabilitacaoAudioNaoPermitida,
   │  CEncerramentoHorarioInvalido, CPerguntaFilaEleitorVazia, CAguardaEleitoresVotarem, CPedeTituloEncerramento,
   │  CTituloEncerramentoInvalido, CEncerramentoAntecipado, CConfirmaEncerramento, CHorarioVotacaoTerminou,
   │  CRegistroMesarioEncerrado
   ├─ vota::IEleitorImpedidoVotar ─ CEleitorOptouPorVotarEmTransito, CEleitorNaoEncontrado (+4 siblings, u27)
   ├─ vota::IIniciaJustificativa   ─ CIniciaJustificativa (+2)
   ├─ vota::IConfirmaJustificativa ─ CConfirmaJustificativa (+2)
   ├─ voter states: CGeraDadosDinamicos, CVisualizarCandidatos, CMenuFiltrarCandidatosPorCargo,
   │  CMenuFiltrarCandidatosPorNumero, CMaisInformacoes, CReinicioComparecimentoMesario
   ├─ vota::CGeraZeresimaBase ─ CGeraZeresima, CRegerarZeresima
   ├─ vota::CEstadoComDesligamentoAutomatico ─ CVerificaHorarioZeresima, testeteclado::CTesteTeclado,
   │                                          testeteclado::CEsperaRetestar,
   │                                          testeteclado::CBase ─ testeteclado::CRetomada, testeteclado::CPreZeresima
   └─ vota::CVotacaoStateAudio ─ CPedeProporcional (+ CPedeMajoritario, CPedeNominal…)
                               └─ vota::IConfereVotoEmCargo ─ CConfereVotoEmCargo<PROXIMO, (ETelaVotacao)N> (10 instances)
api::CThread ─ vota::CThreadVota ─ CThreadOperador, CThreadMonitor
vota::IExecucaoVota (interface; default CExecucaoVota, web: CExecucaoVotaCooperativa)
vota::impl::IInformacaoThreadOperador (interface; CInformacaoThreadOperador)
vota::testeteclado::impl::IGeradorTeclas (interface; CGeradorTeclasAleatorio)
```

Static objects destroyed by this unit, with their addresses (all in `.bss`). Within a pair the mutex comes first
(@X, @X+24), and across translation units the addresses decrease as the function indices of the stubs increase.
The layout cannot be tied to the source order in general: in `CControlaReconhecimento` the stubs come in the order
s_mutex, s_instancia, s_tituloMesario, s_digitalMesario, s_digitalCapturada (10514-10518), while the addresses
ascend s_digitalCapturada < s_digitalMesario < s_tituloMesario < s_mutex < s_instancia.

* each operator/voter state class: `s_mutex` @X and `s_instancia` @X+24 (per class in §12);
* `CConfereVotoEmCargo<CMajoritarioValido, 2>` @1838452/@1838480, `<CProporcionalBranco, 4>` @1838112/@1838140,
  `<CConfirmaVotoLegenda, 10>` @1837976/@1838004;
* `CControlaReconhecimento` static data members: `std::vector<uebyte> s_digitalCapturada` @1908672 (the voter's
  last fingerprint capture, WSQ), `std::vector<uebyte> s_digitalMesario` @1908684 (the mesário's capture) and
  `std::string s_tituloMesario` @1908696 (título of the mesário who released the voter). These are the only
  non-singleton statics of the unit. Their neighbours `s_indiceDedo` @1908668, `s_score` @1908708 and
  `s_qtdEleitores` @1908712 are trivially destructible and have no stub.

---

## 4. Control flow: where the singletons are used

The stubs do nothing at run time. The useful information they carry is **which state lives in which static,
and which function creates it**. The table below lists, per flow, the transitions that instantiate the u38 statics
(accessor = the function holding the lazy creation). Details of each state are in the units named.

**Operator thread** (u10, u17, u27, u33):

| flow | transition (accessor) | statics (stubs) |
|---|---|---|
| thread objects | `CThreadOperador::GetInst` (270), `CThreadMonitor::GetInst` (1898), `IExecucaoVota::GetInst` (3594) | 10207, 10228, 10237 |
| identification | `CPedeIdentidade::ProcessInput` (10677) → `CValidaIdentidade`; `CPedeIdentidade::ProcessTick` (10676) → `CAguardaInspecao` (inspection every 60-90 min) → `CAguardaInspecao::ProcessMessage` (10530) → `CConfirmaInspecionada` | 10629; 10531/10532; 10528/10529 |
| roll lookup | `CProcuraEleitor::StartState` (10631) → `CIniciaJustificativa` + `CConfirmaJustificativa`, or `CEleitorNaoEncontrado` (5424) | 10616, 10621, 10665/10666 |
| impediments | `CEleitorEncontrado::StartState` (10635) → `CEleitorJaVotou`, `CEleitorOptouPorVotarEmTransito` | 10644/10645, 10661/10662 |
| biometrics | `CNomeEleitor` (5401), `CInformaBioDesabilitadaDemo` (5402), `CPedeDigital` (2740), `CPedeDigital::ObtemEstadoPosReconhecimentoBiometrico` (5397) → `CDigitalReconhecida` / `CDigitalNaoReconhecida`; `CDigitalNaoReconhecida::StartState` (10474) → `CDigitalNaoReconhecidaDecBiometria`; `CPedeDigital::ProcessTick` (10465) → `CDigitalNaoReconhecidaPorTempo`; `CRegistraDigitalOperador::ProcessTick` (10451) → `CTentativaCapturaDigitalEsgotada`; `CControlaReconhecimento` (1256) and its static buffers | 10502/10503, 10509/10510, 10471, 10483/10484, 10475/10476, 10479/10480, 10487/10488, 10463, 10514-10518 |
| audio | `CHabilitaAudioEleitor` (2743) | 10525/10526 |
| voter in the booth | `CMostraEleitorVotando` (1150), `CEleitorVotouNaoVotou` (1901), `CEleitorDemorando` (2734) → `CEleitorDemorando::StartState` (10440) → `CSuspensaoAutomaticaEleitor`, `::ProcessInput` (10439) → `CPerguntaEleitorVotando`; `CVerificaDadoEleitor` (2735) → `::ProcessInput` (10443) → `CDadoEleitorNaoConfere` | 10428/10429, 10433/10434, 10441/10442, 10412, 10417/10418, 10445/10446, 10449/10450 |
| outras opções | `CEscolheOpcao` (2753) → `::ProcessInput` (10689) → `CContadoresBiometria`, `CIniciaFinalizacao`, `CHabilitacaoAudioNaoPermitida`; `CHorarioVotacaoTerminou` (5430) | 10696, 10704/10705, 10707/10708, 10755/10756, 10760/10761 |
| encerramento | `CIniciaFinalizacao::StartState` (10706) → `CEncerramentoHorarioInvalido`; `CPerguntaFilaEleitorVazia` (5426) → `::ProcessInput` (10713) → `CAguardaEleitoresVotarem`; `CPedeTituloEncerramento` (3631) → `::StartState` (10722) → `CEncerramentoAntecipado`, `::ProcessInput` (10721) → `CTituloEncerramentoInvalido`; `CConfirmaEncerramento` (3632) | 10711/10712, 10716, 10719/10720, 10724/10725, 10733, 10729, 10735 |
| mesário registration and end | `CControladorRegistraMesariosVota::GetEstadoAposRegistroVotacao` (vf11, 10793) → `CRegistroMesarioEncerrado`; `GetEstadoAposRegistroFinal` (vf12) → `CFinalizaOperador` (5342) | 10765, 10216 |
| shared info | `impl::IInformacaoThreadOperador::GetInst` (599) | 10553 |

**Voter thread** (u06, u07, u09, u26, u30):

| flow | transition (accessor) | statics (stubs) |
|---|---|---|
| start-up / keyboard test | `CAjusteInicial::ValidaTemposDesligamento` (7160) → `testeteclado::CRetomada`; `CVerificaEleicaoPassou::StartState` (11914) → `testeteclado::CPreZeresima` → `::GetEstadoPassouNoTeste` (5945) → `CGeraDadosDinamicos`; `CTesteTeclado` (3855) and, inside its StartState (11805), `IGeradorTeclas::GetInst` (random key sequence); `CEsperaRetestar` (5936); `CReinicioComparecimentoMesario` (3864) | 11832/11833, 11863/11864, 11866/11867, 11807, 11802, 11827/11828, 11918/11919 |
| zerésima | `CVerificaHorarioZeresima` (5947), `CGeraZeresima` (5962), `CConfirmaRegerarZeresima::ProcessInput` (11838) → `CRegerarZeresima` | 11872/11873, 11938, 11844 |
| candidate lookup menus | `CVisualizarCandidatos` (1279), `CMenuVisualizarCandidatos::StartState` (11881) → `CMenuFiltrarCandidatosPorCargo`, `CMenuFiltrarCandidatosPorNumero` (5954), `CMaisInformacoes` (1280) | 11878, 11889/11890, 11893/11894, 11898/11899 |
| the vote | `CPedeProporcional` (3849); `CPedeMajoritario::ProcessInputAudio` (11683) → `CConfereVotoEmCargo<CMajoritarioValido,2>` (valid majoritarian vote); `CPedeProporcional::ProcessInputAudio` (11711) → `<CProporcionalBranco,4>` (blank proportional vote); `CPedeNominal::GetProximoEstado` (slot 16, 11724) → `<CConfirmaVotoLegenda,10>` (party-only vote) | 11713, 11674/11675, 11708/11709, 11722 |

In the recorded votes (sampling profiler), the accessors 3594, 270, 11683, 11711 and 11724 were sampled. An
accessor that ran did not necessarily take the branch that creates a u38 singleton. What the transcripts show:
`CThreadOperador` (270) exists; `CConfereVotoEmCargo<CMajoritarioValido,2>` (valid Governador/Presidente) and
`<CConfirmaVotoLegenda,10>` (the party-only Deputado Federal vote) were created in `vote_geral_t1`;
`<CProporcionalBranco,4>` was **not** created in either recorded session (no blank proportional vote). A
headless run with a blank Vereador (`--scenario municipal-t1 --keys "B  C  12C  C  "`) shows it
(`substate: CConfereVotoEmCargo<vota::CProporcionalBranco, (vota::ETelaVotacao)4>`). `IExecucaoVota::GetInst`
(3594) runs, but its lazy default is never created (§7). `CPedeProporcional` (3849) is the first voting state
of every transcript, although the profiler did not sample its small accessor. Their destructors (these stubs)
never run.

---

## 5. The text sources

All six are called through a pointer, never directly:

* 10660, 10664, 10692-10694 are stored as the callable of a `std::function<std::string()>` whose target type is
  `std::string (*)()` (vtable @1542376 `__func<std::string(*)(), std::string()>`). A lambda would have its own
  `__func<lambda>` vtable, so these are **named functions**, most likely in an anonymous namespace;
* 11240 is stored in a `api::CDataText<std::string (*)()>` (func 604 wraps it into a `CTextFieldPaper`).

### 5.1 Func 10660 (`TextoOptouTransito`, slot 3876)

Returns `"Optou por votar em trânsito"`. The `CEleitorOptouPorVotarEmTransito` constructor, inlined in
`CEleitorEncontrado::StartState` (10635), uses it as line 2 with `TextoVazio` (" ", func 3628) as line 3 and
`CONFIRMA: retornar`. It is reached when the voter's impediment is 2 (requested a transit vote) and the urna does
not accept justifications (u27 §4.4).

### 5.2 Func 10664 (`TextoNaoEncontrado`, slot 3867)

```cpp
const auto digitado = impl::IInformacaoThreadOperador::GetInst().GetTipoIdentidadeDigitada();  // func 5416 (slot 24)
const auto& cfg     = comum::CConfiguracaoEleicao::GetInst();                                // func 187
if (digitado == CPF && cfg.GetTipoIdentificadorPrincipal() == TITULO)                       // +668
    return "CPF não encontrado. Digite o Título.";
return "NÃO CADASTRADO nesta urna";
```

Line 2 of `CEleitorNaoEncontrado` (constructor in func 5424). Checked with the operator harness:

```
node tools/bu/operator_harness.mjs --flow full --out <dir> \
     --script "geradin zeresima ot:20 om:8 ot:50 o:XXXXXXXXXXXC ot:50 o:C ot:50 o:XXXXXXXXXXXXC ot:50"
```

* an unknown **CPF** (XXX.XXX.XXX-XX): `CPedeIdentidade → CProcuraEleitor → CEleitorNaoEncontrado`, MT shows
  `CPF: XXX.XXX.XXX-XX / CPF não encontrado. Digite o Título. / CONFIRMA: retornar`;
* an unknown **título** (XXXX XXXX XXXX): `CProcuraEleitor → CIniciaJustificativa → CConfirmaJustificativa`
  (a voter of another section may justify). Reproduced by this review (same command, same MT lines and
  states). The routing is in `CProcuraEleitor::StartState` (10631): a not-found identifier goes to
  `CEleitorNaoEncontrado` when justification is not accepted (func 4578) **or** when the typed type is not the
  título (`!= 1`). So `"NÃO CADASTRADO nesta urna"` appears for a título when justification is not accepted, for a
  CPF when the election's principal identifier is not the título, and for any other typed type (número livre).

### 5.3 Funcs 10692 / 10693 / 10694 (menu "outras opções")

`"Exibir contadores"`, `"Registrar mesários"`, `"Encerrar votação"`. These are the text sources of `COpcaoDS` entries
4 (or 3), 3 and 2 of `CEscolheOpcao` (func 2753; u27 §4.5). The same literals are also used by
`CEscolheOpcao::ProcessInput` (10689) for the log line `"Operador selecionou: {}"`.

### 5.4 Func 11240 (`CabecalhoSequencialIdentificador`, slot 2961): header of the BEHB report

```cpp
constexpr std::size_t LARGURA_LINHA = 38;
std::string AlinhaEmColunas(const std::string& esq, const std::string& dir)   // inlined
{   const auto n = esq.size() + dir.size();
    const std::string espacos(n < LARGURA_LINHA ? LARGURA_LINHA - n : 0, ' ');
    return esq + espacos + dir; }   // (const&, const&) concat inlined, then append (func 160)

std::string CabecalhoSequencialIdentificador()
{   return AlinhaEmColunas("Sequencial",
        NomeTipoIdentificador(CConfiguracaoEleicao::GetInst().GetTipoIdentificadorPrincipal())); }
// NomeTipoIdentificador: 1 "Título", 2 "CPF", 3 "Número livre", anything else "" (inlined switch).
// Not the same table as vota::TipoToStr (func 10586), which says "Identificador" for 3 and throws 9409 otherwise.
```

The padding string is a named lvalue: the first `+` is compiled as libc++'s `operator+(const string&, const
string&)` (fresh buffer, two inlined `memcpy`s). A temporary `std::string(n, ' ')` would select
`operator+(const string&, string&&)`, i.e. `rhs.insert(0, lhs)`, and no `insert` call exists in 11240.

It is used once, by `CGeraRelatorios::StartState` (func 12105, `cgerarelatorios.cpp:51`). That function prints,
at the end of the day and before the BU, the report **"Eleitores com habilitação biográfica"** (`behb.dat`,
written to `CPath::GetPathTrab(INTERNA)` and mirrored by `CSincronizaVota::SincronizaRelatorios`). The report is
printed only when the urna is biometric and not in demo mode. Per section with biographically released voters,
it prints this header and then one row per voter built with the **same inlined padding code**:
`AlinhaEmColunas(std::format("{:04}", sequencial), identidade)`. `identidade` is
`CEleitorDecorator::GetIdentidadePorTipo(eleitor, <voter's principal type, +104>)`. The width 38 is the
printer's line in the normal font. It matches `CompletaDireita(rótulo, 28)` + a 10-character date, and the
38-character `"======"` separators in the recorded `samples/bu-real/run-full/reports/behb.txt`. In that run no
voter was released biographically ("Nenhum eleitor passou por habilitação biográfica"), so the header itself
was not printed.

---

## 6. Data read and written

* **Read** (text sources only): `comum::CConfiguracaoEleicao +668` = `tipoIdentificadorPrincipalEleitor` of
  ASN.1 `ModuloProcessoEleitoral` (`TipoIdentificadorEleitor`: 1 numeroInscricaoEleitoral, 2 numeroCPF,
  3 numeroLivre), and `IInformacaoThreadOperador::GetTipoIdentidadeDigitada()` (the type of number the mesário
  typed).
* **Written**: nothing. The stubs would only free memory. No file, SQL or ASN.1 access in this unit.
* Static memory that the stubs release: the fingerprint buffers and título of `CControlaReconhecimento` (§3).
  In the web build they stay empty, because the WSQ encoder is a stub (cregistradigitaloperador.cpp).

---

## 7. Web-build specifics

* Emscripten `EXIT_RUNTIME=0`: `atexit`/`__cxa_atexit` are no-ops and the page never calls `exit()`. **None of the
  101 destructors can run**, and no singleton is ever destroyed. In the web build nothing is lost by this: these
  destructors only free memory.
* No pthreads: `std::lock_guard` in every `GetInst` compiles to nothing (lock) plus the empty `noexcept` residue
  (unlock, func 150). The same empty body is what the `s_mutex` stubs contain.
* The operator thread never runs in the public simulator (u27 §2): the operator-side accessors and the text
  sources 10660/10664/10692-10694 are unreachable from the page. They were exercised only through the patched
  harness `tools/bu/operator_harness.mjs`.
* The web entry point pushes `CExecucaoVotaCooperativa` before `IExecucaoVota::GetInst` (3594) runs, so the default
  `CExecucaoVota` guarded by the mutex of stub 10237 is never created. The mutex residue still runs on each
  call.

---

## 8. wasm / Emscripten observations

1. **Exit-time stubs of a never-exiting program.** 101 functions (1,286 bytes of code) and 101 table slots are dead weight.
   wasm-opt cannot remove them, because the table is exported (`__indirect_function_table`, export `Fb`, used by
   the glue's `invoke_*` trampolines through `getWasmTableEntry`) and so is not considered closed.
2. **The destroy helpers exist only because of the WebAssembly C++ ABI**: destructors return `this` and
   `WebAssemblyCXXABI::canCallMismatchedFunctionType()` is `false`. On ARM, where destructors also return `this`,
   that hook keeps its default `true`, so clang registers `&T::~T` directly, as it does on x86-64. The helpers are
   `__cxx_global_array_dtor[.N]` (`void(void*)`, argument null), not clang's `void()` `__dtor_` stubs (§2.2).
3. **merge-similar-functions on atexit stubs.** Stubs that differ only by the variable's address became 12-byte
   thunks `f(a) { body(a, @var) }` over 7 shared bodies (349, 389, 763, 1286, 1564, 1959, 2903). Four 38-byte stubs
   kept their body inline (11833, 11844, 11864, 11938). The mutex stubs are 10 bytes: `call 150` with the address.
4. **Devirtualised `delete`.** Every `~unique_ptr<State>` calls the concrete destructor directly
   (e.g. ICF 1284) instead of the virtual deleting destructor (vtable slot 1). The destructors themselves are
   ICF-merged across states with the same member layout (244 is shared by 49 classes).
5. **Guard variables of template statics.** The only trace of the 10 `CConfereVotoEmCargo<…>` instantiations'
   atexit registrations is 20 `if (!(g & 1)) g = 1;` in `__wasm_call_ctors`. Plain statics left nothing there.
   This gives a way to tell template/inline statics from TU statics in this binary.
6. **Table order follows the translation units.** The two stubs of a pair are adjacent both in function index and in
   table slot (e.g. 10433/10434 in slots 4216/4215; slots decrease as indices increase), and the stubs of one
   translation unit are consecutive, next to that TU's functions. Whether this is the source definition order
   cannot be shown (see the `CControlaReconhecimento` members in §3).
7. Strings are Latin-1 in the binary (`"Título"` is 6 bytes, `"Optou por votar em trânsito"` 27). The long
   literals are built inline (8-byte loads from the data segment into a fresh `operator new` buffer), not
   through `std::string(const char*)`. One exception: in 11240, `"Número livre"` goes through
   `string::assign(const char*)` (func 276).

---

## 9. Boletim de urna (BU) and encerramento

This unit does not generate, sign or print the BU. What it touches:

* the **destructors of the encerramento states** of the operator thread. These are `CIniciaFinalizacao` (10707/10708),
  `CEncerramentoHorarioInvalido` (10711/10712), `CPerguntaFilaEleitorVazia` (10716),
  `CAguardaEleitoresVotarem` (10719/10720), `CPedeTituloEncerramento` (10724/10725),
  `CTituloEncerramentoInvalido` (10729), `CEncerramentoAntecipado` (10733), `CConfirmaEncerramento` (10735),
  `CRegistroMesarioEncerrado` (10765) and `CFinalizaOperador` (10216). The flow itself (menu option 2 → time
  check → "Todas as pessoas presentes já votaram?" → presidente's título → confirmation → `estadoVota = 57` →
  message 7 → `CGeraBU`) is documented in u27 §4.6, u17 and `docs/bu/codepath.md` §2.3;
* the text **"Encerrar votação"** of that menu option (10694);
* the header of the BEHB report (11240). `CGeraRelatorios::StartState` prints BEHB after the BUJ
  (justificativas) and BIM (mesários) reports and before it moves to `CInicioBU` (u08 §4.1). Format, in
  38-column lines: `"Sequencial"` + spaces + the principal identifier label (`"Título"` / `"CPF"` /
  `"Número livre"`), then one line per voter: 4-digit sequential number, spaces, identifier.

---

## 10. Weird or risky code

1. **Biometric data kept in static storage and never wiped** (10516-10518). `CControlaReconhecimento::s_digitalCapturada`
   (the voter's WSQ fingerprint image), `s_digitalMesario` (the mesário's) and `s_tituloMesario` are process-lifetime
   statics. `CControlaReconhecimento::StartState` (10512) only does `clear()` on both vectors (`end = begin`):
   the bytes stay in the allocated capacity until the next capture overwrites them. A new capture is stored with
   `vector::assign` (func 1681, called from 10465 and 10451). When it is larger than the capacity, assign frees the
   old buffer (func 5493) without wiping it, and a smaller one leaves the tail of the previous image in place.
   The only other release is the exit-time destructor, and it also frees the memory without zeroing it. On a real urna built from the same code, the
   latest fingerprint images therefore stay in RAM between voters and in freed heap after exit. In the simulator
   the buffers are empty (WSQ stub) and the destructors never run.
2. **Singletons never destroyed / shutdown order.** In this build, no state or thread singleton is ever destroyed
   (§7). On a native urna these 101 destructors would run at `exit()` in reverse order of registration: reverse
   definition order inside one translation unit, and an order across translation units that follows the link order
   of their initialisers, which the language leaves unspecified. `GetInst()` hands out raw references (`return *s_instancia;`). A thread still running during `exit()`,
   or a destructor of one state that touches another state, would use a destroyed object. This cannot be checked
   from this build.
3. **Stubs that would free live singletons.** The 101 stubs stay callable through the exported function table.
   Any code or JS that calls a table slot by number, e.g. a `dynCall` with a wrong index, could run
   `s_instancia.reset()` on a live state (a use-after-free on the next `GetInst()` reference). No such call exists
   in the glue or the wasm.
4. **BEHB header label vs. row content** (11240). The header's label comes from the election-wide principal
   identifier (config +668). Each row prints the voter's identity by the voter's own principal type (+104). They
   are expected to agree. If they differ, the numbers are printed under the wrong label (e.g. CPFs under
   "Título"). An unknown type gives an empty label silently, where `vota::TipoToStr` throws 9409.
5. **Message selection in func 10664.** `"CPF não encontrado. Digite o Título."` appears only when the election's
   principal identifier is the título. In an election whose principal identifier is the CPF, a CPF that is not
   found gets the generic `"NÃO CADASTRADO nesta urna"`, with no hint to try the other identifier. Separately,
   a título that is not in this section's roll goes to the justification flow when justifications are
   accepted, not to this screen (checked with the harness). Both are consistent with the code's intent. Only
   the first can mislead the mesário.

---

## 11. Open questions

* Static data member (`CFoo::s_mutex`) or namespace-scope variable in the class's `.cpp`: the binary cannot tell.
  It only proves they are not function-local statics (§2.3).
* The original file of func 11240. It is referenced only by `CGeraRelatorios::StartState`, so
  `cgerarelatorios.cpp` is the natural home. With LTO it could also be a helper of `comum/relatorios` (the
  inlined `AlinhaEmColunas` may be a named `CRelUtil` helper).
* Inferred paths: `CAguardaInspecao` / `CConfirmaInspecionada` were placed in `operador/leidentidade/`, next to
  `CPedeIdentidade`, their creator. They could live in `operador/aguardaeleitor/`.
  `CEleitorOptouPorVotarEmTransito`/`CEleitorNaoEncontrado` follow u27's single `ieleitorimpedidovotar.cpp`.
* Why merge-similar-functions left the four 38-byte `~unique_ptr` stubs (11833/11864, 11844/11938) as two
  identical-shape pairs instead of thunks.

---

## 12. Mapping table (all 107 functions of u38)

Columns: *static* = address of the destroyed object (`s_instancia` = mutex + 24, or + 28 for the
`CConfereVotoEmCargo` template statics); *body* = what the stub calls; *original file* = the translation unit
that defines the static (where clang emits the stub; for the `CConfereVotoEmCargo` template statics, the TU that
instantiates them); *accessor / user* = the function that creates the object or uses the text source. Stub
symbols follow the project's descriptive convention `__dtor_<variable>` (shared with u39). clang's real symbol for
every one of them is an internal `__cxx_global_array_dtor[.N]` (§2.2).

| # | func | size | slot | reconstructed symbol | what it does | static | body | original file | reconstructed in | accessor / user |
|---|---|---|---|---|---|---|---|---|---|---|
| 1 | 10207 | 10 | 4593 | `__dtor_vota::CThreadOperador::s_mutex` | ~mutex of s_mutex | @1911684 | func 150 (empty ~mutex) | uenux2/src/app/vota/operador/cthreadoperador.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CThreadOperador::GetInst (func 270) |
| 2 | 10216 | 12 | 4581 | `__dtor_vota::CFinalizaOperador::s_instancia` | ~unique_ptr of s_instancia | @1911652 | thunk→349 (free only) | uenux2/src/app/vota/operador/cfinalizaoperador.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CFinalizaOperador::GetInst (func 5342 -> merged body 764) |
| 3 | 10228 | 10 | 4565 | `__dtor_vota::CThreadMonitor::s_mutex` | ~mutex of s_mutex | @1911572 | func 150 (empty ~mutex) | uenux2/src/app/vota/monitor/cthreadmonitor.cpp | src/uenux2/src/app/vota/u38-foreign-fragments.cpp | CThreadMonitor::GetInst (func 1898) |
| 4 | 10237 | 10 | 4549 | `__dtor_vota::IExecucaoVota::s_mutex` | ~mutex of s_mutex | @1911520 | func 150 (empty ~mutex) | uenux2/src/app/vota/iexecucaovota.cpp | src/uenux2/src/app/vota/u38-foreign-fragments.cpp | IExecucaoVota::GetInst (func 3594, srcloc iexecucaovota.cpp:74) |
| 5 | 10412 | 10 | 4244 | `__dtor_vota::CSuspensaoAutomaticaEleitor::s_mutex` | ~mutex of s_mutex | @1909376 | func 150 (empty ~mutex) | uenux2/src/app/vota/operador/aguardaeleitor/csuspensaoautomaticaeleitor.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlined in CEleitorDemorando::StartState (func 10440) |
| 6 | 10417 | 10 | 4236 | `__dtor_vota::CPerguntaEleitorVotando::s_mutex` | ~mutex of s_mutex | @1909348 | func 150 (empty ~mutex) | uenux2/src/app/vota/operador/aguardaeleitor/cperguntaeleitorvotando.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlined in CEleitorDemorando::ProcessInput (func 10439) |
| 7 | 10418 | 12 | 4235 | `__dtor_vota::CPerguntaEleitorVotando::s_instancia` | ~unique_ptr of s_instancia | @1909372 | thunk→389 (~ICF 244 + free) | uenux2/src/app/vota/operador/aguardaeleitor/cperguntaeleitorvotando.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlined in CEleitorDemorando::ProcessInput (func 10439) |
| 8 | 10428 | 10 | 4222 | `__dtor_vota::CMostraEleitorVotando::s_mutex` | ~mutex of s_mutex | @1909292 | func 150 (empty ~mutex) | uenux2/src/app/vota/operador/aguardaeleitor/cmostraeleitorvotando.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CMostraEleitorVotando::GetInst (func 1150) |
| 9 | 10429 | 12 | 4221 | `__dtor_vota::CMostraEleitorVotando::s_instancia` | ~unique_ptr of s_instancia | @1909316 | thunk→763 (~ICF 448 + free) | uenux2/src/app/vota/operador/aguardaeleitor/cmostraeleitorvotando.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CMostraEleitorVotando::GetInst (func 1150) |
| 10 | 10433 | 10 | 4216 | `__dtor_vota::CEleitorVotouNaoVotou::s_mutex` | ~mutex of s_mutex | @1909264 | func 150 (empty ~mutex) | uenux2/src/app/vota/operador/aguardaeleitor/celeitorvotounaovotou.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CEleitorVotouNaoVotou::GetInst (func 1901) |
| 11 | 10434 | 12 | 4215 | `__dtor_vota::CEleitorVotouNaoVotou::s_instancia` | ~unique_ptr of s_instancia | @1909288 | thunk→2903 (~ICF 1284 + free) | uenux2/src/app/vota/operador/aguardaeleitor/celeitorvotounaovotou.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CEleitorVotouNaoVotou::GetInst (func 1901) |
| 12 | 10441 | 10 | 4203 | `__dtor_vota::CEleitorDemorando::s_mutex` | ~mutex of s_mutex | @1909208 | func 150 (empty ~mutex) | uenux2/src/app/vota/operador/aguardaeleitor/celeitordemorando.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CEleitorDemorando::GetInst (func 2734) |
| 13 | 10442 | 12 | 4202 | `__dtor_vota::CEleitorDemorando::s_instancia` | ~unique_ptr of s_instancia | @1909232 | thunk→2903 (~ICF 1284 + free) | uenux2/src/app/vota/operador/aguardaeleitor/celeitordemorando.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CEleitorDemorando::GetInst (func 2734) |
| 14 | 10445 | 10 | 4197 | `__dtor_vota::CVerificaDadoEleitor::s_mutex` | ~mutex of s_mutex | @1909180 | func 150 (empty ~mutex) | uenux2/src/app/vota/operador/confirmaidentidade/cverificadadoeleitor.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CVerificaDadoEleitor::GetInst (func 2735) |
| 15 | 10446 | 12 | 4196 | `__dtor_vota::CVerificaDadoEleitor::s_instancia` | ~unique_ptr of s_instancia | @1909204 | thunk→389 (~ICF 244 + free) | uenux2/src/app/vota/operador/confirmaidentidade/cverificadadoeleitor.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CVerificaDadoEleitor::GetInst (func 2735) |
| 16 | 10449 | 10 | 4191 | `__dtor_vota::CDadoEleitorNaoConfere::s_mutex` | ~mutex of s_mutex | @1909152 | func 150 (empty ~mutex) | uenux2/src/app/vota/operador/confirmaidentidade/cdadoeleitornaoconfere.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlined in CVerificaDadoEleitor::ProcessInput (func 10443) |
| 17 | 10450 | 12 | 4190 | `__dtor_vota::CDadoEleitorNaoConfere::s_instancia` | ~unique_ptr of s_instancia | @1909176 | thunk→763 (~ICF 448 + free) | uenux2/src/app/vota/operador/confirmaidentidade/cdadoeleitornaoconfere.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlined in CVerificaDadoEleitor::ProcessInput (func 10443) |
| 18 | 10463 | 12 | 4170 | `__dtor_vota::CTentativaCapturaDigitalEsgotada::s_instancia` | ~unique_ptr of s_instancia | @1909092 | thunk→389 (~ICF 244 + free) | uenux2/src/app/vota/operador/confirmaidentidade/ctentativacapturadigitalesgotada.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlined in CRegistraDigitalOperador::ProcessTick (func 10451) |
| 19 | 10471 | 10 | 4163 | `__dtor_vota::CPedeDigital::s_mutex` | ~mutex of s_mutex | @1909040 | func 150 (empty ~mutex) | uenux2/src/app/vota/operador/confirmaidentidade/cpededigital.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CPedeDigital::GetInst (func 2740) |
| 20 | 10475 | 10 | 4157 | `__dtor_vota::CDigitalNaoReconhecida::s_mutex` | ~mutex of s_mutex | @1909012 | func 150 (empty ~mutex) | uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaoreconhecida.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlined in CPedeDigital::ObtemEstadoPosReconhecimentoBiometrico (func 5397) |
| 21 | 10476 | 12 | 4156 | `__dtor_vota::CDigitalNaoReconhecida::s_instancia` | ~unique_ptr of s_instancia | @1909036 | thunk→2903 (~ICF 1284 + free) | uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaoreconhecida.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlined in CPedeDigital::ObtemEstadoPosReconhecimentoBiometrico (func 5397) |
| 22 | 10479 | 10 | 4151 | `__dtor_vota::CDigitalNaoReconhecidaDecBiometria::s_mutex` | ~mutex of s_mutex | @1908984 | func 150 (empty ~mutex) | uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaoreconhecidadecbiometria.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlined in CDigitalNaoReconhecida::StartState (func 10474) |
| 23 | 10480 | 12 | 4150 | `__dtor_vota::CDigitalNaoReconhecidaDecBiometria::s_instancia` | ~unique_ptr of s_instancia | @1909008 | thunk→389 (~ICF 244 + free) | uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaoreconhecidadecbiometria.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlined in CDigitalNaoReconhecida::StartState (func 10474) |
| 24 | 10483 | 10 | 4145 | `__dtor_vota::CDigitalReconhecida::s_mutex` | ~mutex of s_mutex | @1908956 | func 150 (empty ~mutex) | uenux2/src/app/vota/operador/confirmaidentidade/cdigitalreconhecida.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlined in CPedeDigital::ObtemEstadoPosReconhecimentoBiometrico (func 5397) |
| 25 | 10484 | 12 | 4144 | `__dtor_vota::CDigitalReconhecida::s_instancia` | ~unique_ptr of s_instancia | @1908980 | thunk→763 (~ICF 448 + free) | uenux2/src/app/vota/operador/confirmaidentidade/cdigitalreconhecida.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlined in CPedeDigital::ObtemEstadoPosReconhecimentoBiometrico (func 5397) |
| 26 | 10487 | 10 | 4139 | `__dtor_vota::CDigitalNaoReconhecidaPorTempo::s_mutex` | ~mutex of s_mutex | @1908928 | func 150 (empty ~mutex) | uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaoreconhecidaportempo.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlined in CPedeDigital::ProcessTick (func 10465) |
| 27 | 10488 | 12 | 4138 | `__dtor_vota::CDigitalNaoReconhecidaPorTempo::s_instancia` | ~unique_ptr of s_instancia | @1908952 | thunk→763 (~ICF 448 + free) | uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaoreconhecidaportempo.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlined in CPedeDigital::ProcessTick (func 10465) |
| 28 | 10502 | 10 | 4115 | `__dtor_vota::CNomeEleitor::s_mutex` | ~mutex of s_mutex | @1908816 | func 150 (empty ~mutex) | uenux2/src/app/vota/operador/confirmaidentidade/cnomeeleitor.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CNomeEleitor::GetInst (func 5401) |
| 29 | 10503 | 12 | 4114 | `__dtor_vota::CNomeEleitor::s_instancia` | ~unique_ptr of s_instancia | @1908840 | thunk→389 (~ICF 244 + free) | uenux2/src/app/vota/operador/confirmaidentidade/cnomeeleitor.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CNomeEleitor::GetInst (func 5401) |
| 30 | 10509 | 10 | 4102 | `__dtor_vota::CInformaBioDesabilitadaDemo::s_mutex` | ~mutex of s_mutex | @1908760 | func 150 (empty ~mutex) | uenux2/src/app/vota/operador/confirmaidentidade/cinformabiodesabilitadademo.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CInformaBioDesabilitadaDemo::GetInst (func 5402) |
| 31 | 10510 | 12 | 4101 | `__dtor_vota::CInformaBioDesabilitadaDemo::s_instancia` | ~unique_ptr of s_instancia | @1908784 | thunk→389 (~ICF 244 + free) | uenux2/src/app/vota/operador/confirmaidentidade/cinformabiodesabilitadademo.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CInformaBioDesabilitadaDemo::GetInst (func 5402) |
| 32 | 10514 | 10 | 4096 | `__dtor_vota::CControlaReconhecimento::s_mutex` | ~mutex of s_mutex | @1908732 | func 150 (empty ~mutex) | uenux2/src/app/vota/operador/confirmaidentidade/ccontrolareconhecimento.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CControlaReconhecimento::GetInst (func 1256); static data members used by funcs 3614, 10425, 10451, 10465, 10512 |
| 33 | 10515 | 12 | 4095 | `__dtor_vota::CControlaReconhecimento::s_instancia` | ~unique_ptr of s_instancia | @1908756 | thunk→389 (~ICF 244 + free) | uenux2/src/app/vota/operador/confirmaidentidade/ccontrolareconhecimento.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CControlaReconhecimento::GetInst (func 1256); static data members used by funcs 3614, 10425, 10451, 10465, 10512 |
| 34 | 10516 | 36 | 4094 | `__dtor_vota::CControlaReconhecimento::s_tituloMesario` | ~string of s_tituloMesario | @1908696 | inline ~string | uenux2/src/app/vota/operador/confirmaidentidade/ccontrolareconhecimento.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CControlaReconhecimento::GetInst (func 1256); static data members used by funcs 3614, 10425, 10451, 10465, 10512 |
| 35 | 10517 | 39 | 4093 | `__dtor_vota::CControlaReconhecimento::s_digitalMesario` | ~vector<uebyte> of s_digitalMesario | @1908684 | inline ~vector | uenux2/src/app/vota/operador/confirmaidentidade/ccontrolareconhecimento.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CControlaReconhecimento::GetInst (func 1256); static data members used by funcs 3614, 10425, 10451, 10465, 10512 |
| 36 | 10518 | 39 | 4092 | `__dtor_vota::CControlaReconhecimento::s_digitalCapturada` | ~vector<uebyte> of s_digitalCapturada | @1908672 | inline ~vector | uenux2/src/app/vota/operador/confirmaidentidade/ccontrolareconhecimento.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CControlaReconhecimento::GetInst (func 1256); static data members used by funcs 3614, 10425, 10451, 10465, 10512 |
| 37 | 10525 | 10 | 4082 | `__dtor_vota::CHabilitaAudioEleitor::s_mutex` | ~mutex of s_mutex | @1908584 | func 150 (empty ~mutex) | uenux2/src/app/vota/operador/confirmaidentidade/chabilitaaudioeleitor.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CHabilitaAudioEleitor::GetInst (func 2743) |
| 38 | 10526 | 12 | 4081 | `__dtor_vota::CHabilitaAudioEleitor::s_instancia` | ~unique_ptr of s_instancia | @1908608 | thunk→389 (~ICF 244 + free) | uenux2/src/app/vota/operador/confirmaidentidade/chabilitaaudioeleitor.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CHabilitaAudioEleitor::GetInst (func 2743) |
| 39 | 10528 | 10 | 4076 | `__dtor_vota::CConfirmaInspecionada::s_mutex` | ~mutex of s_mutex | @1908556 | func 150 (empty ~mutex) | uenux2/src/app/vota/operador/leidentidade/cconfirmainspecionada.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlined in CAguardaInspecao::ProcessMessage (func 10530) |
| 40 | 10529 | 12 | 4075 | `__dtor_vota::CConfirmaInspecionada::s_instancia` | ~unique_ptr of s_instancia | @1908580 | thunk→389 (~ICF 244 + free) | uenux2/src/app/vota/operador/leidentidade/cconfirmainspecionada.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlined in CAguardaInspecao::ProcessMessage (func 10530) |
| 41 | 10531 | 10 | 4070 | `__dtor_vota::CAguardaInspecao::s_mutex` | ~mutex of s_mutex | @1908528 | func 150 (empty ~mutex) | uenux2/src/app/vota/operador/leidentidade/caguardainspecao.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlined in CPedeIdentidade::ProcessTick (func 10676) |
| 42 | 10532 | 12 | 4069 | `__dtor_vota::CAguardaInspecao::s_instancia` | ~unique_ptr of s_instancia | @1908552 | thunk→389 (~ICF 244 + free) | uenux2/src/app/vota/operador/leidentidade/caguardainspecao.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlined in CPedeIdentidade::ProcessTick (func 10676) |
| 43 | 10553 | 10 | 4004 | `__dtor_vota::impl::IInformacaoThreadOperador::s_mutex` | ~mutex of s_mutex | @1908504 | func 150 (empty ~mutex) | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | IInformacaoThreadOperador::GetInst (func 599, srcloc :554) |
| 44 | 10616 | 10 | 3949 | `__dtor_vota::CIniciaJustificativa::s_mutex` | ~mutex of s_mutex | @1905704 | func 150 (empty ~mutex) | uenux2/src/app/vota/operador/justificativa/iiniciajustificativa.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlined in CProcuraEleitor::StartState (func 10631) |
| 45 | 10621 | 10 | 3943 | `__dtor_vota::CConfirmaJustificativa::s_mutex` | ~mutex of s_mutex | @1905676 | func 150 (empty ~mutex) | uenux2/src/app/vota/operador/justificativa/iconfirmajustificativa.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlined in CProcuraEleitor::StartState (func 10631) |
| 46 | 10629 | 10 | 3931 | `__dtor_vota::CValidaIdentidade::s_mutex` | ~mutex of s_mutex | @1905648 | func 150 (empty ~mutex) | uenux2/src/app/vota/operador/leidentidade/cvalidaidentidade.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlined in CPedeIdentidade::ProcessInput (func 10677) |
| 47 | 10644 | 10 | 3909 | `__dtor_vota::CEleitorJaVotou::s_mutex` | ~mutex of s_mutex | @1905536 | func 150 (empty ~mutex) | uenux2/src/app/vota/operador/confirmaidentidade/celeitorjavotou.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlined in CEleitorEncontrado::StartState (func 10635) |
| 48 | 10645 | 12 | 3908 | `__dtor_vota::CEleitorJaVotou::s_instancia` | ~unique_ptr of s_instancia | @1905560 | thunk→389 (~ICF 244 + free) | uenux2/src/app/vota/operador/confirmaidentidade/celeitorjavotou.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlined in CEleitorEncontrado::StartState (func 10635) |
| 49 | 10660 | 87 | 3876 | `vota::(anonymous namespace)::TextoOptouTransito` | text "Optou por votar em trânsito" | — |  | uenux2/src/app/vota/operador/leidentidade/ieleitorimpedidovotar.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | slot stored by the CEleitorOptouPorVotarEmTransito ctor inlined in CEleitorEncontrado::StartState (func 10635) |
| 50 | 10661 | 10 | 3879 | `__dtor_vota::CEleitorOptouPorVotarEmTransito::s_mutex` | ~mutex of s_mutex | @1905396 | func 150 (empty ~mutex) | uenux2/src/app/vota/operador/leidentidade/ieleitorimpedidovotar.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlined in CEleitorEncontrado::StartState (func 10635) |
| 51 | 10662 | 12 | 3878 | `__dtor_vota::CEleitorOptouPorVotarEmTransito::s_instancia` | ~unique_ptr of s_instancia | @1905420 | thunk→1959 (~IEleitorImpedidoVotar + free) | uenux2/src/app/vota/operador/leidentidade/ieleitorimpedidovotar.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlined in CEleitorEncontrado::StartState (func 10635) |
| 52 | 10664 | 214 | 3867 | `vota::(anonymous namespace)::TextoNaoEncontrado` | text "CPF não encontrado. Digite o Título." / "NÃO CADASTRADO nesta urna" | — |  | uenux2/src/app/vota/operador/leidentidade/ieleitorimpedidovotar.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | slot stored by the CEleitorNaoEncontrado ctor in CEleitorNaoEncontrado::GetInst (func 5424); calls 5416, 187 |
| 53 | 10665 | 10 | 3870 | `__dtor_vota::CEleitorNaoEncontrado::s_mutex` | ~mutex of s_mutex | @1905368 | func 150 (empty ~mutex) | uenux2/src/app/vota/operador/leidentidade/ieleitorimpedidovotar.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CEleitorNaoEncontrado::GetInst (func 5424) |
| 54 | 10666 | 12 | 3869 | `__dtor_vota::CEleitorNaoEncontrado::s_instancia` | ~unique_ptr of s_instancia | @1905392 | thunk→1959 (~IEleitorImpedidoVotar + free) | uenux2/src/app/vota/operador/leidentidade/ieleitorimpedidovotar.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CEleitorNaoEncontrado::GetInst (func 5424) |
| 55 | 10692 | 75 | 3830 | `vota::(anonymous namespace)::TextoOpcaoContadores` | text "Exibir contadores" | — |  | uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | src/uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp (u27; checked by u38) | slot stored by CEscolheOpcao::GetInst/ctor (func 2753), menu entry 4 (or 3) |
| 56 | 10693 | 75 | 3829 | `vota::(anonymous namespace)::TextoOpcaoRegistrarMesarios` | text "Registrar mesários" | — |  | uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | src/uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp (u27; checked by u38) | slot stored by CEscolheOpcao::GetInst/ctor (func 2753), menu entry 3 |
| 57 | 10694 | 63 | 3828 | `vota::(anonymous namespace)::TextoOpcaoEncerrar` | text "Encerrar votação" | — |  | uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | src/uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp (u27; checked by u38) | slot stored by CEscolheOpcao::GetInst/ctor (func 2753), menu entry 2 |
| 58 | 10696 | 10 | 3832 | `__dtor_vota::CEscolheOpcao::s_mutex` | ~mutex of s_mutex | @1905284 | func 150 (empty ~mutex) | uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CEscolheOpcao::GetInst (func 2753) |
| 59 | 10704 | 10 | 3822 | `__dtor_vota::CContadoresBiometria::s_mutex` | ~mutex of s_mutex | @1905256 | func 150 (empty ~mutex) | uenux2/src/app/vota/operador/outrasopcoes/ccontadoresbiometria.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlined in CEscolheOpcao::ProcessInput (func 10689) |
| 60 | 10705 | 12 | 3821 | `__dtor_vota::CContadoresBiometria::s_instancia` | ~unique_ptr of s_instancia | @1905280 | thunk→389 (~ICF 244 + free) | uenux2/src/app/vota/operador/outrasopcoes/ccontadoresbiometria.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlined in CEscolheOpcao::ProcessInput (func 10689) |
| 61 | 10707 | 10 | 3814 | `__dtor_vota::CIniciaFinalizacao::s_mutex` | ~mutex of s_mutex | @1905228 | func 150 (empty ~mutex) | uenux2/src/app/vota/operador/outrasopcoes/ciniciafinalizacao.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlined in CEscolheOpcao::ProcessInput (func 10689) |
| 62 | 10708 | 12 | 3813 | `__dtor_vota::CIniciaFinalizacao::s_instancia` | ~unique_ptr of s_instancia | @1905252 | thunk→349 (free only) | uenux2/src/app/vota/operador/outrasopcoes/ciniciafinalizacao.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlined in CEscolheOpcao::ProcessInput (func 10689) |
| 63 | 10711 | 10 | 3808 | `__dtor_vota::CEncerramentoHorarioInvalido::s_mutex` | ~mutex of s_mutex | @1905200 | func 150 (empty ~mutex) | uenux2/src/app/vota/operador/outrasopcoes/cencerramentohorarioinvalido.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlined in CIniciaFinalizacao::StartState (func 10706) |
| 64 | 10712 | 12 | 3807 | `__dtor_vota::CEncerramentoHorarioInvalido::s_instancia` | ~unique_ptr of s_instancia | @1905224 | thunk→389 (~ICF 244 + free) | uenux2/src/app/vota/operador/outrasopcoes/cencerramentohorarioinvalido.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlined in CIniciaFinalizacao::StartState (func 10706) |
| 65 | 10716 | 10 | 3802 | `__dtor_vota::CPerguntaFilaEleitorVazia::s_mutex` | ~mutex of s_mutex | @1905172 | func 150 (empty ~mutex) | uenux2/src/app/vota/operador/outrasopcoes/cperguntafilaeleitorvazia.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CPerguntaFilaEleitorVazia::GetInst (func 5426) |
| 66 | 10719 | 10 | 3797 | `__dtor_vota::CAguardaEleitoresVotarem::s_mutex` | ~mutex of s_mutex | @1905144 | func 150 (empty ~mutex) | uenux2/src/app/vota/operador/outrasopcoes/caguardaeleitoresvotarem.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlined in CPerguntaFilaEleitorVazia::ProcessInput (func 10713) |
| 67 | 10720 | 12 | 3796 | `__dtor_vota::CAguardaEleitoresVotarem::s_instancia` | ~unique_ptr of s_instancia | @1905168 | thunk→389 (~ICF 244 + free) | uenux2/src/app/vota/operador/outrasopcoes/caguardaeleitoresvotarem.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlined in CPerguntaFilaEleitorVazia::ProcessInput (func 10713) |
| 68 | 10724 | 10 | 3791 | `__dtor_vota::CPedeTituloEncerramento::s_mutex` | ~mutex of s_mutex | @1905116 | func 150 (empty ~mutex) | uenux2/src/app/vota/operador/outrasopcoes/cpedetituloencerramento.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CPedeTituloEncerramento::GetInst (func 3631) |
| 69 | 10725 | 12 | 3790 | `__dtor_vota::CPedeTituloEncerramento::s_instancia` | ~unique_ptr of s_instancia | @1905140 | thunk→389 (~ICF 244 + free) | uenux2/src/app/vota/operador/outrasopcoes/cpedetituloencerramento.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CPedeTituloEncerramento::GetInst (func 3631) |
| 70 | 10729 | 10 | 3783 | `__dtor_vota::CTituloEncerramentoInvalido::s_mutex` | ~mutex of s_mutex | @1905088 | func 150 (empty ~mutex) | uenux2/src/app/vota/operador/outrasopcoes/ctituloencerramentoinvalido.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlined in CPedeTituloEncerramento::ProcessInput (func 10721) |
| 71 | 10733 | 12 | 3775 | `__dtor_vota::CEncerramentoAntecipado::s_instancia` | ~unique_ptr of s_instancia | @1905084 | thunk→389 (~ICF 244 + free) | uenux2/src/app/vota/operador/outrasopcoes/cencerramentoantecipado.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlined in CPedeTituloEncerramento::StartState (func 10722) |
| 72 | 10735 | 10 | 3770 | `__dtor_vota::CConfirmaEncerramento::s_mutex` | ~mutex of s_mutex | @1905032 | func 150 (empty ~mutex) | uenux2/src/app/vota/operador/outrasopcoes/cconfirmaencerramento.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CConfirmaEncerramento::GetInst (func 3632) |
| 73 | 10755 | 10 | 3737 | `__dtor_vota::CHabilitacaoAudioNaoPermitida::s_mutex` | ~mutex of s_mutex | @1904920 | func 150 (empty ~mutex) | uenux2/src/app/vota/operador/outrasopcoes/chabilitacaoaudionaopermitida.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlined in CEscolheOpcao::ProcessInput (func 10689) |
| 74 | 10756 | 12 | 3736 | `__dtor_vota::CHabilitacaoAudioNaoPermitida::s_instancia` | ~unique_ptr of s_instancia | @1904944 | thunk→389 (~ICF 244 + free) | uenux2/src/app/vota/operador/outrasopcoes/chabilitacaoaudionaopermitida.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlined in CEscolheOpcao::ProcessInput (func 10689) |
| 75 | 10760 | 10 | 3731 | `__dtor_vota::CHorarioVotacaoTerminou::s_mutex` | ~mutex of s_mutex | @1904892 | func 150 (empty ~mutex) | uenux2/src/app/vota/operador/outrasopcoes/chorariovotacaoterminou.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CHorarioVotacaoTerminou::GetInst (func 5430) |
| 76 | 10761 | 12 | 3730 | `__dtor_vota::CHorarioVotacaoTerminou::s_instancia` | ~unique_ptr of s_instancia | @1904916 | thunk→389 (~ICF 244 + free) | uenux2/src/app/vota/operador/outrasopcoes/chorariovotacaoterminou.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CHorarioVotacaoTerminou::GetInst (func 5430) |
| 77 | 10765 | 12 | 3724 | `__dtor_vota::CRegistroMesarioEncerrado::s_instancia` | ~unique_ptr of s_instancia | @1904888 | thunk→349 (free only) | uenux2/src/app/vota/operador/outrasopcoes/cregistromesarioencerrado.cpp (path inferred) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlined in CControladorRegistraMesariosVota::GetEstadoAposRegistroVotacao (vf11, func 10793) |
| 78 | 11240 | 772 | 2961 | `vota::(anonymous namespace)::CabecalhoSequencialIdentificador` | BEHB header "Sequencial" + spaces + Título/CPF/Número livre (38 cols) | — |  | uenux2/src/app/vota/eleitor/fimvotacao/cgerarelatorios.cpp | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | slot stored by CGeraRelatorios::StartState (func 12105) via comum_f604 (CDataText<std::string(*)()> in a CTextFieldPaper) |
| 79 | 11674 | 12 | 2095 | `__dtor_vota::CConfereVotoEmCargo<vota::CMajoritarioValido, (vota::ETelaVotacao)2>::s_instancia` | ~unique_ptr of s_instancia | @1838480 | thunk→1286 (~IConfereVotoEmCargo + free) | uenux2/src/app/vota/eleitor/votamajoritario/cpedemajoritario.cpp (instantiating TU; definition in cconferevotoemcargo.h, path inferred) | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | GetInst inlined in CPedeMajoritario::ProcessInputAudio (func 11683) |
| 80 | 11675 | 10 | 2094 | `__dtor_vota::CConfereVotoEmCargo<vota::CMajoritarioValido, (vota::ETelaVotacao)2>::s_mutex` | ~mutex of s_mutex | @1838452 | func 150 (empty ~mutex) | uenux2/src/app/vota/eleitor/votamajoritario/cpedemajoritario.cpp (instantiating TU; definition in cconferevotoemcargo.h, path inferred) | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | GetInst inlined in CPedeMajoritario::ProcessInputAudio (func 11683) |
| 81 | 11708 | 12 | 2044 | `__dtor_vota::CConfereVotoEmCargo<vota::CProporcionalBranco, (vota::ETelaVotacao)4>::s_instancia` | ~unique_ptr of s_instancia | @1838140 | thunk→1286 (~IConfereVotoEmCargo + free) | uenux2/src/app/vota/eleitor/votaproporcional/cpedeproporcional.cpp (instantiating TU; definition in cconferevotoemcargo.h, path inferred) | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | GetInst inlined in CPedeProporcional::ProcessInputAudio (func 11711) |
| 82 | 11709 | 10 | 2043 | `__dtor_vota::CConfereVotoEmCargo<vota::CProporcionalBranco, (vota::ETelaVotacao)4>::s_mutex` | ~mutex of s_mutex | @1838112 | func 150 (empty ~mutex) | uenux2/src/app/vota/eleitor/votaproporcional/cpedeproporcional.cpp (instantiating TU; definition in cconferevotoemcargo.h, path inferred) | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | GetInst inlined in CPedeProporcional::ProcessInputAudio (func 11711) |
| 83 | 11713 | 10 | 2046 | `__dtor_vota::CPedeProporcional::s_mutex` | ~mutex of s_mutex | @1838084 | func 150 (empty ~mutex) | uenux2/src/app/vota/eleitor/votaproporcional/cpedeproporcional.cpp | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | CPedeProporcional::GetInst (func 3849 -> merged body 6051) |
| 84 | 11722 | 12 | 2026 | `__dtor_vota::CConfereVotoEmCargo<vota::CConfirmaVotoLegenda, (vota::ETelaVotacao)10>::s_instancia` | ~unique_ptr of s_instancia | @1838004 | thunk→1286 (~IConfereVotoEmCargo + free) | uenux2/src/app/vota/eleitor/votaproporcional/cpedenominal.cpp (path inferred) (instantiating TU; definition in cconferevotoemcargo.h, path inferred) | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | GetInst inlined in CPedeNominal::GetProximoEstado (vf16, func 11724) |
| 85 | 11802 | 10 | 1898 | `__dtor_vota::testeteclado::impl::IGeradorTeclas::s_mutex` | ~mutex of s_mutex | @1837656 | func 150 (empty ~mutex) | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.cpp | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | IGeradorTeclas::GetInst (srcloc ctesteteclado.cpp:121) inlined in CTesteTeclado::StartState (func 11805) |
| 86 | 11807 | 10 | 1897 | `__dtor_vota::testeteclado::CTesteTeclado::s_mutex` | ~mutex of s_mutex | @1835124 | func 150 (empty ~mutex) | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.cpp | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | CTesteTeclado::GetInst (func 3855) |
| 87 | 11827 | 10 | 1864 | `__dtor_vota::testeteclado::CEsperaRetestar::s_mutex` | ~mutex of s_mutex | @1835012 | func 150 (empty ~mutex) | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cesperaretestar.cpp (path inferred) | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | CEsperaRetestar::GetInst (func 5936) |
| 88 | 11828 | 12 | 1863 | `__dtor_vota::testeteclado::CEsperaRetestar::s_instancia` | ~unique_ptr of s_instancia | @1835036 | thunk→1564 (~ICF 785 + free) | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cesperaretestar.cpp (path inferred) | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | CEsperaRetestar::GetInst (func 5936) |
| 89 | 11832 | 10 | 1856 | `__dtor_vota::testeteclado::CRetomada::s_mutex` | ~mutex of s_mutex | @1834972 | func 150 (empty ~mutex) | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cretomada.cpp (path inferred) | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | GetInst inlined in CAjusteInicial::ValidaTemposDesligamento (func 7160) |
| 90 | 11833 | 38 | 1855 | `__dtor_vota::testeteclado::CRetomada::s_instancia` | ~unique_ptr of s_instancia | @1834996 | inline ~testeteclado::CBase (1559) + free | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cretomada.cpp (path inferred) | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | GetInst inlined in CAjusteInicial::ValidaTemposDesligamento (func 7160) |
| 91 | 11844 | 38 | 1837 | `__dtor_vota::CRegerarZeresima::s_instancia` | ~unique_ptr of s_instancia | @1834912 | inline ~CGeraZeresimaBase (1720) + free | uenux2/src/app/vota/eleitor/iniciovotacao/cregerarzeresima.cpp (path inferred) | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | GetInst inlined in CConfirmaRegerarZeresima::ProcessInput (func 11838) |
| 92 | 11863 | 10 | 1802 | `__dtor_vota::testeteclado::CPreZeresima::s_mutex` | ~mutex of s_mutex | @1834720 | func 150 (empty ~mutex) | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cprezeresima.cpp | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | GetInst inlined in CVerificaEleicaoPassou::StartState (func 11914) |
| 93 | 11864 | 38 | 1801 | `__dtor_vota::testeteclado::CPreZeresima::s_instancia` | ~unique_ptr of s_instancia | @1834744 | inline ~testeteclado::CBase (1559) + free | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cprezeresima.cpp | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | GetInst inlined in CVerificaEleicaoPassou::StartState (func 11914) |
| 94 | 11866 | 10 | 1797 | `__dtor_vota::CGeraDadosDinamicos::s_mutex` | ~mutex of s_mutex | @1834692 | func 150 (empty ~mutex) | uenux2/src/app/vota/eleitor/iniciovotacao/cgeradadosdinamicos.cpp | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | GetInst inlined in testeteclado::CPreZeresima::GetEstadoPassouNoTeste (func 5945) |
| 95 | 11867 | 12 | 1796 | `__dtor_vota::CGeraDadosDinamicos::s_instancia` | ~unique_ptr of s_instancia | @1834716 | thunk→349 (free only) | uenux2/src/app/vota/eleitor/iniciovotacao/cgeradadosdinamicos.cpp | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | GetInst inlined in testeteclado::CPreZeresima::GetEstadoPassouNoTeste (func 5945) |
| 96 | 11872 | 10 | 1789 | `__dtor_vota::CVerificaHorarioZeresima::s_mutex` | ~mutex of s_mutex | @1834664 | func 150 (empty ~mutex) | uenux2/src/app/vota/eleitor/iniciovotacao/cverificahorariozeresima.cpp | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | CVerificaHorarioZeresima::GetInst (func 5947) |
| 97 | 11873 | 12 | 1788 | `__dtor_vota::CVerificaHorarioZeresima::s_instancia` | ~unique_ptr of s_instancia | @1834688 | thunk→1564 (~ICF 785 + free) | uenux2/src/app/vota/eleitor/iniciovotacao/cverificahorariozeresima.cpp | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | CVerificaHorarioZeresima::GetInst (func 5947) |
| 98 | 11878 | 10 | 1780 | `__dtor_vota::CVisualizarCandidatos::s_mutex` | ~mutex of s_mutex | @1834636 | func 150 (empty ~mutex) | uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cvisualizarcandidatos.cpp | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | CVisualizarCandidatos::GetInst (func 1279) |
| 99 | 11889 | 10 | 1764 | `__dtor_vota::CMenuFiltrarCandidatosPorCargo::s_mutex` | ~mutex of s_mutex | @1834552 | func 150 (empty ~mutex) | uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatosporcargo.cpp | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | GetInst inlined in CMenuVisualizarCandidatos::StartState (func 11881) |
| 100 | 11890 | 12 | 1763 | `__dtor_vota::CMenuFiltrarCandidatosPorCargo::s_instancia` | ~unique_ptr of s_instancia | @1834576 | thunk→349 (free only) | uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatosporcargo.cpp | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | GetInst inlined in CMenuVisualizarCandidatos::StartState (func 11881) |
| 101 | 11893 | 10 | 1758 | `__dtor_vota::CMenuFiltrarCandidatosPorNumero::s_mutex` | ~mutex of s_mutex | @1834524 | func 150 (empty ~mutex) | uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatospornumero.cpp (path inferred) | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | CMenuFiltrarCandidatosPorNumero::GetInst (func 5954) |
| 102 | 11894 | 12 | 1757 | `__dtor_vota::CMenuFiltrarCandidatosPorNumero::s_instancia` | ~unique_ptr of s_instancia | @1834548 | thunk→389 (~ICF 244 + free) | uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatospornumero.cpp (path inferred) | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | CMenuFiltrarCandidatosPorNumero::GetInst (func 5954) |
| 103 | 11898 | 10 | 1752 | `__dtor_vota::CMaisInformacoes::s_mutex` | ~mutex of s_mutex | @1834496 | func 150 (empty ~mutex) | uenux2/src/app/vota/eleitor/iniciovotacao/cmaisinformacoes.cpp (path inferred) | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | CMaisInformacoes::GetInst (func 1280) |
| 104 | 11899 | 12 | 1751 | `__dtor_vota::CMaisInformacoes::s_instancia` | ~unique_ptr of s_instancia | @1834520 | thunk→389 (~ICF 244 + free) | uenux2/src/app/vota/eleitor/iniciovotacao/cmaisinformacoes.cpp (path inferred) | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | CMaisInformacoes::GetInst (func 1280) |
| 105 | 11918 | 10 | 1720 | `__dtor_vota::CReinicioComparecimentoMesario::s_mutex` | ~mutex of s_mutex | @1834328 | func 150 (empty ~mutex) | uenux2/src/app/vota/eleitor/creiniciocomparecimentomesario.cpp (path inferred) | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | CReinicioComparecimentoMesario::GetInst (func 3864 -> merged body 764) |
| 106 | 11919 | 12 | 1719 | `__dtor_vota::CReinicioComparecimentoMesario::s_instancia` | ~unique_ptr of s_instancia | @1834352 | thunk→349 (free only) | uenux2/src/app/vota/eleitor/creiniciocomparecimentomesario.cpp (path inferred) | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | CReinicioComparecimentoMesario::GetInst (func 3864 -> merged body 764) |
| 107 | 11938 | 38 | 1690 | `__dtor_vota::CGeraZeresima::s_instancia` | ~unique_ptr of s_instancia | @1834212 | inline ~CGeraZeresimaBase (1720) + free | uenux2/src/app/vota/eleitor/iniciovotacao/cgerazeresima.cpp (path inferred) | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | CGeraZeresima::GetInst (func 5962) |
