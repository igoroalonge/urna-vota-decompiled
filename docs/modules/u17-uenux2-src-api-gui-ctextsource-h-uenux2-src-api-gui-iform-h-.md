# u17: a pilha de formulários (`IForm`), a entrada de teclado, a árvore de hashes de arquivos e os estados do terminal do mesário

A unidade u17 é um apanhado de **106 funções wasm** que as ferramentas atribuíram a onze arquivos originais:
`uenux2/src/api/gui/{ctextsource.h, iform.h, iresource.h, iscreen.h, primitives.cpp, reports/creport.cpp}`,
`uenux2/src/api/hash/{chasharquivo.cpp, cmontadorhash.cpp}` e
`uenux2/src/api/hwil/{iimpressora.h, iimpressorarelatorios.cpp, iinput.h}`.
Só cerca de metade das funções pertence de fato a esses arquivos. As demais foram colocadas aqui porque uma pequena
função inline de um desses headers está compilada dentro delas:

| função inline (srcloc) | inlinada em | o que a função hospedeira realmente é |
|---|---|---|
| `CTextSource::CTextSource` (ctextsource.h:37) | 652, 1150 | `vota::CPedeIdentidade::GetInst`, `vota::CMostraEleitorVotando::GetInst` |
| `CInteractiveForm<IScreenMT,IInputMT>::Read` (cinteractiveform.h:57, que inlina `IInput::GetKey`, iinput.h:86) | 10415, 10430, 10439, 10443, 10590, 10626, 10721, 10731 | `ProcessInput` de oito estados do terminal do mesário |
| `IInput::GetKey` (iinput.h:86) | 6319, 11024 | `IInputField<IScreen/IScreenMT>::Read` (a máquina de estados de tecla para texto de todo campo de entrada) |
| `getResourceMovie` (iresource.h:87) | 1593, 3076 | dois helpers de telas do eleitor de `ctelasvota.cpp` que adicionam as animações GIF |
| `SRect::Top/Bottom` (primitives.cpp:26/38) | 5537 | `CFramedText::DesenhaCaracter` (desenha uma caixa de dígito) |
| ctor/dtor de `CScopedReportFile` (creport.cpp:31/37) | 3654 | "renderizar um relatório em um arquivo" |
| `CalculaHash`, `CalculaHashGeral`, `CalculaHashArquivo`, `CHashDiretorio::ValidaObjeto` (cmontadorhash.cpp:49/78/91/98, chashdiretorio.cpp:41) | 5360 | `CMontadorHash::CriaHashesDiretorio` (as ferramentas escolheram o errado dos seis srclocs) |
| (só pelos chamadores) | 3632, 5341, 5393, 5396, 5401, 5420, 5421, 2880, 10209, 10214, 10722, 10627, 10440, 10762 | mais estados do mesário, `CInicioVotacao::GetInst`, helpers do construtor de formulários |

15 das 106 funções executaram durante os votos gravados (`analysis/runtime/*.functions.tsv`): a pilha de
formulários do eleitor (`IForm<IScreen>` Show/Redraw/OnActivate/GetRenderForm e a notificação da pilha), o
tratamento de teclas do campo de entrada (6319), as caixas de dígitos (5488, 5537), as animações GIF do tipo de voto e do cargo
(1593, 3076, 5543, 6696) e `CThreadVota::CriaTick` (807).

**O que ela faz no processo de votação.** Tudo o que a urna mostra passa por `IForm`: as telas do
eleitor, o microterminal do mesário (*terminal do mesário*, LCD 4×40) e os
relatórios impressos/gravados. Toda tecla que o eleitor ou o mesário pressiona termina em `IInputField::Read`. No
fim do dia (*encerramento*), o mesário encerra a votação no microterminal
(`CPedeTituloEncerramento` → `CConfirmaEncerramento`, §7.5), e entre os arquivos de resultado gravados nas
mídias está `hash.dat`, a lista de hashes de todos os arquivos da urna, montada por `CMontadorHash` (§6).

Arquivos-fonte reconstruídos:

```
src/uenux2/src/api/gui/iform.h                  IForm<MEDIA>, FormControlBlock, form stack, FindFieldAs
src/uenux2/src/api/gui/iscreen.h                IScreen / IScreenMT (slot list, GetFontMetrics, dtors)
src/uenux2/src/api/gui/ctextsource.h            CTextSource
src/uenux2/src/api/gui/iresource.h              IResource + getResourceFile/getResourceMovie/isResource
src/uenux2/src/api/gui/primitives.cpp           SRect::Left/Top/Right/Bottom (u15 wrote MoveTo/Union)
src/uenux2/src/api/gui/reports/creport.cpp      CReport::Imprime, CScopedReportFile, GeraRelatorioEmArquivo
src/uenux2/src/api/gui/cframedtext.u17.cpp      fragment: CFramedText::DesenhaCaracter (owner u15)
src/uenux2/src/api/gui/iinputfield.u17.h        fragment: IInputField<MEDIA>::Read
src/uenux2/src/api/hash/chasharquivo.h / .cpp   CHashArquivo (+ CHashDiretorio declaration)
src/uenux2/src/api/hash/cmontadorhash.h / .cpp  CMontadorHash, CalculaHashArquivo
src/uenux2/src/api/hwil/iimpressora.h           IImpressora defaults, CUePrinterError, CUeCodedPrinterError
src/uenux2/src/api/hwil/iimpressorarelatorios.cpp  IImpressoraRelatorios::SetStyle
src/uenux2/src/api/hwil/iinput.h                IInput::GetKey, KeyName
src/uenux2/src/app/comum/asn/cconversorhasharquivo.u17.cpp  CConversorHashArquivo::DoDesconverte
src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp  22 mesário-terminal states / accessors
src/uenux2/src/app/vota/eleitor/u17-foreign-fragments.cpp   CInicioVotacao::GetInst + the 2 GIF helpers
```

Glossário usado abaixo: *mesário*; *terminal do mesário* / MT, o microterminal; *eleitor*;
*título (de eleitor)*, o número de inscrição do eleitor (12 dígitos); *seção*, a seção eleitoral; *habilitação*,
a liberação de um eleitor para votar; *justificativa*, a justificativa formal de ausência por um eleitor de outra
seção; *encerramento*, o encerramento da votação; *zerésima*, o "relatório zero" impresso antes da votação;
*treinamento de eleitores*, o modo de treinamento de eleitores; MI/ME = *mídia interna/externa* (flash
interna/externa); MV, *mídia de votação*.

---

## 1. Classes e como se relacionam (RTTI)

```
api::IForm<api::IScreen>       (vtable @1578040, 72 B) ── CInteractiveForm<IScreen, IInputKbd>   (@1579176, 92 B, u15)
api::IForm<api::IScreenMT>     (vtable @1579944, 72 B) ── CInteractiveForm<IScreenMT, IInputMT>  (@1580288, 92 B, u15)
api::IForm<api::IPaper>        (vtable @1581100, 72 B) ── IFormImpressao<IPaperRelatorios>
std::__shared_ptr_emplace<api::FormControlBlock>        (lifetime token of a form, 128 B)
api::IScreen   (@1529100, 39 slots) ── simulador::CWasmScreen    (canvas, web build)
api::IScreenMT (@1529856, 20 slots) ── simulador::CWasmScreenMT
api::IResource                      ── simulador::CWasmResource  (@1530184)
api::IImpressora ── api::IImpressoraRelatorios ── simulador::CWasmNullPrinter (@1530652)
ecourna::api::exception::CBaseError<EUePrinterError,{4450,4650}> ── api::CUePrinterError (@1530772)
                                                                   └─ api::CUeCodedPrinterError (@1583872)
api::IText ── CDataText<CTextSource> (@1590656), CDataTextFmt<CTextSource> (@1587072)   (CTextSource: no RTTI)
comum::asn::IConversorASN<ModuloTiposEleitorais::ArquivoAssinatura, api::hash::CHashArquivo>
          └─ comum::asn::CConversorHashArquivo (@1599000)
api::hash::CHashArquivo (24 B), api::hash::CHashDiretorio (36 B), api::hash::CMontadorHash (static only)
api::CState ── comum::CAppState ── vota::CPedeIdentidade, CMostraEleitorVotando, CNomeEleitor,
      CSincronismoOperador, CDesabilitaAudioEleitor, CEleitorVotouNaoVotou, CEleitorDemorando,
      CPerguntaEleitorVotando, CPerguntaCodigoSuspensao, CVerificaDadoEleitor, CDadoEleitorNaoConfere,
      CRegistraDigitalOperador, CPedeAnoNascimento, CAnoInformadoInvalido, CEleitorMenor16Anos,
      CValidaIdentidade, CIdentidadeInvalida, CProcuraEleitor, CPedeTituloEncerramento,
      CEncerramentoAntecipado, CTituloEncerramentoInvalido, CConfirmaEncerramento,
      CRegistroMesarioEncerrado, CFinalizaOperador (operator thread) and CInicioVotacao (voter thread)
```

