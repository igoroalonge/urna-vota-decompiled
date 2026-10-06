# u15: criação do `CZip`, as peças do toolkit de GUI da urna (`api/gui`) e a pilha de contextos de erro fatal

A unidade u15 é um apanhado heterogêneo de 110 funções wasm que as ferramentas atribuíram a 22 arquivos originais:

* **`ecourna-lib/ecourna/api/compression/`** — o construtor de `CZip` e `CZip::CreateZipFile`
  (o gravador de ZIP por trás dos arquivos compactados de resultado `.jez`), além de `ICompressor::Add(map)`.
  O restante de `CZip` pertence à unidade u12.
* **`uenux2/src/api/gui/`** — widgets do próprio toolkit de GUI da urna: as caixas de dígitos das telas de
  votação (`CFramedText`, `CGrayedFramedText`, `CInputField<CFramedText>`, `CMaskedTextField`), campos de
  imagem (`CImageField`, `CDSImageField`, `CImageFieldUpdate`, `CFixedImage`), animações (`CMovie`,
  `CMovieField`), um widget de menu numerado (`CInputMenuField`, `CMenuItem`, `CMenuValidation`), o
  campo de LEDs do microterminal (`CLedFieldMT`), o cabeçalho de status de todas as telas
  (`CFormBuilder::AddStatusHeader`: relógio, bateria, rótulo **"TREINAMENTO"/"SIMULADO"**), as
  teclas de controle com rótulo (`CInteractiveFormBuilder::AddLabeledInputControl`), a lógica de foco
  dos formulários interativos (`CInteractiveForm<…>`), `CBmpConversor` (espelhamento/inversão do bitmap
  da impressão digital), `CApplication` (inicialização + os dois loops de "parada") e a **pilha de
  contextos da aplicação**, que decide o que a urna escreve na tela de erro fatal.
* **`uenux2/src/api/audio/alsa/cwavfile.cpp`** — apenas os dois destrutores de `CWavFile`.
* Sete funções que na verdade pertencem a **vota** / **comum** (um auxiliar da tela de zerésima, um
  hook de pré-exibição, três estados do mesário, o construtor de `comum::CInfoMTLCD`). As ferramentas as
  colocaram aqui porque uma função de `api/gui` está inlinada nelas.

32 das 110 funções executaram durante as duas votações gravadas (`analysis/runtime/*.functions.tsv`):
o cabeçalho de status, as caixas de dígitos, os rótulos das teclas, a lógica de foco e a pilha de
contextos estão em todas as telas do eleitor.

Fontes reconstruídas (todos os fragmentos levam o sufixo `.u15` porque outras unidades são donas de
outras funções dos mesmos arquivos):

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

## 1. Classes e como se relacionam (RTTI)

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

**Protocolo de campo** (toda vtable de `IFormField`; nomes inferidos onde marcado em `gui-common.u15.h`):
0/1 destrutores, 2 `Draw(MEDIA&) const`, 3 `Start()`, 4 `Stop()`, 5 `RedrawIfDirty(MEDIA&)` (func 4118),
6 `SetForm(FormControlBlock*)` (func 3037), 7 `GetClassName()` (o nome literal da classe, usado por
`CFormBuilder::Add` para gerar nomes únicos de campo, como `CImageField3`), 8 `Rect() const`,
9 `Move(const SPoint&)`; os campos de entrada acrescentam 10 `Read(IInput&)`, 11 `Clear()`, 12 `SetLength(size_t)`.
O idioma recorrente de *Invalidate* é `if (m_form && m_form->ativo) { m_dirty = true; m_form->RequestRedraw(); }`.

**Protocolo de formulário** (`IForm<MEDIA>`): 2 `Show()` (limpa a pilha global de formulários @1832632 / @1832892 e
ativa o formulário), 3 `ShowOnTop()`, 4 `Redraw()`, 5 `GetRenderForm()`, 6 `SetFocus(IInputField*)`,
7 `OnActivate()`. Os nomes de 2/3/7 são inferidos; outras unidades chamam o slot 2 de `Exibe()`.

**Coordenadas de tela.** A tela da urna é 640×480 (`ms_areaTeclas` @1577200 = {0,0,639,479}); o canvas
web é 1280×800, então o mock escala x por 2 e y por 5/3 (o log do simulador mostra
`fillText("TREINAMENTO", 640, 78)` para um rótulo desenhado em (320, 30)). `SPoint` é passado como um único i32
(x na parte baixa, y na alta), e `SRect` como um único i64.

---

## 2. Subsistemas e fluxo de controle

### 2.1 `CZip`: como um arquivo `.jez` é aberto (funcs 5193, 9543)

Arquivos `.jez` são arquivos compactados ZIP (minizip sobre zlib, apenas deflate; veja
`docs/libraries/compression-7zip-lzma-zlib.md`). Os dois gravadores são `comum::CGravadorWSQ` (func 5821:
imagens de impressões digitais, nível 0 "stored"; nível 2 no caminho que cria e fecha um arquivo compactado
sem nada dentro) e `comum::CGravadorLog` (func 11584: arquivos de log em `temp.jez`, nível 2 "default").
Só a 5821 chama a func 5193. A 11584 tem o construtor inlinado: ela mesma armazena a vtable de `CZip` e executa
`CreateZipFile` via `invoke_vi` (slot 6119 da tabela).

1. `CZip(path, level)` (5193) constrói `ICompressor` (um sinal de progresso), copia o caminho para +16, armazena
   `level` em +32, `m_zip = nullptr` (+28) e `m_modoAbertura = 0` (+36), e então chama
   `CreateZipFile()` dentro de um `try` (invoke_vi).
2. `CreateZipFile()` (9543): `ConvertToOpen()` (inlinada, czip.cpp:155) mapeia 0 → `APPEND_STATUS_CREATE`,
   1 → `APPEND_STATUS_ADDINZIP` e qualquer outro valor → `CBaseError<ECompressionError>(1040, "Modo de abertura do
   zip incorreto.")`. Em seguida o `zipOpen3` do minizip é inlinado (`fill_fopen64_filefunc`, slots
   8253..8259 da tabela). Se o handle for nulo: erro **1041** `"O arquivo {} não pode ser criado."` (czip.cpp:170).
   Como o modo é sempre 0, os 3 KB de código ADDINZIP inlinado (busca do end-of-central-directory, localizador
   zip64 "PK\6\7", recarga do diretório central em blocos de 4080 bytes) são código morto neste binário.
3. `ICompressor::Add(map<path,path>)` (9529) chama o método virtual `DoAdd(src, dst)` (slot 4, u12) para cada
   par, na ordem das chaves, e retorna `*this`. Só `CGravadorLog` o usa.

O simulador nunca alcança esses gravadores durante uma votação (eles executam no encerramento), então nenhum `.jez`
aparece nos dumps do MEMFS.

### 2.2 Inicialização: `CApplication::InitApplication` (11159, chamada uma vez por `main`)

1. Armazena a identidade da aplicação em variáveis estáticas: nome @1839168 ("VOTA"), descrição @1839180
   ("Software de Votação"), versão @1839192 ("10.23.0.1 - DESENVOLVIMENTO"), `ELogAplicativos` @1839204;
   redefine o int @1577212 (inicialmente -1) para -1.
2. Instala um locale global cujo `numpunct<char>` retorna **','** como ponto decimal (classe local
   `commaAsDecimalSeparator`, func 11156). `std::locale::global` é inlinado: `setlocale(LC_ALL, name)`
   quando o nome não é `"*"`.
3. Limpa a pilha de contextos da aplicação e empilha o contexto genérico de reserva
   `("Não é possível continuar a execução", "Erro inesperado", QR-code hint, ["Desligue e ligue a urna."])`.

