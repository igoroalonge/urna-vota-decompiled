# u17: the form stack (`IForm`), keyboard input, the file-hash tree and the mesário terminal states

Unit u17 is a grab-bag of **106 wasm functions** that the tools attributed to eleven original files:
`uenux2/src/api/gui/{ctextsource.h, iform.h, iresource.h, iscreen.h, primitives.cpp, reports/creport.cpp}`,
`uenux2/src/api/hash/{chasharquivo.cpp, cmontadorhash.cpp}` and
`uenux2/src/api/hwil/{iimpressora.h, iimpressorarelatorios.cpp, iinput.h}`.
Only about half of the functions really belong to those files. The rest were put here because a small
inline function of one of those headers is compiled into them:

| inline function (srcloc) | inlined into | what the host function really is |
|---|---|---|
| `CTextSource::CTextSource` (ctextsource.h:37) | 652, 1150 | `vota::CPedeIdentidade::GetInst`, `vota::CMostraEleitorVotando::GetInst` |
| `CInteractiveForm<IScreenMT,IInputMT>::Read` (cinteractiveform.h:57, which inlines `IInput::GetKey`, iinput.h:86) | 10415, 10430, 10439, 10443, 10590, 10626, 10721, 10731 | `ProcessInput` of eight poll-worker (*mesário*) terminal states |
| `IInput::GetKey` (iinput.h:86) | 6319, 11024 | `IInputField<IScreen/IScreenMT>::Read` (the key-to-text state machine of every input field) |
| `getResourceMovie` (iresource.h:87) | 1593, 3076 | two voter-screen helpers of `ctelasvota.cpp` that add the GIF animations |
| `SRect::Top/Bottom` (primitives.cpp:26/38) | 5537 | `CFramedText::DesenhaCaracter` (draws one digit box) |
| `CScopedReportFile` ctor/dtor (creport.cpp:31/37) | 3654 | "render a report into a file" |
| `CalculaHash`, `CalculaHashGeral`, `CalculaHashArquivo`, `CHashDiretorio::ValidaObjeto` (cmontadorhash.cpp:49/78/91/98, chashdiretorio.cpp:41) | 5360 | `CMontadorHash::CriaHashesDiretorio` (the tools picked the wrong one of the six srclocs) |
| (by callers only) | 3632, 5341, 5393, 5396, 5401, 5420, 5421, 2880, 10209, 10214, 10722, 10627, 10440, 10762 | more mesário states, `CInicioVotacao::GetInst`, form-builder helpers |

15 of the 106 functions ran during the recorded votes (`analysis/runtime/*.functions.tsv`): the voter
form stack (`IForm<IScreen>` Show/Redraw/OnActivate/GetRenderForm and the stack notification), the
input-field key handling (6319), the digit boxes (5488, 5537), the vote-type and cargo GIF animations
(1593, 3076, 5543, 6696) and `CThreadVota::CriaTick` (807).

**What it does in the voting process.** Everything the urna shows goes through `IForm`: the voter
screens (*tela do eleitor*), the poll worker's microterminal (*terminal do mesário*, 4×40 LCD) and the
printed/recorded reports. Every key the voter or the mesário presses ends in `IInputField::Read`. At the
end of the day (*encerramento*) the mesário closes the vote on the microterminal
(`CPedeTituloEncerramento` → `CConfirmaEncerramento`, §7.5), and among the result files written to the
media is `hash.dat`, the list of hashes of every file of the urna, built by `CMontadorHash` (§6).

Reconstructed sources:

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

Glossary used below: *mesário* poll worker; *terminal do mesário* / MT microterminal; *eleitor* voter;
*título (de eleitor)* voter registration number (12 digits); *seção* polling section; *habilitação*
releasing a voter to vote; *justificativa* formal justification of absence by a voter of another
section; *encerramento* closing of the vote; *zerésima* the "zero report" printed before voting;
*treinamento de eleitores* voter-training mode; MI/ME = *mídia interna/externa* (internal/external
flash); MV *mídia de votação*.

---

## 1. Classes and how they relate (RTTI)

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

## 2. `IForm<MEDIA>`: forms and the per-device form stack

### 2.1 Layout and virtual protocol

`IForm` (72 bytes): `+4` active flag, `+8` `vector<shared_ptr<IFormField<MEDIA>>>` fields, `+20`
`shared_ptr<IPreShow<MEDIA>>` pre-show hook (e.g. `CPreShowClearMT` clears the LCD), `+28` `std::mutex`
(24 bytes, zero-initialised, so not recursive), `+52` `shared_ptr<FormControlBlock>`, `+60` name.
The constructor (shared body `vota_f6024`, not in this unit) calls `SetForm(this)` on every field, so a
field's back-pointer (`IFormFieldBase +8`) is the `IForm*` itself. (u15's `gui-common.u15.h` calls that
pointer "FormControlBlock*"; the layout it describes, `+4` active and `+28` mutex, is the `IForm` one.)

| slot | method | IScreen | IScreenMT | IPaper |
|---|---|---|---|---|
| 0/1 | `~IForm` / deleting | 2781 / 11136 | 2776 / 11045 | 5514 / 5513 |
| 2 | `Show()`: the form replaces the stack | 11135 → 5541 | 11044 → 5520 | 11009 (inlined) |
| 3 | `ShowOnTop()`: the form goes to the top | 11134 → 5540 | 11043 → 5519 | 11008 (inlined) |
| 4 | `Redraw()`: redraw dirty fields + device refresh | 11133 | 11042 | 11007 |
| 5 | `GetRenderForm()` = `CPolySingletonList::instance<MEDIA>()` (iform.h:178) | 11132 | 11041 | 11004 |
| 6 | `SetFocus(field)` (no-op, ICF 425; CInteractiveForm overrides) | 425 | 425 | 425 |
| 7 | `OnActivate()`: draw everything, start timers | 5539 | 5518 | 11005 |

The device refresh is `IScreen` slot 27 (`CWasmScreen::vf27` logs `Refresh()` and posts
`{"refreshed":true}` on the `vota:screen` channel of the web page) and `IScreenMT` slot 14. `IPaper` has
none: the code calls `GetRenderForm()` a second time and discards the result.

### 2.2 The stack (state machine)

Each MEDIA has a static `std::vector<IForm*>` (IScreen @1832632, IScreenMT @1832892, IPaper @1833528)
guarded by a static mutex. The top of the stack is the only *active* form.

* **Show** (`DoShow`): deactivate the top (active := false, every field `Stop()`), clear the stack, push
  the form, `OnActivate()` it, publish.
* **ShowOnTop** (`DoShowOnTop`): if the form is already on top, nothing (not even a publication);
  otherwise deactivate the top, remove the form from wherever it is, push it, `OnActivate()`, publish.
* **Remove** (from `~IForm`; out of line for IScreen, func 5987, also called by
  `vota::CEmitirMaisBU::ProcessInput`): if the form is on the stack: deactivate it if it was on top,
  erase it, then *redraw every remaining form from the bottom up* (pre-show + `Draw` of each field),
  re-activating the new top; publish.
