# u32: classes `api::` sem arquivo-fonte conhecido (campos de formulário, fontes de texto, timers, síntese de fala)

A unidade u32 reúne **108 funções wasm** do componente `app:api` cujas classes não têm registro de `std::source_location`,
de modo que as ferramentas não conseguiram associá-las a um arquivo original. Elas pertencem a 37 classes da biblioteca
`uenux2/src/api` do TSE (namespace `api::`). Quase todas são **métodos virtuais encontrados apenas pelas
vtables** (`Class::vfN`). Esta unidade lhes dá nomes reais, reconstrói as classes e infere onde cada classe
fica a partir da convenção de nomes do TSE (classe `CFooBar` em `cfoobar.h/.cpp`, ao lado de seus parentes mais próximos).
Todo caminho original desta unidade é um palpite desse tipo *(caminho inferido)*; os arquivos reconstruídos dizem isso nas
primeiras linhas.

Só **8 das 108 funções executaram** durante as votações gravadas (`analysis/runtime/*.functions.tsv`):
`CRectField::Draw` (10960, a linha separadora de toda tela de votação), `CInputFieldControl<IScreen>::GetClassName`
(11155), três `CDataText<…>::GetText` (12580 os dígitos que o eleitor digita, 12649 o nome do candidato, 12784),
`CSystemDateTime::GetDataHora` (10855, só na inicialização) e dois `CDefaultGenericFactory::Create` (10854, 10869).
Uma execução headless com o áudio do eleitor ligado (`--audio --save-audio`) confirmou, além disso, que o caminho de síntese
de fala (`CRHVoiceTextToSpeech::Sintetiza`, 10841) produz o formato WAV reconstruído aqui (§9).

O conteúdo da unidade, agrupado por tema:

| tema | classes | § |
|---|---|---|
| base de campo de formulário e campos simples | `IFormFieldBase<MEDIA>`, `IFormField`, `CRectField`, `CLineField`, `CFillField`, `CMaskedTextField<MASK>`, `CPreShowClearScreen` | 3 |
| campos do microterminal (mesário) | `CTextFieldUpdateMT`, `CInputFieldMT`, `CClockFieldMT`, `CBuzzFieldMT`, `CBeepFieldMT`, `CPreShowClearMT` | 3.3 |
| campos de relatório impresso e trabalhos de impressão | `CQRCodeImageFieldPaper`, `CNewLineFieldPaper`, `CCutFieldPaper`, `CFormPart`, `CSubReport` | 3.4, 6 |
| fontes de dados de texto e imagem | `IText`, `CFixedText`, `CDataText<SRC>`, `CDataTextFmt<SRC>`, `CDataImage<SRC>` | 4 |
| validação de entrada | `IInputValidation`, `CMenuValidation::IsValidChar`, `IInputField::SetLength`, `CInputFieldControl<IScreen>` | 5 |
| padrão observer | `IObservable<T>` (ícone da bateria, snapshots da pilha de formulários) | 7 |
| timers e o relógio | `CTimer`, `CTimerScheduler`, `CSystemDateTime` | 8 |
| fábricas de primitivas do SO | `CDefaultGenericFactory<…>::Create` (4 instâncias) | 8.3 |
| síntese de fala para o voto acessível | `AudioCollector`, `CRHVoiceTextToSpeech`, `CEsperaAudio` | 9 |
| erro de impressora | destrutores de `CUePrinterError` | 6.3 |

**Fontes reconstruídas** (todas novas, exceto as marcadas como *fragment*):

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

Todos os caminhos são inferidos, exceto os diretórios: `api/gui` (atestado pelos parentes `ctextfield.cpp`,
`cqrcodeimagepaper.cpp`, `cinputmenufield.cpp`, `cformbuilder.cpp`), `api/gui/reports` (`creport.cpp`),
`api/util` (`itimerscheduler.h`, `isystemdatetime.cpp`), `api/pattern` (`igenericfactory.h`),
`api/audio/alsa` (`cwavfile.cpp`) e `api/hwil` (`iimpressora.h`).

Glossário usado abaixo: *terminal do mesário* / MT: o microterminal do mesário (um LCD de 4 x 40 caracteres com teclado,
LEDs e buzzer); *título (de eleitor)*: o número de inscrição do eleitor (12 dígitos); *habilitação*: a liberação de um eleitor
para votar; *voto com áudio* / *áudio do eleitor*: o voto acessível, lido em voz alta por fones de ouvido; *BU (boletim de urna)*:
o resultado impresso/arquivo de cada urna; *via*: cópia impressa; *zerésima*: o relatório zerado impresso antes da votação;
*RDV (registro digital do voto)*: a tabela embaralhada dos votos dados; *DS*: data source (fonte de dados).

---

## 1. Onde este código fica no processo de votação

Tudo o que a urna mostra ou imprime é um **formulário** (`api::IForm<MEDIA>`, iform.h, unidade u17) feito de **campos**:

* `MEDIA = IScreen`: o display do eleitor, de 640 x 480. As telas de votação (caixas do número do candidato, nome, partido,
  foto, o cabeçalho "SEU VOTO PARA", a linha separadora) são montadas por `vota::CTelasVota` (unidade u07) a partir dos
  campos das unidades u15, u16 e desta unidade.
* `MEDIA = IScreenMT`: o microterminal do mesário. O mesário digita o título do eleitor
  (`CInputFieldMT`), vê o status da identificação biométrica (`CTextFieldUpdateMT`) e o relógio
  (`CClockFieldMT`), e ouve o buzzer/os bipes (`CBuzzFieldMT`, `CBeepFieldMT`). Todo formulário do MT limpa o LCD
  primeiro (`CPreShowClearMT`).
* `MEDIA = IPaper`: a impressora térmica. O BU, a zerésima e os outros relatórios são formulários de
  `CTextFieldPaper` (u16), `CNewLineFieldPaper`, `CCutFieldPaper` e, no caso do BU, `CQRCodeImageFieldPaper`
  (os QR codes das seções "BU DIGITAL" e "CERTIFICADO DIGITAL").

Os campos nunca armazenam o seu texto: eles guardam um `IText` (`CFixedText`, `CDataText<SRC>`, `CDataTextFmt<SRC>`)
que é reavaliado a cada desenho. É assim que a tela de votação acompanha os dígitos que estão sendo digitados
(`CDataText<const std::string& (*)()>(&VotoDigitado)`, func 12580, executa a cada tecla) e é assim que o nome do
candidato aparece (`CDataText<comum::CCandidaturasDSNome>`, func 12649).

Para o **voto acessível**, toda tela também é falada: `vota::CVotacaoStateAudio::PlayMessage` pede um WAV a
`ITextToSpeech::GetAudio(texto)`; num cache miss, o engine `CRHVoiceTextToSpeech::Sintetiza`
(func 10841) o sintetiza com o RHVoice (§9), e o dispositivo de som o toca. `CEsperaAudio` é o handle
usado para esperar (ou cancelar) o fim de uma mensagem.

O **serviço de timers** (`ITimerScheduler`) aciona os campos piscantes, o relógio e as atualizações periódicas. A implementação
padrão da urna, `CTimerScheduler`/`CTimer` (uma `std::thread` por timer), está nesta unidade; o build web
a substitui por `simulador::CWasmTimerScheduler` porque não tem threads (§8).

---

## 2. Classes e hierarquia (RTTI)

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

Os layouts estão nos headers reconstruídos (comentários `// +offset`). Os principais:

