# u19: `api::CPolySingletonList`, the registry of "poly-singletons", and the code the tools filed with it

Unit u19 has 184 wasm functions that the tools attributed to one original file,
`uenux2/src/api/pattern/cpolysingletonlist.h`. That header is the **service registry of the urna
application**. Code asks for "the" `api::IScreen`, `api::IInputKbd`, `comum::IInterfaceSavd`,
`vota::IExecucaoVota` and so on, and gets whatever implementation was registered for that interface.
This is the seam that lets the same voting application run on the urna (real drivers) and in the browser
(the `simulador::CWasm*` and `api::teste::*` mocks).

The unit breaks down like this:

| group | functions | what they are |
|---|---:|---|
| `CPolySingleton<T>::instance(info, loc)` | 30 | one per interface, with `CPolySingletonList::instance<T>` and the lock inlined. The tools named them `api::CPolySingletonList::instance@N` |
| `CPolySingletonList::push<T>` / `replace<T>` | 18 | registration bodies: 4 `push` alone (5384, 5395, 5398, 5407) + their merged body 3882; 9 register-or-replace wrappers with `push` inlined (7667, 8758…10090) and 1 without (8728); the by-value helper 4162 and its two fully inlined versions 4879/4890 (§3.4) |
| `CPolySingletonList::exists<T>` | 37 | 6 bodies, 23 thunks and 8 single instantiations |
| other registry members | 16 | `find`, `erase`, `log`, `trace`, the syslog/printf back end, the read/write lock, the error constructor, the entry and registry destructors |
| libc++ helpers | 48 | helpers instantiated for the registry (vector, `shared_ptr`, `std::format` argument packing), two for the battery-icon map, and a few generic ones |
| **foreign functions** | 35 | functions of other classes with a `GetInst()` inlined into them (the reason the tools put them here). Among them: the **simulator's hardware bootstrap** (func 8302), the **BU QR code data source** (func 1956), the battery icon, the TTS front end, and operator (mesário) screens |

82 of the 184 ran during the recorded votes (`analysis/runtime/*.functions.tsv`). Every start-up
registration, the status header's battery icon, the voter-side lookups (screen, keyboard, beep, sound,
log) and `CSincronismoEleitor::ProcessMessage` are among them.

Reconstructed sources:

```
src/uenux2/src/api/pattern/cpolysingletonlist.h          the registry, the lock, instance/exists/push/replace/erase/find (templates)
src/uenux2/src/api/pattern/cpolysingleton.h              CPolySingleton<T>::instance(info, loc) + list of the 30 instantiations
src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp    log() / LogUenux() bodies + index of every instantiation
src/uenux2/mock/app/simulador/wasm/csimuladorwasm.u19.cpp  func 8302: platform singletons of the web build (path inferred)
src/uenux2/mock/app/simulador/wasm/cwasmclp.u19.cpp      CLp::GetInst / ~CLp / static dtor (3876, 5978, 12016)
src/uenux2/src/app/vota/eleitor/fimvotacao/cmostraqrcodebu.u19.cpp   GetQRDSInst (BU QR parts), QRCodeAtual, TextoInstrucaoQRCode
src/uenux2/src/app/vota/iexecucaovota.u19.cpp            IExecucaoVota::GetInst (3594)
src/uenux2/src/app/comum/validamidia/cvalidamidia.u19.cpp  IValidaMidia::GetInst (5575)
src/uenux2/src/api/util/isystemdatetime.u19.cpp          ISystemDateTime::GetInst + CreateInst<T> (1155)
src/uenux2/src/api/util/iajustedatahora.u19.cpp          IAjusteDataHora::CreateInst (10876) (path inferred)
src/uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.u19.cpp  GetInst (599) + outlined calls (3623, 5415, 10584, 10670)
src/uenux2/src/app/vota/operador/u19-foreign-fragments.cpp   MT ProcessInput of 7 operator states (+ merged 6015, 1536)
src/uenux2/src/app/vota/eleitor/u19-foreign-fragments.cpp    CConfirmaVotoNominal slot 17 (5925), CSincronismoEleitor::ProcessMessage (7181)
src/uenux2/src/app/comum/comparecimentomesario/u19-foreign-fragments.cpp   merged ProcessInput 6011
src/uenux2/src/app/vota/monitor/cthreadmonitor.u19.cpp   std::async lambda of SaiPorVotacaoSuspensa (10224)
src/uenux2/src/api/gui/cpowerinformation.u19.cpp         battery icon table/state/timer (2773, 2774, 10981, 10987, 10991)
src/uenux2/src/api/audio/itexttospeech.u19.cpp           ITextToSpeech slot 0 (11530)
```

---

## 1. Purpose, and the Portuguese terms

* **Poly-singleton**: TSE's name for "a singleton chosen by interface" (polymorphic singleton). Every
  platform service is an abstract interface (`api::I…`). One implementation per interface is registered at
  start-up (`push`) and fetched everywhere else (`instance`). Many interfaces also have a `GetInst()` that
  registers an urna default implementation the first time it is called (§3.4).
* **urna / UE (urna eletrônica)**: the voting machine. **mesário**: poll worker. **MT (microterminal)**:
  the poll worker's small keypad and LCD. **eleitor**: voter.
* **BU (boletim de urna)**: the signed per-machine result. **BU digital**: the BU as QR codes (§7).
  **RDV (registro digital do voto)**: the shuffled record of the votes.
* **SAVD**: the security module interface (`comum::IInterfaceSavd`). **MR (mídia de resultado)**: the
  results memory card.

In the voting process this unit is the plumbing under everything. When the voter presses a key, the thread
reads it through `CPolySingleton<IInputKbd>::instance`, draws through `…<IScreen>`, beeps through
`…<IBeep>`, and logs through `…<CEscritorLog>`. At the end of the day the BU QR data source lives in this
registry.

## 2. Data structures and class relationships

### 2.1 The registry

```
TPolySingletonsInfo   (148 bytes, one static instance @0x1BF600 = 1832448, guard byte @1832596)
  +0   std::vector<SPolySingleton> lista
  +12  bool debug                     set to true by the accessor on every call (see §5)
  +16  CUpgradeMutex mutex            (132 bytes)

SPolySingleton  (24 bytes; the name of the struct is inferred)
  +0   std::string nome               typeid(INTERFACE).name(), e.g. "N3api7IScreenE"
  +12  const std::type_info* tipo     &typeid(INTERFACE)
  +16  std::shared_ptr<void> instancia  control block __shared_ptr_pointer<INTERFACE*, default_delete<INTERFACE>>

CUpgradeMutex  (name inferred; a writer-preferring read/write lock with "upgrade")
  +0   std::mutex                     +24 condition_variable (readers' gate)   +72 condition_variable (writers' gate)
  +120 int readers                    +124 int writers waiting                +128 bool writer active
```

* **Accessor.** Code never names the static registry directly. It calls a *function pointer* stored in data
  at **@1526320** (table slot 284 → func 11265). So every "`GetPolySingletonsInfo()`" in the binary is a
  `call_indirect(d_…[0])`. Func 11265 returns the static local and **stores `debug = true` on every call**,
  outside the static-init guard.
* **Lookup** is linear. `std::find_if` (func 6779) walks the 24-byte entries comparing `std::string` keys.
  Then `*it->tipo != typeid(T)` (libc++ compares the `__type_name` pointers).
* **Ownership.** Entries hold `shared_ptr`s, but `instance()` returns a raw `T&`. Callers keep plain
  references, and nothing tracks them.

### 2.2 Error classes (RTTI)

```
ecourna::api::exception::CError
 ├─ CBaseError<api::EUePatternError, SErrorLimits{6750, 6800}>        typeinfo @1526364, vtable @1526448, ctor thunk 235
 │     6754 "{}: solicitada uma instância não criada de {}"               CPolySingletonList::instance  line 99
 │     6755 "{}: instância {} não corresponde a interface solicitada de {}"  instance                  line 105
 │     6756 "{}: instância já criada de {}"                                 push                      line 129
 └─ CBaseError<ecourna::api::pattern::EPatternError, SErrorLimits{1300, 1325}>  typeinfo @1526600, vtable @1526620, ctor thunk shared_f331
       1301 "PolySingleton - solicitada uma instancia nao criada <mangled> [<file>:<line>]"   cpolysingleton.h:78
std::runtime_error: "unlock_shared() called with no shared owner", "unlock() called when no writer is active",
                    "unlock_shared_and_lock() requires an active shared owner"
```

`<file>:<line>` in the 1301 message is the **caller's** `std::source_location`, which every `GetInst()` passes
down. For example, `IExecucaoVota::GetInst` passes `iexecucaovota.cpp:74`.

### 2.3 The interfaces registered in this build (verified at run time, §4)

