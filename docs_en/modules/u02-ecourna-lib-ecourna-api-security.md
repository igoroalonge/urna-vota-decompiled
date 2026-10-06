# u02 — `ecourna-lib/ecourna/api/security` (`chkdfseed.cpp`) and the VOTA start-up function 7787

Unit u02 was built around one source file, `ecourna-lib/ecourna/api/security/chkdfseed.cpp`, and has
160 wasm functions (63 of them ran during the recorded votes). Only two of them, the destructors 11497/11498,
really come from that file. Its constructor and `GetSeed()` are inlined elsewhere. The unit builder attached the rest because of one
misnamed function:

* **wasm func 7787** is 148 912 bytes and 65 253 instructions, the largest application function of the module. The tools
  first called it `ecourna::api::security::CHKDFSeed::GetSeed` because the first `std::source_location`
  record in its body is `chkdfseed.cpp:47`. It is actually the **start-up routine of the voter side of
  VOTA**. `votaInit` calls it once (`invoke_v`, table slot 39). LTO inlined about twenty functions of
  other files into it, and `CHKDFSeed::GetSeed` is one of them. Its name is not in the binary. I call it
  `vota::CInformacaoEleitor::Inicializar()` (name inferred, see §3.1), and the tools now show that name.
* Almost every other function of the unit is **called only by 7787**. Examples are the voter-screen builders of
  `ctelasvota.cpp`, the TTS cache, container instantiations, and singletons and destructors of the
  `comum` data sets. Neighbour voting in `tools/wasmmap/units.py` assigned them to `chkdfseed.cpp` as
  well. Some vota states arrived the same way through shared callees.

What the unit therefore contains:

| part | functions | reconstruction |
|---|---|---|
| the real `chkdfseed.cpp`: HKDF-SHA512 seed generator `CHKDFSeed` | 11497, 11498 + inlined ctor/`GetSeed` | `src/ecourna/api/security/chkdfseed.{hpp,cpp}` |
| `crdv.cpp`: RDV cipher derivation (`GetCifradorCryptoTable`, `CRdv::CRdv`), only inlined in 7787 | inlined | `src/uenux2/src/app/comum/dados/crdv.cpp` |
| the start-up function 7787 and the `CInformacaoEleitor` members inlined into it | 7787 | `src/uenux2/src/app/vota/eleitor/comum/cinformacaoeleitor.{h,cpp}` |
| out-of-line pieces of other units' files (screens, GUI builder, TTS cache, data sets, states) | 145 | `*.u02.cpp` **fragments** next to the owner's file (see §10) |
| libc++ instantiations (`__tree::destroy`, `std::sort<uebyte*>`, hash-map and vector helpers) | ~45 | listed only (mapping table) |

`crdv.cpp` and `cinformacaoeleitor.cpp` have no wasm function of their own. Their only surviving code is inlined
into 7787, so this unit owns them. A `*.u02.cpp` file holds reconstructions of functions whose original
file belongs to another unit. The owner should merge it; u02 did not edit the owner's files.

---

## 1. Place in the voting process (glossary)

* **Urna** is the voting machine. VOTA runs two sides: the **eleitor** (voter terminal: screens, keys,
  audio) and the **operador/mesário** (poll-worker terminal: identification, enabling the voter).
* **Dados estáticos** are the election data loaded from `/dsk/fi/estatico` (scenario files such as
  `t02411ac00001-ca.dat`). **Dados dinâmicos** are the files the urna writes in
  `/dsk/{fi,fe}/dinamico/trab<turno>`: `vota.bin` (state), `rdv.dat`, `uenux.db`, and signatures `*.vsu`.
  `fi` is the internal flash and `fe` the external one (the memory card).
* **RDV**, *Registro Digital do Voto* (digital record of the vote), is the shuffled list of every vote,
  stored encrypted in `rdv.dat`. The BU (*boletim de urna*, the per-section result) is computed from it.
* **Zerésima** is the report printed before voting, proving that every counter is zero.
* **MR / MI**: *mídia de resultado* (result memory card) / *memória interna* (the text "Gravando o banco de dados na MI").
* **EstadoVota** is the VOTA state machine persisted in `vota.bin` (§3.2).

7787 runs once, during `votaInit`, before any screen is shown. It restores the persisted state, validates the
software package, opens the SQLite database, loads every static data set into its singleton, checks
their referential integrity, builds every voter screen, and pre-caches the accessibility audio (a no-op in the
web build, see §3.3 step 10).

---

## 2. `chkdfseed.cpp` and the RDV key

### 2.1 `ecourna::api::security::CHKDFSeed`

RTTI `class` (no base), vtable @1560096 = `[0] ~CHKDFSeed()` (11498), `[1]` deleting dtor (11497).
40 bytes: `vptr, vector<uebyte> m_salt (+4), m_chave (+16), m_info (+28)`.

`GetSeed()` is inlined into 7787 (dcmp 15947-16260). It returns 128 bytes computed by OpenSSL's HKDF:

