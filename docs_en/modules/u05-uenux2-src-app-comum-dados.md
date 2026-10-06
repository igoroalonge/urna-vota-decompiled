# u05 — `uenux2/src/app/comum/dados` (part 3): RDV, voter identity, domain objects, `eleitor_dinamico`, label translation

Unit u05 has 100 wasm functions. They come from 21 original files of the shared `comum` ("common") data
layer, plus stray functions that the unit builder attached to it (template instantiations, bodies
merged by wasm-opt, and a few functions of neighbouring files). 14 of them ran during the recorded
votes: 566, 654, 753, 946, 3509, 5157, 5158, 5660, 5770, 5771, 5772, 11272, 11378 and 11488. All of them
run during `votaInit` (loading the election data and creating `rdv.dat`) or while screen texts are
translated.

Reconstructed sources (all under `src/`):

| file | contents |
|---|---|
| `uenux2/src/app/comum/dados/crdvvota.{h,cpp}` | `comum::CRdv` interface sketch, `comum::CRdvVota`, and the inlined `CVotosEleicoesVota` bodies |
| `uenux2/src/app/comum/dados/crespostas.{h,cpp}` | `comum::CRespostas::GetInst` |
| `uenux2/src/app/comum/dados/dao/celeitordinamicodao.{h,cpp}` | `comum::dao::CEleitorDinamicoDAO` (SQLite table `eleitor_dinamico`) |
| `uenux2/src/app/comum/dados/md/candidatura/ccandidatura.{h,cpp}` | `CCandidatura` |
| `uenux2/src/app/comum/dados/md/{clocal,cinfomunicipio,chorarioveraomunicipio}.{h,cpp}` | location and municipality objects |
| `uenux2/src/app/comum/dados/md/correspondencia/{ccarga,ccorrespondenciaresultado}.{h,cpp}` | result-file correspondence |
| `uenux2/src/app/comum/dados/md/{cvalidadoridentidade,iregraidentidade}.{h,cpp}`, `cregracpf.cpp`*, `cregratitulo.cpp`* | identifier validation |
| `uenux2/src/app/comum/dados/md/eleitor/{celeitor,celeitordecorator,celeitordinamico,cbiometriaeleitor}.{h,cpp}`, `celeitoridentidade.{h,cpp}`* | voter records |
| `uenux2/src/app/comum/dados/md/estadoaplicacao/{cajustedatahora,cdadocarga,cestadogeral,clocalidadeeleitoral}.{h,cpp}` | application state objects |
| `uenux2/src/app/comum/dados/md/municipiozona/{ccomplementomunicipio,cmunicipio}.{h,cpp}` | municipality |
| `uenux2/src/app/comum/dados/md/parametrizacaourna/ctradutorfrase.{h,cpp}` | placeholder translator |
| `uenux2/src/app/vota/operador/comum/capresentacaofotoeleitor.cpp`* | voter photo on the poll worker's display (func 3616 and helpers) |
| `uenux2/src/api/util/cajustedatahora.cpp`* | `api::CAjusteDataHora` (a different class that has the same name) |
| `uenux2/src/app/comum/dados/u05-foreign-fragments.cpp` | functions of this unit that belong to other units' files (§12) and the library instantiations |

`*` = path inferred (no `std::source_location` names this file). The unit also has functions whose
original file belongs to another unit (u04, u20, u21, u22, u25, u27, u35). They are **not** written into
those units' files. Following the fragment convention used by u04, they are in
`src/uenux2/src/app/comum/dados/u05-foreign-fragments.cpp`, one section per original file, and are
summarised in section 12.

## 1. What this part of the program does (with the Portuguese terms)

`comum/dados` holds the **domain model** ("md" = *modelo de domínio*) that the VOTA application loads from
the election media, plus a few services built on it:

* **RDV, *Registro Digital do Voto*** (digital record of the vote): the per-election, per-office list of
  votes. `comum::CRdvVota` is the in-memory RDV that the BU and zerésima reports read, and it is
  serialised to `rdv.dat`.
* **Identifier validation**: *título de eleitor* (the 12-digit voter card number, with a state code and
  two check digits) and **CPF** (*Cadastro de Pessoas Físicas*, the 11-digit taxpayer number). Every
  voter or poll-worker (*mesário*) identity built anywhere in the program goes through
  `CEleitorIdentidade`'s constructor, which validates it.
* **Voter records**: `CEleitor` is the static roll entry (*eleitor* = voter, from `*-el.dat`).
  `CEleitorDecorator` is a voter plus its "principal" identifier type. `CEleitorDinamico` is the state
  that changes on election day (*habilitação* = enabling the voter to vote, *comparecimento* = turnout,
  biometric attempts, audio accessibility, photo shown). `CEleitorDinamicoDAO` stores it in SQLite.
* **Biometrics** (`CBiometriaEleitor`: photo + up to ten fingerprints) and the **presentation of the
  voter's photo** on the poll worker's terminal display (*MT*, micro-terminal; func 3616).
* **Validated value objects** loaded from the ASN.1 data files: candidacies (*candidatura*, with
  *suplentes* = substitutes/vice), municipality (*município*), time zone (*fuso horário*) and daylight
  saving time (*horário de verão*), location (*local*: country, state/UF, *seção* = polling section,
  *urna de contingência* = reserve machine), load identification (*carga*, *correspondência de
  resultado*), load data (*dado de carga*: *turno* = round, model, *fase* = phase), and clock adjustment
  (*ajuste de data/hora*).
* **`CTradutorFrase`** (phrase translator) expands placeholders in every text on screens and reports. It
  uses labels and grammatical gender from the urna parametrisation file (`*-pu.dat`).

## 2. Classes and hierarchy (RTTI)

```
comum::CRdv                     (abstract, 14 pure virtuals, no virtual destructor)   vtable @1559900
 └─ comum::CRdvVota             vtable @1560172, singleton @1838956, sizeof 100
      has: comum::md::CVotosEleicoesVota m_votos (+20)
           comum::asn::CConversorRegistroDigitalVoto<comum::asn::CConversorEleicoesVota> (+44)
             : comum::asn::IConversorASN<ModuloRegistroDigitalVoto::EntidadeRegistroDigitalVoto,
                                        comum::md::CVotosEleicoesVota>
      uses: comum::CRdvPosicionadorVota : comum::md::CRdvPosicionador  (vtable @1560252)

api::persistencia::IDAO
 └─ api::persistencia::IUenuxGenericDAO<comum::md::CEleitorDinamico, std::string>
     └─ comum::dao::IEleitorDinamicoDAO
         └─ comum::dao::CEleitorDinamicoDAO   vtable @1558904
            (siblings with the same slot layout: CJustificadorDAO, CComparecimentoMesarioDAO)

comum::md::IRegraIdentidade
 ├─ comum::md::CRegraTitulo            vtable @1574700
 ├─ comum::md::CRegraCPF               vtable @1574740
 └─ comum::md::CRegraIdentidadeLivre   vtable @1574652 (unit u35)
comum::md::CValidadorIdentidade        singleton @1839028, vector<unique_ptr<IRegraIdentidade>>

api::IAjusteDataHora
 └─ api::CAjusteDataHora               vtable @1584164  (calls api::ISystemDateTime; in the web build
                                        simulador::CWasmSystemDateTime)
```

The other classes of the unit have no vtable and are plain value types: `CEleitorIdentidade` (16 B),
`CEleitor` (104 B), `CEleitorDecorator` (108 B), `CEleitorDinamico` (84 B), `CBiometriaEleitor`,
`CCandidatura` (72 B), `CDadosCandidato` (52 B), `CCarga` (76 B), `CCorrespondenciaResultado` (88 B),
`CLocal`, `CMunicipio`, `CInfoMunicipio`, `CComplementoMunicipio`, `CHorarioVeraoMunicipio`, `CDadoCarga`
(20 B), `CAjusteDataHora` (8 B), `CLocalidadeEleitoral` (8 B), `CTradutorFrase` (all static).

The errors they throw are `ecourna::api::exception::CBaseError<E, SErrorLimits{lo, hi}>`:
`comum::EUeComumDadosError` {7800..8600} (typeinfo @1528076, thrown through `comum_f170`),
`api::EUeRdvError` {4650..4850} (@1559972, through func 655), `comum::EUeComumAsnError`,
`api::EUeIoError`, and `EPatternError` for the singletons. Every throw passes a
`std::source_location`, so the reconstructed code gives the original line numbers.

## 3. The RDV (`CRdvVota`)

### 3.1 Layout and creation

`CRdvVota::CreateInst` (crdvvota.cpp:92) and the constructors of `CRdvVota` and `CRdv` (crdv.cpp) are
fully inlined into the start-up function 7787 (`vota::CInformacaoEleitor::Inicializar`, name inferred in u02;
formerly shown by the tools as `CHKDFSeed::GetSeed`). The
start-up code does this:

1. `new CRdvPosicionadorVota` (the vote "positioner", i.e. the policy that decides where each vote goes in
   the RDV so that the order of voting cannot be recovered).