| # | interface | web implementation | registered by |
|---:|---|---|---|
| 0 | `api::ISystemDateTime` | `api::CSystemDateTime` (default, then replaced) | first `ISystemDateTime::GetInst()` (func 1155), before `main` registers anything |
| 1 | `comum::IInterfaceSavd` | `(anonymous)::CWasmSavd` | `main` → replace 10090 |
| 2 | `ecourna::api::security::IRng` | `ecourna::api::security::CPrng` | `main` → 9233 |
| 3 | `ecourna::api::security::ISymmetricCipherFactory` | `CSymmetricCipherFactory` | `main` → 9135 |
| 4 | `api::IFingerPrepare` | `simulador::CFingerPrepareSimulador` | `main` → 9039 |
| 5 | `api::IAjusteDataHora` | `api::CAjusteDataHora` | `main` → func 10876 |
| 6–9 | `IGenericFactory<IThreadImpl/ISyncCtl/IRWSyncCtl/ISemaphore>` | `CDefaultGenericFactory<…, CWasmThread / CPosixMutex / CPosixRWMutex / CPosixSemaphore>` | `main` → 8986/8911/8834/8758 |
| 10 | `vota::impl::IPoliticaExecucaoEleitor` | `(anonymous)::CPoliticaExecucaoEleitorWeb` | `main` → replace wasm_entry_f9507 (→ push 5407) |
| 11 | `vota::impl::ISincronismoVotoEleitor` | `(anonymous)::CSincronismoVotoEleitorWeb` | `main` → replace wasm_entry_f9437 (→ push 5398) |
| 12 | `vota::IExecucaoVota` | `vota::CExecucaoVotaCooperativa` | `main` → replace 8728 (→ push 5395) |
| 13 | `api::ITimerScheduler` | `simulador::CWasmTimerScheduler` | func 8302 |
| 14 | `comum::IInterfaceInit` | `simulador::CWasmInit` | 8302 |
| 15 | `api::IScreen` | `simulador::CWasmScreen` | 8302 |
| 16 | `api::IScreenMT` | `simulador::CWasmScreenMT` | 8302 |
| 17 | `api::IInputKbd` | `simulador::CWasmInputKbd` | 8302 |
| 18 | `api::IInputMT` | `simulador::CWasmInputMT` | 8302 |
| 19 | `api::IResource` | `simulador::CWasmResource` | 8302 |
| 20 | `api::IUrna` | `api::teste::CUrnaMock` (model 2020, serial 87654321) | 8302 |
| 21 | `api::IPower` | `api::teste::CPowerMock` (mains, full battery) | 8302 |
| 22 | `api::IBeep` | `simulador::CWasmBeep` | 8302 |
| 23 | `api::ISound` | `simulador::CWasmNullSound`, **replaced** by `CWasmWebSound` | 8302, then the `main` lambda (replace libcxx_f10356 → push 5384) |
| 24 | `api::IPaperRelatorios` | `simulador::CWasmNullPaper` | 8302 |
| 25 | `api::IImpressoraRelatorios` | `simulador::CWasmNullPrinter` | 8302 |
| 26 | `api::ITextToSpeech` | `CWasmNullTextToSpeech`, **replaced** by `votaInit` (RHVoice or Null) | 8302 (via 4162), `votaInit` (via 4162 → 7667) |
| – | `api::ISystemDateTime` | **replaced** by `simulador::CWasmSystemDateTime` | 8302 → 4890 |
| 27 | `api::CEscritorLog` | `simulador::CWasmLogd(<log dir>/logd.dat)` | 8302 |

Lazily created on first use (not seen in the start-up trace): `vota::IQRCodeBUDS` (1956),
`vota::impl::IInformacaoThreadOperador` (599), `comum::impl::IValidaMidia` (5575),
`comum::IControladorRegistraMesarios`, `comum::CCalculaCV`, `comum::CGeracaoVersoesContratos`,
`vota::IAjusteInicial`, `vota::testeteclado::impl::IGeradorTeclas`, `comum::IEventosLog`, and the urna-only
`api::IKernelHSM`, `api::pkcs11::IPkcs11`, `api::IFingerScanner/Matcher/Detection`, `api::IPaper`,
`IGenericFactory<IHash/ITextEncoding>`. The last group has no implementation in the web build. The
`instance` functions exist, but calling them would throw 1301.

## 3. Control flow

### 3.1 `exists<T>(info)`

The function builds `std::string(typeid(T).name())`, takes a shared lock, calls `find`, unlocks and returns
`found`. wasm-opt merged the instantiations by the **length of the mangled name**, because the name is copied
with fixed 8-byte loads: 6131 (12 chars), 6132 (16), 6057 (20), 2924 (23). 1166 is the generic
out-of-line version and 3928 reads the name through `&typeid(T)`. `__wasm_call_ctors` makes 24 existence
checks (21 distinct `exists<T>` functions, 15 of them in this unit, plus 3 inlined copies), and discards
every result. They look like dynamic initialisers of never-read static data.

### 3.2 `CPolySingletonList::instance<T>(info)` (cpolysingletonlist.h:99/105)

```
shared lock -> key = typeid(T).name() -> find
  not found            : log("instance", key, "find");   throw 6754 (line 99)
  entry.tipo != typeid : log("instance", key, "typeid"); throw 6755 (line 105)
return *static_pointer_cast<T>(entry.instancia)    // a shared_ptr copy is made and dropped; the raw T& escapes
```

### 3.3 `CPolySingleton<T>::instance(info, loc)` (cpolysingleton.h:78), the 30 "instance@N" functions

```
if (!exists<T>(GetPolySingletonsInfo()))   // note: re-reads the accessor, ignores `info`
    throw 1301 "PolySingleton - solicitada uma instancia nao criada " + typeid(T).name()
               + " [" + loc.file_name() + ":" + to_string(loc.line()) + "]"
return CPolySingletonList::instance<T>(info);
```

Because of the `exists` check, the 6754/6755 throws inside are unreachable in a single-threaded program.

### 3.4 `push<T>(unique_ptr<T>&&, info)` (cpolysingletonlist.h:129) and the register-or-**replace** wrapper

`push` itself only appends. The "if it exists, erase it first" step belongs to a separate wrapper (its name is
not in the binary; the reconstruction calls it `replace`):

```
push(p, info):                                                  // srcloc :129
  exclusive lock (CUpgradeMutex::lock)
  if (find(name).found) { log("push", name, "find"); throw 6756 }   // unreachable single-threaded
  trace("push", name, format("sz[{}] ptr[{}]", lista.size(), (void*)&info))
  lista.emplace_back(name, &typeid(T), shared_ptr<T>(move(p)))

replace(p, info):                                               // name inferred
  if (exists<T>(info)) erase(typeid(T).name(), info);           // REPLACE semantics
  push(move(p), info)
```

Why two functions: the `GetInst()` functions 599, 3594, 5575, 5925 and 7181 call `exists` once and then run
the `push` body (inlined, or the out-of-line 5395/5407/5398) with **no** second `exists` call. `exists` takes
the lock and goes through JS `invoke_*`, so the compiler cannot prove its result and cannot drop a second call.
It did not drop it in 1956 (`GetQRDSInst`) and 10876 (`IAjusteDataHora::CreateInst`), which make the same
`if (!exists)` test and still call `exists` + `erase` again before the `push` body. Those two, the start-up code
(`main`, func 8302, `votaInit`) and `CreateInst<T>` therefore call the wrapper. 8728, `wasm_entry_f9437/9507` and
`libcxx_f10356` are the wrapper with `push` left out of line (5395, 5398, 5407, 5384); 7667, 8758…10090 are the
wrapper with `push` inlined.

