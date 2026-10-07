# The TSE ASN.1 schemas inside `vota_web_wasm.wasm`

Every data file the voting application reads or writes is an ASN.1 value encoded with BER:
the election configuration loaded into the urna, the voter roll, the candidates and their
photos, the urna's own state files, and the result files — the **BU** (*boletim de urna*,
the tally of one section) and the **RDV** (*registro digital do voto*, the list of every vote
cast, with no link to the voter). The C++ code for these types was generated from `.asn1`
modules named `Modulo*`. The `.asn1` sources are not in the binary, but the tables that the
generated code hands to the ASN.1 runtime are, and they hold the whole schema.

This chapter shows how the schemas were rebuilt from those tables, what each module means for
the election, how they compare with the specifications the TSE has published, and which
scenario file uses which type. The rebuilt modules are in [`src/asn1/`](../../src/asn1/)
(32 files, one per module). The tools that produced and checked them are in
[`src/asn1/tools/`](../../src/asn1/tools/).

**Summary**

* 186 constructed types were recovered: **118 SEQUENCE, 10 CHOICE, 58 ENUMERATED**. With them
  come 5 named string types, in **32 modules**. For every type we have the member names, their
  order, which members are OPTIONAL, the tags, the INTEGER ranges, the SIZE constraints and the
  enumeration names and values. These come straight from the tables the encoder and decoder use.
* Type names come from four sources. Every SEQUENCE carries its own name in a field that TSE
  added to the table (118/118). The other types are named from C++ RTTI, from
  `std::source_location` signatures, or from the TSE's published spec. **11 CHOICE/ENUMERATED
  types have no name anywhere in the binary.** They are marked as placeholders in the `.asn`
  files.
