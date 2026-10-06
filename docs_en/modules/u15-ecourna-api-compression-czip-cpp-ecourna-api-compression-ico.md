# u15: `CZip` creation, the urna's GUI toolkit pieces (`api/gui`) and the fatal-error context stack

Unit u15 is a grab-bag of 110 wasm functions that the tools attributed to 22 original files:

* **`ecourna-lib/ecourna/api/compression/`** — the constructor of `CZip` and `CZip::CreateZipFile`
  (the ZIP writer behind the `.jez` result archives), plus `ICompressor::Add(map)`. The rest of `CZip`
  belongs to unit u12.
* **`uenux2/src/api/gui/`** — widgets of the urna's own GUI toolkit: the digit boxes of the voting
  screens (`CFramedText`, `CGrayedFramedText`, `CInputField<CFramedText>`, `CMaskedTextField`), image
  fields (`CImageField`, `CDSImageField`, `CImageFieldUpdate`, `CFixedImage`), animations (`CMovie`,
  `CMovieField`), a numbered menu widget (`CInputMenuField`, `CMenuItem`, `CMenuValidation`), the
  microterminal LED field (`CLedFieldMT`), the status header of every screen
  (`CFormBuilder::AddStatusHeader`: clock, battery, **"TREINAMENTO"/"SIMULADO"** label), the
  labelled control keys (`CInteractiveFormBuilder::AddLabeledInputControl`), the interactive-form
  focus logic (`CInteractiveForm<…>`), `CBmpConversor` (fingerprint bitmap flip/invert),
  `CApplication` (start-up + the two "halt" loops) and the **application-context stack** that
  decides what the urna prints on the fatal-error screen.
