# u31: os mocks de hardware do simulador (`uenux2/mock/app/simulador/wasm`)

A unidade u31 tem **107 funções wasm**, todas nas classes `simulador::CWasm*` da camada do simulador. Na
urna, a aplicação de votação chega ao seu hardware (tela, terminal do mesário, impressora, som, relógio,
timers, daemon de log, serviço "init", threads) através de interfaces abstratas registradas em
`api::CPolySingletonList` (u19). A build web do TSE registra **estas classes** no lugar delas, e cada uma termina
ou em um import JavaScript da página (`js_*`), ou no MEMFS, ou em nada.

27 das 107 funções foram executadas durante os votos gravados (`analysis/runtime/*.functions.tsv`): o caminho de desenho
da tela do eleitor, os timers, o relógio, o gravador de log, o carregador de recursos, `CWasmInit::EnviarMensagem`,
e o mudo/espera do dispositivo de fala.

Apenas um caminho original é atestado, pelo registro `std::source_location` de `CWasmThread::Create`:
`/home/rubio/tse/uenux2/mock/app/simulador/wasm/cwasmthread.cpp:31`. Os outros arquivos recebem o nome das suas
classes, seguindo a convenção do TSE (classe `CFooBar` em `cfoobar.cpp`), no mesmo diretório (**caminho
inferido**).

Fontes reconstruídos (todos escritos por esta unidade):

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

## 1. Finalidade e os termos em português

* **urna / UE (urna eletrônica)**: o equipamento de votação. **eleitor**: quem vota. **mesário**: membro da mesa que opera o terminal do mesário.
* **tela do eleitor**: o display do eleitor, 640 × 480 na urna (`api::IScreen`).
* **MT (microterminal)**: o pequeno teclado do mesário, com um LCD de 4 × 40 caracteres e um LED (`api::IScreenMT`).
  **cabina livre / ocupada**: a cabine de votação está livre / ocupada (um eleitor está votando). A página web mostra o MT
  como uma caixa de texto; o LED vira a linha `CABINA: LIVRE` / `CABINA: OCUPADA`.
* **impressora / papel de relatórios**: a impressora térmica que imprime a **zerésima** (o relatório impresso
  antes do início da votação, que prova que a urna não contém votos), o **boletim de urna (BU)** (o resultado
  assinado de cada máquina, impresso em várias **vias**), o BJE (relatório de justificativas) e outros.
* **logd**: o daemon de log da urna; toda aplicação grava nele os seus eventos, e o log da urna
  é publicado após a eleição. **aplicação / severidade**: o código da aplicação e o nível de um registro.
* **MR (mídia de resultado)**: o cartão de memória USB que recebe os arquivos de resultado no fim do dia.
* **voto com áudio / acessibilidade**: o voto acessível, em que toda tela e toda tecla são faladas (síntese de fala
  RHVoice, `docs/libraries/rhvoice.md`).
* **treinamento**: modo de treinamento. Todo cenário do simulador público é uma eleição de treinamento.

No processo de votação, esta camada é **toda a E/S da urna web**. Quando o eleitor pressiona uma tecla, a tecla vem
de `CWasmInputKbd` (u30). A tela que responde é desenhada por `CWasmScreen`, o bipe de confirmação vem de
`CWasmBeep` (u30), a mensagem falada de `CWasmWebSound`, e o registro "Voto confirmado para [Vereador]" de
`CWasmLogd`. O relógio da barra de status é atualizado por meio de um `CWasmTimer` e lê `CWasmSystemDateTime`.

## 2. Como os objetos entram no programa

O código de registro está em outras unidades: `CSimuladorWasm::Executa` (func 8302, u19 §6.1), a lambda de `main`
(func 10384) e `votaInit` (func 7840, u29). A tabela abaixo é a parte da u19 §2.3 que diz respeito a esta unidade:

| interface | implementação web (esta unidade) | registrada por | substitui |
|---|---|---|---|
| `api::ITimerScheduler` | `CWasmTimerScheduler` | 8302, `ITimerScheduler::CreateInst<>` (itimerscheduler.h:40) | – (impede o `api::CTimerScheduler` da urna, cujo `CTimer` precisa de `std::thread`) |
| `comum::IInterfaceInit` | `CWasmInit(10)` | 8302 | – |
| `api::IScreen` | `CWasmScreen(w, h)` | 8302 | – |
| `api::IScreenMT` | `CWasmScreenMT` | 8302 | – |
| `api::IResource` | `CWasmResource` | 8302 | – |
| `api::ISound` | `CWasmNullSound`, depois **`CWasmWebSound`** | 8302, depois a lambda de `main` 10384 | o dispositivo nulo |
| `api::IPaperRelatorios` | `CWasmNullPaper` | 8302 | – |
| `api::IImpressoraRelatorios` | `CWasmNullPrinter` | 8302 | – |
| `api::ITextToSpeech` | `CWasmNullTextToSpeech` (ou `api::CRHVoiceTextToSpeech`) | 8302 e novamente `votaInit` | – |
| `api::ISystemDateTime` | `CWasmSystemDateTime` | 8302 (push 4890) | `api::CSystemDateTime` (`time(0)`) |
| `api::CEscritorLog` | `CWasmLogd("/dsk/fi/dinamico/log/logd.dat")` | 8302 | – |
| `IGenericFactory<api::IThreadImpl>` | `CDefaultGenericFactory<IThreadImpl, CWasmThread>` | `main` | – |

Os objetos são criados uma vez, quando a página carrega, e vivem até a página ser fechada. A página é recarregada
entre eleitores, então nada aqui persiste (exceto o MEMFS, que também é reconstruído).

## 3. Hierarquia de classes (RTTI)

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

## 4. A tela do eleitor: `CWasmScreen`

### 4.1 Construção e escala

O construtor está inlinado na func 8302. Ele armazena `m_escalaX = w / 640.0` (+16) e `m_escalaY = h / 480.0`
(+24), o tamanho lógico 640/480 (+32/+34, retornado por `GetWidth`/`GetHeight`) e as cores padrão 2/0 (+36/+40,
gravadas por `SetColors`, nunca lidas). Em seguida, chama `js_init(w, h)` e `Clear(1)` (branco), e registra
`"CWasmScreen this=<address>"`. `w` e `h` vêm de `js_ler_dimensao_tela`: os parâmetros de URL `screenWidth`,
`screenHeight`, `screen=WxH` (ou `resolution`/`resolucao`); caso contrário, `Module.votaScreenWidth/Height`; caso contrário,
1280 × 800. Eles são armazenados como `short` pelo chamador.

Toda coordenada atravessa como `ceil(scale × v)`, calculada com o `i32.trunc_sat_f64_s` saturante:

| grandeza | escala |
|---|---|
| x, larguras de retângulos, larguras de linhas, raio de círculos, x/w de paths | `m_escalaX` |
| y, alturas, **tamanho da fonte**, y/h de paths | `m_escalaY` |
| **largura e altura naturais de imagens e animações** (slots 22, 26, e 24 quando não há tamanho explícito) | `m_escalaY` para ambas, de modo que a imagem mantém a sua proporção |
| tamanho explícito de imagem (slot 24) e parte de imagem (slot 25) | `m_escalaX` para a largura, `m_escalaY` para a altura |
| largura de texto retornada à aplicação | `ceil(px / m_escalaX) + 1`, retornada como um `int` completo (sem `extend16`) |

Em 1280 × 800, `sx = 2` e `sy = 1.667`. Uma fonte de 40 px vira 67 px, e uma foto de candidato de 161 × 225 vira
269 × 375.

As cores são índices (`api::TColor`) numa tabela de 38 entradas de strings CSS em @1529596 (listada em
`cwasmscreen.cpp`). Um índice acima de 37, comparado sem sinal, dá `"#000000"`. O compilador transformou o `switch`
do fonte em duas tabelas de consulta. A segunda (@1529448) começa no índice 1 e atende o ponto de chamada que
já testou `cor != 0` (o fundo do texto).

### 4.2 Mapa de slots

