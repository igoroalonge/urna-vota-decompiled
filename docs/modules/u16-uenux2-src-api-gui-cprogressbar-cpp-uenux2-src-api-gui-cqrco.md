# u16: `uenux2/src/api/gui` — barras de progresso, imagens de QR code e campos de texto

A unidade u16 cobre 85 funções wasm de 14 arquivos originais da biblioteca de GUI da urna (`api::`, diretório
`uenux2/src/api/gui`). Todas são *campos de formulário* (os widgets de que uma tela é feita) ou helpers deles:

| arquivo original | classe | o que é |
|---|---|---|
| `cprogressbar.cpp` | `api::CProgressBar` | barra de progresso horizontal ("Gravando" após cada eleitor; "Preparando dados para encerramento") |
| `cstepsprogressbar.cpp` | `api::CStepsProgressBar` | a barra de "etapas" em forma de seta no topo de toda tela de votação (uma etapa por cargo) |
| `cqrcodeguard.cpp` | `api::CQRCodeGuard` | dono RAII de um `QRcode*` da libqrencode |
| `cqrcodeimage.cpp` | `api::CQRCodeImage` | QR code como imagem de tela (BMP em cinza de 8 bits) — QR do BU/certificado no display da urna |
| `cqrcodeimagepaper.cpp` | `api::CQRCodeImagePaper` | QR code como raster de impressora de 1 bit — QR codes impressos no BU |
| `ctextfield.cpp` | `api::CTextField` | uma linha de texto |
| `ctextfieldblinking.cpp` | `api::CTextFieldBlinking` | texto alternando entre duas cores a cada 500 ms ("VOTO NULO", "VOTO EM BRANCO"…) |
| `ctextfieldupdate.cpp` | `api::CTextFieldUpdate` | texto relido periodicamente, redesenhado quando muda (cabeçalho de status, relógio) |
| `ctextfielddoubleline.cpp` | `api::CTextFieldDoubleLine` | texto com quebra de palavras em duas linhas (nomes de candidato / partido / cargo) |
| `ctextfieldmultiline.cpp` | `api::CTextFieldMultiLine` | parágrafo quebrado dentro de um retângulo |
| `ctextrectfield.cpp` | `api::CTextRectField` | texto sobre uma faixa colorida |
| `ctextbox.cpp` | `api::CTextBox` | caixa emoldurada com um rótulo e três aparências (teste de teclado) |
| `ctextfieldmt.cpp` | `api::CTextFieldMT` | texto no microterminal do mesário (4 x 40 caracteres) |
| `ctextfieldpaper.cpp` | `api::CTextFieldPaper` | uma linha de um relatório impresso (BU, zerésima…) |

19 das 85 funções executaram durante os votos gravados (`analysis/runtime/*.functions.tsv`): a barra de etapas
(5504-5507, 10972), a barra de progresso (5545, 10977), os campos de texto piscante/atualizável/de duas linhas/simples e
os seus métodos `Rect`/`Start`. Uma execução headless (`node tools/run/headless.mjs --scenario municipal-t1 --draw`)
reproduz exatamente a geometria da barra de etapas derivada abaixo (veja §5.2).

**Fontes reconstruídas** (todas sob `src/uenux2/src/api/gui/`):

* arquivos próprios: `cprogressbar.{h,cpp}`, `cstepsprogressbar.{h,cpp}`, `cqrcodeguard.{h,cpp}`, `cqrcodeimage.{h,cpp}`,
  `cqrcodeimagepaper.{h,cpp}`, `ctextfield.{h,cpp}` (o seu header também contém o resumo das interfaces de campo de formulário e
  de tela), `ctextfieldblinking.{h,cpp}`, `ctextfieldupdate.{h,cpp}`, `ctextfielddoubleline.{h,cpp}`,
  `ctextfieldmultiline.{h,cpp}`, `ctextrectfield.{h,cpp}`, `ctextbox.{h,cpp}`, `ctextfieldmt.{h,cpp}`,
  `ctextfieldpaper.{h,cpp}`
* fragmentos (funções desta unidade cujo arquivo original pertence a outra unidade):
  `cformbuilder.u16.cpp` (três helpers `CFormBuilder::Add*` que inlinam construtores desta unidade),
  `primitives.u16.cpp` (`SRect::Adjusted`)
* `cprogressbar.cpp` também contém `SetValor` (func 3667, unidade u37) e `Incrementa` (func 5508, unidade u07,
  escrita primeiro em `cprogressbar.u07.cpp`); `cstepsprogressbar.cpp` também contém a segunda estratégia de layout
  (func 10971, unidade u34), porque fazem parte do mesmo arquivo.

---

## 1. Onde este código se situa no processo de votação

Glossário: *campo* = widget; *MT* (*microterminal*) = o pequeno display de caracteres + teclado do *terminal do
mesário*; *cargo* = o cargo em votação; *BU* (*Boletim de Urna*) = o resultado da seção, impresso em papel
(com QR codes, *BU digital*) e gravado como arquivo ASN.1; *via* = cópia impressa; *encerramento* = fechamento do
dia de votação; *zerésima* = relatório de abertura.

Toda tela da aplicação da urna é um `api::CFormBuilder` (um `vector<shared_ptr<IFormField<MEDIA>>>`)
transformado em um `IForm`/`CInteractiveForm` (unidades u02/u07/u15). Os campos desta unidade são as folhas desses
formulários. Onde o eleitor os encontra:

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

## 2. Classes (RTTI) e layouts

Todos os widgets derivam (herança simples, tipo RTTI `si`) de `api::IFormField<MEDIA>` →
`api::IFormFieldBase<MEDIA>`, onde MEDIA é o dispositivo de saída:

```
IFormFieldBase<IScreen> (vtable @1537360) <- IFormField<IScreen> <- CTextField, CTextFieldBlinking,
      CTextFieldUpdate, CTextFieldDoubleLine, CTextFieldMultiLine, CTextRectField, CTextBox,
      CProgressBar, CStepsProgressBar                                  (voter display, 640 x 480 logical)
IFormFieldBase<IScreenMT> (@1579652) <- IFormField<IScreenMT> <- CTextFieldMT          (mesário micro-terminal)
IFormFieldBase<IPaper>    (@1580868) <- IFormField<IPaper>    <- CTextFieldPaper       (thermal printer)
IImage <- CQRCodeImage                     (image source used by CImageField / CImageFieldUpdate)
CQRCodeGuard, CQRCodeImagePaper            (no RTTI)
```

Layout comum da base (IFormFieldBase): `+0 vptr, +4 bool m_precisaRedesenhar, +8 IForm* m_pForm, +12 std::string m_nome`.
Layouts dos membros (derivados dos construtores, todos os offsets conferidos no WAT):

| classe | tamanho | membros após +24 |
|---|---|---|
| CTextField | 64 | +24 SPoint pos, +28 shared_ptr<IText>, +36 SFont, +44 TColor texto, +48 TColor fundo, +52 SRect anterior, +60 bool apagar |
| CTextFieldBlinking | 68 | +24 pos, +28 texto, +36 fonte, +44 cor1, +48 cor2, +52 fundo, +56 shared_ptr<ITimer>, +64 bool fase |
| CTextFieldUpdate | 80 | +24 pos, +28 texto, +36 fonte, +44 cor, +48 fundo, +52 string último texto, +64 SRect anterior, +72 shared_ptr<ITimer> |
| CTextFieldDoubleLine | 60 | +24 SPoint linha1, +28 SPoint linha2, +32 TPosition maxX, +36 texto, +44 fonte, +52 cor (2), +56 fundo |
| CTextFieldMultiLine | 72 | +24 SRect área, +32 SRect desenhado, +40 bool, +44 texto, +52 fonte, +60 cor (2), +64 fundo (1), +68 altura da linha |
| CTextRectField | 68 | +24 SRect, +32 cor da faixa, +36 texto, +44 fonte, +52 cor do texto, +56 SRect anterior, +64 bool fundo opaco |
| CTextBox | 68 | +24 CFixedText (embutido), +44 ETextStatus, +48 pos, +52 TFontSize, +56/+60 TColor, +64 altura, +66 largura |
| CTextFieldMT | 48 | +24 pos, +28 texto, +36 texto anterior, +44 pos anterior |
| CTextFieldPaper | 36 | +24 texto, +32 IPaper::EStyle |
| CProgressBar | 88 | +24 valor, +28 mínimo, +32 máximo, +36 SRect, +44 SRect interno, +52..+64 4 TColor, +68 string formato, +80 TFontSize, +82 uebyte margem, +84 TColor borda |
| CStepsProgressBar | 100 | +24 atual, +28 vector<string> rótulos, +40 SRect, +48..+72 7 TColor, +76 bool recalcular, +80 SFont, +88 vector<SSegmento> |
| CQRCodeImage | 32 | +4 TPosition tamanho, +8 std::function<std::string()> |

Tipos: `TPosition` = int16; `SPoint{x,y}` (4 bytes); `SRect{left,top,right,bottom}` (8 bytes, construído a partir de dois
pontos com normalização por min/max); `SFont{TFontSize size; style}` (8 bytes, estilo 1 = negrito: a cola
web `js_measure_text_width(text, size, bold, italic)`); `TColor` = índice em uma paleta de 38 entradas (a tela
web o mapeia para cores CSS @1529596: 0 transparente, 1 `#ffffff`, 2 `#000000`, 3 `#808080`, 4 `#d3d3d3`,
13 `#006400`, 20 `#ffd300`, 36 `#c9c9c9`…).

## 3. O protocolo dos campos de formulário

Vtable de `IFormFieldBase` (nomes inferidos, salvo quando atestados; veja `ctextfield.h`):

| slot | método | corpo padrão |
|---|---|---|
| 0 / 1 | destrutor / destrutor de deleção | |
| 2 | `void Draw(MEDIA&) const` | puro |
| 3 / 4 | `Start()` / `Stop()` | sem efeito (func 218); os dois campos com timer iniciam/param o seu `ITimer` (slots 2/3 de ITimer) |
| 5 | `bool RedrawIfDirty(MEDIA&)` | func 4118: `a = m_precisaRedesenhar; m_precisaRedesenhar = false; if (a) Draw(m); return a;` |
| 6 | `SetForm(IForm*)` | func 3037: `m_pForm = f` |
| 7 | `std::string GetClassName() const` | puro em `IFormFieldBase`; cada campo de IScreen retorna o seu nome literal ("CTextField"), usado por `CFormBuilder::Add` (func 426) para nomear os campos "CTextField1", "CTextField2"… Para IScreenMT e IPaper, o próprio `IFormField<MEDIA>` o implementa e retorna o seu próprio nome ("IFormField<IScreenMT>", func 11051; "IFormField<IPaper>", func 11014); `CTextFieldMT` e `CTextFieldPaper` **não** o sobrescrevem (um slot de tabela, 3273 / 3367, compartilhado por todos os campos de MT / de papel) |
| 8 | `SRect Rect() const` (atestado, apenas IScreen) | |
| 9 | `void Move(const SPoint&)` (atestado, apenas IScreen) | várias classes compartilham os corpos de ICF 2241/2783/5527 |

Um campo nunca se redesenha sozinho: quando o seu estado muda, ele roda um helper sempre inlinado (`Invalidate`, o
nome usado pela unidade u15 em `gui-common.u15.h`) — "se o meu formulário estiver visível (form +4, lido sob o mutex do formulário
+28): define `m_precisaRedesenhar` e chama o slot 4 do formulário (pedido de redesenho)". O formulário então chama `RedrawIfDirty` nos seus campos. Timers
(o slot 0 de `ITimerScheduler::GetInst()` cria um; `simulador::CWasmTimerScheduler` usa
`emscripten_async_call`) acionam os dois campos animados:

* `CTextFieldBlinking`: a cada 500 ms alterna `m_fase` e notifica; `Draw` pinta o texto em `m_fase ? cor1 : cor2`.
* `CTextFieldUpdate`: a cada `periodo` compara `m_texto->GetText()` com o último texto pintado; notifica se
  for diferente; `Draw` apaga o retângulo anterior e repinta.