2. `CRdv::CRdv(posicionador, cargos, CConfiguracaoEleicao +164 = digitosPartido)`. `cargos` is the
   sorted list of office codes of every election. Unit u02 reconstructed this in
   `src/uenux2/src/app/comum/dados/crdv.cpp` and `crdvvota.u02.cpp`:
   * `digitosPartido == 0` → `EUeRdvError 4650 "Número de dígitos do partido nulo"` (crdv.cpp:90);
     `> 5` → 4651 `"Número de dígitos do partido ({}) supera o limite (5)"` (crdv.cpp:93).
   * `(anonymous)::GetCifradorCryptoTable(cargos)` (crdv.cpp:60/78) derives the symmetric cipher that
     `api::CEncryptedFile` uses for `rdv.dat`. The inputs are the SHA-512 of `cargos`, 32 bytes picked
     from the 128-byte `IUrna` crypto table (all `0x03` in the simulator's `CUrnaMock`) and
     `HKDF-SHA512` with info `"RDV"`.
3. The members of `CRdvVota` are initialised. `m_votos` is built from the election configuration: one
   `CVotosCargos` per election, one `(SCargoInfo, CVotos)` per office, where `SCargoInfo` is taken from
   the 140-byte `CCargo` entries (fields +4, +12, and +13 = `qtdEscolhas`, the number of choices).
   `CVotosCargos` throws `4664 "Vetor de cargos vazio"` and `4665 "Cargo duplicado "`. `m_conversor` is
   loaded with the identification fields (município, zona, seção, eleições) that go into the RDV header.
4. If an instance already exists, it throws `4658 "Instancia ja criada"`.

Layout: `+0 vptr`, `+4 posicionador`, `+8 cipher (shared)`, `+16 uebyte digitosPartido`,
`+20 CVotosEleicoesVota {map<TEleicaoID, CVotosCargos> +20; map<TCargoID, TEleicaoID> +32}`,
`+44 converter` (up to +100).

### 3.2 Interface (vtable slot → method)

| slot | func | method | what it returns |
|---|---|---|---|
| 0 | 5733 | `GetVotos()` | `map<TCargoID, CVotos>` over all elections (first election wins if an office repeats) |
| 1 | 11490 | `GetVotos(TEleicaoID)` | same for one election (`"Eleicao (N) nao encontrada"` 4733) |
| 2 | 11492 | `Comparecimento(eleição)` | votes of the first office ÷ `qtdEscolhas` = voters who voted |
| 3 | 1931 | `Candidato(cargo, número, dígitos)` | votes of one candidate (number left-padded with `'0'`) |
| 4 | 3749 | `Legenda(cargo, partido)` | party-only (*voto de legenda*) votes |
| 5 | 2814 | `Partido(cargo, partido)` | all votes of the party (legenda + its candidates) |
| 6 | 2813 | `Nominais(cargo)` | votes for candidates |
| 7 | 5732 | `Legendas(cargo)` | all party-only votes |
| 8 | 3748 | `Nulos(cargo)` | null votes |
| 9 | 3747 | `Brancos(cargo)` | blank votes |
| 10 | 1930 | `Cargo(cargo)` | total of the office ("Total Apurado") |
| 11 | 11488 | `Converte()` | BER bytes of `ModuloRegistroDigitalVoto::EntidadeRegistroDigitalVoto` |
| 12 | 11487 | `Desconverte(bytes)` | loads bytes; `4660 "Conteudo do arquivo eh incompatível com objeto"` if the structure differs |
| 13 | 11486 | `ConfereConteudo(bytes)` | true if the bytes decode to exactly the in-memory RDV |

Slots 3–10 are one-liners forwarding to `m_votos`. LTO inlined the `CVotosEleicoesVota` bodies. wasm-opt
then merged them into shared bodies 2296 (five `TCargoID`-only queries), 6033 (Legenda/Partido) and 3707
(lookup `cargo → eleição`, `"Cargo N nao encontrado"`, codes 4725..4732 per query). The srclocs inside
them point to `cvotoseleicoesvota.cpp:113..190`.

### 3.3 `rdv.dat` lifecycle

* **Creation (`votaInit`)**: `CGeraDadosDinamicos` (6737) calls `Converte()` and writes the result through
  `api::CEncryptedFile::MemWrite` + `Flush` to `rdv.dat` in both flash areas (`/dsk/fi/dinamico/trab1`
  and `/dsk/fe/...`; funcs 5736/5737). That is the 80-byte encrypted file in
  `analysis/runtime/memfs-after-init` (an empty RDV). This is why 11488 appears as "observed".
* **Load**: `CEleitores::CompleteLoad` (6734) does `CEncryptedFile::Load` + `MemRead` → `Desconverte`.
  It then runs `CIntegridadeReferencial` over `CRdvVota`, `CCargos` and `CRespostas` (comum_f2543).
* **Native vote synchronisation** (`vota::impl::CSincronismoVotoEleitor::vf2`, 7174, not used in the web
  build) writes `rdv.dat.tmp` = encrypt(`Converte()`). It reads the file back, decrypts it and calls
  `ConfereConteudo`. Only if that returns true does it call `PrepareReplace` (rename over `rdv.dat`) and
  update the voter's `eleitor_dinamico` row (`CEleitorDinamicoDAO::Atualizar`). This read-back check is the
  RDV's write-integrity check.
* **Adding votes** is not in this unit. `vota::CEleitorVotando::NeedChangeState` (7352) calls func 4454,
  which inlines `CVotosEleicoesVota::RecebeCedula` (cvotoseleicoesvota.cpp:215) →
  `CVotosCargos::InsereCedula` (the positioner chooses the slot) → `CVotos::Insere`, followed by
  `CVotosCargos::ConfereCedula` (it re-reads the ballot from the RDV and compares). The profiler saw 4454
  run in both recorded votes, so **the in-memory RDV does receive every ballot in the simulator**.
* **Web build**: `main` registers `(anonymous)::CSincronismoVotoEleitorWeb`. Its slot 2 is the ICF body
  `return 1`: the synchronisation "succeeds" without writing anything. So `rdv.dat` stays at its
  `votaInit` content (the recorded votes change only `logd.dat`, see `analysis/runtime/README.md`),
  `ConfereConteudo` is never exercised, and the `eleitor_dinamico` table is never created.
  According to the u04 analysis, `CGeraDadosDinamicos` (6737) skips `CEleitores::DynamicCreate` (5761)
  in voter-training mode (*treinamento do eleitor*), which is the mode of every simulator scenario. The
  DAO constructor has two other callers: `CEleitores::CompleteLoad` (6734) and the native
  `CSincronismoVotoEleitor::vf2` (7174). Neither shows up in the profiles, and since the constructor always
  runs `CREATE TABLE IF NOT EXISTS`, the missing table shows that none of the three ran.

## 4. Voter identifiers: `CEleitorIdentidade`, `CValidadorIdentidade`, rules

`CEleitorIdentidade(std::string, ETipoIdentificadorEleitor)` is func 566. The tools named it
`CValidadorIdentidade::Valida` because that function is inlined into it. It runs for every identity
built from the voter roll (`CVisitanteEleitor::visit`, `CConversorEntidadeEleitores`,
`CConversorImpedido::MontaEleitorIdentidade`), from the DAOs (SQL rows of `eleitor_dinamico` and
`comparecimento_mesario`), and from the operator flows (`CMostraEleitorVotando::SalvaHabilitacaoEleitor`,
the mesário `GetControlador` functions, `vota::TipoToStr`, ...):

1. **Normalise** (`Formata`, func 2798). The width is 11 for CPF (type 2) and 12 otherwise. If the string
   is longer, strip its leading zeros. Then left-pad it with `'0'` to the width.
2. **Pick a rule** (`CValidadorIdentidade::Valida`, cvalidadoridentidade.cpp:61). Take the first rule
   whose `GetTipo()` is `3` (free identifier) **or** the requested type. If none matches:
   `8108 "Tipo de identidade inválido: {}"`.
3. **Validate** (`IRegraIdentidade::Valida`, iregraidentidade.cpp:23). The string must be non-empty and
   digits-only (`find_first_not_of("0123456789")`). The rule's `Verifica` (slot 4) must accept the
   string with its leading zeros stripped (func 5159). Otherwise: `8109 "Identidade inválida: " + id`.
   Func 3723 is the same test without the throw (`EhValida`).
4. Normalise again.

The rules are built at start-up (inlined in 7787) from the identifier types that `CConfiguracaoEleicao`
enables: `1 → CRegraTitulo`, `2 → CRegraCPF`, `3 → CRegraIdentidadeLivre`, other values →
`7825`.

**Título de eleitor check** (`CRegraTitulo::Verifica`, 11272). The string is at most 12 digits, padded
to 12, digits `d0..d11`:
* `uf = d8*10 + d9` must be in 1..28 (compiled as `(uf-29) <u -28`).
* `r1 = (9*d0 + 8*d1 + 7*d2 + 6*d3 + 5*d4 + 4*d5 + 3*d6 + 2*d7) mod 11`,
  `dv1 = r1 < 2 ? (uf < 3 ? r1 ^ 1 : 0) : 11 - r1`, and `d10 == dv1`.
* `r2 = (4*d8 + 3*d9 + 2*d10) mod 11`, same mapping, and `d11 == dv2`.

The weights are the complement mod 11 of the published 2..9 / 7,8,9 weights, which gives the same
digits. The `uf < 3` branch is the published special case for SP (01) and MG (02): remainder 0 gives
digit 1.

**CPF check** (`CRegraCPF::Verifica`, 11270). At most 11 digits, padded to 11. This is the standard
two-digit mod-11 check (weights 10..2, then 11..3 plus `2*dv1`; remainder < 2 gives 0). Repeated-digit
CPFs (`000.000.000-00`, `111.111.111-11`, ...) are accepted.

`CRegraX::Formata` (slot 3, 11269/11271) is `PadLeft(s, '0', 11 | 12)` (func 753).

## 5. Voter records and the `eleitor_dinamico` table

* `CEleitor::ValidaCriacao` (5667): `Trim(nome)` must be non-empty (`8064 "Nome vazio"`, line 103);
  `necessidadeEspecial < 2` (`8065`, line 107); if `nomeSocial` is set, `Trim(nomeSocial)` must be
  non-empty (`8066`, line 113). Funcs 946/337 are the member-wise copy constructor and move assignment
  (vector of identities, four strings, a byte vector, flags).
* `CEleitorDecorator` = `CEleitor` + principal identifier type (+104). `GetIdentidadePorTipo` (708)
  returns the voter's identity of that type or throws `7833 "Tipo de identificador inválido: {}"`.
  `operator<=>` (456) compares the principal identities: digit string first, then type.
  `CEleitores::GetEleitoresEstaticos` (5772, unit u04's file) loads the roll files (`*-el.dat` and
  `*-tte.dat`, the temporary-transfer voters) through a `std::function`, decorates every voter, and
  `std::sort`s the vector with this ordering (introsort
  instantiation 945/3761/5755/5756/5769). The roll is therefore ordered by principal identity, presumably
  for fast lookup of a typed título/CPF.
* `CEleitorDinamico` (84 B): título, the identity used to enable the voter, `estado_comparecimento`,
  `tipo_habilitacao`, `dedo_habilitacao` (finger), `score_habilitacao` (u16), `numero_tentativa` (u8),
  `tipo_ativacao_audio`, `erro_decifrar_biometria`, an optional título of the poll worker who enabled
  the voter manually (`GetTituloMesarioHabilitacao`, 1548: `8067 "Eleitor não habilitado
  manualmente"`), and the photo-presentation pair (estado 0..2, resultado 0..8).

`CEleitorDinamicoDAO` (SQLite, through `ecourna::api::sql`):

| slot | func | method | SQL |
|---|---|---|---|
| ctor | 3768 | `CEleitorDinamicoDAO(path)` | `CREATE TABLE IF NOT EXISTS eleitor_dinamico ( titulo BIGINT PRIMARY KEY UNIQUE NOT NULL, tipo_identificador …, titulo_mesario BIGINT, tipo_identificador_mesario INTEGER CHECK(tipo_identificador IN (1,2,3) ) )` |
| 2 | 11541 | `Clone()` | copy of the DAO sharing the connection |
| 3 | 5776 | `Inserir(e)` | `INSERT INTO eleitor_dinamico (15 columns) VALUES (?×15)`; titles bound as `BIGINT` via `CStringUtils::ToQWord`; mesário columns `NULL` when absent |
| 4 | (11540) | `Excluir` | base default: throws `"Excluir não implementada para entidade …"` |
| 5 | 11543 | `ExcluirID` | throws `7983 "Exclusão de eleitor dinâmico"` (line 130) |
| 6 | 5775 | `Atualizar(e)` | `UPDATE eleitor_dinamico SET identidade_habilitacao = ?, … WHERE titulo = ?` |
| 7 | 11542 | `Recuperar(titulo)` | `SELECT … from eleitor_dinamico WHERE titulo = ?` → `shared_ptr(new CEleitorDinamico)` or null. Quirk: when `titulo_mesario` is NULL the voter's identity is built from the **parameter** `titulo`, not from column 0 (which is formatted and then discarded); with a mesário it uses column 0 |
| 8 | 3766 | `RecuperarTodos()` | `SELECT … from eleitor_dinamico` → `vector<CEleitorDinamico>` |

Rows are turned back into objects through `CEleitorIdentidade` (so they are re-validated) and the two
constructors 5663/5664. The DDL has **two** `CHECK` constraints on the wrong column
(`resultado_decifracao_foto` checks `estado_apresentacao_foto`, and `tipo_identificador_mesario` checks
`tipo_identificador`). The table holds no vote: attendance and ballots are kept apart.

## 6. Biometrics and the voter photo

`CBiometriaEleitor::CriaComum` (2799): at most 10 fingers (`8056 "Mais de dez dedos na biometria do
eleitor: {}"`); duplicates throw `8057 "Dedo já existente ou duplicado"`. `GetDedo` (3720):
`8058 "Dedo não encontrado."`. `GetFoto` (line 115, `8059 "Não há informação de foto."`) exists only
inlined into func 3616.

Func 3616 (`vota::ApresentaFotoEleitor`, name and file inferred) runs in the `StartState` of
`CNomeEleitor`, `IConfirmaJustificativa` and `IEleitorImpedidoVotar` (10625). It shows the identified
voter's photo on the poll worker's LCD and records the outcome in `IInformacaoThreadOperador`
(slots 27/28/29 = sem foto / apresentada / erro). That outcome later becomes
`eleitor_dinamico.estado_apresentacao_foto/resultado_decifracao_foto` and
`ModuloResultadoUrnaCadastro::ApresentacaoFotoEleitor`:

| condition | LCD image | result code (`ResultadoApresentacaoFotoEleitor`) | log |
|---|---|---|---|
| voter has no biometrics (+100) | `semBiometria.jpg` | estado "sem foto" | – |
| decryption state 9 / other > 0 | `fotoIndisponivel.jpg` | 7 erroDecifrandoBuffer / 8 erroDesconhecidoDeCriptografia | – |
| no photo in the package | `fotoIndisponivel.jpg` | estado "sem foto" | – |
| no SOF marker found | `fotoIndisponivel.jpg` | 1 erroFormato | "Erro ao carregar a foto do eleitor" |
| width ∉ 120..240 or height ∉ 160..320 | `fotoIndisponivel.jpg` | 3 erroResolucao | "Largura/Altura da foto do eleitor inválida: {} px" |
| 8 × components ≠ 8 (not grayscale) | `fotoIndisponivel.jpg` | 5 erroProfundidadeCores | "Quantidade de cores não conforme: {} bits por pixel" |
| otherwise | the JPEG itself | estado "apresentada" | "Foto do eleitor apresentada" |

The JPEG header scan is inlined. It runs on a second object built from a copy of the bytes by
`shared_f5111` = `{int 1; vector}` (the constructor that `CConversorFoto::DeconverteFormatoImagem` also
uses, probably `CFoto(formato, bytes)`). It walks `0xFF xx` markers, reads height and width from SOF0..SOF15
(except DHT/JPG/DAC) and DHP, skips DHT/DQT/DRI/DNL/EXP/APPn/COM/0x01/0xF0 by their length (RST0 = 0xD0 too,
although RST markers have no length field), remembers APP1 (flag) and the COM length, and stops at SOS.
Every index is bounds-checked, and a truncated segment raises `std::out_of_range` (`vector::__throw_out_of_range`,
i.e. `vector::at`, see §13).

## 7. Validated value objects

| class (func) | rule → error code, message, line |
|---|---|
| `CCandidatura::CCandidatura` (5660) | cargo < 100 (7991 "Número do cargo inválido", 37); partido < 100 (7992, 40); número < 100000 (7993, 43); the titular's `ordemSuplencia == 0` (7994 "Titular é suplente", 46); ≤ 9 substitutes (7995, 49); substitute *i* has ordem *i+1* (7996 "Ordem de suplente errada ({} [{}])" with ordem, nomeUrna, 55: packed arg types 419 = int, string_view, so the text reads e.g. "(2 [FULANO])") |
| `CCandidatura::GetSuplente(n)` (1389) | 1 ≤ n ≤ size (7997 "Suplente inexistente: {}", 65) |
| `CMunicipio::CMunicipio(codigo, nome, comBiometria)` (3712) | `Trim(nome)` non-empty (8119 "Nome vazio", 27) |
| `CInfoMunicipio::ValidaCriacao` (5675) | código < 100000 (8009, 61); name non-empty (8010, 66); fuso in −720..720 minutes (8011, 69) |
| `CInfoMunicipio::GetHorarioVerao` (3725) | DST present (8008, 51) |
| `CComplementoMunicipio::ValidaCriacao` (5647) / `GetHorarioVerao` (2795) | código < 100000 (8113, 47); fuso ±720 (8114, 52) / DST present (8112, 37) |
| `CHorarioVeraoMunicipio::ValidaCriacao` (5678) | código < 100000 (8006, 47) |
| `md::CLocal::ValidaCriacao` (5673) | país non-empty (8014, 75); UF abbreviation of 2 chars (8015, 78); UF name non-empty (8016, 81); section's municipality == local municipality (8017, 84); contingency's municipality == local (8018, 89) |
| `md::CLocal::GetSecao` / `GetContingencia` (943 / 3724) | optional present (8012 / 8013, "Não há informação.") |
| `CCarga::ValidaCriacao` (5670) | MC/flash serial = 8 hex chars (8019 / 8020, 36/41); `codigoCarga` = 24 decimal digits (8021 / 8022, 46/51) |
| `CCorrespondenciaResultado` ctor (2801) | município < 100000 (8027), zona < 10000 (8028), seção < 10000 (8029), tipo de urna ∈ '1'..'4' (8030) (lines 42..57) |
| `CLocalidadeEleitoral` ctor (5631) | município < 100000 (8095), zona < 10000 (8096), seção < 10000 (8097) (18..24) |
| `CDadoCarga` ctor (5633) | modelo ∈ 2013..2022 (8075 "Modelo inválido: {}", 84) |
| `CDadoCarga::GetFaseChar` (2253) | fase '1'/'2'/'3' → `'o'`/`'s'`/`'t'` (oficial/simulado/treinamento), else 8077 "Fase inválida: {}" (108) |
| `CAjusteDataHora::AdicionaDeltaT` (3702) | only when tipo == 2 `eAlterarDataSistema` (8071), then valor += delta |

`CEstadoGeral::RecuperarCertificado` (5635, line 147) reads the urna certificate once through
`api::pkcs11::IPkcs11` (slots 23 / 10 / 24 = open, read, close; names unknown) and caches it in a static
vector. No class in the binary implements `IPkcs11` (no RTTI, no `CPolySingletonList::push`), so in this
build the call can only throw `EPatternError 1301 "PolySingleton - solicitada uma instancia nao criada
N3api6pkcs117IPkcs11E [<path>/cestadogeral.cpp:147]"` (func 3704 appends the caller's `std::source_location`).
The static evidence: no typeinfo mentions `pkcs11`, and the typeid name `N3api6pkcs117IPkcs11E` is
referenced only by the lookup 3704 (a `push<IPkcs11>` would reference it too).

## 8. `CTradutorFrase` (placeholder translator)

`CTradutorFrase::Traduz` (654, the public entry; name inferred) is called for screen and report texts.
If no labels are loaded it returns the text unchanged. Otherwise it builds three boost regexes. `K` is
the set of label keys loaded from the PU file, e.g. `S` seção, `Z` zona, `M` município, `P` partido:

* `<[K][CL][PS][ABN]>` → `TraduzLabel`. `C/L` = *curto/longo* (short/long), `S/P` = singular/plural,
  `A/B/N` = upper/lower/unchanged case (Latin-1 aware, 3509/5158). Example: `<PLSN>` → "Partido".
  Errors 8129/8130/8131 "Label inválido: ".
* `<[K]\|[^|>]*\|[^|>]*\|[^|>]*>` → `TraduzTexto`: picks part 1/2/3 by the label's gender
  (neutro/masculino/feminino). Example: `"não pertence <S|a|ao|à> <SCSN>"`. Errors 8132/8133
  "Texto inválido: ".
* `<DT[+-][0-9]+>` → reference date ± N days, formatted `DD/MM/YYYY` (`api::CDate` ops 5478/5477/3648).

It repeatedly takes the earliest match, appends the text before it, and appends the translation.
`RecuperaLabel` (5658) looks up `token[0]` (`8128 "Token inválido: "`). The label map and the reference
date are statics (@1839056, @1839068) loaded with the parametrisation.

## 9. Data read and written

| data | format | who |
|---|---|---|
| `rdv.dat` (+ `.tmp`) in `/dsk/f{i,e}/dinamico/trab1` | BER `ModuloRegistroDigitalVoto::EntidadeRegistroDigitalVoto`, encrypted by `api::CEncryptedFile` with the `CRdv` crypto-table cipher | `CRdvVota::Converte/Desconverte/ConfereConteudo` |
| `uenux.db` table `eleitor_dinamico` | SQLite 3.50.4, 15 columns (§5) | `CEleitorDinamicoDAO` |
| `*-ca.dat` (candidates), `*-lo.dat` (local), `*-el.dat` (voters), `*-pu.dat` (labels), `eg.bin` | ASN.1 modules `ModuloCandidatos`, `ModuloLocal`, `ModuloComplementosMunicipios`, `ModuloEleitores`, `ModuloParametrizacaoUrna`, `ModuloEstadoGeralUrna` | the value objects of §7 validate what the converters (units u03/u21) decode |
| PKCS#11 token | certificate bytes | `CEstadoGeral::RecuperarCertificado` |
| MT LCD | JPEG / resource images | func 3616 |

## 10. BU (*Boletim de Urna*): what this unit provides

The BU generator itself (`vota::CGeraBU::StartState` 12110, `comum::CGeradorBUBase<CRdvVota, CEleitores>`,
`CDataSourcesRelatorio<...>`, `CGravadorBU`) belongs to other units. This unit supplies the numbers and
the identification fields:

1. **Vote counts** all come from `CRdvVota::GetInst()` (§3.2). They are read per office while the
   `CCargos` cursor walks the offices:
   * majoritarian offices (`ImprimeMajoritario` 11254): for each candidacy `Candidato(cargo, número,
     dígitos)` plus `Cargo(cargo)`; trailer `TrailerMajoritario` (11980) prints
     `"Brancos {:04}"` = `Brancos(cargo)`, `"Nulos {:04}"` = `Nulos(cargo)`, `"Total Apurado {:04}"` =
     `Cargo(cargo)`, and also reads `Nominais(cargo)`;
   * proportional offices (`ImprimeProporcionalPartido` 11255): per party the header line of func 11261
     (`"{}: {} - {}\n"` = `Traduz("<PLSN>")`, party number, party name/abbreviation), followed by the
     candidate-detail header only when `Legenda(cargo, partido) != Partido(cargo, partido)` (i.e. the party
     has votes for candidates). Then per candidate `Candidato(...)`, and `TrailerProporcionalPartido`
     (11260): `"    Total <P|de|do|da> <PLSB>"` with `Partido(...)` and the legenda votes with
     `Legenda(...)`. `TrailerProporcional` (11981): `"Total de votos de Legenda {:04}"` = `Legendas(cargo)`,
     Brancos, Nulos, Total Apurado, and a `"{:04}{:04}{:04}{:04}{:04}"` string (input to the
     verification code) built from Nominais/Legendas/Brancos/Nulos/Cargo;
   * consultas (`ImprimeConsulta` 11253): per answer `Candidato(cargo, número, dígitos)` (answers with 0
     votes are skipped), formatted `"{:05}{:04}"` for the verification code;
   * turnout: `CRdvVota::Comparecimento()` (no-argument overload, func 1269: highest `Comparecimento(eleição)` over the
     configured elections). It also decides "were votes cast?" (`CQuerReimprimirZeresima`,
     `CReinicioVotacao`, `CGravaResultado`, `CGeraBU`).
2. **File identification**: every result file (BU, RDV and other envelopes) carries a
   `CCorrespondenciaResultado` built by 2801: município (< 100000), zona (< 10000), seção (< 10000), the
   `CCarga` (numero interno da urna, serial of the memory card = 8 hex chars, 24-digit código de carga,
   generator identification), and the tipo de urna ('1'..'4'). `CGravadorBU::vf7` / `CGravadorRDV::vf7` /
   `IGravadorEnvelope::vf7` construct it. The constructor validates it, so a corrupted load
   identification aborts the write.
3. **Phase letter**: `CDadoCarga::GetFaseChar` ('o'/'s'/'t') is used by `CGeraBU`, `CGravaResultado`,
   `CCopiaResultadoParaMR` and `CCargos::GetCurrentEleicaoVersaoPacote` to build the result/package file
   names (the `t…` prefix of the training files).
4. **QR codes**: `comum_f5634` (called by `CGeraBU` with "include certificate" = 1) calls
   `CEstadoGeral::RecuperarCertificado()`. When the certificate is included it computes the number of BU
   QR codes as `ceil(2 * len(cert) / 1082)`. In this build that call throws (§7, §13).
5. **Zerésima** (the "zero report" printed before voting): `vota::CriaTituloExtratoRDV` (5971) calls
   `CRdvVota::GetVotos()` and asserts that every office's vote list is empty (`"Assert (votosCargos.Total()
   == 0)"`) before printing `"-----------EXTRATO DO RDV-------------"`. The consulta answer lines of the
   zerésima come from func 11478 (`"  " + PadRight(nome, 26) + PadLeft(format("{:0{}}", número, dígitos),
   10)`).
6. **RDV file** (companion of the BU): `rdv.dat` = encrypt(BER(`EntidadeRegistroDigitalVoto`)), written
   with the read-back check `ConfereConteudo` (§3.3).

## 11. Web-build specifics

* `CSincronismoVotoEleitorWeb` returns success without persisting anything. `rdv.dat` stays as created by
  `votaInit` (an encrypted empty RDV, 80 bytes), and `uenux.db` never gets the `eleitor_dinamico` table
  (verified: `sqlite3 .tables` on both MEMFS snapshots lists only `comparecimento_mesario` and
  `registro_justificativa`; the reason is voter-training mode, see §3.3). The in-memory RDV does receive
  the ballots (4454 observed).
* No `api::pkcs11::IPkcs11` implementation exists, so `CEstadoGeral::RecuperarCertificado` cannot succeed.
* `api::CAjusteDataHora` (10874/10875) forwards to `ISystemDateTime` slot 1. In the simulator that is
  `simulador::CWasmSystemDateTime::vf1` (7945). It does not touch any clock: it stores
  `novo − js_obter_data_hora_local_navegador()` as an offset. The native `api::CSystemDateTime::vf1`
  (10853) is an empty stub in this build.
* The voter-photo code (3616) targets the poll worker's micro-terminal LCD (`CInfoMTLCD`, whose `GetInst`
  is func 2284). The simulator never runs the poll-worker flows.