### 2.3 A pilha de contextos da aplicação (funcs 676, 3682, 5555, 5557, 5558; 3684 na u33)

*Contexto da aplicação* = o texto mostrado quando uma operação falha. Passos arriscados abrem um guard:

```cpp
api::CApplicationContextGuard contexto(2 /*Actions*/, "", "Gerando boletim de urna na MI",
                                       "Ocorreu um erro durante a geração do boletim de urna na MI.");
```

* `CApplicationContext` (52 bytes): `+0 detalhe`, `+12 título`, `+24 mensagem`, `+36 vector<string> ações`,
  `+48 bool genérico`. A ordem dos campos foi verificada contra o consumidor: `vota::CThreadMonitor::vf3`
  (func 7710) formata `std::format("{} ({})", título, código do erro)` a partir do topo da pilha.
* `Actions` → texto (5555): 17 códigos, de "Desligue a urna." (0) até a instrução de ajuste do relógio da
  ADH (16); códigos desconhecidos dão `"Actions({}) - não reconhecida"`. O código 2 ("Desligue e ligue a
  urna." + "Se o erro persistir, substitua a urna.") é o que os gravadores do BU/resultado usam; o 4 acrescenta
  "…substitua a mídia de votação.". Tabela completa em `capplicationcontextstack.u15.h`.
* Regra de empilhamento (inlinada no guard e na func 3684): um contexto **genérico** só pode ser empilhado numa
  pilha vazia ou sobre outro contexto genérico; caso contrário o guard lança
  `CUeGuiError(5013, "O contexto não pôde ser criado")` (capplicationcontextstack.cpp:45).
* Entre os chamadores do push (3684) estão `CEleitorVotando::IniciaCiclo` e `CPedeDigital::StartState`;
  3682, 5555, 5558 e 676 executaram todas durante as votações gravadas.

### 2.4 O que acontece depois de um erro fatal (funcs 3683, 5566; fora da unidade: 5568, 7710, 11151)

Na urna real, `vota::CThreadMonitor::vf3` (7710) mostra a tela de erro e chama
`CApplication::ShowExceptionMsg` (func 5568), que inicia `std::async(std::launch::async, …)` executando
`EnterLoopBeeping()` — um **SOS em Morse** a 800 Hz (a func 3683 é a lambda; o loop está inlinado na
11151). As chamadas são `Beep(800, 10)` ×3, `Beep(800, 30)` ×3, `Beep(800, 10)` ×3. A unidade de duração de
`IBeep` é **10 ms**: o `CWasmBeep::vf1` web (func 8538) chama `js_wasm_beep_queue(frequency, durationMs)`
com `duração * 10`. Assim, os pontos duram 100 ms e os traços 300 ms (a proporção usual de 1:3 do Morse). As
esperas são em milissegundos brutos: `duração + 120` ms após cada bipe (130 / 150 ms), 360 ms após cada letra e
1.16 s entre repetições. O monitor então fica estacionado em `EnterLoopDoingNothing()` (5566): uma vez por segundo,
atualiza a tela (slot 37 de IScreen) e dorme, até `ms_encerrar` (@1839209) ou `ms_codigoSaida > 0`
(@1577212). Nada no binário escreve em `ms_encerrar`, e `ms_codigoSaida` só recebe o valor -1
(em `InitApplication`), então os dois loops são infinitos por projeto (o operador precisa desligar a urna).

**No build web este caminho é diferente:** a func 5568 sempre lança
`std::system_error("thread constructor failed")` (sem pthreads: o compilador provou que `pthread_create`
falha, então o corpo é `operator new` + throw), de modo que nem o SOS nem `EnterLoopDoingNothing` executam; e, se
executassem, o `std::this_thread::sleep_for` deles virou `if (byte@1584624 == 1) emscripten_sleep(ms)`,
que aborta sem Asyncify.

### 2.5 Cabeçalho de status e rótulo de modo (502, 5564, 4620, 7707)

`AddStatusHeader(campos)` (502) lê a largura da tela (slot 30 de IScreen) e adiciona, por bit:
`1` data/hora `"A DD/MM/YYYY hh:mm:ss"` em (5,5), atualizada a cada 500 ms (fonte de dados 3117);
`8` percentual da bateria `"{: >3}%"` (vazio quando não há leitura de bateria) em largura-70 ou -55;
`4` ícone da bateria (`CDSImageField` sobre `BatteryIconDataSource<Horizontal>`, timer de 1 s, ícones
`:/resource/images/bateria/img-*.jpg`) em (largura-55, 5) — inserido no vetor de campos **sem**
`Add()`, então não recebe nome único; `2` o rótulo de modo a 53 % da largura.
As telas normalmente passam `5` (relógio + ícone da bateria). Das 34 chamadas diretas, 32 passam 5, a func 1255
passa 7 e a func 7787 passa 4. Nenhum chamador liga o bit `8`, então o texto de percentual da bateria é
inalcançável neste build.

O rótulo de modo (`GetModoUrnaTexto`, 5564) é `"DEMONSTRAÇÃO"` se o bool @1839208 estiver ligado (nunca, neste
binário); senão, `"SIMULADO"` para a fase '2', `"TREINAMENTO"` para a fase '3' e nada para a fase oficial.
O int de fase @1577208 é copiado de `CEstadoGeral` +48 na inicialização (func 7787). Os hooks de
pré-exibição o desenham diretamente na tela (4620): `CPreShowFormVota::PreShow` (7707) em (340,5) e
`CPreShowProgressBar::PreShow` em (320,30). **Confirmado executando o simulador**
(`node tools/run/headless.mjs --scenario municipal-t1 --keys 9 --draw`): a primeira tela de votação desenha
`fillText("TREINAMENTO", 640, 78)` — todo cenário do simulador está na fase `te` (treinamento).

### 2.6 As caixas de dígitos das telas de votação (1385, 2780, 5534, 5536, 12387…12708)

`CFramedText(n, pos, font, align)` (1385) pede à tela o maior glifo da fonte (slot 2 de IScreen),
acrescenta um espaçamento de `size<=7 ? 1 : size/8` de cada lado e uma margem de altura de `size<=9 ? 2 : 2*(size/10)`,
e desloca `pos.x` para a esquerda pela largura total (align 1) ou pela metade dela (align 2 = centralizado).
`MaskText` (2780) lança `4917 "Texto [..] eh grande demais"` se o texto for mais longo que as caixas e depois
completa com brancos: as caixas vazias são preenchidas de branco com a moldura normal (cor 2); as digitadas recebem o
caractere e a moldura de cor 3. `CGrayedFramedText::MaskText` (5534, observada) preenche de cinza as caixas não usadas
(cor 5) — essas são as caixas de número das telas de candidato. A variante de entrada
(`CInputField<CFramedText>`, 12438) também desenha a moldura do cursor piscante; `SetLength` (12387) redimensiona
as caixas e limpa o texto.

### 2.7 Teclas de controle com rótulo e foco (653, 5565, 1401, 11074…11079)

`AddLabeledInputControl(teclas)` (653) aceita no máximo 3 pares `(tecla, rótulo)` (`4937 "O número máximo
de teclas suportado é {}"`), apenas `B`/`C`/`D` = BRANCO/CONFIRMA/CORRIGE (`4936 "Tecla de controle não
suportada '{}'"`). As posições são tiradas do fim de `{centro, esquerda, direita}`: a primeira tecla vai
à direita, a segunda à esquerda e a terceira ao centro (`GetLabeledKeyPos`, 5544: x = esquerda+23 / direita-19 / meio,
y = base+dy-47). Uma linha de 3 px é desenhada em y = 429. A função retorna o primeiro campo de entrada já presente
no formulário ou cria um `CInputFieldControl` invisível (5565) que termina nas teclas da máscara
(B=1, D=2, C=4, tabela @524056).

`CInteractiveForm::Show()` (11077, observada) limpa todas as entradas, foca a primeira, substitui a pilha de
formulários e, se a flag +88 do formulário estiver ligada, esvazia o teclado (`ClearKeyboardInput`, cinteractiveform.h:148,
`IInputKbd::Clear` → JS `wasm_input_clear`). `SetFocus(i)` (1401) move o timer de 600 ms que faz o cursor piscar
entre os campos.

### 2.8 O widget de menu `CInputMenuField` (1693, 2770, 3655, 3657, 3675, 5489–5492, 10893–10900)

Uma lista numerada para a tela do eleitor (menus "visualizar candidatos" do vota): os itens são dispostos em
colunas (uma nova coluna começa 20 px à direita do item mais largo quando o próximo passaria de
`min(screen height, top + alturaMáxima)`), exibidos como `"[%S] - %T"` (texto de busca, texto do item; expandido com um
`std::regex("%[ST]")` estático), seguidos da instrução `"Digite a sua opção: "` e de uma caixa de entrada com moldura
dimensionada pelo texto de busca mais longo. No máximo 99 itens (4933); um item que não cabe tem sua inserção desfeita e
lança 4934; textos de busca duplicados lançam 4929. `CMenuValidation::IsValid` (10900) aceita um texto de comprimento
completo igual ao texto de busca de um item visível ou um prefixo de um deles, e bipa (slot 6 de IBeep) caso contrário.
`Read()` (10894) é **bloqueante**: limpa o buffer do teclado, depois faz polling de `HasKey()` com sleeps de 5 ms,
destaca as correspondências durante a digitação e faz o item escolhido piscar no CONFIRMA (2 × 100 ms). Nada deste código
executou nas votações gravadas.

### 2.9 Imagens, animações, LEDs

* `CFixedImage(nome)` (2246, observada): nomes `":…"` vêm dos recursos empacotados (IResource; web:
  `CWasmResource`, que registra em log cada carga via `js_resource_log`); outros nomes são lidos com
  `CFile::ReadFileBinary` (ex.: fotos de eleitores); nome vazio → 4913.
* `CImageField` / `CDSImageField` / `CImageFieldUpdate`: imagem estática, imagem alimentada por uma fonte de
  dados observável (bateria), imagem relida por um timer (QR codes que alternam entre páginas a cada 15 s).
* `CMovie` (5526) exige pelo menos um quadro (4943). `CMovieField::Start` (11061) **não** mexe no
  timer de quadros: liga a flag de reprodução (+40), rebobina a animação para o quadro 0 e pede um redesenho.
  `Stop` (11060) para o timer (slot 3 de ITimer) e limpa a flag. O timer é criado pelo
  construtor (func 5543, outra unidade) com período 0 e não é iniciado ali; seu callback (func 11056)
  avança o quadro, invalida o campo e rearma o timer com o tempo restante do quadro
  (slot 5 de ITimer).
* `CLedFieldMT::Draw` (11050): operação de LED 0..3 → slots 8..11 de IScreenMT (≥ 4 lança 4940). O mock web
  reduz as quatro operações a ligado/desligado.
* `CBmpConversor::VerticalFlip` / `InvertColors` (3664/3663): troca de linhas e NOT bit a bit da imagem da
  impressão digital, chamadas pelos três `ProcessTick`s que leem impressões digitais (eleitor, mesário, operador).

### 2.10 Funções de vota / comum atribuídas incorretamente

* **3059** — corpo das telas de horário da zerésima (`CriaTelaConfirmaImpressaoZeresima`,
  `CriaTelaAntesHorarioZeresima`): cabeçalho de status, cabeçalho da zerésima (func 4160), `"RESUMO DA CORRESPONDÊNCIA:
  …"`, um **QR code rotativo com a identidade da urna** (tags `SERT`, `IDFL`, `SERI`, `NOME`, `UNFE`, `IDCA`
  … de `CEstadoGeral`, 148 px em (626,250), nova página a cada 15 s), três linhas de texto,
  `"Versão: 10.23.0.1 - DESENVOLVIMENTO"` e `"Dados: <8 Base64 chars of the package hash>"`.
* **7707** `CPreShowFormVota::PreShow`: limpeza da tela + rótulo de modo.
* **10588** `IConfirmaJustificativa::ProcessInput` (microterminal do mesário, fluxo de *justificativa*):
  CORRIGE → volta para `CPedeIdentidade`; CONFIRMA → se o eleitor foi identificado por **CPF** (tipo 2), a
  tela singleton "Não é permitido justificar com o CPF / utilize o número do título / CORRIGE: retornar";
  caso contrário, `CPedeAnoNascimento` (5418: "Digite o ANO de nascimento: ", 4 dígitos,
  "CORRIGE: cancelar", "CONFIRMA: justificar").
* **10624** `IEleitorImpedidoVotar::ProcessInput`: as telas de "eleitor não pode votar aqui" voltam para
  `CPedeIdentidade` no CONFIRMA ou no CORRIGE, dependendo de um campo próprio de cada tela (+20).
* **5904** `comum::CInfoMTLCD::CInfoMTLCD()`: ícone da bateria no LCD do microterminal.

---

## 3. Dados lidos e escritos

| o quê | onde | como |
|---|---|---|
| arquivos ZIP `.jez` (`log.jez`, `wsq*.jez`, `temp.jez`) | diretórios de resultado no encerramento | criados por `CZip` (5193/9543), preenchidos pelo `DoAdd` da u12 |
| imagens/animações de recurso `:/resource/images/…` | empacotadas no bundle `.data` web | `CFixedImage`, `CMovie` via `IResource` |
| fotos de eleitores, outros arquivos de imagem | armazenamento da urna | `CFile::ReadFileBinary` em `CFixedImage` |
| `CEstadoGeral` +48 (fase), +60 (correspondência), +168 (hash do pacote) | `eg.bin` (já carregado) | rótulo de modo, tela de zerésima |
| locale do processo | libc | `setlocale(LC_ALL, …)` com ',' como ponto decimal |

Nenhum SQL, nenhuma rede e nenhuma decodificação ASN.1 nesta unidade.

---

## 4. Particularidades do build web e observações sobre o wasm

* **A eliminação de argumentos mortos do Binaryen** removeu parâmetros constantes: `CApplicationContextStack::Push`
  (3684) perdeu o `this` (sempre @1839212) e seu resultado `bool`; `AddLabeledInputControl` perdeu seu
  `SRect` (sempre @1577200) e o `bool`; a lambda do SOS perdeu a frequência de 800 Hz e seu objeto de closure.
* **O inlining mudou nomes de lugar**: a 3059 recebeu o nome do construtor inlinado de `CImageFieldUpdate`,
  a 3675 o de `GetInstructionTextRect`, a 5904 o de `BatteryIconDataSource<Vertical>()` e as 10588/10624 o de
  `CInteractiveForm::Read`. As funções reais estão listadas na tabela de mapeamento.
* **ICF / merge-similar**: os destrutores 4021/5517 compartilham o corpo 6061 (vtables passadas como parâmetros),
  os destrutores de `CFixedImage` usam os corpos mesclados 3936/3937, `SetFocus` (1401) serve as duas
  instanciações de formulário, e o thunk de construtor de erro 406 alimenta o corpo compartilhado 710 de `CBaseError`.
* As strings estão em **Latin-1** no segmento de dados ("Não é possível continuar a execução" tem 35 bytes).
* `std::async` é compilado, mas nunca consegue iniciar uma thread (a 5568 lança exceção incondicionalmente).
* Pontos de chamada de `emscripten_sleep` nesta unidade: 5566 (1000 ms), 3683 (dur+120 e a pausa),
  10894 (polling de 5 ms + piscada de 2×100 ms). Todos protegidos pelo byte @1584624 (= 1); todos abortam se alcançados.

---

## 5. Relação com o BU (boletim de urna) e a zerésima

Esta unidade não monta o BU, mas três peças participam do fluxo de fim do dia/zerésima:

1. **Tratamento de erros em torno da geração do BU.** `vota::CGeraBU::StartState` (12110) envolve a escrita de
   `bu.dat` em `CApplicationContextGuard(2, "", "Gerando boletim de urna na MI", "Ocorreu um erro durante a
   geração do boletim de urna na MI.")`; `CGravaResultado` (12098), `CGeraRelatorios` (12105),
   `CCopiaResultadoParaMR` (12134) e `comum::AssinarUE` (1277) fazem o mesmo para os arquivos de resultado,
   as assinaturas e a cópia para a MR. Se algum passo lançar exceção, o monitor mostra
   `"<título> (<código>)"`, `mensagem` e as linhas de instrução de Actions 2
   ("Desligue e ligue a urna." / "Se o erro persistir, substitua a urna.").
2. **Arquivos compactados de resultado.** Os logs e as imagens de impressões digitais que acompanham o BU na mídia
   de resultado são arquivos ZIP (`.jez`) produzidos por `CZip` (§2.1).
3. **Telas de zerésima.** Antes do horário da zerésima, a urna mostra a tela da func 3059: o resumo da
   correspondência, o QR code de identidade da urna, a versão do software e o hash do pacote de dados, com a opção
   "Emissão do estado da urna" (limitada pelo número máximo de cópias impressas) e "Mais informações".

---

## 6. Tabela de mapeamento completa (110 funções)

Funções fora da unidade que foram reconstruídas para dar contexto: 3684 (`CApplicationContextStack::Push`,
pertencente à u33) e os corpos inlinados de 11151 (`EnterLoopBeeping`) e 5537/2779 (`DesenhaCaracter` /
`DesenhaMoldura`) estão descritos nos fragmentos, mas não são reivindicados aqui.

| func | tamanho | executou | nome nas ferramentas | símbolo reconstruído | reconstruído em (src/…) | arquivo original | conf. |
|---:|---:|:-:|---|---|---|---|---|
| 406 | 18 |  | `api_f406` | `ecourna::api::exception::CBaseError<api::EUeGuiError, SErrorLimits{4900, 5100}>::CBaseError` (thunk de construtor mesclado: passa a vtable @1529396 para o corpo 710) | src/uenux2/src/api/gui/gui-common.u15.h | uenux2/src/api/gui (auxiliar de EUeGuiError; caminho inferido) | alta |
| 502 | 2131 | ✓ | `api::CFormBuilder::AddStatusHeader` | `api::CFormBuilder::AddStatusHeader` | src/uenux2/src/api/gui/cformbuilder.u15.cpp | uenux2/src/api/gui/cformbuilder.cpp | alta |
| 653 | 1531 | ✓ | `api::CInteractiveFormBuilder::AddLabeledInputControl` | `api::CInteractiveFormBuilder::AddLabeledInputControl` | src/uenux2/src/api/gui/cinteractiveformbuilder.u15.cpp | uenux2/src/api/gui/cinteractiveformbuilder.cpp | alta |
| 676 | 1434 | ✓ | `api::CApplicationContextGuard::CApplicationContextGuard` | `api::CApplicationContextGuard::CApplicationContextGuard` | src/uenux2/src/api/gui/capplicationcontextstack.u15.cpp | uenux2/src/api/gui/capplicationcontextstack.cpp | alta |
| 1263 | 235 | ✓ | `api_f1263` | `api::CriaParIcones` | src/uenux2/src/api/gui/cpowerinformation.u15.cpp | uenux2/src/api/gui/cpowerinformation.cpp (caminho inferido) | baixa |
| 1385 | 260 | ✓ | `api::CFramedText::CFramedText` | `api::CFramedText::CFramedText` | src/uenux2/src/api/gui/cframedtext.u15.cpp | uenux2/src/api/gui/cframedtext.cpp | alta |
| 1401 | 372 | ✓ | `api_f1401` | `api::CInteractiveForm::SetFocus` | src/uenux2/src/api/gui/cinteractiveform.u15.h | uenux2/src/api/gui/cinteractiveform.h | média |
| 1693 | 1335 |  | `api::CInputMenuField::AddItem` | `api::CInputMenuField::AddItem` | src/uenux2/src/api/gui/cinputmenufield.u15.cpp | uenux2/src/api/gui/cinputmenufield.cpp | alta |
| 1915 | 70 |  | `api_f1915` | `api::SRect::MoveTo` | src/uenux2/src/api/gui/gui-common.u15.h | uenux2/src/api/gui/primitives.cpp (caminho inferido) | média |
| 1916 | 150 |  | `api_f1916` | `api::Union` | src/uenux2/src/api/gui/primitives.u15.cpp | uenux2/src/api/gui/primitives.cpp (caminho inferido) | média |
| 2242 | 99 |  | `api::CImageField::CImageField` | `api::CImageField::CImageField` | src/uenux2/src/api/gui/cimagefield.u15.cpp | uenux2/src/api/gui/cimagefield.cpp | alta |
| 2246 | 491 | ✓ | `api::CFixedImage::CFixedImage` | `api::CFixedImage::CFixedImage` | src/uenux2/src/api/gui/cfixedimage.u15.cpp | uenux2/src/api/gui/cfixedimage.cpp | alta |
| 2770 | 219 |  | `api_f2770` | `api::CInputMenuField::AtualizaLayout` | src/uenux2/src/api/gui/cinputmenufield.u15.cpp | uenux2/src/api/gui/cinputmenufield.cpp | baixa |
| 2778 | 368 |  | `api::CDSImageField::vf0@2778` | `api::CDSImageField::~CDSImageField` | src/uenux2/src/api/gui/cdsimagefield.u15.cpp | uenux2/src/api/gui/cdsimagefield.cpp | alta |
| 2780 | 766 | ✓ | `api::CFramedText::MaskText` | `api::CFramedText::MaskText` | src/uenux2/src/api/gui/cframedtext.u15.cpp | uenux2/src/api/gui/cframedtext.cpp | alta |
| 3059 | 2801 | ✓ | `api::CImageFieldUpdate::CImageFieldUpdate` | `vota::(anonymous namespace)::adicionaBlocoMensagem` | src/uenux2/src/app/vota/u15-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | média |
| 3655 | 648 |  | `api::CMenuItem::SetSearchText` | `api::CMenuItem::SetSearchText` | src/uenux2/src/api/gui/cinputmenufield.u15.cpp | uenux2/src/api/gui/cinputmenufield.cpp | alta |
| 3657 | 215 |  | `api::CMenuItem::UpdateRect` | `api::CMenuItem::UpdateRect` | src/uenux2/src/api/gui/cinputmenufield.u15.cpp | uenux2/src/api/gui/cinputmenufield.cpp | alta |
| 3663 | 234 |  | `api::CBmpConversor::InvertColors` | `api::CBmpConversor::InvertColors` | src/uenux2/src/api/gui/cbmpconversor.u15.cpp | uenux2/src/api/gui/cbmpconversor.cpp | alta |
| 3664 | 274 |  | `api::CBmpConversor::VerticalFlip` | `api::CBmpConversor::VerticalFlip` | src/uenux2/src/api/gui/cbmpconversor.u15.cpp | uenux2/src/api/gui/cbmpconversor.cpp | alta |
| 3675 | 1152 |  | `api::CInputMenuField::GetInstructionTextRect` | `api::CInteractiveFormBuilder::AddInputMenu` | src/uenux2/src/api/gui/cinteractiveformbuilder.u15.cpp | uenux2/src/api/gui/cinteractiveformbuilder.cpp (caminho inferido) | baixa |
| 3682 | 333 | ✓ | `vota_f3682` | `api::CApplicationContext::CApplicationContext(Actions,string,string,string)` | src/uenux2/src/api/gui/capplicationcontextstack.u15.cpp | uenux2/src/api/gui/capplicationcontextstack.cpp | média |
| 3683 | 253 |  | `api::CApplication::EnterLoopBeeping()::(lambda)::operator()` | `api::CApplication::EnterLoopBeeping()::(lambda)::operator()` | src/uenux2/src/api/gui/capplication.u15.cpp | uenux2/src/api/gui/capplication.cpp | alta |
| 4021 | 17 |  | `api::IInputField<api::IScreen>::vf0` | `api::IInputField<api::IScreen>::~IInputField` | src/uenux2/src/api/gui/cinputmenufield.u15.cpp | uenux2/src/api/gui/iinputfield.h (caminho inferido) | alta |
| 4620 | 229 | ✓ | `api_f4620` | `api::CFormBuilder::DesenhaModoUrna` | src/uenux2/src/api/gui/cformbuilder.u15.cpp | uenux2/src/api/gui/cformbuilder.cpp (caminho inferido) | baixa |
| 5193 | 238 |  | `ecourna::api::compression::CZip::CZip` | `ecourna::api::compression::CZip::CZip` | src/ecourna/api/compression/czip.u15.cpp | ecourna-lib/ecourna/api/compression/czip.cpp | alta |
| 5345 | 53 |  | `api::CWavFile::vf0` | `api::CWavFile::~CWavFile` | src/uenux2/src/api/audio/alsa/cwavfile.u15.cpp | uenux2/src/api/audio/alsa/cwavfile.cpp | alta |
| 5416 | 20 |  | `vota_f5416` | `vota::TipoIdentificacaoEleitor` | src/uenux2/src/app/vota/u15-foreign-fragments.cpp | uenux2/src/app/vota/operador (caminho inferido) | baixa |
| 5418 | 1000 |  | `vota_f5418` | `vota::CPedeAnoNascimento::GetInst` | src/uenux2/src/app/vota/u15-foreign-fragments.cpp | uenux2/src/app/vota/operador/confirmaidentidade/cpedeanonascimento.cpp (caminho inferido) | média |
| 5489 | 333 |  | `api_f5489` | `api::CInputMenuField::SelecionaPorTextoBusca` | src/uenux2/src/api/gui/cinputmenufield.u15.cpp | uenux2/src/api/gui/cinputmenufield.cpp | baixa |
| 5490 | 156 |  | `api_f5490` | `api::CMenuItem::~CMenuItem` | src/uenux2/src/api/gui/cinputmenufield.u15.cpp | uenux2/src/api/gui/cinputmenufield.cpp | média |
| 5491 | 167 |  | `api::CInputMenuField::vf0` | `api::CInputMenuField::~CInputMenuField` | src/uenux2/src/api/gui/cinputmenufield.u15.cpp | uenux2/src/api/gui/cinputmenufield.cpp | alta |
| 5492 | 1445 |  | `api_f5492` | `api::CMenuItem::GetDisplayText` | src/uenux2/src/api/gui/cinputmenufield.u15.cpp | uenux2/src/api/gui/cinputmenufield.cpp | baixa |
| 5517 | 17 |  | `api::IInputField<api::IScreenMT>::vf0` | `api::IInputField<api::IScreenMT>::~IInputField` | src/uenux2/src/api/gui/cinputmenufield.u15.cpp | uenux2/src/api/gui/iinputfield.h (caminho inferido) | alta |
| 5524 | 176 |  | `api::CMovieField::vf0` | `api::CMovieField::~CMovieField` | src/uenux2/src/api/gui/cmoviefield.u15.cpp | uenux2/src/api/gui/cmoviefield.cpp | alta |
| 5526 | 148 |  | `api::CMovie::CMovie` | `api::CMovie::CMovie` | src/uenux2/src/api/gui/cmovie.u15.cpp | uenux2/src/api/gui/cmovie.cpp | alta |
| 5531 | 175 |  | `api::CImageFieldUpdate::vf0` | `api::CImageFieldUpdate::~CImageFieldUpdate` | src/uenux2/src/api/gui/cimagefieldupdate.u15.cpp | uenux2/src/api/gui/cimagefieldupdate.cpp | alta |
| 5533 | 201 |  | `api_f5533` | `api::CImageField::SetImage` | src/uenux2/src/api/gui/cimagefield.u15.cpp | uenux2/src/api/gui/cimagefield.cpp (caminho inferido) | média |
| 5534 | 269 | ✓ | `api::CGrayedFramedText::MaskText` | `api::CGrayedFramedText::MaskText` | src/uenux2/src/api/gui/cgrayedframedtext.u15.cpp | uenux2/src/api/gui/cgrayedframedtext.cpp | alta |
| 5536 | 164 |  | `api_f5536` | `api::CFramedText::PreencheCaixa` | src/uenux2/src/api/gui/cgrayedframedtext.u15.cpp | uenux2/src/api/gui/cframedtext.cpp (caminho inferido) | baixa |
| 5544 | 559 |  | `api::CFormBuilder::GetLabeledKeyPos` | `api::CFormBuilder::GetLabeledKeyPos` | src/uenux2/src/api/gui/cformbuilder.u15.cpp | uenux2/src/api/gui/cformbuilder.cpp | alta |
| 5554 | 561 |  | `api_f5554` | `std::vector<api::CApplicationContext>::__push_back_slow_path` | biblioteca/auxiliar inlinado | libc++ <vector> (instanciação) | alta |
| 5555 | 3197 | ✓ | `vota_f5555` | `api::ActionsToText` | src/uenux2/src/api/gui/capplicationcontextstack.u15.cpp | uenux2/src/api/gui/capplicationcontextstack.cpp (caminho inferido) | média |
| 5557 | 169 |  | `vota_f5557` | `api::CApplicationContext::CApplicationContext` | src/uenux2/src/api/gui/capplicationcontextstack.u15.cpp | uenux2/src/api/gui/capplicationcontextstack.cpp (caminho inferido) | média |
| 5558 | 349 | ✓ | `vota_f5558` | `api::CApplicationContext::CApplicationContext(string,string)` | src/uenux2/src/api/gui/capplicationcontextstack.u15.cpp | uenux2/src/api/gui/capplicationcontextstack.cpp (caminho inferido) | média |
| 5564 | 208 | ✓ | `api_f5564` | `api::CFormBuilder::GetModoUrnaTexto` | src/uenux2/src/api/gui/cformbuilder.u15.cpp | uenux2/src/api/gui/cformbuilder.cpp | média |
| 5565 | 193 | ✓ | `api_f5565` | `api::CInputFieldControlBase<api::IScreen>::CInputFieldControlBase` | src/uenux2/src/api/gui/cinteractiveformbuilder.u15.cpp | uenux2/src/api/gui/cinputfieldcontrol.h (caminho inferido) | média |
| 5566 | 138 |  | `api::CApplication::EnterLoopDoingNothing` | `api::CApplication::EnterLoopDoingNothing` | src/uenux2/src/api/gui/capplication.u15.cpp | uenux2/src/api/gui/capplication.cpp | alta |
| 5904 | 656 | ✓ | `api::BatteryIconDataSource<api::CPowerInformation::IconOr…` | `comum::CInfoMTLCD::CInfoMTLCD` | src/uenux2/src/app/comum/cinfomtlcd.u15.cpp | uenux2/src/app/comum/cinfomtlcd.cpp | média |
| 6061 | 209 |  | `api_f6061` | `api::IInputField<MEDIA>::~IInputField (shared body)` | src/uenux2/src/api/gui/cinputmenufield.u15.cpp | uenux2/src/api/gui/iinputfield.h (caminho inferido) | média |
| 7707 | 61 | ✓ | `vota::CPreShowFormVota::vf2` | `vota::CPreShowFormVota::PreShow` | src/uenux2/src/app/vota/u15-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/comum/cpreshowformvota.cpp (caminho inferido) | média |
| 8554 | 12 |  | `api::CFixedImage::vf1` | `api::CFixedImage::~CFixedImage (deleting)` | src/uenux2/src/api/gui/cfixedimage.u15.cpp | uenux2/src/api/gui/cfixedimage.cpp | alta |
| 8563 | 12 |  | `api::CFixedImage::vf0` | `api::CFixedImage::~CFixedImage` | src/uenux2/src/api/gui/cfixedimage.u15.cpp | uenux2/src/api/gui/cfixedimage.cpp | alta |
| 9529 | 105 |  | `ecourna::api::compression::ICompressor::Add` | `ecourna::api::compression::ICompressor::Add` | src/ecourna/api/compression/icompressor.u15.cpp | ecourna-lib/ecourna/api/compression/icompressor.cpp | média |
| 9543 | 4447 |  | `ecourna::api::compression::CZip::CreateZipFile` | `ecourna::api::compression::CZip::CreateZipFile` | src/ecourna/api/compression/czip.u15.cpp | ecourna-lib/ecourna/api/compression/czip.cpp | alta |
| 10238 | 13 |  | `api::CWavFile::vf1` | `api::CWavFile::~CWavFile (deleting)` | src/uenux2/src/api/audio/alsa/cwavfile.u15.cpp | uenux2/src/api/audio/alsa/cwavfile.cpp | alta |
| 10588 | 1112 |  | `api::CInteractiveForm<api::IScreenMT, api::IInputMT>::Rea…` | `vota::IConfirmaJustificativa::ProcessInput` | src/uenux2/src/app/vota/u15-foreign-fragments.cpp | uenux2/src/app/vota/operador/justificativa/iconfirmajustificativa.cpp | média |
| 10624 | 167 |  | `api::CInteractiveForm<api::IScreenMT, api::IInputMT>::Rea…` | `vota::IEleitorImpedidoVotar::ProcessInput` | src/uenux2/src/app/vota/u15-foreign-fragments.cpp | uenux2/src/app/vota/operador/leidentidade/ieleitorimpedidovotar.cpp (caminho inferido) | média |
| 10893 | 61 |  | `api::CInputMenuField::vf7` | `api::CInputMenuField::GetClassName` | src/uenux2/src/api/gui/cinputmenufield.u15.h | uenux2/src/api/gui/cinputmenufield.cpp | alta |
| 10894 | 783 |  | `api::CInputMenuField::Read` | `api::CInputMenuField::Read` | src/uenux2/src/api/gui/cinputmenufield.u15.cpp | uenux2/src/api/gui/cinputmenufield.cpp | alta |
| 10895 | 255 |  | `api::CInputMenuField::Move` | `api::CInputMenuField::Move` | src/uenux2/src/api/gui/cinputmenufield.u15.cpp | uenux2/src/api/gui/cinputmenufield.cpp | alta |
| 10896 | 61 |  | `api::CInputMenuField::vf8` | `api::CInputMenuField::Rect` | src/uenux2/src/api/gui/cinputmenufield.u15.cpp | uenux2/src/api/gui/cinputmenufield.cpp | média |
| 10897 | 607 |  | `api::CInputMenuField::vf2` | `api::CInputMenuField::Draw` | src/uenux2/src/api/gui/cinputmenufield.u15.cpp | uenux2/src/api/gui/cinputmenufield.cpp | alta |
| 10898 | 13 |  | `api::CInputMenuField::vf1` | `api::CInputMenuField::~CInputMenuField (deleting)` | src/uenux2/src/api/gui/cinputmenufield.u15.cpp | uenux2/src/api/gui/cinputmenufield.cpp | alta |
| 10900 | 623 |  | `api::CMenuValidation::matchesExactly` | `api::CMenuValidation::IsValid` (slot 2; os srclocs só nomeiam as funções inlinadas `matchesExactly`/`matchesPartially`) | src/uenux2/src/api/gui/cinputmenufield.u15.cpp | uenux2/src/api/gui/cinputmenufield.cpp | média |
| 11006 | 49 |  | `api::IFormImpressao<api::IPaperRelatorios>::GetRenderForm` | `api::IFormImpressao<api::IPaperRelatorios>::GetRenderForm` | src/uenux2/src/api/gui/cpaperformbuilder.u15.h | uenux2/src/api/gui/cpaperformbuilder.h | alta |
| 11028 | 17 |  | `api::CInteractiveForm<api::IScreenMT, api::IInputMT>::vf7` | `api::CInteractiveForm<api::IScreenMT, api::IInputMT>::OnActivate` | src/uenux2/src/api/gui/cinteractiveform.u15.h | uenux2/src/api/gui/cinteractiveform.h | média |
| 11029 | 136 |  | `api::CInteractiveForm<api::IScreenMT, api::IInputMT>::Cle…` | `api::CInteractiveForm<api::IScreenMT, api::IInputMT>::ShowOnTop` | src/uenux2/src/api/gui/cinteractiveform.u15.h | uenux2/src/api/gui/cinteractiveform.h | média |
| 11030 | 136 |  | `api::CInteractiveForm<api::IScreenMT, api::IInputMT>::Cle…` | `api::CInteractiveForm<api::IScreenMT, api::IInputMT>::Show` | src/uenux2/src/api/gui/cinteractiveform.u15.h | uenux2/src/api/gui/cinteractiveform.h | média |
| 11031 | 50 |  | `api::CInteractiveForm<api::IScreenMT, api::IInputMT>::vf1` | `api::CInteractiveForm<api::IScreenMT, api::IInputMT>::~CInteractiveForm (deleting)` | src/uenux2/src/api/gui/cinteractiveform.u15.h | uenux2/src/api/gui/cinteractiveform.h | alta |
| 11032 | 47 |  | `api::CInteractiveForm<api::IScreenMT, api::IInputMT>::vf0` | `api::CInteractiveForm<api::IScreenMT, api::IInputMT>::~CInteractiveForm` | src/uenux2/src/api/gui/cinteractiveform.u15.h | uenux2/src/api/gui/cinteractiveform.h | alta |
| 11050 | 497 |  | `api::CLedFieldMT::Draw` | `api::CLedFieldMT::Draw` | src/uenux2/src/api/gui/cledfieldmt.u15.cpp | uenux2/src/api/gui/cledfieldmt.cpp | alta |
| 11059 | 17 |  | `api::CMovieField::vf7` | `api::CMovieField::GetClassName` | src/uenux2/src/api/gui/cmoviefield.u15.cpp | uenux2/src/api/gui/cmoviefield.cpp | alta |
| 11060 | 29 | ✓ | `api::CMovieField::vf4` | `api::CMovieField::Stop` | src/uenux2/src/api/gui/cmoviefield.u15.cpp | uenux2/src/api/gui/cmoviefield.cpp | média |
| 11061 | 77 | ✓ | `api::CMovieField::vf3` | `api::CMovieField::Start` | src/uenux2/src/api/gui/cmoviefield.u15.cpp | uenux2/src/api/gui/cmoviefield.cpp | média |
| 11062 | 97 |  | `api::CMovieField::vf9` | `api::CMovieField::Move` | src/uenux2/src/api/gui/cmoviefield.u15.cpp | uenux2/src/api/gui/cmoviefield.cpp | média |
| 11063 | 109 |  | `api::CMovieField::Rect` | `api::CMovieField::Rect` | src/uenux2/src/api/gui/cmoviefield.u15.cpp | uenux2/src/api/gui/cmoviefield.cpp | alta |
| 11064 | 30 | ✓ | `api::CMovieField::vf2` | `api::CMovieField::Draw` | src/uenux2/src/api/gui/cmoviefield.u15.cpp | uenux2/src/api/gui/cmoviefield.cpp | média |
| 11065 | 13 |  | `api::CMovieField::vf1` | `api::CMovieField::~CMovieField (deleting)` | src/uenux2/src/api/gui/cmoviefield.u15.cpp | uenux2/src/api/gui/cmoviefield.cpp | alta |
| 11074 | 17 | ✓ | `api::CInteractiveForm<api::IScreen, api::IInputKbd>::vf7` | `api::CInteractiveForm<api::IScreen, api::IInputKbd>::OnActivate` | src/uenux2/src/api/gui/cinteractiveform.u15.h | uenux2/src/api/gui/cinteractiveform.h | média |
| 11075 | 136 |  | `api::CInteractiveForm<api::IScreen, api::IInputKbd>::Clea…` | `api::CInteractiveForm<api::IScreen, api::IInputKbd>::ShowOnTop` | src/uenux2/src/api/gui/cinteractiveform.u15.h | uenux2/src/api/gui/cinteractiveform.h | média |
| 11077 | 136 | ✓ | `api::CInteractiveForm<api::IScreen, api::IInputKbd>::Clea…` | `api::CInteractiveForm<api::IScreen, api::IInputKbd>::Show` | src/uenux2/src/api/gui/cinteractiveform.u15.h | uenux2/src/api/gui/cinteractiveform.h | média |
| 11078 | 50 |  | `api::CInteractiveForm<api::IScreen, api::IInputKbd>::vf1` | `api::CInteractiveForm<api::IScreen, api::IInputKbd>::~CInteractiveForm (deleting)` | src/uenux2/src/api/gui/cinteractiveform.u15.h | uenux2/src/api/gui/cinteractiveform.h | alta |
| 11079 | 47 |  | `api::CInteractiveForm<api::IScreen, api::IInputKbd>::vf0` | `api::CInteractiveForm<api::IScreen, api::IInputKbd>::~CInteractiveForm` | src/uenux2/src/api/gui/cinteractiveform.u15.h | uenux2/src/api/gui/cinteractiveform.h | alta |
| 11085 | 73 |  | `api::CImageFieldUpdate::vf7` | `api::CImageFieldUpdate::GetClassName` | src/uenux2/src/api/gui/cimagefieldupdate.u15.cpp | uenux2/src/api/gui/cimagefieldupdate.cpp | alta |
| 11086 | 10 |  | `api::CImageFieldUpdate::vf1` | `api::CImageFieldUpdate::~CImageFieldUpdate (deleting)` | src/uenux2/src/api/gui/cimagefieldupdate.u15.cpp | uenux2/src/api/gui/cimagefieldupdate.cpp | alta |
| 11087 | 97 |  | `api::CImageFieldUpdate::Rect` | `api::CImageFieldUpdate::Rect` | src/uenux2/src/api/gui/cimagefieldupdate.u15.cpp | uenux2/src/api/gui/cimagefieldupdate.cpp | alta |
| 11088 | 20 |  | `api::CImageFieldUpdate::vf4` | `api::CImageFieldUpdate::Stop` | src/uenux2/src/api/gui/cimagefieldupdate.u15.cpp | uenux2/src/api/gui/cimagefieldupdate.cpp | média |
| 11089 | 20 |  | `api::CImageFieldUpdate::vf3` | `api::CImageFieldUpdate::Start` | src/uenux2/src/api/gui/cimagefieldupdate.u15.cpp | uenux2/src/api/gui/cimagefieldupdate.cpp | média |
| 11090 | 78 |  | `api::CImageFieldUpdate::vf2` | `api::CImageFieldUpdate::Draw` | src/uenux2/src/api/gui/cimagefieldupdate.u15.cpp | uenux2/src/api/gui/cimagefieldupdate.cpp | média |
| 11091 | 61 |  | `api::CDSImageField::vf7` | `api::CDSImageField::GetClassName` | src/uenux2/src/api/gui/cdsimagefield.u15.cpp | uenux2/src/api/gui/cdsimagefield.cpp | alta |
| 11092 | 80 | ✓ | `api::CDSImageField::vf2` | `api::CDSImageField::Update (thunk)` | src/uenux2/src/api/gui/cdsimagefield.u15.cpp | uenux2/src/api/gui/cdsimagefield.cpp | média |
| 11093 | 77 |  | `api::CDSImageField::vf10` | `api::CDSImageField::Update` | src/uenux2/src/api/gui/cdsimagefield.u15.cpp | uenux2/src/api/gui/cdsimagefield.cpp | média |
| 11094 | 18 |  | `api::CDSImageField::vf1@11094` | `api::CDSImageField::~CDSImageField (deleting thunk)` | src/uenux2/src/api/gui/cdsimagefield.u15.cpp | uenux2/src/api/gui/cdsimagefield.cpp | alta |
| 11095 | 13 |  | `api::CDSImageField::vf1@11095` | `api::CDSImageField::~CDSImageField (deleting)` | src/uenux2/src/api/gui/cdsimagefield.u15.cpp | uenux2/src/api/gui/cdsimagefield.cpp | alta |
| 11096 | 10 |  | `api::CDSImageField::vf0@11096` | `api::CDSImageField::~CDSImageField (thunk)` | src/uenux2/src/api/gui/cdsimagefield.u15.cpp | uenux2/src/api/gui/cdsimagefield.cpp | alta |
| 11097 | 17 |  | `api::CImageField::vf7` | `api::CImageField::GetClassName` | src/uenux2/src/api/gui/cimagefield.u15.cpp | uenux2/src/api/gui/cimagefield.cpp | alta |
| 11098 | 128 |  | `api::CImageField::vf1` | `api::CImageField::~CImageField (deleting)` | src/uenux2/src/api/gui/cimagefield.u15.cpp | uenux2/src/api/gui/cimagefield.cpp | alta |
| 11099 | 125 |  | `api::CImageField::vf0` | `api::CImageField::~CImageField` | src/uenux2/src/api/gui/cimagefield.u15.cpp | uenux2/src/api/gui/cimagefield.cpp | alta |
| 11101 | 118 |  | `api::CImageField::Rect` | `api::CImageField::Rect` | src/uenux2/src/api/gui/cimagefield.u15.cpp | uenux2/src/api/gui/cimagefield.cpp | alta |
| 11102 | 81 | ✓ | `api::CImageField::vf2` | `api::CImageField::Draw` | src/uenux2/src/api/gui/cimagefield.u15.cpp | uenux2/src/api/gui/cimagefield.cpp | média |
| 11156 | 4 |  | `api::CApplication::InitApplication(std::basic_string<char…` | `api::CApplication::InitApplication::commaAsDecimalSeparator::do_decimal_point` | src/uenux2/src/api/gui/capplication.u15.cpp | uenux2/src/api/gui/capplication.cpp | alta |
| 11159 | 1536 | ✓ | `api::CApplication::InitApplication` | `api::CApplication::InitApplication` | src/uenux2/src/api/gui/capplication.u15.cpp | uenux2/src/api/gui/capplication.cpp | alta |
| 12387 | 318 |  | `api::CInputField<api::CFramedText>::SetLength` | `api::CInputField<api::CFramedText>::SetLength` | src/uenux2/src/api/gui/cinputfield.u15.h | uenux2/src/api/gui/cinputfield.h | alta |
| 12402 | 70 | ✓ | `api::CInputField<api::CFramedText>::vf9` | `api::CInputField<api::CFramedText>::Move` | src/uenux2/src/api/gui/cinputfield.u15.h | uenux2/src/api/gui/cinputfield.h | média |
| 12405 | 12 | ✓ | `api::CInputField<api::CFramedText>::vf8` | `api::CInputField<api::CFramedText>::Rect` | src/uenux2/src/api/gui/cinputfield.u15.h | uenux2/src/api/gui/cinputfield.h | média |
| 12411 | 17 | ✓ | `api::CInputField<api::CFramedText>::vf7` | `api::CInputField<api::CFramedText>::GetClassName` | src/uenux2/src/api/gui/cinputfield.u15.h | uenux2/src/api/gui/cinputfield.h | alta |
| 12438 | 83 | ✓ | `api::CInputField<api::CFramedText>::vf2` | `api::CInputField<api::CFramedText>::Draw` | src/uenux2/src/api/gui/cinputfield.u15.h | uenux2/src/api/gui/cinputfield.h | média |
| 12593 | 82 | ✓ | `api::CMaskedTextField<api::CFramedText>::vf2` | `api::CMaskedTextField<api::CFramedText>::Draw` | src/uenux2/src/api/gui/cframedtext.u15.h | uenux2/src/api/gui/cmaskedtextfield.h (caminho inferido) | média |
| 12708 | 82 | ✓ | `api::CMaskedTextField<api::CGrayedFramedText>::vf2` | `api::CMaskedTextField<api::CGrayedFramedText>::Draw` | src/uenux2/src/api/gui/cframedtext.u15.h | uenux2/src/api/gui/cmaskedtextfield.h (caminho inferido) | média |

---

## 7. Código estranho ou arriscado

1. **`CInputMenuField::Read` aborta o simulador** (func 10894). Primeiro descarta as teclas pendentes
   (`IInput::Clear` → JS `wasm_input_clear`), depois espera uma tecla com
   `while (!HasKey()) sleep_for(5 ms)` e faz o item escolhido piscar com dois sleeps de 100 ms. Neste build
   cada sleep é `emscripten_sleep`, que o glue implementa como `abort()`. Qualquer tela que leia um
   `CInputMenuField` (os menus "visualizar candidatos": `CMenuVisualizarCandidatos`,
   `CMenuFiltrarCandidatosPorPartido/Cargo`) faria portanto o módulo quebrar na primeira leitura: o
   byte de guarda @1584624 vale 1 no segmento de dados e nenhuma instrução escreve nele, e, depois do
   `Clear()`, nenhuma tecla pode chegar durante a chamada síncrona, então o primeiro `HasKey()` é falso. Não
   alcançado nas sessões gravadas, e não foi estabelecido se o fluxo web consegue sequer chegar a esses menus.
   Média (só no simulador; na urna isto é uma leitura bloqueante normal na thread do eleitor).
2. **O caminho de erro fatal é diferente no build web** (funcs 5568 → 3683/11151 → 5566). Na urna, o
   monitor mostra o erro, bipa SOS para sempre e fica estacionado em `EnterLoopDoingNothing`. No wasm,
   `ShowExceptionMsg` lança `std::system_error("thread constructor failed")` incondicionalmente, então o
   SOS e o loop de estacionamento nunca executam, e uma segunda exceção escapa do handler do monitor. Mesmo se
   alcançado, `EnterLoopDoingNothing` aborta no seu primeiro `emscripten_sleep(1000)`. Média (simulador).
3. **Espera ativa em `EnterLoopBeeping`** (inlinado na 11151): quando nenhum singleton de `IBeep` está registrado, o
   loop `do { if (exists<IBeep>()) {…sleep…} } while (…)` não tem sleep nenhum, então gira a 100 % de CPU
   para sempre. Baixa (urna: só se o bipador estiver ausente; o build web nunca inicia esta tarefa, veja o item 2).
4. **Flags que nunca são escritas** (@1839208 `ms_demonstracao`, @1839209 `ms_encerrar` e
   @1577212 `ms_codigoSaida`, que só recebe o valor -1). O rótulo de status "DEMONSTRAÇÃO" (5564) é código morto
   neste binário, e os loops de parada nunca podem terminar. Info.
5. **`CInputMenuField::Move` move os itens pela posição absoluta** (10895): cada item recebe
   `item.pos + newPos` em vez de `item.pos + (newPos - oldPos)`. Um menu movido depois de seus itens terem sido
   adicionados os desenha no lugar errado, a menos que estivesse em (0,0). Baixa (código original; não foi encontrado
   nenhum chamador que mova um menu, mas o `Move` do campo é uma chamada virtual (slot 9) e nem todos os seus pontos de
   chamada foram rastreados).
6. **`CInputField<MASK>::SetLength` realinha uma posição já alinhada** (12387): em máscaras alinhadas à direita ou
   centralizadas, cada redimensionamento desloca as caixas de novo sem desfazer o deslocamento anterior. Os chamadores
   encontrados (o menu, funcs 1693 e 3655) são alinhados à esquerda, então o problema está latente ali; outros pontos
   de chamada virtual (slot 12) não foram rastreados. Info.
7. **Produtos `int` não verificados em `CBmpConversor`** (3663/3664): `largura * altura` pode estourar;
   `VerticalFlip` rejeita apenas um produto negativo e `InvertColors` não verifica nada. As dimensões vêm
   do leitor de impressões digitais (o mock web `CFingerPrepareSimulador`), não de entrada do usuário. Baixa.
8. **`CFixedImage` faz swap com o buffer do recurso** (2246): ela esvazia o `SharedVector` retornado por
   `IResource::getResourceFile`. O `CWasmResource` web retorna um vetor novo a cada vez, então nada se
   perde; um `IResource` que guardasse seus vetores em cache entregaria imagens vazias na segunda vez. Info.
9. **Ícone da bateria adicionado sem `Add()`** (502): o `CDSImageField` não recebe nome único, então
   `FindFieldAs` não consegue encontrá-lo. Inofensivo. Info.
10. **Hardware simulado visível aqui**: `CLedFieldMT` (11050): as quatro operações de LED do
    microterminal se reduzem a ligado/desligado em `CWasmScreenMT`. O bipador, a impressora (`IPaperRelatorios`), a bateria
    (`IPower`) e os recursos são todos mocks web. Info.

Nenhum acesso à rede, nenhum parsing de URL/JSON e nenhum parsing de formato de arquivo (além da busca inlinada do
end-of-central-directory do minizip, morta aqui) acontecem nesta unidade.

---

## 8. Questões em aberto

* Nomes oficiais dos slots 2 de IScreen (métricas de glifo), 5 (chamado com `(rect, 1)` por `Move`), 19 (desenhar
  texto: significado dos seus dois últimos argumentos int), 22/24/26 (desenhar imagem/animação), 30/31 (largura/altura), 37
  (no-op em `CWasmScreen`, chamado uma vez por segundo por `EnterLoopDoingNothing`), e dos slots 8..11 de IScreenMT.
* Nomes dos slots 2/3/7 de `IForm` (`Show`/`ShowOnTop`/`OnActivate` aqui; outras unidades usam `Exibe()`), dos
  slots 3/4 de `IFormFieldBase` (`Start`/`Stop`), e o nome real dos enumeradores de `Actions`.
* A finalidade do retângulo em `CInputMenuField` +132 (inicializado com `{pos,pos}` e nunca lido nesta
  unidade) e do parâmetro `bool` de `AddLabeledInputControl`.
* O uso que `comum_f5783` faz do argumento `variante` (0 ou 2) ao montar o QR code de estado da urna da func 3059.
* O `csincronizavota.u02.cpp` da unidade u02 passa as strings do guard como `(acao, título, mensagem, "")`; a
  ordem de campos encontrada aqui (`detalhe, título, mensagem`, verificada contra a formatação
  `"{} ({})"` do monitor) bate com o `(acao, "", título, mensagem)` da u07/u08.