| slot | func | método | o que chega ao JavaScript |
|---:|---:|---|---|
| 0 / 1 | 5071 / 8963 | `~CWasmScreen` | `IForm<IScreen>::RemoveAll` (5069) |
| 2 | 9265 | `GetMaxCharSize(w&, h&, font)` | – (`w = max(1, size/2)`, `h = size`) |
| 3 | 9261 | `GetFontMetrics(w&, h&, font, text)` | via slot 32 |
| 4 | 9254 | `Clear(color)` | `js_log("Clear(full)")`, `js_clear` (também remove as sobreposições de GIF) |
| 5 | 9243 | `ClearRect(rect, color)` | → slot 6 |
| 6 | 9232 | `FillRect(rect, color)` | `js_fill` |
| 7 | 9223 | `SetPixel(p, color)` | `js_fill(x, y, 1, 1)` |
| 8 | 9213 | `DrawLine(p1, p2, color, width)` | `js_line` |
| 9 | 9205 | `DrawRect(rect, color, width)` | 4 × slot 8 |
| 10 | 9194 | `DrawCircle(c, r, color, width)` | `js_circle` |
| 11 | 9184 | `DrawRoundRect(rect, r, color, width)` | `js_round_rect` |
| 12 | 9175 | `FillGrayGradient(rect)` | `js_gray_gradient` |
| 13 | 218 | no-op **herdado** de `api::IScreen` (mesma entrada de tabela 558 nas duas vtables) | – |
| 14 | 218 | override no-op (entrada de tabela própria 559; puro em `IScreen`) | – |
| 15 | 9165 | `DrawTriangle(a, b, c, color, width)` | 3 × slot 8 |
| 16 | 9155 | `DrawPolygon(vector<SPoint>, color, width)` | n × slot 8 (fechado) |
| 17 / 18 | 9146 / 9125 | `FillPath` / `DrawPath` | `js_path(n, int*, double*, css, fill, width)` (docs/03 §8.3) |
| 19 | 9115 | `WriteText(SPoint, IText, font, fg, bg) → SRect` | → slot 20 |
| 20 | 9104 | `WriteText(SRect, IText, font, fg, bg) → SRect` | `js_fill` (fundo se bg ≠ 0), `js_log("Write '…'")`, `js_text` |
| 21 | 9090 | `WriteText(SPoint, SRect clip, …)` | → slot 19; **o clip é ignorado** |
| 22 | 9079 | `DrawImage(pos, IImage, anchor)` | `js_image` |
| 23 | 9068 | `DrawSurface(pos, ?, anchor)` (?) | um placeholder cinza de 64 × 64 (`FillRect` 5 + `DrawRect` 3) |
| 24 | 9058 | `DrawImage(pos, IImage, size, anchor)` | `js_image` |
| 25 | 9049 | `DrawImage(pos, IImage, SRect part)` | `js_image`; a imagem inteira é espremida no tamanho da parte |
| 26 | 9038 | `DrawMovie(pos, CMovie, anchor)` | `js_image` de `frames.at(frameAtual)` |
| 27 | 9022 | `Refresh()` | `js_log("Refresh()")`, evento `vota:screen {"refreshed":true}` |
| 28 | 9018 | `RefreshRect(rect)` | evento `vota:screen {"refreshed":true,"partial":true,"rect":{…}}` |
| 29 | 9013 | `SetColors(fg, bg)` | – |
| 30 / 31 | 8959 / 8953 | `GetWidth` / `GetHeight` | – (640 / 480) |
| 32 | 9009 | `int GetTextWidth(text, font)` | `js_measure_text_width` |
| 33 | 9004 | `Clear(int, color)` (?) | → slot 4 |
| 34 | 8945 | `GetImageSurfaceOps()` | – (`this + 44`) |
| 35, 36 | 218 | overrides no-op (entradas de tabela 580/581; puros em `IScreen`) | – |
| 37, 38 | 218 | no-ops **herdados** de `api::IScreen` (entradas 582/583 nas duas vtables) | – |

### 4.3 Texto (`WriteText`, func 9104), passo a passo

1. `utf8 = Latin1ParaUtf8(text.GetText())` (func 5098). As strings da aplicação são ISO-8859-1, e o glue
   lê UTF-8.
2. `GetFontMetrics` (slot 3 → slot 32, uma ida e volta até `canvas.measureText`) dá a largura lógica. A altura é
   o tamanho da fonte, e ambas são no mínimo 1.
3. A caixa lógica começa na esquerda/topo do retângulo. Ela é movida para `right − w + 1` em `Right` (1), ou centralizada
   em `Center` (2). Essa caixa é o valor de retorno: os campos a guardam para apagar o texto depois.
4. A caixa é escalada. Se `bg ≠ 0` (0 é transparente), a caixa é preenchida com a cor de fundo.
5. O alinhamento do canvas é 0/1/2 = esquerda/centro/direita, com `x` na borda esquerda, no meio ou na borda direita da caixa.
6. `js_log("Write '<first 40 chars>'")`. A string é construída mesmo que o glue só a imprima quando
   `Module.uenuxDebug` está definido.
7. `js_text(x, baseline, boxWidth, utf8, ceil(sy·size), bold = style&1, italic = style>>1 &1, css(fg), align)`,
   com `baseline = bottom − height/4 + ceil(2·sy)`. O glue ignora `boxWidth`.

`GetText()` é chamado três vezes por texto (passos 1, 2 e 6), e cada chamada copia a string.

### 4.4 Imagens e animações

`DrawImage` (slot 22) copia os bytes codificados de `IImage::GetImage()`. Mede-os com
`CWasmImageSurfaceOps::GetImageSize` (func 3493), escala o tamanho por `sy`, aplica a âncora e passa os bytes para
`js_image`. O glue os copia novamente (`HEAPU8.slice`), detecta o tipo (PNG/JPEG/GIF) e os desenha. Os GIFs são
desenhados como uma sobreposição `<img>` para que o navegador os anime.

A regra de âncora é compartilhada por todas as primitivas ancoradas (`EAnchorPoint`, nomes inferidos):

| valor | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
|---|---|---|---|---|---|---|---|---|---|
| o ponto é, na imagem, o | canto superior esquerdo | canto superior direito | centro superior | meio à esquerda | meio à direita | centro | canto inferior esquerdo | canto inferior direito | centro inferior |

**O parser de tamanho (func 3493, também inlinado em `CWasmResource::GetMovie`)**

| formato | regra | resultado |
|---|---|---|
| GIF (`GIF`, ≥ 10 B) | LE16 em 6 / 8 | largura, altura |
| PNG (assinatura, ≥ 24 B) | BE16 em 18 / 22 (metade baixa dos campos do IHDR) | largura, altura |
| JPEG (`FF D8`, ≥ 12 B) | percorre os segmentos até o primeiro SOFn (C0–C3, C5–C7, C9–CB, CD–CF), para em EOI/SOS ou num comprimento inválido | largura, altura |
| qualquer outro, inclusive **BMP** | – | **1 × 1** |

Todo QR code que a urna mostra na tela é um **BMP** de 8 bits construído por `api::CQRCodeImage` (u16). Portanto, na build web,
todo QR code é desenhado como **1 × 1 pixel lógico (2 × 2 px)**. Isso é verificado no §11.

Na build web, as animações (`DrawMovie`) têm um único "frame" que contém o GIF inteiro (§8).

`CWasmImageSurfaceOps` (18 slots) contém:

* o parser de tamanho, como slot 12 (bytes) e slot 13 (`IImage`, usado por `comum::CInfoMTLCD::Update` para centralizar uma imagem
  no MT);
* `CalcRect(w, h, pos, anchor)` (slot 14; as suas bordas distantes são `pos + size`, não `pos + size − 1`);
* `GetImageRect` para bytes, para `IImage&` e para `shared_ptr<IImage>` (slots 15–17; `CImageField::Rect`,
  `CImageFieldUpdate::Rect`).

Os slots 2–11 gerenciam "superfícies", que aqui são apenas cópias `shared_ptr<vector<uebyte>>` do arquivo codificado, além de buffers
`new[]`/`delete[]` crus. Nada neste binário os chama, e um tipo de argumento (um objeto cujo slot virtual 3
retorna um int) é desconhecido. Os seus nomes estão marcados com `?`.

### 4.5 Eventos de refresh

`IForm::Redraw` termina com `IScreen::Refresh()` (slot 27). A versão web posta `vota:screen` para a página, onde o
adaptador mostra "Tela atualizada.". A forma parcial (slot 28) não ocorreu nas sessões
gravadas.

---

## 5. O terminal do mesário: `CWasmScreenMT`

Estado: quatro linhas `std::string` de exatamente 40 caracteres (+4 … +40) e `bool m_cabinaOcupada` (+52).