| classe | tamanho | membros |
|---|---|---|
| `IFormFieldBase<MEDIA>` | 24 | +4 flag dirty, +8 `IForm<MEDIA>*`, +12 `std::string m_nome` |
| `CRectField` / `CFillField` | 36 | +24 `SRect`, +32 `TColor` |
| `CLineField` | 36 | +24 `SPoint` de origem, +28 `SPoint` de destino, +32 `TColor` |
| `CTextFieldUpdateMT` | 56 | +24 `SPoint`, +28 `SharedIText`, +36 último texto, +48 `shared_ptr<ITimer>` |
| `CInputFieldMT` | 72 | `IInputField<IScreenMT>` (64) + +64 `SPoint`, +68 máscara `bool` |
| `CQRCodeImageFieldPaper` | 44 | +24 bitmap `vector<uebyte>`, +36 largura em módulos, +40 escala |
| `CFixedText` | 20 | +4 alinhamento, +8 `std::string` |
| `CDataTextFmt<SRC>` | varia | +4 alinhamento, +8 `SRC`, depois o formato `std::string` |
| `CSubReport` | 40 | +4 nome, +16 via, +28 `shared_ptr<IReportPart>`, +36 `bool` |
| `CTimer` | 144 | +8 intervalo em ms, +16 `std::function<void()>`, +40 em execução, +44 cv, +92 mutex, +116 `std::thread`, +120 mutex |
| `CRHVoiceTextToSpeech` | 112 | `ITextToSpeech` (96) + +96 `shared_ptr<RHVoice::engine>`, +104 `unique_ptr<voice_profile>` |
| `AudioCollector` | 24 | +8 amostras `vector<short>`, +20 taxa de amostragem (16000) |

---

## 3. Campos de formulário

### 3.1 O protocolo (IFormFieldBase / IFormField)

| slot | método | notas |
|---|---|---|
| 0 / 1 | destrutores | o slot 0 de todo campo cujos membros próprios são triviais é o próprio destrutor da base: 12658 (IScreen), 11052 (IScreenMT), 11013 (IPaper) — thunks para o corpo mesclado `api_f1566` |
| 2 | `Draw(MEDIA&) const` | puro |
| 3 / 4 | `Start()` / `Stop()` | no-op padrão (ICF 218); os campos com timers os sobrescrevem |
| 5 | `RedrawIfDirty(MEDIA&)` | func 4118 |
| 6 | `SetForm(IForm<MEDIA>*)` | func 3037 |
| 7 | `GetClassName() const` | usado por `CFormBuilder::Add` para dar nomes únicos |
| 8 / 9 | `Rect()` / `Move(const SPoint&)` | só `IFormField<IScreen>` |

Um detalhe visível só no binário: `IFormField<IScreenMT>` e `IFormField<IPaper>` implementam o slot 7 com o
**nome do próprio template** (`"IFormField<IScreenMT>"`, func 11051; `"IFormField<IPaper>"`, func 11014), e
nenhuma das classes de campo de MT/papel o sobrescreve. Os builders, porém, nunca o pedem: só o builder de IScreen,
`CFormBuilder::Add` (func 426), chama o slot 7 para montar os nomes `"<class><n>"`. Os builders do microterminal e do
papel (funcs 728, 941, 1072/3890, 1151/3674, 1152 e 5409 → 1400, 198/3890, 1264, 2775) só inserem
o `shared_ptr` no vetor de campos, então os campos de MT e de papel ficam com `m_nome` **vazio**.

### 3.2 Campos da tela do eleitor

| classe | Draw | GetClassName | Rect / Move |
|---|---|---|---|
| `CRectField` | `IScreen::DrawRect(rect, colour, 1)` (slot 9) | "CRectField" | rect / `MoveTo` (ICF 2783) |
| `CLineField` | `IScreen::DrawLine(p1, p2, colour, 1)` (slot 8 → `js_line`) | "CLineField" | caixa delimitadora dos 2 pontos / translada ambos (ICF 5527) |
| `CFillField` | `IScreen::FillRect(rect, colour)` (slot 6 → `js_fill`) | "CFillField" | rect / `MoveTo` |
| `CMaskedTextField<MASK>` | `MASK::MaskText(tela, texto)` (u15) | compartilhado 6507 | compartilhados 6505 / 6504 |
| `CInputFieldControl<IScreen>` | nada | "CInputFieldControl" | `SRect{}` vazio / no-op |
| `CPreShowClearScreen` (pré-exibição) | `IScreen::Clear(1)` | – | – |

O membro de cor de `CRectField` (+32) recebe 2 (preto) do seu único construtor (func 5501). A unidade u02 o chamou de
`m_espessura`; o Draw o passa como argumento de cor de `DrawRect(rect, colour, thickness)`, e
a espessura é o literal 1 — `CWasmScreen::vf9` (9205) confirma a ordem dos argumentos ao repassar
`(colour, thickness)` para quatro chamadas de `DrawLine`.

### 3.3 Campos do microterminal (terminal do mesário)

| classe | Draw → slot de `IScreenMT` | comportamento na web (`simulador::CWasmScreenMT`) |
|---|---|---|
| `CTextFieldUpdateMT` | 3 `Write(pos, IText)` | veja abaixo |
| `CInputFieldMT` | 3 `Write` | veja abaixo |
| `CClockFieldMT` | 12 `ShowClock(pos)` | 8757 formata a hora e a escreve |
| `CBuzzFieldMT(a, b)` | 6 `Buzz(a, b)` (chamadores: (51, 10), (52, 5)) | 8787: só atualiza — sem buzzer |
| `CBeepFieldMT(n)` | 7 `Beep(n)` (n = 1 ou 2) | 5039: só atualiza |
| `CPreShowClearMT` | 2 `Clear()` + 13 (desconhecido, no-op no mock) | limpa as 4 linhas |

**`CTextFieldUpdateMT`** (func 10548 e construtor inlinado em `CPedeIdentidade::GetInst`, func 652): um
timer periódico (300 ms em `CPedeIdentidade`) compara `m_texto->GetText()` com o último texto desenhado e chama
`Invalidate()` quando ele muda (lambda 10542). `Draw` primeiro sobrescreve o texto anterior com o mesmo número de
espaços quando o novo é mais curto (o LCD não tem apagamento de retângulo), escreve o novo texto e o memoriza.
A verificação de texto nulo lança `std::invalid_argument("CTextFieldUpdate - campo estava com o texto nulo")` — uma
exceção padrão comum, ao contrário da versão de tela, que lança `CUeGuiError(4974)`.

**`CInputFieldMT::Draw`** (func 10726): os dígitos digitados, completados com `_` até o comprimento máximo; enquanto o campo
tem o foco, na metade "apagada" da piscada de 600 ms o primeiro `_` livre vira um espaço (cursor piscante). Uma
flag em +68 mostraria `*` no lugar dos dígitos, mas o único construtor (inlinado na func 1151, sempre com
`CNumberValidation("0123456789")`) a define como false.

### 3.4 Campos de relatório impresso

| classe | Draw → slot de `IPaper` | usado para |
|---|---|---|
| `CNewLineFieldPaper(n)` | 4 `NewLine()` n vezes | espaçamento: depois de "<n>a. VIA" (3 linhas); antes de um corte, 20 linhas nos rodapés do BU/relatórios (12110, 12105, 5579), 2 em `CortaPapel` (2882), 5596 e 11908, 1 ou 8 em 5591 |
| `CCutFieldPaper` | 3 `Cut()` | fim de cada documento impresso |
| `CQRCodeImageFieldPaper` | 12 `PrintImage(bitmap, largura, escala)` | os QR codes do BU |