## 2. `IForm<MEDIA>`: formulários e a pilha de formulários por dispositivo

### 2.1 Layout e protocolo virtual

`IForm` (72 bytes): `+4` flag de ativo, `+8` campos `vector<shared_ptr<IFormField<MEDIA>>>`, `+20`
hook de pré-exibição `shared_ptr<IPreShow<MEDIA>>` (p. ex. `CPreShowClearMT` limpa o LCD), `+28` `std::mutex`
(24 bytes, inicializado com zeros, portanto não recursivo), `+52` `shared_ptr<FormControlBlock>`, `+60` nome.
O construtor (corpo compartilhado `vota_f6024`, fora desta unidade) chama `SetForm(this)` em todo campo, então o
ponteiro de volta de um campo (`IFormFieldBase +8`) é o próprio `IForm*`. (O `gui-common.u15.h` da u15 chama esse
ponteiro de "FormControlBlock*"; o layout que ele descreve, `+4` ativo e `+28` mutex, é o de `IForm`.)

| slot | método | IScreen | IScreenMT | IPaper |
|---|---|---|---|---|
| 0/1 | `~IForm` / de exclusão | 2781 / 11136 | 2776 / 11045 | 5514 / 5513 |
| 2 | `Show()`: o formulário substitui a pilha | 11135 → 5541 | 11044 → 5520 | 11009 (inlinado) |
| 3 | `ShowOnTop()`: o formulário vai para o topo | 11134 → 5540 | 11043 → 5519 | 11008 (inlinado) |
| 4 | `Redraw()`: redesenha os campos sujos + refresh do dispositivo | 11133 | 11042 | 11007 |
| 5 | `GetRenderForm()` = `CPolySingletonList::instance<MEDIA>()` (iform.h:178) | 11132 | 11041 | 11004 |
| 6 | `SetFocus(field)` (não faz nada, ICF 425; CInteractiveForm sobrescreve) | 425 | 425 | 425 |
| 7 | `OnActivate()`: desenha tudo, inicia os timers | 5539 | 5518 | 11005 |

O refresh do dispositivo é o slot 27 de `IScreen` (`CWasmScreen::vf27` registra `Refresh()` no log e publica
`{"refreshed":true}` no canal `vota:screen` da página web) e o slot 14 de `IScreenMT`. `IPaper` não
tem nenhum: o código chama `GetRenderForm()` uma segunda vez e descarta o resultado.

### 2.2 A pilha (máquina de estados)

Cada MEDIA tem um `std::vector<IForm*>` estático (IScreen @1832632, IScreenMT @1832892, IPaper @1833528)
protegido por um mutex estático. O topo da pilha é o único formulário *ativo*.

* **Show** (`DoShow`): desativa o topo (active := false, `Stop()` em todo campo), limpa a pilha, empilha
  o formulário, chama `OnActivate()` nele, publica.
* **ShowOnTop** (`DoShowOnTop`): se o formulário já está no topo, nada (nem mesmo uma publicação);
  caso contrário, desativa o topo, remove o formulário de onde ele estiver, empilha-o, `OnActivate()`, publica.
* **Remove** (a partir de `~IForm`; fora de linha para IScreen, func 5987, também chamada por
  `vota::CEmitirMaisBU::ProcessInput`): se o formulário estiver na pilha: desativa-o se estava no topo,
  apaga-o e então *redesenha todos os formulários restantes de baixo para cima* (pré-exibição + `Draw` de cada campo),
  reativando o novo topo; publica.
* **RemoveAll** (`~IScreen` func 5071 → 5069, `~IScreenMT` func 8741 → 3461): desempilha todos os formulários,
  desativando cada um; publica.
* **OnActivate**: active := true; sob o mutex do formulário: pré-exibição, `Draw` de todo campo, refresh; depois
  `Start()` de todo campo (cursores piscando, relógios, animações: timers de `ITimerScheduler`).
* **Redraw** (chamado por um campo que mudou, por meio do seu `Invalidate()` inlinado): sob o mutex do formulário,
  se ativo: `RedrawIfDirty` em todo campo (slot 5 do campo), refresh.

**Publicação.** Após cada mudança, a pilha é copiada como pares `{IForm*, shared_ptr<FormControlBlock>}`
(`GetFormStack`, corpo mesclado 3941) e atribuída a um observable estático (IScreen @1529260,
IScreenMT @1529940, IPaper @1581136: `{vector value; vector<IObserver*>; mutex}`), cujos observadores são
notificados (slot 2). A primeira publicação é feita uma única vez com `std::call_once` (lambdas 8902 / 8716 /
11002). **Nada no binário registra um observador**: os vetores de observadores (@1529272, @1529952,
@1581148) só são lidos, pelos laços de notificação. O `FormControlBlock` (`bool m_vivo` +
`std::shared_mutex`) existe para que um observador que guarda um `IForm*` cru possa verificar se o formulário
ainda está vivo; o destrutor pega o lock exclusivo e define `m_vivo = false` (as funcs 3328/3327 são
`__shared_mutex_base::lock/unlock` da libc++). Isso parece um hook de inspeção/teste sem usuário
neste build.

### 2.3 `FindFieldAs<T>(nome)` (func 1721, iform.h:142)

Busca linear dos campos pelo seu nome único (dado por `CFormBuilder::Add`). Uma correspondência do tipo
errado lança `CUeGuiError` 4978 "Campo encontrado, mas não é do tipo solicitado: " + nome; nenhuma correspondência
retorna `nullptr`. A única instância é `<IScreen, CTextField>`, usada por
`vota::CMostraQRCodeBU::AjustaTela` (a tela do QR code do BU) para atualizar um campo de texto no lugar.

## 3. Entrada de teclado

### 3.1 `IInput::GetKey` (iinput.h:86, sempre inlinada) e `KeyName` (func 744, iinput.h:61)

`GetKey()` lança `CBaseError<EUeHwilError>` 5170 "IInput - Nao havia um caractere disponivel" se nenhuma tecla
estiver no buffer (slot 3); caso contrário, lê uma (slot 2) e incrementa um contador (`+4`). As teclas são caracteres:
`'0'..'9'`, `'B'` BRANCO, `'C'` CONFIRMA, `'D'` CORRIGE. `KeyName` os mapeia para
"0".."9", "BRANCO", "CONFIRMA", "CORRIGE", e lança 5169 "Tecla desconhecida ({})" nos demais casos. É
usada para os rótulos das teclas nas telas e para os anúncios em áudio (*voto em áudio*)
(`CVotacaoStateAudio::PlayKey`).

### 3.2 `IInputField<MEDIA>::Read(IInput&)` (6319 tela do eleitor, 11024 microterminal)

O autômato de tecla para texto de todo campo de entrada, uma chamada por lote de teclas no buffer:

```
loop:
  no key buffered                       -> return 13 (no result yet)
  k = GetKey()
  text not full && validation.IsValidChar(k) -> append, Invalidate()
  k == 'C' && m_confirmaFinaliza        -> validation.IsValid(text) ? return 9 (CONFIRMA) : Clear()
  k == 'D' (CORRIGE):  text empty       -> return 5
                       m_corrigeLimpaTudo -> Clear(); if m_corrigeRetornaAposLimpar return 5
                       else             -> erase last char, Invalidate()
  text full && !(m_aguardaConfirma && k != 'C') -> IsValid ? return 9 : Clear()
  text empty && m_aceitaBranco && k == 'B' -> return 3 (BRANCO)
```

O `IInputValidation::IsValid` padrão (ICF 4022, compartilhado por `CNumberValidation`, `COptionValidation`
e `CControlValidation`) retorna **true para um texto vazio**, então CONFIRMA em um campo vazio retorna 9
sempre que `+52` está ligado. Todo chamador verificado nesta unidade testa o próprio texto (10626: exatamente um
caractere; 10721: não vazio; 10590: pelo menos 4; 10443: mais de 3), e uma execução headless (`--keys "  C  "`,
municipal-t1) mostra que CONFIRMA no campo vazio de Vereador mantém o eleitor em `CPedeProporcional`. Os
campos do microterminal são construídos com `+52 m_confirmaFinaliza = 1` e um `CNumberValidation("0123456789")`
(func 1151).

## 4. Recursos, animações, fontes de texto e helpers de desenho

* `IResource` (poly-singleton; web: `simulador::CWasmResource`, que lê o MEMFS). `getResourceFile`,
  `getResourceMovie`, `isResource` são inlines de uma linha (iresource.h:82/87/97).
