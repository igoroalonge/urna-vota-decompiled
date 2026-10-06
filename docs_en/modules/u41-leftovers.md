# u41: leftovers, seven functions re-attributed to TSE code after the first pass

Unit u41 holds 7 wasm functions that the first reconstruction pass (u01–u40) did not own. Three had been
filed under a library or under `unknown` (`rhvoice_f501`, `shared_f10255`, `unknown_f7909`) and were later
re-attributed to TSE code. The other four had a component but no name (`api_f2741`, `api_f2744`, `api_f3884`,
`simulador_f3326`). Five of the seven ran during the recorded votes (`analysis/runtime/vote_*.functions.tsv`).
The consistency pass later added an eighth function, func 5922 `vota::LegendaValida` (see the Addendum at the end),
so `analysis/units.json` now lists 8 functions for u41.

After reading them, they fall into five small groups:

| group | funcs | what they are |
|---|---|---|
| inter-thread message queue | 501 | `api::CPriorityMessageQueue<api::SMessage>::Add`, the only way a message reaches the voter or the operator (mesário) state machine |
| poly-singleton registry | 3884, 2741, 2744 | one merged body of `CPolySingletonList::exists<T>` and two of its thunks (`ISincronismoVotoEleitor`, `IPoliticaExecucaoEleitor`) |
| web timer | 7909 | `simulador::CWasmTimer::Dispara`, the `setTimeout` callback behind every periodic GUI timer of the simulator |
| thread entry point | 10255 | `api::CThread::ThreadProc`, the start routine handed to `IThreadImpl::Create` (never runs in the browser) |
| **not TSE code** | 3326 | libc++ `std::string::insert(const_iterator, char)`. The tools called it `simulador_f3326` because `simulador::ResolveCaminho` is one of its three callers |

Three of the seven (501, 7909, 10255) already had a reconstruction written by the unit that owns their file
(u18, u31). Those reconstructions were re-checked instruction by instruction here. They are faithful, and
this unit only added comments (the byte-level order, the exception paths, one wrong claim fixed). The other four are
new: 3884/2741/2744 now have a comment on the `exists<>` template and explicit instantiations, and 3326 has a
reference body.

Reconstructed sources touched by this unit (all edits are additive and marked "u41"):

```
src/uenux2/src/api/ipc/cmessagequeue.h                      Add (501): exception-path notes, prioridade-100 sender,
                                                            +16 = priority_queue comparator
src/uenux2/src/api/pattern/cpolysingletonlist.h             exists<INTERFACE>: merged body 3884 and its thunks
src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp       instantiation index: 3884, 2741, 2744
src/uenux2/src/app/vota/eleitor/csincronismovotoeleitor.cpp      template exists<ISincronismoVotoEleitor>  (2741)
src/uenux2/src/app/vota/eleitor/comum/cpoliticaexecucaoeleitor.cpp template exists<IPoliticaExecucaoEleitor> (2744)
src/uenux2/mock/app/simulador/wasm/cwasmresource.u29.cpp    call site + reference body of libc++ string::insert (3326)
src/uenux2/mock/app/simulador/wasm/cwasmtimer.cpp           Dispara (7909): re-check notes
src/uenux2/src/api/ipc/cthread.cpp                          ThreadProc (10255): re-check notes
```

## 0. Words

| term | meaning |
|---|---|
| urna / UE | the voting machine (*urna eletrônica*) |
| eleitor / mesário | voter / poll worker. Each has a thread with a state machine: `vota::CThreadEleitor`, `vota::CThreadOperador` |
| MT | the mesário's micro-terminal (keypad + LCD) |
| poly-singleton | TSE's name for a singleton looked up by interface (`api::CPolySingletonList`, see u19) |
| prioridade / sequência | priority / sequence number of a queued message |
| Dispara | "fires" (the timer callback) |

## 1. Where these functions fit

