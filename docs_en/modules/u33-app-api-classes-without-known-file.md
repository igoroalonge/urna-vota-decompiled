# u33: `app:api` classes without a known file (the simulator's hardware mocks, the TTS base class, `mr.ver`, and stray helpers)

Unit u33 has 72 wasm functions that the tools attributed to the `app:api` component but could not place
in a source file. The tools grouped them as five classes (`api::teste::CUrnaMock`,
`api::teste::CPowerMock`, `api::ITextToSpeech`, `api::Logger`, `comum::CGravadorVersoesArquivos`) and
three groups of "free functions". Once each function is read, the unit turns out to be four different
things:

| group | functions | what they are |
|---|---:|---|
| **Hardware mocks of the web build** | 15 | `api::teste::CUrnaMock` (the urna's identity and its secret tables) and `api::teste::CPowerMock` (power supply / battery) |
| **Text-to-speech and result files** | 11 | `api::ITextToSpeech` slots 1-9 (speech rate, voice parameters, destructor), `api::Logger::log` (RHVoice's error log) and `comum::CGravadorVersoesArquivos::GravaResultado`, the writer of **`mr.ver`**, one of the result files of the encerramento |
| **Stray TSE helpers** | 29 | functions of `api/gui`, `api/pattern`, `api/util`, `api/ipc`, `comum` and `vota` whose callers are spread over several files: form-builder helpers, poly-singleton registry instances, string helpers, lazy singletons of operator (mesário) states, and **merged bodies** that wasm-opt shared between classes |
| **Library code** | 17 | 10 RHVoice 1.14.0 functions (Unicode table lookups, emoji scanner, user-dictionary cursor, `fst::translate` …), 5 libc++ templates instantiated for RHVoice types, and 2 other libc++ instantiations |

11 of the 72 functions ran during the recorded votes (`analysis/runtime/*.functions.tsv`): 1005, 1562
(start-up registrations), 1840, 2199, 2792, 3266, 3684 (start-up), 2779, 3058, 3889 (voter screens) and
3903 (the messages that `votaInit`/`votaTick` post to the voter thread).

Reconstructed sources (the `.u33.*` files are fragments of files owned by other units; merge them there):

```
src/uenux2/src/api/hwil/iurna.h                                   IUrna interface (path inferred)
src/uenux2/mock/api/hwil/curnamock.h                              api::teste::CUrnaMock (path inferred)
src/uenux2/mock/api/hwil/cpowermock.h                             api::teste::CPowerMock + SStatusEnergia (path inferred)
src/uenux2/src/api/audio/itexttospeech.h / .cpp                   api::ITextToSpeech (path inferred)
src/uenux2/src/api/audio/crhvoicetexttospeech.u33.cpp             api::Logger::log + index of the RHVoice helpers
src/uenux2/src/app/comum/gravadores/cgravadorversoesarquivos.h / .cpp   mr.ver writer (path inferred)
src/uenux2/src/api/pattern/cpolysingletonlist.u33.cpp             1005, 1562, 2860, 3853
src/uenux2/src/api/pattern/iobservable.u33.cpp                    3940
src/uenux2/src/api/gui/cdatatext.u33.cpp                          1565, 1566
src/uenux2/src/api/gui/cformbuilder.u33.cpp                       2244 AddLine, 3058 AddDataTextFmt
src/uenux2/src/api/gui/cframedtext.u33.cpp                        2779, 3677
src/uenux2/src/api/gui/ctextfield.u33.cpp                         3889
src/uenux2/src/api/gui/iformfield.u33.cpp                         2904
src/uenux2/src/api/util/cstringutils.u33.cpp                      2199, 2677, 2763
src/uenux2/src/api/hash/chasharquivo.u33.cpp                      2723
src/uenux2/src/api/ipc/cmessagequeue.u33.h                        3903
src/uenux2/src/app/comum/dados/celeitores.u33.cpp                 1936
src/uenux2/src/app/comum/relatorios/crelutil.u33.cpp              2792
src/uenux2/src/app/comum/comparecimentomesario/estados/estadosregistromesarios.u33.cpp   2895
src/uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.u33.cpp                 1535, 2746, 3620
src/uenux2/src/app/vota/operador/confirmaidentidade/chabilitaaudioeleitor.u33.cpp        2743
src/uenux2/src/app/vota/operador/confirmaidentidade/cinformaeleitorpodevotar.u33.cpp     3883
src/uenux2/src/app/vota/operador/leidentidade/ieleitorimpedidovotar.u33.cpp              3628
src/uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cmenuvisualizarcandidatos.u33.cpp  2288
```

3684 (`CApplicationContextStack::Push`) is already reconstructed by unit u15 and is only mapped here.

---

## 1. Purpose, and the Portuguese terms

* **urna / UE (urna eletrônica)**: the voting machine. Models are named by year (UE2009 … UE2020, UE2022).
  **eleitor**: voter. **mesário**: poll worker. **MT (microterminal)**: the mesário's keypad with a
  4 × 40 LCD.
* **encerramento**: the end of voting, when the urna writes its **arquivos de resultado** (result files: the
  BU, the RDV, `jufa.dat`, `hash.dat`, `log.jez`, **`mr.ver`** …) and copies them to the **MR (mídia de
  resultado)**, the USB stick taken to the TSE.
* **BU (boletim de urna)**: the signed per-urna tally. **RDV (registro digital do voto)**: the shuffled
  record of every vote. **CEPESC**: TSE's encryption library, used to encrypt the BU and the fingerprint
  images when the election parameters ask for it.
* **código de carga**: the 24-digit identifier of the loading of the urna's media ("carga"). Its last six
  digits are printed as **"RESUMO DA CORRESPONDÊNCIA"** on the BU and the zerésima (the report printed at
  the opening that proves the urna holds no votes).
* **voto com áudio / áudio do eleitor**: accessible voting. The urna reads every screen aloud through
  headphones for visually impaired voters (TTS = text-to-speech, RHVoice, see `docs/libraries/rhvoice.md`).
* **liberar / habilitar o eleitor**: the mesário releases the identified voter to vote.
* **inspeção**: a random check of the voting booth that the operator thread schedules.

How the unit fits in the voting process:

1. **Start-up** (`votaInit`, `main`). The simulator bootstrap (func 8302) registers `CUrnaMock` and
   `CPowerMock` as the urna's hardware. `main` registers the rest through the raw-pointer `push` (1562), whose
   element constructor is 1005. `votaInit` posts message 9 (audio on) to the voter thread through 3903, but only
   when the page option `audioEleitorHabilitado` is true, and then message 1 (start the voter), always.
2. **Voter screens.** Every digit box is drawn by 2779; every text field's bounding box comes from 3889; the
   status header's clock is built by 3058. With accessibility on, the voter changes the speech rate with
   keys 4/6 (`ITextToSpeech` slots 6/7).
3. **Operator side** (dead in the web build, where the operator thread never runs). The mesário releases the
   voter (3883), is asked to plug the headphones in (2743), or goes back to "type your título" after an error
   (2895).
4. **Encerramento.** `CGravaResultado` registers `CGeracaoVersoesContratos` (2860) and writes `mr.ver`
   (11582). The encrypted BU uses the CEPESC table of `IUrna` slot 4, which is **1024 × 0x01** in this build.

## 2. Classes and how they relate (RTTI)

```
api::IUrna (typeinfo @1530936, no vtable of its own)            src/api/hwil/iurna.h (inferred)
 └ api::teste::CUrnaMock      vtable @1530888, 28 B               mock/api/hwil/curnamock.h (inferred)
api::IPower (typeinfo @1531032)                                   src/api/hwil/ipower.h (u18)
 └ api::teste::CPowerMock     vtable @1530952, 48 B, 17 slots     mock/api/hwil/cpowermock.h (inferred)
api::ITextToSpeech (typeinfo @1526340, vtable @1526528, 12 slots) src/api/audio/itexttospeech.h (inferred)
 ├ api::CRHVoiceTextToSpeech  vtable @1585556, 112 B              (u32)
 └ simulador::CWasmNullTextToSpeech  vtable @1528704, 96 B        (u28/u29)
RHVoice::event_logger
 └ api::Logger                vtable @1586196 = {174, 144, 10815}, 4 B
comum::IResultado ─ comum::IGravador (u23)
 └ comum::CGravadorVersoesArquivos  vtable @1557248, 64 B          comum/gravadores/cgravadorversoesarquivos.* (inferred)
```

The namespace `api::teste` ("test") says what the two mocks are: test doubles of the hardware interfaces,
reused by the simulator. Their paths are inferred from the only mock path the binary knows,
`uenux2/mock/app/comum/cappinfobuilder.cpp`, which mirrors `src/app/comum`.

### 2.1 `api::teste::CUrnaMock` (IUrna)

| slot | func | method (all names inferred) | mock behaviour |
|---|---|---|---|
| 0 | 1661 (ICF `return this[+4]`) | `int GetModelo()` | **2020** |
| 1 | 8168 | `std::string GetModeloAbreviado()` | `std::format("{:02}", modelo - 2000)` = `"20"` |
| 2 | 2587 (ICF `return this[+8]`) | `GetNumeroInterno()` ? | **87654321** (the `numeroInternoUrna` of the simulator's `eg.bin` fixture) |
| 3 | 3383 (ICF `return this[+12]`) | `GetRevisaoHardware()` ? | 255 |
| 4 | 8144 | `GetTabelaCepesc(array<uebyte,1024>&)` | the configured table, padded with 0x01. Nothing configures it, so the table is **1024 × 0x01** |
| 5 | 8134 | `GetDados32(array<uebyte,32>&)` ? | 32 × 0x02 |
| 6 | 8131 | `GetTabelaRdv(array<uebyte,128>&)` | 128 × 0x03 |
| 7 / 8 | 8130 / 8129 | destructor / deleting destructor | frees the table vector (+16) |

Layout: `+4 int modelo = 2020`, `+8 uint32 = 87654321`, `+12 int = 255`, `+16 std::vector<uebyte>` (empty).
The constructor is inlined in func 8302 as three `i64` stores.

Only slots 0, 4 and 6 have callers (14 call sites in 13 functions, all through `CPolySingletonList::instance<IUrna>`,
func 923).
Slot 0 decides model-dependent behaviour: the audio instruction says the keypad is "abaixo" (below the
screen) for models > 2019, the fingerprint capture accepts the first frame on 2009/2010/2020/2022,
`CEscolheOpcao` and `CPedeIdentidade` switch the microterminal's menu mode (IScreenMT slot 19) from 2020,
`CGravaResultado` logs a session on the "MSE" instead of the "MSD" security module, `CThreadMonitor` logs the
power voltages only from 2020, and `IInterfaceInit` picks the MR port. The web build is therefore always a
**UE2020**.
Slot 4 feeds the CEPESC encryption of the BU (`CGravadorBU::GravaResultado`, srcloc `cgravadorbu.cpp:484`)
and of the fingerprint images (`CControlaArmazenamentoDeImagens`). Slot 6 is the table from which the RDV key
is derived (u01/u02: 32 bytes chosen by the SHA-512 of the office list, then HKDF). **All three "secret" tables
are public constants in this build.** That is expected in a simulator. For the RDV the consequence is direct:
its AES key is a function of public data, so anyone with the binary can decrypt a simulator `rdv.dat` (u01/u02).
For CEPESC the table is only one input, next to a 32-byte random seed (`IRng`) and the TSE public key, so what
a public table weakens cannot be judged from this unit.

### 2.2 `api::teste::CPowerMock` (IPower)

Despite the interface's name, the 16-byte status word of `IPower` is the urna's general hardware status. The
meanings below come from the code that tests the bits (CPowerInformation, CFormBuilder, and the log messages of
`vota::CThreadMonitor`):

```
+0  u32 flags  & 0x06 fonte (0 rede elétrica, 2 bateria interna, 4 bateria externa)
               & 0x18 bateria interna (0 cheia, 0x08 parcial, 0x10 crítica, 0x18 ausente)
               & 0x60 bateria externa (same codes)      0x0200 mídia externa ausente (error 9390)
               0x4000 chave "URNA DESLIGADA"            0x2000 slot 0      0x0800 ? (set by the mock)
+4  u16  bit 1 = fone de ouvido desconectado            +6  u16 tensão bateria interna (V x 100)
+8  u16  tensão bateria externa (V x 100)               +10 u16 tensão rede CA (V x 100)
+12 u16  corrente (A x 100), read as "percentual" by the mock's slot 7
+14 u8   ? (slot 6)                                     +15 u8 bit 0 = teclado do eleitor desconectado (9391)
```

| slot | func | method (`?` = inferred) | body |
|---|---|---|---|
| 0 | 8128 | `EhAlimentacaoExterna()` ? | `!(flags & 0x2000)` |
| 1 | 8120 | `GetTensaoBateriaInterna()` (u18: `vf1`) | +6 |
| 2 | 8116 | `GetCorrenteBateria()` | `±`corrente, negative when the AC voltage is 0; 0 if the internal battery is absent |
| 3 | 8113 | `GetTensaoBateriaExterna()` (u18: `GetTensaoBateria`) | +8 |
| 4 | 8111 | `GetTensaoRedeCA()` (u18: `GetTensaoCarregador`) | +10 |
| 5 | 8109 | `GetCargaEstimada()` ? | `round(fator(tensão) × corrente)`, a 20-step voltage table (see §9, a bug) |
| 6 | 8108 | `GetTemperatura()` ? | +14 (byte) |
| 7 | 8107 | `GetPercentualBateria()` | the mock's own +32 (its copy of +12), clamped to 100 **in place** |
| 8-12 | 8105 … 8074 | `IPower` defaults (`"Not supported"`, `ipower.h:354…424`) | |
| 13/14 | 174/144 | trivial destructors | |
| 15 | 8071 | `AtualizaStatus(SStatusEnergia&)` | copies the 16-byte simulated status (+20..+35) |
| 16 | 434 (ICF `return 1`) | ? | |

Callers: slot 15 plus inline bit tests (status header, battery icon, `CMonitoraAlimentacao`, the automatic
shut-down states, `CRelatorioTesteImpressora`, `CThreadMonitor`); slot 7 (the header text `"{: >3}%"`); slots
1-4 (`CThreadMonitor::LogaStatusRedeAcBateria`, models ≥ 2020 only: "Urna ligada conectada na rede CA em
[{:3.2f}V] e na Bateria Interna com [{:2.2f}V/{}A]"). Slots 0, 5, 6 and 16 have no caller. The monitor thread
never runs in the web build, so in the recordings only slots 15 and 7 are reachable.

Slots 0-6 refresh `IPower::m_status` (+4) through slot 15 and then read only that base member. They may be
default bodies in `ipower.h` rather than mock overrides; the vtable cannot tell. **Cross-unit mismatch:** u18's
`ipower.h` declares no data member at all (the 16-byte status at +4 must be added there as `m_status`) and names
slots 1/3/4 `vf1` / `GetTensaoBateria` / `GetTensaoCarregador`, so the `override`s of `cpowermock.h` do not match
it yet. The names used here come from `CThreadMonitor::LogaStatusRedeAcBateria`, which prints slot 4 as "rede CA"
and uses slot 3 ("Bateria Externa") when it is > 0, otherwise slot 1 ("Bateria Interna"). The mock's constant status
`{0x840, 2, 0, 0, 0, 100}` means: mains power, internal battery full, external battery "crítica" (0x40),
headphones reported as disconnected, all voltages 0, 100 %. The battery icon is therefore `img-ac-bateria-full`
and the header shows "100%". `+40` holds `steady_clock::now()` from construction and is never read.

### 2.3 `api::ITextToSpeech` and `api::Logger`

Layout (96 bytes; constructor inlined in both implementations, funcs 13877 and 10842):

```
+4   CLruCache<string, SharedWav>  m_cacheRecente   capacity 256, list + unordered_map (max_load 1.0)
+40  unordered_map<string, SharedWav> m_cachePermanente
+60  int m_nivelVelocidade = 2      +64 std::string m_perfilVoz = ""
+76  int m_p76 = 50  (?)            +80 int m_p80 = 100 (tom/pitch %)
+84  int m_taxa = 100 (rate %)      +88 int64 m_p88 = 0 (?)
```

| slot | func | method | body |
|---|---|---|---|
| 1 | 11518 | `SetPerfilVoz(const string&)` | `m_perfilVoz = s` |
| 2 | 11511 | `SetTom(int)` | `m_p80 = v` |
| 3 | 11507 | `SetP76(int)` ? | `m_p76 = v` |
| 4 | 11505 | `SetVelocidade(int nivel)` | `nivel`, `taxa = nivel*20 + 60` (no range check) |
| 5 | 11504 | `GetVelocidade()` | `nivel` |
| 6 | 11500 | `AumentaVelocidade()` | `nivel = min(nivel+1, 4)` |
| 7 | 11494 | `DiminuiVelocidade()` | `nivel = max(nivel-1, 0)` |
| 8 | 11491 | `SetP88(int64)` ? | `m_p88 = v` |
| 9 | 11489 | `~ITextToSpeech()` | string, permanent cache (libc++ 3746), LRU cache (3764); also slot 9 of the Null TTS |
| 10 | 325 | deleting destructor of the abstract base | a bare `unreachable` (clang emits a trap for D0 of an abstract class) |

Rates: level 0..4 = **60, 80, 100, 120, 140 %**. **Run-time check** (`headless.mjs --audio --save-audio`,
keys `6 6 6 4 4 4 4 4` on the audio-instruction screen): the answers "Fala mais rápida" (120 %, 37,832 B → 140 %,
32,192 B), "Fala em velocidade máxima" (at 4, 140 %), "Fala mais lenta" at 120/100/80/60 % (33,544 / 40,044 /
50,044 / 66,544 B, ratios 1.19, 1.25, 1.33 = 120/100, 100/80, 80/60), then "Fala em velocidade mínima".
This confirms the table and the saturation of slots 6 and 7.

Slots 1, 2, 3 and 8 are never called, so every request uses perfil `""`, tom 100 %, and the constant fields.
The cache key (func 5758, u02) formats `(texto, taxa, perfil, tom, p88)`, in that order. `p76` is in neither the
key nor the RHVoice request (func 10841 reads only `taxa` and `tom`). It is dead data in this build.

`api::Logger::log` (10815) overrides `RHVoice::event_logger::log(tag, level, message)`. It prints only
`RHVoice_log_level_error` (4), as `RHVoice [<tag>]: <message>` + `std::endl`, to **`std::clog`**
(@1923832). `docs/libraries/rhvoice.md` says `std::cerr`. The init code in `__wasm_call_ctors` shows 1923688 = cerr
(unitbuf, tied to cout) and 1923832 = clog, both on stderr. In the page stderr goes to `console.error`.

## 3. The `mr.ver` file (`CGravadorVersoesArquivos::GravaResultado`, func 11582)

`mr.ver` is one of the 11 result files of the encerramento. It records the **version of every ASN.1 contract**
(module) used by the other result files, so that the TSE's reading software knows how to decode them. This
section is the part of the BU/encerramento flow that this unit owns. The BU itself is in u23 (`CGravadorBU`) and
docs/10-boletim-de-urna.md.

Step by step (writer #10 of `vota::CGravaResultado::StartState`, func 12098, units u07/u09):

1. `CGravaResultado` collects the ASN.1 modules of the writers (`ModuloBoletimUrna`, `ModuloEnvelopeGenerico`,
   `ModuloResultadoUrnaCadastro`, `ModuloHashes`, `ModuloAssinaturaEcourna`, `ModuloVersaoArquivos`).
2. `CPolySingletonList::exists<comum::CGeracaoVersoesContratos>()` (**func 2860**, this unit). If the object
   is not registered yet, it is built from `<root>/etc/dependencias.properties` and `<root>/etc/versoes.properties`.
   Both must carry `tag=20260601173148`. In the simulator both are 0-byte stubs, so this throws 8662 (u09). The
   same `exists` is also evaluated once by `__wasm_call_ctors`.
3. The module list is closed over its transitive dependencies, and each module gets its version →
   `md::CVersoesArquivos(TAG_CONTRATOS, map<módulo, versão>)` (func 5867: 8694 "Versão da TAG inválida" if the
   tag is empty, 8695 if the map is empty).
4. `new CGravadorVersoesArquivos(município, zona, local, seção, fase, versoes)`: `IGravador` with
   `EExtensaoArquivoResultado 19` = `mr.ver` and SAVD file id 70. The object is 64 bytes, with the tag at +40 and the map at +52.
5. `IGravador::Grava()` / `GravaMV()` (slots 4/5, u23) open the file named by `CGravadorUtil::DeterminaNomeArquivo`
   (u23: `<fase><pleito:05><uf><mun:05><zona:04><seção:04>-` + the `mr.ver` suffix) in the work directory, with
   `"wb"`, on the MI (internal flash) and the MV (voting media), and call **slot 7 = func 11582**:
   * `CConversorVersoesArquivos::DoConverte` (10264, u21) builds
     `EntidadeVersaoArquivos ::= SEQUENCE { versaoTag, arquivos SEQUENCE OF ArquivoAssinatura { nome, versão } }`
     in `std::map` order (sorted by module name);
   * `isValid() && isStrictlyValid()`, or `CBaseError<EUeComumAsnError>` **7653** "Entidade deixada em estado
     inválido: {}" + `ASN1::trace_invalid` of `N20ModuloVersaoArquivos22EntidadeVersaoArquivosE` (iconversorasn.h:56);
   * context `"WriteToFile de " + arquivo.GetName()`; `isValid()` again or `CBaseError<EUeIoError>` **5955**
     "Objeto com conteúdo inválido para {}: {}" (cfileasn.h:161);
   * BER encoding (`ASN1::CoderEnv::encodeBER`), or **5956** "Arquivo não foi codificado para " + context
     (cfileasn.h:171);
   * `CFile::RawWrite(buffer)` (func 1886).
6. The file is copied to the result directories (IGravador slots 2/3/6), signed by SAVD with the other result
   files into `…-vota.vsc` (`CAssinador::AssinaArquivosResultado`, u09), and copied to the MR. In this build
   the signatures are the literal `assinatura simulada para vota_web_wasm`.

The web page never reaches this step: it stays in voter-training mode and never runs the encerramento
(docs/bu/codepath.md). Other BU-related pieces in this unit:

* **2792** `CRelUtil::FormataCorrespondencia`: the "RESUMO DA CORRESPONDÊNCIA" of the printed BU, the zerésima
  and the zerésima-time screens = `codigoCarga.substr(18,3) + "." + codigoCarga.substr(21)` (last 6 of the 24
  digits: `537864991559016480376254` → `376.254`, fixture → `901.234`).
* **IUrna slot 4** (`CUrnaMock` 8144): the 1024-byte CEPESC table of the encrypted BU (`CGravadorBU`,
  `cgravadorbu.cpp:484`). In the simulator it is 1024 × 0x01.
* **2244** `CFormBuilder::AddLine`: the separator lines of `CMostraQRCodeBU` (the "BU digital" QR code screen)
  and `CImprimirBUOutrasObrigatorias`.
* **2763** `CStringUtils::NomeArquivo`: the file-name part of the signature package path (`…-vota.vsc`) in
  `CGravaResultado`, and the names written into `hash.dat` by `CMontadorHash`.

## 4. The other TSE helpers

### 4.1 GUI (`api/gui`)

* **2244** `CFormBuilder::AddLine(inicio, fim, cor)`: `make_shared<CLineField>` (36 B, vtable @1579364) + `Add` (426).
* **3058** `CFormBuilder::AddDataTextFmt(fonte, pos, período, fonte, formato, alinhamento)`:
  `CDataTextFmt<std::string (*)(const std::string&)>` inside a `CTextFieldUpdate`. The status header uses it
  for the clock `"A DD/MM/YYYY hh:mm:ss"`, refreshed every 500 ms. Observed.
* **2779** `CFramedText::DesenhaMoldura(i, cor, tela)` → `IScreen::DrawRect(caixa i, cor, 1)`, and
  **3677** `CFramedText::Rect()`: the digit boxes. Box width = the widest glyph + margins, spacing =
  `size <= 7 ? 1 : size / 8`.
* **3889**: the shared body of `CTextField::Rect`, `CTextFieldBlinking::Rect` and `CTextFieldUpdate::Rect`
  (thunks 10951, 10943, 10913). It measures the text with `IScreen::GetFontMetrics` and shifts it by the
  alignment (right: x−w, centre: x−w/2).
* **1566 / 1565**: complete / deleting destructor bodies shared by `IFormFieldBase<MEDIA>` and
  `CDataTextFmt<SRC>` (an `IText`). Both are `{vptr, 2 words, std::string @+12}`. 1565 is also the D0 of nine
  `IFormField` leaf classes with no member to destroy, through three ICF thunks that pass the *base* vtable
  (their own vptr store is dead): 2777 `IFormFieldBase<IScreenMT>` (CBeepFieldMT, CLedFieldMT, CBuzzFieldMT,
  CClockFieldMT), 3681 `IFormFieldBase<IScreen>` (CFillField, CLineField, CRectField) and 5516
  `IFormFieldBase<IPaper>` (CCutFieldPaper, CNewLineFieldPaper).
* **2904**: `std::string` from an 11-character literal. It is shared by `CMovieField/CImageField/CInputField::GetClassName`
  and RHVoice's `userdict::empty_string::describe` ("EmptyString").
* **3940**: `IObservable<std::vector<FormHandle<MEDIA>>>::~IObservable` (MEDIA = IScreen/IScreenMT/IPaper).
* **3684**: `CApplicationContextStack::Push` (u15). A generic context is refused on top of a specific one.

### 4.2 Registry (`api/pattern`)

* **1005**: the `SPolySingleton{nome, tipo, instancia}` element constructor that every `emplace_back`
  instantiation passes by table slot (12 slots, one folded body). Observed.
* **1562**: `push<INTERFACE>(INTERFACE* p, info)` = `push(std::unique_ptr<INTERFACE>(p), info)`. The `push` to
  call is a parameter. It is used by the 8 wrappers `main` invokes (IExecucaoVota, the 3 IPC factories,
  IThreadImpl factory, IFingerPrepare, ISymmetricCipherFactory, IRng). Observed.
* **2860 / 3853**: `exists<comum::CGeracaoVersoesContratos>` and `exists<vota::testeteclado::impl::IGeradorTeclas>`
  (the latter from the keyboard test `CTesteTeclado::StartState`). They did not fold with the other `exists`
  bodies because the mangled name is built inline and its length (34, 42) has no partner.

### 4.3 Strings, errors, messages

* **2199** `CStringUtils::ProcuraRegex(expressão, texto)` → `{início, fim}` of the first boost::regex match,
  or `{-1,-1}`. The second field is the **end**, not the length (u20 declared `{posicao, tamanho}`). Every
  exception is swallowed. Used by `GetVersionNumber` ("10.23.0.1 - DESENVOLVIMENTO" → "10.23.0.1") and by
  `CTradutorFrase::TraduzLabel` (report label templates). Observed only through `TraduzLabel` (654, called by
  4160/4161; `GetVersionNumber` never appears in the profiles). TraduzLabel calls it three times per loop and every
  call compiles its pattern again (`basic_regex::do_assign` 9401 is 139 of the 182 profiler samples of 2199 in the
  municipal session: about 2.8 ms of 3.6 ms at 20 µs per sample).
* **2677** `CStringUtils::ReplaceAll` (copy + in-place loop 9407): `CProgressBar` "%p", `CIniStrings`,
  `CSubstituidorTitulo`.
* **2763** `CStringUtils::NomeArquivo`: the text after the last `/`.
* **2723**: the `CBaseError<api::EUeHashError>` constructor thunk (5100-5103 of `api/hash`).
* **1936** `MontaCaminhos(dir, nomes)` → `{dir + nome}` (voter-roll files of `CEleitores`).
* **3903** `CPriorityMessageQueue<SMessage>::Envia(id)` = `Add({id, this}, 1)`. The web entry point's three
  lambdas post MSG_AUDIO_HABILITADO (9, only if `audioEleitorHabilitado`) and then MSG_INICIA_ELEITOR (1) at
  `votaInit`, and MSG_SINCRONIZA_VOTO (5) from `votaTick` after the four progress-bar steps. Observed (only the
  `votaInit` → 11026 → 3903 path has samples). The `Add(msg, 1)` it calls is a separate one-line function (7708),
  i.e. an `Envia(const SMessage&)` overload that u18's `cmessagequeue.h` does not declare.

### 4.4 Operator (mesário) states: dead in the web build

```
CNomeEleitor / CDigitalReconhecida / CControlaReconhecimento / CInformaEleitorPodeVotar
CInformaAnoDesabilitadoDemo / CInformaBioDesabilitadaDemo
        │ CONFIRMA (3883 = merged ProcessInput; the same sequence is inlined in the first three)
        ├─ IInformacaoThreadOperador::DeveHabilitarAudio() (2746)? ── yes ─▶ CHabilitaAudioEleitor (2743)
        │                                                                 CONFIRMA: msgs 8|9 then 1 (u19 10523)
        └─ no: CThreadEleitor fila.Add({1 MSG_INICIA_ELEITOR}, 1) ─────▶ CMostraEleitorVotando (1150)
```

* **2743** `CHabilitaAudioEleitor::GetInst()`, with the constructor inlined: MT form with buzzer (51, 10), clock
  {33,1}, "Este eleitor necessita de áudio" {1,2}, "Coloque o fone de ouvido na urna" {1,3},
  "CONFIRMA: continuar" {1,4} with alignment 1, and an input control. (For right alignment the MT ignores x:
  `CWasmScreenMT::Write`, func 8799, writes at column `40 - len`, so the line ends at column 40.)
* **2895**: the shared `ProcessInput` of the four mesário-registration error screens (`CTituloMesarioVazio`,
  `…Invalido`, `…JaRegistrado` with CORRIGE, `CDigitalMesarioNaoReconhecida` with CONFIRMA) → back to
  `CPedeTituloMesario`.
* **1535 / 2746 / 3620**: out-of-line `IInformacaoThreadOperador::GetInst().GetIdentidadeEleitor()` (by value;
  the sret pointer comes before `this` in the wasm ABI), `.DeveHabilitarAudio()` and `.SorteiaProximaInspecao()`.
* **3628** `TextoVazio()` → `" "` (data source of blank MT lines). **2288** `CMenuVisualizarCandidatos::GetInst()`.

## 5. Library functions (not reconstructed)

10 RHVoice 1.14.0 functions and 5 libc++ templates instantiated for RHVoice types (checked against the upstream
tag), called from the inlined TTS pipeline (func 10841) or from other RHVoice code:
the Unicode table lookups `unicode::properties` (590), `tolower` (854) and `category` (1144) over
`records[23697]` @626784 (`{code, category, upper, lower, properties}`); `emoji_scanner::process` (1896);
`userdict::position::set_token` (2703) / `forward_token` (2210) and `dict::should_ignore_token` (3547);
`item::append_child(item&)` (2214); `voice_search_criteria::operator()` (3529); `fst::translate<text_iterator,
item::back_insert_iterator>` (3574); the two `std::find_if` instances of `str::tokenizer<is_space>` (2756, 3571);
the text-iterator copy (2754); and the `std::map<string, shared_ptr<T>>` node destroyer (2715) and
`set<string, str::less>` hinted insert (2716). Plus libc++: `filesystem::path::__parent_path()` (1840) and
`map<uebyte, string>` hinted insert (3266). See the index in `crhvoicetexttospeech.u33.cpp`.

## 6. Web-build specifics

* The two mocks are the whole "hardware" of the urna for identification, secrets and power: fixed model 2020,
  fixed identification 87654321, public tables (0x01 / 0x02 / 0x03), permanent mains power at 100 %.
* The TTS base class is shared by the real engine (RHVoice, only when the page enables accessibility) and the
  silent `CWasmNullTextToSpeech`. The recorded votes ran with the silent one, so no `ITextToSpeech` slot shows up
  in the profiles. The audio run above exercised slots 4-7.
* `mr.ver` (and the whole encerramento) is never written by the page.
* Operator states (2743, 2895, 3883, 1535, 2746, 3620, 3628) are dead: `CThreadOperador::Run` never starts.
  `votaInit` posts the "voter released" message itself (3903).

## 7. wasm / Emscripten observations

* **merge-similar-functions everywhere.** A third of this unit is bodies that Binaryen shared between unrelated
  functions by turning the differing constant into a parameter:
  * a **`std::source_location`** as parameter: 3889 (three `Rect()`), 3883 (three `ProcessInput`), 2895 (four
    `ProcessInput`, plus the expected key);
  * a **vtable** as parameter: 1565/1566 (destructors), 3940 (`~IObservable`), 2723 (`CBaseError` constructor);
  * a **function (table slot)** as parameter: 1562 (which `push` to call);
  * **string literal addresses** as parameters: 2904 (`s` and `s+7` of an 11-character literal).
  The tools file such a body under the class of whichever caller they meet first, which is why these ended up
  "without a known file".
* **ICF across unrelated classes.** `CUrnaMock` slots 0/2/3 are the generic `return this->[+4/+8/+12]` bodies
  (1661, 2587, 3383). 1005 is the one element constructor behind 12 table slots.
* **sret before `this`.** 1535 is a thunk into slot 25, which returns a class by value. The call reads
  `vf25(result, instance)` because the wasm (Itanium) ABI passes the return slot first.
* **Abstract deleting destructors are traps.** `ITextToSpeech` slot 10 = func 325 = `unreachable`.
* **Short strings are immediates.** 3628 returns `" "` by storing `0x0020` and the size byte. 2743, 2860 and 3853
  build their literals with 8-byte loads (see the libcxx chapter).
* **Floating-point tables stay in code.** `CPowerMock::vf5` is a chain of `f64.const` / `br_if` (a `select` chain
  in the decompiler), which is how the 10× typo in two constants shows up in §9.

## 8. Mapping table (all 72 functions)

`run` = seen in `analysis/runtime/*.functions.tsv`. Paths are under `src/`; `(u15)` etc. = reconstructed by that unit.

| func | size | run | reconstructed symbol | component | reconstruction |
|---:|---:|:-:|---|---|---|
| 590 | 139 | | `RHVoice::unicode::properties(utf8::uint32_t)` | lib:rhvoice | library (RHVoice `src/core/unicode.cpp`), index in `api/audio/crhvoicetexttospeech.u33.cpp` |
| 854 | 135 | | `RHVoice::unicode::tolower(utf8::uint32_t)` | lib:rhvoice | library (unicode.cpp) |
| 1005 | 61 | * | `api::SPolySingleton::SPolySingleton(nome, tipo, instancia)` (emplace_back element ctor, 12 slots) | app:api | `api/pattern/cpolysingletonlist.u33.cpp` (orig. cpolysingletonlist.h) |
| 1144 | 153 | | `RHVoice::unicode::category(utf8::uint32_t)` | lib:rhvoice | library (unicode.cpp) |
| 1535 | 20 | | `vota::IdentidadeEleitor()` = `IInformacaoThreadOperador::GetInst().GetIdentidadeEleitor()` | app:vota | `vota/operador/comum/cinformacaothreadoperador.u33.cpp` |
| 1562 | 142 | * | `api::CPolySingletonList::push<I>(I*, TPolySingletonsInfo&)` (merged body) | app:api | `api/pattern/cpolysingletonlist.u33.cpp` |
| 1565 | 39 | | deleting-destructor body shared by `CDataTextFmt<SRC>` and (via ICF 2777/3681/5516) nine `IFormField` leaf classes; takes the vtable | app:api | `api/gui/cdatatext.u33.cpp` |
| 1566 | 36 | | `IFormFieldBase<MEDIA>::~IFormFieldBase` / `CDataTextFmt<SRC>::~CDataTextFmt` (merged body) | app:api | `api/gui/cdatatext.u33.cpp` |
| 1840 | 316 | * | `std::filesystem::path::__parent_path() const` | rt:libcxx | library/inlined helper (libc++ `filesystem/path.cpp`) |
| 1896 | 294 | | `RHVoice::emoji_scanner::process(utf8::uint32_t)` | lib:rhvoice | library (emoji.cpp) |
| 1936 | 370 | | `comum::(anon)::MontaCaminhos(dir, nomes)` | app:comum | `app/comum/dados/celeitores.u33.cpp` |
| 2199 | 723 | * | `api::CStringUtils::ProcuraRegex(expressao, texto)` | app:api | `api/util/cstringutils.u33.cpp` |
| 2210 | 119 | | `RHVoice::userdict::position::forward_token()` | lib:rhvoice | library (userdict.cpp) |
| 2214 | 189 | | `RHVoice::item::append_child(item&)` | lib:rhvoice | library (item.cpp) |
| 2244 | 167 | | `api::CFormBuilder::AddLine(inicio, fim, cor)` | app:api | `api/gui/cformbuilder.u33.cpp` |
| 2288 | 22 | | `vota::CMenuVisualizarCandidatos::GetInst()` | app:vota | `app/vota/eleitor/iniciovotacao/auxiliares/cmenuvisualizarcandidatos.u33.cpp` |
| 2677 | 128 | | `api::CStringUtils::ReplaceAll(texto, de, para)` | app:api | `api/util/cstringutils.u33.cpp` |
| 2703 | 224 | | `RHVoice::userdict::position::set_token(item&)` | lib:rhvoice | library (userdict.hpp) |
| 2715 | 109 | | `std::__tree<string, shared_ptr<T>>::destroy(node)` (ICF, RHVoice maps) | lib:rhvoice | library/inlined helper (libc++ template) |
| 2716 | 704 | | `std::__tree<string, str::less>::__emplace_hint_unique_key_args` | lib:rhvoice | library/inlined helper (libc++ template) |
| 2723 | 18 | | `CBaseError<api::EUeHashError>::CBaseError(code, msg, loc)` (thunk into 710) | app:api | `api/hash/chasharquivo.u33.cpp` |
| 2743 | 882 | | `vota::CHabilitaAudioEleitor::GetInst()` + inlined constructor | app:vota | `app/vota/operador/confirmaidentidade/chabilitaaudioeleitor.u33.cpp` |
| 2746 | 20 | | `vota::DeveHabilitarAudio()` = `IInformacaoThreadOperador::GetInst().DeveHabilitarAudio()` | app:vota | `vota/operador/comum/cinformacaothreadoperador.u33.cpp` |
| 2754 | 1398 | | copy of a `utf::text_iterator` range into `utf8::uint32_t*` (`std::copy` / `__uninitialized_allocator_copy`) | lib:rhvoice | library/inlined helper (utfcpp `next` inlined) |
| 2756 | 484 | | `std::find_if(first, last, std::not1(str::is_space))` | lib:rhvoice | library (str.hpp tokenizer) |
| 2763 | 281 | | `api::CStringUtils::NomeArquivo(caminho)` | app:api | `api/util/cstringutils.u33.cpp` |
| 2779 | 166 | * | `api::CFramedText::DesenhaMoldura(i, cor, tela)` | app:api | `api/gui/cframedtext.u33.cpp` |
| 2792 | 499 | * | `comum::CRelUtil::FormataCorrespondencia(correspondencia)` | app:comum | `app/comum/relatorios/crelutil.u33.cpp` |
| 2860 | 381 | | `api::CPolySingletonList::exists<comum::CGeracaoVersoesContratos>` | app:api | `api/pattern/cpolysingletonlist.u33.cpp` |
| 2895 | 127 | | merged `ProcessInput` of CTituloMesarioVazio/Invalido/JaRegistrado, CDigitalMesarioNaoReconhecida | app:comum | `app/comum/comparecimentomesario/estados/estadosregistromesarios.u33.cpp` |
| 2904 | 57 | | merged `GetClassName()` body for 11-char literals (CMovieField, CImageField, CInputField, EmptyString) | app:api | `api/gui/iformfield.u33.cpp` |
| 3058 | 352 | * | `api::CFormBuilder::AddDataTextFmt(fonte, pos, periodo, fonte, formato, alinhamento)` | app:api | `api/gui/cformbuilder.u33.cpp` |
| 3266 | 217 | * | `std::map<uebyte, std::string>` hinted insert (`__emplace_hint_unique_key_args`) | rt:libcxx | library/inlined helper (initializer-list construction) |
| 3529 | 191 | | `RHVoice::voice_search_criteria::operator()(const voice_info&)` | lib:rhvoice | library (voice.cpp) |
| 3547 | 454 | | `RHVoice::userdict::dict::should_ignore_token(const position&)` | lib:rhvoice | library (userdict.cpp) |
| 3571 | 486 | | `std::find_if(first, last, str::is_space)` | lib:rhvoice | library (str.hpp tokenizer) |
| 3574 | 361 | | `RHVoice::fst::translate<utf8_string_iterator, item::back_insert_iterator>` | lib:rhvoice | library (fst.hpp) |
| 3620 | 20 | | `vota::SorteiaProximaInspecao()` = `IInformacaoThreadOperador::GetInst().SorteiaProximaInspecao()` | app:vota | `vota/operador/comum/cinformacaothreadoperador.u33.cpp` |
| 3628 | 16 | | `vota::(anon)::TextoVazio()` → `" "` | app:vota | `app/vota/operador/leidentidade/ieleitorimpedidovotar.u33.cpp` |
| 3677 | 133 | | `api::CFramedText::Rect()` | app:api | `api/gui/cframedtext.u33.cpp` |
| 3684 | 628 | * | `api::CApplicationContextStack::Push(CApplicationContext)` | app:api | `api/gui/capplicationcontextstack.u15.cpp` (u15) |
| 3853 | 393 | | `api::CPolySingletonList::exists<vota::testeteclado::impl::IGeradorTeclas>` | app:api | `api/pattern/cpolysingletonlist.u33.cpp` |
| 3883 | 173 | | merged `ProcessInput` of CInformaEleitorPodeVotar / CInformaAnoDesabilitadoDemo / CInformaBioDesabilitadaDemo | app:vota | `app/vota/operador/confirmaidentidade/cinformaeleitorpodevotar.u33.cpp` |
| 3889 | 285 | * | merged `Rect()` of CTextField / CTextFieldBlinking / CTextFieldUpdate | app:api | `api/gui/ctextfield.u33.cpp` |
| 3903 | 44 | * | `api::CPriorityMessageQueue<SMessage>::Envia(int16 id)` | app:api | `api/ipc/cmessagequeue.u33.h` |
| 3940 | 167 | | `IObservable<vector<FormHandle<MEDIA>>>::~IObservable` (merged body) | app:api | `api/pattern/iobservable.u33.cpp` |
| 8071 | 22 | | `api::teste::CPowerMock::AtualizaStatus(SStatusEnergia&)` (slot 15) | app:mock | `uenux2/mock/api/hwil/cpowermock.h` |
| 8107 | 47 | | `CPowerMock::GetPercentualBateria()` (slot 7) | app:mock | `uenux2/mock/api/hwil/cpowermock.h` |
| 8108 | 25 | | `CPowerMock::GetTemperatura()` ? (slot 6) | app:mock | `uenux2/mock/api/hwil/cpowermock.h` |
| 8109 | 401 | | `CPowerMock::GetCargaEstimada()` ? (slot 5) | app:mock | `uenux2/mock/api/hwil/cpowermock.h` |
| 8111 | 25 | | `CPowerMock::GetTensaoRedeCA()` (slot 4) | app:mock | `uenux2/mock/api/hwil/cpowermock.h` |
| 8113 | 25 | | `CPowerMock::GetTensaoBateriaExterna()` (slot 3) | app:mock | `uenux2/mock/api/hwil/cpowermock.h` |
| 8116 | 62 | | `CPowerMock::GetCorrenteBateria()` (slot 2) | app:mock | `uenux2/mock/api/hwil/cpowermock.h` |
| 8120 | 25 | | `CPowerMock::GetTensaoBateriaInterna()` (slot 1) | app:mock | `uenux2/mock/api/hwil/cpowermock.h` |
| 8128 | 29 | | `CPowerMock::EhAlimentacaoExterna()` ? (slot 0) | app:mock | `uenux2/mock/api/hwil/cpowermock.h` |
| 8129 | 47 | | `api::teste::CUrnaMock::~CUrnaMock()` deleting (slot 8) | app:mock | `uenux2/mock/api/hwil/curnamock.h` |
| 8130 | 44 | | `api::teste::CUrnaMock::~CUrnaMock()` (slot 7) | app:mock | `uenux2/mock/api/hwil/curnamock.h` |
| 8131 | 12 | | `CUrnaMock::GetTabelaRdv(array<uebyte,128>&)` → 128 × 0x03 (slot 6) | app:mock | `uenux2/mock/api/hwil/curnamock.h` |
| 8134 | 62 | | `CUrnaMock::GetDados32(array<uebyte,32>&)` ? → 32 × 0x02 (slot 5) | app:mock | `uenux2/mock/api/hwil/curnamock.h` |
| 8144 | 87 | | `CUrnaMock::GetTabelaCepesc(array<uebyte,1024>&)` → 1024 × 0x01 (slot 4) | app:mock | `uenux2/mock/api/hwil/curnamock.h` |
| 8168 | 366 | | `CUrnaMock::GetModeloAbreviado()` → `"20"` (slot 1) | app:mock | `uenux2/mock/api/hwil/curnamock.h` |
| 10815 | 187 | | `api::Logger::log(tag, nivel, mensagem)` | app:api | `api/audio/crhvoicetexttospeech.u33.cpp` |
| 11489 | 39 | | `api::ITextToSpeech::~ITextToSpeech()` (slot 9) | app:api | `api/audio/itexttospeech.cpp` |
| 11491 | 9 | | `ITextToSpeech::SetP88(int64)` ? (slot 8) | app:api | `api/audio/itexttospeech.cpp` |
| 11494 | 37 | | `ITextToSpeech::DiminuiVelocidade()` (slot 7) | app:api | `api/audio/itexttospeech.cpp` |
| 11500 | 42 | | `ITextToSpeech::AumentaVelocidade()` (slot 6) | app:api | `api/audio/itexttospeech.cpp` |
| 11504 | 7 | | `ITextToSpeech::GetVelocidade()` (slot 5) | app:api | `api/audio/itexttospeech.cpp` |
| 11505 | 22 | | `ITextToSpeech::SetVelocidade(int)` (slot 4) | app:api | `api/audio/itexttospeech.cpp` |
| 11507 | 9 | | `ITextToSpeech::SetP76(int)` ? (slot 3) | app:api | `api/audio/itexttospeech.cpp` |
| 11511 | 9 | | `ITextToSpeech::SetTom(int)` (slot 2) | app:api | `api/audio/itexttospeech.cpp` |
| 11518 | 13 | | `ITextToSpeech::SetPerfilVoz(const std::string&)` (slot 1) | app:api | `api/audio/itexttospeech.cpp` |
| 11582 | 1668 | | `comum::CGravadorVersoesArquivos::GravaResultado(api::CFile&)` (slot 7) | app:comum | `app/comum/gravadores/cgravadorversoesarquivos.cpp` |

## 9. Suspicious or noteworthy code

1. **`CPowerMock::GetCargaEstimada` (8109): two constants ten times too small.** The voltage-to-charge factors
   are 0.035, 0.055, 0.086, **0.0133**, **0.0208**, 0.29, 0.357 … (a geometric progression with ratio about 1.56
   everywhere else). Between 8.20 and 10.19 V the estimate drops to about 1-2 % instead of 13-21 %, and it is
   not monotonic. Below 2.81 V ("no reading") the factor is 1.0 (full). Nothing in this binary calls slot 5
   (the neighbouring slots 1-4 are used by `CThreadMonitor`), and the mock's voltages are 0. The body only uses
   `IPower` members, so it may be a default of `ipower.h` that other urna applications (or a real driver that
   does not override it) would inherit. Impact on this binary: none.
2. **Public "hardware secrets"** (`CUrnaMock` 8131/8134/8144): the RDV key table (128 × 0x03), the CEPESC table
   (1024 × 0x01) and a 32-byte block (0x02) are constants. The RDV key can be recomputed from public data (u01/u02
   already show this). For CEPESC (BU, fingerprint files) the table is one input next to a random seed and the TSE
   public key, so the effect is unknown. Expected for a simulator; it only matters if simulator output were ever
   taken for real output.
3. **`CUrnaMock::GetTabelaCepesc` (8144) has no bound check.** It `memcpy`s the whole configured vector into
   the caller's 1024-byte buffer, then `memory.fill`s `1024 - n` bytes. A vector longer than 1024 bytes would
   overflow the caller's stack buffer and then trap. Unreachable (nothing fills the vector).
4. **Fixed identity**: model 2020 and identification 87654321 are hard-coded. Every model-dependent path of the web
   build follows the UE2020 branch (keypad "abaixo", fingerprint capture, MR port, MSE log, MT menu mode).
5. **`CStringUtils::ProcuraRegex` (2199) swallows every exception** (`catch (...)` → `{-1,-1}`). A malformed
   pattern, or boost's `logic_error`, becomes "no match". The second field is the end offset, not the length, as
   u20's declaration says. Callers that use it as a length only work because they require a match at 0.
6. **`CRelUtil::FormataCorrespondencia` (2792)**: no length check. A `codigoCarga` shorter than 21 characters
   throws `std::out_of_range`. It is reached from the screens built by the `CTelasVota` blob (7787; one profiler
   sample in the general-election session, from the "O horário de emissão da zerésima passou" screen,
   `ctelasvota.cpp:2382`), so a scenario whose state has a short code could fail while the screens are built. Data that passed `CCarga::ValidaCriacao` (24 digits, u05) cannot trigger it.
7. **Operator release in demonstration mode (3883).** `CInformaAnoDesabilitadoDemo` and
   `CInformaBioDesabilitadaDemo` share the body of "ELEITOR(A) PODE VOTAR". In *modo demonstração* the
   birth-year / fingerprint check is skipped and CONFIRMA releases the voter. This is by design (demo), and dead
   in the web build.
8. **Busy-wait lock in 2860/3853**: the inlined `CUpgradeMutex::lock_shared` loops on
   `condition_variable::wait`, which returns at once without pthreads. A blocked request would spin forever
   (same finding as u19 §9.1; not triggered in the recorded sessions).
9. **Dead TTS parameters**: slots 1/2/3/8 of `ITextToSpeech` are never called. `m_p76` is read by nobody, and the
   profile stays `""` (u32 notes that the empty profile replaces "Letícia-F123" on the first request).
   `SetVelocidade` accepts any level without a range check (only called with 2).

## 10. Open questions

* The real meaning of `IUrna` slots 1, 2, 3, 5 (no caller) and of the `IPower` status bits 0x800/0x2000 and of
  the status fields +4/+6/+8/+10/+14. The names given are guesses.
* Whether `IPower` slots 0-6 are mock overrides or `ipower.h` defaults (this decides whether finding 9.1 can
  reach the real urna).
* The purpose of `m_p76` / `m_p88`. (The TTS cache key format is known: `"{}:{}:{}:{}:{}"` @1147, func 5758,
  already in u02's `MontaChave`.)
* Whether 2763/2677/2199 are members of `api::CStringUtils` or free functions of another `api/util` file (no srcloc).

## 11. Fidelity review (adversarial pass)

Compared line by line with `q.py f`/`wat`: all 15 mock functions (8071-8168, plus the inlined constructors in
8302), the 9 `ITextToSpeech` slots, 10815, 11582, 2199, 2792, 2743, 3883, 2895, 3889, 2779, 3677, 3058, 2244,
1005, 1562 (and its 8 wrappers/slots), 2860, 3853, 3903, 1565, 1566, 3940, 2904, 3628, 2288, 1535, 2746, 3620,
2723, 1936, 2677, 2763, and the library bodies 590, 854, 1144, 1896, 2214, 3266, 3547. The speech-rate run of
§2.3 was repeated (same WAV sizes, byte for byte). Corrections made:

* 1565 is not only `~CDataTextFmt` D0: the ICF bodies 2777/3681/5516 that call it are the D0 of nine
  `IFormField` leaf classes (CBeepFieldMT … CNewLineFieldPaper), passing the `IFormFieldBase<MEDIA>` vtable.
* `votaInit` posts message 9 only when `audioEleitorHabilitado` is set, and before message 1.
* 2677 takes `texto` by `const&` and copies it itself (it calls `__init_copy_ctor_external`), not by value.
* 2199 is observed only through `CTradutorFrase::TraduzLabel`, never through `GetVersionNumber`.
* The TTS cache-key format is `"{}:{}:{}:{}:{}"` (@1147); u02 already had it, with the same argument order.
* `IUrna` has 14 call sites in 13 functions (CEscolheOpcao has two), not 13 sites.
* `cpowermock.h` relied on an `IPower::m_status` member and slot names that u18's `ipower.h` does not have
  (now flagged in both places); the 3883 fragment used `EInputResult::CONFIRMA` instead of u15's `Confirma`;
  the hash fragment missed error 5102 (`CHashDiretorio: nome inválido.`, `chashdiretorio.cpp:41`).
* Outside this unit, `docs/libraries/rhvoice.md` (line 179) and the curated note of 10815 in
  `analysis/names.override.json` still say `std::cerr`; the stream is `std::clog` (§2.3).