* **Animação do tipo de voto** (1593, executou): tipo 1 → `:/resource/gifs/votoLegenda.gif`, 3 → `votoBranco.gif`,
  4 → `votoNulo.gif`, qualquer outro valor não adiciona nada. O `CMovie` fica em cache em um
  `map<pair<string,bool>, shared_ptr<CMovie>>` estático (@1833352). Com a flag ligada (telas de confirmação), um
  filme de um quadro só é montado a partir do quadro atual (imagem congelada); o quadro atual é obtido com
  `vector::at` (e copiado) antes de a flag ser testada, portanto nos dois casos. Posicionado no `SPoint` compactado
  31457920 = 0x01E00280, isto é, **(640, 480)**, com âncora 7 = canto inferior direito (5543 armazena `e[8] = pos`,
  `e[9] = 7`): o canto inferior direito da tela, como no `js_image(…, 865, 400, 415, 400)` observado
  (1280 − 415 = 865, 800 − 400 = 400; são os clipes do intérprete de Libras, 09 §3.4).
* **Animação do cargo** (3076, executou): map estático código do cargo → GIF: 1 presidente, 3 governador, 5 senador,
  6 depFederal, 7 depEstadual, 8 depDistrital, 25 conselheiroDistrital, 11 prefeito, 13 vereador; sem
  cache (o GIF é carregado de novo a cada chamada). Também em (640, 480), âncora 7 (canto inferior direito).
* `CTextSource` (ctextsource.h): um `shared_ptr<std::string>` que o dono altera e o campo de exibição
  relê; nulo → `CUeGuiError` 4977 "Texto nulo".
* `SRect::Left/Top/Right/Bottom` (primitives.cpp:20/26/32/38): UE_ASSERTs 3400–3403 "Assert (right >= l)",
  "Assert (bottom >= t)", "Assert (r >= left)", "Assert (b >= top)".
* `CFramedText::DesenhaCaracter` (5537, executou): encolhe a caixa do dígito pelo espaçamento entre caixas
  (`size <= 7 ? 1 : size/8`) e por uma margem vertical (`size <= 9 ? 1 : size/10`) e escreve o caractere
  centralizado (`IScreen::WriteText`, slot 19, cores 2 sobre 1).
* `IScreen::GetFontMetrics` padrão (8938, iscreen.h:69): lança 4982 "Método não implementado"
  (CWasmScreen a sobrescreve).

## 5. Relatórios e impressora

* `api::GeraRelatorioEmArquivo(nome, relatorio)` (3654, nome inferido): `CScopedReportFile` abre o
  "arquivo" do relatório em `IPaperRelatorios` (slot 5, creport.cpp:31), `CReport::Imprime` (5486) imprime cada
  parte (slot 2 da parte), o destrutor o fecha (slot 6, creport.cpp:37). Usada para `ze.dat` (zerésima),
  seu resumo e os relatórios de fim de dia (`CGeraRelatorios::StartState`). O mesmo `Imprime` é o corpo
  da lambda de job de impressão `api::(anonymous)::DoPrintJob::$_0` (10884).
* `IImpressora::EnableSensors/DisableSensors` (8341/8360, iimpressora.h:116/103): a implementação
  padrão lança `CUePrinterError` 4467/4466 "Not supported ({})" com a máscara de sensores. Ambos são
  thunks de constantes para um único corpo, a func 6133: *merge-similar-functions* do Binaryen (§9), não um helper
  do código-fonte. O `CWasmNullPrinter` da web não os sobrescreve (slots 6/7 da vtable = 8360/8341).
* `IImpressoraRelatorios::SetStyle` (10881, iimpressorarelatorios.cpp:22): o estilo 0 lança
  `CUeCodedPrinterError`(4468, "Invalid style", "no style", código 2); caso contrário, chama o `DoSetStyle`
  protegido (slot 12) somente quando o estilo muda. `CWasmNullPrinter::GetColumns` (slot 5) retorna
  19 colunas para o estilo 3 (largura dupla) e 38 nos demais casos.

## 6. `CMontadorHash`: a árvore de hashes de arquivos de `hash.dat` (encerramento)

### 6.1 Onde ela fica

No fim do dia, a thread do eleitor grava os arquivos de resultado (`vota::CGravaResultado`, unidade u09).
Um deles é `…-hash.dat`: `comum::CGravadorHashes::GravaResultado` (11620) →
`comum_f5359` → **`CMontadorHash::CriaHashesDiretorio` (5360)** → `comum_f5358` achata a árvore em
`vector<CHashArquivo>` com caminhos completos (`directory + file name`) → ASN.1
`ModuloHashes::EntidadeHashes { cabecalho, fase, siglaUF, identificacaoUrna OPTIONAL, versaoUENUX,
hashesArquivos SEQUENCE OF ArquivoAssinatura { nomeArquivo GeneralString, assinatura GeneralString } }`
(`src/asn1/ModuloHashes.asn`). `CConversorHashArquivo` (10267/10269) converte entre
`ArquivoAssinatura` e `CHashArquivo`; o texto do hash vai no campo chamado `assinatura`.
`CGravadorHashes` parte da raiz da urna (raiz de `comum::CPath` @1838600) e passa um conjunto de exclusão
montado a partir de `<root>dev/`, `<root>dsk/`, `<root>proc/`, `<root>sys/`, `/tmp/`, três caminhos `…dinamico/`
e `tmp/` (os detalhes cabem ao dono de cgravadorhashes.cpp).

### 6.2 Algoritmo (func 5360, recursiva; cmontadorhash.cpp:121)

```
CriaHashesDiretorio(diretorio /* ends with '/' */, ignorados):
  open diretorio with CDirReader         -> failure: CBaseError<EUeHashError> 5103
                                            "CMontadorHash::CriaHashesDiretorio - diretório [<d>] não pode ser aberto"
  for each entry nome:
     skip ".", "..", "lost+found", "tmp"                       (at every level)
     caminho = diretorio + nome + ("/" if stat() says directory)   (stat follows links)
     skip if caminho ∈ ignorados
     if lstat() says symlink:
        alvo = read_symlink(diretorio + nome); relative -> diretorio + alvo
        c = weakly_canonical(alvo)
        skip if c(+"/") ∈ ignorados, or any parent directory of c (+"/") ∈ ignorados (stops at the root)
     regular file (S_IFREG) -> arquivos ; anything else -> diretorios (recursed)
  sort both lists (byte order of the full path)
  for each file f (in order):
     H = Base64(SHA-512(contents of f))     (CalculaHash :78 over CalculaHashArquivo :49, 4096-byte reads, "rb")
     nome = text after the last '/'
     if nome ∉ {"uenux.cfg", "avbootcfg.vsu"}: ms_hashGeral = CalculaHashGeral(ms_hashGeral, H)
     push CHashArquivo(nome, H)            (ValidaObjeto: 5100 empty name, 5101 empty hash)
  for each sub-directory s: push CriaHashesDiretorio(s, ignorados)
  return CHashDiretorio(diretorio, files, subdirs)   (ValidaObjeto chashdiretorio.cpp:41: 5102 empty name)

CalculaHashGeral(G, H) (:91/:98) = G empty ? H : Base64(SHA-512(bytes(G) ‖ bytes(H)))
```

O hash e a codificação de texto são factories poly-singleton: `IGenericFactory<ecourna::api::security::IHash>`
(o único `IHash` do binário é `CSha512`, OpenSSL "SHA2-512") e
`IGenericFactory<ITextEncoding>` (Base64). O encadeamento é exatamente a regra do "HASH GERAL" que
reproduz as listas publicadas pelo TSE (docs/libraries/openssl.md §10). Duas correções àquele capítulo:
`uenux.cfg` e `avbootcfg.vsu` **não são excluídos** da árvore (eles recebem sua própria entrada em
`hash.dat`); apenas ficam de fora do hash geral. E entradas chamadas `tmp` são puladas em todos os
níveis, como `lost+found`.

**O hash geral é só de escrita neste build.** Ele é acumulado em uma `std::string` estática da classe
(@1910016, destruída na saída pela 10259) que nenhuma outra função lê, e nunca é zerado, então uma segunda
chamada encadearia sobre o valor da primeira.

**Build web.** Nenhuma das factories é registrada: o primeiro arquivo faz `CPolySingletonList::instance`
lançar "PolySingleton - solicitada uma instancia nao criada N3api15IGenericFactoryIN7ecourna3api8security5IHashEEE"
(cpolysingletonlist.h). A execução com patch de `docs/bu/codepath.md` §8.4 parou exatamente aí, deixando
`hash.dat` vazio. No simulador sem patch, o caminho nunca é alcançado (não há encerramento sem a
thread do operador).

## 7. Os estados do mesário (microterminal) desta unidade

Todos eles rodam na thread do operador, que o build web nunca executa (u10 §2). Eles são a lógica do microterminal
da urna real. As coordenadas são `{coluna, linha}` no LCD 4×40.

### 7.1 Ocioso e identificação