* **RemoveAll** (`~IScreen` func 5071 → 5069, `~IScreenMT` func 8741 → 3461): pop every form,
  deactivating each; publish.
* **OnActivate**: active := true; under the form mutex: pre-show, `Draw` every field, refresh; then
  `Start()` every field (blinking cursors, clocks, animations: `ITimerScheduler` timers).
* **Redraw** (called by a field that changed, through its inlined `Invalidate()`): under the form mutex,
  if active: `RedrawIfDirty` on every field (field slot 5), refresh.

**Publication.** After every change the stack is copied as `{IForm*, shared_ptr<FormControlBlock>}`
pairs (`GetFormStack`, merged body 3941) and assigned to a static observable (IScreen @1529260,
IScreenMT @1529940, IPaper @1581136: `{vector value; vector<IObserver*>; mutex}`), whose observers are
notified (slot 2). The first publication is done once with `std::call_once` (lambdas 8902 / 8716 /
11002). **Nothing in the binary registers an observer**: the observer vectors (@1529272, @1529952,
@1581148) are only read, by the notification loops. The `FormControlBlock` (`bool m_vivo` +
`std::shared_mutex`) exists so that an observer holding a raw `IForm*` could check that the form is
still alive; the destructor takes the exclusive lock and sets `m_vivo = false` (funcs 3328/3327 are
libc++'s `__shared_mutex_base::lock/unlock`). This looks like an inspection/testing hook with no user
in this build.

### 2.3 `FindFieldAs<T>(nome)` (func 1721, iform.h:142)

Linear search of the fields by their unique name (given by `CFormBuilder::Add`). A match of the wrong
type throws `CUeGuiError` 4978 "Campo encontrado, mas não é do tipo solicitado: " + nome; no match
returns `nullptr`. The only instance is `<IScreen, CTextField>`, used by
`vota::CMostraQRCodeBU::AjustaTela` (the BU QR-code screen) to update a text field in place.

## 3. Keyboard input

### 3.1 `IInput::GetKey` (iinput.h:86, always inlined) and `KeyName` (func 744, iinput.h:61)

`GetKey()` throws `CBaseError<EUeHwilError>` 5170 "IInput - Nao havia um caractere disponivel" if no key
is buffered (slot 3), else reads one (slot 2) and increments a counter (`+4`). Keys are characters:
`'0'..'9'`, `'B'` BRANCO (blank), `'C'` CONFIRMA, `'D'` CORRIGE (correct). `KeyName` maps them to
"0".."9", "BRANCO", "CONFIRMA", "CORRIGE", and throws 5169 "Tecla desconhecida ({})" otherwise. It is
used for the key labels on the screens and for the audio (*voto em áudio*) announcements
(`CVotacaoStateAudio::PlayKey`).

### 3.2 `IInputField<MEDIA>::Read(IInput&)` (6319 voter screen, 11024 microterminal)

The key-to-text automaton of every input field, one call per batch of buffered keys:

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

The default `IInputValidation::IsValid` (ICF 4022, shared by `CNumberValidation`, `COptionValidation`
and `CControlValidation`) returns **true for an empty text**, so CONFIRMA on an empty field returns 9
whenever `+52` is set. Every caller checked in this unit tests the text itself (10626: exactly one
character; 10721: not empty; 10590: at least 4; 10443: more than 3), and a headless run (`--keys "  C  "`,
municipal-t1) shows that CONFIRMA on the empty Vereador field leaves the voter in `CPedeProporcional`. The
microterminal fields are built with `+52 m_confirmaFinaliza = 1` and a `CNumberValidation("0123456789")`
(func 1151).

## 4. Resources, animations, text sources and drawing helpers

* `IResource` (poly-singleton; web: `simulador::CWasmResource`, reading MEMFS). `getResourceFile`,
  `getResourceMovie`, `isResource` are one-line inlines (iresource.h:82/87/97).
* **Vote-type animation** (1593, ran): tipo 1 → `:/resource/gifs/votoLegenda.gif`, 3 → `votoBranco.gif`,
  4 → `votoNulo.gif`, anything else adds nothing. The `CMovie` is cached in a static
  `map<pair<string,bool>, shared_ptr<CMovie>>` (@1833352). With the flag set (confirmation screens) a
  one-frame movie is built from the current frame (frozen image); the current frame is fetched with
  `vector::at` (and copied) before the flag is tested, so in both cases. Placed at the packed `SPoint`
  31457920 = 0x01E00280, that is **(640, 480)**, with anchor 7 = bottom-right (5543 stores `e[8] = pos`,
  `e[9] = 7`): the bottom-right corner of the screen, as in the observed `js_image(…, 865, 400, 415, 400)`
  (1280 − 415 = 865, 800 − 400 = 400; these are the Libras interpreter clips, 09 §3.4).
* **Cargo animation** (3076, ran): static map cargo code → GIF: 1 presidente, 3 governador, 5 senador,
  6 depFederal, 7 depEstadual, 8 depDistrital, 25 conselheiroDistrital, 11 prefeito, 13 vereador; not
  cached (the GIF is loaded again at every call). Also at (640, 480), anchor 7 (bottom-right).
* `CTextSource` (ctextsource.h): a `shared_ptr<std::string>` the owner changes and the display field
  re-reads; null → `CUeGuiError` 4977 "Texto nulo".
* `SRect::Left/Top/Right/Bottom` (primitives.cpp:20/26/32/38): UE_ASSERTs 3400–3403 "Assert (right >= l)",
  "Assert (bottom >= t)", "Assert (r >= left)", "Assert (b >= top)".
* `CFramedText::DesenhaCaracter` (5537, ran): shrinks the digit box by the inter-box spacing
  (`size <= 7 ? 1 : size/8`) and a vertical margin (`size <= 9 ? 1 : size/10`) and writes the character
  centred (`IScreen::WriteText`, slot 19, colours 2 on 1).
* `IScreen::GetFontMetrics` default (8938, iscreen.h:69): throws 4982 "Método não implementado"
  (CWasmScreen overrides it).

## 5. Reports and printer

* `api::GeraRelatorioEmArquivo(nome, relatorio)` (3654, name inferred): `CScopedReportFile` opens the
  report "file" on `IPaperRelatorios` (slot 5, creport.cpp:31), `CReport::Imprime` (5486) prints every
  part (part slot 2), the destructor closes it (slot 6, creport.cpp:37). Used for `ze.dat` (zerésima),
  its summary and the end-of-day reports (`CGeraRelatorios::StartState`). The same `Imprime` is the body
  of the printer job lambda `api::(anonymous)::DoPrintJob::$_0` (10884).
* `IImpressora::EnableSensors/DisableSensors` (8341/8360, iimpressora.h:116/103): default
  implementation throws `CUePrinterError` 4467/4466 "Not supported ({})" with the sensor mask. Both are
  constant thunks into one body, func 6133: Binaryen's *merge-similar-functions* (§9), not a source
  helper. The web `CWasmNullPrinter` does not override them (vtable slots 6/7 = 8360/8341).
* `IImpressoraRelatorios::SetStyle` (10881, iimpressorarelatorios.cpp:22): style 0 throws
  `CUeCodedPrinterError`(4468, "Invalid style", "no style", code 2); otherwise calls the protected
  `DoSetStyle` (slot 12) only when the style changes. `CWasmNullPrinter::GetColumns` (slot 5) returns
  19 columns for style 3 (double width) and 38 otherwise.

## 6. `CMontadorHash`: the file-hash tree of `hash.dat` (encerramento)

### 6.1 Where it sits

At the end of the day the voter thread writes the result files (`vota::CGravaResultado`, unit u09).
One of them is `…-hash.dat`: `comum::CGravadorHashes::GravaResultado` (11620) →
`comum_f5359` → **`CMontadorHash::CriaHashesDiretorio` (5360)** → `comum_f5358` flattens the tree into
`vector<CHashArquivo>` with full paths (`directory + file name`) → ASN.1
`ModuloHashes::EntidadeHashes { cabecalho, fase, siglaUF, identificacaoUrna OPTIONAL, versaoUENUX,
hashesArquivos SEQUENCE OF ArquivoAssinatura { nomeArquivo GeneralString, assinatura GeneralString } }`
(`src/asn1/ModuloHashes.asn`). `CConversorHashArquivo` (10267/10269) converts between
`ArquivoAssinatura` and `CHashArquivo`; the hash text goes in the field named `assinatura`.
`CGravadorHashes` starts from the urna root (`comum::CPath` root @1838600) and passes an exclusion set
built from `<root>dev/`, `<root>dsk/`, `<root>proc/`, `<root>sys/`, `/tmp/`, three `…dinamico/` paths
and `tmp/` (details belong to the owner of cgravadorhashes.cpp).

### 6.2 Algorithm (func 5360, recursive; cmontadorhash.cpp:121)

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

Hash and text encoding are poly-singleton factories: `IGenericFactory<ecourna::api::security::IHash>`
(the only `IHash` in the binary is `CSha512`, OpenSSL "SHA2-512") and
`IGenericFactory<ITextEncoding>` (Base64). The chaining is exactly the "HASH GERAL" rule that
reproduces the TSE published lists (docs/libraries/openssl.md §10). Two corrections to that chapter:
`uenux.cfg` and `avbootcfg.vsu` are **not excluded** from the tree (they get their own entry in
`hash.dat`); they are only kept out of the overall hash. And entries named `tmp` are skipped at every
level, like `lost+found`.

**The overall hash is write-only in this build.** It is accumulated in a class-static `std::string`
(@1910016, destroyed at exit by 10259) that no other function reads, and it is never reset, so a second
call would chain on the first one's value.

**Web build.** Neither factory is registered: the first file makes `CPolySingletonList::instance`
throw "PolySingleton - solicitada uma instancia nao criada N3api15IGenericFactoryIN7ecourna3api8security5IHashEEE"
(cpolysingletonlist.h). The patched run of `docs/bu/codepath.md` §8.4 stopped exactly there, leaving
`hash.dat` empty. In the unpatched simulator the path is never reached (no encerramento without the
operator thread).

## 7. The mesário (microterminal) states of this unit

All of them run in the operator thread, which the web build never runs (u10 §2). They are the real
urna's microterminal logic. Coordinates are `{coluna, linha}` on the 4×40 LCD.

### 7.1 Idle and identification

* **`CPedeIdentidade`** (constructor in 652): the idle screen. Normal mode:
  `Digite o Título ou o CPF` (one phrase per identity type the election accepts, joined with " ou "),
  a 12-digit input at `{1,2}`, votes so far (`GetTextoQtdVotaram`) + `"/{:04}"` of the section's
  *aptos* (voters able to vote, counted once at construction), the audio indicator, and a status line at
  `{1,4}` refreshed every 300 ms from a `CTextSource` (`CTextFieldUpdateMT`). Voter-training mode:
  "TREINAMENTO DE ELEITORES", "Votos:", "CORRIGE: outras opções", "CONFIRMA: votar". Ticks of 60 s, 5 s
  and 1 s; the constructor also draws the time of the first booth inspection
  (`SorteiaProximaInspecao`).
* **`CValidaIdentidade`** (10627/10626): the typed number is checked by every
  `CValidadorIdentidade` validator. 0 matches → `CIdentidadeInvalida` ("Identidade: <n> / Número
  errado", beep); 1 → log "Identificador digitado pelo mesário foi: (<tipo>)", set the type,
  `CProcuraEleitor`; several → menu "Tipo de identidade digitada / 1-Título de eleitor / 2-CPF /
  3-Identificador … ESCOLHA O TIPO:" with a 1-digit input (`aguardaConfirma` false: typing the digit
  is enough). A valid choice logs the same text and goes to `CProcuraEleitor`. A choice that is not
  exactly one character (CONFIRMA on the empty field) returns no state; a digit that is not a listed
  option also logs "Identificador inválido para o tipo informado". In both cases ProcessInput stores
  **only a non-null result**, so the menu stays (`m_proximoEstado` is still `this`). CORRIGE logs
  "Habilitação cancelada durante confirmação de dado do eleitor" and goes back to `CPedeIdentidade`.
* **`CNomeEleitor`** (5401): name, identity, "Seq:", "Seção:", TTE text, "CORRIGE: cancelar /
  CONFIRMA: prosseguir"; form name `telaNomeEleitor`, keyboard not flushed.
* **`CVerificaDadoEleitor`** (10443): the birth year typed after the fingerprint failed. Fewer than 4
  digits: "Dado digitado está incompleto" (stay). Equal to the roll's year (`std::stoul` vs the first 4
  chars of `dataNascimento`): "Dado digitado confere" → **`CRegistraDigitalOperador`** (5396: the
  mesário places his own finger, "MESÁRIO: / Posicione seu dedo POLEGAR ou INDICADOR / sobre o sensor",
  a counter initialised to 3 (attempts, inferred), 15 s timeout tick, 50 ms polling tick). Different: "Dado digitado não confere" →
  `CDadoEleitorNaoConfere` (two screens: "tentar novamente" and "procurar o cartorio eleitoral").

