# u32: `api::` classes without a known source file (form fields, text sources, timers, speech synthesis)

Unit u32 collects **108 wasm functions** of component `app:api` whose classes have no `std::source_location`
record, so the tools could not attach them to an original file. They belong to 37 classes of TSE's
`uenux2/src/api` library (namespace `api::`). Almost all of them are **virtual methods found only through
vtables** (`Class::vfN`). This unit gives them real names, rebuilds the classes, and infers where each class
lives from the TSE naming convention (class `CFooBar` in `cfoobar.h/.cpp`, next to its closest relatives).
Every original path of this unit is such a guess *(path inferred)*; the reconstructed files say so in their
first lines.

Only **8 of the 108 functions ran** during the recorded votes (`analysis/runtime/*.functions.tsv`):
`CRectField::Draw` (10960, the separator line of every voting screen), `CInputFieldControl<IScreen>::GetClassName`
(11155), three `CDataText<…>::GetText` (12580 the digits the voter types, 12649 the candidate's name, 12784),
`CSystemDateTime::GetDataHora` (10855, start-up only) and two `CDefaultGenericFactory::Create` (10854, 10869).
A headless run with voter audio on (`--audio --save-audio`) additionally confirmed that the speech synthesis
path (`CRHVoiceTextToSpeech::Sintetiza`, 10841) produces the WAV format reconstructed here (§9).

The unit's contents, grouped by topic:

| topic | classes | § |
|---|---|---|
| form-field base and simple fields | `IFormFieldBase<MEDIA>`, `IFormField`, `CRectField`, `CLineField`, `CFillField`, `CMaskedTextField<MASK>`, `CPreShowClearScreen` | 3 |
| microterminal (poll worker) fields | `CTextFieldUpdateMT`, `CInputFieldMT`, `CClockFieldMT`, `CBuzzFieldMT`, `CBeepFieldMT`, `CPreShowClearMT` | 3.3 |
| printed-report fields and print jobs | `CQRCodeImageFieldPaper`, `CNewLineFieldPaper`, `CCutFieldPaper`, `CFormPart`, `CSubReport` | 3.4, 6 |
| text and image data sources | `IText`, `CFixedText`, `CDataText<SRC>`, `CDataTextFmt<SRC>`, `CDataImage<SRC>` | 4 |
| input validation | `IInputValidation`, `CMenuValidation::IsValidChar`, `IInputField::SetLength`, `CInputFieldControl<IScreen>` | 5 |
| observer pattern | `IObservable<T>` (battery icon, form-stack snapshots) | 7 |
| timers and the clock | `CTimer`, `CTimerScheduler`, `CSystemDateTime` | 8 |
| OS-primitive factories | `CDefaultGenericFactory<…>::Create` (4 instances) | 8.3 |
| speech synthesis for the accessible vote | `AudioCollector`, `CRHVoiceTextToSpeech`, `CEsperaAudio` | 9 |
| printer error | `CUePrinterError` destructors | 6.3 |

**Reconstructed sources** (all new unless marked *fragment*):

```
src/uenux2/src/api/gui/iformfield.h                 IFormFieldBase<MEDIA>, IFormField<MEDIA>, IPreShow<MEDIA>
src/uenux2/src/api/gui/itext.h                      IText, SharedIText
src/uenux2/src/api/gui/cfixedtext.h                 CFixedText
src/uenux2/src/api/gui/cdatatext.h                  CDataText<SRC>, CDataTextFmt<SRC> + list of all instantiations
src/uenux2/src/api/gui/cdataimage.h                 IImage, CDataImage<SRC>
src/uenux2/src/api/gui/crectfield.{h,cpp}           CRectField
src/uenux2/src/api/gui/clinefield.{h,cpp}           CLineField
src/uenux2/src/api/gui/cfillfield.{h,cpp}           CFillField
src/uenux2/src/api/gui/cmaskedtextfield.h           CMaskedTextField<MASK>
src/uenux2/src/api/gui/cinputfieldcontrol.h         CInputFieldControlBase/CInputFieldControl<MEDIA>
src/uenux2/src/api/gui/iinputvalidation.h           IInputValidation
src/uenux2/src/api/gui/iinputfield.u32.h            fragment: IInputField<MEDIA>::SetLength
src/uenux2/src/api/gui/cinputmenufield.u32.cpp      fragment: CMenuValidation::IsValidChar
src/uenux2/src/api/gui/ctextfieldupdatemt.{h,cpp}   CTextFieldUpdateMT
src/uenux2/src/api/gui/cinputfieldmt.{h,cpp}        CInputFieldMT
src/uenux2/src/api/gui/cclockfieldmt.{h,cpp}        CClockFieldMT
src/uenux2/src/api/gui/cbuzzfieldmt.{h,cpp}         CBuzzFieldMT
src/uenux2/src/api/gui/cbeepfieldmt.{h,cpp}         CBeepFieldMT
src/uenux2/src/api/gui/cpreshowclearmt.{h,cpp}      CPreShowClearMT
src/uenux2/src/api/gui/cpreshowclearscreen.{h,cpp}  CPreShowClearScreen
src/uenux2/src/api/gui/cqrcodeimagefieldpaper.{h,cpp} CQRCodeImageFieldPaper
src/uenux2/src/api/gui/cnewlinefieldpaper.{h,cpp}   CNewLineFieldPaper
src/uenux2/src/api/gui/ccutfieldpaper.{h,cpp}       CCutFieldPaper
src/uenux2/src/api/gui/cformpart.{h,cpp}            IReportPart, CFormPart
src/uenux2/src/api/gui/reports/csubreport.{h,cpp}   ISubReport, CSubReport
src/uenux2/src/api/pattern/iobservable.h            IObserver<T>, IObservable<T>
src/uenux2/src/api/pattern/igenericfactory.u32.cpp  fragment: the 4 CDefaultGenericFactory<…>::Create
src/uenux2/src/api/util/ctimerscheduler.{h,cpp}     CTimer, CTimerScheduler
src/uenux2/src/api/util/csystemdatetime.{h,cpp}     CSystemDateTime
src/uenux2/src/api/audio/crhvoicetexttospeech.{h,cpp} AudioCollector, CRHVoiceTextToSpeech
src/uenux2/src/api/audio/cesperaaudio.h             IEsperaAudio, CEsperaAudio
src/uenux2/src/api/hwil/iimpressora.u32.cpp         fragment: CUePrinterError destructors
```

All paths are inferred except the directories: `api/gui` (attested for the relatives `ctextfield.cpp`,
`cqrcodeimagepaper.cpp`, `cinputmenufield.cpp`, `cformbuilder.cpp`), `api/gui/reports` (`creport.cpp`),
`api/util` (`itimerscheduler.h`, `isystemdatetime.cpp`), `api/pattern` (`igenericfactory.h`),
`api/audio/alsa` (`cwavfile.cpp`) and `api/hwil` (`iimpressora.h`).

Glossary used below: *urna* voting machine; *eleitor* voter; *mesário* poll worker; *terminal do mesário* / MT
the poll worker's microterminal (a 4 x 40 character LCD with keypad, LEDs and buzzer); *título (de eleitor)*
voter registration number (12 digits); *habilitação* releasing a voter to vote; *voto com áudio* / *áudio do
eleitor* the accessible vote read aloud through headphones; *BU (boletim de urna)* the per-machine result
printout/file; *via* printed copy; *zerésima* the zero report printed before voting; *RDV (registro digital
do voto)* the shuffled table of cast votes; *relatório* report; *DS* data source.

---

## 1. Where this code sits in the voting process

Everything the urna shows or prints is a **form** (`api::IForm<MEDIA>`, iform.h, unit u17) made of **fields**:

* `MEDIA = IScreen`: the 640 x 480 voter display. The voting screens (candidate number boxes, name, party,
  photo, the "SEU VOTO PARA" header, the separator line) are built by `vota::CTelasVota` (unit u07) out of the
  fields of units u15, u16 and this unit.
* `MEDIA = IScreenMT`: the poll worker's microterminal. The mesário types the voter's título
  (`CInputFieldMT`), sees the status of the biometric identification (`CTextFieldUpdateMT`), the clock
  (`CClockFieldMT`), hears the buzzer/beeps (`CBuzzFieldMT`, `CBeepFieldMT`). Every MT form clears the LCD
  first (`CPreShowClearMT`).
* `MEDIA = IPaper`: the thermal printer. The BU, the zerésima and the other reports are forms of
  `CTextFieldPaper` (u16), `CNewLineFieldPaper`, `CCutFieldPaper` and, for the BU, `CQRCodeImageFieldPaper`
  (the QR codes of the "BU DIGITAL" and "CERTIFICADO DIGITAL" sections).

The fields never store their text: they hold an `IText` (`CFixedText`, `CDataText<SRC>`, `CDataTextFmt<SRC>`)
that is re-evaluated on each draw. That is how the voting screen follows the digits being typed
(`CDataText<const std::string& (*)()>(&VotoDigitado)`, func 12580, runs on every key) and how the candidate
name appears (`CDataText<comum::CCandidaturasDSNome>`, func 12649).

For the **accessible vote**, every screen is also spoken: `vota::CVotacaoStateAudio::PlayMessage` asks
`ITextToSpeech::GetAudio(texto)` for a WAV; on a cache miss the engine `CRHVoiceTextToSpeech::Sintetiza`
(func 10841) synthesises it with RHVoice (§9), and the sound device plays it. `CEsperaAudio` is the handle
used to wait for (or cancel) the end of a message.

The **timer service** (`ITimerScheduler`) drives blinking fields, the clock and periodic refreshes. The urna's
default implementation `CTimerScheduler`/`CTimer` (one `std::thread` per timer) is in this unit; the web build
replaces it with `simulador::CWasmTimerScheduler` because it has no threads (§8).

---

## 2. Classes and hierarchy (RTTI)

```
api::IFormFieldBase<MEDIA>                               (class; vtables IScreen @1537360, MT @1579652, Paper @1580868)
 └─ api::IFormField<MEDIA>
     ├─ IScreen:   CRectField @1582180, CLineField @1579364, CFillField @1577788,
     │             CMaskedTextField<CFramedText> @1537916, CMaskedTextField<CGrayedFramedText> @1537280,
     │             IInputField<IScreen> @1538852 ─ CInputFieldControlBase<IScreen> @1577424
     │                                              └─ CInputFieldControl<IScreen> @1577340
     ├─ IScreenMT: CTextFieldUpdateMT @1590484, CClockFieldMT @1579812, CBuzzFieldMT @1579760,
     │             CBeepFieldMT @1579580, (CLedFieldMT @1579692, u15),
     │             IInputField<IScreenMT> @1580584 ─ CInputFieldMT @1587492
     │                                           └─ CInputFieldControlBase<IScreenMT> ─ CInputFieldControl<IScreenMT> @1580456
     └─ IPaper:    CQRCodeImageFieldPaper @1582080, CNewLineFieldPaper @1580908, CCutFieldPaper @1580796,
                   (CTextFieldPaper, u16)
api::IPreShow<MEDIA>  ─ CPreShowClearScreen @1577896 (IScreen), CPreShowClearMT @1579864 (IScreenMT)
api::IText            ─ CFixedText @1532648, CDataText<SRC> (12 instances), CDataTextFmt<SRC> (7 instances)
api::IImage           ─ CDataImage<SRC> (3 instances)
api::IInputValidation ─ CNumberValidation, COptionValidation, CControlValidation, CMenuValidation
api::IReportPart      ─ CFormPart @1583696 (+ comum::CParteEleitores/CParteRdv/CParteCargos, u24)
api::ISubReport       ─ CSubReport @1543180
api::IObservable<T>   ─ CFormStack<MEDIA>::CObservable (3), BatteryIconDataSource<…> (u15)
api::ITimer           ─ CTimer @1585360          (simulador::CWasmTimer in the web build)
api::ITimerScheduler  ─ CTimerScheduler @1585404 (simulador::CWasmTimerScheduler in the web build)
api::ISystemDateTime  ─ CSystemDateTime @1585244 (simulador::CWasmSystemDateTime in the web build)
api::IGenericFactory<AP> ─ CDefaultGenericFactory<AP, C> (4 instances)
api::ITextToSpeech    ─ CRHVoiceTextToSpeech @1585556 (simulador::CWasmNullTextToSpeech when audio is off)
RHVoice::client       ─ api::AudioCollector @1586036
api::IEsperaAudio     ─ CEsperaAudio @1528584
ecourna::api::exception::CBaseError<EUePrinterError> ─ CUePrinterError @1530772 ─ CUeCodedPrinterError @1583872
```

Layouts are in the reconstructed headers (`// +offset` comments). The key ones:

| class | size | members |
|---|---|---|
| `IFormFieldBase<MEDIA>` | 24 | +4 dirty flag, +8 `IForm<MEDIA>*`, +12 `std::string m_nome` |
| `CRectField` / `CFillField` | 36 | +24 `SRect`, +32 `TColor` |
| `CLineField` | 36 | +24 `SPoint` from, +28 `SPoint` to, +32 `TColor` |
| `CTextFieldUpdateMT` | 56 | +24 `SPoint`, +28 `SharedIText`, +36 last text, +48 `shared_ptr<ITimer>` |
| `CInputFieldMT` | 72 | `IInputField<IScreenMT>` (64) + +64 `SPoint`, +68 `bool` mask |
| `CQRCodeImageFieldPaper` | 44 | +24 `vector<uebyte>` bitmap, +36 width in modules, +40 scale |
| `CFixedText` | 20 | +4 alignment, +8 `std::string` |
| `CDataTextFmt<SRC>` | varies | +4 alignment, +8 `SRC`, then `std::string` format |
| `CSubReport` | 40 | +4 name, +16 via, +28 `shared_ptr<IReportPart>`, +36 `bool` |
| `CTimer` | 144 | +8 interval ms, +16 `std::function<void()>`, +40 running, +44 cv, +92 mutex, +116 `std::thread`, +120 mutex |
| `CRHVoiceTextToSpeech` | 112 | `ITextToSpeech` (96) + +96 `shared_ptr<RHVoice::engine>`, +104 `unique_ptr<voice_profile>` |
| `AudioCollector` | 24 | +8 `vector<short>` samples, +20 sample rate (16000) |

---

## 3. Form fields

### 3.1 The protocol (IFormFieldBase / IFormField)

| slot | method | notes |
|---|---|---|
| 0 / 1 | destructors | slot 0 of every field with trivial own members is the base destructor itself: 12658 (IScreen), 11052 (IScreenMT), 11013 (IPaper) — thunks into the merged body `api_f1566` |
| 2 | `Draw(MEDIA&) const` | pure |
| 3 / 4 | `Start()` / `Stop()` | default no-op (ICF 218); fields with timers override |
| 5 | `RedrawIfDirty(MEDIA&)` | func 4118 |
| 6 | `SetForm(IForm<MEDIA>*)` | func 3037 |
| 7 | `GetClassName() const` | used by `CFormBuilder::Add` to give unique names |
| 8 / 9 | `Rect()` / `Move(const SPoint&)` | only `IFormField<IScreen>` |

A detail visible only in the binary: `IFormField<IScreenMT>` and `IFormField<IPaper>` implement slot 7 with the
**name of the template itself** (`"IFormField<IScreenMT>"`, func 11051; `"IFormField<IPaper>"`, func 11014), and
none of the MT/paper field classes override it. The builders never ask for it, though: only the IScreen
builder `CFormBuilder::Add` (func 426) calls slot 7 to build the `"<class><n>"` names. The microterminal and
paper builders (funcs 728, 941, 1072/3890, 1151/3674, 1152 and 5409 → 1400, 198/3890, 1264, 2775) only push
the `shared_ptr` into the field vector, so MT and paper fields keep an **empty** `m_nome`.

### 3.2 Voter-screen fields

| class | Draw | GetClassName | Rect / Move |
|---|---|---|---|
| `CRectField` | `IScreen::DrawRect(rect, colour, 1)` (slot 9) | "CRectField" | rect / `MoveTo` (ICF 2783) |
| `CLineField` | `IScreen::DrawLine(p1, p2, colour, 1)` (slot 8 → `js_line`) | "CLineField" | bounding box of the 2 points / translate both (ICF 5527) |
| `CFillField` | `IScreen::FillRect(rect, colour)` (slot 6 → `js_fill`) | "CFillField" | rect / `MoveTo` |
| `CMaskedTextField<MASK>` | `MASK::MaskText(tela, texto)` (u15) | shared 6507 | shared 6505 / 6504 |
| `CInputFieldControl<IScreen>` | nothing | "CInputFieldControl" | empty `SRect{}` / no-op |
| `CPreShowClearScreen` (pre-show) | `IScreen::Clear(1)` | – | – |

`CRectField`'s colour member (+32) is set to 2 (black) by its only constructor (func 5501). Unit u02 named it
`m_espessura` (thickness); the Draw passes it as the colour argument of `DrawRect(rect, colour, thickness)`, and
the thickness is the literal 1 — `CWasmScreen::vf9` (9205) confirms the argument order by forwarding
`(colour, thickness)` to four `DrawLine` calls.

### 3.3 Microterminal fields (terminal do mesário)

| class | Draw → `IScreenMT` slot | web behaviour (`simulador::CWasmScreenMT`) |
|---|---|---|
| `CTextFieldUpdateMT` | 3 `Write(pos, IText)` | see below |
| `CInputFieldMT` | 3 `Write` | see below |
| `CClockFieldMT` | 12 `ShowClock(pos)` | 8757 formats the time and writes it |
| `CBuzzFieldMT(a, b)` | 6 `Buzz(a, b)` (callers: (51, 10), (52, 5)) | 8787: refresh only — no buzzer |
| `CBeepFieldMT(n)` | 7 `Beep(n)` (n = 1 or 2) | 5039: refresh only |
| `CPreShowClearMT` | 2 `Clear()` + 13 (unknown, no-op in the mock) | clears the 4 lines |

**`CTextFieldUpdateMT`** (func 10548 and constructor inlined in `CPedeIdentidade::GetInst`, func 652): a
periodic timer (300 ms in `CPedeIdentidade`) compares `m_texto->GetText()` with the last text drawn and calls
`Invalidate()` on change (lambda 10542). `Draw` first overwrites the previous text with the same number of
spaces when the new one is shorter (the LCD has no rectangle erase), writes the new text and remembers it.
The null-text check throws `std::invalid_argument("CTextFieldUpdate - campo estava com o texto nulo")` — a
plain standard exception, unlike the screen version which throws `CUeGuiError(4974)`.

**`CInputFieldMT::Draw`** (func 10726): the typed digits, padded with `_` to the maximum length; while the field
has the focus, on the "off" half of the 600 ms blink the first free `_` becomes a space (blinking cursor). A
flag at +68 would show `*` instead of the digits, but the only constructor (inlined in func 1151, always with
`CNumberValidation("0123456789")`) sets it to false.

### 3.4 Printed-report fields

| class | Draw → `IPaper` slot | used for |
|---|---|---|
| `CNewLineFieldPaper(n)` | 4 `NewLine()` n times | spacing: after "<n>a. VIA" (3 lines); before a cut 20 lines in the BU/report trailers (12110, 12105, 5579), 2 in `CortaPapel` (2882), 5596 and 11908, 1 or 8 in 5591 |
| `CCutFieldPaper` | 3 `Cut()` | end of each printed document |
| `CQRCodeImageFieldPaper` | 12 `PrintImage(bitmap, largura, escala)` | the QR codes of the BU |

In the web build `IPaper` is `simulador::CWasmNullPaper`, whose slots 3, 4 and 12 are no-ops: nothing is printed.

---

## 4. Text and image data sources

`IText` has two virtual functions besides the destructors: slot 2 `GetText()` and slot 3 `GetAlignment()`
(ICF 1661, `return +4`). The implementations:

* **`CFixedText`** — constant text. The most constructed class of the GUI (its vtable is stored by 39
  functions). Fields also build temporaries on the stack to draw computed strings.
* **`CDataText<SRC>`** — `GetText() { return m_fonte(); }`. SRC is a function pointer, a `std::function`, a
  lambda or a small functor. The interesting bodies are those where the functor's `operator()` was inlined:
  * `CDataText<vota::CEscolheOpcao::COpcaoDS>` (10685): `std::format("{}-{}", numero, descricao())` — the
    numbered entries of the poll worker's options menu.
  * `CDataText<CFormBuilder::AddStatusHeader(unsigned)::$_0>` (11107): the battery percentage of the status
    header: refresh `IPower` (slot 15); if `(estado & 6) == 4` return an empty string; else
    `std::format("{: >3}%", IPower::GetPercentualBateria())`.
* **`CDataTextFmt<SRC>`** — the same plus a format string, with **three different behaviours**:
  * source callable with the format (`std::function<std::string(const std::string&)>`, function pointers):
    `m_fonte(m_formato)` (12287, ICF 6404);
  * `CTextSource` (a `shared_ptr<std::string>` owned by a state): **printf-style**
    `snprintf(buf, 512, m_formato.c_str(), texto.c_str())` (10746); every caller passes `"%s"`;
  * value-returning sources (`comum::(anonymous)::CComparecimentoMesariosDS`, a count of registered poll
    workers): `std::vformat(m_formato, make_format_args(valor))` (5387, 10368).

  `CComparecimentoMesariosDS` exists as **three distinct types with the same RTTI name** (three vtables
  @1594036/@1594188/@1594340): an anonymous-namespace class defined in a header included by three translation
  units, whose StartState helpers were then inlined by LTO into `comum::CPedeTituloMesario::StartState` (10388).
* **`CDataImage<SRC>`** — images: `GetImage() { return m_fonte(); }`. The instance in this unit (12520) is the
  candidate photo of the "visualizar candidatos" screen: the lambda bound to a copy of `CDadosCandidato` calls
  `comum::CFotos::GetInst()` and `CFotos::GetImagem(id)` (func 3755, the photo read from the ASN.1 photo file).

`cdatatext.h` lists every instantiation of the two templates (12 `CDataText`, 7 `CDataTextFmt`) with its vtable
and functions; 5 of the `CDataText` ones (`CCandidaturasDSNumero`, `CCargoDSNomeSexoCandidato`,
`CRespostasDSNumero`, `DS_NomeCargoNeutroComEscolha`, `CPadDS<CToUpperDS<CCargoDSNome>>`) have their `GetText`
in other units.

---

## 5. Input fields and validation

* **`IInputValidation`** (16 bytes: vptr + `std::string m_caracteres`). Slot 3 `IsValidChar(c)` =
  `m_caracteres.find(c) != npos` (12473) for CNumber/COption/CControlValidation; slot 2 `IsValid(texto)` is
  the shared 4022 (every char valid). Slot 0 (12502) is the destructor of all five validation classes.
* **`CMenuValidation::IsValidChar`** (10899): a key typed in a screen menu is accepted only if it is one of
  "1234567890" **and** `IsValid(menu.texto + c)` still matches a visible item (exact match when full, prefix
  otherwise; a mismatch beeps — func 10900, unit u15).
* **`IInputField<MEDIA>::SetLength`** (6312, shared by the IScreen and IScreenMT instantiations): if the
  length changes, the typed text is discarded, the new maximum stored and the form asked to redraw.
* **`CInputFieldControl<IScreen>`**: the invisible input of screens that only wait for CONFIRMA / CORRIGE /
  BRANCO. `Rect()` is an empty rectangle (11153); `GetClassName` is "CInputFieldControl" (11155, runs on every
  voter screen).

---

## 6. Reports and print jobs

### 6.1 CFormPart

A composite report (`api::CReport`, u17) is a vector of `IReportPart`s printed in order. `CFormPart` wraps a
paper form: `Imprime()` = `m_form->Show()` (10889) — showing a paper form prints it. Built only by func 601,
whose callers are the RDV extract title (`CriaTituloExtratoRDV`, 5971), the zerésima and its summary (11946,
11943; `cgeradorresumozeresima.cpp`, unit u09), the voter list (`CImpressaoListaEleitores`, 11908) and
`CGeraRelatorios::StartState` (12105).

### 6.2 CSubReport and CLp

`CSubReport(nome, via, relatorio)` is a stack object describing one print job; the report printer front end
`api::CLp::Imprime(sub, job)` (func 3875, name inferred; unit u19's `cwasmclp.cpp`) calls the printer's slot 9, prints the
optional header part (`TemCabecalho()`/`ImprimeCabecalho()`, slots 5/6) and runs the job. Built by
`CRelVotaUtil::CortaPapel` (`("", "", false)`, only a paper cut), `CPaperFormBuilder::Show(nome, via)` (func
3671, used by the "Estado da urna" report, the PU and data-package reports) and `CImpressaoListaEleitores`
(`via = std::format("{}ª via", n)`). The header part (+28) is never set in this build. All `ISubReport`
method names are inferred (no string or srcloc names them).

### 6.3 CUePrinterError

`CUePrinterError` and `CUeCodedPrinterError` (the second only adds an `int`) share their two destructors
(8351, 4910): free the two extra strings (+52, +40), then the two strings of `ecourna::api::exception::CError`.

---

## 7. Observables

`IObservable<T>` = { vptr, `T m_valor`, `std::vector<IObserver<T>*>`, `std::mutex` }. The unit holds the six
destructors of the three `IObservable<std::vector<FormHandle<MEDIA>>>` (the published snapshot of each device's
form stack, `CFormStack<MEDIA>::CObservable`; body 3940 releases each `FormHandle`'s control block) and the two
of `IObservable<std::shared_ptr<IImage>>` (the battery icon data sources observed by `CDSImageField` and
`comum::CInfoMTLCD`).

---

## 8. Timers and the clock

### 8.1 CTimerScheduler / CTimer (the urna's default timer service)

`ITimerScheduler` declares its factory **before** its destructor: slot 0 `CreateTimer(periodo, callback)`,
slots 1/2 destructors. `CTimerScheduler::CreateTimer` (10846) = `std::make_shared<CTimer>(periodo, callback)`.

`CTimer` (ITimer slots: 2 Start, 3 Stop, 4 IsRunning, 5 SetInterval) owns a `std::thread` that waits on a
condition variable for `intervalo` ms and calls the callback:

* `Stop()` (10847): running = false; notify the condition variable; join the thread (under a second mutex).
* `IsRunning()` (10851), `SetInterval(ms)` (10848: stored at +8, used at the next wait).
* `~CTimer()` (5445/10852): `Stop()` then destroy the members (`std::terminate` if the thread were still joinable).
* `Start()` (10850, other unit): `Stop(); running = true; m_thread = std::thread(...)`. **In this build the
  `std::thread` constructor is constant-folded to its failure path**: the function ends in
  `throw std::system_error(138 /*ENOTSUP*/, "thread constructor failed")`, and the thread body was removed as
  dead code (its reconstruction in `ctimerscheduler.cpp` is marked as such).

The simulator installs `simulador::CWasmTimerScheduler` (timers advanced by `votaTick`) at the start of
`CSimuladorWasm::Executa`, before anything asks for a timer, so `CTimerScheduler` is never created in the
recorded sessions.

### 8.2 CSystemDateTime

The default `ISystemDateTime` (slot 0 `GetDataHora`, slot 1 `SetDataHora`, then the destructors):
`GetDataHora()` = `time(nullptr)` (10855); **`SetDataHora(time_t)` is empty** (10853). It is created by the
first `ISystemDateTime::GetInst()`, which happens during static initialisation: `__wasm_call_ctors` builds two
static `CDate` objects (@1833312, @1839068) through func 1382 = `gmtime_r(GetDataHora())`
(`analysis/runtime/*.edges.tsv`: 14478 → 1382 → 10855). The simulator then replaces the clock with
`CWasmSystemDateTime` (browser local time + offset).

### 8.3 OS-primitive factories

`CDefaultGenericFactory<AP, C>::Create()` = `std::make_unique<C>()` for `CPosixSemaphore` (initial count 0,
10854), `CPosixRWMutex` (10864), `CPosixMutex` (10869) and — web-specific — `simulador::CWasmThread` (10878,
constructor inlined: 16 bytes, vptr @1528400). They are what `api::CThread`, the message queues and the locks
use; the thread implementation is the cooperative mock. Its `Wait()` polls with `emscripten_sleep`, which the
glue implements as `abort()` because the module is built without Asyncify, so that path must never run (unit u18).

---

## 9. Speech synthesis for the accessible vote

`CRHVoiceTextToSpeech` is the `ITextToSpeech` installed by `votaInit` when the page enables accessibility
(`localStorage.acessibilidade === "sim"`); docs/libraries/rhvoice.md covers the engine. This unit holds the
destructor (5441/10838), the synthesis (10841) and the RHVoice client that collects the samples.

`Sintetiza(texto, parametros)` (10841, 75 KB because RHVoice's request pipeline is inlined), step by step:

1. **Latin-1 → UTF-8**: each byte ≥ 0x80 becomes `0xC0 | b >> 6`, `b & 0xBF` (the TSE strings are ISO-8859-1).
2. **Voice profile**: `SelecionaPerfil(parametros.perfil)` (func 5442). `parametros.perfil` is
   `ITextToSpeech::m_perfilVoz` (+64), which nothing sets in the recorded sessions; the empty name replaces the
   "Letícia-F123" profile chosen by the constructor with an empty profile on the first call and RHVoice falls
   back to its default (and only) voice.
3. **Document** (`RHVoice::document`, 608 bytes, built inline) with `rate = taxa / 100.0` (60…140 %),
   `pitch = parametros[+16] / 100.0` (default 100 → 1.0), `volume = 1.0` (volume is applied by the sound
   device: keys 3/9 of the accessible keypad).
4. **Synthesis into an `AudioCollector`** (a `RHVoice::client`, sample rate 16000): `play_speech` (10820)
   appends the samples to a `std::vector<short>` and returns true; `get_audio_buffer_size` (10821) returns 100;
   the event callbacks keep RHVoice's defaults (func 1908 = `return true`, an ICF body that wasm-opt also
   shares with OpenSSL's null-digest `update`). After synthesis, `done()` (slot 12) is called only if
   `get_supported_events() & 64` (event_done), which is 0 here.
5. **WAV**: a 44-byte header is assembled from immediates and `std::make_shared<CWavFile>(header, samples)`
   (the `CWavFile` constructor, `cwavfile.cpp:47`, is inlined: `malloc` + copy, error
   `CBaseError<EUeAudioError>(4851, "Falha ao alocar memória <n> bytes")`).

Verified on the WAVs saved by `node tools/run/headless.mjs --scenario municipal-t1 --audio --save-audio DIR`:

| offset | value |
|---|---|
| 0 | `RIFF`, size = data + 36 |
| 8 | `WAVE`, `fmt ` chunk of 16 bytes: PCM (1), mono (1), 16000 Hz, 32000 bytes/s, block align 2, 16 bits |
| 36 | `data`, n = 2 × samples |

Five messages were produced for a short session (0.8 s to 20.8 s, 26 KB to 665 KB, RMS ≈ 3500–4400: real speech).
The results are cached by `ITextToSpeech` (LRU + permanent cache, unit u02), so `Sintetiza` only runs on a miss.

`CEsperaAudio` (8 bytes) is the "wait for the end of the audio" handle returned by `ISound` slot 9:
`Cancela()` (9509) sets the flag that `Cancelada()` (ICF 2683) reads; the voting states cancel it when the
voter presses a key.

---

## 10. Boletim de Urna: what this unit contributes

The BU's content, signatures and files are built elsewhere (`CGeraBU`, `CGeradorBU*`, units u06–u09, u21, and
docs/10-boletim-de-urna.md). This unit supplies the **printing primitives** of the BU:

1. `vota::CGeraBU` (func 12110) builds the paper forms of the BU with `CPaperFormBuilder`. For each QR code
   payload of the "BU DIGITAL" and "CERTIFICADO DIGITAL" sections it computes the raster itself with
   `CQRCodeImagePaper::MontaImagem(400, payload)` (func 2772; u16: 1 bit per module, 2-module quiet zone, scale
   `min(400 / width, 4)` printer dots per module) and passes the result to `CPaperFormBuilder::AddQRCode`
   (func 2775), which only copies it into a new **`CQRCodeImageFieldPaper`** `{bitmap, width in modules, scale}`
   and appends it (one `AddNewLine(1)` between consecutive QR codes). Func 5591 (the "BU das outras
   obrigatórias" path) does the same.
2. The trailer ends with `AddNewLine(20)` (**`CNewLineFieldPaper(20)`**) and `AddCut()` (**`CCutFieldPaper`**).
3. When a form is shown on paper each field draws itself on `IPaper`: text lines (`CTextFieldPaper`, slot 2),
   blank lines (slot 4), the QR image (slot 12 — on the urna the driver serialises it as the image block
   `0B | u32 len | u32 largura | u8 escala | bitmap` found in the published `*-imgbu.dat` files,
   docs/bu/qrcode.md), the cut (slot 3).
4. The printed **vias** of the BU are not composed here: `CImprimindoBU::ImprimeBU(numVia, modo)` (func 2890)
   prints a small header form ("<n>a. VIA" + **`CNewLineFieldPaper(3)`**) and asks the report printer to print
   the stored, signature-checked `bu.dat` image (`IPaperRelatorios` slot 8 with "{}ª via"). Other reports that
   carry a via label go through `CSubReport` (§6.2).
5. Web build: the paper device is `CWasmNullPaper` — every one of these calls is a no-op. The simulator shows the
   BU QR codes on screen instead (`CMostraQRCodeBU`, which uses `CLineField` separators from this unit).

---

## 11. What is specific to the web build

* `CTimerScheduler`/`CTimer` cannot work (no pthreads: `std::thread` always throws) and are replaced by
  `simulador::CWasmTimerScheduler`.
* `CSystemDateTime` is only used during static initialisation; its `SetDataHora` is an empty body.
* `CDefaultGenericFactory<IThreadImpl, simulador::CWasmThread>` replaces the POSIX thread implementation.
* Microterminal devices: `Buzz`, `Beep` and slot 13 are no-ops in `CWasmScreenMT`; LEDs are drawn (u15).
* Paper devices: no printing at all (`CWasmNullPaper` / `CWasmNullPrinter`).
* Speech: RHVoice runs inside the wasm module; the voice data is a separate 19.6 MB package loaded only when
  accessibility is on.

---

## 12. Notable wasm / Emscripten observations

* **Destructors as thunks.** Most destructors of this unit are 12-byte thunks `return merged_body(this,
  vtable)`: wasm-opt's *merge-similar-functions* turned every destructor with the same member layout into one
  body that receives the vtable to store (`api_f1565/1566` for {…, std::string @+12}, `shared_f1727/1969` for
  {…, std::string @+8}, `shared_f6052/6053`, `api_f6062/6063`, `api_f3940`, `shared_f3904`, `shared_f1723`).
* **ICF across unrelated classes.** `CUePrinterError` and `CUeCodedPrinterError` share destructors;
  `IInputField<IScreen>::SetLength` and `IInputField<IScreenMT>::SetLength` are one function (6312); the four
  RHVoice event callbacks are one `return true` (1908), which is also OpenSSL's null-digest update (the legacy
  `EVP_md_null` struct @1647304, which `pkey_ecd_ctrl` (7438) compares against, and the provider's
  `nullmd_update` dispatch entry @1752060); `CEsperaAudio::Cancelada` is the same body as a getter of
  `vota::impl::CInformacaoThreadOperador` (2683).
* **Base destructor as the class destructor.** A field class whose members are trivially destructible has the
  base-class destructor (12658 / 11052 / 11013) in its own slot 0.
* **Class names as immediates.** `GetClassName()` of `CRectField`/`CLineField`/`CFillField` is an SSO string
  written with one 8-byte and one 2-byte store from a data segment (no call).
* **Constant-folded failure.** `std::thread`'s constructor inlined against Emscripten's pthread stubs leaves
  only `__throw_system_error(138, "thread constructor failed")` in `CTimer::Start`; the thread body is gone.
* **Noexcept stubs.** The calls to func 150 on `CTimer` members (+44, +92, +120) are what remains of
  `condition_variable::notify_all`, `mutex::unlock`, `~mutex`, `~condition_variable` (see libcxx-core.md §1.3);
  `mutex::lock()` vanished entirely. That residue is what allowed the member layout of `CTimer` to be inferred.
* **A 75 KB method.** `CRHVoiceTextToSpeech::Sintetiza` contains RHVoice's whole per-request path; its only
  srcloc is the inlined `CWavFile` constructor, which misled the first naming pass.

---

## 13. Suspicious or noteworthy code

| # | where | what | impact |
|---|---|---|---|
| 1 | 10850 / 10846 / 5445 | `CTimer::Start` always throws `std::system_error("thread constructor failed")` in this build | latent: any use of the default `CTimerScheduler` (e.g. if `ITimerScheduler::GetInst()` ran before the simulator registered its own scheduler) would throw from every field's `Start()`; not reachable in the recorded sessions. 10850 has no `invoke_*`, so nothing is undone on the throw: `m_ativo` stays true (`IsRunning()` then reports a timer that never runs) and the `__thread_struct` (4 + 24 bytes, func 2541) and the 8-byte thread-argument tuple leak |
| 2 | 10548 / 10542 (and `CTextSource`) | the timer callback of `CTextFieldUpdateMT` reads a `std::string` shared with the state that writes it, without a lock | on the real urna the default `CTimer` presumably runs callbacks on its own `std::thread`: a potential data race on `std::string` (torn read / crash) if the same code is used; harmless in the cooperative web build. Not verifiable here: the `CTimer` worker loop is absent from the binary, and the lambda takes only the form mutex (+28), and only after reading the text |
| 3 | 10746 | `CDataTextFmt<CTextSource>` uses `snprintf` with a runtime format and a 512-byte buffer, while the other instantiations use `std::vformat` | only `"%s"` is ever passed, so no bug today; longer texts are truncated silently, and a `{}` format would print literally |
| 4 | 10853 / 10855 | `CSystemDateTime::SetDataHora` is empty; `GetDataHora` is UTC `time()`; two static `CDate`s are initialised from it before the simulator's local-time clock is installed | simulator only: statics hold the UTC date (can differ by one day from the simulated local date near midnight); date adjustments are ignored while the default clock is active |
| 5 | 10841 (+5442) | voice-profile handling: the Latin-1 requested name is compared with the stored (UTF-8) profile name; the empty default name replaces the constructor's "Letícia-F123" profile | works only because RHVoice falls back to its single installed voice; a non-ASCII profile name would rebuild the profile on every synthesis |
| 6 | 10841 | RHVoice/utfcpp exceptions (`file_format_error`, `item_not_found`, `bad_cast`, `utf8::…`) are not caught in `Sintetiza`: the function has no `invoke_*` at all, so it has neither a `catch` nor cleanup landing pads | a throw skips every destructor of `Sintetiza` (the 608-byte document, the samples vector, the UTF-8 copy leak); the first handler is `ITextToSpeech::GetAudio`'s `invoke_viiii` (11530), which only frees its own copy of the parameters and rethrows to the voting state that asked for the audio (`CVotacaoStateAudio`); with inconsistent voice data the accessible-vote flow could abort (RHVoice's own `catch` blocks are compiled out, rhvoice.md §8.5) |
| 7 | 10726 | `CInputFieldMT` has a `*` masking mode that nothing enables | dead code; the título is shown in clear on the MT, as expected |
| 8 | 11051 / 11014 (base of this unit's MT/paper fields) | all MT fields would report the class name "IFormField<IScreenMT>" (paper fields "IFormField<IPaper>"), but no builder asks: MT and paper fields are never named (empty `m_nome`) | a name lookup on an MT/paper form cannot find any field by name; cosmetic, nothing in the binary does such a lookup through the builders |
| 9 | 12008–12014 | `CSubReport`'s header part (+28) is never set | `ImprimeCabecalho` is dead in this build |
| 10 | 10963 / 11012 / 11015 | printing primitives are no-ops in the web build | the simulator never prints the BU/zerésima; users only see the on-screen QR codes |

---

## 14. Complete mapping table (all 108 functions of u32)

"run" = observed executing during the recorded votes. D1 = complete-object destructor (vtable slot 0 or the
interface's destructor slot), D0 = deleting destructor.

| # | func | size | run | reconstructed symbol | src file |
|---|---|---|---|---|---|
| 1 | 1908 | 4 |  | `icf_ret_1_vf8`: `RHVoice::client::{sentence_starts,sentence_ends,word_starts,word_ends}` default `return true` (slots 8-11 of AudioCollector), ICF with OpenSSL's null-digest `update` (EVP_md_null @1647304, `nullmd_update` @1752060) | src/uenux2/src/api/audio/crhvoicetexttospeech.cpp (comment) |
| 2 | 3458 | 12 |  | `api::IObservable<std::vector<FormHandle<IScreenMT>>>::~IObservable()` (D1, thunk into 3940) | src/uenux2/src/api/pattern/iobservable.h |
| 3 | 3470 | 12 |  | `api::IObservable<std::vector<FormHandle<IScreen>>>::~IObservable()` (D1) | src/uenux2/src/api/pattern/iobservable.h |
| 4 | 3669 | 12 |  | `api::IObservable<std::vector<FormHandle<IPaper>>>::~IObservable()` (D1) | src/uenux2/src/api/pattern/iobservable.h |
| 5 | 4910 | 127 |  | `api::CUePrinterError::~CUePrinterError()` (D0; also CUeCodedPrinterError) | src/uenux2/src/api/hwil/iimpressora.u32.cpp |
| 6 | 5019 | 10 |  | `IObservable<vector<FormHandle<IScreenMT>>>::~IObservable()` (D0) | src/uenux2/src/api/pattern/iobservable.h |
| 7 | 5052 | 10 |  | `IObservable<vector<FormHandle<IScreen>>>::~IObservable()` (D0) | src/uenux2/src/api/pattern/iobservable.h |
| 8 | 5411 | 176 |  | `api::CTextFieldUpdateMT::~CTextFieldUpdateMT()` (D1) | src/uenux2/src/api/gui/ctextfieldupdatemt.cpp |
| 9 | 5441 | 184 |  | `api::CRHVoiceTextToSpeech::~CRHVoiceTextToSpeech()` (D1) | src/uenux2/src/api/audio/crhvoicetexttospeech.cpp |
| 10 | 5445 | 144 |  | `api::CTimer::~CTimer()` (D1, Stop() inlined) | src/uenux2/src/api/util/ctimerscheduler.cpp |
| 11 | 5511 | 10 |  | `IObservable<vector<FormHandle<IPaper>>>::~IObservable()` (D0) | src/uenux2/src/api/pattern/iobservable.h |
| 12 | 6312 | 124 |  | `api::IInputField<MEDIA>::SetLength(size_t)` (one body for IScreen slot 12 and IScreenMT slot 10) | src/uenux2/src/api/gui/iinputfield.u32.h |
| 13 | 7666 | 12 |  | `api::CFixedText::~CFixedText()` (D0, thunk into 1969) | src/uenux2/src/api/gui/cfixedtext.h |
| 14 | 7706 | 12 |  | `api::CFixedText::~CFixedText()` (D1, thunk into 1727) | src/uenux2/src/api/gui/cfixedtext.h |
| 15 | 8351 | 124 |  | `api::CUePrinterError::~CUePrinterError()` (D1; also CUeCodedPrinterError) | src/uenux2/src/api/hwil/iimpressora.u32.cpp |
| 16 | 9509 | 9 |  | `api::CEsperaAudio::Cancela()` | src/uenux2/src/api/audio/cesperaaudio.h |
| 17 | 10362 | 12 |  | `CDataTextFmt<(anon)::CComparecimentoMesariosDS>::~CDataTextFmt()` (D0, vtable @1594340) | src/uenux2/src/api/gui/cdatatext.h |
| 18 | 10363 | 12 |  | `CDataTextFmt<(anon)::CComparecimentoMesariosDS>::~CDataTextFmt()` (D1, vtable @1594340) | src/uenux2/src/api/gui/cdatatext.h |
| 19 | 10369 | 12 |  | `CDataTextFmt<(anon)::CComparecimentoMesariosDS>::~CDataTextFmt()` (D0, vtable @1594188) | src/uenux2/src/api/gui/cdatatext.h |
| 20 | 10370 | 12 |  | `CDataTextFmt<(anon)::CComparecimentoMesariosDS>::~CDataTextFmt()` (D1, vtable @1594188) | src/uenux2/src/api/gui/cdatatext.h |
| 21 | 10374 | 12 |  | `CDataTextFmt<(anon)::CComparecimentoMesariosDS>::~CDataTextFmt()` (D0, vtable @1594036) | src/uenux2/src/api/gui/cdatatext.h |
| 22 | 10377 | 12 |  | `CDataTextFmt<(anon)::CComparecimentoMesariosDS>::~CDataTextFmt()` (D1, vtable @1594036) | src/uenux2/src/api/gui/cdatatext.h |
| 23 | 10534 | 54 |  | `api::CDataText<CTextSource>::GetText() const` | src/uenux2/src/api/gui/cdatatext.h |
| 24 | 10535 | 69 |  | `api::CDataText<CTextSource>::~CDataText()` (D0) | src/uenux2/src/api/gui/cdatatext.h |
| 25 | 10536 | 66 |  | `api::CDataText<CTextSource>::~CDataText()` (D1) | src/uenux2/src/api/gui/cdatatext.h |
| 26 | 10545 | 20 |  | `api::CTextFieldUpdateMT::Stop()` | src/uenux2/src/api/gui/ctextfieldupdatemt.cpp |
| 27 | 10546 | 20 |  | `api::CTextFieldUpdateMT::Start()` | src/uenux2/src/api/gui/ctextfieldupdatemt.cpp |
| 28 | 10548 | 503 |  | `api::CTextFieldUpdateMT::Draw(IScreenMT&) const` | src/uenux2/src/api/gui/ctextfieldupdatemt.cpp |
| 29 | 10549 | 13 |  | `api::CTextFieldUpdateMT::~CTextFieldUpdateMT()` (D0) | src/uenux2/src/api/gui/ctextfieldupdatemt.cpp |
| 30 | 10685 | 473 |  | `api::CDataText<vota::CEscolheOpcao::COpcaoDS>::GetText() const` (COpcaoDS::operator() inlined: `"{}-{}"`) | src/uenux2/src/api/gui/cdatatext.h |
| 31 | 10686 | 12 |  | `CDataText<COpcaoDS>::~CDataText()` (D0, thunk into 6052) | src/uenux2/src/api/gui/cdatatext.h |
| 32 | 10688 | 12 |  | `CDataText<COpcaoDS>::~CDataText()` (D1, thunk into 6053) | src/uenux2/src/api/gui/cdatatext.h |
| 33 | 10726 | 643 |  | `api::CInputFieldMT::Draw(IScreenMT&) const` | src/uenux2/src/api/gui/cinputfieldmt.cpp |
| 34 | 10746 | 216 |  | `api::CDataTextFmt<CTextSource>::GetText() const` (snprintf 512) | src/uenux2/src/api/gui/cdatatext.h |
| 35 | 10747 | 94 |  | `CDataTextFmt<CTextSource>::~CDataTextFmt()` (D0) | src/uenux2/src/api/gui/cdatatext.h |
| 36 | 10748 | 91 |  | `CDataTextFmt<CTextSource>::~CDataTextFmt()` (D1) | src/uenux2/src/api/gui/cdatatext.h |
| 37 | 10820 | 830 |  | `api::AudioCollector::play_speech(const short*, size_t)` | src/uenux2/src/api/audio/crhvoicetexttospeech.cpp |
| 38 | 10821 | 5 |  | `api::AudioCollector::get_audio_buffer_size() const` (100) | src/uenux2/src/api/audio/crhvoicetexttospeech.cpp |
| 39 | 10822 | 47 |  | `api::AudioCollector::~AudioCollector()` (D0) | src/uenux2/src/api/audio/crhvoicetexttospeech.cpp |
| 40 | 10838 | 10 |  | `api::CRHVoiceTextToSpeech::~CRHVoiceTextToSpeech()` (D0) | src/uenux2/src/api/audio/crhvoicetexttospeech.cpp |
| 41 | 10840 | 44 |  | `api::AudioCollector::~AudioCollector()` (D1) | src/uenux2/src/api/audio/crhvoicetexttospeech.cpp |
| 42 | 10841 | 75081 |  | `api::CRHVoiceTextToSpeech::Sintetiza(const std::string&, const SParametrosFala&)` (75 KB, RHVoice inlined) | src/uenux2/src/api/audio/crhvoicetexttospeech.cpp |
| 43 | 10846 | 403 |  | `api::CTimerScheduler::CreateTimer(const milliseconds&, const std::function<void()>&)` | src/uenux2/src/api/util/ctimerscheduler.cpp |
| 44 | 10847 | 45 |  | `api::CTimer::Stop()` | src/uenux2/src/api/util/ctimerscheduler.cpp |
| 45 | 10848 | 9 |  | `api::CTimer::SetInterval(int64_t)` | src/uenux2/src/api/util/ctimerscheduler.cpp |
| 46 | 10851 | 7 |  | `api::CTimer::IsRunning() const` | src/uenux2/src/api/util/ctimerscheduler.cpp |
| 47 | 10852 | 13 |  | `api::CTimer::~CTimer()` (D0) | src/uenux2/src/api/util/ctimerscheduler.cpp |
| 48 | 10853 | 2 |  | `api::CSystemDateTime::SetDataHora(time_t)` (empty) | src/uenux2/src/api/util/csystemdatetime.cpp |
| 49 | 10854 | 67 | yes | `CDefaultGenericFactory<ISemaphore, CPosixSemaphore>::Create()` | src/uenux2/src/api/pattern/igenericfactory.u32.cpp |
| 50 | 10855 | 7 | yes | `api::CSystemDateTime::GetDataHora() const` (time(nullptr)) | src/uenux2/src/api/util/csystemdatetime.cpp |
| 51 | 10864 | 12 |  | `CDefaultGenericFactory<IRWSyncCtl, CPosixRWMutex>::Create()` | src/uenux2/src/api/pattern/igenericfactory.u32.cpp |
| 52 | 10869 | 12 | yes | `CDefaultGenericFactory<ISyncCtl, CPosixMutex>::Create()` | src/uenux2/src/api/pattern/igenericfactory.u32.cpp |
| 53 | 10878 | 91 |  | `CDefaultGenericFactory<IThreadImpl, simulador::CWasmThread>::Create()` | src/uenux2/src/api/pattern/igenericfactory.u32.cpp |
| 54 | 10887 | 12 |  | `api::CFormPart::~CFormPart()` (D0, thunk into 6026) | src/uenux2/src/api/gui/cformpart.cpp |
| 55 | 10889 | 20 |  | `api::CFormPart::Imprime() const` | src/uenux2/src/api/gui/cformpart.cpp |
| 56 | 10899 | 253 |  | `api::CMenuValidation::IsValidChar(char) const` | src/uenux2/src/api/gui/cinputmenufield.u32.cpp |
| 57 | 10959 | 34 |  | `api::CRectField::GetClassName() const` ("CRectField") | src/uenux2/src/api/gui/crectfield.cpp |
| 58 | 10960 | 27 | yes | `api::CRectField::Draw(IScreen&) const` | src/uenux2/src/api/gui/crectfield.cpp |
| 59 | 10961 | 82 |  | `api::CQRCodeImageFieldPaper::~CQRCodeImageFieldPaper()` (D0) | src/uenux2/src/api/gui/cqrcodeimagefieldpaper.cpp |
| 60 | 10962 | 79 |  | `api::CQRCodeImageFieldPaper::~CQRCodeImageFieldPaper()` (D1) | src/uenux2/src/api/gui/cqrcodeimagefieldpaper.cpp |
| 61 | 10963 | 30 |  | `api::CQRCodeImageFieldPaper::Draw(IPaper&) const` | src/uenux2/src/api/gui/cqrcodeimagefieldpaper.cpp |
| 62 | 11012 | 43 |  | `api::CNewLineFieldPaper::Draw(IPaper&) const` | src/uenux2/src/api/gui/cnewlinefieldpaper.cpp |
| 63 | 11013 | 12 |  | `api::IFormFieldBase<IPaper>::~IFormFieldBase()` (D1; also CCutFieldPaper, CNewLineFieldPaper) | src/uenux2/src/api/gui/iformfield.h |
| 64 | 11015 | 15 |  | `api::CCutFieldPaper::Draw(IPaper&) const` | src/uenux2/src/api/gui/ccutfieldpaper.cpp |
| 65 | 11047 | 28 |  | `api::CPreShowClearMT::PreShow(IScreenMT&)` | src/uenux2/src/api/gui/cpreshowclearmt.cpp |
| 66 | 11048 | 20 |  | `api::CClockFieldMT::Draw(IScreenMT&) const` | src/uenux2/src/api/gui/cclockfieldmt.cpp |
| 67 | 11049 | 25 |  | `api::CBuzzFieldMT::Draw(IScreenMT&) const` | src/uenux2/src/api/gui/cbuzzfieldmt.cpp |
| 68 | 11052 | 12 |  | `api::IFormFieldBase<IScreenMT>::~IFormFieldBase()` (D1; also CBeep/CBuzz/CClock/CLedFieldMT) | src/uenux2/src/api/gui/iformfield.h |
| 69 | 11053 | 20 |  | `api::CBeepFieldMT::Draw(IScreenMT&) const` | src/uenux2/src/api/gui/cbeepfieldmt.cpp |
| 70 | 11066 | 34 |  | `api::CLineField::GetClassName() const` ("CLineField") | src/uenux2/src/api/gui/clinefield.cpp |
| 71 | 11067 | 86 |  | `api::CLineField::Rect() const` | src/uenux2/src/api/gui/clinefield.cpp |
| 72 | 11068 | 32 |  | `api::CLineField::Draw(IScreen&) const` | src/uenux2/src/api/gui/clinefield.cpp |
| 73 | 11107 | 432 |  | `CDataText<CFormBuilder::AddStatusHeader(unsigned)::$_0>::GetText() const` (battery "{: >3}%") | src/uenux2/src/api/gui/cdatatext.h |
| 74 | 11139 | 17 |  | `api::CPreShowClearScreen::PreShow(IScreen&)` | src/uenux2/src/api/gui/cpreshowclearscreen.cpp |
| 75 | 11141 | 34 |  | `api::CFillField::GetClassName() const` ("CFillField") | src/uenux2/src/api/gui/cfillfield.cpp |
| 76 | 11142 | 25 |  | `api::CFillField::Draw(IScreen&) const` | src/uenux2/src/api/gui/cfillfield.cpp |
| 77 | 11153 | 9 |  | `api::CInputFieldControl<IScreen>::Rect() const` (empty SRect) | src/uenux2/src/api/gui/cinputfieldcontrol.h |
| 78 | 11155 | 21 | yes | `api::CInputFieldControl<IScreen>::GetClassName() const` ("CInputFieldControl") | src/uenux2/src/api/gui/cinputfieldcontrol.h |
| 79 | 11650 | 105 |  | `api::IObservable<std::shared_ptr<IImage>>::~IObservable()` (D0) | src/uenux2/src/api/pattern/iobservable.h |
| 80 | 11651 | 102 |  | `api::IObservable<std::shared_ptr<IImage>>::~IObservable()` (D1) | src/uenux2/src/api/pattern/iobservable.h |
| 81 | 12008 | 7 |  | `api::CSubReport::EhRelatorio() const` (bool +36) | src/uenux2/src/api/gui/reports/csubreport.cpp |
| 82 | 12009 | 20 |  | `api::CSubReport::ImprimeCabecalho() const` | src/uenux2/src/api/gui/reports/csubreport.cpp |
| 83 | 12010 | 10 |  | `api::CSubReport::TemCabecalho() const` | src/uenux2/src/api/gui/reports/csubreport.cpp |
| 84 | 12011 | 23 |  | `api::CSubReport::TemNome() const` | src/uenux2/src/api/gui/reports/csubreport.cpp |
| 85 | 12012 | 49 |  | `api::CSubReport::GetVia() const` | src/uenux2/src/api/gui/reports/csubreport.cpp |
| 86 | 12013 | 119 |  | `api::CSubReport::~CSubReport()` (D0) | src/uenux2/src/api/gui/reports/csubreport.cpp |
| 87 | 12014 | 12 |  | `api::CSubReport::~CSubReport()` (D1, thunk into 3904) | src/uenux2/src/api/gui/reports/csubreport.cpp |
| 88 | 12287 | 39 |  | `CDataTextFmt<std::function<std::string(const std::string&)>>::GetText() const` | src/uenux2/src/api/gui/cdatatext.h |
| 89 | 12290 | 91 |  | `CDataTextFmt<std::function<...>>::~CDataTextFmt()` (D0) | src/uenux2/src/api/gui/cdatatext.h |
| 90 | 12291 | 88 |  | `CDataTextFmt<std::function<...>>::~CDataTextFmt()` (D1) | src/uenux2/src/api/gui/cdatatext.h |
| 91 | 12301 | 32 |  | `CDataText<std::function<std::string()>>::GetText() const` | src/uenux2/src/api/gui/cdatatext.h |
| 92 | 12317 | 12 |  | `CDataTextFmt<std::string(*)(const std::string&)>::~CDataTextFmt()` (D0) | src/uenux2/src/api/gui/cdatatext.h |
| 93 | 12323 | 12 |  | `CDataTextFmt<std::string(*)(const std::string&)>::~CDataTextFmt()` (D1) | src/uenux2/src/api/gui/cdatatext.h |
| 94 | 12473 | 58 |  | `api::IInputValidation::IsValidChar(char) const` | src/uenux2/src/api/gui/iinputvalidation.h |
| 95 | 12502 | 12 |  | `api::IInputValidation::~IInputValidation()` (D1, all 5 validations) | src/uenux2/src/api/gui/iinputvalidation.h |
| 96 | 12520 | 16 |  | `CDataImage<bind<CriaTelaVisualizacaoCandidato::$_0&, const CDadosCandidato&>>::GetImage() const` | src/uenux2/src/api/gui/cdataimage.h |
| 97 | 12522 | 104 |  | `CDataImage<bind<...>>::~CDataImage()` (D0) | src/uenux2/src/api/gui/cdataimage.h |
| 98 | 12524 | 101 |  | `CDataImage<bind<...>>::~CDataImage()` (D1) | src/uenux2/src/api/gui/cdataimage.h |
| 99 | 12564 | 12 |  | `CDataTextFmt<const std::string(*)(const std::string&)>::~CDataTextFmt()` (D0) | src/uenux2/src/api/gui/cdatatext.h |
| 100 | 12567 | 12 |  | `CDataTextFmt<const std::string(*)(const std::string&)>::~CDataTextFmt()` (D1) | src/uenux2/src/api/gui/cdatatext.h |
| 101 | 12580 | 57 | yes | `CDataText<const std::string& (*)()>::GetText() const` | src/uenux2/src/api/gui/cdatatext.h |
| 102 | 12595 | 12 |  | `CMaskedTextField<CFramedText>::~CMaskedTextField()` (D0, thunk into 6062) | src/uenux2/src/api/gui/cmaskedtextfield.h |
| 103 | 12604 | 12 |  | `CMaskedTextField<CFramedText>::~CMaskedTextField()` (D1, thunk into 6063) | src/uenux2/src/api/gui/cmaskedtextfield.h |
| 104 | 12649 | 12 | yes | `CDataText<comum::CCandidaturasDSNome>::GetText() const` | src/uenux2/src/api/gui/cdatatext.h |
| 105 | 12658 | 12 |  | `api::IFormFieldBase<IScreen>::~IFormFieldBase()` (D1; also CRect/CLine/CFillField) | src/uenux2/src/api/gui/iformfield.h |
| 106 | 12713 | 12 |  | `CMaskedTextField<CGrayedFramedText>::~CMaskedTextField()` (D0) | src/uenux2/src/api/gui/cmaskedtextfield.h |
| 107 | 12719 | 12 |  | `CMaskedTextField<CGrayedFramedText>::~CMaskedTextField()` (D1) | src/uenux2/src/api/gui/cmaskedtextfield.h |
| 108 | 12784 | 12 | yes | `CDataText<std::string (*)()>::GetText() const` | src/uenux2/src/api/gui/cdatatext.h |

Related functions reconstructed or documented here but listed in other units: 10850 `CTimer::Start`, 10542 (the
`CTextFieldUpdateMT` timer lambda), 5442 `CRHVoiceTextToSpeech::SelecionaPerfil`, 10842 its constructor, 10888
`~CFormPart` (u24), 12593/12708 `CMaskedTextField::Draw` (u15), 11051/11014 default `GetClassName`s, 6404
(function-pointer `CDataTextFmt::GetText`), 5387/10368 (`CComparecimentoMesariosDS` `GetText`), 12641 / 6330
(`CDataImage` instances), 4022 (`IsValid`), 10900 (`CMenuValidation::IsValid`), 3660 (`CSubReport` constructor),
3875 (`CLp::Imprime`).

---

## 15. Open questions

* Official names of the `ISubReport` slots and of the `CSubReport` bool (+36) and header part (+28).
* The meaning of the two `CBuzzFieldMT` numbers ((51, 10) and (52, 5)) and of `IScreenMT` slot 13 (called after
  `Clear()` by `CPreShowClearMT`, no-op in the mock).
* The exact roles of `CTimer`'s two mutexes (+92, +120) and the worker loop (absent from the binary).
* Whether `CSystemDateTime::SetDataHora` is empty on the urna too, or only in the Emscripten build.
* `AudioCollector`'s int at +4 (always 0) and the order of the slots inside two groups of `RHVoice::client`
  (from RHVoice 1.14's `client.hpp`). The wasm signatures separate slot 4 ((this)->i32, returns 0),
  slots 5–7 ((this, x)->i32, ICF 371), slots 8–11 ((this, x, y)->i32, func 1908) and slot 12 ((this)->void);
  only the order inside {5,6,7} and {8..11} is assumed.
* Whether `CDataTextFmt<SRC>` uses overloads/specialisation or `if constexpr` for its three behaviours.
