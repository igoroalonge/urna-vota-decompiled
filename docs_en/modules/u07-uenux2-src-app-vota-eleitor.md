# u07: `uenux2/src/app/vota/eleitor` — the voter thread, vote persistence (RDV sync) and the voting screens

Unit u07 covers 65 wasm functions. Three original files own most of them:

| original file | what it is |
|---|---|
| `uenux2/src/app/vota/eleitor/cthreadeleitor.cpp` | `vota::CThreadEleitor`, the **voter thread** (*thread do eleitor*): it owns the voter terminal (screen, keypad, audio) and runs the voter-side state machine |
| `uenux2/src/app/vota/eleitor/csincronismovotoeleitor.cpp` | `vota::impl::CSincronismoVotoEleitor`, the step that makes one voter's votes **durable**: it writes the encrypted RDV and the voting state to both flash memories and re-signs them |
| `uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp` | `vota::CTelasVota`, the **factory of every voting screen** (*telas de votação*): cargo name, typed digits, candidate name/photo, "VOTO NULO", "VOTO DE LEGENDA", consulta questions… |

The unit tool also attached (by call-graph proximity) functions whose source is elsewhere: GUI helpers from
`uenux2/src/api/gui` (`CFormBuilder::Add`, `CProgressBar`, `CGrayedFramedText`), two `comum::md::CCargo`
accessors, three setters of `vota::CInformacaoEleitor`, the restart/zerésima states (`CReinicioVotacao`,
`CQuerImprimirZeresima`, `CGeraZeresima*`, `testeteclado::CRetomada`), `CProgressoEncerramento`, and two
wasm-opt merged bodies of `GetTelaCargoAtual`. They are reconstructed in *fragment* files named
`<original>.u07.cpp` next to the path of their original file, so that they do not overwrite the files of the
units that own them (same convention as u02).

27 of the 65 functions ran during the two recorded votes (`analysis/runtime/*.functions.tsv`).

**Reconstructed sources** (all under `src/`):

* own files: `uenux2/src/app/vota/eleitor/cthreadeleitor.{h,cpp}`, `uenux2/src/app/vota/eleitor/csincronismovotoeleitor.{h,cpp}`,
  `uenux2/src/app/vota/eleitor/comum/ctelasvota.{h,cpp}`
* fragments: `uenux2/src/app/vota/comum/csincronizavota.u07.cpp`, `uenux2/src/api/gui/{cformbuilder,cprogressbar,cgrayedframedtext}.u07.cpp`,
  `uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.u07.cpp`, `uenux2/src/app/vota/eleitor/comum/cinformacaoeleitor.u07.cpp`,
  `uenux2/src/app/vota/eleitor/iniciovotacao/{creiniciovotacao,cquerimprimirzeresima,cquerreimprimirzeresima,cgerazeresima}.u07.cpp`,
  `uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cretomada.u07.cpp`, `uenux2/src/app/vota/eleitor/fimvotacao/cprogressoencerramento.u07.cpp`,
  `uenux2/src/app/vota/eleitor/votamajoritario/cpedemajoritario.u07.cpp`

---

## 1. Where this code sits in the voting process