No build web, `IPaper` é `simulador::CWasmNullPaper`, cujos slots 3, 4 e 12 são no-ops: nada é impresso.

---

## 4. Fontes de dados de texto e imagem

`IText` tem duas funções virtuais além dos destrutores: slot 2 `GetText()` e slot 3 `GetAlignment()`
(ICF 1661, `return +4`). As implementações:

* **`CFixedText`** — texto constante. A classe mais construída da GUI (sua vtable é armazenada por 39
  funções). Os campos também constroem temporários na pilha para desenhar strings calculadas.
* **`CDataText<SRC>`** — `GetText() { return m_fonte(); }`. SRC é um ponteiro de função, um `std::function`, uma
  lambda ou um pequeno functor. Os corpos interessantes são aqueles em que o `operator()` do functor foi inlinado:
  * `CDataText<vota::CEscolheOpcao::COpcaoDS>` (10685): `std::format("{}-{}", numero, descricao())` — as
    entradas numeradas do menu de opções do mesário.
  * `CDataText<CFormBuilder::AddStatusHeader(unsigned)::$_0>` (11107): o percentual da bateria do cabeçalho de
    status: atualiza `IPower` (slot 15); se `(estado & 6) == 4`, retorna uma string vazia; senão,
    `std::format("{: >3}%", IPower::GetPercentualBateria())`.
* **`CDataTextFmt<SRC>`** — o mesmo, mais uma string de formato, com **três comportamentos diferentes**:
  * fonte que pode ser chamada com o formato (`std::function<std::string(const std::string&)>`, ponteiros de função):
    `m_fonte(m_formato)` (12287, ICF 6404);
  * `CTextSource` (um `shared_ptr<std::string>` pertencente a um estado): **no estilo printf**
    `snprintf(buf, 512, m_formato.c_str(), texto.c_str())` (10746); todo chamador passa `"%s"`;
  * fontes que retornam um valor (`comum::(anonymous)::CComparecimentoMesariosDS`, uma contagem de mesários
    registrados): `std::vformat(m_formato, make_format_args(valor))` (5387, 10368).

  `CComparecimentoMesariosDS` existe como **três tipos distintos com o mesmo nome RTTI** (três vtables
  @1594036/@1594188/@1594340): uma classe de namespace anônimo definida num header incluído por três unidades
  de tradução, cujos auxiliares de StartState foram depois inlinados pela LTO em `comum::CPedeTituloMesario::StartState` (10388).
* **`CDataImage<SRC>`** — imagens: `GetImage() { return m_fonte(); }`. A instância nesta unidade (12520) é a
  foto do candidato da tela "visualizar candidatos": a lambda ligada a uma cópia de `CDadosCandidato` chama
  `comum::CFotos::GetInst()` e `CFotos::GetImagem(id)` (func 3755, a foto lida do arquivo ASN.1 de fotos).

`cdatatext.h` lista todas as instanciações dos dois templates (12 `CDataText`, 7 `CDataTextFmt`) com sua vtable
e suas funções; 5 das de `CDataText` (`CCandidaturasDSNumero`, `CCargoDSNomeSexoCandidato`,
`CRespostasDSNumero`, `DS_NomeCargoNeutroComEscolha`, `CPadDS<CToUpperDS<CCargoDSNome>>`) têm o seu `GetText`
em outras unidades.

---

## 5. Campos de entrada e validação

* **`IInputValidation`** (16 bytes: vptr + `std::string m_caracteres`). O slot 3 `IsValidChar(c)` =
  `m_caracteres.find(c) != npos` (12473) para CNumber/COption/CControlValidation; o slot 2 `IsValid(texto)` é
  o 4022 compartilhado (todo caractere é válido). O slot 0 (12502) é o destrutor de todas as cinco classes de validação.
* **`CMenuValidation::IsValidChar`** (10899): uma tecla digitada num menu de tela só é aceita se for uma de
  "1234567890" **e** se `IsValid(menu.texto + c)` ainda casar com um item visível (correspondência exata quando completo,
  prefixo caso contrário; uma não correspondência bipa — func 10900, unidade u15).
* **`IInputField<MEDIA>::SetLength`** (6312, compartilhada pelas instanciações de IScreen e IScreenMT): se o
  comprimento mudar, o texto digitado é descartado, o novo máximo é armazenado e o formulário é instado a redesenhar.
* **`CInputFieldControl<IScreen>`**: a entrada invisível das telas que só esperam CONFIRMA / CORRIGE /
  BRANCO. `Rect()` é um retângulo vazio (11153); `GetClassName` é "CInputFieldControl" (11155, executa em toda
  tela do eleitor).

---

## 6. Relatórios e trabalhos de impressão

### 6.1 CFormPart

Um relatório composto (`api::CReport`, u17) é um vetor de `IReportPart`s impressos em ordem. `CFormPart` envolve um
formulário de papel: `Imprime()` = `m_form->Show()` (10889) — mostrar um formulário de papel o imprime. É construído só pela func 601,
cujos chamadores são o título do extrato do RDV (`CriaTituloExtratoRDV`, 5971), a zerésima e seu resumo (11946,
11943; `cgeradorresumozeresima.cpp`, unidade u09), a lista de eleitores (`CImpressaoListaEleitores`, 11908) e
`CGeraRelatorios::StartState` (12105).

### 6.2 CSubReport e CLp

`CSubReport(nome, via, relatorio)` é um objeto de pilha que descreve um trabalho de impressão; o front end da impressora
de relatórios `api::CLp::Imprime(sub, job)` (func 3875, nome inferido; `cwasmclp.cpp` da unidade u19) chama o slot 9 da
impressora, imprime a parte de cabeçalho opcional (`TemCabecalho()`/`ImprimeCabecalho()`, slots 5/6) e executa o trabalho. É
construído por `CRelVotaUtil::CortaPapel` (`("", "", false)`, só um corte de papel), `CPaperFormBuilder::Show(nome, via)` (func
3671, usada pelo relatório "Estado da urna" e pelos relatórios do PU e do pacote de dados) e `CImpressaoListaEleitores`
(`via = std::format("{}ª via", n)`). A parte de cabeçalho (+28) nunca é definida neste build. Todos os nomes de método
de `ISubReport` são inferidos (nenhuma string ou srcloc os nomeia).

### 6.3 CUePrinterError

`CUePrinterError` e `CUeCodedPrinterError` (o segundo só acrescenta um `int`) compartilham os seus dois destrutores
(8351, 4910): liberam as duas strings extras (+52, +40) e depois as duas strings de `ecourna::api::exception::CError`.

---

## 7. Observables

`IObservable<T>` = { vptr, `T m_valor`, `std::vector<IObserver<T>*>`, `std::mutex` }. A unidade contém os seis
destrutores dos três `IObservable<std::vector<FormHandle<MEDIA>>>` (o snapshot publicado da pilha de formulários de
cada dispositivo, `CFormStack<MEDIA>::CObservable`; o corpo 3940 libera o bloco de controle de cada `FormHandle`) e os dois
de `IObservable<std::shared_ptr<IImage>>` (as fontes de dados do ícone da bateria observadas por `CDSImageField` e
`comum::CInfoMTLCD`).

---

## 8. Timers e o relógio