* **Check.** An independent ASN.1 compiler, [`asn1tools`](https://github.com/eerimoq/asn1tools)
  0.169.0, compiles the 32 modules. It BER-decodes, and re-encodes **byte for byte**, 289 files:
  all 272 decodable scenario files of the 8 simulator scenarios, the 7 state files written at
  run time, and **10 official printed-BU envelopes (`*-imgbu.dat`) produced in 2024 by the real
  urna software `9.29.0.1 - Kayapó`**. Their payload is the text sent to the printer, so these 10
  test the envelope and urna-identity types (`EntidadeEnvelopeGenerico`, `Urna`, `Carga`…),
  not the BU body (`EntidadeBoletimUrna`), which is checked against the published spec only (§5).
* The BU and RDV modules agree, type by type, with the **v2 (2024) ASN.1 specification
  published by the TSE**. The only differences are 3 enumerations. They point to a version
  **later than 2024**, which fits the build string `10.23.0.1 - DESENVOLVIMENTO`. The TSE's
  signature module (`assinatura.asn1`) is **not** compiled in.

---

## 1. Background: ASN.1, BER and the election files

**ASN.1** (ITU-T X.680) is a language for describing data structures. **BER** (X.690) is one
way to turn values into bytes. Every value is written as a **TLV**: a **T**ag that says what it
is, a **L**ength, and a **V**alue. The value of a SEQUENCE is just its members' TLVs one after
another:

```
DataHoraJE ::= GeneralString (SIZE(15))          -- "YYYYMMDDThhmmss"
CabecalhoEntidade ::= SEQUENCE { dataGeracao DataHoraJE, idEleitoral IDEleitoral }
IDEleitoral ::= CHOICE { idProcessoEleitoral [1] INTEGER (0..99999), idPleito [2] ..., idEleicao [3] ... }

30 15                               SEQUENCE, 21 bytes
   1b 0f 32303236...3137            GeneralString (UNIVERSAL 27), "20260910T162217"
   81 02 09 60                      context tag [1], 2 bytes: INTEGER 2400  -> idProcessoEleitoral
```

A module declared with `IMPLICIT TAGS` replaces a type's own tag when a member is given a
`[n]` tag. That is why `[1]` above is `0x81` (context class, primitive, number 1) and not
`0x02` (INTEGER). A decoder that does not know the schema sees only tags and bytes. That is
why recovering the schema matters.

The TSE vocabulary used below:

| term | meaning |
|---|---|
| *processo eleitoral* / *pleito* / *eleição* | The election "process" (e.g. the 2026 general elections) holds *pleitos* (1st and 2nd round), which hold *eleições* (federal, state…). Each level has a numeric id. In the training scenarios these are 2400 / 2410 / 2411. |
| *cargo* | An office to vote for (presidente, vereador…). A *consulta popular* (referendum) question is modelled as a cargo too. |
| *votável* | Anything that can receive a vote: a candidate number, or a party number for a *voto de legenda* (party-only vote). |
| *seção* / *zona* / *município* / *local* | Polling section / electoral zone / municipality / polling place. |
| *urna de contingência* | Spare urna that replaces a broken one. |
| *SA* (Sistema de Apuração) | Counting system used when the urna cannot be used, fed from paper ballots or the memory card. |
| *carga* | Loading of the election data into an urna. The *código de carga* identifies each load. |
| *mesário* | Poll worker. *zerésima*: the report printed before voting starts, proving that all counts are zero. |
| *TTE* | *transferência temporária de eleitores*: voters temporarily moved to another section (e.g. on duty, in prison, with accessibility needs). |
| *fase* | `oficial` (real election), `simulado` (rehearsal), `treinamento` (training). The simulator only uses `treinamento`. |

---

## 2. Which ASN.1 tool: III ASN.1

The C++ class names give the tool away. `ASN1::SEQUENCE`, `ASN1::CHOICE`,
`ASN1::SEQUENCE_OF<ModuloBoletimUrna::TotalVotosCargo, ASN1::EmptyConstraint>`,
`ASN1::Constrained_INTEGER<(ASN1::ConstraintType)2, 0, 9999u>` are all in `analysis/classes.tsv`.
They are the runtime of the **III ASN.1 Tool** (Institute for Information Industry, Taiwan,
MPL 1.0; source: <https://github.com/Kampbell/III-ASN.1>). The runtime itself is documented in
[`docs/libraries/asn1-runtime.md`](../libraries/asn1-runtime.md). What matters here is how
its compiler, `asnparser`, turns a type into C++.

For `Federacao ::= SEQUENCE { identificador INTEGER (100..999), ... }`, `asnparser` writes a
class `ModuloFederacoes::Federacao : public ASN1::SEQUENCE` with almost no code. It also writes
static arrays and one **`theInfo`** struct (from `asnparser/main.cxx`,
`SequenceType::GenerateInfo`):

```c++
const void* Federacao::fieldInfos[4] = { &identificador::value_type::theInfo, ... };
int         Federacao::fieldIds[4]   = { -1, -1, -1, -1 };      // -1 = mandatory, n = n-th OPTIONAL
const char* Federacao::fieldNames[4] = { "identificador", "sigla", "nome", "partidos" };
const Federacao::InfoType Federacao::theInfo = {
    Federacao::create /* = inherited ASN1::SEQUENCE::create */, 0x00000010 /*tag*/, 0, false /*extensible*/,
    fieldInfos, fieldIds, 4 /*numFields*/, 0 /*knownExtensions*/, 0 /*numOptional*/,
    NULL, &ASN1::SEQUENCE::defaultTag /*tags*/, fieldNames };
```

The BER encoder and decoder never see `Federacao`. They walk `theInfo`. So this struct, and the
ones it points to, is the schema. It survives compilation because the program needs it at run
time. Three things do **not** survive:

1. **Module-level syntax**: `DEFINITIONS IMPLICIT TAGS`, `IMPORTS`, comments.
2. **Names of simple type aliases.** `QtdEleitores ::= INTEGER (0..9999)` becomes
   `typedef ASN1::Constrained_INTEGER<FixedConstraint, 0, 9999> QtdEleitores;`. Every alias with
   the same range therefore shares **one** template instance (and one `theInfo`). The name is
   gone.
3. The class names of types that the program never builds directly. Their constructor and
   vtable were removed as dead code, and only the `theInfo` struct is left, reachable from a
   parent's `fieldInfos`.

TSE modified the runtime (see the runtime chapter, §7). The change that helps us most:
**`SEQUENCE::InfoType` has one extra field at +48, `const char* name`**, the ASN.1 type name.
It is returned by vtable slot 6 (func 11627) for diagnostics. It fixes problem 3 for every
SEQUENCE.

---

## 3. Recovering the schema, step by step

### 3.1 From a member name to its table

Start from a string you expect to be a member name: `q.py grep qtdEleitoresAptos` finds it at
**@65671**. No function references it, because only data points at it. Search the memory image
for the 4-byte value 65671 (0x00010087). It is found at 1625092, inside an array of string
pointers:

```
$ python3 tools/wasmmap/q.py mem 1625088 28          # pointer lines only
  [1625088] -> 141711: 'idEleicao'
  [1625092] -> 65671:  'qtdEleitoresAptos'
  [1625096] -> 141786: 'qtdEleitoresAptosSecao'
  [1625100] -> 330231: 'qtdEleitoresAptosTTE'
  [1625104] -> 142098: 'resultadosVotacao'
  [1625108] -> 154639: 'ultimoHashVotosVotavel'
  [1625112] -> 154662: 'assinaturaUltimoHashVotosVotavel'
```

That is a `fieldNames[]` array. Searching for the pointer 1625088 finds the word at
1144092+44, the `names` slot of a `SEQUENCE::InfoType` whose `name` (+48) is
`"ResultadoVotacaoPorEleicao"`.

Going one table at a time does not scale, so `extract_asn1.py` **scans the whole image by
shape**. A word *W* starts a SEQUENCE table if:

* *W*+24 (`numFields`) plus *W*+28 (`knownExtensions`) is small,
* the byte at *W*+12 is a bool,
* *W*+44 points to exactly that many pointers, each to an identifier-like string.

CHOICE (`names` at +32, count at +24) and ENUMERATED (`names` at +20, `maxEnumValue`+1
entries) are found the same way. The scan finds 118 + 10 + 58 tables and no false positives.
All of them turn out to share the same `create` function-table slot per kind (below).

### 3.2 Reading one table: `EntidadeBoletimUrna` (the BU)

```
$ python3 tools/wasmmap/q.py mem 1144144 52          # condensed and annotated
1144144  7054 16 0 0            create=slot 7054 (SEQUENCE::create), tag=UNIVERSAL 16, parent=0, extensible=false
1144160  1625120 1625168 11 0   fieldInfos, ids, numFields=11, knownExtensions=0
1144176  2 0 1625216 1625264    numOptional=2, nonOptionalExtensions=NULL, tags, names
1144192  (name) -> "EntidadeBoletimUrna"

ids  @1625168: -1 -1 -1 -1 -1 -1 -1  0 -1 -1  1        members 7 and 10 are OPTIONAL (#0 and #1)
tags @1625216:  0  0  0  0  0  0  0  0x80000001  16 16 16
names@1625264: cabecalho fase urna identificacaoSecao dataHoraEmissao dadosSecaoSA
               qtdEleitoresCompareceram detalhamentoComparecimento resultadosVotacaoPorEleicao
               historicoCodigosCarga historicoVotoImpresso
fieldInfos @1625120: 1141220 (CabecalhoEntidade) 1140604 (Fase) 1144660 (Urna) ...
                     1913292 (DataHoraJE) ... 1597636 (SEQUENCE OF ResultadoVotacaoPorEleicao) ...
```

A tag is stored as `class << 30 | explicit << 29 | number`. `0x80000001` is therefore context
`[1]` (implicit), and `16` is UNIVERSAL 16, the default tag of a SEQUENCE OF member, which means
"not re-tagged". In [`ModuloBoletimUrna.asn`](../../src/asn1/ModuloBoletimUrna.asn) this becomes:

```asn1
EntidadeBoletimUrna ::= SEQUENCE {
    cabecalho                              CabecalhoEntidade,
    fase                                   Fase,
    urna                                   Urna,
    identificacaoSecao                     IdentificacaoSecaoEleitoral,
    dataHoraEmissao                        DataHoraJE,
    dadosSecaoSA                           DadosSecaoSA,
    qtdEleitoresCompareceram               INTEGER (0..9999),        -- spec: QtdEleitores
    detalhamentoComparecimento             [1] DetalhamentoComparecimento OPTIONAL,
    resultadosVotacaoPorEleicao            SEQUENCE OF ResultadoVotacaoPorEleicao,
    historicoCodigosCarga                  SEQUENCE OF GeneralString,
    historicoVotoImpresso                  SEQUENCE OF HistoricoVotoImpresso OPTIONAL
}
```

### 3.3 The member types: one `create` slot per kind

Every table starts with a `create` function pointer. In wasm, a function pointer is an index
into the function table, so all tables of one kind share one slot. That slot identifies the kind
of an anonymous member table:

| table slot | func | body (decompiled) | kind; layout after `{create, tag, parent}` |
|---|---|---|---|
| 7054 | 8962 | `return SEQUENCE::SEQUENCE(operator_new(52), info)` (func 219) | SEQUENCE; see above |
| 7055 | 8999 | `new(16)`, vtable `ASN1::CHOICE`, choiceID −1 | CHOICE: `ext, selectionInfos, numChoices, totalChoices, tags, names` |
| 7053 | 8941 | `new(12)`, vtable `ASN1::ENUMERATED` | ENUMERATED: `ext, maxEnumValue, names` (one name per value, `"<undefined>=n"` for gaps) |
| 2284 | 2654 | `new(12)`, vtable `ASN1::INTEGER` | INTEGER: `constraintType, lower, upper` |
| 2445 | 9010 | vtable `ASN1::SEQUENCE_OF_Base` | SEQUENCE OF: `constraintType, lower, upper, elementInfo` |
| 4444 | 8967 | vtable `ASN1::OCTET_STRING` | OCTET STRING: `constraintType, lower, upper` |
| 7052 | 9003 | vtable `ASN1::AbstractString` | strings: `constraintType, lower, upper, charset, ...`; UNIVERSAL tag 27 = GeneralString, 18 = NumericString |
| 7115 | 8950 | `new(12)`, vtable `ASN1::BOOLEAN` | BOOLEAN |

`constraintType` 2 is `FixedConstraint`, so `{2, 100, 999}` means `INTEGER (100..999)`.
For strings the same triple is a SIZE constraint. BOOLEAN, INTEGER, OCTET STRING, GeneralString,
NumericString, ENUMERATED, SEQUENCE, SEQUENCE OF and CHOICE are the only kinds in use: there is
no BIT STRING, UTF8String, SET or time type in any TSE schema, and no module uses extension
markers (`...`).

### 3.4 Tags

* `SEQUENCE::InfoType.tags` points to either a per-member array, or to
  **`&ASN1::SEQUENCE::defaultTag` (@1149108)**. The latter means "every member keeps its own
  tag" (99 of the 118 SEQUENCEs). In an array, 0 or a UNIVERSAL value also means "not re-tagged".
* `CHOICE::InfoType.tags == NULL` means automatic tags `[0]`, `[1]`, … (6 of the 10 CHOICEs).
* Bit 29 marks an EXPLICIT tag. It appears in `ModuloLocal.IdentificacaoLocal`
  (`[1] EXPLICIT`, `[2] EXPLICIT`), `ModuloEstadoGeralVota.EstadoGeralVota` (`[1]..[5] EXPLICIT`)
  and `ModuloInformacaoMidia.Autenticacao`. Every other tag is implicit. This, and the TSE's own
  published files, is why the rebuilt modules say `DEFINITIONS IMPLICIT TAGS`.

### 3.5 Tables that only exist at run time

Some member tables point outside the initial data segment, e.g. `DataHoraJE` at **1913292**.
The wasm data segment ends at 1811674. Everything above it is `.bss`, which is zero in the file.
Every constrained **string** type (`GeneralString (SIZE(15))`, `NumericString (SIZE(1..5))`…) has
a table whose `parent_info` and character-set fields are copied from `GeneralString::theInfo` or
`NumericString::theInfo`. The copy is made by C++ **dynamic initialisers** that run in
`__wasm_call_ctors` before `main`. `q.py mem` therefore cannot show those 47 tables (40 of them
are used by recovered types). The fix is to run the module and dump the memory after start-up:
[`dump_heap.mjs`](../../src/asn1/tools/dump_heap.mjs) patches `tools/run/headless.mjs` in
memory to write `HEAPU8` from `onRuntimeInitialized`. There,

```
1913292: create=7052 tag=27 parent=1148180 (GeneralString::theInfo) type=2 lower=15 upper=15
         -> DataHoraJE ::= GeneralString (SIZE(15))
```

### 3.6 Naming the types

| source | how | types |
|---|---|---|
| **`name` field at `SEQUENCE::InfoType`+48** (TSE addition) | read directly | all 118 SEQUENCE |
| **RTTI** | Generated constructors store the table address next to the class vtable. An example is func 10273: `l = ASN1_SEQUENCE_SEQUENCE_1(a, 1144144); l[0] = 1597580 /* vtable ModuloBoletimUrna::EntidadeBoletimUrna */` (func 219 is `ASN1::SEQUENCE::SEQUENCE(const void*)`). The helper form stores the vtable first: func 9162 `ecourna_app_dados_asn_ConverteEnum0_merged_body(..., 1128864 /* vtable FormaSuspenderComVoto */, 1138880, ...)`. `extract_asn1.py` pairs such constants in `work/full.wat` and takes a majority vote. | module of 97 SEQUENCE; 35 ENUMERATED, 4 CHOICE, 5 string types |
| **RTTI + structure** | The class exists, but its code copy-constructs from an already decoded object. The table is found through the member the code reads: func 11411 reads `fields[7]` of `EleitorUrna` and stores vtable `NecessidadeEspecial`. | module of `EleitorSequencia`, `EleitorUrna`; `NecessidadeEspecial`, `TipoImpedimento` |
| **`std::source_location` signatures** | e.g. `static ModuloBoletimUrna::TipoVoto::NamedNumber comum::asn::CConversorEntidadeBU::ConverteTipoVoto(...)`, `IConversorASN<ModuloEleicao::CargoPergunta, comum::md::CCargo>` | module of 8 SEQUENCE; 9 ENUMERATED, 1 CHOICE |
| **TSE published spec** | a type with exactly the same members/values and position | `CargoConstitucional`, `TipoApuracao`, `TipoCedulaSA`, `OrigemVotosSA`, `DadosSecaoSA`, `IdentificacaoUrna` |
| **placeholder** (no name in the binary) | from the member that uses it; marked `<<< placeholder name` | CHOICE `IdentificacaoLocal`, `DetalheCargoPergunta`, `InfoDadosComparecimento`; ENUMERATED `TipoEleicao`, `TipoTransferenciaTemporaria`, `TipoSituacaoEleicao`, `SituacaoEleitor`, `ComposicaoBiometria`, `SituacaoCandidatura`, `SituacaoCassacao`, `TipoChave` |

Two placeholders, `TipoEleicao` and `TipoTransferenciaTemporaria`, are also the names of TSE C++
value types: their error-message names are built in the static initialiser func 14478 (strings
@141566, @229277). That makes them likely, not certain.

The SEQUENCE names were first assigned without the +48 field. When the field was read, it
confirmed every name that had come from RTTI, srcloc or the spec (108). Of the 10 names that had
been guessed from member names, it confirmed 4 (`SituacaoEleicao`, `EleitorImpedido`,
`ComplementosMunicipiosUF`, `DescritorChave`) and corrected 6 (`Calendario` became
`CalendarioEleicao`, `Domicilio` became `DomicilioEleitoral`, …). This is why the remaining
CHOICE/ENUM placeholders should be read as labels, not names.

**Modules.** The module of a type is known when a qualified RTTI or srcloc name gives it
(158 of the 186 constructed types, plus the 5 string types). For the other 28 it is inferred
from the types that use it, and the `.asn` file says so.
Typical example: `CargoConstitucional` is used by `ModuloTiposEleitorais::CodigoCargoConsulta`.

### 3.7 Verification

1. **Our own decoder.** `extract_asn1.py --decode FILE` holds a ~100-line BER decoder driven
   only by the recovered tables. With no type given, it tries every SEQUENCE as the top-level
   type and reports those that consume the file exactly. For most files exactly one type fits
   (§6).
2. **An independent compiler.** [`verify_asn1tools.py`](../../src/asn1/tools/verify_asn1tools.py)
   compiles the 32 `.asn` files with `asn1tools` and checks decode → encode → same bytes:

```
$ python3 src/asn1/tools/verify_asn1tools.py  <10 official 2024 files> ModuloEnvelopeGenerico EntidadeEnvelopeGenerico
compiled 32 modules, 191 types
289 files decode and re-encode byte-identically, 0 failures
```

A wrong tag, a missing OPTIONAL, a wrong member order or a wrong base type breaks either the
decode or the byte-identical re-encode. Ranges are not tested by BER. They are read directly
and match the published spec wherever the spec shows them (§5).

---

## 4. The modules

The files: [`src/asn1/<Module>.asn`](../../src/asn1/). Above each type, a comment gives the
table address, the name source and confidence.

### 4.1 Shared types

| module | contents |
|---|---|
| [`ModuloTiposEleitorais`](../../src/asn1/ModuloTiposEleitorais.asn) (32 types) | `DataHoraJE` = `GeneralString (SIZE(15))` `YYYYMMDDThhmmss`; `DataJE` (8 digits), `HoraJE` (6), `SiglaUF` (2). `IDEleitoral` CHOICE of process/pleito/eleição id. `CabecalhoEntidade {dataGeracao, idEleitoral}` starts **every** entity file. `CabecalhoPacote` is the package header stored in `*.pid`. `MunicipioZona`, `IdentificacaoSecaoEleitoral {municipioZona, local, secao}`, `IdentificacaoUrna` (section or contingency). `CodigoCargoConsulta` CHOICE `[1] CargoConstitucional` / `[2] INTEGER (25..99)` (free cargo/question number). Enums `Fase`, `Turno`, `TipoCargoConsulta`, `TipoDedo` (fingers), `CodigoSexo`, `Sistema` (the TSE systems that produce packages: `configurador`, `candidaturas`, `gedai`, `urnaEletronica`, `simulador`, `padaUE`…). `Seguranca` (encryption parameters for the CEPESC library, the government cryptography library). `IdentificadorEleitor` CHOICE (voter registration number / CPF / free id). |
| [`ModuloTiposEcoUrna`](../../src/asn1/ModuloTiposEcoUrna.asn) | `Carga {numeroInternoUrna, numeroSerieFC, identificadorGeradorMidia, dataHoraCarga, codigoCarga}`, the record of the load. `IdentificadorGeradorMidia {nome, serialCertificadoTPM, serialInstalacao}` identifies the computer that generated the load media (by its TPM certificate). `ModeloEquipamento {tpm20(2), ue2013(13), ue2015(15), ue2020(20), ue2022(22)}`. |
| [`ModuloTiposResultadosEcoUrna`](../../src/asn1/ModuloTiposResultadosEcoUrna.asn) | `Urna {tipoUrna, versaoVotacao, correspondenciaResultado, tipoArquivo, numeroSerieFV, motivoUtilizacaoSA}`, the urna identity stamped on every result file. `TipoArquivo` (produced by the urna, the RED data recoverer, or the SA in its modes). `TipoApuracaoSA` and the `Motivo*` enums say why the SA was used. |
| [`ModuloTiposCadastro`](../../src/asn1/ModuloTiposCadastro.asn) | `Municipio {codigo, nome, capital, [1] codigoIBGE, [2] comBiometria}`, `TipoLocalVotacao` (normal, emTransito, presoProvisorio, temporario). |
| [`ModuloTiposPacotes`](../../src/asn1/ModuloTiposPacotes.asn) | `TipoPacote`: 56 named package kinds (values 1..65) exchanged between TSE systems (`pacoteEleitores(1)`, `pacoteParametrosUrna(23)`, `pacoteFotosCandidatosTRE(8)`…). |

### 4.2 Results produced by the urna

**`ModuloBoletimUrna` — the BU.** `EntidadeBoletimUrna` (§3.2) holds, for each eleição, a
`ResultadoVotacaoPorEleicao`. It lists the numbers of eligible voters (total, originally in the
section, TTE), a `ResultadoVotacao` per cargo type, and for each cargo a `TotalVotosCargo` with
one `TotalVotosVotavel {tipoVoto, quantidadeVotos, identificacaoVotavel, ordemGeracaoHash,
hash}` per candidate/party/blank/null line. The last two members form a **hash chain**. The TSE
documentation describes the first link as
`idPleito(5)|idEleicao(5)|municipio(5)|zona(4)|secao(4)|codigoCarga(24)`. The binary has the
format string `"{:05}|{:05}|{:05}|{:04}|{:04}|{:24}"` (@8483), used only by func 10273. That
function is the BU converter, `comum::asn::CConversorEntidadeBU::DoConverte`: it is slot 2 of
that class's vtable, and it builds the table-1144144 object. `functions.tsv` currently
mislabels it `CDadosBUVota::GetDataHoraDesligamentoVotoImpresso`, after the srcloc of an
inlined callee. Each line hashes the previous hash and its own fields. `ResultadoVotacaoPorEleicao`
ends with `ultimoHashVotosVotavel` and its signature `assinaturaUltimoHashVotosVotavel`
(`comum::asn::CAssinaVotavelBu::Assinar`, srcloc `cassinavotavelbu.cpp:89`). The BU `TipoVoto`
is `nominal(1), branco(2), nulo(3), legenda(4), cargoSemCandidato(5)`.

**`ModuloRegistroDigitalVoto` — the RDV.** `EntidadeResultadoRDV {cabecalho, urna, rdv}` wraps
`EntidadeRegistroDigitalVoto {pleito, fase, identificacao, historicoCodigosCarga, eleicoes}`.
`Eleicoes` is a CHOICE of `[1] SEQUENCE OF EleicaoVota` (urna) or `[2] SEQUENCE OF EleicaoSA`
(counting system, with the ballot type and vote origin). Each `VotosCargo` holds
`SEQUENCE OF Voto {tipoVoto, digitacao VotoDigitado OPTIONAL}`. `VotoDigitado` is
`NumericString (SIZE(1..5))`: the digits **as typed** by the voter. The RDV `TipoVoto` is
finer-grained than the BU's: `legenda(1) nominal(2) branco(3) nulo(4) brancoAposSuspensao(5)
nuloAposSuspensao(6) nuloPorRepeticao(7) nuloCargoSemCandidato(8)
nuloAposSuspensaoCargoSemCandidato(9)`. In this web build votes are not persisted to the RDV: `rdv.dat` stays at 80 bytes (see
`analysis/runtime/README.md`). The type and its encoder (`CFileASN::CodeObjectFunction`, func
11488 / 11587) are compiled in anyway.

**`ModuloEnvelopeGenerico`.** `EntidadeEnvelopeGenerico {cabecalho, fase, urna OPTIONAL,
identificacao, tipoEnvelope, seguranca OPTIONAL, conteudo OCTET STRING}` wraps a result file.
`tipoEnvelope`: `envelopeBoletimUrna(1)`, `envelopeRegistroDigitalVoto(2)`,
`envelopeBoletimUrnaImpresso(4)` (the printed BU text, `*-imgbu.dat`),
`envelopeImagemBiometria(5)`, **`envelopeZeresimaImpressa(6)`** (the printed zerésima). When
`seguranca` is present, `conteudo` is encrypted.

**`ModuloResultadoUrnaCadastro`.** Data returned to the voter registry: for each voter,
`SituacaoComparecimentoEleitor` (faltou/naoVotou/votou/semCargoParaVotar), how they were enabled
(biometric attempts, finger, last score, enabled by code by a mesário, audio), photo display
status, `justificativas` (voters who justified absence, with birth year), and the mesários
present at opening and closing. `InfoDadosComparecimento` selects the plain or the encrypted
(`DadosComparecimentoCifrado {dadosCifracao {chave, salt, informacaoAdicional}, conteudo}`)
variant.

**Also:** `ModuloHashes.EntidadeHashes` (hashes of the installed files, `versaoUENUX` = OS
version), `ModuloVersaoArquivos.EntidadeVersaoArquivos` and
`ModuloEnvelopeChave.EntidadeChave` (a key `{tagChaves, descritor {nomeUsuario, serial},
cifrado, tipo, chave}`). `DescritorChave` is the only type the binary shares with the TSE's
signature spec.

### 4.3 Election data loaded into the urna

| module | top-level type | what it describes |
|---|---|---|
| `ModuloProcessoEleitoral` | `EntidadeProcessoEleitoral` | name, `pleito1`/`pleito2` (`Pleito {id, nome, data}`), main election type and scope, whether mesa-receptora-de-justificativa and biometrics are used, allowed TTE types, voter id kinds, UFs |
| `ModuloEleicao` | `EntidadeEleicao` | one eleição: `cargos` as `SEQUENCE OF CargoPergunta {codigo, tipo, numeroDigitos, qtdeEscolhas, podeRepetir, ordemAquisicao/Impressao/Apuracao, paginaImpressaoVoto, detalhe}`. `detalhe` is either a `DetalheCargo` (names in neutral/male/female/abbreviated forms, `suplencias` = running mates such as vice or suplente) or a `DetalhePergunta` (referendum question with `RespostaPergunta` answers and `textoFonetico` for the audio). Optional `CalendarioEleicao` |
| `ModuloSituacoesEleicoes` | `EntidadeSituacoesEleicoes` | per eleição: `ativa/suspensa/cancelada/excluida` and the three orderings |
| `ModuloCandidatos` | `EntidadeCandidatos` | `CandidatosPorCargos` → `CandidatoPorPartido` → `Candidatura {numero, titular DadosCandidato, drap, suplentesVices}`. `DadosCandidato` has name, ballot name, social name, phonetic name, birth date, sex, `SituacaoCandidatura` (deferido, indeferidoComRecurso, …) and `SituacaoCassacao` |
| `ModuloFotosCandidatos` | `EntidadeFotosCandidatos` | `FotoCandidato {codigoCandidato, foto Foto {formato JPEG/BMP, imagem}}` (read with TSE's streaming "partial" decoder) |
| `ModuloPartidos` / `ModuloFederacoes` | `EntidadePartidos` / `EntidadeFederacoes` | parties `{numero, sigla, nome}`; federations `{identificador (100..999), sigla, nome, partidos}` |
| `ModuloParametrizacaoUrna` | `EntidadeParametrizacaoUrna` | the **PU**, 47 parameters: how a suspended vote is closed (`FormaSuspenderSemVoto/ComVoto`), copies of each report (zerésima, BU for Vota/RED/SA), time-outs (vote confirmation, suspension, automatic shutdown), report headers and footers, labels ("Município", "Zona"… with gender and plural), and flags `imprimirQrCodeNoBU`, `aceitarJustificativa`, `aceitarBrancoNulo`, `exibirScoreBiometria`, `registrarMesarios`, `pedeAnoNascimentoEleitor`… |
| `ModuloConfiguracaoMunicipios` | `EntidadeConfiguracaoMunicipios` | per municipality: times of zerésima, start, end and termination of voting |
| `ModuloComplementosMunicipios` | `EntidadeComplementosMunicipios` | per UF and municipality: time zone (`fuso` −720..720 minutes), daylight saving, `desligaColetaBiometria` |
| `ModuloLocal` | `Local` | this urna's place: country, `UF`, `Municipio`, complements, and `IdentificacaoLocal` = `[1] SecaoEleitoral` (with aggregated sections) or `[2]` contingency |
| `ModuloEleitores` | `EntidadeEleitores` | the section's voters: `EleitorSequencia {sequencial, eleitor EleitorUrna, [1] tipoTransferenciaTemporaria}`. `EleitorUrna` has ids, name, social name, mother's name, birth date, `SituacaoEleitor`, `[2] DomicilioEleitoral` (home UF/zone/section and dates; probably used for TTE voters), `NecessidadeEspecial` (`necessitaAudio`) and `BiometriaEleitorCifrada {composicaoBiometria, conteudo, salt}` (encrypted photo/fingerprint templates, `BiometriaEleitor`/`Dedo {tipo, qtdMinucias, minucias}` once decrypted) |
| `ModuloImpedidos` | `EntidadeImpedidos` | voters who cannot vote here, per round: `TipoImpedimento` (votaNaSecaoOriginal, solicitouVotoEmTransito, presoProvisorio, suspenso, cancelado, semIdadeMinima, tte*…) |
| `ModuloInformacaoMidia` | `InformacaoMidia` | what a flash medium is: `tipoMidia` (`mr` = memória de resultado, `fc` = flash de carga, `fv` = flash de votação), fase, pleito, UF, round, `DadosGeracaoMidia {serialMidia, usuario, identificadorGeradorMidia, data}` and the applications it enables (`vota`, `sa`, `red`, …, with optional password authentication) |
| `ModuloDadosDisponiveisCarga` | `DadosDisponiveisCarga` | what the load medium offers: municipalities with their sections, and the imported package headers |

### 4.4 The urna's persistent state

| module | file | contents |
|---|---|---|
| `ModuloEstadoGeralUrna` | `dinamico/eg.bin` | `EstadoGeralUrna`: pleito, `EstadoUrna` (carregando/carregada/testada), `DadoCarga` (round, type in rounds 1 and 2, model, fase), `DadoLocal`, clock adjustment, the load `DadoCorrespondencia` (with a signature), software version, hash of package versions |
| `ModuloEstadoGeralVota` | `trab1/vota.bin`, `trab2/vota.bin` | `EstadoGeralVota`: the VOTA state machine (`EstadoVota`: inicial … gerarze, zeresimaimpressa, registromesarioinicial, **votar**, fimaquisicaovotos … gerarbu, imprimirbu, gravarresultados, encerrada), BU count, attendance, justifications, and `[1]..[5] EXPLICIT` optional members (urna that generated the zerésima; start/end of vote acquisition, last vote, BU emission timestamps) |
| `ModuloEstadoGeralSA` | `sa.bin` | counting-system state machine (`EstadoSA`, 24 states) and section |
| `ModuloEstadoGeralGap` | `gap.bin` | the application launcher: up to 10 load correspondences, current and previous application (`UrnaAplicativo`: apvotatreinaeleitor1 … apvotaoficial2, apapuracao*), RED executed, audio, 2nd-round date |
| `ModuloEstadoGeralDefs` | — | `NumViasImpressasRelatorios` (copies already printed) |

A decoded `eg.bin` shows how much of this state the simulator makes up. It says:
`modelo ue2015, numeroInternoUrna 87654321, codigoCarga "123456789012345678901234",
dataHoraCarga "20201231T235958", identificadorGeradorMidia {nome "nome_maquina",
serialCertificadoTPM "12345678", serialInstalacao "99999999"}, versao "7.2.1.3 - TESTE EG ASN1"`.
These are test fixtures hard-coded in the binary: @354064 (the `versao`) is used by func 10268,
@227209 `nome_maquina` (with @341300 / @339893) by func 9952, and @346764 (the `codigoCarga`) by
func 10261.

---

## 5. Comparison with the TSE's published specifications

The TSE publishes a simplified ASN.1 of the result files with its *documentação técnica do
software da urna* / open-data pages: `bu.asn1`, `rdv.asn1`, `assinatura.asn1` with Python
examples. The 2022 version (v1) and the 2024 version (v2) were used here, from public mirrors
(`github.com/danarrib/TSEParser` `TSE_Docs/`, `github.com/doccaz/urnas-br` `doc/` and `docv2/`).
The v2 header says it "aggregates, for didactic purposes, `tiposeleitorais.asn1,
tiposecourna.asn1, tiposresultadosecourna.asn1, envelopegenerico.asn1, boletimurna.asn1`".
These are exactly the modules `ModuloTiposEleitorais`, `ModuloTiposEcoUrna`,
`ModuloTiposResultadosEcoUrna`, `ModuloEnvelopeGenerico` and `ModuloBoletimUrna` found in the
binary.

Comparison by type, over members/values, order, OPTIONAL, tags and ranges:

| published file | identical | different |
|---|---|---|
| `bu.asn1` v2 (2024), 41 types | 38 | 3 enumerations (below) |
| `rdv.asn1` v2 (2024), 35 types | 33 | 2 enumerations (below) |
| `assinatura.asn1` v2, 12 types | `DescritorChave` | `ModeloEquipamento` has fewer values; the other 10 types are **absent** from the binary |
| `bu.asn1` v1 (2022) | 32 | `EntidadeBoletimUrna`, `ResultadoVotacaoPorEleicao`, `TotalVotosVotavel`, `Carga` changed; `IdentificacaoMesaJustificativa` removed; the 3 enumerations below |

Every constrained INTEGER, OCTET STRING and string alias in the v2 files has the same range as
the table in the binary. The differences from v2:

| type | published v2 (2024) | binary (10.23.0.1) | effect on the bytes |
|---|---|---|---|
| `TipoEnvelope` | 1, 2, 4, 5 | adds **`envelopeZeresimaImpressa(6)`** | new value |
| `TipoUrna` | `reservaSecao(4)`, `reservaEncerrandoSecao(6)` | `contingenciaSecao(4)`, `contingenciaEncerrandoSecao(6)` | same numbers, other names |
| `MotivoApuracaoMistaComBU` | `urnaChegouAposInicioVotacao(5)` (also v1) | `urnaEncerradaComEleitoresNaFila(5)` | **same number, different meaning** |
| `ModeloEquipamento` | `tpm12(1)`, `ue2009`, `ue2010`, `ue2011` … | only `tpm20, ue2013, ue2015, ue2020, ue2022` | older models dropped |

From 2022 to 2024 the data model changed:

* `Carga` gained `identificadorGeradorMidia`.
* The RDV gained `historicoCodigosCarga`.
* The BU replaced `qtdEleitoresLibCodigo`/`qtdEleitoresCompBiometrico` with
  `qtdEleitoresCompareceram` + `[1] DetalhamentoComparecimento`, replaced
  `historicoCorrespondencias` with `historicoCodigosCarga`, and moved the signature from one
  `chaveAssinaturaVotosVotavel` to the per-line hash chain.
* The mesa-de-justificativa identification disappeared.
* The RDV `Eleicoes` CHOICE tags moved from `[0]`/`[1]` (v1 text) to `[1]`/`[2]`.

The binary has the 2024 shape **plus** the later changes above. Consistent with that, the two v1
example BUs and two v1 example RDVs (`TSE_Docs/exemplos`, `s02100-…` and `t02510-…`) fail to
decode with the recovered schema at exactly `Carga.identificadorGeradorMidia`.

**Official 2024 files decode byte-exactly.** The TSE's 2024 BU examples (the zip's `leiame.txt`
is headed "QR Code no Boletim de Urna") hold 10 sample printed-BU envelopes
(`TSE-exemplos-boletim-urna-eleicoes-2024.zip`, from the 2024 *simulado*: normal
section, RED, SA manual count, a *consulta popular*, a Senate race). Each one is an
`EntidadeEnvelopeGenerico` with `urna.versaoVotacao = "9.29.0.1 - Kayapó"`. The media generator
is `tsesevinw09` with a real TPM serial, and `tipoEnvelope = envelopeBoletimUrnaImpresso`. All 10
decode with the recovered modules and re-encode to identical bytes.

Know what this test covers. The zip's `leiame.txt` calls these files a "cópia do conteúdo enviado
para a impressora da urna" (a copy of what was sent to the printer). `conteudo` holds that
printer text (e.g. `"Justiça Eleitoral … Boletim de Urna …"`), not an `EntidadeBoletimUrna`. So
the byte-exact result proves the real 2024 software uses the same **envelope and header** types:
`EntidadeEnvelopeGenerico`, `CabecalhoEntidade`, `Fase`, `Urna` (`TipoUrna` 1 and 3,
`TipoArquivo` votacaoUE / votacaoRED / saManual), `CorrespondenciaResultado`, `Carga`,
`IdentificadorGeradorMidia`, `IdentificacaoSecaoEleitoral` and `TipoEnvelope`. The BU body
(`EntidadeBoletimUrna`, `ResultadoVotacaoPorEleicao`, `TotalVotosVotavel`…) and the RDV body are
checked only against the published v2 text (table above). No real v2 `-bu.dat` or `-rdv.dat`
was decoded.

**The 2026 package.** TSE's 2026 "Formato dos arquivos de BU, RDV e assinatura digital" package
(README dated 2026-09-10) was read from a third-party copy, not from TSE's site (see "Sources and
provenance" in the investigation report). Compared with the binary:

* `bu.asn1` equals `src/asn1` on 41/41 structured types, including the three enumeration changes
  of the table above (`envelopeZeresimaImpressa(6)`, `contingenciaSecao(4)`,
  `contingenciaEncerrandoSecao(6)`, `urnaEncerradaComEleitoresNaFila(5)`).
* `assinatura.asn1` has the binary's `ModeloEquipamento {tpm20, ue2013, ue2015, ue2020, ue2022}`
  and adds `OrigemAssinaturaHardware {modeloEquipamento, algoritmoAssinatura}` and
  `InfoChave ::= CHOICE {tagChaves [0], certificadoDigital [1]}`. It decodes and re-encodes
  110/110 real 2026 signature files; the v2 text fails on all 110.
* `rdv.asn1` is byte-identical to the 2024 v2 file and still says `reservaSecao(4)`,
  `reservaEncerrandoSecao(6)` and `urnaChegouAposInicioVotacao(5)`. Decoding is unaffected (same
  numbers), but `MotivoApuracaoMistaComBU = 5` means "arrived after voting started" in one TSE
  file and "closed with voters still in line" in the other.

The real 2026 files of 135 sections carry no coded value outside the binary's enumerations (2026
urna data: [`investigation/README.md`](../../investigation/README.md), findings H6, H7, H8, B22).

---

## 6. Which scenario files use which type

Files in `upstream/fs/municipal-t1/dsk/fi/estatico/`. **Method:** each file was decoded with
every recovered SEQUENCE as the top-level type (`extract_asn1.py --decode`). The *package* files
(`*.pid`) say which package the `.dat` belongs to. The reader functions give a third check
(`api::CFileASN::ReadFromFile [T = …]` srclocs, funcs 3735, 3771, 5706, 5740, 5778, 11517,
11905…).

**Name pattern.** `t` = fase *treinamento*; `02400` = processo eleitoral, `02410` = pleito,
`02411` = eleição; `ac` = UF; then municipality (5 digits), zone (4) and section (4).
`t02400ac000010001-el` is therefore "voters, process 2400, AC, municipality 1, zone 1". The
matching `IDPacote` in the `.pid` says `{idProcessoEleitoral 2400, fase treinamento, siglaUF
"AC", codigoMunicipio 1, numeroZona 1}`.

| file (size) | type | notes |
|---|---|---|
| `*.pid` (46–68 B) | `ModuloTiposEleitorais.CabecalhoPacote` | e.g. `t02411ac00001-ca.pid` = `pacoteCandidatosTRE`, `nomepacote "t02411ac00001-ca.jez"`, `versao "202609101622"`, `origem padaUE` |
| `t00000br-pu.dat`, `t02400ac-pu.dat` (1421) | `ModuloParametrizacaoUrna.EntidadeParametrizacaoUrna` | package `pacoteParametrosUrna` |
| `t02400-cp.dat` (169) | `ModuloProcessoEleitoral.EntidadeProcessoEleitoral` | `pacoteConfiguracaoProcessoEleitoral` |
| `t02410ac-cfm.dat` (254) | `ModuloConfiguracaoMunicipios.EntidadeConfiguracaoMunicipios` | |
| `t02410ac-cm.dat` (74) | `ModuloComplementosMunicipios.EntidadeComplementosMunicipios` | `pacoteComplementosMunicipios` |
| `t02410ac-ste.dat` (45) | `ModuloSituacoesEleicoes.EntidadeSituacoesEleicoes` | `pacoteSituacoesEleicoes` |
| `t02411ac-ce.dat` (305) | `ModuloEleicao.EntidadeEleicao` | `pacoteEleicao`; Prefeito (vice = free cargo 99) and Vereador |
| `t02411ac00001-ca.dat` (1704) | `ModuloCandidatos.EntidadeCandidatos` | `pacoteCandidatosTRE` |
| `t02411ac00001-fo.dat` (330 018) | `ModuloFotosCandidatos.EntidadeFotosCandidatos` | `pacoteFotosCandidatosTRE` |
| `t02411ac00001-pa.dat` (227) | `ModuloPartidos.EntidadePartidos` | |
| `t02411ac00001-fe.dat` (25) | `ModuloFederacoes.EntidadeFederacoes` | header only, `federacoes` absent (OPTIONAL) |
| `t02400ac0000100010001-el.dat` (137) | `ModuloEleitores.EntidadeEleitores` | one voter, `NOME OMITIDO`, `nomeMae "Não utilizado"` |
| `t02400ac0000100010001-tte.dat` (42) | `ModuloEleitores.EntidadeEleitores` (medium) | no voters; the `.pid` says `pacoteTransferenciaTemporariaEleitores`. `EntidadeImpedidos` has the same shape when empty |
| `t02400ac0000100010001-imp.dat` (66) | `ModuloImpedidos.EntidadeImpedidos` | |
| `0000100010001-lo.dat` (87) | `ModuloLocal.Local` | name built with `"{:05}{:04}{:04}-lo.dat"` (@60623) |
| `infomidia-fv-1-t.dat` (172) | `ModuloInformacaoMidia.InformacaoMidia` | generated by `"simulador-votacao-ng"`, TPM serial all zeros |
| `t02400ac-mme.dat` (27), `t02411ac00001-pi.dat` (27) | undetermined | header + empty `SEQUENCE OF`: fits 4 types. The `.pid` of `mme` says `pacoteMigracaoMunicipioEleicao` |
| `t02411ac00001-co.dat`, `-le.dat` (41) | undetermined | header + `Abrangencia` + empty list: fits `EntidadeCandidatos`, `EntidadePartidos` and `EntidadeFotosCandidatos`. Both are signed inside the candidates package (see `*.vsc`), with `-pi`, `-fe`, `-pa` and `-rdj` |
| `t02400ac-mz.dat` (59) | **module not in the binary** | `pacoteMunicipiosZonas`: `{cabecalho, SEQUENCE OF {uf, SEQUENCE OF MunicipioZona}}` |
| `t02400ac-mu.dat` (118) | **module not in the binary** | `{cabecalho, SEQUENCE OF {UF, SEQUENCE OF Municipio}}`: the elements are `ModuloTiposEleitorais.UF` and `ModuloTiposCadastro.Municipio`; signed in the `-mz` package |
| `t02400ac000010001-se.dat` (150) | **module not in the binary** | `pacoteSecoes`: polling places with their sections |
| `scueconf-t1.dat` (78) | **module not in the binary** | a load configuration: `INTEGER 2410`, and a package list with `"pacoteunico.sh"` |
| `t02411ac00001-rdj.dat` (29) | not ASN.1 | Latin-1 text `"registro de decisão judicial"` |
| `*.vsc` (0 B, or 14–73 KB) | `EntidadeAssinatura` of the **published** `assinatura.asn1` (not compiled into the wasm) | Package signature: `algoritmoHash sha512`, `algoritmoAssinatura cepesc, 256 bits`, key serial `20260223`, then `[0] "202602231028"` (key set). The inner `Assinatura` lists the signed files. The signer (`nomeUsuario`) is the TSE unit that produced the package: `SEVIN` (PU), `SEINT` (processo, eleição, situações, mme), `SECAD` (eleitores, impedidos, TTE, municípios/zonas, complementos), `SECINP` (candidates and photos). `t02411ac00001-ca.vsc` signs `-ca`, `-co`, `-fe`, `-le`, `-pa`, `-pi`, `-rdj` + `.pid` + a `.ver`. The zone packages `-el`/`-imp`/`-tte` sign six sections (0001–0006), of which only 0001 is shipped. Many `.vsc` files are empty (0 bytes): those of the per-section files (covered by their zone package), and those of `-se`, `-lo`, `infomidia` and `scueconf`, which no signature in the scenario covers |

The four "module not in the binary" files are for systems upstream of VOTA (the load
preparation). The urna application reads the already-assembled `Local` / `DadosDisponiveisCarga`
instead.

Files written at run time (`analysis/runtime/memfs-after-vote/dsk/…`): `eg.bin`
(`EstadoGeralUrna`), `vota.bin` (`EstadoGeralVota`; after one vote `estadoVota = votar`,
`treinamentoEleitor = TRUE`), `sa.bin`, `gap.bin`. The rest is not ASN.1: `rdv.dat` (80 bytes,
not BER, looks encrypted), `uenux.db` (SQLite), `*.vsu` (the text `assinatura simulada para
vota_web_wasm`), `logd.dat` (text log) and `serialv.dat` (`ABCDDCBA`).

### 6.1 Decoding a file by hand: `t02400-cp.pid` (46 bytes)

```
30 2c                          CabecalhoPacote ::= SEQUENCE, 44 bytes
   0a 01 0e                    tipoPacote   ENUMERATED 14 = pacoteConfiguracaoProcessoEleitoral
   30 07                       idPacote     IDPacote ::= SEQUENCE
      81 02 09 60                 idPacoteEleitoral: IDEleitoral is a CHOICE, [1] = idProcessoEleitoral = 2400
      0a 01 03                    fase ENUMERATED 3 = treinamento
                                  (siglaUF [1], codigoMunicipio [2], numeroZona [3]: OPTIONAL, absent)
   1b 0d 74 30 32 ... 7a       nomepacote   GeneralString "t02400-cp.jez"
   12 0c 32 30 32 36 ... 32    versao       NumericString (UNIVERSAL 18) "202609101622"
                               abrangencia  [1] OPTIONAL, absent
   0a 01 0e                    origem       Sistema ENUMERATED 14 = padaUE
```

Notice that the same byte `0x81` means `idProcessoEleitoral` inside `IDEleitoral` and would
mean `siglaUF` directly inside `IDPacote`. Context tags only mean something relative to the
enclosing type. Without the schema, a BER dump cannot tell them apart.

---

## 7. What the schemas say about "is this the software of the real urna?"

This question comes up with the TSE hash lists in the project root. `pc1hsfg.html` is
*Eleições Gerais 2026 — HOT SWAP FLASH — 02/09/2026*: SHA-512 digests of
`D:\Aplic\SEVIN\HSF\HotSwapFlash.exe`, `NotificadorHSF.exe`, an icon and MinGW DLLs.
`pc1geda.html` is *GEDAI-UE*, the TSE application that prepares the urna media. It lists
`gedai-ue.exe`, Qt 6 DLLs, and the packages it writes to the media: `vota_apl.jez`,
`vota_ofi.jez`, `vota_sim.jez`, `vota_tre.jez`, `dsk*.img`, `vpe99_*.jez`…
[`docs/00-provenance.md`](../00-provenance.md) covers the general argument. The ASN.1 layer adds
these facts:

1. **Same data model and codebase.** The simulator's modules are the ones the TSE documents for
   the real urna's BU and RDV, with the same type names, members, tags and ranges. Official
   printed-BU envelopes written in 2024 by the real urna software (`9.29.0.1 - Kayapó`) decode
   and re-encode exactly with the schema taken from this wasm. That test covers the envelope,
   header, urna-identity and load-record types; the BU and RDV bodies match the published text
   but were not tested against real result files (§5). The generated code, the TSE-modified
   III ASN.1 runtime and the `Modulo*` naming are the urna's.
2. **A later version than 2024.** `envelopeZeresimaImpressa`, the renamed `TipoUrna` values, the
   changed meaning of `MotivoApuracaoMistaComBU(5)` and the shorter `ModeloEquipamento` are not
   in any published spec. This fits `"10.23.0.1 - DESENVOLVIMENTO"`: a development build of the
   2026-cycle code, not the sealed release.
3. **Not the real load chain.** The medium's `InformacaoMidia` says it was generated by
   `"simulador-votacao-ng"` with an all-zero TPM serial. A real medium is generated by GEDAI-UE
   on a machine with a TPM; the 2024 samples carry `tsesevinw09` and a real TPM serial. The
   urna's state `eg.bin` is a hard-coded test fixture (§4.4). The signature module
   (`EntidadeAssinatura`, `.vscmr` files) is not compiled in, and the `.vsu` "signatures" are a
   fixed text. The **input packages**, however, carry signature files in the official format
   (`*.vsc`, algorithm `cepesc`). The signers are `SEVIN`, `SEINT`, `SECAD` and `SECINP`;
   `SEVIN` is also the directory in the `pc1*.html` paths (`D:\Aplic\SEVIN\…`). Whether those
   signatures verify was not checked.
4. **Why the hash lists cannot settle it.** `pc1hsfg.html` hashes only the Windows *Hot Swap
   Flash* tool (7 files and a combined "hash geral"); it names no urna package at all.
   `pc1geda.html` hashes GEDAI-UE and the `.jez`/`.img` packages it installs. None of them is,
   or contains, this wasm module. Checked: none of the 78 digests in the two files (75
   distinct) equals the SHA-512 of any of the 1,047 local files (the wasm, the glue, the data
   bundles, the unpacked filesystem, the files written at run time). A WebAssembly build of the
   same sources never has the same bytes as the native build inside `vota_*.jez`. At the
   data-format level the answer is **yes**: the same codebase (`uenux2`, `ecourna`) and the same
   file formats. Whether the application code matches the 2026 release line for line cannot be
   decided from these files. At the binary level it is **no**: a different,
   development build, with the hardware, the load chain and the signatures simulated.

---

## 8. Limits and open points

* **Placeholders.** 11 CHOICE/ENUMERATED names are placeholders (§3.6). Enumeration *values*
  and member names are certain; only the type identifiers are guessed.
* **Inferred modules.** 28 types have an inferred module (IMPORTS lists are built from that).
  Moving a type to another module does not change a single encoded byte.
* **Alias names.** Integer/OCTET STRING aliases have no names in the binary. The `-- spec:`
  comments carry the published names where the TSE files have them. Two string aliases are
  shared by several members: @1913468 `GeneralString (SIZE(0..18))` (candidate codes, 4 members)
  and @1913028 `NumericString (SIZE(12))` (2 members). They have their own table but no name. The RTTI class `ModuloTiposEleitorais::VersaoMontagem` is probably
  @1913028, `NumericString (SIZE(12))`, used by `CabecalhoPacote.versao` and
  `EntidadeChave.tagChaves`. Its only constructor copies from another object (func 12110), so
  this is not proven.
* **Unused modules.** Only types reachable from a compiled table are recovered. Whole modules
  that VOTA does not link (municipios/zonas, seções, the signature module, SA/RED/GEDAI-only
  entities) are absent. Their existence is known only from files (§6) and the published docs.
* **Tag default.** `IMPLICIT TAGS` is inferred: all non-CHOICE tags are implicit unless flagged
  explicit, and the published files use it. `asnparser` output would be the same for a module
  that writes `IMPLICIT` on each tag.

---

## 9. Reproduce

```sh
# 1. memory after static constructors (needs node; ~3 s)
node src/asn1/tools/dump_heap.mjs /tmp/vota_heap.bin
# 2. rebuild src/asn1/*.asn (scan + name + print); --json dumps the model
python3 src/asn1/tools/extract_asn1.py --heap /tmp/vota_heap.bin --out src/asn1
# 3. decode one file with the recovered tables (all candidate types, or a given one)
python3 src/asn1/tools/extract_asn1.py --heap /tmp/vota_heap.bin \
        --decode upstream/fs/municipal-t1/dsk/fi/estatico/t02411ac-ce.dat EntidadeEleicao
# 4. independent check with asn1tools (pip install asn1tools)
python3 src/asn1/tools/verify_asn1tools.py
```

Useful `q.py` commands for this chapter:

```
q.py mem 1144144 52        # SEQUENCE table of EntidadeBoletimUrna (name at +48)
q.py mem 1625264 44        # its member names
q.py mem 1140568 36        # CHOICE table IDEleitoral (tags [1] [2] [3] at 1621776)
q.py mem 1143720 24        # ENUMERATED table of the BU TipoVoto (max 5, names at 1624672)
q.py f 10273               # BU converter: builds EntidadeBoletimUrna, hash-chain header format
q.py f 5825                # CFileASN::DecodeObjectFunction<EntidadeRegistroDigitalVoto>
```

Functions named from this work (the `create` entries of §3.3, and func 10273 = BU converter) are
in the structured results for the coordinator. The runtime functions themselves (visitors,
coders, constructors) are covered in [`docs/libraries/asn1-runtime.md`](../libraries/asn1-runtime.md).
