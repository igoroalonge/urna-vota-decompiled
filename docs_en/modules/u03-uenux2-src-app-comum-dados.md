# u03 — `uenux2/src/app/comum/dados`: ASN.1 converters for the election data and the urna state, plus cargo text sources

Unit u03 has 89 functions. 35 of them were seen executing in the recorded votes. It covers 25 original
source files under `uenux2/src/app/comum/dados/`, plus one file whose path was inferred
(`asn/municipiozona/cconversorhorarioverao.cpp`). About three quarters of the code is **converters**.
A converter turns the BER-decoded ASN.1 objects of the data files and state files (`Modulo*`, see
`docs/data-model/asn1-schemas.md`) into the application's model classes (`comum::md::*`), and in the
other direction when the urna writes a file. The rest is text **data sources** for the screens and the
printed reports (`ccargods.cpp`), and the `CCandidaturas` singleton helpers.

The reconstructed sources are in `src/uenux2/src/app/comum/dados/`. There is one `.cpp` per original
file, and a `.h` when a class declaration helps. The complete function map is in §13.

---

## 1. Glossary

| term | meaning |
|---|---|
| *converter* (`CConversorX`) | a class derived from `comum::asn::IConversorASN<ENTIDADE, DADO>`. It converts ASN.1 into md with **Desconverte** and md into ASN.1 with **Converte** |
| *md* (`comum::md`) | "modelo de dados": the application's plain C++ data classes |
| *processo eleitoral / pleito / eleição* | the election process, its rounds (1st/2nd *pleito*) and the elections inside each round |
| *cargo / consulta* | an office to vote for. A *consulta* (referendum question) is modelled as a cargo whose detail is a question with answers |
| *suplente / suplência* | substitute candidates shown next to the titular: vice-prefeito, vice-governador, 1st/2nd suplente de senador |
| *seção / zona / município / local* | polling section / electoral zone / municipality / polling place |
| *urna de contingência* | spare urna. Its `Local` has only município + zona, with no section |
| *TTE* (transferência temporária de eleitor) | voter temporarily moved to another section (voto em trânsito, preso provisório, acessibilidade, …) |
| *impedido* | voter who is barred from voting in this section, per round (P1/P2) |
| *biometria* | a voter's photo and fingerprint templates (*minúcias* = minutiae) |
| *CEPESC* | the Brazilian government cryptography centre (ABIN) and the cipher library `ecourna::api::cepesc` |
| *RDV* (registro digital do voto) | the list of every vote cast, per eleição and cargo, not linked to any voter |
| *GAP* | the urna's application launcher: it decides which application (vota, apuração, treinamento…) runs next |
| *SA* (sistema de apuração) | the counting application used when an urna fails |
| *zerésima / BU* | the pre-voting zero report and the *boletim de urna*, the section's tally |
| *DS* (data source) | a function object that returns a text for a GUI field or a printed report (`api::CDataText<DS>`) |

---

## 2. Files, classes and the data they handle

| original file | class(es) | ASN.1 type ↔ md type | data file |
|---|---|---|---|
| `asn/cconversorlocal.cpp` | `CConversorLocal` | `ModuloLocal::Local` ↔ `md::CLocal` | `<mun><zona><secao>-lo.dat` |
| `asn/candidatura/cvisitantefoto.cpp` | `CVisitanteFoto` (ASN1 visitor) | index of `EntidadeFotosCandidatos` | `-fo.dat` (candidate photos) |
| `asn/eleitor/cconversorbiometriaeleitor.cpp` | `CConversorBiometriaEleitor` | `BiometriaEleitor` ↔ `md::CBiometriaEleitor` | decrypted part of `-el.dat` |
| `asn/eleitor/cconversorbiometriaeleitorcifrada.cpp` | `CConversorBiometriaEleitorCifrada` | `BiometriaEleitorCifrada` → `md::CBiometriaEleitor` | `-el.dat` + `chave/bio.sk1` |
| `asn/eleitor/cconversordedo.cpp` | `CConversorDedo` | `Dedo` → `md::CDedo` (fingerprint template) | inside the biometria |
| `asn/eleitor/cconversorentidadeeleitores.cpp` | `CConversorEntidadeEleitores` (partial) | `EntidadeEleitores` → `md::CEntidadeEleitores` | `-el.dat`, `-tte.dat` (voter roll) |
| `asn/eleitor/cconversorimpedido.cpp` | `CConversorImpedido` | `EntidadeImpedidos` → `vector<md::CImpedido>` | `-imp.dat` |
| `asn/eleitor/cvisitanteeleitor.cpp` | `CVisitanteEleitor` (ASN1 visitor) | biometria offsets per voter identity | `-el.dat` |
| `asn/estadoaplicacao/cconversorajustedatahora.cpp` | `CConversorAjusteDataHora` | `AjusteDataHora` ↔ `CAjusteDataHora` | in `eg.bin` |
| `asn/estadoaplicacao/cconversordadocarga.cpp` | `CConversorDadoCarga` | `DadoCarga` ↔ `CDadoCarga` | in `eg.bin` |
| `asn/estadoaplicacao/cconversorestadogeral.cpp` | `CConversorEstadoGeral` | `EstadoGeralUrna` ↔ `CEstadoGeral` | `dinamico/eg.bin` |
| `asn/estadoaplicacao/cconversorestadogeralgap.cpp` | `CConversorEstadoGeralGap` | `EstadoGeralGap` ↔ `CEstadoGeralGap` | `trab1\|2/gap.bin` |
| `asn/estadoaplicacao/cconversorestadogeralsa.cpp` | `CConversorEstadoGeralSA` | `EstadoGeralSA` ↔ `CEstadoGeralSA` | `trab1\|2/sa.bin` |
| `asn/estadoaplicacao/cconversorestadogeralvota.cpp` | `CConversorEstadoGeralVota` | `EstadoGeralVota` ↔ `CEstadoGeralVota` | `trab1\|2/vota.bin` |
| `asn/municipiozona/cconversorcomplementosmunicipios.cpp` | `CConversorComplementosMunicipios` | `EntidadeComplementosMunicipios` → `CComplementosMunicipiosUF` | `-cm.dat` |
| `asn/municipiozona/cconversorhorarioverao.cpp` *(path inferred)* | `CConversorHorarioVerao` (DoConverte only) | `HorarioVerao` ← `CHorarioVerao` | in `Local` / `-cm.dat` |
| `asn/processoeleitoral/cconversorcargo.cpp` | `CConversorCargo` | `CargoPergunta` → `md::CCargo` | `-ce.dat` (eleição) |
| `asn/processoeleitoral/cconversordetalhecandidato.cpp` | `CConversorDetalheCandidato` | `DetalheCargo` → `CDetalheCandidato` | `-ce.dat` |
| `asn/processoeleitoral/cconversororigemconfiguracao.cpp` | `CConversorOrigemConfiguracao` | `OrigemConfiguracao` ↔ `EOrigemConfiguracao` | `-cp.dat` |
| `asn/processoeleitoral/cconversorprocessoeleitoral.cpp` | `CConversorProcessoEleitoral` | `EntidadeProcessoEleitoral` → `CProcessoEleitoralDTO` | `-cp.dat` |
| `asn/processoeleitoral/cconversortipoidentificadoreleitor.cpp` | `CConversorTipoIdentificadorEleitor` | `TipoIdentificadorEleitor` ↔ `ETipoIdentificadorEleitor` | `-cp.dat` |
| `asn/rdv/cconversoreleicoesvota.cpp` | `CConversorEleicoesVota` | `Eleicoes` ↔ `md::CVotosEleicoesVota` | body of the RDV (`rdv.dat`) |
| `asn/rdv/cconversorvoto.cpp` | `CConversorVoto` | `Voto` ↔ `md::CVoto` | one RDV vote |
| `asn/rdv/cconversorvotoscargo.cpp` | `CConversorVotosCargo` | `VotosCargo` ↔ `pair<SCargoInfo, CVotos>` | RDV votes of one cargo |
| `ccandidaturas.cpp` | `CCandidaturas`, `GetCandidaturaAtual` | — | candidacy list in memory |
| `ccargods.cpp` | `CCargoDSNomeSexoCandidato`, `CCargoDSLabelRelatorio`, `GetNomeCargoComGenero` | — | screen / report texts |