### 7.2 While the voter is at the urna

* **`CMostraEleitorVotando`** (1150): see u10 §4.3. Line 3 is the voter-side audio indicator
  (`"ÁUDIO ATIVADO"` or blank, func 10539), not the TTE.
* **`CEleitorDemorando`** (10440/10439): inactivity reported by the voter thread. In voter-training mode
  it switches directly to `CSuspensaoAutomaticaEleitor` (u10). Otherwise "Votou parcialmente" /
  "Não votou", log "Eleitor sem atividade por {N} segundos" (N = the configured timeout, byte +168 of
  `CConfiguracaoEleicao`), the cargo text on line 3, and CONFIRMA → **`CPerguntaEleitorVotando`**
  ("O eleitor ainda está votando?"): CONFIRMA = yes → log "Eleitor está votando", message 4
  (*continua*) to the voter thread, back to `CMostraEleitorVotando`; CORRIGE = no → log "Eleitor não
  está votando" → `CPerguntaCodigoSuspensao` ("Informe seu título para / suspender a votação", 12-digit
  input; methods in another unit).
* **`CSincronismoOperador::ProcessMessage`** (10209): message 1 (vote finished) → if the voter used
  audio, `CDesabilitaAudioEleitor` ("Retire o fone de ouvido da urna"); else the next voter
  (`CPedeIdentidade`); after an automatic suspension, `CEleitorVotouNaoVotou` (normal mode) or
  `CPedeIdentidade` (training). Messages 14/15 show a failure screen (5341): 14 → "FALHA NA MI:
  SUBSTITUA A URNA / ELEITOR DEVE VOTAR NOVAMENTE", 15 → "FALHA NA ME: SUBSTITUA A ME / ÚLTIMO VOTO
  FOI COMPUTADO".