* Everything else in the unit (validation rules, check digits, label translation) is ordinary
  application code with no mock involved.

## 12. Functions of this unit whose original file belongs to another unit

The full versions are in `src/uenux2/src/app/comum/dados/u05-foreign-fragments.cpp`, with one section
per original file so that the owning units can merge them. Summary:

```cpp
// func 10625 — vota::IEleitorImpedidoVotar::StartState (vtable slot 2), file of unit u27
// (path inferred: uenux2/src/app/vota/operador/leidentidade/ieleitorimpedidovotar.cpp).
// Base of CEleitorNaoEncontrado, CEleitorOptouPorVotarEmTransito, CEleitorImpedidoJustificar,
// CEleitorNaoTemIdadeMinima, CEleitorNaoPossuiCargosParaVotar, CEleitorImpedidoJustificarVotoTransito.
void IEleitorImpedidoVotar::StartState()
{
    m_form->Exibe();          // member at +12, its vtable slot 2
    this->vf9();              // hook, icf_nop in the base
    ApresentaFotoEleitor();   // func 3616
    m_proximo = this;         // CState "next state" (+4) = stay here
}

// func 11261 — static std::string CDataSourcesRelatorio<CRdvVota,CEleitores>::HeaderProporcionalPartido()
// (name inferred; cdatasourcesrelatorio.h, unit u25). Passed as a text source (table slot 2921) to the BU.
static std::string HeaderProporcionalPartido()
{
    const TCargoID cargo = CCargos::GetInst().GetCurrent().GetCodigo();
    const md::CPartido& partido = CPartidos::GetInst().GetCurrent();          // ecourna_f819
    std::string linha = std::format("{}: {} - {}\n", md::CTradutorFrase::Traduz("<PLSN>"),
                                    partido.GetNumero(), partido.GetNome());   // +0 u16, +4 string
    const auto& rdv = CRdvVota::GetInst();
    if (rdv.Legenda(cargo, partido.GetNumero()) != rdv.Partido(cargo, partido.GetNumero())) {
        linha += CCargoDSLabelRelatorio::HeaderDetalhe();   // two appends, no operator+ temporary
        linha += "\n";
    }
    return linha;
}

// func 11478 — zerésima line of a consulta answer (name inferred; table slot 3051 next to
// CRelUtil::DSCodigoVerificador and (anonymous)::TituloPartido, i.e. comum/relatorios/crelutil.cpp).
static std::string DSLinhaRespostaZE()
{
    const uebyte digitos = CCargos::GetInst().GetCurrent().GetQtdDigitos();       // +12
    const auto& resposta = CRespostas::GetInst().GetCurrent();                   // CDataMap cursor
    const std::string numero = std::format("{:0{}}", resposta.GetNumero(), digitos);
    return "  " + PadRight(resposta.GetNome(), 26) + api::CStringUtils::PadLeft(numero, ' ', 10);
}

// func 11479 — current consulta answer name (text source, slot 1096), used by
// vota::(anonymous)::adicionaBaseTelaCompletaConsulta (ctelasvota.cpp).
static std::string NomeRespostaCorrente() { return CRespostas::GetInst().GetCurrent().GetNome(); }

// func 11378 — comum::asn::CConversorMunicipio::DoDesconverte (slot 3; DoConverte 11379 is in u21)
md::CMunicipio CConversorMunicipio::DoDesconverte(const ModuloTiposCadastro::Municipio& e) const
{
    return md::CMunicipio(e.codigo, e.nome,
                          e.hasOptionalField(1 /*comBiometria*/) ? bool(e.comBiometria) : false);
}

// func 11389 — comum::asn::CConversorLocalidadeEleitoral::DoDesconverte (class of unit u35)
md::estadoaplicacao::CLocalidadeEleitoral DoDesconverte(const TEntidade& e) const
{
    return {e.municipio, e.zona, e.secao};      // constructor 5631 validates
}

// func 5772 — std::vector<md::CEleitorDecorator> CEleitores::GetEleitoresEstaticos(const std::string& dir) const
//   (u04's file; name and signature from the RTTI of its lambda, GetEleitoresEstaticos(std::string const&)::$_0)
//   lists = file-name lists at this+76 and this+64 (api_f1936 builds full paths);
//   eleitores = Concatena(lista1, [](const std::string& arquivo) { /* $_0: partial ASN.1 decode of
//               ModuloEleitores::EntidadeEleitores via CVisitanteEleitor (11523) */ });
//   eleitores.insert(end, Concatena(lista2, same lambda))           // 5770
//   for (auto& e : eleitores) result.push_back(CEleitorDecorator(e, tipoPrincipal /* this+100 */));
//   std::sort(result.begin(), result.end());                          // 5769, operator<=> 456
// func 5771 — Concatena(const std::vector<std::string>& chaves (arrives as begin/end),
//                        std::function<std::vector<md::CEleitor>(const std::string&)> f /* by value */):
//   for each key: auto v = f(key); r.insert(r.end(), v.begin(), v.end());   (bad_function_call if empty)

// func 753  — std::string api::CStringUtils::PadLeft(const std::string& s, char c, size_t n)
//             { std::string r = s; if (r.size() < n) r.insert(0, n - r.size(), c); return r; }
// func 2284 — comum::CInfoMTLCD& CInfoMTLCD::GetInst(): lazy `new` (64 bytes) singleton @1838572; its
//             constructor 5904 stores vtable comum::CInfoMTLCD (the tools named 5904 after the srcloc of
//             its inlined member ctor api::BatteryIconDataSource<Vertical>)
// funcs 5478 / 5477 / 3648 — api::CDate::operator+=(int) / operator-=(int) / operator-(CDate, int):
//             day arithmetic with a month-length table; a negative argument delegates to the other one
// func 2794 — md::CVotos::CVotos(std::vector<CVoto> v) : m_votos(std::move(v)) { m_votos.reserve(1000); }
// func 655  — CBaseError<api::EUeRdvError,{4650,4850}> constructor thunk (ecourna_f710 + vtable)
```