Os `operator()` das lambdas de timer são as funcs 10937 e 10909 (componente rt:libcxx, sem unidade); destruir um campo
destrói o seu `CWasmTimer`, cujo destrutor incrementa um contador de geração, de modo que um callback assíncrono pendente se torna
sem efeito (func 3347) — sem `this` pendente.

## 4. Os campos de texto

* **CTextField** (1918/10952/10951): `Draw` = (apaga o retângulo anterior se a flag em +60 estiver definida — nenhum
  código que grave essa flag foi encontrado, então este ramo parece morto) + `IScreen::WriteText(pos, texto, fonte, cor,
  fundo)` (slot 19), guardando o retângulo retornado. `Rect()` mede o texto com
  `IScreen::GetFontMetrics` e o desloca conforme o alinhamento do texto (Right: x − w, Center: x − w/2). Os três
  métodos `Rect()` de CTextField/CTextFieldBlinking/CTextFieldUpdate são thunks de 14 bytes para um único
  corpo de merge-similar (func 3889) cujo único parâmetro é o srcloc (:55, :70, :73).
* **CTextFieldDoubleLine::GetTextLines** (5499, :68) — quebra de palavras em duas linhas, larguras medidas com a fonte
  real (`IScreen` slot 32, `js_measure_text_width` no build web):
  1. se o texto inteiro cabe em `maxX − linha1.x + 1` → `{texto, ""}`;
  2. senão, move a(s) última(s) palavra(s) para a linha 2 (`rfind(' ')`), uma palavra por vez, até a linha 1 caber;
  3. se a linha 1 é uma única palavra que ainda é larga demais, move-a caractere a caractere (um espaço é inserido
     uma vez entre a palavra quebrada e as palavras já movidas); um texto sem nenhum espaço é quebrado da mesma forma;
  4. a linha 2 é então truncada caractere a caractere até caber em `maxX − linha2.x + 1` — sem reticências.
  `Rect()` (10929; as ferramentas a chamaram de `CalcLineRect` por causa do srcloc do helper inlinado :133) é a união
  dos retângulos das duas linhas.
* **CTextFieldMultiLine::GetTextLines** (5497, :116): `std::istringstream` + `std::getline` por parágrafo; parágrafos
  vazios são mantidos; um parágrafo que não cabe é cortado após o último espaço cujo prefixo cabe, senão dentro
  da palavra, após o maior prefixo que cabe; o espaço no corte é descartado. `Draw` (10925) apaga a união
  de tudo o que foi pintado antes, depois escreve linha a linha com altura de linha de `size + ceil(size/6)`,
  alinhando cada linha conforme o alinhamento do texto; linhas abaixo do retângulo não são desenhadas.
* **CTextRectField**: `IScreen::WriteText(SRect, …)` (slot 20) dentro da faixa, fundo `m_corFundo`.
* **CTextBox**: `Rect()` = tamanho explícito, ou tamanho do texto + 20 x 8 pixels, deslocado pelo alinhamento; `Draw`
  limpa a caixa, depois status 0 = contorno (`DrawRect`, 1 px) + texto preto, 1 = preenchida de preto + texto branco,
  2 = preenchida de `#d3d3d3` + texto branco, outro = `CBaseError<EUeGuiError>(4962, "Status indefinido")`.
* **CTextFieldMT**: o microterminal não tem primitiva de apagar; quando o novo texto é mais curto que o desenhado
  antes, o antigo é primeiro sobrescrito com espaços (`IScreenMT` slot 3, buffer de linha de 40 colunas de
  `simulador::CWasmScreenMT`). Ele não tem `GetClassName` próprio (slot 7 = herdado
  `IFormField<IScreenMT>::GetClassName`, "IFormField<IScreenMT>"); o mesmo vale para `CTextFieldPaper`
  ("IFormField<IPaper>").
* **CTextFieldPaper**: `IPaper::Print(texto, estilo)` (slot 2) — no build web, o slot 2 de `CWasmNullPaper` é uma
  função vazia (func 1528): nada é impresso.

## 5. As barras de progresso

### 5.1 CProgressBar

Construída apenas por meio de `CFormBuilder::AddProgressBar(maximo, a, b, formato)` (func 5545, construtor inlinado,
constantes: mínimo 0, valor 0, cores barra 13 `#006400` / fundo 1 / texto sobre a barra 1 / texto 2 / borda 2,
margem 2, tamanho de fonte 0 = automático). Verificações do construtor, em ordem (EUeGuiError, mensagens com `std::format`):
`mínimo >= máximo` → 4944 "Limite inferior ({}) >= limite superior ({})" (:52); largura ≤ 2·margem → 4945
"Largura da barra ({}) não comporta a margem ({})" (:58); altura ≤ 2·margem → 4946 "Altura da barra ({})…"
(:67). O tamanho de fonte automático é `floor((height − 2·margem) · 0.8)`.

`Draw` (10977): texto = `formato` com `%p` → `"{}%"` de `(valor−min)·100/(max−min)` e `%v` → valor
(func 2677, substitui todas as ocorrências); `ClearRect(area, fundo)`; preenche `larguraInterna · (valor−min) / (max−min)` pixels do
retângulo interno `area.Adjusted(1,1,−3,−3)`; texto centralizado: branco onde fica sobre a barra, preto
no resto, usando o `WriteText` com recorte (slot 21) duas vezes quando o fim da barra corta o texto; por fim a
borda `DrawRect(area.Adjusted(m/2, m/2, −m/2, −m/2), borda, m)`. `SetValor` limita a [min, max]; `Incrementa`
avança uma etapa. No fluxo do eleitor a barra tem 4 etapas e nenhum texto ("Gravando"); no encerramento, 30 etapas
e `%p`.

### 5.2 CStepsProgressBar