### 7.3 Justification of absence (`CPedeAnoNascimento`, 10590)

After the título, the mesário types the birth year (at least 4 digits, else nothing happens; the text
is kept in `CThreadOperador +84`). The "election year" is the ushort at `CConfiguracaoEleicao +48` (year
of the pleito's `CDate`, +44). `< 1900` or not before the election year →
"Ano de nascimento inválido" (`CAnoInformadoInvalido`); election year − birth year `<= 15` → "Eleitor não
tem idade mínima" (`CEleitorMenor16Anos`: "Eleitor não pode votar ou justificar / por não ter idade
mínima"). Otherwise:

1. `CJustificador::Justifica(título, ano)` (cjustificador.cpp:55): an existing entry throws 8800 "Já
   havia justificativa para o título"; else it is added to the in-memory map. The título comes from
   `IInformacaoThreadOperador::GetIdentidadeEleitor()` (slot 25) through a **dangling reference**: the
   returned `CEleitorIdentidade` temporary is destroyed before `CNumeroInscricaoEleitoral` reads it
   (§12).
2. `EstadoGeralVota.qtdJustificativas++` (short at +54 of the state).
   If the urna is shutting down (byte @1832936) → `CUeDesligandoError` here, **before** any MI/MV write
   (the justification is already in the in-memory map and counted).
3. Under `CApplicationContextGuard(2, "Gravando a justificativa na MI", "Ocorreu um erro durante a
   sincronização da justificativa na MI.")`: `vota.bin` saved on the MI, `SaveCurrentInternal()`
   (cjustificador.cpp:83; 8801 "Não há dados a serem salvos na MI" if nothing is current) inserts the
   `CRegistroIdentificacaoEleitor` + year into the `registro_justificativa` table of `uenux.db`
   unless present, `vota.bin` is signed (CAssinador 122/123 by turno, file id 31), fsync.
4. `GravaBancoDadosNaMI()` (4657: signs `uenux.db`, copies it to the external flash).
5. The same on the MV: "Gravando a justificativa na MV", `vota.bin` saved externally, 4687 copies the
   `vota.vsu` signature package MI → MV (CArquivosSavd 122 → 124 in the 1st turno, 123 → 125 otherwise;
   the MV copy of `vota.bin` is not re-signed), fsync.
6. Then `CJustificativaEfetuada` (flag +28 = new justification).

Log texts: "Mesário cancelou entrada dos dados" (CORRIGE).

### 7.4 Start of voting (voter thread): `CInicioVotacao::GetInst` (2880)

The microterminal shows "Aguarde o horário de início da votação / Votação a partir das 8h (or
8h30min) / Hora atual: hh:mm:ss"; the start time is a `CDateTime` copied from
`CConfiguracaoEleicao +556`; a 1 s tick of the voter thread checks it (StartState/ProcessTick 11988/11987,
other unit). The voter screen uses a form of `CTelasVota` (+116).

### 7.5 The mesário's encerramento flow (closing the vote)

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

Note that in normal mode the título is only checked for **format** (digits + TÍTULO check digits); the
code of this unit does not compare it with a mesário list.

## 8. Web-build specifics