### 8.1 CTimerScheduler / CTimer (o serviço de timers padrão da urna)

`ITimerScheduler` declara a sua fábrica **antes** do seu destrutor: slot 0 `CreateTimer(periodo, callback)`,
slots 1/2 destrutores. `CTimerScheduler::CreateTimer` (10846) = `std::make_shared<CTimer>(periodo, callback)`.

`CTimer` (slots de ITimer: 2 Start, 3 Stop, 4 IsRunning, 5 SetInterval) possui uma `std::thread` que espera numa
variável de condição por `intervalo` ms e chama o callback:

* `Stop()` (10847): running = false; notifica a variável de condição; faz join da thread (sob um segundo mutex).
* `IsRunning()` (10851), `SetInterval(ms)` (10848: armazenado em +8, usado na próxima espera).
* `~CTimer()` (5445/10852): `Stop()` e depois destrói os membros (`std::terminate` se a thread ainda fosse joinable).
* `Start()` (10850, outra unidade): `Stop(); running = true; m_thread = std::thread(...)`. **Neste build, o
  construtor de `std::thread` é reduzido por constant folding ao seu caminho de falha**: a função termina em
  `throw std::system_error(138 /*ENOTSUP*/, "thread constructor failed")`, e o corpo da thread foi removido como
  código morto (a sua reconstrução em `ctimerscheduler.cpp` está marcada como tal).

O simulador instala `simulador::CWasmTimerScheduler` (timers avançados por `votaTick`) no início de
`CSimuladorWasm::Executa`, antes que qualquer coisa peça um timer, então `CTimerScheduler` nunca é criado nas
sessões gravadas.

### 8.2 CSystemDateTime

O `ISystemDateTime` padrão (slot 0 `GetDataHora`, slot 1 `SetDataHora`, depois os destrutores):
`GetDataHora()` = `time(nullptr)` (10855); **`SetDataHora(time_t)` é vazio** (10853). Ele é criado pelo
primeiro `ISystemDateTime::GetInst()`, o que acontece durante a inicialização estática: `__wasm_call_ctors` constrói dois
objetos `CDate` estáticos (@1833312, @1839068) através da func 1382 = `gmtime_r(GetDataHora())`
(`analysis/runtime/*.edges.tsv`: 14478 → 1382 → 10855). O simulador depois substitui o relógio por
`CWasmSystemDateTime` (hora local do navegador + deslocamento).

### 8.3 Fábricas de primitivas do SO

`CDefaultGenericFactory<AP, C>::Create()` = `std::make_unique<C>()` para `CPosixSemaphore` (contagem inicial 0,
10854), `CPosixRWMutex` (10864), `CPosixMutex` (10869) e — específico da web — `simulador::CWasmThread` (10878,
construtor inlinado: 16 bytes, vptr @1528400). São o que `api::CThread`, as filas de mensagens e os locks
usam; a implementação de thread é o mock cooperativo. O seu `Wait()` faz polling com `emscripten_sleep`, que o
glue implementa como `abort()` porque o módulo é compilado sem Asyncify, então esse caminho nunca pode executar (unidade u18).

---

## 9. Síntese de fala para o voto acessível

`CRHVoiceTextToSpeech` é o `ITextToSpeech` instalado por `votaInit` quando a página habilita a acessibilidade
(`localStorage.acessibilidade === "sim"`); docs/libraries/rhvoice.md trata do engine. Esta unidade contém o
destrutor (5441/10838), a síntese (10841) e o cliente RHVoice que coleta as amostras.

`Sintetiza(texto, parametros)` (10841, 75 KB porque o pipeline de requisição do RHVoice está inlinado), passo a passo:

1. **Latin-1 → UTF-8**: cada byte ≥ 0x80 vira `0xC0 | b >> 6`, `b & 0xBF` (as strings do TSE são ISO-8859-1).
2. **Perfil de voz**: `SelecionaPerfil(parametros.perfil)` (func 5442). `parametros.perfil` é
   `ITextToSpeech::m_perfilVoz` (+64), que nada define nas sessões gravadas; o nome vazio substitui o
   perfil "Letícia-F123", escolhido pelo construtor, por um perfil vazio na primeira chamada, e o RHVoice recorre
   à sua voz padrão (e única).
3. **Documento** (`RHVoice::document`, 608 bytes, construído inline) com `rate = taxa / 100.0` (60…140 %),
   `pitch = parametros[+16] / 100.0` (padrão 100 → 1.0), `volume = 1.0` (o volume é aplicado pelo dispositivo
   de som: teclas 3/9 do teclado acessível).
4. **Síntese num `AudioCollector`** (um `RHVoice::client`, taxa de amostragem 16000): `play_speech` (10820)
   acrescenta as amostras a um `std::vector<short>` e retorna true; `get_audio_buffer_size` (10821) retorna 100;
   os callbacks de evento mantêm os padrões do RHVoice (func 1908 = `return true`, um corpo ICF que o wasm-opt também
   compartilha com o `update` do null-digest do OpenSSL). Depois da síntese, `done()` (slot 12) só é chamado se
   `get_supported_events() & 64` (event_done), que aqui é 0.
5. **WAV**: um header de 44 bytes é montado a partir de valores imediatos e `std::make_shared<CWavFile>(header, samples)`
   (o construtor de `CWavFile`, `cwavfile.cpp:47`, está inlinado: `malloc` + cópia, erro
   `CBaseError<EUeAudioError>(4851, "Falha ao alocar memória <n> bytes")`).

Verificado nos WAVs salvos por `node tools/run/headless.mjs --scenario municipal-t1 --audio --save-audio DIR`:

| offset | valor |
|---|---|
| 0 | `RIFF`, tamanho = data + 36 |
| 8 | `WAVE`, chunk `fmt ` de 16 bytes: PCM (1), mono (1), 16000 Hz, 32000 bytes/s, alinhamento de bloco 2, 16 bits |
| 36 | `data`, n = 2 × amostras |

Cinco mensagens foram produzidas numa sessão curta (de 0.8 s a 20.8 s, de 26 KB a 665 KB, RMS ≈ 3500–4400: fala real).
Os resultados são guardados em cache por `ITextToSpeech` (LRU + cache permanente, unidade u02), então `Sintetiza` só executa num cache miss.

`CEsperaAudio` (8 bytes) é o handle de "esperar pelo fim do áudio" retornado pelo slot 9 de `ISound`:
`Cancela()` (9509) liga a flag que `Cancelada()` (ICF 2683) lê; os estados de votação o cancelam quando o
eleitor pressiona uma tecla.

---

## 10. Boletim de Urna: o que esta unidade contribui

O conteúdo, as assinaturas e os arquivos do BU são montados em outro lugar (`CGeraBU`, `CGeradorBU*`, unidades u06–u09, u21, e
docs/10-boletim-de-urna.md). Esta unidade fornece as **primitivas de impressão** do BU:

1. `vota::CGeraBU` (func 12110) monta os formulários de papel do BU com `CPaperFormBuilder`. Para cada payload de QR code
   das seções "BU DIGITAL" e "CERTIFICADO DIGITAL", ela mesma calcula o raster com
   `CQRCodeImagePaper::MontaImagem(400, payload)` (func 2772; u16: 1 bit por módulo, zona de silêncio de 2 módulos, escala de
   `min(400 / width, 4)` pontos de impressora por módulo) e passa o resultado a `CPaperFormBuilder::AddQRCode`
   (func 2775), que só o copia para um novo **`CQRCodeImageFieldPaper`** `{bitmap, width in modules, scale}`
   e o acrescenta (um `AddNewLine(1)` entre QR codes consecutivos). A func 5591 (o caminho "BU das outras
   obrigatórias") faz o mesmo.
2. O rodapé termina com `AddNewLine(20)` (**`CNewLineFieldPaper(20)`**) e `AddCut()` (**`CCutFieldPaper`**).
3. Quando um formulário é mostrado no papel, cada campo se desenha em `IPaper`: linhas de texto (`CTextFieldPaper`, slot 2),
   linhas em branco (slot 4), a imagem do QR (slot 12 — na urna, o driver a serializa como o bloco de imagem
   `0B | u32 len | u32 largura | u8 escala | bitmap` encontrado nos arquivos `*-imgbu.dat` publicados,
   docs/bu/qrcode.md), o corte (slot 3).
4. As **vias** impressas do BU não são compostas aqui: `CImprimindoBU::ImprimeBU(numVia, modo)` (func 2890)
   imprime um pequeno formulário de cabeçalho ("<n>a. VIA" + **`CNewLineFieldPaper(3)`**) e pede à impressora de relatórios que imprima
   a imagem armazenada, com assinatura verificada, do `bu.dat` (slot 8 de `IPaperRelatorios` com "{}ª via"). Outros relatórios que
   levam um rótulo de via passam por `CSubReport` (§6.2).
5. Build web: o dispositivo de papel é `CWasmNullPaper` — cada uma dessas chamadas é um no-op. Em vez disso, o simulador mostra os
   QR codes do BU na tela (`CMostraQRCodeBU`, que usa separadores `CLineField` desta unidade).

---

## 11. O que é específico do build web

* `CTimerScheduler`/`CTimer` não podem funcionar (sem pthreads: `std::thread` sempre lança exceção) e são substituídos por
  `simulador::CWasmTimerScheduler`.
* `CSystemDateTime` só é usado durante a inicialização estática; o seu `SetDataHora` tem corpo vazio.
* `CDefaultGenericFactory<IThreadImpl, simulador::CWasmThread>` substitui a implementação de threads POSIX.
* Dispositivos do microterminal: `Buzz`, `Beep` e o slot 13 são no-ops em `CWasmScreenMT`; os LEDs são desenhados (u15).
* Dispositivos de papel: nenhuma impressão (`CWasmNullPaper` / `CWasmNullPrinter`).
* Fala: o RHVoice executa dentro do módulo wasm; os dados de voz são um pacote separado de 19.6 MB, carregado só quando
  a acessibilidade está ligada.

---

## 12. Observações notáveis sobre wasm / Emscripten

* **Destrutores como thunks.** A maioria dos destrutores desta unidade são thunks de 12 bytes `return merged_body(this,
  vtable)`: o *merge-similar-functions* do wasm-opt transformou todo destrutor com o mesmo layout de membros num único
  corpo que recebe a vtable a armazenar (`api_f1565/1566` para {…, std::string @+12}, `shared_f1727/1969` para
  {…, std::string @+8}, `shared_f6052/6053`, `api_f6062/6063`, `api_f3940`, `shared_f3904`, `shared_f1723`).
* **ICF entre classes não relacionadas.** `CUePrinterError` e `CUeCodedPrinterError` compartilham destrutores;
  `IInputField<IScreen>::SetLength` e `IInputField<IScreenMT>::SetLength` são uma única função (6312); os quatro
  callbacks de evento do RHVoice são um único `return true` (1908), que também é o update do null-digest do OpenSSL (a
  struct legada `EVP_md_null` @1647304, com a qual `pkey_ecd_ctrl` (7438) compara, e a entrada de dispatch
  `nullmd_update` do provider @1752060); `CEsperaAudio::Cancelada` é o mesmo corpo que um getter de
  `vota::impl::CInformacaoThreadOperador` (2683).
* **Destrutor da base como destrutor da classe.** Uma classe de campo cujos membros são trivialmente destrutíveis tem o
  destrutor da classe base (12658 / 11052 / 11013) no seu próprio slot 0.
* **Nomes de classe como imediatos.** O `GetClassName()` de `CRectField`/`CLineField`/`CFillField` é uma string SSO
  escrita com um store de 8 bytes e um de 2 bytes a partir de um segmento de dados (sem chamada).
* **Falha reduzida por constant folding.** O construtor de `std::thread`, inlinado contra os stubs de pthread do Emscripten, deixa
  apenas `__throw_system_error(138, "thread constructor failed")` em `CTimer::Start`; o corpo da thread desapareceu.
* **Stubs noexcept.** As chamadas à func 150 sobre membros de `CTimer` (+44, +92, +120) são o que resta de
  `condition_variable::notify_all`, `mutex::unlock`, `~mutex`, `~condition_variable` (veja libcxx-core.md §1.3);
  `mutex::lock()` desapareceu por completo. Esse resíduo é o que permitiu inferir o layout de membros de `CTimer`.
* **Um método de 75 KB.** `CRHVoiceTextToSpeech::Sintetiza` contém todo o caminho por requisição do RHVoice; o seu único
  srcloc é o do construtor inlinado de `CWavFile`, o que enganou a primeira passada de nomeação.

---

## 13. Código suspeito ou digno de nota

| # | onde | o quê | impacto |
|---|---|---|---|
| 1 | 10850 / 10846 / 5445 | `CTimer::Start` sempre lança `std::system_error("thread constructor failed")` neste build | latente: qualquer uso do `CTimerScheduler` padrão (ex.: se `ITimerScheduler::GetInst()` executasse antes de o simulador registrar o seu próprio scheduler) lançaria exceção a partir do `Start()` de todo campo; não alcançável nas sessões gravadas. A 10850 não tem `invoke_*`, então nada é desfeito no throw: `m_ativo` continua true (`IsRunning()` passa a reportar um timer que nunca executa) e o `__thread_struct` (4 + 24 bytes, func 2541) e a tupla de 8 bytes com o argumento da thread vazam |
| 2 | 10548 / 10542 (e `CTextSource`) | o callback de timer de `CTextFieldUpdateMT` lê uma `std::string` compartilhada com o estado que a escreve, sem lock | na urna real, o `CTimer` padrão presumivelmente executa os callbacks na sua própria `std::thread`: uma potencial condição de corrida em `std::string` (leitura parcial / crash), se o mesmo código for usado; inofensivo no build web cooperativo. Não verificável aqui: o loop de trabalho de `CTimer` está ausente do binário, e a lambda só pega o mutex do formulário (+28), e só depois de ler o texto |
| 3 | 10746 | `CDataTextFmt<CTextSource>` usa `snprintf` com um formato definido em tempo de execução e um buffer de 512 bytes, enquanto as outras instanciações usam `std::vformat` | só `"%s"` é passado, então hoje não há bug; textos mais longos são truncados silenciosamente, e um formato `{}` seria impresso literalmente |
| 4 | 10853 / 10855 | `CSystemDateTime::SetDataHora` é vazio; `GetDataHora` é o `time()` em UTC; dois `CDate` estáticos são inicializados a partir dele antes de o relógio de hora local do simulador ser instalado | só no simulador: as estáticas guardam a data UTC (pode diferir em um dia da data local simulada perto da meia-noite); ajustes de data são ignorados enquanto o relógio padrão está ativo |
| 5 | 10841 (+5442) | tratamento do perfil de voz: o nome pedido, em Latin-1, é comparado com o nome do perfil armazenado (em UTF-8); o nome padrão vazio substitui o perfil "Letícia-F123" do construtor | só funciona porque o RHVoice recorre à sua única voz instalada; um nome de perfil não ASCII reconstruiria o perfil a cada síntese |
| 6 | 10841 | exceções do RHVoice/utfcpp (`file_format_error`, `item_not_found`, `bad_cast`, `utf8::…`) não são capturadas em `Sintetiza`: a função não tem nenhum `invoke_*`, então não tem nem `catch` nem landing pads de limpeza | um throw pula todos os destrutores de `Sintetiza` (o documento de 608 bytes, o vetor de amostras e a cópia UTF-8 vazam); o primeiro handler é o `invoke_viiii` de `ITextToSpeech::GetAudio` (11530), que só libera a sua própria cópia dos parâmetros e relança para o estado de votação que pediu o áudio (`CVotacaoStateAudio`); com dados de voz inconsistentes, o fluxo do voto acessível poderia abortar (os blocos `catch` do próprio RHVoice foram removidos na compilação, rhvoice.md §8.5) |
| 7 | 10726 | `CInputFieldMT` tem um modo de mascaramento com `*` que nada habilita | código morto; o título é mostrado em claro no MT, como esperado |
| 8 | 11051 / 11014 (base dos campos de MT/papel desta unidade) | todos os campos de MT reportariam o nome de classe "IFormField<IScreenMT>" (os de papel, "IFormField<IPaper>"), mas nenhum builder pergunta: os campos de MT e de papel nunca recebem nome (`m_nome` vazio) | uma busca por nome num formulário de MT/papel não consegue encontrar nenhum campo pelo nome; cosmético, nada no binário faz tal busca pelos builders |
| 9 | 12008–12014 | a parte de cabeçalho de `CSubReport` (+28) nunca é definida | `ImprimeCabecalho` é código morto neste build |
| 10 | 10963 / 11012 / 11015 | as primitivas de impressão são no-ops no build web | o simulador nunca imprime o BU/zerésima; os usuários só veem os QR codes na tela |

---

## 14. Tabela de mapeamento completa (todas as 108 funções da u32)

"executou" = vista executando durante as votações gravadas. D1 = destrutor de objeto completo (slot 0 da vtable ou o slot de
destrutor da interface), D0 = destrutor de deleção.

| # | func | tamanho | executou | símbolo reconstruído | arquivo src |
|---|---|---|---|---|---|
| 1 | 1908 | 4 |  | `icf_ret_1_vf8`: `RHVoice::client::{sentence_starts,sentence_ends,word_starts,word_ends}` padrão `return true` (slots 8-11 de AudioCollector), ICF com a função `update` do null-digest do OpenSSL (EVP_md_null @1647304, `nullmd_update` @1752060) | src/uenux2/src/api/audio/crhvoicetexttospeech.cpp (comentário) |
| 2 | 3458 | 12 |  | `api::IObservable<std::vector<FormHandle<IScreenMT>>>::~IObservable()` (D1, thunk para o 3940) | src/uenux2/src/api/pattern/iobservable.h |
| 3 | 3470 | 12 |  | `api::IObservable<std::vector<FormHandle<IScreen>>>::~IObservable()` (D1) | src/uenux2/src/api/pattern/iobservable.h |
| 4 | 3669 | 12 |  | `api::IObservable<std::vector<FormHandle<IPaper>>>::~IObservable()` (D1) | src/uenux2/src/api/pattern/iobservable.h |
| 5 | 4910 | 127 |  | `api::CUePrinterError::~CUePrinterError()` (D0; também CUeCodedPrinterError) | src/uenux2/src/api/hwil/iimpressora.u32.cpp |
| 6 | 5019 | 10 |  | `IObservable<vector<FormHandle<IScreenMT>>>::~IObservable()` (D0) | src/uenux2/src/api/pattern/iobservable.h |
| 7 | 5052 | 10 |  | `IObservable<vector<FormHandle<IScreen>>>::~IObservable()` (D0) | src/uenux2/src/api/pattern/iobservable.h |
| 8 | 5411 | 176 |  | `api::CTextFieldUpdateMT::~CTextFieldUpdateMT()` (D1) | src/uenux2/src/api/gui/ctextfieldupdatemt.cpp |
| 9 | 5441 | 184 |  | `api::CRHVoiceTextToSpeech::~CRHVoiceTextToSpeech()` (D1) | src/uenux2/src/api/audio/crhvoicetexttospeech.cpp |
| 10 | 5445 | 144 |  | `api::CTimer::~CTimer()` (D1, Stop() inlinado) | src/uenux2/src/api/util/ctimerscheduler.cpp |
| 11 | 5511 | 10 |  | `IObservable<vector<FormHandle<IPaper>>>::~IObservable()` (D0) | src/uenux2/src/api/pattern/iobservable.h |
| 12 | 6312 | 124 |  | `api::IInputField<MEDIA>::SetLength(size_t)` (um único corpo para o slot 12 de IScreen e o slot 10 de IScreenMT) | src/uenux2/src/api/gui/iinputfield.u32.h |
| 13 | 7666 | 12 |  | `api::CFixedText::~CFixedText()` (D0, thunk para o 1969) | src/uenux2/src/api/gui/cfixedtext.h |
| 14 | 7706 | 12 |  | `api::CFixedText::~CFixedText()` (D1, thunk para o 1727) | src/uenux2/src/api/gui/cfixedtext.h |
| 15 | 8351 | 124 |  | `api::CUePrinterError::~CUePrinterError()` (D1; também CUeCodedPrinterError) | src/uenux2/src/api/hwil/iimpressora.u32.cpp |
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
| 30 | 10685 | 473 |  | `api::CDataText<vota::CEscolheOpcao::COpcaoDS>::GetText() const` (COpcaoDS::operator() inlinado: `"{}-{}"`) | src/uenux2/src/api/gui/cdatatext.h |
| 31 | 10686 | 12 |  | `CDataText<COpcaoDS>::~CDataText()` (D0, thunk para o 6052) | src/uenux2/src/api/gui/cdatatext.h |
| 32 | 10688 | 12 |  | `CDataText<COpcaoDS>::~CDataText()` (D1, thunk para o 6053) | src/uenux2/src/api/gui/cdatatext.h |
| 33 | 10726 | 643 |  | `api::CInputFieldMT::Draw(IScreenMT&) const` | src/uenux2/src/api/gui/cinputfieldmt.cpp |
| 34 | 10746 | 216 |  | `api::CDataTextFmt<CTextSource>::GetText() const` (snprintf 512) | src/uenux2/src/api/gui/cdatatext.h |
| 35 | 10747 | 94 |  | `CDataTextFmt<CTextSource>::~CDataTextFmt()` (D0) | src/uenux2/src/api/gui/cdatatext.h |
| 36 | 10748 | 91 |  | `CDataTextFmt<CTextSource>::~CDataTextFmt()` (D1) | src/uenux2/src/api/gui/cdatatext.h |
| 37 | 10820 | 830 |  | `api::AudioCollector::play_speech(const short*, size_t)` | src/uenux2/src/api/audio/crhvoicetexttospeech.cpp |
| 38 | 10821 | 5 |  | `api::AudioCollector::get_audio_buffer_size() const` (100) | src/uenux2/src/api/audio/crhvoicetexttospeech.cpp |
| 39 | 10822 | 47 |  | `api::AudioCollector::~AudioCollector()` (D0) | src/uenux2/src/api/audio/crhvoicetexttospeech.cpp |
| 40 | 10838 | 10 |  | `api::CRHVoiceTextToSpeech::~CRHVoiceTextToSpeech()` (D0) | src/uenux2/src/api/audio/crhvoicetexttospeech.cpp |
| 41 | 10840 | 44 |  | `api::AudioCollector::~AudioCollector()` (D1) | src/uenux2/src/api/audio/crhvoicetexttospeech.cpp |
| 42 | 10841 | 75081 |  | `api::CRHVoiceTextToSpeech::Sintetiza(const std::string&, const SParametrosFala&)` (75 KB, RHVoice inlinado) | src/uenux2/src/api/audio/crhvoicetexttospeech.cpp |
| 43 | 10846 | 403 |  | `api::CTimerScheduler::CreateTimer(const milliseconds&, const std::function<void()>&)` | src/uenux2/src/api/util/ctimerscheduler.cpp |
| 44 | 10847 | 45 |  | `api::CTimer::Stop()` | src/uenux2/src/api/util/ctimerscheduler.cpp |
| 45 | 10848 | 9 |  | `api::CTimer::SetInterval(int64_t)` | src/uenux2/src/api/util/ctimerscheduler.cpp |
| 46 | 10851 | 7 |  | `api::CTimer::IsRunning() const` | src/uenux2/src/api/util/ctimerscheduler.cpp |
| 47 | 10852 | 13 |  | `api::CTimer::~CTimer()` (D0) | src/uenux2/src/api/util/ctimerscheduler.cpp |
| 48 | 10853 | 2 |  | `api::CSystemDateTime::SetDataHora(time_t)` (vazio) | src/uenux2/src/api/util/csystemdatetime.cpp |
| 49 | 10854 | 67 | sim | `CDefaultGenericFactory<ISemaphore, CPosixSemaphore>::Create()` | src/uenux2/src/api/pattern/igenericfactory.u32.cpp |
| 50 | 10855 | 7 | sim | `api::CSystemDateTime::GetDataHora() const` (time(nullptr)) | src/uenux2/src/api/util/csystemdatetime.cpp |
| 51 | 10864 | 12 |  | `CDefaultGenericFactory<IRWSyncCtl, CPosixRWMutex>::Create()` | src/uenux2/src/api/pattern/igenericfactory.u32.cpp |
| 52 | 10869 | 12 | sim | `CDefaultGenericFactory<ISyncCtl, CPosixMutex>::Create()` | src/uenux2/src/api/pattern/igenericfactory.u32.cpp |
| 53 | 10878 | 91 |  | `CDefaultGenericFactory<IThreadImpl, simulador::CWasmThread>::Create()` | src/uenux2/src/api/pattern/igenericfactory.u32.cpp |
| 54 | 10887 | 12 |  | `api::CFormPart::~CFormPart()` (D0, thunk para o 6026) | src/uenux2/src/api/gui/cformpart.cpp |
| 55 | 10889 | 20 |  | `api::CFormPart::Imprime() const` | src/uenux2/src/api/gui/cformpart.cpp |
| 56 | 10899 | 253 |  | `api::CMenuValidation::IsValidChar(char) const` | src/uenux2/src/api/gui/cinputmenufield.u32.cpp |
| 57 | 10959 | 34 |  | `api::CRectField::GetClassName() const` ("CRectField") | src/uenux2/src/api/gui/crectfield.cpp |
| 58 | 10960 | 27 | sim | `api::CRectField::Draw(IScreen&) const` | src/uenux2/src/api/gui/crectfield.cpp |
| 59 | 10961 | 82 |  | `api::CQRCodeImageFieldPaper::~CQRCodeImageFieldPaper()` (D0) | src/uenux2/src/api/gui/cqrcodeimagefieldpaper.cpp |
| 60 | 10962 | 79 |  | `api::CQRCodeImageFieldPaper::~CQRCodeImageFieldPaper()` (D1) | src/uenux2/src/api/gui/cqrcodeimagefieldpaper.cpp |
| 61 | 10963 | 30 |  | `api::CQRCodeImageFieldPaper::Draw(IPaper&) const` | src/uenux2/src/api/gui/cqrcodeimagefieldpaper.cpp |
| 62 | 11012 | 43 |  | `api::CNewLineFieldPaper::Draw(IPaper&) const` | src/uenux2/src/api/gui/cnewlinefieldpaper.cpp |
| 63 | 11013 | 12 |  | `api::IFormFieldBase<IPaper>::~IFormFieldBase()` (D1; também CCutFieldPaper, CNewLineFieldPaper) | src/uenux2/src/api/gui/iformfield.h |
| 64 | 11015 | 15 |  | `api::CCutFieldPaper::Draw(IPaper&) const` | src/uenux2/src/api/gui/ccutfieldpaper.cpp |
| 65 | 11047 | 28 |  | `api::CPreShowClearMT::PreShow(IScreenMT&)` | src/uenux2/src/api/gui/cpreshowclearmt.cpp |
| 66 | 11048 | 20 |  | `api::CClockFieldMT::Draw(IScreenMT&) const` | src/uenux2/src/api/gui/cclockfieldmt.cpp |
| 67 | 11049 | 25 |  | `api::CBuzzFieldMT::Draw(IScreenMT&) const` | src/uenux2/src/api/gui/cbuzzfieldmt.cpp |
| 68 | 11052 | 12 |  | `api::IFormFieldBase<IScreenMT>::~IFormFieldBase()` (D1; também CBeep/CBuzz/CClock/CLedFieldMT) | src/uenux2/src/api/gui/iformfield.h |
| 69 | 11053 | 20 |  | `api::CBeepFieldMT::Draw(IScreenMT&) const` | src/uenux2/src/api/gui/cbeepfieldmt.cpp |
| 70 | 11066 | 34 |  | `api::CLineField::GetClassName() const` ("CLineField") | src/uenux2/src/api/gui/clinefield.cpp |
| 71 | 11067 | 86 |  | `api::CLineField::Rect() const` | src/uenux2/src/api/gui/clinefield.cpp |
| 72 | 11068 | 32 |  | `api::CLineField::Draw(IScreen&) const` | src/uenux2/src/api/gui/clinefield.cpp |
| 73 | 11107 | 432 |  | `CDataText<CFormBuilder::AddStatusHeader(unsigned)::$_0>::GetText() const` (bateria "{: >3}%") | src/uenux2/src/api/gui/cdatatext.h |
| 74 | 11139 | 17 |  | `api::CPreShowClearScreen::PreShow(IScreen&)` | src/uenux2/src/api/gui/cpreshowclearscreen.cpp |
| 75 | 11141 | 34 |  | `api::CFillField::GetClassName() const` ("CFillField") | src/uenux2/src/api/gui/cfillfield.cpp |
| 76 | 11142 | 25 |  | `api::CFillField::Draw(IScreen&) const` | src/uenux2/src/api/gui/cfillfield.cpp |
| 77 | 11153 | 9 |  | `api::CInputFieldControl<IScreen>::Rect() const` (SRect vazio) | src/uenux2/src/api/gui/cinputfieldcontrol.h |
| 78 | 11155 | 21 | sim | `api::CInputFieldControl<IScreen>::GetClassName() const` ("CInputFieldControl") | src/uenux2/src/api/gui/cinputfieldcontrol.h |
| 79 | 11650 | 105 |  | `api::IObservable<std::shared_ptr<IImage>>::~IObservable()` (D0) | src/uenux2/src/api/pattern/iobservable.h |
| 80 | 11651 | 102 |  | `api::IObservable<std::shared_ptr<IImage>>::~IObservable()` (D1) | src/uenux2/src/api/pattern/iobservable.h |
| 81 | 12008 | 7 |  | `api::CSubReport::EhRelatorio() const` (bool +36) | src/uenux2/src/api/gui/reports/csubreport.cpp |
| 82 | 12009 | 20 |  | `api::CSubReport::ImprimeCabecalho() const` | src/uenux2/src/api/gui/reports/csubreport.cpp |
| 83 | 12010 | 10 |  | `api::CSubReport::TemCabecalho() const` | src/uenux2/src/api/gui/reports/csubreport.cpp |
| 84 | 12011 | 23 |  | `api::CSubReport::TemNome() const` | src/uenux2/src/api/gui/reports/csubreport.cpp |
| 85 | 12012 | 49 |  | `api::CSubReport::GetVia() const` | src/uenux2/src/api/gui/reports/csubreport.cpp |
| 86 | 12013 | 119 |  | `api::CSubReport::~CSubReport()` (D0) | src/uenux2/src/api/gui/reports/csubreport.cpp |
| 87 | 12014 | 12 |  | `api::CSubReport::~CSubReport()` (D1, thunk para o 3904) | src/uenux2/src/api/gui/reports/csubreport.cpp |
| 88 | 12287 | 39 |  | `CDataTextFmt<std::function<std::string(const std::string&)>>::GetText() const` | src/uenux2/src/api/gui/cdatatext.h |
| 89 | 12290 | 91 |  | `CDataTextFmt<std::function<...>>::~CDataTextFmt()` (D0) | src/uenux2/src/api/gui/cdatatext.h |
| 90 | 12291 | 88 |  | `CDataTextFmt<std::function<...>>::~CDataTextFmt()` (D1) | src/uenux2/src/api/gui/cdatatext.h |
| 91 | 12301 | 32 |  | `CDataText<std::function<std::string()>>::GetText() const` | src/uenux2/src/api/gui/cdatatext.h |
| 92 | 12317 | 12 |  | `CDataTextFmt<std::string(*)(const std::string&)>::~CDataTextFmt()` (D0) | src/uenux2/src/api/gui/cdatatext.h |
| 93 | 12323 | 12 |  | `CDataTextFmt<std::string(*)(const std::string&)>::~CDataTextFmt()` (D1) | src/uenux2/src/api/gui/cdatatext.h |
| 94 | 12473 | 58 |  | `api::IInputValidation::IsValidChar(char) const` | src/uenux2/src/api/gui/iinputvalidation.h |
| 95 | 12502 | 12 |  | `api::IInputValidation::~IInputValidation()` (D1, todas as 5 validações) | src/uenux2/src/api/gui/iinputvalidation.h |
| 96 | 12520 | 16 |  | `CDataImage<bind<CriaTelaVisualizacaoCandidato::$_0&, const CDadosCandidato&>>::GetImage() const` | src/uenux2/src/api/gui/cdataimage.h |
| 97 | 12522 | 104 |  | `CDataImage<bind<...>>::~CDataImage()` (D0) | src/uenux2/src/api/gui/cdataimage.h |
| 98 | 12524 | 101 |  | `CDataImage<bind<...>>::~CDataImage()` (D1) | src/uenux2/src/api/gui/cdataimage.h |
| 99 | 12564 | 12 |  | `CDataTextFmt<const std::string(*)(const std::string&)>::~CDataTextFmt()` (D0) | src/uenux2/src/api/gui/cdatatext.h |
| 100 | 12567 | 12 |  | `CDataTextFmt<const std::string(*)(const std::string&)>::~CDataTextFmt()` (D1) | src/uenux2/src/api/gui/cdatatext.h |
| 101 | 12580 | 57 | sim | `CDataText<const std::string& (*)()>::GetText() const` | src/uenux2/src/api/gui/cdatatext.h |
| 102 | 12595 | 12 |  | `CMaskedTextField<CFramedText>::~CMaskedTextField()` (D0, thunk para o 6062) | src/uenux2/src/api/gui/cmaskedtextfield.h |
| 103 | 12604 | 12 |  | `CMaskedTextField<CFramedText>::~CMaskedTextField()` (D1, thunk para o 6063) | src/uenux2/src/api/gui/cmaskedtextfield.h |
| 104 | 12649 | 12 | sim | `CDataText<comum::CCandidaturasDSNome>::GetText() const` | src/uenux2/src/api/gui/cdatatext.h |
| 105 | 12658 | 12 |  | `api::IFormFieldBase<IScreen>::~IFormFieldBase()` (D1; também CRect/CLine/CFillField) | src/uenux2/src/api/gui/iformfield.h |
| 106 | 12713 | 12 |  | `CMaskedTextField<CGrayedFramedText>::~CMaskedTextField()` (D0) | src/uenux2/src/api/gui/cmaskedtextfield.h |
| 107 | 12719 | 12 |  | `CMaskedTextField<CGrayedFramedText>::~CMaskedTextField()` (D1) | src/uenux2/src/api/gui/cmaskedtextfield.h |
| 108 | 12784 | 12 | sim | `CDataText<std::string (*)()>::GetText() const` | src/uenux2/src/api/gui/cdatatext.h |

Funções relacionadas reconstruídas ou documentadas aqui, mas listadas em outras unidades: 10850 `CTimer::Start`, 10542 (a
lambda de timer de `CTextFieldUpdateMT`), 5442 `CRHVoiceTextToSpeech::SelecionaPerfil`, 10842 o seu construtor, 10888
`~CFormPart` (u24), 12593/12708 `CMaskedTextField::Draw` (u15), 11051/11014 os `GetClassName`s padrão, 6404
(`CDataTextFmt::GetText` com ponteiro de função), 5387/10368 (`GetText` de `CComparecimentoMesariosDS`), 12641 / 6330
(instâncias de `CDataImage`), 4022 (`IsValid`), 10900 (`CMenuValidation::IsValid`), 3660 (construtor de `CSubReport`),
3875 (`CLp::Imprime`).

---

## 15. Questões em aberto

* Os nomes oficiais dos slots de `ISubReport` e do bool (+36) e da parte de cabeçalho (+28) de `CSubReport`.
* O significado dos dois números de `CBuzzFieldMT` ((51, 10) e (52, 5)) e do slot 13 de `IScreenMT` (chamado depois de
  `Clear()` por `CPreShowClearMT`, no-op no mock).
* Os papéis exatos dos dois mutexes de `CTimer` (+92, +120) e o loop de trabalho (ausente do binário).
* Se `CSystemDateTime::SetDataHora` também é vazio na urna, ou só no build Emscripten.
* O int de `AudioCollector` em +4 (sempre 0) e a ordem dos slots dentro de dois grupos de `RHVoice::client`
  (do `client.hpp` do RHVoice 1.14). As assinaturas wasm separam o slot 4 ((this)->i32, retorna 0),
  os slots 5–7 ((this, x)->i32, ICF 371), os slots 8–11 ((this, x, y)->i32, func 1908) e o slot 12 ((this)->void);
  só a ordem dentro de {5,6,7} e {8..11} é suposta.
* Se `CDataTextFmt<SRC>` usa sobrecargas/especialização ou `if constexpr` para os seus três comportamentos.