`vota::CPreShowProgressBar::PreShow` (13550, u04) constrói, antes de cada tela de votação, a lista de rótulos (nome do
cargo, mais " - " + ordinal para cargos de múltipla escolha, como as duas vagas de Senado), encontra o índice da
etapa atual e, quando há pelo menos duas etapas, constrói a barra na pilha sobre
`{2, 2}-{W−3, 30}` (W = largura da tela), chama `Draw` e a destrói. Verificação do construtor: nenhum rótulo → 5014
"Quantidade de segmentos deve ser maior que zero" (:428).

`Draw` (5504), primeira chamada — layout (`CalculaSegmentos`, inlinado):

1. fonte inicial = `min(max(height·3/5, 10), height)`; duas estratégias são tentadas desse tamanho até 10
   (`TentaLayout`, 5507): **larguras iguais** (`CabeLarguraIgual`, 10972, slot de tabela 3442; retângulos de
   `DivideArea`, 5506) e **larguras proporcionais** (`CabeLarguraProporcional`, 10971, slot 3443). Um rótulo
   "cabe" quando `altura − size/5 ≤ height − 2·margem` e `largura − size/3 ≤ segment width − 2·margem −
   10 (notch, not for the first) − size·10/height (arrow, not for the last)`, com `margem = max(size/4, 1)`;
2. a estratégia com a fonte maior vence (empate → larguras iguais); se nenhuma cabe no tamanho 10, larguras iguais são
   usadas **e todos os rótulos são descartados**;
3. os segmentos se sobrepõem em 10 − 4 = 6 px; cada um vira um path no estilo Qt (`SPathElement`, 56 bytes: tipo 0 moveTo /
   1 lineTo / 2 arcTo(x, y, w, h, start°, sweep°) / 3 close; raio `min(height/2, 10)`): etapa única = retângulo
   arredondado; primeira = esquerda arredondada + ponta de seta; do meio = entalhe + ponta; última = entalhe + direita arredondada;
4. posição do rótulo = `{left + margem (+10 after the first), top + margem + max(0, vertical slack/2)}`, negrito.

Toda chamada então pinta cada segmento: contorno (`DrawPath`, cor 0 transparente, 1 px), preenchimento (etapas concluídas
`#808080`, atual preto, próximas `#c9c9c9`) e rótulo (preto, branco, cinza). Verificação headless no cenário
municipal (escala do canvas 2 x 1,667): segmento 1 `moveTo(24,3.33) ellipse(24,20,…) lineTo(4,35) ellipse(24,35,…)
lineTo(626,51.67) lineTo(646,27.5) lineTo(626,3.33) closePath` = lógico `{2,2}-{322,30}` esquerda arredondada + ponta,
segmento 2 começando em x = 317 (sobreposição de 6 px) com entalhe e direita arredondada — exatamente os paths acima.

## 6. QR codes

`CQRCodeGuard(const std::string&)` (5502, :23) é o `QRcode_encodeString(texto, 0, QR_ECLEVEL_L,
QR_MODE_8, 1)` da libqrencode com a maior parte da biblioteca inlinada (13,8 KB): versão 0 = a menor que couber, correção de erros
**L**, dica de modo de 8 bits, sensível a maiúsculas/minúsculas. NULL → 4947 "Não foi possível criar o QR code". O destrutor
(`QRcode_free`) é inlinado nos dois usuários.

| | `CQRCodeImagePaper::MontaImagem(400, texto)` (2772) — impressora | `CQRCodeImage::GetImage()` (10967) — tela |
|---|---|---|
| verificação da entrada | vazia → 4949 "Criação de QRCode com texto vazio." (:35) | `std::function` vazia → `std::bad_function_call` |
| verificação do tamanho | `width + 4 > 400` → 4951 "A largura do qrcode ({}) superou o espaço disponível ({})" (:49) | `tamanho < width + 4` → 4948, mesma mensagem (:36) |
| borda | 2 módulos brancos de cada lado | 2 módulos brancos, mais os pixels não usados (`tamanho mod (width+4)`) à esquerda e embaixo |
| codificação | 1 bit por módulo, LSB primeiro, em ordem de linha (row-major), 1 = escuro (`CStringUtils::SetBit`, cstringutils.cpp:83, 7027 "Bit fora dos limites…") | 1 byte por pixel (0 preto / 255 branco), `escala = tamanho/(width+4)` pixels por módulo, linhas de baixo para cima, alinhadas a 4 bytes |
| resultado | `QrcodeData{bitmap, largura = width+4, escala = min(400/largura, 4)}` | BMP completo: cabeçalho de 1078 bytes (`BM`, BITMAPINFOHEADER 40, 8 bpp, 20000 px/m, paleta de 256 cinzas) + pixels |

## 7. Boletim de Urna: o que esta unidade contribui

O payload do BU, a cadeia de hashes e a assinatura são construídos em outro lugar (`comum::CGeradorBUQRCode`, func 5604;
`docs/bu/qrcode.md`, `docs/bu/build-a-bu.md`). Esta unidade apenas os renderiza:

1. **BU impresso** (`vota::CGeraBU::vf2`, u08): as linhas de cabeçalho/corpo são campos `CTextFieldPaper(CDataText<…>, estilo)`;
   para cada parte "BU DIGITAL" (`QRBU:i:n VRQR:6.0 …`, ≤ 1100 caracteres) o gerador chama
   `CQRCodeImagePaper::MontaImagem(400, parte)` e acrescenta o raster com `CPaperFormBuilder::AddQRCode`
   (2775) sob o rótulo "-------------- 01 / 02 ---------------"; os QR codes de "CERTIFICADO DIGITAL"
   (`QRCE:… IDUE:… MDUE:… CERT:…`, func 5634 / vota_f5591) usam a mesma função. O driver da impressora grava
   o raster como `0B | u32 len(bitmap) | u32 largura | u8 escala | bitmap`; os arquivos `-imgbu.dat` reais de 2024 têm
   exatamente `len = ceil(largura²/8)` e `escala = 4` (símbolo mais largo de 93 módulos = 97 com a borda → 388
   pontos de impressora ≤ 400), nível L, versões 9-19 (docs/bu/qrcode.md §2.1).