## 13. Wasm/Emscripten observations

* **LTO inlining across files.** Many srclocs in a function name *another* function: 566 contains
  cvalidadoridentidade.cpp:61 and iregraidentidade.cpp:23, 3616 contains cbiometriaeleitor.cpp:115,
  11478/11479 contain crespostas.cpp:28, and 1930/2813/... contain cvotoseleicoesvota.cpp:1xx. The tools
  name a function after its first srcloc, so several unit entries were misnamed (566, 654, 2801, 3616,
  5631, 5633, 11478/11479). They are renamed in the overrides.
* **Constructors return `this`** (wasm C++ ABI). That is how 566, 2801, 5663/5664 and 3712 are recognised
  as constructors.
* **merge-similar-functions**: CRdvVota vf6..vf10 are 22-byte thunks into 2296 that pass
  `(srcloc, error code, lambda vtable)`. vf4/vf5 go into 6033. `CLocal::GetContingencia` is a thunk into
  the optional getter 6029 shared with `CCargo::GetDetalheConsulta`. The pseudo-code annotates its
  constant 8013 as a string ("Falha ao validar assinatura UE...") by accident: it is the error code.
* **Identical code folding**: all three DAO destructors share 1704/3767. `CRegraTitulo::GetTipo` is
  `icf_ret_1_vf3` (434) and `CRegraCPF::GetTipo` is `icf_tiny_vf2` (2254).
