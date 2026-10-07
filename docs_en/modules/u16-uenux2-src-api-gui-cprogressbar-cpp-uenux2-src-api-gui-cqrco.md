# u16: `uenux2/src/api/gui` — progress bars, QR code images and text fields

Unit u16 covers 85 wasm functions from 14 original files of the urna's GUI library (`api::`, directory
`uenux2/src/api/gui`). They are all *form fields* (the widgets a screen is made of) or helpers of them:

| original file | class | what it is |
|---|---|---|
| `cprogressbar.cpp` | `api::CProgressBar` | horizontal progress bar ("Gravando" after each voter; "Preparando dados para encerramento") |
| `cstepsprogressbar.cpp` | `api::CStepsProgressBar` | the arrow-shaped "steps" bar at the top of every voting screen (one step per cargo) |
| `cqrcodeguard.cpp` | `api::CQRCodeGuard` | RAII owner of a libqrencode `QRcode*` |
| `cqrcodeimage.cpp` | `api::CQRCodeImage` | QR code as a screen image (8-bit grey BMP) — BU/certificate QR on the urna display |
| `cqrcodeimagepaper.cpp` | `api::CQRCodeImagePaper` | QR code as a 1-bit printer raster — QR codes printed on the BU |
| `ctextfield.cpp` | `api::CTextField` | one line of text |
| `ctextfieldblinking.cpp` | `api::CTextFieldBlinking` | text alternating between two colours every 500 ms ("VOTO NULO", "VOTO EM BRANCO"…) |
| `ctextfieldupdate.cpp` | `api::CTextFieldUpdate` | text re-read periodically, redrawn when it changes (status header, clock) |
| `ctextfielddoubleline.cpp` | `api::CTextFieldDoubleLine` | text word-wrapped onto two lines (candidate / party / cargo names) |
| `ctextfieldmultiline.cpp` | `api::CTextFieldMultiLine` | paragraph wrapped inside a rectangle |
| `ctextrectfield.cpp` | `api::CTextRectField` | text on a coloured band |
| `ctextbox.cpp` | `api::CTextBox` | framed box with a label and three looks (keyboard test) |
| `ctextfieldmt.cpp` | `api::CTextFieldMT` | text on the mesário's micro-terminal (4 x 40 characters) |
| `ctextfieldpaper.cpp` | `api::CTextFieldPaper` | one line of a printed report (BU, zerésima…) |

19 of the 85 functions ran during the recorded votes (`analysis/runtime/*.functions.tsv`): the steps bar
(5504-5507, 10972), the progress bar (5545, 10977), the blinking/update/double-line/plain text fields and
their `Rect`/`Start` methods. A headless run (`node tools/run/headless.mjs --scenario municipal-t1 --draw`)
reproduces the steps bar geometry derived below exactly (see §5.2).

**Reconstructed sources** (all under `src/uenux2/src/api/gui/`):

* own files: `cprogressbar.{h,cpp}`, `cstepsprogressbar.{h,cpp}`, `cqrcodeguard.{h,cpp}`, `cqrcodeimage.{h,cpp}`,
  `cqrcodeimagepaper.{h,cpp}`, `ctextfield.{h,cpp}` (its header also holds the recap of the form-field and
  screen interfaces), `ctextfieldblinking.{h,cpp}`, `ctextfieldupdate.{h,cpp}`, `ctextfielddoubleline.{h,cpp}`,
  `ctextfieldmultiline.{h,cpp}`, `ctextrectfield.{h,cpp}`, `ctextbox.{h,cpp}`, `ctextfieldmt.{h,cpp}`,
  `ctextfieldpaper.{h,cpp}`
* fragments (functions of this unit whose original file belongs to another unit):
  `cformbuilder.u16.cpp` (three `CFormBuilder::Add*` helpers that inline constructors of this unit),
  `primitives.u16.cpp` (`SRect::Adjusted`)
* `cprogressbar.cpp` also contains `SetValor` (func 3667, unit u37) and `Incrementa` (func 5508, unit u07,
  first written in `cprogressbar.u07.cpp`); `cstepsprogressbar.cpp` also contains the second layout strategy
  (func 10971, unit u34) because they are part of the same file.

---

## 1. Where this code sits in the voting process

Glossary: *tela* = screen; *campo* = field/widget; *barra de progresso* = progress bar; *eleitor* = voter;
*mesário* = poll worker; *MT* (*microterminal*) = the small character display + keypad of the *terminal do
mesário*; *cargo* = office being voted; *BU* (*Boletim de Urna*) = the poll-station result, printed on paper
(with QR codes, *BU digital*) and written as an ASN.1 file; *via* = printed copy; *encerramento* = closing of
the voting day; *zerésima* = opening report.

Every screen of the urna application is an `api::CFormBuilder` (a `vector<shared_ptr<IFormField<MEDIA>>>`)
turned into an `IForm`/`CInteractiveForm` (units u02/u07/u15). The fields of this unit are the leaves of those
forms. Where the voter meets them:

```
 voter screen, every cargo       CPreShowProgressBar::PreShow (u04) -> CStepsProgressBar  [Vereador > Prefeito]
                                 CTextFieldDoubleLine: cargo / candidate / party names
                                 CTextFieldBlinking: "VOTO NULO", "VOTO EM BRANCO", "VOTO DE LEGENDA"
                                 CTextFieldUpdate: status header (clock...)
 after the last CONFIRMA         CSincronismoEleitor "Gravando" -> CProgressBar 0..4 (one step per sync phase)
 encerramento (closing)          CProgressoEncerramento -> CProgressBar 0..30, text "%p" ("37%")
                                 CGeraBU -> CTextFieldPaper lines + CQRCodeImagePaper QR rasters (printed BU)
                                 CMostraQRCodeBU / CMostraQRCodeCertificado -> CQRCodeImage 380 x 380 on screen
 urna-state screen               func 3059 ("RESUMO DA CORRESPONDÊNCIA: ...") -> CQRCodeImage 148 x 148 fed by
                                 comum::CQRCodeDS, in a CImageFieldUpdate refreshed every 15 s
 mesário terminal                CTextFieldMT
 start-of-day keyboard test      CTextBox per key, CTextFieldUpdate (200 ms)
```

## 2. Classes (RTTI) and layouts