| source line | call (table slot) | error code (`ESecurityError`, range 1325..1725) |
|---|---|---|
| 47 | `EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF=1036, NULL)` (6218) | 1397 |
| 50 | `EVP_PKEY_derive_init(ctx)` (6219 → func 7373, whose error strings say `EVP_PKEY_derive_init_ex`: OpenSSL 3's `derive_init(ctx)` is `derive_init_ex(ctx, NULL)` with the NULL constant-propagated) | 1398 |
| 53 | `EVP_PKEY_CTX_set_hkdf_md(ctx, EVP_sha512())` (6221; `EVP_sha512()` folded to 1646408) | 1399 |
| 56 | `EVP_PKEY_CTX_set1_hkdf_salt(ctx, m_salt)` (6222) | 1400 |
| 59 | `EVP_PKEY_CTX_set1_hkdf_key(ctx, m_chave)` (6223 — the `.dcmp` prints it as `622 /*…*/3`, a mis-parse) | 1401 |
| 62 | `EVP_PKEY_CTX_add1_hkdf_info(ctx, m_info)` (6224) | 1402 |
| 67 | `EVP_PKEY_derive(ctx, out, &len=128)` (6225) | 1403 |
| — | `EVP_PKEY_CTX_free(ctx)` (6226), **success path only** | |

Every error throws `CBaseError<ESecurityError>` with the message "Falha ao gerar as sementes criptográficas HKDF."
("failed to generate the HKDF cryptographic seeds").

### 2.2 Its only user: `comum::(anonymous)::GetCifradorCryptoTable(cargos)` (crdv.cpp:60/78)

`CRdvVota::CreateInst()` → `CRdvVota()` → `CRdv(make_unique<CRdvPosicionadorVota>(), cargos,
CConfiguracaoEleicao +164)` → `m_cifrador = GetCifradorCryptoTable(cargos)`, all inlined:

1. `cargos` is the **sorted** list of office codes (byte 0 of every 140-byte `CCargo`, sorted with
   `std::sort<uebyte*>`, funcs 4811–4820). Municipal scenario: `{11 Prefeito, 13 Vereador}`.
2. `tabela[128]` = `CPolySingleton<api::IUrna>::instance()` vtable slot 6 (line 60). **In the simulator IUrna is
   `api::teste::CUrnaMock`. Its slot 6 (func 8131) is `memset(tabela, 3, 128)`.**
3. `hash = SHA-512(cargos)` (CSha512 = 2684, Update 3519, Finish 3518).
4. `indice[i] = hash[32+i] ^ ((hash[i] & 0xC0) << 2)` for i = 0..31 (`vector<uint16_t>`, `.at()`),
   `chave[i] = tabela[indice[i] & 0x7F]`. Then `tabela` is zeroed.
5. `CHKDFSeed(salt = hash, chave, info = "RDV").GetSeed()` → 128 bytes.
6. `CPolySingleton<ISymmetricCipherFactory>::instance()` slot 3 (`CSymmetricCipherFactory::vf3`, 9489, line 78)
   with **key = seed[0..32) (AES-256), IV = seed[32..48)**. The result is a `CBlockCipher<CAesCipher, CTrng>` in CBC mode.
7. The `CRdv` constructor then checks the party-number length (byte +164): 0 → `EUeRdvError 4650` "Número de dígitos
   do partido nulo" (line 90), ≥ 6 → 4651 "Número de dígitos do partido ({}) supera o limite (5)" (line 93).

**Runtime check.** A pure-Python re-implementation (hmac/hashlib) of steps 1–6 of this reconstruction, with
`cargos = [11, 13]` and a table of `0x03`, gives key `c4f9eff4…495a4952` and IV `568d9298776ea63bc8a58367190a4d51`.
These are the same values u01 logged from `EVP_EncryptInit_ex` at runtime. They decrypt
`analysis/runtime/memfs-after-init/dsk/fi/dinamico/trab1/rdv.dat` (80 bytes; identical in `fe`):

```
openssl enc -d -aes-256-cbc -nopad -K c4f9eff475648bef6767d69e414babb26429a0e0f4aa4cc5227d2aa6495a4952 \
            -iv 568d9298776ea63bc8a58367190a4d51 -in rdv.dat
30 38 | 02 02 09 6a | 0a 01 03 | 30 0f (30 06 02 01 01 02 01 01) 02 02 03 f3 02 01 01 | 30 00 |
a1 1c 30 1a 02 02 09 6b 30 14 (30 08 81 01 0b 02 01 01 30 00) (30 08 81 01 0d 02 01 01 30 00)
06 06 06 06 06 06 | 10 × 16
```

This is `ModuloRegistroDigitalVoto.EntidadeRegistroDigitalVoto`: pleito 2410, fase 3 (treinamento),
identification {município 1, zona 1}, local 1011, seção 1, empty `historicoCodigosCarga`, `eleicoesVota`
{idEleicao 2411, offices 11 and 13 with `quantidadeEscolhas` 1 and no votes}. It is followed by an application PKCS#7 pad
(6 × 06) and the EVP pad (16 × 10). The post-vote snapshot has the same bytes, because the web build does not persist votes to
the RDV (u05 §3.3, u07 §4.3).

Security notes (see "suspicious"):

* In the simulator anyone can decrypt `rdv.dat`: the key depends only on the public office list and the
  constant mock table. This is expected for a simulator.
* The **IV is deterministic**: it is fixed for a given (table, office list). Every rewrite of `rdv.dat` on a real urna
  that runs the same code uses the same key and IV in CBC mode.
* The XOR in step 4 has no effect: `(x & 0xC0) << 2` only touches bits 8–9, and `& 0x7F` then clears them. Bit 7 of every
  `hash[32+i]` is also dropped. The 10-bit index suggests a 1024-entry table. `IUrna` slot 4 does fill 1024 bytes
  (`CUrnaMock::vf4`), but this code reads the 128-byte table.
* `EVP_PKEY_CTX` leaks on every error path of `GetSeed`.

---

## 3. wasm func 7787 — `CInformacaoEleitor::Inicializar()` (name inferred)

### 3.1 Why this name

`vota::CInformacaoEleitor` is a 12-byte singleton (`GetInst` = wasm 509, pointer @1833284). Its layout is
`+0 dadosEstaticosCarregados`, `+1 dadosDinamicosCarregados`, `+4 m_modoAudio` (2 = off; see u07's `cinformacaoeleitor.u07.cpp`),
`+8 persistenciaInicializada`, `+9/+10` flags. 7787 reads and sets those flags. The function contains
the only `std::source_location` of `cinformacaoeleitor.cpp` (line 181, inside
`CarregarDadosEstaticos()`). `votaInit` calls it right before `CInformacaoEleitor::GetInst()` + wasm 6737
(`GerarDadosDinamicos`, also called by `vota::CGeraDadosDinamicos::StartState`). 6734 (tools:
`CEleitores::CompleteLoad`) takes the same object as `this` and tests flag +1. That makes it
`CarregarDadosDinamicos()`. 7787 takes no parameters, so it is a static member or a free function. The
name `Inicializar` is a guess.

### 3.2 `EstadoVota` in C++

The C++ enum of `CEstadoGeralVota` is `'1' + ASN.1 value`, as `CConversorEstadoGeralVota::Converte/DesconverteEstadoVota`
(11390/11391) show: INICIAL `'1'`, GERABASEDINAMICA `'2'`, AGUARDAHORAZERESIMA `'3'`, GERARZE `'4'`,
ZERESIMAGERADA `'5'`, ZERESIMAIMPRESSA `'6'`, REGISTROMESARIOINICIAL `'7'`, VOTAR `'8'`, FIMAQUISICAOVOTOS
`'9'`, REGISTROMESARIOFINAL `':'`, GERARBU `';'`, GERARRELATORIOS `'<'`, IMPRIMIRBU `'='`, GRAVARRESULTADOS `'>'`,
COPIARESULTADOSMR `'?'`, ENCERRADA `'@'`. The ASN.1 value 16 (`exibealertadesligamento`) throws "Estado não deve ser
usado: {}".

### 3.3 Step by step (dcmp line ranges in `decompiled/app-api/ecourna-lib/ecourna/api/security/chkdfseed.cpp.dcmp`)

| # | dcmp | what happens (inlined function → owner) |
|---|---|---|
| 1 | 6674 | global @1577208 = `CAppInfo::GetGeral()` +48, the **fase** of the urna (`CEstadoGeral` +48, see u06/u20; the same field starts the file-name identification 5728 and feeds the report separator 2785). `CFormBuilder`'s status header (5564) shows '2' as "SIMULADO" and '3' as "TREINAMENTO" |
| 2 | 6675–7132 | `CAppInfo::CarregaVotaInternoEmCache()`: `vota.bin` of the internal flash (`CServicoEstadoGeralVota`) → `CFileASN::ReadFromFile<EstadoGeralVota>` (5 MiB limit) → `CConversorEstadoGeralVota` → per-turn cache |
| 3 | 7133–7200 | `estado = GetVota(TURNO_ATUAL).estadoVota`. If INICIAL, it becomes GERABASEDINAMICA and is saved (wasm 491, "Gravando o estado da urna"). If GERABASEDINAMICA (an interrupted generation), `ApagarDadosDinamicos()` removes `rdv.dat` and `uenux.db` in both flashes (1551/1701/2826/5762) |
| 4 | 7202–7215 | `CPacoteArquivos::ValidarChaveEAplicacaoValida(GetPathTrab(interna))` and `(externa)` |
| 5 | 7216–7607 | `InicializarPersistencia()` (once, flag +8): opens `trab/uenux.db` and registers `CComparecimentoMesarioDAO` and `CJustificadorDAO` in `CDAORepositorio` (map @1909964 keyed by `typeid(I).name()`). Each DAO runs `CREATE TABLE IF NOT EXISTS` (below). Then @1838488 = 1 and `CSincronizaVota::SincronizaBancoDados()` (4662: inlined `VerificaUrnaDesligando()`, then 4657: sign `uenux.db`, copy it to the external flash, then the MV copy 4682) |
| 6 | 7608–18393 | if `estado` ∈ '1'..'@': `CarregarDadosEstaticos()` (once, flag +0), §3.4 |
| 7 | 18394–18395 | still inside step 6's range test (the `estado - '1' > 15` branch jumps past it too): if `estado` ≥ AGUARDAHORAZERESIMA, `CarregarDadosDinamicos()` (6734). An out-of-range state loads neither static nor dynamic data |
| 8 | 18396–25754 | `CTelasVota::CreateInst()` (ctelasvota.cpp:3435/3502): builds every voter screen; 252-byte object @1833396 |
| 9 | 25755–25776 | `CInfoMTLCD::CreateInst()` (cinfomtlcd.cpp:33): 64-byte object @1838572 |
| 10 | 25777–25928 | `CInstrucaoVotacaoAcessibilidade::GetInst().CacheInstructionAudio()`: for each of the 5 speech rates (60–140 %), `CacheMessage` (921) the names of keys 0–9, BRANCO, CORRIGE, CONFIRMA, and the instruction text into the permanent TTS cache. A '{' in the text throws 9315 "Não é permitido fazer cache de mensagem dinâmica". **In the web build this synthesises nothing:** the `ITextToSpeech` singleton at this point is the `simulador::CWasmNullTextToSpeech` that `votaInit` pushes first; `votaInit` creates and pushes `CRHVoiceTextToSpeech` only after 7787 returns (after 509/6737/11733). The null object's slot 11 (9436) returns an empty `shared_ptr`, so the 70 entries are null WAVs in the null object's cache, and the RHVoice object starts with an empty cache. Runtime: `votaInit` takes 76 ms with `--audio` and 74 ms without (headless, municipal-t1) |

SQL executed in step 5 (strings @435498 and @435360):

```sql
CREATE TABLE IF NOT EXISTS comparecimento_mesario (
  numero_titulo BIGINT NOT NULL, tipo_identificador INTEGER CHECK(tipo_identificador IN (1,2,3)) NOT NULL,
  periodo_presente INTEGER NOT NULL, pertence_secao INTEGER NOT NULL, estado_biometria INTEGER NOT NULL,
  dedo_habilitacao INTEGER NOT NULL, data_hora_registro DATETIME NOT NULL, id_arquivo INTEGER,
  UNIQUE (numero_titulo, periodo_presente) )
CREATE TABLE IF NOT EXISTS registro_justificativa (
  numero_titulo BIGINT PRIMARY KEY UNIQUE NOT NULL, ano_nascimento SMALLINT )
```

### 3.4 `CarregarDadosEstaticos()`: static data sets, in load order

| dcmp | data set (owner) | file(s) / type | singleton |
|---|---|---|---|
| 7613–8090 | `CAppInfo::CarregaGapInternoEmCache()` | `gap.bin` → `EstadoGeralGap` | CAppInfo cache |
| 8095 | `CLocal` (401 + 5740) | `*-lo.dat` → `ModuloLocal::Local` | @1838900 |
| 8096–10270 | `CConfiguracaoEleicao::CreateInst(eg, secoes, INTERNA)`: ProcessoEleitoral, ParametrizacaoUrna (`t02400ac-pu`, `t00000br-pu`), ConfiguracaoMunicipios, `CPleito` (3713), `CEleicaoDataHora` ("Datas inválidas"), then `CCargos::CreateInst` and `CRespostas::CreateInst` | several `ModuloProcessoEleitoral`/`ParametrizacaoUrna`/`ConfiguracaoMunicipios` files | @1838752, @1838720, @1838984 |
| 10271–10978 | `CHV::CreateInst` (daylight-saving / time zone of the municipality) | `ComplementosMunicipios` | @1838872 |
| 10979–11760 | `CPartidos` (819) | `*-pa.dat` per abrangência (names from 2811) → `EntidadePartidos` | @1838928 |
| 11761–13091 | `CFederacoes::Load` | `*-fe.*` → `EntidadeFederacoes` | @1838748 |
| 13092–14028 | `CFotos::Load` | `*-fo.dat` (partial ASN.1 reader) | @1838844 |
| 14029–15643 | `CCandidaturas` (`LoadFromFile`) | `*-ca.dat` + `CabecalhoPacote` | @1838692 |
| 15644–16951 | `CRdvVota::CreateInst` → `CRdv` → **§2.2** | — | @1838956 |
| 16953–17153 | if `CConfiguracaoEleicao` +665 (biometric urna): `CPolySingleton<IFingerPrepare>` slot 2 (line 181). In the web build this is a no-op (`CFingerPrepareSimulador`) | — | — |
| 17154–18039 | `CEleitores::StaticLoad` (celeitores.cpp:395/407) | `*-el.*`, `*-tte.dat` (names 3745/5727/3772) | @1838792 |
| 18040–18392 | four integrity checks, each followed by `CIntegridadeReferencial::Lanca` (2263): office of each candidacy (5747), party of each candidacy, photo of candidate and alternates (5746), parties of each federation. Then flag +0 = true | — | — |

---

## 4. Voter screens built by the inlined `CTelasVota` constructor

The out-of-line screen builders of this unit (they belong to `ctelasvota.cpp`, owner u07) all follow one pattern:

```cpp
api::CFormBuilder builder;                                  // zero-initialised by the caller
adicionaAnimacao(tipo, builder, conferencia);               // wasm 1593 (only for vote/review screens with a GIF):
                                                            //   votoLegenda/votoBranco/votoNulo.gif
adicionaBase<Tela>(builder, __func__, cargo, ...);          // srcloc-named helpers 6602..6678 (u07)
adicionaInstrucoesConfirmaCorrige(builder, "CONFIRMAR este voto", largura)   // 1102, or
adicionaRodapeConfiraSeuVoto(builder, largura)                               // 1190 (review screens)
builder.AddControlInput() | AddNumberInput(...)            // 901 / 2383
return CriaFormInterativo(builder, make_shared<CPreShowProgressBar>(), "tela<Nome>");   // 554
```

`__func__` is visible because the helpers receive it for their error messages. The string
"CriaTelaVotoNuloCandidato" sits next to the form name "telaVotoNuloCandidato", so both names can be recovered
with high confidence.

| func | screen | form name |
|---|---|---|
| 1591 / 1767 | null vote for a candidate office / its review | `telaVotoNuloCandidato` / `telaConferenciaVotoNuloCandidato` |
| 3063 / 3067 | blank vote / review | `telaVotoBrancoCandidato` / `telaConferenciaVotoBrancoCandidato` |
| 6639 / 6681 | complete candidate screen without photo (Com0) / review | `telaCompletaCandidatoCom0` / `telaConferenciaCandidatoCom0` |
| 6626 / 6616 | null vote in a *consulta* (referendum) / review | `telaVotoNuloConsulta` / `telaConferenciaVotoNuloConsulta` |
| 6601 | "FIM" (end of vote) | `telaFim` |
| 2380, 6557 | centred message ('&' = new line); "CONTINUAÇÃO DA VOTAÇÃO&IDENTIFIQUE O ELEITOR" | `telaNeutra` |
| 2376 | message with optional CONFIRMA/CORRIGE labels | `telaTextoConfirmaCorrige` |
| 6592 | confirm printing the zerésima | `telaConfirmaImpressaoZeresima` |
| 6595 | "Esta urna eletrônica só funcionará a partir de HHhMM do dia DD/MM/YYYY." | `telaAntesHorarioZeresima` |
| 6599 | "Mais informações" menu: Estado da urna / Lista de eleitores / Versões de pacotes / Parâmetros de urna ({vias}/{max}), Visualizar candidatos; Processo eleitoral, Pleito, "Local com biometria: Sim/Não" | `telaMaisInformacoes` |

Helpers: 4161 bottom band "Município: 00001 - NAME     Zona: 0001     Seção: 0001", with the labels `<MCSN>/<ZCSN>/<SCSN>`
translated by `CTradutorFrase`. For a contingency urna (no seção: `md::CLocal` +120 clear, +136 set) the text is
"Município: 00001 - NAME     Zona: 0001     CONTINGÊNCIA" (constant @335107); with neither flag the band has no text. 4160 is the zerésima header (`<ZCSA>/<SCSA>`, aggregated sections). 2384 is the number-entry block.
The GUI builder primitives (202 AddText, 3680 AddRect, 2245 AddFill, 3678/2243 labelled keys, 6689 data
image, 2024 `IInputField` ctor, 554/5550 form factories) are in `cformbuilder.u02.cpp`.

---

## 5. Other code pulled into the unit

### 5.1 VOTA states

* **`vota::CAguardaMensagem::ProcessMessage(int)`** (7481, vtable slot 6) is the voter terminal waiting for the poll
  worker. Message 1 → log "Eleitor foi habilitado" ("voter enabled") → `CEleitorVotando` (seen in
  `logd.dat` after the recorded vote). Message 7 → **start of encerramento** (§6). Message 11 → `CIniciodeCiclo`. Message 12 →
  `CInspecionaUrna` ("Por favor, inspecione cabina e urna.", key CONFIRMA = "Continuar"). Message 14 →
  `CMostraTelaContinuaVotacao`.
* `CInspecionaUrna::ProcessInput` (11797): CONFIRMA → `CUrnaInspecionada`, log "Inspeção da urna confirmada",
  message 11 on the priority queue.
* Keyboard test (*teste do teclado*): `CTesteFalhou`/`CEnviarManutencao::ProcessInput` (11809/11812) and
  `CEsperaRetestar::GetInst` (5936, "Por favor, espere {}s para a realização de uma nova tentativa").
* Candidate viewing: `CMenuFiltrarCandidatosPorNumero::ProcessInput` (11891, "Candidato não encontrado!" for 2 s),
  the candidate-list filler of the party/office filters (5953), and `CItemVisualizarCandidatosVota` slot 2 (12244).
* `CRetirarMR::GetInst` (2291) is the "retire/keep the result memory card" screens, including the demonstration-mode screen
  `telaMRModoDemo`.
* Operator counters: `TextoHabilitacao{Biometrica,Biografica,SemBiometria}` (10701/10700/10699) show "n.a." when
  the polling place is not biometric.

State objects are lazy singletons. The simple ones share the merged body 764
(`vota_f764(mutex, &inst, vtable, arg)`), e.g. `CIniciodeCiclo::GetInst` = 4433.

### 5.2 TTS cache (`api::ITextToSpeech`, 921, 5750/5754/5758, 11436 …)

`ITextToSpeech` has two caches: a bounded LRU (`std::list` + `unordered_map<string, list::iterator>` at +4) used by
normal synthesis (vtable slot 0, 11530), and a permanent `unordered_map<string, shared_ptr<CWavFile>>` at +40
filled by `CacheMessage` (921). The key (5758) is `std::vformat("{}:{}:{}:{}:{}", texto, taxa (+84), perfil (+64),
int +80, int64 +88)`; the int at +76 is not part of it. Every speech rate therefore has its own entries. §3.3 step 10
calls `CacheMessage` for 14 texts × 5 rates, but in the web build it runs against `CWasmNullTextToSpeech` (see step 10),
so no synthesis happens at start-up. After `votaInit` the TTS is `CRHVoiceTextToSpeech` (audio on) or a new
`CWasmNullTextToSpeech`.

### 5.3 comum helpers

`CLocal` accessors (1004 GetMunicipio, 1077 GetNomeMunicipio, 1702 GetUF, 5743 GetLocalData, 820
UrnaBiometrica) pass their own name to `CLocal::VerificaLido`. `CNomeArquivo::MontaNome` (3772) builds
`<fase><pleito:05><uf><mun:05><zona:04><secao:04>-<suf>.<ext>`, e.g. `t02400ac0000100010001-tte.dat`.
`CMenuBase` (2285 AdicionaItem, 5913 Monta: "[{}] - {}", "Escolha a sua opção:"). Only items whose slot 2
(`Disponivel`, e.g. 12244) returns true add their id to the accepted options; unavailable items are drawn with
style 5 instead of 2. The report
`CGeradorRelVersaoPacoteDados` slot 3 (11223) prints "Versões de Pacotes" with "Código do processo eleitoral",
"Código do pleito 1/2". `comum::GravaBancoDadosNaMI` (4657) signs `uenux.db` and copies it to the
external flash (CArquivosSavd ids 110/111 = `uenux.db`), then calls 4682 ("Gravando o banco de dados na MV":
copies CArquivosSavd 199 → 201 in the 1st turn, 200 → 202 otherwise).

---

## 6. BU / encerramento: what this unit contributes

The unit does not generate the BU. It has two pieces of the path that leads there:

1. **The trigger.** When the poll worker closes the vote, the operator side sends **message 7** to the voter side.
   `CAguardaMensagem::ProcessMessage(7)` (7481) then does, in this order:
   `GetVota(TURNO_ATUAL).estadoVota = GERARBU (';')` → `comum::SalvaEstadoVota()` (wasm 491: signs and syncs
   `vota.bin` in both flashes; error text "Ocorreu um erro durante a sincronização do estado geral do VOTA.") →
   `CLogVota` "Inicio do Encerramento" → next state `CGeraBU::GetInst()` (6129, lazy @1833496). That state
   first shows "Votação encerrada" and "Preparando dados para encerramento" (`telaVotacaoEncerrada`,
   `telaPreparandoDadosEncerramento`). BU generation itself is in u08 (`cgerabu.cpp`) and u25.
2. **The RDV key** (§2.2). The BU is computed from the RDV (u05: `CRdvVota` slots 0–10). `rdv.dat` is protected by
   AES-256-CBC with the key and IV from `GetCifradorCryptoTable`. On restart, 7787 step 7 reloads it (6734 → `CEncryptedFile::Load`
   → `Desconverte`) and 2543 re-checks the RDV against offices, candidacies, parties and answers.

`CRetirarMR` (2291) is the end of the closing flow on the voter screen: "Retire a mídia de resultado e faça a entrega
conforme as instruções." and "Lacre a tampa do compartimento da mídia de resultado conforme as instruções."

---

## 7. Class relations (RTTI)

```
ecourna::api::security::CHKDFSeed                         (no base; vtable @1560096)
comum::CRdv  <- comum::CRdvVota                            (vtables @1559900 / @1560172)
comum::asn::CConversorEleicoesVota <- CConversorRegistroDigitalVoto<CConversorEleicoesVota>   (@1571224 / @1560304)
comum::IConversorASN<EstadoGeralVota, CEstadoGeralVota> <- comum::asn::CConversorEstadoGeralVota
api::IUrna <- api::teste::CUrnaMock                        (simulator: slot 6 = memset 3, slot 4 = 1024 bytes, slot 5 = 32 × 0x02)
api::IFingerPrepare <- simulador::CFingerPrepareSimulador  (slot 2 = no-op)
api::ITextToSpeech <- api::CRHVoiceTextToSpeech, simulador::CWasmNullTextToSpeech
ecourna::api::security::ISymmetricCipherFactory <- CSymmetricCipherFactory
comum::CAppState (api::CState) <- vota::CAguardaMensagem, CInspecionaUrna, CUrnaInspecionada, CRetirarMR,
     CMostraTelaContinuaVotacao, CIniciodeCiclo, CGeraBU, CEleitorVotando, testeteclado::{CTesteFalhou,
     CEnviarManutencao, CErroTesteTecladoFim, CEsperaRetestar}, CMenuFiltrarCandidatosPorNumero, ...
comum::CMenuBase <- vota::CMenuMaisInformacoesVota;  comum::CItemMenu <- CItemImprimeEstadoUrna(Vota), CItemImprimeListaEleitores(Vota),
     CItemVersoesPacotes(Vota), CItemParametrosUrna(Vota), vota::CItemVisualizarCandidatosVota
comum::dao::IComparecimentoMesarioDAO <- CComparecimentoMesarioDAO;  IJustificadorDAO <- CJustificadorDAO
```

Non-polymorphic singletons that 7787 fills: `vota::CInformacaoEleitor`, `comum::CAppInfo`, `CLocal`, `CConfiguracaoEleicao`,
`CCargos`, `CRespostas`, `CHV`, `CPartidos`, `CFederacoes`, `CFotos`, `CCandidaturas`, `CEleitores`, `vota::CTelasVota`,
`comum::CInfoMTLCD`. Each one has an atexit handler (e.g. 11481, 11495, 11535, 12882, 11551, 11553, 11561).

---

## 8. Web build specifics

* `api::IUrna` is `api::teste::CUrnaMock`. The RDV "crypto table" is 128 × `0x03`, so the RDV key is public (§2.2).
* `IFingerPrepare` is `CFingerPrepareSimulador`, whose `Prepara` is a no-op. The scenario data decides whether the urna is biometric.
* `.vsu` signatures are fake: `votaInit` writes "assinatura simulada para vota_web_wasm", and `CWasmSavd` signs.
  `ValidarChaveEAplicacaoValida` and `AssinarUE` in steps 4–5 therefore work against simulated signatures.
* After step 5, `uenux.db` is copied to `/dsk/fe/dinamico/trab1` (4657, observed at runtime).
* `CSincronismoVotoEleitorWeb` never writes votes. `rdv.dat` keeps the empty RDV written at start-up (§2.2).
* 7787 has no JS import calls and no `emscripten_sleep`. It runs synchronously inside `votaInit`, and any exception goes to
  votaInit's `catch (std::exception&)` ("erro desconhecido em votaInit" for others).

## 9. Wasm / Emscripten observations

* **LTO + one caller = one giant function.** About 20 functions from 15 files (`CTelasVota::CTelasVota` alone is ~7 000 dcmp
  lines) were inlined into 7787. The result is far above the inliner's threshold, so it was not inlined into `votaInit`. The call sits
  in votaInit's `try` block, so it is an `invoke_v` through table slot 39. The tools first named the function after the *first* srcloc
  in its body (it is now curated). Treat any name given to a function above ~20 KB with suspicion.
* **merge-similar-functions** produced several shared bodies with constants as parameters: 6117 (`CriaForm<PRESHOW>`,
  vtables as args), 3942 (copy-and-transform string, transform as table slot), 6035/6036 (the same path helper
  specialised for flash 0/1), 764 (lazy state singletons).
* **The libc++ `__constexpr_memmove` detail.** 3713 copies `n-1` bytes into an `n`-byte vector. libc++ copies
  `(n-1)*sizeof(T) + __datasizeof(T)`, and for an empty class `__datasizeof` is 0. So this is not a bug. It shows
  that the element type (the "situações" of `CPleito`) is an empty class.
* **The shared_mutex emulation inlined in `CPolySingletonList::instance`.** It appears twice in 7787 (dcmp 16304–16470, 16982–17146).
  `condition_variable::wait` is a no-op in this single-threaded build, so the `while (writer || writersWaiting) wait()` loop would spin
  forever if a writer flag were left set.
* **Fonts** are passed as addresses of static descriptors (474880 = 40 px, 474888 = 30, 474896 = 20, 475016 = 35, …).
  Screen coordinates are packed `{x, y}` int16 pairs stored as one int32 (e.g. 26542081 → {1, 405}).

---

## 10. Reconstructed files

| file | contents |
|---|---|
| `src/ecourna/api/security/chkdfseed.hpp`, `.cpp` | `CHKDFSeed` (ctor, dtor 11497/11498, `GetSeed`) — **the unit's own file** |
| `src/uenux2/src/app/comum/dados/crdv.cpp` | `GetCifradorCryptoTable`, `CRdv::CRdv` (orphan file, inlined in 7787) |
| `src/uenux2/src/app/vota/eleitor/comum/cinformacaoeleitor.{h,cpp}` | `CInformacaoEleitor` layout, `Inicializar` (7787), `ApagarDadosDinamicos`, `InicializarPersistencia`, `CarregarDadosEstaticos` |
| `src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp` | fragment for u07: CriaTela* and footer helpers, `~CTelasVota` |
| `src/uenux2/src/api/gui/cformbuilder.u02.cpp` | fragment for u15/u17: builder primitives, form factories, `IInputField` ctor, VOTA form wrappers |
| `src/uenux2/src/api/audio/itexttospeech.u02.cpp` | fragment: TTS permanent/LRU cache |
| `src/uenux2/src/app/comum/dados/{clocal,cconfiguracaoeleicao,cintegridadereferencial,crdvvota,cdadosestaticos}.u02.cpp` | fragments for u03/u04/u05 (data-set helpers, destructors, singletons) |
| `src/uenux2/src/app/comum/{cpath,cmenubase}.u02.cpp`, `comum/nomearquivo/cnomearquivo.u02.cpp`, `comum/relatorios/cgeradorrelversaopacotedados.u02.cpp`, `comum/util/cstringutil.u02.cpp` | comum fragments |
| `src/uenux2/src/api/util/cdatetime.u02.cpp`, `src/ecourna/app/dados/parametrizacaourna/cparametrosurna.u02.cpp` | api/ecourna fragments |
| `src/uenux2/src/app/vota/eleitor/cestadosvota.u02.cpp` | `CAguardaMensagem::ProcessMessage`, `CInspecionaUrna::ProcessInput`, `CRetirarMR::GetInst`, `CacheInstructionAudio` (inlined) |
| `src/uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.u02.cpp`, `.../auxiliares/cmenufiltrarcandidatos.u02.cpp`, `src/uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.u02.cpp`, `src/uenux2/src/app/vota/comum/csincronizavota.u02.cpp` | vota fragments |

## 11. Suspicious / noteworthy (summary; details in the StructuredOutput)

1. **RDV key derivable from public data in the simulator.** The mock `IUrna` table is 128 × `0x03`. This was verified by decrypting `rdv.dat`.
2. **Deterministic IV for RDV encryption.** The key and IV are both fixed per (urna table, office list). This matters for the real urna if it runs the same code.
3. **Dead XOR / discarded index bits** in `GetCifradorCryptoTable`: only 7 of the 10 computed index bits are used.
4. **`EVP_PKEY_CTX` leak** on every `GetSeed` error path.
5. **Double PKCS#7 padding** of `rdv.dat` (application pad + EVP pad). This is harmless but unusual. Decoders must strip both.
6. **Infinite-spin hazard** in the inlined `CPolySingletonList` shared lock (no-op `condition_variable::wait`).
7. `ApagarDadosDinamicos` deletes both copies of `rdv.dat` and `uenux.db` whenever `vota.bin` says GERABASEDINAMICA
   (an interrupted generation of the dynamic base). The rule relies only on the persisted state value.
8. **2-second busy-wait in a key handler** (11891): after an unknown candidate number, `CWait(2000)` + `CWait::Espera`
   (5446: `gettimeofday` + `usleep(200)` loop; Emscripten's `nanosleep` (6265) spins on `emscripten_get_now`) runs inside the
   wasm call that processes the key (`votaPressKey`/`votaTick`). The loop runs only while the byte @1584624 is 1. That byte is
   initialised to 1 in the data segment, no instruction stores to it through its constant address, and it is also read by
   `CApplication::EnterLoop*`, so it is presumably the application-running flag. The browser thread therefore freezes for 2 s.
   Not confirmed at runtime: that the "Candidato não encontrado!" box is replaced before the browser can present it (which
   would mean the user never sees it). Web build only; this is the pre-election "Mais informações → Visualizar candidatos" menu.
9. **Apparently write-only global** @1838488. It is set to 1 after DAO registration, and `i32.const 1838488` appears nowhere else in the module.
10. The start-up pre-caching of 70 TTS utterances (step 10) is **ineffective in the web build**: it runs against
    `CWasmNullTextToSpeech` (RHVoice is registered by `votaInit` only after 7787), so it costs nothing and caches only
    null WAVs in an object that is then replaced. With audio on, the RHVoice engine synthesises key names and the
    instruction on demand later. (An earlier version of this list claimed 70 synchronous syntheses at start-up; the
    code and the 76 ms / 74 ms `votaInit` timings with/without `--audio` contradict it.)

## 12. Complete mapping table (all 160 functions)

`run` = seen by the V8 sampling profiler during the recorded votes. "reconstruction" is the file under `src/` that holds
the code, or "library/inlined helper" for libc++ instantiations that are only listed. "original file" is the source file of
the function (`(unknown)` when no evidence names one; see the fragment headers for inferred paths).

| idx | size | run | reconstructed symbol | original file | reconstruction | conf. |
|---:|---:|:-:|---|---|---|:-:|
| 202 | 347 | * | `api::CFormBuilder::AddText` | uenux2/src/api/gui/cformbuilder.cpp | src/uenux2/src/api/gui/cformbuilder.u02.cpp | medium |
| 401 | 85 | * | `comum::CLocal::GetInst` | uenux2/src/app/comum/dados/clocal.cpp | src/uenux2/src/app/comum/dados/clocal.u02.cpp | medium |
| 436 | 18 |  | `comum::CPath::GetPathTrab` | uenux2/src/app/comum/cpath.cpp | src/uenux2/src/app/comum/cpath.u02.cpp | medium |
| 554 | 418 | * | `api::CriaFormInterativo` | uenux2/src/api/gui/cinteractiveformbuilder.cpp | src/uenux2/src/api/gui/cformbuilder.u02.cpp | medium |
| 576 | 144 | * | `vota::CriaFormInterativoVota` | (unknown) | src/uenux2/src/api/gui/cformbuilder.u02.cpp | medium |
| 820 | 143 |  | `comum::CLocal::UrnaBiometrica` | uenux2/src/app/comum/dados/clocal.cpp | src/uenux2/src/app/comum/dados/clocal.u02.cpp | high |
| 886 | 21 | * | `vota::CriaFormVota` | (unknown) | src/uenux2/src/api/gui/cformbuilder.u02.cpp | medium |
| 901 | 279 | * | `api::CInteractiveFormBuilder::AddControlInput` | uenux2/src/api/gui/cinteractiveformbuilder.cpp | src/uenux2/src/api/gui/cformbuilder.u02.cpp | medium |
| 921 | 584 | * | `api::ITextToSpeech::CacheMessage` | uenux2/src/api/audio/itexttospeech.cpp (path inferred) | src/uenux2/src/api/audio/itexttospeech.u02.cpp | medium |
| 1004 | 122 |  | `comum::CLocal::GetMunicipio` | uenux2/src/app/comum/dados/clocal.cpp | src/uenux2/src/app/comum/dados/clocal.u02.cpp | high |
| 1077 | 122 |  | `comum::CLocal::GetNomeMunicipio` | uenux2/src/app/comum/dados/clocal.cpp | src/uenux2/src/app/comum/dados/clocal.u02.cpp | high |
| 1102 | 1002 | * | `vota::(anonymous namespace)::adicionaInstrucoesConfirmaCorrige` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | medium |
| 1159 | 45 |  | `std::__tree<map<K, map<K2, vector<X>>>>::destroy` | (libc++ instantiation) | library/inlined helper | medium |
| 1160 | 147 |  | `std::__tree<map<TEleicaoID, vector<comum::md::CCargo>>>::destroy` | (libc++ instantiation) | library/inlined helper | medium |
| 1190 | 305 | * | `vota::(anonymous namespace)::adicionaRodapeConfiraSeuVoto` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | medium |
| 1268 | 135 |  | `std::__tree<map<K, vector<X16>>>::destroy` | (libc++ instantiation) | library/inlined helper | medium |
| 1314 | 84 |  | `std::__tree<map<K, shared_ptr<T>>>::destroy` | (libc++ instantiation) | library/inlined helper | medium |
| 1507 | 53 |  | `std::__sort3<uebyte*>` | (libc++ instantiation) | library/inlined helper | medium |
| 1591 | 658 | * | `vota::CTelasVota::CriaTelaVotoNuloCandidato` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | high |
| 1594 | 32 |  | `std::__tree<set<T>>::destroy` | (libc++ instantiation) | library/inlined helper | medium |
| 1701 | 15 | * | `comum::ArquivoRdvExterno` | (unknown) | src/uenux2/src/app/comum/cpath.u02.cpp | medium |
| 1702 | 101 |  | `comum::CLocal::GetUF` | uenux2/src/app/comum/dados/clocal.cpp | src/uenux2/src/app/comum/dados/clocal.u02.cpp | high |
| 1709 | 156 |  | `std::__tree<map<K, comum::md::CCandidatura>>::destroy` | (libc++ instantiation) | library/inlined helper | medium |
| 1767 | 509 | * | `vota::CTelasVota::CriaTelaConferenciaVotoNuloCandidato` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | high |
| 1835 | 72 |  | `std::__tree<map<TCargoID, comum::md::CCargo>>::destroy` | (libc++ instantiation) | library/inlined helper | medium |
| 1929 | 82 | * | `std::__tree<map<uedword, SResposta>>::destroy` | (libc++ instantiation) | library/inlined helper | medium |
| 1932 | 82 |  | `std::__tree<map<TNumeroPartido, CPartido>>::destroy` | (libc++ instantiation) | library/inlined helper | medium |
| 1934 | 107 |  | `std::__tree<map<std::string, CIndiceFoto>>::destroy` | (libc++ instantiation) | library/inlined helper | medium |
| 2024 | 441 | * | `api::IInputField<api::IScreen>::IInputField` | uenux2/src/api/gui/iinputfield.h (path inferred) | src/uenux2/src/api/gui/cformbuilder.u02.cpp | medium |
| 2243 | 68 | * | `api::CFormBuilder::AddLabeledKey` | uenux2/src/api/gui/cformbuilder.cpp | src/uenux2/src/api/gui/cformbuilder.u02.cpp | low |
| 2245 | 244 |  | `api::CFormBuilder::AddFill` | uenux2/src/api/gui/cformbuilder.cpp | src/uenux2/src/api/gui/cformbuilder.u02.cpp | medium |
| 2262 | 11 |  | `comum::CLocal::CLocal` | uenux2/src/app/comum/dados/clocal.cpp | src/uenux2/src/app/comum/dados/clocal.u02.cpp | low |
| 2267 | 1092 |  | `ecourna::app::dados::CParametrosUrna::~CParametrosUrna` | ecourna-lib/ecourna/app/dados/parametrizacaourna/cparametrosurna.cpp (path inferred) | src/ecourna/app/dados/parametrizacaourna/cparametrosurna.u02.cpp | medium |
| 2285 | 534 | * | `comum::CMenuBase::AdicionaItem` | uenux2/src/app/comum/cmenubase.cpp (path inferred) | src/uenux2/src/app/comum/cmenubase.u02.cpp | medium |
| 2291 | 3426 |  | `vota::CRetirarMR::GetInst` | uenux2/src/app/vota/eleitor/fimvotacao/cretirarmr.cpp | src/uenux2/src/app/vota/eleitor/cestadosvota.u02.cpp | medium |
| 2376 | 935 | * | `vota::CTelasVota::CriaTelaTextoConfirmaCorrige` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | medium |
| 2380 | 566 | * | `vota::CTelasVota::CriaTelaNeutra` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | medium |
| 2383 | 297 | * | `api::CInteractiveFormBuilder::AddNumberInput` | uenux2/src/api/gui/cinteractiveformbuilder.cpp | src/uenux2/src/api/gui/cformbuilder.u02.cpp | medium |
| 2384 | 784 | * | `vota::(anonymous namespace)::adicionaCampoNumero` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | low |
| 2543 | 104 |  | `comum::CIntegridadeReferencial::VerificaRdv` | uenux2/src/app/comum/dados/cintegridadereferencial.cpp | src/uenux2/src/app/comum/dados/cintegridadereferencial.u02.cpp | medium |
| 2579 | 79 |  | `std::__sort3<uebyte*>` | (libc++ instantiation) | library/inlined helper | medium |
| 2685 | 17 | * | `api::FormataAAAAMMDDhhmmss` | uenux2/src/api/util/cdatetime.cpp | src/uenux2/src/api/util/cdatetime.u02.cpp | medium |
| 2804 | 61 |  | `std::unique_ptr<__hash_node<string, list::iterator>, __hash_node_destructor>::reset` | (libc++ instantiation) | library/inlined helper | medium |
| 2808 | 187 | * | `std::unordered_map<std::string, ...>::find` | (libc++ instantiation) | library/inlined helper | medium |
| 2811 | 678 | * | `comum::CNomeArquivo::NomesPorAbrangencia` | uenux2/src/app/comum/nomearquivo/cnomearquivo.cpp | src/uenux2/src/app/comum/nomearquivo/cnomearquivo.u02.cpp | medium |
| 2819 | 84 |  | `comum::CFotos::GetInst` | uenux2/src/app/comum/dados/cfotos.cpp | src/uenux2/src/app/comum/dados/cdadosestaticos.u02.cpp | medium |
| 2831 | 265 |  | `std::__uninitialized_allocator_move_if_noexcept<ecourna::app::dados::CFederacao>` | (libc++ instantiation) | library/inlined helper | medium |
| 2832 | 148 |  | `comum::CFederacoes::GetInst` | uenux2/src/app/comum/dados/cfederacoes.cpp | src/uenux2/src/app/comum/dados/cdadosestaticos.u02.cpp | high |
| 2834 | 918 | * | `comum::CCargos::GetMapaCargos` | uenux2/src/app/comum/dados/ccargos.cpp | src/uenux2/src/app/comum/dados/cdadosestaticos.u02.cpp | low |
| 2839 | 57 |  | `std::__tree<map<int, std::string>>::destroy` | (libc++ instantiation) | library/inlined helper | medium |
| 2870 | 323 |  | `std::vector<comum::md::CDadosCandidato>::vector(first, last)` | (libc++ instantiation) | library/inlined helper | medium |
| 3063 | 617 | * | `vota::CTelasVota::CriaTelaVotoBrancoCandidato` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | high |
| 3067 | 507 | * | `vota::CTelasVota::CriaTelaConferenciaVotoBrancoCandidato` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | high |
| 3678 | 564 | * | `api::CFormBuilder::AddLabeledKey` | uenux2/src/api/gui/cformbuilder.cpp | src/uenux2/src/api/gui/cformbuilder.u02.cpp | medium |
| 3680 | 200 |  | `api::CFormBuilder::AddRect` | uenux2/src/api/gui/cformbuilder.cpp | src/uenux2/src/api/gui/cformbuilder.u02.cpp | medium |
| 3713 | 437 | * | `comum::(anonymous namespace)::CriaPleito` | uenux2/src/app/comum/dados/cconfiguracaoeleicao.cpp | src/uenux2/src/app/comum/dados/cconfiguracaoeleicao.u02.cpp | low |
| 3722 | 61 |  | `std::unique_ptr<__hash_node<string, shared_ptr<CWavFile>>, __hash_node_destructor>::reset` | (libc++ instantiation) | library/inlined helper | medium |
| 3729 | 123 | * | `std::hash<std::string>::operator()` | (libc++ instantiation) | library/inlined helper | medium |
| 3732 | 767 |  | `std::unordered_map<std::string, list::iterator>::operator[]` | (libc++ instantiation) | library/inlined helper | medium |
| 3745 | 175 | * | `comum::CEleitores::NomeArquivoEleitores` | uenux2/src/app/comum/dados/celeitores.cpp | src/uenux2/src/app/comum/dados/cdadosestaticos.u02.cpp | low |
| 3754 | 42 | * | `comum::md::CHorarioVerao::CHorarioVerao(const CHorarioVerao&)` | (unknown) | src/uenux2/src/app/comum/dados/cconfiguracaoeleicao.u02.cpp | low |
| 3772 | 1233 | * | `comum::CNomeArquivo::MontaNome` | uenux2/src/app/comum/nomearquivo/cnomearquivo.cpp | src/uenux2/src/app/comum/nomearquivo/cnomearquivo.u02.cpp | medium |
| 3776 | 278 |  | `std::vector<std::unique_ptr<T>>::push_back(unique_ptr&&)` | (libc++ instantiation) | library/inlined helper | medium |
| 3777 | 2274 | * | `ecourna::app::dados::CParametrosUrna::CParametrosUrna(const CParametrosUrna&)` | ecourna-lib/ecourna/app/dados/parametrizacaourna/cparametrosurna.cpp (path inferred) | src/ecourna/app/dados/parametrizacaourna/cparametrosurna.u02.cpp | medium |
| 3778 | 361 |  | `std::set<uebyte>::insert(first, last)` | (libc++ instantiation) | library/inlined helper | medium |
| 3779 | 173 |  | `std::__split_buffer<ecourna::app::dados::CFederacao>::~__split_buffer` | (libc++ instantiation) | library/inlined helper | medium |
| 3780 | 235 |  | `ecourna::app::dados::CFederacao::CFederacao(const CFederacao&)` | ecourna-lib/ecourna/app/dados/federacoes/cfederacao.cpp | src/uenux2/src/app/comum/dados/cdadosestaticos.u02.cpp | low |
| 3781 | 108 |  | `std::__tree<map<K, CFederacao>>::destroy` | (libc++ instantiation) | library/inlined helper | medium |
| 3785 | 85 |  | `std::__tree<map<K, {string, vector}>>::destroy` | (libc++ instantiation) | library/inlined helper | medium |
| 3942 | 123 | * | `comum::CopiaTransformada` | (unknown) | src/uenux2/src/app/comum/util/cstringutil.u02.cpp | low |
| 4137 | 45 |  | `std::__tree<map<TCargoID, CTelasCargo>>::destroy` | (libc++ instantiation) | library/inlined helper | medium |
| 4160 | 3713 | * | `vota::(anonymous namespace)::adicionaCabecalhoZeresima` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | low |
| 4161 | 2019 | * | `vota::(anonymous namespace)::adicionaRodapeLocal` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | medium |
| 4433 | 22 |  | `vota::CIniciodeCiclo::GetInst` | uenux2/src/app/vota/eleitor/ciniciodeciclo.cpp | src/uenux2/src/app/vota/eleitor/cestadosvota.u02.cpp | medium |
| 4657 | 1048 | * | `comum::GravaBancoDadosNaMI` | (unknown) | src/uenux2/src/app/vota/comum/csincronizavota.u02.cpp | low |
| 4662 | 38 | * | `vota::CSincronizaVota::SincronizaBancoDados` | uenux2/src/app/vota/comum/csincronizavota.cpp | src/uenux2/src/app/vota/comum/csincronizavota.u02.cpp | low |
| 4811 | 268 |  | `std::__floyd_sift_down<uebyte*> (heap sort fallback)` | (libc++ instantiation) | library/inlined helper | medium |
| 4812 | 101 |  | `std::__swap_bitmap_pos<uebyte*>` | (libc++ instantiation) | library/inlined helper | medium |
| 4817 | 287 |  | `std::__insertion_sort_incomplete<uebyte*>` | (libc++ instantiation) | library/inlined helper | medium |
| 4818 | 151 |  | `std::__sort5<uebyte*>` | (libc++ instantiation) | library/inlined helper | medium |
| 4819 | 198 |  | `std::__sort4<uebyte*>` | (libc++ instantiation) | library/inlined helper | medium |
| 4820 | 2432 | * | `std::__introsort<uebyte*>` | (libc++ instantiation) | library/inlined helper | medium |
| 5501 | 58 |  | `api::CRectField::CRectField` | (unknown) | src/uenux2/src/api/gui/cformbuilder.u02.cpp | medium |
| 5548 | 16 | * | `api::IForm<api::IScreen>::IForm` | uenux2/src/api/gui/iform.h | src/uenux2/src/api/gui/cformbuilder.u02.cpp | medium |
| 5550 | 296 | * | `api::CriaForm` | uenux2/src/api/gui/cinteractiveformbuilder.cpp | src/uenux2/src/api/gui/cformbuilder.u02.cpp | medium |
| 5649 | 257 | * | `comum::(anonymous namespace)::EleicoesDoMunicipio` (filters `CEleicaoPE::municipios`; was "EleicoesDoTurno") | uenux2/src/app/comum/dados/cconfiguracaoeleicao.cpp | src/uenux2/src/app/comum/dados/cconfiguracaoeleicao.u02.cpp | low |
| 5654 | 243 |  | `comum::(anonymous namespace)::CodigosDosCargos` | uenux2/src/app/comum/dados/cconfiguracaoeleicao.cpp | src/uenux2/src/app/comum/dados/cconfiguracaoeleicao.u02.cpp | low |
| 5671 | 101 |  | `std::vector<std::unique_ptr<T>>::~vector` | (libc++ instantiation) | library/inlined helper | medium |
| 5677 | 53 |  | `comum::md::CHorarioVeraoMunicipio::CHorarioVeraoMunicipio` | uenux2/src/app/comum/dados/md/chorarioveraomunicipio.cpp | src/uenux2/src/app/comum/dados/cconfiguracaoeleicao.u02.cpp | medium |
| 5679 | 30 |  | `comum::md::CHorarioVeraoMunicipio::CHorarioVeraoMunicipio` | uenux2/src/app/comum/dados/md/chorarioveraomunicipio.cpp | src/uenux2/src/app/comum/dados/cconfiguracaoeleicao.u02.cpp | medium |
| 5685 | 725 | * | `std::unordered_map<std::string, SharedWav>::__emplace_unique_key_args` | (libc++ instantiation) | library/inlined helper | medium |
| 5701 | 106 |  | `std::list<std::pair<std::string, SharedWav>>::splice(pos, list, it)` | (libc++ instantiation) | library/inlined helper | medium |
| 5703 | 251 |  | `std::copy<SFotoRef>` | (libc++ instantiation) | library/inlined helper | medium |
| 5704 | 762 |  | `std::vector<SFotoRef>::assign(first, last)` | (libc++ instantiation) | library/inlined helper | medium |
| 5707 | 93 |  | `std::shared_ptr<CWavFile>::operator=` | (libc++ instantiation) | library/inlined helper | medium |
| 5727 | 190 |  | `comum::CEleitores::NomeArquivoTTE` | uenux2/src/app/comum/dados/celeitores.cpp | src/uenux2/src/app/comum/dados/cdadosestaticos.u02.cpp | low |
| 5728 | 37 | * | `comum::(anonymous namespace)::SIdentificacaoCarga::SIdentificacaoCarga` | (unknown) | src/uenux2/src/app/comum/dados/cconfiguracaoeleicao.u02.cpp | low |
| 5730 | 37 |  | `comum::CRespostas::~CRespostas` | uenux2/src/app/comum/dados/crespostas.cpp | src/uenux2/src/app/comum/dados/cdadosestaticos.u02.cpp | medium |
| 5734 | 247 |  | `comum::CRdvVota::~CRdvVota` | uenux2/src/app/comum/dados/crdvvota.cpp | src/uenux2/src/app/comum/dados/crdvvota.u02.cpp | medium |
| 5735 | 1046 |  | `comum::CRdvVota::CriaMapaEleicoesCargos` | uenux2/src/app/comum/dados/crdvvota.cpp | src/uenux2/src/app/comum/dados/crdvvota.u02.cpp | low |
| 5743 | 119 |  | `comum::CLocal::GetLocalData` | uenux2/src/app/comum/dados/clocal.cpp | src/uenux2/src/app/comum/dados/clocal.u02.cpp | high |
| 5746 | 1021 | * | `comum::CIntegridadeReferencial::VerificaFotoCandidatura` | uenux2/src/app/comum/dados/cintegridadereferencial.cpp | src/uenux2/src/app/comum/dados/cintegridadereferencial.u02.cpp | medium |
| 5747 | 1495 | * | `comum::CIntegridadeReferencial::VerificaCargoCandidatura` | uenux2/src/app/comum/dados/cintegridadereferencial.cpp | src/uenux2/src/app/comum/dados/cintegridadereferencial.u02.cpp | medium |
| 5750 | 133 | * | `api::ITextToSpeech::Armazena` | uenux2/src/api/audio/itexttospeech.cpp (path inferred) | src/uenux2/src/api/audio/itexttospeech.u02.cpp | medium |
| 5754 | 169 | * | `api::ITextToSpeech::Busca` | uenux2/src/api/audio/itexttospeech.cpp (path inferred) | src/uenux2/src/api/audio/itexttospeech.u02.cpp | medium |
| 5758 | 839 | * | `api::ITextToSpeech::MontaChave` | uenux2/src/api/audio/itexttospeech.cpp (path inferred) | src/uenux2/src/api/audio/itexttospeech.u02.cpp | low |
| 5762 | 15 |  | `comum::ArquivoBancoExterno` | (unknown) | src/uenux2/src/app/comum/cpath.u02.cpp | medium |
| 5773 | 394 |  | `comum::CEleitores::~CEleitores` | uenux2/src/app/comum/dados/celeitores.cpp | src/uenux2/src/app/comum/dados/cdadosestaticos.u02.cpp | medium |
| 5787 | 682 |  | `std::insert_iterator<std::set<uebyte>>::operator=` | (libc++ instantiation) | library/inlined helper | medium |
| 5788 | 482 |  | `std::__lower_bound_onesided<set<uebyte>::const_iterator>` | (libc++ instantiation) | library/inlined helper | medium |
| 5790 | 3572 | * | `comum::(anonymous namespace)::TodosOsCargos` | uenux2/src/app/comum/dados/cconfiguracaoeleicao.cpp | src/uenux2/src/app/comum/dados/cconfiguracaoeleicao.u02.cpp | low |
| 5791 | 87 |  | `comum::IdsEleicoes` | (unknown) | src/uenux2/src/app/comum/dados/crdvvota.u02.cpp | low |
| 5792 | 294 |  | `comum::CConfiguracaoEleicao::~CConfiguracaoEleicao` | uenux2/src/app/comum/dados/cconfiguracaoeleicao.cpp | src/uenux2/src/app/comum/dados/cconfiguracaoeleicao.u02.cpp | medium |
| 5796 | 373 |  | `std::vector<ecourna::app::dados::CFederacao>::__construct_at_end(first, n)` | (libc++ instantiation) | library/inlined helper | medium |
| 5797 | 348 |  | `std::move_backward<ecourna::app::dados::CFederacao*>` | (libc++ instantiation) | library/inlined helper | medium |
| 5800 | 60 |  | `comum::CFederacoes::~CFederacoes` | uenux2/src/app/comum/dados/cfederacoes.cpp | src/uenux2/src/app/comum/dados/cdadosestaticos.u02.cpp | medium |
| 5805 | 60 |  | `comum::CCargos::~CCargos` | uenux2/src/app/comum/dados/ccargos.cpp | src/uenux2/src/app/comum/dados/cdadosestaticos.u02.cpp | medium |
| 5808 | 501 | * | `std::map<K, comum::md::CCandidatura>::insert(first, last)` | (libc++ instantiation) | library/inlined helper | medium |
| 5913 | 1334 | * | `comum::CMenuBase::Monta` | uenux2/src/app/comum/cmenubase.cpp (path inferred) | src/uenux2/src/app/comum/cmenubase.u02.cpp | medium |
| 5936 | 1802 |  | `vota::testeteclado::CEsperaRetestar::GetInst` | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cesperaretestar.cpp (path inferred) | src/uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.u02.cpp | medium |
| 5953 | 527 |  | `vota::CMenuFiltrarCandidatosBase::CarregaCandidaturas` | uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatosporpartido.cpp | src/uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatos.u02.cpp | low |
| 6035 | 250 |  | `comum::ArquivoTrabExterno` | (unknown) | src/uenux2/src/app/comum/cpath.u02.cpp | low |
| 6036 | 250 |  | `comum::ArquivoTrabInterno` | (unknown) | src/uenux2/src/app/comum/cpath.u02.cpp | low |
| 6117 | 136 | * | `vota::CriaForm<PRESHOW>` | (unknown) | src/uenux2/src/api/gui/cformbuilder.u02.cpp | low |
| 6155 | 1623 | * | `api::FormataDataHora` | uenux2/src/api/util/cdatetime.cpp | src/uenux2/src/api/util/cdatetime.u02.cpp | low |
| 6548 | 1530 |  | `vota::CTelasVota::~CTelasVota` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | medium |
| 6557 | 166 |  | `vota::CTelasVota::CriaTelaContinuacaoVotacao` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | medium |
| 6592 | 942 | * | `vota::CTelasVota::CriaTelaConfirmaImpressaoZeresima` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | medium |
| 6594 | 332 |  | `std::vector<std::pair<char, std::string>>::__emplace_back_slow_path` | (libc++ instantiation) | library/inlined helper | medium |
| 6595 | 1740 | * | `vota::CTelasVota::CriaTelaAntesHorarioZeresima` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | medium |
| 6599 | 4974 | * | `vota::CTelasVota::CriaTelaMaisInformacoes` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | medium |
| 6601 | 504 | * | `vota::CTelasVota::CriaTelaFim` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | medium |
| 6616 | 490 |  | `vota::CTelasVota::CriaTelaConferenciaVotoNuloConsulta` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | high |
| 6626 | 600 |  | `vota::CTelasVota::CriaTelaVotoNuloConsulta` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | high |
| 6639 | 622 | * | `vota::CTelasVota::CriaTelaCompletaCandidatoCom0` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | high |
| 6681 | 476 |  | `vota::CTelasVota::CriaTelaConferenciaCandidatoCom0` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | high |
| 6689 | 281 |  | `api::CFormBuilder::AddDataImage` | (unknown) | src/uenux2/src/api/gui/cformbuilder.u02.cpp | low |
| 7481 | 742 | * | `vota::CAguardaMensagem::ProcessMessage` | uenux2/src/app/vota/eleitor/caguardamensagem.cpp (path inferred) | src/uenux2/src/app/vota/eleitor/cestadosvota.u02.cpp | medium |
| 7787 | 148912 | * | `vota::CInformacaoEleitor::Inicializar` | uenux2/src/app/vota/eleitor/comum/cinformacaoeleitor.cpp | src/uenux2/src/app/vota/eleitor/comum/cinformacaoeleitor.cpp | low |
| 10699 | 871 |  | `vota::(anonymous namespace)::TextoHabilitacaoSemBiometria` | uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | src/uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.u02.cpp | medium |
| 10700 | 871 |  | `vota::(anonymous namespace)::TextoHabilitacaoBiografica` | uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | src/uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.u02.cpp | medium |
| 10701 | 871 |  | `vota::(anonymous namespace)::TextoHabilitacaoBiometrica` | uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | src/uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.u02.cpp | medium |
| 10786 | 5 |  | `vota::CControladorRegistraMesariosVota::SincronizaBancoDados` | (unknown) | src/uenux2/src/app/vota/comum/csincronizavota.u02.cpp | low |
| 11223 | 2163 |  | `comum::CGeradorRelVersaoPacoteDados::MontaCorpo` | (unknown) | src/uenux2/src/app/comum/relatorios/cgeradorrelversaopacotedados.u02.cpp | low |
| 11353 | 415 |  | `std::unordered_map<std::string, list::iterator>::erase(const key&)` | (libc++ instantiation) | library/inlined helper | medium |
| 11436 | 471 |  | `api::CLruCache<std::string, SharedWav>::Insere` | uenux2/src/api/audio/itexttospeech.cpp (path inferred) | src/uenux2/src/api/audio/itexttospeech.u02.cpp | medium |
| 11447 | 52 | * | `std::unordered_map<std::string, SharedWav>::operator[]` | (libc++ instantiation) | library/inlined helper | medium |
| 11481 | 37 |  | `comum::CRespostas static unique_ptr reset (atexit)` | uenux2/src/app/comum/dados/crespostas.cpp | src/uenux2/src/app/comum/dados/cdadosestaticos.u02.cpp | medium |
| 11484 | 132 |  | `comum::asn::CConversorRegistroDigitalVoto<comum::asn::CConversorEleicoesVota>::~CConversorRegistroDigitalVoto` | (unknown) | src/uenux2/src/app/comum/dados/crdvvota.u02.cpp | high |
| 11485 | 129 |  | `comum::asn::CConversorRegistroDigitalVoto<comum::asn::CConversorEleicoesVota>::~CConversorRegistroDigitalVoto` | (unknown) | src/uenux2/src/app/comum/dados/crdvvota.u02.cpp | high |
| 11495 | 37 |  | `comum::CRdvVota static unique_ptr reset (atexit)` | uenux2/src/app/comum/dados/crdvvota.cpp | src/uenux2/src/app/comum/dados/crdvvota.u02.cpp | medium |
| 11497 | 103 |  | `ecourna::api::security::CHKDFSeed::~CHKDFSeed` | ecourna-lib/ecourna/api/security/chkdfseed.cpp | src/ecourna/api/security/chkdfseed.cpp | high |
| 11498 | 100 |  | `ecourna::api::security::CHKDFSeed::~CHKDFSeed` | ecourna-lib/ecourna/api/security/chkdfseed.cpp | src/ecourna/api/security/chkdfseed.cpp | high |
| 11535 | 37 |  | `comum::CEleitores static unique_ptr reset (atexit)` | uenux2/src/app/comum/dados/celeitores.cpp | src/uenux2/src/app/comum/dados/cdadosestaticos.u02.cpp | medium |
| 11797 | 593 |  | `vota::CInspecionaUrna::ProcessInput` | uenux2/src/app/vota/eleitor/cinspecionaurna.cpp (path inferred) | src/uenux2/src/app/vota/eleitor/cestadosvota.u02.cpp | medium |
| 11809 | 1111 |  | `vota::testeteclado::CTesteFalhou::ProcessInput` | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/ctestefalhou.cpp (path inferred) | src/uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.u02.cpp | medium |
| 11812 | 1108 |  | `vota::testeteclado::CEnviarManutencao::ProcessInput` | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cenviarmanutencao.cpp (path inferred) | src/uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.u02.cpp | medium |
| 11891 | 999 |  | `vota::CMenuFiltrarCandidatosPorNumero::ProcessInput` | uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatospornumero.cpp (path inferred) | src/uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatos.u02.cpp | medium |
| 12244 | 248 | * | `vota::CItemVisualizarCandidatosVota::Disponivel` | uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/citemvisualizarcandidatosvota.cpp (path inferred) | src/uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatos.u02.cpp | low |
| 12882 | 37 |  | `vota::CTelasVota static unique_ptr reset (atexit)` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | medium |

## 13. Open questions

* The real name of 7787: nothing in the binary names it. `CInformacaoEleitor::Inicializar` fits the evidence
  (the flags, the only srcloc of `cinformacaoeleitor.cpp`, and the call order in `votaInit`), but it could also be a free
  function of the web entry `vota_web_wasm.cpp` that calls the inlined `CInformacaoEleitor` methods.
* The meaning of `IUrna` slots 4/5/6 (1024-byte, 32-byte and 128-byte tables) on the real urna (TPM / hardware
  secrets?). Slot 6 is the RDV table here. The 10-bit index built in `GetCifradorCryptoTable` hints that the
  design once used the 1024-byte table.
* `api::EInputResult` numeric values (5 = CORRIGE, 9 = CONFIRMA) are inferred from the key labels of the
  keyboard-test screens.
* The global @1577208 is `CEstadoGeral` +48, the fase (answered: u06/u20 and the 5728/2785 uses). Its C++ member
  name is still unknown.
* 6689 always uses data-image id 1086. Is it a `CFormBuilder` method or a `ctelasvota.cpp` helper?
* (answered) 4161's second format (six arguments) is used for a contingency urna (`md::CLocal` +136 set, no seção);
  its sixth argument is the constant "CONTINGÊNCIA".
