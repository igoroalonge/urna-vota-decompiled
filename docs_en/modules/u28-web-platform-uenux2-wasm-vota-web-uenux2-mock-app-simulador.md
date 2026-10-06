# u28 — web platform (1 of 4): the web SAVD, the web keypad-flush policy, the state-file fixture, two base destructors

Unit u28 of the reverse-engineering of `vota_web_wasm.wasm` (TSE voting application VOTA, `uenux2` + `ecourna`,
Emscripten build of the public training simulator). It is the smallest of the four "web platform" units
(u28–u31 share `uenux2/wasm/vota_web` and `uenux2/mock/app/…`): **7 functions, 3 of them seen executing** during
the recorded votes. Function indices are wasm function indices (`python3 tools/wasmmap/q.py f <n>`).

Reconstructed sources (fragments, because the rest of each file belongs to another unit):

| file | content |
|---|---|
| [`src/uenux2/wasm/vota_web/vota_web_wasm.u28.cpp`](../../src/uenux2/wasm/vota_web/vota_web_wasm.u28.cpp) | `(anonymous)::CWasmSavd`, `(anonymous)::CPoliticaExecucaoEleitorWeb` |
| [`src/uenux2/mock/app/comum/cappinfobuilder.u28.h`](../../src/uenux2/mock/app/comum/cappinfobuilder.u28.h) / [`.u28.cpp`](../../src/uenux2/mock/app/comum/cappinfobuilder.u28.cpp) | `comum::teste::CAppInfoBuilder::SalvaGeral / SalvaApps / SalvaApp`, `Converte(EUrnaTurno)` |
| [`src/uenux2/src/app/comum/iinterfaceinit.u28.cpp`](../../src/uenux2/src/app/comum/iinterfaceinit.u28.cpp) | `comum::IInterfaceInit::~IInterfaceInit` |
| [`src/uenux2/src/api/gui/iscreen.u28.h`](../../src/uenux2/src/api/gui/iscreen.u28.h) | `api::IScreen::~IScreen` (documented; the body is already in `iscreen.h`, u17) |

Related chapters: [u23](u23-uenux2-src-app-comum-gravadores-uenux2-src-app-comum-iinterf.md) §9 (the SAVD client
protocol that `CWasmSavd` answers), [u06](u06-uenux2-src-app-vota-eleitor.md) (the urna's
`CPoliticaExecucaoEleitor`), [u19](u19-uenux2-src-api-pattern-cpolysingletonlist-h.md) §2.3 (what `main` registers),
[u03](u03-uenux2-src-app-comum-dados.md) §6 (content of `eg.bin`/`gap.bin`/`sa.bin`/`vota.bin`),
docs/03-js-wasm-interface.md §10 (the keyboard queue), docs/bu/codepath.md.

## 0. Words

| term | meaning |
|---|---|
| SAVD | the urna's signing / signature-validation service (a separate daemon on the real machine). The application never holds keys: it sends binary requests (sign this file, open the HSM session, validate this package) through `comum::IInterfaceSavd` |
| `.vsu` | signature file of a dynamic file (`vota.vsu`, `gap.vsu`, `eg.vsu`, …), written by the SAVD on the urna |
| *assinatura simulada* | "simulated signature" |
| *política de execução do eleitor* | "voter execution policy": a replaceable singleton with one method, `LimpaBufferInput` ("clear the input buffer") |
| *CORRIGE* / *CONFIRMA* | the orange "correct" and green "confirm" keys of the voter keypad |
| *tela de confirmação* | the confirmation screen shown after the voter types a number (candidate photo, name, party) |
| *estado geral* | "general state": the urna's persistent state files `eg.bin` (urna), `gap.bin` (launcher / GAP), `sa.bin` (SA application), `vota.bin` (VOTA application), BER-encoded ASN.1 (`ModuloEstadoGeral*`) |
| *trab1* / *trab2* | work directory of the 1st / 2nd *turno* (round) inside `dinamico/` |
| MI / MV, *flash interna* / *externa* | internal flash `/dsk/fi` and external flash (memória de votação) `/dsk/fe` |
| *builder* / *teste* | `comum::teste::CAppInfoBuilder` is a test fixture ("teste" = test) of the urna code base |
| *mídia*, *turno*, *app* | medium (which flash), round, application |