All widgets derive (single inheritance, RTTI kind `si`) from `api::IFormField<MEDIA>` →
`api::IFormFieldBase<MEDIA>`, where MEDIA is the output device:

```
IFormFieldBase<IScreen> (vtable @1537360) <- IFormField<IScreen> <- CTextField, CTextFieldBlinking,
      CTextFieldUpdate, CTextFieldDoubleLine, CTextFieldMultiLine, CTextRectField, CTextBox,
      CProgressBar, CStepsProgressBar                                  (voter display, 640 x 480 logical)
IFormFieldBase<IScreenMT> (@1579652) <- IFormField<IScreenMT> <- CTextFieldMT          (mesário micro-terminal)
IFormFieldBase<IPaper>    (@1580868) <- IFormField<IPaper>    <- CTextFieldPaper       (thermal printer)
IImage <- CQRCodeImage                     (image source used by CImageField / CImageFieldUpdate)
CQRCodeGuard, CQRCodeImagePaper            (no RTTI)
```

Common base layout (IFormFieldBase): `+0 vptr, +4 bool m_precisaRedesenhar, +8 IForm* m_pForm, +12 std::string m_nome`.
Member layouts (derived from constructors, all offsets checked in the WAT):

| class | size | members after +24 |
|---|---|---|
| CTextField | 64 | +24 SPoint pos, +28 shared_ptr<IText>, +36 SFont, +44 TColor texto, +48 TColor fundo, +52 SRect anterior, +60 bool apagar |
| CTextFieldBlinking | 68 | +24 pos, +28 texto, +36 fonte, +44 cor1, +48 cor2, +52 fundo, +56 shared_ptr<ITimer>, +64 bool fase |
| CTextFieldUpdate | 80 | +24 pos, +28 texto, +36 fonte, +44 cor, +48 fundo, +52 string último texto, +64 SRect anterior, +72 shared_ptr<ITimer> |
| CTextFieldDoubleLine | 60 | +24 SPoint linha1, +28 SPoint linha2, +32 TPosition maxX, +36 texto, +44 fonte, +52 cor (2), +56 fundo |
| CTextFieldMultiLine | 72 | +24 SRect área, +32 SRect desenhado, +40 bool, +44 texto, +52 fonte, +60 cor (2), +64 fundo (1), +68 altura da linha |
| CTextRectField | 68 | +24 SRect, +32 cor da faixa, +36 texto, +44 fonte, +52 cor do texto, +56 SRect anterior, +64 bool fundo opaco |
| CTextBox | 68 | +24 CFixedText (embedded), +44 ETextStatus, +48 pos, +52 TFontSize, +56/+60 TColor, +64 altura, +66 largura |
| CTextFieldMT | 48 | +24 pos, +28 texto, +36 texto anterior, +44 pos anterior |
| CTextFieldPaper | 36 | +24 texto, +32 IPaper::EStyle |
| CProgressBar | 88 | +24 valor, +28 mínimo, +32 máximo, +36 SRect, +44 SRect interno, +52..+64 4 TColor, +68 string formato, +80 TFontSize, +82 uebyte margem, +84 TColor borda |
| CStepsProgressBar | 100 | +24 atual, +28 vector<string> rótulos, +40 SRect, +48..+72 7 TColor, +76 bool recalcular, +80 SFont, +88 vector<SSegmento> |
| CQRCodeImage | 32 | +4 TPosition tamanho, +8 std::function<std::string()> |

Types: `TPosition` = int16; `SPoint{x,y}` (4 bytes); `SRect{left,top,right,bottom}` (8 bytes, built from two
points with min/max normalisation); `SFont{TFontSize size; style}` (8 bytes, estilo 1 = bold: the web
glue `js_measure_text_width(text, size, bold, italic)`); `TColor` = index into a 38-entry palette (the web
screen maps it to CSS colours @1529596: 0 transparent, 1 `#ffffff`, 2 `#000000`, 3 `#808080`, 4 `#d3d3d3`,
13 `#006400`, 20 `#ffd300`, 36 `#c9c9c9`…).

## 3. The form-field protocol

`IFormFieldBase` vtable (names inferred unless attested; see `ctextfield.h`):

| slot | method | default body |
|---|---|---|
| 0 / 1 | destructor / deleting destructor | |
| 2 | `void Draw(MEDIA&) const` | pure |
| 3 / 4 | `Start()` / `Stop()` | no-op (func 218); the two timer fields start/stop their `ITimer` (ITimer slots 2/3) |
| 5 | `bool RedrawIfDirty(MEDIA&)` | func 4118: `a = m_precisaRedesenhar; m_precisaRedesenhar = false; if (a) Draw(m); return a;` |
| 6 | `SetForm(IForm*)` | func 3037: `m_pForm = f` |
| 7 | `std::string GetClassName() const` | pure in `IFormFieldBase`; each IScreen field returns its literal name ("CTextField"), used by `CFormBuilder::Add` (func 426) to name fields "CTextField1", "CTextField2"… For IScreenMT and IPaper, `IFormField<MEDIA>` itself implements it and returns its own name ("IFormField<IScreenMT>", func 11051; "IFormField<IPaper>", func 11014); `CTextFieldMT` and `CTextFieldPaper` do **not** override it (one table slot, 3273 / 3367, shared by all MT / paper fields) |
| 8 | `SRect Rect() const` (attested, IScreen only) | |
| 9 | `void Move(const SPoint&)` (attested, IScreen only) | several classes share ICF bodies 2241/2783/5527 |

A field never redraws itself: when its state changes it runs an always-inlined helper (`Invalidate`, the
name used by unit u15 in `gui-common.u15.h`) — "if my form is visible (form +4, read under the form mutex
+28): set `m_precisaRedesenhar` and call the form's slot 4 (redraw request)". The form then calls `RedrawIfDirty` on its fields. Timers
(`ITimerScheduler::GetInst()` slot 0 creates one; `simulador::CWasmTimerScheduler` uses
`emscripten_async_call`) drive the two animated fields:

* `CTextFieldBlinking`: every 500 ms toggle `m_fase` and notify; `Draw` paints the text in `m_fase ? cor1 : cor2`.
* `CTextFieldUpdate`: every `periodo` compare `m_texto->GetText()` with the last painted text; notify if
  different; `Draw` erases the previous rectangle and repaints.