* **Function-local statics** (guard bytes @1911832/@1911848, set roots @1911820/@1911836) for the
  Latin-1 case tables. The global `std::string TABELA = "eleitor_dinamico"` (@1838780) is built inline in
  `__wasm_call_ctors`, and its destructor is registered as func 11545.
* **`__THREW__`** (invoke_* trampolines) appears in this unit only in 753 and 5159 (string insert/erase
  after a copy, cleanup of the copy), 5157 (`operator new` inside the `std::set<int>` constructor) and 3509
  (the set insert while building its function-local static). `vector::at` / `string::at` failures call
  `__throw_out_of_range` directly, and nothing in this unit catches them.
* Unsigned range checks show up as `(x - hi - 1) <u -(hi - lo + 1)`: UF 1..28, model 2013..2022, urna
  type '1'..'4', photo width/height.
* Nothing in this unit calls `emscripten_sleep`, touches the network, or reads URL/JS input.

## 14. Suspicious or risky code (summary; details in the structured report)

1. **`CEstadoGeral::RecuperarCertificado` (5635) cannot succeed in this build.** No
   `api::pkcs11::IPkcs11` implementation is compiled in, so `CGeraBU`'s QR-code step (`comum_f5634`)
   would throw `EPatternError 1301` as soon as it runs. It is dead in the recorded sessions (no
   encerramento, the closing of the vote).
