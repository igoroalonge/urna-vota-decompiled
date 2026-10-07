# u36: `app:comum` functions without a known source file

Unit u36 holds **101 wasm functions** that the tools put in component `app:comum` but could not attach to an
original file, because none of them contains a `std::source_location` record of its own. Most of them turn out to be
one of these:

* **small methods of well-known `comum` classes** that were not inlined: `CLocal::GetAgregadas`,
  `CEnvelopeGenerico`'s constructors, `CCabecalhoQRCodeBuilder::SetHistoricoCargas`, `CVoto::EhBranco`...;
* **wasm-opt "merge-similar-functions" bodies**: several source functions that differed only in a constant
  (a `std::source_location`, an error code, a vtable, a string literal) were folded into one body that takes the
  constants as parameters. The source functions survive as 1-line thunks, most of them already reconstructed by
  other units;
* **compiler-generated code**: implicit destructors, copy/move members, and **40 static-storage destructors**
  ("atexit" handlers) of singletons. None of the atexit handlers can run in this build (§6.1);
* **C++ library template instances** that the tools attributed to `comum` by caller: the `std::deque` behind
  `std::filesystem::recursive_directory_iterator`, `std::sort`, `std::map` node copies, and libc++ `<locale>`
  static arrays.

The sampling profiler saw **7 of the 101 functions** during the recorded votes (`analysis/runtime/*.functions.tsv`):
`GetEleicoesCargos` (3774), the `std::map` copy 3870, `AssinaDadosDinamicos` (4668, signs `rdv.dat` +
`uenux.db` at start-up), `CApplicationContext::operator==` (5556), the urna-state QR code fields (5636),
`vector<string>::emplace_back` for `CStringUtils::Split` (9404) and the party-number text source of the voter screen
(11499). The profiler misses short functions. We therefore ran the simulator with an **entry counter on each of the 101
functions** (review run: a copy of the wasm with one exported counter global per function, driven by a copy of
`tools/run/headless.mjs`; scenarios municipal-t1 with keys `91001  C  12C  C  `, and geral-t1, municipal-t2,
geral-zz-t1, geral-df-t2 with `--auto mixed`). **19 functions execute**. 18 of them run in every scenario; 11499 runs
only in municipal-t1 and geral-t1, where the voter reaches a party screen:

* during `votaInit`: 3512 `PreencheZerosEsquerda` (4 calls), 3774 (2), 3870 (7..18), 3896 `GetPathArquivo` of
  vota/gap/sa.bin (18), 4668 (1), 5556 (11), 5636 and 5783 (5 each: the zerésima screens' QR code), 5653
  `PossuiCargoEletivo` (6..12), 5726 `SIdentificacaoCarga` (1), 5744 `~md::CLocal` (5), 5763 (1), **5905
  `NomeArquivoSavd` (5: evaluated before every signature, §3.5)**, 6030 enum converters (4..8), 6048
  `CPath::GetPathRoot` (1), 9404 (2);
* while the voter votes: 3774 (+1), 5556 (+1), 5681 (1..5, the `CVotosCargos` map copy of `GravaVotos`), 11499
  (only when a party screen appears) and 11501 (8..20 calls, the party sigla of the candidate screens, §3.7).

The other 82 never ran, in any of the five scenarios. They belong to the encerramento, mesário registration on the MT,
fingerprints, error paths, aggregated sections (no scenario has any) and atexit handlers.

Most interesting findings:

* **The urna-state QR code** (§3.4). Func 5636 builds the tagged record `PROC TURN FASE UNFE MUNI ZONA SECA IDUE
  MDUE IDFL IDCA DTCA HRCA NOME SERT SERI ASSI` from `eg.bin`: the identification of the urna and of its *carga*,
  including the TPM-certificate serial of the media generator and the whole signature of the carga in hex. It
  appears on the zerésima screens and on the printed "ESTADO DA URNA" report. Func 5783 adds `ORIG:T` or `ORIG:I`.
* **BU pieces** (§4): the `HIQT`/`HICA` carga-history field of the BU QR code (5622), the two
  `CEnvelopeGenerico` constructors that wrap `-bu.dat` (5861 plain, 3821 encrypted), the `CPlainText` delegating
  constructor used to encrypt the BU with CEPESC (5168), the blank-vote predicate `CVoto::EhBranco` (types 3 and 5,
  11315), and the atexit destructor of the BU hash-chain state (10274).
* **A validation that the optimiser deleted** (§3.1): `CIdentificacaoUrnaContingencia`'s constructor (5888) had a type
  check at `cidentificacaournacontingencia.cpp:32` ("Tipo inválido para urna de contingência: {}"). Every caller passes
  `'2'`, so the check and its throw disappeared. Only an **unreferenced** `std::source_location` record, an
  unreferenced string and an unused 480-byte stack frame remain. This also explains a false "string" annotation in
  two other functions (§6.4).
* **Plural by chopping a character** (§3.7): the total line of the Boletim de Justificativa, `"{:05} Justificativas"`,
  becomes singular by `pop_back()` on the format string when the count is 1 (11474).
* The **SAVD file-name table** (§3.5, 96 names, ids 25..120) that error messages use when signing fails, e.g.
  83 = "RDV MI", 110 = "UENUXDB INT".

Glossary: *urna* voting machine; *eleitor* voter; *mesário* poll worker; *MT* (terminal do mesário) the poll worker's
micro-terminal; *seção* polling section; *seções agregadas* small sections that vote in another section's urna;
*urna de contingência* spare urna that replaces a broken one; *cargo* office (Prefeito, Vereador...); *eleição*
one election of the *pleito* (the day's set of elections); *carga* the load of software and election data into the
urna (with a *código de carga*); *BU (boletim de urna)* the per-urna result; *BUJ* boletim de justificativa;
*RDV (registro digital do voto)* the shuffled table of votes; *zerésima* the zero report printed before voting; *MI /
ME (MV)* memória interna (internal flash, `/dsk/fi`) / memória externa or de votação (the removable card, `/dsk/fe`);
*SAVD* the urna's signing daemon; *CEPESC* the TSE's encryption scheme; *DS* data source (a function that produces
the text of a screen/report field at draw time).

## 1. What the unit contains

| topic | functions | reconstruction | § |
|---|---|---|---|
| polling place and urna identification | 3752, 3753, 5744, 5726, 5888 | `dados/clocal.u36.cpp`, `nomearquivo/cnomearquivo.u36.cpp`, `md/cidentificacaournacontingencia.{h,cpp}` (new file) | 3.1 |
| offices, votes, counters | 3774, 3782, 3784, 5653, 11315, 11531, 3870, 5681, 5717-5719 | `dados/cconfiguracaoeleicao.u36.cpp`, `dados/ccargos.u36.cpp`, `…/celeicaope.u36.cpp`, `…/rdv/cvoto.u36.cpp` | 3.2 |
| BU file and BU QR code | 5861, 3821, 5168, 5622, 5624, 10274 | `gravadores/md/cenvelopegenerico.u36.cpp`, `ecourna/…/cplaintext.u36.cpp`, `relatorios/ccabecalhoqrcodebuilder.u36.cpp` | 3.3, 4 |
| urna-state QR code | 5636, 5783 | `relatorios/cqrcodeds.cpp` (path inferred) | 3.4 |
| SAVD signatures | 4668, 5905 | `vota/eleitor/comum/cinformacaoeleitor.u36.cpp`, `iinterfacesavd.u36.cpp` | 3.5 |
| mesário registration (MT) | 5383, 6007, 6008, 6009, 6010, 11140, 19 atexit stubs | `comparecimentomesario/cregistradormesario.u36.cpp` + owner files (u22) | 3.6 |
| screen / report text sources | 11474, 11499, 11501 | `justificativa/cjustificador.u36.cpp`, `dados/cpartidos.u36.cpp` | 3.7 |
| fingerprints (biometria) | 3719, 3794, 3898, 5816, 5817, 10519-10522 | `…/cbiometriaeleitor.u36.cpp`, `cpath.u36.cpp` | 3.8 |
| other merged bodies and small members | 3894, 3896, 6030, 6043, 6044, 10877, 5552, 5556, 5760, 3834, 6048, 3510, 3512 | `u36-foreign-fragments.cpp`, `api/io/cinisection.u36.cpp`, `api/gui/capplicationcontextstack.u36.cpp` | 3.9 |
| static-storage destructors | 34 functions (10274 … 11660) | listed in `u36-foreign-fragments.cpp` §C1 | 6.1 |
| C++ library | 3368, 4778, 4779, 8097, 8099, 9404, 9873, 5956, 5763, 7765-7776 | listed in `u36-foreign-fragments.cpp` §C2 | 3.10 |

**Reconstructed sources written by this unit** (all under `src/`; `*.u36.cpp` are fragments of files owned by other
units, to be merged into them):

```
src/uenux2/src/app/comum/u36-foreign-fragments.cpp                 merged bodies, implicit members, atexit + library lists
src/uenux2/src/app/comum/dados/clocal.u36.cpp                      CLocal::GetAgregadas (3752), GetUFMinuscula (3753)
src/uenux2/src/app/comum/dados/cconfiguracaoeleicao.u36.cpp        CConfiguracaoEleicao::GetEleicoesCargos (3774)
src/uenux2/src/app/comum/dados/ccargos.u36.cpp                     CCargos::LimpaFiltro (3784)
src/uenux2/src/app/comum/dados/cpartidos.u36.cpp                   CPartidosDSNumero/DSSigla::Text (11499/11501)  (path inferred)
src/uenux2/src/app/comum/dados/md/eleitor/cbiometriaeleitor.u36.cpp  CBiometriaEleitor::PossuiDedo (3719)
src/uenux2/src/app/comum/dados/md/processoeleitoral/celeicaope.u36.cpp  CEleicaoPE::PossuiCargoEletivo (5653)
src/uenux2/src/app/comum/dados/md/rdv/cvoto.u36.cpp                CVoto::EhBranco (11315)
src/uenux2/src/app/comum/md/cidentificacaournacontingencia.{h,cpp} CIdentificacaoUrnaContingencia (5888) - new file
src/uenux2/src/app/comum/nomearquivo/cnomearquivo.u36.cpp          SIdentificacaoCarga ctor (5726)
src/uenux2/src/app/comum/gravadores/md/cenvelopegenerico.u36.cpp   CEnvelopeGenerico ctors (5861, 3821)
src/uenux2/src/app/comum/relatorios/ccabecalhoqrcodebuilder.u36.cpp  builder ctor (5624), SetHistoricoCargas (5622)
src/uenux2/src/app/comum/relatorios/cqrcodeds.cpp                  urna-state QR fields (5636, 5783)  (path inferred) - new file
src/uenux2/src/app/comum/iinterfacesavd.u36.cpp                    NomeArquivoSavd (5905) + the 96-name table
src/uenux2/src/app/comum/comparecimentomesario/cregistradormesario.u36.cpp  QuantidadeRegistrados (6007)  (path inferred)
src/uenux2/src/app/comum/justificativa/cjustificador.u36.cpp       CJustificadorDadoQuantidade::Text (11474)
src/uenux2/src/app/comum/cpath.u36.cpp                             external WSQ directories (3898, 3794, 5816, 5817)
src/uenux2/src/app/vota/eleitor/comum/cinformacaoeleitor.u36.cpp   AssinaDadosDinamicos (4668)
src/uenux2/src/api/gui/capplicationcontextstack.u36.cpp            CApplicationContext operator= / operator== (5552, 5556)
src/uenux2/src/api/io/cinisection.u36.cpp                          CIniSection::Find (10877)
src/ecourna/api/security/cepesc/cplaintext.u36.cpp                 CPlainText delegating ctor (5168)
```

Seven functions of the unit were already written, with the same meaning, by the owner units (3510/3512 in
`celeitoridentidade.cpp`, 3782 in `ccargos.cpp`, 3834/6048 in `cpath.cpp`, 5383 in `cgestordadomesario.cpp`,
11531 in `celeitores.cpp`). The mapping table (§9) points to them.

## 2. Classes and how they relate

The unit has no vtable slot of its own: every function is reached by direct call, as a thunk target, or through a
function-table slot (comparators, text sources, atexit handlers). The classes it touches, with their RTTI where it
exists:

```
comum::md::CIdentificacaoUrna                         (not polymorphic, 12 bytes: tipo, município, zona)
 ├ comum::md::CIdentificacaoSecao                       tipo '1'  (5887)
 └ comum::md::CIdentificacaoUrnaContingencia            tipo '2'  (5888, this unit)
comum::IResultado (vtable @1541028)                    +20 std::string m_nome
 └ comum::IGravador (@1553656)
     ├ comum::CGravadorBU (@1554140)                   member CDadoCorrespondencia m_correspondencia
     ├ comum::CGravadorRDV (@1556856)                   … at +56   -> destructor body 6043 (this unit)
     └ comum::IGravadorEnvelope (@1554456)              … at +76   -> destructor body 6043
comum::CAssinador (@1553260) └ vota::CAssinadorVota (@1532312)   ctor 1501, used by 4668
api::IText └ api::CDataTextFmt<const std::string (*)(const std::string&)> (@1538280)   11499, 11501
           └ api::CDataTextFmt<std::string (*)(const std::string&)> (@1539168)         11474
api::CDataMap<K, V> (not polymorphic) <- comum::CPartidos ("CPartidos"), comum::CJustificador ("CJustificador")
api::IPreShow └ api::CPreShowClearScreen (@1577896)   11140 = CriaForm<CPreShowClearScreen>
```

Non-polymorphic records whose members are used here: `md::CLocal` / `md::CSecaoEleitoral` / `md::CIdentificacaoAgregada`
(polling place), `md::CEstadoGeral` / `md::CDadoCarga` / `md::CDadoCorrespondencia` (`eg.bin`), `md::CEnvelopeGenerico`
(result-file envelope), `CCabecalhoQRCode` (33 `"TAG:value "` strings), `api::CApplicationContext` (error-screen text),
`md::CEleitorDinamico`, `md::CVoto`, `md::CEleicaoPE`, `md::CCargo`.

## 3. The functions, by topic

### 3.1 Polling place and urna identification

* **`CLocal::GetAgregadas` (3752).** `CLocal` is the singleton that wraps the polling-place file `<zona><secao>-lo.dat`
  (`ModuloLocal`). Like every `CLocal` accessor it first calls `VerificaLido("GetAgregadas")` (func 782). That call throws
  `"GetAgregadas: o arquivo de locais ainda não foi carregado"` if the file was not read. Then, if the local is a
  seção (optional `CSecaoEleitoral` engaged, byte +120), it copies the `uint16` seção number of each
  `CIdentificacaoAgregada` into a new `vector<TSecaoID>`. A contingency urna gets an empty vector. Users: the `AGRE:`
  field of the BU QR code (11242), `CRelUtil::FormataSecoesAgregadas` (5738) and the zerésima screen header (4160). No
  simulator scenario has aggregated sections, and the function never ran in the entry-counter runs. In
  the binary `md::CLocal::GetSecao()` is called three times. Two of the results are unused, and the calls survive only
  because `GetSecao` can throw.
* **`CLocal::GetUFMinuscula` (3753).** `ToLower(GetUF())`, with `GetUF` inlined (`VerificaLido("GetUF")`). It gives the
  lower-case UF of every result file name, `t02410`**`ac`**`0000100010001-bu.dat`. It is called by
  `CGravadorUtil::DeterminaNomeArquivoSemLetra` (3798) and `CGravaResultado` (12098). Unlike
  `comum::(anon)::FormataUF` (3773), it does not check that the UF has 2 letters.
* **`md::CLocal::~CLocal` (5744)** is the implicit destructor. It frees the optional section (and its vector of
  aggregated sections), then the strings município name, UF name, UF sigla and país.
* **`SIdentificacaoCarga` constructor (5726).** This 24-byte record holds `{fase, id do processo eleitoral, uf lower-cased
  by ToLower (1879), const CPleito*}` and identifies the carga in file names. `CPE` embeds it at +180; there the pleito is
  the one of the current turno. `CConfiguracaoEleicao` embeds it at +620; there it points to its own `m_pleito`, and it is
  followed by município, zona and the aggregated sections. The struct name comes from `cnomearquivo.h` (u24). The fourth
  member is new.
* **`md::CIdentificacaoUrnaContingencia` constructor (5888).** It is `CIdentificacaoUrna('2', município, zona)` (5886,
  which validates type ∈ '1'..'4' with an unsigned test, município < 100000 and zona < 10000). The `std::source_location` record @1552708 gives the file,
  the line and the full signature `CIdentificacaoUrnaContingencia(const EUrnaTipo, const TMunicipioID, const TZonaID)`.
  No code references that record. The only callers, `CConversorLocal::DoDesconverte` (11452, the `identificacaoContingencia`
  branch of `-lo.dat`) and `CGravaResultado` (12098), pass `'2'`. So the optimiser removed the `tipo` parameter and the
  check at line 32 together with its message "Tipo inválido para urna de contingência: {}" (@6328, also unreferenced).
  The only trace is an unused 480-byte stack frame, the size of the `std::format` buffer that the throw needed
  (the other `std::format` + throw helpers of this build, e.g. 2919 and 6044, also have 480-byte frames). The optimiser
  is **wasm-opt**, after linking, not LLVM: see §6.3.

### 3.2 Offices, votes and counters

* **`CConfiguracaoEleicao::GetEleicoesCargos` (3774, ran).** It returns one `{TEleicaoID, TCargoID}` pair (8 bytes) per
  cargo of each eleição of the pleito, in file order. It walks `CConfiguracaoEleicao +52` (the pleito's
  `vector<CEleicaoPE>`, 52-byte elements) and, for each eleição, `CEleicaoPE::GetCargos()` (2257, 140-byte `CCargo`).
  The start-up code calls it twice, to fill `CCargos::m_todos` and `m_cargos` (`CCargos::CreateInst`, inlined in 7787).
  `CEleitorVotando::GravaVotos` (4454) calls it once per voter, to find the eleição, i.e. the cédula of the RDV, of each
  confirmed vote.
* **`CCargos::LimpaFiltro` (3784) and `OrdenaPorOrdemImpressao` (3782).** While a voter votes, `m_cargos` holds only
  the cargos he can vote for (`FiltraPorAbrangencia`). At the end of the day `CGeraBU` (12110) and `CGravaResultado`
  (12098) first restore the full list (`m_cargos = m_todos`, 3784, which leaves the cursor alone; other units' code calls
  it `Inicio()`). Then they sort it in print order (3782: `std::sort` with comparator slot 2358 = func 11558, then cursor
  = 0) before they walk the cargos for the BU, the RDV file and the QR codes.
* **`CEleicaoPE::PossuiCargoEletivo` (5653)** is `any_of(cargos, has CDetalheCandidato)`. It is false for an eleição
  that has only referendum questions ("consultas"), which then gets no candidate file (`CNomeArquivo::MontaNomesEleicao`,
  3744). It runs at start-up (3744 and 7787: 6 calls in the municipal scenarios, 12 in the general ones).
* **`CVoto::EhBranco` (11315)** is `tipo == 3 || tipo == 5` (branco, branco após suspensão). It is passed as a
  **pointer to member function** `{slot 2834, adj 0}` to `CVotos::TotalQue(bool (CVoto::*)() const)` (11340, a
  `std::function` wrapping the call). `CVotosEleicoesVota::Brancos` (3747 = `CRdvVota` vtable slot 9) uses it. That
  counter is the number of blank votes of a cargo in the BU and in the reports. Its siblings are `EhNulo` 11316 (4, 6, 7),
  `EhLegenda` 11317 (1) and `EhNominal` 11318 (2).
* **`ComparaDinamico` (11531)** is the function-pointer comparator (slot 2387) that `CEleitores::CompleteLoad` passes to
  `std::sort` for the voters' dynamic rows. It compares the `CEleitorIdentidade` (título string, then type). It was
  already written in `celeitores.cpp`.
* Library instances: 3870 (`map<TEleicaoID, string>` node copy: the per-eleição package versions of `CPleito`), 5681
  (`map<SCargoInfo, CVotos>` node copy, used by `CVotosCargos::InsereCedula`'s transactional copy), 5717/5718/5719
  (`std::sort` of `CIdentificacaoAgregada` by seção number in `CConversorSecaoEleitoral::DoDesconverte`).

### 3.3 BU file, encryption and QR header

These functions are described in §4, step by step: 5861/3821 (envelope), 5168 (`CPlainText`), 5624/5622 (QR header
builder and `HIQT`/`HICA`), 10274 (hash-chain state).

### 3.4 The urna-state QR code (5636, 5783)

`MontaCamposQRCodeEstadoUrna(const CEstadoGeral&)` (5636, ran during `votaInit`) returns a
`vector<pair<string,string>>`. `CQRCodeDS::operator()` (5782) later appends the volatile fields (`VERS`, pleito date...)
and joins everything as `TAG:value TAG:value …`. Fields, in order:

| tag | value | source (`eg.bin`, `ModuloEstadoGeralUrna`) | simulator value (fixture of `CAppInfoBuilder`, u30) |
|---|---|---|---|
| `PROC` | id do processo eleitoral | `idPE` (+4), `to_string(unsigned)` | scenario's PE |
| `TURN` | `1` if turno = '1' else `2` | only when the current turno's `TipoUrnaOperacao` is **not** `contingencia` ('2') or `contingenciavotarecupera` ('4') | `1` (`2` in the 2nd-round scenarios) |
| `FASE` | `toupper(GetFaseChar())` | `DadoCarga.fase` → `O`/`S`/`T` | `T` (training) |
| `UNFE` | UF, copied as stored | +8 | set by `votaInit` (upper case) |
| `MUNI`, `ZONA` | município, zona | +20, +24 | scenario |
| `SECA` | seção | only when the tipo is `vota` ('1') or `contingenciavota` ('3') | scenario |
| `IDUE` | número interno da urna | `correspondencia.carga.numeroInternoUrna` (+60) | `87654321` |
| `MDUE` | modelo da urna | `DadoCarga.modelo` (+44), as an int | `2015` |
| `IDFL` | série da flash de carga (8 hex) | +64 | `12345678` |
| `IDCA` | código da carga | +88 | `123456789012345678901234` |
| `DTCA` / `HRCA` | carga date `YYYYMMDD` / time `hhmmss` | +76 / +84 | `20201231` / `235958` |
| `NOME`, `SERT`, `SERI` | media generator: host name, TPM certificate serial, installation serial | +120, +132, +144 | `nome_maquina`, `12345678`, `99999999` |
| `ASSI` | the carga correspondence **signature**, upper-case hex (func 1243) | +108 | 278 hex digits of the fake signature |

`AdicionaOrigemQRCode(campos, variante)` (5783) appends `ORIG:T` for variante 0 (the "antes do horário" zerésima screen),
`ORIG:I` for 1 (the printed ESTADO DA URNA report, `CRelatorioTesteImpressora` 11200), and nothing otherwise (2 = the
"confirma impressão da zerésima" screen). The meanings *tela* / *impresso* are our guess. On screen the QR image is
regenerated every 15 s (`CImageFieldUpdate`, 3059).

### 3.5 SAVD signatures

* **`AssinaDadosDinamicos` (4668, ran in `votaInit`).** It builds a `vota::CAssinadorVota` with package 122 (1st turno)
  or 123 (otherwise) and asks SAVD to sign file id **83 "RDV MI"** (`rdv.dat`) and id **110 "UENUXDB INT"**
  (`uenux.db`). `CInformacaoEleitor::GerarDadosDinamicos` (6737) calls it right after the RDV is created, once in the
  voter-training branch and once in the normal branch. In the web build the SAVD client is `(anonymous)::CWasmSavd`,
  which answers "OK" and does nothing (u23). The `.vsu` files that exist are the fake ones `votaInit` writes.
* **`NomeArquivoSavd` (5905)** maps a SAVD file id (25..120) to a name through a 96-entry `const char*` table @1551420.
  Other ids give "Arquivo não identificado". The name ends up in the error-screen text "Ocorreu um erro durante a
  assinatura do arquivo: …" (`CAssinador::Assina` 1277, `CGravaResultado` 12098). `CAssinador::Assina` builds that text
  **before** it calls SAVD, for the `api::CApplicationContextGuard` (676, title "Erro na assinatura") that is on the
  error-context stack during the signature. So 5905 runs on **every** signature, not only when one fails. It ran 5 times
  in `votaInit` in the entry-counter runs. The text is shown only if the signature fails. The full table is in
  `iinterfacesavd.u36.cpp`. Some entries: 25 "EG Geral MI", 31 "EG VOTA MI", 35 "BU do VOTA MI (res)", 37 "RDV MI (res)",
  43 "BU imp. do VOTA MI (res)", 44 "ZE imp. do VOTA MI (res)", 60 "Log MI (res)", 83 "RDV MI", 88 "Zerésima MI",
  110 "UENUXDB INT", 112 "Arquivo de local". Id 120 maps to an empty string (a literal merged with the tail of another).

### 3.6 Mesário registration on the MT

Before the voting starts and again at the end, the mesários register at the micro-terminal. They type their título,
then give a fingerprint (u22 reconstructed the state machine: `CRegistrarMesarios` → `CPedeTituloMesario` →
`CTituloMesarioVazio` / `CTituloMesarioInvalido` / `CTituloMesarioJaRegistrado` → `CPedeDigitalMesario` →
`CGestorDadoMesario*` → `CConfirmaFimRegistroMesarios` → `CEncerraRegistroMesarios`). This unit holds the pieces that the
compiler shared or generated:

* 6007 is `CRegistradorMesario::QuantidadeRegistrados(periodo)`: it counts the cached `comparecimento_mesario` rows whose
  key period equals the argument. It is a merge-similar body. The thunks are `shared_f3606` (period 1) and
  `shared_f2727` (period 2). Users: the MT counters, `CControladorRegistraMesariosVota` and the BIM report.
* 6008 is the body of both `CNomeMesariosUrnaDS::Text` (MT line with the mesário's name). If the mesário is not a voter
  of this section, it returns the raw título **without** applying the format. Otherwise it formats the first 40 **bytes**
  of the social name, or of the name when the social name is empty.
* 6009 is the body of the two "título" text sources.
* 6010 is the body of `CTituloMesarioVazio::StartState` and `CTituloMesarioInvalido::StartState`. The two differ only in
  their srcloc, so an **empty** título is logged with the same controller call (slot 26) as an invalid one. In VOTA
  that slot is `CControladorRegistraMesariosVota::vf26` (10776), which logs "Digitado título inválido para o registro de
  mesário" (`CLoga::loga`, level 2). An empty título therefore leaves the same log line as a wrong one.
* 5383 is `CGestorDadoMesario::GetInst` (merged singleton body `vota_f764`). 11140 is
  `api::CriaForm<api::CPreShowClearScreen>`, the voter-screen form shown during registration (§7).
* 19 atexit handlers of the state singletons (§6.1): 10320/10321, 10334/10336, 10346, 10350/10351, 10354/10355, 10371,
  10378, 10381/10382, 10386/10387, 10389/10390, 10393/10394.

### 3.7 Text sources of screens and reports

* **`CPartidosDSNumero::Text` (11499, ran) and `CPartidosDSSigla::Text` (11501)** return
  `std::vformat(formato, current party number / sigla)`. The current party is taken from `CPartidos` (a lazy singleton
  inlined here: `unique_ptr` @1838928, a `CDataMap<TPartidoID, CPartido>` named "CPartidos", ctor 3751).
  `GetCurrent()` (1283) throws "O registro corrente estava inválido {}" when no party is selected. Slot 1100 (number)
  is the party screen of a proportional vote (`CTelasVota::CriaTelaPartido`, `ctelasvota.cpp:1741`, built at start-up):
  it is shown while the voter has typed only the party digits ("91" of "91001") or votes for the party only (voto de
  legenda). Slot 1089 (sigla) is the "partido" line of the candidate screen, added by `vota_f2028` (`adicionaPartido`)
  when `CConfiguracaoEleicao +484` ("apresentar partido") is set. A headless run
  (`node tools/run/headless.mjs --scenario municipal-t1 --keys "91001 " --draw`) draws `fillText("PEsp", 20, 451)` on
  the Vereador candidate screen. "PEsp" is the sigla of party 91 in `t02411ac00001-pa.dat` ("Partido dos Esportes" is
  its name), so 11501 runs as well even though the profiler did not sample it. The entry counters give 8 to 20 calls per
  voter, including municipal-t2, where the only cargo is Prefeito.
* **`CJustificadorDadoQuantidade::Text` (11474)** is the total line of the printed Boletim de Justificativa,
  `CDataTextFmt(slot 2948, "{:05} Justificativas")`. When the count is exactly 1, the function removes the **last
  character of the format**, so the output is "00001 Justificativa". The count is `CJustificador`'s map size (+8).

### 3.8 Fingerprints

* **`CBiometriaEleitor::PossuiDedo` (3719)** returns `m_dedos.has_value() && m_dedos->contains(tipo)`. Byte +32 is the
  engaged flag of an **optional** map at +20; `cbiometriaeleitor.h` (u05) has to be corrected on this point (§7).
* **External WSQ directories (3898 + thunks 3794, 5816, 5817)** build
  `<flash externa>/dinamico/trab<turno>/wsq/{operador,nao-habilitado,habilitado}/`, where the turno comes from `eg.bin`.
  Three source functions differed only in the literal, and wasm-opt merged them. The `"habilitado/"` literal is the tail
  of `"nao-habilitado/"`. The internal-flash twins are `vota_f2298` + 5 thunks (other units). Users:
  `CGravadorWSQ::CompactaWsq` (packs the `wsq*.jez` result files), `CMostraEleitorVotando::SalvaHabilitacaoEleitor`,
  `CPedeDigitalMesario`. None of them runs in the simulator.

### 3.9 Other merged bodies and small members

* 3894: DAO `Clonar()`, `new T(*this)`, copies the vptr + `shared_ptr` to the SQLite connection. Three DAOs use it.
* 3896: `IServicoEstado::GetPathArquivo` for `vota.bin` / `gap.bin` / `sa.bin`. It returns
  `CPath::GetPathTrab(midia, turno) / nome`, or throws "Urna sem turno em contexto onde turno era esperado"
  (`CUeComumAppInfoError` 7607 / 7605 / 7606, srcloc line 32 of each service file) when the turno is `'0'`. It runs 18
  times in `votaInit` (entry counters), where `trab1/` and `trab2/` receive `vota.bin`, `gap.bin` and `sa.bin`.
* 6030: enum converter check `unsigned(v - 1) >= n` → `CUeComumDadosError` (8184 "Tipo de identificador de eleitor
  inválido", n = 3; 7953 "Origem configuração inválida", n = 2).
* 6044 + 10877: `CVersoesContratos::GetValor` / `CDependenciasContratos::GetValor`. They look the key up in the
  `api::CIniSection` map (`Find` = 10877). A missing key throws "Propriedade inexistente: {}" (8697 at `cversoescontratos.cpp:42`, 8663 at `cdependenciascontratos.cpp:59`).
  Otherwise they return the key's value (`CIniKey +12`). The simulator ships 0-byte `/etc/*.properties`, so every lookup
  would throw.
* 6043: destructor body of `CGravadorRDV` and `IGravadorEnvelope`: `~CDadoCorrespondencia` (857) on the member at +56 / +76,
  then `IResultado::m_nome`.
* 5552 / 5556: `api::CApplicationContext` move assignment and `operator==` (52 bytes). `~CApplicationContextGuard`
  (675, ran) uses them to remove its context from the global error-context stack: it searches from the top for an equal
  element and erases it.
* 5760: implicit copy constructor of `md::CEleitorDinamico` (84 bytes). 5763: implicit move constructor of the
  `CEleitores` map value `pair<const CEleitorIdentidade, CEleitorDetalhe>` (220 bytes; the const key is copied).

### 3.10 C++ library code

* 3368, 4778, 4779, 8097, 8099: the `std::deque<std::filesystem::__dir_stream>` (80-byte elements, 51 per 4080-byte
  block, allocated with `operator new(size_t, align_val_t 16)`). It is the directory stack of
  `recursive_directory_iterator` (8096/8101), which ecourna's glob (`ICompressor::Add`) uses while packing WSQ files.
* 9404: `vector<string>::__emplace_back_slow_path(str, pos, n)` (`CStringUtils::Split`, 1880; ran).
* 9873: exception-rollback destroyer of a `vector<CDadoCorrespondencia>` copy (table slot 473).
* 5956: `~vector<md::CMunicipioDisponivel>` (the "dados disponíveis na carga" report).
* 7765/7768/7770/7772/7774/7776: libc++ `locale.cpp` static arrays of `__time_get_c_storage` (`"%m/%d/%y"`, AM/PM,
  months, week days, `char` and `wchar_t`). These are atexit destructors, so they are dead too (§6.1).

## 4. Boletim de urna: what this unit contributes, step by step

The BU flow as a whole is in `docs/10-boletim-de-urna.md` and `docs/bu/*.md`. The steps below are the ones that run
through u36 functions, in execution order during the encerramento (`vota::CGeraBU::StartState` 12110 and
`vota::CGravaResultado::StartState` 12098; neither runs in the public simulator, which never closes the voting).

1. **Cargo order.** Before counting, `CCargos::LimpaFiltro()` (3784) restores the full list of (eleição, cargo) pairs,
   which `GetEleicoesCargos` (3774) built at start-up in pleito-file order. Then `OrdenaPorOrdemImpressao()` (3782) sorts
   them by `CSituacoesEleicoes.ordemImpressao` (byte +5) and then `CCargo.ordemImpressao` (+16), and rewinds the cursor.
   The BU entities, the RDV file, the printed BU and the QR codes all walk the cargos in this order.
2. **Counting blank votes.** For each cargo, `CVotosEleicoesVota::Brancos(cargo)` counts the RDV votes for which
   `CVoto::EhBranco()` (11315) is true: types **3 (branco)** and **5 (branco após suspensão)**. The BU has no separate
   "blank after suspension" counter; both types count as blank (`TipoVoto branco(2)` in `ModuloBoletimUrna`). Null
   votes are `EhNulo` (4, 6, 7). Types 8/9 (null for a cargo without candidates) are counted apart (see
   `docs/bu/build-a-bu.md` §4).
3. **Hash chain state.** `CConversorEntidadeBU::DoConverte` (10273) chains SHA-512 hashes of the vote tuples in a
   static `std::vector<uebyte>` @1910004 (`CAssinaVotavelBU` state, `docs/bu/build-a-bu.md` §6). Its static
   destructor is func 10274 (free the buffer). Because of `EXIT_RUNTIME=0` it is never registered, so the last hash stays
   in memory until the page is unloaded.
4. **Optional encryption of the BU** (parameter `criptografarBU` of the urna parameters, `CConfiguracaoEleicao +160`,
   **and** `CGravadorBU +212`; `False` in the simulator's data and in the published BUs). `CGravadorBU::GravaResultado`
   (11629) builds the plaintext with the **delegating constructor `CPlainText(zona, seção, tabela, aleatório, chave,
   conteúdo)` (5168)**, which forwards to the full constructor (9465, `cplaintext.cpp:89..104`) with
   `tipoArquivo = 0`, `idCriptografia = 1` and no `CInfoSalt`:
   * `tabela`: the urna's 1024-byte crypto table (`IUrna` slot 4),
   * `aleatório`: 32 random bytes (`IRng` slot 4),
   * `chave`: the BU public key read from file by `LeChavePublica`,
   * `conteúdo`: the DER bytes of `EntidadeBoletimUrna`.

   The full constructor rejects out-of-range ids (1517/1518) and empty vectors (1519..1522). `CCepescCipher::Cifra` then
   produces the ciphertext and `md::CSeguranca(0, 1, cifrado)`.
5. **The envelope** (`ModuloEnvelopeGenerico::EntidadeEnvelopeGenerico`, file `<fase><pleito:05><uf><mun:05><zona:04>
   <secao:04>-bu.dat`, e.g. `t02410ac0000100010001-bu.dat`; the lower-case `uf` comes from `CLocal::GetUFMinuscula`,
   3753). `CGravadorBU` builds it with one of the two constructors of this unit:
   * **plain** `CEnvelopeGenerico(cabecalho, fase, município, zona, local, seção, tipo, conteúdo, tipoUrna)` (5861):
     `urna` absent, `seguranca` absent, `conteudo` = the DER bytes of the BU;
   * **encrypted** `CEnvelopeGenerico(…, tipo, seguranca, conteúdo, tipoUrna)` (3821): `seguranca` present,
     `conteudo` = the CEPESC ciphertext.

   Both copy the 20-byte `CCabecalhoEntidade` (dataGeracao + idPleito), the fields and the content, then call
   `ValidaCriacao()` (3822). That call rejects fase == '0' or fase ≥ '4' (8678, signed test: a value below '0' would
   pass), município ≥ 100000 (8679), zona ≥ 10000 (8680), an engaged local ≥ 10000 (8681), **tipoUrna ∉ '1'..'4'**
   (8682: `(tipoUrna - '5') ≤ -5` compiled as `i32.le_u`, so it is an unsigned range test and rejects values above '4'
   too), seção ≥ 10000 (8683) and envelope type ≥ 5 (8684, signed). Finally **the local is
   dropped unless `tipoUrna` is '1' (seção), '3' or '4'** (compiled as `(unsigned)(tipoUrna - '1') > 3 || tipoUrna ==
   '2'`; after `ValidaCriacao` only '2' can reach the reset), i.e. a pure contingency urna ('2') writes an
   `identificacaoContingencia` without a local. `CConversorEnvelopeGenerico` (10271) encodes the result. Envelope type 0 is
   `envelopeBoletimUrna (1)`.
6. **The QR codes of the BU** (printed BU: parts of ≤ 1100 characters; on-screen "BU digital": ≤ 2500; except the last
   part, which can reach 1245 (Ed521) or 1259 (ECDSA) characters printed and about 2659 on screen, because in format 6.0
   its fixed text takes 422–436 characters, more than the 277 reserved; 2026 urna data: `investigation/README.md`,
   finding H5). The header is
   built with `CCabecalhoQRCodeBuilder`, whose constructor (5624) zero-fills the 33 header strings. `SetHistoricoCargas`
   (5622) fills the `HistoricoCarga` field (+168) with **both** tags:

   ```
   HIQT:<n>                       n = number of carga codes in gap.bin's correspondence history (size_t)
   HICA:<i>:<código, max 24 chars>   repeated n times, i = 1..n
   ```

   Each item is followed by one space (`"HIQT:{} "`, `"HICA:{}:{:.24s} "`, strings @440013/@440426). `preBuild()` later
   requires the field to be non-empty (`EUeComumRelatoriosError` 9050, "Campo (HistoricoCarga) não informado."), and
   `HIQT:0 ` satisfies that. `docs/bu/qrcode.md` gives the whole payload and the hash/signature of the QR chain.
7. **Signing** (`CAssinador::Assina` → SAVD, simulated in the web build). Before each signature the error-context text
   names the file through `NomeArquivoSavd` (5905); it is shown only on failure: 35 "BU do VOTA MI (res)", 63 "BU do VOTA ME (res)", 43/66 printed BU, 37/65 RDV...
8. **RDV writer tear-down.** `CGravadorRDV` and `IGravadorEnvelope` (the `imgbu.dat` / `imgze.dat` writer) are destroyed
   through the shared body 6043. It frees their copy of the carga correspondence (`CDadoCorrespondencia`, 96 bytes) and
   the file name.

Not in this unit but related: the urna-state QR code (§3.4) is **not** the BU QR code. It identifies the urna and its
carga (with the carga signature in `ASSI`) on the zerésima screens and the ESTADO DA URNA report.

## 5. Web build specifics

* Entry counters (see the introduction) show that `votaInit` runs 3512, 3774, 3870, 3896, 4668, 5556, 5636, 5653, 5726,
  5744, 5763, 5783, 5905, 6030, 6048 and 9404. While the voter votes, 11499 (party number) and 11501 (party sigla)
  produce screen text, 5556 runs once more (error-context guard), and 3774 and 5681 run when the votes are stored
  (GravaVotos). The other 82 functions belong to the encerramento, the mesário registration on the MT, the
  fingerprint paths, aggregated sections, error handling or atexit, and none of them ran in the five scenarios.
* 4668 signs through `CWasmSavd`, a no-op that always answers OK: no signature is made.
* The eg.bin fixture of the web build (`CAppInfoBuilder`, u30) is what the urna-state QR code (5636) shows:
  urna 87654321, model 2015, flash 12345678, carga `123456789012345678901234` of 31/12/2020 23:59:58, generator
  `nome_maquina` / `12345678` / `99999999`, and a fake 139-byte signature.
* `/etc/versoes.properties` and `/etc/dependencias.properties` are 0-byte files, so 6044 would throw "Propriedade
  inexistente" on any lookup (only at the encerramento).

## 6. Wasm / Emscripten observations

### 6.1 Static destructors that can never run (40 functions)

The build uses `EXIT_RUNTIME=0` (`docs/libraries/libc-and-emscripten-runtime.md`): `__cxa_atexit` is a no-op and no
registration survives in `__wasm_call_ctors` or in the lazy `GetInst()` functions. The destructors of statics still
exist because their addresses were taken before wasm-opt ran, and the function table keeps them (slots 2140..8600).
Check: no instruction of the module loads any of these handlers' table slots as a constant (e.g. 4476 for 10274, 4388 for
10320, 2914 for 11268, 8600 for 7765; the few `i32.const` hits for 2158/2159/4090 are string addresses and bit masks
in 706, 2919, 2056 and 11390). The handlers are also absent from every entry-counter run. The
unit has 34 of them for application statics (list with addresses and owners in `u36-foreign-fragments.cpp` §C1) and 6
for libc++ `<locale>` arrays. A `std::mutex` destructor shows up as a call of func 150 (the no-op pthread residue) on the
mutex address. A `unique_ptr` destructor is a reset through `shared_f349`, `shared_f3886` or `unknown_f763` (the latter
with a virtual delete). The singletons of the mesário states, `CLogComum`, `CArquivosSavd`, `CArquivosResultado`,
`CRespostas`, `CSubstituidorTitulo`, `CPath`'s root strings, the BU hash-chain vector, the urna certificate cache
(`CEstadoGeral::RecuperarCertificado`) and the two biometric helper singletons are therefore never destroyed. That is
harmless in a page that is reloaded between voters.

### 6.2 merge-similar-functions

Ten bodies of the unit were produced by wasm-opt's "merge similar functions": 3894, 3896, 3898, 6007, 6008, 6009, 6010,
6030, 6043, 6044 (plus 6117 behind 11140). In each case two or three source functions differed only in constants, and
the pass moved those constants into extra parameters: a `std::source_location` record address (so the srcloc names the
thunk's file, not the body's), an error code, a vtable, a `string_view` literal as `(begin, end)`, a numeric period. The
thunks keep the real names. The tools named the merged bodies `comum_fNNNN`, or `shared_fNNNN` for the thunks called
from several components.

### 6.3 Constant propagation removed a check (5888)

`CIdentificacaoUrnaContingencia`'s constructor had a `tipo` check at line 32. All call sites pass `'2'`, so the check
is always false, and it was deleted together with its `std::format` call. What is left: the constructor lost its first
parameter, a 480-byte stack frame is allocated and never used, and the srcloc record @1552708 and the message @6328 have
no reference. Behaviour is unchanged.

The leftovers show that **wasm-opt** did this after linking, not LLVM. The two callers are in other translation units
(`comum/dados/asn`, `vota`), so LLVM without LTO could not know the argument. LLVM would also have dropped the dead
480-byte frame, and `wasm-ld --gc-sections` would have discarded the unreferenced read-only data. wasm-opt's
dead-argument elimination saw the same constant `50` at every call site, removed the parameter and propagated it, and
the branch folded away. wasm-opt does not rewrite the shadow-stack prologue or the data segments, so both survive.

### 6.4 Tool artefacts found while reading the unit

* The annotator printed `str 6328 'Tipo inválido para urna de contingência: {}'` in `comum_f5168` and
  `CGravadorRCSecao::LeChavePublica` (11616). There 6328 is the **table slot** of `CPlainText::CPlainText` (9465), passed
  to `invoke_iiiiiiiiiii`. The inline comment `632 /* &… */8 /* "…" */` shows the split. The two values coincide by chance.
* Callers once shown as `ecourna::api::security::CHKDFSeed::GetSeed` are func 7787, the start-up routine
  (`vota::CInformacaoEleitor::Inicializar`, u02; the tools now show that name). `api::CImageFieldUpdate::CImageFieldUpdate` as a caller of 5636/5783 is
  func 3059, `adicionaBlocoMensagem` of the zerésima screens (u15). `api::CPolySingletonList::instance@1956` is
  `vota::GetQRDSInst` (u19).
* The five "free functions #2..#7" groups of `unit.py` are only groupings by address, not classes.

## 7. Corrections to other units' reconstructions

* `cbiometriaeleitor.h` (u05): `m_dedos` is `std::optional<std::map<CDedo::TipoDedo, CDedo>>` (engaged flag +32),
  not a plain map plus a flag.
* `capplicationcontextstack.u15.h`: the member at +48 of `CApplicationContext` is stored, copied and compared as a
  4-byte integer (`a[12]:int = f` in 5557; `i32.eq` in 5556), not as a `bool`.
* `cregistrarmesarios.cpp` (u22) writes `b.CriaFormInterativo("", std::make_shared<api::CPreShowClearScreen>())` for
  func 11140. The function is the non-interactive `api::CriaForm<api::CPreShowClearScreen>(builder, nome)`. It allocates
  the pre-show with `new` into a `shared_ptr` (control block `__shared_ptr_pointer`, not `make_shared`) and wraps a plain
  `api::IForm<IScreen>` (72 bytes, func 5550).
* Func 3784 is referred to as `CCargos::Inicio()` in `cgerabu.cpp`, `cgeradorbu.cpp` and `cgravaresultado.cpp`. It
  copies `m_todos` into `m_cargos` and does not rewind the cursor, so this unit names it `LimpaFiltro()`.
* Func 1277, used by `AssinaDadosDinamicos`, is `CAssinador::Assina(ESavdArquivoUE)` (as u23 wrote). The analyzer's name
  `comum::AssinarUE` is the inlined callee.
* `gravadores/md/cenvelopegenerico.cpp` (u23), `CEnvelopeGenerico::ValidaCriacao` (3822): the tipo-de-urna test is written
  `if (static_cast<int>(m_tipoUrna) <= '0')`. The wasm is `i32.load offset=220; i32.const 53; i32.sub; i32.const -5;
  i32.le_u`, an **unsigned** test that throws 8682 for every value outside '1'..'4' (source form:
  `m_tipoUrna < EUrnaTipo{'1'} || m_tipoUrna > EUrnaTipo{'4'}`). The same unsigned idiom (`i32.gt_u`) validates the
  tipo in `CIdentificacaoUrna::CIdentificacaoUrna` (5886). The other tests of that function are right (fase: `== '0' ||
  >= '4'` signed; tipo de envelope: `>= 5` signed).

## 8. Suspicious or noteworthy code (summary)

| # | where | what | impact |
|---|---|---|---|
| 1 | 11474 | plural made by `pop_back()` on the format string when count == 1. There is no emptiness check, so an empty format would be UB (write one byte before the buffer), and a format whose last character is not the plural `s` would lose it | printed BUJ text only; the format is the constant `"{:05} Justificativas"`, so the UB case cannot happen today |
| 2 | 5861 / 3821 | `ValidaCriacao()` runs **before** the local is discarded for a contingency urna ('2'), so an irrelevant out-of-range local still aborts the envelope (8681). Confirmed in the code order of both constructors. We did not check whether any caller passes an engaged local for a '2' urna | contingency urnas on the real machine; not the simulator |
| 3 | 6010 | empty título and invalid título share the same body and log call (slot 26 = `CControladorRegistraMesariosVota::vf26`, "Digitado título inválido para o registro de mesário") | log precision of the mesário registration |
| 4 | 6008 | mesário name cut to 40 **bytes** (`substr`), which can split a UTF-8 character; the fallback (not a voter of the section) returns the título without applying the format | MT display only |
| 5 | 5636 | the urna-state QR code publishes the carga signature (hex) and the media generator's host name / TPM-certificate serial / installation serial | by design (shown on screen and printed); fixture values in the simulator |
| 6 | 4668 | the RDV and database "signatures" go to the no-op `CWasmSavd` | simulator: security simulated (known) |
| 7 | 5888 | validation deleted by constant propagation; orphan srcloc and message | none (behaviour unchanged); explains a misleading tool annotation |
| 8 | 34 + 6 atexit stubs | static destructors never registered (`EXIT_RUNTIME=0`) | none in the browser |
| 9 | 5905 | id 120 has an empty name (table entry @1551800 → 450187, the NUL of `" \t"`), so an error about that file would print "…arquivo: " followed by nothing. The name is computed before every signature (5 times in `votaInit`), not only on error | cosmetic |

## 9. Complete mapping table (101 functions)

`ran` = executed: `yes` = seen by the sampling profiler during the recorded votes, `yes*` = not sampled but counted by the entry-counter runs of the introduction (votaInit + one voter, five scenarios). Paths are original paths. "(u05)" etc. means the owner
unit already wrote the function in that file. `library` = C++ library code, not reconstructed (listed in
`src/uenux2/src/app/comum/u36-foreign-fragments.cpp`).

| idx | size | ran | reconstructed symbol | original file | reconstruction | conf. |
|---:|---:|:-:|---|---|---|---|
| 3368 | 374 |  | `std::__split_buffer<std::filesystem::__dir_stream*, std::allocator<std::filesystem::__dir_stream*>>::push_back` | libcxx/include/__split_buffer | library (listed in src/uenux2/src/app/comum/u36-foreign-fragments.cpp) | medium |
| 3510 | 167 |  | `comum::md::(anonymous namespace)::RemoveZerosEsquerda` | uenux2/src/app/comum/dados/md/eleitor/celeitoridentidade.cpp | src/uenux2/src/app/comum/dados/md/eleitor/celeitoridentidade.cpp (u05) | high |
| 3512 | 59 | yes* | `comum::md::(anonymous namespace)::PreencheZerosEsquerda` | uenux2/src/app/comum/dados/md/eleitor/celeitoridentidade.cpp | src/uenux2/src/app/comum/dados/md/eleitor/celeitoridentidade.cpp (u05) | high |
| 3719 | 101 |  | `comum::md::CBiometriaEleitor::PossuiDedo` | uenux2/src/app/comum/dados/md/eleitor/cbiometriaeleitor.cpp | src/uenux2/src/app/comum/dados/md/eleitor/cbiometriaeleitor.u36.cpp | medium |
| 3752 | 861 |  | `comum::CLocal::GetAgregadas` | uenux2/src/app/comum/dados/clocal.cpp | src/uenux2/src/app/comum/dados/clocal.u36.cpp | high |
| 3753 | 106 |  | `comum::CLocal::GetUFMinuscula` | uenux2/src/app/comum/dados/clocal.cpp | src/uenux2/src/app/comum/dados/clocal.u36.cpp | medium |
| 3774 | 389 | yes | `comum::CConfiguracaoEleicao::GetEleicoesCargos` | uenux2/src/app/comum/dados/cconfiguracaoeleicao.cpp | src/uenux2/src/app/comum/dados/cconfiguracaoeleicao.u36.cpp | medium |
| 3782 | 86 |  | `comum::CCargos::OrdenaPorOrdemImpressao` | uenux2/src/app/comum/dados/ccargos.cpp | src/uenux2/src/app/comum/dados/ccargos.cpp (u04) | medium |
| 3784 | 34 |  | `comum::CCargos::LimpaFiltro` | uenux2/src/app/comum/dados/ccargos.cpp | src/uenux2/src/app/comum/dados/ccargos.u36.cpp | medium |
| 3794 | 15 |  | `comum::CPath::GetPathWsqOperadorExterno` | uenux2/src/app/comum/cpath.cpp | src/uenux2/src/app/comum/cpath.u36.cpp | low |
| 3821 | 399 |  | `comum::md::CEnvelopeGenerico::CEnvelopeGenerico` | uenux2/src/app/comum/gravadores/md/cenvelopegenerico.cpp | src/uenux2/src/app/comum/gravadores/md/cenvelopegenerico.u36.cpp | high |
| 3834 | 64 |  | `comum::CPath::GetRaiz` | uenux2/src/app/comum/cpath.cpp | src/uenux2/src/app/comum/cpath.cpp (u22) | medium |
| 3870 | 217 | yes | `std::map<unsigned int, std::string>::__emplace_hint_unique_key_args` | libcxx/include/__tree | library (listed in u36-foreign-fragments.cpp) | medium |
| 3894 | 58 |  | `comum::dao::CComparecimentoMesarioDAO::Clonar [merged body: +CJustificadorDAO, CEleitorDinamicoDAO]` | uenux2/src/app/comum/comparecimentomesario/dao/ccomparecimentomesariodao.cpp | src/uenux2/src/app/comum/{comparecimentomesario,justificativa,dados}/dao/*.cpp (u20/u24/u05) + u36-foreign-fragments.cpp | high |
| 3896 | 223 | yes* | `comum::CServicoEstadoGeralVota::GetPathArquivo [merged body: +Gap, SA]` | uenux2/src/app/comum/appinfo/servicos/cservicoestadogeralvota.cpp | src/uenux2/src/app/comum/appinfo/servicos/cservicoestadogeral*.cpp (u20) + u36-foreign-fragments.cpp | high |
| 3898 | 182 |  | `comum::CPath::GetPathWsqExterno [merged body]` | uenux2/src/app/comum/cpath.cpp | src/uenux2/src/app/comum/cpath.u36.cpp | low |
| 4668 | 66 | yes | `vota::(anonymous namespace)::AssinaDadosDinamicos` | uenux2/src/app/vota/eleitor/comum/cinformacaoeleitor.cpp | src/uenux2/src/app/vota/eleitor/comum/cinformacaoeleitor.u36.cpp | low |
| 4778 | 34 |  | `std::unique_ptr<std::filesystem::__dir_stream, std::__allocator_destructor<std::allocator<std::filesystem::__dir_stream>>>::~unique_ptr` | libcxx/include/deque | library (u36-foreign-fragments.cpp) | medium |
| 4779 | 48 |  | `std::deque<std::filesystem::__dir_stream>::__back_spare` | libcxx/include/deque | library (u36-foreign-fragments.cpp) | medium |
| 5168 | 459 |  | `ecourna::api::cepesc::CPlainText::CPlainText` | ecourna-lib/ecourna/api/security/cepesc/cplaintext.cpp | src/ecourna/api/security/cepesc/cplaintext.u36.cpp | medium |
| 5383 | 22 |  | `comum::CGestorDadoMesario::GetInst` | uenux2/src/app/comum/comparecimentomesario/estados/cgestordadomesario.cpp | src/uenux2/src/app/comum/comparecimentomesario/estados/cgestordadomesario.cpp (u22) | high |
| 5552 | 349 |  | `api::CApplicationContext::operator=` | uenux2/src/api/gui/capplicationcontextstack.h (header inferred) | src/uenux2/src/api/gui/capplicationcontextstack.u36.cpp | medium |
| 5556 | 387 | yes | `api::CApplicationContext::operator==` | uenux2/src/api/gui/capplicationcontextstack.h (header inferred) | src/uenux2/src/api/gui/capplicationcontextstack.u36.cpp | medium |
| 5622 | 942 |  | `comum::CCabecalhoQRCodeBuilder::SetHistoricoCargas` | uenux2/src/app/comum/relatorios/ccabecalhoqrcodebuilder.cpp | src/uenux2/src/app/comum/relatorios/ccabecalhoqrcodebuilder.u36.cpp | medium |
| 5624 | 14 |  | `comum::CCabecalhoQRCodeBuilder::CCabecalhoQRCodeBuilder` | uenux2/src/app/comum/relatorios/ccabecalhoqrcodebuilder.cpp | src/uenux2/src/app/comum/relatorios/ccabecalhoqrcodebuilder.u36.cpp | medium |
| 5636 | 3903 | yes | `comum::MontaCamposQRCodeEstadoUrna` | uenux2/src/app/comum/relatorios/cqrcodeds.cpp (path inferred) | src/uenux2/src/app/comum/relatorios/cqrcodeds.cpp | medium |
| 5653 | 75 | yes* | `comum::md::CEleicaoPE::PossuiCargoEletivo` | uenux2/src/app/comum/dados/md/processoeleitoral/celeicaope.cpp | src/uenux2/src/app/comum/dados/md/processoeleitoral/celeicaope.u36.cpp | medium |
| 5681 | 350 | yes* | `std::map<comum::md::SCargoInfo, comum::md::CVotos>::__emplace_hint_unique_key_args` | libcxx/include/__tree | library (u36-foreign-fragments.cpp) | medium |
| 5717 | 745 |  | `std::__sort5<std::_ClassicAlgPolicy, (lambda)&, comum::md::CIdentificacaoAgregada*>` | libcxx/include/__algorithm/sort.h | library (u36-foreign-fragments.cpp) | medium |
| 5718 | 1427 |  | `std::__insertion_sort_incomplete<std::_ClassicAlgPolicy, (lambda)&, comum::md::CIdentificacaoAgregada*>` | libcxx/include/__algorithm/sort.h | library (u36-foreign-fragments.cpp) | medium |
| 5719 | 4179 |  | `std::__introsort<std::_ClassicAlgPolicy, (lambda)&, comum::md::CIdentificacaoAgregada*, false>` | libcxx/include/__algorithm/sort.h | library (u36-foreign-fragments.cpp) | medium |
| 5726 | 35 | yes* | `comum::SIdentificacaoCarga::SIdentificacaoCarga` | uenux2/src/app/comum/nomearquivo/cnomearquivo.h (header inferred) | src/uenux2/src/app/comum/nomearquivo/cnomearquivo.u36.cpp | low |
| 5744 | 147 | yes* | `comum::md::CLocal::~CLocal` | uenux2/src/app/comum/dados/md/clocal.cpp | u36-foreign-fragments.cpp (implicit dtor) | high |
| 5760 | 277 |  | `comum::md::CEleitorDinamico::CEleitorDinamico` | uenux2/src/app/comum/dados/md/eleitor/celeitordinamico.h (header inferred) | u36-foreign-fragments.cpp (implicit copy ctor) | high |
| 5763 | 695 | yes* | `std::pair<const comum::md::CEleitorIdentidade, comum::CEleitorDetalhe>::pair` | libcxx/include/__utility/pair.h | library (u36-foreign-fragments.cpp) | medium |
| 5783 | 238 | yes* | `comum::AdicionaOrigemQRCode` | uenux2/src/app/comum/relatorios/cqrcodeds.cpp (path inferred) | src/uenux2/src/app/comum/relatorios/cqrcodeds.cpp | medium |
| 5816 | 15 |  | `comum::CPath::GetPathWsqNaoHabilitadoExterno` | uenux2/src/app/comum/cpath.cpp | src/uenux2/src/app/comum/cpath.u36.cpp | low |
| 5817 | 15 |  | `comum::CPath::GetPathWsqHabilitadoExterno` | uenux2/src/app/comum/cpath.cpp | src/uenux2/src/app/comum/cpath.u36.cpp | low |
| 5861 | 289 |  | `comum::md::CEnvelopeGenerico::CEnvelopeGenerico` | uenux2/src/app/comum/gravadores/md/cenvelopegenerico.cpp | src/uenux2/src/app/comum/gravadores/md/cenvelopegenerico.u36.cpp | high |
| 5888 | 33 |  | `comum::md::CIdentificacaoUrnaContingencia::CIdentificacaoUrnaContingencia` | uenux2/src/app/comum/md/cidentificacaournacontingencia.cpp | src/uenux2/src/app/comum/md/cidentificacaournacontingencia.cpp | high |
| 5905 | 47 | yes* | `comum::NomeArquivoSavd` | uenux2/src/app/comum/iinterfacesavd.cpp | src/uenux2/src/app/comum/iinterfacesavd.u36.cpp | medium |
| 5956 | 230 |  | `std::vector<comum::md::CMunicipioDisponivel>::__destroy_vector::operator()` | libcxx/include/__vector/vector.h | library (u36-foreign-fragments.cpp) | medium |
| 6007 | 99 |  | `comum::CRegistradorMesario::QuantidadeRegistrados` | uenux2/src/app/comum/comparecimentomesario/cregistradormesario.cpp (path inferred) | src/uenux2/src/app/comum/comparecimentomesario/cregistradormesario.u36.cpp | medium |
| 6008 | 1045 |  | `comum::(anonymous namespace)::CNomeMesariosUrnaDS::Text [merged body]` | uenux2/src/app/comum/comparecimentomesario/estados/cpededigitalmesario.cpp | src/uenux2/src/app/comum/comparecimentomesario/estados/{cpededigitalmesario,cdigitalmesarionaoreconhecida}.cpp (u22) + u36-foreign-fragments.cpp | high |
| 6009 | 603 |  | `comum::(anonymous namespace)::CTituloMesarioInvalidoDS::Text [merged body: +CTituloMesarioJaRegistradoDS]` | uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesarioinvalido.cpp | src/uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesario{invalido,jaregistrado}.cpp (u22) + u36-foreign-fragments.cpp | high |
| 6010 | 102 |  | `comum::CTituloMesarioInvalido::StartState [merged body: +CTituloMesarioVazio]` | uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesarioinvalido.cpp | src/uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesario{invalido,vazio}.cpp (u22) + u36-foreign-fragments.cpp | high |
| 6030 | 69 | yes* | `comum::asn::CConversorTipoIdentificadorEleitor::DoDesconverte [merged body: +CConversorOrigemConfiguracao]` | uenux2/src/app/comum/dados/asn/processoeleitoral/cconversortipoidentificadoreleitor.cpp | src/uenux2/src/app/comum/dados/asn/processoeleitoral/cconversor{tipoidentificadoreleitor,origemconfiguracao}.cpp (u03) + u36-foreign-fragments.cpp | high |
| 6043 | 55 |  | `comum::CGravadorRDV::~CGravadorRDV [merged body: +IGravadorEnvelope]` | uenux2/src/app/comum/gravadores/cgravadorrdv.cpp (path inferred) | u36-foreign-fragments.cpp (merged dtor body) | high |
| 6044 | 571 |  | `comum::md::CVersoesContratos::GetValor [merged body: +CDependenciasContratos]` | uenux2/src/app/comum/gravadores/md/cversoescontratos.cpp | src/uenux2/src/app/comum/gravadores/md/c{versoes,dependencias}contratos.cpp (u23) + u36-foreign-fragments.cpp | high |
| 6048 | 335 | yes* | `comum::CPath::GetPathRoot` | uenux2/src/app/comum/cpath.cpp | src/uenux2/src/app/comum/cpath.cpp (u22) | medium |
| 7765 | 11 |  | `std::__time_get_c_storage<char>::__x()::s (atexit dtor)` | libcxx/src/locale.cpp | library (u36-foreign-fragments.cpp) | high |
| 7768 | 30 |  | `std::__time_get_c_storage<wchar_t>::__am_pm()::am_pm (atexit dtor)` | libcxx/src/locale.cpp | library (u36-foreign-fragments.cpp) | high |
| 7770 | 30 |  | `std::__time_get_c_storage<char>::__am_pm()::am_pm (atexit dtor)` | libcxx/src/locale.cpp | library (u36-foreign-fragments.cpp) | high |
| 7772 | 30 |  | `std::__time_get_c_storage<wchar_t>::__months()::months (atexit dtor)` | libcxx/src/locale.cpp | library (u36-foreign-fragments.cpp) | high |
| 7774 | 30 |  | `std::__time_get_c_storage<char>::__months()::months (atexit dtor)` | libcxx/src/locale.cpp | library (u36-foreign-fragments.cpp) | high |
| 7776 | 30 |  | `std::__time_get_c_storage<wchar_t>::__weeks()::weeks (atexit dtor)` | libcxx/src/locale.cpp | library (u36-foreign-fragments.cpp) | high |
| 8097 | 107 |  | `std::deque<std::filesystem::__dir_stream>::pop_back` | libcxx/include/deque | library (u36-foreign-fragments.cpp) | medium |
| 8099 | 1118 |  | `std::deque<std::filesystem::__dir_stream>::push_back` | libcxx/include/deque | library (u36-foreign-fragments.cpp) | medium |
| 9404 | 608 | yes | `std::vector<std::string>::__emplace_back_slow_path<const std::string&, std::size_t&, std::size_t&>` | libcxx/include/__vector/vector.h | library (u36-foreign-fragments.cpp) | medium |
| 9873 | 54 |  | `std::_AllocatorDestroyRangeReverse<std::allocator<comum::md::estadoaplicacao::CDadoCorrespondencia>, comum::md::estadoaplicacao::CDadoCorrespondencia*>::operator()` | libcxx/include/__memory/uninitialized_algorithms.h | library (u36-foreign-fragments.cpp) | medium |
| 10274 | 39 |  | `comum::asn::(anonymous namespace)::s_hashAnterior (atexit dtor)` | uenux2/src/app/comum/gravadores/asn/cassinavotavelbu.cpp | u36-foreign-fragments.cpp (atexit list) | medium (object certain: vector @1910004 of 10273; the variable name was invented by u23) |
| 10320 | 10 |  | `comum::CPedeDigitalMesario::GetInst()::mutex (atexit dtor)` | uenux2/src/app/comum/comparecimentomesario/estados/cpededigitalmesario.cpp | u36-foreign-fragments.cpp (atexit list) | high |
| 10321 | 12 |  | `comum::CPedeDigitalMesario::GetInst()::s_inst (atexit dtor)` | uenux2/src/app/comum/comparecimentomesario/estados/cpededigitalmesario.cpp | u36-foreign-fragments.cpp (atexit list) | high |
| 10334 | 10 |  | `comum::CGestorDadoMesarioInicial::GetInst()::mutex (atexit dtor)` | uenux2/src/app/comum/comparecimentomesario/estados/cgestordadomesario.cpp | u36-foreign-fragments.cpp (atexit list) | high |
| 10336 | 12 |  | `comum::CGestorDadoMesarioInicial::GetInst()::s_inst (atexit dtor)` | uenux2/src/app/comum/comparecimentomesario/estados/cgestordadomesario.cpp | u36-foreign-fragments.cpp (atexit list) | high |
| 10346 | 12 |  | `comum::CTituloMesarioJaRegistrado::GetInst()::s_inst (atexit dtor)` | uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesariojaregistrado.cpp | u36-foreign-fragments.cpp (atexit list) | high |
| 10350 | 10 |  | `comum::CTituloMesarioInvalido::GetInst()::mutex (atexit dtor)` | uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesarioinvalido.cpp | u36-foreign-fragments.cpp (atexit list) | high |
| 10351 | 12 |  | `comum::CTituloMesarioInvalido::GetInst()::s_inst (atexit dtor)` | uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesarioinvalido.cpp | u36-foreign-fragments.cpp (atexit list) | high |
| 10354 | 10 |  | `comum::CTituloMesarioVazio::GetInst()::mutex (atexit dtor)` | uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesariovazio.cpp | u36-foreign-fragments.cpp (atexit list) | high |
| 10355 | 12 |  | `comum::CTituloMesarioVazio::GetInst()::s_inst (atexit dtor)` | uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesariovazio.cpp | u36-foreign-fragments.cpp (atexit list) | high |
| 10371 | 10 |  | `comum::CPedeTituloMesarioFinal::GetInst()::mutex (atexit dtor)` | uenux2/src/app/comum/comparecimentomesario/estados/cpedetitulomesariofinal.cpp | u36-foreign-fragments.cpp (atexit list) | high |
| 10378 | 10 |  | `comum::CPedeTituloMesarioInicial::GetInst()::mutex (atexit dtor)` | uenux2/src/app/comum/comparecimentomesario/estados/cpedetitulomesarioinicial.cpp | u36-foreign-fragments.cpp (atexit list) | high |
| 10381 | 10 |  | `comum::CEncerraRegistroMesarios::GetInst()::mutex (atexit dtor)` | uenux2/src/app/comum/comparecimentomesario/estados/cencerraregistromesarios.cpp | u36-foreign-fragments.cpp (atexit list) | high |
| 10382 | 12 |  | `comum::CEncerraRegistroMesarios::GetInst()::s_inst (atexit dtor)` | uenux2/src/app/comum/comparecimentomesario/estados/cencerraregistromesarios.cpp | u36-foreign-fragments.cpp (atexit list) | high |
| 10386 | 10 |  | `comum::CConfirmaFimRegistroMesarios::GetInst()::mutex (atexit dtor)` | uenux2/src/app/comum/comparecimentomesario/estados/cconfirmafimregistromesarios.cpp | u36-foreign-fragments.cpp (atexit list) | high |
| 10387 | 12 |  | `comum::CConfirmaFimRegistroMesarios::GetInst()::s_inst (atexit dtor)` | uenux2/src/app/comum/comparecimentomesario/estados/cconfirmafimregistromesarios.cpp | u36-foreign-fragments.cpp (atexit list) | high |
| 10389 | 10 |  | `comum::CPedeTituloMesario::GetInst()::mutex (atexit dtor)` | uenux2/src/app/comum/comparecimentomesario/estados/cpedetitulomesario.cpp | u36-foreign-fragments.cpp (atexit list) | high |
| 10390 | 12 |  | `comum::CPedeTituloMesario::GetInst()::s_inst (atexit dtor)` | uenux2/src/app/comum/comparecimentomesario/estados/cpedetitulomesario.cpp | u36-foreign-fragments.cpp (atexit list) | high |
| 10393 | 10 |  | `comum::CRegistrarMesarios::GetInst()::mutex (atexit dtor)` | uenux2/src/app/comum/comparecimentomesario/estados/cregistrarmesarios.cpp | u36-foreign-fragments.cpp (atexit list) | high |
| 10394 | 12 |  | `comum::CRegistrarMesarios::GetInst()::s_inst (atexit dtor)` | uenux2/src/app/comum/comparecimentomesario/estados/cregistrarmesarios.cpp | u36-foreign-fragments.cpp (atexit list) | high |
| 10519 | 10 |  | `ConfiguracaoExtrator::GetInst()::mutex (atexit dtor)` | (file unknown: fingerprint-extractor configuration singleton, vota_f1903) | u36-foreign-fragments.cpp (atexit list) | low (object certain; the class name is a placeholder, no string or RTTI gives it) |
| 10520 | 12 |  | `ConfiguracaoExtrator::GetInst()::s_inst (atexit dtor)` | (file unknown: fingerprint-extractor configuration singleton, vota_f1903) | u36-foreign-fragments.cpp (atexit list) | low (object certain; the class name is a placeholder, no string or RTTI gives it) |
| 10521 | 10 |  | `CodificadorWsq::GetInst()::mutex (atexit dtor)` | (file unknown: WSQ encoder singleton, comum_f2742) | u36-foreign-fragments.cpp (atexit list) | low (object certain; the class name is a placeholder, no string or RTTI gives it) |
| 10522 | 12 |  | `CodificadorWsq::GetInst()::s_inst (atexit dtor)` | (file unknown: WSQ encoder singleton, comum_f2742) | u36-foreign-fragments.cpp (atexit list) | low (object certain; the class name is a placeholder, no string or RTTI gives it) |
| 10877 | 12 |  | `api::CIniSection::Find` | uenux2/src/api/io/cinisection.cpp | src/uenux2/src/api/io/cinisection.u36.cpp | low |
| 11140 | 21 |  | `api::CriaForm<api::CPreShowClearScreen>` | uenux2/src/api/gui/cformbuilder.h (header inferred) | u36-foreign-fragments.cpp (template instance); used in comparecimentomesario/estados/cregistrarmesarios.cpp | medium |
| 11174 | 10 |  | `comum::CSubstituidorTitulo::s_mutex (atexit dtor)` | uenux2/src/app/comum/relatorios/csubstituidortitulo.cpp | u36-foreign-fragments.cpp (atexit list) | high |
| 11268 | 39 |  | `comum::md::estadoaplicacao::CEstadoGeral::RecuperarCertificado()::certificado (atexit dtor)` | uenux2/src/app/comum/dados/md/estadoaplicacao/cestadogeral.cpp | u36-foreign-fragments.cpp (atexit list) | high |
| 11315 | 18 |  | `comum::md::CVoto::EhBranco` | uenux2/src/app/comum/dados/md/rdv/cvoto.cpp | src/uenux2/src/app/comum/dados/md/rdv/cvoto.u36.cpp | medium |
| 11474 | 695 |  | `comum::CJustificadorDadoQuantidade::Text` | uenux2/src/app/comum/justificativa/cjustificador.cpp | src/uenux2/src/app/comum/justificativa/cjustificador.u36.cpp | low |
| 11480 | 10 |  | `comum::CRespostas::s_mutex (atexit dtor)` | uenux2/src/app/comum/dados/crespostas.cpp | u36-foreign-fragments.cpp (atexit list) | high |
| 11499 | 531 | yes | `comum::CPartidosDSNumero::Text` | uenux2/src/app/comum/dados/cpartidos.cpp (path inferred) | src/uenux2/src/app/comum/dados/cpartidos.u36.cpp | low |
| 11501 | 573 | yes* | `comum::CPartidosDSSigla::Text` | uenux2/src/app/comum/dados/cpartidos.cpp (path inferred) | src/uenux2/src/app/comum/dados/cpartidos.u36.cpp | low |
| 11531 | 120 |  | `comum::(anonymous namespace)::ComparaDinamico` | uenux2/src/app/comum/dados/celeitores.cpp | src/uenux2/src/app/comum/dados/celeitores.cpp (u04) | high |
| 11639 | 10 |  | `comum::CLogComum::s_mutex (atexit dtor)` | uenux2/src/app/comum/log/clogcomum.cpp | u36-foreign-fragments.cpp (atexit list) | high |
| 11640 | 12 |  | `comum::CLogComum::s_pInstancia (atexit dtor)` | uenux2/src/app/comum/log/clogcomum.cpp | u36-foreign-fragments.cpp (atexit list) | high |
| 11648 | 36 |  | `comum::CPath::ms_raiz (atexit dtor)` | uenux2/src/app/comum/cpath.cpp | u36-foreign-fragments.cpp (atexit list); cpath.h | high |
| 11649 | 70 |  | `comum::CPath::ms_raizesFlash (atexit dtor)` | uenux2/src/app/comum/cpath.cpp | u36-foreign-fragments.cpp (atexit list); cpath.h | high |
| 11657 | 10 |  | `comum::CArquivosSavd::GetInst()::s_mutex (atexit dtor)` | uenux2/src/app/comum/carquivossavd.cpp | u36-foreign-fragments.cpp (atexit list) | high |
| 11659 | 10 |  | `comum::CArquivosResultado::GetInst()::s_mutex (atexit dtor)` | uenux2/src/app/comum/carquivosresultado.cpp | u36-foreign-fragments.cpp (atexit list) | high |
| 11660 | 12 |  | `comum::CArquivosResultado::GetInst()::s_inst (atexit dtor)` | uenux2/src/app/comum/carquivosresultado.cpp | u36-foreign-fragments.cpp (atexit list) | high |

## 10. Open questions

* The exact meaning of `ORIG:T` / `ORIG:I` in the urna-state QR code (tela / impresso is a guess) and why variante 2
  omits it.
* Whether the three external WSQ directory functions (3794/5816/5817) are `CPath` members or free functions of a
  biometrics helper. They are called from both `comum` and `vota`, so they must be declared in a shared header.
* The type of the 4-byte member at `CApplicationContext +48`.
* The error class and code of the deleted check in `CIdentificacaoUrnaContingencia` (line 32).
* Periods 1 and 2 of `CRegistradorMesario::QuantidadeRegistrados`: inicial / final is inferred from the callers (MT
  counters, BIM report). A third period (votação) exists in the state names, but no thunk uses it.
* Names of the fingerprint-extractor configuration singleton (vota_f1903: `{8, 500, 196.85}` = 8 bits per pixel?,
  500 dpi, 500/2.54 pixels per cm) and of the WSQ-encoder singleton (2742), whose atexit handlers are 10519-10522.