* **`CPedeIdentidade`** (construtor em 652): a tela ociosa. Modo normal:
  `Digite o Título ou o CPF` (uma frase por tipo de identidade que a eleição aceita, unidas com " ou "),
  uma entrada de 12 dígitos em `{1,2}`, os votos até o momento (`GetTextoQtdVotaram`) + `"/{:04}"` dos
  *aptos* da seção (eleitores aptos a votar, contados uma vez na construção), o indicador de áudio e uma linha de status em
  `{1,4}` atualizada a cada 300 ms a partir de um `CTextSource` (`CTextFieldUpdateMT`). Modo de treinamento de eleitores:
  "TREINAMENTO DE ELEITORES", "Votos:", "CORRIGE: outras opções", "CONFIRMA: votar". Ticks de 60 s, 5 s
  e 1 s; o construtor também sorteia o horário da primeira inspeção da cabine
  (`SorteiaProximaInspecao`).
* **`CValidaIdentidade`** (10627/10626): o número digitado é verificado por todo
  validador `CValidadorIdentidade`. 0 correspondências → `CIdentidadeInvalida` ("Identidade: <n> / Número
  errado", bipe); 1 → log "Identificador digitado pelo mesário foi: (<tipo>)", define o tipo,
  `CProcuraEleitor`; várias → menu "Tipo de identidade digitada / 1-Título de eleitor / 2-CPF /
  3-Identificador … ESCOLHA O TIPO:" com uma entrada de 1 dígito (`aguardaConfirma` false: digitar o dígito
  basta). Uma escolha válida registra o mesmo texto no log e vai para `CProcuraEleitor`. Uma escolha que não é
  exatamente um caractere (CONFIRMA no campo vazio) não retorna estado; um dígito que não é uma opção listada
  também registra "Identificador inválido para o tipo informado". Nos dois casos, ProcessInput armazena
  **apenas um resultado não nulo**, então o menu permanece (`m_proximoEstado` ainda é `this`). CORRIGE registra
  "Habilitação cancelada durante confirmação de dado do eleitor" e volta para `CPedeIdentidade`.
* **`CNomeEleitor`** (5401): nome, identidade, "Seq:", "Seção:", texto do TTE, "CORRIGE: cancelar /
  CONFIRMA: prosseguir"; nome do formulário `telaNomeEleitor`, teclado não esvaziado.
* **`CVerificaDadoEleitor`** (10443): o ano de nascimento digitado depois que a digital falhou. Menos de 4
  dígitos: "Dado digitado está incompleto" (permanece). Igual ao ano do cadastro (`std::stoul` contra os 4 primeiros
  caracteres de `dataNascimento`): "Dado digitado confere" → **`CRegistraDigitalOperador`** (5396: o
  mesário coloca o próprio dedo, "MESÁRIO: / Posicione seu dedo POLEGAR ou INDICADOR / sobre o sensor",
  um contador inicializado com 3 (tentativas, inferido), tick de timeout de 15 s, tick de polling de 50 ms). Diferente: "Dado digitado não confere" →
  `CDadoEleitorNaoConfere` (duas telas: "tentar novamente" e "procurar o cartorio eleitoral").

### 7.2 Enquanto o eleitor está na urna

* **`CMostraEleitorVotando`** (1150): ver u10 §4.3. A linha 3 é o indicador de áudio do lado do eleitor
  (`"ÁUDIO ATIVADO"` ou em branco, func 10539), não o TTE.
* **`CEleitorDemorando`** (10440/10439): inatividade reportada pela thread do eleitor. No modo de treinamento de eleitores,
  ele passa diretamente para `CSuspensaoAutomaticaEleitor` (u10). Caso contrário, "Votou parcialmente" /
  "Não votou", log "Eleitor sem atividade por {N} segundos" (N = o timeout configurado, byte +168 de
  `CConfiguracaoEleicao`), o texto do cargo na linha 3, e CONFIRMA → **`CPerguntaEleitorVotando`**
  ("O eleitor ainda está votando?"): CONFIRMA = sim → log "Eleitor está votando", mensagem 4
  (*continua*) para a thread do eleitor, volta para `CMostraEleitorVotando`; CORRIGE = não → log "Eleitor não
  está votando" → `CPerguntaCodigoSuspensao` ("Informe seu título para / suspender a votação", entrada de 12 dígitos;
  métodos em outra unidade).
* **`CSincronismoOperador::ProcessMessage`** (10209): mensagem 1 (voto concluído) → se o eleitor usou
  áudio, `CDesabilitaAudioEleitor` ("Retire o fone de ouvido da urna"); senão, o próximo eleitor
  (`CPedeIdentidade`); após uma suspensão automática, `CEleitorVotouNaoVotou` (modo normal) ou
  `CPedeIdentidade` (treinamento). As mensagens 14/15 mostram uma tela de falha (5341): 14 → "FALHA NA MI:
  SUBSTITUA A URNA / ELEITOR DEVE VOTAR NOVAMENTE", 15 → "FALHA NA ME: SUBSTITUA A ME / ÚLTIMO VOTO
  FOI COMPUTADO".

### 7.3 Justificativa de ausência (`CPedeAnoNascimento`, 10590)

Depois do título, o mesário digita o ano de nascimento (pelo menos 4 dígitos, senão nada acontece; o texto
é guardado em `CThreadOperador +84`). O "ano da eleição" é o ushort em `CConfiguracaoEleicao +48` (ano
do `CDate` do pleito, +44). `< 1900` ou não anterior ao ano da eleição →
"Ano de nascimento inválido" (`CAnoInformadoInvalido`); ano da eleição − ano de nascimento `<= 15` → "Eleitor não
tem idade mínima" (`CEleitorMenor16Anos`: "Eleitor não pode votar ou justificar / por não ter idade
mínima"). Caso contrário:

1. `CJustificador::Justifica(título, ano)` (cjustificador.cpp:55): uma entrada já existente lança 8800 "Já
   havia justificativa para o título"; senão ela é adicionada ao map em memória. O título vem de
   `IInformacaoThreadOperador::GetIdentidadeEleitor()` (slot 25) por meio de uma **referência pendente**: o
   temporário `CEleitorIdentidade` retornado é destruído antes de `CNumeroInscricaoEleitoral` lê-lo
   (§12).
2. `EstadoGeralVota.qtdJustificativas++` (short em +54 do estado).
   Se a urna estiver desligando (byte @1832936) → `CUeDesligandoError` aqui, **antes** de qualquer escrita na MI/MV
   (a justificativa já está no map em memória e contada).
3. Sob `CApplicationContextGuard(2, "Gravando a justificativa na MI", "Ocorreu um erro durante a
   sincronização da justificativa na MI.")`: `vota.bin` é salvo na MI, `SaveCurrentInternal()`
   (cjustificador.cpp:83; 8801 "Não há dados a serem salvos na MI" se não houver nada corrente) insere o
   `CRegistroIdentificacaoEleitor` + ano na tabela `registro_justificativa` de `uenux.db`
   se ainda não estiver presente, `vota.bin` é assinado (CAssinador 122/123 conforme o turno, id de arquivo 31), fsync.
4. `GravaBancoDadosNaMI()` (4657: assina `uenux.db` e o copia para a flash externa).
5. O mesmo na MV: "Gravando a justificativa na MV", `vota.bin` salvo externamente, 4687 copia o
   pacote de assinatura `vota.vsu` MI → MV (CArquivosSavd 122 → 124 no 1º turno, 123 → 125 nos demais casos;
   a cópia de `vota.bin` na MV não é reassinada), fsync.
6. Depois, `CJustificativaEfetuada` (flag +28 = nova justificativa).

Textos de log: "Mesário cancelou entrada dos dados" (CORRIGE).

### 7.4 Início da votação (thread do eleitor): `CInicioVotacao::GetInst` (2880)

O microterminal mostra "Aguarde o horário de início da votação / Votação a partir das 8h (ou
8h30min) / Hora atual: hh:mm:ss"; o horário de início é um `CDateTime` copiado de
`CConfiguracaoEleicao +556`; um tick de 1 s da thread do eleitor o verifica (StartState/ProcessTick 11988/11987,
outra unidade). A tela do eleitor usa um formulário de `CTelasVota` (+116).

### 7.5 O fluxo de encerramento do mesário (encerramento da votação)