The start-up code reaches the wrapper through a by-value helper, `f(std::unique_ptr<T> p, info) { replace(move(p),
info); }` (func 4162 for `ITextToSpeech`, merged body 1562 for `main`'s calls). With libc++ ABI v2,
`std::unique_ptr` is `[[clang::trivial_abi]]`: a by-value `unique_ptr` is passed as the raw pointer and the
callee destroys it. 4879 and 4890 are that helper with `replace` and `push` inlined, which is why their first
parameter is the raw object pointer.

`erase(nome, info)` (func 640) takes a shared lock, finds the entry, **upgrades** it to exclusive
(func 13621), keeps a copy of the `shared_ptr` and erases the vector element. The old object is then
destroyed while the exclusive lock is still held.

The `GetInst()` pattern that shows up inlined in this unit is:

```
[static mutex lock_guard]              // 599, 3594, 5575, 5925, 7181 (unlock residue only)
if (!CPolySingleton<I>::exists())  push<I>(make_unique<CDefault>())      // 599, 3594, 5575, 5925, 7181
                                   replace<I>(make_unique<...>())        // 1956, 10876
                                   CreateInst<T>() (throws "Tentativa    // 1155
                                   de recriar o singleton", then replace)
return CPolySingleton<I>::instance(info, source_location::current());   // not in 10876 (returns void)
```

For `ISystemDateTime` (1155) and `ITimerScheduler` (1259/5444, not u19), `CreateInst<T>` throws
`CBaseError<api::EUeUtilError>` 7084/7085 "Tentativa de recriar o singleton" if the interface already exists,
so re-registration there is refused, while `replace` replaces and a bare `push` would throw 6756.

### 3.5 The lock

`lock_shared` waits while `writer || writersWaiting`. `lock` waits while `writer || readers`. `unlock`
wakes a waiting writer first, and otherwise all readers. In this build there are no pthreads, so
`std::condition_variable::wait` (func 251) **returns immediately**. Every "wait until" is therefore a busy loop
that never ends if the condition is false (§11).

## 4. Start-up, verified at run time

With a copy of `tools/run/headless.mjs` that sets `ENV.DEBUG_UENUX = "1"` in `preRun`, the trace lines stay
in the stdout TTY buffer, because the format has no `\n`. Dumping that buffer at the end of a municipal-t1 run
gives 31 lines:

```
: CPolySingletonList::push(N3api15ISystemDateTimeE[api::ISystemDateTime])[sz[0] ptr[0x1bf600]]
: CPolySingletonList::push(N5comum14IInterfaceSavdE[comum::IInterfaceSavd])[sz[1] ptr[0x1bf600]]
...
bf60: CPolySingletonList::push(N3api15IAjusteDataHoraE[api::IAjusteDataHora])[sz[5] ptr[0x1bf600]]
...
À: CPolySingletonList::push(N3api13ITextToSpeechE[api::ITextToSpeech])[sz[26] ptr[0x1bf600]]
: CPolySingletonList::push(N3api15ISystemDateTimeE[api::ISystemDateTime])[sz[26] ptr[0x1bf600]]   <- replace (size unchanged)
: CPolySingletonList::push(N3api12CEscritorLogE[api::CEscritorLog])[sz[27] ptr[0x1bf600]]
: CPolySingletonList::push(N3api6ISoundE[api::ISound])[sz[27] ptr[0x1bf600]]                      <- main lambda: CWasmWebSound
: CPolySingletonList::push(N3api13ITextToSpeechE[api::ITextToSpeech])[sz[27] ptr[0x1bf600]]       <- votaInit
```

The trace confirms these points:
* the registration order in §2.3;
* the replace semantics (`sz` stays the same after an erase);
* `ptr[...]` is the address of the registry (0x1BF600);
* the **garbage prefix** before `": "` (`bf60`, `À`, `ã`, `bf600§`): uninitialised stack bytes printed by
  `printf("%s: ", buf)` (§11).

## 5. Diagnostics: `log`, `trace` and the syslog/printf back end

* `log(func, type, detail)` (func 212) demangles `type` with `abi::__cxa_demangle` (`"..."` on failure). If
  `func` does not start with `"list"`, it appends a call-stack text. That text is always empty in this build.
  It then calls func 13156 with `"CPolySingletonList::%s(%s[%s])[%s] %s%s"`.
* Func 13156: `static int naUrna = access("/dev/urna", F_OK) == 0`. On a real urna the line goes to
  **syslog** (`LOG_INFO`). Elsewhere it goes to stdout, and only if `getenv("DEBUG_UENUX")`. MEMFS has no
  `/dev/urna` and the page sets no environment, so nothing is printed in the browser.
* `trace()` (func 14476, and its inlined copies in every `push`) calls `log` only if
  `GetPolySingletonsInfo().debug`: it calls the accessor again and ignores the `info` passed to `push`. The
  accessor sets `debug = true` on every call, so tracing is **always on**. Every registration pays for a
  `std::format` and a demangle, and funcs 212 and 13156 ran in the recorded sessions.

## 6. The foreign functions

### 6.1 Simulator start-up: func 8302 (`simulador::CSimuladorWasm::Executa`, names inferred)

`main` builds `{int 10, std::string "VOTA Web", short width, short height}` (ctor func 8311) and invokes
func 8302 (through its table slot 128) with the `main` lambda as a `std::function`. Func 8302 does the following, in order:

1. It sets the `comum::CPath` storage roots: root `"/"`, internal flash `/dsk/fi/`, external flash `/dsk/fe/`
   (statics @1838600/@1838576/@1838588).
2. It calls `create_directories("/uenux")`, `setenv("PROJECT_DIR", ".")` and `setenv("BUILD_DIR", "/")`.
3. It runs `ITimerScheduler::CreateInst<CWasmTimerScheduler>()` (itimerscheduler.h:40). It then registers, each
   through the register-or-replace wrapper of §3.4 (inlined, or 5384/4162/4890),
   `CWasmInit(10)`, `CWasmScreen(w, h)`, `CWasmScreenMT` (4 text lines, each `std::string(40, ' ')`), `CWasmInputKbd`,
   `CWasmInputMT`, `CWasmResource`, `CUrnaMock`, `CPowerMock`, `CWasmBeep`, `CWasmNullSound(9)`,
   `CWasmNullPaper`, `CWasmNullPrinter`, `CWasmNullTextToSpeech`, `CWasmSystemDateTime` and
   `CWasmLogd(logdir/"logd.dat")` as `CEscritorLog`.
   * `CWasmScreen(w, h)` scales by `sx = w/640`, `sy = h/480`, calls `js_init(w, h)` and `Clear(1)`, and logs
     `"CWasmScreen this=<address>"` through `js_log`.
   * The `CWasmBeep` constructor calls `js_wasm_beep_init()` (the object is allocated and its vptr stored first).
4. It calls the lambda. If the lambda is empty, it throws `bad_function_call`.

The mock values it installs:
* `CUrnaMock`: +4 = **2020** (IUrna slot 0, the model year: `CTesteTeclado`/`CPedeIdentidade` test
  `<= 2019` / `>= 2020`), +8 = 87654321, +12 = 255.
* `CPowerMock`: status `{0x840, 2, 0, 100}`. That decodes to mains power with a full internal battery, so the
  battery icon key is 0.

### 6.2 `GetInst()` functions with a lazy default

| func | function | default implementation | srcloc |
|---:|---|---|---|
| 599 | `vota::impl::IInformacaoThreadOperador::GetInst` | `CInformacaoThreadOperador` (96 B) | cinformacaothreadoperador.cpp:554 |
| 1155 | `api::ISystemDateTime::GetInst` | `CreateInst<CSystemDateTime>` (throws 7084 if re-created) | isystemdatetime.cpp:23, .h:46 |
| 1956 | `vota::(anon)::GetQRDSInst` | `IQRCodeBUDS(GeraPartesQRCodeBU())`, see §7 | cmostraqrcodebu.cpp:31/38 |
| 3594 | `vota::IExecucaoVota::GetInst` | `CExecucaoVota` (urna threads) | iexecucaovota.cpp:74 |
| 3876 | `api::CLp::GetInst` (static `unique_ptr`, not a poly-singleton) | `CLp()` looks up `IImpressoraRelatorios` | cwasmclp.cpp:10 |
| 5575 | `comum::impl::IValidaMidia::GetInst` | `CValidaMidia` | cvalidamidia.cpp:182 |
| 10876 | `api::IAjusteDataHora::CreateInst` (name inferred) | `CAjusteDataHora` (no re-creation check) | – |
| 5925 | inlined `IPoliticaExecucaoEleitor::GetInst` | `CPoliticaExecucaoEleitor` | cpoliticaexecucaoeleitor.cpp:45 |
| 7181 | inlined `ISincronismoVotoEleitor::GetInst` | `CSincronismoVotoEleitor` | csincronismovotoeleitor.cpp:67 |

In the web build, `main` registers the web versions of IExecucaoVota, IPoliticaExecucaoEleitor and
ISincronismoVotoEleitor first, so their urna defaults are never created.

### 6.3 Voter side

* **5925**, vtable slot 17 of `CConfirmaVotoNominal` and `CMajoritarioValido` (identical code folding). The
  base `CConfirmaVotoEmCargo` has a no-op there. `EmiteEcoCorrigeConfirma` calls it with the key it just
  handled. On CORRIGE (5) it logs "Eleitor corrigiu na tela de confirmação de candidato" and calls
  `IPoliticaExecucaoEleitor::LimpaBufferInput()`. That flushes the keypad, so keys typed on the confirmation
  screen are not taken as the next vote. The web policy flushes once, and the urna's flushes 3–6 times with
  random pauses (u06).
* **7181** `CSincronismoEleitor::ProcessMessage`: on message 5 it calls
  `ISincronismoVotoEleitor::GetInst().SincronizaVoto()`, and if that succeeds it goes to `CFimVotoEleitor` (the
  "FIM" screen). It ran in the recorded votes.

### 6.4 Operator (MT) side: none of it runs in the browser

These are `ProcessInput` bodies (slot 7) built on `CInteractiveForm<IScreenMT, IInputMT>::Read()`
(cinteractiveform.h:57). The key codes are 5 = CORRIGE and 9 = CONFIRMA.

| func | state | behaviour |
|---:|---|---|
| 10493, 10641 (→ 6015) | `CInformaAnoNascimentoErrado`, `CEleitorJaVotou` | CONFIRMA → `CCancelaHabilitacaoEleitor` (1536) |
| 10593 | `CJustificativaEfetuada` | CONFIRMA → `CCancelaHabilitacaoEleitor` |
| 10727 | `CTituloEncerramentoInvalido` | CORRIGE → state from vota_f3631 |
| 10477 | `CDigitalNaoReconhecidaDecBiometria` | CORRIGE → log + cancel habilitação; CONFIRMA → `CVerificaDadoEleitor` |
| 10523 | `CHabilitaAudioEleitor` | CONFIRMA: clear manual audio, post msg 8 (voter needs audio) or 9, then msg 1 ("habilitado") to the voter queue, go to `CMostraEleitorVotando`, and, only if the voter needs audio, log "Áudio ativado conforme cadastro". "Needs audio" (slot 5) is read before the key is tested |
| 10420 | `CPerguntaCodigoSuspensao` | Suspends the voter's session. The mesário types a título. It must be a valid título and different from the voter's typed identity. Valid: log "Título {} é válido para suspender a votação" and post msg 2. Invalid: log "… inválido …", show the error screen, **`emscripten_sleep(3000)`**, redraw. CORRIGE: log, post msg 4, back to `CMostraEleitorVotando` |
| 6011 | `comum::CMesarioRegistrado` / `CRegistrarMesarios` | mesário attendance: CORRIGE → controller slot 25 + "Finalizar registro de mesários?"; CONFIRMA → slot 24 + next state |
| 3623, 5415, 10584, 10670 | outlined `IInformacaoThreadOperador::GetInst().slotN()` | GetIdentidadeDigitada, SetAudioHabilitadoManualmente, GetTextoAudio data source |

### 6.5 Other

* **Battery icon** (2773, 2774, 10981, 10987, 10991, 2238). The icon key 0–9 maps to a pair of images
  (horizontal for the screen, vertical for the MT LCD). Key 10 means no icon:

  | key | state | image |
  |---:|---|---|
  | 0 / 1 / 2 | on mains, battery full / partial / critical | `img-ac-bateria-full`, `…-parcial`, `…-critical` (hor: `-critical-h.jpg`) |
  | 3 | no battery | `img-ac-sem-bateria` |
  | 4 / 5 / 6 | on battery: full / partial / critical | `img-bateria-full`, `…-parcial`, `…-critical` |
  | 7 / 8 / 9 | external battery: full / partial / critical | `img-bateria-ext-full`, `…-ext-parc(ial)`, `…-ext-crit(ical)` |

  `GetEstadoIcone` re-reads the status (`IPower` slot 15) before each bit test. It reads the source in bits
  1–2, the internal battery in bits 3–4 and the external battery in bits 5–6. A 1-second timer lambda
  compares the key with the previous one and notifies the observers.
* **11530** `ITextToSpeech` slot 0: the synthesis front end with the LRU cache (see u02 and rhvoice.md).
* **10224**: the `std::async` body in `CThreadMonitor::SaiPorVotacaoSuspensa`, which makes a beep (`IBeep`
  slot 4).
* **3876/5978/12016** `api::CLp`: the report printer front end of the mock (`cwasmclp.cpp`).

## 7. BU: the on-screen "BU digital" data source (func 1956, `GetQRDSInst`)

At the end of the day, `vota::CMostraQRCodeBU` (u09) shows the BU as QR codes on the urna screen, one part at
a time (keys 3/9). Its data comes from the poly-singleton `vota::IQRCodeBUDS`. That object is built **once**,
the first time `GetQRDSInst()` runs:

1. `CPolySingleton<IQRCodeBUDS>::exists()` (func 2886). If it is true, skip to step 9.
2. `eg = GetEstado<CEstadoGeral>()` (func 291, `eg.bin`).
3. `gap = GetEstado<CEstadoGeralGap>()` (inlined). If that state was not loaded, it throws
   `CUeComumAppInfoError 7600 "O estado não foi carregado: GetGap"` (cappinfo.cpp:42).
4. `comparecimento = CRdvVota::GetInst()` → max over the eleições of `CVotosEleicoesVota::Comparecimento`
   (shared_f1269).
5. `vota = GetEstado<CEstadoGeralVota>()` (func 261, `vota.bin`). `dhEmissao = vota.GetDtHrEmissaoBU()`
   (optional at +80). If it is empty, it throws `CUeComumDadosError 8093 "A data/hora da emissão do BU não foi
   registrada"` (cestadogeralvota.h:159). The date is recorded when the printed BU is generated, so the
   screen BU can only exist after it.
6. Header `CCabecalhoQRCode` (func 5624, 33 empty `TAG:value ` strings):
   * `ZONA` = eg+24 (func 5618)
   * `SECA` = eg+26 (5620)
   * `IDUE` = carga `numeroInternoUrna` eg+60 (5621)
   * `IDCA` = carga `codigoCarga` eg+88 (5623)
   * `HIQT`/`HICA` = the `codigoCarga` of every `gap.correspondencias` entry (96-byte entries, string at +28)
     (5622)
7. `CGeradorBUQRCodeVota(cabecalho, comparecimento, dhEmissao)` (func 5603). Then
   **`GeraQRCodes(2500)`** (func 5604). The printed BU (`CGeraBU`, func 12110) uses exactly the same header
   and generator with **1100**. The screen QR codes therefore carry slices of 2500 − 277 = 2223 payload characters
   (the 277-character reserve described in `docs/bu/qrcode.md` §4), so a BU needs fewer, denser codes on screen than
   on paper. Every code but the last holds at most 2500 characters; the last can reach about 2659 (1245 with Ed521 or
   1259 with ECDSA on paper), because in format 6.0 its fixed text (header, ` HASH:`, ` ASSI:`) takes 422–436
   characters, more than the 277 reserved (2026 urna data: `investigation/README.md`, finding H5). The result is
   `{vector<string> conteudos, string assinatura}`. The signature string is dropped, because the `ASSI:`
   field is already inside the last payload.
8. `IQRCodeBUDS(partes)` (vtable @1542092): `{vector<string> m_qrcodes (+4), size_t m_indice = 0 (+16)}`. It
   throws `CUeVotaError 9372 "Vetor de partes vazio"` (cmostraqrcodebu.cpp:38) if the vector is empty. Then
   `replace<IQRCodeBUDS>` (§3.4, inlined: a second `exists` + `erase`, then the `push` body). The parts are
   generated before the registry accessor is called.
9. `return CPolySingleton<IQRCodeBUDS>::instance(info, loc = cmostraqrcodebu.cpp:31)`.

The screen binds two data-source functions from this unit into `std::function<std::string()>` fields:
* `QRCodeAtual()` (func 12053, table slot 1506) returns `m_qrcodes.at(m_indice)`. That string is the QR
  payload shown by `CQRCodeImage`.
* `TextoInstrucaoQRCode()` (func 12054, slot 1505) returns "O QR code ao lado contém o resultado da votação
  para esta urna." when there is 1 part. With several parts it adds " Use as teclas 3 e 9 para navegar pelas
  partes do BU.".

The payload format, hash chain and `ASSI` signature are in `docs/bu/qrcode.md`. Nothing here ran in the
simulator, because the web build never reaches the encerramento (`docs/bu/codepath.md` §1.3). The screen
payloads are generated independently of the printed ones. They are signed by the same generator with the
same header, but the slices, and so the hash chain and the signature, differ from the paper QR codes of the
same BU.

## 8. Web-build specifics

* The platform services are all mocks registered by func 8302 and `main`, and some urna defaults are
  replaced (`ISystemDateTime`, `ISound`, `ITextToSpeech`) (§2.3).
* `CUrnaMock` gives the model year 2020 and the fixed serial 87654321. `CPowerMock` reports mains with a full
  battery.
* Tracing is compiled in and always "enabled", but invisible: there is no `/dev/urna` and no
  `DEBUG_UENUX`.
* The operator thread never runs, so §6.4 and the BU screen (§7) are dead code in the browser.

## 9. wasm / Emscripten observations

* **Naming trap.** 30 functions carry the name `CPolySingletonList::instance@N`. They are really
  `CPolySingleton<T>::instance` (signature `(info, loc)`) or a whole `GetInst()` (signature `()`). Func
  8302 was shown as "push" but is the simulator start-up (§6.1); the tools now show the inferred name
  `simulador::CSimuladorWasm::Executa`.
* **merge-similar-functions** merged bodies by the byte length of the inlined type name (exists: 6131, 6132,
  6057, 2924; instance: 6013), by constant vtables (`shared_ptr` from `unique_ptr`: 1010), and by constant
  table slots (`emplace_back`: 1009; `find`: 6161; lock guards: 6147; exception guards: 3895).
* **Two registration functions, not a split one.** The "heads" 8728, wasm_entry_f9437, wasm_entry_f9507 and
  libcxx_f10356 are the register-or-replace wrapper (`if (exists) erase`), and the "tails" 5395/5398/5407 (→ merged
  body 3882) and 5384 are `push` itself, left out of line. The `GetInst()` functions call `push` directly, which is
  why they have no second `exists` (§3.4). The compiler did not remove anything: `exists` locks and calls into JS,
  and 1956/10876 keep the second `exists` call after the same `if (!exists)` test.
* **Two exception models.** Only 26 of the 184 functions use `invoke_*` (for example the `instance` of
  IInputKbd/ISound, the `push`/`replace` bodies called from `main`/`votaInit`, `erase`, the lock primitives,
  `ITextToSpeech::GetAudio`). The copies inlined into most uenux2 application code have **no landing pads**.
  If they throw while holding the registry lock, they never release it. This matches the u11/rhvoice finding
  that most uenux2 translation units are compiled without exception catching. It is not true of all of them:
  the out-of-line `instance` copies 455/1091, which vota:: states call, and `GetAudio` (11530) do have landing
  pads.
* **The accessor is a function pointer** (@1526320) and not a direct call. This costs one `call_indirect`
  per lookup, and every `instance` calls it twice.
* **Dead-store static initialisers.** 24 existence checks in `__wasm_call_ctors` (21 distinct `exists<T>`
  functions) have their results discarded.
* The static registry's destructor (10620) is table slot 2. The tools mislabelled it as the vf0 of
  `IObservableProgressWithDescription`. It never runs, because of `noExitRuntime`.

## 10. Complete mapping table (184 functions)

"ran" = observed executing in `analysis/runtime/*.functions.tsv`. "Reconstructed in" is relative to the
project root. Library/inlined helpers are listed with the reason.

| func | size | ran | tools name | reconstructed symbol | reconstructed in | original file | conf. |
|---:|---:|:-:|---|---|---|---|---|
| 212 | 725 | ✓ | `api_f212` | `api::CPolySingletonList::log(funcao, tipo, detalhe)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 235 | 21 |  | `api_f235` | `api::CUePatternError::CUePatternError(code, msg, loc) (thunk)` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | high |
| 305 | 14 | ✓ | `api_f305` | `api::CPolySingletonList::find (thunk, lambda slot 156)` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 321 | 32 |  | `api_f321` | `std::__format::__allocating_buffer<char>::~__allocating_buffer` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ std::format buffer destructor | medium |
| 327 | 53 | ✓ | `api_f327` | `std::to_string(unsigned)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ to_string for loc.line() | high |
| 356 | 1808 |  | `api::CPolySingletonList::instance@356` | `api::CPolySingleton<comum::IControladorRegistraMesarios>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | high |
| 383 | 1772 |  | `api::CPolySingletonList::instance@383` | `api::CPolySingleton<api::IInputMT>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | high |
| 388 | 55 | ✓ | `api_f388` | `std::shared_ptr<T>::~shared_ptr()` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ shared_ptr destructor, shared by many types (ICF) | high |
| 429 | 135 |  | `api_f429` | `std::make_format_args("instance", const std::string&, const std::string&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ std::format argument packing | medium |
| 430 | 93 |  | `api_f430` | `std::make_format_args("instance", const std::string&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ std::format argument packing | medium |
| 455 | 2898 | ✓ | `api::CPolySingletonList::instance@455` | `api::CPolySingleton<api::IInputKbd>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | high |
| 512 | 1772 | ✓ | `api::CPolySingletonList::instance@512` | `api::CPolySingleton<api::IScreen>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | high |
| 599 | 4114 |  | `api::CPolySingletonList::instance@599` | `vota::impl::IInformacaoThreadOperador::GetInst()` | src/uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.u19.cpp | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | high |
| 608 | 14 | ✓ | `api_f608` | `api::CPolySingletonList::find (thunk, lambda slot 162)` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 611 | 1846 | ✓ | `api::CPolySingletonList::instance@611` | `api::CPolySingleton<comum::IInterfaceInit>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | high |
| 631 | 536 | ✓ | `api_f631` | `std::vector<api::SPolySingleton>::__emplace_back_slow_path(const char*&, const type_info*, const shared_ptr&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ vector growth for the registry | medium |
| 640 | 327 | ✓ | `api_f640` | `api::CPolySingletonList::erase(const std::string&, TPolySingletonsInfo&)` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 642 | 10 |  | `api_f642` | `std::unique_lock<api::CUpgradeMutex>::~unique_lock()` | src/uenux2/src/api/pattern/cpolysingletonlist.h | library/inlined helper: libc++ lock guard destructor for the registry lock | medium |
| 816 | 36 |  | `api::CPolySingletonList::instance@816` | `api::CPolySingleton<api::IFingerScanner>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | high |
| 837 | 1784 | ✓ | `api::CPolySingletonList::instance@837` | `api::CPolySingleton<comum::IEventosLog>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | high |
| 862 | 1772 | ✓ | `api::CPolySingletonList::instance@862` | `api::CPolySingleton<api::IPower>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | high |
| 905 | 1784 |  | `api::CPolySingletonList::instance@905` | `api::CPolySingleton<api::IPaperRelatorios>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | high |
| 923 | 1772 | ✓ | `api::CPolySingletonList::instance@923` | `api::CPolySingleton<api::IUrna>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | high |
| 925 | 1772 | ✓ | `api::CPolySingletonList::instance@925` | `api::CPolySingleton<api::IBeep>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | high |
| 1010 | 63 |  | `api_f1010` | `std::shared_ptr<T>::shared_ptr(std::unique_ptr<T>&&) (merged body)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ shared_ptr from unique_ptr (merged body) | medium |
| 1091 | 2898 | ✓ | `api::CPolySingletonList::instance@1091` | `api::CPolySingleton<api::ISound>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | high |
| 1113 | 227 |  | `api_f1113` | `std::make_format_args(size_t, const void*)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ std::format argument packing | medium |
| 1119 | 395 |  | `api_f1119` | `std::make_format_args("push", const char*)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ std::format argument packing | medium |
| 1126 | 141 | ✓ | `api_f1126` | `api::CUpgradeMutex::lock()` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 1155 | 1944 | ✓ | `api::CPolySingletonList::instance@1155` | `api::ISystemDateTime::GetInst()` | src/uenux2/src/api/util/isystemdatetime.u19.cpp | uenux2/src/api/util/isystemdatetime.cpp | high |
| 1166 | 198 | ✓ | `api_f1166` | `api::CPolySingletonList::exists<T> generic body (string key, out-of-line lock_shared/find)` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 1282 | 1784 |  | `api::CPolySingletonList::instance@1282` | `api::CPolySingleton<comum::CCalculaCV>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | high |
| 1322 | 10 | ✓ | `api_f1322` | `std::shared_lock<api::CUpgradeMutex>::~shared_lock()` | src/uenux2/src/api/pattern/cpolysingletonlist.h | library/inlined helper: libc++ lock guard destructor for the registry lock | medium |
| 1536 | 22 |  | `api_f1536` | `vota::CCancelaHabilitacaoEleitor::GetInst()` | src/uenux2/src/app/vota/operador/u19-foreign-fragments.cpp | uenux2/src/app/vota/operador/comum/ccancelahabilitacaoeleitor.cpp (path inferred) | medium |
| 1549 | 200 |  | `api_f1549` | `std::__format::__create_packed_storage lambda for const std::string&` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ std::format argument packing (shared with other callers) | medium |
| 1654 | 19 | ✓ | `api_f1654` | `api::CPolySingletonList::exists<api::ITimerScheduler>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 1686 | 36 |  | `api::CPolySingletonList::instance@1686` | `api::CPolySingleton<api::IFingerMatcher>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | high |
| 1714 | 1772 |  | `api::CPolySingletonList::instance@1714` | `api::CPolySingleton<api::IScreenMT>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | high |
| 1822 | 1784 | ✓ | `api::CPolySingletonList::instance@1822` | `api::CPolySingleton<comum::IInterfaceSavd>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | high |
| 1956 | 5663 |  | `api::CPolySingletonList::instance@1956` | `vota::(anonymous namespace)::GetQRDSInst()` | src/uenux2/src/app/vota/eleitor/fimvotacao/cmostraqrcodebu.u19.cpp | uenux2/src/app/vota/eleitor/fimvotacao/cmostraqrcodebu.cpp | high |
| 2030 | 1796 |  | `api::CPolySingletonList::instance@2030` | `api::CPolySingleton<ecourna::api::security::IRng>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | high |
| 2079 | 1784 |  | `api::CPolySingletonList::instance@2079` | `api::CPolySingleton<api::ITextToSpeech>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | high |
| 2160 | 19 | ✓ | `api_f2160` | `api::CPolySingletonList::exists<api::ISystemDateTime>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 2238 | 152 |  | `api_f2238` | `std::__tree<pair<int, vector<shared_ptr<api::IImage>>>>::destroy(node)` | src/uenux2/src/api/gui/cpowerinformation.u19.cpp | library/inlined helper (for CPowerInformation::ms_icones) | medium |
| 2279 | 2126 |  | `api::CPolySingletonList::instance@2279` | `api::CPolySingleton<api::IKernelHSM>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | high |
| 2446 | 381 |  | `api_f2446` | `api::CPolySingletonList::exists<comum::IControladorRegistraMesarios>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 2469 | 19 | ✓ | `api_f2469` | `api::CPolySingletonList::exists<api::IAjusteDataHora>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 2509 | 357 | ✓ | `api_f2509` | `api::CPolySingletonList::exists<comum::IEventosLog>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 2584 | 16 |  | `ecourna_f2584` | `api::SPolySingleton::~SPolySingleton()` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 2736 | 12 | ✓ | `api_f2736` | `api::CPolySingletonList::exists<vota::IExecucaoVota>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 2773 | 5642 | ✓ | `api_f2773` | `api::CPowerInformation::CarregaIcones()` | src/uenux2/src/api/gui/cpowerinformation.u19.cpp | uenux2/src/api/gui/cpowerinformation.cpp | medium |
| 2774 | 302 | ✓ | `api_f2774` | `api::CPowerInformation::GetEstadoIcone(api::IPower&)` | src/uenux2/src/api/gui/cpowerinformation.u19.cpp | uenux2/src/api/gui/cpowerinformation.cpp | medium |
| 2886 | 19 | ✓ | `api_f2886` | `api::CPolySingletonList::exists<vota::IQRCodeBUDS>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 2924 | 351 | ✓ | `api_f2924` | `api::CPolySingletonList::exists<T> merged body, 23-char type names` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 3075 | 1772 | ✓ | `api::CPolySingletonList::instance@3075` | `api::CPolySingleton<api::IResource>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | high |
| 3095 | 119 | ✓ | `api_f3095` | `api::CUpgradeMutex::lock_shared()` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 3164 | 1808 | ✓ | `api::CPolySingletonList::instance@3164` | `api::CPolySingleton<api::IGenericFactory<api::ISyncCtl>>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | high |
| 3204 | 1784 |  | `api::CPolySingletonList::instance@3204` | `api::CPolySingleton<api::IAjusteDataHora>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | high |
| 3281 | 11 | ✓ | `api_f3281` | `api::CPolySingletonList::exists<api::ITextToSpeech>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 3343 | 11 | ✓ | `api_f3343` | `api::CPolySingletonList::exists<api::ISound>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 3386 | 15 |  | `api_f3386` | `api::CPolySingletonList::exists<api::IBeep>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 3387 | 345 | ✓ | `api_f3387` | `api::CPolySingletonList::exists<api::IScreen>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 3391 | 12 |  | `api_f3391` | `api::CPolySingletonList::exists<api::CEscritorLog>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 3392 | 12 |  | `api_f3392` | `api::CPolySingletonList::exists<api::IImpressoraRelatorios>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 3394 | 12 | ✓ | `api_f3394` | `api::CPolySingletonList::exists<comum::IInterfaceInit>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 3594 | 1923 | ✓ | `api::CPolySingletonList::instance@3594` | `vota::IExecucaoVota::GetInst()` | src/uenux2/src/app/vota/iexecucaovota.u19.cpp | uenux2/src/app/vota/iexecucaovota.cpp | high |
| 3615 | 2126 |  | `api::CPolySingletonList::instance@3615` | `api::CPolySingleton<api::IFingerDetection>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | high |
| 3618 | 11 | ✓ | `api_f3618` | `api::CPolySingletonList::exists<api::IGenericFactory<api::ISemaphore>>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 3622 | 381 |  | `api_f3622` | `api::CPolySingletonList::exists<vota::impl::IInformacaoThreadOperador>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 3623 | 20 |  | `api_f3623` | `IInformacaoThreadOperador::GetInst().GetIdentidadeDigitada() (outlined)` | src/uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.u19.cpp | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.h (inline call, outlined) | medium |
| 3627 | 11 | ✓ | `api_f3627` | `api::CPolySingletonList::exists<api::IGenericFactory<api::ISyncCtl>>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 3668 | 803 |  | `api_f3668` | `std::vector<std::shared_ptr<T>>::__assign_with_size(first, last, n)` | src/uenux2/src/api/gui/cpowerinformation.u19.cpp | library/inlined helper (ICF with RHVoice rhvoice_f1245) | medium |
| 3686 | 369 |  | `api_f3686` | `api::CPolySingletonList::exists<comum::impl::IValidaMidia>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 3704 | 2126 |  | `api::CPolySingletonList::instance@3704` | `api::CPolySingleton<api::pkcs11::IPkcs11>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | high |
| 3876 | 2002 |  | `api::CPolySingletonList::instance@3876` | `api::CLp::GetInst()` | src/uenux2/mock/app/simulador/wasm/cwasmclp.u19.cpp | uenux2/mock/app/simulador/wasm/cwasmclp.cpp | medium |
| 3882 | 2072 | ✓ | `api_f3882` | `api::CPolySingletonList::push<T>` merged body (IExecucaoVota/ISincronismoVotoEleitor/IPoliticaExecucaoEleitor) | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | high |
| 3895 | 62 |  | `api_f3895` | `std::__exception_guard_exceptions<Rollback>::~__exception_guard_exceptions (merged body)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ exception guard (merged body) | medium |
| 3928 | 413 |  | `api_f3928` | `api::CPolySingletonList::exists<T> body taking &typeid(T).__type_name` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 4162 | 143 | ✓ | `api_f4162` | by-value helper `f(std::unique_ptr<api::ITextToSpeech>, info)` → `replace<api::ITextToSpeech>` 7667 | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 4180 | 84 |  | `api_f4180` | `std::shared_ptr<T>::operator=(std::shared_ptr<T>&&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ shared_ptr move-assign (ICF) | medium |
| 4356 | 1820 |  | `api::CPolySingletonList::instance@4356` | `api::CPolySingleton<api::IGenericFactory<api::ISemaphore>>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | high |
| 4852 | 42 |  | `ecourna_f4852` | `std::vector<api::SPolySingleton>::__base_destruct_at_end` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ vector destroy-at-end | medium |
| 4853 | 357 |  | `api_f4853` | `api::CPolySingletonList::exists<api::IPaperRelatorios>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 4867 | 345 | ✓ | `api_f4867` | `api::CPolySingletonList::exists<api::IPower>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 4871 | 15 | ✓ | `api_f4871` | `api::CPolySingletonList::exists<api::IUrna>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 4872 | 15 | ✓ | `api_f4872` | `api::CPolySingletonList::exists<api::IResource>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 4873 | 345 |  | `api_f4873` | `api::CPolySingletonList::exists<api::IInputMT>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 4877 | 15 |  | `api_f4877` | `api::CPolySingletonList::exists<api::IScreenMT>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 4879 | 2097 | ✓ | `api::CPolySingletonList::push@4879` | by-value helper → `replace<api::ITimerScheduler>` → `push` (all inlined) | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 4890 | 2097 | ✓ | `api::CPolySingletonList::push@4890` | by-value helper → `replace<api::ISystemDateTime>` → `push` (all inlined) | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 5310 | 10 |  | `api_f5310` | `std::__exception_guard_exceptions<...>::~ (rollback slot 473 = comum_f9873)` | - | library/inlined helper (unrelated vector of comum; caller unknown_f9896) | low |
| 5361 | 2248 |  | `api::CPolySingletonList::instance@5361` | `api::CPolySingleton<api::IGenericFactory<ecourna::api::security::ITextEncoding>>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | high |
| 5362 | 2222 |  | `api::CPolySingletonList::instance@5362` | `api::CPolySingleton<api::IGenericFactory<ecourna::api::security::IHash>>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | high |
| 5384 | 2079 | ✓ | `api::CPolySingletonList::push@5384` | `api::CPolySingletonList::push<api::ISound>` (no exists/erase prefix) | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | high |
| 5395 | 30 | ✓ | `api::CPolySingletonList::push@5395` | `api::CPolySingletonList::push<vota::IExecucaoVota>` (thunk of 3882) | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | high |
| 5398 | 30 | ✓ | `api::CPolySingletonList::push@5398` | `api::CPolySingletonList::push<vota::impl::ISincronismoVotoEleitor>` (thunk of 3882) | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | high |
| 5407 | 30 | ✓ | `api::CPolySingletonList::push@5407` | `api::CPolySingletonList::push<vota::impl::IPoliticaExecucaoEleitor>` (thunk of 3882) | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | high |
| 5415 | 22 |  | `api_f5415` | `IInformacaoThreadOperador::GetInst().SetAudioHabilitadoManualmente(bool) (outlined)` | src/uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.u19.cpp | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.h (inline call, outlined) | medium |
| 5417 | 11 |  | `api_f5417` | `api::CPolySingletonList::exists<api::IGenericFactory<api::IRWSyncCtl>>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 5423 | 11 |  | `api_f5423` | `api::CPolySingletonList::exists<api::IGenericFactory<api::IThreadImpl>>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 5425 | 11 | ✓ | `api_f5425` | `api::CPolySingletonList::exists<api::IFingerPrepare>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 5427 | 11 | ✓ | `api_f5427` | `api::CPolySingletonList::exists<ecourna::api::security::ISymmetricCipherFactory>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 5431 | 11 |  | `api_f5431` | `api::CPolySingletonList::exists<ecourna::api::security::IRng>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 5434 | 11 | ✓ | `api_f5434` | `api::CPolySingletonList::exists<comum::IInterfaceSavd>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 5436 | 11 | ✓ | `api_f5436` | `api::CPolySingletonList::exists<api::IInputKbd>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 5443 | 1784 | ✓ | `api::CPolySingletonList::instance@5443` | `api::CPolySingleton<api::ITimerScheduler>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | high |
| 5562 | 10 |  | `api_f5562` | `std::__exception_guard_exceptions<...>::~ (rollback slot 235 = libcxx_f11146)` | - | library/inlined helper (unrelated vector of strings; caller unknown_f11158) | low |
| 5575 | 3863 |  | `api::CPolySingletonList::instance@5575` | `comum::impl::IValidaMidia::GetInst()` | src/uenux2/src/app/comum/validamidia/cvalidamidia.u19.cpp | uenux2/src/app/comum/validamidia/cvalidamidia.cpp | high |
| 5646 | 178 |  | `api_f5646` | `std::__format::__create_packed_storage lambda for const char(&)[9]` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ std::format argument packing | medium |
| 5650 | 168 |  | `api_f5650` | `std::make_format_args("instance", const std::string&, const std::string&) (non-inlined packing)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ std::format argument packing | medium |
| 5656 | 132 |  | `api_f5656` | `std::make_format_args("instance", const std::string&) (non-inlined packing)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ std::format argument packing | medium |
| 5774 | 10 |  | `api_f5774` | `std::__exception_guard_exceptions<_AllocatorDestroyRangeReverse<...SPolySingleton>>::~__exception_guard_exceptions` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ exception guard | medium |
| 5779 | 68 |  | `api_f5779` | `std::__split_buffer<api::SPolySingleton>::~__split_buffer` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ split_buffer destructor | medium |
| 5786 | 382 | ✓ | `api_f5786` | `std::vector<api::SPolySingleton>::__swap_out_circular_buffer` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ vector relocation | medium |
| 5925 | 2141 |  | `vota::CConfirmaVotoNominal::vf17` | `vota::CConfirmaVotoNominal::PosTecla(int) [= CMajoritarioValido slot 17]` | src/uenux2/src/app/vota/eleitor/u19-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/votaproporcional/cconfirmavotonominal.cpp (path inferred) | medium |
| 5978 | 97 |  | `api_f5978` | `api::CLp::~CLp()` | src/uenux2/mock/app/simulador/wasm/cwasmclp.u19.cpp | uenux2/mock/app/simulador/wasm/cwasmclp.cpp | low |
| 6011 | 227 |  | `comum_f6011` | `ProcessInput merged body (comum::CMesarioRegistrado / comum::CRegistrarMesarios)` | src/uenux2/src/app/comum/comparecimentomesario/u19-foreign-fragments.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cmesarioregistrado.cpp + cregistrarmesarios.cpp | medium |
| 6013 | 2101 |  | `api_f6013` | `api::CPolySingleton<T>::instance (merged body for IFingerScanner/IFingerMatcher)` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | high |
| 6015 | 127 |  | `api_f6015` | `ProcessInput merged body (CInformaAnoNascimentoErrado / CEleitorJaVotou)` | src/uenux2/src/app/vota/operador/u19-foreign-fragments.cpp | uenux2/src/app/vota/operador/confirmaidentidade/cinformaanonascimentoerrado.cpp + celeitorjavotou.cpp (path inferred) | medium |
| 6057 | 351 | ✓ | `api_f6057` | `api::CPolySingletonList::exists<T> merged body, 20-char type names` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 6131 | 341 | ✓ | `api_f6131` | `api::CPolySingletonList::exists<T> merged body, 12-char type names` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 6132 | 341 | ✓ | `api_f6132` | `api::CPolySingletonList::exists<T> merged body, 16-char type names` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 6147 | 71 | ✓ | `api_f6147` | `lock-guard destructor merged body: if (owns) invoke(unlock slot, m)` | src/uenux2/src/api/pattern/cpolysingletonlist.h | library/inlined helper: merged body of the two lock-guard destructors | medium |
| 6161 | 138 | ✓ | `api_f6161` | `api::CPolySingletonList::find (merged body)` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 7181 | 2072 | ✓ | `vota::CSincronismoEleitor::vf6` | `vota::CSincronismoEleitor::ProcessMessage(short)` | src/uenux2/src/app/vota/eleitor/u19-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/csincronismoeleitor.cpp (path inferred) | high |
| 7667 | 2186 | ✓ | `api::CPolySingletonList::push@7667` | `api::CPolySingletonList::replace<api::ITextToSpeech>` (push inlined) | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 8302 | 28435 | ✓ | `simulador::CSimuladorWasm::Executa` (formerly shown by the tools as `api::CPolySingletonList::push@8302`) | `simulador::CSimuladorWasm::Executa(const std::function<void()>&)` | src/uenux2/mock/app/simulador/wasm/csimuladorwasm.u19.cpp | uenux2/mock/app/simulador/wasm/csimuladorwasm.cpp (path inferred) | low |
| 8728 | 118 | ✓ | `api_f8728` | `api::CPolySingletonList::replace<vota::IExecucaoVota>` (exists/erase, then push 5395) | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 8758 | 2186 | ✓ | `api::CPolySingletonList::push@8758` | `api::CPolySingletonList::replace<api::IGenericFactory<api::ISemaphore>>` (push inlined) | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 8834 | 2186 | ✓ | `api::CPolySingletonList::push@8834` | `api::CPolySingletonList::replace<api::IGenericFactory<api::IRWSyncCtl>>` (push inlined) | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 8911 | 2186 | ✓ | `api::CPolySingletonList::push@8911` | `api::CPolySingletonList::replace<api::IGenericFactory<api::ISyncCtl>>` (push inlined) | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 8986 | 2186 | ✓ | `api::CPolySingletonList::push@8986` | `api::CPolySingletonList::replace<api::IGenericFactory<api::IThreadImpl>>` (push inlined) | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 9039 | 2186 | ✓ | `api::CPolySingletonList::push@9039` | `api::CPolySingletonList::replace<api::IFingerPrepare>` (push inlined) | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 9135 | 2186 | ✓ | `api::CPolySingletonList::push@9135` | `api::CPolySingletonList::replace<ecourna::api::security::ISymmetricCipherFactory>` (push inlined) | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 9233 | 2186 | ✓ | `api::CPolySingletonList::push@9233` | `api::CPolySingletonList::replace<ecourna::api::security::IRng>` (push inlined) | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 10090 | 2186 | ✓ | `api::CPolySingletonList::push@10090` | `api::CPolySingletonList::replace<comum::IInterfaceSavd>` (push inlined) | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 10224 | 67 |  | `std::__async_assoc_state<void, std::__async_func<vota::CThreadMonitor::SaiPorVotacaoSuspensa()::$_0>>::vf3` | `std::__async_assoc_state<void, __async_func<CThreadMonitor::SaiPorVotacaoSuspensa()::$_0>>::__execute()` | src/uenux2/src/app/vota/monitor/cthreadmonitor.u19.cpp | uenux2/src/app/vota/monitor/cthreadmonitor.cpp | high |
| 10325 | 16 |  | `api_f10325` | `std::vector<api::SPolySingleton>::emplace_back(const char*&, &typeid(api::ISound), shared_ptr<api::ISound>&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ shared_ptr from unique_ptr (thunk of 1010) | medium |
| 10335 | 14 |  | `api_f10335` | `std::shared_ptr<api::ISound>::shared_ptr(std::unique_ptr<api::ISound>&&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ shared_ptr from unique_ptr (thunk of 1010) | medium |
| 10420 | 1917 |  | `vota::CPerguntaCodigoSuspensao::vf7` | `vota::CPerguntaCodigoSuspensao::ProcessInput()` | src/uenux2/src/app/vota/operador/u19-foreign-fragments.cpp | uenux2/src/app/vota/operador/aguardaeleitor/cperguntacodigosuspensao.cpp (path inferred) | high |
| 10432 | 14 |  | `api_f10432` | `std::shared_ptr<vota::IExecucaoVota>::shared_ptr(std::unique_ptr<vota::IExecucaoVota>&&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ shared_ptr from unique_ptr (thunk of 1010) | medium |
| 10470 | 14 |  | `api_f10470` | `std::shared_ptr<vota::impl::ISincronismoVotoEleitor>::shared_ptr(std::unique_ptr<vota::impl::ISincronismoVotoEleitor>&&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ shared_ptr from unique_ptr (thunk of 1010) | medium |
| 10477 | 158 |  | `vota::CDigitalNaoReconhecidaDecBiometria::vf7` | `vota::CDigitalNaoReconhecidaDecBiometria::ProcessInput()` | src/uenux2/src/app/vota/operador/u19-foreign-fragments.cpp | uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaoreconhecidadecbiometria.cpp (path inferred) | high |
| 10493 | 12 |  | `vota::CInformaAnoNascimentoErrado::vf7` | `vota::CInformaAnoNascimentoErrado::ProcessInput()` | src/uenux2/src/app/vota/operador/u19-foreign-fragments.cpp | uenux2/src/app/vota/operador/confirmaidentidade/cinformaanonascimentoerrado.cpp (path inferred) | high |
| 10513 | 14 |  | `api_f10513` | `std::shared_ptr<vota::impl::IPoliticaExecucaoEleitor>::shared_ptr(std::unique_ptr<vota::impl::IPoliticaExecucaoEleitor>&&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ shared_ptr from unique_ptr (thunk of 1010) | medium |
| 10523 | 400 |  | `vota::CHabilitaAudioEleitor::vf7` | `vota::CHabilitaAudioEleitor::ProcessInput()` | src/uenux2/src/app/vota/operador/u19-foreign-fragments.cpp | uenux2/src/app/vota/operador/confirmaidentidade/chabilitaaudioeleitor.cpp (path inferred) | high |
| 10547 | 16 |  | `api_f10547` | `std::vector<api::SPolySingleton>::emplace_back(const char*&, &typeid(api::IGenericFactory<api::ISemaphore>), shared_ptr<api::IGenericFactory<api::ISemaphore>>&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ shared_ptr from unique_ptr (thunk of 1010) | medium |
| 10555 | 14 |  | `api_f10555` | `std::shared_ptr<api::IGenericFactory<api::ISemaphore>>::shared_ptr(std::unique_ptr<api::IGenericFactory<api::ISemaphore>>&&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ shared_ptr from unique_ptr (thunk of 1010) | medium |
| 10582 | 16 |  | `api_f10582` | `std::vector<api::SPolySingleton>::emplace_back(const char*&, &typeid(api::IGenericFactory<api::IRWSyncCtl>), shared_ptr<api::IGenericFactory<api::IRWSyncCtl>>&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ shared_ptr from unique_ptr (thunk of 1010) | medium |
| 10584 | 20 |  | `api_f10584` | `vota::DS_TextoAudio() -> IInformacaoThreadOperador slot 8 GetTextoAudio` | src/uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.u19.cpp | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp (path inferred) | medium |
| 10585 | 14 |  | `api_f10585` | `std::shared_ptr<api::IGenericFactory<api::IRWSyncCtl>>::shared_ptr(std::unique_ptr<api::IGenericFactory<api::IRWSyncCtl>>&&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ shared_ptr from unique_ptr (thunk of 1010) | medium |
| 10593 | 130 |  | `vota::CJustificativaEfetuada::vf7` | `vota::CJustificativaEfetuada::ProcessInput()` | src/uenux2/src/app/vota/operador/u19-foreign-fragments.cpp | uenux2/src/app/vota/operador/justificativa/cjustificativaefetuada.cpp (path inferred) | high |
| 10607 | 16 |  | `api_f10607` | `std::vector<api::SPolySingleton>::emplace_back(const char*&, &typeid(api::IGenericFactory<api::ISyncCtl>), shared_ptr<api::IGenericFactory<api::ISyncCtl>>&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ shared_ptr from unique_ptr (thunk of 1010) | medium |
| 10618 | 14 |  | `api_f10618` | `std::shared_ptr<api::IGenericFactory<api::ISyncCtl>>::shared_ptr(std::unique_ptr<api::IGenericFactory<api::ISyncCtl>>&&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ shared_ptr from unique_ptr (thunk of 1010) | medium |
| 10620 | 96 |  | `ecourna::api::pattern::IObservableProgressWithDescription::vf0@10620` | `api::TPolySingletonsInfo::~TPolySingletonsInfo() (static @1832448)` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 10641 | 12 |  | `vota::CEleitorJaVotou::vf7` | `vota::CEleitorJaVotou::ProcessInput()` | src/uenux2/src/app/vota/operador/u19-foreign-fragments.cpp | uenux2/src/app/vota/operador/confirmaidentidade/celeitorjavotou.cpp (path inferred) | high |
| 10643 | 16 |  | `api_f10643` | `std::vector<api::SPolySingleton>::emplace_back(const char*&, &typeid(api::IGenericFactory<api::IThreadImpl>), shared_ptr<api::IGenericFactory<api::IThreadImpl>>&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ shared_ptr from unique_ptr (thunk of 1010) | medium |
| 10652 | 14 |  | `api_f10652` | `std::shared_ptr<api::IGenericFactory<api::IThreadImpl>>::shared_ptr(std::unique_ptr<api::IGenericFactory<api::IThreadImpl>>&&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ shared_ptr from unique_ptr (thunk of 1010) | medium |
| 10670 | 7 |  | `api_f10670` | `vota::DS_IdentidadeDigitada() -> 3623` | src/uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.u19.cpp | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp (path inferred) | low |
| 10679 | 16 |  | `api_f10679` | `std::vector<api::SPolySingleton>::emplace_back(const char*&, &typeid(api::IFingerPrepare), shared_ptr<api::IFingerPrepare>&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ shared_ptr from unique_ptr (thunk of 1010) | medium |
| 10687 | 14 |  | `api_f10687` | `std::shared_ptr<api::IFingerPrepare>::shared_ptr(std::unique_ptr<api::IFingerPrepare>&&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ shared_ptr from unique_ptr (thunk of 1010) | medium |
| 10714 | 16 |  | `api_f10714` | `std::vector<api::SPolySingleton>::emplace_back(const char*&, &typeid(ecourna::api::security::ISymmetricCipherFactory), shared_ptr<ecourna::api::security::ISymmetricCipherFactory>&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ shared_ptr from unique_ptr (thunk of 1010) | medium |
| 10723 | 14 |  | `api_f10723` | `std::shared_ptr<ecourna::api::security::ISymmetricCipherFactory>::shared_ptr(std::unique_ptr<ecourna::api::security::ISymmetricCipherFactory>&&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ shared_ptr from unique_ptr (thunk of 1010) | medium |
| 10727 | 130 |  | `vota::CTituloEncerramentoInvalido::vf7` | `vota::CTituloEncerramentoInvalido::ProcessInput()` | src/uenux2/src/app/vota/operador/u19-foreign-fragments.cpp | uenux2/src/app/vota/operador/outrasopcoes/ctituloencerramentoinvalido.cpp (path inferred) | high |
| 10750 | 16 |  | `api_f10750` | `std::vector<api::SPolySingleton>::emplace_back(const char*&, &typeid(ecourna::api::security::IRng), shared_ptr<ecourna::api::security::IRng>&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ shared_ptr from unique_ptr (thunk of 1010) | medium |
| 10759 | 14 |  | `api_f10759` | `std::shared_ptr<ecourna::api::security::IRng>::shared_ptr(std::unique_ptr<ecourna::api::security::IRng>&&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ shared_ptr from unique_ptr (thunk of 1010) | medium |
| 10801 | 16 | ✓ | `api_f10801` | `std::vector<api::SPolySingleton>::emplace_back(const char*&, &typeid(comum::IInterfaceSavd), shared_ptr<comum::IInterfaceSavd>&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ shared_ptr from unique_ptr (thunk of 1010) | medium |
| 10807 | 14 | ✓ | `api_f10807` | `std::shared_ptr<comum::IInterfaceSavd>::shared_ptr(std::unique_ptr<comum::IInterfaceSavd>&&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ shared_ptr from unique_ptr (thunk of 1010) | medium |
| 10876 | 2140 | ✓ | `api::CPolySingletonList::push@10876` | `api::IAjusteDataHora::CreateInst()` | src/uenux2/src/api/util/iajustedatahora.u19.cpp | uenux2/src/api/util/iajustedatahora.h (path inferred) | low |
| 10981 | 433 | ✓ | `std::function<api::BatteryIconDataSource<0>::BatteryIconDataSource::'lambda'>::operator()` | `api::BatteryIconDataSource<Horizontal>::UpdateBatteryIcon (timer lambda operator())` | src/uenux2/src/api/gui/cpowerinformation.u19.cpp | uenux2/src/api/gui/cpowerinformation.cpp | high |
| 10987 | 433 | ✓ | `std::function<api::BatteryIconDataSource<1>::BatteryIconDataSource::'lambda'>::operator()` | `api::BatteryIconDataSource<Vertical>::UpdateBatteryIcon (timer lambda operator())` | src/uenux2/src/api/gui/cpowerinformation.u19.cpp | uenux2/src/api/gui/cpowerinformation.cpp | high |
| 10991 | 18 |  | `api_f10991` | `exit-time destructor of api::CPowerInformation::ms_icones` | src/uenux2/src/api/gui/cpowerinformation.u19.cpp | uenux2/src/api/gui/cpowerinformation.cpp | medium |
| 11157 | 1846 |  | `api::CPolySingletonList::instance@11157` | `api::CPolySingleton<api::CEscritorLog>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | high |
| 11530 | 415 |  | `api::ITextToSpeech::vf0` | `api::ITextToSpeech::GetAudio(const std::string&) [vtable slot 0]` | src/uenux2/src/api/audio/itexttospeech.u19.cpp | uenux2/src/api/audio/itexttospeech.cpp (path inferred) | medium |
| 11533 | 53 |  | `ecourna_f11533` | `std::_AllocatorDestroyRangeReverse<allocator<SPolySingleton>>::operator()` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ rollback functor | medium |
| 11544 | 26 |  | `api_f11544` | `std::__allocator_destroy(first, last) for SPolySingleton` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ destroy range | medium |
| 12016 | 37 |  | `api_f12016` | `exit-time destructor of the static std::unique_ptr<api::CLp>` | src/uenux2/mock/app/simulador/wasm/cwasmclp.u19.cpp | uenux2/mock/app/simulador/wasm/cwasmclp.cpp | medium |
| 12053 | 94 |  | `api_f12053` | `vota::(anonymous namespace)::QRCodeAtual()` | src/uenux2/src/app/vota/eleitor/fimvotacao/cmostraqrcodebu.u19.cpp | uenux2/src/app/vota/eleitor/fimvotacao/cmostraqrcodebu.cpp | medium |
| 12054 | 208 |  | `api_f12054` | `vota::(anonymous namespace)::TextoInstrucaoQRCode()` | src/uenux2/src/app/vota/eleitor/fimvotacao/cmostraqrcodebu.u19.cpp | uenux2/src/app/vota/eleitor/fimvotacao/cmostraqrcodebu.cpp | medium |
| 13156 | 129 | ✓ | `api_f13156` | `api::(anonymous)::LogUenux(int, const char*, ...)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | uenux2/src/api/pattern/cpolysingletonlist.h | low |
| 13621 | 318 | ✓ | `api_f13621` | `api::CUpgradeMutex::upgrade(std::shared_lock&) / unlock_shared_and_lock()` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |
| 14423 | 16 | ✓ | `api_f14423` | `std::vector<api::SPolySingleton>::emplace_back(const char*&, &typeid(api::ITextToSpeech), shared_ptr<api::ITextToSpeech>&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ shared_ptr from unique_ptr (thunk of 1010) | medium |
| 14456 | 14 |  | `api_f14456` | `std::shared_ptr<api::ITextToSpeech>::shared_ptr(std::unique_ptr<api::ITextToSpeech>&&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | library/inlined helper: libc++ shared_ptr from unique_ptr (thunk of 1010) | medium |
| 14476 | 31 | ✓ | `api_f14476` | `api::CPolySingletonList::trace(funcao, tipo, detalhe)` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | medium |

## 11. Weird or risky code

1. **Busy-wait locks that can hang the tab** (`CUpgradeMutex`, funcs 1126/3095/13621 and every inlined
   copy). `condition_variable::wait` is a no-op without pthreads, so any lock request that cannot be
   granted spins forever. Two ways to get there:
   * `erase` (640) destroys the replaced object **while holding the exclusive lock**. If that destructor
     calls any `GetInst()`/`instance()`, it loops in `lock_shared`.
   * The inlined `instance`/`push` copies have no landing pads (§9). A throw at lines 99/105 leaves a reader
     counted (the next writer spins); a throw at line 129 leaves the writer lock taken (the next reader or writer
     spins).

   Neither is reachable today: the replaced objects have trivial destructors, and the throws are guarded by
   `exists`. The code is fragile, though, because a later change would freeze the page with no error.
   Impact: simulator (hang). On the urna the same code blocks threads instead of spinning. Low.
2. **Replacing a singleton destroys the object behind outstanding references.** `instance()` returns a raw
   `T&`, and `replace` of an already-registered interface erases and destroys the old instance (`ISound`,
   `ITextToSpeech` and `ISystemDateTime` are replaced at start-up, and `ITextToSpeech` again on every
   `votaInit`). A class that cached the old reference would hold a dangling one. No such cache was found on
   the paths that ran. Impact: simulator (and urna if replacements happened there). Low.
3. **Uninitialised stack buffer printed** (func 13156). With `DEBUG_UENUX` set, `printf("%s: ", buf)` prints
   140 bytes of stack that nothing wrote. This was verified: prefixes such as `bf60`, `À`, `bf600§`. It can
   read past the buffer until a NUL byte (information leak of stack contents to stdout, and a potential
   over-read). Impact: simulator only when a developer sets the variable. The urna uses syslog. Low.
4. **Tracing always on** (func 11265 sets `debug = true`). Every `push` runs `std::format` + `__cxa_demangle`
   + `access("/dev/urna")`/`getenv`. The cost is small, but it shows the diagnostic switch is hard-wired.
   Info.
5. **`emscripten_sleep(3000)` in `CPerguntaCodigoSuspensao::ProcessInput`** (10420), behind the flag
   @1584624 (= 1). In a build without Asyncify it would abort the module when a mesário types an invalid
   título to suspend a vote. It is unreachable in the browser (operator thread not started). On the urna it
   is a 3 s pause. Info.
6. **`std::async` without threads** (10224, `CThreadMonitor::SaiPorVotacaoSuspensa`). `launch::async` needs
   `pthread_create`. In this no-pthread build it would throw `system_error`. The path is unreachable (the
   monitor thread is not started). Info.
7. **Screen BU vs printed BU** (1956). The on-screen QR codes are regenerated with a 2500-character limit
   instead of 1100. They carry the same data but a different slicing, hash chain and signature. Anyone who
   compares screen and paper QR codes must not expect identical payloads. This is also a second, independent
   generation from the in-memory state (RDV and `vota.bin`) at display time, not a copy of what was printed.
   Info.
8. **`CPerguntaCodigoSuspensao` check** (10420): the typed título must be valid and different from the typed
   identity of the voter being suspended. Nothing else limits who can suspend (no check that the título
   belongs to a registered mesário in this function). Same code as the urna. Info.

## 12. Open questions

* The real names of the registry struct (`SPolySingleton`), the lock class (`CUpgradeMutex`), the
  `debug` flag, the function-pointer accessor, the register-or-replace wrapper (`replace`) and its by-value
  helper (4162/1562), and the choice between `exists` and `contains`. None of them has a string or srcloc.
  Other units' reconstructions (e.g. `vota_web_wasm.u29/u30.cpp`) still write the start-up registrations as
  `push<T>`; in the binary they go through the wrapper (§3.4).
* What the uninitialised 140-byte buffer of func 13156 is meant to hold (a timestamp or a thread/process
  name, filled by a call that the web build compiled to nothing).
* Why the accessor (func 11265) writes `debug = true` on every call. It could be a web-build override
  (the pointer variable sits next to `vota_web_wasm.cpp` data) or the stock behaviour of the header.
* The meaning of the int `10` that `main` passes to `CWasmInit` (IInterfaceInit slot 4), and the slot names
  of `IControladorRegistraMesarios` 24/25 and `IBeep` 4.
* The exact file of func 8302 (`csimuladorwasm.cpp` is a guess) and of the operator states in §6.4.