```
                 JavaScript (page)                                 urna (not the web build)
   votaInit / votaTick            setTimeout(838, arg, ms)           IThreadImpl::Create(4527, this)
          │                                  │                                   │
   3903 Envia(id) ─► 7708 ─► 501 Add    7909 CWasmTimer::Dispara          10255 CThread::ThreadProc
          │               (lock, heap,       │  (std::function,                   │  Run(); m_estado = 3
          │                sem_post)         │   re-arm)                          │
   votaTick: CurrentStateName, BuildStateJson ──► IExecucaoVota::GetInst ─► 2736 ─┐
   CSincronismoEleitor::ProcessMessage ─► ISincronismoVotoEleitor::GetInst ─► 2741 ─┼─► 3884 exists body
   CConfirmaVotoNominal::PosTecla ─► IPoliticaExecucaoEleitor::GetInst ─► 2744 ─┘     (lock_shared, find)
   simulador::ResolveCaminho ─► 3326 std::string::insert(begin(), '/')   (also ASN.1 runtime, Boost.Regex)
```

## 2. `api::CPriorityMessageQueue<api::SMessage>::Add` (func 501)

The class is described in u18 (`cmessagequeue.h`, srclocs :113, :114, :153). Each state-machine thread owns
one queue (`CThreadEleitor+36`, `CThreadOperador+36`). Other threads `Add` messages and the owner `Remove`s
them (func 2073) in priority order, FIFO within one priority.

Layout used by `Add` (`this` = the queue):

| offset | member |
|---:|---|
| +4 | `std::priority_queue<SEntrada, vector<SEntrada>, CMenorPrioridade>`: the vector (begin/end/cap) |
| +16 | the empty comparator `comp` of `std::priority_queue`. libc++ declares it as a plain member, so it takes 1 byte padded to 4. u18 had left this word unexplained |
| +20 | `std::unique_ptr<ISemaphore> m_pSemaforo` (web: `CPosixSemaphore`) |
| +24 | `std::unique_ptr<ISyncCtl> m_pLock` (web: `CPosixMutex`) |
| +28 | `unsigned m_sequencia` |

Body, in the order the wasm executes it:

1. `CLockGuard lock(*m_pLock)`: `ISyncCtl` slot 2 (`Lock`) as a plain `call_indirect`. It runs before the
   guard exists, so nothing needs undoing if it throws.
2. `seq = m_sequencia++`. The counter is written back before the push, so a failed push still uses up a number.
3. `m_fila.push({mensagem, prioridade, seq})` via `invoke_vii` of func 11076 (`push_back` with the
   `__swap_out_circular_buffer` slow path, then the `push_heap` sift-up). The 8-byte `SMessage` is copied as one
   i64. The comparator compares `prioridade` **signed** and `sequencia` **unsigned** (older first on ties).
4. `m_pSemaforo->Unlock()`: `ISemaphore` slot 3 = `CPosixSemaphore::Unlock` (srcloc :99, `sem_post`), via `invoke_vi`.
5. `~CLockGuard` (func 5528, `ISyncCtl` slot 3). The landing pad shared by steps 3 and 4 runs the same destructor
   and rethrows (`__resumeException`).