* The whole operator-thread part (§7 except 7.4) is dead in the simulator (CThreadOperador::Run never
  runs); its accessors are never called (u10 probe: `CPedeIdentidade`'s singleton stays null).
* The voter part is live: `IForm<IScreen>` stack operations, `IInputField<IScreen>::Read`, the digit
  boxes and both GIF helpers ran. `CWasmScreen::Refresh` forwards `{"refreshed":true}` to the page.
* `CWasmNullPrinter` keeps the default `EnableSensors/DisableSensors` (they would throw "Not supported").
* `hash.dat` cannot be produced (no hash factory, §6).

## 9. wasm / Emscripten observations

* **Misleading srcloc names.** A function that contains an inlined `std::source_location::current()`
  of a header gets that header's name: two state accessors are named `CTextSource::CTextSource`, the
  two GIF helpers `getResourceMovie`, the input automaton `IInput::GetKey`, a CFramedText method
  `SRect::Top`, CriaHashesDiretorio `CalculaHashGeral`.
* **Merged bodies.** `GetFormStack` is one body (3941) for the three MEDIA, called through thunks that
  pass the static addresses; the vector helpers 1872/8872/8879 serve the three `SFormInfo<MEDIA>`
  types; 1300 copies both `vector<CHashArquivo>` and `vector<pair<string,string>>`; the vota singleton
  accessor without members goes through `vota_f764(mutex, &inst, vtable, flags)`.
* **merge-similar-functions.** Func 6133 is the merged body of `IImpressora::DisableSensors` and
  `EnableSensors` (docs/02-wasm-binary-anatomy.md §5.4): 8360/8341 pass `(this, sensores)` and then
  their own constants `(srcloc, código)` = `(iimpressora.h:103, 4466)` / `(iimpressora.h:116, 4467)`.
* **Vtable thunks vs bodies.** For IScreen/IScreenMT, slots 2/3 are 7-byte thunks to static stack
  functions (5541/5540, 5520/5519); for IPaper the same bodies are inlined into the slots.
* **Lock residue.** `std::mutex::lock()` vanishes; `unlock()` is the ICF stub 150; the shared_mutex of
  FormControlBlock remains as real libc++ code (3328/3327) with `condition_variable::wait` calls that can
  never block in this single-threaded build.
* **Inlined libc++ filesystem.** `read_symlink` and `weakly_canonical` are inlined into 5360; their
  out-of-line pieces (`ErrorHandler::report` 3335, `lexically_normal` 7741, `path` builders 4676/4686,
  `vector<string_view>` helpers) are attributed to cmontadorhash.cpp by the tools.
* **Coordinates as one int32.** `SPoint{x, y}` is stored as `x | y << 16` with 2-byte alignment; the
  decompiler shows `p[N]:int@2 = 65569` (= `{33, 1}`) and inline-string comments that are just
  coincidences of the constant (e.g. `65569 /* "dadeVersaoArquivos" */`).
* **Latin-1 strings.** All literals are ISO-8859-1 (e.g. "CORRIGE: outras opções" is 22 bytes).
* The printer interface puts its destructor at slots 10/11, after the pure virtual methods.

## 10. For the BU team: what this unit contributes to the end of the day

This unit does not build the BU itself. It holds four pieces of the encerramento path:

1. **Closing the vote on the microterminal** (§7.5). Normal mode: the mesário types a título; it is only
   checked to be digits and a valid TÍTULO number once left-padded with zeros to 12 digits
   (`CValidadorIdentidade::Valida(1, "{:0>{}}")`); it is kept in `CThreadOperador +120` and logged as
   "Título digitado para encerramento: <título>". Voter-training mode: no título; before the scheduled
   closing time the MT asks "Encerramento de votação antecipado?". Then `CConfirmaEncerramento`
   ("Encerramento da Votação", CONFIRMA: encerrar) hands over to the voter thread, and the MT shows
   "Procedimentos de Encerramento / Siga as instruções na tela do eleitor" (`CFinalizaOperador`), after
   which the operator state machine has no state.
2. **Report files** (§5): `ze.dat` (zerésima) and the end-of-day reports are rendered by
   `GeraRelatorioEmArquivo` = `CScopedReportFile` (IPaperRelatorios slot 5 "open file" / slot 6
   "close") around `CReport::Imprime`.
3. **`hash.dat`** (§6): `EntidadeHashes.hashesArquivos` = one `{nomeArquivo (full path), assinatura =
   Base64(SHA-512(file))}` per regular file under the urna root minus the excluded trees, in directory
   order: files of a directory sorted by path, then its sub-directories (depth first). `uenux.cfg` and
   `avbootcfg.vsu` are listed but kept out of the in-memory overall hash, which is not written anywhere
   by this build. Web build: fails at the first file (no hash factory).
4. **BU QR screen**: `IForm<IScreen>::FindFieldAs<CTextField>` (1721) is how
   `vota::CMostraQRCodeBU::AjustaTela` finds the text field it updates on the BU QR-code screen.

## 11. Open questions

* The real name/role of `GeraRelatorioEmArquivo` (3654): it takes the file name by value and the
  `CReport` second, so it is not a `CReport` member; unit u09 writes the call as `relatorio.Gera(path)`.
* Ticks `+12` (5 s) and `+13` (1 s) of `CPedeIdentidade` (ProcessTick 10676 is in another unit).
* The meaning of `CLedFieldMT` state 2 (used only by the recording-failure screen).
* Whether any build registers an observer on the form-stack observables (a test or inspection tool).
* `CInicioVotacao +12` takes `CTelasVota +116`, a form member not yet named in `ctelasvota.h`.
* `IInput +4` is incremented by every `GetKey` (key counter?); nothing in this unit reads it.

## 12. Suspicious / weird code

* **(Rejected on review) "the identity-type menu can leave the operator state machine without a
  state".** `CValidaIdentidade::ProcessInput` (10626) stores its choice parser's result only when it is
  non-null (`local.tee; i32.eqz; br_if` jumps over the `i32.store offset=4`), exactly like
  `CPedeTituloEncerramento`. CONFIRMA on the empty "ESCOLHA O TIPO:" field (which `IInputField::Read`
  does report as CONFIRMA, §3.2) or an unknown digit leaves `m_proximoEstado = this`: the menu stays.
* **Dangling reference in the justification (real urna code, dead in the simulator).**
  `CPedeAnoNascimento::ProcessInput` (10590) calls `IInformacaoThreadOperador` slot 25
  (`GetIdentidadeEleitor`, func 1535), which returns a `CEleitorIdentidade` temporary (16 bytes,
  `std::string` at +0). The binary frees that string immediately (`i32.load8_s offset=11 … call $free`)
  and then passes the same object to `CNumeroInscricaoEleitoral(const std::string&)` (func 1241): the
  source binds a reference to a member of the temporary (`const std::string& t =
  GetIdentidadeEleitor().GetIdentidade();`). With libc++ a 12-digit título does not fit the 10-char
  small-string buffer, so the constructor copies **freed heap memory**; what it reads depends on the
  allocator (dlmalloc may already have written free-list pointers into the first 8 bytes). With a
  standard library whose small-string buffer holds 12 characters the digits stay inside the dead stack
  object and the bug would go unnoticed. `IIniciaJustificativa::StartState` (10587) copies the string
  first and is correct. Not observable here: the operator thread never runs in the web build.
* **Menu overlaps the key legend.** In the same menu option *n* is drawn on line *n*+1 and the key
  legend ("CORRIGE: retornar" at {1,4}, "ESCOLHA O TIPO:" at {24,4}, the input at {40,4}) is on line 4:
  a third option ("3-Identificador", 15 characters) would be at {1,4}, the same point as "CORRIGE:
  retornar" (17 characters), which is added later and so covers it completely. Whether any number can
  validate as all three identity types was not established.
* **`hash.dat` cannot be produced in the web build**: `IGenericFactory<IHash>` and
  `IGenericFactory<ITextEncoding>` are not registered, so `CriaHashesDiretorio` throws at the first
  file (confirmed by the patched run in docs/bu/codepath.md §8.4).
* **Overall hash is dead state.** `CMontadorHash`'s static accumulator (@1910016) is written for every
  file but never read and never reset; a second `CriaHashesDiretorio` would chain onto the previous
  value.
* **Fragile tree walk.** Every entry that is not a regular file (FIFO, device, socket, *dangling*
  symlink, symlink loop) is treated as a directory and recursed; `opendir` then fails and the whole
  `hash.dat` generation throws 5103. Symlinks to directories are followed, so a link to an ancestor that
  is not in the exclusion set recurses until the kernel's symlink limit makes `opendir` fail (same
  exception). Directory names must already end with '/' (paths are concatenated without separator).
* **Operator dead code in the simulator.** 22 of the functions (all `vota::` operator states) never run
  in the web build; their behaviour cannot be observed with `tools/run/headless.mjs`.
* **Default printer methods throw.** `CWasmNullPrinter` does not override `EnableSensors/DisableSensors`;
  any code that calls them on the web build gets `CUePrinterError` "Not supported (n)".
* **Form-stack observable with no observers** (§2.2): the notification code, the `call_once`
  initialisation and the `FormControlBlock` shared lock run on every screen change for nothing in
  this build (harmless).
* **Minimum-age check by year only**: `CPedeAnoNascimento` rejects when `anoEleição − anoNascimento <= 15`,
  without looking at the day or month (a voter who turns 16 later in the election year passes).
* No `emscripten_sleep`, network or privacy-relevant code in this unit.

## 13. Complete mapping table (106 functions)

Columns: wasm index, size in bytes, ✓ = observed executing in the recorded votes, name given by the
tools, reconstructed symbol, where it is reconstructed (or why not), original file, confidence.