The timer lambdas' `operator()` are funcs 10937 and 10909 (component rt:libcxx, no unit); destroying a field
destroys its `CWasmTimer`, whose destructor bumps a generation counter so a pending async callback becomes a
no-op (func 3347) — no dangling `this`.

## 4. The text fields

* **CTextField** (1918/10952/10951): `Draw` = (erase the previous rectangle if the flag at +60 is set — no
  writer of that flag was found, so this branch looks dead) + `IScreen::WriteText(pos, texto, fonte, cor,
  fundo)` (slot 19), remembering the returned rectangle. `Rect()` measures the text with
  `IScreen::GetFontMetrics` and shifts it by the text alignment (Right: x − w, Center: x − w/2). The three
  `Rect()` methods of CTextField/CTextFieldBlinking/CTextFieldUpdate are 14-byte thunks into one
  merge-similar body (func 3889) whose only parameter is the srcloc (:55, :70, :73).
* **CTextFieldDoubleLine::GetTextLines** (5499, :68) — word wrap on two lines, widths measured with the real
  font (`IScreen` slot 32, `js_measure_text_width` in the web build):
  1. if the whole text fits in `maxX − linha1.x + 1` → `{texto, ""}`;
  2. otherwise move the last word(s) to line 2 (`rfind(' ')`), one word at a time, until line 1 fits;
  3. if line 1 is a single word that is still too wide, move it character by character (a blank is inserted
     once between the broken word and the words already moved); a text without any blank is broken the same way;
  4. line 2 is then truncated character by character until it fits `maxX − linha2.x + 1` — no ellipsis.
  `Rect()` (10929; the tools called it `CalcLineRect` after the srcloc of the inlined helper :133) is the union
  of the two line rectangles.
* **CTextFieldMultiLine::GetTextLines** (5497, :116): `std::istringstream` + `std::getline` per paragraph; empty
  paragraphs are kept; a paragraph that does not fit is cut after the last blank whose prefix fits, else inside
  the word after the longest fitting prefix; the blank at the cut is dropped. `Draw` (10925) erases the union
  of everything painted before, then writes line by line with a line height of `size + ceil(size/6)`,
  aligning each line by the text alignment; lines below the rectangle are not drawn.
* **CTextRectField**: `IScreen::WriteText(SRect, …)` (slot 20) inside the band, background `m_corFundo`.
* **CTextBox**: `Rect()` = explicit size, or text size + 20 x 8 pixels, shifted by the alignment; `Draw`
  clears the box, then status 0 = outline (`DrawRect`, 1 px) + black text, 1 = filled black + white text,
  2 = filled `#d3d3d3` + white text, other = `CBaseError<EUeGuiError>(4962, "Status indefinido")`.
* **CTextFieldMT**: the micro-terminal has no erase primitive; when the new text is shorter than the one drawn
  before, the old one is overwritten with blanks first (`IScreenMT` slot 3, 40-column line buffer of
  `simulador::CWasmScreenMT`). It has no `GetClassName` of its own (slot 7 = inherited
  `IFormField<IScreenMT>::GetClassName`, "IFormField<IScreenMT>"); the same holds for `CTextFieldPaper`
  ("IFormField<IPaper>").
* **CTextFieldPaper**: `IPaper::Print(texto, estilo)` (slot 2) — in the web build `CWasmNullPaper` slot 2 is an
  empty function (func 1528): nothing is printed.

## 5. The progress bars

### 5.1 CProgressBar

Built only through `CFormBuilder::AddProgressBar(maximo, a, b, formato)` (func 5545, constructor inlined,
constants: mínimo 0, valor 0, colours bar 13 `#006400` / background 1 / text over bar 1 / text 2 / border 2,
margem 2, font size 0 = automatic). Constructor checks, in order (EUeGuiError, `std::format` messages):
`mínimo >= máximo` → 4944 "Limite inferior ({}) >= limite superior ({})" (:52); width ≤ 2·margem → 4945
"Largura da barra ({}) não comporta a margem ({})" (:58); height ≤ 2·margem → 4946 "Altura da barra ({})…"
(:67). The automatic font size is `floor((height − 2·margem) · 0.8)`.

`Draw` (10977): text = `formato` with `%p` → `"{}%"` of `(valor−min)·100/(max−min)` and `%v` → valor
(func 2677, replace-all); `ClearRect(area, fundo)`; fill `larguraInterna · (valor−min) / (max−min)` pixels of
the inner rectangle `area.Adjusted(1,1,−3,−3)`; centred text: white where it lies over the bar, black
elsewhere, using the clipped `WriteText` (slot 21) twice when the end of the bar cuts the text; finally the
border `DrawRect(area.Adjusted(m/2, m/2, −m/2, −m/2), borda, m)`. `SetValor` clamps to [min, max]; `Incrementa`
adds one step. In the voter flow the bar has 4 steps and no text ("Gravando"); at the encerramento 30 steps
and `%p`.

### 5.2 CStepsProgressBar

`vota::CPreShowProgressBar::PreShow` (13550, u04) builds, before each voting screen, the list of labels (cargo
name, plus " - " + ordinal for multi-choice cargos such as the two Senate seats), finds the index of the
current step and, when there are at least two steps, constructs the bar on the stack over
`{2, 2}-{W−3, 30}` (W = screen width), calls `Draw` and destroys it. Constructor check: no labels → 5014
"Quantidade de segmentos deve ser maior que zero" (:428).

`Draw` (5504), first call — layout (`CalculaSegmentos`, inlined):

1. start font = `min(max(height·3/5, 10), height)`; two strategies are tried from that size down to 10
   (`TentaLayout`, 5507): **equal widths** (`CabeLarguraIgual`, 10972, table slot 3442; rectangles from
   `DivideArea`, 5506) and **proportional widths** (`CabeLarguraProporcional`, 10971, slot 3443). A label
   "fits" when `altura − size/5 ≤ height − 2·margem` and `largura − size/3 ≤ segment width − 2·margem −
   10 (notch, not for the first) − size·10/height (arrow, not for the last)`, with `margem = max(size/4, 1)`;
2. the strategy with the larger font wins (ties → equal widths); if neither fits at size 10, equal widths are
   used **and all labels are dropped**;