```
CIniciaFinalizacao / CPerguntaFilaEleitorVazia (other units)
   └─> CPedeTituloEncerramento (constructor 3631: "Informe seu título para / encerrar a votação",
        │                       12-digit input, "CORRIGE: retornar / CONFIRMA: prosseguir")
        │  StartState (10722): voter-training mode -> no título asked:
        │        now < scheduled closing time (+20) -> CEncerramentoAntecipado
        │        else                               -> CConfirmaEncerramento
        │  ProcessInput (10721):
        │     CORRIGE -> log "Operador cancelou o encerramento da votação" -> CPedeIdentidade
        │     CONFIRMA, empty -> stay
        │     CONFIRMA -> título stored in CThreadOperador +120;
        │                 not all digits, or not a valid TÍTULO once zero-padded to 12 ("{:0>{}}")
        │                     -> CTituloEncerramentoInvalido ("Título inválido: <título>", CORRIGE)
        │                 else log "Título digitado para encerramento: <título>" -> CConfirmaEncerramento
   CEncerramentoAntecipado (10731): "Encerramento de votação antecipado?"
        CORRIGE -> CPedeIdentidade, CONFIRMA -> CConfirmaEncerramento
   CConfirmaEncerramento (3632): "Encerramento da Votação / CORRIGE: cancelar / CONFIRMA: encerrar"
        ProcessInput 10734 (other unit): "Procedimento de encerramento confirmado" / "... abortado";
        registers the end of acquisition ("O fim da aquisição já havia sido registrado" ...)
   ...  the voter thread then generates the BU, RDV, reports and result files (units u09/u23; hash.dat §6)
   CFinalizaOperador (10214): "Procedimentos de Encerramento / Siga as instruções na tela do eleitor",
        m_proximoEstado = nullptr: the operator state machine stops.
```

Observe que, no modo normal, o título só é verificado quanto ao **formato** (dígitos + dígitos verificadores do TÍTULO); o
código desta unidade não o compara com uma lista de mesários.

## 8. Particularidades do build web

* Toda a parte da thread do operador (§7 exceto 7.4) é morta no simulador (CThreadOperador::Run nunca
  executa); seus acessores nunca são chamados (sonda da u10: o singleton de `CPedeIdentidade` continua nulo).
* A parte do eleitor está ativa: operações da pilha de `IForm<IScreen>`, `IInputField<IScreen>::Read`, as caixas
  de dígitos e os dois helpers de GIF executaram. `CWasmScreen::Refresh` encaminha `{"refreshed":true}` para a página.
* `CWasmNullPrinter` mantém os `EnableSensors/DisableSensors` padrão (eles lançariam "Not supported").
* `hash.dat` não pode ser produzido (sem factory de hash, §6).

## 9. Observações sobre wasm / Emscripten

* **Nomes de srcloc enganosos.** Uma função que contém um `std::source_location::current()` inlinado
  de um header recebe o nome desse header: dois acessores de estado se chamam `CTextSource::CTextSource`, os
  dois helpers de GIF `getResourceMovie`, o autômato de entrada `IInput::GetKey`, um método de CFramedText
  `SRect::Top`, CriaHashesDiretorio `CalculaHashGeral`.
* **Corpos mesclados.** `GetFormStack` é um único corpo (3941) para as três MEDIA, chamado por thunks que
  passam os endereços estáticos; os helpers de vector 1872/8872/8879 servem os três tipos `SFormInfo<MEDIA>`;
  1300 copia tanto `vector<CHashArquivo>` quanto `vector<pair<string,string>>`; o acessor do singleton de vota
  sem membros passa por `vota_f764(mutex, &inst, vtable, flags)`.
* **merge-similar-functions.** A func 6133 é o corpo mesclado de `IImpressora::DisableSensors` e
  `EnableSensors` (docs/02-wasm-binary-anatomy.md §5.4): 8360/8341 passam `(this, sensores)` e então
  suas próprias constantes `(srcloc, código)` = `(iimpressora.h:103, 4466)` / `(iimpressora.h:116, 4467)`.
* **Thunks de vtable vs. corpos.** Para IScreen/IScreenMT, os slots 2/3 são thunks de 7 bytes para funções estáticas
  da pilha (5541/5540, 5520/5519); para IPaper, os mesmos corpos estão inlinados nos slots.
* **Resíduo de locks.** `std::mutex::lock()` desaparece; `unlock()` é o stub de ICF 150; o shared_mutex do
  FormControlBlock permanece como código real da libc++ (3328/3327), com chamadas a `condition_variable::wait` que
  nunca podem bloquear neste build de thread única.
* **filesystem da libc++ inlinado.** `read_symlink` e `weakly_canonical` estão inlinados na 5360; suas
  partes fora de linha (`ErrorHandler::report` 3335, `lexically_normal` 7741, construtores de `path` 4676/4686,
  helpers de `vector<string_view>`) são atribuídas a cmontadorhash.cpp pelas ferramentas.
* **Coordenadas como um único int32.** `SPoint{x, y}` é armazenado como `x | y << 16` com alinhamento de 2 bytes; o
  descompilador mostra `p[N]:int@2 = 65569` (= `{33, 1}`) e comentários de string inline que são apenas
  coincidências da constante (p. ex. `65569 /* "dadeVersaoArquivos" */`).
* **Strings em Latin-1.** Todos os literais são ISO-8859-1 (p. ex. "CORRIGE: outras opções" tem 22 bytes).
* A interface da impressora coloca seu destrutor nos slots 10/11, depois dos métodos virtuais puros.

## 10. Para a equipe do BU: o que esta unidade contribui para o fim do dia

Esta unidade não monta o BU em si. Ela contém quatro peças do caminho do encerramento:

1. **O encerramento da votação no microterminal** (§7.5). Modo normal: o mesário digita um título; ele só
   é verificado quanto a ser composto de dígitos e ser um número de TÍTULO válido depois de completado com zeros à esquerda até 12 dígitos
   (`CValidadorIdentidade::Valida(1, "{:0>{}}")`); ele é guardado em `CThreadOperador +120` e registrado no log como
   "Título digitado para encerramento: <título>". Modo de treinamento de eleitores: sem título; antes do horário
   de encerramento programado, o MT pergunta "Encerramento de votação antecipado?". Depois, `CConfirmaEncerramento`
   ("Encerramento da Votação", CONFIRMA: encerrar) passa o controle para a thread do eleitor, e o MT mostra
   "Procedimentos de Encerramento / Siga as instruções na tela do eleitor" (`CFinalizaOperador`), após o
   que a máquina de estados do operador fica sem estado.
2. **Arquivos de relatório** (§5): `ze.dat` (zerésima) e os relatórios de fim de dia são renderizados por
   `GeraRelatorioEmArquivo` = `CScopedReportFile` (slot 5 de IPaperRelatorios "abrir arquivo" / slot 6
   "fechar") em torno de `CReport::Imprime`.
3. **`hash.dat`** (§6): `EntidadeHashes.hashesArquivos` = um `{nomeArquivo (full path), assinatura =
   Base64(SHA-512(file))}` por arquivo regular sob a raiz da urna, menos as árvores excluídas, na ordem
   dos diretórios: arquivos de um diretório ordenados por caminho, depois seus subdiretórios (profundidade primeiro). `uenux.cfg` e
   `avbootcfg.vsu` são listados, mas ficam de fora do hash geral em memória, que não é gravado em lugar nenhum
   por este build. Build web: falha no primeiro arquivo (sem factory de hash).
4. **Tela do QR do BU**: `IForm<IScreen>::FindFieldAs<CTextField>` (1721) é como
   `vota::CMostraQRCodeBU::AjustaTela` encontra o campo de texto que atualiza na tela do QR code do BU.

## 11. Questões em aberto

* O nome/papel real de `GeraRelatorioEmArquivo` (3654): ela recebe o nome do arquivo por valor e o
  `CReport` em segundo lugar, então não é um membro de `CReport`; a unidade u09 escreve a chamada como `relatorio.Gera(path)`.
* Ticks `+12` (5 s) e `+13` (1 s) de `CPedeIdentidade` (ProcessTick 10676 está em outra unidade).
* O significado do estado 2 de `CLedFieldMT` (usado apenas pela tela de falha de gravação).
* Se algum build registra um observador nos observables da pilha de formulários (uma ferramenta de teste ou de inspeção).
* `CInicioVotacao +12` recebe `CTelasVota +116`, um membro de formulário ainda sem nome em `ctelasvota.h`.
* `IInput +4` é incrementado por todo `GetKey` (contador de teclas?); nada nesta unidade o lê.

## 12. Código suspeito / estranho

* **(Rejeitado na revisão) "o menu de tipo de identidade pode deixar a máquina de estados do operador sem
  estado".** `CValidaIdentidade::ProcessInput` (10626) armazena o resultado do seu parser de escolha somente quando ele
  não é nulo (`local.tee; i32.eqz; br_if` pula o `i32.store offset=4`), exatamente como
  `CPedeTituloEncerramento`. CONFIRMA no campo vazio "ESCOLHA O TIPO:" (que `IInputField::Read`
  de fato reporta como CONFIRMA, §3.2) ou um dígito desconhecido deixa `m_proximoEstado = this`: o menu permanece.