2. **BU na tela** (`vota::CMostraQRCodeBU`, 12055, tela `telaQRCodeBU`, após a última via):
   `CQRCodeImage(380, <part i>)` dentro de um campo de imagem; o texto voltado ao eleitor é um `CTextFieldMultiLine`
   ("O QR code ao lado contém o resultado da votação para esta urna…"); BRANCO alterna para
   `CMostraQRCodeCertificado` (QR do certificado, mesma classe).
3. **Progresso**: `CProgressoEncerramento` mostra uma `CProgressBar` de 30 etapas com "%p" enquanto
   `CGravaResultado` grava e assina os arquivos de resultado (BU, RDV, logs).
4. Não é um BU: a func 3059 (nomeada `CImageFieldUpdate::CImageFieldUpdate` pelas ferramentas porque inlina esse
   construtor; é um construtor de tela com o cabeçalho de status e uma linha "RESUMO DA CORRESPONDÊNCIA: …")
   mostra o QR code do estado da urna (`comum::CQRCodeDS`, campos de `MontaCamposQRCodeEstadoUrna`, veja u36 §3.4)
   como `CQRCodeImage(148, …)` em um `CImageFieldUpdate` redesenhado a cada 15 000 ms.
5. As faixas amarelas "Número de cópias" / "acima do limite permitido" (`CTextRectField`) são o aviso mostrado
   quando são pedidas mais cópias do BU do que o permitido (provavelmente a tela de `CLimiteCopiasBUAtingido`).

## 8. O que é específico do build web

* `simulador::CWasmScreen` (canvas, 640 x 480 lógicos escalados para 1280 x 800) implementa os slots de IScreen usados
  aqui com `js_fill`, `js_text`, `js_path`, `js_measure_text_width`; o slot 5 (`ClearRect`) apenas repassa para
  o slot 6 (`FillRect`).
* **Texto recortado não é recortado**: `CWasmScreen::vf21` (func 9090) ignora o seu retângulo de recorte e chama
  `WriteText(pos, …)` (slot 19 → slot 20, que faz `js_fill` da caixa de texto inteira sempre que a cor de fundo
  não é 0). `CProgressBar::Draw` depende do recorte para pintar a porcentagem metade branca / metade preta; no
  simulador, a primeira chamada preenche a caixa de texto inteira com a cor da barra (#006400, mesmo além do fim da
  barra) e a segunda chamada repinta o texto inteiro em preto.
* **Nada é impresso**: o slot 2 de `CWasmNullPaper` é vazio, então `CTextFieldPaper::Draw` e os rasters de QR do BU
  não vão a lugar nenhum (o raster ainda é calculado).
* Os timers são `CWasmTimer` (`emscripten_async_call`), não threads; o `api::CTimer::Start` nativo lançaria
  "thread constructor failed" neste build (ele não é usado).
* O microterminal é emulado por `CWasmScreenMT` (um buffer de linhas 4 x 40 exportado no JSON de estado).

## 9. Observações notáveis sobre wasm / Emscripten

* **Construtores inlinados em builders**: 1265, 5545, 5546 carregam os registros de srcloc dos construtores de
  CTextFieldBlinking, CProgressBar e CTextRectField, então as ferramentas as nomearam a partir desses construtores, mas
  elas recebem o builder e retornam um `shared_ptr` (sret) — são helpers `CFormBuilder::Add*`. O mesmo
  acontece com `CTextBox` (inlinado em `CTesteTeclado::StartState`, 11805) e `CStepsProgressBar`
  (inlinado em `CPreShowProgressBar::PreShow`, 13550); `CTextFieldDoubleLine::Rect` carrega o registro do
  `CalcLineRect` inlinado, e `CQRCodeImage::GetImage` o do `MontaImagem` estático.
* **Parâmetros propagados como constantes**: as cores de CTextFieldUpdate (2, 1), CTextFieldMultiLine (2, 1),
  CTextFieldDoubleLine (texto 2) e o `size_t` de `CQRCodeImagePaper::MontaImagem` (400) não são mais
  parâmetros das funções compiladas; o registro de srcloc `cqrcodeimagepaper.cpp:38` ficou órfão porque a
  verificação que o usava foi eliminada por constant folding.
* **merge-similar-functions**: `Rect` de três campos de texto → 3889 (srcloc como parâmetro); destrutores de
  CTextRectField/CTextFieldDoubleLine → 6021/6022 e de CQRCodeImage/`CDataText<std::function<string()>>`
  → 6059/6060 (vtable como parâmetro); `GetClassName` de CTextFieldBlinking → 3891 (pedaços de string como
  parâmetros; o mesmo corpo de 18 caracteres também serve `IFormField<IPaper>::GetClassName`, func 11014).
* Os corpos de `GetClassName` constroem o nome da classe inline como uma string SSO (`a[5]:short = 2560` = tamanho 10 no byte 11)
  ou uma string de heap de 16/24 bytes; o wasm-decompile mostra os bytes de origem como `d_operator…[N]`, que fica 1024 abaixo
  do endereço real (ex.: 196630 → "CTextField" em 197654).
* O guard do QR code mostra a numeração de errno WASI do Emscripten: `EINVAL` é armazenado como 28 em `errno` (@1931660).
* O `QRinput` da libqrencode (28 bytes) é alocado com `malloc` e cada campo é zerado inline — é assim que
  as constantes de versão/nível de `QRcode_encodeString` podem ser lidas.

## 10. Códigos de erro (`ecourna::api::exception::CBaseError<api::EUeGuiError, SErrorLimits{4900, 5100}>`)