* **`uenux2/src/api/audio/alsa/cwavfile.cpp`** — only the two destructors of `CWavFile`.
* Seven functions that really belong to **vota** / **comum** (a zerésima-screen helper, a
  pre-show hook, three poll-worker states, `comum::CInfoMTLCD`'s constructor). The tools put them
  here because an `api/gui` function is inlined into them.

32 of the 110 functions ran during the two recorded votes (`analysis/runtime/*.functions.tsv`):
the status header, the digit boxes, the key labels, the focus logic and the context stack are on
every voter screen.

Reconstructed sources (all fragments carry the `.u15` suffix because other units own other
functions of the same files):

```
src/ecourna/api/compression/czip.u15.hpp, czip.u15.cpp, icompressor.u15.cpp
src/uenux2/src/api/audio/alsa/cwavfile.u15.cpp
src/uenux2/src/api/gui/gui-common.u15.h          shared layouts, IFormFieldBase protocol, EUeGuiError, SRect
src/uenux2/src/api/gui/primitives.u15.cpp        SRect::MoveTo / Union
src/uenux2/src/api/gui/capplication.u15.cpp
src/uenux2/src/api/gui/capplicationcontextstack.u15.h / .u15.cpp
src/uenux2/src/api/gui/cformbuilder.u15.cpp, cinteractiveformbuilder.u15.cpp, cinteractiveform.u15.h
src/uenux2/src/api/gui/cframedtext.u15.h / .u15.cpp, cgrayedframedtext.u15.cpp, cinputfield.u15.h
src/uenux2/src/api/gui/cimagefield.u15.cpp, cdsimagefield.u15.cpp, cimagefieldupdate.u15.cpp, cfixedimage.u15.cpp
src/uenux2/src/api/gui/cmovie.u15.cpp, cmoviefield.u15.cpp, cbmpconversor.u15.cpp, cledfieldmt.u15.cpp
src/uenux2/src/api/gui/cinputmenufield.u15.h / .u15.cpp, cpaperformbuilder.u15.h, cpowerinformation.u15.cpp
src/uenux2/src/app/comum/cinfomtlcd.u15.cpp
src/uenux2/src/app/vota/u15-foreign-fragments.cpp  (3059, 5416, 5418, 7707, 10588, 10624)
```

---

## 1. Classes and how they relate (RTTI)

```
ecourna::api::pattern::NonCopyable ─┐
ecourna::api::pattern::IObservableProgressWithDescription ─┴─ compression::ICompressor (vmi, vtable @1110896)
                                                                 └─ compression::CZip       (vtable @1110676, 40 bytes)
api::IFormFieldBase<IScreen> (vtable @1537360)
 └─ api::IFormField<IScreen>
     ├─ CImageField (@1578792, 44 B) ── CDSImageField (vmi + IObserver<shared_ptr<IImage>>, @1578868 / @1578920)
     ├─ CImageFieldUpdate (@1578988, 52 B)
     ├─ CMovieField (@1579440)
     ├─ CMaskedTextField<CFramedText> (@1537916), CMaskedTextField<CGrayedFramedText>
     └─ IInputField<IScreen> (@1538852, 64 B)
          ├─ CInputField<CFramedText> (@1538768, 92 B) ── CInputMenuField (@1583364, 172 B)
          └─ CInputFieldControlBase<IScreen> (@1577424) ── CInputFieldControl<IScreen> (@1577340)
api::IFormField<IScreenMT> ── CLedFieldMT (@1579692)
api::CFramedText (@1578700, 28 B) ── CGrayedFramedText (@1578752)
api::IInputValidation (@1538704) ── CMenuValidation (@1583308), CControlValidation (@1577484), CNumberValidation
api::IForm<IScreen> (@1578040) ── CInteractiveForm<IScreen, IInputKbd> (@1579176, 92 B)
api::IForm<IScreenMT>          ── CInteractiveForm<IScreenMT, IInputMT> (@1580288)
api::IObservable<shared_ptr<IImage>> ── BatteryIconDataSource<Horizontal> (@1581580), <Vertical> (@1551916)
api::IObserver<shared_ptr<IImage>>   ── comum::CInfoMTLCD (@1551828)
std::numpunct<char> ── CApplication::InitApplication(...)::commaAsDecimalSeparator (@1577272)
non-polymorphic: CApplicationContext (52 B), CApplicationContextStack (global vector @1839212),
                 CApplicationContextGuard (56 B), CMenuItem (112 B: members end at +108, 8-byte
                 aligned because of its std::function; list node 120 B), CMovie (20 B), CBmpConversor (static)
error type: ecourna::api::exception::CBaseError<api::EUeGuiError, SErrorLimits{4900, 5100}>
            (typeinfo @1529376, vtable @1529396, ctor thunk = func 406). There is no derived
            "CUeGuiError" class in the RTTI (unlike api::CUeDesligandoError); the sources use that name
            only as a local alias.
```

**Field protocol** (every `IFormField` vtable; names inferred where marked in `gui-common.u15.h`):
0/1 destructors, 2 `Draw(MEDIA&) const`, 3 `Start()`, 4 `Stop()`, 5 `RedrawIfDirty(MEDIA&)` (func 4118),
6 `SetForm(FormControlBlock*)` (func 3037), 7 `GetClassName()` (the literal class name, used by
`CFormBuilder::Add` to generate unique field names such as `CImageField3`), 8 `Rect() const`,
9 `Move(const SPoint&)`; input fields add 10 `Read(IInput&)`, 11 `Clear()`, 12 `SetLength(size_t)`.
The recurring *Invalidate* idiom is `if (m_form && m_form->ativo) { m_dirty = true; m_form->RequestRedraw(); }`.

**Form protocol** (`IForm<MEDIA>`): 2 `Show()` (clears the global form stack @1832632 / @1832892 and
activates the form), 3 `ShowOnTop()`, 4 `Redraw()`, 5 `GetRenderForm()`, 6 `SetFocus(IInputField*)`,
7 `OnActivate()`. Names of 2/3/7 are inferred; other units call slot 2 `Exibe()`.

**Screen coordinates.** The urna screen is 640×480 (`ms_areaTeclas` @1577200 = {0,0,639,479}); the web
canvas is 1280×800, so the mock scales x by 2 and y by 5/3 (the simulator log shows
`fillText("TREINAMENTO", 640, 78)` for a label drawn at (320, 30)). `SPoint` is passed as one i32
(x low, y high), `SRect` as one i64.

---

## 2. Subsystems and control flow

### 2.1 `CZip`: how a `.jez` archive is opened (funcs 5193, 9543)

`.jez` files are ZIP archives (minizip over zlib, deflate only; see
`docs/libraries/compression-7zip-lzma-zlib.md`). The two writers are `comum::CGravadorWSQ` (func 5821:
fingerprint images, level 0 "stored"; level 2 on the path that creates and closes an archive with
nothing in it) and `comum::CGravadorLog` (func 11584: log files into `temp.jez`, level 2 "default").
Only 5821 calls func 5193. 11584 has the constructor inlined: it stores the `CZip` vtable itself and runs
`CreateZipFile` through `invoke_vi` (table slot 6119).

1. `CZip(path, level)` (5193) builds `ICompressor` (a progress signal), copies the path to +16, stores
   `level` at +32, `m_zip = nullptr` (+28) and `m_modoAbertura = 0` (+36), then calls
   `CreateZipFile()` inside a `try` (invoke_vi).
2. `CreateZipFile()` (9543): `ConvertToOpen()` (inlined, czip.cpp:155) maps 0 → `APPEND_STATUS_CREATE`,
   1 → `APPEND_STATUS_ADDINZIP`, anything else → `CBaseError<ECompressionError>(1040, "Modo de abertura do
   zip incorreto.")`. Then minizip `zipOpen3` is inlined (`fill_fopen64_filefunc`, table slots
   8253..8259). If the handle is null: error **1041** `"O arquivo {} não pode ser criado."` (czip.cpp:170).
   Because the mode is always 0, the 3 KB of inlined ADDINZIP code (end-of-central-directory search,
   zip64 locator "PK\6\7", reloading the central directory in 4080-byte blocks) is dead in this binary.
3. `ICompressor::Add(map<path,path>)` (9529) calls the virtual `DoAdd(src, dst)` (slot 4, u12) for each
   pair, in key order, and returns `*this`. Only `CGravadorLog` uses it.

The simulator never reaches these writers during a vote (they run at encerramento), so no `.jez`
appears in the MEMFS dumps.

### 2.2 Start-up: `CApplication::InitApplication` (11159, called once by `main`)

1. Stores the application identity in statics: name @1839168 ("VOTA"), description @1839180
   ("Software de Votação"), version @1839192 ("10.23.0.1 - DESENVOLVIMENTO"), `ELogAplicativos` @1839204;
   resets the int @1577212 (initial -1) to -1.
2. Installs a global locale whose `numpunct<char>` returns **','** as decimal point (local class
   `commaAsDecimalSeparator`, func 11156). `std::locale::global` is inlined: `setlocale(LC_ALL, name)`
   when the name is not `"*"`.
3. Clears the application-context stack and pushes the generic fallback context
   `("Não é possível continuar a execução", "Erro inesperado", QR-code hint, ["Desligue e ligue a urna."])`.

### 2.3 The application-context stack (funcs 676, 3682, 5555, 5557, 5558; 3684 in u33)

*Contexto da aplicação* = the text shown when an operation fails. Risky steps open a guard:

```cpp
api::CApplicationContextGuard contexto(2 /*Actions*/, "", "Gerando boletim de urna na MI",
                                       "Ocorreu um erro durante a geração do boletim de urna na MI.");
```

* `CApplicationContext` (52 bytes): `+0 detalhe`, `+12 título`, `+24 mensagem`, `+36 vector<string> ações`,
  `+48 bool genérico`. The field order was checked against the consumer: `vota::CThreadMonitor::vf3`
  (func 7710) formats `std::format("{} ({})", título, código do erro)` from the stack top.
* `Actions` → text (5555): 17 codes, from "Desligue a urna." (0) to the ADH clock-adjustment
  instruction (16); unknown codes give `"Actions({}) - não reconhecida"`. Code 2 ("Desligue e ligue a
  urna." + "Se o erro persistir, substitua a urna.") is what the BU/result writers use; 4 adds
  "…substitua a mídia de votação.". Full table in `capplicationcontextstack.u15.h`.
* Push rule (inlined in the guard, and in func 3684): a **generic** context can only be pushed on an
  empty stack or on another generic context; otherwise the guard throws
  `CUeGuiError(5013, "O contexto não pôde ser criado")` (capplicationcontextstack.cpp:45).
* Callers of the push (3684) include `CEleitorVotando::IniciaCiclo` and `CPedeDigital::StartState`;
  3682, 5555, 5558 and 676 all ran during the recorded votes.

### 2.4 What happens after a fatal error (funcs 3683, 5566; outside the unit: 5568, 7710, 11151)

On the real urna, `vota::CThreadMonitor::vf3` (7710) shows the error screen, calls
`CApplication::ShowExceptionMsg` (func 5568), which starts `std::async(std::launch::async, …)` running
`EnterLoopBeeping()` — an **SOS in Morse** at 800 Hz (func 3683 is the lambda, the loop is inlined in
11151). The calls are `Beep(800, 10)` ×3, `Beep(800, 30)` ×3, `Beep(800, 10)` ×3. `IBeep`'s duration
unit is **10 ms**: the web `CWasmBeep::vf1` (func 8538) calls `js_wasm_beep_queue(frequency, durationMs)`
with `duração * 10`. So the dots last 100 ms and the dashes 300 ms (the usual 1:3 Morse ratio). The waits
are raw milliseconds: `duração + 120` ms after each beep (130 / 150 ms), 360 ms after each letter and
1.16 s between repetitions. The monitor then parks in `EnterLoopDoingNothing()` (5566): once per second,
refresh the screen (IScreen slot 37) and sleep, until `ms_encerrar` (@1839209) or `ms_codigoSaida > 0`
(@1577212). Nothing in the binary writes `ms_encerrar`, and `ms_codigoSaida` is only ever set to -1
(by `InitApplication`), so both loops are infinite by design (the operator must switch the urna off).

**In the web build this path is different:** func 5568 always throws
`std::system_error("thread constructor failed")` (no pthreads: the compiler proved `pthread_create`
fails, so the body is `operator new` + throw), so neither the SOS nor `EnterLoopDoingNothing` runs; and if
they did, their `std::this_thread::sleep_for` became `if (byte@1584624 == 1) emscripten_sleep(ms)`,
which aborts without Asyncify.

### 2.5 Status header and mode label (502, 5564, 4620, 7707)

`AddStatusHeader(campos)` (502) reads the screen width (IScreen slot 30) and adds, by bit:
`1` date/time `"A DD/MM/YYYY hh:mm:ss"` at (5,5) refreshed every 500 ms (data source 3117);
`8` battery percentage `"{: >3}%"` (empty when there is no battery reading) at width-70 or -55;
`4` battery icon (`CDSImageField` over `BatteryIconDataSource<Horizontal>`, 1 s timer, icons
`:/resource/images/bateria/img-*.jpg`) at (width-55, 5) — pushed into the field vector **without**
`Add()`, so it gets no unique name; `2` the mode label at 53 % of the width.
Screens normally pass `5` (clock + battery icon). Of the 34 direct calls, 32 pass 5, func 1255 passes 7
and func 7787 passes 4. No caller sets bit `8`, so the battery-percentage text is unreachable in this
build.

The mode label (`GetModoUrnaTexto`, 5564) is `"DEMONSTRAÇÃO"` if the bool @1839208 is set (never, in this
binary), else `"SIMULADO"` for phase '2', `"TREINAMENTO"` for phase '3', nothing for the official phase.
The phase int @1577208 is copied from `CEstadoGeral` +48 at initialisation (func 7787). The pre-show
hooks draw it directly on the screen (4620): `CPreShowFormVota::PreShow` (7707) at (340,5) and
`CPreShowProgressBar::PreShow` at (320,30). **Confirmed by running the simulator**
(`node tools/run/headless.mjs --scenario municipal-t1 --keys 9 --draw`): the first voting screen draws
`fillText("TREINAMENTO", 640, 78)` — every simulator scenario is phase `te` (treinamento).

### 2.6 The digit boxes of the voting screens (1385, 2780, 5534, 5536, 12387…12708)

`CFramedText(n, pos, font, align)` (1385) asks the screen for the font's largest glyph (IScreen slot 2),
adds a spacing of `size<=7 ? 1 : size/8` on each side, a height margin of `size<=9 ? 2 : 2*(size/10)`,
and shifts `pos.x` left by the total width (align 1) or half of it (align 2 = centred).
`MaskText` (2780) throws `4917 "Texto [..] eh grande demais"` if the text is longer than the boxes, then
pads with blanks: blank boxes are filled white with the normal frame (colour 2), typed ones get the
character and frame colour 3. `CGrayedFramedText::MaskText` (5534, observed) fills unused boxes grey
(colour 5) — these are the number boxes of the candidate screens. The input variant
(`CInputField<CFramedText>`, 12438) also draws the blinking cursor frame; `SetLength` (12387) resizes
the boxes and clears the text.

### 2.7 Labelled control keys and focus (653, 5565, 1401, 11074…11079)

`AddLabeledInputControl(teclas)` (653) accepts at most 3 `(tecla, rótulo)` pairs (`4937 "O número máximo
de teclas suportado é {}"`), only `B`/`C`/`D` = BRANCO/CONFIRMA/CORRIGE (`4936 "Tecla de controle não
suportada '{}'"`). Positions are taken from the back of `{centro, esquerda, direita}`: the first key goes
right, the second left, the third centred (`GetLabeledKeyPos`, 5544: x = left+23 / right-19 / middle,
y = bottom+dy-47). A 3-px line is drawn at y = 429. The function returns the first input field already
in the form or creates an invisible `CInputFieldControl` (5565) that finishes on the keys of the mask
(B=1, D=2, C=4, table @524056).

`CInteractiveForm::Show()` (11077, observed) clears every input, focuses the first one, replaces the form
stack and, if the form's flag +88 is set, flushes the keyboard (`ClearKeyboardInput`, cinteractiveform.h:148,
`IInputKbd::Clear` → JS `wasm_input_clear`). `SetFocus(i)` (1401) moves the 600 ms cursor-blink timer
between fields.

### 2.8 The menu widget `CInputMenuField` (1693, 2770, 3655, 3657, 3675, 5489–5492, 10893–10900)

A numbered list for the voter screen ("visualizar candidatos" menus of vota): items are laid out in
columns (a new column starts 20 px right of the widest item when the next one would pass
`min(screen height, top + alturaMáxima)`), shown as `"[%S] - %T"` (search text, item text; expanded with a
static `std::regex("%[ST]")`), followed by the instruction `"Digite a sua opção: "` and a framed input box
sized to the longest search text. Max 99 items (4933); an item that does not fit rolls back and throws
4934; duplicate search texts throw 4929. `CMenuValidation::IsValid` (10900) accepts a full-length text
equal to a visible item's search text or a prefix of one, and beeps (IBeep slot 6) otherwise.
`Read()` (10894) is **blocking**: it clears the keyboard buffer, then polls `HasKey()` with 5 ms sleeps,
highlights matches while typing and blinks the chosen item on CONFIRMA (2 × 100 ms). None of this code
ran in the recorded votes.

### 2.9 Images, animations, LEDs

* `CFixedImage(nome)` (2246, observed): `":…"` names come from the packaged resources (IResource; web:
  `CWasmResource`, which logs every load through `js_resource_log`), other names are read with
  `CFile::ReadFileBinary` (e.g. voter photos); empty name → 4913.
* `CImageField` / `CDSImageField` / `CImageFieldUpdate`: static image, image fed by an observable data
  source (battery), image re-read on a timer (QR codes that cycle through pages every 15 s).
* `CMovie` (5526) requires at least one frame (4943). `CMovieField::Start` (11061) does **not** touch the
  frame timer: it sets the playing flag (+40), rewinds the movie to frame 0 and asks for a redraw.
  `Stop` (11060) stops the timer (ITimer slot 3) and clears the flag. The timer is created by the
  constructor (func 5543, other unit) with period 0 and is not started there; its callback (func 11056)
  advances the frame, invalidates the field and re-arms the timer with the remaining frame time
  (ITimer slot 5).
* `CLedFieldMT::Draw` (11050): LED operation 0..3 → IScreenMT slots 8..11 (≥ 4 throws 4940). The web mock
  collapses the four operations to on/off.
* `CBmpConversor::VerticalFlip` / `InvertColors` (3664/3663): row swap and bitwise NOT of the fingerprint
  image, called by the three `ProcessTick`s that read fingerprints (voter, poll worker, operator).

### 2.10 Misattributed vota / comum functions

* **3059** — body of the zerésima-time screens (`CriaTelaConfirmaImpressaoZeresima`,
  `CriaTelaAntesHorarioZeresima`): status header, zerésima header (func 4160), `"RESUMO DA CORRESPONDÊNCIA:
  …"`, a **rotating QR code with the urna identity** (tags `SERT`, `IDFL`, `SERI`, `NOME`, `UNFE`, `IDCA`
  … from `CEstadoGeral`, 148 px at (626,250), new page every 15 s), three text lines,
  `"Versão: 10.23.0.1 - DESENVOLVIMENTO"` and `"Dados: <8 Base64 chars of the package hash>"`.
* **7707** `CPreShowFormVota::PreShow`: clear + mode label.
* **10588** `IConfirmaJustificativa::ProcessInput` (poll-worker microterminal, *justificativa* flow):
  CORRIGE → back to `CPedeIdentidade`; CONFIRMA → if the voter was identified by **CPF** (tipo 2) the
  singleton screen "Não é permitido justificar com o CPF / utilize o número do título / CORRIGE: retornar",
  otherwise `CPedeAnoNascimento` (5418: "Digite o ANO de nascimento: ", 4 digits,
  "CORRIGE: cancelar", "CONFIRMA: justificar").
* **10624** `IEleitorImpedidoVotar::ProcessInput`: the "voter cannot vote here" screens return to
  `CPedeIdentidade` on CONFIRMA or CORRIGE depending on a per-screen field (+20).
* **5904** `comum::CInfoMTLCD::CInfoMTLCD()`: battery icon on the microterminal LCD.

---

## 3. Data read and written

| what | where | how |
|---|---|---|
| `.jez` ZIP archives (`log.jez`, `wsq*.jez`, `temp.jez`) | result directories at encerramento | created by `CZip` (5193/9543), filled by u12's `DoAdd` |
| resource images/movies `:/resource/images/…` | packaged in the web `.data` bundle | `CFixedImage`, `CMovie` via `IResource` |
| voter photos, other image files | urna storage | `CFile::ReadFileBinary` in `CFixedImage` |
| `CEstadoGeral` +48 (phase), +60 (correspondence), +168 (package hash) | `eg.bin` (already loaded) | mode label, zerésima screen |
| process locale | libc | `setlocale(LC_ALL, …)` with ',' decimal point |

No SQL, no network, no ASN.1 decoding in this unit.

---

## 4. Web-build specifics and wasm observations

* **Binaryen dead-argument elimination** removed constant parameters: `CApplicationContextStack::Push`
  (3684) lost `this` (always @1839212) and its `bool` result; `AddLabeledInputControl` lost its
  `SRect` (always @1577200) and `bool`; the SOS lambda lost the 800 Hz frequency and its closure object.
* **Inlining moved names around**: 3059 was named after the inlined `CImageFieldUpdate` constructor,
  3675 after `GetInstructionTextRect`, 5904 after `BatteryIconDataSource<Vertical>()`, 10588/10624 after
  `CInteractiveForm::Read`. The real functions are listed in the mapping table.
* **ICF / merge-similar**: destructors 4021/5517 share body 6061 (vtables passed as parameters),
  `CFixedImage` destructors use merged bodies 3936/3937, `SetFocus` (1401) serves both form
  instantiations, the error constructor thunk 406 feeds the shared `CBaseError` body 710.
* Strings are **Latin-1** in the data segment ("Não é possível continuar a execução" is 35 bytes).
* `std::async` is compiled but can never start a thread (5568 throws unconditionally).
* `emscripten_sleep` call sites in this unit: 5566 (1000 ms), 3683 (dur+120 and the pause),
  10894 (5 ms poll + 2×100 ms blink). All guarded by the byte @1584624 (= 1), all abort if reached.

---

## 5. Relation to the BU (boletim de urna) and the zerésima

This unit does not build the BU, but three pieces take part in the end-of-day/zerésima flow:

1. **Error handling around BU generation.** `vota::CGeraBU::StartState` (12110) wraps the writing of
   `bu.dat` in `CApplicationContextGuard(2, "", "Gerando boletim de urna na MI", "Ocorreu um erro durante a
   geração do boletim de urna na MI.")`; `CGravaResultado` (12098), `CGeraRelatorios` (12105),
   `CCopiaResultadoParaMR` (12134) and `comum::AssinarUE` (1277) do the same for the result files,
   signatures and MR copy. If any step throws, the monitor shows
   `"<título> (<código>)"`, `mensagem` and the instruction lines of Actions 2
   ("Desligue e ligue a urna." / "Se o erro persistir, substitua a urna.").
2. **Result archives.** Logs and fingerprint images that go with the BU in the result media are ZIP
   (`.jez`) archives produced by `CZip` (§2.1).
3. **Zerésima screens.** Before the zerésima time the urna shows func 3059's screen: the correspondence
   summary, the urna-identity QR code, the software version and the data-package hash, with the option
   "Emissão do estado da urna" (limited by the maximum number of printed copies) and "Mais informações".

---

## 6. Complete mapping table (110 functions)

Functions outside the unit that were reconstructed for context: 3684 (`CApplicationContextStack::Push`,
owned by u33) and the inlined bodies of 11151 (`EnterLoopBeeping`), 5537/2779 (`DesenhaCaracter` /
`DesenhaMoldura`) are described in the fragments but not claimed here.

| func | size | ran | tools name | reconstructed symbol | reconstructed in (src/…) | original file | conf. |
|---:|---:|:-:|---|---|---|---|---|
| 406 | 18 |  | `api_f406` | `ecourna::api::exception::CBaseError<api::EUeGuiError, SErrorLimits{4900, 5100}>::CBaseError` (merged ctor thunk: passes vtable @1529396 to body 710) | src/uenux2/src/api/gui/gui-common.u15.h | uenux2/src/api/gui (EUeGuiError helper; path inferred) | high |
| 502 | 2131 | ✓ | `api::CFormBuilder::AddStatusHeader` | `api::CFormBuilder::AddStatusHeader` | src/uenux2/src/api/gui/cformbuilder.u15.cpp | uenux2/src/api/gui/cformbuilder.cpp | high |
| 653 | 1531 | ✓ | `api::CInteractiveFormBuilder::AddLabeledInputControl` | `api::CInteractiveFormBuilder::AddLabeledInputControl` | src/uenux2/src/api/gui/cinteractiveformbuilder.u15.cpp | uenux2/src/api/gui/cinteractiveformbuilder.cpp | high |
| 676 | 1434 | ✓ | `api::CApplicationContextGuard::CApplicationContextGuard` | `api::CApplicationContextGuard::CApplicationContextGuard` | src/uenux2/src/api/gui/capplicationcontextstack.u15.cpp | uenux2/src/api/gui/capplicationcontextstack.cpp | high |
| 1263 | 235 | ✓ | `api_f1263` | `api::CriaParIcones` | src/uenux2/src/api/gui/cpowerinformation.u15.cpp | uenux2/src/api/gui/cpowerinformation.cpp (path inferred) | low |
| 1385 | 260 | ✓ | `api::CFramedText::CFramedText` | `api::CFramedText::CFramedText` | src/uenux2/src/api/gui/cframedtext.u15.cpp | uenux2/src/api/gui/cframedtext.cpp | high |
| 1401 | 372 | ✓ | `api_f1401` | `api::CInteractiveForm::SetFocus` | src/uenux2/src/api/gui/cinteractiveform.u15.h | uenux2/src/api/gui/cinteractiveform.h | medium |
| 1693 | 1335 |  | `api::CInputMenuField::AddItem` | `api::CInputMenuField::AddItem` | src/uenux2/src/api/gui/cinputmenufield.u15.cpp | uenux2/src/api/gui/cinputmenufield.cpp | high |
| 1915 | 70 |  | `api_f1915` | `api::SRect::MoveTo` | src/uenux2/src/api/gui/gui-common.u15.h | uenux2/src/api/gui/primitives.cpp (path inferred) | medium |
| 1916 | 150 |  | `api_f1916` | `api::Union` | src/uenux2/src/api/gui/primitives.u15.cpp | uenux2/src/api/gui/primitives.cpp (path inferred) | medium |
| 2242 | 99 |  | `api::CImageField::CImageField` | `api::CImageField::CImageField` | src/uenux2/src/api/gui/cimagefield.u15.cpp | uenux2/src/api/gui/cimagefield.cpp | high |
| 2246 | 491 | ✓ | `api::CFixedImage::CFixedImage` | `api::CFixedImage::CFixedImage` | src/uenux2/src/api/gui/cfixedimage.u15.cpp | uenux2/src/api/gui/cfixedimage.cpp | high |
| 2770 | 219 |  | `api_f2770` | `api::CInputMenuField::AtualizaLayout` | src/uenux2/src/api/gui/cinputmenufield.u15.cpp | uenux2/src/api/gui/cinputmenufield.cpp | low |
| 2778 | 368 |  | `api::CDSImageField::vf0@2778` | `api::CDSImageField::~CDSImageField` | src/uenux2/src/api/gui/cdsimagefield.u15.cpp | uenux2/src/api/gui/cdsimagefield.cpp | high |
| 2780 | 766 | ✓ | `api::CFramedText::MaskText` | `api::CFramedText::MaskText` | src/uenux2/src/api/gui/cframedtext.u15.cpp | uenux2/src/api/gui/cframedtext.cpp | high |
| 3059 | 2801 | ✓ | `api::CImageFieldUpdate::CImageFieldUpdate` | `vota::(anonymous namespace)::adicionaBlocoMensagem` | src/uenux2/src/app/vota/u15-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | medium |
| 3655 | 648 |  | `api::CMenuItem::SetSearchText` | `api::CMenuItem::SetSearchText` | src/uenux2/src/api/gui/cinputmenufield.u15.cpp | uenux2/src/api/gui/cinputmenufield.cpp | high |
| 3657 | 215 |  | `api::CMenuItem::UpdateRect` | `api::CMenuItem::UpdateRect` | src/uenux2/src/api/gui/cinputmenufield.u15.cpp | uenux2/src/api/gui/cinputmenufield.cpp | high |
| 3663 | 234 |  | `api::CBmpConversor::InvertColors` | `api::CBmpConversor::InvertColors` | src/uenux2/src/api/gui/cbmpconversor.u15.cpp | uenux2/src/api/gui/cbmpconversor.cpp | high |
| 3664 | 274 |  | `api::CBmpConversor::VerticalFlip` | `api::CBmpConversor::VerticalFlip` | src/uenux2/src/api/gui/cbmpconversor.u15.cpp | uenux2/src/api/gui/cbmpconversor.cpp | high |
| 3675 | 1152 |  | `api::CInputMenuField::GetInstructionTextRect` | `api::CInteractiveFormBuilder::AddInputMenu` | src/uenux2/src/api/gui/cinteractiveformbuilder.u15.cpp | uenux2/src/api/gui/cinteractiveformbuilder.cpp (path inferred) | low |
| 3682 | 333 | ✓ | `vota_f3682` | `api::CApplicationContext::CApplicationContext(Actions,string,string,string)` | src/uenux2/src/api/gui/capplicationcontextstack.u15.cpp | uenux2/src/api/gui/capplicationcontextstack.cpp | medium |
| 3683 | 253 |  | `api::CApplication::EnterLoopBeeping()::(lambda)::operator()` | `api::CApplication::EnterLoopBeeping()::(lambda)::operator()` | src/uenux2/src/api/gui/capplication.u15.cpp | uenux2/src/api/gui/capplication.cpp | high |
| 4021 | 17 |  | `api::IInputField<api::IScreen>::vf0` | `api::IInputField<api::IScreen>::~IInputField` | src/uenux2/src/api/gui/cinputmenufield.u15.cpp | uenux2/src/api/gui/iinputfield.h (path inferred) | high |
| 4620 | 229 | ✓ | `api_f4620` | `api::CFormBuilder::DesenhaModoUrna` | src/uenux2/src/api/gui/cformbuilder.u15.cpp | uenux2/src/api/gui/cformbuilder.cpp (path inferred) | low |
| 5193 | 238 |  | `ecourna::api::compression::CZip::CZip` | `ecourna::api::compression::CZip::CZip` | src/ecourna/api/compression/czip.u15.cpp | ecourna-lib/ecourna/api/compression/czip.cpp | high |
| 5345 | 53 |  | `api::CWavFile::vf0` | `api::CWavFile::~CWavFile` | src/uenux2/src/api/audio/alsa/cwavfile.u15.cpp | uenux2/src/api/audio/alsa/cwavfile.cpp | high |
| 5416 | 20 |  | `vota_f5416` | `vota::TipoIdentificacaoEleitor` | src/uenux2/src/app/vota/u15-foreign-fragments.cpp | uenux2/src/app/vota/operador (path inferred) | low |
| 5418 | 1000 |  | `vota_f5418` | `vota::CPedeAnoNascimento::GetInst` | src/uenux2/src/app/vota/u15-foreign-fragments.cpp | uenux2/src/app/vota/operador/confirmaidentidade/cpedeanonascimento.cpp (path inferred) | medium |
| 5489 | 333 |  | `api_f5489` | `api::CInputMenuField::SelecionaPorTextoBusca` | src/uenux2/src/api/gui/cinputmenufield.u15.cpp | uenux2/src/api/gui/cinputmenufield.cpp | low |
| 5490 | 156 |  | `api_f5490` | `api::CMenuItem::~CMenuItem` | src/uenux2/src/api/gui/cinputmenufield.u15.cpp | uenux2/src/api/gui/cinputmenufield.cpp | medium |
| 5491 | 167 |  | `api::CInputMenuField::vf0` | `api::CInputMenuField::~CInputMenuField` | src/uenux2/src/api/gui/cinputmenufield.u15.cpp | uenux2/src/api/gui/cinputmenufield.cpp | high |
| 5492 | 1445 |  | `api_f5492` | `api::CMenuItem::GetDisplayText` | src/uenux2/src/api/gui/cinputmenufield.u15.cpp | uenux2/src/api/gui/cinputmenufield.cpp | low |
| 5517 | 17 |  | `api::IInputField<api::IScreenMT>::vf0` | `api::IInputField<api::IScreenMT>::~IInputField` | src/uenux2/src/api/gui/cinputmenufield.u15.cpp | uenux2/src/api/gui/iinputfield.h (path inferred) | high |
| 5524 | 176 |  | `api::CMovieField::vf0` | `api::CMovieField::~CMovieField` | src/uenux2/src/api/gui/cmoviefield.u15.cpp | uenux2/src/api/gui/cmoviefield.cpp | high |
| 5526 | 148 |  | `api::CMovie::CMovie` | `api::CMovie::CMovie` | src/uenux2/src/api/gui/cmovie.u15.cpp | uenux2/src/api/gui/cmovie.cpp | high |
| 5531 | 175 |  | `api::CImageFieldUpdate::vf0` | `api::CImageFieldUpdate::~CImageFieldUpdate` | src/uenux2/src/api/gui/cimagefieldupdate.u15.cpp | uenux2/src/api/gui/cimagefieldupdate.cpp | high |
| 5533 | 201 |  | `api_f5533` | `api::CImageField::SetImage` | src/uenux2/src/api/gui/cimagefield.u15.cpp | uenux2/src/api/gui/cimagefield.cpp (path inferred) | medium |
| 5534 | 269 | ✓ | `api::CGrayedFramedText::MaskText` | `api::CGrayedFramedText::MaskText` | src/uenux2/src/api/gui/cgrayedframedtext.u15.cpp | uenux2/src/api/gui/cgrayedframedtext.cpp | high |
| 5536 | 164 |  | `api_f5536` | `api::CFramedText::PreencheCaixa` | src/uenux2/src/api/gui/cgrayedframedtext.u15.cpp | uenux2/src/api/gui/cframedtext.cpp (path inferred) | low |
| 5544 | 559 |  | `api::CFormBuilder::GetLabeledKeyPos` | `api::CFormBuilder::GetLabeledKeyPos` | src/uenux2/src/api/gui/cformbuilder.u15.cpp | uenux2/src/api/gui/cformbuilder.cpp | high |
| 5554 | 561 |  | `api_f5554` | `std::vector<api::CApplicationContext>::__push_back_slow_path` | library/inlined helper | libc++ <vector> (instantiation) | high |
| 5555 | 3197 | ✓ | `vota_f5555` | `api::ActionsToText` | src/uenux2/src/api/gui/capplicationcontextstack.u15.cpp | uenux2/src/api/gui/capplicationcontextstack.cpp (path inferred) | medium |
| 5557 | 169 |  | `vota_f5557` | `api::CApplicationContext::CApplicationContext` | src/uenux2/src/api/gui/capplicationcontextstack.u15.cpp | uenux2/src/api/gui/capplicationcontextstack.cpp (path inferred) | medium |
| 5558 | 349 | ✓ | `vota_f5558` | `api::CApplicationContext::CApplicationContext(string,string)` | src/uenux2/src/api/gui/capplicationcontextstack.u15.cpp | uenux2/src/api/gui/capplicationcontextstack.cpp (path inferred) | medium |
| 5564 | 208 | ✓ | `api_f5564` | `api::CFormBuilder::GetModoUrnaTexto` | src/uenux2/src/api/gui/cformbuilder.u15.cpp | uenux2/src/api/gui/cformbuilder.cpp | medium |
| 5565 | 193 | ✓ | `api_f5565` | `api::CInputFieldControlBase<api::IScreen>::CInputFieldControlBase` | src/uenux2/src/api/gui/cinteractiveformbuilder.u15.cpp | uenux2/src/api/gui/cinputfieldcontrol.h (path inferred) | medium |
| 5566 | 138 |  | `api::CApplication::EnterLoopDoingNothing` | `api::CApplication::EnterLoopDoingNothing` | src/uenux2/src/api/gui/capplication.u15.cpp | uenux2/src/api/gui/capplication.cpp | high |
| 5904 | 656 | ✓ | `api::BatteryIconDataSource<api::CPowerInformation::IconOr…` | `comum::CInfoMTLCD::CInfoMTLCD` | src/uenux2/src/app/comum/cinfomtlcd.u15.cpp | uenux2/src/app/comum/cinfomtlcd.cpp | medium |
| 6061 | 209 |  | `api_f6061` | `api::IInputField<MEDIA>::~IInputField (shared body)` | src/uenux2/src/api/gui/cinputmenufield.u15.cpp | uenux2/src/api/gui/iinputfield.h (path inferred) | medium |
| 7707 | 61 | ✓ | `vota::CPreShowFormVota::vf2` | `vota::CPreShowFormVota::PreShow` | src/uenux2/src/app/vota/u15-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/comum/cpreshowformvota.cpp (path inferred) | medium |
| 8554 | 12 |  | `api::CFixedImage::vf1` | `api::CFixedImage::~CFixedImage (deleting)` | src/uenux2/src/api/gui/cfixedimage.u15.cpp | uenux2/src/api/gui/cfixedimage.cpp | high |
| 8563 | 12 |  | `api::CFixedImage::vf0` | `api::CFixedImage::~CFixedImage` | src/uenux2/src/api/gui/cfixedimage.u15.cpp | uenux2/src/api/gui/cfixedimage.cpp | high |
| 9529 | 105 |  | `ecourna::api::compression::ICompressor::Add` | `ecourna::api::compression::ICompressor::Add` | src/ecourna/api/compression/icompressor.u15.cpp | ecourna-lib/ecourna/api/compression/icompressor.cpp | medium |
| 9543 | 4447 |  | `ecourna::api::compression::CZip::CreateZipFile` | `ecourna::api::compression::CZip::CreateZipFile` | src/ecourna/api/compression/czip.u15.cpp | ecourna-lib/ecourna/api/compression/czip.cpp | high |
| 10238 | 13 |  | `api::CWavFile::vf1` | `api::CWavFile::~CWavFile (deleting)` | src/uenux2/src/api/audio/alsa/cwavfile.u15.cpp | uenux2/src/api/audio/alsa/cwavfile.cpp | high |
| 10588 | 1112 |  | `api::CInteractiveForm<api::IScreenMT, api::IInputMT>::Rea…` | `vota::IConfirmaJustificativa::ProcessInput` | src/uenux2/src/app/vota/u15-foreign-fragments.cpp | uenux2/src/app/vota/operador/justificativa/iconfirmajustificativa.cpp | medium |
| 10624 | 167 |  | `api::CInteractiveForm<api::IScreenMT, api::IInputMT>::Rea…` | `vota::IEleitorImpedidoVotar::ProcessInput` | src/uenux2/src/app/vota/u15-foreign-fragments.cpp | uenux2/src/app/vota/operador/leidentidade/ieleitorimpedidovotar.cpp (path inferred) | medium |
| 10893 | 61 |  | `api::CInputMenuField::vf7` | `api::CInputMenuField::GetClassName` | src/uenux2/src/api/gui/cinputmenufield.u15.h | uenux2/src/api/gui/cinputmenufield.cpp | high |
| 10894 | 783 |  | `api::CInputMenuField::Read` | `api::CInputMenuField::Read` | src/uenux2/src/api/gui/cinputmenufield.u15.cpp | uenux2/src/api/gui/cinputmenufield.cpp | high |
| 10895 | 255 |  | `api::CInputMenuField::Move` | `api::CInputMenuField::Move` | src/uenux2/src/api/gui/cinputmenufield.u15.cpp | uenux2/src/api/gui/cinputmenufield.cpp | high |
| 10896 | 61 |  | `api::CInputMenuField::vf8` | `api::CInputMenuField::Rect` | src/uenux2/src/api/gui/cinputmenufield.u15.cpp | uenux2/src/api/gui/cinputmenufield.cpp | medium |
| 10897 | 607 |  | `api::CInputMenuField::vf2` | `api::CInputMenuField::Draw` | src/uenux2/src/api/gui/cinputmenufield.u15.cpp | uenux2/src/api/gui/cinputmenufield.cpp | high |
| 10898 | 13 |  | `api::CInputMenuField::vf1` | `api::CInputMenuField::~CInputMenuField (deleting)` | src/uenux2/src/api/gui/cinputmenufield.u15.cpp | uenux2/src/api/gui/cinputmenufield.cpp | high |
| 10900 | 623 |  | `api::CMenuValidation::matchesExactly` | `api::CMenuValidation::IsValid` (slot 2; the srclocs name only the inlined `matchesExactly`/`matchesPartially`) | src/uenux2/src/api/gui/cinputmenufield.u15.cpp | uenux2/src/api/gui/cinputmenufield.cpp | medium |
| 11006 | 49 |  | `api::IFormImpressao<api::IPaperRelatorios>::GetRenderForm` | `api::IFormImpressao<api::IPaperRelatorios>::GetRenderForm` | src/uenux2/src/api/gui/cpaperformbuilder.u15.h | uenux2/src/api/gui/cpaperformbuilder.h | high |
| 11028 | 17 |  | `api::CInteractiveForm<api::IScreenMT, api::IInputMT>::vf7` | `api::CInteractiveForm<api::IScreenMT, api::IInputMT>::OnActivate` | src/uenux2/src/api/gui/cinteractiveform.u15.h | uenux2/src/api/gui/cinteractiveform.h | medium |
| 11029 | 136 |  | `api::CInteractiveForm<api::IScreenMT, api::IInputMT>::Cle…` | `api::CInteractiveForm<api::IScreenMT, api::IInputMT>::ShowOnTop` | src/uenux2/src/api/gui/cinteractiveform.u15.h | uenux2/src/api/gui/cinteractiveform.h | medium |
| 11030 | 136 |  | `api::CInteractiveForm<api::IScreenMT, api::IInputMT>::Cle…` | `api::CInteractiveForm<api::IScreenMT, api::IInputMT>::Show` | src/uenux2/src/api/gui/cinteractiveform.u15.h | uenux2/src/api/gui/cinteractiveform.h | medium |
| 11031 | 50 |  | `api::CInteractiveForm<api::IScreenMT, api::IInputMT>::vf1` | `api::CInteractiveForm<api::IScreenMT, api::IInputMT>::~CInteractiveForm (deleting)` | src/uenux2/src/api/gui/cinteractiveform.u15.h | uenux2/src/api/gui/cinteractiveform.h | high |
| 11032 | 47 |  | `api::CInteractiveForm<api::IScreenMT, api::IInputMT>::vf0` | `api::CInteractiveForm<api::IScreenMT, api::IInputMT>::~CInteractiveForm` | src/uenux2/src/api/gui/cinteractiveform.u15.h | uenux2/src/api/gui/cinteractiveform.h | high |
| 11050 | 497 |  | `api::CLedFieldMT::Draw` | `api::CLedFieldMT::Draw` | src/uenux2/src/api/gui/cledfieldmt.u15.cpp | uenux2/src/api/gui/cledfieldmt.cpp | high |
| 11059 | 17 |  | `api::CMovieField::vf7` | `api::CMovieField::GetClassName` | src/uenux2/src/api/gui/cmoviefield.u15.cpp | uenux2/src/api/gui/cmoviefield.cpp | high |
| 11060 | 29 | ✓ | `api::CMovieField::vf4` | `api::CMovieField::Stop` | src/uenux2/src/api/gui/cmoviefield.u15.cpp | uenux2/src/api/gui/cmoviefield.cpp | medium |
| 11061 | 77 | ✓ | `api::CMovieField::vf3` | `api::CMovieField::Start` | src/uenux2/src/api/gui/cmoviefield.u15.cpp | uenux2/src/api/gui/cmoviefield.cpp | medium |
| 11062 | 97 |  | `api::CMovieField::vf9` | `api::CMovieField::Move` | src/uenux2/src/api/gui/cmoviefield.u15.cpp | uenux2/src/api/gui/cmoviefield.cpp | medium |
| 11063 | 109 |  | `api::CMovieField::Rect` | `api::CMovieField::Rect` | src/uenux2/src/api/gui/cmoviefield.u15.cpp | uenux2/src/api/gui/cmoviefield.cpp | high |
| 11064 | 30 | ✓ | `api::CMovieField::vf2` | `api::CMovieField::Draw` | src/uenux2/src/api/gui/cmoviefield.u15.cpp | uenux2/src/api/gui/cmoviefield.cpp | medium |
| 11065 | 13 |  | `api::CMovieField::vf1` | `api::CMovieField::~CMovieField (deleting)` | src/uenux2/src/api/gui/cmoviefield.u15.cpp | uenux2/src/api/gui/cmoviefield.cpp | high |
| 11074 | 17 | ✓ | `api::CInteractiveForm<api::IScreen, api::IInputKbd>::vf7` | `api::CInteractiveForm<api::IScreen, api::IInputKbd>::OnActivate` | src/uenux2/src/api/gui/cinteractiveform.u15.h | uenux2/src/api/gui/cinteractiveform.h | medium |
| 11075 | 136 |  | `api::CInteractiveForm<api::IScreen, api::IInputKbd>::Clea…` | `api::CInteractiveForm<api::IScreen, api::IInputKbd>::ShowOnTop` | src/uenux2/src/api/gui/cinteractiveform.u15.h | uenux2/src/api/gui/cinteractiveform.h | medium |
| 11077 | 136 | ✓ | `api::CInteractiveForm<api::IScreen, api::IInputKbd>::Clea…` | `api::CInteractiveForm<api::IScreen, api::IInputKbd>::Show` | src/uenux2/src/api/gui/cinteractiveform.u15.h | uenux2/src/api/gui/cinteractiveform.h | medium |
| 11078 | 50 |  | `api::CInteractiveForm<api::IScreen, api::IInputKbd>::vf1` | `api::CInteractiveForm<api::IScreen, api::IInputKbd>::~CInteractiveForm (deleting)` | src/uenux2/src/api/gui/cinteractiveform.u15.h | uenux2/src/api/gui/cinteractiveform.h | high |
| 11079 | 47 |  | `api::CInteractiveForm<api::IScreen, api::IInputKbd>::vf0` | `api::CInteractiveForm<api::IScreen, api::IInputKbd>::~CInteractiveForm` | src/uenux2/src/api/gui/cinteractiveform.u15.h | uenux2/src/api/gui/cinteractiveform.h | high |
| 11085 | 73 |  | `api::CImageFieldUpdate::vf7` | `api::CImageFieldUpdate::GetClassName` | src/uenux2/src/api/gui/cimagefieldupdate.u15.cpp | uenux2/src/api/gui/cimagefieldupdate.cpp | high |
| 11086 | 10 |  | `api::CImageFieldUpdate::vf1` | `api::CImageFieldUpdate::~CImageFieldUpdate (deleting)` | src/uenux2/src/api/gui/cimagefieldupdate.u15.cpp | uenux2/src/api/gui/cimagefieldupdate.cpp | high |
| 11087 | 97 |  | `api::CImageFieldUpdate::Rect` | `api::CImageFieldUpdate::Rect` | src/uenux2/src/api/gui/cimagefieldupdate.u15.cpp | uenux2/src/api/gui/cimagefieldupdate.cpp | high |
| 11088 | 20 |  | `api::CImageFieldUpdate::vf4` | `api::CImageFieldUpdate::Stop` | src/uenux2/src/api/gui/cimagefieldupdate.u15.cpp | uenux2/src/api/gui/cimagefieldupdate.cpp | medium |
| 11089 | 20 |  | `api::CImageFieldUpdate::vf3` | `api::CImageFieldUpdate::Start` | src/uenux2/src/api/gui/cimagefieldupdate.u15.cpp | uenux2/src/api/gui/cimagefieldupdate.cpp | medium |
| 11090 | 78 |  | `api::CImageFieldUpdate::vf2` | `api::CImageFieldUpdate::Draw` | src/uenux2/src/api/gui/cimagefieldupdate.u15.cpp | uenux2/src/api/gui/cimagefieldupdate.cpp | medium |
| 11091 | 61 |  | `api::CDSImageField::vf7` | `api::CDSImageField::GetClassName` | src/uenux2/src/api/gui/cdsimagefield.u15.cpp | uenux2/src/api/gui/cdsimagefield.cpp | high |
| 11092 | 80 | ✓ | `api::CDSImageField::vf2` | `api::CDSImageField::Update (thunk)` | src/uenux2/src/api/gui/cdsimagefield.u15.cpp | uenux2/src/api/gui/cdsimagefield.cpp | medium |
| 11093 | 77 |  | `api::CDSImageField::vf10` | `api::CDSImageField::Update` | src/uenux2/src/api/gui/cdsimagefield.u15.cpp | uenux2/src/api/gui/cdsimagefield.cpp | medium |
| 11094 | 18 |  | `api::CDSImageField::vf1@11094` | `api::CDSImageField::~CDSImageField (deleting thunk)` | src/uenux2/src/api/gui/cdsimagefield.u15.cpp | uenux2/src/api/gui/cdsimagefield.cpp | high |
| 11095 | 13 |  | `api::CDSImageField::vf1@11095` | `api::CDSImageField::~CDSImageField (deleting)` | src/uenux2/src/api/gui/cdsimagefield.u15.cpp | uenux2/src/api/gui/cdsimagefield.cpp | high |
| 11096 | 10 |  | `api::CDSImageField::vf0@11096` | `api::CDSImageField::~CDSImageField (thunk)` | src/uenux2/src/api/gui/cdsimagefield.u15.cpp | uenux2/src/api/gui/cdsimagefield.cpp | high |
| 11097 | 17 |  | `api::CImageField::vf7` | `api::CImageField::GetClassName` | src/uenux2/src/api/gui/cimagefield.u15.cpp | uenux2/src/api/gui/cimagefield.cpp | high |
| 11098 | 128 |  | `api::CImageField::vf1` | `api::CImageField::~CImageField (deleting)` | src/uenux2/src/api/gui/cimagefield.u15.cpp | uenux2/src/api/gui/cimagefield.cpp | high |
| 11099 | 125 |  | `api::CImageField::vf0` | `api::CImageField::~CImageField` | src/uenux2/src/api/gui/cimagefield.u15.cpp | uenux2/src/api/gui/cimagefield.cpp | high |
| 11101 | 118 |  | `api::CImageField::Rect` | `api::CImageField::Rect` | src/uenux2/src/api/gui/cimagefield.u15.cpp | uenux2/src/api/gui/cimagefield.cpp | high |
| 11102 | 81 | ✓ | `api::CImageField::vf2` | `api::CImageField::Draw` | src/uenux2/src/api/gui/cimagefield.u15.cpp | uenux2/src/api/gui/cimagefield.cpp | medium |
| 11156 | 4 |  | `api::CApplication::InitApplication(std::basic_string<char…` | `api::CApplication::InitApplication::commaAsDecimalSeparator::do_decimal_point` | src/uenux2/src/api/gui/capplication.u15.cpp | uenux2/src/api/gui/capplication.cpp | high |
| 11159 | 1536 | ✓ | `api::CApplication::InitApplication` | `api::CApplication::InitApplication` | src/uenux2/src/api/gui/capplication.u15.cpp | uenux2/src/api/gui/capplication.cpp | high |
| 12387 | 318 |  | `api::CInputField<api::CFramedText>::SetLength` | `api::CInputField<api::CFramedText>::SetLength` | src/uenux2/src/api/gui/cinputfield.u15.h | uenux2/src/api/gui/cinputfield.h | high |
| 12402 | 70 | ✓ | `api::CInputField<api::CFramedText>::vf9` | `api::CInputField<api::CFramedText>::Move` | src/uenux2/src/api/gui/cinputfield.u15.h | uenux2/src/api/gui/cinputfield.h | medium |
| 12405 | 12 | ✓ | `api::CInputField<api::CFramedText>::vf8` | `api::CInputField<api::CFramedText>::Rect` | src/uenux2/src/api/gui/cinputfield.u15.h | uenux2/src/api/gui/cinputfield.h | medium |
| 12411 | 17 | ✓ | `api::CInputField<api::CFramedText>::vf7` | `api::CInputField<api::CFramedText>::GetClassName` | src/uenux2/src/api/gui/cinputfield.u15.h | uenux2/src/api/gui/cinputfield.h | high |
| 12438 | 83 | ✓ | `api::CInputField<api::CFramedText>::vf2` | `api::CInputField<api::CFramedText>::Draw` | src/uenux2/src/api/gui/cinputfield.u15.h | uenux2/src/api/gui/cinputfield.h | medium |
| 12593 | 82 | ✓ | `api::CMaskedTextField<api::CFramedText>::vf2` | `api::CMaskedTextField<api::CFramedText>::Draw` | src/uenux2/src/api/gui/cframedtext.u15.h | uenux2/src/api/gui/cmaskedtextfield.h (path inferred) | medium |
| 12708 | 82 | ✓ | `api::CMaskedTextField<api::CGrayedFramedText>::vf2` | `api::CMaskedTextField<api::CGrayedFramedText>::Draw` | src/uenux2/src/api/gui/cframedtext.u15.h | uenux2/src/api/gui/cmaskedtextfield.h (path inferred) | medium |

---

## 7. Weird or risky code

1. **`CInputMenuField::Read` aborts the simulator** (func 10894). It first discards pending keys
   (`IInput::Clear` → JS `wasm_input_clear`), then waits for a key with
   `while (!HasKey()) sleep_for(5 ms)`, and blinks the chosen item with two 100 ms sleeps. In this build
   each sleep is `emscripten_sleep`, which the glue implements as `abort()`. Any screen that reads a
   `CInputMenuField` (the "visualizar candidatos" menus: `CMenuVisualizarCandidatos`,
   `CMenuFiltrarCandidatosPorPartido/Cargo`) would therefore crash the module on its first read: the
   guard byte @1584624 is 1 in the data segment and no instruction stores to it, and after the
   `Clear()` no key can arrive during the synchronous call, so the first `HasKey()` is false. Not
   reached in the recorded sessions, and whether the web flow can reach these menus at all was not
   established. Medium (simulator only; on the urna this is a normal blocking read in the voter thread).
2. **Fatal-error path differs in the web build** (funcs 5568 → 3683/11151 → 5566). On the urna the
   monitor shows the error, beeps SOS forever and parks in `EnterLoopDoingNothing`. In the wasm,
   `ShowExceptionMsg` throws `std::system_error("thread constructor failed")` unconditionally, so the
   SOS and the parking loop never run, and a second exception escapes the monitor's handler. Even if
   reached, `EnterLoopDoingNothing` aborts on its first `emscripten_sleep(1000)`. Medium (simulator).
3. **Busy spin in `EnterLoopBeeping`** (inlined in 11151): when no `IBeep` singleton is registered the
   `do { if (exists<IBeep>()) {…sleep…} } while (…)` loop has no sleep at all, so it spins at 100 % CPU
   forever. Low (urna: only if the beeper is missing; the web build never starts this task, see item 2).
4. **Flags that are never written** (@1839208 `ms_demonstracao`, @1839209 `ms_encerrar`, and
   @1577212 `ms_codigoSaida`, which is only ever set to -1). The "DEMONSTRAÇÃO" status label (5564) is dead
   in this binary, and the halt loops can never end. Info.
5. **`CInputMenuField::Move` moves items by the absolute position** (10895): each item gets
   `item.pos + newPos` instead of `item.pos + (newPos - oldPos)`. A menu moved after its items were added
   draws them at the wrong place, unless it was at (0,0). Low (original code; no caller that moves a menu
   was found, but the field `Move` is a virtual call (slot 9) and its call sites were not all traced).
6. **`CInputField<MASK>::SetLength` re-aligns an already aligned position** (12387): for right-aligned or
   centred masks every resize shifts the boxes again without undoing the previous shift. The callers
   found (the menu, funcs 1693 and 3655) are left-aligned, so this is latent there; other virtual
   (slot 12) call sites were not traced. Info.
7. **Unchecked `int` products in `CBmpConversor`** (3663/3664): `largura * altura` can overflow;
   `VerticalFlip` rejects only a negative product and `InvertColors` checks nothing. The dimensions come
   from the fingerprint reader (the web mock `CFingerPrepareSimulador`), not from user input. Low.
8. **`CFixedImage` swaps with the resource's buffer** (2246): it empties the `SharedVector` returned by
   `IResource::getResourceFile`. The web `CWasmResource` returns a fresh vector each time, so nothing is
   lost; an `IResource` that cached its vectors would hand out empty images the second time. Info.
9. **Battery icon added without `Add()`** (502): the `CDSImageField` gets no unique name, so
   `FindFieldAs` cannot find it. Harmless. Info.
10. **Simulated hardware visible here**: `CLedFieldMT` (11050): the four LED operations of the
    microterminal collapse to on/off in `CWasmScreenMT`. The beeper, printer (`IPaperRelatorios`), battery
    (`IPower`) and resources are all web mocks. Info.

No network access, no URL/JSON parsing and no file-format parsing (other than minizip's inlined
end-of-central-directory search, dead here) happen in this unit.

---

## 8. Open questions

* Official names of IScreen slots 2 (glyph metrics), 5 (called with `(rect, 1)` by `Move`), 19 (draw
  text: meaning of its last two int arguments), 22/24/26 (draw image/movie), 30/31 (width/height), 37
  (no-op in `CWasmScreen`, called once per second by `EnterLoopDoingNothing`), and of IScreenMT slots 8..11.
* Names of `IForm` slots 2/3/7 (`Show`/`ShowOnTop`/`OnActivate` here; other units use `Exibe()`), of
  `IFormFieldBase` slots 3/4 (`Start`/`Stop`), and the real name of the `Actions` enumerators.
* The purpose of the rectangle at `CInputMenuField` +132 (initialised to `{pos,pos}` and never read in
  this unit), and of the `bool` parameter of `AddLabeledInputControl`.
* `comum_f5783`'s use of the `variante` argument (0 or 2) when building the urna-state QR code of func 3059.
* Unit u02's `csincronizavota.u02.cpp` passes the guard strings as `(acao, título, mensagem, "")`; the
  field order found here (`detalhe, título, mensagem`, checked against the monitor's
  `"{} ({})"` formatting) matches u07/u08's `(acao, "", título, mensagem)`.