## 1. What the subsystem does

This unit collects the pieces of the web platform that sit **between the voting application and the missing
hardware/services**, plus two destructors of hardware interfaces:

1. **`CWasmSavd`** replaces the SAVD daemon. Every signature request and every signature check of the
   application goes through it, and it answers "OK" to all of them without doing anything. Observed on every
   run, and only inside `votaInit`: `SalvaEstado` (491) signs the state files through `AssinarUE`, and
   `CPacoteArquivos::ValidarChaveEAplicacaoValida` (4625) checks the election data packages. A CPU-profile timeline
   of a complete vote (`--cpu-prof`, `--keys "91001  C  12  C  "`) shows no SAVD call after `votaInit`: the web
   `CSincronismoVotoEleitorWeb` writes neither `vota.bin` nor `rdv.dat`, so nothing is signed when a vote is confirmed.
2. **`CPoliticaExecucaoEleitorWeb`** replaces the voter-thread policy that flushes the keypad after CORRIGE on a
   confirmation screen. The urna version waits between flushes with `sleep_for`, which would abort this build.
3. **`CAppInfoBuilder::SalvaGeral / SalvaApps`** fabricate the urna's persistent state (`eg.bin`, and
   `gap.bin`/`sa.bin`/`vota.bin` of both turnos on both flashes) during `votaInit`. On a real urna these files come
   from the carga (media preparation) and from the applications themselves.
4. `IScreen::~IScreen` and `IInterfaceInit::~IInterfaceInit`: base-class destructors. The web implementations
   (`CWasmScreen`, `CWasmInit`) reuse them unchanged (their own destructors are aliases).

## 2. Classes (RTTI) and layouts

```
comum::IInterfaceSavd (vtable @1526736, typeinfo @1526720)                     iinterfacesavd.cpp (u23)
  └ (anonymous namespace)::CWasmSavd (vtable @1526688, typeinfo @1526708)       vota_web_wasm.cpp (path inferred)
      20 bytes = base members only: +0 vptr, +4 std::string m_mensagem, +16 ueint32 m_codigoErro
      [0] 5503 ~IInterfaceSavd   [1] 10965 deleting dtor   [2] 425 EnviaMensagem = no-op (ICF)
      [3] 10949 RecebeMensagem   [4] 10930 slot 4 (meaning unknown) -> 0x0CABECA0

vota::impl::IPoliticaExecucaoEleitor
  ├ vota::impl::CPoliticaExecucaoEleitor         (urna; cpoliticaexecucaoeleitor.cpp, func 13564, u06)
  └ (anonymous namespace)::CPoliticaExecucaoEleitorWeb (vtable @1527156, typeinfo @1527168)  vota_web_wasm.cpp:361
      4 bytes (vptr only)   [0] 174 trivial dtor   [1] 144 operator delete   [2] 10835 LimpaBufferInput

api::IScreen (vtable @1529100)                  └ simulador::CWasmScreen (vtable @1528824)   slot 0 = 5071 for both
comum::IInterfaceInit (vtable @1552160)         └ simulador::CWasmInit   (vtable @1528772)   slot 0 = 5898 for both
      IInterfaceInit: +4/+8 std::shared_ptr<bool> m_demoMode, +12 std::string m_serialMR (u23 names)

comum::teste::CAppInfoBuilder (no RTTI, no virtuals; 500 bytes; cappinfobuilder.cpp)
      +0   md::estadoaplicacao::CEstadoGeral      m_geral    180 B  -> dinamico/eg.bin
      +180 md::estadoaplicacao::CEstadoGeralGap   m_gap[2]    44 B  -> trab1|2/gap.bin
      +268 md::estadoaplicacao::CEstadoGeralSA    m_sa[2]     16 B  -> trab1|2/sa.bin
      +300 md::estadoaplicacao::CEstadoGeralVota  m_vota[2]  100 B  -> trab1|2/vota.bin
```