| código | onde | mensagem |
|---|---|---|
| 4944 / 4945 / 4946 | construtor de CProgressBar :52 / :58 / :67 | limites / largura / altura (veja §5.1) |
| 4947 | CQRCodeGuard :23 | Não foi possível criar o QR code |
| 4948 | CQRCodeImage::MontaImagem :36 | A largura do qrcode ({}) superou o espaço disponível ({}) |
| 4949 / 4951 | CQRCodeImagePaper::MontaImagem :35 / :49 | Criação de QRCode com texto vazio. / largura do qrcode… |
| 4961 / 4963 | construtor de CTextBox :41 / Move :90 | A posição da caixa não pode ter x < 3 ou y < 1. |
| 4962 | CTextBox::Draw :69 | Status indefinido |
| 4964 4965 4967 4970 4972 4973 4974 4976 | construtores de CTextField, Blinking, DoubleLine, MT, MultiLine, Paper, Update, TextRect | Campo estava com o texto nulo |
| 4966 | CTextFieldBlinking :42 | As cores estavam iguais |
| 4968 / 4969 | CTextFieldDoubleLine :43 / :46 | maxX menor que a posição x da linha 1 / 2 |
| 5014 | CStepsProgressBar :428 | Quantidade de segmentos deve ser maior que zero |
| 7027 (EUeUtilError) | CStringUtils::SetBit (inlinado em 2772) | Bit fora dos limites ({}) do vetor ({}) |

(Em `decompiled/…/ctextfieldupdate.cpp.dcmp` o código 4974 está anotado erroneamente com a string no endereço 4974.)

## 11. Código suspeito ou digno de nota

1. **CTextFieldMultiLine descarta o resto do texto** (5497): quando a posição de quebra é 0 (um parágrafo que
   começa com um espaço seguido de uma palavra mais larga que a área — ex.: após um espaço duplo — ou um primeiro
   caractere mais largo que a área), a função empilha uma linha vazia e retorna imediatamente (o
   `br_table` sobre `quebra + 1` salta para fora dos dois laços); todas as linhas e parágrafos restantes se perdem sem
   nenhum erro. Baixo (textos fixos do TSE), mas é um truncamento silencioso de instruções na tela.
2. **Erro de off-by-one na mesma função**: quando uma linha não tem espaço utilizável e todo prefixo próprio (comprimentos
   1 … size−1) cabe, o laço para sem posição de quebra e a linha inteira — já medida como larga
   demais — é mantida, ultrapassando a área no seu último caractere.
3. **Truncamento silencioso por projeto**: `CTextFieldDoubleLine` corta a segunda linha de um nome sem reticências;
   `CTextFieldMultiLine` descarta linhas abaixo do seu retângulo; `CStepsProgressBar` descarta *todos* os rótulos quando nenhum cabe
   com a fonte 10. Um nome longo de candidato/partido pode aparecer cortado na tela de confirmação.
4. **CProgressBar::Move** (10976) move apenas o retângulo externo; o retângulo interno (barra e texto) fica onde está.
   Latente (nada move uma barra de progresso).
5. **CTextBox::Move** (10957) armazena a nova posição antes de validá-la; após a exceção, o campo
   mantém a posição inválida.
6. **Build web: o texto de progresso em duas cores colapsa** (o slot 21 de CWasmScreen ignora o retângulo de recorte): a
   caixa de texto é preenchida com a cor da barra e o texto inteiro acaba preto (veja §8).
7. **Nível de correção de erros L** para os QR codes impressos do BU (o nível mais fraco, ~7 % de recuperação) — coincide com as impressões
   reais de 2024; danos à impressão térmica podem tornar uma parte ilegível (o texto do BU em papel continua sendo a referência).
8. O QR code na tela não fica centralizado quando 380 não é múltiplo de `width + 4` (a sobra vai para a esquerda
   e para baixo); cosmético. Para os tamanhos de símbolo do BU (53–93 módulos, §7), a sobra vai de 38 px (57 × 6 = 342) a
   89 px (97 × 3 = 291), então o código fica visivelmente à direita e acima do centro da sua caixa de 380 px.
9. Código morto: o ramo de apagar de `CTextField::Draw` (flag +60 nunca definida no binário) e a
   verificação órfã em `cqrcodeimagepaper.cpp:38`.

## 12. Tabela de mapeamento completa (todas as 85 funções de u16)

"executou" = observada em execução durante os votos gravados. D1 = destrutor de objeto completo (retorna `this`),
D0 = destrutor de deleção.