Glossary: *eleitor* = voter; *mesário* = poll worker at the operator terminal (*terminal do mesário*); *cargo* =
office being voted (Prefeito, Vereador, Senador…); *voto nominal / de legenda / branco / nulo* = vote for a
candidate / for a party only / blank / null; *consulta* = referendum-style question whose "candidates" are
answers; *suplente / vice* = running mates; *RDV* (*Registro Digital do Voto*) = the anonymised, encrypted list of
every vote (`rdv.dat`); *MI* (*memória interna*) = internal flash `/dsk/fi`; *MV* (*memória de votação*) = the
removable flash card `/dsk/fe`; *zerésima* = the report printed when voting opens, proving every candidate
starts at zero; *reinício* = restart of the application during election day; *treinamento eleitor* = voter
training mode (the simulator's scenarios are in this mode).

The urna application VOTA runs three cooperative "threads" (see `docs/modules/u06-…` for the states):

```
 operator terminal                        voter terminal (this unit)
 CThreadOperador  --msg 1 (inicia) ---->  CThreadEleitor.m_fila (vota::CMessageEleitor)
 (CHabilitaAudio*) --msg 8/9/10 ------->      |  Processar(): messages -> keyboard -> ticks
                                              v
                         comum::CAppStateContext: CAguardaMensagem -> CEleitorVotando -> ... ->
                         CSincronismoEleitor ("Gravando") -> CFimVotoEleitor ("FIM") -> CAguardaMensagem
 CMostraEleitorVotando <--msg 13--------  CSincronismoEleitor::StartState
 -> CSincronismoOperador::StartState (func 10210)
    ---msg 5--------------------------->  CSincronismoEleitor::ProcessMessage(5)
                                              -> ISincronismoVotoEleitor::GetInst().SincronizaVoto()
                                                 = CSincronismoVotoEleitor (urna) / CSincronismoVotoEleitorWeb (simulator: "return true")
```

Every screen the voter sees during those states is a `CFormInterativoTelaVota` taken from `CTelasVota`.

## 2. Classes (RTTI) and layouts

```
api::CThread                                   typeinfo @1600028  (cthread.cpp)
 └─ vota::CThreadVota                          typeinfo @1532484  slots 2 and 5 pure
     ├─ vota::CThreadEleitor                   typeinfo @1534664, vtable @1534624  (this unit)
     ├─ vota::CThreadOperador                  vtable @1601208  (Run = func 10204, same loop inlined)
     └─ vota::CThreadMonitor                   vtable @1600768
api::CPriorityMessageQueue<api::SMessage> ─┐
api::CMessageInterface ────────────────────┴─ vota::CMessageEleitor   typeinfo @1534716 (vmi), member of CThreadEleitor
vota::impl::ISincronismoVotoEleitor
 ├─ vota::impl::CSincronismoVotoEleitor        typeinfo @1534176, vtable @1534164   (urna implementation)
 └─ (anonymous)::CSincronismoVotoEleitorWeb    typeinfo @1527264, vtable @1527252   (vota_web_wasm.cpp, returns true)
vota::CTelasVota                               not polymorphic, 252 bytes, singleton @1833396
comum::CAppState ─ vota::CReinicioVotacao (@1546636), CQuerImprimirZeresima (@1544848), CQuerReimprimirZeresima (@1546348),
                   CGeraZeresimaBase (@1544392) ─ CGeraZeresima (@1544584), CRegerarZeresima (@1546500)
                   CGeraResumoZeresimaBase (@1544456) ─ CGeraResumoZeresima (@1544520), CRegeraResumoZeresima (@1546436)
                   CEstadoComDesligamentoAutomatico ─ testeteclado::CBase ─ testeteclado::CRetomada (@1546708)
comum::CAbstractTelaProgresso ─ vota::CProgressoEncerramento (@1540596)
```

Virtual slots used by this unit (names consistent with u06):

| class | slots |
|---|---|
| `api::CThread` / `CThreadVota` | 0 dtor, 1 deleting dtor, 2 `Run`, 3 `TrataExcecao(const std::exception&)` (func 7710), 4 `TrataExcecaoDesconhecida` (7709), 5 `FinalizaExecucao` (pure in CThreadVota) |
| `api::IThreadImpl` (`simulador::CWasmThread`) | 2 `Create(void*(*)(void*), void*)` (9655), 5 `Yield` (9651) |
| `comum::CAppState` | 2 `StartState`, 3 `NeedChangeState` (= `GetNextState() != this`, func 7480), 4 `GetNextState` (returns +4), 5 `FinishState`, 6 `ProcessMessage(uebyte)`, 7 `ProcessInput()`, 8 `ProcessTick(uebyte)` |
| `vota::IExecucaoVota` | 2/3 start the voter, operator and power-monitor threads (2 then joins them), 4 join, 5 start the operator thread only (`CExecucaoVota`, func 10233), 6 `FinalizaThreads` (name inferred), 7 cooperative step (`CExecucaoVotaCooperativa::Processa`, func 7823, = `CThreadEleitor::GetInst().Processar()`), 8 `GetFilaEleitor()` (= `&CThreadEleitor::GetInst().m_fila`) |
| `ISincronismoVotoEleitor` | 0 dtor, 1 deleting dtor, 2 `SincronizaVoto()` (name inferred) |
| `testeteclado::CBase` | 10 `CriaTela`, 11 `GetEstadoPassouNoTeste` (attested name), 12 `GetEstadoSemTeste` (inferred) |
| `CGeraZeresimaBase` / `CGeraResumoZeresimaBase` | 9 post-print hook (`CRegerarZeresima`: `CRelVotaUtil::CortaPapel`), 10 `GetEstadoResumo` (inferred) |
| `CAbstractTelaProgresso` | 2 `Inicia` (bar = min, show), 3 `Finaliza` (bar = max), 4 `Avanca` |

`comum::CAppState` = `{+0 vptr, +4 m_proximoEstado (= this at StartState), +8 bool recebe mensagens, +9 bool recebe teclado, +10 bool recebe ticks}`;
the constructor (func 224) takes the three flags as a bit mask (1 = messages, 2 = keyboard, 4 = ticks).
`comum::CAppStateContext` = `{+0 vptr, +4 CAppState* m_pEstado}`.

`CThreadEleitor` (84 bytes, `operator new(84)` in func 316):

| offset | member |
|---|---|
| +0 | vptr |
| +4 | `int m_estado` (`CThread::Start` sets 1) |
| +8 | `bool m_bParar` (stop requested) |
| +9 | `bool m_bDormindo` |
| +12 | `unique_ptr<api::IThreadImpl> m_pImpl` (from `IGenericFactory<IThreadImpl>`, cthread.cpp:31/32) |
| +16 | `unique_ptr<api::ISyncCtl> m_pSync` |
| +20 | `std::map<uebyte, STick> m_ticks` (CThreadVota; expiry list = func 5450, `gettimeofday`) |
| +32 | `unique_ptr<comum::CAppStateContext> m_pContexto` (CThreadVota) |
| +36 | `CMessageEleitor m_fila` (vector of `SMessage` +40..+48, semaphore +56, lock +60, 2nd vptr +68) |

`CTelasVota` (252 bytes) members referenced by this unit: `+0 std::map<TCargoID, CTelasCargo>`, `+84` screen of
`CConfirmaRegerarZeresima`, `+124` accessibility instructions, `+148` reinício screen, `+172` "telaProgressoRegistroVoto"
("Gravando" + bar), `+180 shared_ptr<api::CProgressBar>` (max 4, rect (70,225)-(570,255)), `+212`/`+220` zerésima
generation screens, `+228` "quer imprimir a zerésima?", `+244` "quer reimprimir a zerésima?".

---

## 3. The voter thread (`CThreadEleitor`)

### 3.1 Life cycle

* **Creation**: `CThreadEleitor::GetInst()` (func 316, unit u18, mis-named `CPriorityMessageQueue<SMessage>::ctor`)
  lazily builds the object: `api::CThread()`, an empty tick map, no context, and the `CMessageEleitor` queue (its
  semaphore and lock come from the `IGenericFactory<ISemaphore>` / `IGenericFactory<ISyncCtl>` poly-singletons,
  cmessagequeue.h:113/114). The static `unique_ptr` is freed at exit by func 7122.
* **Run** (slot 2, func 7061) — only on the urna:
  1. `IAjusteInicial::GetInst()` (cajusteinicial.cpp:57, inlined): if the `CPolySingletonList` has no
     `IAjusteInicial` (func 2450 `contains`), push a `CAjusteInicial` (12-byte `CAppState` with flags 0); then
     `CPolySingleton<IAjusteInicial>::instance()`. This is the *default registration* pattern used all over VOTA:
     a platform can pre-register its own implementation, otherwise the urna's one is created.
  2. `m_pContexto = make_unique<CAppStateContext>(&ajuste)`, then `StartState()` of that first state.
  3. `while (!m_bParar) { r = Processar(); if (m_bParar) break; m_pImpl->Yield(); if (!r) Sleep(50 ms); }` —
     the sleep is compiled as `if (flag@1584624) emscripten_sleep(50)`; the flag is 1 in the data segment and
     never written. In the web build the `Yield` itself is `simulador::CWasmThread::Yield` (func 9651), which calls
     `emscripten_sleep(0)`: without Asyncify both calls abort, so `Run` could not survive its first cycle.
* **FinalizaExecucao** (slot 5, func 7030): `IExecucaoVota::GetInst().FinalizaThreads()` (slot 6). `CThreadVota`'s
  exception handlers (funcs 7710/7709) call it before showing a fatal error (unless the exception is
  `api::CUeDesligandoError` or the shutdown flag @1832936 is set), so a crash of the voter thread stops the operator
  thread and the power monitor (`CExecucaoVota::vf6`: `m_bParar = 1` on both). The cooperative web policy has a no-op.
* **Destructor** (func 2438, slot 0; 7070 = deleting): member destruction only.

### 3.2 One cycle: `Processar()` (func 4349)

The function the tools first named `ProcessarEntrada` (they now show `vota::CThreadEleitor::Processar`) is the
*outer* cycle. Its only srcloc record (cthreadeleitor.cpp:134,
signature `bool ProcessarEntrada(comum::CAppStateContext&)`) belongs to one of the three inlined helpers; the
debug-log strings give the name of a second one (`"ProcessarMensagens"`). Order of work, each part skipped once
`m_bParar` is set:

1. **ProcessarMensagens**: while the queue is not empty (checked under the queue lock), `Remove()` one `SMessage`
   (func 2073) and dispatch on its 16-bit id:

   | id | action (in the thread itself) |
   |---|---|
   | 0, 6 | `m_bParar = true` (stop the thread) |
   | 8 | `CInformacaoEleitor::m_modoAudio = 0` — audio "conforme cadastro" (posted by the operator state `CHabilitaAudioEleitor` when the voter's registration asks for audio) |
   | 9 | `m_modoAudio = 1` — audio enabled (`CHabilitaAudioEleitor`, and `votaInit` when the page option `audioEleitorHabilitado` is set) |
   | 10 | `m_modoAudio = 2` — audio disabled (`CDesabilitaAudioEleitor`; `votaInit` sets 2 directly otherwise) |
   | anything else | if the current state accepts messages (flag +8): `state->ProcessMessage(uebyte(id))`, then the state-change check. Known ids: 1 start (votaInit / CHabilitaAudioEleitor), 2/3 suspension, 4 continue (u06), 5 synchronise the vote |

   Each message is logged at LOG_DEBUG: `"%s: 1 msg[%d] context[%s]"` (ids handled by the thread) or
   `"%s: 2 msg[%d] context[%s]"` (forwarded), with the mangled `typeid` name of the current state.
2. **ProcessarEntrada** (srcloc :134 is the `api::IInputKbd::GetInst()` call): if the state accepts keyboard
   (flag +9) and `IInputKbd::HasKey()` (slot 3), log `"%s: hasKey[%d] context[%s]"`, `state->ProcessInput()`,
   state-change check. The state reads the key itself (through `CInteractiveForm::Read`).
3. **ProcessarTicks** (name inferred): the expired tick ids of `CThreadVota` (func 5450); if any and the state
   accepts ticks (flag +10), `state->ProcessTick(id)` for each; then the state-change check.

The **state-change check** (inlined everywhere) is
`if (s && s->NeedChangeState()) { s->FinishState(); s = s->GetNextState(); if (s) s->StartState(); }`.
States are singletons; the old one is not deleted. Return value: "something was processed" (used by `Run` to
decide whether to sleep).

### 3.3 Logging helper (funcs 2072 / 2921)

`LogDebug(fmt, ...)` → `LogSistema(LOG_DEBUG, s_urnaReal, fmt, ap)`: the first call caches
`access("/dev/urna", F_OK) == 0` (−1 = unknown). On an urna it calls musl `vsyslog`; elsewhere it prints only
if the environment variable `DEBUG_UENUX` exists, as `printf("%s: ", <local buffer>)` + `vprintf`. The helper
comes from a header with internal linkage; wasm-opt merged the copies of four translation units into func 2921
(caches @1534908 cthreadeleitor, @1601364 cthreadoperador, @1577092 cvalidamidia, @1552472 func 5895). In the
simulator both branches are silent (no `/dev/urna`, no `DEBUG_UENUX`, and syslog cannot reach `/dev/log`, see
`docs/libraries/libc-and-emscripten-runtime.md` §12).

### 3.4 Web build: the thread never runs

`main()` registers `vota::CExecucaoVotaCooperativa` as `IExecucaoVota` (func 7828). Its slot 2 only installs the
first state (a new `CAppStateContext` in the voter thread, func 4713) and its slot 7 (func 7823) calls
`CThreadEleitor::GetInst().Processar()` once. `votaTick` (func 10619) calls slot 7 on every tick. `CThread::Start`
(func 1684) is only called by the urna policy `CExecucaoVota` (funcs 10233/10235/10236), so `Run`, the `Yield`
and the `emscripten_sleep(50)` are dead code in the simulator. The operator thread is not stepped at all.
(Even if a thread were started, `simulador::CWasmThread::Create` (func 9655) only appends the entry point to a
queue, and `CWasmThread::Yield`/`vf3` (Wait) call `emscripten_sleep`, which the glue implements as
`abort("Please compile your program with async support…")`.) `CThread::Start` is also the only place that
clears `m_bParar` (+8); the cooperative policy never does.

---

## 4. Making a vote durable: `CSincronismoVotoEleitor::SincronizaVoto`

### 4.1 Trigger (urna)

1. `CEleitorVotando` records the confirmed votes in the in-memory RDV (func 4454, unit u22) and moves to
   `CSincronismoEleitor`.
2. `CSincronismoEleitor::StartState` (func 7178): posts message **13** to the operator queue
   (`CThreadOperador` +36), resets `CTelasVota::m_barraProgresso` to its minimum (func 3667) and shows
   `m_telaProgressoRegistroVoto` ("Gravando" + a 4-step bar).
3. On the operator thread, message 13 moves `CMostraEleitorVotando` (func 10425) to `CSincronismoOperador`. Its
   `StartState` (func 10210) marks the voter as having voted (`CEleitores::MarcaVotou`, inlined, skipped in
   treinamento eleitor) and posts message **5** back to the voter queue (`CThreadEleitor` +36).
   `CSincronismoOperador::ProcessMessage` (func 10209) only handles messages 1, 14 and 15.
4. `CSincronismoEleitor::ProcessMessage(5)` (func 7181) calls `ISincronismoVotoEleitor::GetInst().SincronizaVoto()`
   (the `GetInst` of csincronismovotoeleitor.cpp:67 is inlined there: default-registers `CSincronismoVotoEleitor`,
   4 bytes) and, when it returns true, moves to `CFimVotoEleitor` ("FIM").

### 4.2 `SincronizaVoto` (func 7174), step by step

The function advances the progress bar four times around two inlined functions of `csincronizavota.cpp`
(anonymous namespace; srcloc lines 290 and 303 are their `throw` sites):

```
AvancaBarraProgresso()                                 (1/4)
SincronizaRDVInternoSeguro()     ── MI
AvancaBarraProgresso()                                 (2/4)
CLogVota::Loga("O voto do eleitor foi computado")      (log level 1)
AvancaBarraProgresso()                                 (3/4)
SincronizaRDVExternoSeguro()     ── MV
AvancaBarraProgresso()                                 (4/4)
return true
```

**SincronizaRDVInternoSeguro** (MI):

1. `CSincronizaVota::VerificaUrnaDesligando()`: if the shutdown flag @1832936 is set, throw
   `api::CUeDesligandoError` (csincronizavota.cpp:47).
2. `rdv = comum::CRdvVota::GetInst()`; path `P = <trab dir MI>/rdv.dat` (func 1551).
3. Write: `CEncryptedFile f(rdv.m_cifrador)` (func 1692; the cipher `shared_ptr` is at `CRdvVota+8`),
   `f.MemWrite(rdv.Converte())` (CRdv slot 11 = BER of `ModuloRegistroDigitalVoto`), `f.Save(P + ".tmp")`
   (func 2767, cencryptedfile.cpp:88).
4. Read back: new `CEncryptedFile`, `Load(P + ".tmp")`, `MemRead(buf)`, `rdv.ConfereConteudo(buf)` (slot 13).
   On mismatch: throw `CBaseError<vota::EUeVotaError>` **9300 "Falha na gravação do RDV na MI"** (:290).
   The `.tmp` file is left behind in that case.
5. `comum::VerificaIntegridadeReferencial()` (func 2543: cross-checks CRdvVota, CCargos, CRespostas).
6. `api::CSystem::ReplaceFile(P + ".tmp", P, "")` (func 5455, "CSystem::ReplaceFile").
7. `CAppInfo::SalvaVotaInterno()` (func 3790): `vota.bin` (`ModuloEstadoGeralVota`) of the current turno.
8. Unless `EhTreinamentoEleitor()` (func 697: phase `'3'` treinamento **and** `EstadoGeralVota.treinamentoEleitor`):
   write the current voter's dynamic record into `<trab MI>/uenux.db` (`CEleitorDinamicoDAO`, SQLite), sync, then
   `CAssinador(pkg).Assina(110 = uenux.db)`, sync.
9. `CAssinador(pkg).Assina(83 = rdv.dat); Assina(31 = vota.bin)`; sync. `pkg` = ESavdPacote 122 (turno 1) or 123
   (turno 2) = `vota.vsu`; the turno is `CEstadoGeral +32` (`'1'`/`'2'`), and each turno has its own work
   directory (`dinamico/trab1`, `dinamico/trab2`) on both memories.
10. `VerificaAssinaturaMI("vota.vsu")`, `("rdv.vsu")`, and `("uenux.vsu")` unless treinamento eleitor
    (votadefs.cpp:103: `IInterfaceSavd::ValidarUE(<trab MI>/<name>)`).

**SincronizaRDVExternoSeguro** (MV): steps 1–7 again with `<trab MV>/rdv.dat` (func 1701), error **9301
"Falha na gravação do RDV na MV"** (:303) and `SalvaVotaExterno` (func 3789). Then, instead of re-signing:

* unless treinamento eleitor: copy `uenux.db` MI → MV (`CArquivosSavd` ids 110 → 111) and call
  `CopiaAssinaturaBancoDadosParaMV()` (func 4682: under `CApplicationContextGuard(4, "", "Gravando o banco de
  dados na MV", "Ocorreu um erro durante a persitência dos dados na MV.")` copy `uenux.vsu` 199 → 201 (turno 1)
  or 200 → 202 (turno 2));
* copy the MI signature packages to the MV: `vota.vsu` 122 → 124 and `rdv.vsu` 134 → 136 (turno 1) or 123 → 125
  and 135 → 137 (turno 2);
* sync; `VerificaAssinaturaMV` of `vota.vsu`, `rdv.vsu` and (unless treinamento eleitor) `uenux.vsu`.

Because the MV copies of `rdv.dat`/`vota.bin` are written again but their signatures are copied from the MI,
the MV verification can only pass if the two writes produce identical bytes, i.e. if the RDV encryption is
deterministic (inference; the `CAesKey` of u01 carries its IV).

`ESavdArquivoUE`/`ESavdPacote` ids come from the tables built in `CArquivosSavd::GetInst` (func 1164):
31 `vota.bin`, 83 `rdv.dat`, 110/111 `uenux.db`; 120/121 `eg.vsu`; 122–125 `vota.vsu`; 126–129 `bu.vsu`;
130–133 `buj.vsu`; 134–137 `rdv.vsu`; 142–145 `rze.vsu`; 146–149 `gap.vsu`; 152–155 `red.vsu`; 191–194 `bim.vsu`;
195–198 `behb.vsu`; 199–202 `uenux.vsu`; 203 `dadoscarga.vsu`.

**Relation to the BU.** This unit never builds the Boletim de Urna. It is the per-voter half of the result chain:
every vote ends in the encrypted RDV (`rdv.dat`) and in the counters of `vota.bin`, on both memories, signed
(`rdv.vsu`, `vota.vsu`). The BU generated at *encerramento* (units u08/u09) is computed from that RDV/state.

### 4.3 In the simulator

`main()` pushes `CSincronismoVotoEleitorWeb` first, so `SincronizaVoto` is `return true` (ICF func 434) and nothing
above runs: no `rdv.dat`, `vota.bin`, `uenux.db` or `.vsu` changes after a vote (confirmed by the MEMFS snapshots,
`analysis/runtime/README.md`). The operator thread is not stepped either, so `votaTick` itself plays the part of
steps 3–4 of §4.1: while the state name contains `"CSincronismoEleitor"` it calls
`CTelasVota::GetInst().AvancaBarraProgresso()` (func 2369) at t+120 ms, +280, +440, +600 ms (every 160 ms, four
times), then posts message 5 through `IExecucaoVota` slot 8 (func 10376). The "Gravando" bar the voter sees is a
timed animation; the transcripts show `CSincronismoEleitor` → 5 screen refreshes → `CAguardaMensagem` ("fim").

`CopiaAssinaturaBancoDadosParaMV` (func 4682) *does* run in the simulator, but only during `votaInit`: the
start-up persistence initialisation (func 4662 → func 4657, "Gravando o banco de dados na MI": signs `uenux.db`,
copies it to the MV) calls it to copy `uenux.vsu` to the MV (profiler samples in both recorded sessions). In the
MEMFS snapshot after `votaInit`, `/dsk/fi/dinamico/trab{1,2}/uenux.vsu` and `/dsk/fe/dinamico/trab{1,2}/uenux.vsu`
all contain the simulated signature text "assinatura simulada para vota_web_wasm".

---

## 5. The voting screens (`CTelasVota`)

### 5.1 How screens are built

`CTelasVota`'s constructor (ctelasvota.cpp:3502, inlined into the start-up function 7787 of u02) builds, for each
cargo, one `CTelasCargo` holding one `CFormInterativoTelaVota` per `ETelaVotacao` (18 kinds, see u06 §5), plus the
fixed screens. Each screen is an `api::CFormBuilder` (a `vector<shared_ptr<IFormField<IScreen>>>`) turned into an
`api::CInteractiveForm<IScreen, IInputKbd>` (func 554) with a `CPreShowFormVota`/`CPreShowProgressBar` hook.
`CFormBuilder::Add` (func 426) names every field `<ClassName><n>` (`"CTextField1"`, …) using `IFormFieldBase`
slot 7. The per-`ETelaVotacao` factories that call the builders below (funcs 1591, 1767, 3063, 3067, 6616, 6626,
6639, 6681: `CriaTela…(cargo)`, each passing `__func__` as `nomeTela`), the footers ("Aperte a tecla: CONFIRMA para …
/ CORRIGE para REINICIAR este voto", func 1102) and the constructor are reconstructed by unit u02 in
`src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp`. Texts are **data sources**, evaluated at draw time:

| source | used for |
|---|---|
| `DS_NomeCargoNeutroComEscolha{copy of CCargo}` (func 6627 / 12587) | top line: gender-neutral cargo name + `" - " + "{}ª vaga"` when the cargo has several seats (Senate) |
| slot 1088 → func 12811 | cargo name in the *candidate's gender* (`CCargoDSNomeSexoCandidato{0,0}`: "Prefeita") |
| slot 1089 → func 11501 (`CDataTextFmt`, format `"{}"`) | party text of the current candidate |
| slots 1090/1092, 1093 | names of running mate 1 / 2;  slots 1091/1095, 1094: their cargo titles (photo captions) |
| slot 1096 → `CRespostas` current answer; `CRespostasDSNumero` | consulta answer text / number |
| slot 1098 → `g_votoDigitado` (@1833288) | the digits typed so far (grey boxes) |
| slot 1099 → `DS_CandidatoNaoConcorre` (func 13101) | "CANDIDATO NÃO CONCORRE" / "CANDIDATA NÃO CONCORRE" (sexo 2 = feminino); throws 9333 if no candidate is selected |
| `CCandidaturasDSNumero`, `CCandidaturasDSNome{0}`, `CCandidaturasDSFoto{i}` | number, name and photo *i* (0 = titular) of the current candidate |

Logical screen 640×480. Fonts: {40} digit boxes / big messages, {30 bold} cargo name, {25} names, {20} small
texts, {35} consulta question, {13} photo captions.

### 5.2 Screen catalogue (the functions of this unit)

| builder (line) | used for | content (positions x,y) |
|---|---|---|
| `adicionaNomeCargo` (func 1192) | every screen | cargo name at (10,60); two lines (10,60)/(10,95) up to x=639 when the cargo shows photos |
| `adicionaNumeroDigitado` (3060) | nulo/legenda/inexistente/inapto | grey boxes with the typed digits at (10,115) |
| `adicionaPartido` (2028) | legenda, inexistente, tela completa | party line at (10,250), only if `CConfiguracaoEleicao +484` |
| `adicionaDadosCandidato` (4185) | tela completa | cargo (candidate's gender) at (10,60)[/(10,95)], number boxes (10,115), candidate name (10,190)/(10,215) |
| `adicionaFotoCandidato` (4184) | tela completa | photo 161×225, top-right at (640,36); optional caption (cargo in candidate's gender) centred below |
| `adicionaFotoSuplente` (4181) | tela completa com vice/suplentes | photo 111×155, top-right at (x+1,288), caption centred below (font 13) |
| `adicionaPerguntaConsulta` (3074) | consultas | the question (`'|'` = new line) centred in (10,50)-(629,255); returns bottom+23 |
| `adicionaBaseTelaCompletaCandidatoCom0` (:822, 6678) | candidate found, 0 running mates | header + party (except cargo code 25) + photo without caption |
| `…Com1` (:853, 6671) | 1 running mate (vice) | + running mate name (10,320)/(10,340) + captioned photos |
| `…Com2` (:890, 6660) | 2 running mates (Senate) | + names (10,310)/(10,330) and (10,355)/(10,375) + up to two small photos, right to left |
| `adicionaBaseTelaCompletaConsulta` (:942, 6652) | consulta answer found | question + number boxes at (100−40·digits, y) + answer text at (100, y+3) |
| `adicionaBaseTelaVotoBrancoCandidato` (:964, 6646) | blank vote | cargo + blinking **"VOTO EM BRANCO"** at (195,200) |
| `adicionaBaseTelaVotoBrancoConsulta` (:993, 6641) | blank consulta | question + blinking "VOTO EM BRANCO" at (320, y) |
| `adicionaBaseTelaVotoNuloConsulta` (:1014, 6624) | null consulta | question + boxes + text + blinking **"VOTO NULO"** at (320,345) |
| `adicionaBaseTelaVotoNuloCandidato` (:1049, 6608) | null vote | cargo + boxes (digits − n) + text at (10,190) + blinking "VOTO NULO" at (195,345) |
| `adicionaBaseTelaCandidatoInexistente` (:1075, 6605) | proportional: unknown number, valid party | cargo + boxes + **"CANDIDATO INEXISTENTE"** + party + blinking **"VOTO DE LEGENDA"** |
| `adicionaBaseTelaCandidatoInapto` (:1101, 6603) | proportional: candidate not running | cargo + boxes + "CANDIDATO/A NÃO CONCORRE" + blinking "VOTO NULO" at (320,345) |
| `adicionaBaseTelaVotoLegenda` (:1126, 6602) | proportional: party-only vote | cargo + boxes + party + blinking "VOTO DE LEGENDA" |
| `CriaTelaVotoCargoSemCandidato` (:1585, 3064) | cargo without candidates | cargo + blinking **"NÃO HÁ CANDIDATOS CONCORRENDO"** at (320,200), line (0,400)-(639,401), "Aperte a tecla:" (1,405), "CONFIRMA" right-aligned + " para continuar" left-aligned at (130,430); form name "telaVotoCargoSemCandidato" |
| lambda `$_2` of `CriaTelaVisualizacaoCandidato` (:3366, 12272) | operator's "visualizar candidatos" | for each running mate: photo 111×155, caption, "<cargo>: <name>" row, "Gênero: masculino/feminino/não informado" row; throws 9356 "Sem suporte a mais de 2 suplentes" |

The `adicionaBase*` builders check the cargo kind first and throw `CBaseError<vota::EUeVotaError, {9300,9500}>`
with the message `"<tela> - cargo <código> (<nome>) - não é de candidato | não é de proporcional | não é de
consulta | com número de suplentes incompatível"` (func 690). Codes: 9333 (:294) … 9357 (:3502), listed in
`ctelasvota.h`; 9343 is unused because `adicionaBaseTelaVotoNuloConsulta` throws 9344 like
`adicionaBaseTelaVotoNuloCandidato`.

Blinking texts are `api::CTextFieldBlinking` (colours 2/3 on 1, 500 ms timer from `ITimerScheduler`). Strings are
stored in Latin-1 (e.g. `"NÃO HÁ"` = `4E C3 4F 20 48 C1`), so the original sources are ISO-8859-1 encoded.

---

## 6. Restart and zerésima states (fragments)

**`CReinicioVotacao::StartState`** (func 11835) decides what to show after a restart:

```
if RDV comparecimento (max over eleições, func 1269) != 0  or  CJustificador count (+8) != 0:
    log "Apresentada tela do reinício da votação"; show CTelasVota+148   (mesário confirms: ProcessInput 11834
                                                                          logs "Mesário confirmou o reinício da votação")
elif EhTreinamentoEleitor():
    next = exists(<trab MI>/ze.dat) ? CQuerReimprimirZeresima : CQuerImprimirZeresima
elif EstadoGeralVota.urnaIdGerouZeresima present and != CEstadoGeral id (+60):
    next = CConfirmaRegerarZeresima          (the zerésima was produced by another urna)
else:
    next = CQuerReimprimirZeresima
```

`testeteclado::CRetomada` (keyboard test before resuming) returns `CReinicioVotacao` from slot 12 when skipping the
test was allowed. `CGeraZeresima`/`CRegerarZeresima` return the matching "resumo" state from slot 10 (merged
singleton body func 6056). All these states take their screen from `CTelasVota` in their constructors, which is
why the tools grouped them with this unit. `CProgressoEncerramento::Avanca` (func 12101) steps the progress bar
of the *encerramento* screen under its mutex (same `CProgressBar::Incrementa`, func 5508).

---

## 7. What is specific to the web build

* **Voter thread not started.** `CExecucaoVotaCooperativa` replaces thread start/join; `votaTick` → IExecucaoVota
  slot 7 → `CThreadEleitor::Processar()` once per tick. `Run`/`emscripten_sleep` are dead (§3.4).
* **Vote persistence replaced.** `CSincronismoVotoEleitorWeb::SincronizaVoto()` returns `true`; the real
  RDV/`vota.bin`/`uenux.db` writing, signing and verification of §4.2 are compiled in but unreachable.
* **"Gravando" bar animated by timers** in `votaTick` (4 × `AvancaBarraProgresso`, 120 ms then 160 ms apart),
  then message 5 is posted by the adapter instead of the operator thread.
* **Audio mode** set by `votaInit` from the page option `audioEleitorHabilitado` (message 9 or direct mode 2).
* **Treinamento eleitor**: the scenarios run in phase `'3'` with `treinamentoEleitor = TRUE`, so
  `EhTreinamentoEleitor()` (func 697) is true and every voter-database branch (uenux.db, `uenux.vsu`) would be
  skipped even by the real code.
* Syslog/`DEBUG_UENUX` logging of this file is silent in the browser.

## 8. Notable wasm / Emscripten observations

* **Inlined srcloc misleads names.** func 4349 carries the srcloc of the inlined `ProcessarEntrada(CAppStateContext&)`;
  the outer function has no such parameter (wasm signature `(i32)->i32`). funcs 4181/4184 carry the srcloc of the
  inlined `adicionaFotoEmoldurada<CCandidaturasDSFoto>` (:487, the `IScreen::GetInst()` call) but are the callers
  that add the photo captions. func 7061 was named only by vtable (`vf2`, now curated as `CThreadEleitor::Run`)
  while its srclocs belong to `IAjusteInicial::GetInst` (cajusteinicial.cpp:57) and the CPolySingletonList templates.
  func 7787 (formerly shown by the tools as `CHKDFSeed::GetSeed`, now `vota::CInformacaoEleitor::Inicializar`, name
  inferred in u02) is the start-up function that contains the whole `CTelasVota` constructor.
* **merge-similar-functions**: func 2921 (logging helper of 4 TUs, priority + cache as parameters), func 3921
  (3 × `GetTelaCargoAtual(const std::string&)`, srcloc + error code as parameters), func 6050 (2 × `GetTelaCargoAtual()`),
  func 6056 (2 singleton getters, mutex + pointer + vtable as parameters), func 253 → 710 (every
  `CBaseError<E>` constructor takes its vtable as a parameter), func 2902 (destructors of states holding one screen).
* **Constant propagation removed parameters**: `CriaTelaVotoCargoSemCandidato(const CCargo&, TPosition)` has one
  wasm parameter besides the result pointer (the `TPosition` is gone); `adicionaBaseTelaVotoNuloCandidato` lost its 3 `TPosition` and the `bool`; `adicionaBaseTelaVotoBrancoCandidato`
  lost its `TPosition`; `adicionaFotoEmoldurada<CCandidaturasDSFoto>` was specialised twice (x/y/w/h constants).
  The `CTextFieldDoubleLine` constructor (func 3659) is called with one colour less than its srcloc signature (its first
  colour is stored as the constant 2), and `CTextFieldMultiLine` (func 5498, wasm `(i32,i32,i32,i32)`) with both
  colours removed.
* **Single-threaded residue**: `std::lock_guard` shows only as the `mutex::unlock` stub (func 150); `CSynchronizer::Sync()`
  (func 620) compiled to a dead load; `condition_variable::wait` loops in the inlined poly-singleton code.
* **Static-initialiser calls**: `__wasm_call_ctors` calls `CPolySingletonList::contains<IAjusteInicial>` (2450) and
  `contains<ISincronismoVotoEleitor>` (2741) and discards the results (probably `static const bool` registration
  checks in cajusteinicial.cpp / csincronismovotoeleitor.cpp).
* Function-table slots are used as data sources (`CDataText<std::string(*)()>` stores the slot number: 1088–1100).

## 9. Suspicious or noteworthy code (details in the StructuredOutput "suspicious" list)

1. `CThreadEleitor::Run` (7061) calls `emscripten_sleep(50)` on every idle cycle, and on every cycle its
   `m_pImpl->Yield()` reaches `CWasmThread::Yield` (9651), which calls `emscripten_sleep(0)`. This build has no
   Asyncify (the glue's `_emscripten_sleep` is an `abort`), so `Run` would abort on its first cycle. It is unreachable
   in the shipped page only because `main()` registers the cooperative policy.
2. The simulator's "Gravando" screen is an animation: `SincronizaVoto` is `return true`; nothing is recorded or signed.
   Re-checked on 2026-09-23 with `headless.mjs --dump-fs` (municipal-t1, Vereador 91001 + Prefeito 12): the only file
   that differs from a run without a vote is `dsk/fi/dinamico/log/logd.dat`, and it has no "O voto do eleitor foi
   computado" record (the log line the real `SincronizaVoto` writes).
3. `LogSistema` (2921) prints an uninitialised 140-byte stack buffer (sp+16..sp+155) as prefix when `DEBUG_UENUX` is
   set and `/dev/urna` is absent. Latent: the page and the glue never set `DEBUG_UENUX`.
4. Error code 9344 is used by two different builders (`…VotoNuloConsulta` :1014 and `…VotoNuloCandidato` :1049), and
   9343 appears nowhere as an error code. Only the number is ambiguous: the srcloc line and the message ("não é de
   consulta" / "não é de candidato") still identify the builder.
5. Fallback caption positions in 4181/4184 are on the wrong side of right-anchored photos (x + w/2 instead of x − w/2;
   (719,261) is off-screen). Dead in practice: `CWasmImageSurfaceOps::CalcRect` (func 9289) always returns a w+1 wide
   rectangle, so the `largura <= 1` branch never runs.
6. MV signatures are copied from the MI instead of being recomputed (§4.2); correctness depends on deterministic RDV
   encryption (not verified here).
7. No rollback between the MI and MV halves of `SincronizaVoto`: a 9301 failure leaves the vote committed on the MI only.
8. Voter-queue messages 0 and 6 stop the voter thread; in the cooperative build nothing restarts it (`CThread::Start`
   is the only code that clears `m_bParar`, and every later `votaTick` step would return `false`). Latent: in the web
   build only messages 1, 5 and 9 are posted (funcs 11026, 10376, 11100 through func 3903).
9. `adicionaDadosCandidato` (4185) gives the two-line cargo title a right limit of `xLimiteNome + 237`: 707 when
   `…Com1`/`…Com2` show the titular's photo (they pass 470) and 876 when `…Com0` does (it always passes 639). Both
   limits are past the right edge of the screen (639), so the title is never wrapped before the 161-px photo at
   x 479..640. Other double-line fields use limits inside the screen.
10. The running-mate count checks disagree: `…Com2` (6660) only rejects `qtd <= 1`, so it would accept 3 or more,
    while the lambda of `CriaTelaVisualizacaoCandidato` (12272) throws 9356 "Sem suporte a mais de 2 suplentes".
    Harmless in this build: the only caller (the constructor in func 7787) dispatches on exactly 0, 1 or 2 and throws
    9357 "Tipo de cargo nao identificado" for any other count.

## 10. Complete mapping table (all 65 functions of u07)

"ran" = observed executing during the recorded votes.

| # | wasm func | size | ran | reconstructed symbol | src file / disposition |
|---|---|---|---|---|---|
| 1 | 253 | 18 |  | `ecourna::api::exception::CBaseError<vota::EUeVotaError, SErrorLimits{9300,9500}>::CBaseError(int, std::string&&, const std::source_location&)` (= `vota::CUeVotaError`) | library/inlined helper: merge-similar thunk into the shared ctor body 710 (vtable @1532440 as extra arg); thrown by every error path of this unit |
| 2 | 407 | 24 |  | `vota::CTelasVota::GetInst()` (ctelasvota.cpp:3434) | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 3 | 426 | 658 | yes | `api::CFormBuilder::Add(std::shared_ptr<IFormField<IScreen>>)` (unique name "<Classe><n>" + push_back) | src/uenux2/src/api/gui/cformbuilder.u07.cpp |
| 4 | 690 | 735 |  | `vota::(anon)::MensagemErroCargo(nomeTela, cargo, motivo)` | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 5 | 1157 | 36 |  | `comum::md::CCargo::GetQtdSuplentes() const` | src/uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.u07.cpp |
| 6 | 1191 | 292 | yes | `api::CFormBuilder::Add<CTextField>(SPoint, make_shared<CDataText<std::string(*)()>>, SFont, 2, 1)` instantiation | library/inlined helper (template of cformbuilder.h); summarised in src/uenux2/src/api/gui/cformbuilder.u07.cpp |
| 7 | 1192 | 1027 | yes | `vota::(anon)::adicionaNomeCargo(const CCargo&, CFormBuilder&)` | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 8 | 1546 | 16 |  | `comum::md::CCargo::PossuiFoto() const` | src/uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.u07.cpp |
| 9 | 1952 | 12 |  | `vota::CGeraResumoZeresimaBase::~CGeraResumoZeresimaBase()` (D1, via shared body 2902) | src/uenux2/src/app/vota/eleitor/iniciovotacao/cgerazeresima.u07.cpp (path inferred) |
| 10 | 2028 | 502 | yes | `vota::(anon)::adicionaPartido(CFormBuilder&)` | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 11 | 2072 | 16 | yes | `vota::(anon)::LogDebug(const char*, ...)` (LOG_DEBUG, cache @1534908) | src/uenux2/src/app/vota/eleitor/cthreadeleitor.cpp |
| 12 | 2369 | 11 | yes | `vota::CTelasVota::AvancaBarraProgresso()` | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 13 | 2438 | 166 |  | `vota::CThreadEleitor::~CThreadEleitor()` (vtable slot 0) | src/uenux2/src/app/vota/eleitor/cthreadeleitor.cpp |
| 14 | 2450 | 19 | yes | `api::CPolySingletonList::contains<vota::IAjusteInicial>(TPolySingletonsInfo&)` | library/inlined helper: template instance (cpolysingletonlist.h) used by IAjusteInicial::GetInst inlined in Run; also called (result discarded) by __wasm_call_ctors |
| 15 | 2782 | 343 | yes | `api::CFormBuilder::Add<CTextFieldMultiLine>(SRect, make_shared<CFixedText>, SFont)` instantiation | library/inlined helper (template); summarised in src/uenux2/src/api/gui/cformbuilder.u07.cpp |
| 16 | 2921 | 118 | yes | `LogSistema(prioridade, int& urnaReal, fmt, va_list)` - merged body of a header helper (access("/dev/urna") -> vsyslog, else DEBUG_UENUX -> printf) | src/uenux2/src/app/vota/eleitor/cthreadeleitor.cpp (header of origin unknown; also serves cthreadoperador.cpp, cvalidamidia.cpp, 5895) |
| 17 | 3060 | 294 | yes | `vota::(anon)::adicionaNumeroDigitado(form, digitos, pos)` (= Add<CMaskedTextField<CGrayedFramedText>>) | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 18 | 3064 | 1307 | yes | `vota::CTelasVota::CriaTelaVotoCargoSemCandidato(const CCargo&, TPosition)` (:1585) | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 19 | 3068 | 294 |  | `api::CFormBuilder::Add<CTextFieldDoubleLine>(SPoint, SPoint, TPosition, make_shared<CDataText<std::string(*)()>>, SFont, TColor)` instantiation | library/inlined helper (template); summarised in src/uenux2/src/api/gui/cformbuilder.u07.cpp |
| 20 | 3074 | 428 |  | `vota::(anon)::adicionaPerguntaConsulta(form, cargo)` -> TPosition | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 21 | 3921 | 97 | yes | merged body of `CConfirmaVotoEmCargo::GetTelaCargoAtual(const std::string&)` / `CConfirmaVotoSemCandidato::` / `CCompletaProporcional::` (srcloc + code as extra args) | src/uenux2/src/app/vota/eleitor/cconfirmavotoemcargo.cpp (written by u06) |
| 22 | 4129 | 280 |  | `api::CFormBuilder::Add<CImageField>(SPoint, make_shared<CDataImage<comum::CCandidaturasDSFoto>>, anchor 1)` instantiation | library/inlined helper (template); summarised in src/uenux2/src/api/gui/cformbuilder.u07.cpp |
| 23 | 4181 | 427 | yes | `vota::(anon)::adicionaFotoSuplente(form, legenda, x, indice)` -> SRect (contains `adicionaFotoEmoldurada<CCandidaturasDSFoto>` :487 inlined) | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 24 | 4184 | 610 | yes | `vota::(anon)::adicionaFotoCandidato(form, comLegenda)` (contains `adicionaFotoEmoldurada<CCandidaturasDSFoto>` :487 inlined) | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 25 | 4185 | 937 | yes | `vota::(anon)::adicionaDadosCandidato(form, cargo, xLimiteNome)` | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 26 | 4349 | 1427 | yes | `vota::CThreadEleitor::Processar()` (outer; inlines ProcessarMensagens, ProcessarEntrada(CAppStateContext&) :134, ProcessarTicks) | src/uenux2/src/app/vota/eleitor/cthreadeleitor.cpp |
| 27 | 4682 | 549 | yes | `vota::CopiaAssinaturaBancoDadosParaMV()` (uenux.vsu MI->MV under a CApplicationContextGuard) | src/uenux2/src/app/vota/comum/csincronizavota.u07.cpp |
| 28 | 5508 | 110 | yes | `api::CProgressBar::Incrementa()` | src/uenux2/src/api/gui/cprogressbar.u07.cpp |
| 29 | 5535 | 25 | yes | `api::CGrayedFramedText::CGrayedFramedText(size_t, const SPoint&)` | src/uenux2/src/api/gui/cgrayedframedtext.u07.cpp |
| 30 | 5938 | 142 |  | `vota::CReinicioVotacao::GetInst()` (ctor inlined) | src/uenux2/src/app/vota/eleitor/iniciovotacao/creiniciovotacao.u07.cpp (path inferred) |
| 31 | 5940 | 142 |  | `vota::CQuerReimprimirZeresima::GetInst()` (ctor inlined) | src/uenux2/src/app/vota/eleitor/iniciovotacao/cquerreimprimirzeresima.u07.cpp |
| 32 | 5957 | 142 |  | `vota::CQuerImprimirZeresima::GetInst()` (ctor inlined) | src/uenux2/src/app/vota/eleitor/iniciovotacao/cquerimprimirzeresima.u07.cpp (path inferred) |
| 33 | 5965 | 68 |  | `vota::CGeraZeresimaBase::CGeraZeresimaBase()` | src/uenux2/src/app/vota/eleitor/iniciovotacao/cgerazeresima.u07.cpp (path inferred) |
| 34 | 6050 | 88 | yes | merged body of `CPedeMajoritario::GetTelaCargoAtual()` / `CPedeProporcional::GetTelaCargoAtual()` | src/uenux2/src/app/vota/eleitor/votamajoritario/cpedemajoritario.u07.cpp |
| 35 | 6056 | 75 |  | merged body of `CGeraResumoZeresima::GetInst()` / `CRegeraResumoZeresima::GetInst()` (mutex, instance, vtable as args) | src/uenux2/src/app/vota/eleitor/iniciovotacao/cgerazeresima.u07.cpp (path inferred) |
| 36 | 6528 | 295 |  | `api::CFormBuilder::Add<CTextField>(SPoint, make_shared<CDataText<comum::CCargoDSNomeSexoCandidato>>, SFont, 2, 1)` instantiation | library/inlined helper (template); summarised in src/uenux2/src/api/gui/cformbuilder.u07.cpp |
| 37 | 6561 | 1320 |  | `vota::(anon)::adicionaFotoEmoldurada<std::__bind<lambda@3323&, const CDadosCandidato&>>(form, x, y, w, h, fonte)` (:487) | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 38 | 6602 | 374 | yes | `vota::(anon)::adicionaBaseTelaVotoLegenda` (:1126) | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 39 | 6603 | 436 |  | `vota::(anon)::adicionaBaseTelaCandidatoInapto` (:1101) | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 40 | 6605 | 557 | yes | `vota::(anon)::adicionaBaseTelaCandidatoInexistente` (:1075) | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 41 | 6608 | 473 | yes | `vota::(anon)::adicionaBaseTelaVotoNuloCandidato` (:1049) | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 42 | 6624 | 729 |  | `vota::(anon)::adicionaBaseTelaVotoNuloConsulta` (:1014) | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 43 | 6641 | 272 |  | `vota::(anon)::adicionaBaseTelaVotoBrancoConsulta` (:993) | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 44 | 6646 | 264 | yes | `vota::(anon)::adicionaBaseTelaVotoBrancoCandidato` (:964) | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 45 | 6652 | 577 |  | `vota::(anon)::adicionaBaseTelaCompletaConsulta` (:942) | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 46 | 6660 | 514 | yes | `vota::(anon)::adicionaBaseTelaCompletaCandidatoCom2` (:890/:895) | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 47 | 6671 | 363 | yes | `vota::(anon)::adicionaBaseTelaCompletaCandidatoCom1` (:853/:858) | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 48 | 6678 | 192 | yes | `vota::(anon)::adicionaBaseTelaCompletaCandidatoCom0` (:822/:827) | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 49 | 6743 | 9 |  | `vota::CInformacaoEleitor::HabilitaAudio()` (m_modoAudio = 1) | src/uenux2/src/app/vota/eleitor/comum/cinformacaoeleitor.u07.cpp |
| 50 | 6745 | 9 |  | `vota::CInformacaoEleitor::HabilitaAudioConformeCadastro()` (m_modoAudio = 0) | src/uenux2/src/app/vota/eleitor/comum/cinformacaoeleitor.u07.cpp |
| 51 | 7030 | 18 |  | `vota::CThreadEleitor::FinalizaExecucao()` (vtable slot 5) | src/uenux2/src/app/vota/eleitor/cthreadeleitor.cpp |
| 52 | 7061 | 4119 |  | `vota::CThreadEleitor::Run()` (vtable slot 2; IAjusteInicial::GetInst cajusteinicial.cpp:57 inlined) | src/uenux2/src/app/vota/eleitor/cthreadeleitor.cpp |
| 53 | 7070 | 13 |  | `vota::CThreadEleitor::~CThreadEleitor()` deleting destructor (slot 1) | src/uenux2/src/app/vota/eleitor/cthreadeleitor.cpp |
| 54 | 7122 | 38 |  | atexit destructor of `CThreadEleitor` static `std::unique_ptr` (@1833212) | src/uenux2/src/app/vota/eleitor/cthreadeleitor.cpp |
| 55 | 7174 | 3288 |  | `vota::impl::CSincronismoVotoEleitor::SincronizaVoto()` (slot 2; csincronizavota.cpp:290/:303 inlined) | src/uenux2/src/app/vota/eleitor/csincronismovotoeleitor.cpp (+ comum/csincronizavota.u07.cpp for the inlined parts) |
| 56 | 11829 | 19 |  | `vota::testeteclado::CRetomada::GetEstadoSemTeste()` (slot 12) | src/uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cretomada.u07.cpp (path inferred) |
| 57 | 11835 | 712 |  | `vota::CReinicioVotacao::StartState()` (slot 2) | src/uenux2/src/app/vota/eleitor/iniciovotacao/creiniciovotacao.u07.cpp (path inferred) |
| 58 | 11841 | 22 |  | `vota::CRegerarZeresima::GetEstadoResumo()` (slot 10) | src/uenux2/src/app/vota/eleitor/iniciovotacao/cgerazeresima.u07.cpp (path inferred) |
| 59 | 11847 | 38 |  | atexit destructor of the `CRegeraResumoZeresima` instance (@1834884) | src/uenux2/src/app/vota/eleitor/iniciovotacao/cgerazeresima.u07.cpp (path inferred) |
| 60 | 11935 | 22 |  | `vota::CGeraZeresima::GetEstadoResumo()` (slot 10) | src/uenux2/src/app/vota/eleitor/iniciovotacao/cgerazeresima.u07.cpp (path inferred) |
| 61 | 11942 | 38 |  | atexit destructor of the `CGeraResumoZeresima` instance (@1834184) | src/uenux2/src/app/vota/eleitor/iniciovotacao/cgerazeresima.u07.cpp (path inferred) |
| 62 | 11945 | 68 |  | `vota::CGeraResumoZeresimaBase::CGeraResumoZeresimaBase()` | src/uenux2/src/app/vota/eleitor/iniciovotacao/cgerazeresima.u07.cpp (path inferred) |
| 63 | 12101 | 18 |  | `vota::CProgressoEncerramento::Avanca()` (slot 4) | src/uenux2/src/app/vota/eleitor/fimvotacao/cprogressoencerramento.u07.cpp (path inferred) |
| 64 | 12272 | 1126 |  | operator() of lambda `$_2` in `CTelasVota::CriaTelaVisualizacaoCandidato` (:3366; recursive std::function over suplentes) | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp (as commented excerpt; the enclosing function is u09's func 6569) |
| 65 | 13101 | 217 |  | `vota::(anon)::DS_CandidatoNaoConcorre()` (:294; table slot 1099) | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |

Functions of these files that are **not** in u07 but belong to them: func 316 `CThreadEleitor::GetInst` (u18,
reproduced in cthreadeleitor.cpp), `ISincronismoVotoEleitor::GetInst` (inlined in func 7181, u19), the
`CTelasVota` constructor/`CreateInst`/`CriaTelaInputVazio*`/`CriaTelaPartido`/`CriaTelaZeresimaTardia`
(inlined in func 7787, u02), `CTelasVota::GetTelaCargo` (inlined in func 4135, u06),
`CriaTelaVisualizacaoCandidato` (func 6569, u09) and `adicionaLinha` (func 1588, u09).

## 11. Open questions

* Exact names of the outer cycle (func 4349), of `SincronizaVoto` and of the `IExecucaoVota` slots (no srcloc).
* Meaning of voter-queue messages 0 vs 6 (both stop the thread) and of message 13 on the operator side.
* The content of the 140-byte prefix buffer of `LogSistema` on Linux (thread name? `prctl(PR_GET_NAME)`?).
* Which `CConfiguracaoEleicao` field is +484 (party line shown) and why cargo code 25 hides the party in `…Com0`.
* Semantics of `CDetalheSuplente +1` (photo flag) and of the two strings of `CDadosCandidato` (+0, +12).
* Whether RDV encryption is deterministic (needed for the MV signature copy to validate).
