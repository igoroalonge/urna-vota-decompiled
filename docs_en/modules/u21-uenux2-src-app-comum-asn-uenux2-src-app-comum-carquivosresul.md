# u21 — `comum::asn`: the voting application's ASN.1 converter framework, its converters and helpers, and the result / SAVD file-name tables

Unit u21 has **103 functions**; **36** were seen running during the recorded votes (`analysis/runtime/*.functions.tsv`).
Its attested original files are:

| original file | what it holds |
|---|---|
| `uenux2/src/app/comum/asn/iconversorasn.h` | the class template `comum::asn::IConversorASN<ENTIDADE, DADO>`: base of every converter between an ASN.1 object and an application data object |
| `uenux2/src/app/comum/asn/iconversorbiometriaasn.h` | the same idea for encrypted fingerprints (`IConversorBiometriaASN`, extra `md::CParametro`) |
| `uenux2/src/app/comum/asn/legravaentidade.h` | `LeEntidadeEm` / `LeEntidadeBiometria`: read ONE BER element at a byte offset of a big file |
| `uenux2/src/app/comum/asn/util.cpp` | `comum::asn::Utils`: static enum / date conversions (fase, abrangência, tipo de local, tipo de pacote, sistema…) |
| `uenux2/src/app/comum/asn/cconversorabrangencia.cpp` | `CConversorAbrangencia` (scope of a data file) |
| `uenux2/src/app/comum/asn/cconversorcabecalhoentidade.cpp` | `CConversorCabecalhoEntidade` (header of every data / result file, including the BU) |
| `uenux2/src/app/comum/carquivosresultado.cpp` | `CArquivosResultado::operator[]`: suffix of each result file (`bu.dat`, `rdv.dat`, `hash.dat`, …) |
| `uenux2/src/app/comum/carquivossavd.cpp` | `CArquivosSavd::operator[]`: path of each signed file / signature package |

Because the template is instantiated and inlined everywhere, the unit also received converter methods of about
twenty other `comum::asn::CConversorXxx` classes (their vtables give the real names), two photo functions of
`cfotos.cpp`, a GUI data source of `ccandidaturas.cpp`, two constructors of `md::CComplementoMunicipio` and a
dozen ASN.1-runtime / libc++ helpers. Section 14 maps every index.

---

## 1. Glossary

| term | meaning |
|---|---|
| *conversor* | converter object between an ASN.1 *entidade* and an application *dado* |
| *entidade* (`TEntidade`) | a class generated from the TSE ASN.1 modules (`ModuloTiposEleitorais::CabecalhoEntidade`, …) on top of the III ASN.1 runtime (`docs/libraries/asn1-runtime.md`); BER-encoded in the files |
| *dado* (`TDado`) | the application's data object (`comum::md::CCabecalhoEntidade`, `md::CAbrangencia`, …), often a range-checked value class |
| **Converte** / **Desconverte** | dado → entidade (to WRITE a file) / entidade → dado (after READING a file). ecourna spells the second one *Deconverte* |
| *DoConverte* / *DoDesconverte* | the virtual workers (vtable slots 2 and 3) each concrete converter overrides |
| *cabeçalho* | header of a file: generation date-time + election id (`IDEleitoral`: processo eleitoral / pleito / eleição) |
| *abrangência* | geographic scope of a data file: municipal / estadual / federal (+ UF and município) |
| *fase* | phase of the installation: **oficial** (real election), **simulado** (public simulation), **treinamento** (training). The application stores it as the character `'1'`/`'2'`/`'3'` (`EUrnaFase`); ASN.1 numbers it simulado 1, oficial 2, treinamento 3 |
| *pacote* | a data package produced by the TSE systems and imported by the load system (*carga*); `CabecalhoPacote` describes it |
| *carga* / *flash de carga* | the preparation of an urna with the election data, and the medium used for it |
| *seção agregada* | a small section merged into this one: its voters vote on this urna |
| *suplência* | the substitute(s) / vice attached to a cargo (vice-prefeito, 1º/2º suplente de senador) |
| *resultado* / *MR* | the result files written at the end of the day / *Mídia de Resultado*, the removable medium taken to the transmission point |
| **SAVD** | the urna's signing / verification service (files `*.vsu` on the urna, `*.vsc` on the MR); expansion inferred ("Serviço de Assinatura e Verificação Digital") |
| **BU** | *boletim de urna*, the section result (`-bu.dat`); **RDV** *registro digital do voto* (`-rdv.dat`) |
| *jufa* | "justificativas e faltosos": the attendance file `-jufa.dat` (`EntidadeResultadoUrnaCadastro`) |
| *zerésima* | the report printed before voting starts proving every counter is zero (`-imgze.dat` = its image) |

---

## 2. Classes and how they relate (RTTI)

```
comum::asn::IConversorASN<ENTIDADE, DADO>                typeinfo only (no vtable of its own); iconversorasn.h
 │   vtable of every subclass: [0] ~T (ICF 174 "return this")  [1] deleting ~T (144 free)
 │                             [2] DoConverte(const TDado&)    [3] DoDesconverte(const TEntidade&)
 ├─ CConversorCabecalhoEntidade       <CabecalhoEntidade, CCabecalhoEntidade>          11589 / 11588   (comum/asn)
 ├─ CConversorAbrangencia             <Abrangencia, CAbrangencia>                      11462 / 11461   (comum/asn)
 ├─ CConversorCabecalhoPacote         <CabecalhoPacote, CCabecalhoPacote>              11438 / 11437*  (comum/asn, inferred)
 ├─ CConversorDadosDisponiveisCarga   <DadosDisponiveisCarga, CDadosDisponiveisCarga>  11424 / 11423
 ├─ CConversorMunicipio               <Municipio, CMunicipio>                          11379 / 11378*
 ├─ CConversorComplementoMunicipio    <ComplementoMunicipio, CComplementoMunicipio>    11383 / 11382
 ├─ CConversorSecaoEleitoral          <SecaoEleitoral, CSecaoEleitoral>                11454 / 11453*
 ├─ CConversorIdentificacaoAgregada   <IdentificacaoAgregada, CIdentificacaoAgregada>  11456* / 11455
 ├─ CConversorDadoLocal               <DadoLocal, CDadoLocal>                          11403 / 11404*
 ├─ CConversorEntidadePartidos        <EntidadePartidos, CEntidadePartidos>            11458 / 11457
 ├─ CConversorCandidaturas            <EntidadeCandidatos, vector<CCandidatura>>       11444 (base) / 11445
 ├─ CConversorCandidatura             <Candidatura, CCandidatura>  (+cargo, partido, apto)  11446 (base) / 11448*
 ├─ CConversorSuplencia               <Suplencias, CSuplencia>                         11358 (base) / 11359
 ├─ CConversorEntidadeHashes          <EntidadeHashes, CEntidadeHashes>                10266 / 10265 (base)
 ├─ CConversorVersoesArquivos         <EntidadeVersaoArquivos, CVersoesArquivos>       10264 / 10263*
 ├─ CConversorRegistroDigitalVoto<CConversorEleicoesVota>
 │                                    <EntidadeRegistroDigitalVoto, CVotosEleicoesVota> 11483 / 11482  (own ~T 11485/11484)
 ├─ CConversorEntidadeBU, CConversorHistoricoVotoImpresso: only the base DoDesconverte (10272, 10275) is here
 └─ … 14 more classes whose only method in this unit is the base-class DoConverte thunk (list in §4.4)
                                                            (* = method in another unit)
comum::asn::IConversorBiometriaASN<BiometriaEleitorCifrada, CBiometriaEleitor>
 └─ CConversorBiometriaEleitorCifrada  [2] 11418 (base DoConverte, throws)  [3] 11419 (other unit)

ecourna::api::exception::CBaseError<comum::EUeComumAsnError, SErrorLimits{7650, 7750}> : CError
    typeinfo 1528108, vtable 1528128, constructor thunk func 255 (shared body 2294)
ecourna::api::exception::CBaseError<comum::EUeComumError, SErrorLimits{7200, 7600}>   (CArquivosResultado/Savd)
    typeinfo 1551372, constructor thunk func 480
```

Every converter is stateless (`sizeof` 4, vptr only) and is built on the caller's stack, except
`CConversorCandidatura` (cargo, partido, apto: 12 bytes), `CConversorEleicoesVota` (a map of cargos) and
`CConversorRegistroDigitalVoto` (the RDV header fields, 56 bytes), which are configured once by the start-up code.

Non-converter classes touched by the unit: `comum::CArquivosResultado` (stateless), `comum::CArquivosSavd`
(two `std::map`s), `comum::CFotos` (photo index singleton), `api::CDataImage<comum::CCandidaturasDSFoto>`
(`api::IImage` subclass: `[0] 174 [1] 144 [2] 12641 GetImage`).

---

## 3. What the subsystem does in the voting process

The urna reads all its election data from BER-encoded ASN.1 files prepared by the TSE (candidates `-ca.dat`, parties
`-pa.dat`, photos `-fo.dat`, municipality complements `-cm.dat`, the local `-lo.dat`, the election configuration, the
voters…) and writes its own state (`eg.bin`, `vota.bin`, `sa.bin`, `gap.bin`), the RDV and, at the end of the day,
the result files (BU, RDV, jufa, hashes, images, versions). Between the file layer (`api::CFileASN`, which only
BER-decodes/encodes) and the application objects (`comum::md::*`) sits this converter layer:

```
 file bytes ──CFileASN::DecodeObject──▶ TEntidade ──IConversorASN::Desconverte──▶ TDado   (reading data files)
                                          │ 1. isValid() && isStrictlyValid()  (every ASN.1 constraint) else 7654
                                          │ 2. DoDesconverte (vtable slot 3) — may throw md-level errors
 TDado ──IConversorASN::Converte──▶ TEntidade ──CFileASN::CodeObjectFunction──▶ file bytes (writing state/results)
            1. DoConverte (slot 2)
            2. isValid() && isStrictlyValid() else 7653 "Entidade deixada em estado inválido"
```

So nothing the urna writes can violate the schema (a count above the INTEGER bound, a string longer than the SIZE
constraint, an absent mandatory CHOICE) without an exception, and nothing it reads reaches the application unless it
satisfies the schema. The md constructors then add semantic checks (e.g. `md::CAbrangencia`: "UF informada para
abrangência federal").

---

## 4. The template `IConversorASN` (iconversorasn.h)

### 4.1 Contract (reconstruction: `src/uenux2/src/app/comum/asn/iconversorasn.h`)

| method | line | code | message | wasm |
|---|---|---|---|---|
| `TEntidade Converte(const TDado&) const` | 56 | 7653 | `Entidade deixada em estado inválido: {}` | merged body **1563**; exception-safe copy **2893** |
| `TDado Desconverte(const TEntidade&) const` | 71 | 7654 | `Entidade está inválida: {}` | merged body **1008** |
| `TDado Desconverte(const std::vector<uebyte>&) const` (overload, name inferred) | — | — | decodes the bytes, then the line-71 check | **5680** (RDV only) |
| `virtual TEntidade DoConverte(const TDado&) const` (default) | 89 | 7655 | `Método DoConverte() não implementado para {}` | merged body **731** |
| `virtual TDado DoDesconverte(const TEntidade&) const` (default) | 98 | 7656 | `Método DoDesconverte() não implementado para {}` | merged body **731** |

The `{}` of the first two is the text of `ASN1::trace_invalid(stream, typeid(TEntidade).name(), entidade)`: the raw
mangled type name (e.g. `N21ModuloTiposEleitorais17CabecalhoEntidadeE`) followed by the Portuguese InvalidTracer
report (dotted path of the bad field + reason). The `{}` of the last two is the mangled name.

Differences with the ecourna twin (`ecourna::api::asn::IConversorASN`, unit u11): the defaults are not pure
virtual (a read-only file just leaves `DoConverte` alone, a write-only one `DoDesconverte`), the error enum is
`EUeComumAsnError` (7650..7750) instead of `EApiAsnError` (1900..), `std::ostringstream` instead of
`std::stringstream`, and no `": "` suffix is added to the type name.

### 4.2 Error codes of `comum::EUeComumAsnError` seen in this unit and its neighbours

| code | where | message |
|---|---|---|
| 7650 | CConversorAbrangencia::DoConverte (cconversorabrangencia.cpp:45) | `Tipo inválido [{}]` |
| 7651 | CConversorCabecalhoEntidade::DoConverte (:35) | `Tipo inválido [{}]` |
| 7652 | CConversorCabecalhoEntidade::DoDesconverte (:56) | `Tipo de id de cabeçalho inválido` |
| 7653 / 7654 / 7655 / 7656 | IConversorASN (:56 / :71 / :89 / :98) | see §4.1 |
| 7658 / 7659 | IConversorBiometriaASN (:62 / :81) | `Entidade está inválida: {}` / `Método DoConverte() não implementado para {}` |
| 7667 / 7668 | LeEntidadeEm (legravaentidade.h:92 / :98) | `CLeitorASN::{} - não foi possível fazer o seek em {}` / `… ler a entidade de {}` |
| 7669 / 7670 | LeEntidadeBiometria (:119 / :125) | same two messages |
| 7671 | Utils::DesconverteDataHoraJE (util.cpp:41) | `Formato de data inválido.` |
| 7674 / 7675 | Utils::ConverteFase (:102) / DesconverteFase (:116) | `Fase inválida: {}` |
| 7678 | Utils::ConverteTipoPacote (:231) | `Tipo de pacote inválido: {}` |
| 7680 | Utils::ConverteIdPacote (:344) | `Tipo IDEleitoral inválido.` |
| 7681 | Utils::ConverteIdSistema (:419) | `Sistema inválido: {}` |
| 7685 / 7686 | Utils::ConverteAbrangencia (:513) / DesconverteAbrangencia (:498) | `Abrangência inválida: {}` |
| 7687 / 7688 | Utils::ConverteTipoLocalVotacao (:530) / DesconverteTipoLocalVotacao (:549) | `Tipo de local de votação inválido: {}` |

(7673, 7679, 7682, 7684, 7689, 7690 are the `Desconverte*` of util.cpp inlined into other units' functions.)

### 4.3 How the template looks in the wasm

* **merge-similar-functions.** All out-of-line `Converte` instantiations became 20-byte thunks
  `1563(result, this, dado, &srcloc(:56), typeid-name)`, all `Desconverte` thunks call `1008(…, &srcloc(:71), name)`,
  and all default `DoConverte`/`DoDesconverte` are 29-byte thunks
  `731(result, this, arg, &srcloc(:89|:98), 7655|7656, fmt_ptr2, fmt_ptr1, name)`. The format string travels as two
  pointers into the literal (2923 and 2967 for "Método DoConverte() não implementado para {}"; 2875/2922 for the
  DoDesconverte text), so the merged 731 formats it at run time (`__vformat_to` directly, no
  `__try_constant_folding`).
* **Two exception models.** 2893 is the same `Converte` as 1563 compiled *with* `invoke_*` landing pads (on unwind
  it runs `~ostringstream`, `~string`, `__cxa_free_exception` and `ASN1::SEQUENCE::~SEQUENCE` on the half-built
  entity). Its only users are the four EstadoGeral thunks (9997 vota.bin, 10019 sa.bin, 10051 gap.bin, 10170
  eg.bin): the linker kept the copy from a translation unit compiled with exception support. Every other
  instantiation (1563/1008 and all the inlined copies) has no landing pads: an exception leaks the local strings
  and ASN.1 objects (see u11 §4.2 for the same pattern).
* **CHOICE / ENUMERATED entities are not merged.** For `CodigoCargoConsulta` (a CHOICE) the validity test is
  `CHOICE::isValid` (1068) + `CHOICE::isStrictlyValid` (1067) (func 5828); for the ENUMERATED
  `TipoIdentificadorEleitor` it is inlined as `value > info->maxEnumValue` (func 5684). Same for
  `Eleicoes` (CHOICE) in 11482/11483.

### 4.4 Naming traps: functions named after an inlined template method

A `std::source_location` record belongs to the function that contains it after inlining. The tool therefore named
14 converter methods after the `Converte`/`Desconverte` they inline. The vtable slot gives the real name:

| func | tool name (inlined method) | real identity | evidence |
|---|---|---|---|
| 10266 | `IConversorASN<ArquivoAssinatura, CHashArquivo>::Converte` | `CConversorEntidadeHashes::DoConverte` | vtable @1599092 slot 2 |
| 11382 | `IConversorASN<HorarioVerao, CHorarioVerao>::Desconverte` | `CConversorComplementoMunicipio::DoDesconverte` | vtable @1569480 slot 3 |
| 11383 | `IConversorASN<HorarioVerao, CHorarioVerao>::Converte` | `CConversorComplementoMunicipio::DoConverte` | slot 2 |
| 11424 | `IConversorASN<CabecalhoPacote, CCabecalhoPacote>::Converte` | `CConversorDadosDisponiveisCarga::DoConverte` | vtable @1565796 slot 2 |
| 11454 | `IConversorASN<IdentificacaoAgregada, …>::Converte` | `CConversorSecaoEleitoral::DoConverte` | vtable @1561712 slot 2 |
| 11457 | `IConversorASN<Partido, CPartido>::Desconverte` | `CConversorEntidadePartidos::DoDesconverte` | vtable @1561344 slot 3 |
| 11458 | `IConversorASN<Abrangencia, CAbrangencia>::Converte` | `CConversorEntidadePartidos::DoConverte` | slot 2 |
| 11482 | `IConversorASN<Eleicoes, CVotosEleicoesVota>::Desconverte` | `CConversorRegistroDigitalVoto<CConversorEleicoesVota>::DoDesconverte` | vtable @1560304 slot 3 |
| 11483 | `IConversorASN<Eleicoes, CVotosEleicoesVota>::Converte` | `CConversorRegistroDigitalVoto<…>::DoConverte` | slot 2 |
| 5710 | `IConversorASN<Candidatura, CCandidatura>::Desconverte` | helper appending one candidate list (`DesconverteLista`, name inferred) | signature `(vector&, converter, SEQUENCE OF)`, index loop + push_back |
| 5680 | `IConversorASN<EntidadeRegistroDigitalVoto, …>::Desconverte` | the `Desconverte(const std::vector<uebyte>&)` overload | decodes bytes first (5825), callers pass rdv.dat bytes |
| 3755 | `(anonymous)::LeEntidadeEm` | `comum::CFotos::GetImagem` (no `this`: static, or its unused `this` removed) | context string "CFotos::GetImagem(id)", cfotos.cpp:39/51 records |
| 11423 | `CConversorDadosDisponiveisCarga::vf3` (name candidate: `md::CDadosDisponiveisCarga` ctor) | `CConversorDadosDisponiveisCarga::DoDesconverte` with the md constructor inlined | slot 3 |
| 11438 | `CConversorCabecalhoPacote::vf2` (candidates `Utils::Converte*`, `CIDPacote::UF…`) | `CConversorCabecalhoPacote::DoConverte` with Utils/CIDPacote inlined | slot 2 |

**Base-class default** thunks (→ 731) in this unit (for most of these classes it is their only function here;
Suplencia, Candidaturas and Candidatura also have 11359, 11445 and the constructor 5712): `DoConverte` of
`CConversorSuplencia` (11358), `CConversorSituacoesEleicoes` (11360), `CConversorProcessoEleitoral` (11362),
`CConversorPleito` (11364), `CConversorNomesCargo` (11368), `CConversorEleicaoPE` (11370), `CConversorCargo` (11372),
`CConversorDetalheCandidato` (11374), `CConversorDetalheConsulta` (11376), `CConversorComplementosMunicipios`
(11380), `CConversorImpedido` (11409), `CConversorDedo` (11415), `CConversorBiometriaEleitor` (11421),
`CConversorFotoCandidato` (11439), `CConversorCandidaturas` (11444), `CConversorCandidatura` (11446),
`CConversorDadosCandidato` (11449) — all read-only data; and `DoDesconverte` of `CConversorEntidadeHashes`
(10265), **`CConversorEntidadeBU` (10272)** and **`CConversorHistoricoVotoImpresso` (10275)** — write-only results.

---

## 5. `comum::asn::Utils` (util.cpp)

All static. Mappings (tables read from the data segment):

| function | direction | mapping / check | wasm |
|---|---|---|---|
| `DesconverteDataHoraJE` | `"YYYYMMDDTHHMMSS"` → `api::CDateTime` | `substr(0,8)` → `DataJE` → `DesconverteDataJE` (2276); `substr(8,1) != "T"` → 7671; `substr(9,6)` → `HoraJE` → `api::CTime` (3643, "Hora inválida") | 1713 |
| `ConverteDataHoraJE` | `CDateTime` → text | `CDate::Format("YYYYMMDD") + "T" + CTime::Format("hhmmss")` | 1080 (other unit) |
| `ConverteFase` | `EUrnaFase` → `Fase` | `'1'`→oficial(2), `'2'`→simulado(1), `'3'`→treinamento(3) (table @497848); else 7674 | 1712 |
| `DesconverteFase` | `Fase` → `EUrnaFase` | 1→`'2'`, 2→`'1'`, 3→`'3'` (table @497860); else 7675 | 2843 |
| `ConverteTipoPacote` | `md::ETipoPacote` 0..32 → `TipoPacote` | table @497872 = {1,2,5,7,8,9,10,13,14,15,16,17,18,21,…,37,39,40,48}; ≥33 → 7678. The ASN.1 enum has 56 values up to 65: 23 of them (41..47, 49, 50, 52..65) are unreachable from the md enum | inlined in 11438 |
| `ConverteIdPacote` | `md::CIDPacote` → `IDPacote` | IDEleitoral alternative = tipo−1 (1..3 else 7680), fase, then each optional (UF / município / zona) included only if present | inlined in 11438 |
| `ConverteIdSistema` | `ESistemaJE` 1..12 → `Sistema` | {1,2,3,**13**,**12**,6,7,8,9,10,11,**14**} (table @498196): 4→simon, 5→seweb, 12→padaUE; else 7681 | inlined in 11438 |
| `DesconverteAbrangencia` / `ConverteAbrangencia` | identity municipal 0 / estadual 1 / federal 2 | ≥3 → 7686 / 7685 | 5827 / inlined in 11462 |
| `ConverteTipoLocalVotacao` / `DesconverteTipoLocalVotacao` | identity normal 1 / emTrânsito 2 / presoProvisório 3 / temporário 4 | outside 1..4 → 7687 / 7688 | 3801 / 3800 |

Formatting detail: `EUrnaFase` (slot 2285), `md::ETipoPacote` / `ESistemaJE` (2286/2287, in 11438) and
`ETipoLocalVotacao` (2290) are formatted through `std::formatter` specialisations (format-arg type 15 "handle",
body 536), and so is the ASN.1 `TipoAbrangencia::NamedNumber` of `DesconverteAbrangencia` (slot 2289, body 1711).
The values read from an ASN.1 object (`Fase`, `TipoLocalVotacao` via `asInt()`) and `md::ETipoAbrangencia` are
formatted as plain `int` (type 3).

---

## 6. The converters of this unit (what they read and write)

Field numbers follow the SEQUENCE order of `src/asn1/*.asn`.

* **`CConversorCabecalhoEntidade`** (11589 / 11588). Write: `dataGeracao = ConverteDataHoraJE(dado.dataGeracao)`;
  `tipo` (+16) ≥ 3 → 7651; `idEleitoral.select(tipo, INTEGER(0..99999)) = id` (+12). Read: `choiceID` (unsigned) ≥ 3
  → 7652 (also catches an unselected CHOICE, −1), then `CCabecalhoEntidade(DesconverteDataHoraJE(…), valor, tipo)`
  (1945). Observed: every data-file header read at `votaInit`.
* **`CConversorAbrangencia`** (11462 / 11461). Write: tipo via `ConverteAbrangencia`; municipal → `id{siglaUF,
  codigoMunicipio}`, estadual → `id{siglaUF}` (municipality omitted), federal → no `id`; default → 7650 (dead in
  practice: the inlined `ConverteAbrangencia` has already thrown 7685 for tipo ≥ 3). Read:
  absent `id` → `CAbrangencia(tipo, "", 0)`; absent municipality → `(tipo, UF, 0)`; the UF is copied through its C
  string. `md::CAbrangencia` (3739) then enforces the combination.
* **`CConversorCabecalhoPacote::DoConverte`** (11438): tipoPacote, idPacote, nome, versão, origem; the optional
  `abrangencia` of `CabecalhoPacote` is **never written** (md::CCabecalhoPacote has no such member).
* **`CConversorDadosDisponiveisCarga`** (11424 / 11423), the description of what the load medium offers, read only by
  the "versão dos pacotes" report (`vota::CImpressaoVersaoPacotes`, 11905). Read builds, per município,
  `CInfoMunicipio(código, nome, fuso, comBiometria [, horário de verão])` + its sections, and the imported package
  headers, then the inlined `md::CDadosDisponiveisCarga` constructor checks (8040..8048): processo ≠ 0, pleito ≠ 0,
  **serial of the load medium exactly 8 characters**, país / sigla UF / nome UF not blank, zona ≠ 0, at least one
  município. `comBiometria` = `Municipio.comBiometria` if present, else false.
* **`CConversorMunicipio::DoConverte`** (11379): código, nome, **`capital` always FALSE**, `codigoIBGE` absent,
  `comBiometria` included.
* **`CConversorComplementoMunicipio`** (11383 / 11382): código, fuso (−720..720 minutes, checked by
  `CComplementoMunicipio::ValidaCriacao`), optional daylight-saving period through `CConversorHorarioVerao`.
  **`desligaColetaBiometria` is neither read nor written** (the md object has no field for it; constructors 3710 /
  3711 store nothing at +6).
* **`CConversorSecaoEleitoral::DoConverte`** (11454): tipo, `IdentificacaoSecaoEleitoral{município, zona, local,
  seção}`, and the aggregated sections (omitted when the vector is empty, else each converted by
  `CConversorIdentificacaoAgregada`). **`CConversorIdentificacaoAgregada::DoDesconverte`** (11455): número + tipo
  de local de origem → `CIdentificacaoAgregada` (ctor 5672: +0 tipo, +4 número).
* **`CConversorDadoLocal::DoConverte`** (11403), part of `eg.bin`: tipo de local, UF, and the section through
  `CConversorLocalidadeEleitoral`. Observed (the web mock writes eg.bin during `votaInit`).
* **`CConversorEntidadePartidos`** (11458 / 11457), `-pa.dat`: cabeçalho, abrangência, list of
  `CPartido{número, sigla, nome}` (28 bytes). Observed (read at `votaInit`).
* **`CConversorCandidaturas::DoDesconverte`** (11445), `-ca.dat`: for each `CandidatosPorCargos`: cargo code (CHOICE
  `CodigoCargoConsulta`, 5828); if `partidos` is present, for each party two passes with
  `CConversorCandidatura(cargo, partido, apto = true)` over `candidatosAptos` and `(…, false)` over
  `candidatosInaptos` (5712 + 5710). `cabecalho`, `abrangencia` and `quantidadeVagas` are not used here.
* **`CConversorSuplencia::DoDesconverte`** (11359): `CSuplencia{ordem (field 1), temFoto (field 3), nomes}`; the cargo
  code (field 0) is dropped. (This corrects u03's guess "+0 codigo, ordem ?": +0 is `ordem`, +1 `temFoto`.)
* **`CConversorEntidadeHashes::DoConverte`** (10266; `CConversorHashArquivo` itself is reconstructed by u17 in
  `comum/asn/cconversorhasharquivo.u17.cpp`), **`CConversorVersoesArquivos::DoConverte`** (10264),
  **`CConversorRegistroDigitalVoto`** (11483 / 11482): result files, see §9.

---

## 7. Reading one element of a big file: `legravaentidade.h`, candidate photos

`LeEntidadeEm<CONVERSOR>(arquivo, CIndexer{offset, tamanho}, contexto)` (legravaentidade.h:92/98) opens the file
(`CFile(arquivo, "rb")`), seeks to `offset` (failure → 7667), reads exactly `tamanho` bytes (short read → 7668),
decodes them with `api::CFileASN::DecodeObjectFunction<TEntidade>(buffer, "DecodeObject de <typeid>")`
(cfileasn.h:135 "Conteúdo não foi decodificado para {}: {}" 5953, :143 "Conteúdo inválido para {}: {}" 5954) and
returns `CONVERSOR().Desconverte(entidade)`. `LeEntidadeBiometria` (:119/:125, codes 7669/7670) is the same with a
`CParametro` for the fingerprint decryption; it is inlined into `CEleitorDetalhe::GetBiometria` (1937, other unit).

The only `LeEntidadeEm` instance is inlined into **`CFotos::GetImagem(id)`** (3755):

```
CFotos::GetInst()                     singleton @1838844 (created on first use: operator new(28) + ctor 3756)
  m_fotos.find(id)                    map<codigo, SIndiceFoto{id, CIndexer{offset, tamanho}, arquivo}>
    not found → CUeComumDadosError 7861 "{} - id de foto [{}] não encontrado"          (cfotos.cpp:51)
  api::ExisteArquivo(arquivo)         stat() + S_ISREG (func 412 → 6019)
    missing  → 7860 "{} - arquivo [{}] da foto [{}] não encontrado"                   (cfotos.cpp:39)
  LeEntidadeEm<CConversorFotoCandidato>(arquivo, indexer, "LeFotoCandidato(" + id + ")")
  return foto.GetFoto().GetImagem()   copy of the JPEG bytes
```

It has two callers. The voting-screen image source **`CDataImage<CCandidaturasDSFoto>::GetImage`** (12641):
current candidacy (`GetCandidaturaAtual("CCandidaturasDSFoto")`), titular (`m_indice == 0`) or suplente
`m_indice`, lookup of the candidate code in `CFotos`; if found, it calls `GetImagem` once to check the JPEG markers
(`at(0)==FF, at(1)==D8`, last two bytes `FF D9`) and **calls it a second time** to return the image. Every photo
drawn is therefore opened, read and BER-decoded twice (plus two validations). Missing photo or non-JPEG → empty
vector (nothing drawn). Confirmed at run time: with `FS.open` hooked in a scratch copy of `tools/run/headless.mjs`, typing Vereador
`91001` opened `t02411ac00001-fo.dat` twice after the 5th digit and twice again when the screen settled on
`CConfirmaVotoNominal` (one pair per draw). The second caller, the image of
`vota::CTelasVota::CriaTelaVisualizacaoCandidato` (`CDataImage<std::__bind<…>>::GetImage`, 12520, a 16-byte body),
calls `GetImagem` once, without the JPEG check. Both callers call `CFotos::GetInst()` (2819) just before and drop
the result.

---

## 8. File-name tables: `CArquivosResultado`, `CArquivosSavd`

**`CArquivosResultado::operator[](EExtensaoArquivoResultado)`** (347, carquivosresultado.cpp:99) is a pure switch
on a stateless singleton (`GetInst` = func 348 → 2900, other unit; the unused `this` was removed by wasm-opt
dead-argument elimination). Unknown value → `CUeComumError` 7200
"Extensão de arquivo de resultado associada ao identificador {} não encontrada".

| id | suffix | | id | suffix | | id | suffix |
|---|---|---|---|---|---|---|---|
| 1 | `vota.vsc` | | 8 | `jufa.dat` | | 15 | `logsa.jez` |
| 2 | `sa.vsc` | | 9 | `imgbu.dat` | | 16 | `wsqbio.jez` |
| 3 | `red.vsc` | | 10 | `imgbusa.dat` | | 17 | `wsqman.jez` |
| 4 | `bu.dat` | | 11 | `imgze.dat` | | 18 | `wsqmes.jez` |
| 5 | `busa.dat` | | 12 | `hash.dat` | | 19 | `mr.ver` |
| 6 | `rdv.dat` | | 13 | `log.jez` | | 20 | `asw.vsc` |
| 7 | `rdvred.dat` | | 14 | `log.jez` (same as 13) | | 21 | `ahw.vsc` |

The full name is `<fase><pleito:05><UF><município:05><zona:04><seção:04>-<suffix>` (built by
`CGravadorUtil::DeterminaNomeArquivoSemLetra` / `CCopiaResultadoParaMR`, u08/u09).

**`CArquivosSavd`** (36 bytes) holds `std::map<ESavdPacote, std::string>` (+0, ids 120..203),
`std::map<ESavdArquivoUE, std::string>` (+12, ids 25..115) and `std::map<ESavdAplicacao, std::string>` (+24, never
read), filled by the singleton constructor inlined into `GetInst` (func 1164, 77 KB; full table reconstructed by u22
in `src/uenux2/src/app/comum/carquivossavd.u22.cpp`: `CPath::GetPathTrab/GetPathDinamico/GetPathResult` + names like
`uenux.vsu`, `rdv.vsu`, `bu.vsu`, `eg.vsu`, and the result prefix `"{:c}{:05}{}{:05}{:04}{:04}-"` + a
`CArquivosResultado` suffix). `operator[](ESavdPacote)` (275, :49) and
`operator[](ESavdArquivoUE)` (680, :73) return a copy of the path or throw 7201 "Arquivo de assinatura associado
ao identificador {} não encontrado" / 7203 "Arquivo assinado associado ao identificador {} não encontrado". The
package paths contain `{}` placeholders that the callers fill with `std::vformat` (fase, pleito, UF, município,
zona, seção — see u09 `CAssinador`).

---

## 9. BU and result files: what this unit contributes

This unit does not build the BU contents (that is `CGeraBU` / `CGravadorBU` / `CConversorEntidadeBU`, see
`docs/10-boletim-de-urna.md`), but every result file of the *encerramento* (end of the voting day) passes through
code of this unit. Step by step, for the files written by `CGravacaoResultados` (u09 §5):

1. **File names.** Each writer (`IResultado`) has an `EExtensaoArquivoResultado`; `CArquivosResultado` gives the
   suffix (table §8): `bu.dat` (4), `rdv.dat` (6), `jufa.dat` (8), `imgbu.dat` (9), `imgze.dat` (11),
   `hash.dat` (12), `log.jez` (13), `wsqbio/wsqman/wsqmes.jez` (16/17/18), `mr.ver` (19), and the signature package
   `vota.vsc` (1). SA (*sistema de apuração*, contingency counting) variants exist (`busa.dat`, `imgbusa.dat`,
   `sa.vsc`, `logsa.jez`) but are not written by VOTA. `CArquivosSavd` gives the path of each file's SAVD id and of
   the `vota.vsc` signature package (pacotes 158 / 159 = turno 1 / 2 in the internal flash `res1`/`res2`,
   160 / 161 = the same on the external flash; see `carquivossavd.u22.cpp`).
2. **Header of every result entity.** `CConversorCabecalhoEntidade::DoConverte` writes `CabecalhoEntidade {
   dataGeracao = "YYYYMMDDTHHMMSS" (CDate::Format("YYYYMMDD") + "T" + CTime::Format("hhmmss")), idEleitoral =
   CHOICE alternative ETipoCabecalho (0 idProcessoEleitoral, 1 idPleito, 2 idEleicao) = INTEGER (0..99999) }`.
3. **Phase.** `Utils::ConverteFase` (1712) maps the installation phase: oficial `'1'` → `Fase 2`, simulado `'2'` →
   `1`, treinamento `'3'` → `3` (used by the BU, RDV, hash, envelope and carga converters).
4. **Validation before encoding.** Every result entity goes through `IConversorASN::Converte` (inlined at
   iconversorasn.h:56 into `CGravadorBU` 11629 for the BU, into `CFileASN::CodeObjectFunction` 5367 for hash.dat,
   `CGravadorVersoesArquivos` 11582 for mr.ver, `CRdvVota::Converte` 11488 for the RDV): after `DoConverte`, `isValid()` and `isStrictlyValid()` must hold,
   otherwise `CUeComumAsnError 7653 "Entidade deixada em estado inválido: <type>: <tracer text>"` aborts the writer.
   E.g. a vote count above an INTEGER bound of `ModuloBoletimUrna` cannot be written.
5. **BU is write-only for VOTA.** `CConversorEntidadeBU` and `CConversorHistoricoVotoImpresso` keep the base
   `DoDesconverte` (10272, 10275): trying to read a BU back with them throws 7656 "Método DoDesconverte() não
   implementado para N17ModuloBoletimUrna19EntidadeBoletimUrnaE". The integrity check of the BU copy on the MR is a
   byte comparison (u08), not a decode.
6. **hash.dat** (`CConversorEntidadeHashes::DoConverte`, 10266, used by `CGravadorHashes` through
   `CFileASN::CodeObjectFunction` 5367): `EntidadeHashes { cabecalho, fase = ConverteFase, siglaUF, identificacaoUrna =
   CHOICE [0] IdentificacaoSecaoEleitoral{municipioZona{município, zona}, local, seção} if the
   `optional<md::CIdentificacaoSecao>` (+36, flag +56) is engaged, else [1] IdentificacaoContingencia{municipioZona}
   if the `optional<md::CIdentificacaoUrnaContingencia>` (+60, flag +72) is, else omitted, versaoUENUX,
   hashesArquivos = one ArquivoAssinatura{nomeArquivo, assinatura} per api::hash::CHashArquivo }`, each element
   validated (7653) by `CConversorHashArquivo::Converte`. Not read back (base `DoDesconverte` 10265).
7. **rdv.dat** (`CConversorRegistroDigitalVoto<CConversorEleicoesVota>`): `DoConverte` (11483, observed: the empty
   RDV is written at `votaInit`) builds `EntidadeRegistroDigitalVoto { pleito, fase, identificacao{municipioZona,
   local, seção}, historicoCodigosCarga (vector<string> of the converter), eleicoes = CConversorEleicoesVota::Converte
   (CHOICE validated with CHOICE::isValid/isStrictlyValid) }`. The header values are members of the converter
   (+20..+44), set once at start-up. When the RDV is read back (`CRdvVota::Desconverte` / `ConfereConteudo`, via
   the byte-vector overload 5680 → `CFileASN::DecodeObject` 5825 → validation → 11482), **only `eleicoes` is
   converted**: pleito, fase, identification and load history of the file are not compared with the urna's own.
8. **mr.ver** (`CConversorVersoesArquivos::DoConverte`, 10264, used by `CGravadorVersoesArquivos` 11582):
   `EntidadeVersaoArquivos { versaoTag, arquivos = ArquivoAssinatura{nome, versão} per std::map entry (sorted by
   name) }`.
9. **Signing.** Not in this unit (u09 `CAssinador`); in this build the signature files contain the literal
   `assinatura simulada para vota_web_wasm` (analysis/runtime/README.md).

---

## 10. Web-build specifics and run-time observations

* The 36 functions seen running are all on the `votaInit` data-load path (the merged bodies 1563/1008, the ASN.1
  helpers 515/1067, headers 11588/3738/1713, abrangência 11461/5721, parties 11457, candidates 11445/5710/5711,
  suplências 11359/5688, municipality complements 11382/3737/3711 (reached from the urna's Local file:
  `CConversorLocal::DoDesconverte` 11452 → 3737 → 1008 → 11382), section 5715 and município 5716, pleito 5686,
  cabeçalho de pacote 5705, correspondência 5691/5692, dado secao 5696/5697, dado local 11403,
  NumViasImpressasRelatorios 3726, the four EstadoGeral writes 9997/10019/10051/10170 → 2893, the initial RDV 11483
  called from `CRdvVota` 11488), plus the photo path 12641 → 3755 during the candidate screens. `Utils::
  DesconverteAbrangencia` (5827) and the `CodigoCargoConsulta` check (5828) are direct callees of the observed 11461
  and 11445, so they certainly ran, but the sampler did not catch them (they are not among the 36).
* The state files `eg.bin`, `vota.bin`, `sa.bin`, `gap.bin` are produced in the web build by the mock
  `uenux2/mock/app/comum/cappinfobuilder.cpp` (func 2894) through the EstadoGeral converters and the exception-safe
  `Converte` body 2893.
* Nothing in this unit is web-specific: no mock, no JS import, no `emscripten_sleep`. The SAVD signatures behind
  `CArquivosSavd` are simulated elsewhere (fixed text in every `.vsu`).
* `CFotos`' "mutex" (`@1838820`) is the no-op `std::mutex::unlock` residue (func 150) of the single-threaded build.

---

## 11. Notable wasm / Emscripten observations

* **Dead-argument elimination.** `CArquivosResultado::operator[]` is `(sret, id)`: its unused `this` was removed
  (the object is a stateless singleton, func 348).
  `CFotos::GetImagem` has no `this` either (static, or the same elimination; see §15).
* **Cross-TU inlining.** `CIDPacote::UF/Municipio/Zona` (cidpacote.cpp), `md::CDadosDisponiveisCarga`'s
  constructor, `CFotos` code and the `cfileasn.h` templates are inlined into converters of other files: the build
  uses LTO, which is why so many srcloc records sit in functions of another source file.
* **Tail-padding memcpy.** Copies of `std::vector<md::CIdentificacaoAgregada>` (`{int32, uint16}`, 8 bytes) are
  `memcpy(dst, src, bytes - 2)` (funcs 2807, 11454): LLVM does not copy the last element's padding. Not a bug.
* **String literals built in place.** `CArquivosResultado` builds every suffix in the SSO buffer of the result:
  the four 8-character ones (`'vota.vsc'`, `'busa.dat'`, `'jufa.dat'`, `'hash.dat'`) as immediate 64-bit constants,
  the others (`bu.dat`, `rdv.dat`, …) by unaligned loads from the string data; only `"imgbusa.dat"` (11 chars)
  needs a heap allocation (`operator new(16)`).
* **Temporaries of constrained INTEGERs.** Several `DoConverte`s build a constrained-INTEGER temporary just to
  assign its value: `Constrained_INTEGER<2, 1, 99999>` (vtable @1567692) for the município codes in 11379 and
  11383, `Constrained_INTEGER<2, 0, 99999>` (vtable @1560492) for idPE / idPleito in 11424 and pleito in 11483:
  source-level `entidade.set_x(INTEGER(v))` idiom.

---

## 12. Weird / risky code

| # | func | finding | impact |
|---|---|---|---|
| 1 | 12641 → 3755 | Each candidate photo is opened, read, BER-decoded and validated **twice** per draw (JPEG check, then the real read). Confirmed at run time (§7: one pair of `-fo.dat` opens per draw). | Simulator and real urna if the same code: extra I/O and CPU on every confirmation screen. |
| 2 | 12641 | JPEG check uses `vector::at(0)` / `at(1)`: an empty image, or a 1-byte image whose byte is `FF` (the `&&` stops at a first byte ≠ `FF`), throws `std::out_of_range` (not a `CBaseError`) from a GUI data source. `Foto.imagem` is an unconstrained OCTET STRING and `CConversorFoto` only checks the format code, so an empty image does pass the ASN.1 validation. | Only with malformed `-fo.dat` data (signed TSE data on the urna). |
| 3 | 11382/11383, 3710/3711 | `ComplementoMunicipio.desligaColetaBiometria` is silently dropped in both directions; the md object has no field for it. | If the TSE sets it to switch biometric capture off for a municipality, this converter does not propagate it (the switch that is used comes from `Municipio.comBiometria`). |
| 4 | 11482 (with 5680) | Reading `rdv.dat` back ignores the file's pleito, fase, section identification and load history; only the `eleicoes` structure is converted and compared (EstruturaCompativel). | An RDV of another section/pleito with the same office structure would be accepted by `CRdvVota::Desconverte`; mitigated by encryption and SAVD signatures. |
| 5 | 347 | Ids 13 and 14 both map to `"log.jez"`. | Two result kinds would get the same file name; harmless if 14 is never used for a separate file. |
| 6 | 11379 | `Municipio.capital` is always written FALSE and `codigoIBGE` omitted. | Files written back by the urna (`-lo.dat`, carga data) lose these fields. |
| 7 | 11438, util.cpp:231 | `CabecalhoPacote.abrangencia` never written; `md::ETipoPacote` (33 values) cannot express 23 ASN.1 package types (41..47, 49, 50, 52..65). | Informational (package headers are produced by TSE systems, not by VOTA). |
| 8 | 1563/1008 vs 2893 | Only the EstadoGeral instantiations have landing pads; everywhere else an ASN.1 validation exception leaks the half-built entity and strings. | Memory leak on error paths only. |
| 9 | 10272, 10275, 10265 | BU, HistoricoVotoImpresso and hashes have no `DoDesconverte`: a read attempt throws 7656 at run time (not a compile-time error). | By design (write-only); any future reader must not use these converters. |
| 10 | 11423 | `md::CDadosDisponiveisCarga` rejects a load-medium serial whose length is not exactly 8. | Report "versão dos pacotes" (11905, the only user) fails with 8043 if `dadoscarga.dat` carries another serial format. None of the simulator scenarios ships a `dadoscarga.dat` (`upstream/fs/`), so the check is never reached there. |

---

## 13. Reconstructed sources

Attested files of the unit:

* `src/uenux2/src/app/comum/asn/iconversorasn.h`, `iconversorbiometriaasn.h`, `legravaentidade.h`
* `src/uenux2/src/app/comum/asn/util.h`, `util.cpp` (+ `util.u13.cpp` from u13 for `DesconverteDataJE`)
* `src/uenux2/src/app/comum/asn/cconversorabrangencia.h/.cpp`, `cconversorcabecalhoentidade.h/.cpp`
* `src/uenux2/src/app/comum/carquivosresultado.h/.cpp`, `carquivossavd.h/.cpp`

Converter classes without an attested file (path inferred from TSE conventions: md class directory mirrored under
`asn/`, sibling converters attested in the same directory, and include paths already used by u03/u05):

* `src/uenux2/src/app/comum/asn/cconversorcabecalhopacote.h/.cpp` (md in `comum/md/`, like Abrangencia/CabecalhoEntidade)
* `src/uenux2/src/app/comum/dados/asn/correspondencia/cconversordadosdisponiveiscarga.h/.cpp`
* `src/uenux2/src/app/comum/dados/asn/municipiozona/cconversormunicipio.h/.cpp`, `cconversorcomplementomunicipio.h/.cpp`
* `src/uenux2/src/app/comum/dados/asn/cconversorsecaoeleitoral.h/.cpp`, `cconversoridentificacaoagregada.h/.cpp`
* `src/uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversordadolocal.h/.cpp`
* `src/uenux2/src/app/comum/dados/asn/cconversorentidadepartidos.h/.cpp`
* `src/uenux2/src/app/comum/dados/asn/candidatura/cconversorcandidaturas.h/.cpp`, `cconversorcandidatura.h`
* `src/uenux2/src/app/comum/dados/asn/processoeleitoral/cconversorsuplencia.h/.cpp`
* `src/uenux2/src/app/comum/gravadores/asn/cconversorentidadehashes.h/.cpp`, `cconversorversoesarquivos.h/.cpp`,
  `cconversorregistrodigitalvoto.h`

Fragments for files owned by other units: `src/uenux2/src/app/comum/dados/cfotos.u21.cpp` (3755, 3756, 2818, 11513),
`src/uenux2/src/app/comum/dados/ccandidaturas.u21.cpp` (12641),
`src/uenux2/src/app/comum/dados/md/municipiozona/ccomplementomunicipio.u21.cpp` (3710, 3711).

---

## 14. Complete mapping table (all 103 functions)

"ran" = seen in the V8 samples of the recorded votes. Library helpers have no TSE source.

| # | func | size | ran | reconstructed symbol | original file | src file / status | conf. |
|---|---|---|---|---|---|---|---|
| 1 | 255 | 21 |  | `ecourna::api::exception::CBaseError<comum::EUeComumAsnError, ecourna::api::exception::SErrorLimits{7650, 7750}>::CBaseError` | ecourna-lib/ecourna/api/exception/cbaseerror.hpp | library/inlined helper (no TSE source) | high |
| 2 | 275 | 612 |  | `comum::CArquivosSavd::operator[](ESavdPacote) const` | uenux2/src/app/comum/carquivossavd.cpp | src/uenux2/src/app/comum/carquivossavd.cpp | high |
| 3 | 347 | 1322 |  | `comum::CArquivosResultado::operator[](EExtensaoArquivoResultado) const` | uenux2/src/app/comum/carquivosresultado.cpp | src/uenux2/src/app/comum/carquivosresultado.cpp | high |
| 4 | 515 | 233 | yes | `ASN1::SEQUENCE::includeOptionalField(int optional, int field)` | (III ASN.1 runtime, TSE build) | library/inlined helper (no TSE source) | medium |
| 5 | 680 | 612 |  | `comum::CArquivosSavd::operator[](ESavdArquivoUE) const` | uenux2/src/app/comum/carquivossavd.cpp | src/uenux2/src/app/comum/carquivossavd.cpp | high |
| 6 | 731 | 410 |  | `comum::asn::IConversorASN<E, D>::DoConverte / DoDesconverte (merged default body)` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 7 | 751 | 14 |  | `ASN1::ENUMERATED::operator=(const ENUMERATED&) (value copy)` | (III ASN.1 runtime) | library/inlined helper (no TSE source) | medium |
| 8 | 1008 | 572 | yes | `comum::asn::IConversorASN<E, D>::Desconverte (merged body)` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 9 | 1067 | 44 | yes | `ASN1::CHOICE::isStrictlyValid() const` | (III ASN.1 runtime) | library/inlined helper (no TSE source) | high |
| 10 | 1076 | 496 |  | `std::vector<ASN1::AbstractData*>::insert(const_iterator, const value_type&)` | libcxx/include/vector | library/inlined helper (no TSE source) | high |
| 11 | 1563 | 572 | yes | `comum::asn::IConversorASN<E, D>::Converte (merged body)` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 12 | 1712 | 524 |  | `comum::asn::Utils::ConverteFase` | uenux2/src/app/comum/asn/util.cpp | src/uenux2/src/app/comum/asn/util.cpp | high |
| 13 | 1713 | 556 | yes | `comum::asn::Utils::DesconverteDataHoraJE` | uenux2/src/app/comum/asn/util.cpp | src/uenux2/src/app/comum/asn/util.cpp | high |
| 14 | 2275 | 20 |  | `comum::asn::IConversorASN<ModuloTiposEleitorais::CabecalhoEntidade, comum::md::CCabecalhoEntidade>::Converte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 15 | 2807 | 237 |  | `std::vector<comum::md::CSecaoEleitoral>::__init_with_size(first, last, n) (vector copy)` | libcxx/include/vector | library/inlined helper (no TSE source) | medium |
| 16 | 2818 | 37 |  | `comum::CFotos::~CFotos` | uenux2/src/app/comum/dados/cfotos.cpp | src/uenux2/src/app/comum/dados/cfotos.u21.cpp | medium |
| 17 | 2843 | 494 |  | `comum::asn::Utils::DesconverteFase` | uenux2/src/app/comum/asn/util.cpp | src/uenux2/src/app/comum/asn/util.cpp | high |
| 18 | 2893 | 1021 | yes | `comum::asn::IConversorASN<E, D>::Converte (merged body, exception-safe copy)` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 19 | 3710 | 60 |  | `comum::md::CComplementoMunicipio::CComplementoMunicipio(TMunicipioID, int16, const CHorarioVerao&)` | uenux2/src/app/comum/dados/md/municipiozona/ccomplementomunicipio.cpp | src/uenux2/src/app/comum/dados/md/municipiozona/ccomplementomunicipio.u21.cpp | medium |
| 20 | 3711 | 37 | yes | `comum::md::CComplementoMunicipio::CComplementoMunicipio(TMunicipioID, int16)` | uenux2/src/app/comum/dados/md/municipiozona/ccomplementomunicipio.cpp | src/uenux2/src/app/comum/dados/md/municipiozona/ccomplementomunicipio.u21.cpp | medium |
| 21 | 3726 | 20 | yes | `comum::asn::IConversorASN<ModuloEstadoGeralDefs::NumViasImpressasRelatorios, comum::md::estadoaplicacao::CNumViasImpressasRelatorios>::Converte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 22 | 3727 | 20 |  | `comum::asn::IConversorASN<ModuloEstadoGeralDefs::NumViasImpressasRelatorios, comum::md::estadoaplicacao::CNumViasImpressasRelatorios>::Desconverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 23 | 3730 | 20 |  | `comum::asn::IConversorASN<ModuloTiposEleitorais::Seguranca, comum::md::CSeguranca>::Desconverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 24 | 3736 | 20 |  | `comum::asn::IConversorASN<ModuloComplementosMunicipios::ComplementoMunicipio, comum::md::CComplementoMunicipio>::Converte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 25 | 3737 | 20 | yes | `comum::asn::IConversorASN<ModuloComplementosMunicipios::ComplementoMunicipio, comum::md::CComplementoMunicipio>::Desconverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 26 | 3738 | 20 | yes | `comum::asn::IConversorASN<ModuloTiposEleitorais::CabecalhoEntidade, comum::md::CCabecalhoEntidade>::Desconverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 27 | 3755 | 4858 | yes | `comum::CFotos::GetImagem` | uenux2/src/app/comum/dados/cfotos.cpp | src/uenux2/src/app/comum/dados/cfotos.u21.cpp | medium |
| 28 | 3756 | 53 |  | `comum::CFotos::CFotos` | uenux2/src/app/comum/dados/cfotos.cpp | src/uenux2/src/app/comum/dados/cfotos.u21.cpp | medium |
| 29 | 3799 | 20 |  | `comum::asn::IConversorASN<ModuloTiposResultadosEcoUrna::Urna, comum::md::CUrna>::Converte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 30 | 3800 | 483 |  | `comum::asn::Utils::DesconverteTipoLocalVotacao` | uenux2/src/app/comum/asn/util.cpp | src/uenux2/src/app/comum/asn/util.cpp | high |
| 31 | 3801 | 518 |  | `comum::asn::Utils::ConverteTipoLocalVotacao` | uenux2/src/app/comum/asn/util.cpp | src/uenux2/src/app/comum/asn/util.cpp | high |
| 32 | 5672 | 18 |  | `comum::md::CIdentificacaoAgregada::CIdentificacaoAgregada(TSecaoID numero, ETipoLocalVotacao tipo)` | uenux2/src/app/comum/dados/md/csecaoeleitoral.h | src/uenux2/src/app/comum/dados/asn/cconversoridentificacaoagregada.cpp (comment) | low |
| 33 | 5680 | 739 |  | `comum::asn::IConversorASN<ModuloRegistroDigitalVoto::EntidadeRegistroDigitalVoto, comum::md::CVotosEleicoesVota>::Desconverte(const std::vector<uebyte>&)` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | medium |
| 34 | 5684 | 574 |  | `comum::asn::IConversorASN<ModuloTiposEleitorais::TipoIdentificadorEleitor, ecourna::app::dados::ETipoIdentificadorEleitor>::Desconverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 35 | 5686 | 20 | yes | `comum::asn::IConversorASN<ModuloProcessoEleitoral::Pleito, comum::md::CPleitoDTO>::Desconverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 36 | 5688 | 20 | yes | `comum::asn::IConversorASN<ModuloEleicao::NomesCargo, comum::md::CNomesCargo>::Desconverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 37 | 5691 | 20 | yes | `comum::asn::IConversorASN<ModuloEstadoGeralUrna::DadoCorrespondencia, comum::md::estadoaplicacao::CDadoCorrespondencia>::Converte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 38 | 5692 | 20 | yes | `comum::asn::IConversorASN<ModuloEstadoGeralUrna::DadoCorrespondencia, comum::md::estadoaplicacao::CDadoCorrespondencia>::Desconverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 39 | 5696 | 20 | yes | `comum::asn::IConversorASN<ModuloEstadoGeralUrna::DadoSecao, comum::md::estadoaplicacao::CLocalidadeEleitoral>::Converte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 40 | 5697 | 20 | yes | `comum::asn::IConversorASN<ModuloEstadoGeralUrna::DadoSecao, comum::md::estadoaplicacao::CLocalidadeEleitoral>::Desconverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 41 | 5705 | 20 | yes | `comum::asn::IConversorASN<ModuloTiposEleitorais::CabecalhoPacote, comum::md::CCabecalhoPacote>::Desconverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 42 | 5710 | 1431 | yes | `comum::asn::(anonymous namespace)::DesconverteLista(std::vector<md::CCandidatura>&, const CConversorCandidatura&, const SEQUENCE_OF<Candidatura>&)` | uenux2/src/app/comum/dados/asn/candidatura/cconversorcandidaturas.cpp | src/uenux2/src/app/comum/dados/asn/candidatura/cconversorcandidaturas.cpp | low |
| 43 | 5711 | 20 | yes | `comum::asn::IConversorASN<ModuloCandidatos::DadosCandidato, comum::md::CDadosCandidato>::Desconverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 44 | 5712 | 35 |  | `comum::asn::CConversorCandidatura::CConversorCandidatura(TCargoID, TPartidoID, bool)` | uenux2/src/app/comum/dados/asn/candidatura/cconversorcandidatura.h | src/uenux2/src/app/comum/dados/asn/candidatura/cconversorcandidatura.h | medium |
| 45 | 5713 | 20 |  | `comum::asn::IConversorASN<ModuloLocal::SecaoEleitoral, comum::md::CSecaoEleitoral>::Converte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 46 | 5714 | 20 |  | `comum::asn::IConversorASN<ModuloTiposCadastro::Municipio, comum::md::CMunicipio>::Converte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 47 | 5715 | 20 | yes | `comum::asn::IConversorASN<ModuloLocal::SecaoEleitoral, comum::md::CSecaoEleitoral>::Desconverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 48 | 5716 | 20 | yes | `comum::asn::IConversorASN<ModuloTiposCadastro::Municipio, comum::md::CMunicipio>::Desconverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 49 | 5721 | 20 | yes | `comum::asn::IConversorASN<ModuloTiposEleitorais::Abrangencia, comum::md::CAbrangencia>::Desconverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 50 | 5731 | 264 |  | `ASN1::SEQUENCE_OF<ASN1::GeneralString>::operator=(const SEQUENCE_OF&)` | (III ASN.1 runtime template) | library/inlined helper (no TSE source) | medium |
| 51 | 5827 | 490 |  | `comum::asn::Utils::DesconverteAbrangencia` | uenux2/src/app/comum/asn/util.cpp | src/uenux2/src/app/comum/asn/util.cpp | high |
| 52 | 5828 | 575 |  | `comum::asn::IConversorASN<ModuloTiposEleitorais::CodigoCargoConsulta, unsigned char>::Desconverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 53 | 5950 | 399 |  | `std::__uninitialized_allocator_relocate<std::allocator<comum::md::CCandidatura>> (vector<CCandidatura> growth)` | libcxx/include/__memory/uninitialized_algorithms.h | library/inlined helper (no TSE source) | medium |
| 54 | 9209 | 492 |  | `ModuloFederacoes::EntidadeFederacoes::set_federacoes(const SEQUENCE_OF<Federacao>&)` | (III ASN.1 generated code, ModuloFederacoes) | library/inlined helper (no TSE source) | low |
| 55 | 9843 | 88 |  | `std::make_format_args<std::format_context>(std::string&)` | libcxx/include/__format/format_arg_store.h | library/inlined helper (no TSE source) | medium |
| 56 | 9997 | 20 | yes | `comum::asn::IConversorASN<ModuloEstadoGeralVota::EstadoGeralVota, comum::md::estadoaplicacao::CEstadoGeralVota>::Converte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 57 | 10019 | 20 | yes | `comum::asn::IConversorASN<ModuloEstadoGeralSA::EstadoGeralSA, comum::md::estadoaplicacao::CEstadoGeralSA>::Converte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 58 | 10051 | 20 | yes | `comum::asn::IConversorASN<ModuloEstadoGeralGap::EstadoGeralGap, comum::md::estadoaplicacao::CEstadoGeralGap>::Converte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 59 | 10170 | 20 | yes | `comum::asn::IConversorASN<ModuloEstadoGeralUrna::EstadoGeralUrna, comum::md::estadoaplicacao::CEstadoGeral>::Converte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 60 | 10264 | 909 |  | `comum::asn::CConversorVersoesArquivos::DoConverte` | uenux2/src/app/comum/gravadores/asn/cconversorversoesarquivos.cpp (path inferred) | src/uenux2/src/app/comum/gravadores/asn/cconversorversoesarquivos.cpp | high |
| 61 | 10265 | 29 |  | `comum::asn::IConversorASN<ModuloHashes::EntidadeHashes, comum::md::CEntidadeHashes>::DoDesconverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 62 | 10266 | 1338 |  | `comum::asn::CConversorEntidadeHashes::DoConverte` | uenux2/src/app/comum/gravadores/asn/cconversorentidadehashes.cpp (path inferred) | src/uenux2/src/app/comum/gravadores/asn/cconversorentidadehashes.cpp | high |
| 63 | 10272 | 29 |  | `comum::asn::IConversorASN<ModuloBoletimUrna::EntidadeBoletimUrna, comum::md::CEntidadeBU>::DoDesconverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 64 | 10275 | 29 |  | `comum::asn::IConversorASN<ModuloBoletimUrna::HistoricoVotoImpresso, comum::md::CHistoricoVotoImpresso>::DoDesconverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 65 | 11358 | 29 |  | `comum::asn::IConversorASN<ModuloEleicao::Suplencias, comum::md::CSuplencia>::DoConverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 66 | 11359 | 419 | yes | `comum::asn::CConversorSuplencia::DoDesconverte` | uenux2/src/app/comum/dados/asn/processoeleitoral/cconversorsuplencia.cpp (path inferred) | src/uenux2/src/app/comum/dados/asn/processoeleitoral/cconversorsuplencia.cpp | high |
| 67 | 11360 | 29 |  | `comum::asn::IConversorASN<ModuloSituacoesEleicoes::EntidadeSituacoesEleicoes, std::vector<comum::md::CSituacoesEleicoes>>::DoConverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 68 | 11362 | 29 |  | `comum::asn::IConversorASN<ModuloProcessoEleitoral::EntidadeProcessoEleitoral, comum::md::CProcessoEleitoralDTO>::DoConverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 69 | 11364 | 29 |  | `comum::asn::IConversorASN<ModuloProcessoEleitoral::Pleito, comum::md::CPleitoDTO>::DoConverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 70 | 11368 | 29 |  | `comum::asn::IConversorASN<ModuloEleicao::NomesCargo, comum::md::CNomesCargo>::DoConverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 71 | 11370 | 29 |  | `comum::asn::IConversorASN<ModuloEleicao::EntidadeEleicao, comum::md::CEleicaoPE>::DoConverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 72 | 11372 | 29 |  | `comum::asn::IConversorASN<ModuloEleicao::CargoPergunta, comum::md::CCargo>::DoConverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 73 | 11374 | 29 |  | `comum::asn::IConversorASN<ModuloEleicao::DetalheCargo, comum::md::CDetalheCandidato>::DoConverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 74 | 11376 | 29 |  | `comum::asn::IConversorASN<ModuloEleicao::DetalhePergunta, comum::md::CDetalheConsulta>::DoConverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 75 | 11379 | 257 |  | `comum::asn::CConversorMunicipio::DoConverte` | uenux2/src/app/comum/dados/asn/municipiozona/cconversormunicipio.cpp (path inferred) | src/uenux2/src/app/comum/dados/asn/municipiozona/cconversormunicipio.cpp | high |
| 76 | 11380 | 29 |  | `comum::asn::IConversorASN<ModuloComplementosMunicipios::EntidadeComplementosMunicipios, comum::md::CComplementosMunicipiosUF>::DoConverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 77 | 11382 | 691 | yes | `comum::asn::CConversorComplementoMunicipio::DoDesconverte` | uenux2/src/app/comum/dados/asn/municipiozona/cconversorcomplementomunicipio.cpp (path inferred) | src/uenux2/src/app/comum/dados/asn/municipiozona/cconversorcomplementomunicipio.cpp | high |
| 78 | 11383 | 753 |  | `comum::asn::CConversorComplementoMunicipio::DoConverte` | uenux2/src/app/comum/dados/asn/municipiozona/cconversorcomplementomunicipio.cpp (path inferred) | src/uenux2/src/app/comum/dados/asn/municipiozona/cconversorcomplementomunicipio.cpp | high |
| 79 | 11403 | 295 | yes | `comum::asn::CConversorDadoLocal::DoConverte` | uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversordadolocal.cpp (path inferred) | src/uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversordadolocal.cpp | high |
| 80 | 11409 | 29 |  | `comum::asn::IConversorASN<ModuloImpedidos::EntidadeImpedidos, std::vector<comum::md::CImpedido>>::DoConverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 81 | 11415 | 29 |  | `comum::asn::IConversorASN<ModuloEleitores::Dedo, comum::md::CDedo>::DoConverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 82 | 11418 | 418 |  | `comum::asn::IConversorBiometriaASN<ModuloEleitores::BiometriaEleitorCifrada, comum::md::CBiometriaEleitor>::DoConverte` | uenux2/src/app/comum/asn/iconversorbiometriaasn.h | src/uenux2/src/app/comum/asn/iconversorbiometriaasn.h | high |
| 83 | 11421 | 29 |  | `comum::asn::IConversorASN<ModuloEleitores::BiometriaEleitor, comum::md::CBiometriaEleitor>::DoConverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 84 | 11423 | 5864 |  | `comum::asn::CConversorDadosDisponiveisCarga::DoDesconverte` | uenux2/src/app/comum/dados/asn/correspondencia/cconversordadosdisponiveiscarga.cpp (path inferred) | src/uenux2/src/app/comum/dados/asn/correspondencia/cconversordadosdisponiveiscarga.cpp | high |
| 85 | 11424 | 3377 |  | `comum::asn::CConversorDadosDisponiveisCarga::DoConverte` | uenux2/src/app/comum/dados/asn/correspondencia/cconversordadosdisponiveiscarga.cpp (path inferred) | src/uenux2/src/app/comum/dados/asn/correspondencia/cconversordadosdisponiveiscarga.cpp | high |
| 86 | 11438 | 2119 |  | `comum::asn::CConversorCabecalhoPacote::DoConverte` | uenux2/src/app/comum/asn/cconversorcabecalhopacote.cpp (path inferred) | src/uenux2/src/app/comum/asn/cconversorcabecalhopacote.cpp | high |
| 87 | 11439 | 29 |  | `comum::asn::IConversorASN<ModuloFotosCandidatos::FotoCandidato, comum::md::CFotoCandidato>::DoConverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 88 | 11444 | 29 |  | `comum::asn::IConversorASN<ModuloCandidatos::EntidadeCandidatos, std::vector<comum::md::CCandidatura>>::DoConverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 89 | 11445 | 281 | yes | `comum::asn::CConversorCandidaturas::DoDesconverte` | uenux2/src/app/comum/dados/asn/candidatura/cconversorcandidaturas.cpp (path inferred) | src/uenux2/src/app/comum/dados/asn/candidatura/cconversorcandidaturas.cpp | high |
| 90 | 11446 | 29 |  | `comum::asn::IConversorASN<ModuloCandidatos::Candidatura, comum::md::CCandidatura>::DoConverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 91 | 11449 | 29 |  | `comum::asn::IConversorASN<ModuloCandidatos::DadosCandidato, comum::md::CDadosCandidato>::DoConverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | high |
| 92 | 11454 | 1522 |  | `comum::asn::CConversorSecaoEleitoral::DoConverte` | uenux2/src/app/comum/dados/asn/cconversorsecaoeleitoral.cpp (path inferred) | src/uenux2/src/app/comum/dados/asn/cconversorsecaoeleitoral.cpp | high |
| 93 | 11455 | 71 |  | `comum::asn::CConversorIdentificacaoAgregada::DoDesconverte` | uenux2/src/app/comum/dados/asn/cconversoridentificacaoagregada.cpp (path inferred) | src/uenux2/src/app/comum/dados/asn/cconversoridentificacaoagregada.cpp | high |
| 94 | 11457 | 2011 | yes | `comum::asn::CConversorEntidadePartidos::DoDesconverte` | uenux2/src/app/comum/dados/asn/cconversorentidadepartidos.cpp (path inferred) | src/uenux2/src/app/comum/dados/asn/cconversorentidadepartidos.cpp | high |
| 95 | 11458 | 1955 |  | `comum::asn::CConversorEntidadePartidos::DoConverte` | uenux2/src/app/comum/dados/asn/cconversorentidadepartidos.cpp (path inferred) | src/uenux2/src/app/comum/dados/asn/cconversorentidadepartidos.cpp | high |
| 96 | 11461 | 526 | yes | `comum::asn::CConversorAbrangencia::DoDesconverte` | uenux2/src/app/comum/asn/cconversorabrangencia.cpp | src/uenux2/src/app/comum/asn/cconversorabrangencia.cpp | high |
| 97 | 11462 | 1402 |  | `comum::asn::CConversorAbrangencia::DoConverte` | uenux2/src/app/comum/asn/cconversorabrangencia.cpp | src/uenux2/src/app/comum/asn/cconversorabrangencia.cpp | high |
| 98 | 11482 | 594 |  | `comum::asn::CConversorRegistroDigitalVoto<comum::asn::CConversorEleicoesVota>::DoDesconverte` | uenux2/src/app/comum/gravadores/asn/cconversorregistrodigitalvoto.h (path inferred) | src/uenux2/src/app/comum/gravadores/asn/cconversorregistrodigitalvoto.h | high |
| 99 | 11483 | 1181 | yes | `comum::asn::CConversorRegistroDigitalVoto<comum::asn::CConversorEleicoesVota>::DoConverte` | uenux2/src/app/comum/gravadores/asn/cconversorregistrodigitalvoto.h (path inferred) | src/uenux2/src/app/comum/gravadores/asn/cconversorregistrodigitalvoto.h | high |
| 100 | 11513 | 37 |  | `std::unique_ptr<comum::CFotos>::~unique_ptr (atexit handler of the CFotos singleton @1838844)` | uenux2/src/app/comum/dados/cfotos.cpp | src/uenux2/src/app/comum/dados/cfotos.u21.cpp (comment) | medium |
| 101 | 11588 | 119 | yes | `comum::asn::CConversorCabecalhoEntidade::DoDesconverte` | uenux2/src/app/comum/asn/cconversorcabecalhoentidade.cpp | src/uenux2/src/app/comum/asn/cconversorcabecalhoentidade.cpp | high |
| 102 | 11589 | 701 |  | `comum::asn::CConversorCabecalhoEntidade::DoConverte` | uenux2/src/app/comum/asn/cconversorcabecalhoentidade.cpp | src/uenux2/src/app/comum/asn/cconversorcabecalhoentidade.cpp | high |
| 103 | 12641 | 544 | yes | `api::CDataImage<comum::CCandidaturasDSFoto>::GetImage` | uenux2/src/app/comum/dados/ccandidaturas.cpp | src/uenux2/src/app/comum/dados/ccandidaturas.u21.cpp | medium |

---

## 15. Open questions

* Names of the `EExtensaoArquivoResultado` enumerators (only the suffixes are in the binary); meaning of `red.vsc`,
  `rdvred.dat`, `asw.vsc`, `ahw.vsc`, and why ids 13 and 14 share `log.jez`.
* The `IConversorBiometriaASN` default `DoDesconverte` is absent (pure virtual or never instantiated).
* `CFotos` member names (`Busca`, `Existe`, `m_atual`) and the `SIndiceFoto` / `md::CIndexer` field names are
  inferred. Both callers of `GetImagem` (12641 and 12520) call `CFotos::GetInst()` (2819) and drop the result right
  before it, which fits a source form `CFotos::GetInst().GetImagem(id)` with the unused `this` removed as well as a
  static `GetImagem` preceded by a separate `GetInst()` statement.
* Paths of the converter files without srcloc records are inferred (§13).