---

## 3. Class hierarchy (RTTI) and the converter protocol

```
comum::asn::IConversorASN<ENTIDADE, DADO>                 (uenux2/src/app/comum/asn/iconversorasn.h, unit u21)
 ├─ CConversorLocal, CConversorCargo, CConversorDetalheCandidato, CConversorOrigemConfiguracao,
 │  CConversorProcessoEleitoral, CConversorTipoIdentificadorEleitor, CConversorComplementosMunicipios,
 │  CConversorHorarioVerao, CConversorBiometriaEleitor, CConversorDedo, CConversorImpedido,
 │  CConversorAjusteDataHora, CConversorDadoCarga, CConversorEstadoGeral, CConversorEstadoGeralGap,
 │  CConversorEstadoGeralSA, CConversorEstadoGeralVota, CConversorVoto, CConversorVotosCargo,
 │  CConversorEleicoesVota                                (all "si" = single inheritance)
comum::asn::IConversorBiometriaASN<BiometriaEleitorCifrada, CBiometriaEleitor>
 └─ CConversorBiometriaEleitorCifrada
comum::asn::IConversorParcialASN<EntidadeEleitores, CEntidadeEleitores, CVisitanteEleitor>
 └─ CConversorEntidadeEleitores                (sibling: CIndexadorFotos over CVisitanteFoto, in another unit)
ASN1::IAbstractVisitor
 ├─ CVisitanteEleitor
 └─ CVisitanteFoto
api::IText
 ├─ api::CDataText<CCargoDSNomeSexoCandidato>
 └─ api::CDataText<CPadDS<CToUpperDS<CCargoDSNome>>>
comum::IServicoEstado<CEstadoGeral|CEstadoGeralVota|CEstadoGeralGap|CEstadoGeralSA, CConversor...>  (users)
```

**`IConversorASN` vtable** (4 slots, reconstructed from 20 vtables): `[0]` destructor (almost always the ICF
`return this`, func 174). `[1]` deleting destructor (func 144 `free`). `[2]` `DoConverte(const TDado&)`
returning `TEntidade`. `[3]` `DoDesconverte(const TEntidade&)` returning `TDado`. The srcloc signatures
confirm the names and the return by value, e.g. `virtual md::CLocal comum::asn::CConversorLocal::DoDesconverte(const TEntidade &) const`.
The non-virtual wrappers in the base class are:

* `Desconverte(e)` first requires `e.isValid() && e.isStrictlyValid()`. Otherwise it throws
  `CBaseError<EUeComumAsnError>(7654, "Entidade está inválida: {}")` with the `ASN1::trace_invalid` dump
  (`iconversorasn.h:71`). Then it calls `DoDesconverte`.
* `Converte(d)` calls `DoConverte` and then requires the *result* to be valid. Otherwise it throws `7653 "Entidade
  deixada em estado inválido: {}"` (`iconversorasn.h:56`).
* A converter that only implements one direction leaves the base slot in place. That base slot throws
  `"Método DoConverte() não implementado para {}"`. The urna never writes voter, candidate or eleição files.

`IConversorParcialASN` has only 3 slots. Its slot 2 is `DoDesconverte(const TEntidade&, const TVisitor&)`.
`IConversorBiometriaASN` adds a `const md::CParametro&` parameter to both directions.

**Naming trap.** Many vtable functions were named by the tool after an **inlined helper**, because the
srcloc record belongs to that helper (e.g. 11355 "`CConversorVoto::ConverteTipo`" is really
`CConversorVoto::DoConverte`, and 11410 "`MontaEleitorIdentidade`" is `CConversorImpedido::DoDesconverte`).
The vtable slot decides the real method. §13 gives the corrected names.

---

## 4. Loading the configuration of the section

These converters run while the urna loads its static data (`/dsk/fi/estatico/...`).

* **Local** (`CConversorLocal`, 11451/11452). The fields are idPE, país, UF (sigla + nome), Município (+ optional
  `comBiometria`, absent = false), ComplementoMunicipio (fuso, horário de verão) and the `id` CHOICE:
  `secao [1] SecaoEleitoral` for a normal urna, or `contingencia [2] {municipio, zona}` for a spare urna. A
  CHOICE id outside 0/1 throws `7887 "CHOICE de id inválida."`. `DoConverte` (the urna writes its local)
  checks contingência **before** seção. If neither is set it throws `7888 "Tipo de local indefinido."`. It
  also validates the result itself (`7889`/`7890`) on top of the base-class check. The md object is
  `md::CLocal` (140 bytes). Its municipality part is an `md::CInfoMunicipio`, built by the inline constructors
  5674/5676. Both end in `CInfoMunicipio::ValidaCriacao`, and the `CLocal` constructor ends in `CLocal::ValidaCriacao`.
* **Complementos de municípios** (11381). The file must contain **exactly one** UF (`7944 "Quantidade inválida
  de UFs [{}]"`). The UF sigla must have 2 characters (8115) and the list must not be empty (8116).
  `fusoTRE` and the cabeçalho are ignored.
* **Processo eleitoral** (11363). It keeps id, nome, pleito1, optional pleito2, `utilizaBiometria`,
  `origemConfiguracao` and the identifier types. The main identifier type must appear in the list of
  allowed types (`8185 "Identidade principal ({}) ausente na lista de identidades"`). The id comes from the
  cabeçalho `IDEleitoral` CHOICE, and the code **does not check which alternative is selected**.
  `dataEleitorado`, `tipoEleicaoPrincipal`, abrangência, `permiteMRJ`, `tiposTTEPermitidos` and `ufs` are
  dropped here.
* **Cargo** (11373). `TipoCargoConsulta` 1..3 maps to `CCargo::ETipo` 0..2. The converter object carries
  the eleição's abrangência (set by `CConversorEleicaoPE`, func 11371) and copies it into every cargo.
  `detalhe` is a CHOICE: `cargo [0]` goes to `CConversorDetalheCandidato`, `pergunta [1]` goes to
  `CConversorDetalheConsulta`, anything else throws `7948 "Opção de Detalhe Inválida."`. The md `CCargo` is
  140 bytes and holds `optional<CDetalheCandidato>` (flag +84) and `optional<CDetalheConsulta>` (flag
  +136). The "is this a consulta?" tests elsewhere read the flag at +136.
* **DetalheCandidato** (11375) enforces `qtdeSuplentes == size(suplencias)` with three distinct errors
  (7950/7951/7952). `temFoto` and `nomes` (neutral/male/female/abbreviated cargo name) are copied.
* **OrigemConfiguracao** (11366/11367). 1 = oficial, 2 = comunitária. `DoConverte` writes anything that
  is not "comunitária" as "oficial".
* **TipoIdentificadorEleitor** (11356/11357). 1 = inscrição (título de eleitor), 2 = CPF, 3 = livre. Both
  directions range-check the value (8183/8184).

---

## 5. The voter roll and the biometrics

### 5.1 Streaming ("partial") decoding of `-el.dat`

The voter file can contain encrypted fingerprints for every voter, so it is read with TSE's partial BER
decoder (docs/libraries/asn1-runtime.md §6). The inlined
`IConversorParcialASN<EntidadeEleitores,…>::Desconverte(const api::CFile&, shared_ptr<CVisitanteEleitor>)`
(inside func 11523, the `std::function` behind `CEleitores::GetEleitoresEstaticos`) does four things:

1. It allocates a `CVisitanteEleitor` (64 bytes) and sets its word at +4 to 1.
2. It registers the visitor for two dotted paths: `EntidadeEleitores.eleitores.eleitor.identificacaoEleitor.OF`
   and `EntidadeEleitores.eleitores.eleitor.biometria`.
3. It decodes the file with `api::CPartialFileASN::ReadFromFile`.
4. It calls `CConversorEntidadeEleitores::DoDesconverte(entidade, *visitor)` (func 11411).

`CVisitanteEleitor::visit` (11407) handles each registered path:

* `…identificacaoEleitor.OF`: it stores the number into one of three strings (inscrição / CPF / livre),
  selected by the CHOICE id, and returns 0 (keep).
* `…biometria`: it stores the element's `[begin, end)` file offsets. Then, for each non-empty stored
  identity, it calls `CValidadorIdentidade::Valida(numero, tipo)` and does
  `map.emplace(identidade, {begin, end})`. It returns 1, so the decoded blob is freed by `ASN1::DataFree`.
* Any other path throws `7905 "Entidade visitada inválida: " + path`.

### 5.2 `CConversorEntidadeEleitores::DoDesconverte` (11411)

For each `EleitorSequencia`:

| step | what the code does |
|---|---|
| identities | `GetInscricoesEleitor(EleitorSequencia)` (a namespace-level function that takes the SEQUENCE **by value**). It throws 7899 if there is no identity. It builds `CEleitorIdentidade {numero, tipo}` with `CValidadorIdentidade::Valida` (tipo 1/2/3 from the CHOICE id). It rejects duplicates with `std::adjacent_find`, so **only consecutive duplicates** are caught (7900) |
| necessidade | `ConverteNecessidadeEspecial(EleitorUrna)` (by value again): semNecessidade → 0, necessitaAudio → 1, otherwise 7898 |
| names | `nome` and the optional `nomeSocial` are **cut to 40 characters**. The schema allows 70 |
| TTE | `DesconverteTipoTransferenciaTemporaria`: votoEmTransito, servidorEmServico, eleitorConvocado and justicaEleitoral all map to **md value 1**. presoProvisorio → 2, acessibilidade → 3, ofício → 4, indígena/quilombola → 5, situação de rua → 6. Absent **or any unknown value** → 0 (no transfer). Only the value −1 throws (7901). If there is a transfer, the voter must have a `domicilio` (7902). `CTransferenciaTemporaria` keeps the domicílio's UF and código de município |
| birth date | `md::CDataJE` requires exactly 8 ASCII digits (8002) |
| biometrics | the first identity of the voter found in the visitor's map gives the offsets. The voter is built with the offsets **only if** the file says `biometria` is present **and** an offset was found. Otherwise the biometrics are silently dropped |
| key | if `seguranca` is present, `Seguranca.idArquivoChave` is copied into every voter with biometrics. The Seguranca record is converted twice |

Nothing else is read from `EleitorUrna`: `nomeMae`, `situacao`, and the domicílio's zone, section and
dates are all ignored. `md::CEleitor` is 104 bytes. Its layout is in the header
`cconversorentidadeeleitores.h`. The training scenarios contain **one voter** each, with no biometria, no
seguranca and no TTE.

### 5.3 Impedidos (`-imp.dat`, 11410)

For each `EleitorImpedido`, the code converts `impedimentoP1` (`TipoImpedimento` 1..15 → 0..14, otherwise
7904), then the identity (7903 for an unknown CHOICE id), then the optional `impedimentoP2`.
`md::CImpedido` stores `p2 = 15` and `temP2 = false` when P2 is absent (constructors 5661/5662). Example
from the `geral-t1` scenario: `{numeroInscricao XXXXXXXXXXXX, P1 semImpedimento, P2 suspenso}`.

### 5.4 Encrypted biometrics (11419), step by step

1. `LeChave()` (inlined, lines 91/101/109) reads the key:
   * path = `CPath` key directory `"/dsk/fi/estatico/chave/"` + `"bio.sk1"`. If the file is missing it
     throws `7894 "O arquivo … não existe"`.
   * The file is decoded as `ModuloEnvelopeChave::EntidadeChave`, and its `chave` OCTET STRING is taken.
     **`cifrado` and `tipo` are ignored**.
   * `api::CPolySingletonList::instance<api::IKernelHSM>()` (srcloc line 101) → HSM vtable slot 3 returns a
     secret. That secret is turned into a `std::string`, then `CSymmetricCipherFactory` builds a cipher,
     and cipher slot 3 decrypts the key. An empty result throws `7895 "O arquivo … está vazio"`.
2. `CCipheredIn(0, 1, 0, chaveSessao = CParametro+16, chaveSecreta, conteudo, CInfoSalt(salt, CParametro+0))`.
   Its constructor refuses an empty session key, secret key or content (ESecurityError 1501/1502/1503).
3. `CCepescCipher::Decifra` (func 5171) → plaintext.
4. `api::CFileASN::DecodeObject<ModuloEleitores::BiometriaEleitor>` ("DecodeObject de " + type name). It
   raises EUeIoError 5953 if decoding fails and 5954 if the result is invalid.