| # | func wasm | tamanho | executou | símbolo reconstruído | arquivo-fonte / destino |
|---|---|---|---|---|---|
| 1 | 1262 | 171 |  | `api::CTextFieldMT::CTextFieldMT(const SPoint&, const SharedIText&) (:25)` | src/uenux2/src/api/gui/ctextfieldmt.cpp |
| 2 | 1265 | 692 | sim | `api::CFormBuilder::AddBlinkingText(texto, pos, fonte, alinhamento) - inlines CTextFieldBlinking::CTextFieldBlinking (:39/:42)` | src/uenux2/src/api/gui/cformbuilder.u16.cpp + src/uenux2/src/api/gui/ctextfieldblinking.cpp (construtor) |
| 3 | 1918 | 201 |  | `api::CTextField::CTextField(const SPoint&, const SharedIText&, const SFont&, TColor, TColor) (:30)` | src/uenux2/src/api/gui/ctextfield.cpp |
| 4 | 2768 | 106 |  | `api::SRect::Adjusted(TPosition, TPosition, TPosition, TPosition) const` | src/uenux2/src/api/gui/primitives.u16.cpp (fragmento) |
| 5 | 2771 | 154 |  | `api::CTextFieldPaper::CTextFieldPaper(const SharedIText&, IPaper::EStyle) (:23)` | src/uenux2/src/api/gui/ctextfieldpaper.cpp |
| 6 | 2772 | 1582 |  | `api::CQRCodeImagePaper::MontaImagem(size_t, const std::string&) (:35/:49)` | src/uenux2/src/api/gui/cqrcodeimagepaper.cpp |
| 7 | 3658 | 303 | sim | `api::CTextFieldUpdate::CTextFieldUpdate(pos, texto, periodo, fonte[, 2, 1]) (:38)` | src/uenux2/src/api/gui/ctextfieldupdate.cpp |
| 8 | 3659 | 306 | sim | `api::CTextFieldDoubleLine::CTextFieldDoubleLine(...) (:40/:43/:46)` | src/uenux2/src/api/gui/ctextfielddoubleline.cpp |
| 9 | 3661 | 423 |  | `std::vector<uebyte>::insert(const_iterator, size_type, const uebyte&)` | biblioteca/helper inlinado: instanciação de fill-insert da libc++ (zonas de silêncio de CQRCodeImage::MontaImagem); anotado em src/uenux2/src/api/gui/cqrcodeimage.cpp |
| 10 | 3662 | 103 |  | `api::CQRCodeImage::CQRCodeImage(TPosition, const std::function<std::string()>&)` | src/uenux2/src/api/gui/cqrcodeimage.cpp |
| 11 | 3665 | 286 |  | `api::CStepsProgressBar::~CStepsProgressBar() (D1)` | src/uenux2/src/api/gui/cstepsprogressbar.cpp |
| 12 | 5495 | 176 |  | `api::CTextFieldUpdate::~CTextFieldUpdate() (D1)` | src/uenux2/src/api/gui/ctextfieldupdate.cpp |
| 13 | 5497 | 1944 |  | `api::CTextFieldMultiLine::GetTextLines() const (:116)` | src/uenux2/src/api/gui/ctextfieldmultiline.cpp |
| 14 | 5498 | 230 |  | `api::CTextFieldMultiLine::CTextFieldMultiLine(const SRect&, const SharedIText&, const SFont&[, 2, 1]) (:71)` | src/uenux2/src/api/gui/ctextfieldmultiline.cpp |
| 15 | 5499 | 2912 | sim | `api::CTextFieldDoubleLine::GetTextLines() const (:68)` | src/uenux2/src/api/gui/ctextfielddoubleline.cpp |
| 16 | 5502 | 13815 |  | `api::CQRCodeGuard::CQRCodeGuard(const std::string&) (:23) - inlines QRcode_encodeString(s, 0, QR_ECLEVEL_L, QR_MODE_8, 1)` | src/uenux2/src/api/gui/cqrcodeguard.cpp |
| 17 | 5504 | 11502 | sim | `api::CStepsProgressBar::Draw(IScreen&) const (inlines CalculaSegmentos)` | src/uenux2/src/api/gui/cstepsprogressbar.cpp |
| 18 | 5505 | 337 | sim | `std::vector<CStepsProgressBar::SSegmento>::__swap_out_circular_buffer` | biblioteca/helper inlinado: crescimento de vetor da libc++ para SSegmento de 28 bytes; anotado em src/uenux2/src/api/gui/cstepsprogressbar.cpp |
| 19 | 5506 | 836 | sim | `api::(anonymous)::DivideArea(const SRect&, size_t, TPosition, TPosition)` | src/uenux2/src/api/gui/cstepsprogressbar.cpp |
| 20 | 5507 | 504 | sim | `api::(anonymous)::TentaLayout(IScreen&, rotulos, area, TFontSize, TFuncaoLayout, SLayout&)` | src/uenux2/src/api/gui/cstepsprogressbar.cpp |
| 21 | 5509 | 74 |  | `api::CProgressBar::~CProgressBar() (D1)` | src/uenux2/src/api/gui/cprogressbar.cpp |
| 22 | 5545 | 2040 | sim | `api::CFormBuilder::AddProgressBar(uedword maximo, const SPoint&, const SPoint&, const std::string& formato) - inlines CProgressBar::CProgressBar (:52/:58/:67)` | src/uenux2/src/api/gui/cformbuilder.u16.cpp + src/uenux2/src/api/gui/cprogressbar.cpp (construtor) |
| 23 | 5546 | 534 | sim | `api::CFormBuilder::AddTextRect(const std::string&, const SRect&) - inlines CTextRectField::CTextRectField (:51)` | src/uenux2/src/api/gui/cformbuilder.u16.cpp + src/uenux2/src/api/gui/ctextrectfield.cpp (construtor) |
| 24 | 6059 | 63 |  | `shared D0 body (vtable param) of CQRCodeImage / CDataText<std::function<std::string()>>` | biblioteca/helper inlinado: corpo de merge-similar (~std::function em +8, free); descrito em src/uenux2/src/api/gui/cqrcodeimage.cpp |
| 25 | 6060 | 60 |  | `shared D1 body (vtable param) of CQRCodeImage / CDataText<std::function<std::string()>>` | biblioteca/helper inlinado: corpo de merge-similar (~std::function em +8); descrito em src/uenux2/src/api/gui/cqrcodeimage.cpp |
| 26 | 10902 | 61 |  | `api::CTextRectField::GetClassName() const` | src/uenux2/src/api/gui/ctextrectfield.cpp |
| 27 | 10903 | 12 |  | `api::CTextRectField::~CTextRectField() (D0, via 6021)` | src/uenux2/src/api/gui/ctextrectfield.cpp |
| 28 | 10904 | 12 |  | `api::CTextRectField::~CTextRectField() (D1, via 6022)` | src/uenux2/src/api/gui/ctextrectfield.cpp |
| 29 | 10905 | 215 |  | `api::CTextRectField::Rect() const (:71)` | src/uenux2/src/api/gui/ctextrectfield.cpp |
| 30 | 10906 | 185 |  | `api::CTextRectField::Draw(IScreen&) const` | src/uenux2/src/api/gui/ctextrectfield.cpp |
| 31 | 10912 | 61 |  | `api::CTextFieldUpdate::GetClassName() const` | src/uenux2/src/api/gui/ctextfieldupdate.cpp |
| 32 | 10913 | 14 | sim | `api::CTextFieldUpdate::Rect() const (:73)` | src/uenux2/src/api/gui/ctextfieldupdate.cpp |
| 33 | 10914 | 20 |  | `api::CTextFieldUpdate::Stop()` | src/uenux2/src/api/gui/ctextfieldupdate.cpp |
| 34 | 10915 | 20 |  | `api::CTextFieldUpdate::Start()` | src/uenux2/src/api/gui/ctextfieldupdate.cpp |
| 35 | 10916 | 177 | sim | `api::CTextFieldUpdate::Draw(IScreen&) const` | src/uenux2/src/api/gui/ctextfieldupdate.cpp |
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
| 49 | 10931 | 458 | sim | `api::CTextFieldDoubleLine::Draw(IScreen&) const` | src/uenux2/src/api/gui/ctextfielddoubleline.cpp |
| 50 | 10932 | 154 |  | `api::CTextFieldMT::~CTextFieldMT() (D0)` | src/uenux2/src/api/gui/ctextfieldmt.cpp |
| 51 | 10933 | 151 |  | `api::CTextFieldMT::~CTextFieldMT() (D1)` | src/uenux2/src/api/gui/ctextfieldmt.cpp |
| 52 | 10934 | 666 |  | `api::CTextFieldMT::Draw(IScreenMT&) const` | src/uenux2/src/api/gui/ctextfieldmt.cpp |
| 53 | 10940 | 21 |  | `api::CTextFieldBlinking::GetClassName() const` | src/uenux2/src/api/gui/ctextfieldblinking.cpp |
| 54 | 10941 | 154 |  | `api::CTextFieldBlinking::~CTextFieldBlinking() (D0)` | src/uenux2/src/api/gui/ctextfieldblinking.cpp |
| 55 | 10942 | 151 |  | `api::CTextFieldBlinking::~CTextFieldBlinking() (D1)` | src/uenux2/src/api/gui/ctextfieldblinking.cpp |
| 56 | 10943 | 14 |  | `api::CTextFieldBlinking::Rect() const (:70)` | src/uenux2/src/api/gui/ctextfieldblinking.cpp |
| 57 | 10944 | 20 |  | `api::CTextFieldBlinking::Stop()` | src/uenux2/src/api/gui/ctextfieldblinking.cpp |
| 58 | 10945 | 20 | sim | `api::CTextFieldBlinking::Start()` | src/uenux2/src/api/gui/ctextfieldblinking.cpp |
| 59 | 10946 | 74 | sim | `api::CTextFieldBlinking::Draw(IScreen&) const` | src/uenux2/src/api/gui/ctextfieldblinking.cpp |
| 60 | 10947 | 34 |  | `api::CTextField::GetClassName() const` | src/uenux2/src/api/gui/ctextfield.cpp |
| 61 | 10948 | 104 |  | `api::CTextField::~CTextField() (D0)` | src/uenux2/src/api/gui/ctextfield.cpp |
| 62 | 10950 | 101 |  | `api::CTextField::~CTextField() (D1)` | src/uenux2/src/api/gui/ctextfield.cpp |
| 63 | 10951 | 14 | sim | `api::CTextField::Rect() const (:55)` | src/uenux2/src/api/gui/ctextfield.cpp |
| 64 | 10952 | 114 | sim | `api::CTextField::Draw(IScreen&) const` | src/uenux2/src/api/gui/ctextfield.cpp |
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
| 77 | 10972 | 514 | sim | `api::(anonymous)::CabeLarguraIgual(IScreen&, rotulos, area, TFontSize, TPosition, TPosition, std::vector<SRect>&)` | src/uenux2/src/api/gui/cstepsprogressbar.cpp |
| 78 | 10973 | 13 |  | `api::CStepsProgressBar::~CStepsProgressBar() (D0)` | src/uenux2/src/api/gui/cstepsprogressbar.cpp |
| 79 | 10974 | 12 |  | `api::CProgressBar::Rect() const` | src/uenux2/src/api/gui/cprogressbar.cpp |
| 80 | 10975 | 61 |  | `api::CProgressBar::GetClassName() const` | src/uenux2/src/api/gui/cprogressbar.cpp |
| 81 | 10976 | 97 |  | `api::CProgressBar::Move(const SPoint&)` | src/uenux2/src/api/gui/cprogressbar.cpp |
| 82 | 10977 | 1914 | sim | `api::CProgressBar::Draw(IScreen&) const` | src/uenux2/src/api/gui/cprogressbar.cpp |
| 83 | 10978 | 13 |  | `api::CProgressBar::~CProgressBar() (D0)` | src/uenux2/src/api/gui/cprogressbar.cpp |
| 84 | 12303 | 12 |  | `api::CDataText<std::function<std::string()>>::~CDataText() (D0, via 6059)` | biblioteca/helper inlinado: destrutor trivial de template (ctextsource.h), resumido em src/uenux2/src/api/gui/cqrcodeimage.cpp |
| 85 | 12305 | 12 |  | `api::CDataText<std::function<std::string()>>::~CDataText() (D1, via 6060)` | biblioteca/helper inlinado: destrutor trivial de template (ctextsource.h), resumido em src/uenux2/src/api/gui/cqrcodeimage.cpp |