* **Referência pendente na justificativa (código da urna real, morto no simulador).**
  `CPedeAnoNascimento::ProcessInput` (10590) chama o slot 25 de `IInformacaoThreadOperador`
  (`GetIdentidadeEleitor`, func 1535), que retorna um temporário `CEleitorIdentidade` (16 bytes,
  `std::string` em +0). O binário libera essa string imediatamente (`i32.load8_s offset=11 … call $free`)
  e depois passa o mesmo objeto para `CNumeroInscricaoEleitoral(const std::string&)` (func 1241): o
  código-fonte liga uma referência a um membro do temporário (`const std::string& t =
  GetIdentidadeEleitor().GetIdentidade();`). Com a libc++, um título de 12 dígitos não cabe no buffer de
  small string de 10 caracteres, então o construtor copia **memória de heap já liberada**; o que ele lê depende do
  alocador (o dlmalloc pode já ter escrito ponteiros da lista livre nos primeiros 8 bytes). Com uma
  biblioteca padrão cujo buffer de small string comporte 12 caracteres, os dígitos ficam dentro do objeto morto na pilha
  e o bug passaria despercebido. `IIniciaJustificativa::StartState` (10587) copia a string
  antes e está correto. Não observável aqui: a thread do operador nunca executa no build web.
* **O menu se sobrepõe à legenda das teclas.** No mesmo menu, a opção *n* é desenhada na linha *n*+1 e a
  legenda das teclas ("CORRIGE: retornar" em {1,4}, "ESCOLHA O TIPO:" em {24,4}, a entrada em {40,4}) fica na linha 4:
  uma terceira opção ("3-Identificador", 15 caracteres) ficaria em {1,4}, o mesmo ponto de "CORRIGE:
  retornar" (17 caracteres), que é adicionado depois e por isso a cobre completamente. Não foi estabelecido se algum número
  pode ser validado como os três tipos de identidade.
* **`hash.dat` não pode ser produzido no build web**: `IGenericFactory<IHash>` e
  `IGenericFactory<ITextEncoding>` não são registradas, então `CriaHashesDiretorio` lança no primeiro
  arquivo (confirmado pela execução com patch em docs/bu/codepath.md §8.4).
* **O hash geral é estado morto.** O acumulador estático de `CMontadorHash` (@1910016) é escrito para todo
  arquivo, mas nunca é lido nem zerado; um segundo `CriaHashesDiretorio` encadearia sobre o valor
  anterior.
* **Percurso frágil da árvore.** Toda entrada que não é um arquivo regular (FIFO, dispositivo, socket, symlink
  *pendente*, laço de symlinks) é tratada como diretório e percorrida recursivamente; `opendir` então falha e toda a
  geração de `hash.dat` lança 5103. Symlinks para diretórios são seguidos, então um link para um ancestral que
  não está no conjunto de exclusão gera recursão até que o limite de symlinks do kernel faça `opendir` falhar (a mesma
  exceção). Os nomes de diretório já precisam terminar com '/' (os caminhos são concatenados sem separador).
* **Código morto do operador no simulador.** 22 das funções (todas estados do operador de `vota::`) nunca executam
  no build web; seu comportamento não pode ser observado com `tools/run/headless.mjs`.
* **Métodos padrão da impressora lançam exceção.** `CWasmNullPrinter` não sobrescreve `EnableSensors/DisableSensors`;
  qualquer código que os chame no build web recebe `CUePrinterError` "Not supported (n)".
* **Observable da pilha de formulários sem observadores** (§2.2): o código de notificação, a inicialização com `call_once`
  e o lock compartilhado do `FormControlBlock` executam a cada mudança de tela à toa
  neste build (inofensivo).
* **Verificação de idade mínima só pelo ano**: `CPedeAnoNascimento` rejeita quando `anoEleição − anoNascimento <= 15`,
  sem olhar o dia ou o mês (um eleitor que faz 16 anos mais tarde no ano da eleição passa).
* Nenhum `emscripten_sleep`, código de rede ou relevante para privacidade nesta unidade.

## 13. Tabela de mapeamento completa (106 funções)

Colunas: índice wasm, tamanho em bytes, ✓ = observada executando nos votos gravados, nome dado pelas
ferramentas, símbolo reconstruído, onde ela é reconstruída (ou por que não), arquivo original, confiança.