3. segments overlap by 10 − 4 = 6 px; each becomes a Qt-style path (`SPathElement`, 56 bytes: type 0 moveTo /
   1 lineTo / 2 arcTo(x, y, w, h, start°, sweep°) / 3 close; radius `min(height/2, 10)`): single step = rounded
   rectangle; first = rounded left + arrow tip; middle = notch + tip; last = notch + rounded right;
4. label position = `{left + margem (+10 after the first), top + margem + max(0, vertical slack/2)}`, bold.

Every call then paints each segment: outline (`DrawPath`, colour 0 transparent, 1 px), fill (steps done
`#808080`, current black, next `#c9c9c9`) and label (black, white, grey). Headless check on the municipal
scenario (canvas scale 2 x 1.667): segment 1 `moveTo(24,3.33) ellipse(24,20,…) lineTo(4,35) ellipse(24,35,…)
lineTo(626,51.67) lineTo(646,27.5) lineTo(626,3.33) closePath` = logical `{2,2}-{322,30}` rounded-left + tip,
segment 2 starting at x = 317 (6 px overlap) with notch and rounded right — exactly the paths above.

## 6. QR codes

`CQRCodeGuard(const std::string&)` (5502, :23) is libqrencode's `QRcode_encodeString(texto, 0, QR_ECLEVEL_L,
QR_MODE_8, 1)` with most of the library inlined (13.8 KB): version 0 = smallest that fits, error correction
**L**, 8-bit mode hint, case-sensitive. NULL → 4947 "Não foi possível criar o QR code". The destructor
(`QRcode_free`) is inlined into both users.

| | `CQRCodeImagePaper::MontaImagem(400, texto)` (2772) — printer | `CQRCodeImage::GetImage()` (10967) — screen |
|---|---|---|
| input check | empty → 4949 "Criação de QRCode com texto vazio." (:35) | `std::function` empty → `std::bad_function_call` |
| size check | `width + 4 > 400` → 4951 "A largura do qrcode ({}) superou o espaço disponível ({})" (:49) | `tamanho < width + 4` → 4948, same message (:36) |
| border | 2 white modules on each side | 2 white modules, plus the unused pixels (`tamanho mod (width+4)`) on the left and at the bottom |
| encoding | 1 bit per module, LSB first, row-major, 1 = dark (`CStringUtils::SetBit`, cstringutils.cpp:83, 7027 "Bit fora dos limites…") | 1 byte per pixel (0 black / 255 white), `escala = tamanho/(width+4)` pixels per module, rows bottom-up, 4-byte aligned |
| result | `QrcodeData{bitmap, largura = width+4, escala = min(400/largura, 4)}` | complete BMP: 1078-byte header (`BM`, BITMAPINFOHEADER 40, 8 bpp, 20000 px/m, 256-grey palette) + pixels |

## 7. Boletim de Urna: what this unit contributes

The BU payload, the hash chain and the signature are built elsewhere (`comum::CGeradorBUQRCode`, func 5604;
`docs/bu/qrcode.md`, `docs/bu/build-a-bu.md`). This unit only renders them:

1. **Printed BU** (`vota::CGeraBU::vf2`, u08): the header/body lines are `CTextFieldPaper(CDataText<…>, estilo)`
   fields; for every "BU DIGITAL" part (`QRBU:i:n VRQR:6.0 …`; ≤ 1100 characters except the last, which can reach 1245
   with an Ed521 signature or 1259 with ECDSA, because in format 6.0 the last part's fixed text takes 422–436 characters,
   more than the 277 reserved; only the width check above limits it; 2026 urna data: `investigation/README.md`,
   finding H5) the generator calls
   `CQRCodeImagePaper::MontaImagem(400, parte)` and appends the raster with `CPaperFormBuilder::AddQRCode`
   (2775) under the label "-------------- 01 / 02 ---------------"; the "CERTIFICADO DIGITAL" QR codes
   (`QRCE:… IDUE:… MDUE:… CERT:…`, func 5634 / vota_f5591) use the same function. The printer driver writes
   the raster as `0B | u32 len(bitmap) | u32 largura | u8 escala | bitmap`; the real 2024 `-imgbu.dat` files have
   exactly `len = ceil(largura²/8)` and `escala = 4` (widest symbol 93 modules = 97 with the border → 388
   printer dots ≤ 400), level L, versions 9-19 (docs/bu/qrcode.md §2.1).
2. **BU on the screen** (`vota::CMostraQRCodeBU`, 12055, screen `telaQRCodeBU`, after the last via):
   `CQRCodeImage(380, <part i>)` inside an image field; the voter-facing text is a `CTextFieldMultiLine`
   ("O QR code ao lado contém o resultado da votação para esta urna…"); BRANCO switches to
   `CMostraQRCodeCertificado` (certificate QR, same class).
3. **Progress**: `CProgressoEncerramento` shows a `CProgressBar` of 30 steps with "%p" while
   `CGravaResultado` writes and signs the result files (BU, RDV, logs).
4. Not a BU: func 3059 (named `CImageFieldUpdate::CImageFieldUpdate` by the tools because it inlines that
   constructor; it is a screen builder with the status header and a "RESUMO DA CORRESPONDÊNCIA: …" line)
   shows the urna-state QR code (`comum::CQRCodeDS`, fields from `MontaCamposQRCodeEstadoUrna`, see u36 §3.4)
   as `CQRCodeImage(148, …)` in a `CImageFieldUpdate` redrawn every 15 000 ms.
5. The yellow bands "Número de cópias" / "acima do limite permitido" (`CTextRectField`) are the warning shown
   when more BU copies are requested than allowed (probably `CLimiteCopiasBUAtingido`'s screen).

## 8. What is specific to the web build

* `simulador::CWasmScreen` (canvas, 640 x 480 logical scaled to 1280 x 800) implements the IScreen slots used
  here with `js_fill`, `js_text`, `js_path`, `js_measure_text_width`; slot 5 (`ClearRect`) just forwards to
  slot 6 (`FillRect`).
* **Clipped text is not clipped**: `CWasmScreen::vf21` (func 9090) ignores its clip rectangle and calls
  `WriteText(pos, …)` (slot 19 → slot 20, which `js_fill`s the whole text box whenever the background colour
  is not 0). `CProgressBar::Draw` relies on clipping to paint the percentage half white / half black; in the
  simulator the first call fills the whole text box with the bar colour (#006400, even past the end of the
  bar) and the second call repaints the whole text in black.
* **Nothing is printed**: `CWasmNullPaper` slot 2 is empty, so `CTextFieldPaper::Draw` and the BU QR rasters
  go nowhere (the raster is still computed).
* Timers are `CWasmTimer` (`emscripten_async_call`), not threads; the native `api::CTimer::Start` would throw
  "thread constructor failed" in this build (it is not used).
* The micro-terminal is emulated by `CWasmScreenMT` (a 4 x 40 line buffer exported in the state JSON).

## 9. Notable wasm / Emscripten observations

* **Constructors inlined into builders**: 1265, 5545, 5546 carry the srcloc records of the constructors of
  CTextFieldBlinking, CProgressBar and CTextRectField, so the tools named them after those constructors, but
  they take the builder and return a `shared_ptr` (sret) — they are `CFormBuilder::Add*` helpers. The same
  happens with `CTextBox` (inlined into `CTesteTeclado::StartState`, 11805) and `CStepsProgressBar`
  (inlined into `CPreShowProgressBar::PreShow`, 13550); `CTextFieldDoubleLine::Rect` carries the record of
  the inlined `CalcLineRect`, `CQRCodeImage::GetImage` that of the static `MontaImagem`.
* **Constant-propagated parameters**: the colours of CTextFieldUpdate (2, 1), CTextFieldMultiLine (2, 1),
  CTextFieldDoubleLine (text 2) and the `size_t` of `CQRCodeImagePaper::MontaImagem` (400) are no longer
  parameters of the compiled functions; the `cqrcodeimagepaper.cpp:38` srcloc record is orphaned because the
  check that used it was folded away.
* **merge-similar-functions**: `Rect` of three text fields → 3889 (srcloc as parameter); destructors of
  CTextRectField/CTextFieldDoubleLine → 6021/6022 and of CQRCodeImage/`CDataText<std::function<string()>>`
  → 6059/6060 (vtable as parameter); `GetClassName` of CTextFieldBlinking → 3891 (string pieces as
  parameters; the same 18-character body also serves `IFormField<IPaper>::GetClassName`, func 11014).
* `GetClassName` bodies build the class name inline as an SSO string (`a[5]:short = 2560` = size 10 in byte 11)
  or a 16/24-byte heap string; wasm-decompile shows the source bytes as `d_operator…[N]`, which is 1024 below
  the real address (e.g. 196630 → "CTextField" at 197654).
* The QR code guard shows Emscripten's WASI errno numbering: `EINVAL` is stored as 28 in `errno` (@1931660).
* libqrencode's `QRinput` (28 bytes) is allocated with `malloc` and every field zeroed inline — this is how
  the version/level constants of `QRcode_encodeString` can be read.

## 10. Error codes (`ecourna::api::exception::CBaseError<api::EUeGuiError, SErrorLimits{4900, 5100}>`)

| code | where | message |
|---|---|---|
| 4944 / 4945 / 4946 | CProgressBar ctor :52 / :58 / :67 | limits / width / height (see §5.1) |
| 4947 | CQRCodeGuard :23 | Não foi possível criar o QR code |
| 4948 | CQRCodeImage::MontaImagem :36 | A largura do qrcode ({}) superou o espaço disponível ({}) |
| 4949 / 4951 | CQRCodeImagePaper::MontaImagem :35 / :49 | Criação de QRCode com texto vazio. / largura do qrcode… |
| 4961 / 4963 | CTextBox ctor :41 / Move :90 | A posição da caixa não pode ter x < 3 ou y < 1. |
| 4962 | CTextBox::Draw :69 | Status indefinido |
| 4964 4965 4967 4970 4972 4973 4974 4976 | CTextField, Blinking, DoubleLine, MT, MultiLine, Paper, Update, TextRect ctors | Campo estava com o texto nulo |
| 4966 | CTextFieldBlinking :42 | As cores estavam iguais |
| 4968 / 4969 | CTextFieldDoubleLine :43 / :46 | maxX menor que a posição x da linha 1 / 2 |
| 5014 | CStepsProgressBar :428 | Quantidade de segmentos deve ser maior que zero |
| 7027 (EUeUtilError) | CStringUtils::SetBit (inlined in 2772) | Bit fora dos limites ({}) do vetor ({}) |

(In `decompiled/…/ctextfieldupdate.cpp.dcmp` the code 4974 is mis-annotated with the string at address 4974.)

## 11. Suspicious or noteworthy code

1. **CTextFieldMultiLine drops the rest of the text** (5497): when the break position is 0 (a paragraph that
   starts with a blank followed by a word wider than the area — e.g. after a double space — or a first
   character wider than the area) the function pushes an empty line and returns immediately (the
   `br_table` on `quebra + 1` jumps out of both loops); all remaining lines and paragraphs are lost without
   any error. Low (fixed TSE texts), but it is silent truncation of on-screen instructions.
2. **Off-by-one in the same function**: when a line has no usable blank and every proper prefix (lengths
   1 … size−1) fits, the loop stops without a break position and the whole line — already measured too
   wide — is kept, overflowing the area by its last character.
3. **Silent truncation by design**: `CTextFieldDoubleLine` cuts the second line of a name without ellipsis;
   `CTextFieldMultiLine` drops lines below its rectangle; `CStepsProgressBar` drops *all* labels when none fits
   at font 10. A long candidate/party name can be shown cut on the confirmation screen.
4. **CProgressBar::Move** (10976) moves only the outer rectangle; the inner rectangle (bar and text) stays.
   Latent (nothing moves a progress bar).
5. **CTextBox::Move** (10957) stores the new position before validating it; after the exception the field
   keeps the invalid position.
6. **Web build: two-colour progress text collapses** (CWasmScreen slot 21 ignores the clip rectangle): the
   text box is filled with the bar colour and the whole text ends up black (see §8).
7. **QR error correction level L** for printed BU QR codes (the weakest level, ~7 % recovery) — matches the real
   2024 prints; thermal-print damage can make a part unreadable (the paper BU text remains the reference).
8. The QR code on screen is not centred when 380 is not a multiple of `width + 4` (the slack goes to the left
   and bottom); cosmetic. For the BU symbol sizes (53–93 modules, §7) the slack is 38 px (57 × 6 = 342) to
   89 px (97 × 3 = 291), so the code sits visibly right of and above the centre of its 380 px box.
9. Dead code: the erase branch of `CTextField::Draw` (flag +60 never set in the binary) and the orphaned
   check at `cqrcodeimagepaper.cpp:38`.

## 12. Complete mapping table (all 85 functions of u16)

"ran" = observed executing during the recorded votes. D1 = complete-object destructor (returns `this`),
D0 = deleting destructor.

| # | wasm func | size | ran | reconstructed symbol | src file / disposition |
|---|---|---|---|---|---|
| 1 | 1262 | 171 |  | `api::CTextFieldMT::CTextFieldMT(const SPoint&, const SharedIText&) (:25)` | src/uenux2/src/api/gui/ctextfieldmt.cpp |
| 2 | 1265 | 692 | yes | `api::CFormBuilder::AddBlinkingText(texto, pos, fonte, alinhamento) - inlines CTextFieldBlinking::CTextFieldBlinking (:39/:42)` | src/uenux2/src/api/gui/cformbuilder.u16.cpp + src/uenux2/src/api/gui/ctextfieldblinking.cpp (ctor) |
| 3 | 1918 | 201 |  | `api::CTextField::CTextField(const SPoint&, const SharedIText&, const SFont&, TColor, TColor) (:30)` | src/uenux2/src/api/gui/ctextfield.cpp |
| 4 | 2768 | 106 |  | `api::SRect::Adjusted(TPosition, TPosition, TPosition, TPosition) const` | src/uenux2/src/api/gui/primitives.u16.cpp (fragment) |
| 5 | 2771 | 154 |  | `api::CTextFieldPaper::CTextFieldPaper(const SharedIText&, IPaper::EStyle) (:23)` | src/uenux2/src/api/gui/ctextfieldpaper.cpp |
| 6 | 2772 | 1582 |  | `api::CQRCodeImagePaper::MontaImagem(size_t, const std::string&) (:35/:49)` | src/uenux2/src/api/gui/cqrcodeimagepaper.cpp |
| 7 | 3658 | 303 | yes | `api::CTextFieldUpdate::CTextFieldUpdate(pos, texto, periodo, fonte[, 2, 1]) (:38)` | src/uenux2/src/api/gui/ctextfieldupdate.cpp |
| 8 | 3659 | 306 | yes | `api::CTextFieldDoubleLine::CTextFieldDoubleLine(...) (:40/:43/:46)` | src/uenux2/src/api/gui/ctextfielddoubleline.cpp |
| 9 | 3661 | 423 |  | `std::vector<uebyte>::insert(const_iterator, size_type, const uebyte&)` | library/inlined helper: libc++ fill-insert instantiation (quiet zones of CQRCodeImage::MontaImagem); noted in src/uenux2/src/api/gui/cqrcodeimage.cpp |
| 10 | 3662 | 103 |  | `api::CQRCodeImage::CQRCodeImage(TPosition, const std::function<std::string()>&)` | src/uenux2/src/api/gui/cqrcodeimage.cpp |
| 11 | 3665 | 286 |  | `api::CStepsProgressBar::~CStepsProgressBar() (D1)` | src/uenux2/src/api/gui/cstepsprogressbar.cpp |
| 12 | 5495 | 176 |  | `api::CTextFieldUpdate::~CTextFieldUpdate() (D1)` | src/uenux2/src/api/gui/ctextfieldupdate.cpp |
| 13 | 5497 | 1944 |  | `api::CTextFieldMultiLine::GetTextLines() const (:116)` | src/uenux2/src/api/gui/ctextfieldmultiline.cpp |
| 14 | 5498 | 230 |  | `api::CTextFieldMultiLine::CTextFieldMultiLine(const SRect&, const SharedIText&, const SFont&[, 2, 1]) (:71)` | src/uenux2/src/api/gui/ctextfieldmultiline.cpp |
| 15 | 5499 | 2912 | yes | `api::CTextFieldDoubleLine::GetTextLines() const (:68)` | src/uenux2/src/api/gui/ctextfielddoubleline.cpp |
| 16 | 5502 | 13815 |  | `api::CQRCodeGuard::CQRCodeGuard(const std::string&) (:23) - inlines QRcode_encodeString(s, 0, QR_ECLEVEL_L, QR_MODE_8, 1)` | src/uenux2/src/api/gui/cqrcodeguard.cpp |
| 17 | 5504 | 11502 | yes | `api::CStepsProgressBar::Draw(IScreen&) const (inlines CalculaSegmentos)` | src/uenux2/src/api/gui/cstepsprogressbar.cpp |
| 18 | 5505 | 337 | yes | `std::vector<CStepsProgressBar::SSegmento>::__swap_out_circular_buffer` | library/inlined helper: libc++ vector growth for 28-byte SSegmento; noted in src/uenux2/src/api/gui/cstepsprogressbar.cpp |
| 19 | 5506 | 836 | yes | `api::(anonymous)::DivideArea(const SRect&, size_t, TPosition, TPosition)` | src/uenux2/src/api/gui/cstepsprogressbar.cpp |
| 20 | 5507 | 504 | yes | `api::(anonymous)::TentaLayout(IScreen&, rotulos, area, TFontSize, TFuncaoLayout, SLayout&)` | src/uenux2/src/api/gui/cstepsprogressbar.cpp |
| 21 | 5509 | 74 |  | `api::CProgressBar::~CProgressBar() (D1)` | src/uenux2/src/api/gui/cprogressbar.cpp |
| 22 | 5545 | 2040 | yes | `api::CFormBuilder::AddProgressBar(uedword maximo, const SPoint&, const SPoint&, const std::string& formato) - inlines CProgressBar::CProgressBar (:52/:58/:67)` | src/uenux2/src/api/gui/cformbuilder.u16.cpp + src/uenux2/src/api/gui/cprogressbar.cpp (ctor) |
| 23 | 5546 | 534 | yes | `api::CFormBuilder::AddTextRect(const std::string&, const SRect&) - inlines CTextRectField::CTextRectField (:51)` | src/uenux2/src/api/gui/cformbuilder.u16.cpp + src/uenux2/src/api/gui/ctextrectfield.cpp (ctor) |
| 24 | 6059 | 63 |  | `shared D0 body (vtable param) of CQRCodeImage / CDataText<std::function<std::string()>>` | library/inlined helper: merge-similar body (~std::function at +8, free); described in src/uenux2/src/api/gui/cqrcodeimage.cpp |
| 25 | 6060 | 60 |  | `shared D1 body (vtable param) of CQRCodeImage / CDataText<std::function<std::string()>>` | library/inlined helper: merge-similar body (~std::function at +8); described in src/uenux2/src/api/gui/cqrcodeimage.cpp |
| 26 | 10902 | 61 |  | `api::CTextRectField::GetClassName() const` | src/uenux2/src/api/gui/ctextrectfield.cpp |
| 27 | 10903 | 12 |  | `api::CTextRectField::~CTextRectField() (D0, via 6021)` | src/uenux2/src/api/gui/ctextrectfield.cpp |
| 28 | 10904 | 12 |  | `api::CTextRectField::~CTextRectField() (D1, via 6022)` | src/uenux2/src/api/gui/ctextrectfield.cpp |
| 29 | 10905 | 215 |  | `api::CTextRectField::Rect() const (:71)` | src/uenux2/src/api/gui/ctextrectfield.cpp |
| 30 | 10906 | 185 |  | `api::CTextRectField::Draw(IScreen&) const` | src/uenux2/src/api/gui/ctextrectfield.cpp |
| 31 | 10912 | 61 |  | `api::CTextFieldUpdate::GetClassName() const` | src/uenux2/src/api/gui/ctextfieldupdate.cpp |
| 32 | 10913 | 14 | yes | `api::CTextFieldUpdate::Rect() const (:73)` | src/uenux2/src/api/gui/ctextfieldupdate.cpp |
| 33 | 10914 | 20 |  | `api::CTextFieldUpdate::Stop()` | src/uenux2/src/api/gui/ctextfieldupdate.cpp |
| 34 | 10915 | 20 |  | `api::CTextFieldUpdate::Start()` | src/uenux2/src/api/gui/ctextfieldupdate.cpp |
| 35 | 10916 | 177 | yes | `api::CTextFieldUpdate::Draw(IScreen&) const` | src/uenux2/src/api/gui/ctextfieldupdate.cpp |
| 36 | 10917 | 13 |  | `api::CTextFieldUpdate::~CTextFieldUpdate() (D0)` | src/uenux2/src/api/gui/ctextfieldupdate.cpp |
| 37 | 10918 | 104 |  | `api::CTextFieldPaper::~CTextFieldPaper() (D0)` | src/uenux2/src/api/gui/ctextfieldpaper.cpp |
| 38 | 10919 | 101 |  | `api::CTextFieldPaper::~CTextFieldPaper() (D1)` | src/uenux2/src/api/gui/ctextfieldpaper.cpp |
| 39 | 10920 | 25 |  | `api::CTextFieldPaper::Draw(IPaper&) const` | src/uenux2/src/api/gui/ctextfieldpaper.cpp |
| 40 | 10921 | 73 |  | `api::CTextFieldMultiLine::GetClassName() const` | src/uenux2/src/api/gui/ctextfieldmultiline.cpp |
| 41 | 10922 | 104 |  | `api::CTextFieldMultiLine::~CTextFieldMultiLine() (D0)` | src/uenux2/src/api/gui/ctextfieldmultiline.cpp |
| 42 | 10923 | 101 |  | `api::CTextFieldMultiLine::~CTextFieldMultiLine() (D1)` | src/uenux2/src/api/gui/ctextfieldmultiline.cpp |
| 43 | 10924 | 295 |  | `api::CTextFieldMultiLine::Rect() const` | src/uenux2/src/api/gui/ctextfieldmultiline.cpp |
| 44 | 10925 | 564 |  | `api::CTextFieldMultiLine::Draw(IScreen&) const` | src/uenux2/src/api/gui/ctextfieldmultiline.cpp |
| 45 | 10926 | 73 |  | `api::CTextFieldDoubleLine::GetClassName() const` | src/uenux2/src/api/gui/ctextfielddoubleline.cpp |
| 46 | 10927 | 12 |  | `api::CTextFieldDoubleLine::~CTextFieldDoubleLine() (D0, via 6021)` | src/uenux2/src/api/gui/ctextfielddoubleline.cpp |
| 47 | 10928 | 12 |  | `api::CTextFieldDoubleLine::~CTextFieldDoubleLine() (D1, via 6022)` | src/uenux2/src/api/gui/ctextfielddoubleline.cpp |
| 48 | 10929 | 562 |  | `api::CTextFieldDoubleLine::Rect() const (inlines CalcLineRect :133 twice)` | src/uenux2/src/api/gui/ctextfielddoubleline.cpp |
| 49 | 10931 | 458 | yes | `api::CTextFieldDoubleLine::Draw(IScreen&) const` | src/uenux2/src/api/gui/ctextfielddoubleline.cpp |
| 50 | 10932 | 154 |  | `api::CTextFieldMT::~CTextFieldMT() (D0)` | src/uenux2/src/api/gui/ctextfieldmt.cpp |
| 51 | 10933 | 151 |  | `api::CTextFieldMT::~CTextFieldMT() (D1)` | src/uenux2/src/api/gui/ctextfieldmt.cpp |
| 52 | 10934 | 666 |  | `api::CTextFieldMT::Draw(IScreenMT&) const` | src/uenux2/src/api/gui/ctextfieldmt.cpp |
| 53 | 10940 | 21 |  | `api::CTextFieldBlinking::GetClassName() const` | src/uenux2/src/api/gui/ctextfieldblinking.cpp |
| 54 | 10941 | 154 |  | `api::CTextFieldBlinking::~CTextFieldBlinking() (D0)` | src/uenux2/src/api/gui/ctextfieldblinking.cpp |
| 55 | 10942 | 151 |  | `api::CTextFieldBlinking::~CTextFieldBlinking() (D1)` | src/uenux2/src/api/gui/ctextfieldblinking.cpp |
| 56 | 10943 | 14 |  | `api::CTextFieldBlinking::Rect() const (:70)` | src/uenux2/src/api/gui/ctextfieldblinking.cpp |
| 57 | 10944 | 20 |  | `api::CTextFieldBlinking::Stop()` | src/uenux2/src/api/gui/ctextfieldblinking.cpp |
| 58 | 10945 | 20 | yes | `api::CTextFieldBlinking::Start()` | src/uenux2/src/api/gui/ctextfieldblinking.cpp |
| 59 | 10946 | 74 | yes | `api::CTextFieldBlinking::Draw(IScreen&) const` | src/uenux2/src/api/gui/ctextfieldblinking.cpp |
| 60 | 10947 | 34 |  | `api::CTextField::GetClassName() const` | src/uenux2/src/api/gui/ctextfield.cpp |
| 61 | 10948 | 104 |  | `api::CTextField::~CTextField() (D0)` | src/uenux2/src/api/gui/ctextfield.cpp |
| 62 | 10950 | 101 |  | `api::CTextField::~CTextField() (D1)` | src/uenux2/src/api/gui/ctextfield.cpp |
| 63 | 10951 | 14 | yes | `api::CTextField::Rect() const (:55)` | src/uenux2/src/api/gui/ctextfield.cpp |
| 64 | 10952 | 114 | yes | `api::CTextField::Draw(IScreen&) const` | src/uenux2/src/api/gui/ctextfield.cpp |
| 65 | 10953 | 32 |  | `api::CTextBox::GetClassName() const` | src/uenux2/src/api/gui/ctextbox.cpp |
| 66 | 10954 | 87 |  | `api::CTextBox::~CTextBox() (D0)` | src/uenux2/src/api/gui/ctextbox.cpp |
| 67 | 10955 | 84 |  | `api::CTextBox::~CTextBox() (D1)` | src/uenux2/src/api/gui/ctextbox.cpp |
| 68 | 10956 | 500 |  | `api::CTextBox::Rect() const (:110)` | src/uenux2/src/api/gui/ctextbox.cpp |
| 69 | 10957 | 175 |  | `api::CTextBox::Move(const SPoint&) (:90)` | src/uenux2/src/api/gui/ctextbox.cpp |
| 70 | 10958 | 706 |  | `api::CTextBox::Draw(IScreen&) const (:69)` | src/uenux2/src/api/gui/ctextbox.cpp |
| 71 | 10964 | 12 |  | `api::CQRCodeImage::~CQRCodeImage() (D0, via 6059)` | src/uenux2/src/api/gui/cqrcodeimage.cpp |
| 72 | 10966 | 12 |  | `api::CQRCodeImage::~CQRCodeImage() (D1, via 6060)` | src/uenux2/src/api/gui/cqrcodeimage.cpp |
| 73 | 10967 | 1751 |  | `api::CQRCodeImage::GetImage() const (inlines static MontaImagem :36 + CBmpConversor::CreateBitmapHeader)` | src/uenux2/src/api/gui/cqrcodeimage.cpp |
| 74 | 10968 | 12 |  | `api::CStepsProgressBar::Rect() const` | src/uenux2/src/api/gui/cstepsprogressbar.cpp |
| 75 | 10969 | 73 |  | `api::CStepsProgressBar::GetClassName() const` | src/uenux2/src/api/gui/cstepsprogressbar.cpp |
| 76 | 10970 | 214 |  | `api::CStepsProgressBar::Move(const SPoint&)` | src/uenux2/src/api/gui/cstepsprogressbar.cpp |
| 77 | 10972 | 514 | yes | `api::(anonymous)::CabeLarguraIgual(IScreen&, rotulos, area, TFontSize, TPosition, TPosition, std::vector<SRect>&)` | src/uenux2/src/api/gui/cstepsprogressbar.cpp |
| 78 | 10973 | 13 |  | `api::CStepsProgressBar::~CStepsProgressBar() (D0)` | src/uenux2/src/api/gui/cstepsprogressbar.cpp |
| 79 | 10974 | 12 |  | `api::CProgressBar::Rect() const` | src/uenux2/src/api/gui/cprogressbar.cpp |
| 80 | 10975 | 61 |  | `api::CProgressBar::GetClassName() const` | src/uenux2/src/api/gui/cprogressbar.cpp |
| 81 | 10976 | 97 |  | `api::CProgressBar::Move(const SPoint&)` | src/uenux2/src/api/gui/cprogressbar.cpp |
| 82 | 10977 | 1914 | yes | `api::CProgressBar::Draw(IScreen&) const` | src/uenux2/src/api/gui/cprogressbar.cpp |
| 83 | 10978 | 13 |  | `api::CProgressBar::~CProgressBar() (D0)` | src/uenux2/src/api/gui/cprogressbar.cpp |
| 84 | 12303 | 12 |  | `api::CDataText<std::function<std::string()>>::~CDataText() (D0, via 6059)` | library/inlined helper: trivial template dtor (ctextsource.h), summarised in src/uenux2/src/api/gui/cqrcodeimage.cpp |
| 85 | 12305 | 12 |  | `api::CDataText<std::function<std::string()>>::~CDataText() (D1, via 6060)` | library/inlined helper: trivial template dtor (ctextsource.h), summarised in src/uenux2/src/api/gui/cqrcodeimage.cpp |

Related functions reconstructed here but listed in other units: 3667 `CProgressBar::SetValor` (u37),
5508 `CProgressBar::Incrementa` (u07), 10971 `CabeLarguraProporcional` (u34), 3889 shared `Rect` body (u33),
13550 `CPreShowProgressBar::PreShow` (u04, inlines the CStepsProgressBar constructor), 11805
`CTesteTeclado::StartState` (u26, inlines the CTextBox constructor), 10937 / 10909 (timer lambdas of
CTextFieldBlinking / CTextFieldUpdate, no unit).

## 13. Open questions

* Official names of `IFormFieldBase` slots 3-6 (`Start`/`Stop`/`RedrawIfDirty`/`SetForm` here), of the
  inlined "notify the form" helper, of `IImage` slot 2 (`GetImage`) and of most `IScreen` slots (only
  `GetFontMetrics` is attested); `IScreen` slot 29 (stores two colours in `CWasmScreen`) is unclear.
* Order of the three `uedword` parameters of the CProgressBar constructor (both callers pass 0, 0, max) and of
  the two colours of CTextBox; the member names of `IPaper::EStyle`.
* Whether `CTextField`'s "erase previous rectangle" flag (+60) is set by code that was not compiled in.
* What the orphaned check at `cqrcodeimagepaper.cpp:38` tested (probably `larguraMaxima == 0`, error 4950).
* Which state shows the "Número de cópias / acima do limite permitido" bands (presumably
  `CLimiteCopiasBUAtingido`).