2. **Wrong-column `CHECK` constraints** in the `eleitor_dinamico` DDL (3768). Both the
   `resultado_decifracao_foto` and the `tipo_identificador_mesario` constraints test other columns, so
   those two columns are not validated by SQLite. This is the original code (the same on the urna).
3. **Unchecked `map::find(...)->second`** in the inlined `CVotosEleicoesVota` queries (2296/6033/1931).
   The lookup `cargo → eleição` is checked, but the election's `CVotosCargos` is not. An inconsistent RDV
   would make the query read the map's end node as an object (undefined behaviour, wrong counts or a
   trap). It is only reachable if the two maps disagree. `EstruturaCompativel` protects the load path.
4. **Unhandled `std::out_of_range` in the voter-photo JPEG scan (3616).** A truncated segment in the
   (decrypted) photo throws from `vector::at` inside the operator's `StartState`. Nothing in 3616 or in the
   three `StartState` callers catches it. Whether the state-machine driver above them catches it was not
   checked.
5. **CPF rule accepts repeated-digit CPFs** (all zeros included). In `CValidadorIdentidade`, a registered
   "free identifier" rule placed first wins for *every* type, so the título/CPF check digits are skipped
   (configuration-dependent).
6. **`CTradutorFrase::Traduz`** puts the label keys unescaped inside a regex character class. It compiles
   three `boost::regex` per placeholder (CPU-heavy in wasm). The regex helper swallows exceptions, so a
   malformed pattern silently leaves the text untranslated. `<DT±N>` with N > 65535 goes through `ToWord`.
7. `CDadoCarga` accepts any model number in 2013..2022 (including models that do not exist) and rejects
   anything newer than 2022.
8. **`CEleitorDinamicoDAO::Recuperar` (11542) builds the voter identity from two different sources.** With a
   poll-worker override the título comes from the row (column 0). Without one it comes from the lookup key.
   They are equal under `WHERE titulo = ?` (both are normalised by `CEleitorIdentidade`), so the only
   effect is that the two branches of the original code are inconsistent.

## 15. Complete mapping table (100 functions)

`run` = observed executing during the recorded votes. "src" is the reconstructed file under `src/`, or
where the function is documented when it is a library instantiation or belongs to another unit's file.