| func | tamanho | executou | nome das ferramentas | símbolo reconstruído | reconstruído em (src/…) | arquivo original | conf. |
|---:|---:|:-:|---|---|---|---|---|
| 180 | 258 |  | `api_f180` | `api::CFormBuilderMT::Add<api::CTextFieldMT, api::CFixedText>` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp (vocabulário de header) | uenux2/src/api/gui/cformbuildermt.h (caminho inferido) | média |
| 435 | 19 |  | `vota_f435` | `api::CFormBuilderMT::Add<api::CLedFieldMT>` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp (vocabulário de header) | uenux2/src/api/gui/cformbuildermt.h (caminho inferido) | média |
| 463 | 18 |  | `ecourna_f463` | `ecourna::api::exception::CBaseError<api::EUeAssertError>::CBaseError` | biblioteca/helper inlinado (thunk do ctor mesclado de CBaseError 710; usado por UE_ASSERT, ver primitives.cpp) | ecourna-lib/ecourna/api/exception/cbaseerror.hpp | alta |
| 619 | 207 |  | `api_f619` | `api::CFormBuilderMT::Add<api::CTextFieldMT, api::CDataText<std::string (*)()>>` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp (vocabulário de header) | uenux2/src/api/gui/cformbuildermt.h (caminho inferido) | média |
| 652 | 4609 |  | `api::CTextSource::CTextSource@652` | `vota::CPedeIdentidade::GetInst` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/leidentidade/cpedeidentidade.cpp | alta |
| 744 | 625 |  | `api::KeyName` | `api::KeyName` | src/uenux2/src/api/hwil/iinput.h | uenux2/src/api/hwil/iinput.h | alta |
| 807 | 12 | ✓ | `vota_f807` | `vota::CThreadVota::CriaTick` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp (vocabulário de header) | uenux2/src/app/vota/comum/cthreadvota.cpp (caminho inferido) | média |
| 1150 | 1768 |  | `api::CTextSource::CTextSource@1150` | `vota::CMostraEleitorVotando::GetInst` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/aguardaeleitor/cmostraeleitorvotando.cpp | alta |
| 1300 | 235 |  | `comum_f1300` | `std::vector<api::hash::CHashArquivo>::vector(const vector&)` | biblioteca/helper inlinado (cópia de um vector de elementos com duas std::string; compartilhado via ICF com o vector<pair<string,string>> do código do QR code) | libcxx <vector> (instância de template) | média |
| 1444 | 141 |  | `api_f1444` | `std::pair<const uebyte, std::string>::pair<int, const char (&)[N]>(int&&, const char (&)[N])` (a chave é lida como um int de 4 bytes e truncada: a lista de inicialização escreve `{1, ":/resource/…"}` com literais int; um único corpo para todo N) | biblioteca/helper inlinado (inicializador do map cargo -> gif na 3076) | libcxx <utility> (instância de template) | média |
| 1593 | 1945 | ✓ | `api::getResourceMovie@1593` | `vota::(anonymous namespace)::adicionaAnimacaoVoto` | src/uenux2/src/app/vota/eleitor/u17-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | média |
| 1721 | 296 |  | `api::IForm<api::IScreen>::FindFieldAs` | `api::IForm<api::IScreen>::FindFieldAs<api::CTextField>` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | alta |
| 1872 | 848 | ✓ | `api_f1872` | `std::vector<api::SFormInfo<MEDIA>>::assign` | biblioteca/helper inlinado (vector::__assign_with_size, elementos {IForm*, shared_ptr} de 12 bytes; um único corpo para as 3 MEDIA) | libcxx <vector> (instância de template para iform.h) | média |
| 2220 | 233 |  | `api_f2220` | `api::hash::CHashDiretorio::~CHashDiretorio` | src/uenux2/src/api/hash/chasharquivo.h | uenux2/src/api/hash/chashdiretorio.h (caminho a partir do srcloc chashdiretorio.cpp) | média |
| 2547 | 60 |  | `api_f2547` | `std::vector<std::string_view>::~vector` | biblioteca/helper inlinado (internos de weakly_canonical / lexically_normal do filesystem) | libcxx <vector> (instância de template) | média |
| 2631 | 321 |  | `api_f2631` | `api::IForm<api::IScreenMT>::NotifyStackChanged` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | média |
| 2651 | 321 | ✓ | `api_f2651` | `api::IForm<api::IScreen>::NotifyStackChanged` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | média |
| 2776 | 761 |  | `api::IForm<api::IScreenMT>::vf0` | `api::IForm<api::IScreenMT>::~IForm` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | alta |
| 2781 | 355 |  | `api::IForm<api::IScreen>::vf0` | `api::IForm<api::IScreen>::~IForm` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | alta |
| 2880 | 2039 |  | `vota_f2880` | `vota::CInicioVotacao::GetInst` | src/uenux2/src/app/vota/eleitor/u17-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/iniciovotacao/ciniciovotacao.cpp (caminho inferido) | alta |
| 3076 | 1224 | ✓ | `api::getResourceMovie@3076` | `vota::(anonymous namespace)::adicionaAnimacaoCargo` | src/uenux2/src/app/vota/eleitor/u17-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | média |
| 3327 | 22 |  | `api_f3327` | `std::__shared_mutex_base::unlock` | biblioteca/helper inlinado (mutex do FormControlBlock, iform.h) | libcxx src/shared_mutex.cpp | média |
| 3328 | 135 |  | `api_f3328` | `std::__shared_mutex_base::lock` | biblioteca/helper inlinado (mutex do FormControlBlock, iform.h) | libcxx src/shared_mutex.cpp | média |
| 3335 | 356 |  | `api_f3335` | `std::filesystem::detail::ErrorHandler<std::filesystem::path>::report` | biblioteca/helper inlinado (read_symlink/weakly_canonical inlinados na 5360) | libcxx src/filesystem/error.h | média |
| 3370 | 23 |  | `api_f3370` | `std::filesystem::_PathCVT<char>::__append_source<std::string_view>` (= `__pn_.append(first, last)`, func 178; `string::append(string_view)` chamaria a sobrecarga (ptr, n) 160) | biblioteca/helper inlinado (chamado apenas por 7742 path::append e 7744 path::assign) | libcxx <__filesystem/path.h> (instância de template) | média |
| 3461 | 148 |  | `api_f3461` | `api::IForm<api::IScreenMT>::RemoveAll` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | média |
| 3600 | 121 |  | `api_f3600` | `api::hash::CHashArquivo::CHashArquivo` | src/uenux2/src/api/hash/chasharquivo.h | uenux2/src/api/hash/chasharquivo.h (caminho inferido) | alta |
| 3632 | 856 |  | `api_f3632` | `vota::CConfirmaEncerramento::GetInst` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/outrasopcoes/cconfirmaencerramento.cpp (caminho inferido) | alta |
| 3654 | 393 |  | `api::CScopedReportFile::CScopedReportFile` | `api::GeraRelatorioEmArquivo` | src/uenux2/src/api/gui/reports/creport.cpp | uenux2/src/api/gui/reports/creport.cpp | baixa |
| 3670 | 321 |  | `api_f3670` | `api::IForm<api::IPaper>::NotifyStackChanged` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | média |
| 3941 | 269 | ✓ | `api_f3941` | `api::IForm<MEDIA>::GetFormStack` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | média |
| 4675 | 9 |  | `api_f4675` | `std::__split_buffer<std::string_view>::~__split_buffer` | biblioteca/helper inlinado (crescimento de vector<string_view> em weakly_canonical) | libcxx <__split_buffer> (instância de template) | média |
| 4676 | 15 |  | `api_f4676` | `std::filesystem::path::path<char[1]>` | biblioteca/helper inlinado (path("") para __canonical("") em weakly_canonical) | libcxx <filesystem> (instância de template) | baixa |
| 4686 | 15 |  | `api_f4686` | `std::filesystem::path::path<char*>` | biblioteca/helper inlinado (path(buffer) em read_symlink) | libcxx <filesystem> (instância de template) | baixa |
| 5022 | 22 |  | `api_f5022` | `api::IForm<api::IScreenMT>::GetFormStack` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | média |
| 5341 | 1052 |  | `vota_f5341` | `vota::CSincronismoOperador::MostraFalhaGravacao` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/aguardaeleitor/csincronismooperador.cpp (caminho inferido) | média |
| 5360 | 9613 |  | `api::hash::CMontadorHash::CalculaHashGeral` | `api::hash::CMontadorHash::CriaHashesDiretorio` | src/uenux2/src/api/hash/cmontadorhash.cpp | uenux2/src/api/hash/cmontadorhash.cpp | alta |
| 5363 | 198 |  | `api_f5363` | `std::vector<api::hash::CHashDiretorio>::vector(const vector&)` | biblioteca/helper inlinado (ctor de cópia implícito de CHashDiretorio inlinado) | libcxx <vector> (instância de template) | média |
| 5364 | 144 |  | `api::hash::CHashArquivo::ValidaObjeto` | `api::hash::CHashArquivo::ValidaObjeto` | src/uenux2/src/api/hash/chasharquivo.cpp | uenux2/src/api/hash/chasharquivo.cpp | alta |
| 5393 | 652 |  | `vota_f5393` | `vota::CDesabilitaAudioEleitor::GetInst` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/aguardaeleitor/cdesabilitaaudioeleitor.cpp (caminho inferido) | alta |
| 5396 | 1586 |  | `api_f5396` | `vota::CRegistraDigitalOperador::GetInst` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | alta |
| 5401 | 1583 |  | `vota_f5401` | `vota::CNomeEleitor::GetInst` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/confirmaidentidade/cnomeeleitor.cpp | alta |
| 5420 | 788 |  | `vota_f5420` | `vota::CValidaIdentidade::MontaOpcoes` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/leidentidade/cvalidaidentidade.cpp (caminho inferido) | média |
| 5421 | 22 |  | `api_f5421` | `vota::CProcuraEleitor::GetInst` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/leidentidade/cprocuraeleitor.cpp (caminho inferido) | alta |
| 5486 | 55 |  | `api_f5486` | `api::CReport::Imprime` | src/uenux2/src/api/gui/reports/creport.cpp | uenux2/src/api/gui/reports/creport.cpp | média |
| 5487 | 77 |  | `api::SRect::Right` | `api::SRect::Right` | src/uenux2/src/api/gui/primitives.cpp | uenux2/src/api/gui/primitives.cpp | alta |
| 5488 | 77 | ✓ | `api::SRect::Left` | `api::SRect::Left` | src/uenux2/src/api/gui/primitives.cpp | uenux2/src/api/gui/primitives.cpp | alta |
| 5512 | 22 |  | `api_f5512` | `api::IForm<api::IPaper>::GetFormStack` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | média |
| 5513 | 10 |  | `api::IForm<api::IPaper>::vf1` | `api::IForm<api::IPaper>::~IForm [deleting]` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | alta |
| 5514 | 761 |  | `api::IForm<api::IPaper>::vf0` | `api::IForm<api::IPaper>::~IForm` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | alta |
| 5518 | 195 |  | `api::IForm<api::IScreenMT>::vf7` | `api::IForm<api::IScreenMT>::OnActivate` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | média |
| 5519 | 289 |  | `api_f5519` | `api::IForm<api::IScreenMT>::DoShowOnTop` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | média |
| 5520 | 212 |  | `api_f5520` | `api::IForm<api::IScreenMT>::DoShow` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | média |
| 5537 | 547 | ✓ | `api::SRect::Top` | `api::CFramedText::DesenhaCaracter` | src/uenux2/src/api/gui/cframedtext.u17.cpp | uenux2/src/api/gui/cframedtext.cpp | média |
| 5539 | 195 | ✓ | `api::IForm<api::IScreen>::vf7` | `api::IForm<api::IScreen>::OnActivate` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | média |
| 5543 | 333 | ✓ | `api_f5543` | `api::CFormBuilder::Add<api::CMovieField>` | biblioteca/helper inlinado (instância de template do construtor de formulários usada por 1593/3076) | uenux2/src/api/gui/cformbuilder.h (instância de template) | média |
| 6133 | 415 |  | `api_f6133` | corpo mesclado de `api::IImpressora::DisableSensors`/`EnableSensors` (merge-similar-functions do Binaryen; sem função no código-fonte) | src/uenux2/src/api/hwil/iimpressora.h (ambos os métodos) | uenux2/src/api/hwil/iimpressora.h | média |
| 6319 | 673 | ✓ | `api::IInput::GetKey@6319` | `api::IInputField<api::IScreen>::Read` | src/uenux2/src/api/gui/iinputfield.u17.h | uenux2/src/api/gui/iinputfield.h (caminho inferido) | alta |
| 6643 | 285 |  | `api_f6643` | `std::map<std::pair<std::string,bool>, std::shared_ptr<api::CMovie>>::__find_equal` | biblioteca/helper inlinado (cache de animações da 1593) | libcxx <map> (instância de template) | média |
| 6696 | 213 | ✓ | `api_f6696` | `std::vector<api::CMovieFrame>::__init_with_size` | biblioteca/helper inlinado (cópia de CMovie em 1593/3076) | libcxx <vector> (instância de template) | média |
| 7735 | 172 |  | `api_f7735` | `std::string::reserve` | biblioteca/helper inlinado (weakly_canonical: result.reserve) | libcxx <string> | média |
| 7741 | 1079 |  | `api_f7741` | `std::filesystem::path::lexically_normal` | biblioteca/helper inlinado (fim de weakly_canonical, inlinado na 5360) | libcxx src/filesystem/path.cpp | média |
| 7742 | 60 |  | `api_f7742` | `std::filesystem::path::append<std::string_view>` | biblioteca/helper inlinado (weakly_canonical: result /= part) | libcxx <filesystem> (instância de template) | média |
| 7743 | 353 |  | `api_f7743` | `std::vector<std::string_view>::push_back` | biblioteca/helper inlinado (weakly_canonical DNEParts) | libcxx <vector> (instância de template) | média |
| 7744 | 16 |  | `api_f7744` | `std::filesystem::path::assign<std::string_view>` | biblioteca/helper inlinado (weakly_canonical) | libcxx <filesystem> (instância de template) | média |
| 8341 | 17 |  | `api::IImpressora::EnableSensors` | `api::IImpressora::EnableSensors` | src/uenux2/src/api/hwil/iimpressora.h | uenux2/src/api/hwil/iimpressora.h | alta |
| 8360 | 17 |  | `api::IImpressora::DisableSensors` | `api::IImpressora::DisableSensors` | src/uenux2/src/api/hwil/iimpressora.h | uenux2/src/api/hwil/iimpressora.h | alta |
| 8716 | 193 |  | `api_f8716` | `api::IForm<api::IScreenMT>::NotifyStackChanged::$_0` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | média |
| 8741 | 17 |  | `api::IScreenMT::vf0` | `api::IScreenMT::~IScreenMT` | src/uenux2/src/api/gui/iscreen.h | uenux2/src/api/gui/iscreen.h | alta |
| 8872 | 363 |  | `api_f8872` | `std::vector<api::SFormInfo<MEDIA>>::__emplace_back_slow_path` | biblioteca/helper inlinado | libcxx <vector> (instância de template para iform.h) | média |
| 8879 | 278 |  | `api_f8879` | `std::vector<api::SFormInfo<MEDIA>>::reserve` | biblioteca/helper inlinado | libcxx <vector> (instância de template para iform.h) | média |
| 8938 | 50 |  | `api::IScreen::GetFontMetrics` | `api::IScreen::GetFontMetrics` | src/uenux2/src/api/gui/iscreen.h | uenux2/src/api/gui/iscreen.h | alta |
| 10209 | 120 |  | `vota::CSincronismoOperador::vf6` | `vota::CSincronismoOperador::ProcessMessage` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/aguardaeleitor/csincronismooperador.cpp (caminho inferido) | alta |
| 10214 | 685 |  | `vota::CFinalizaOperador::vf2` | `vota::CFinalizaOperador::StartState` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/cfinalizaoperador.cpp (caminho inferido) | alta |
| 10257 | 138 |  | `api_f10257` | `api::CUePrinterError::CUePrinterError` | src/uenux2/src/api/hwil/iimpressora.h | uenux2/src/api/hwil/iimpressora.h (caminho inferido) | média |
| 10269 | 210 |  | `comum::asn::CConversorHashArquivo::vf3` | `comum::asn::CConversorHashArquivo::DoDesconverte` | src/uenux2/src/app/comum/asn/cconversorhasharquivo.u17.cpp | uenux2/src/app/comum/asn/cconversorhasharquivo.cpp (caminho inferido) | média |
| 10415 | 2081 |  | `vota::CPerguntaEleitorVotando::vf7` | `vota::CPerguntaEleitorVotando::ProcessInput` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/aguardaeleitor/cperguntaeleitorvotando.cpp (caminho inferido) | alta |
| 10430 | 150 |  | `vota::CEleitorVotouNaoVotou::vf7` | `vota::CEleitorVotouNaoVotou::ProcessInput` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/aguardaeleitor/celeitorvotounaovotou.cpp (caminho inferido) | alta |
| 10439 | 1039 |  | `vota::CEleitorDemorando::vf7` | `vota::CEleitorDemorando::ProcessInput` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/aguardaeleitor/celeitordemorando.cpp (caminho inferido) | alta |
| 10440 | 1876 |  | `vota::CEleitorDemorando::vf2` | `vota::CEleitorDemorando::StartState` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/aguardaeleitor/celeitordemorando.cpp (caminho inferido) | alta |
| 10443 | 2761 |  | `vota::CVerificaDadoEleitor::vf7` | `vota::CVerificaDadoEleitor::ProcessInput` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/confirmaidentidade/cverificadadoeleitor.cpp (caminho inferido) | alta |
| 10590 | 4305 |  | `vota::CPedeAnoNascimento::vf7` | `vota::CPedeAnoNascimento::ProcessInput` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/justificativa/cpedeanonascimento.cpp (caminho inferido) | alta |
| 10626 | 1004 |  | `vota::CValidaIdentidade::vf7` | `vota::CValidaIdentidade::ProcessInput` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/leidentidade/cvalidaidentidade.cpp (caminho inferido) | alta |
| 10627 | 3842 |  | `vota::CValidaIdentidade::vf2` | `vota::CValidaIdentidade::StartState` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/leidentidade/cvalidaidentidade.cpp (caminho inferido) | alta |
| 10721 | 2309 |  | `vota::CPedeTituloEncerramento::vf7` | `vota::CPedeTituloEncerramento::ProcessInput` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/outrasopcoes/cpedetituloencerramento.cpp (caminho inferido) | alta |
| 10722 | 974 |  | `vota::CPedeTituloEncerramento::vf2` | `vota::CPedeTituloEncerramento::StartState` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/outrasopcoes/cpedetituloencerramento.cpp (caminho inferido) | alta |
| 10731 | 152 |  | `vota::CEncerramentoAntecipado::vf7` | `vota::CEncerramentoAntecipado::ProcessInput` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/outrasopcoes/cencerramentoantecipado.cpp (caminho inferido) | alta |
| 10762 | 18 |  | `vota::CRegistroMesarioEncerrado::vf6` | `vota::CRegistroMesarioEncerrado::ProcessMessage` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/outrasopcoes/cregistromesarioencerrado.cpp (caminho inferido) | alta |
| 10881 | 418 |  | `api::IImpressoraRelatorios::SetStyle` | `api::IImpressoraRelatorios::SetStyle` | src/uenux2/src/api/hwil/iimpressorarelatorios.cpp | uenux2/src/api/hwil/iimpressorarelatorios.cpp | alta |
| 11002 | 193 |  | `api_f11002` | `api::IForm<api::IPaper>::NotifyStackChanged::$_0` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | média |
| 11004 | 2141 |  | `api::IForm<api::IPaper>::vf5` | `api::IForm<api::IPaper>::GetRenderForm` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | alta |
| 11005 | 183 |  | `api::IForm<api::IPaper>::vf7` | `api::IForm<api::IPaper>::OnActivate` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | média |
| 11007 | 106 |  | `api::IForm<api::IPaper>::vf4` | `api::IForm<api::IPaper>::Redraw` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | média |
| 11008 | 289 |  | `api::IForm<api::IPaper>::vf3` | `api::IForm<api::IPaper>::ShowOnTop` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | média |
| 11009 | 212 |  | `api::IForm<api::IPaper>::vf2` | `api::IForm<api::IPaper>::Show` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | média |
| 11024 | 673 |  | `api::IInput::GetKey@11024` | `api::IInputField<api::IScreenMT>::Read` | src/uenux2/src/api/gui/iinputfield.u17.h | uenux2/src/api/gui/iinputfield.h (caminho inferido) | alta |
| 11041 | 49 |  | `api::IForm<api::IScreenMT>::GetRenderForm` | `api::IForm<api::IScreenMT>::GetRenderForm` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | alta |
| 11042 | 118 |  | `api::IForm<api::IScreenMT>::vf4` | `api::IForm<api::IScreenMT>::Redraw` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | média |
| 11043 | 7 |  | `api::IForm<api::IScreenMT>::vf3` | `api::IForm<api::IScreenMT>::ShowOnTop` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | média |
| 11044 | 7 |  | `api::IForm<api::IScreenMT>::vf2` | `api::IForm<api::IScreenMT>::Show` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | média |
| 11045 | 10 |  | `api::IForm<api::IScreenMT>::vf1` | `api::IForm<api::IScreenMT>::~IForm [deleting]` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | alta |
| 11132 | 49 | ✓ | `api::IForm<api::IScreen>::GetRenderForm` | `api::IForm<api::IScreen>::GetRenderForm` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | alta |
| 11133 | 118 | ✓ | `api::IForm<api::IScreen>::vf4` | `api::IForm<api::IScreen>::Redraw` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | média |
| 11134 | 7 |  | `api::IForm<api::IScreen>::vf3` | `api::IForm<api::IScreen>::ShowOnTop` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | média |
| 11135 | 7 | ✓ | `api::IForm<api::IScreen>::vf2` | `api::IForm<api::IScreen>::Show` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | média |
| 11136 | 10 |  | `api::IForm<api::IScreen>::vf1` | `api::IForm<api::IScreen>::~IForm [deleting]` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | alta |