* `Refresh` (slot 14, func 8746) publica
  `line1\nline2\nline3\nline4\nCABINA: LIVRE|OCUPADA` com `js_mt_set(text, ocupada)`. A página põe o texto em
  `<pre id="mt">` e deixa a sua borda verde (#34a853) quando a cabine está ocupada.
* `Write(pos, text)` (slot 3, func 8799) trata apenas as linhas `pos.y` = 1..4 (a partir de 1). A coluna é:
  * o centro, para textos `Center` de no máximo 39 caracteres;
  * `40 − len` para `Right`;
  * caso contrário, `max(pos.x − 1, 0)`.

  O texto é cortado em 40 caracteres e gravado no lugar. Nada fora da grade de 4 × 40 é jamais gravado.
* Operações do LED (`api::CLedFieldMT`, ops 0..3 = slots 8..11): as ops 0 e 3 apagam o LED (func 5038) e as ops 1
  e 2 o acendem (func 5036); depois o MT é repintado.
* `ShowClock(pos)` (slot 12, func 8757; o nome vem do RTTI da sua classe local `WasmText`) escreve
  `CDateTime::Now().GetTime().Format("hh:mm")` com um `IText` local alinhado à esquerda.
* `Bipa(freq, n)` (slot 6), o slot 7 e o slot 19 apenas repintam: o MT não tem buzzer na página. O slot 19 é chamado com
  1 por `CPedeIdentidade` e com 0 por `CEscolheOpcao`, e só em urnas de modelo ≥ 2020.
* `GetWidth`/`GetHeight` retornam 480 × 80, o tamanho do LCD do MT em pixels. O slot 4 (desenhar uma imagem no LCD) é um
  no-op.

Na página, nada aciona a thread do operador (u29, `docs/bu/codepath.md` §6). O MT, portanto, mostra apenas o que
a inicialização escreveu: quatro linhas em branco e `CABINA: LIVRE`. Verificado: uma cópia de `tools/run/headless.mjs` que registra
`js_mt_set` grava exatamente uma chamada para um voto municipal inteiro (`--keys "91001C  C  12C  C  "`). A cabine nunca
aparece como ocupada, nem mesmo enquanto o eleitor está votando.

---

## 6. Tempo: timers e o relógio

### 6.1 `CWasmTimer` / `CWasmTimerScheduler`

O `api::CTimer` da urna executa uma `std::thread`, que lança "thread constructor failed" nesta build. O timer web
mantém o seu estado num `State {bool ativo; uint64 geracao; int intervalo; std::function<void()> callback}` compartilhado
e se rearma através do JavaScript:

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

O contador de geração torna seguro parar ou reiniciar enquanto callbacks já estão enfileirados no JavaScript. O callback
é executado **fora de `votaTick`**, a partir do event loop do navegador, então não fica dentro do `try/catch` das funções
exportadas (§13).

### 6.2 `CWasmSystemDateTime`

`GetDataHora()` = `m_deslocamento + trunc(js_obter_data_hora_local_navegador())`. O import retorna
`Date.getTime()/1000 − getTimezoneOffset()·60`: a hora local de relógio de parede codificada como uma época UTC.
`api::CDateTime::ConvertFromLocalTime` (5476) a decodifica com `gmtime_r`, então a urna mostra a hora local do
navegador. `SetDataHora(t)` (alcançado a partir de `api::CAjusteDataHora`, por exemplo quando o ciclo de votação começa) apenas armazena
`t − browserNow` como deslocamento. Nenhum relógio é alterado. O layout de `ISystemDateTime` põe os seus dois métodos antes do
destrutor virtual (slots 0/1, destrutor 2/3).

---

## 7. Som

### 7.1 `api::ISound` e os seus defaults inline (`src/uenux2/src/api/hwil/isound.h`)

`ISound` não tem vtable própria, mas define quatro defaults virtuais inline. O código deles foi emitido junto com
`CWasmNullSound`, a única classe que não os sobrescreve, e por isso as ferramentas os nomearam `CWasmNullSound::vf2/4/8/9`.
O RTTI das suas lambdas prova a quem pertencem: `api::ISound::Wait()::'lambda'()` e
`api::ISound::WaitAsync(std::function<bool ()>, std::function<void (bool)>)::'lambda'()`. O Clang escreve as lambdas de
funções inline como `'lambda'`, e as de funções comuns como `$_N`.

| slot | default (func) | comportamento |
|---:|---|---|
| 2 | `Play(arquivo, enfileira)` (8479) | `if (enfileira) Wait(); Play(arquivo);` |
| 4 | `Play(audio, enfileira)` (8472) | `if (enfileira) Wait(); Play(audio);` |
| 8 | `Wait(interromper)` (8461) | `while (GetStatus()==Tocando) { if (interromper && interromper()) return false; sleep_for(20ms); } return true;`. Nesta build, `sleep_for` é `emscripten_sleep(20)`, que **aborta** |
| 9 | `WaitAsync(interromper, aoTerminar)` (8450) | síncrono: `espera = make_shared<CEsperaAudio>(); ok = Wait([espera, interromper]{…}); if (!espera->Cancelada() && aoTerminar) aoTerminar(ok); return espera;` |

`m_volume` (+4, 0..10, começando em 9) é um membro público. `vota::CInstrucaoVotacaoAcessibilidade` o altera
diretamente com as teclas 3/9 (u08).

### 7.2 `CWasmNullSound`

`CWasmNullSound` é silencioso, e o seu `GetStatus()` sempre retorna `Parado`, então o laço de consulta acima nunca
é executado. A lambda de `main` o substitui por `CWasmWebSound` antes que qualquer voto comece.

### 7.3 `CWasmWebSound`

Estado: `m_status` (+8, gravado mas nunca lido, porque `GetStatus` pergunta ao JavaScript) e `m_mudo` (+12).

* `Play(audio, enfileira)` (9559): quando não está mudo e o WAV contém amostras, chama
  `js_wasm_web_sound_play_wav(header, 44, samples, size, m_volume·5 + 50, enfileira)`. O lado JS enfileira o som
  (`enfileira` = 1) ou para o som atual e limpa a fila antes (0).
* `Play(arquivo)` (slots 1/2): arquivos de som não são suportados, então o clique de tecla `":/resource/sounds/tecE.wav"` nunca é
  ouvido.
* `Wait(interromper)` (9523) **não espera**. Ele apenas responde se o som terminou.
* `WaitAsync(interromper, aoTerminar)` (9515) tem três casos:
  1. Se `interromper()` já é verdadeiro, retorna um `CEsperaAudio` já cancelado e não chama
     `aoTerminar`.
  2. Se nada está tocando, chama `aoTerminar(true)` de forma síncrona e retorna um `CEsperaAudio` novo.
  3. Caso contrário, pega `id = s_proximoIdEspera++` (@1528464, começando em 1) e armazena as duas funções em
     `std::map<int, SEspera>` (@1832664). Em seguida, chama `js_wasm_web_sound_wait_async(id)`: o JS consulta a cada 20 ms,
     pergunta ao export 9614 "cancelamento solicitado?" e por fim chama o export 9604 "terminou(id, ok)". A função retorna
     `make_shared<CEsperaAudioWasm>(id)`, cujo `Cancela()` (u30) apaga a entrada e chama
     `js_wasm_web_sound_cancel_wait`.
* `Mute`, `Stop`, `Pause` e `GetStatus` mapeiam um a um para os seus imports.

### 7.4 `CWasmNullTextToSpeech`

`CWasmNullTextToSpeech` é o `ITextToSpeech` usado quando o áudio do eleitor está desligado. O construtor (13877) executa apenas
o construtor da base: uma capacidade de LRU de 256, nível de velocidade 2 (100 %) e campos 50/100. `Sintetiza` (slot 11,
func 9436) retorna um ponteiro de WAV vazio, que `CWasmWebSound::Play` ignora.

---

## 8. Recursos: `CWasmResource`

Os nomes de recursos seguem o estilo do Qt: `":/resource/images/…"`, `":/resource/gifs/…"`, `":/resource/sounds/…"`.

* `IsResource(nome)` (slot 6) retorna `nome[0] == ':'`.
* `Exists(nome)` (slot 5) é verdadeiro quando o resolvedor encontra um arquivo.

  A u17 chama o slot 6 de `Exists`. O helper que chama o slot 6 é `api::isResource` (iresource.h:97), e o corpo
  testa o prefixo, então esta unidade nomeia o slot 6 como `IsResource` e o slot 5 como `Exists`.
* `GetFile` (slot 3, "file"), `GetImage` (slot 2, "image", que embrulha os bytes num `CFixedImage`) e
  `GetMovie` (slot 4, "movie") todos:
  1. resolvem o nome com a func 2626 (u29), que remove o `:` e mapeia `/resource/images/` para
     `/uenux/app/img/` ou `/pkg/img/`, e `/resource/gifs/` para `/pkg/gifs/`, tentando cada candidato;
  2. leem o arquivo inteiro (func 3452);
  3. chamam `js_resource_log(action, name, path, size)` (impresso apenas durante a depuração).

  Um recurso ausente dá zero bytes, e a tela o pula silenciosamente.
* `GetMovie` constrói **um** `CMovieFrame{whole GIF, 80 ms}` e `CMovie(frames, size)` (func 5526, que recebe o
  vector por valor e o move). O vector de frames é construído a partir de uma `initializer_list`, então o GIF inteiro é copiado
  mais uma vez (`operator new` + `memcpy` em 8605) antes de o frame temporário ser liberado. O tamanho vem do cabeçalho do
  GIF. O navegador faz a animação.

## 9. Log: `CWasmLogd`

O construtor (inlinado em 8302):

```
create_directories("/dsk/fi/dinamico/log"); fopen(".../logd.dat", "wb"); fclose   // truncated at each page load
```

`Escreve(aplicativo, severidade, mensagem)` (slot 4, func 8327; `CEscritorLog::loga` verifica se o código da aplicação
é < 61 antes de chamá-lo):

```
linha = std::format("{}|{}|{}", aplicativo, severidade, mensagem)      // e.g. "1|1|Voto confirmado para [Vereador]"
lock; f = fopen(arquivo, "ab"); fwrite(linha); fputc('\n'); fflush; fclose; unlock   // errors ignored
CWasmLogBus::GetInst().Publica(LOGD, linha)                           // func 5152 (u29 "log bus", channel 2): "[date] linha"
                                                                      // kept in a deque (max 2000 lines), no subscriber
```

O arquivo é texto puro em Latin-1. Ele não tem nada da estrutura do log da urna (encadeamento de hashes, assinaturas): o
`logd` real não está nesta build. Após um voto completo, o log é o único arquivo do MEMFS que muda
(`analysis/runtime/README.md`). O texto registra eventos ("Voto confirmado para [Vereador]"), nunca o conteúdo dos votos.

## 10. Stubs de hardware

### 10.1 `CWasmInit`: o serviço "init" (func 9405)

Todo comando é publicado no canal 0 do barramento de log em memória do simulador como `CWasmInit::<cmd> <text>` (func 1524,
`LogComando`, u29).

| comando (nomes da u23) | resposta `{status}` | texto de log |
|---|---:|---|
| 6 (?) | 1 | `= FE_NOT_PRESENT` |
| 7 | 0 | – |
| 10 habilita MR, 11, 34 monta MR, 36, 47, 48 desliga, 58 desmonta MR, 68 | 0 (OK) | – |
| 15, 16 | 1 | `= no` |
| 17 MR montada?, 18 MR presente? | 1 (= falso) | `= no` |
| 39 (?) | 512000 | `= 512000` |
| 67 modo demonstração? | 0 (não) | `= 0` |
| qualquer outro, inclusive 12 (índice do dispositivo da MR → 0) e 37 (dispositivo de boot → 0) | 0 | `= default` |

Os outros slots retornam valores fixos: dispositivo de boot 1 (slot 2), `true` (slot 3), 10 (slot 4, o argumento do construtor)
e `false` (slot 6).

### 10.2 `CWasmNullPrinter`, `CWasmNullPaper`

`CWasmNullPrinter` apenas lembra o estilo (`DoSetStyle`) e reporta 19 colunas para o estilo 3 (largura dupla), 38
nos demais casos. Os seus métodos de sensor de papel são os defaults de `IImpressora`, que lançam "Not supported".

`CWasmNullPaper` descarta todos os relatórios, tanto quando são **compostos** quanto quando são **impressos**:

* slot 5 `Abre(arquivo)`, slot 2 `Print(IText, style)`, slots 4/3/12 (avanço de linha, corte, imagem de QR: os papéis registrados por
  `tools/bu/operator_harness.mjs`) e slot 6 `Fecha`. É assim que a aplicação compõe um relatório num arquivo
  como `<trab>/bu.dat`; na build web, esse arquivo nunca é gravado;
* slot 7 e slot 8, `ImprimeArquivo` (imprimir um arquivo composto);
* slot 10, `AguardaFimImpressao`.

### 10.3 `CWasmThread`

`Create(fn, arg)` (cwasmthread.cpp:31) lança `CUeIpcError` 6231 "CWasmThread::Create sem função válida" quando `fn` é
nulo. Caso contrário, acrescenta `{fn, arg, terminou=false, id=++n}` a um vector global (@1832648) e registra
`"Create #n - enqueued"`.

**Nada jamais lê esse vector**: a função da thread nunca é executada. `Wait()` fica em laço em `emscripten_sleep(100)` enquanto
`terminou` for falso, e `terminou` nunca é ligado. `Yield()` chama `emscripten_sleep(0)`. Os dois abortam o módulo (sem
Asyncify). `js_thread_log` imprime no console incondicionalmente.

---

## 11. O boletim de urna (BU) nesta camada

O BU é gerado por `vota::CGeraBU` e impresso por `vota::CImprimindoBU` (u09, `docs/bu/codepath.md`). A página
pública nunca chega ao encerramento, porque a thread do operador não roda. O harness
`tools/bu/operator_harness.mjs` chega até ele, e nesse ponto as seguintes classes desta unidade tomam
parte:

1. **Relógio.** Os campos de data/hora do BU (`dataHoraEmissao`, início/fim da votação) vêm de `CDateTime::Now()`, que é
   `CWasmSystemDateTime`: o relógio local do navegador mais o deslocamento deixado por qualquer `SetDataHora`. O harness define um
   relógio falso (`docs/bu/codepath.md` §6).
2. **Log.** Todo passo do BU registra através de `CWasmLogd`, por exemplo `"Imprimindo relatório [BU] via nº [1]"`. Cada linha
   vira `<app>|<sev>|<text>` em `/dsk/fi/dinamico/log/logd.dat`.
3. **Impressão das vias.** `CImprimindoBU::ImprimeBU(via, modo)` chama
   `IPaperRelatorios::ImprimeArquivo(trab/bu.dat, CSigVerifier(trab,"bu.dat","bu.vsu"), header "<n>a. VIA",
   "boletim de urna", "<n>ª via")` (slot 8), depois `AguardaFimImpressao()` (slot 10).
   * Na build web, o slot 8 é `CWasmNullPaper::ImprimeArquivo` (func 8371). Ele apenas libera o formulário de cabeçalho: nenhum
     byte é impresso, e a **verificação de assinatura que o serviço de impressão faria em `bu.dat` contra `bu.vsu` nunca
     é executada**.
   * O slot 10 retorna imediatamente.
   * O chamador ainda incrementa `qtdBU` em `vota.bin`, então a urna acredita que a via foi impressa.
   * Antes disso, o próprio texto do BU foi **composto** através dos slots 5 (`Abre(<trab>/bu.dat)`), 2/4/3/12 (texto, avanço
     de linha, corte, QR) e 6 (`Fecha`), que também são no-ops: o arquivo que o slot 8 é solicitado a imprimir não existe na
     build web. Verificado com o harness (fluxo treino, cujo papel de gravação substitui esses slots): quatro
     relatórios são compostos (`bu`, `buj`, `bim`, `behb`: 124/69/42/73 chamadas entre `Abre` e `Fecha`), e então exatamente
     uma chamada ao slot 8 e uma ao slot 10.
   * A zerésima e os outros relatórios passam pelos mesmos slots de composição e pelo slot 7, todos no-ops.
4. **Largura da impressora.** `CWasmNullPrinter::GetColumns` dá 38 (19 em largura dupla), que é o que o código de layout
   dos relatórios vê.
5. **Mídias de resultado.** `CWasmInit` responde "MR não presente / não montada" (17/18 → 1) e "OK" para montar, desmontar e
   habilitar (34/58/10 → 0). `IInterfaceInit::DispositivoMR` também dorme 500 ms através de `emscripten_sleep` (u23), o que
   aborta. A cópia dos arquivos de resultado para a MR, portanto, não pode funcionar no navegador.
6. **BU na tela (BU digital).** `vota::CMostraQRCodeBU` mostra o BU como QR codes na tela do eleitor. Ele usa
   `CQRCodeImage(380, QRCodeAtual)`, um **BMP de 8 bits** de 145.478 bytes (cabeçalho de 1078 bytes + 380 × 380 pixels), desenhado
   através de `CImageField` → `CWasmScreen::DrawImage` (slot 22).

   `GetImageSize` não interpreta BMP, então a imagem mede 1 × 1 e é desenhada com **2 × 2 px**. Verificado executando
   o harness com `js_image` registrado (uma cópia de `tools/bu/operator_harness.mjs --flow treino` em que `js_image`
   imprime em vez de retornar):

   ```
   [state] voter=vota::CEmitirMaisBU
   IMG 145478 42 4d 1258 59 2 2          <- js_image(bytes, 145478 ("BM"), x=1258, y=59, w=2, h=2)
   [state] voter=vota::CMostraQRCodeBU
   ```

   O **conteúdo** do QR está correto (`docs/bu/qrcode.md`). Apenas a exibição fica inutilizável no simulador. O QR do
   certificado (`CMostraQRCodeCertificado`) e o QR de estado da urna da tela da zerésima passam pelo mesmo caminho. Na urna,
   o `IScreen` real decodifica BMP.
7. **Assinaturas.** Esta camada não assina nada. Os arquivos `.vsu` contêm `"assinatura simulada para vota_web_wasm"`,
   gravado pela u29.

## 12. Observações sobre wasm / Emscripten

* **Unificação de código idêntico.** Muitos slots são corpos no-op compartilhados:
  * 218 `void()`, 425 `void(x)`, 1528 `void(x,y)`, 1870 `void(x,y,z)`;
  * 340 `return 0`, 434 `return 1`, 371 `return 1` com um argumento;
  * 174 `return this` (destrutor trivial), 144 `operator delete` (destrutor de deleção).

  Métodos idênticos de uma mesma classe também são unidos: os slots do MT 8 = 11, 9 = 10 e 7 = 19. Destrutores de classes com uma
  `std::string` em +8 são unidos entre classes: `WasmText` usa 1727/1969, que recebem a vtable como parâmetro.
* **`std::shared_ptr` é `[[clang::trivial_abi]]`** (ABI v2 da libc++). Um `shared_ptr` passado por valor é liberado pela
  **função chamada**. Isso explica os corpos 8487 e 8371, que de outro modo pareceriam vazios (apenas uma liberação), e a
  contagem de referências +1 / −2 em 9565. Uma `std::function` passada por valor ainda é destruída pelo chamador.
* **Conversão float→int saturante.** `ceil(scale·v)` compila para `i32.trunc_sat_f64_s`. Com uma escala zero ou negativa
  (por exemplo, `?screenWidth=65536`, que trunca para o `short` 0), as divisões dão ±inf/NaN, que saturam em vez de
  gerar trap.
* **`emscripten_async_call`** recebe um índice de tabela (838) como ponteiro de função C. O glue o chama através de
  `getWasmTableEntry` dentro de `callUserCallback`, de modo que o wasm é reentrado a partir de um timer JS sem passar por um
  export.
* **`std::this_thread::sleep_for`** vira `if (byte@1584624 == 1) emscripten_sleep(ms)`. O byte é inicializado com 1
  e nunca é gravado.
* **Switch → tabela de consulta.** O switch de cores produziu duas tabelas, a segunda com o índice 0 removido (§4.1).
* **Statics locais de função.** Os buffers do log web são um único static protegido (@1832676/@1832844), inlinado tanto em 8327
  quanto em 1524.

## 13. Código suspeito ou arriscado

| # | func(s) | achado | impacto | severidade |
|---:|---|---|---|---|
| 1 | 3493 (+ inlinado em 8605) | O parser de tamanho de imagem só conhece GIF/PNG/JPEG. **Todo BMP é 1 × 1**, e todos os QR codes na tela (BU digital, certificado, estado da urna) são BMPs, então são desenhados como pontos de 2 × 2 px. Verificado para o QR do BU (BMP de 145.478 bytes → `js_image(…, 1258, 59, 2, 2)`, executado novamente para esta revisão com o harness, fluxo treino). O BMP de 66 bytes das telas de digitação de número (u39) também chega a `js_image(…, 2, 2)`, mas ele realmente é um placeholder de 1 × 1, então não é evidência do bug. | Simulador: a tela do BU digital fica inutilizável. Ela só é alcançada através do harness. A urna real tem o seu próprio `IScreen`. | média |
| 2 | 9655, 9652, 9651 | `CWasmThread` nunca executa threads. `Create` enfileira num vector que nada lê, `Wait` gira em `emscripten_sleep(100)` esperando uma flag que ninguém liga, e `Yield` chama `emscripten_sleep(0)`. Os dois abortam o módulo (sem Asyncify). | Qualquer caminho que inicie e faça join de uma `api::CThread`, ou que execute um laço de thread, derruba o simulador. A página os evita acionando as máquinas de estados a partir de `votaTick`. | média |
| 3 | 8461 (+ 8479, 8472, 8450) | O default de `ISound::Wait` consulta com `sleep_for(20ms)`, que é `emscripten_sleep` e aborta. Os defaults dos slots 2/4/9 o chamam. | Inalcançável: `CWasmNullSound` sempre reporta parado, e `CWasmWebSound` sobrescreve todos eles. Abortaria se um dispositivo futuro dependesse dos defaults. | baixa |
| 4 | 9523, 9515 | `CWasmWebSound::Wait` não espera. `WaitAsync` chama `aoTerminar(true)` **de forma síncrona**, antes que o chamador tenha armazenado o handle retornado, quando nada está tocando. Ele nunca chama `aoTerminar` quando `interromper()` já é verdadeiro. | O timing difere do da urna. `CVotacaoStateAudio::PlayMessage` (CONFIRMA) não bloqueia mais até o fim da mensagem. | baixa |
| 5 | 7909, 7918 | Os callbacks de timer rodam a partir de um `setTimeout` JS, fora do try/catch de `votaTick`. Uma exceção vinda de um callback (inclusive `bad_function_call`) escapa para o navegador como um erro não capturado: o timer nunca é rearmado, e o contexto de 16 bytes vaza junto com a sua referência `shared_ptr`, de modo que o `State` e as capturas do callback nunca são liberados. `__stack_pointer` também não é restaurado: `callUserCallback` não tem `stackRestore` (só os wrappers `invoke_*` têm). | Simulador: um relógio, um campo piscante ou um ícone de bateria poderiam parar de atualizar silenciosamente. Sem crash do módulo. | baixa |
| 6 | 8302 → construtor de CWasmScreen, 9009 | O tamanho da tela vindo dos parâmetros de URL é truncado para `short` pelo chamador. `?screenWidth=65536` dá 0: `m_escalaX = 0`, todo x colapsa para 0 (verificado com o headless), e `GetTextWidth` divide por 0: `+inf` satura para `INT_MAX` no seu resultado `int`, que `GetFontMetrics` armazena como o `short` −1 (limitado a 1 por `WriteText`), e um texto vazio dá `NaN` → 0. Valores de 32768 a 65535 dão escalas negativas. | Só é afetado o usuário que forja a URL: um layout quebrado, sem crash. | baixa |
| 7 | 9090, 9049, 9068 | `WriteText` com clip ignora o clip, `DrawImage(pos, img, part)` espreme a imagem inteira em vez de recortá-la, e o slot 23 desenha um placeholder cinza de 64 × 64. | Diferenças visuais em relação à urna (o rótulo de duas cores da barra de progresso, partes de imagens). | baixa |
| 8 | 8371, 8372 (+ slots unidos por ICF) | A impressão é um no-op: as vias do BU, a zerésima, o BJE e o boletim de mesários nunca são produzidos. Até a composição deles (`Abre`/`Print`/`Fecha`, slots 5/2/6) é descartada, então os arquivos de relatório entregues a `ImprimeArquivo` nunca são gravados. O `CSigVerifier` passado com `bu.dat`/`bu.vsu` nunca é avaliado, e `AguardaFimImpressao` retorna imediatamente, mas a aplicação conta a via como impressa. | Só no simulador. A verificação de integridade antes da impressão está ausente, não simulada. | info |
| 9 | 9405 | `CWasmInit` responde a todos os comandos de hardware com valores prontos: desligar retorna OK e não faz nada, e a MR está sempre ausente. | Só no simulador. Isso explica por que o fim do dia não consegue copiar os resultados. | info |
| 10 | 7954, 7945 | O relógio da urna é o relógio do navegador (hora local como época) mais um deslocamento. `SetDataHora` não altera nenhum relógio real. | Só no simulador. As datas em qualquer arquivo gerado seguem o relógio do PC do visitante. | info |
| 11 | 8327 | `CWasmLogd` trunca `logd.dat` a cada carregamento da página e ignora erros de gravação. Os registros são texto puro, sem encadeamento nem assinatura. | Só no simulador. O log não é um modelo fiel do log da urna. | info |
| 12 | 9104, 9079, 9058, 8991, 8605 | Desempenho: cada desenho de texto copia a string três vezes (`GetText()` por valor), a converte e constrói uma string de depuração que o JS depois descarta. `DrawImage` copia o arquivo codificado uma vez (`IImage::GetImage()` por valor), e o retângulo do campo (8991) o copia de novo. `DrawMovie` não copia, mas `GetMovie` copia cada GIF (0,8–1,6 MB) mais uma vez depois de lê-lo (`initializer_list`). A cada desenho, o glue copia os bytes (`HEAPU8.slice`) e calcula o hash FNV de todos eles para o seu cache de imagens. | Custo de CPU e memória apenas quando algo é desenhado. **Não** faz parte das ~2.000 idas e voltas `invoke_*` por tick ocioso de `analysis/runtime/README.md`, que acontecem sem nenhum redesenho. | info |

Nenhum código desta unidade usa a rede, lê cookies ou o armazenamento local, ou envia dados para fora da página. As únicas
entradas da página das quais ela depende são o tamanho de tela da URL (§4.1) e a flag de áudio ligado/desligado (através de `votaInit`, u29).

## 14. Tabela de mapeamento: todas as funções da unidade

`ran` = observada em execução durante os votos gravados (coluna "executou"). Os caminhos são relativos a `uenux2/…`; "inferido" marca um caminho
inferido a partir do nome da classe. Uma função extra, fora da unidade, pertence a estas classes:
**7909** `simulador::CWasmTimer::Dispara` (slot de tabela 838; as ferramentas a deixaram como `unknown_f7909`), em `cwasmtimer.cpp`.
Os helpers que essas classes chamam mas que pertencem à u29 estão listados em `cwasmutil.h`: 5098, 5113, 3533, 2626, 3452, 5152,
1524, 2662.

| func | tamanho | executou | nome nas ferramentas | símbolo reconstruído | arquivo original | src | conf. | evidência |
|---:|---:|:-:|---|---|---|---|---|---|
| 3347 | 84 |  | `simulador::CWasmTimer::vf0` | `simulador::CWasmTimer::~CWasmTimer` | `uenux2/mock/app/simulador/wasm/cwasmtimer.cpp` (inferido) | […/wasm/cwasmtimer.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmtimer.cpp) | alta | slot 0; Stop inlinado + liberação do State |
| 3493 | 487 |  | `simulador::CWasmImageSurfaceOps::vf12` | `simulador::CWasmImageSurfaceOps::GetImageSize` | `uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp` (inferido) | […/wasm/cwasmimagesurfaceops.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp) | alta | slot 12; parser de cabeçalho GIF/PNG/JPEG (BMP -> 1x1) |
| 5036 | 22 |  | `simulador::CWasmScreenMT::vf9` | `simulador::CWasmScreenMT::LedOperacao1` | `uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp` (inferido) | […/wasm/cwasmscreenmt.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp) | baixa | slots 9 e 10 (CLedFieldMT::Draw chama o slot op+8, ops 1/2); LED aceso -> CABINA: OCUPADA; nome descritivo |
| 5038 | 22 |  | `simulador::CWasmScreenMT::vf8` | `simulador::CWasmScreenMT::LedOperacao0` | `uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp` (inferido) | […/wasm/cwasmscreenmt.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp) | baixa | slots 8 e 11 (CLedFieldMT::Draw chama o slot op+8, ops 0/3); LED apagado -> CABINA: LIVRE; nome descritivo |
| 5039 | 15 |  | `simulador::CWasmScreenMT::vf7` | `simulador::CWasmScreenMT::Slot7` | `uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp` (inferido) | […/wasm/cwasmscreenmt.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp) | baixa | slots 7 e 19 (ICF); apenas Refresh |
| 7871 | 498 | ✓ | `simulador::CWasmTimerScheduler::vf0` | `simulador::CWasmTimerScheduler::CreateTimer` | `uenux2/mock/app/simulador/wasm/cwasmtimerscheduler.cpp` (inferido) | […/wasm/cwasmtimerscheduler.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmtimerscheduler.cpp) | alta | slot 0 de ITimerScheduler; make_shared<CWasmTimer> |
| 7896 | 12 |  | `simulador::CWasmTimer::vf5` | `simulador::CWasmTimer::SetInterval` | `uenux2/mock/app/simulador/wasm/cwasmtimer.cpp` (inferido) | […/wasm/cwasmtimer.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmtimer.cpp) | alta | slot 5; armazena ms |
| 7902 | 10 |  | `simulador::CWasmTimer::vf4` | `simulador::CWasmTimer::IsRunning` | `uenux2/mock/app/simulador/wasm/cwasmtimer.cpp` (inferido) | […/wasm/cwasmtimer.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmtimer.cpp) | alta | slot 4; retorna ativo |
| 7918 | 137 | ✓ | `simulador::CWasmTimer::vf2` | `simulador::CWasmTimer::Start` | `uenux2/mock/app/simulador/wasm/cwasmtimer.cpp` (inferido) | […/wasm/cwasmtimer.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmtimer.cpp) | alta | slot 2 (ITimer, gui-common.u15.h); emscripten_async_call(838,...) |
| 7926 | 13 |  | `simulador::CWasmTimer::vf1` | `simulador::CWasmTimer::~CWasmTimer [deleting]` | `uenux2/mock/app/simulador/wasm/cwasmtimer.cpp` (inferido) | […/wasm/cwasmtimer.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmtimer.cpp) | alta | slot 1 |
| 7934 | 30 | ✓ | `simulador::CWasmTimer::vf3` | `simulador::CWasmTimer::Stop` | `uenux2/mock/app/simulador/wasm/cwasmtimer.cpp` (inferido) | […/wasm/cwasmtimer.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmtimer.cpp) | alta | slot 3; ativo=false, ++geracao |
| 7945 | 14 |  | `simulador::CWasmSystemDateTime::vf1` | `simulador::CWasmSystemDateTime::SetDataHora` | `uenux2/mock/app/simulador/wasm/cwasmsystemdatetime.cpp` (inferido) | […/wasm/cwasmsystemdatetime.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmsystemdatetime.cpp) | média | slot 1 (nome da u05); armazena o deslocamento |
| 7954 | 18 | ✓ | `simulador::CWasmSystemDateTime::vf0` | `simulador::CWasmSystemDateTime::GetDataHora` | `uenux2/mock/app/simulador/wasm/cwasmsystemdatetime.cpp` (inferido) | […/wasm/cwasmsystemdatetime.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmsystemdatetime.cpp) | média | slot 0; deslocamento + js_obter_data_hora_local_navegador() |
| 8316 | 40 |  | `simulador::CWasmLogd::vf1` | `simulador::CWasmLogd::~CWasmLogd [deleting]` | `uenux2/mock/app/simulador/wasm/cwasmlogd.cpp` (inferido) | […/wasm/cwasmlogd.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmlogd.cpp) | alta | slot 1 |
| 8318 | 37 |  | `simulador::CWasmLogd::vf0` | `simulador::CWasmLogd::~CWasmLogd` | `uenux2/mock/app/simulador/wasm/cwasmlogd.cpp` (inferido) | […/wasm/cwasmlogd.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmlogd.cpp) | alta | slot 0 |
| 8327 | 818 | ✓ | `simulador::CWasmLogd::vf4` | `simulador::CWasmLogd::Escreve` | `uenux2/mock/app/simulador/wasm/cwasmlogd.cpp` (inferido) | […/wasm/cwasmlogd.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmlogd.cpp) | média | slot 4 (nome da u20); linha std::format("{}\|{}\|{}") acrescentada a logd.dat |
| 8365 | 9 |  | `simulador::CWasmNullPrinter::vf12` | `simulador::CWasmNullPrinter::DoSetStyle` | `uenux2/mock/app/simulador/wasm/cwasmnullprinter.cpp` (inferido) | […/wasm/cwasmnullprinter.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmnullprinter.cpp) | média | slot 12 (nome da u17); armazena o estilo |
| 8371 | 52 |  | `simulador::CWasmNullPaper::vf8` | `simulador::CWasmNullPaper::ImprimeArquivo` | `uenux2/mock/app/simulador/wasm/cwasmnullpaper.cpp` (inferido) | […/wasm/cwasmnullpaper.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmnullpaper.cpp) | média | slot 8 de IPaperRelatorios (nome da u09, vias do BU); libera o shared_ptr do cabeçalho |
| 8372 | 2 |  | `simulador::CWasmNullPaper::vf7` | `simulador::CWasmNullPaper::ImprimeArquivo` | `uenux2/mock/app/simulador/wasm/cwasmnullpaper.cpp` (inferido) | […/wasm/cwasmnullpaper.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmnullpaper.cpp) | média | slot 7 de IPaperRelatorios (nome da u09); vazio |
| 8450 | 524 |  | `simulador::CWasmNullSound::vf9` | `api::ISound::WaitAsync` | `uenux2/src/api/hwil/isound.h` (inferido) | [src/…/api/hwil/isound.h](../../src/uenux2/src/api/hwil/isound.h) | alta | default inline do slot 9; RTTI api::ISound::WaitAsync(std::function<bool ()>, std::function<void (bool)>)::lambda |
| 8461 | 96 |  | `simulador::CWasmNullSound::vf8` | `api::ISound::Wait` | `uenux2/src/api/hwil/isound.h` (inferido) | [src/…/api/hwil/isound.h](../../src/uenux2/src/api/hwil/isound.h) | média | default inline do slot 8; consulta de status + emscripten_sleep(20) |
| 8472 | 253 |  | `simulador::CWasmNullSound::vf4` | `api::ISound::Play` | `uenux2/src/api/hwil/isound.h` (inferido) | [src/…/api/hwil/isound.h](../../src/uenux2/src/api/hwil/isound.h) | média | default inline do slot 4; usa ISound::Wait()::lambda (RTTI) |
| 8479 | 124 |  | `simulador::CWasmNullSound::vf2` | `api::ISound::Play` | `uenux2/src/api/hwil/isound.h` (inferido) | [src/…/api/hwil/isound.h](../../src/uenux2/src/api/hwil/isound.h) | média | default inline do slot 2 na vtable de CWasmNullSound; usa ISound::Wait()::lambda (RTTI) |
| 8487 | 52 |  | `simulador::CWasmNullSound::vf3` | `simulador::CWasmNullSound::Play` | `uenux2/mock/app/simulador/wasm/cwasmnullsound.cpp` (inferido) | […/wasm/cwasmnullsound.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmnullsound.cpp) | média | slot 3 Play(shared_ptr<CWavFile>); apenas libera o parâmetro |
| 8591 | 50 |  | `simulador::CWasmResource::vf6` | `simulador::CWasmResource::IsResource` | `uenux2/mock/app/simulador/wasm/cwasmresource.cpp` (inferido) | […/wasm/cwasmresource.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmresource.cpp) | média | slot 6 = isResource (iresource.h:97); name[0]==':' |
| 8600 | 67 |  | `simulador::CWasmResource::vf5` | `simulador::CWasmResource::Exists` | `uenux2/mock/app/simulador/wasm/cwasmresource.cpp` (inferido) | […/wasm/cwasmresource.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmresource.cpp) | baixa | slot 5; resultado do resolvedor não vazio |
| 8605 | 1029 | ✓ | `simulador::CWasmResource::vf4` | `simulador::CWasmResource::GetMovie` | `uenux2/mock/app/simulador/wasm/cwasmresource.cpp` (inferido) | […/wasm/cwasmresource.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmresource.cpp) | alta | slot 4 = getResourceMovie (iresource.h:87); js_resource_log("movie"); construtor de CMovie 5526 |
| 8611 | 185 | ✓ | `simulador::CWasmResource::vf3` | `simulador::CWasmResource::GetFile` | `uenux2/mock/app/simulador/wasm/cwasmresource.cpp` (inferido) | […/wasm/cwasmresource.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmresource.cpp) | alta | slot 3 = getResourceFile (iresource.h:82); js_resource_log("file") |
| 8633 | 189 |  | `simulador::CWasmResource::vf2` | `simulador::CWasmResource::GetImage` | `uenux2/mock/app/simulador/wasm/cwasmresource.cpp` (inferido) | […/wasm/cwasmresource.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmresource.cpp) | média | slot 2; js_resource_log("image"); make_shared<CFixedImage> |
| 8680 | 12 |  | `simulador::CWasmScreenMT::ShowClock(api::SPoint const&)::WasmText::vf1` | `simulador::CWasmScreenMT::ShowClock::WasmText::~WasmText [deleting]` | `uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp` (inferido) | […/wasm/cwasmscreenmt.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp) | alta | slot 1 da vtable da classe local @1530048 |
| 8742 | 5 |  | `simulador::CWasmScreenMT::vf18` | `simulador::CWasmScreenMT::GetHeight` | `uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp` (inferido) | […/wasm/cwasmscreenmt.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp) | média | slot 18; retorna 80 |
| 8743 | 5 |  | `simulador::CWasmScreenMT::vf17` | `simulador::CWasmScreenMT::GetWidth` | `uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp` (inferido) | […/wasm/cwasmscreenmt.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp) | média | slot 17; retorna 480 (centralização de CInfoMTLCD) |
| 8744 | 130 |  | `simulador::CWasmScreenMT::vf1` | `simulador::CWasmScreenMT::~CWasmScreenMT [deleting]` | `uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp` (inferido) | […/wasm/cwasmscreenmt.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp) | alta | slot 1 |
| 8745 | 127 |  | `simulador::CWasmScreenMT::vf0` | `simulador::CWasmScreenMT::~CWasmScreenMT` | `uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp` (inferido) | […/wasm/cwasmscreenmt.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp) | alta | slot 0; 4 strings + vptr de IScreenMT + RemoveAll(3461) |
| 8746 | 1371 | ✓ | `simulador::CWasmScreenMT::vf14` | `simulador::CWasmScreenMT::Refresh` | `uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp` (inferido) | […/wasm/cwasmscreenmt.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp) | alta | slot 14 (refresh do dispositivo em IForm::Redraw, u17); js_mt_set |
| 8748 | 12 |  | `simulador::CWasmScreenMT::ShowClock(api::SPoint const&)::WasmText::vf0` | `simulador::CWasmScreenMT::ShowClock::WasmText::~WasmText` | `uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp` (inferido) | […/wasm/cwasmscreenmt.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp) | alta | slot 0 da vtable da classe local @1530048 |
| 8757 | 250 |  | `simulador::CWasmScreenMT::vf12` | `simulador::CWasmScreenMT::ShowClock` | `uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp` (inferido) | […/wasm/cwasmscreenmt.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp) | alta | slot 12; RTTI da classe local CWasmScreenMT::ShowClock(api::SPoint const&)::WasmText; "hh:mm" |
| 8787 | 15 |  | `simulador::CWasmScreenMT::vf6` | `simulador::CWasmScreenMT::Bipa` | `uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp` (inferido) | […/wasm/cwasmscreenmt.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp) | baixa | slot 6 (nome da u10 Bipa(50,3)); apenas Refresh |
| 8799 | 499 |  | `simulador::CWasmScreenMT::vf3` | `simulador::CWasmScreenMT::Write` | `uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp` (inferido) | […/wasm/cwasmscreenmt.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp) | média | slot 3 (nome de ctextfieldmt da u17); escritor da grade 4x40 |
| 8810 | 67 |  | `simulador::CWasmScreenMT::vf2` | `simulador::CWasmScreenMT::Clear` | `uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp` (inferido) | […/wasm/cwasmscreenmt.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreenmt.cpp) | média | slot 2; 4 x assign(40,' ') + Refresh |
| 8945 | 7 |  | `simulador::CWasmScreen::vf34` | `simulador::CWasmScreen::GetImageSurfaceOps` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferido) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | alta | slot 34; retorna this+44 (CWasmImageSurfaceOps) |
| 8953 | 7 |  | `simulador::CWasmScreen::vf31` | `simulador::CWasmScreen::GetHeight` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferido) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | alta | slot 31; retorna 480 |
| 8959 | 7 |  | `simulador::CWasmScreen::vf30` | `simulador::CWasmScreen::GetWidth` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferido) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | alta | slot 30 (chamador com srcloc cformbuilder :292); retorna 640 |
| 8963 | 20 |  | `simulador::CWasmScreen::vf1` | `simulador::CWasmScreen::~CWasmScreen [deleting]` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferido) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | alta | slot 1 da vtable; vptr de IScreen + RemoveAll(5069) + free |
| 8985 | 144 |  | `simulador::CWasmImageSurfaceOps::vf17` | `simulador::CWasmImageSurfaceOps::GetImageRect` | `uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp` (inferido) | […/wasm/cwasmimagesurfaceops.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp) | média | slot 17 (shared_ptr<IImage> por valor) |
| 8991 | 89 |  | `simulador::CWasmImageSurfaceOps::vf16` | `simulador::CWasmImageSurfaceOps::GetImageRect` | `uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp` (inferido) | […/wasm/cwasmimagesurfaceops.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp) | alta | slot 16 (IImage&); chamadores CImageField::Rect, CImageFieldUpdate::Rect (nome da u15) |
| 8994 | 69 |  | `simulador::CWasmImageSurfaceOps::vf15` | `simulador::CWasmImageSurfaceOps::GetImageRect` | `uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp` (inferido) | […/wasm/cwasmimagesurfaceops.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp) | média | slot 15 (vector) = slot12+slot14 |
| 8998 | 85 | ✓ | `simulador::CWasmImageSurfaceOps::vf13` | `simulador::CWasmImageSurfaceOps::GetImageSize` | `uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp` (inferido) | […/wasm/cwasmimagesurfaceops.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp) | alta | slot 13 (IImage&) -> slot 12; chamador CInfoMTLCD::Update |
| 9004 | 17 |  | `simulador::CWasmScreen::vf33` | `simulador::CWasmScreen::Clear` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferido) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | baixa | slot 33 (int, TColor) -> slot 4 |
| 9009 | 144 | ✓ | `simulador::CWasmScreen::vf32` | `simulador::CWasmScreen::GetTextWidth` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferido) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | alta | slot 32; js_measure_text_width; retorna um int completo (sem extend16) |
| 9013 | 16 |  | `simulador::CWasmScreen::vf29` | `simulador::CWasmScreen::SetColors` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferido) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | baixa | slot 29; armazena +36/+40 |
| 9018 | 1621 |  | `simulador::CWasmScreen::vf28` | `simulador::CWasmScreen::RefreshRect` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferido) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | média | slot 28; JSON parcial de vota:screen |
| 9022 | 134 | ✓ | `simulador::CWasmScreen::vf27` | `simulador::CWasmScreen::Refresh` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferido) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | alta | slot 27; js_log("Refresh()") + evento vota:screen |
| 9038 | 392 | ✓ | `simulador::CWasmScreen::vf26` | `simulador::CWasmScreen::DrawMovie` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferido) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | alta | slot 26 (CMovieField::Draw); frames.at(frameAtual) |
| 9049 | 277 |  | `simulador::CWasmScreen::vf25` | `simulador::CWasmScreen::DrawImage` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferido) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | média | slot 25 DrawImage(pos,img,SRect part) |
| 9058 | 476 |  | `simulador::CWasmScreen::vf24` | `simulador::CWasmScreen::DrawImage` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferido) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | alta | slot 24 DrawImage(pos,img,size,anchor) |
| 9068 | 292 |  | `simulador::CWasmScreen::vf23` | `simulador::CWasmScreen::DrawSurface` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferido) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | baixa | slot 23; placeholder cinza 64x64, argumento não usado |
| 9079 | 435 | ✓ | `simulador::CWasmScreen::vf22` | `simulador::CWasmScreen::DrawImage` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferido) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | alta | slot 22 (CImageField::Draw); js_image |
| 9090 | 46 |  | `simulador::CWasmScreen::vf21` | `simulador::CWasmScreen::WriteText` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferido) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | média | slot 21, variante com clip (clip ignorado) -> slot 19 |
| 9104 | 1420 | ✓ | `simulador::CWasmScreen::vf20` | `simulador::CWasmScreen::WriteText` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferido) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | alta | slot 20 WriteText(const SRect&,...); js_log("Write '") + js_text |
| 9115 | 294 | ✓ | `simulador::CWasmScreen::vf19` | `simulador::CWasmScreen::WriteText` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferido) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | alta | slot 19 WriteText(const SPoint&,...) -> slot 20 |
| 9125 | 627 | ✓ | `simulador::CWasmScreen::vf18` | `simulador::CWasmScreen::DrawPath` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferido) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | alta | slot 18; js_path(fill=0) |
| 9146 | 617 | ✓ | `simulador::CWasmScreen::vf17` | `simulador::CWasmScreen::FillPath` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferido) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | alta | slot 17; js_path(fill=1); chamador CStepsProgressBar (u16) |
| 9155 | 114 |  | `simulador::CWasmScreen::vf16` | `simulador::CWasmScreen::DrawPolygon` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferido) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | baixa | slot 16; polilinha fechada de um vector<SPoint> |
| 9165 | 65 |  | `simulador::CWasmScreen::vf15` | `simulador::CWasmScreen::DrawTriangle` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferido) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | baixa | slot 15; 3 x slot 8 |
| 9175 | 130 |  | `simulador::CWasmScreen::vf12` | `simulador::CWasmScreen::FillGrayGradient` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferido) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | média | slot 12; js_gray_gradient |
| 9184 | 202 |  | `simulador::CWasmScreen::vf11` | `simulador::CWasmScreen::DrawRoundRect` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferido) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | média | slot 11; js_round_rect |
| 9194 | 90 |  | `simulador::CWasmScreen::vf10` | `simulador::CWasmScreen::DrawCircle` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferido) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | média | slot 10; js_circle |
| 9205 | 290 | ✓ | `simulador::CWasmScreen::vf9` | `simulador::CWasmScreen::DrawRect` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferido) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | alta | slot 9; 4 x slot 8 |
| 9213 | 111 | ✓ | `simulador::CWasmScreen::vf8` | `simulador::CWasmScreen::DrawLine` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferido) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | média | slot 8; js_line |
| 9223 | 73 |  | `simulador::CWasmScreen::vf7` | `simulador::CWasmScreen::SetPixel` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferido) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | média | slot 7; js_fill 1x1 |
| 9232 | 181 | ✓ | `simulador::CWasmScreen::vf6` | `simulador::CWasmScreen::FillRect` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferido) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | alta | slot 6; js_fill do retângulo escalado |
| 9243 | 19 | ✓ | `simulador::CWasmScreen::vf5` | `simulador::CWasmScreen::ClearRect` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferido) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | média | slot 5 -> slot 6 |
| 9254 | 39 | ✓ | `simulador::CWasmScreen::vf4` | `simulador::CWasmScreen::Clear` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferido) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | alta | slot 4; js_log("Clear(full)") + js_clear |
| 9261 | 45 | ✓ | `simulador::CWasmScreen::vf3` | `simulador::CWasmScreen::GetFontMetrics` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferido) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | alta | slot 3 = IScreen::GetFontMetrics (srcloc iscreen.h:69 no default da base 8938) |
| 9265 | 36 |  | `simulador::CWasmScreen::vf2` | `simulador::CWasmScreen::GetMaxCharSize` | `uenux2/mock/app/simulador/wasm/cwasmscreen.cpp` (inferido) | […/wasm/cwasmscreen.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmscreen.cpp) | média | slot 2 (nome de iscreen.h da u17); width=max(1,size/2), height=size |
| 9289 | 253 |  | `simulador::CWasmImageSurfaceOps::vf14` | `simulador::CWasmImageSurfaceOps::CalcRect` | `uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp` (inferido) | […/wasm/cwasmimagesurfaceops.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp) | média | slot 14 (nome vindo de ctelasvota.cpp da u04); switch de âncora |
| 9307 | 12 |  | `simulador::CWasmImageSurfaceOps::vf11` | `simulador::CWasmImageSurfaceOps::FreeBuffer` | `uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp` (inferido) | […/wasm/cwasmimagesurfaceops.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp) | baixa | slot 11; delete[] |
| 9317 | 58 |  | `simulador::CWasmImageSurfaceOps::vf10` | `simulador::CWasmImageSurfaceOps::CreateEmptySurface` | `uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp` (inferido) | […/wasm/cwasmimagesurfaceops.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp) | baixa | slot 10 -> slot 9 |
| 9322 | 55 |  | `simulador::CWasmImageSurfaceOps::vf9` | `simulador::CWasmImageSurfaceOps::CreateEmptySurface` | `uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp` (inferido) | […/wasm/cwasmimagesurfaceops.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp) | baixa | slot 9; make_shared<vector<uebyte>>() |
| 9328 | 51 |  | `simulador::CWasmImageSurfaceOps::vf8` | `simulador::CWasmImageSurfaceOps::CopyToBuffer` | `uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp` (inferido) | […/wasm/cwasmimagesurfaceops.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp) | baixa | slot 8; new[] + memcpy |
| 9333 | 7 |  | `simulador::CWasmImageSurfaceOps::vf7` | `simulador::CWasmImageSurfaceOps::Slot7` | `uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp` (inferido) | […/wasm/cwasmimagesurfaceops.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp) | baixa | slot 7; retorna *arg |
| 9339 | 15 |  | `simulador::CWasmImageSurfaceOps::vf6` | `simulador::CWasmImageSurfaceOps::Slot6` | `uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp` (inferido) | […/wasm/cwasmimagesurfaceops.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp) | baixa | slot 6; retorna arg->vtable[3]() |
| 9348 | 105 |  | `simulador::CWasmImageSurfaceOps::vf5` | `simulador::CWasmImageSurfaceOps::CreateSurface` | `uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp` (inferido) | […/wasm/cwasmimagesurfaceops.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp) | baixa | slot 5; como o slot 2 |
| 9356 | 7 |  | `simulador::CWasmImageSurfaceOps::vf4` | `simulador::CWasmImageSurfaceOps::Slot4` | `uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp` (inferido) | […/wasm/cwasmimagesurfaceops.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp) | baixa | slot 4; retorna *arg |
| 9362 | 15 |  | `simulador::CWasmImageSurfaceOps::vf3` | `simulador::CWasmImageSurfaceOps::Slot3` | `uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp` (inferido) | […/wasm/cwasmimagesurfaceops.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp) | baixa | slot 3; retorna arg->vtable[3]() |
| 9365 | 105 |  | `simulador::CWasmImageSurfaceOps::vf2` | `simulador::CWasmImageSurfaceOps::CreateSurface` | `uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp` (inferido) | […/wasm/cwasmimagesurfaceops.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmimagesurfaceops.cpp) | baixa | slot 2; make_shared<vector<uebyte>>(img.GetImage()) |
| 9388 | 10 |  | `simulador::CWasmInit::vf1` | `simulador::CWasmInit::~CWasmInit [deleting]` | `uenux2/mock/app/simulador/wasm/cwasminit.cpp` (inferido) | […/wasm/cwasminit.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasminit.cpp) | alta | slot 1; destrutor de IInterfaceInit 5898 + free |
| 9405 | 706 | ✓ | `simulador::CWasmInit::vf5` | `simulador::CWasmInit::EnviarMensagem` | `uenux2/mock/app/simulador/wasm/cwasminit.cpp` (inferido) | […/wasm/cwasminit.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasminit.cpp) | alta | slot 5 (nome da u23); tabela de comandos, log "CWasmInit::{} {}" |
| 9418 | 7 |  | `simulador::CWasmInit::vf4` | `simulador::CWasmInit::Slot4` | `uenux2/mock/app/simulador/wasm/cwasminit.cpp` (inferido) | […/wasm/cwasminit.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasminit.cpp) | baixa | slot 4; retorna +24 (10) |
| 9427 | 58 | ✓ | `simulador::CWasmNullTextToSpeech::vf10` | `simulador::CWasmNullTextToSpeech::~CWasmNullTextToSpeech [deleting]` | `uenux2/mock/app/simulador/wasm/cwasmnulltexttospeech.cpp` (inferido) | […/wasm/cwasmnulltexttospeech.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmnulltexttospeech.cpp) | alta | slot 10; ~ITextToSpeech inlinado + free |
| 9436 | 9 |  | `simulador::CWasmNullTextToSpeech::vf11` | `simulador::CWasmNullTextToSpeech::Sintetiza` | `uenux2/mock/app/simulador/wasm/cwasmnulltexttospeech.cpp` (inferido) | […/wasm/cwasmnulltexttospeech.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmnulltexttospeech.cpp) | média | slot 11 (nome da u02); retorna shared_ptr vazio |
| 9515 | 995 |  | `simulador::CWasmWebSound::vf9` | `simulador::CWasmWebSound::WaitAsync` | `uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp` (inferido) | […/wasm/cwasmwebsound.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp) | alta | slot 9 (nome vindo do RTTI da lambda da base); js_wasm_web_sound_wait_async |
| 9523 | 38 | ✓ | `simulador::CWasmWebSound::vf8` | `simulador::CWasmWebSound::Wait` | `uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp` (inferido) | […/wasm/cwasmwebsound.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp) | média | slot 8 (chamador da lambda de ISound::Wait); não bloqueante |
| 9533 | 4 |  | `simulador::CWasmWebSound::vf7` | `simulador::CWasmWebSound::GetStatus` | `uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp` (inferido) | […/wasm/cwasmwebsound.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp) | alta | slot 7 (nome da u08); js_wasm_web_sound_get_status |
| 9541 | 13 | ✓ | `simulador::CWasmWebSound::vf6` | `simulador::CWasmWebSound::Mute` | `uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp` (inferido) | […/wasm/cwasmwebsound.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp) | média | slot 6; js_wasm_web_sound_mute |
| 9550 | 11 |  | `simulador::CWasmWebSound::vf5` | `simulador::CWasmWebSound::Stop` | `uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp` (inferido) | […/wasm/cwasmwebsound.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp) | alta | slot 5 (nome da u08); js_wasm_web_sound_stop |
| 9559 | 157 |  | `simulador::CWasmWebSound::vf4` | `simulador::CWasmWebSound::Play` | `uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp` (inferido) | […/wasm/cwasmwebsound.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp) | alta | slot 4 Play(audio, enfileira) (nome da u08); js_wasm_web_sound_play_wav |
| 9565 | 211 |  | `simulador::CWasmWebSound::vf3` | `simulador::CWasmWebSound::Play` | `uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp` (inferido) | […/wasm/cwasmwebsound.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp) | média | slot 3 Play(audio) = slot 4 com false |
| 9573 | 18 |  | `simulador::CWasmWebSound::vf2` | `simulador::CWasmWebSound::Play` | `uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp` (inferido) | […/wasm/cwasmwebsound.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp) | média | slot 2 Play(arquivo, enfileira): não suportado |
| 9580 | 18 |  | `simulador::CWasmWebSound::vf1` | `simulador::CWasmWebSound::Play` | `uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp` (inferido) | […/wasm/cwasmwebsound.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp) | média | slot 1 Play(arquivo): não suportado |
| 9589 | 11 |  | `simulador::CWasmWebSound::vf0` | `simulador::CWasmWebSound::Pause` | `uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp` (inferido) | […/wasm/cwasmwebsound.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp) | média | slot 0; js_wasm_web_sound_pause |
| 9651 | 18 |  | `simulador::CWasmThread::Yield` | `simulador::CWasmThread::Yield` | `uenux2/mock/app/simulador/wasm/cwasmthread.cpp (attested)` | […/wasm/cwasmthread.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmthread.cpp) | alta | slot 5; registra "Yield"; chamadores na u07/u10 |
| 9652 | 171 |  | `simulador::CWasmThread::vf3` | `simulador::CWasmThread::Wait` | `uenux2/mock/app/simulador/wasm/cwasmthread.cpp (attested)` | […/wasm/cwasmthread.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmthread.cpp) | média | slot 3; registra "Wait - polling"/"Wait - finished" |
| 9655 | 670 |  | `simulador::CWasmThread::Create` | `simulador::CWasmThread::Create` | `uenux2/mock/app/simulador/wasm/cwasmthread.cpp (attested)` | […/wasm/cwasmthread.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmthread.cpp) | alta | srcloc cwasmthread.cpp:31 |
| 10880 | 15 |  | `simulador::CWasmNullPrinter::vf5` | `simulador::CWasmNullPrinter::GetColumns` | `uenux2/mock/app/simulador/wasm/cwasmnullprinter.cpp` (inferido) | […/wasm/cwasmnullprinter.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmnullprinter.cpp) | média | slot 5 (nome da u17); 19 ou 38 |
| 13877 | 148 |  | `simulador::CWasmNullTextToSpeech::CWasmNullTextToSpeech` | `simulador::CWasmNullTextToSpeech::CWasmNullTextToSpeech` | `uenux2/mock/app/simulador/wasm/cwasmnulltexttospeech.cpp` (inferido) | […/wasm/cwasmnulltexttospeech.cpp](../../src/uenux2/mock/app/simulador/wasm/cwasmnulltexttospeech.cpp) | alta | gravação da vtable; chamado por votaInit |

## 15. Questões em aberto

* Os nomes dos slots 2–11 de `IImageSurfaceOps`, e o tipo do argumento polimórfico dos slots 3/6/10. Não existe nenhum chamador
  neste binário.
* A finalidade dos slots 13/14/23/33/35–38 de `IScreen` e dos slots 5/7/13/15/16/19 de `IScreenMT`. Os corpos web são no-ops
  ou placeholders.
* O significado dos comandos 6 (`FE_NOT_PRESENT`: flash externa?), 15/16, 39 (`512000`) e 68 de `CWasmInit`, e dos
  slots 3/4/6/7 de `IInterfaceInit`.
* Os dois parâmetros `int` ignorados de `IResource::GetImage` (slot 2), e os campos +4 (`true`) e +8 (0) que o
  construtor de `CWasmScreen` grava.
* Se algo deveria ler o barramento de log da func 5152 (u29). Nada neste binário o assina nem lê
  o seu histórico.