| func | size | run | name before this unit | reconstructed symbol | src |
|---|---|---|---|---|---|
| 337 | 554 |  | `comum_f337` | `comum::md::CEleitor::operator=(CEleitor&&)` | `uenux2/src/app/comum/dados/md/eleitor/celeitor.h` |
| 456 | 237 |  | `comum_f456` | `comum::md::CEleitorDecorator::operator<=> (name inferred)` | `uenux2/src/app/comum/dados/md/eleitor/celeitordecorator.cpp` |
| 555 | 24 |  | `comum::CRdvVota::GetInst` | `comum::CRdvVota::GetInst` | `uenux2/src/app/comum/dados/crdvvota.cpp` |
| 566 | 997 | ✓ | `comum::md::CValidadorIdentidade::Valida` | `comum::md::CEleitorIdentidade::CEleitorIdentidade(std::string, ETipoIdentificadorEleitor)` | `uenux2/src/app/comum/dados/md/eleitor/celeitoridentidade.cpp` |
| 654 | 4752 | ✓ | `comum::md::CTradutorFrase::TraduzLabel` | `comum::md::CTradutorFrase::Traduz (name inferred; tools: TraduzLabel)` | `uenux2/src/app/comum/dados/md/parametrizacaourna/ctradutorfrase.cpp` |
| 655 | 18 |  | `comum_f655` | `ecourna::api::exception::CBaseError<api::EUeRdvError,{4650,4850}>::CBaseError (merged ctor thunk)` | library/inlined helper (listed in u05-foreign-fragments.cpp) |
| 708 | 581 |  | `comum::md::CEleitorDecorator::GetIdentidadePorTipo` | `comum::md::CEleitorDecorator::GetIdentidadePorTipo` | `uenux2/src/app/comum/dados/md/eleitor/celeitordecorator.cpp` |
| 753 | 173 | ✓ | `api_f753` | `api::CStringUtils::PadLeft(const std::string&, char, size_t) (name inferred)` | `uenux2/src/app/comum/dados/u05-foreign-fragments.cpp` (owner uenux2/src/api/util/cstringutils.cpp, unit u20) |
| 943 | 74 |  | `comum::md::CLocal::GetSecao` | `comum::md::CLocal::GetSecao` | `uenux2/src/app/comum/dados/md/clocal.cpp` |
| 945 | 1833 |  | `comum_f945` | `std::__sort3<_ClassicAlgPolicy, __less<>&, comum::md::CEleitorDecorator*>` | library/inlined helper (listed in u05-foreign-fragments.cpp) |
| 946 | 584 | ✓ | `api_f946` | `comum::md::CEleitor::CEleitor(const CEleitor&)` | `uenux2/src/app/comum/dados/md/eleitor/celeitor.h` |
| 1389 | 512 |  | `comum::md::CCandidatura::GetSuplente` | `comum::md::CCandidatura::GetSuplente` | `uenux2/src/app/comum/dados/md/candidatura/ccandidatura.cpp` |
| 1548 | 76 |  | `comum::md::CEleitorDinamico::GetTituloMesarioHabilitacao` | `comum::md::CEleitorDinamico::GetTituloMesarioHabilitacao` | `uenux2/src/app/comum/dados/md/eleitor/celeitordinamico.cpp` |
| 1550 | 32 |  | `comum_f1550` | `std::__tree<pair<TCargoID,TEleicaoID>>::destroy (map node free)` | library/inlined helper (listed in u05-foreign-fragments.cpp) |
| 1904 | 22 |  | `comum_f1904` | `vota::(anonymous)::RegistraErroFoto (name inferred)` | `uenux2/src/app/vota/operador/comum/capresentacaofotoeleitor.cpp` |
| 1926 | 24 |  | `comum::md::CValidadorIdentidade::GetInst` | `comum::md::CValidadorIdentidade::GetInst` | `uenux2/src/app/comum/dados/md/cvalidadoridentidade.cpp` |
| 1930 | 22 |  | `comum::CRdvVota::vf10` | `comum::CRdvVota::Cargo` | `uenux2/src/app/comum/dados/crdvvota.cpp` |
| 1931 | 343 |  | `comum::CRdvVota::vf3` | `comum::CRdvVota::Candidato` | `uenux2/src/app/comum/dados/crdvvota.cpp` |
| 2253 | 510 |  | `comum::md::estadoaplicacao::CDadoCarga::GetFaseChar` | `comum::md::estadoaplicacao::CDadoCarga::GetFaseChar` | `uenux2/src/app/comum/dados/md/estadoaplicacao/cdadocarga.cpp` |
| 2284 | 88 |  | `api_f2284` | `comum::CInfoMTLCD::GetInst (name inferred)` | `uenux2/src/app/comum/dados/u05-foreign-fragments.cpp` (owner uenux2/src/app/comum/cinfomtlcd.cpp, unit u22) |
| 2296 | 239 |  | `comum_f2296` | `comum::md::CVotosEleicoesVota::Consulta (merged body of Nominais/Legendas/Nulos/Brancos/Cargo, inlined into CRdvVota)` | `uenux2/src/app/comum/dados/crdvvota.cpp` |
| 2794 | 290 |  | `comum_f2794` | `comum::md::CVotos::CVotos(std::vector<CVoto>)` | `uenux2/src/app/comum/dados/u05-foreign-fragments.cpp` (owner md/rdv, unit u22) |
| 2795 | 73 |  | `comum::md::CComplementoMunicipio::GetHorarioVerao` | `comum::md::CComplementoMunicipio::GetHorarioVerao` | `uenux2/src/app/comum/dados/md/municipiozona/ccomplementomunicipio.cpp` |
| 2798 | 84 |  | `comum_f2798` | `comum::md::CEleitorIdentidade::Formata (name inferred)` | `uenux2/src/app/comum/dados/md/eleitor/celeitoridentidade.cpp` |
| 2799 | 878 |  | `comum::md::CBiometriaEleitor::CriaComum` | `comum::md::CBiometriaEleitor::CriaComum` | `uenux2/src/app/comum/dados/md/eleitor/cbiometriaeleitor.cpp` |
| 2801 | 1953 |  | `comum::md::CCorrespondenciaResultado::ValidaCriacao` | `comum::md::CCorrespondenciaResultado::CCorrespondenciaResultado (+ inlined ValidaCriacao)` | `uenux2/src/app/comum/dados/md/correspondencia/ccorrespondenciaresultado.cpp` |
| 2812 | 24 |  | `comum::CRespostas::GetInst@2812` | `comum::CRespostas::GetInst` | `uenux2/src/app/comum/dados/crespostas.cpp` |
| 2813 | 22 |  | `comum::CRdvVota::vf6` | `comum::CRdvVota::Nominais` | `uenux2/src/app/comum/dados/crdvvota.cpp` |
| 2814 | 24 |  | `comum::CRdvVota::vf5` | `comum::CRdvVota::Partido` | `uenux2/src/app/comum/dados/crdvvota.cpp` |
| 3509 | 530 | ✓ | `comum_f3509` | `comum::md::ConverteMaiusculas (name inferred)` | `uenux2/src/app/comum/dados/md/parametrizacaourna/ctradutorfrase.cpp` |
| 3616 | 3477 |  | `comum::md::CBiometriaEleitor::GetFoto` | `vota::ApresentaFotoEleitor (name inferred)` | `uenux2/src/app/vota/operador/comum/capresentacaofotoeleitor.cpp` |
| 3617 | 222 |  | `comum_f3617` | `vota::(anonymous)::MostraFotoIndisponivel (name inferred)` | `uenux2/src/app/vota/operador/comum/capresentacaofotoeleitor.cpp` |
| 3648 | 19 |  | `comum_f3648` | `api::operator-(const CDate&, int dias) (name inferred)` | `uenux2/src/app/comum/dados/u05-foreign-fragments.cpp` (owner uenux2/src/api/util/cdate.cpp, unit u20) |
| 3702 | 494 |  | `comum::md::estadoaplicacao::CAjusteDataHora::AdicionaDeltaT` | `comum::md::estadoaplicacao::CAjusteDataHora::AdicionaDeltaT` | `uenux2/src/app/comum/dados/md/estadoaplicacao/cajustedatahora.cpp` |
| 3707 | 173 |  | `comum_f3707` | `comum::md::CVotosEleicoesVota::EleicaoDoCargo (name inferred)` | `uenux2/src/app/comum/dados/crdvvota.cpp` |
| 3712 | 107 |  | `comum::md::CMunicipio::CMunicipio` | `comum::md::CMunicipio::CMunicipio` | `uenux2/src/app/comum/dados/md/municipiozona/cmunicipio.cpp` |
| 3720 | 249 |  | `comum::md::CBiometriaEleitor::GetDedo` | `comum::md::CBiometriaEleitor::GetDedo` | `uenux2/src/app/comum/dados/md/eleitor/cbiometriaeleitor.cpp` |
| 3723 | 192 |  | `vota_f3723` | `comum::md::IRegraIdentidade::EhValida (name inferred)` | `uenux2/src/app/comum/dados/md/iregraidentidade.cpp` |
| 3724 | 22 |  | `comum::md::CLocal::GetContingencia` | `comum::md::CLocal::GetContingencia` | `uenux2/src/app/comum/dados/md/clocal.cpp` |
| 3725 | 73 |  | `comum::md::CInfoMunicipio::GetHorarioVerao` | `comum::md::CInfoMunicipio::GetHorarioVerao` | `uenux2/src/app/comum/dados/md/cinfomunicipio.cpp` |
| 3747 | 22 |  | `comum::CRdvVota::vf9` | `comum::CRdvVota::Brancos` | `uenux2/src/app/comum/dados/crdvvota.cpp` |
| 3748 | 22 |  | `comum::CRdvVota::vf8` | `comum::CRdvVota::Nulos` | `uenux2/src/app/comum/dados/crdvvota.cpp` |
| 3749 | 24 |  | `comum::CRdvVota::vf4` | `comum::CRdvVota::Legenda` | `uenux2/src/app/comum/dados/crdvvota.cpp` |
| 3761 | 1110 |  | `comum_f3761` | `std::__sort4<..., comum::md::CEleitorDecorator*>` | library/inlined helper (listed in u05-foreign-fragments.cpp) |
| 3766 | 3613 |  | `comum::dao::CEleitorDinamicoDAO::vf8` | `comum::dao::CEleitorDinamicoDAO::RecuperarTodos (name inferred)` | `uenux2/src/app/comum/dados/dao/celeitordinamicodao.cpp` |
| 3768 | 618 |  | `comum::dao::CEleitorDinamicoDAO::CEleitorDinamicoDAO` | `comum::dao::CEleitorDinamicoDAO::CEleitorDinamicoDAO` | `uenux2/src/app/comum/dados/dao/celeitordinamicodao.cpp` |
| 5157 | 288 | ✓ | `comum_f5157` | `std::set<int>::set(std::initializer_list<int>)` | library/inlined helper (listed in u05-foreign-fragments.cpp) |
| 5158 | 291 | ✓ | `comum_f5158` | `comum::md::ConverteMinusculas (name inferred)` | `uenux2/src/app/comum/dados/md/parametrizacaourna/ctradutorfrase.cpp` |
| 5159 | 283 |  | `comum_f5159` | `comum::md::(anonymous)::SemZerosEsquerda (name inferred)` | `uenux2/src/app/comum/dados/md/iregraidentidade.cpp` |
| 5413 | 20 |  | `comum_f5413` | `vota::(anonymous)::RegistraEleitorSemFoto (name inferred)` | `uenux2/src/app/vota/operador/comum/capresentacaofotoeleitor.cpp` |
| 5477 | 949 |  | `comum_f5477` | `api::CDate::operator-=(int dias) (name inferred)` | `uenux2/src/app/comum/dados/u05-foreign-fragments.cpp` (owner uenux2/src/api/util/cdate.cpp, unit u20) |
| 5478 | 945 |  | `comum_f5478` | `api::CDate::operator+=(int dias) (name inferred)` | `uenux2/src/app/comum/dados/u05-foreign-fragments.cpp` (owner uenux2/src/api/util/cdate.cpp, unit u20) |
| 5631 | 204 |  | `comum::md::estadoaplicacao::CLocalidadeEleitoral::ValidaCriacao` | `comum::md::estadoaplicacao::CLocalidadeEleitoral::CLocalidadeEleitoral (+ inlined ValidaCriacao)` | `uenux2/src/app/comum/dados/md/estadoaplicacao/clocalidadeeleitoral.cpp` |
| 5633 | 525 |  | `comum::md::estadoaplicacao::CDadoCarga::ValidaModelo` | `comum::md::estadoaplicacao::CDadoCarga::CDadoCarga (+ inlined ValidaModelo)` | `uenux2/src/app/comum/dados/md/estadoaplicacao/cdadocarga.cpp` |
| 5635 | 279 |  | `comum::md::estadoaplicacao::CEstadoGeral::RecuperarCertificado` | `comum::md::estadoaplicacao::CEstadoGeral::RecuperarCertificado` | `uenux2/src/app/comum/dados/md/estadoaplicacao/cestadogeral.cpp` |
| 5638 | 598 |  | `comum_f5638` | `comum::md::CVotosEleicoesVota::EstruturaCompativel (name inferred)` | `uenux2/src/app/comum/dados/crdvvota.cpp` |
| 5647 | 136 |  | `comum::md::CComplementoMunicipio::ValidaCriacao` | `comum::md::CComplementoMunicipio::ValidaCriacao` | `uenux2/src/app/comum/dados/md/municipiozona/ccomplementomunicipio.cpp` |
| 5658 | 193 |  | `comum::md::CTradutorFrase::RecuperaLabel` | `comum::md::CTradutorFrase::RecuperaLabel` | `uenux2/src/app/comum/dados/md/parametrizacaourna/ctradutorfrase.cpp` |
| 5660 | 1196 | ✓ | `comum::md::CCandidatura::CCandidatura` | `comum::md::CCandidatura::CCandidatura` | `uenux2/src/app/comum/dados/md/candidatura/ccandidatura.cpp` |
| 5663 | 275 |  | `comum_f5663` | `comum::md::CEleitorDinamico::CEleitorDinamico (with título do mesário)` | `uenux2/src/app/comum/dados/md/eleitor/celeitordinamico.cpp` |
| 5664 | 210 |  | `comum_f5664` | `comum::md::CEleitorDinamico::CEleitorDinamico (without título do mesário)` | `uenux2/src/app/comum/dados/md/eleitor/celeitordinamico.cpp` |
| 5667 | 293 |  | `comum::md::CEleitor::ValidaCriacao` | `comum::md::CEleitor::ValidaCriacao` | `uenux2/src/app/comum/dados/md/eleitor/celeitor.cpp` |
| 5670 | 1221 |  | `comum::md::CCarga::ValidaCriacao` | `comum::md::CCarga::ValidaCriacao` | `uenux2/src/app/comum/dados/md/correspondencia/ccarga.cpp` |
| 5673 | 343 |  | `comum::md::CLocal::ValidaCriacao` | `comum::md::CLocal::ValidaCriacao` | `uenux2/src/app/comum/dados/md/clocal.cpp` |
| 5675 | 1033 |  | `comum::md::CInfoMunicipio::ValidaCriacao` | `comum::md::CInfoMunicipio::ValidaCriacao` | `uenux2/src/app/comum/dados/md/cinfomunicipio.cpp` |
| 5678 | 478 |  | `comum::md::CHorarioVeraoMunicipio::ValidaCriacao` | `comum::md::CHorarioVeraoMunicipio::ValidaCriacao` | `uenux2/src/app/comum/dados/md/chorarioveraomunicipio.cpp` |
| 5732 | 22 |  | `comum::CRdvVota::vf7` | `comum::CRdvVota::Legendas` | `uenux2/src/app/comum/dados/crdvvota.cpp` |
| 5733 | 1192 |  | `comum::CRdvVota::vf0` | `comum::CRdvVota::GetVotos` | `uenux2/src/app/comum/dados/crdvvota.cpp` |
| 5755 | 1469 |  | `comum_f5755` | `std::__sort5<..., comum::md::CEleitorDecorator*>` | library/inlined helper (listed in u05-foreign-fragments.cpp) |
| 5756 | 1027 |  | `comum_f5756` | `std::__insertion_sort_incomplete<..., comum::md::CEleitorDecorator*>` | library/inlined helper (listed in u05-foreign-fragments.cpp) |
| 5757 | 423 |  | `api_f5757` | `std::vector<comum::md::CEleitor>::__move_range` | library/inlined helper (listed in u05-foreign-fragments.cpp) |
| 5759 | 277 |  | `comum_f5759` | `std::vector<comum::md::CEleitorDinamico>::__push_back_slow_path` | library/inlined helper (listed in u05-foreign-fragments.cpp) |
| 5768 | 520 |  | `api_f5768` | `std::vector<comum::md::CEleitorDecorator>::__swap_out_circular_buffer` | library/inlined helper (listed in u05-foreign-fragments.cpp) |
| 5769 | 6341 |  | `comum_f5769` | `std::__introsort<..., comum::md::CEleitorDecorator*>` | library/inlined helper (listed in u05-foreign-fragments.cpp) |
| 5770 | 1515 | ✓ | `api_f5770` | `std::vector<comum::md::CEleitor>::__insert_with_size (range insert)` | library/inlined helper (listed in u05-foreign-fragments.cpp) |
| 5771 | 487 | ✓ | `api_f5771` | `comum::(anonymous)::ConcatenaEleitores (name inferred: for each file path, f(path) appended)` | `uenux2/src/app/comum/dados/u05-foreign-fragments.cpp` (owner celeitores.cpp, unit u04) |
| 5772 | 2143 | ✓ | `api_f5772` | `comum::CEleitores::GetEleitoresEstaticos(const std::string&) const (lambda RTTI)` | `uenux2/src/app/comum/dados/u05-foreign-fragments.cpp` (owner celeitores.cpp, unit u04) |
| 5775 | 931 |  | `comum::dao::CEleitorDinamicoDAO::vf6` | `comum::dao::CEleitorDinamicoDAO::Atualizar` | `uenux2/src/app/comum/dados/dao/celeitordinamicodao.cpp` |
| 5776 | 942 |  | `comum::dao::CEleitorDinamicoDAO::vf3` | `comum::dao::CEleitorDinamicoDAO::Inserir (name inferred)` | `uenux2/src/app/comum/dados/dao/celeitordinamicodao.cpp` |
| 6033 | 339 |  | `comum_f6033` | `comum::md::CVotosEleicoesVota::Consulta (merged body of Legenda/Partido, inlined into CRdvVota)` | `uenux2/src/app/comum/dados/crdvvota.cpp` |
| 10625 | 45 |  | `vota::IEleitorImpedidoVotar::vf2` | `vota::IEleitorImpedidoVotar::StartState` | `uenux2/src/app/comum/dados/u05-foreign-fragments.cpp` (owner vota/operador, unit u27) |
| 10874 | 23 |  | `api::CAjusteDataHora::vf1` | `api::CAjusteDataHora::AjustaDataHora(time_t) (name inferred)` | `uenux2/src/api/util/cajustedatahora.cpp` |
| 10875 | 23 |  | `api::CAjusteDataHora::vf0` | `api::CAjusteDataHora::AjustaDataHora(const CDateTime&) (name inferred)` | `uenux2/src/api/util/cajustedatahora.cpp` |
| 11261 | 699 |  | `comum_f11261` | `comum::CDataSourcesRelatorio<CRdvVota,CEleitores>::HeaderProporcionalPartido (name inferred)` | `uenux2/src/app/comum/dados/u05-foreign-fragments.cpp` (owner cdatasourcesrelatorio.h, unit u25) |
| 11269 | 13 |  | `comum::md::CRegraCPF::vf3` | `comum::md::CRegraCPF::Formata (name inferred)` | `uenux2/src/app/comum/dados/md/cregracpf.cpp` |
| 11270 | 571 |  | `comum::md::CRegraCPF::vf4` | `comum::md::CRegraCPF::Verifica (name inferred)` | `uenux2/src/app/comum/dados/md/cregracpf.cpp` |
| 11271 | 13 |  | `comum::md::CRegraTitulo::vf3` | `comum::md::CRegraTitulo::Formata (name inferred)` | `uenux2/src/app/comum/dados/md/cregratitulo.cpp` |
| 11272 | 537 | ✓ | `comum::md::CRegraTitulo::vf4` | `comum::md::CRegraTitulo::Verifica (name inferred)` | `uenux2/src/app/comum/dados/md/cregratitulo.cpp` |
| 11378 | 153 | ✓ | `comum::asn::CConversorMunicipio::DoDesconverte` | `comum::asn::CConversorMunicipio::DoDesconverte` | `uenux2/src/app/comum/dados/u05-foreign-fragments.cpp` (class file owned by unit u21) |
| 11389 | 36 |  | `comum::asn::CConversorLocalidadeEleitoral::vf3` | `comum::asn::CConversorLocalidadeEleitoral::DoDesconverte` | `uenux2/src/app/comum/dados/u05-foreign-fragments.cpp` (class owned by unit u35) |
| 11478 | 721 |  | `comum::CRespostas::GetInst@11478` | `comum::CRelUtil::DSLinhaRespostaZE (name inferred; zeresima consulta answer line)` | `uenux2/src/app/comum/dados/u05-foreign-fragments.cpp` (owner comum/relatorios, unit u25) |
| 11479 | 137 |  | `comum::CRespostas::GetInst@11479` | `vota::(anonymous)::NomeRespostaCorrente (name inferred)` | `uenux2/src/app/comum/dados/u05-foreign-fragments.cpp` (owner ctelasvota.cpp) |
| 11486 | 768 |  | `comum::CRdvVota::vf13` | `comum::CRdvVota::ConfereConteudo (name inferred)` | `uenux2/src/app/comum/dados/crdvvota.cpp` |
| 11487 | 707 |  | `comum::CRdvVota::Desconverte` | `comum::CRdvVota::Desconverte` | `uenux2/src/app/comum/dados/crdvvota.cpp` |
| 11488 | 1661 | ✓ | `comum::CRdvVota::vf11` | `comum::CRdvVota::Converte (name inferred)` | `uenux2/src/app/comum/dados/crdvvota.cpp` |
| 11490 | 597 |  | `comum::CRdvVota::vf1` | `comum::CRdvVota::GetVotos(TEleicaoID)` | `uenux2/src/app/comum/dados/crdvvota.cpp` |
| 11492 | 12 |  | `comum::CRdvVota::vf2` | `comum::CRdvVota::Comparecimento` | `uenux2/src/app/comum/dados/crdvvota.cpp` |
| 11541 | 12 |  | `comum::dao::CEleitorDinamicoDAO::vf2` | `comum::dao::CEleitorDinamicoDAO::Clone (name inferred)` | `uenux2/src/app/comum/dados/dao/celeitordinamicodao.cpp` |
| 11542 | 2930 |  | `comum::dao::CEleitorDinamicoDAO::vf7` | `comum::dao::CEleitorDinamicoDAO::Recuperar (name inferred)` | `uenux2/src/app/comum/dados/dao/celeitordinamicodao.cpp` |
| 11543 | 50 |  | `comum::dao::CEleitorDinamicoDAO::ExcluirID` | `comum::dao::CEleitorDinamicoDAO::ExcluirID` | `uenux2/src/app/comum/dados/dao/celeitordinamicodao.cpp` |

## 16. Open questions

* The exact names of the CRdv slots 0/1/11/13 (`GetVotos`, `Converte`, `ConfereConteudo`) and of the DAO
  slots 2/3/7/8 (`Clone`, `Inserir`, `Recuperar`, `RecuperarTodos`) are inferred from behaviour. No srcloc
  names them.
* The type returned by `CRdvVota::GetVotos()` is shown as `std::map<TCargoID, CVotos>`. The zerésima
  assertion text `votosCargos.Total()` suggests a thin wrapper class with a `Total()` method.
* The original file and name of func 3616 (voter-photo presentation) and of the text sources 11478 /
  11479 are unknown. The `api::CAjusteDataHora` file path is inferred.
* `IPkcs11` slots 10/23/24 are unnamed. Confirming that `CGeraBU` really throws in the simulator needs a
  scripted encerramento in `tools/run/headless.mjs`. The recorded scenarios stop after the votes.
* Several `CEleitor`/`CCarga`/`CLocal` member offsets (marked `?` in the headers) are known only from
  copy code.