Funções relacionadas reconstruídas aqui, mas listadas em outras unidades: 3667 `CProgressBar::SetValor` (u37),
5508 `CProgressBar::Incrementa` (u07), 10971 `CabeLarguraProporcional` (u34), 3889 corpo compartilhado de `Rect` (u33),
13550 `CPreShowProgressBar::PreShow` (u04, inlina o construtor de CStepsProgressBar), 11805
`CTesteTeclado::StartState` (u26, inlina o construtor de CTextBox), 10937 / 10909 (lambdas de timer de
CTextFieldBlinking / CTextFieldUpdate, sem unidade).

## 13. Questões em aberto

* Nomes oficiais dos slots 3-6 de `IFormFieldBase` (`Start`/`Stop`/`RedrawIfDirty`/`SetForm` aqui), do
  helper inlinado de "notificar o formulário", do slot 2 de `IImage` (`GetImage`) e da maioria dos slots de `IScreen` (só
  `GetFontMetrics` é atestado); o slot 29 de `IScreen` (armazena duas cores em `CWasmScreen`) não está claro.
* Ordem dos três parâmetros `uedword` do construtor de CProgressBar (os dois chamadores passam 0, 0, max) e das
  duas cores de CTextBox; os nomes dos membros de `IPaper::EStyle`.
* Se a flag de "apagar o retângulo anterior" de `CTextField` (+60) é definida por código que não foi compilado no binário.
* O que a verificação órfã em `cqrcodeimagepaper.cpp:38` testava (provavelmente `larguraMaxima == 0`, erro 4950).
* Qual estado mostra as faixas "Número de cópias / acima do limite permitido" (presumivelmente
  `CLimiteCopiasBUAtingido`).
