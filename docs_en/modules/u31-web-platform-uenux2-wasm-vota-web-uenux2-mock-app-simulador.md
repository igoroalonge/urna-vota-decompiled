# u31: the simulator's hardware mocks (`uenux2/mock/app/simulador/wasm`)

Unit u31 has **107 wasm functions**, all in the classes `simulador::CWasm*` of the simulator layer. On the
urna, the voting application reaches its hardware (screen, poll-worker terminal, printer, sound, clock,
timers, log daemon, "init" service, threads) through abstract interfaces registered in
`api::CPolySingletonList` (u19). The TSE web build registers **these classes** instead, and each one ends
either in a JavaScript import of the page (`js_*`), in MEMFS, or in nothing at all.

27 of the 107 functions ran during the recorded votes (`analysis/runtime/*.functions.tsv`): the drawing path
of the voter screen, the timers, the clock, the log writer, the resource loader, `CWasmInit::EnviarMensagem`,
and the mute/wait of the speech device.

Only one original path is attested, by the `std::source_location` record of `CWasmThread::Create`:
`/home/rubio/tse/uenux2/mock/app/simulador/wasm/cwasmthread.cpp:31`. The other files are named after their
classes, following the TSE convention (class `CFooBar` in `cfoobar.cpp`), in the same directory (**path
inferred**).

Reconstructed sources (all written by this unit):

```
src/uenux2/mock/app/simulador/wasm/cwasmscreen.{h,cpp}           CWasmScreen            (32 funcs)  voter screen -> <canvas>
src/uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.{h,cpp}  CWasmImageSurfaceOps   (16)        image size / placement
src/uenux2/mock/app/simulador/wasm/cwasmscreenmt.{h,cpp}         CWasmScreenMT + WasmText (14)      poll-worker terminal -> <pre id="mt">
src/uenux2/mock/app/simulador/wasm/cwasmtimer.{h,cpp}            CWasmTimer             (6 + 7909)  timers on setTimeout
src/uenux2/mock/app/simulador/wasm/cwasmtimerscheduler.cpp       CWasmTimerScheduler    (1)
src/uenux2/mock/app/simulador/wasm/cwasmsystemdatetime.cpp       CWasmSystemDateTime    (2)         urna clock = browser clock + offset
src/uenux2/mock/app/simulador/wasm/cwasmlogd.cpp                 CWasmLogd              (3)         log daemon -> dinamico/log/logd.dat
src/uenux2/mock/app/simulador/wasm/cwasmnullprinter.cpp          CWasmNullPrinter       (2)         report printer: nothing
src/uenux2/mock/app/simulador/wasm/cwasmnullpaper.cpp            CWasmNullPaper         (2)         report paper: nothing (BU vias!)
src/uenux2/mock/app/simulador/wasm/cwasmnullsound.cpp            CWasmNullSound         (1)
src/uenux2/src/api/hwil/isound.h                                 api::ISound inline defaults (4) (path inferred)
src/uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp             CWasmWebSound          (10)        speech -> <audio>
src/uenux2/mock/app/simulador/wasm/cwasmnulltexttospeech.cpp     CWasmNullTextToSpeech  (3)
src/uenux2/mock/app/simulador/wasm/cwasmresource.cpp             CWasmResource          (5)         ":/resource/..." -> MEMFS files
src/uenux2/mock/app/simulador/wasm/cwasminit.cpp                 CWasmInit              (3)         "init" service: fixed answers
src/uenux2/mock/app/simulador/wasm/cwasmthread.cpp               CWasmThread            (3)         threads: recorded, never run (attested path)
src/uenux2/mock/app/simulador/wasm/cwasmjs.h                     declarations of the js_* imports used above (name inferred)
src/uenux2/mock/app/simulador/wasm/cwasmutil.h                   declarations of the u29 helpers used above (names inferred)
```

---

## 1. Purpose, and the Portuguese terms

* **urna / UE (urna eletrônica)**: the voting machine. **eleitor**: voter. **mesário**: poll worker.
* **tela do eleitor**: the voter's display, 640 × 480 on the urna (`api::IScreen`).
* **MT (microterminal)**: the mesário's small keypad with a 4 × 40 character LCD and an LED (`api::IScreenMT`).
  **cabina livre / ocupada**: the voting booth is free / occupied (a voter is voting). The web page shows the MT
  as a text box; the LED becomes the line `CABINA: LIVRE` / `CABINA: OCUPADA`.
* **impressora / papel de relatórios**: the thermal printer that prints the **zerésima** (the "zero report" printed
  before the vote starts, proving the urna holds no votes), the **boletim de urna (BU)** (the signed
  per-machine result, printed in several **vias** = copies), the BJE (justification report) and others.
* **logd**: the urna's log daemon; every application writes its events to it, and the log of the urna
  is published after the election. **aplicação / severidade**: the application code and the level of a record.
* **MR (mídia de resultado)**: the USB memory card that receives the result files at the end of the day.
* **voto com áudio / acessibilidade**: the accessible vote, in which every screen and key is spoken (RHVoice speech
  synthesis, `docs/libraries/rhvoice.md`).
* **treinamento**: training mode. Every scenario of the public simulator is a training election.

In the voting process this layer is **all the I/O of the web urna**. When the voter presses a key, the key comes
from `CWasmInputKbd` (u30). The screen that answers is drawn by `CWasmScreen`, the confirmation beep comes from
`CWasmBeep` (u30), the spoken message from `CWasmWebSound`, and the "Voto confirmado para [Vereador]" record from
`CWasmLogd`. The status bar clock updates through a `CWasmTimer` and reads `CWasmSystemDateTime`.

## 2. How the objects get into the program

The registration code is in other units: `CSimuladorWasm::Executa` (func 8302, u19 §6.1), the lambda of `main`
(func 10384), and `votaInit` (func 7840, u29). The table below is the part of u19 §2.3 that concerns this unit:

| interface | web implementation (this unit) | registered by | replaces |
|---|---|---|---|
| `api::ITimerScheduler` | `CWasmTimerScheduler` | 8302, `ITimerScheduler::CreateInst<>` (itimerscheduler.h:40) | – (prevents the urna's `api::CTimerScheduler`, whose `CTimer` needs `std::thread`) |
| `comum::IInterfaceInit` | `CWasmInit(10)` | 8302 | – |
| `api::IScreen` | `CWasmScreen(w, h)` | 8302 | – |
| `api::IScreenMT` | `CWasmScreenMT` | 8302 | – |
| `api::IResource` | `CWasmResource` | 8302 | – |
| `api::ISound` | `CWasmNullSound`, then **`CWasmWebSound`** | 8302, then the `main` lambda 10384 | the null device |
| `api::IPaperRelatorios` | `CWasmNullPaper` | 8302 | – |
| `api::IImpressoraRelatorios` | `CWasmNullPrinter` | 8302 | – |
| `api::ITextToSpeech` | `CWasmNullTextToSpeech` (or `api::CRHVoiceTextToSpeech`) | 8302 and again `votaInit` | – |
| `api::ISystemDateTime` | `CWasmSystemDateTime` | 8302 (push 4890) | `api::CSystemDateTime` (`time(0)`) |
| `api::CEscritorLog` | `CWasmLogd("/dsk/fi/dinamico/log/logd.dat")` | 8302 | – |
| `IGenericFactory<api::IThreadImpl>` | `CDefaultGenericFactory<IThreadImpl, CWasmThread>` | `main` | – |

The objects are created once, when the page loads, and live until the page is closed. The page reloads
between voters, so nothing here persists (except MEMFS, which is also rebuilt).

## 3. Class hierarchy (RTTI)

```
api::IScreen (typeinfo 1531404, vtable 1529100, 39 slots)
 └─ simulador::CWasmScreen                      typeinfo 1529080  vtable 1528824 (table 545..583)   48 B
      has-a simulador::CWasmImageSurfaceOps at +44
api::IImageSurfaceOps (1529072)
 └─ simulador::CWasmImageSurfaceOps             typeinfo 1529060  vtable 1528988 (584..601)          4 B
api::IScreenMT (1531468, vtable 1529856, 20 slots)
 └─ simulador::CWasmScreenMT                    typeinfo 1529836  vtable 1529756 (615..634)         56 B
api::IText
 └─ simulador::CWasmScreenMT::ShowClock(api::SPoint const&)::WasmText   typeinfo 1530064  vtable 1530048
api::ITimer (1532124)
 └─ simulador::CWasmTimer                       typeinfo 1532112  vtable 1532088 (839..844)         12 B
      + std::__shared_ptr_emplace<CWasmTimer> (1532212), <CWasmTimer::State> (1532140)
api::ITimerScheduler (1531264)
 ├─ simulador::CWasmTimerScheduler              typeinfo 1532192  vtable 1532180                      4 B
 └─ api::CTimerScheduler (urna, unused)          vtable 1585404
api::ISystemDateTime (1531172)
 ├─ simulador::CWasmSystemDateTime              typeinfo 1532068  vtable 1532052                     16 B
 └─ api::CSystemDateTime (urna, replaced)        vtable 1585244
api::CEscritorLog (1599616, abstract)
 └─ simulador::CWasmLogd                        typeinfo 1530828  vtable 1530808                     40 B
api::IImpressora ← api::IImpressoraRelatorios (1583844)
 └─ simulador::CWasmNullPrinter                 typeinfo 1530712  vtable 1530652                      8 B
api::IPaper ← api::IPaperRelatorios (1531152)
 └─ simulador::CWasmNullPaper                   typeinfo 1530632  vtable 1530572                      4 B
api::ISound (1526576, abstract, no vtable of its own)
 ├─ simulador::CWasmNullSound                   typeinfo 1530416  vtable 1530368                      8 B
 └─ simulador::CWasmWebSound                    typeinfo 1528524  vtable 1528476                     16 B
api::IEsperaAudio
 ├─ api::CEsperaAudio                           typeinfo 1528600  vtable 1528584 (u32)
 └─ simulador::(anonymous)::CEsperaAudioWasm    typeinfo 1528684  vtable 1528668 (u30)
api::ITextToSpeech (1526340, vtable 1526528)
 ├─ simulador::CWasmNullTextToSpeech            typeinfo 1528752  vtable 1528704                     96 B
 └─ api::CRHVoiceTextToSpeech (vtable 1585556, voter audio on)
api::IResource
 └─ simulador::CWasmResource                    typeinfo 1530212  vtable 1530184                      4 B
comum::IInterfaceInit (1552464, vtable 1552160)
 └─ simulador::CWasmInit                        typeinfo 1528804  vtable 1528772                     28 B
api::IThreadImpl (1528436)
 └─ simulador::CWasmThread                      typeinfo 1528424  vtable 1528400                      8 B
```

---

## 4. The voter screen: `CWasmScreen`

### 4.1 Construction and scaling

The constructor is inlined into func 8302. It stores `m_escalaX = w / 640.0` (+16) and `m_escalaY = h / 480.0`
(+24), the logical size 640/480 (+32/+34, returned by `GetWidth`/`GetHeight`), and the default colours 2/0 (+36/+40,
written by `SetColors`, never read). It then calls `js_init(w, h)` and `Clear(1)` (white), and logs
`"CWasmScreen this=<address>"`. `w` and `h` come from `js_ler_dimensao_tela`: the URL parameters `screenWidth`,
`screenHeight`, `screen=WxH` (or `resolution`/`resolucao`), otherwise `Module.votaScreenWidth/Height`, otherwise
1280 × 800. They are stored as `short` by the caller.

Every coordinate crosses as `ceil(scale × v)`, computed with the saturating `i32.trunc_sat_f64_s`:

| quantity | scale |
|---|---|
| x, rectangle widths, line widths, circle radius, path x/w | `m_escalaX` |
| y, heights, **font size**, path y/h | `m_escalaY` |
| **natural image and animation width and height** (slots 22, 26, and 24 when no explicit size) | `m_escalaY` for both, so the image keeps its aspect ratio |
| explicit image size (slot 24) and image part (slot 25) | `m_escalaX` for the width, `m_escalaY` for the height |
| text width returned to the application | `ceil(px / m_escalaX) + 1`, returned as a full `int` (no `extend16`) |

At 1280 × 800, `sx = 2` and `sy = 1.667`. A 40-px font becomes 67 px, and a 161 × 225 candidate photo becomes
269 × 375.

Colours are indices (`api::TColor`) into a 38-entry table of CSS strings at @1529596 (listed in
`cwasmscreen.cpp`). An index above 37, compared unsigned, gives `"#000000"`. The compiler turned the source
`switch` into two lookup tables. The second one (@1529448) starts at index 1 and serves the call site that has
already tested `cor != 0` (the text background).

### 4.2 Slot map

| slot | func | method | what reaches JavaScript |
|---:|---:|---|---|
| 0 / 1 | 5071 / 8963 | `~CWasmScreen` | `IForm<IScreen>::RemoveAll` (5069) |
| 2 | 9265 | `GetMaxCharSize(w&, h&, font)` | – (`w = max(1, size/2)`, `h = size`) |
| 3 | 9261 | `GetFontMetrics(w&, h&, font, text)` | via slot 32 |
| 4 | 9254 | `Clear(color)` | `js_log("Clear(full)")`, `js_clear` (also removes the GIF overlays) |
| 5 | 9243 | `ClearRect(rect, color)` | → slot 6 |
| 6 | 9232 | `FillRect(rect, color)` | `js_fill` |
| 7 | 9223 | `SetPixel(p, color)` | `js_fill(x, y, 1, 1)` |
| 8 | 9213 | `DrawLine(p1, p2, color, width)` | `js_line` |
| 9 | 9205 | `DrawRect(rect, color, width)` | 4 × slot 8 |
| 10 | 9194 | `DrawCircle(c, r, color, width)` | `js_circle` |
| 11 | 9184 | `DrawRoundRect(rect, r, color, width)` | `js_round_rect` |
| 12 | 9175 | `FillGrayGradient(rect)` | `js_gray_gradient` |
| 13 | 218 | no-op **inherited** from `api::IScreen` (same table entry 558 in both vtables) | – |
| 14 | 218 | no-op override (own table entry 559; pure in `IScreen`) | – |
| 15 | 9165 | `DrawTriangle(a, b, c, color, width)` | 3 × slot 8 |
| 16 | 9155 | `DrawPolygon(vector<SPoint>, color, width)` | n × slot 8 (closed) |
| 17 / 18 | 9146 / 9125 | `FillPath` / `DrawPath` | `js_path(n, int*, double*, css, fill, width)` (docs/03 §8.3) |
| 19 | 9115 | `WriteText(SPoint, IText, font, fg, bg) → SRect` | → slot 20 |
| 20 | 9104 | `WriteText(SRect, IText, font, fg, bg) → SRect` | `js_fill` (background if bg ≠ 0), `js_log("Write '…'")`, `js_text` |
| 21 | 9090 | `WriteText(SPoint, SRect clip, …)` | → slot 19; **the clip is ignored** |
| 22 | 9079 | `DrawImage(pos, IImage, anchor)` | `js_image` |
| 23 | 9068 | `DrawSurface(pos, ?, anchor)` (?) | a 64 × 64 grey placeholder (`FillRect` 5 + `DrawRect` 3) |
| 24 | 9058 | `DrawImage(pos, IImage, size, anchor)` | `js_image` |
| 25 | 9049 | `DrawImage(pos, IImage, SRect part)` | `js_image`; the whole image is squeezed into the part's size |
| 26 | 9038 | `DrawMovie(pos, CMovie, anchor)` | `js_image` of `frames.at(frameAtual)` |
| 27 | 9022 | `Refresh()` | `js_log("Refresh()")`, event `vota:screen {"refreshed":true}` |
| 28 | 9018 | `RefreshRect(rect)` | event `vota:screen {"refreshed":true,"partial":true,"rect":{…}}` |
| 29 | 9013 | `SetColors(fg, bg)` | – |
| 30 / 31 | 8959 / 8953 | `GetWidth` / `GetHeight` | – (640 / 480) |
| 32 | 9009 | `int GetTextWidth(text, font)` | `js_measure_text_width` |
| 33 | 9004 | `Clear(int, color)` (?) | → slot 4 |
| 34 | 8945 | `GetImageSurfaceOps()` | – (`this + 44`) |
| 35, 36 | 218 | no-op overrides (table entries 580/581; pure in `IScreen`) | – |
| 37, 38 | 218 | no-ops **inherited** from `api::IScreen` (entries 582/583 in both vtables) | – |

### 4.3 Text (`WriteText`, func 9104), step by step

1. `utf8 = Latin1ParaUtf8(text.GetText())` (func 5098). The application's strings are ISO-8859-1, and the glue
   reads UTF-8.
2. `GetFontMetrics` (slot 3 → slot 32, a round trip to `canvas.measureText`) gives the logical width. The height is
   the font size, and both are at least 1.
3. The logical box starts at the rectangle's left/top. It is moved to `right − w + 1` for `Right` (1), or centred
   for `Center` (2). This box is the return value: fields keep it to erase the text later.
4. The box is scaled. If `bg ≠ 0` (0 is transparent), the box is filled with the background colour.
5. The canvas alignment is 0/1/2 = left/centre/right, with `x` at the box's left, middle or right edge.
6. `js_log("Write '<first 40 chars>'")`. The string is built even though the glue prints it only when
   `Module.uenuxDebug` is set.
7. `js_text(x, baseline, boxWidth, utf8, ceil(sy·size), bold = style&1, italic = style>>1 &1, css(fg), align)`,
   with `baseline = bottom − height/4 + ceil(2·sy)`. The glue ignores `boxWidth`.

`GetText()` is called three times per text (steps 1, 2 and 6), and each call copies the string.

### 4.4 Images and animations

`DrawImage` (slot 22) copies the encoded bytes from `IImage::GetImage()`. It measures them with
`CWasmImageSurfaceOps::GetImageSize` (func 3493), scales the size by `sy`, applies the anchor and passes the bytes to
`js_image`. The glue copies them again (`HEAPU8.slice`), detects the type (PNG/JPEG/GIF) and draws them. GIFs are
drawn as an `<img>` overlay so that the browser animates them.

The anchor rule is shared by every anchored primitive (`EAnchorPoint`, names inferred):

| value | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
|---|---|---|---|---|---|---|---|---|---|
| point is the image's | top-left | top-right | top-centre | middle-left | middle-right | centre | bottom-left | bottom-right | bottom-centre |

**The size parser (func 3493, also inlined in `CWasmResource::GetMovie`)**

| format | rule | result |
|---|---|---|
| GIF (`GIF`, ≥ 10 B) | LE16 at 6 / 8 | width, height |
| PNG (signature, ≥ 24 B) | BE16 at 18 / 22 (low half of the IHDR fields) | width, height |
| JPEG (`FF D8`, ≥ 12 B) | walks the segments up to the first SOFn (C0–C3, C5–C7, C9–CB, CD–CF), stops at EOI/SOS or on a bad length | width, height |
| anything else, including **BMP** | – | **1 × 1** |

Every QR code the urna shows on screen is an 8-bit **BMP** built by `api::CQRCodeImage` (u16). So in the web build
every QR code is drawn as a **1 × 1 logical pixel (2 × 2 px)**. This is verified in §11.

In the web build, animations (`DrawMovie`) have a single "frame" that holds the whole GIF (§8).

`CWasmImageSurfaceOps` (18 slots) contains:

* the size parser, as slot 12 (bytes) and slot 13 (`IImage`, used by `comum::CInfoMTLCD::Update` to centre an image
  on the MT);
* `CalcRect(w, h, pos, anchor)` (slot 14; its far edges are `pos + size`, not `pos + size − 1`);
* `GetImageRect` for bytes, for `IImage&` and for `shared_ptr<IImage>` (slots 15–17; `CImageField::Rect`,
  `CImageFieldUpdate::Rect`).

Slots 2–11 manage "surfaces", which here are just `shared_ptr<vector<uebyte>>` copies of the encoded file, plus raw
`new[]`/`delete[]` buffers. Nothing in this binary calls them, and one argument type (an object whose virtual slot 3
returns an int) is unknown. Their names are marked `?`.

### 4.5 Refresh events

`IForm::Redraw` ends with `IScreen::Refresh()` (slot 27). The web version posts `vota:screen` to the page, where the
adapter shows "Tela atualizada." ("screen updated"). The partial form (slot 28) did not occur in the recorded
sessions.

---

## 5. The poll worker's terminal: `CWasmScreenMT`

State: four `std::string` lines of exactly 40 characters (+4 … +40) and `bool m_cabinaOcupada` (+52).

* `Refresh` (slot 14, func 8746) publishes
  `line1\nline2\nline3\nline4\nCABINA: LIVRE|OCUPADA` with `js_mt_set(text, ocupada)`. The page puts the text in
  `<pre id="mt">` and turns its border green (#34a853) when the booth is occupied.
* `Write(pos, text)` (slot 3, func 8799) handles only lines `pos.y` = 1..4 (1-based). The column is:
  * the centre for `Center` texts of at most 39 characters;
  * `40 − len` for `Right`;
  * otherwise `max(pos.x − 1, 0)`.

  The text is cut at 40 characters and written in place. Nothing outside the 4 × 40 grid is ever written.
* LED operations (`api::CLedFieldMT`, ops 0..3 = slots 8..11): ops 0 and 3 turn the LED off (func 5038) and ops 1
  and 2 turn it on (func 5036), then the MT is repainted.
* `ShowClock(pos)` (slot 12, func 8757; the name comes from the RTTI of its local class `WasmText`) writes
  `CDateTime::Now().GetTime().Format("hh:mm")` with a left-aligned local `IText`.
* `Bipa(freq, n)` (slot 6), slot 7 and slot 19 only repaint: the MT has no buzzer in the page. Slot 19 is called with
  1 by `CPedeIdentidade` and with 0 by `CEscolheOpcao`, and only on urnas of model ≥ 2020.
* `GetWidth`/`GetHeight` return 480 × 80, the size of the MT LCD in pixels. Slot 4 (draw an image on the LCD) is a
  no-op.

In the page, nothing drives the operator thread (u29, `docs/bu/codepath.md` §6). The MT therefore shows only what
the start-up wrote: four blank lines and `CABINA: LIVRE`. Verified: a copy of `tools/run/headless.mjs` that logs
`js_mt_set` records exactly one call for a whole municipal vote (`--keys "91001C  C  12C  C  "`). The booth never
shows as occupied, even while the voter is voting.

---

## 6. Time: timers and the clock

### 6.1 `CWasmTimer` / `CWasmTimerScheduler`

The urna's `api::CTimer` runs a `std::thread`, which throws "thread constructor failed" in this build. The web timer
keeps its state in a shared `State {bool ativo; uint64 geracao; int intervalo; std::function<void()> callback}`
and re-arms itself through JavaScript:

```
CreateTimer(ms, cb)          (7871)  make_shared<CWasmTimer>: State{ativo=false, geracao=0, ms, cb}  -- returned stopped
Start()                      (7918)  ativo=false,++geracao (Stop inlined); ativo=true; g=++geracao;
                                     emscripten_async_call(838, new {shared_ptr<State>, g}, intervalo)
   JS: setTimeout(intervalo) -> callUserCallback(table[838](ctx))
Dispara(ctx)                 (7909)  if (ativo && geracao == g) { callback();
                                         if (ativo && geracao == g) emscripten_async_call(838, new {state, g}, intervalo) }
                                     delete ctx
Stop()                       (7934)  ativo=false; ++geracao           -- queued callbacks become no-ops
~CWasmTimer                  (3347)  Stop(); release State (queued contexts still own it)
```

The generation counter makes a stop or restart safe while callbacks are already queued in JavaScript. The callback
runs **outside `votaTick`**, from the browser event loop, so it is not inside the `try/catch` of the exported
functions (§13).

### 6.2 `CWasmSystemDateTime`

`GetDataHora()` = `m_deslocamento + trunc(js_obter_data_hora_local_navegador())`. The import returns
`Date.getTime()/1000 − getTimezoneOffset()·60`: the local wall-clock time encoded as a UTC epoch.
`api::CDateTime::ConvertFromLocalTime` (5476) decodes it with `gmtime_r`, so the urna shows the browser's local
time. `SetDataHora(t)` (reached from `api::CAjusteDataHora`, e.g. when the voting cycle starts) only stores
`t − browserNow` as an offset. No clock is changed. The layout of `ISystemDateTime` puts its two methods before the
virtual destructor (slots 0/1, destructor 2/3).

---

## 7. Sound

### 7.1 `api::ISound` and its inline defaults (`src/uenux2/src/api/hwil/isound.h`)

`ISound` has no vtable of its own, but it defines four inline virtual defaults. Their code was emitted with
`CWasmNullSound`, the only class that does not override them, so the tools named them `CWasmNullSound::vf2/4/8/9`.
The RTTI of their lambdas proves where they belong: `api::ISound::Wait()::'lambda'()` and
`api::ISound::WaitAsync(std::function<bool ()>, std::function<void (bool)>)::'lambda'()`. Clang spells lambdas of
inline functions `'lambda'`, and those of ordinary functions `$_N`.

| slot | default (func) | behaviour |
|---:|---|---|
| 2 | `Play(arquivo, enfileira)` (8479) | `if (enfileira) Wait(); Play(arquivo);` |
| 4 | `Play(audio, enfileira)` (8472) | `if (enfileira) Wait(); Play(audio);` |
| 8 | `Wait(interromper)` (8461) | `while (GetStatus()==Tocando) { if (interromper && interromper()) return false; sleep_for(20ms); } return true;`. In this build, `sleep_for` is `emscripten_sleep(20)`, which **aborts** |
| 9 | `WaitAsync(interromper, aoTerminar)` (8450) | synchronous: `espera = make_shared<CEsperaAudio>(); ok = Wait([espera, interromper]{…}); if (!espera->Cancelada() && aoTerminar) aoTerminar(ok); return espera;` |

`m_volume` (+4, 0..10, starting at 9) is a public member. `vota::CInstrucaoVotacaoAcessibilidade` changes it
directly with keys 3/9 (u08).

### 7.2 `CWasmNullSound`

`CWasmNullSound` is silent, and its `GetStatus()` always returns `Parado` (stopped), so the polling loop above never
runs. The lambda of `main` replaces it with `CWasmWebSound` before any vote starts.

### 7.3 `CWasmWebSound`

State: `m_status` (+8, written but never read, because `GetStatus` asks JavaScript) and `m_mudo` (+12).

* `Play(audio, enfileira)` (9559): when not muted and the WAV holds samples, it calls
  `js_wasm_web_sound_play_wav(header, 44, samples, size, m_volume·5 + 50, enfileira)`. The JS side queues the sound
  (`enfileira` = 1) or stops the current sound and clears the queue first (0).
* `Play(arquivo)` (slots 1/2): sound files are not supported, so the key click `":/resource/sounds/tecE.wav"` is never
  heard.
* `Wait(interromper)` (9523) **does not wait**. It only answers whether the sound has ended.
* `WaitAsync(interromper, aoTerminar)` (9515) has three cases:
  1. If `interromper()` is already true, it returns an already-cancelled `CEsperaAudio` and does not call
     `aoTerminar`.
  2. If nothing is playing, it calls `aoTerminar(true)` synchronously and returns a fresh `CEsperaAudio`.
  3. Otherwise it takes `id = s_proximoIdEspera++` (@1528464, starting at 1) and stores both functions in
     `std::map<int, SEspera>` (@1832664). It then calls `js_wasm_web_sound_wait_async(id)`: JS polls every 20 ms,
     asks export 9614 "cancel requested?", and finally calls export 9604 "finished(id, ok)". The function returns
     `make_shared<CEsperaAudioWasm>(id)`, whose `Cancela()` (u30) erases the entry and calls
     `js_wasm_web_sound_cancel_wait`.
* `Mute`, `Stop`, `Pause` and `GetStatus` map one to one onto their imports.

### 7.4 `CWasmNullTextToSpeech`

`CWasmNullTextToSpeech` is the `ITextToSpeech` used when the voter audio is off. The constructor (13877) runs only
the base constructor: an LRU capacity of 256, speed level 2 (100 %), and fields 50/100. `Sintetiza` (slot 11,
func 9436) returns an empty WAV pointer, which `CWasmWebSound::Play` ignores.

---

## 8. Resources: `CWasmResource`

Resource names are Qt-style: `":/resource/images/…"`, `":/resource/gifs/…"`, `":/resource/sounds/…"`.

* `IsResource(nome)` (slot 6) returns `nome[0] == ':'`.
* `Exists(nome)` (slot 5) is true when the resolver finds a file.

  u17 calls slot 6 `Exists`. The helper that calls slot 6 is `api::isResource` (iresource.h:97), and the body
  tests the prefix, so this unit names slot 6 `IsResource` and slot 5 `Exists`.
* `GetFile` (slot 3, "file"), `GetImage` (slot 2, "image", which wraps the bytes in a `CFixedImage`) and
  `GetMovie` (slot 4, "movie") all:
  1. resolve the name with func 2626 (u29), which strips the `:` and maps `/resource/images/` to
     `/uenux/app/img/` or `/pkg/img/`, and `/resource/gifs/` to `/pkg/gifs/`, trying each candidate;
  2. read the file whole (func 3452);
  3. call `js_resource_log(action, name, path, size)` (printed only when debugging).

  A missing resource gives zero bytes, and the screen skips it silently.
* `GetMovie` builds **one** `CMovieFrame{whole GIF, 80 ms}` and `CMovie(frames, size)` (func 5526, which takes the
  vector by value and moves it). The frame vector is built from an `initializer_list`, so the whole GIF is copied
  once more (`operator new` + `memcpy` in 8605) before the temporary frame is freed. The size comes from the GIF
  header. The browser does the animation.

## 9. Log: `CWasmLogd`

The constructor (inlined in 8302):

```
create_directories("/dsk/fi/dinamico/log"); fopen(".../logd.dat", "wb"); fclose   // truncated at each page load
```

`Escreve(aplicativo, severidade, mensagem)` (slot 4, func 8327; `CEscritorLog::loga` checks the application
code < 61 before calling it):

```
linha = std::format("{}|{}|{}", aplicativo, severidade, mensagem)      // e.g. "1|1|Voto confirmado para [Vereador]"
lock; f = fopen(arquivo, "ab"); fwrite(linha); fputc('\n'); fflush; fclose; unlock   // errors ignored
CWasmLogBus::GetInst().Publica(LOGD, linha)                           // func 5152 (u29 "log bus", channel 2): "[date] linha"
                                                                      // kept in a deque (max 2000 lines), no subscriber
```

The file is Latin-1 plain text. It has none of the urna log's structure (hash chaining, signatures): the real
`logd` is not in this build. After a complete vote the log is the only file in MEMFS that changes
(`analysis/runtime/README.md`). The text records events ("Voto confirmado para [Vereador]"), never vote contents.

## 10. Hardware stubs

### 10.1 `CWasmInit`: the "init" service (func 9405)

Every command is published on channel 0 of the simulator's in-memory log bus as `CWasmInit::<cmd> <text>` (func 1524,
`LogComando`, u29).

| command (u23 names) | answer `{status}` | log text |
|---|---:|---|
| 6 (?) | 1 | `= FE_NOT_PRESENT` |
| 7 | 0 | – |
| 10 habilita MR, 11, 34 monta MR, 36, 47, 48 desliga, 58 desmonta MR, 68 | 0 (OK) | – |
| 15, 16 | 1 | `= no` |
| 17 MR montada?, 18 MR presente? | 1 (= false) | `= no` |
| 39 (?) | 512000 | `= 512000` |
| 67 modo demonstração? | 0 (no) | `= 0` |
| any other, including 12 (MR device index → 0) and 37 (boot device → 0) | 0 | `= default` |

Other slots return fixed values: boot device 1 (slot 2), `true` (slot 3), 10 (slot 4, the constructor argument),
and `false` (slot 6).

### 10.2 `CWasmNullPrinter`, `CWasmNullPaper`

`CWasmNullPrinter` only remembers the style (`DoSetStyle`) and reports 19 columns for style 3 (double width), 38
otherwise. Its paper-sensor methods are the `IImpressora` defaults, which throw "Not supported".

`CWasmNullPaper` discards every report, both when it is **composed** and when it is **printed**:

* slot 5 `Abre(arquivo)`, slot 2 `Print(IText, style)`, slots 4/3/12 (line feed, cut, QR image: the roles recorded by
  `tools/bu/operator_harness.mjs`) and slot 6 `Fecha`. This is how the application composes a report into a file
  such as `<trab>/bu.dat`; in the web build that file is never written;
* slot 7 and slot 8, `ImprimeArquivo` (print a composed file);
* slot 10, `AguardaFimImpressao`.

### 10.3 `CWasmThread`

`Create(fn, arg)` (cwasmthread.cpp:31) throws `CUeIpcError` 6231 "CWasmThread::Create sem função válida" when `fn` is
null. Otherwise it appends `{fn, arg, terminou=false, id=++n}` to a global vector (@1832648) and logs
`"Create #n - enqueued"`.

**Nothing ever reads that vector**: the thread function never runs. `Wait()` loops on `emscripten_sleep(100)` while
`terminou` is false, and `terminou` is never set. `Yield()` calls `emscripten_sleep(0)`. Both abort the module (no
Asyncify). `js_thread_log` prints to the console unconditionally.

---

## 11. The boletim de urna (BU) at this layer

The BU is generated by `vota::CGeraBU` and printed by `vota::CImprimindoBU` (u09, `docs/bu/codepath.md`). The public
page never reaches the encerramento (the end-of-day closing), because the operator thread does not run. The
`tools/bu/operator_harness.mjs` harness does reach it, and at that point the following classes of this unit take
part:

1. **Clock.** The BU date/time fields (`dataHoraEmissao`, the vote start/end) come from `CDateTime::Now()`, which is
   `CWasmSystemDateTime`: the browser's local clock plus the offset left by any `SetDataHora`. The harness sets a
   fake clock (`docs/bu/codepath.md` §6).
2. **Log.** Every BU step logs through `CWasmLogd`, for example `"Imprimindo relatório [BU] via nº [1]"`. Each line
   becomes `<app>|<sev>|<text>` in `/dsk/fi/dinamico/log/logd.dat`.
3. **Printing the vias.** `CImprimindoBU::ImprimeBU(via, modo)` calls
   `IPaperRelatorios::ImprimeArquivo(trab/bu.dat, CSigVerifier(trab,"bu.dat","bu.vsu"), header "<n>a. VIA",
   "boletim de urna", "<n>ª via")` (slot 8), then `AguardaFimImpressao()` (slot 10).
   * In the web build slot 8 is `CWasmNullPaper::ImprimeArquivo` (func 8371). It only releases the header form: no
     byte is printed, and the **signature check the printer service would make on `bu.dat` against `bu.vsu` never
     runs**.
   * Slot 10 returns at once.
   * The caller still increments `qtdBU` in `vota.bin`, so the urna believes the via was printed.
   * Before that, the BU text itself was **composed** through slots 5 (`Abre(<trab>/bu.dat)`), 2/4/3/12 (text, line
     feed, cut, QR) and 6 (`Fecha`), which are no-ops too: the file that slot 8 is asked to print does not exist in
     the web build. Verified with the harness (flow treino, whose recording paper replaces these slots): four
     reports are composed (`bu`, `buj`, `bim`, `behb`: 124/69/42/73 calls between `Abre` and `Fecha`), then exactly
     one slot-8 call and one slot-10 call.
   * The zerésima and the other reports go through the same composition slots and slot 7, all no-ops.
4. **Printer width.** `CWasmNullPrinter::GetColumns` gives 38 (19 in double width), which is what the report
   layout code sees.
5. **Result media.** `CWasmInit` answers "MR not present / not mounted" (17/18 → 1) and "OK" to mount, unmount and
   enable (34/58/10 → 0). `IInterfaceInit::DispositivoMR` also sleeps 500 ms through `emscripten_sleep` (u23), which
   aborts. The copy of the result files to the MR therefore cannot work in the browser.
6. **BU on screen (BU digital).** `vota::CMostraQRCodeBU` shows the BU as QR codes on the voter screen. It uses
   `CQRCodeImage(380, QRCodeAtual)`, an **8-bit BMP** of 145,478 bytes (1078-byte header + 380 × 380 pixels), drawn
   through `CImageField` → `CWasmScreen::DrawImage` (slot 22).

   `GetImageSize` does not parse BMP, so the image measures 1 × 1 and is drawn at **2 × 2 px**. Verified by running
   the harness with `js_image` logged (a copy of `tools/bu/operator_harness.mjs --flow treino` in which `js_image`
   prints instead of returning):

   ```
   [state] voter=vota::CEmitirMaisBU
   IMG 145478 42 4d 1258 59 2 2          <- js_image(bytes, 145478 ("BM"), x=1258, y=59, w=2, h=2)
   [state] voter=vota::CMostraQRCodeBU
   ```

   The QR **content** is correct (`docs/bu/qrcode.md`). Only the display is unusable in the simulator. The certificate
   QR (`CMostraQRCodeCertificado`) and the urna-state QR of the zerésima screen go through the same path. On the urna
   the real `IScreen` decodes BMP.
7. **Signatures.** This layer signs nothing. The `.vsu` files contain `"assinatura simulada para vota_web_wasm"`,
   written by u29.

## 12. wasm / Emscripten observations

* **Identical-code folding.** Many slots are shared no-op bodies:
  * 218 `void()`, 425 `void(x)`, 1528 `void(x,y)`, 1870 `void(x,y,z)`;
  * 340 `return 0`, 434 `return 1`, 371 `return 1` with an argument;
  * 174 `return this` (trivial destructor), 144 `operator delete` (deleting destructor).

  Identical methods of one class also fold: MT slots 8 = 11, 9 = 10 and 7 = 19. Destructors of classes with a
  `std::string` at +8 fold across classes: `WasmText` uses 1727/1969, which take the vtable as a parameter.
* **`std::shared_ptr` is `[[clang::trivial_abi]]`** (libc++ ABI v2). A `shared_ptr` passed by value is released by
  the **callee**. That explains the otherwise empty-looking bodies 8487 and 8371 (only a release) and the
  +1 / −2 reference counting in 9565. A `std::function` passed by value is still destroyed by the caller.
* **Saturating float→int.** `ceil(scale·v)` compiles to `i32.trunc_sat_f64_s`. With a zero or negative scale
  (for example `?screenWidth=65536`, which truncates to `short` 0), divisions give ±inf/NaN, which saturate instead of
  trapping.
* **`emscripten_async_call`** takes a table index (838) as a C function pointer. The glue calls it through
  `getWasmTableEntry` inside `callUserCallback`, so the wasm is re-entered from a JS timer without going through an
  export.
* **`std::this_thread::sleep_for`** becomes `if (byte@1584624 == 1) emscripten_sleep(ms)`. The byte is initialised to 1
  and never written.
* **Switch → lookup table.** The colour switch produced two tables, the second with index 0 removed (§4.1).
* **Function-local statics.** The web-log buffers are one guarded static (@1832676/@1832844), inlined into both 8327
  and 1524.

## 13. Suspicious or risky code

| # | func(s) | finding | impact | severity |
|---:|---|---|---|---|
| 1 | 3493 (+ inlined in 8605) | The image size parser knows GIF/PNG/JPEG only. **Every BMP is 1 × 1**, and all on-screen QR codes (BU digital, certificate, urna state) are BMPs, so they are drawn as 2 × 2 px dots. Verified for the BU QR (145,478-byte BMP → `js_image(…, 1258, 59, 2, 2)`, re-run for this review with the harness, flow treino). The 66-byte BMP of the number-entry screens (u39) also reaches `js_image(…, 2, 2)`, but it really is a 1 × 1 placeholder, so it is not evidence of the bug. | Simulator: the BU digital screen is unusable. It is reached only through the harness. The real urna has its own `IScreen`. | medium |
| 2 | 9655, 9652, 9651 | `CWasmThread` never runs threads. `Create` enqueues into a vector that nothing reads, `Wait` spins on `emscripten_sleep(100)` for a flag nobody sets, and `Yield` calls `emscripten_sleep(0)`. Both abort the module (no Asyncify). | Any path that starts and joins an `api::CThread`, or that runs a thread loop, crashes the simulator. The page avoids them by driving the state machines from `votaTick`. | medium |
| 3 | 8461 (+ 8479, 8472, 8450) | `ISound::Wait` default polls with `sleep_for(20ms)`, which is `emscripten_sleep` and aborts. The defaults for slots 2/4/9 call it. | Unreachable: `CWasmNullSound` always reports stopped, and `CWasmWebSound` overrides all of them. Would abort if a future device relied on the defaults. | low |
| 4 | 9523, 9515 | `CWasmWebSound::Wait` does not wait. `WaitAsync` calls `aoTerminar(true)` **synchronously**, before the caller has stored the returned handle, when nothing is playing. It never calls `aoTerminar` when `interromper()` is already true. | Timing differs from the urna. `CVotacaoStateAudio::PlayMessage` (CONFIRMA) no longer blocks until the end of the message. | low |
| 5 | 7909, 7918 | Timer callbacks run from a JS `setTimeout`, outside `votaTick`'s try/catch. An exception from a callback (including `bad_function_call`) escapes to the browser as an uncaught error: the timer is never re-armed, and the 16-byte context leaks together with its `shared_ptr` reference, so the `State` and the callback's captures are never freed. `__stack_pointer` is not restored either: `callUserCallback` has no `stackRestore` (only the `invoke_*` wrappers do). | Simulator: a clock, blinking field or battery icon could silently stop updating. No crash of the module. | low |
| 6 | 8302 → CWasmScreen ctor, 9009 | The screen size from URL parameters is truncated to `short` by the caller. `?screenWidth=65536` gives 0: `m_escalaX = 0`, every x collapses to 0 (verified with headless), and `GetTextWidth` divides by 0: `+inf` saturates to `INT_MAX` in its `int` result, which `GetFontMetrics` stores as the `short` −1 (clamped to 1 by `WriteText`), and an empty text gives `NaN` → 0. Values from 32768 to 65535 give negative scales. | Only the user who crafts the URL is affected: a broken layout, no crash. | low |
| 7 | 9090, 9049, 9068 | `WriteText` with a clip ignores the clip, `DrawImage(pos, img, part)` squeezes the whole image instead of cropping, and slot 23 draws a grey 64 × 64 placeholder. | Visual differences from the urna (the two-colour progress-bar label, image parts). | low |
| 8 | 8371, 8372 (+ ICF slots) | Printing is a no-op: the BU vias, zerésima, BJE and boletim de mesários are never produced. Even their composition (`Abre`/`Print`/`Fecha`, slots 5/2/6) is discarded, so the report files handed to `ImprimeArquivo` are never written. The `CSigVerifier` passed with `bu.dat`/`bu.vsu` is never evaluated, and `AguardaFimImpressao` returns at once, yet the application counts the via as printed. | Simulator only. The integrity check before printing is absent, not simulated. | info |
| 9 | 9405 | `CWasmInit` answers every hardware command with canned values: power-off returns OK and does nothing, and the MR is always absent. | Simulator only. It explains why the end of day cannot copy results. | info |
| 10 | 7954, 7945 | The urna clock is the browser clock (local time as epoch) plus an offset. `SetDataHora` changes no real clock. | Simulator only. Dates in any generated file follow the visitor's PC clock. | info |
| 11 | 8327 | `CWasmLogd` truncates `logd.dat` at every page load and ignores write errors. The records are plain text with no chaining or signature. | Simulator only. The log is not a faithful model of the urna log. | info |
| 12 | 9104, 9079, 9058, 8991, 8605 | Performance: each text draw copies the string three times (`GetText()` by value), converts it, and builds a debug string that JS then drops. `DrawImage` copies the encoded file once (`IImage::GetImage()` by value), and the field's rectangle (8991) copies it again. `DrawMovie` does not copy, but `GetMovie` copies each GIF (0.8–1.6 MB) once more after reading it (`initializer_list`). On every draw, the glue copies the bytes (`HEAPU8.slice`) and FNV-hashes all of them for its image cache. | CPU and memory cost only when something is drawn. It is **not** part of the ~2,000 `invoke_*` round trips per idle tick of `analysis/runtime/README.md`, which happen without any redraw. | info |

No code in this unit uses the network, reads cookies or local storage, or sends data out of the page. The only
page inputs it depends on are the URL screen size (§4.1) and the audio on/off flag (through `votaInit`, u29).

## 14. Mapping table: every function of the unit

`ran` = observed executing during the recorded votes. Paths are relative to `uenux2/…`; "inferred" marks a path
inferred from the class name. One extra function outside the unit belongs to these classes:
**7909** `simulador::CWasmTimer::Dispara` (table slot 838; the tools left it `unknown_f7909`), in `cwasmtimer.cpp`.
Helpers these classes call but that belong to u29 are listed in `cwasmutil.h`: 5098, 5113, 3533, 2626, 3452, 5152,
1524, 2662.

| func | size | ran | tools name | reconstructed symbol | original file | src | conf. | evidence |
|---:|---:|:-:|---|---|---|---|---|---|
| 3347 | 84 |  | `simulador::CWasmTimer::vf0` | `simulador::CWasmTimer::~CWasmTimer` | `uenux2/mock/app/simulador/wasm/cwasmtimer.cpp` (inferred) | […/wasm/cwasmtimer.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmtimer.cpp) | high | slot 0; Stop inlined + release State |
| 3493 | 487 |  | `simulador::CWasmImageSurfaceOps::vf12` | `simulador::CWasmImageSurfaceOps::GetImageSize` | `uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp` (inferred) | […/wasm/cwasmimagesurfaceops.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp) | high | slot 12; GIF/PNG/JPEG header parser (BMP -> 1x1) |
| 5036 | 22 |  | `simulador::CWasmScreenMT::vf9` | `simulador::CWasmScreenMT::LedOperacao1` | `uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp` (inferred) | […/wasm/cwasmscreenmt.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp) | low | slots 9 and 10 (CLedFieldMT::Draw calls slot op+8, ops 1/2); LED on -> CABINA: OCUPADA; descriptive name |
| 5038 | 22 |  | `simulador::CWasmScreenMT::vf8` | `simulador::CWasmScreenMT::LedOperacao0` | `uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp` (inferred) | […/wasm/cwasmscreenmt.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp) | low | slots 8 and 11 (CLedFieldMT::Draw calls slot op+8, ops 0/3); LED off -> CABINA: LIVRE; descriptive name |
| 5039 | 15 |  | `simulador::CWasmScreenMT::vf7` | `simulador::CWasmScreenMT::Slot7` | `uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp` (inferred) | […/wasm/cwasmscreenmt.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp) | low | slots 7 and 19 (ICF); Refresh only |
| 7871 | 498 | ✓ | `simulador::CWasmTimerScheduler::vf0` | `simulador::CWasmTimerScheduler::CreateTimer` | `uenux2/mock/app/simulador/wasm/cwasmtimerscheduler.cpp` (inferred) | […/wasm/cwasmtimerscheduler.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmtimerscheduler.cpp) | high | ITimerScheduler slot 0; make_shared<CWasmTimer> |
| 7896 | 12 |  | `simulador::CWasmTimer::vf5` | `simulador::CWasmTimer::SetInterval` | `uenux2/mock/app/simulador/wasm/cwasmtimer.cpp` (inferred) | […/wasm/cwasmtimer.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmtimer.cpp) | high | slot 5; stores ms |
| 7902 | 10 |  | `simulador::CWasmTimer::vf4` | `simulador::CWasmTimer::IsRunning` | `uenux2/mock/app/simulador/wasm/cwasmtimer.cpp` (inferred) | […/wasm/cwasmtimer.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmtimer.cpp) | high | slot 4; returns ativo |
| 7918 | 137 | ✓ | `simulador::CWasmTimer::vf2` | `simulador::CWasmTimer::Start` | `uenux2/mock/app/simulador/wasm/cwasmtimer.cpp` (inferred) | […/wasm/cwasmtimer.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmtimer.cpp) | high | slot 2 (ITimer, gui-common.u15.h); emscripten_async_call(838,...) |
| 7926 | 13 |  | `simulador::CWasmTimer::vf1` | `simulador::CWasmTimer::~CWasmTimer [deleting]` | `uenux2/mock/app/simulador/wasm/cwasmtimer.cpp` (inferred) | […/wasm/cwasmtimer.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmtimer.cpp) | high | slot 1 |
| 7934 | 30 | ✓ | `simulador::CWasmTimer::vf3` | `simulador::CWasmTimer::Stop` | `uenux2/mock/app/simulador/wasm/cwasmtimer.cpp` (inferred) | […/wasm/cwasmtimer.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmtimer.cpp) | high | slot 3; ativo=false, ++geracao |
| 7945 | 14 |  | `simulador::CWasmSystemDateTime::vf1` | `simulador::CWasmSystemDateTime::SetDataHora` | `uenux2/mock/app/simulador/wasm/cwasmsystemdatetime.cpp` (inferred) | […/wasm/cwasmsystemdatetime.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmsystemdatetime.cpp) | medium | slot 1 (u05 name); stores offset |
| 7954 | 18 | ✓ | `simulador::CWasmSystemDateTime::vf0` | `simulador::CWasmSystemDateTime::GetDataHora` | `uenux2/mock/app/simulador/wasm/cwasmsystemdatetime.cpp` (inferred) | […/wasm/cwasmsystemdatetime.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmsystemdatetime.cpp) | medium | slot 0; offset + js_obter_data_hora_local_navegador() |
| 8316 | 40 |  | `simulador::CWasmLogd::vf1` | `simulador::CWasmLogd::~CWasmLogd [deleting]` | `uenux2/mock/app/simulador/wasm/cwasmlogd.cpp` (inferred) | […/wasm/cwasmlogd.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmlogd.cpp) | high | slot 1 |
| 8318 | 37 |  | `simulador::CWasmLogd::vf0` | `simulador::CWasmLogd::~CWasmLogd` | `uenux2/mock/app/simulador/wasm/cwasmlogd.cpp` (inferred) | […/wasm/cwasmlogd.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmlogd.cpp) | high | slot 0 |
| 8327 | 818 | ✓ | `simulador::CWasmLogd::vf4` | `simulador::CWasmLogd::Escreve` | `uenux2/mock/app/simulador/wasm/cwasmlogd.cpp` (inferred) | […/wasm/cwasmlogd.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmlogd.cpp) | medium | slot 4 (u20 name); std::format("{}\|{}\|{}") line appended to logd.dat |
| 8365 | 9 |  | `simulador::CWasmNullPrinter::vf12` | `simulador::CWasmNullPrinter::DoSetStyle` | `uenux2/mock/app/simulador/wasm/cwasmnullprinter.cpp` (inferred) | […/wasm/cwasmnullprinter.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmnullprinter.cpp) | medium | slot 12 (u17 name); stores style |
| 8371 | 52 |  | `simulador::CWasmNullPaper::vf8` | `simulador::CWasmNullPaper::ImprimeArquivo` | `uenux2/mock/app/simulador/wasm/cwasmnullpaper.cpp` (inferred) | […/wasm/cwasmnullpaper.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmnullpaper.cpp) | medium | IPaperRelatorios slot 8 (u09 name, BU vias); releases header shared_ptr |
| 8372 | 2 |  | `simulador::CWasmNullPaper::vf7` | `simulador::CWasmNullPaper::ImprimeArquivo` | `uenux2/mock/app/simulador/wasm/cwasmnullpaper.cpp` (inferred) | […/wasm/cwasmnullpaper.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmnullpaper.cpp) | medium | IPaperRelatorios slot 7 (u09 name); empty |
| 8450 | 524 |  | `simulador::CWasmNullSound::vf9` | `api::ISound::WaitAsync` | `uenux2/src/api/hwil/isound.h` (inferred) | [src/…/api/hwil/isound.h](../../src/uenux2/src/api/hwil/isound.h) | high | inline default slot 9; RTTI api::ISound::WaitAsync(std::function<bool ()>, std::function<void (bool)>)::lambda |
| 8461 | 96 |  | `simulador::CWasmNullSound::vf8` | `api::ISound::Wait` | `uenux2/src/api/hwil/isound.h` (inferred) | [src/…/api/hwil/isound.h](../../src/uenux2/src/api/hwil/isound.h) | medium | inline default slot 8; status poll + emscripten_sleep(20) |
| 8472 | 253 |  | `simulador::CWasmNullSound::vf4` | `api::ISound::Play` | `uenux2/src/api/hwil/isound.h` (inferred) | [src/…/api/hwil/isound.h](../../src/uenux2/src/api/hwil/isound.h) | medium | inline default slot 4; uses ISound::Wait()::lambda (RTTI) |
| 8479 | 124 |  | `simulador::CWasmNullSound::vf2` | `api::ISound::Play` | `uenux2/src/api/hwil/isound.h` (inferred) | [src/…/api/hwil/isound.h](../../src/uenux2/src/api/hwil/isound.h) | medium | inline default slot 2 in CWasmNullSound vtable; uses ISound::Wait()::lambda (RTTI) |
| 8487 | 52 |  | `simulador::CWasmNullSound::vf3` | `simulador::CWasmNullSound::Play` | `uenux2/mock/app/simulador/wasm/cwasmnullsound.cpp` (inferred) | […/wasm/cwasmnullsound.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmnullsound.cpp) | medium | slot 3 Play(shared_ptr<CWavFile>); only releases the param |
| 8591 | 50 |  | `simulador::CWasmResource::vf6` | `simulador::CWasmResource::IsResource` | `uenux2/mock/app/simulador/wasm/cwasmresource.cpp` (inferred) | […/wasm/cwasmresource.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmresource.cpp) | medium | slot 6 = isResource (iresource.h:97); name[0]==':' |
| 8600 | 67 |  | `simulador::CWasmResource::vf5` | `simulador::CWasmResource::Exists` | `uenux2/mock/app/simulador/wasm/cwasmresource.cpp` (inferred) | […/wasm/cwasmresource.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmresource.cpp) | low | slot 5; resolver result non-empty |
| 8605 | 1029 | ✓ | `simulador::CWasmResource::vf4` | `simulador::CWasmResource::GetMovie` | `uenux2/mock/app/simulador/wasm/cwasmresource.cpp` (inferred) | […/wasm/cwasmresource.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmresource.cpp) | high | slot 4 = getResourceMovie (iresource.h:87); js_resource_log("movie"); CMovie ctor 5526 |
| 8611 | 185 | ✓ | `simulador::CWasmResource::vf3` | `simulador::CWasmResource::GetFile` | `uenux2/mock/app/simulador/wasm/cwasmresource.cpp` (inferred) | […/wasm/cwasmresource.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmresource.cpp) | high | slot 3 = getResourceFile (iresource.h:82); js_resource_log("file") |
| 8633 | 189 |  | `simulador::CWasmResource::vf2` | `simulador::CWasmResource::GetImage` | `uenux2/mock/app/simulador/wasm/cwasmresource.cpp` (inferred) | […/wasm/cwasmresource.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmresource.cpp) | medium | slot 2; js_resource_log("image"); make_shared<CFixedImage> |
| 8680 | 12 |  | `simulador::CWasmScreenMT::ShowClock(api::SPoint const&)::WasmText::vf1` | `simulador::CWasmScreenMT::ShowClock::WasmText::~WasmText [deleting]` | `uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp` (inferred) | […/wasm/cwasmscreenmt.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp) | high | slot 1 of local class vtable @1530048 |
| 8742 | 5 |  | `simulador::CWasmScreenMT::vf18` | `simulador::CWasmScreenMT::GetHeight` | `uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp` (inferred) | […/wasm/cwasmscreenmt.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp) | medium | slot 18; returns 80 |
| 8743 | 5 |  | `simulador::CWasmScreenMT::vf17` | `simulador::CWasmScreenMT::GetWidth` | `uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp` (inferred) | […/wasm/cwasmscreenmt.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp) | medium | slot 17; returns 480 (CInfoMTLCD centring) |
| 8744 | 130 |  | `simulador::CWasmScreenMT::vf1` | `simulador::CWasmScreenMT::~CWasmScreenMT [deleting]` | `uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp` (inferred) | […/wasm/cwasmscreenmt.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp) | high | slot 1 |
| 8745 | 127 |  | `simulador::CWasmScreenMT::vf0` | `simulador::CWasmScreenMT::~CWasmScreenMT` | `uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp` (inferred) | […/wasm/cwasmscreenmt.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp) | high | slot 0; 4 strings + IScreenMT vptr + RemoveAll(3461) |
| 8746 | 1371 | ✓ | `simulador::CWasmScreenMT::vf14` | `simulador::CWasmScreenMT::Refresh` | `uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp` (inferred) | […/wasm/cwasmscreenmt.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp) | high | slot 14 (IForm::Redraw device refresh, u17); js_mt_set |
| 8748 | 12 |  | `simulador::CWasmScreenMT::ShowClock(api::SPoint const&)::WasmText::vf0` | `simulador::CWasmScreenMT::ShowClock::WasmText::~WasmText` | `uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp` (inferred) | […/wasm/cwasmscreenmt.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp) | high | slot 0 of local class vtable @1530048 |
| 8757 | 250 |  | `simulador::CWasmScreenMT::vf12` | `simulador::CWasmScreenMT::ShowClock` | `uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp` (inferred) | […/wasm/cwasmscreenmt.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp) | high | slot 12; RTTI local class CWasmScreenMT::ShowClock(api::SPoint const&)::WasmText; "hh:mm" |
| 8787 | 15 |  | `simulador::CWasmScreenMT::vf6` | `simulador::CWasmScreenMT::Bipa` | `uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp` (inferred) | […/wasm/cwasmscreenmt.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp) | low | slot 6 (u10 name Bipa(50,3)); Refresh only |
| 8799 | 499 |  | `simulador::CWasmScreenMT::vf3` | `simulador::CWasmScreenMT::Write` | `uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp` (inferred) | […/wasm/cwasmscreenmt.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp) | medium | slot 3 (u17 ctextfieldmt name); 4x40 grid writer |
| 8810 | 67 |  | `simulador::CWasmScreenMT::vf2` | `simulador::CWasmScreenMT::Clear` | `uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp` (inferred) | […/wasm/cwasmscreenmt.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp) | medium | slot 2; 4 x assign(40,' ') + Refresh |
| 8945 | 7 |  | `simulador::CWasmScreen::vf34` | `simulador::CWasmScreen::GetImageSurfaceOps` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferred) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | high | slot 34; returns this+44 (CWasmImageSurfaceOps) |
| 8953 | 7 |  | `simulador::CWasmScreen::vf31` | `simulador::CWasmScreen::GetHeight` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferred) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | high | slot 31; returns 480 |
| 8959 | 7 |  | `simulador::CWasmScreen::vf30` | `simulador::CWasmScreen::GetWidth` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferred) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | high | slot 30 (srcloc cformbuilder :292 caller); returns 640 |
| 8963 | 20 |  | `simulador::CWasmScreen::vf1` | `simulador::CWasmScreen::~CWasmScreen [deleting]` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferred) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | high | vtable slot 1; IScreen vptr + RemoveAll(5069) + free |
| 8985 | 144 |  | `simulador::CWasmImageSurfaceOps::vf17` | `simulador::CWasmImageSurfaceOps::GetImageRect` | `uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp` (inferred) | […/wasm/cwasmimagesurfaceops.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp) | medium | slot 17 (shared_ptr<IImage> by value) |
| 8991 | 89 |  | `simulador::CWasmImageSurfaceOps::vf16` | `simulador::CWasmImageSurfaceOps::GetImageRect` | `uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp` (inferred) | […/wasm/cwasmimagesurfaceops.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp) | high | slot 16 (IImage&); callers CImageField::Rect, CImageFieldUpdate::Rect (u15 name) |
| 8994 | 69 |  | `simulador::CWasmImageSurfaceOps::vf15` | `simulador::CWasmImageSurfaceOps::GetImageRect` | `uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp` (inferred) | […/wasm/cwasmimagesurfaceops.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp) | medium | slot 15 (vector) = slot12+slot14 |
| 8998 | 85 | ✓ | `simulador::CWasmImageSurfaceOps::vf13` | `simulador::CWasmImageSurfaceOps::GetImageSize` | `uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp` (inferred) | […/wasm/cwasmimagesurfaceops.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp) | high | slot 13 (IImage&) -> slot 12; caller CInfoMTLCD::Update |
| 9004 | 17 |  | `simulador::CWasmScreen::vf33` | `simulador::CWasmScreen::Clear` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferred) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | low | slot 33 (int, TColor) -> slot 4 |
| 9009 | 144 | ✓ | `simulador::CWasmScreen::vf32` | `simulador::CWasmScreen::GetTextWidth` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferred) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | high | slot 32; js_measure_text_width; returns a full int (no extend16) |
| 9013 | 16 |  | `simulador::CWasmScreen::vf29` | `simulador::CWasmScreen::SetColors` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferred) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | low | slot 29; stores +36/+40 |
| 9018 | 1621 |  | `simulador::CWasmScreen::vf28` | `simulador::CWasmScreen::RefreshRect` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferred) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | medium | slot 28; partial vota:screen JSON |
| 9022 | 134 | ✓ | `simulador::CWasmScreen::vf27` | `simulador::CWasmScreen::Refresh` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferred) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | high | slot 27; js_log("Refresh()") + event vota:screen |
| 9038 | 392 | ✓ | `simulador::CWasmScreen::vf26` | `simulador::CWasmScreen::DrawMovie` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferred) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | high | slot 26 (CMovieField::Draw); frames.at(frameAtual) |
| 9049 | 277 |  | `simulador::CWasmScreen::vf25` | `simulador::CWasmScreen::DrawImage` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferred) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | medium | slot 25 DrawImage(pos,img,SRect part) |
| 9058 | 476 |  | `simulador::CWasmScreen::vf24` | `simulador::CWasmScreen::DrawImage` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferred) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | high | slot 24 DrawImage(pos,img,size,anchor) |
| 9068 | 292 |  | `simulador::CWasmScreen::vf23` | `simulador::CWasmScreen::DrawSurface` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferred) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | low | slot 23; 64x64 grey placeholder, argument unused |
| 9079 | 435 | ✓ | `simulador::CWasmScreen::vf22` | `simulador::CWasmScreen::DrawImage` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferred) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | high | slot 22 (CImageField::Draw); js_image |
| 9090 | 46 |  | `simulador::CWasmScreen::vf21` | `simulador::CWasmScreen::WriteText` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferred) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | medium | slot 21 clip variant (clip ignored) -> slot 19 |
| 9104 | 1420 | ✓ | `simulador::CWasmScreen::vf20` | `simulador::CWasmScreen::WriteText` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferred) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | high | slot 20 WriteText(const SRect&,...); js_log("Write '") + js_text |
| 9115 | 294 | ✓ | `simulador::CWasmScreen::vf19` | `simulador::CWasmScreen::WriteText` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferred) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | high | slot 19 WriteText(const SPoint&,...) -> slot 20 |
| 9125 | 627 | ✓ | `simulador::CWasmScreen::vf18` | `simulador::CWasmScreen::DrawPath` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferred) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | high | slot 18; js_path(fill=0) |
| 9146 | 617 | ✓ | `simulador::CWasmScreen::vf17` | `simulador::CWasmScreen::FillPath` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferred) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | high | slot 17; js_path(fill=1); caller CStepsProgressBar (u16) |
| 9155 | 114 |  | `simulador::CWasmScreen::vf16` | `simulador::CWasmScreen::DrawPolygon` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferred) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | low | slot 16; closed polyline of a vector<SPoint> |
| 9165 | 65 |  | `simulador::CWasmScreen::vf15` | `simulador::CWasmScreen::DrawTriangle` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferred) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | low | slot 15; 3 x slot 8 |
| 9175 | 130 |  | `simulador::CWasmScreen::vf12` | `simulador::CWasmScreen::FillGrayGradient` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferred) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | medium | slot 12; js_gray_gradient |
| 9184 | 202 |  | `simulador::CWasmScreen::vf11` | `simulador::CWasmScreen::DrawRoundRect` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferred) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | medium | slot 11; js_round_rect |
| 9194 | 90 |  | `simulador::CWasmScreen::vf10` | `simulador::CWasmScreen::DrawCircle` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferred) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | medium | slot 10; js_circle |
| 9205 | 290 | ✓ | `simulador::CWasmScreen::vf9` | `simulador::CWasmScreen::DrawRect` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferred) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | high | slot 9; 4 x slot 8 |
| 9213 | 111 | ✓ | `simulador::CWasmScreen::vf8` | `simulador::CWasmScreen::DrawLine` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferred) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | medium | slot 8; js_line |
| 9223 | 73 |  | `simulador::CWasmScreen::vf7` | `simulador::CWasmScreen::SetPixel` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferred) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | medium | slot 7; js_fill 1x1 |
| 9232 | 181 | ✓ | `simulador::CWasmScreen::vf6` | `simulador::CWasmScreen::FillRect` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferred) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | high | slot 6; js_fill of the scaled rect |
| 9243 | 19 | ✓ | `simulador::CWasmScreen::vf5` | `simulador::CWasmScreen::ClearRect` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferred) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | medium | slot 5 -> slot 6 |
| 9254 | 39 | ✓ | `simulador::CWasmScreen::vf4` | `simulador::CWasmScreen::Clear` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferred) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | high | slot 4; js_log("Clear(full)") + js_clear |
| 9261 | 45 | ✓ | `simulador::CWasmScreen::vf3` | `simulador::CWasmScreen::GetFontMetrics` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferred) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | high | slot 3 = IScreen::GetFontMetrics (srcloc iscreen.h:69 on the base default 8938) |
| 9265 | 36 |  | `simulador::CWasmScreen::vf2` | `simulador::CWasmScreen::GetMaxCharSize` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferred) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | medium | slot 2 (u17 iscreen.h name); width=max(1,size/2), height=size |
| 9289 | 253 |  | `simulador::CWasmImageSurfaceOps::vf14` | `simulador::CWasmImageSurfaceOps::CalcRect` | `uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp` (inferred) | […/wasm/cwasmimagesurfaceops.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp) | medium | slot 14 (name from u04 ctelasvota.cpp); anchor switch |
| 9307 | 12 |  | `simulador::CWasmImageSurfaceOps::vf11` | `simulador::CWasmImageSurfaceOps::FreeBuffer` | `uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp` (inferred) | […/wasm/cwasmimagesurfaceops.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp) | low | slot 11; delete[] |
| 9317 | 58 |  | `simulador::CWasmImageSurfaceOps::vf10` | `simulador::CWasmImageSurfaceOps::CreateEmptySurface` | `uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp` (inferred) | […/wasm/cwasmimagesurfaceops.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp) | low | slot 10 -> slot 9 |
| 9322 | 55 |  | `simulador::CWasmImageSurfaceOps::vf9` | `simulador::CWasmImageSurfaceOps::CreateEmptySurface` | `uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp` (inferred) | […/wasm/cwasmimagesurfaceops.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp) | low | slot 9; make_shared<vector<uebyte>>() |
| 9328 | 51 |  | `simulador::CWasmImageSurfaceOps::vf8` | `simulador::CWasmImageSurfaceOps::CopyToBuffer` | `uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp` (inferred) | […/wasm/cwasmimagesurfaceops.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp) | low | slot 8; new[] + memcpy |
| 9333 | 7 |  | `simulador::CWasmImageSurfaceOps::vf7` | `simulador::CWasmImageSurfaceOps::Slot7` | `uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp` (inferred) | […/wasm/cwasmimagesurfaceops.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp) | low | slot 7; returns *arg |
| 9339 | 15 |  | `simulador::CWasmImageSurfaceOps::vf6` | `simulador::CWasmImageSurfaceOps::Slot6` | `uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp` (inferred) | […/wasm/cwasmimagesurfaceops.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp) | low | slot 6; returns arg->vtable[3]() |
| 9348 | 105 |  | `simulador::CWasmImageSurfaceOps::vf5` | `simulador::CWasmImageSurfaceOps::CreateSurface` | `uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp` (inferred) | […/wasm/cwasmimagesurfaceops.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp) | low | slot 5; like slot 2 |
| 9356 | 7 |  | `simulador::CWasmImageSurfaceOps::vf4` | `simulador::CWasmImageSurfaceOps::Slot4` | `uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp` (inferred) | […/wasm/cwasmimagesurfaceops.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp) | low | slot 4; returns *arg |
| 9362 | 15 |  | `simulador::CWasmImageSurfaceOps::vf3` | `simulador::CWasmImageSurfaceOps::Slot3` | `uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp` (inferred) | […/wasm/cwasmimagesurfaceops.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp) | low | slot 3; returns arg->vtable[3]() |
| 9365 | 105 |  | `simulador::CWasmImageSurfaceOps::vf2` | `simulador::CWasmImageSurfaceOps::CreateSurface` | `uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp` (inferred) | […/wasm/cwasmimagesurfaceops.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp) | low | slot 2; make_shared<vector<uebyte>>(img.GetImage()) |
| 9388 | 10 |  | `simulador::CWasmInit::vf1` | `simulador::CWasmInit::~CWasmInit [deleting]` | `uenux2/mock/app/simulador/wasm/cwasminit.cpp` (inferred) | […/wasm/cwasminit.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasminit.cpp) | high | slot 1; IInterfaceInit dtor 5898 + free |
| 9405 | 706 | ✓ | `simulador::CWasmInit::vf5` | `simulador::CWasmInit::EnviarMensagem` | `uenux2/mock/app/simulador/wasm/cwasminit.cpp` (inferred) | […/wasm/cwasminit.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasminit.cpp) | high | slot 5 (u23 name); command table, log "CWasmInit::{} {}" |
| 9418 | 7 |  | `simulador::CWasmInit::vf4` | `simulador::CWasmInit::Slot4` | `uenux2/mock/app/simulador/wasm/cwasminit.cpp` (inferred) | […/wasm/cwasminit.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasminit.cpp) | low | slot 4; returns +24 (10) |
| 9427 | 58 | ✓ | `simulador::CWasmNullTextToSpeech::vf10` | `simulador::CWasmNullTextToSpeech::~CWasmNullTextToSpeech [deleting]` | `uenux2/mock/app/simulador/wasm/cwasmnulltexttospeech.cpp` (inferred) | […/wasm/cwasmnulltexttospeech.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmnulltexttospeech.cpp) | high | slot 10; ~ITextToSpeech inlined + free |
| 9436 | 9 |  | `simulador::CWasmNullTextToSpeech::vf11` | `simulador::CWasmNullTextToSpeech::Sintetiza` | `uenux2/mock/app/simulador/wasm/cwasmnulltexttospeech.cpp` (inferred) | […/wasm/cwasmnulltexttospeech.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmnulltexttospeech.cpp) | medium | slot 11 (u02 name); returns empty shared_ptr |
| 9515 | 995 |  | `simulador::CWasmWebSound::vf9` | `simulador::CWasmWebSound::WaitAsync` | `uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp` (inferred) | […/wasm/cwasmwebsound.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp) | high | slot 9 (name from the base lambda RTTI); js_wasm_web_sound_wait_async |
| 9523 | 38 | ✓ | `simulador::CWasmWebSound::vf8` | `simulador::CWasmWebSound::Wait` | `uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp` (inferred) | […/wasm/cwasmwebsound.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp) | medium | slot 8 (ISound::Wait lambda caller); non-blocking |
| 9533 | 4 |  | `simulador::CWasmWebSound::vf7` | `simulador::CWasmWebSound::GetStatus` | `uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp` (inferred) | […/wasm/cwasmwebsound.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp) | high | slot 7 (u08 name); js_wasm_web_sound_get_status |
| 9541 | 13 | ✓ | `simulador::CWasmWebSound::vf6` | `simulador::CWasmWebSound::Mute` | `uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp` (inferred) | […/wasm/cwasmwebsound.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp) | medium | slot 6; js_wasm_web_sound_mute |
| 9550 | 11 |  | `simulador::CWasmWebSound::vf5` | `simulador::CWasmWebSound::Stop` | `uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp` (inferred) | […/wasm/cwasmwebsound.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp) | high | slot 5 (u08 name); js_wasm_web_sound_stop |
| 9559 | 157 |  | `simulador::CWasmWebSound::vf4` | `simulador::CWasmWebSound::Play` | `uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp` (inferred) | […/wasm/cwasmwebsound.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp) | high | slot 4 Play(audio, enfileira) (u08 name); js_wasm_web_sound_play_wav |
| 9565 | 211 |  | `simulador::CWasmWebSound::vf3` | `simulador::CWasmWebSound::Play` | `uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp` (inferred) | […/wasm/cwasmwebsound.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp) | medium | slot 3 Play(audio) = slot 4 with false |
| 9573 | 18 |  | `simulador::CWasmWebSound::vf2` | `simulador::CWasmWebSound::Play` | `uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp` (inferred) | […/wasm/cwasmwebsound.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp) | medium | slot 2 Play(arquivo, enfileira): unsupported |
| 9580 | 18 |  | `simulador::CWasmWebSound::vf1` | `simulador::CWasmWebSound::Play` | `uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp` (inferred) | […/wasm/cwasmwebsound.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp) | medium | slot 1 Play(arquivo): unsupported |
| 9589 | 11 |  | `simulador::CWasmWebSound::vf0` | `simulador::CWasmWebSound::Pause` | `uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp` (inferred) | […/wasm/cwasmwebsound.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp) | medium | slot 0; js_wasm_web_sound_pause |
| 9651 | 18 |  | `simulador::CWasmThread::Yield` | `simulador::CWasmThread::Yield` | `uenux2/mock/app/simulador/wasm/cwasmthread.cpp (attested)` | […/wasm/cwasmthread.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmthread.cpp) | high | slot 5; logs "Yield"; u07/u10 callers |
| 9652 | 171 |  | `simulador::CWasmThread::vf3` | `simulador::CWasmThread::Wait` | `uenux2/mock/app/simulador/wasm/cwasmthread.cpp (attested)` | […/wasm/cwasmthread.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmthread.cpp) | medium | slot 3; logs "Wait - polling"/"Wait - finished" |
| 9655 | 670 |  | `simulador::CWasmThread::Create` | `simulador::CWasmThread::Create` | `uenux2/mock/app/simulador/wasm/cwasmthread.cpp (attested)` | […/wasm/cwasmthread.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmthread.cpp) | high | srcloc cwasmthread.cpp:31 |
| 10880 | 15 |  | `simulador::CWasmNullPrinter::vf5` | `simulador::CWasmNullPrinter::GetColumns` | `uenux2/mock/app/simulador/wasm/cwasmnullprinter.cpp` (inferred) | […/wasm/cwasmnullprinter.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmnullprinter.cpp) | medium | slot 5 (u17 name); 19 or 38 |
| 13877 | 148 |  | `simulador::CWasmNullTextToSpeech::CWasmNullTextToSpeech` | `simulador::CWasmNullTextToSpeech::CWasmNullTextToSpeech` | `uenux2/mock/app/simulador/wasm/cwasmnulltexttospeech.cpp` (inferred) | […/wasm/cwasmnulltexttospeech.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmnulltexttospeech.cpp) | high | vtable store; called by votaInit |

## 15. Open questions

* The names of `IImageSurfaceOps` slots 2–11, and the type of the polymorphic argument of slots 3/6/10. No caller
  exists in this binary.
* The purpose of `IScreen` slots 13/14/23/33/35–38 and `IScreenMT` slots 5/7/13/15/16/19. The web bodies are no-ops
  or placeholders.
* The meaning of `CWasmInit` commands 6 (`FE_NOT_PRESENT`: external flash?), 15/16, 39 (`512000`) and 68, and of
  `IInterfaceInit` slots 3/4/6/7.
* The two ignored `int` parameters of `IResource::GetImage` (slot 2), and the fields +4 (`true`) and +8 (0) that the
  `CWasmScreen` constructor writes.
* Whether anything was meant to read the log bus of func 5152 (u29). Nothing in this binary subscribes to it or reads
  its history.