5. `CConversorBiometriaEleitor` (11422): if there is neither foto nor dedos it throws 7893 ("…mas falou
   que tem biometria"). Otherwise it converts the fingers with `CConversorDedo` and the photo with
   `ecourna CConversorFoto`, then builds `CBiometriaEleitor(foto[, dedos])` or `CBiometriaEleitor(dedos)`.

`composicaoBiometria` is never checked against what is actually present.

### 5.5 Fingerprint template format (`CConversorDedo`, 11416 + 5702)

`Dedo ::= { tipo TipoDedo, qtdMinucias INTEGER, minucias OCTET STRING }`. The OCTET STRING is bit-packed.
Bits are read most-significant first through `ecourna::api::util::CBitArray::GetValor(n)`:

```
xBase:10  bitsX:4  yBase:10  bitsY:4  qtdGrupos:8
repeat qtdGrupos:  angulo:9  quantidade:8  repeat quantidade: dx:bitsX  dy:bitsY
minutia = { x = xBase + dx, y = yBase + dy, t = angulo, 0 }   (api::SXYT, 16 bytes)
```

`DoDesconverte` unpacks the template, checks `qtdMinucias == number of minutiae` (7897), maps `tipo` 1..10
(`naoIdentificado(0)` is refused, 7896), and **unpacks the template a second time** for `md::CDedo`. The
`CDedo` constructor limits a template to 1..200 minutiae (8061/8062). The unpacker's helper object also
computes the bounding box and `ceil(log2(max-min))` bit widths. These would be needed to re-pack a
template, and they are unused here.

### 5.6 Candidate photos index (`CVisitanteFoto`, 11442)

A two-state sequencer runs over `EntidadeFotosCandidatos.fotos.OF` (it records the `[begin, end)` range of
the `FotoCandidato`) and `EntidadeFotosCandidatos.fotos.codigoCandidato` (the code, taken through
`dynamic_cast<AbstractString*>` and cut at the first NUL). Each pair becomes an index entry
`{codigo, {begin, end}}`. Visiting out of order throws 7883/7884. The photos themselves are freed
(`DataFree`) and read later by offset. This path ran during the recorded general-election vote (func
11442 is in the profile).

---

## 6. The urna's state files (eg.bin, vota.bin, gap.bin, sa.bin)

`comum::IServicoEstado<CEstadoX, CConversorX>` reads and writes these files. The runtime profile shows
`DoConverte` of the eg/gap/vota converters (11397, 11394, 11390) and `DoDesconverte` of eg/gap (11398, 11395)
running during `votaInit`. The SA converter and `CConversorEstadoGeralVota::DoDesconverte` were not sampled,
which does not prove that they are unused: `sa.bin` is written at init. The md enums are **character codes**,
while the ASN.1 enums are integers. Every converter maps them explicitly and refuses the sentinel values:

| file / field | ASN.1 | md | refused |
|---|---|---|---|
| eg.bin `estadoUrna` | carregando 1, carregada 2, testada 3 | `'1' '2' '3'` | md `'4'` (line 106), ASN −1 |
| eg.bin `dadoCarga.turno` | semTurno 0, turno1, turno2 | `'0'..'2'` (`Utils::ConverteTurno`, util.cpp:566/581, EUeComumAsnError 7689/7690) | ≥ 3 |
| eg.bin `tipoUrnaT1/T2` | semtipo 0 … contingenciavotarecupera 4 | `'0'..'4'` | others |
| eg.bin `modelo` | ue2013 13, ue2015 15, ue2020 20, ue2022 22 | model **year** 2013/2015/2020/2022 | `tpm20` and −1 "não suportado" (7915), others (7916) |
| eg.bin `ajusteDataHora.tipo` | semAjuste 1, alterarDataEleicao 2, alterarDataSistema 3 | `ETipoDeltaT` 0..2 | |
| vota.bin `estadoVota` | inicial 0 … encerrada 15, exibealertadesligamento 16, ultimoid 17 | `'1'..'@'` (`'1'`+v) | 16: **"Estado não deve ser usado"** (7931); 17/−1 (7932); md `'A'` (7929) |
| vota.bin `estadoEncerramento` | inicial 0, imprimirobrigatoriabu 1, retirarmr 2, fimdostrabalhos 3, ultimoid 4 | `'1'..'4'` | ultimoid/−1 "não suportado" (7936); md `'5'` (7934) |
| gap.bin `appId/appAnteriorId` | 0..14, apultimoid 15 | same numbers | 15 and −1 |
| sa.bin `estadoSA` | inicial 0 … encerrada 22, ultimoid 23 | `'1'..'G'` | ultimoid/−1; md `'H'` |

What is kept (see the headers for byte layouts):

* **eg.bin**. The md `CEstadoGeral` keeps every field. In the web build `eg.bin` is a fixture:
  `idPE 2400, testada, turno1, vota/vota, ue2015, treinamento, AC 1/1/1, versão "7.2.1.3 - TESTE EG ASN1"`
  (decoded from `analysis/runtime/memfs-after-init`).
* **vota.bin**. qtdBU (truncated to 8 bits), comparecimento and qtdJustificativa (truncated to 16 bits), the
  printed-copies counters (4 × uebyte), the flags, and five optional fields (`urnaIdGerouZeresima`,
  `dhIniAquisicao`, `dhFimAquisicao`, `dhUltimoVoto`, `dhEmissao`) held as `std::optional`. After
  `votaInit` it decodes to `{votar, inicial, qtdBU 0, comparecimento 0, …, treinamentoEleitor TRUE,
  dhIniAquisicao "<now>"}`. **It is identical after a vote**: the web build does not update comparecimento.
* **gap.bin**. Up to 10 load correspondences, current and previous application, 4 flags, copies counters
  for rounds 1 and 2, optional 2nd-round date. The md constructor (5626) **overwrites `data2T`** with
  "the 2nd-round date is present and is today or earlier" (`CDate::Hoje()` from the clock singleton). The
  flag stored in the file is therefore never used. Observed value: `appId = apsemaplicativo`,
  `dataSegundoTurno "20801231"`.
* **sa.bin**. State, `atualizacaoBloqueada`, município/zona/seção. Observed: `{inicial, FALSE, 1/1/2}`.

---

## 7. The RDV body (`asn/rdv/`), step by step

The RDV file wraps `EntidadeRegistroDigitalVoto { pleito, fase, identificacao, historicoCodigosCarga,
eleicoes }`. The wrapper is `CConversorRegistroDigitalVoto<CConversorEleicoesVota>`, in another unit. It
owns a `CConversorEleicoesVota` at +4, built by **moving in** `std::map<TEleicaoID, std::vector<md::CCargo>>`
(the cargos configured for each eleição). This unit converts `eleicoes`.

**md → ASN.1 (`CConversorEleicoesVota::DoConverte`, 11350)**

1. The number of eleições in `md::CVotosEleicoesVota` must equal the number configured. Otherwise it
   throws `7965 "Eleições incompatíveis"`.
2. It selects `Eleicoes.eleicoesVota` (CHOICE 0, `[1]` tag). `eleicoesSA` is the SA's alternative.
3. For each `(idEleicao, CVotosCargos)` in ascending id order (std::map):
   `EleicaoVota { idEleicao, votosCargos }`. The configured cargos are looked up (`7966 "Eleição N não
   encontrada"`, built with `+` and not with format).
4. For each **configured** cargo, in configuration order:
   `SCargoInfo {codigo, tipo, numeroDigitos, qtdeEscolhas}` is copied from `md::CCargo` (+0, +4, +12, +13).
   The votes come from `CVotosCargos::GetVotos(codigo)`, which is inlined and throws
   `CBaseError<api::EUeRdvError>(4671, "Cargo N nao encontrado")`. The rest is done by
   `CConversorVotosCargo(info).Converte({info, votos})`.
5. `CConversorVotosCargo::DoConverte` (11352) refuses a pair whose cargo code differs from its own (`7976
   "Cargo info incompatível"`). It writes `idCargo` (through ecourna `CConversorCargoID`, `CBaseType<ushort,1,99,3>`),
   `quantidadeEscolhas = qtdeEscolhas`, then one `Voto` per `md::CVoto`.
6. `CConversorVoto::DoConverte` (11355) writes `tipoVoto` from `CVoto::ETipo`. The two share the
   numbering: 1 legenda, 2 nominal, 3 branco, 4 nulo, 5 brancoAposSuspensao, 6 nuloAposSuspensao,
   7 nuloPorRepeticao, 8 nuloCargoSemCandidato, 9 nuloAposSuspensaoCargoSemCandidato. Anything else throws
   7973. `digitacao` (the digits as typed, NumericString 1..5) is **omitted when empty**.
7. Each `Converte` call re-validates its product (`isValid && isStrictlyValid`, 7653). A vote with 6 typed
   digits would therefore be refused here.

**ASN.1 → md (`DoDesconverte`, 11349)**: the choice must be `eleicoesVota` (`7967 "Dado não é do Vota"`),
and the counts must match (7968). The eleição (7969) and each cargo code (7970 "Cargo {} não encontrado
para eleição {}") must exist in the configuration. `CConversorVotosCargo::DoDesconverte` (11351) checks
the cargo (7977 "Cargo {} difere do esperado: {}") and `quantidadeEscolhas` (7978 "Quantidade de escolhas
difere do esperado: {}/{}"). Votes decode with `CConversorVoto::DoDesconverte` (11354). An absent
`digitacao` becomes "". The results go into `std::map<SCargoInfo, CVotos>`, where a **duplicate cargo is
silently ignored**, then `CVotosCargos(TMapa)`, then `CVotosEleicoesVota(TMapa)`. Both of these take the
map **by value**. A **duplicate eleição** is silently ignored in the same way (unique-key insert into
`std::map<TEleicaoID, CVotosCargos>`). Because 7968 compares only the counts, an RDV `{X, X}` checked against
a configuration `{X, Y}` decodes without error into a result that has no entry for `Y`.

**In the web build** this code ran only during `votaInit`: `CGeraDadosDinamicos` → `CRdvVota` slot 11 →
`CFileASN::CodeObjectFunction` → `IConversorASN<Eleicoes>::Converte` → 11350 → 11352. It serialises an
empty RDV into the 80-byte encrypted `rdv.dat`. After a vote the RDV is not rewritten
(`analysis/runtime/README.md`).

### Relation to the BU and the encerramento

This unit does **not** build the BU. It supplies pieces that the BU/encerramento flow (units u05, u06,
u21, u23, u27…) relies on:

* the RDV body format and its consistency checks (§7);
* `vota.bin` fields that drive the encerramento: `estadoVota` (`gerarbu → gerarrelatorios → imprimirbu →
  gravarresultados → copiaresultadosmr → encerrada`), `estadoEncerramento` (`imprimirobrigatoriabu →
  retirarmr → fimdostrabalhos`), `qtdBU` (number of BUs printed), `urnaIdGerouZeresima`, `dhEmissao` (BU
  emission time), and `numViasImpressasRelatorios` (copies already printed);
* the labels of the printed BU/zerésima tables (`CCargoDSLabelRelatorio`, §8).

---

## 8. Cargo and candidacy text sources (`ccargods.cpp`, `ccandidaturas.cpp`)

* `GetNomeCargoComGenero(contexto, sexo, suplente)` (5803) returns the cargo name in the gender of the
  current candidate (`CCargo::GetNome(sexo)`), or the suplente's cargo name. `contexto` is only used in
  the error message; callers pass their own pretty function name.
* `CCargoDSNomeSexoCandidato{suplente, abreviaNomeLongo}()` (2268) requires both cursors to be positioned
  and to agree: `"O candidato não foi posicionado corretamente"` (7808), 7809, and `"Cargo e candidato não
  foram posicionados corretamente"` (7810). It takes the candidate's sex from `CCandidaturasDSSexo`. If
  `abreviaNomeLongo` is set and the name has ≥ 19 characters, it returns the abbreviated name instead.
  **Confirmed on the running simulator** (`--keys "91001C  C  91    " --draw`): the confirmation screen for
  prefeito 91 draws `"Prefeita"` (the candidate is female), `"Vice-Prefeito"` (func 6664) and
  `"Vice-Prefeito: Judô"` (func 6669).
* `CCargoDSLabelRelatorio::HeaderDetalhe/HeaderDetalheZE/TotalVotoNominal` return the report column headers
  and the totals line. They switch between candidates and consulta answers:
  `"Nome do candidato       Num cand Votos"` / `"Resposta                Num resp Votos"`, and
  `"Total de votos Nominais           {:04}\n"` / `"Total de votos Válidos            {:04}\n"`
  (Latin-1 literals, format patterns for the caller).
* `CCandidaturas`: 44-byte singleton (static `unique_ptr` @0x1C0E64). It is `api::CDataMap<md::CCandidatura>`
  named `"CCandidaturas"` plus `map<TCargoID, string>` of package versions (`RecuperaVersaoPacote`, 7801).
  `GetCandidaturaAtual(contexto)` returns the current candidacy or throws `"{} - não posicionado no
  candidato corretamente"` (7805).

---

## 9. Web-build specifics

* **CEPESC is a pass-through.** `CCepescCipher::Decifra` (5171) returns the input content unchanged. Its
  sibling `Cifra` (2681, unit u40) wraps the plaintext with a 32-byte zero session key. Voter biometrics
  "encryption" would be the identity in this build. No scenario ships `bio.sk1` or voters with
  biometrics (there is no `chave/` directory in any scenario), so §5.4 never runs in the simulator. No class in the binary's RTTI derives from `api::IKernelHSM`, so on this path the
  `CPolySingletonList` lookup would most likely throw "solicitada uma instância não criada" before any decryption.
* **The state files are fixtures or are frozen.** `eg.bin` comes from a hard-coded `CEstadoGeral` (mock_f10205
  calls ctor 5637). The GAP/SA/VOTA constructors 5625/5626/5628 are also called by simulator fixtures
  (mock_f10101/10123/10067). `vota.bin` is written once at init and not updated after votes.
* **The RDV conversion runs only at init** (empty RDV); see §7.

---

## 10. wasm / Emscripten observations

* **Every converter's non-throwing helpers were inlined** into `DoConverte`/`DoDesconverte`, so the
  srcloc-based name is the helper's (e.g. "ConverteEstadoVota"). The vtable slot gives the real method.
  Some helpers stayed out-of-line (5689/5690/5693/5694/5699) because they are called twice.
* **merge-similar-functions** created bodies without a source equivalent:
  * 6030: shared by `CConversorOrigemConfiguracao::DoDesconverte` and `CConversorTipoIdentificadorEleitor::DoDesconverte`. The parameters are srcloc, message, code and range.
  * 6038: `HeaderDetalhe`/`HeaderDetalheZE`. The parameters are srcloc, code and 10 pointers into the two 38-char literals, because the copy is done in 5 overlapping 8-byte loads.
  * 6075 and 6076: the four suplente text-source lambdas.
* **Captureless lambdas as table slots.** 6664 sits in slots 1091 and 1095 (two identical lambdas folded by ICF); 6669 in 1090 and 1092; 13301 in 1093; 13298 in 1094. They are passed by `adicionaBaseTelaCompletaCandidatoCom1` (6671) and `…Com2` (6660). The "cargo: nome" sources (slots 1090/1092/1093) go to the text-line builder `vota_f3068`. The "cargo only" sources (slots 1091/1094/1095) go to `adicionaFotoEmoldurada` (4181) as the caption under the suplente's photo. The `--draw` run in §8 shows both: `"Vice-Prefeito: Judô"` on a text line at (20, 561) and `"Vice-Prefeito"` under the photo at (1170, 759).
* **Enum formatting.** `std::format("… {}", enum)` goes through `__handle` lambdas 536/1699/1711 (a TSE
  `std::formatter` for enums and ASN.1 ENUMERATED types). The char enums therefore print as numbers (e.g.
  `'4'` prints as 52). The packed argument type tells which case applies (15 = handle, 3 = int, 6 = unsigned).
  Some sites format a plain `int` instead of the enum: 7909 (`EUrnaTipo`, 5693) and 8185
  (`ETipoIdentificadorEleitor`, 11363), plus every `asInt()` of an ASN→md switch (7898, 7901, 7904, 7906/7907,
  7974/7975). The DadoCarga model errors 7915/7916 format the ENUMERATED object itself (handle).
* **The mysterious −1 case.** Every ASN→md enum switch has an explicit case for **−1**, thrown from a line
  *inside* the switch and separate from the default throw after it. The md→ASN switches instead
  special-case their own sentinel (`'4'`, `'A'`, `'H'`, 15…). The simplest explanation is an extra
  "invalid" enumerator (value −1) in each generated `NamedNumber`. Its name is not in the binary.
* **Visible struct layouts.** The md objects are passed as packed words: `CCargoDSNomeSexoCandidato` is one
  16-bit word (257 = `{1, true}`), `CNumViasImpressasRelatorios` is 4 bytes unaligned at +73 of
  `CEstadoGeralVota`, and `api::CDate` is 4 shorts compared by func 1261.
* The partial-decoder path comparisons are inlined `memcmp` of 8-byte words (XOR against constants).
  `std::string(const char*)` from `c_str()` becomes `strlen` + copy.
* The 16-byte literal `"DecodeObject de "` at @443005 was mis-decoded by the tool as `"duplicado …"`,
  because of a +1024 data-segment offset in the pseudo-code's `d_…[N]` references. Read raw constants from
  the `.wat`.

---

## 11. Weird or risky code

The same list is returned to the orchestrator as `suspicious`:

1. **CEPESC decryption is the identity (5171)** and `LeChave` ignores the envelope's `cifrado` flag. See §9.
2. **Stale identities in `CVisitanteEleitor`**. The three identity strings are never cleared between
   voters. A voter without a CPF gets the previous voter's CPF mapped to *its* biometria offset. The
   `emplace` keeps the first mapping, so this only creates bogus entries for identities that had no
   biometria of their own. In this unit the bogus entries are never used: 11411 applies a position only when
   the voter's own `biometria` field is present, and such a voter's identities were already mapped to its
   own offset. The bug would only matter if the map were reused elsewhere, for example for a lookup by CPF.
3. **Duplicate-identity check only catches adjacent duplicates** (`adjacent_find` on an unsorted list).
4. **Unknown TTE values are silently accepted as "no transfer"**. Only −1 throws. A future TTE type sent
   to an old urna is read as a regular voter.
5. **Names cut to 40 characters.** The ASN.1 schema allows 70, so long names are cut on screen and in
   reports.
6. **Truncating conversions.** `qtdBU` (0..999) is stored in 8 bits, `comparecimento`/`qtdJustificativa`
   (0..99999) in 16 bits, and the copies counters (0..999) in 8 bits each.
7. **`data2T` from gap.bin is ignored.** The md constructor recomputes it from today's date. On a machine
   with a wrong clock, the "2nd round" flag follows the clock.
8. **The processo eleitoral id is read from the IDEleitoral CHOICE without checking the alternative.**
9. **Duplicate cargo in an RDV is silently ignored** on read (`map::insert`). A duplicate eleição is also
   silently ignored, and together with the count-only check (7968) a configured eleição can be missing from the result (§7).
10. **Heavy copies**: `GetInscricoesEleitor` and `ConverteNecessidadeEspecial` copy the whole
    `EleitorSequencia`/`EleitorUrna` per voter; each fingerprint template is unpacked twice; the Seguranca
    record is converted twice; maps are copied into by-value constructors.
11. **Bit-width computation** `ceil(log2(max-min))` is one bit short when `max-min` is a power of two, and
    gives 0 for a range of 1. It is unused on this path, but wrong if reused for re-packing.
12. **Dead temporary** in `CConversorBiometriaEleitor::DoDesconverte`: an empty `SEQUENCE OF Dedo` is
    built and destroyed.

No network access, no `emscripten_sleep`, no JS interaction and no unbounded loops were found in this unit.
Every loop is bounded by container sizes, and every index is checked with `at()`.

---

## 12. Error codes raised by this unit

`CBaseError<comum::EUeComumDadosError, SErrorLimits{7800, 8600}>` unless noted. The line numbers are the
original ones.

| code | message | where |
|---|---|---|
| 7801 | Versão de pacote não encontrada para o cargo {} | ccandidaturas.cpp:89 |
| 7805 | {} - não posicionado no candidato corretamente | ccandidaturas.cpp:261 |
| 7806 / 7807 | O cargo não foi posicionado corretamente para {} | ccargods.cpp:25 / :41 |
| 7808 / 7809 / 7810 | O candidato… / O cargo… / Cargo e candidato não foram posicionados corretamente | ccargods.cpp:65/72/78 |
| 7811 / 7812 / 7813 | O cargo não foi posicionado corretamente | ccargods.cpp:98/113/128 |
| 7883–7886 | foto sequencer / "Erro ao carregar codigo do candidato." / "Entidade visitada invalida " | cvisitantefoto.cpp:40/48/56/63 |
| 7887–7890 | Local: CHOICE / tipo indefinido / não válida / não estritamente válida | cconversorlocal.cpp:70/120/124/128 |
| 7893 | Não tem nem foto nem dedos, mas falou que tem biometria. | cconversorbiometriaeleitor.cpp:26 |
| 7894 / 7895 | O arquivo … não existe / está vazio | cconversorbiometriaeleitorcifrada.cpp:91/109 |
| 7896 / 7897 | Tipo inválido: {} / Campo qtdMinucias não confere… | cconversordedo.cpp:159/169 |
| 7898–7902 | necessidade / inscrição não encontrada / identidade duplicada / transferência / sem domicílio | cconversorentidadeeleitores.cpp:38/49/76/210/236 |
| 7903 / 7904 | Tipo desconhecido de identidade / Tipo de impedimento inválido: {} | cconversorimpedido.cpp:40/109 |
| 7905 | Entidade visitada inválida: … | cvisitanteeleitor.cpp:91 |
| 7906–7908 | TipoAjusteDataHora / ETipoDeltaT | cconversorajustedatahora.cpp:49/53/69 |
| 7909, 7911–7913, 7915–7916 | EUrnaTipo / TipoUrnaOperacao / EUrnaModelo / ModeloUrna | cconversordadocarga.cpp:61/84/88/105/127/131 |
| 7917–7920 | estado da urna | cconversorestadogeral.cpp:87/91/106/110 |
| 7921–7924 | UrnaAplicativo / EUrnaAplicativo | cconversorestadogeralgap.cpp:150/154/194/198 |
| 7925–7928 | EEstadoSA / EstadoSA | cconversorestadogeralsa.cpp:96/100/156/160 |
| 7929–7937 | EEstadoVota / EstadoVota ("Estado não deve ser usado") / (E)EstadoEncerramento | cconversorestadogeralvota.cpp:152…252 |
| 7944 | Quantidade inválida de UFs [{}] | cconversorcomplementosmunicipios.cpp:29 |
| 7948 / 7949 | Opção de Detalhe Inválida. / Tipo inválido: {} | cconversorcargo.cpp:69/83 |
| 7950–7952 | suplências | cconversordetalhecandidato.cpp:29/34/39 |
| 7953 | Origem configuração inválida | cconversororigemconfiguracao.cpp:38 |
| 7965–7970 | Eleições incompatíveis / Eleição … não encontrada / Dado não é do Vota / Cargo {} não encontrado para eleição {} | cconversoreleicoesvota.cpp:34/46/69/74/94/104 |
| 7973–7975 | Valor inválido para tipo voto eleitor: {} | cconversorvoto.cpp:44/71/75 |
| 7976–7978 | Cargo info incompatível / Cargo {} difere… / Quantidade de escolhas difere… | cconversorvotoscargo.cpp:28/50/55 |
| 8183 / 8184 / 8185 | Tipo de identificador de eleitor inválido / Identidade principal ({}) ausente… | cconversortipoidentificadoreleitor.cpp:32/51, cconversorprocessoeleitoral.cpp:50 |
| inlined md | 8002 DataJE, 8061/8062 CDedo, 8070 CTransferenciaTemporaria, 8115/8116 CComplementosMunicipiosUF | md/*.cpp |
| other enums | EUeComumAsnError 7653/7654 (iconversorasn.h), 7689/7690 (util.cpp Turno); EApiAsnError 1900/1902 (ecourna iconversorasn.hpp); EUeIoError 5953/5954 (cfileasn.h); ESecurityError 1501–1503 (ccipheredin.cpp); EUeRdvError 4671 (cvotoscargos.cpp:91) | |

---

## 13. Mapping table (all 89 functions of u03)

"ran" = observed executing in `analysis/runtime/vote_*.functions.tsv`. Paths in the 5th column are relative
to `src/uenux2/src/app/comum/dados/`.

| func | size | ran | reconstructed symbol | reconstructed in | notes |
|---:|---:|:-:|---|---|---|
| 1135 | 175 |  | `std::vector<comum::md::CSuplencia>::__base_destruct_at_end` | `asn/processoeleitoral/cconversordetalhecandidato.cpp` | library/inlined helper (no source of its own) |
| 2268 | 1189 | ✔ | `comum::CCargoDSNomeSexoCandidato::operator()` | `ccargods.cpp` |  |
| 2341 | 242 | ✔ | `comum::md::CSuplencia::CSuplencia(const CSuplencia&)` | `asn/processoeleitoral/cconversordetalhecandidato.cpp` | out-of-line copy emitted in this TU; real home `uenux2/src/app/comum/dados/md/processoeleitoral/csuplencia.h` |
| 2838 | 599 |  | `comum::GetCandidaturaAtual` | `ccandidaturas.cpp` |  |
| 3018 | 231 |  | `std::vector<comum::md::CResposta>::__init_with_size` | `asn/processoeleitoral/cconversorcargo.cpp` | library/inlined helper (no source of its own) |
| 3728 | 220 |  | `std::map<comum::md::CEleitorIdentidade, std::pair<size_t,size_t>>::__emplace_unique_key_args` | `asn/eleitor/cvisitanteeleitor.cpp` | library/inlined helper (no source of its own) |
| 3786 | 50 |  | `comum::CCandidaturas::~CCandidaturas` | `ccandidaturas.cpp` |  |
| 3802 | 199 | ✔ | `comum::asn::Utils::ConverteDataJE` | `asn/estadoaplicacao/cconversorestadogeralgap.cpp` | out-of-line copy emitted in this TU; real home `uenux2/src/app/comum/asn/util.cpp` |
| 3869 | 466 | ✔ | `std::map<TEleicaoID, comum::md::CVotosCargos>::__find_equal (hint)` | `asn/rdv/cconversoreleicoesvota.cpp` | library/inlined helper (no source of its own) |
| 5171 | 221 |  | `ecourna::api::cepesc::CCepescCipher::Decifra` | `asn/eleitor/cconversorbiometriaeleitorcifrada.cpp` | out-of-line copy emitted in this TU; real home `ecourna-lib/ecourna/api/security/cepesc/ccepesccipher.cpp` |
| 5493 | 46 | ✔ | `std::vector<unsigned char>::__vdeallocate` | `asn/eleitor/cconversordedo.cpp` | library/inlined helper (no source of its own) |
| 5494 | 80 | ✔ | `std::vector<unsigned char>::__construct_at_end(first,last)` | `asn/eleitor/cconversordedo.cpp` | library/inlined helper (no source of its own) |
| 5496 | 159 | ✔ | `std::vector<unsigned char>::__assign_with_size` | `asn/eleitor/cconversordedo.cpp` | library/inlined helper (no source of its own) |
| 5625 | 37 |  | `comum::md::estadoaplicacao::CEstadoGeralSA::CEstadoGeralSA` | `asn/estadoaplicacao/cconversorestadogeralsa.cpp` | out-of-line copy emitted in this TU; real home `uenux2/src/app/comum/dados/md/estadoaplicacao/cestadogeralsa.cpp` |
| 5626 | 267 | ✔ | `comum::md::estadoaplicacao::CEstadoGeralGap::CEstadoGeralGap` | `asn/estadoaplicacao/cconversorestadogeralgap.cpp` | out-of-line copy emitted in this TU; real home `uenux2/src/app/comum/dados/md/estadoaplicacao/cestadogeralgap.cpp` |
| 5628 | 151 |  | `comum::md::estadoaplicacao::CEstadoGeralVota::CEstadoGeralVota` | `asn/estadoaplicacao/cconversorestadogeralvota.cpp` | out-of-line copy emitted in this TU; real home `uenux2/src/app/comum/dados/md/estadoaplicacao/cestadogeralvota.cpp` |
| 5637 | 222 | ✔ | `comum::md::estadoaplicacao::CEstadoGeral::CEstadoGeral` | `asn/estadoaplicacao/cconversorestadogeral.cpp` | out-of-line copy emitted in this TU; real home `uenux2/src/app/comum/dados/md/estadoaplicacao/cestadogeral.cpp` |
| 5659 | 65 | ✔ | `std::pair<std::string, std::pair<size_t,size_t>>::pair (CVisitanteFoto index entry)` | `asn/candidatura/cvisitantefoto.cpp` | library/inlined helper (no source of its own) |
| 5661 | 104 |  | `comum::md::CImpedido::CImpedido(secao, identidade, p1, p2)` | `asn/eleitor/cconversorimpedido.cpp` | out-of-line copy emitted in this TU; real home `uenux2/src/app/comum/dados/md/eleitor/cimpedido.h` |
| 5662 | 104 |  | `comum::md::CImpedido::CImpedido(secao, identidade, p1)` | `asn/eleitor/cconversorimpedido.cpp` | out-of-line copy emitted in this TU; real home `uenux2/src/app/comum/dados/md/eleitor/cimpedido.h` |
| 5666 | 477 | ✔ | `comum::md::CEleitor::CEleitor (without biometria)` | `asn/eleitor/cconversorentidadeeleitores.cpp` | out-of-line copy emitted in this TU; real home `uenux2/src/app/comum/dados/md/eleitor/celeitor.h` |
| 5668 | 583 |  | `comum::md::CEleitor::CEleitor (with biometria)` | `asn/eleitor/cconversorentidadeeleitores.cpp` | out-of-line copy emitted in this TU; real home `uenux2/src/app/comum/dados/md/eleitor/celeitor.h` |
| 5674 | 125 |  | `comum::md::CInfoMunicipio::CInfoMunicipio (with horário de verão)` | `asn/cconversorlocal.cpp` | out-of-line copy emitted in this TU; real home `uenux2/src/app/comum/dados/md/cinfomunicipio.h` |
| 5676 | 102 | ✔ | `comum::md::CInfoMunicipio::CInfoMunicipio (without horário de verão)` | `asn/cconversorlocal.cpp` | out-of-line copy emitted in this TU; real home `uenux2/src/app/comum/dados/md/cinfomunicipio.h` |
| 5683 | 44 |  | `comum::asn::CConversorVotosCargo::CConversorVotosCargo` | `asn/rdv/cconversorvotoscargo.h` |  |
| 5689 | 997 |  | `comum::asn::CConversorEstadoGeralGap::ConverteUrnaAplicativo` | `asn/estadoaplicacao/cconversorestadogeralgap.cpp` |  |
| 5690 | 971 |  | `comum::asn::CConversorEstadoGeralGap::DesconverteUrnaAplicativo` | `asn/estadoaplicacao/cconversorestadogeralgap.cpp` |  |
| 5693 | 502 |  | `comum::asn::CConversorDadoCarga::ConverteTipoUrna` | `asn/estadoaplicacao/cconversordadocarga.cpp` |  |
| 5694 | 1005 |  | `comum::asn::CConversorDadoCarga::DesconverteTipoUrna` | `asn/estadoaplicacao/cconversordadocarga.cpp` |  |
| 5698 | 102 | ✔ | `comum::asn::CVisitanteEleitor::~CVisitanteEleitor` | `asn/eleitor/cvisitanteeleitor.h` |  |
| 5699 | 483 |  | `comum::asn::CConversorImpedido::DesconverteTipoImpedimento` | `asn/eleitor/cconversorimpedido.cpp` |  |
| 5700 | 510 |  | `std::vector<comum::md::CEleitor>::__swap_out_circular_buffer` | `asn/eleitor/cconversorentidadeeleitores.cpp` | library/inlined helper (no source of its own) |
| 5702 | 2565 |  | `comum::asn::(anonymous namespace)::DescompactaMinucias` | `asn/eleitor/cconversordedo.cpp` |  |
| 5709 | 116 |  | `comum::asn::CVisitanteFoto::~CVisitanteFoto` | `asn/candidatura/cvisitantefoto.h` |  |
| 5801 | 253 |  | `comum::CCargoDSLabelRelatorio::TotalVotoNominal` | `ccargods.cpp` |  |
| 5802 | 55 |  | `comum::CCargoDSLabelRelatorio::HeaderDetalhe` | `ccargods.cpp` |  |
| 5803 | 554 | ✔ | `comum::(anonymous namespace)::GetNomeCargoComGenero` | `ccargods.cpp` |  |
| 5810 | 592 |  | `comum::CCandidaturas::RecuperaVersaoPacote` | `ccandidaturas.cpp` |  |
| 5811 | 107 |  | `comum::CCandidaturas::CCandidaturas` | `ccandidaturas.h` |  |
| 6038 | 229 |  | `wasm-opt merged body of CCargoDSLabelRelatorio::HeaderDetalhe/HeaderDetalheZE` | `ccargods.cpp` | wasm-opt merge-similar-functions body (no source equivalent) |
| 6075 | 37 | ✔ | `wasm-opt merged body: CCargoDSNomeSexoCandidato{packed}() text source` | `ccargods.cpp` | wasm-opt merge-similar-functions body (no source equivalent) |
| 6076 | 255 | ✔ | `wasm-opt merged body: cargo name + ': ' + CCandidaturasDSNome text source` | `ccargods.cpp` | wasm-opt merge-similar-functions body (no source equivalent) |
| 6664 | 10 | ✔ | `vota::(anonymous namespace)::<lambda: nome do cargo do 1o suplente>` | `ccargods.cpp` | captureless lambda; real home `uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp` (inferred) |
| 6669 | 12 | ✔ | `vota::(anonymous namespace)::<lambda: cargo e nome do 1o suplente>` | `ccargods.cpp` | captureless lambda; real home `uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp` (inferred) |
| 11247 | 904 |  | `api::CDataText<comum::CPadDS<comum::CToUpperDS<comum::CCargoDSNome>>>::GetText` | `ccargods.cpp` | template instantiation (`uenux2/src/api/gui/ctextsource.h` + `ccargods.h`) |
| 11347 | 20 |  | `comum::asn::CConversorEleicoesVota::~CConversorEleicoesVota (deleting)` | `asn/rdv/cconversoreleicoesvota.h` |  |
| 11348 | 17 |  | `comum::asn::CConversorEleicoesVota::~CConversorEleicoesVota` | `asn/rdv/cconversoreleicoesvota.h` |  |
| 11349 | 5054 |  | `comum::asn::CConversorEleicoesVota::DoDesconverte` | `asn/rdv/cconversoreleicoesvota.cpp` |  |
| 11350 | 1911 | ✔ | `comum::asn::CConversorEleicoesVota::DoConverte` | `asn/rdv/cconversoreleicoesvota.cpp` |  |
| 11351 | 2825 |  | `comum::asn::CConversorVotosCargo::DoDesconverte` | `asn/rdv/cconversorvotoscargo.cpp` |  |
| 11352 | 980 | ✔ | `comum::asn::CConversorVotosCargo::DoConverte` | `asn/rdv/cconversorvotoscargo.cpp` |  |
| 11354 | 1175 |  | `comum::asn::CConversorVoto::DoDesconverte` | `asn/rdv/cconversorvoto.cpp` |  |
| 11355 | 745 |  | `comum::asn::CConversorVoto::DoConverte` | `asn/rdv/cconversorvoto.cpp` |  |
| 11356 | 23 |  | `comum::asn::CConversorTipoIdentificadorEleitor::DoDesconverte` | `asn/processoeleitoral/cconversortipoidentificadoreleitor.cpp` |  |
| 11357 | 100 |  | `comum::asn::CConversorTipoIdentificadorEleitor::DoConverte` | `asn/processoeleitoral/cconversortipoidentificadoreleitor.cpp` |  |
| 11363 | 2456 | ✔ | `comum::asn::CConversorProcessoEleitoral::DoDesconverte` | `asn/processoeleitoral/cconversorprocessoeleitoral.cpp` |  |
| 11366 | 23 |  | `comum::asn::CConversorOrigemConfiguracao::DoDesconverte` | `asn/processoeleitoral/cconversororigemconfiguracao.cpp` |  |
| 11367 | 44 |  | `comum::asn::CConversorOrigemConfiguracao::DoConverte` | `asn/processoeleitoral/cconversororigemconfiguracao.cpp` |  |
| 11373 | 2358 | ✔ | `comum::asn::CConversorCargo::DoDesconverte` | `asn/processoeleitoral/cconversorcargo.cpp` |  |
| 11375 | 2626 | ✔ | `comum::asn::CConversorDetalheCandidato::DoDesconverte` | `asn/processoeleitoral/cconversordetalhecandidato.cpp` |  |
| 11381 | 1912 |  | `comum::asn::CConversorComplementosMunicipios::DoDesconverte` | `asn/municipiozona/cconversorcomplementosmunicipios.cpp` |  |
| 11385 | 391 |  | `comum::asn::CConversorHorarioVerao::DoConverte` | `asn/municipiozona/cconversorhorarioverao.cpp` |  |
| 11390 | 3335 | ✔ | `comum::asn::CConversorEstadoGeralVota::DoConverte` | `asn/estadoaplicacao/cconversorestadogeralvota.cpp` |  |
| 11391 | 3356 |  | `comum::asn::CConversorEstadoGeralVota::DoDesconverte` | `asn/estadoaplicacao/cconversorestadogeralvota.cpp` |  |
| 11392 | 1362 |  | `comum::asn::CConversorEstadoGeralSA::DoConverte` | `asn/estadoaplicacao/cconversorestadogeralsa.cpp` |  |
| 11393 | 1254 |  | `comum::asn::CConversorEstadoGeralSA::DoDesconverte` | `asn/estadoaplicacao/cconversorestadogeralsa.cpp` |  |
| 11394 | 1109 | ✔ | `comum::asn::CConversorEstadoGeralGap::DoConverte` | `asn/estadoaplicacao/cconversorestadogeralgap.cpp` |  |
| 11395 | 1421 | ✔ | `comum::asn::CConversorEstadoGeralGap::DoDesconverte` | `asn/estadoaplicacao/cconversorestadogeralgap.cpp` |  |
| 11397 | 3182 | ✔ | `comum::asn::CConversorEstadoGeral::DoConverte` | `asn/estadoaplicacao/cconversorestadogeral.cpp` |  |
| 11398 | 3672 | ✔ | `comum::asn::CConversorEstadoGeral::DoDesconverte` | `asn/estadoaplicacao/cconversorestadogeral.cpp` |  |
| 11401 | 1227 | ✔ | `comum::asn::CConversorDadoCarga::DoConverte` | `asn/estadoaplicacao/cconversordadocarga.cpp` |  |
| 11402 | 1603 | ✔ | `comum::asn::CConversorDadoCarga::DoDesconverte` | `asn/estadoaplicacao/cconversordadocarga.cpp` |  |
| 11405 | 594 |  | `comum::asn::CConversorAjusteDataHora::DoConverte` | `asn/estadoaplicacao/cconversorajustedatahora.cpp` |  |
| 11406 | 999 |  | `comum::asn::CConversorAjusteDataHora::DoDesconverte` | `asn/estadoaplicacao/cconversorajustedatahora.cpp` |  |
| 11407 | 1313 |  | `comum::asn::CVisitanteEleitor::visit` | `asn/eleitor/cvisitanteeleitor.cpp` |  |
| 11408 | 13 | ✔ | `comum::asn::CVisitanteEleitor::~CVisitanteEleitor (deleting)` | `asn/eleitor/cvisitanteeleitor.h` |  |
| 11410 | 1444 | ✔ | `comum::asn::CConversorImpedido::DoDesconverte` | `asn/eleitor/cconversorimpedido.cpp` |  |
| 11411 | 7299 | ✔ | `comum::asn::CConversorEntidadeEleitores::DoDesconverte` | `asn/eleitor/cconversorentidadeeleitores.cpp` |  |
| 11416 | 1778 |  | `comum::asn::CConversorDedo::DoDesconverte` | `asn/eleitor/cconversordedo.cpp` |  |
| 11419 | 5935 |  | `comum::asn::CConversorBiometriaEleitorCifrada::DoDesconverte` | `asn/eleitor/cconversorbiometriaeleitorcifrada.cpp` |  |
| 11422 | 2685 |  | `comum::asn::CConversorBiometriaEleitor::DoDesconverte` | `asn/eleitor/cconversorbiometriaeleitor.cpp` |  |
| 11442 | 1198 | ✔ | `comum::asn::CVisitanteFoto::visit` | `asn/candidatura/cvisitantefoto.cpp` |  |
| 11443 | 13 |  | `comum::asn::CVisitanteFoto::~CVisitanteFoto (deleting)` | `asn/candidatura/cvisitantefoto.h` |  |
| 11451 | 1097 |  | `comum::asn::CConversorLocal::DoConverte` | `asn/cconversorlocal.cpp` |  |
| 11452 | 1671 | ✔ | `comum::asn::CConversorLocal::DoDesconverte` | `asn/cconversorlocal.cpp` |  |
| 11555 | 55 |  | `comum::CCargoDSLabelRelatorio::HeaderDetalheZE` | `ccargods.cpp` |  |
| 12632 | 12 |  | `api::CDataText<comum::CCargoDSNomeSexoCandidato>::GetText` | `ccargods.cpp` | template instantiation (`uenux2/src/api/gui/ctextsource.h` + `ccargods.h`) |
| 13298 | 10 |  | `vota::(anonymous namespace)::<lambda: nome do cargo do 2o suplente>` | `ccargods.cpp` | captureless lambda; real home `uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp` (inferred) |
| 13301 | 12 |  | `vota::(anonymous namespace)::<lambda: cargo e nome do 2o suplente>` | `ccargods.cpp` | captureless lambda; real home `uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp` (inferred) |

---

## 14. Open questions

* The name and meaning of the −1 enumerator handled in every ASN→md switch (§10).
* The names of the md enumerators (`EEstadoVota`, `ETransferenciaTemporaria`, `ETipoDeltaT`…) and the
  semantics of md TTE value 1. That value groups em trânsito, servidor em serviço, eleitor convocado and
  justiça eleitoral.
* The exact meaning of `EVisitorResult` 0/1 (keep vs. free is inferred from the doc of the partial
  decoder), and of the word at +4 of `CVisitanteEleitor`, which the partial converter sets to 1.
* What `api::IKernelHSM` slot 3 returns on a real urna (the key that protects `bio.sk1`), and which cipher
  `CSymmetricCipherFactory` builds from it.
* The layout of `CPadDS`/`CToUpperDS` between +24 and +48 in func 11247.
* Whether the four suplente text sources are lambdas in `ctelasvota.cpp` (most likely) or small functions
  in a comum header.