Why `CWasmSavd` is placed in `vota_web_wasm.cpp`: anonymous-namespace classes have internal linkage, and `main`
(func 10307) stores its vtable (`operator new(20)`, zero-fill, vptr @1526688, then
`CPolySingletonList::push<comum::IInterfaceSavd>`, func 10090). `main` also stores the vtable of
`CPoliticaExecucaoEleitorWeb`, whose method has a srcloc in `vota_web_wasm.cpp`, so `main`, both classes and
`CSincronismoVotoEleitorWeb` are in the same translation unit.

The builder's member types are deduced from sizes and destructors: the builder constructor (func 10268, u30) and
the temporaries of `SalvaGeral`/`SalvaApps` use the same destructors (2793 for the EG object, 1698 for the GAP
object), and the sizes equal those of the md classes (`CAppInfo` caches them in `std::optional`s of 184/48/104
bytes, u20).

## 3. Control flow

### 3.1 One SAVD exchange (func 10949)

```
client (IInterfaceSavd::EnviaRequisicao 3830 / EnviaComando 3829)          CWasmSavd
  EnviaMensagem({0xFE, aplicação, u16 comando, u32 tamanho})   ─────────►   slot 2: nothing
  EnviaMensagem(payload)                                       ─────────►   slot 2: nothing
  RecebeMensagem(estado, header, 12)                           ─────────►   slot 3 (func 10949):
                                                                              estado = 0x0CABECA0
                                                                              tamanho != 12 ? throw std::invalid_argument(
                                                                                "CWasmSavd::RecebeMensagem - mensagem de tamanho invalido")
                                                                              header = FE 00 00 00 | 00 00 00 00 | 00 00 00 00
  DesconverteHeader: size 12 ✓, marca 0xFE ✓, erro 0 → success, no second read
```

