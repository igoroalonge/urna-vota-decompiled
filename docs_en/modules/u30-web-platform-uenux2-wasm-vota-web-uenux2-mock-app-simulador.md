# u30: web platform - `uenux2/wasm/vota_web` + `uenux2/mock/app/simulador`

Unit u30 has 109 wasm functions (36 of them ran in the recorded votes). They are the **browser side of the
simulator**: code that does not exist on the urna (the Brazilian electronic voting machine) and replaces its
process start-up, its hardware and its persistent state:

* the **C API that JavaScript drives** (`main`, `votaTick`, `votaPressKey`, `votaGetStateJson`,
  `votaSetAudioEnabled`) and the helpers of `votaInit` that live in this unit (option-JSON readers, the
  "simulated signature" writer, the error reporter);
* two **hardware mocks**: `simulador::CWasmBeep` (the urna's buzzer as Web Audio tones) and
  `simulador::(anonymous)::CEsperaAudioWasm` (a cancellable handle on an asynchronous "wait for the end of the
  speech");
* most of **`comum::teste::CAppInfoBuilder`**, a *test fixture* of the urna code base that the web build runs
  in production to fabricate the urna's state files (`eg.bin`, `gap.bin`, `sa.bin`, `vota.bin`) on first start;
* 62 functions that the tools put here only because of table neighbourhood or because `votaInit` calls them.
  They are identified below: libc++ (13), SQLite (5), OpenSSL (14), sort comparators of the cargo lists (3),
  compiler-generated `atexit` destructors of singletons (22), and small functions of `comum` classes (5).

The 47 TSE functions of the unit proper: 15 in `vota_web_wasm.cpp`, 11 in the simulator mocks, 21 in the
builder (`cappinfobuilder.cpp`).

Other units cover the rest of the same files: u28 (`CWasmSavd`, `CPoliticaExecucaoEleitorWeb`, `SalvaGeral`,
`SalvaApps`), u29 (`CVotaWebEngine`, `votaInit`, the state JSON), u31 (`CWasmScreen`, `CWasmWebSound`,
`CWasmInputKbd`, ...), u19 (`CSimuladorWasm::Executa` and the poly-singleton registry). The page protocol is
documented in [`docs/03-js-wasm-interface.md`](../03-js-wasm-interface.md); this chapter does not repeat it.

Reconstructed sources:

```
src/uenux2/wasm/vota_web/vota_web_wasm.u30.cpp        main, votaTick (CVotaWebEngine::Tick), votaPressKey,
                                                     votaGetStateJson, votaSetAudioEnabled (SetAudioEnabled :592),
                                                     JsonBool/JsonInt/JsonString, WriteFileIfMissing,
                                                     WriteSimulatedSignatures, ReportError, voter-queue thunks
src/uenux2/mock/app/simulador/wasm/cwasmbeep.{h,cpp}  simulador::CWasmBeep (whole class)             (path inferred)
src/uenux2/mock/app/simulador/wasm/cwasmwebsound.u30.cpp   CEsperaAudioWasm + the waits map        (path inferred)
src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp     CAppInfoBuilder ctor, setters, Salva, CriaEstado/Copia,
                                                     fixture defaults, AssinaturaFicticia, EscreveArquivo
src/uenux2/src/app/comum/u30-foreign-fragments.cpp    CAppInfo::CarregaGeral, CEstadoGeralVota::SetEstadoVota,
                                                     IServicoEstado<..>::Salva x2, CServicoEstadoGeralSA ctor,
                                                     the 3 cargo comparators, comum singleton atexit dtors
src/uenux2/src/app/vota/u30-foreign-fragments.cpp     vota singleton atexit dtors (table)
```

## 1. Glossary

| term | meaning |
|---|---|
| urna / UE | the voting machine (*urna eletrônica*); models UE2013..UE2022 |
| VOTA | the election-day application of the urna (voter terminal + poll-worker terminal) |
| eleitor / mesário | voter / poll worker. The mesário "releases" (*habilita*) the urna for each voter |
| carga | the loading of an urna with the election data before election day, from a *flash de carga* (FC) |
| MI / MV (flash interna / externa) | the internal flash (`/dsk/fi/`) and the removable voting card (`/dsk/fe/`); every state file exists on both |
| `dinamico/`, `trab1/`, `trab2/` | the dynamic (writable) area and the work directories of the 1st and 2nd round (*turno*) |
| `eg.bin`, `gap.bin`, `sa.bin`, `vota.bin` | persistent state: *estado geral* (general), GAP (application launcher), SA (manual counting system), VOTA; BER-encoded `ModuloEstadoGeral*` |
| `.vsu` | signature package of a file, produced on the urna by the SAVD (signature service) with the HSM |
| fase | `'1'` oficial, `'2'` simulado, `'3'` treinamento (training); selects the data-file prefix `o`/`s`/`t` |
| zerésima | the report printed before voting starts, proving the vote counters are zero |
| BU / RDV | *Boletim de Urna* (per-section result, printed and recorded) / *Registro Digital do Voto* (shuffled record of every vote) |
| encerramento | closing of the section by the mesário: generation of BU, RDV and other result files |

## 2. Classes and hierarchy (RTTI)

```
api::IBeep                                   (class, typeinfo @1531756; destructor declared LAST: slots 7/8)
  └─ simulador::CWasmBeep                    (si, typeinfo @1530348, vtable @1530312, 4 bytes)

api::IEsperaAudio                            (class, typeinfo @1528612)       "wait for the end of the audio"
  ├─ api::CEsperaAudio                       (si, vtable @1528584, 8 bytes: +4 bool cancelled)   (u31/api)
  └─ simulador::(anonymous)::CEsperaAudioWasm(si, typeinfo @1528684, vtable @1528668, 12 bytes:
                                                +4 int id, +8 bool cancelled), created with make_shared
                                                (control block vtable @1528628)

comum::teste::CAppInfoBuilder                (no RTTI, 500 bytes)             test fixture, mock/app/comum
    +0   md::CEstadoGeral      (180)   -> dinamico/eg.bin
    +180 md::CEstadoGeralGap[2] (44)   -> trab1|2/gap.bin
    +268 md::CEstadoGeralSA[2]  (16)   -> trab1|2/sa.bin
    +300 md::CEstadoGeralVota[2](100)  -> trab1|2/vota.bin

(anonymous)::CVotaWebEngine                  (no RTTI, 40 bytes, unit u29): the state kept between API calls
```

`IBeep` slot names are inferred from the callers (the destructor occupies the last two slots, 7 and 8):

| slot | CWasmBeep | tones (Hz x 1/100 s) | callers |
|---:|---|---|---|
| 0 | `SetVolume(int)` 8541 | `js_wasm_beep_set_volume(v)` | none found |
| 1 | `Beep(freq, dur)` 8538 | `js_wasm_beep_queue(freq, dur*10)` | melodies below (virtually), `CExibeAlertaDesligamento` (2000, 100), `EnterLoopBeeping` (800, n) |
| 2 | `BeepConfirma(int n)` 8533 | n x {2300x10, 2200x10} | `CEleitorVotando::NeedChangeState` (1: cargo confirmed), `CFimVotoEleitor::StartState` (4: the "FIM" beep) |
| 3 | `BeepAlerta()` 8523 | 392x50, 370x50, 349x50, then 3 x {329, 334, 329, 324}x10, 329x10 ("sad trombone") | `CVerificaEleicaoPassou::StartState` (election date invalid) |
| 4 | `BeepFinalizacao()` 8514 | 17 notes: 1244 830 1046 1244 / 1661 1568 1661 rest / 1864 1661 1864 rest / 2489 2349 2489 2349 2489x40 | `CThreadMonitor::SaiPorVotacaoSuspensa` (async), `CInformacaoZeresimaTardia::ProcessInput` |
| 5 | `BeepSuspensao()` 8505 | 2489 2349 1864 1568 1244 1174 (x11), 1244x69 | `CEleitorVotando::DescartaVotos` |
| 6 | `BeepErro()` 8494 | 554x20 (C#5, 200 ms) | refused keys: `CVotacaoStateAudio::PlayKey`, `CInputMenuField::Read`, `CMenuValidation` |

The tone tables are the i64 constants stored into the `std::vector<{int,int}>` of each method; `BeepAlerta`
builds its vector with `push_back` (func 760, folded with RHVoice code) with the vibrato loop unrolled.

## 3. Control flow

### 3.1 `main` (func 10307)

`main` runs once, inside the glue's `callMain`, before the page adapter exists:

1. `js_use_external_keyboard()`; width/height from `js_ler_dimensao_tela(0, 1280)` / `(1, 800)` - JS reads
   `?screenWidth=`, `?screen=WxH` (also `resolution`, `resolucao`) or `Module.votaScreenWidth/Height`. The values
   are passed to `CSimuladorWasm` **narrowed to `short`** (`i32.extend16_s`).
2. `CSimuladorWasm simulador(10, "VOTA Web", w, h)` (func 8311, u19).
3. `CApplication::InitApplication("VOTA", "Software de Votação", "10.23.0.1 - DESENVOLVIMENTO", true)` - this
   version string is the one the BU and the logs carry (`urna.versaoVotacao`).
4. Registers the process services in `api::CPolySingletonList`, each through the register-or-replace wrapper
   (u19's `replace`: `exists` -> `erase` -> `push`; 10090/9507/9437/8728 start with that test; a bare `push`
   would throw 6756 on a duplicate), in this order: `IInterfaceSavd` = `CWasmSavd`
   (answers OK to every signing/validation request), `IRng` = `CPrng`, `ISymmetricCipherFactory`,
   `IFingerPrepare` = `CFingerPrepareSimulador`, `IAjusteDataHora::CreateInst()`, the four `IGenericFactory`
   (threads = `CWasmThread`, locks = POSIX wrappers over stubbed pthreads), `IPoliticaExecucaoEleitor` =
   `CPoliticaExecucaoEleitorWeb`, `ISincronismoVotoEleitor` = `CSincronismoVotoEleitorWeb` (does not write the
   RDV), `IExecucaoVota` = `CExecucaoVotaCooperativa(CAguardaMensagem::GetInst())`.
5. `simulador.Executa(lambda)` (func 8302): registers the hardware mocks (screen, keyboard, `CWasmBeep`, null
   printer...), then the lambda (func 10384) replaces `ISound` by `CWasmWebSound` and emits `vota:ready`.
6. Returns 0; `noExitRuntime` keeps the module alive.

Registering the web policies *before anything asks for them* is what keeps the urna defaults
(`CExecucaoVota` with real threads, `CPoliticaExecucaoEleitor` with `emscripten_sleep`, `CSincronismoVotoEleitor`
writing `rdv.dat`) from being created: their `GetInst()` only create a default when nothing is registered.

### 3.2 `votaTick` (func 10619 = `CVotaWebEngine::Tick`)

```
votaTick():
  if !initialized or done: return 0
  before = CurrentStateName()                          (RTTI name of the voter thread's state, demangled)
  IExecucaoVota::GetInst().Processa()                  (slot 7: one step of CThreadEleitor)
  after  = CurrentStateName()
  if after contains "CSincronismoEleitor":             ("Gravando..." screen)
      now = emscripten_get_now()
      first tick:  recStarted=1, step=0, deadline=now+120
      later:       if !recFinished and now >= deadline:
                      step <= 3 : CTelasVota::AvancaBarraProgresso(); deadline=now+160; step++
                      else      : post message 5 to the voter queue; recFinished=1
  else: reset the recording fields
  if after contains "CAguardaMensagem" and before != after: done = 1
  json = BuildStateJson()
  if before != after or json != lastJson: lastJson = json; emit vota:state
  if done and (same condition): emit vota:done (same JSON)
  return 1
  catch std::exception -> ReportError(what()), return 0;  catch ... -> ReportError("erro desconhecido em votaTick")
```

Timeline of the recording animation: bar steps at +120, +280, +440, +600 ms, message 5 at +760 ms. On the urna,
the order is the other way round: the *operator* thread sends message 5 first (`CSincronismoOperador::StartState`,
func 10210), and the voter thread then calls `SincronizaVoto` from `CSincronismoEleitor::ProcessMessage(5)`
(func 7181), which writes the RDV and `vota.bin` on both flashes and advances the bar (u07 §4). Here message 5
comes from `votaTick`, the same func 7181 calls `CSincronismoVotoEleitorWeb::SincronizaVoto` (`return true`),
nothing is written, and the bar is only a timer.

`done` is set on **any** transition back to `CAguardaMensagem` (a completed vote or a discarded one); after
that `votaTick` returns 0 forever, so each page load serves exactly one voter.

### 3.3 The other exports

* `votaPressKey(const char*)` (10703): creates the engine if needed, then `js_push_key(key)`: the key goes to
  `Module.uenuxKeys` and comes back through `CWasmInputKbd` on a later tick. No check of `initialized`.
* `votaGetStateJson()` (10171): `lastJson = BuildStateJson(); return lastJson.c_str()`. It overwrites the cache
  `votaTick` compares against (see §9).
* `votaSetAudioEnabled(int)` (10240): `CVotaWebEngine::SetAudioEnabled` (srcloc `vota_web_wasm.cpp:592`):
  `audioEnabled = a != 0; ISound::instance().Mute(!a)` (ISound slot 6 -> `js_wasm_web_sound_mute`).
* `ReportError(text)` (10857): `console.error(text)` and `vota:error` with `{"message":"<escaped>"}`.

### 3.4 The option JSON readers (14477, 11571, 11546)

`votaInit` receives `{"fase":"te","pe":2400,"turno":1,"uf":"ac","municipio":1,"zona":1,"secao":1,
"audioEleitorHabilitado":false,"reproduzirAudio":false}`. The three readers share one algorithm: find the first
occurrence of `"<key>"` **anywhere in the text**, then the next `:`, skip `" \t\r\n"`, and read:

| reader | value | default | quirks |
|---|---|---|---|
| `JsonBool(json, key, def)` 14477 | `compare(pos,4,"true")` / `compare(pos,5,"false")` | the argument | any other token keeps the default |
| `JsonInt(json, key)` 11571 | `strtol(p, &end, 10)` | 1 (constant: every caller passes 1) | no range check; negative accepted; `long` is 32-bit so it saturates at 2^31-1 |
| `JsonString(json, key, def)` 11546 | characters up to the next unescaped `"` | the argument | `\x` keeps `x` literally: JSON escapes (`\n`, `\u00e9`) are not decoded; unterminated -> default |

`votaInit` then truncates `zona` and `secao` to 16 bits and maps `fase` = `"oficial"`/`"o"` -> `'1'`,
`"simulado"`/`"s"` -> `'2'`, anything else -> `'3'`, `turno == 2 ? '2' : '1'`.

### 3.5 Fabricating the urna state: `CAppInfoBuilder`

On the first `votaInit` (when `<MI>/dinamico/eg.bin` does not exist; test = func 11661 + `exists`), `votaInit`
builds the fixture and saves it:

```
CAppInfoBuilder b(pe)                                   10268
 .SetFase(f).SetTurno(t).SetTipoUrna('1').SetLocal(m, z, s)   10197 10188 10181 10176
b.geral.uf = ToUpper(uf); b.vota[0..1] = {estado '1', treinamentoEleitor = true}      (inlined)
b.Salva(false, {MI, MV})                                10256
    SalvaGeral(false, {MI,MV})                          10243 (u28): eg.bin on MI and MV
    SalvaAppsPrimeiroTurno(false, {MI,MV}, TodosApps()) 10242 -> 6004 -> SalvaApps(.., '1', {Gap,SA,Vota})
    SalvaAppsSegundoTurno (false, {MI,MV}, TodosApps()) 10239 -> 6004 -> SalvaApps(.., '2', ...)
WriteSimulatedSignatures()                              11733
```

Every state object is **rebuilt through its md member-wise constructor** before being encoded
(`CriaEstado` 10205/10123/10101/10067, leaf helpers `Copia` 9863/10167/10155, plus u29's `Copia` 5324/9862 and
the probable `Copia(CAjusteDataHora)` 10162): that re-runs the md validations the
in-place defaults skipped (município < 100000, zona/seção < 10000, model 2013..2022, fase '1'..'3') and the
gap's `data2T` recomputation. Each file is written by `IServicoEstado<T, Conversor>::Salva` (merged body 2894:
`CFileASN::WriteToFile(GetPathArquivo(), Conversor().Converte(estado))`; instantiations 10089 SA, 10112 GAP in
this unit).

Fixture values (constructor 10268 and the default constructors 10261, 9952, 10278):

| field | value | notes |
|---|---|---|
| `estadoUrna` | `'3'` testada | the carga is complete and tested |
| `idPE` | `pe` from the options (15000 before the body runs) | with the fase letter it names the election files, e.g. `t02400-cp.dat` |
| `dadoLocal` | UF `"EE"` (then `ToUpper(uf)`), município/zona/seção 1/1/1 (then the options), tipo local 1 | |
| `dadoCarga` | turno `'1'`, tipo de urna T1/T2 `'1'` (seção), modelo **2015**, fase `'2'` (then the options) | |
| `ajusteDataHora` | {0, 0} | |
| `correspondencia` (the carga) | urna **87654321**, FC serial `"12345678"`, carga time **31/12/2020 23:59:58**, código de carga **`123456789012345678901234`**, localidade 1/1/1, assinatura = 139 fake bytes, gerador `{"nome_maquina", "12345678", "99999999"}` | the BU's `urna.correspondenciaResultado.carga` |
| `versao` | `"7.2.1.3 - TESTE EG ASN1"` | |
| `hashVersoesPacotes` | 64 bytes `0x0A` | |
| GAP (x2) | no correspondences, app/appAnterior 14 (none), flags false, 2nd-round date 31/12/2080 | the BU's `historicoCodigosCarga` comes from here: empty |
| SA (x2) | estado `'1'` inicial, not blocked, município/zona/**seção 1/1/2** | the seção differs from the geral's |
| VOTA (x2) | estado `'1'` inicial, encerramento `'1'`, all counters 0, no dates; `votaInit` sets `treinamentoEleitor` | then `votaInit` sets `EAVVOTAR` ('8', func 11266) and saves |

The fake signature (`AssinaturaFicticia`, 9963) is the hex text `123456789abcdefABCDEF` repeated to 278
digits and decoded: 139 bytes, the maximum DER size of an ECDSA P-521 signature.

Then, on both paths (fixture just written, or `eg.bin` already present), `votaInit` loads `eg.bin` into the
`CAppInfo` cache with `CAppInfo::CarregaGeral` (11572: `m_geral = CServicoEstadoGeral(MI).Carrega()`, also clears
CAppInfo+528).

### 3.6 Simulated signatures (`WriteSimulatedSignatures`, 11733, and `WriteFileIfMissing`, 11818)

Called four times by `votaInit`. For each flash (`fi`, `fe`): `dinamico/eg.vsu`, and for `trab1` and `trab2`:
`eg.vsu gap.vsu sa.vsu vota.vsu rdv.vsu uenux.vsu`. Content: the 38 bytes
`assinatura simulada para vota_web_wasm`. `WriteFileIfMissing` creates the parent directories and writes only
when the path does not exist; `votaInit` also uses it for `serialv.dat` = `"ABCDDCBA"` on both roots.
Nothing verifies these files: `CWasmSavd` answers "OK" to every `ValidarUE` (u28).

### 3.7 Asynchronous audio waits (`CEsperaAudioWasm`)

`CWasmWebSound::vf9` (u31) stores `{cancelado, terminado}` (two `std::function`) in a static
`std::map<int, SEspera>` (@1832664) under a fresh id and calls `js_wasm_web_sound_wait_async(id)`; JavaScript polls
every 20 ms through the exports `uenux_wasm_web_sound_wait_cancel_requested` (9614) and
`uenux_wasm_web_sound_wait_finished` (9604). The returned handle is a `CEsperaAudioWasm`:

* `Cancela()` (2680, slot 2): once only; erases the map entry **without calling `terminado`** and stops the JS
  poll (`js_wasm_web_sound_cancel_wait(id)`, sent even if the entry was already gone).
* `Cancelada()` (9445, slot 3): the flag.
* the destructors (9459, 9448) call `Cancela()`: dropping the handle cancels the wait.
  `CVotacaoStateAudio` keeps the handle and cancels the previous wait before starting a new one (func 7010).

## 4. Data read and written

| what | how | where |
|---|---|---|
| `/dsk/fi|fe/dinamico/eg.bin` | `ModuloEstadoGeralUrna.EstadoGeralUrna` via `CConversorEstadoGeral` | written once by the builder, read by `CarregaGeral` |
| `/dsk/fi|fe/dinamico/trab1|2/gap.bin, sa.bin, vota.bin` | `ModuloEstadoGeralGap/SA/Vota` | written by `SalvaApps` (u28) with this unit's `CriaEstado` |
| `.../*.vsu` | plain text | `WriteSimulatedSignatures` |
| `/dsk/fi|fe/serialv.dat` | `"ABCDDCBA"` | `WriteFileIfMissing` (from `votaInit`) |
| imports | `js_use_external_keyboard`, `js_ler_dimensao_tela`, `js_wasm_beep_init/queue/set_volume`, `js_wasm_web_sound_cancel_wait`, `emscripten_get_now` (+ `js_push_key`, `js_console_error`, `js_emit_event` through u29 helpers) | |
| exports | `Kb` main, `Hb` votaTick, `Gb` votaPressKey, `Jb` votaGetStateJson, `Ib` votaSetAudioEnabled | |
| DOM events | `vota:state`, `vota:done` (votaTick), `vota:error` (ReportError) | via `js_emit_event` |

No network, no SQL and no cryptography in this unit (the OpenSSL/SQLite functions listed in §7 are library code
that the tools misattributed).

## 5. Web-build specifics

* The C API, `main`, the fixture use and the signature placeholders exist only in the simulator.
* The operator (mesário) thread never runs. `votaTick` plays its only role needed by a voter: releasing the
  voter from the "Gravando" screen (message 5). Message 1 ("Eleitor foi habilitado") is posted by `votaInit`.
* The urna's state is not produced by a carga and a zerésima: it is fabricated (§3.5) and `votaInit` jumps to
  "voting open" (`EAVVOTAR`) directly.
* The fixture is written only if `eg.bin` is missing (see §9: a failed init makes later inits reuse it).
* The buzzer is Web Audio; `CWasmBeep` preserves the urna's durations (1/100 s units).

## 6. BOLETIM DE URNA (BU)

This unit never generates a BU: the encerramento is started by message 7 from the operator thread
(`CFimAquisicaoVotos` -> `CAguardaMensagem`), which does not run in the web build, and `votaTick` stops at the
first return to `CAguardaMensagem` (see [`docs/10-boletim-de-urna.md`](../10-boletim-de-urna.md) §5.1). It does
define data and code the BU uses when the BU code is driven directly (harness in `docs/bu/`):

1. **Header identity from the fixture.** `urna.correspondenciaResultado.carga` = urna 87654321, FC `12345678`,
   `nome_maquina`/`12345678`/`99999999`, `20201231T235958`, código de carga `123456789012345678901234`
   (func 10261/9952); `fase` treinamento (`'3'` from the page's `"te"`), `tipoUrna` seção (`'1'`), and the
   município/zona/seção of the options. `historicoCodigosCarga` is empty because the fixture's `gap.bin` has no
   correspondences (func 10278).
2. **Software version.** `urna.versaoVotacao` = `"10.23.0.1 - DESENVOLVIMENTO"`, set by `main` through
   `CApplication::InitApplication`. (`eg.bin`'s own `versao` is the fixture's `"7.2.1.3 - TESTE EG ASN1"`.)
3. **Order of the cargos.** The BU text, QR codes and result files list the eleições by the eleição's
   `ordemImpressao` (byte +5 of its `CSituacoesEleicoes` record) and then the cargos by `CCargo::ordemImpressao`
   (+16): comparator 11558 used by `CCargos::OrdenaPorOrdemImpressao` (func 3782, called by
   `CGeraBU::StartState` 12110 and `CGravaResultado` 12098). Inside the BU file `CGravadorBU` (slot 7) takes
   `CConfiguracaoEleicao::GetCargos(eleição, true)` (func 3775) sorted by comparator 11547 (`CCargo +16`). The
   voter is asked in *acquisition* order instead (comparator 11559: eleição +4, then `CCargo +15`).
4. **Signatures.** The BU's own signature packages (`bu.vsu`, ids 126-129) are not among the placeholders; the
   hash chain and its signature need keys that the web build does not have (`docs/10` §4).
5. **Votes.** No vote is persisted: `CSincronismoVotoEleitorWeb` returns true without writing `rdv.dat` or
   `vota.bin`, and the "Gravando" bar is the timer of §3.2. A BU generated after a web session would count zero
   votes from the RDV.

## 7. wasm / Emscripten observations

* **Specialised call-site thunks.** Funcs 11026/11100/10376 (`api_f3903(queue, 1|9|5)`), 11661
  (`GetPathDinamico(a, 0)`) and 10242/10239 (`6004(.., '1'|'2')`) are 9-15 byte functions that call a shared body
  with a constant. This is the shape left by LLVM function specialisation of a constant argument followed by
  wasm-opt `merge-similar-functions`; in the source they are most likely plain calls with a constant.
* **An import in the function table.** `votaTick` calls `emscripten_get_now` through `invoke_d(107)`: table slot
  107 holds the import itself, because the call is inside a `try`.
* **`i32.extend16_s` in main**: the screen size from the URL is narrowed to `short` (§9).
* **This-return constructors reveal defaults**: 10261, 9952, 10278 return `this` and are called on member
  addresses of the builder: they are constructors with no argument that set fixture values in place.
* **Dead `atexit` destructors.** 22 functions are the compiler-generated destructors of singleton statics
  (`std::mutex` = the folded pthread-stub body 150; `std::unique_ptr` = reset). Emscripten never runs
  `atexit` handlers here; they only survive because their addresses are in the table.
* **Misattributed library functions.** The "data-table neighbours" heuristic put these here: SQLite
  `jsonCacheDelete` (12382) and the `ALTER TABLE ... RENAME` walker callbacks `renameQuotefixExprCb`,
  `renameTableExprCb`, `renameTableSelectCb`, `renameColumnExprCb` (12433-12441); OpenSSL's DES provider
  (`cipher_des_hw.c`: cfb8/cfb1/cfb64/ofb64/cbc/copyctx/ecb/initkey, 13089-13096) and the TLS CBC MAC helpers
  `tls1_{sha512,sha256,sha1,md5}_final_raw` (13135-13139) + `SHA512_Transform`/`SHA256_Transform` (14363/14364),
  whose table slots are used by `hmac_update` (`ssl3_cbc_digest_record` of `ssl/s3_cbc.c`, inlined into the HMAC provider of OpenSSL 3.0.17); libc++
  `std::__formatter::__escape` (11769), the grapheme-cluster view of `<format>` width estimation (11971/11977),
  `basic_stringbuf::str()` (11016), `optional<locale>::operator=` (12020) and several vector/path/filebuf
  instantiations.

## 8. Runtime checks made for this chapter

With a copy of `tools/run/headless.mjs` whose `votaInit` JSON can be overridden:

* `"fase":"oficial"`: the fixture is written with fase `'1'`, then the load fails with
  `O arquivo [/dsk/fi/estatico/o02400-cp.dat] não existe` -> `vota:error`, `votaInit` = 0. The fase only selects the
  data-file prefix; the scenarios ship `t` files only.
* A second `votaInit` in the same page with the correct `"te"` options fails the same way (`o02400-cp.dat`):
  `eg.bin` already exists, so the fixture (and the options) are not applied again.
* Normal `"te"` run (`--keys "91001  C  12  C  "`): `vota:ready`, 10 `vota:state`, one `vota:done`, `votaTick`
  returns 0 afterwards.

## 9. Weird or risky code

1. **Sticky fixture** (funcs 10256/10268 via `votaInit`): the state is fabricated only if `eg.bin` is absent, and
   a failed `votaInit` has already written it. Any later `votaInit` in the page ignores its options (fase, pe,
   section, turno). Harmless for the page (it reloads per voter), confusing for embedders/tests.
2. **Screen size from the URL narrowed to 16 bits** (main 10307): `?screenWidth=40000` becomes -25536,
   `?screen=70000x800` becomes 4464; no upper bound either (`?screen=30000x30000` asks for a 3.6 GB canvas).
   A crafted link can break or crash the victim's simulator tab. No memory-safety issue in the wasm.
3. **Option parsing is not JSON** (14477/11571/11546): the first textual match of `"key"` wins (a nested object
   or an earlier key named the same would shadow it), escapes are not decoded, numbers are not range-checked,
   and `zona`/`secao` are truncated to 16 bits before validation (`"zona":70000` becomes 4464 and passes
   `zona < 10000`). Inputs come from the page's own scenario manifest, so impact is low.
4. **`done` = "back to `CAguardaMensagem`"** (10619): a discarded session (voter suspended, "NÃO VOTOU") would
   also produce `vota:done` and "Votação concluída" on the page. Not reachable in the web build today (only the
   operator suspends), but the page logic relies on RTTI class-name substrings.
5. **"Gravando" is an animation** (10619): the voter sees the recording bar and "FIM", but nothing is recorded
   (no RDV, no `vota.bin` update). Expected for a training simulator; worth knowing when reading its screens.
6. **Signatures are placeholders** (11733/11818): every `.vsu` is a fixed text and every validation passes
   (with `CWasmSavd`). Security is simulated, not reduced: the real SAVD/HSM path is absent.
7. **`votaGetStateJson` mutates the event cache** (10171): polling it can suppress a `vota:state` event. It only
   matters for a change made outside `votaTick`, e.g. `votaSetAudioEnabled` flips `"audioEnabled"`, which is part
   of the JSON: a `votaGetStateJson` between that call and the next tick stores the new JSON, and the tick then
   sees no change and emits nothing. The page never calls `votaGetStateJson` (only the headless runner does).
8. **`CEsperaAudioWasm::Cancela` never calls `terminado`** (2680) and the destructor cancels: code that drops the
   handle returned by `ISound::vf9` loses its completion callback. The only caller keeps it.
9. **Registration order is load-bearing** (10307): if anything obtained `IExecucaoVota`,
   `IPoliticaExecucaoEleitor` or `ISincronismoVotoEleitor` before `main` registers them, the urna defaults would be
   created (real threads, `emscripten_sleep` -> abort without Asyncify, RDV writes). `main`'s registration
   wrapper (u19's `replace`) would then erase and **destroy** them while earlier callers may still hold the raw
   reference `instance()` returned (dangling).
10. **`EscreveArquivo` mode** (10293): `ios::out | ios::binary` (20), no explicit `trunc` (the u28 header says
    the same); plain `out` truncates anyway. Only reached with `assina == true`, never in the web build.

## 10. Open questions

* Whether the fixture defaults (10261, 9952, 10278 and the inlined ones) are default member initialisers of the
  md classes as compiled for tests, or builder-side types with the same layout. One constraint: 10261 calls
  `AssinaturaFicticia` (9963) out of line, so the two are in the same translation unit. If 10261 is
  `md::CDadoCorrespondencia`'s own default constructor, 9963 lives in that md file and not in an anonymous
  namespace of `cappinfobuilder.cpp` (and 9952 would be a default constructor inside the ecourna library).
* The member at `CAppInfo+528` cleared by `CarregaGeral`.
* Real names of the `IBeep` slots 2-5 and of `IEsperaAudio` slots 2-3 (inferred from callers and behaviour).
* Whether the three voter-queue thunks and 11661 are source-level helpers or compiler specialisations.
* Why `CPrng` is constructed with argument 0 (`nullptr` entropy source? default seed?).

## 11. Complete mapping table (109 functions)

"ran" = observed executing in the recorded votes. "library/inlined helper" = standard-library, SQLite or
OpenSSL code, or a compiler-generated function, with no TSE source of its own.

| func | size | ran | tool name | reconstructed symbol | component | original file | reconstruction | conf. |
|---:|---:|:-:|---|---|---|---|---|---|
| 2680 | 327 |  | `simulador::(anonymous namespace)::CEsperaAudioWasm::vf2` | `simulador::(anonymous namespace)::CEsperaAudioWasm::Cancela` | app:simulador | `uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp` | `src/uenux2/mock/app/simulador/wasm/cwasmwebsound.u30.cpp` | medium |
| 8494 | 20 | ✓ | `simulador::CWasmBeep::vf6` | `simulador::CWasmBeep::BeepErro` | app:simulador | `uenux2/mock/app/simulador/wasm/cwasmbeep.cpp` | `src/uenux2/mock/app/simulador/wasm/cwasmbeep.cpp` | medium |
| 8505 | 221 |  | `simulador::CWasmBeep::vf5` | `simulador::CWasmBeep::BeepSuspensao` | app:simulador | `uenux2/mock/app/simulador/wasm/cwasmbeep.cpp` | `src/uenux2/mock/app/simulador/wasm/cwasmbeep.cpp` | low |
| 8514 | 344 |  | `simulador::CWasmBeep::vf4` | `simulador::CWasmBeep::BeepFinalizacao` | app:simulador | `uenux2/mock/app/simulador/wasm/cwasmbeep.cpp` | `src/uenux2/mock/app/simulador/wasm/cwasmbeep.cpp` | low |
| 8523 | 450 |  | `simulador::CWasmBeep::vf3` | `simulador::CWasmBeep::BeepAlerta` | app:simulador | `uenux2/mock/app/simulador/wasm/cwasmbeep.cpp` | `src/uenux2/mock/app/simulador/wasm/cwasmbeep.cpp` | low |
| 8533 | 203 | ✓ | `simulador::CWasmBeep::vf2` | `simulador::CWasmBeep::BeepConfirma` | app:simulador | `uenux2/mock/app/simulador/wasm/cwasmbeep.cpp` | `src/uenux2/mock/app/simulador/wasm/cwasmbeep.cpp` | medium |
| 8538 | 11 | ✓ | `simulador::CWasmBeep::vf1` | `simulador::CWasmBeep::Beep` | app:simulador | `uenux2/mock/app/simulador/wasm/cwasmbeep.cpp` | `src/uenux2/mock/app/simulador/wasm/cwasmbeep.cpp` | high |
| 8541 | 6 |  | `simulador::CWasmBeep::vf0` | `simulador::CWasmBeep::SetVolume` | app:simulador | `uenux2/mock/app/simulador/wasm/cwasmbeep.cpp` | `src/uenux2/mock/app/simulador/wasm/cwasmbeep.cpp` | high |
| 9445 | 7 |  | `simulador::(anonymous namespace)::CEsperaAudioWasm::vf3` | `simulador::(anonymous namespace)::CEsperaAudioWasm::Cancelada` | app:simulador | `uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp` | `src/uenux2/mock/app/simulador/wasm/cwasmwebsound.u30.cpp` | medium |
| 9448 | 12 |  | `simulador::(anonymous namespace)::CEsperaAudioWasm::vf1` | `simulador::(anonymous namespace)::CEsperaAudioWasm::~CEsperaAudioWasm (deleting)` | app:simulador | `uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp` | `src/uenux2/mock/app/simulador/wasm/cwasmwebsound.u30.cpp` | high |
| 9459 | 9 |  | `simulador::(anonymous namespace)::CEsperaAudioWasm::vf0` | `simulador::(anonymous namespace)::CEsperaAudioWasm::~CEsperaAudioWasm` | app:simulador | `uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp` | `src/uenux2/mock/app/simulador/wasm/cwasmwebsound.u30.cpp` | high |
| 9863 | 22 | ✓ | `mock_f9863` | `comum::teste::(anonymous namespace)::Copia(const md::estadoaplicacao::CLocalidadeEleitoral&)` | app:mock | `uenux2/mock/app/comum/cappinfobuilder.cpp` | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp` | low |
| 9905 | 69 |  | `mock_f9905` | `std::vector<comum::md::estadoaplicacao::CDadoCorrespondencia>::__vallocate` | rt:libcxx | `libcxx <vector>` | library/inlined helper | high |
| 9952 | 151 | ✓ | `wasm_entry_f9952` | `ecourna::app::dados::CIdentificadorGeradorMidia::CIdentificadorGeradorMidia() (fixture defaults)` | app:mock | `uenux2/mock/app/comum/cappinfobuilder.cpp` | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp` | low |
| 9963 | 336 | ✓ | `mock_f9963` | `comum::teste::(anonymous namespace)::AssinaturaFicticia` | app:mock | `uenux2/mock/app/comum/cappinfobuilder.cpp` | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp` | medium |
| 10067 | 307 |  | `mock_f10067` | `comum::teste::CAppInfoBuilder::CriaEstado(const md::estadoaplicacao::CEstadoGeralVota&)` | app:mock | `uenux2/mock/app/comum/cappinfobuilder.cpp` | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp` | medium |
| 10089 | 20 | ✓ | `mock_f10089` | `comum::IServicoEstado<md::estadoaplicacao::CEstadoGeralSA, asn::CConversorEstadoGeralSA>::Salva` | app:comum | `uenux2/src/app/comum/appinfo/servicos/iservicoestado.h` | `src/uenux2/src/app/comum/u30-foreign-fragments.cpp` | medium |
| 10101 | 32 |  | `mock_f10101` | `comum::teste::CAppInfoBuilder::CriaEstado(const md::estadoaplicacao::CEstadoGeralSA&)` | app:mock | `uenux2/mock/app/comum/cappinfobuilder.cpp` | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp` | medium |
| 10112 | 20 | ✓ | `mock_f10112` | `comum::IServicoEstado<md::estadoaplicacao::CEstadoGeralGap, asn::CConversorEstadoGeralGap>::Salva` | app:comum | `uenux2/src/app/comum/appinfo/servicos/iservicoestado.h` | `src/uenux2/src/app/comum/u30-foreign-fragments.cpp` | medium |
| 10123 | 502 | ✓ | `mock_f10123` | `comum::teste::CAppInfoBuilder::CriaEstado(const md::estadoaplicacao::CEstadoGeralGap&)` | app:mock | `uenux2/mock/app/comum/cappinfobuilder.cpp` | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp` | medium |
| 10145 | 192 | ✓ | `comum_f10145` | `std::vector<uint8_t>::vector(const std::vector<uint8_t>&)` | rt:libcxx | `libcxx <vector>` | library/inlined helper | high |
| 10155 | 461 | ✓ | `mock_f10155` | `comum::teste::(anonymous namespace)::Copia(const md::estadoaplicacao::CDadoCorrespondencia&)` | app:mock | `uenux2/mock/app/comum/cappinfobuilder.cpp` | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp` | medium |
| 10167 | 32 | ✓ | `mock_f10167` | `comum::teste::(anonymous namespace)::Copia(const md::estadoaplicacao::CDadoCarga&)` | app:mock | `uenux2/mock/app/comum/cappinfobuilder.cpp` | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp` | medium |
| 10171 | 73 | ✓ | `votaGetStateJson` | `votaGetStateJson` | app:wasm-entry | `uenux2/wasm/vota_web/vota_web_wasm.cpp` | `src/uenux2/wasm/vota_web/vota_web_wasm.u30.cpp` | high |
| 10176 | 25 |  | `mock_f10176` | `comum::teste::CAppInfoBuilder::SetLocal` | app:mock | `uenux2/mock/app/comum/cappinfobuilder.cpp` | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp` | medium |
| 10181 | 18 |  | `mock_f10181` | `comum::teste::CAppInfoBuilder::SetTipoUrna` | app:mock | `uenux2/mock/app/comum/cappinfobuilder.cpp` | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp` | medium |
| 10188 | 11 |  | `mock_f10188` | `comum::teste::CAppInfoBuilder::SetTurno` | app:mock | `uenux2/mock/app/comum/cappinfobuilder.cpp` | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp` | medium |
| 10197 | 11 |  | `mock_f10197` | `comum::teste::CAppInfoBuilder::SetFase` | app:mock | `uenux2/mock/app/comum/cappinfobuilder.cpp` | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp` | medium |
| 10205 | 682 | ✓ | `mock_f10205` | `comum::teste::CAppInfoBuilder::CriaEstado(const md::estadoaplicacao::CEstadoGeral&)` | app:mock | `uenux2/mock/app/comum/cappinfobuilder.cpp` | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp` | medium |
| 10221 | 12 |  | `wasm_entry_f10221` | `std::vector<comum::teste::EApp>::vector(const std::vector<EApp>&)` | rt:libcxx | `libcxx <vector>` | library/inlined helper | high |
| 10239 | 15 | ✓ | `wasm_entry_f10239` | `comum::teste::CAppInfoBuilder::SalvaAppsSegundoTurno` | app:mock | `uenux2/mock/app/comum/cappinfobuilder.cpp` | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp` | low |
| 10240 | 76 |  | `votaSetAudioEnabled` | `votaSetAudioEnabled` | app:wasm-entry | `uenux2/wasm/vota_web/vota_web_wasm.cpp` | `src/uenux2/wasm/vota_web/vota_web_wasm.u30.cpp` | high |
| 10242 | 15 | ✓ | `wasm_entry_f10242` | `comum::teste::CAppInfoBuilder::SalvaAppsPrimeiroTurno` | app:mock | `uenux2/mock/app/comum/cappinfobuilder.cpp` | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp` | low |
| 10256 | 386 | ✓ | `wasm_entry_f10256` | `comum::teste::CAppInfoBuilder::Salva` | app:mock | `uenux2/mock/app/comum/cappinfobuilder.cpp` | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp` | low |
| 10258 | 173 | ✓ | `wasm_entry_f10258` | `std::vector<uint8_t>::vector(size_type n, const uint8_t& v)` | rt:libcxx | `libcxx <vector>` | library/inlined helper | high |
| 10261 | 373 | ✓ | `mock_f10261` | `comum::md::estadoaplicacao::CDadoCorrespondencia::CDadoCorrespondencia() (fixture defaults)` | app:mock | `uenux2/mock/app/comum/cappinfobuilder.cpp` | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp` | low |
| 10268 | 660 | ✓ | `mock_f10268` | `comum::teste::CAppInfoBuilder::CAppInfoBuilder` | app:mock | `uenux2/mock/app/comum/cappinfobuilder.cpp` | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp` | medium |
| 10278 | 220 | ✓ | `wasm_entry_f10278` | `comum::md::estadoaplicacao::CEstadoGeralGap::CEstadoGeralGap() (fixture defaults)` | app:mock | `uenux2/mock/app/comum/cappinfobuilder.cpp` | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp` | low |
| 10286 | 258 | ✓ | `wasm_entry_f10286` | `comum::teste::CAppInfoBuilder::TodosApps` | app:mock | `uenux2/mock/app/comum/cappinfobuilder.cpp` | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp` | low |
| 10293 | 561 |  | `mock_f10293` | `comum::teste::EscreveArquivo` | app:mock | `uenux2/mock/app/comum/cappinfobuilder.cpp` | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp` | medium |
| 10304 | 25 |  | `mock_f10304` | `std::basic_filebuf<char>::open(const std::string&, std::ios_base::openmode)` | rt:libcxx | `libcxx <fstream>` | library/inlined helper | high |
| 10307 | 2480 | ✓ | `main` | `main` | app:wasm-entry | `uenux2/wasm/vota_web/vota_web_wasm.cpp` | `src/uenux2/wasm/vota_web/vota_web_wasm.u30.cpp` | high |
| 10376 | 9 | ✓ | `wasm_entry_f10376` | `(anonymous namespace)::EnviaMensagemEleitor<5> (call-site thunk of api_f3903: vote synchronised)` | app:wasm-entry | `uenux2/wasm/vota_web/vota_web_wasm.cpp` | `src/uenux2/wasm/vota_web/vota_web_wasm.u30.cpp` | medium |
| 10619 | 1561 | ✓ | `votaTick` | `votaTick` | app:wasm-entry | `uenux2/wasm/vota_web/vota_web_wasm.cpp` | `src/uenux2/wasm/vota_web/vota_web_wasm.u30.cpp` | high |
| 10703 | 102 | ✓ | `votaPressKey` | `votaPressKey` | app:wasm-entry | `uenux2/wasm/vota_web/vota_web_wasm.cpp` | `src/uenux2/wasm/vota_web/vota_web_wasm.u30.cpp` | high |
| 10857 | 363 |  | `wasm_entry_f10857` | `(anonymous namespace)::ReportError` | app:wasm-entry | `uenux2/wasm/vota_web/vota_web_wasm.cpp` | `src/uenux2/wasm/vota_web/vota_web_wasm.u30.cpp` | medium |
| 11016 | 166 | ✓ | `wasm_entry_f11016` | `std::basic_stringbuf<char>::str() const` | rt:libcxx | `libcxx <sstream>` | library/inlined helper | high |
| 11026 | 9 | ✓ | `wasm_entry_f11026` | `(anonymous namespace)::EnviaMensagemEleitor<1> (call-site thunk of api_f3903: Eleitor foi habilitado)` | app:wasm-entry | `uenux2/wasm/vota_web/vota_web_wasm.cpp` | `src/uenux2/wasm/vota_web/vota_web_wasm.u30.cpp` | medium |
| 11100 | 9 |  | `wasm_entry_f11100` | `(anonymous namespace)::EnviaMensagemEleitor<9> (call-site thunk of api_f3903: voter audio mode)` | app:wasm-entry | `uenux2/wasm/vota_web/vota_web_wasm.cpp` | `src/uenux2/wasm/vota_web/vota_web_wasm.u30.cpp` | medium |
| 11211 | 14 |  | `wasm_entry_f11211` | `std::filesystem::path::path(const char (&)[15])` | rt:libcxx | `libcxx <filesystem>` | library/inlined helper | high |
| 11266 | 9 |  | `wasm_entry_f11266` | `comum::md::estadoaplicacao::CEstadoGeralVota::SetEstadoVota` | app:comum | `uenux2/src/app/comum/dados/md/estadoaplicacao/cestadogeralvota.cpp` | `src/uenux2/src/app/comum/u30-foreign-fragments.cpp` | low |
| 11426 | 195 | ✓ | `wasm_entry_f11426` | `std::vector<comum::teste::EMidia>::vector(std::initializer_list<EMidia>)` | rt:libcxx | `libcxx <vector>` | library/inlined helper | high |
| 11546 | 702 |  | `wasm_entry_f11546` | `(anonymous namespace)::JsonString` | app:wasm-entry | `uenux2/wasm/vota_web/vota_web_wasm.cpp` | `src/uenux2/wasm/vota_web/vota_web_wasm.u30.cpp` | medium |
| 11547 | 13 |  | `wasm_entry_f11547` | `comum::CConfiguracaoEleicao::GetCargos lambda (CCargo ordemImpressao <)` | app:comum | `uenux2/src/app/comum/cconfiguracaoeleicao.cpp` | `src/uenux2/src/app/comum/u30-foreign-fragments.cpp` | medium |
| 11550 | 10 |  | `wasm_entry_f11550` | `comum::CConfiguracaoEleicao::GetInst::s_mutex.~mutex [atexit]` | app:comum | `uenux2/src/app/comum/cconfiguracaoeleicao.cpp` | `src/uenux2/src/app/comum/u30-foreign-fragments.cpp` (compiler-generated; listed) | medium |
| 11551 | 37 |  | `wasm_entry_f11551` | `comum::CConfiguracaoEleicao::GetInst::s_inst.~unique_ptr [atexit]` | app:comum | `uenux2/src/app/comum/cconfiguracaoeleicao.cpp` | `src/uenux2/src/app/comum/u30-foreign-fragments.cpp` (compiler-generated; listed) | medium |
| 11552 | 10 |  | `wasm_entry_f11552` | `comum::CFederacoes::GetInst::s_mutex.~mutex [atexit]` | app:comum | `uenux2/src/app/comum/dados/cfederacoes.cpp (path inferred)` | `src/uenux2/src/app/comum/u30-foreign-fragments.cpp` (compiler-generated; listed) | low |
| 11553 | 37 |  | `wasm_entry_f11553` | `comum::CFederacoes::GetInst::s_inst.~unique_ptr [atexit]` | app:comum | `uenux2/src/app/comum/dados/cfederacoes.cpp (path inferred)` | `src/uenux2/src/app/comum/u30-foreign-fragments.cpp` (compiler-generated; listed) | low |
| 11558 | 105 |  | `wasm_entry_f11558` | `comum::CCargos::OrdenaPorOrdemImpressao lambda` | app:comum | `uenux2/src/app/comum/dados/ccargos.cpp` | `src/uenux2/src/app/comum/u30-foreign-fragments.cpp` | medium |
| 11559 | 105 | ✓ | `wasm_entry_f11559` | `comum::CCargos acquisition-order lambda (ordemAquisicao)` | app:comum | `uenux2/src/app/comum/dados/ccargos.cpp` | `src/uenux2/src/app/comum/u30-foreign-fragments.cpp` | medium |
| 11560 | 10 |  | `wasm_entry_f11560` | `comum::CCargos::s_mutex.~mutex [atexit]` | app:comum | `uenux2/src/app/comum/dados/ccargos.cpp` | `src/uenux2/src/app/comum/u30-foreign-fragments.cpp` (compiler-generated; listed) | medium |
| 11561 | 37 |  | `wasm_entry_f11561` | `comum::CCargos::s_inst.~unique_ptr [atexit]` | app:comum | `uenux2/src/app/comum/dados/ccargos.cpp` | `src/uenux2/src/app/comum/u30-foreign-fragments.cpp` (compiler-generated; listed) | medium |
| 11563 | 10 |  | `wasm_entry_f11563` | `comum::CCandidaturas::GetInst::s_mutex.~mutex [atexit]` | app:comum | `uenux2/src/app/comum/dados/ccandidaturas.cpp` | `src/uenux2/src/app/comum/u30-foreign-fragments.cpp` (compiler-generated; listed) | medium |
| 11564 | 37 |  | `wasm_entry_f11564` | `comum::CCandidaturas::GetInst::s_inst.~unique_ptr [atexit]` | app:comum | `uenux2/src/app/comum/dados/ccandidaturas.cpp` | `src/uenux2/src/app/comum/u30-foreign-fragments.cpp` (compiler-generated; listed) | medium |
| 11566 | 16 |  | `mock_f11566` | `comum::CServicoEstadoGeralSA::CServicoEstadoGeralSA` | app:comum | `uenux2/src/app/comum/appinfo/servicos/cservicoestadogeralsa.cpp` | `src/uenux2/src/app/comum/u30-foreign-fragments.cpp` | medium |
| 11571 | 328 | ✓ | `wasm_entry_f11571` | `(anonymous namespace)::JsonInt` | app:wasm-entry | `uenux2/wasm/vota_web/vota_web_wasm.cpp` | `src/uenux2/wasm/vota_web/vota_web_wasm.u30.cpp` | medium |
| 11572 | 1550 | ✓ | `mock_f11572` | `comum::CAppInfo::CarregaGeral` | app:comum | `uenux2/src/app/comum/appinfo/cappinfo.cpp` | `src/uenux2/src/app/comum/u30-foreign-fragments.cpp` | low |
| 11661 | 9 |  | `wasm_entry_f11661` | `(anonymous namespace)::PathDinamicoInterno (call-site thunk of CPath::GetPathDinamico(INTERNA))` | app:wasm-entry | `uenux2/wasm/vota_web/vota_web_wasm.cpp` | `src/uenux2/wasm/vota_web/vota_web_wasm.u30.cpp` | low |
| 11676 | 12 |  | `wasm_entry_f11676` | `vota::CConfereVotoEmCargo<CMajoritarioRepetido>::GetInst::s_inst.~unique_ptr [atexit]` | app:vota | `uenux2/src/app/vota/eleitor/votamajoritario/cpedemajoritario.cpp` | `src/uenux2/src/app/vota/u30-foreign-fragments.cpp` (compiler-generated; listed) | medium |
| 11677 | 10 |  | `wasm_entry_f11677` | `vota::CConfereVotoEmCargo<CMajoritarioRepetido>::GetInst::s_mutex.~mutex [atexit]` | app:vota | `uenux2/src/app/vota/eleitor/votamajoritario/cpedemajoritario.cpp` | `src/uenux2/src/app/vota/u30-foreign-fragments.cpp` (compiler-generated; listed) | medium |
| 11678 | 12 |  | `wasm_entry_f11678` | `vota::CConfereVotoEmCargo<CMajoritarioNulo>::GetInst::s_inst.~unique_ptr [atexit]` | app:vota | `uenux2/src/app/vota/eleitor/votamajoritario/cpedemajoritario.cpp` | `src/uenux2/src/app/vota/u30-foreign-fragments.cpp` (compiler-generated; listed) | medium |
| 11679 | 10 |  | `wasm_entry_f11679` | `vota::CConfereVotoEmCargo<CMajoritarioNulo>::GetInst::s_mutex.~mutex [atexit]` | app:vota | `uenux2/src/app/vota/eleitor/votamajoritario/cpedemajoritario.cpp` | `src/uenux2/src/app/vota/u30-foreign-fragments.cpp` (compiler-generated; listed) | medium |
| 11680 | 12 |  | `wasm_entry_f11680` | `vota::CConfereVotoEmCargo<CMajoritarioBranco>::GetInst::s_inst.~unique_ptr [atexit]` | app:vota | `uenux2/src/app/vota/eleitor/votamajoritario/cpedemajoritario.cpp` | `src/uenux2/src/app/vota/u30-foreign-fragments.cpp` (compiler-generated; listed) | medium |
| 11681 | 10 |  | `wasm_entry_f11681` | `vota::CConfereVotoEmCargo<CMajoritarioBranco>::GetInst::s_mutex.~mutex [atexit]` | app:vota | `uenux2/src/app/vota/eleitor/votamajoritario/cpedemajoritario.cpp` | `src/uenux2/src/app/vota/u30-foreign-fragments.cpp` (compiler-generated; listed) | medium |
| 11685 | 10 |  | `wasm_entry_f11685` | `vota::CPedeMajoritario::GetInst::s_mutex.~mutex [atexit]` | app:vota | `uenux2/src/app/vota/eleitor/votamajoritario/cpedemajoritario.cpp` | `src/uenux2/src/app/vota/u30-foreign-fragments.cpp` (compiler-generated; listed) | medium |
| 11686 | 12 |  | `wasm_entry_f11686` | `vota::CPedeMajoritario::GetInst::s_inst.~unique_ptr [atexit]` | app:vota | `uenux2/src/app/vota/eleitor/votamajoritario/cpedemajoritario.cpp` | `src/uenux2/src/app/vota/u30-foreign-fragments.cpp` (compiler-generated; listed) | medium |
| 11733 | 1207 | ✓ | `wasm_entry_f11733` | `(anonymous namespace)::WriteSimulatedSignatures` | app:wasm-entry | `uenux2/wasm/vota_web/vota_web_wasm.cpp` | `src/uenux2/wasm/vota_web/vota_web_wasm.u30.cpp` | medium |
| 11740 | 12 |  | `wasm_entry_f11740` | `vota::CConfereVotoEmCargo<CProporcionalNulo>::GetInst::s_inst.~unique_ptr [atexit]` | app:vota | `uenux2/src/app/vota/eleitor/votaproporcional/cpedenulo.cpp` | `src/uenux2/src/app/vota/u30-foreign-fragments.cpp` (compiler-generated; listed) | medium |
| 11741 | 10 |  | `wasm_entry_f11741` | `vota::CConfereVotoEmCargo<CProporcionalNulo>::GetInst::s_mutex.~mutex [atexit]` | app:vota | `uenux2/src/app/vota/eleitor/votaproporcional/cpedenulo.cpp` | `src/uenux2/src/app/vota/u30-foreign-fragments.cpp` (compiler-generated; listed) | medium |
| 11742 | 12 |  | `wasm_entry_f11742` | `vota::CConfereVotoEmCargo<CCandidatoInapto>::GetInst::s_inst.~unique_ptr [atexit]` | app:vota | `uenux2/src/app/vota/eleitor/votaproporcional/cpedenulo.cpp` | `src/uenux2/src/app/vota/u30-foreign-fragments.cpp` (compiler-generated; listed) | medium |
| 11743 | 10 |  | `wasm_entry_f11743` | `vota::CConfereVotoEmCargo<CCandidatoInapto>::GetInst::s_mutex.~mutex [atexit]` | app:vota | `uenux2/src/app/vota/eleitor/votaproporcional/cpedenulo.cpp` | `src/uenux2/src/app/vota/u30-foreign-fragments.cpp` (compiler-generated; listed) | medium |
| 11746 | 10 |  | `wasm_entry_f11746` | `vota::CPedeNulo::GetInst::s_mutex.~mutex [atexit]` | app:vota | `uenux2/src/app/vota/eleitor/votaproporcional/cpedeproporcional.cpp` | `src/uenux2/src/app/vota/u30-foreign-fragments.cpp` (compiler-generated; listed) | medium |
| 11747 | 12 |  | `wasm_entry_f11747` | `vota::CPedeNulo::GetInst::s_inst.~unique_ptr [atexit]` | app:vota | `uenux2/src/app/vota/eleitor/votaproporcional/cpedeproporcional.cpp` | `src/uenux2/src/app/vota/u30-foreign-fragments.cpp` (compiler-generated; listed) | medium |
| 11769 | 640 |  | `wasm_entry_f11769` | `std::__formatter::__escape<char> (std::format "{:?}" escaping)` | rt:libcxx | `libcxx <__format/formatter_output.h>` | library/inlined helper | high |
| 11818 | 512 | ✓ | `wasm_entry_f11818` | `(anonymous namespace)::WriteFileIfMissing` | app:wasm-entry | `uenux2/wasm/vota_web/vota_web_wasm.cpp` | `src/uenux2/wasm/vota_web/vota_web_wasm.u30.cpp` | medium |
| 11971 | 406 |  | `wasm_entry_f11971` | `std::__unicode::__extended_grapheme_cluster_view<char>::__consume` | rt:libcxx | `libcxx <__format/unicode.h>` | library/inlined helper | medium |
| 11977 | 133 |  | `wasm_entry_f11977` | `std::__unicode::__extended_grapheme_cluster_view<char>::__extended_grapheme_cluster_view` | rt:libcxx | `libcxx <__format/unicode.h>` | library/inlined helper | medium |
| 12020 | 33 |  | `wasm_entry_f12020` | `std::optional<std::locale>::operator=(const std::locale&)` | rt:libcxx | `libcxx <optional>` | library/inlined helper | high |
| 12121 | 9 | ✓ | `api_f12121` | `std::filesystem::create_directories(const std::filesystem::path&)` | rt:libcxx | `libcxx <filesystem>` | library/inlined helper | high |
| 12382 | 115 |  | `wasm_entry_f12382` | `jsonCacheDelete` | lib:sqlite | `sqlite3.c (json.c)` | library/inlined helper | high |
| 12433 | 149 |  | `wasm_entry_f12433` | `renameQuotefixExprCb` | lib:sqlite | `sqlite3.c (alter.c)` | library/inlined helper | high |
| 12437 | 163 |  | `wasm_entry_f12437` | `renameTableExprCb` | lib:sqlite | `sqlite3.c (alter.c)` | library/inlined helper | high |
| 12439 | 235 |  | `wasm_entry_f12439` | `renameTableSelectCb` | lib:sqlite | `sqlite3.c (alter.c)` | library/inlined helper | high |
| 12441 | 279 |  | `wasm_entry_f12441` | `renameColumnExprCb` | lib:sqlite | `sqlite3.c (alter.c)` | library/inlined helper | high |
| 13089 | 146 |  | `wasm_entry_f13089` | `cipher_hw_des_cfb8_cipher` | lib:openssl | `providers/implementations/ciphers/cipher_des_hw.c` | library/inlined helper | high |
| 13090 | 228 |  | `wasm_entry_f13090` | `cipher_hw_des_cfb1_cipher` | lib:openssl | `providers/implementations/ciphers/cipher_des_hw.c` | library/inlined helper | high |
| 13091 | 154 |  | `wasm_entry_f13091` | `cipher_hw_des_cfb64_cipher` | lib:openssl | `providers/implementations/ciphers/cipher_des_hw.c` | library/inlined helper | high |
| 13092 | 166 |  | `wasm_entry_f13092` | `cipher_hw_des_ofb64_cipher` | lib:openssl | `providers/implementations/ciphers/cipher_des_hw.c` | library/inlined helper | high |
| 13093 | 175 |  | `wasm_entry_f13093` | `cipher_hw_des_cbc_cipher` | lib:openssl | `providers/implementations/ciphers/cipher_des_hw.c` | library/inlined helper | high |
| 13094 | 24 |  | `wasm_entry_f13094` | `cipher_hw_des_copyctx` | lib:openssl | `providers/implementations/ciphers/cipher_des_hw.c` | library/inlined helper | high |
| 13095 | 79 |  | `wasm_entry_f13095` | `cipher_hw_des_ecb_cipher` | lib:openssl | `providers/implementations/ciphers/cipher_des_hw.c` | library/inlined helper | high |
| 13096 | 23 |  | `wasm_entry_f13096` | `cipher_hw_des_initkey` | lib:openssl | `providers/implementations/ciphers/cipher_des_hw.c` | library/inlined helper | high |
| 13135 | 738 |  | `wasm_entry_f13135` | `tls1_sha512_final_raw` | lib:openssl | `ssl/s3_cbc.c` | library/inlined helper | high |
| 13136 | 346 |  | `wasm_entry_f13136` | `tls1_sha256_final_raw` | lib:openssl | `ssl/s3_cbc.c` | library/inlined helper | high |
| 13138 | 217 |  | `wasm_entry_f13138` | `tls1_sha1_final_raw` | lib:openssl | `ssl/s3_cbc.c` | library/inlined helper | high |
| 13139 | 174 |  | `wasm_entry_f13139` | `tls1_md5_final_raw` | lib:openssl | `ssl/s3_cbc.c` | library/inlined helper | high |
| 14363 | 48 |  | `wasm_entry_f14363` | `SHA512_Transform` | lib:openssl | `crypto/sha/sha512.c` | library/inlined helper | high |
| 14364 | 11 |  | `wasm_entry_f14364` | `SHA256_Transform` | lib:openssl | `crypto/sha/sha256.c` | library/inlined helper | high |
| 14477 | 378 | ✓ | `wasm_entry_f14477` | `(anonymous namespace)::JsonBool` | app:wasm-entry | `uenux2/wasm/vota_web/vota_web_wasm.cpp` | `src/uenux2/wasm/vota_web/vota_web_wasm.u30.cpp` | medium |