Callers: 34 direct call sites in 30 functions, almost all voter and mesário states (`CEleitorVotando`,
`CSincronismoEleitor`, `CFimVotoEleitor`, `CSincronismoOperador`, `CHabilitaAudioEleitor`, …) and the one-line
`Envia` wrapper 7708 used by the web entry points (u33: 3903 ← 11026/11100/10376). **All of them pass priority 1
except one.** `vota::CVerificaEleicaoPassou::StartState` (func 11914, u09) posts message 0 (stop) to the
*operator* queue with **priority 100** when the election date limit has passed outside training mode ("Data
da eleição inválida"). The u18 comment and the tools' evidence string ("every sender posts SMessage with
priority 1") were wrong on this point. The header comment is now corrected.

Observed executing in both recorded votes (edges 7708 → 501 and 4464 → 501).

## 3. `CPolySingletonList::exists<T>`: body 3884, thunks 2741 and 2744

`exists<INTERFACE>(info)` (u19; other chapters call it `contains`) answers "was an implementation of
INTERFACE registered?":

```cpp
template <typename INTERFACE>
static bool exists(TPolySingletonsInfo& info)
{
    const std::string nome = typeid(INTERFACE).name();
    std::shared_lock leitura(info.mutex);
    return find(nome, info).first;
}
```

wasm-opt's *merge-similar-functions* reduced its ~37 instantiations to a few bodies that differ only in how
the name reaches them. The thunks pass the part that varies as a constant (u19 §3.1, `cpolysingletonlist.u19.cpp`):

| body | name argument | thunks |
|---|---|---|
| 1166 | the mangled name string itself (`"N3api13ITextToSpeechE"`) | 11 |
| **3884** | **the address of the `type_info`'s name field** (`&typeid(T) + 4`), dereferenced by the body | **2736 `vota::IExecucaoVota`, 2741 `vota::impl::ISincronismoVotoEleitor`, 2744 `vota::impl::IPoliticaExecucaoEleitor`** |
| 3928 | same as 3884, but with the `std::string(const char*)` constructor inlined (`strlen` + `memcpy`) | 3 |
| 6131/6132/6057/2924 | none: the name is built inline from 8-byte pieces | 10 |

3884 is instruction-for-instruction func 1166 plus one `i32.load` (verified by diffing the WAT). The thunk
constants are the `__type_name` fields of the three interfaces' `type_info` objects:

| thunk | constant | `type_info` at | `*constant` |
|---|---|---|---|
| 2736 | 1600568 | 1600564 (vtable `__class_type_info` @1525564) | `"N4vota13IExecucaoVotaE"` |
| 2741 | 1534152 | 1534148 | `"N4vota4impl23ISincronismoVotoEleitorE"` |
| 2744 | 1536364 | 1536360 | `"N4vota4impl24IPoliticaExecucaoEleitorE"` |

Their callers confirm the types. `replace<ISincronismoVotoEleitor>` (9437, called from `main`) runs
`if (2741(info)) erase(string(*1534152), info); push 5398`. The inlined
`IPoliticaExecucaoEleitor::GetInst()` in 5925 runs `if (!2744(info)) push 5407(new CPoliticaExecucaoEleitor)`
and later tests 2744 again for `CPolySingleton<>::instance` (cpolysingleton.h:78, "PolySingleton - solicitada uma
instancia nao criada N4vota4impl24IPoliticaExecucaoEleitorE"). `__wasm_call_ctors` calls both thunks and
discards the result (u07 §8).

Why these three (and the three thunks of 3928) read the name at run time: in the translation units that
emitted them, `typeid(T).name()` was not folded to a string constant but kept as a load from the `type_info`
object. One likely reason is that the `type_info` was defined in another TU (the interface has an out-of-line
key function, whereas the u06/u07 headers declare these destructors inline). wasm-opt then rewrote
`i32.const ti; i32.load offset=4` as `i32.const ti+4; i32.load`, and merging turned that constant into a
parameter.

Body 3884, step by step (sp-relative frame of 32 bytes):

1. `std::string nome(*pNome)`: func 148, at sp+20. It runs before the lock, as a plain call.
2. `std::shared_lock leitura(info.mutex)`: `{&info + 16, owns = 1}` at sp+12, then `CUpgradeMutex::lock_shared`
   (func 3095) through `invoke_vi`. If that throws, only `nome` is destroyed, because the `shared_lock`
   constructor did not finish.
3. `find(nome, info)`: func 608 through `invoke_viii`, with the pair returned at sp+4. The result is `.first`
   (the byte at sp+4).
4. `~shared_lock` (func 1322 → `unlock_shared`) and `~string` (func 149). If `find` throws, the landing pad runs
   both and rethrows.

Runtime: `IExecucaoVota::GetInst()` (func 3594) is called over and over by the web adapter. On every
`votaTick`, `CurrentStateName` (5408) and `CVotaWebEngine::BuildStateJson` (5500) ask it for the current state.
The edge 3594 → 2736 has 1450 samples in `vote_geral_t1` (2736 → 3884: 1424), about 1 % of `votaTick`. A
string is allocated and a linear `find_if` runs every time the execution policy is looked up. 2741 ran from
`__wasm_call_ctors`, `main` (9437) and `CSincronismoEleitor::ProcessMessage` (7181). 2744 ran from `main`
(9507).

## 4. func 3326 is libc++ `std::string::insert(const_iterator, char)`, not simulator code

The body matches libc++'s `basic_string<char>::insert(const_iterator __pos, value_type __c)` exactly:
`ip = pos - data()`, a capacity test (`(cap_word & 0x7fffffff) - 1` for a long string, 10 for a short one), then
either `__grow_by_without_replace(cap, 1, sz, ip, 0, 1)` (func 1716) or a `memmove` of the tail by one byte.
It then stores `c` and the terminating NUL, sets the size (long: word +4, short: byte +11 `& 0x7f`) and returns
`begin() + ip`.

Its three callers come from three unrelated components:

* `simulador::ResolveCaminho` (2626, u29): `s.insert(s.begin(), '/')` makes a resource name absolute after the
  Qt `':'` prefix has been stripped. It did not run in the recorded votes. All 36 resource-name literals in the
  binary are `":/resource/…"`, and those already start with `'/'` once the `':'` is gone.
* the ASN.1 runtime AVN decoder (5055, for `OCTET STRING` / `BIT STRING`);
* Boost.Regex `basic_regex_creator<char>::append_set` (5139), through table slot 6483 and `invoke_iiii`.

The right attribution is **lib:libcxx** (an instantiation from `<string>`). The reference body is written
as a comment next to its call site in `cwasmresource.u29.cpp`.

## 5. `simulador::CWasmTimer::Dispara` (func 7909)

The simulator has no pthreads, so `api::CTimer` (a `std::thread`) cannot work. `simulador::CWasmTimer` (u31)
re-arms itself with `emscripten_async_call(838, arg, ms)`, i.e. a JavaScript `setTimeout`, and table slot 838 is
this function. `arg` is a heap `SAgendamento {shared_ptr<State> estado; uint64 geracao;}` (16 bytes). The shared
`State` (48 bytes) holds `ativo` (+0), `geracao` (+8), `intervalo` in ms (+16) and the `std::function<void()>` (+24,
`__f_` at +40).

```cpp
void CWasmTimer::Dispara(void* arg)
{
    auto* ag = static_cast<SAgendamento*>(arg);
    State& st = *ag->estado;
    if (st.ativo && st.geracao == ag->geracao) {
        st.callback();                               // __f_ null -> __throw_bad_function_call (648); else slot 6
        if (st.ativo && st.geracao == ag->geracao)   // the callback may have stopped/restarted the timer
            emscripten_async_call(&Dispara, new SAgendamento{ag->estado, ag->geracao}, st.intervalo);
    }
    delete ag;                                       // ~shared_ptr (ctrl slot 2 + __release_weak) + free
}
```

The u31 reconstruction matches the wasm, including the double check, the copy of the `shared_ptr` (use count at
ctrl+4, skipped for a null control block) and the stored generation, which is the State's current value and
equal to `ag->geracao` at that point. Each pending call owns a reference to the `State`. If the callback destroys
the `CWasmTimer` (its destructor calls `Stop()`, which bumps `geracao`), the `State` and the running
`std::function` stay alive until `Dispara` returns. The re-check then fails and nothing is re-armed.
Observed executing: every status-bar clock tick, blinking field and battery-icon refresh (u31: 179 arms in one
municipal vote).

## 6. `api::CThread::ThreadProc` (func 10255)

```cpp
void* CThread::ThreadProc(void* parametro)        // table slot 4527
{
    auto* thread = static_cast<CThread*>(parametro);
    thread->Run();                                 // vtable slot 2, plain call_indirect (no try)
    thread->m_estado = TERMINADA;                  // +4 = 3
    return nullptr;
}
```

`CThread::Start` (1684, cthread.cpp:68) passes it to `IThreadImpl::Create` (slot 2) with `this`. Start's only
callers are the urna execution policy `vota::CExecucaoVota` (10233 `IniciaOperador`, 10235 `Inicia`,
10236 `Executa`). The web build uses the cooperative policy and never starts a real thread, so 10255 is dead in
the simulator (not in the profiles). The u18 reconstruction is exact.

## 7. wasm / Emscripten observations

* **merge-similar-functions with a pointer-to-field constant** (3884). Merged bodies usually take a string or a
  vtable as their parameter. Here it is `&typeid(T) + 4`, which exists only because `OptimizeInstructions` folded
  the load offset into the constant address before the merge.
* **Library code named after a caller** (3326). A libc++ instantiation shared by the simulator, the ASN.1
  runtime and Boost.Regex got a `simulador_` name because the classifier picked one caller. It is also in the
  function table (slot 6483) because Boost calls it through `invoke_iiii`.
* **setTimeout callbacks run outside `votaTick`** (7909). An exception from a timer callback is not caught by
  the C API's `try` and goes up to `callUserCallback`. See the comment in `cwasmtimer.cpp` (u31).
* **`invoke_*` shows the RAII scopes**. In 501 and 3884 the calls made *before* the RAII object is complete
  (`Lock`, `std::string` constructor) are plain calls. The ones made while it is alive go through `invoke_*`,
  and their landing pads run the destructors. This is how the guard and lock scopes above were recovered.

## 8. Weird or risky code

| # | func | what | impact |
|---|---|---|---|
| 1 | 501 | the sequence number is consumed before the push. If the push throws (`bad_alloc`), a number is skipped | harmless: the numbers only order the messages |
| 2 | 501 | if `ISemaphore::Unlock` throws, the message stays queued without its semaphore count, so the consumer may never wake for it | unreachable in the web build (`sem_post` is a stub) |
| 3 | 501 / 11914 | a priority-100 "stop" message jumps ahead of every pending priority-1 message in the operator queue | presumably intended (it is the only priority other than 1) |
| 4 | 3884 | every `IExecucaoVota::GetInst()` allocates a `std::string`, takes the shared lock and runs a linear search. The web adapter does this several times per tick | about 1 % of `votaTick` time |
| 5 | 7909 | an exception from the callback leaks the `SAgendamento` and its `State` reference and stops the timer for good | see u31 |

## 9. Open questions

* The member names `Add`, `exists`, `Dispara` and `ThreadProc` are inferred. No string or srcloc names them.
* Why `__wasm_call_ctors` calls `exists<ISincronismoVotoEleitor>` / `exists<IPoliticaExecucaoEleitor>` and
  discards the result is still open (u07 suggests `static const bool` registration checks).
* The analysis database still has the old names for 2741, 2744, 3884 and 3326 (`api_fNNNN`, `simulador_f3326`).
  The names in the table below are the ones to put in `analysis/names.override.json`.

## 10. Mapping table (the 7 functions of the first pass; 5922 is in the Addendum)

`ran` = seen executing in the recorded votes (`analysis/runtime/vote_*.functions.tsv`).

| func | bytes | ran | tools name | reconstructed symbol | original file | reconstruction | conf. |
|---|---|---|---|---|---|---|---|
| 501 | 217 | ✔ | `api::CPriorityMessageQueue<api::SMessage>::Add` (was `rhvoice_f501`) | `api::CPriorityMessageQueue<api::SMessage>::Add(const SMessage&, int prioridade)` | uenux2/src/api/ipc/cmessagequeue.h (file attested by srclocs :113/:114/:153; name inferred) | src/uenux2/src/api/ipc/cmessagequeue.h | high (code), medium (name) |
| 2741 | 12 | ✔ | `api_f2741` | `api::CPolySingletonList::exists<vota::impl::ISincronismoVotoEleitor>(TPolySingletonsInfo&)` (thunk → 3884) | uenux2/src/api/pattern/cpolysingletonlist.h | src/uenux2/src/app/vota/eleitor/csincronismovotoeleitor.cpp (explicit instantiation); template in cpolysingletonlist.h | high (type), medium (member name) |
| 2744 | 12 | ✔ | `api_f2744` | `api::CPolySingletonList::exists<vota::impl::IPoliticaExecucaoEleitor>(TPolySingletonsInfo&)` (thunk → 3884) | uenux2/src/api/pattern/cpolysingletonlist.h | src/uenux2/src/app/vota/eleitor/comum/cpoliticaexecucaoeleitor.cpp (explicit instantiation); template in cpolysingletonlist.h | high (type), medium (member name) |
| 3326 | 208 |  | `simulador_f3326` | `std::basic_string<char>::insert(const_iterator, char)` (libc++; component **lib:libcxx**, not TSE) | libc++ `<string>` (instantiation) | src/uenux2/mock/app/simulador/wasm/cwasmresource.u29.cpp (call site + reference body, comment) | high |
| 3884 | 201 | ✔ | `api_f3884` | `api::CPolySingletonList::exists<T>` merged body `(info, const char* const* pNome)` for `IExecucaoVota` / `ISincronismoVotoEleitor` / `IPoliticaExecucaoEleitor` | uenux2/src/api/pattern/cpolysingletonlist.h | src/uenux2/src/api/pattern/cpolysingletonlist.h (template + comment), cpolysingletonlist.u19.cpp (index) | high |
| 7909 | 231 | ✔ | `simulador::CWasmTimer::Dispara` (was `unknown_f7909`) | `static void simulador::CWasmTimer::Dispara(void*)` (table slot 838) | uenux2/mock/app/simulador/wasm/cwasmtimer.cpp (path inferred) | src/uenux2/mock/app/simulador/wasm/cwasmtimer.cpp | high (code), medium (name) |
| 10255 | 24 |  | `api::CThread::ThreadProc` (was `shared_f10255`) | `static void* api::CThread::ThreadProc(void*)` (table slot 4527) | uenux2/src/api/ipc/cthread.cpp (file attested by srclocs :31/:32/:68; name inferred) | src/uenux2/src/api/ipc/cthread.cpp | high (code), medium (name) |

Functions of other units referenced above (for navigation, not part of u41): 1166/3928/6131/6132/6057/2924
(other `exists` bodies), 2736 (`exists<IExecucaoVota>`), 3594 (`IExecucaoVota::GetInst`), 608/6161 (`find`), 3095
(`lock_shared`), 1322 (`~shared_lock`), 148/149 (`std::string` ctor/dtor), 5398/5407 (`push<>`), 9437/9507
(`replace<>` from `main`), 640 (`erase`), 5925 (`CConfirmaVotoNominal::PosTecla`), 7181
(`CSincronismoEleitor::ProcessMessage`), 11076 (heap push), 5528 (`~CLockGuard`), 2073 (`Remove`), 7708/3903
(`Envia`), 11914 (`CVerificaEleicaoPassou::StartState`), 1716 (`__grow_by_without_replace`), 2626
(`ResolveCaminho`), 5055 (ASN.1 AVN decoder), 5139 (Boost `append_set`), 7918/7934/3347 (`CWasmTimer`
Start/Stop/dtor), 648 (`__throw_bad_function_call`), 1684 (`CThread::Start`), 10233/10235/10236 (`CExecucaoVota`).

## 11. Fidelity review (2026-09-23)

Each function was compared with its decompiled body and its WAT. The neighbours the text relies on were checked
too: 1166 (WAT diff with 3884), 3928, 9437, 5925, 11076, 11914, 1684, 7918, and the runtime edge files.
Findings and fixes:

* **501**: the u18 reconstruction matches (lock → seq++ → heap push → semaphore `Unlock` → guard dtor, with the
  landing pad covering steps 3–4). Fixed: the claim "all senders use prioridade 1" (func 11914 uses 100). Added:
  the +16 word is the `priority_queue` comparator, `prioridade` is compared signed and `sequencia` unsigned
  (`i32.lt_s` / `i32.gt_u` in 11076), and what the exception paths leave behind.
* **3884 / 2741 / 2744**: new. The identity of each thunk was checked three ways: the constant points at the
  `__type_name` field of the matching `type_info`, `replace<>` (9437/9507) passes the same name to `erase`, and
  `CPolySingleton<>::instance` in 5925 formats the same mangled name into its error message.
* **3326**: re-attributed to libc++. The body is libc++'s `insert(const_iterator, char)` down to the short-string
  capacity constant 10 and the `& 0x7f` size byte.
* **7909**: the u31 reconstruction matches (both `ativo`/`geracao` checks, `std::function` null check → func 648,
  slot 6 call, `shared_ptr` copy and release, `free`). No change to the code. Notes added.
* **10255**: the u18 reconstruction matches (slot 2 call, `+4 = 3`, `return 0`). No change to the code. Notes added.


## Addendum: func 5922 `vota::LegendaValida` (added in the consistency pass)

Func 5922 (243 bytes, `(i32 cargo, i32 partido) -> i32`) was left without a component because only two
callers use it: `CVotaWebEngine::BuildStateJson` (func 5500, the web state JSON's `legendaValida`) and
`vota::CPedeProporcional::ProcessInputAudio` (func 11711). It looks the party number up in the
`comum::CPartidos` map (func 819). If the party exists, it scans `comum::CCandidaturas` (func 521) for a
candidatura of that cargo and party, also accepting a federação through `comum::CFederacoes` (func 2832).
The result decides whether the first two digits typed for a proportional cargo are a valid **voto de
legenda**. The name is inferred, as in u26 and u29. It is reconstructed as `LegendaValida` next to its
caller in `src/uenux2/src/app/vota/eleitor/votaproporcional/cpedeproporcional.cpp`, and used in
`src/uenux2/wasm/vota_web/vota_web_wasm.u29.cpp`.

| wasm func | symbol | src |
|---|---|---|
| 5922 | `vota::LegendaValida` (name inferred) | `src/uenux2/src/app/vota/eleitor/votaproporcional/cpedeproporcional.cpp` |