| func | size | ran | tools name | reconstructed symbol | reconstructed in (src/…) | original file | conf. |
|---:|---:|:-:|---|---|---|---|---|
| 180 | 258 |  | `api_f180` | `api::CFormBuilderMT::Add<api::CTextFieldMT, api::CFixedText>` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp (header vocabulary) | uenux2/src/api/gui/cformbuildermt.h (path inferred) | medium |
| 435 | 19 |  | `vota_f435` | `api::CFormBuilderMT::Add<api::CLedFieldMT>` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp (header vocabulary) | uenux2/src/api/gui/cformbuildermt.h (path inferred) | medium |
| 463 | 18 |  | `ecourna_f463` | `ecourna::api::exception::CBaseError<api::EUeAssertError>::CBaseError` | library/inlined helper (thunk of the merged CBaseError ctor 710; used by UE_ASSERT, see primitives.cpp) | ecourna-lib/ecourna/api/exception/cbaseerror.hpp | high |
| 619 | 207 |  | `api_f619` | `api::CFormBuilderMT::Add<api::CTextFieldMT, api::CDataText<std::string (*)()>>` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp (header vocabulary) | uenux2/src/api/gui/cformbuildermt.h (path inferred) | medium |
| 652 | 4609 |  | `api::CTextSource::CTextSource@652` | `vota::CPedeIdentidade::GetInst` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/leidentidade/cpedeidentidade.cpp | high |
| 744 | 625 |  | `api::KeyName` | `api::KeyName` | src/uenux2/src/api/hwil/iinput.h | uenux2/src/api/hwil/iinput.h | high |
| 807 | 12 | ✓ | `vota_f807` | `vota::CThreadVota::CriaTick` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp (header vocabulary) | uenux2/src/app/vota/comum/cthreadvota.cpp (path inferred) | medium |
| 1150 | 1768 |  | `api::CTextSource::CTextSource@1150` | `vota::CMostraEleitorVotando::GetInst` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/aguardaeleitor/cmostraeleitorvotando.cpp | high |
| 1300 | 235 |  | `comum_f1300` | `std::vector<api::hash::CHashArquivo>::vector(const vector&)` | library/inlined helper (copy of a vector of two-std::string elements; ICF-shared with vector<pair<string,string>> of the QR-code code) | libcxx <vector> (template instance) | medium |
| 1444 | 141 |  | `api_f1444` | `std::pair<const uebyte, std::string>::pair<int, const char (&)[N]>(int&&, const char (&)[N])` (the key is read as a 4-byte int and truncated: the initializer list writes `{1, ":/resource/…"}` with int literals; one body for every N) | library/inlined helper (initializer of the cargo -> gif map in 3076) | libcxx <utility> (template instance) | medium |
| 1593 | 1945 | ✓ | `api::getResourceMovie@1593` | `vota::(anonymous namespace)::adicionaAnimacaoVoto` | src/uenux2/src/app/vota/eleitor/u17-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | medium |
| 1721 | 296 |  | `api::IForm<api::IScreen>::FindFieldAs` | `api::IForm<api::IScreen>::FindFieldAs<api::CTextField>` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | high |
| 1872 | 848 | ✓ | `api_f1872` | `std::vector<api::SFormInfo<MEDIA>>::assign` | library/inlined helper (vector::__assign_with_size, 12-byte {IForm*, shared_ptr} elements; one body for the 3 MEDIA) | libcxx <vector> (template instance for iform.h) | medium |
| 2220 | 233 |  | `api_f2220` | `api::hash::CHashDiretorio::~CHashDiretorio` | src/uenux2/src/api/hash/chasharquivo.h | uenux2/src/api/hash/chashdiretorio.h (path from srcloc chashdiretorio.cpp) | medium |
| 2547 | 60 |  | `api_f2547` | `std::vector<std::string_view>::~vector` | library/inlined helper (filesystem weakly_canonical / lexically_normal internals) | libcxx <vector> (template instance) | medium |
| 2631 | 321 |  | `api_f2631` | `api::IForm<api::IScreenMT>::NotifyStackChanged` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | medium |
| 2651 | 321 | ✓ | `api_f2651` | `api::IForm<api::IScreen>::NotifyStackChanged` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | medium |
| 2776 | 761 |  | `api::IForm<api::IScreenMT>::vf0` | `api::IForm<api::IScreenMT>::~IForm` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | high |
| 2781 | 355 |  | `api::IForm<api::IScreen>::vf0` | `api::IForm<api::IScreen>::~IForm` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | high |
| 2880 | 2039 |  | `vota_f2880` | `vota::CInicioVotacao::GetInst` | src/uenux2/src/app/vota/eleitor/u17-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/iniciovotacao/ciniciovotacao.cpp (path inferred) | high |
| 3076 | 1224 | ✓ | `api::getResourceMovie@3076` | `vota::(anonymous namespace)::adicionaAnimacaoCargo` | src/uenux2/src/app/vota/eleitor/u17-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | medium |
| 3327 | 22 |  | `api_f3327` | `std::__shared_mutex_base::unlock` | library/inlined helper (FormControlBlock mutex, iform.h) | libcxx src/shared_mutex.cpp | medium |
| 3328 | 135 |  | `api_f3328` | `std::__shared_mutex_base::lock` | library/inlined helper (FormControlBlock mutex, iform.h) | libcxx src/shared_mutex.cpp | medium |
| 3335 | 356 |  | `api_f3335` | `std::filesystem::detail::ErrorHandler<std::filesystem::path>::report` | library/inlined helper (read_symlink/weakly_canonical inlined into 5360) | libcxx src/filesystem/error.h | medium |
| 3370 | 23 |  | `api_f3370` | `std::filesystem::_PathCVT<char>::__append_source<std::string_view>` (= `__pn_.append(first, last)`, func 178; `string::append(string_view)` would call the (ptr, n) overload 160) | library/inlined helper (called only by 7742 path::append and 7744 path::assign) | libcxx <__filesystem/path.h> (template instance) | medium |
| 3461 | 148 |  | `api_f3461` | `api::IForm<api::IScreenMT>::RemoveAll` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | medium |
| 3600 | 121 |  | `api_f3600` | `api::hash::CHashArquivo::CHashArquivo` | src/uenux2/src/api/hash/chasharquivo.h | uenux2/src/api/hash/chasharquivo.h (path inferred) | high |
| 3632 | 856 |  | `api_f3632` | `vota::CConfirmaEncerramento::GetInst` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/outrasopcoes/cconfirmaencerramento.cpp (path inferred) | high |
| 3654 | 393 |  | `api::CScopedReportFile::CScopedReportFile` | `api::GeraRelatorioEmArquivo` | src/uenux2/src/api/gui/reports/creport.cpp | uenux2/src/api/gui/reports/creport.cpp | low |
| 3670 | 321 |  | `api_f3670` | `api::IForm<api::IPaper>::NotifyStackChanged` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | medium |
| 3941 | 269 | ✓ | `api_f3941` | `api::IForm<MEDIA>::GetFormStack` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | medium |
| 4675 | 9 |  | `api_f4675` | `std::__split_buffer<std::string_view>::~__split_buffer` | library/inlined helper (vector<string_view> growth in weakly_canonical) | libcxx <__split_buffer> (template instance) | medium |
| 4676 | 15 |  | `api_f4676` | `std::filesystem::path::path<char[1]>` | library/inlined helper (path("") for __canonical("") in weakly_canonical) | libcxx <filesystem> (template instance) | low |
| 4686 | 15 |  | `api_f4686` | `std::filesystem::path::path<char*>` | library/inlined helper (path(buffer) in read_symlink) | libcxx <filesystem> (template instance) | low |
| 5022 | 22 |  | `api_f5022` | `api::IForm<api::IScreenMT>::GetFormStack` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | medium |
| 5341 | 1052 |  | `vota_f5341` | `vota::CSincronismoOperador::MostraFalhaGravacao` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/aguardaeleitor/csincronismooperador.cpp (path inferred) | medium |
| 5360 | 9613 |  | `api::hash::CMontadorHash::CalculaHashGeral` | `api::hash::CMontadorHash::CriaHashesDiretorio` | src/uenux2/src/api/hash/cmontadorhash.cpp | uenux2/src/api/hash/cmontadorhash.cpp | high |
| 5363 | 198 |  | `api_f5363` | `std::vector<api::hash::CHashDiretorio>::vector(const vector&)` | library/inlined helper (implicit CHashDiretorio copy ctor inlined) | libcxx <vector> (template instance) | medium |
| 5364 | 144 |  | `api::hash::CHashArquivo::ValidaObjeto` | `api::hash::CHashArquivo::ValidaObjeto` | src/uenux2/src/api/hash/chasharquivo.cpp | uenux2/src/api/hash/chasharquivo.cpp | high |
| 5393 | 652 |  | `vota_f5393` | `vota::CDesabilitaAudioEleitor::GetInst` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/aguardaeleitor/cdesabilitaaudioeleitor.cpp (path inferred) | high |
| 5396 | 1586 |  | `api_f5396` | `vota::CRegistraDigitalOperador::GetInst` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | high |
| 5401 | 1583 |  | `vota_f5401` | `vota::CNomeEleitor::GetInst` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/confirmaidentidade/cnomeeleitor.cpp | high |
| 5420 | 788 |  | `vota_f5420` | `vota::CValidaIdentidade::MontaOpcoes` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/leidentidade/cvalidaidentidade.cpp (path inferred) | medium |
| 5421 | 22 |  | `api_f5421` | `vota::CProcuraEleitor::GetInst` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/leidentidade/cprocuraeleitor.cpp (path inferred) | high |
| 5486 | 55 |  | `api_f5486` | `api::CReport::Imprime` | src/uenux2/src/api/gui/reports/creport.cpp | uenux2/src/api/gui/reports/creport.cpp | medium |
| 5487 | 77 |  | `api::SRect::Right` | `api::SRect::Right` | src/uenux2/src/api/gui/primitives.cpp | uenux2/src/api/gui/primitives.cpp | high |
| 5488 | 77 | ✓ | `api::SRect::Left` | `api::SRect::Left` | src/uenux2/src/api/gui/primitives.cpp | uenux2/src/api/gui/primitives.cpp | high |
| 5512 | 22 |  | `api_f5512` | `api::IForm<api::IPaper>::GetFormStack` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | medium |
| 5513 | 10 |  | `api::IForm<api::IPaper>::vf1` | `api::IForm<api::IPaper>::~IForm [deleting]` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | high |
| 5514 | 761 |  | `api::IForm<api::IPaper>::vf0` | `api::IForm<api::IPaper>::~IForm` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | high |
| 5518 | 195 |  | `api::IForm<api::IScreenMT>::vf7` | `api::IForm<api::IScreenMT>::OnActivate` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | medium |
| 5519 | 289 |  | `api_f5519` | `api::IForm<api::IScreenMT>::DoShowOnTop` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | medium |
| 5520 | 212 |  | `api_f5520` | `api::IForm<api::IScreenMT>::DoShow` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | medium |
| 5537 | 547 | ✓ | `api::SRect::Top` | `api::CFramedText::DesenhaCaracter` | src/uenux2/src/api/gui/cframedtext.u17.cpp | uenux2/src/api/gui/cframedtext.cpp | medium |
| 5539 | 195 | ✓ | `api::IForm<api::IScreen>::vf7` | `api::IForm<api::IScreen>::OnActivate` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | medium |
| 5543 | 333 | ✓ | `api_f5543` | `api::CFormBuilder::Add<api::CMovieField>` | library/inlined helper (form-builder template instance used by 1593/3076) | uenux2/src/api/gui/cformbuilder.h (template instance) | medium |
| 6133 | 415 |  | `api_f6133` | merged body of `api::IImpressora::DisableSensors`/`EnableSensors` (Binaryen merge-similar-functions; no source function) | src/uenux2/src/api/hwil/iimpressora.h (both methods) | uenux2/src/api/hwil/iimpressora.h | medium |
| 6319 | 673 | ✓ | `api::IInput::GetKey@6319` | `api::IInputField<api::IScreen>::Read` | src/uenux2/src/api/gui/iinputfield.u17.h | uenux2/src/api/gui/iinputfield.h (path inferred) | high |
| 6643 | 285 |  | `api_f6643` | `std::map<std::pair<std::string,bool>, std::shared_ptr<api::CMovie>>::__find_equal` | library/inlined helper (animation cache of 1593) | libcxx <map> (template instance) | medium |
| 6696 | 213 | ✓ | `api_f6696` | `std::vector<api::CMovieFrame>::__init_with_size` | library/inlined helper (CMovie copy in 1593/3076) | libcxx <vector> (template instance) | medium |
| 7735 | 172 |  | `api_f7735` | `std::string::reserve` | library/inlined helper (weakly_canonical: result.reserve) | libcxx <string> | medium |
| 7741 | 1079 |  | `api_f7741` | `std::filesystem::path::lexically_normal` | library/inlined helper (end of weakly_canonical, inlined into 5360) | libcxx src/filesystem/path.cpp | medium |
| 7742 | 60 |  | `api_f7742` | `std::filesystem::path::append<std::string_view>` | library/inlined helper (weakly_canonical: result /= part) | libcxx <filesystem> (template instance) | medium |
| 7743 | 353 |  | `api_f7743` | `std::vector<std::string_view>::push_back` | library/inlined helper (weakly_canonical DNEParts) | libcxx <vector> (template instance) | medium |
| 7744 | 16 |  | `api_f7744` | `std::filesystem::path::assign<std::string_view>` | library/inlined helper (weakly_canonical) | libcxx <filesystem> (template instance) | medium |
| 8341 | 17 |  | `api::IImpressora::EnableSensors` | `api::IImpressora::EnableSensors` | src/uenux2/src/api/hwil/iimpressora.h | uenux2/src/api/hwil/iimpressora.h | high |
| 8360 | 17 |  | `api::IImpressora::DisableSensors` | `api::IImpressora::DisableSensors` | src/uenux2/src/api/hwil/iimpressora.h | uenux2/src/api/hwil/iimpressora.h | high |
| 8716 | 193 |  | `api_f8716` | `api::IForm<api::IScreenMT>::NotifyStackChanged::$_0` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | medium |
| 8741 | 17 |  | `api::IScreenMT::vf0` | `api::IScreenMT::~IScreenMT` | src/uenux2/src/api/gui/iscreen.h | uenux2/src/api/gui/iscreen.h | high |
| 8872 | 363 |  | `api_f8872` | `std::vector<api::SFormInfo<MEDIA>>::__emplace_back_slow_path` | library/inlined helper | libcxx <vector> (template instance for iform.h) | medium |
| 8879 | 278 |  | `api_f8879` | `std::vector<api::SFormInfo<MEDIA>>::reserve` | library/inlined helper | libcxx <vector> (template instance for iform.h) | medium |
| 8938 | 50 |  | `api::IScreen::GetFontMetrics` | `api::IScreen::GetFontMetrics` | src/uenux2/src/api/gui/iscreen.h | uenux2/src/api/gui/iscreen.h | high |
| 10209 | 120 |  | `vota::CSincronismoOperador::vf6` | `vota::CSincronismoOperador::ProcessMessage` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/aguardaeleitor/csincronismooperador.cpp (path inferred) | high |
| 10214 | 685 |  | `vota::CFinalizaOperador::vf2` | `vota::CFinalizaOperador::StartState` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/cfinalizaoperador.cpp (path inferred) | high |
| 10257 | 138 |  | `api_f10257` | `api::CUePrinterError::CUePrinterError` | src/uenux2/src/api/hwil/iimpressora.h | uenux2/src/api/hwil/iimpressora.h (path inferred) | medium |
| 10269 | 210 |  | `comum::asn::CConversorHashArquivo::vf3` | `comum::asn::CConversorHashArquivo::DoDesconverte` | src/uenux2/src/app/comum/asn/cconversorhasharquivo.u17.cpp | uenux2/src/app/comum/asn/cconversorhasharquivo.cpp (path inferred) | medium |
| 10415 | 2081 |  | `vota::CPerguntaEleitorVotando::vf7` | `vota::CPerguntaEleitorVotando::ProcessInput` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/aguardaeleitor/cperguntaeleitorvotando.cpp (path inferred) | high |
| 10430 | 150 |  | `vota::CEleitorVotouNaoVotou::vf7` | `vota::CEleitorVotouNaoVotou::ProcessInput` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/aguardaeleitor/celeitorvotounaovotou.cpp (path inferred) | high |
| 10439 | 1039 |  | `vota::CEleitorDemorando::vf7` | `vota::CEleitorDemorando::ProcessInput` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/aguardaeleitor/celeitordemorando.cpp (path inferred) | high |
| 10440 | 1876 |  | `vota::CEleitorDemorando::vf2` | `vota::CEleitorDemorando::StartState` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/aguardaeleitor/celeitordemorando.cpp (path inferred) | high |
| 10443 | 2761 |  | `vota::CVerificaDadoEleitor::vf7` | `vota::CVerificaDadoEleitor::ProcessInput` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/confirmaidentidade/cverificadadoeleitor.cpp (path inferred) | high |
| 10590 | 4305 |  | `vota::CPedeAnoNascimento::vf7` | `vota::CPedeAnoNascimento::ProcessInput` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/justificativa/cpedeanonascimento.cpp (path inferred) | high |
| 10626 | 1004 |  | `vota::CValidaIdentidade::vf7` | `vota::CValidaIdentidade::ProcessInput` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/leidentidade/cvalidaidentidade.cpp (path inferred) | high |
| 10627 | 3842 |  | `vota::CValidaIdentidade::vf2` | `vota::CValidaIdentidade::StartState` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/leidentidade/cvalidaidentidade.cpp (path inferred) | high |
| 10721 | 2309 |  | `vota::CPedeTituloEncerramento::vf7` | `vota::CPedeTituloEncerramento::ProcessInput` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/outrasopcoes/cpedetituloencerramento.cpp (path inferred) | high |
| 10722 | 974 |  | `vota::CPedeTituloEncerramento::vf2` | `vota::CPedeTituloEncerramento::StartState` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/outrasopcoes/cpedetituloencerramento.cpp (path inferred) | high |
| 10731 | 152 |  | `vota::CEncerramentoAntecipado::vf7` | `vota::CEncerramentoAntecipado::ProcessInput` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/outrasopcoes/cencerramentoantecipado.cpp (path inferred) | high |
| 10762 | 18 |  | `vota::CRegistroMesarioEncerrado::vf6` | `vota::CRegistroMesarioEncerrado::ProcessMessage` | src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp | uenux2/src/app/vota/operador/outrasopcoes/cregistromesarioencerrado.cpp (path inferred) | high |
| 10881 | 418 |  | `api::IImpressoraRelatorios::SetStyle` | `api::IImpressoraRelatorios::SetStyle` | src/uenux2/src/api/hwil/iimpressorarelatorios.cpp | uenux2/src/api/hwil/iimpressorarelatorios.cpp | high |
| 11002 | 193 |  | `api_f11002` | `api::IForm<api::IPaper>::NotifyStackChanged::$_0` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | medium |
| 11004 | 2141 |  | `api::IForm<api::IPaper>::vf5` | `api::IForm<api::IPaper>::GetRenderForm` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | high |
| 11005 | 183 |  | `api::IForm<api::IPaper>::vf7` | `api::IForm<api::IPaper>::OnActivate` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | medium |
| 11007 | 106 |  | `api::IForm<api::IPaper>::vf4` | `api::IForm<api::IPaper>::Redraw` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | medium |
| 11008 | 289 |  | `api::IForm<api::IPaper>::vf3` | `api::IForm<api::IPaper>::ShowOnTop` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | medium |
| 11009 | 212 |  | `api::IForm<api::IPaper>::vf2` | `api::IForm<api::IPaper>::Show` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | medium |
| 11024 | 673 |  | `api::IInput::GetKey@11024` | `api::IInputField<api::IScreenMT>::Read` | src/uenux2/src/api/gui/iinputfield.u17.h | uenux2/src/api/gui/iinputfield.h (path inferred) | high |
| 11041 | 49 |  | `api::IForm<api::IScreenMT>::GetRenderForm` | `api::IForm<api::IScreenMT>::GetRenderForm` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | high |
| 11042 | 118 |  | `api::IForm<api::IScreenMT>::vf4` | `api::IForm<api::IScreenMT>::Redraw` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | medium |
| 11043 | 7 |  | `api::IForm<api::IScreenMT>::vf3` | `api::IForm<api::IScreenMT>::ShowOnTop` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | medium |
| 11044 | 7 |  | `api::IForm<api::IScreenMT>::vf2` | `api::IForm<api::IScreenMT>::Show` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | medium |
| 11045 | 10 |  | `api::IForm<api::IScreenMT>::vf1` | `api::IForm<api::IScreenMT>::~IForm [deleting]` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | high |
| 11132 | 49 | ✓ | `api::IForm<api::IScreen>::GetRenderForm` | `api::IForm<api::IScreen>::GetRenderForm` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | high |
| 11133 | 118 | ✓ | `api::IForm<api::IScreen>::vf4` | `api::IForm<api::IScreen>::Redraw` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | medium |
| 11134 | 7 |  | `api::IForm<api::IScreen>::vf3` | `api::IForm<api::IScreen>::ShowOnTop` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | medium |
| 11135 | 7 | ✓ | `api::IForm<api::IScreen>::vf2` | `api::IForm<api::IScreen>::Show` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | medium |
| 11136 | 10 |  | `api::IForm<api::IScreen>::vf1` | `api::IForm<api::IScreen>::~IForm [deleting]` | src/uenux2/src/api/gui/iform.h | uenux2/src/api/gui/iform.h | high |