Every command therefore succeeds: `0x0042` sign a dynamic file (`AssinarUE`, called by `CAssinador::Assina`),
`0x0080` sign a result file (`AssinarEcourna`, the BU's `-vota.vsc`), `0x1604` HSM session open/close,
`0x0404` define the section "local", package / file signature checks: `CPacoteArquivos::ValidarChaveEAplicacaoValida`
(4625: one package request when the command is `0x2021`/`0x1011`; with `0x1001` one request per file through
func 5892 → 3830, which is the path that ran at start-up), `ValidarUE` (5890, `0x2021`, from
`VerificaAssinaturaMI/MV`; not seen running) and `CSigVerifier` (11179). The request is never inspected, and the SAVD
writes no file. The `.vsu` files of the simulator are created by `votaInit` (funcs 11733/11818) with the text
`assinatura simulada para vota_web_wasm` (checked in `analysis/runtime/memfs-after-init`).

The out-parameter `estado` is an uninitialised local in both clients and is never read afterwards. Slot 4 (func
10930) returns the same constant and has no caller. Their meaning on the urna (message type, channel id?) cannot be
recovered from this build. `0x0CABECA0` reads as hex-speak "cabeça" ("head").

### 3.2 CORRIGE on a confirmation screen (func 10835)

Verified at run time by wrapping the import `wasm_input_clear` with a stack trace
(`node --import <hook> tools/run/headless.mjs --named --keys "91001  D  "`):

```
wasm_input_clear  (JS: Module.uenuxKeys = [])
 ← icf_tiny_vf4@5013             (simulador::CWasmInputKbd::Flush, IInput slot 4)
 ← (anonymous)::CPoliticaExecucaoEleitorWeb::LimpaBufferInput     func 10835
 ← vota::CConfirmaVotoNominal::vf17
 ← vota::CVotacaoStateAudio::EmiteEcoCorrigeConfirma
 ← vota::CVotacaoStateAudio::ProcessInput ← vota::CEleitorVotando::vf7
```

In that session the queue was flushed 6 times, against 4 without the `D` (re-checked by this review with the same
hook). The other flushes are one direct `IInputKbd` flush in `CEleitorVotando::IniciaCiclo` (the first one) and
`CInteractiveForm::ClearKeyboardInput` (func 11077) when a screen starts.

| | urna (`CPoliticaExecucaoEleitor`, func 13564) | web (`CPoliticaExecucaoEleitorWeb`, func 10835) |
|---|---|---|
| flushes | `IRng::Gera() % 4 + 3` → 3..6 (signed remainder: a negative `Gera()` gives 0..2, u06) | 1 |
| wait | one delay drawn before the loop, `(IRng::Gera() % 100 + 50) & 0xFF` ms → 50..149 ms, slept after **every** flush (`sleep_for` → `if (byte@1584624 == 1) emscripten_sleep(ms)`) | none |
| voter thread after CORRIGE | blocked for about 0.15–0.9 s; keys pressed meanwhile are discarded | continues at once |
| next screen | appears after that delay; its `ClearKeyboardInput` flushes again | appears immediately; its `ClearKeyboardInput` flushes (the 6th flush above) |
| in this build | would call `emscripten_sleep` → abort (no ASYNCIFY) | safe |

`IPoliticaExecucaoEleitor::GetInst()` registers the urna policy lazily **only if no implementation exists**.
Because `main` registers the web policy first, the aborting version is never reached.

### 3.3 The state-file fixture at `votaInit` (funcs 10243, 10212)

`votaInit` (func 7840, u30) builds a `CAppInfoBuilder` with the fixture values and calls
`wasm_entry_f10256(builder, assina = false, midias = {0, 1})`, which chains:

```
SalvaGeral(false, {FlashInterna, FlashExterna})                                   func 10243
  .SalvaApps(false, {…}, '1', {Gap, SA, Vota})   via thunk 10242 → 6004(…, 49)       func 10212
  .SalvaApps(false, {…}, '2', {Gap, SA, Vota})   via thunk 10239 → 6004(…, 50)
```

(`6004` copies both vectors and calls `SalvaApps`. `10242`/`10239` differ only in the turno constant, which is why
wasm-opt merged them. `10286` builds the `{0, 1, 2}` app list each time.)

`SalvaGeral`, for each medium:

1. `origem = Converte(midia)` (func 5344): midia ∉ {0, 1} (one unsigned test, `i32.ge_u 2`, so negative values
   too) → `std::logic_error("Converte - midia invalida [{}] da linha [{}]")`; 0/1 are returned unchanged;
2. if `assina` (never here): write the text `"assinatura EG"` into `<flash>/dinamico/eg.vsu`;
3. `CServicoEstadoGeral(origem).Salva(CriaEstado(m_geral))`: rebuild the `CEstadoGeral` through its field-wise
   constructor (10205 → 5637), BER-encode it with `CConversorEstadoGeral` and `CFileASN::WriteToFile` it to
   `<flash>/dinamico/eg.bin` (2894).

`SalvaApps`, for each medium (`Converte(midia)`, srcloc :277) and each app, the inlined `SalvaApp`:

1. `idx = Converte(turno)` (srcloc :287): `'1'` → 0, `'2'` → 1, anything else →
   `std::logic_error("Converte - turno invalido [{}] da linha [{}]")` (turno printed as an integer);
2. `trab = CPath::GetPathTrab(origem, turno)` → `<flash>/dinamico/trab<turno>/`;
3. `switch (app)`: `Gap` → (`gap.vsu` "assinatura EG Gap" if assina) `CServicoEstadoGeralGap(origem, turno).Salva(CriaEstado(m_gap[idx]))`;
   `SA` → `sa.vsu` "assinatura EG SA", `sa.bin`; `Vota` → `vota.vsu` "assinatura EG Vota", `vota.bin`; other
   values: nothing.

Result: 14 files, exactly the `.bin` files of `analysis/runtime/memfs-after-init` (sizes: eg 396, gap 60, sa 19,
vota 56 in trab1 after the application rewrote it, 37 in trab2). The contents are decoded in u03 §6. The
readable strings of `eg.bin` are the fixture's `nome_maquina`, `12345678`, `99999999`, `20201231T235958`,
`1234567890123456789012340` and versão `7.2.1.3 - TESTE EG ASN1`; `gap.bin` carries `dataSegundoTurno 20801231`.

### 3.4 Destructors (funcs 5071, 5898)

* `~IScreen()` = `IForm<IScreen>::RemoveAll()` (func 5069): empties the static stack of voter-screen forms.
* `~IInterfaceInit()` = compiler-generated: `~string(m_serialMR)`, `~shared_ptr<bool>(m_demoMode)`.

Neither ran in the recorded sessions. The singletons live until the page is reloaded, and the web build serves one
voter per page load.

## 4. Data read / written

| data | direction | function | format |
|---|---|---|---|
| SAVD request header + payload | written by the client, **discarded** | slot 2 (ICF 425) | `{0xFE, aplic, u16 cmd, u32 len}` + payload (u23 §9) |
| SAVD answer header | produced | 10949 | 12 bytes `FE 00 00 00 00 00 00 00 00 00 00 00` |
| `/dsk/fi|fe/dinamico/eg.bin` | written | 10243 → 2894 | BER `ModuloEstadoGeralUrna` via `CConversorEstadoGeral` |
| `/dsk/fi|fe/dinamico/trab{1,2}/gap.bin` | written | 10212 → 10112 → 2894 | BER `ModuloEstadoGeralGap` |
| `/dsk/fi|fe/dinamico/trab{1,2}/sa.bin` | written | 10212 → 10089 → 2894 | BER `ModuloEstadoGeralSA` |
| `/dsk/fi|fe/dinamico/trab{1,2}/vota.bin` | written | 10212 → 5329 → 2894 | BER `ModuloEstadoGeralVota` |
| `…/eg.vsu`, `…/trabN/{gap,sa,vota}.vsu` | would be written (`assina == true` only; never in this build) | 10293 | plain text `assinatura EG[ Gap| SA| Vota]` |
| keypad queue `Module.uenuxKeys` | cleared | 10835 → 5013 → import `wasm_input_clear` | JS array |

No SQL, no network, and no data from the page is parsed here. The builder's values come from `votaInit`'s JSON
(`fase`: `"oficial"`/`"o"` → '1', `"simulado"`/`"s"` → '2', else '3'; `turno`; `municipio`/`zona`/`secao`),
which is parsed in u30.

## 5. What is specific to the web build

* **Simulated signing/validation**: `CWasmSavd` (§3.1). Every integrity check that goes through the SAVD passes.
* **Simplified keypad flush**: `CPoliticaExecucaoEleitorWeb` (§3.2). It was changed because of the missing
  ASYNCIFY, and it also changes timing behaviour.
* **Fabricated state**: the state files come from a test fixture (`uenux2/mock/`, namespace `comum::teste`), not
  from a carga. The fixture's own "signature" branch (`assina`) is present but disabled. The web init writes a
  different fake text into the `.vsu` files.
* **Reused base destructors**: the mocks `CWasmScreen` and `CWasmInit` add no destructible member.

## 6. Boletim de urna (BU)

This unit does not build the BU. It touches the BU chain in one place; a second one (the printing check) is listed
because it does **not** go through this mock:

* **Result-file signature.** At the encerramento, `CAssinador::AssinaArquivosResultado` / `AssinarEcourna` (inlined
  in `CGravaResultado`, func 12098) would send one `0x0080` request per result file (`-bu.dat`, `-rdv.dat`,
  `-imgbu.dat`, …) to the SAVD, which should add the signatures to `…-vota.vsc`. With `CWasmSavd` each request gets
  the OK header and **no `.vsc` content is produced**. `docs/bu/codepath.md` §8.4 shows that this step is not reached
  in the simulator anyway.
* **Printing check (not through this mock).** `ImprimeBU` (func 2890) does not call the SAVD itself: it hands
  `IPaperRelatorios` slot 8 the image `trab/bu.dat` plus a `CSigVerifier(trab, "bu.dat", "bu.vsu")`, and on the urna
  the printer service runs that verifier (which would reach the SAVD through 11179 → 5892 → 3830). In the web build
  the paper is `simulador::CWasmNullPaper`, whose slot 8 (func 8371) only releases the verifier's `shared_ptr`, so
  the check is never evaluated at all (u31 §11 and §13 #8); `CWasmSavd` is not involved.

Section identity: the BU header's município/zona/seção come from `CLocal`, which takes the triple from `eg.bin` and
loads `<município><zona><seção>-lo.dat` with it (`CLocal::Carrega`, func 5740, u18). In the simulator `eg.bin` is the
fixture written by `SalvaGeral` (from the scenario values passed to `votaInit`).

## 7. wasm / Emscripten observations

* **Destructor aliasing.** Slot 0 of `CWasmScreen`/`CWasmInit` points to the base destructor (5071/5898), and only
  the deleting destructors are separate functions: 9388 is `free(~IInterfaceInit(this))`, while 8963 inlines the
  small `~IScreen` body (IScreen vptr, `RemoveAll` 5069) and then frees. `CWasmSavd` does the same as `CWasmInit`
  (slot 0 = 5503, slot 1 = 10965 = `free(~IInterfaceSavd(this))`).
* **ICF.** `CWasmSavd::EnviaMensagem` is the shared empty body 425. `CWasmInputKbd::Flush` and `CWasmInputMT::Flush`
  are one function, 5013.
* **Default-argument `source_location`.** The three srclocs of `cappinfobuilder.cpp` belong to the Converte call
  sites, not to the Converte functions. After inlining, `Converte(turno)` keeps only `loc.line()`: an
  `i32.load 1528068` of the static record's line field (= 287). The out-of-line `Converte(midia)` (5344) receives the
  record pointer and tests it for null (`loc.line()` returns 0 for a null `source_location`).
* **Format strings hidden in i64 constants.** `std::string_view{ptr, len}` is materialised as one `i64.const`:
  `188978796328` = `{235304, 44}` = `"Converte - turno invalido [{}] da linha [{}]"`, and `188978796373` =
  `{235349, 44}` = `"Converte - midia invalida [{}] da linha [{}]"`. That is why `q.py grep` lists no referrer for
  these strings. Enum arguments are packed as `handle` (type 15) with formatter slots 474/468 (integer
  formatting).
* **Inlining.** `SalvaApp` (a member with its own srcloc) exists only inside `SalvaApps` (2,747 bytes). Every call
  in it goes through `invoke_*` because each temporary (`std::filesystem::path`, `std::string`, the rebuilt md
  object) needs a landing pad.
* **Service-object argument order.** In `CServicoEstadoGeralGap(o, t).Salva(CriaEstado(x))` the service is
  constructed before the argument. This is the C++17 sequencing of the postfix expression, and it is visible in the
  call order 5812 → 10123 → 10112.
* The registry (`TPolySingletonsInfo`) is fetched by an indirect call through the function pointer @1526320 (table
  slot 284 → func 11265, u19). It is the same accessor for every interface (`IInputKbd` here, `IRng` in 13564,
  `IPoliticaExecucaoEleitor` in 5925); the interface is chosen by the `instance` instantiation (455 = `IInputKbd`).

## 8. Open questions

* Meaning of `IInterfaceSavd` slot 4 and of the first (`ueint32&`) parameter of `RecebeMensagem` on the real urna.
  In this build both carry `0x0CABECA0`, and nothing reads them.
* Whether `EMidia`/`EApp` live in `comum` or in `comum::teste`. The srcloc prints them unqualified.
* The exact names of the builder's rebuild helpers (10205/10123/10101/10067, u30) and of the file writer 10293.

## 9. Weird or risky code

| # | func | what | impact |
|---|---|---|---|
| 1 | 10949 | **The SAVD is a yes-man.** Every request (sign, HSM, validate) is answered "OK" without being read, so the start-up validation of the election data packages (`CPacoteArquivos::ValidarChaveEAplicacaoValida`, 4625, observed; `ValidarUE` if reached) and every `.vsu`/`.vsc` signature are simulated. (The printed-BU check against `bu.vsu` does not even reach this mock: the null paper never runs its `CSigVerifier`, §6) | simulator only. Modified scenario data would be accepted, and nothing in the web build demonstrates the real integrity checks |
| 2 | 10835 | web keypad flush = 1 flush with no delay, where the urna does 3–6 flushes, each followed by the same random 50–149 ms pause (for non-negative random numbers) | after CORRIGE on a confirmation screen the simulator shows the next screen at once, while the urna's voter thread pauses 0.15–0.9 s and discards keys pressed meanwhile. It is a small timing difference in a training tool. The urna version would abort this build (`emscripten_sleep`), and only the registration order in `main` prevents it from running |
| 3 | 10212, 10243 | test-fixture code (`comum::teste::CAppInfoBuilder`) creates the urna's persistent state; its `assina` branches would write yet another fake signature text (`"assinatura EG Gap/SA/Vota"`, `"assinatura EG"`) | dead in this build (`votaInit` passes `false`). Informative: the "state of the urna" in the simulator is hard-coded |
| 4 | 10949 | writes the out-parameter before validating, and throws `std::invalid_argument` (not a TSE error) for any length ≠ 12. The client code only expects TSE protocol errors | unreachable: the mock never reports an error, so the client never asks for the message body |
| 5 | 10212 | `Converte(turno)` accepts only `'1'`/`'2'` (throws `std::logic_error` for `'0'`/`'3'`), and unknown `EApp` values are skipped silently | unreachable with the arguments `votaInit` uses |
| 6 | 10930 | `IInterfaceSavd` slot 4 has no caller and returns a magic constant | dead code |

## 10. Mapping table (all 7 functions of the unit)

`ran` = seen executing in the recorded votes (`analysis/runtime/vote_*.functions.tsv`).

| func | bytes | ran | tools name | reconstructed symbol | original file | reconstruction | conf. |
|---|---|---|---|---|---|---|---|
| 5071 | 17 |  | `api::IScreen::vf0` | `api::IScreen::~IScreen()` (complete dtor; also slot 0 of `simulador::CWasmScreen`) | uenux2/src/api/gui/iscreen.h | src/uenux2/src/api/gui/iscreen.u28.h (body already inline in iscreen.h, u17) | high |
| 5898 | 91 |  | `comum::IInterfaceInit::vf0` | `comum::IInterfaceInit::~IInterfaceInit()` (also slot 0 of `simulador::CWasmInit`) | uenux2/src/app/comum/iinterfaceinit.cpp | src/uenux2/src/app/comum/iinterfaceinit.u28.cpp | high |
| 10212 | 2747 | ✔ | `comum::teste::CAppInfoBuilder::SalvaApps` | `comum::teste::CAppInfoBuilder::SalvaApps` (+ inlined `SalvaApp` :287 and `Converte(EUrnaTurno)`) | uenux2/mock/app/comum/cappinfobuilder.cpp | src/uenux2/mock/app/comum/cappinfobuilder.u28.cpp | high |
| 10243 | 598 | ✔ | `comum::teste::CAppInfoBuilder::SalvaGeral` | `comum::teste::CAppInfoBuilder::SalvaGeral` | uenux2/mock/app/comum/cappinfobuilder.cpp | src/uenux2/mock/app/comum/cappinfobuilder.u28.cpp | high |
| 10835 | 62 |  | `(anonymous namespace)::CPoliticaExecucaoEleitorWeb::LimpaBufferInput` | same (srcloc vota_web_wasm.cpp:361) | uenux2/wasm/vota_web/vota_web_wasm.cpp | src/uenux2/wasm/vota_web/vota_web_wasm.u28.cpp | high |
| 10930 | 8 |  | `(anonymous namespace)::CWasmSavd::vf4` | `CWasmSavd` override of `IInterfaceSavd` slot 4 (`Slot4()` in the u23 header; meaning unknown, returns `0x0CABECA0`) | uenux2/wasm/vota_web/vota_web_wasm.cpp (path inferred) | src/uenux2/wasm/vota_web/vota_web_wasm.u28.cpp | low (name) |
| 10949 | 160 | ✔ | `(anonymous namespace)::CWasmSavd::RecebeMensagem` | `(anonymous namespace)::CWasmSavd::RecebeMensagem(ueint32&, std::vector<uebyte>&, size_t)` | uenux2/wasm/vota_web/vota_web_wasm.cpp (path inferred) | src/uenux2/wasm/vota_web/vota_web_wasm.u28.cpp | high |

Functions of other units referenced above (for navigation, not part of u28): 425 (no-op ICF, `CWasmSavd::EnviaMensagem`),
5503/10965 (`~IInterfaceSavd` / `CWasmSavd` deleting dtor, u23), 3829/3830 (SAVD client, u23), 5013 (`CWasmInputKbd::Flush`),
13564 (urna `LimpaBufferInput`, u06), 5069 (`IForm<IScreen>::RemoveAll`), 8963/9388 (deleting dtors of `CWasmScreen`/`CWasmInit`),
10307 `main`, 7840 `votaInit`, 10256/6004/10242/10239/10286 (builder driver), 10268 (builder ctor), 5344 (`Converte(EMidia)`),
10293 (text-file writer), 10205/10123/10101/10067 (rebuild the md estados), 2894 (`IServicoEstado<…>::Salva` body),
3897/5812/11566/3787/1941 (service constructors), 3592/10112/10089/5329 (`Salva` thunks), 358/1082 (`CPath::GetPathTrab/GetPathDinamico`),
5968 (`path operator/`), 10134/11637/10078 (`path(const char(&)[N])`), 9868 (enum formatter handle).

## 11. Fidelity review (2026-09-23)

All 7 functions were compared with the decompiled code and the WAT, together with the neighbours the text relies on
(425, 5503, 10965, 3829, 3830, 5344, 10256, 6004, 10242, 10239, 10286, 13564, 5013, 5069, 8963, 9388, 5925, 10293).
Branch conditions, constants, strings, callee order and struct offsets of the reconstructions match. Corrections:

* §1 / `vota_web_wasm.u28.cpp`: the SAVD runs only inside `votaInit`. A `--cpu-prof` timeline of a full vote has
  no SAVD call after the vote, so the claim that `SalvaEstado` signs `vota.bin`/`rdv.dat` after each vote was removed.
* §6 / §9 #1: the printed-BU check against `bu.vsu` does not go through `CWasmSavd`. The `CSigVerifier` is handed
  to the null paper, which never runs it.
* §3.1: the start-up validation that was observed is `CPacoteArquivos::ValidarChaveEAplicacaoValida` (4625 → 5892 →
  3830). `ValidarUE` (5890) did not run.
* §3.2: the urna delay is drawn once and slept after every flush. The remainders are signed. The first flush of a
  session comes from `CEleitorVotando::IniciaCiclo`, not from `ClearKeyboardInput`. The 6-vs-4 flush count was
  reproduced.
* §3.3: `Converte(EMidia)` uses an unsigned test, so every value outside {0, 1} throws.
* Header/comments: `EscreveArquivo` opens with `out|binary` (mode 20) after `create_directories`, not `out|trunc`.
  `CEstadoGeral` also has a sub-object at +60 (dtor 857). `RemoveAll` also unlocks the form mutex residue at +28.
  The `EUrnaTurno` declared in `cpath.h` (char) conflicts with the one in `cappinfo.h` (int). The wasm supports the
  int-sized one.
