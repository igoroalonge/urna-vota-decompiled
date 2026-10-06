# u14 — `ecourna-lib/ecourna/app/dados`: the ecourna data model and its ASN.1 converters

Unit u14 has **136 functions**. Six of them were seen executing in the recorded votes (§12). The unit
covers 35 original files of the TSE library **ecourna** (built through Conan, path
`/home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/app/dados/…`) plus a few classes whose files
had to be inferred (§2). *Dados* means "data": this directory holds the library's **data classes**
(voter identifiers, media descriptions, urna parameters, municipal schedules, party federations, the
attendance result) and, under `asn/`, the **converters** between these classes and the ASN.1 objects
of the TSE file formats.

Its place in the voting process:

* **At start-up** VOTA reads the urna parameters (`…-pu.dat`, *parametrização da urna*: how many BU
  copies to print, suspension rules, time-outs, report headers and footers, labels), the municipal
  schedule (`…-cfm.dat`: zerésima, start and end of voting), party federations (`…-fe.dat`) and the
  candidate photos. Each file goes through one of these converters.
* **During voting** the voter identifiers (título de eleitor, CPF, free number) and the cargo codes
  are converted when the voter state and the RDV are written.
* **At the end of voting** (*encerramento*) the result writers read the medium description
  (`infomidia.dat`), and `comum::CGravadorRCSecao` writes the section's **attendance result**
  (*resultado urna cadastro*: who voted, how each voter was enabled, which mesários were present),
  optionally encrypted (parameter `criptografarJUFA`). The web build never reaches this step (§10).

The reconstructed sources are in `src/ecourna/app/dados/` (one `.cpp` per original file; headers
group small declarations). The complete function map is §14.

---

## 1. Glossary

| term | meaning |
|---|---|
| *converter* (`CConversorX`) | class derived from `ecourna::api::asn::IConversorASN<ENTIDADE, DADO>`; `Converte` = data class → ASN.1, `Deconverte` = ASN.1 → data class |
| *entidade* | the ASN.1 object (III ASN.1 runtime, `docs/libraries/asn1-runtime.md`); *dado* = the C++ data class |
| *título de eleitor / número de inscrição eleitoral* | the voter's registration number, 12 digits; printed `NNNN NNNN NNNN` |
| *identificação livre* | a free 12-digit identification number, the third kind of voter identifier |
| *habilitação* | enabling a voter to vote at the mesário's terminal: by fingerprint (*biométrica*), by release code (*por código*), with audio guidance (*áudio*) |
| *mesário* | poll worker; *registrar mesários* records which mesários were present at *abertura* (opening) and *encerramento* (closing) |
| *comparecimento* | attendance (voted / did not vote / absent); *justificativa* = justification of absence |
| *mídia* | removable flash medium: **MR** *mídia de resultado* (result medium, taken to the Junta), **FC** *flash de carga* (load flash), **FV** *flash de votação* |
| *parametrização da urna (PU)* | the urna parameters file `…-pu.dat` |
| *zerésima (ZE)* | the pre-voting zero report; *BU* = *boletim de urna*, the section's tally |
| *RED / SA* | *recuperação de dados* / *sistema de apuração* (manual counting system), other programs that print BUs |
| *JUFA* | the justification/attendance data (key `jufa.pk1`, parameter `criptografarJUFA`) |
| *CEPESC* | the government crypto library used to encrypt it |
| *federação partidária* | party federation (several parties acting as one) |

---

## 2. Files, classes and data

| original file | class(es) | ASN.1 type ↔ data class | data file |
|---|---|---|---|
| `tiposbasicos.cpp` | `CErroLeituraBiometria` (+ the named `CBaseType`s) | — | — |
| `iidentificadoreleitor.cpp` | `IIdentificadorEleitor` (abstract) | — | — |
| `cnumeroinscricaoeleitoral.cpp`, `cnumerocpf.cpp`, `cnumeroidentificacaolivre.cpp` | the three identifiers | — | — |
| `cregistroidentificacaoeleitor.cpp` | `CRegistroIdentificacaoEleitor` | `ModuloTiposEcoUrna::RegistroIdentificacaoEleitor` (converter in u40) | voter / mesário records |
| `cserialmidia.cpp` | `CSerialMidia` | `serialMidia` | `infomidia.dat` |
| `federacoes/cfederacao.cpp` | `CFederacao` | `ModuloFederacoes::Federacao` | `…-fe.dat` |
| `midias/caplicativo.cpp`, `cautenticacao.cpp`, `cinformacaomidia.cpp` | `CAplicativo`, `CAutenticacao`, `CInformacaoMidia` (+ `CDadosGeracaoMidia`, `CIdentificadorGeradorMidia` declared with them) | `ModuloInformacaoMidia::*` | `infomidia.dat` |
| `processoeleitoral/cconfiguracaomunicipios.cpp` | `CConfiguracaoMunicipios` | `EntidadeConfiguracaoMunicipios` | `…-cfm.dat` |
| `parametrizacaourna/cparametrosurna.cpp` *(path inferred)* | `CParametrosUrna` (+ `CTituloRelatorio`, `CLabelParametrizado`) | `ModuloParametrizacaoUrna::ParametrosUrna` | `…-pu.dat` |
| `resultadournacadastro/*.cpp` (6 files) + `ccomparecimentomesario.cpp` *(path inferred)* | `CResultadoUrnaCadastro`, `CDadosComparecimento`, `CEstadoComparecimento`, `CHabilitacaoBiometrica`, `CEstadoHabilitacaoPorCodigo`, `CDadosCifracao`, `CComparecimentoMesario` | `ModuloResultadoUrnaCadastro::*` | attendance (RC) file written at encerramento |
| `asn/cconversorturno.cpp`, `cconversorfase.cpp`, `cconversorcargoid.cpp`, `cconversorfoto.cpp`, `cconversorcabecalhoentidade.cpp`, `cconversoridentificadoreleitor.cpp` | basic-type converters | `Turno`, `Fase`, `CodigoCargoConsulta`, `Foto`, `CabecalhoEntidade`, `IdentificadorEleitor` | all ecourna files |
| `asn/federacoes/cconversorfederacao.cpp` *(path inferred)* | `CConversorFederacao` (DoConverte) | `Federacao` | `…-fe.dat` |
| `asn/midias/*.cpp` | `CConversorAplicativo`, `CConversorInformacaoMidia` | `Aplicativo`, `InformacaoMidia` | `infomidia.dat` |
| `asn/parametrizacaourna/*.cpp` | `CConversorParametrosUrna`, `CConversorTituloRelatorio`, `CConversorLabelParametrizado` | `ParametrosUrna`, `TituloRelatorio`, `LabelParametrizado` | `…-pu.dat` |
| `asn/resultadournacadastro/*.cpp` | 6 converters | `EntidadeResultadoUrnaCadastro`, `EstadoComparecimento`, `HabilitacaoBiometrica`, `EstadoHabilitacaoPorCodigo`, `ComparecimentoMesario`, `ApresentacaoFotoEleitor` | RC file |

Reconstructed files: `src/ecourna/app/dados/` — `tiposbasicos.{h,cpp}`, `dadoserros.h` (error families,
inferred), `iidentificadoreleitor.{h,cpp}`, `cnumeroinscricaoeleitoral.{h,cpp}`, `cnumerocpf.cpp`,
`cnumeroidentificacaolivre.cpp`, `cregistroidentificacaoeleitor.{h,cpp}`, `cserialmidia.{h,cpp}`,
`federacoes/cfederacao.{h,cpp}`, `midias/{cinformacaomidia.h, caplicativo.cpp, cautenticacao.cpp,
cinformacaomidia.cpp}`, `processoeleitoral/cconfiguracaomunicipios.{h,cpp}`,
`parametrizacaourna/cparametrosurna.{h,cpp}`, `resultadournacadastro/{cdadoscifracao,
cdadoscomparecimento, cestadocomparecimento, cestadohabilitacaoporcodigo, chabilitacaobiometrica,
cresultadournacadastro}.{h,cpp}` + `ccomparecimentomesario.cpp`, `asn/cconversores.h` (all converter
declarations), and one `.cpp` per converter file under `asn/`.

---

## 3. Class hierarchy (RTTI) and the converter protocol

```
ecourna::api::asn::IConversorASN<ENTIDADE, DADO>        (ecourna/api/asn/iconversorasn.hpp, unit u11)
 ├─ asn::CConversorTurno          <Turno, CBaseType<ushort,1,2,11>>
 ├─ asn::CConversorFase           <Fase, CFaseID>
 ├─ asn::CConversorCargoID        <CodigoCargoConsulta, CBaseType<ushort,1,99,3>>
 ├─ asn::CConversorFoto           <Foto, CFoto>
 ├─ asn::CConversorCabecalhoEntidade <CabecalhoEntidade, CCabecalhoEntidade>
 ├─ asn::CConversorIdentificadorEleitor <IdentificadorEleitor, CIdentificadorEleitor>
 ├─ asn::CConversorFederacao, CConversorAplicativo, CConversorInformacaoMidia,
 │  CConversorTituloRelatorio, CConversorLabelParametrizado, CConversorParametrosUrna,
 │  CConversorApresentacaoFotoEleitor, CConversorComparecimentoMesario,
 │  CConversorEstadoComparecimento, CConversorEstadoHabilitacaoPorCodigo,
 │  CConversorHabilitacaoBiometrica, CConversorResultadoUrnaCadastro   (all "si")
 └─ (siblings in units u40/u11: CConversorAutenticacao, CConversorDadosGeracaoMidia,
    CConversorFederacoes, CConversorParametrizacaoUrna, CConversorConfiguracaoMunicipio(s),
    CConversorHorariosUrna, CConversorRegistroIdentificacaoEleitor, CConversorDadosComparecimento,
    CConversorComparecimentoSecao, CConversorIdentificacaoJustificativa, CConversorDadosCifracao, …)
ecourna::app::dados::IIdentificadorEleitor               (vtable @1122204, slot 2 pure virtual)
 ├─ CNumeroInscricaoEleitoral   (@1122028)
 ├─ CNumeroCPF                  (@1121848)
 └─ CNumeroIdentificacaoLivre   (@1121932)
ecourna::api::exception::CError
 └─ CBaseError<E, SErrorLimits{lo,hi}>  for EDadosError {1935,2135}, EDadosFederacoesError {2940,2965},
    EDadosMidiasError {3040,3065}, EDadosProcessoEleitoralError {3140,3190},
    EDadosResultadoUrnaCadastroError {3265,3315}, asn::EAsnError {2235,2335},
    asn::EAsnMidiasError {2485,2510}, asn::EAsnParametrizacaoUrnaError {2560,2585},
    asn::EAsnResultadoUrnaCadastroError {2635,2665}
```

**ecourna `IConversorASN` vtable** (4 slots, like the uenux2 `comum::asn` one described in u03):
`[0]` destructor (ICF `return this`, func 174), `[1]` deleting destructor (func 144), `[2]`
`DoConverte(const TDado&)` → `TEntidade`, `[3]` `DoDeconverte(const TEntidade&)` → `TDado`.
The public wrappers are non-virtual and inlined into their callers: `Deconverte` first requires
`isValid() && isStrictlyValid()` and otherwise throws `CBaseError<EApiAsnError>(1902, typeid(E).name() +
": " + trace_invalid(...))` (srcloc `iconversorasn.hpp:66`); `Converte` validates the result
(`iconversorasn.hpp:49`). This is why the converters themselves never check ASN.1 ranges (`numZeresimas
1..50`, `tempoDispararSuspensao 15..360`, …): the III runtime does, before `DoDeconverte` runs.

**Naming trap.** Like in u03, many slot functions were named by the tool after an inlined static
helper that carries the `source_location` (func 9156 "`DesconverterFormaSuspenderComVoto`" is
`CConversorParametrosUrna::DoDeconverte`; 9145, 9167, 9179, 9196, 9087, 9226, 9227 likewise). The
vtable slot decides; §14 gives the corrected names. Slots shown as `vf2`/`vf3` are `DoConverte`/`DoDeconverte`.

### 3.1 How enumerations are converted

The data classes use their own C++ enums. Four conventions appear, and wasm-opt merged the helpers
that differ only in constants into shared bodies:

| convention | shared body | used by |
|---|---|---|
| C++ `0..n-1` → ASN `v+1` (Converte) | func 2306 (`c >= 3` → throw) | `ConverteTipoMidia`, `ConverterFormaSuspender{Com,Sem}Voto`, `ConverteSituacaoReconhecimentoMesario`, `ConverteSituacaoHabilitacaoAudio` |
| same, message from a static `std::string` | func 6148 | `ConverterAlinhamentoTitulo`, `ConverterGeneroLabel` |
| identity with range `[1, k]` | func 6151 | `ConverteTipoAplicativo` (k=7), `ConverteEstadoColetaDigital` (k=4) |
| ASN `v` → C++ `v-1`, range 3 | func 6144 | `DeconverteSituacaoReconhecimentoMesario`, `DeconverteSituacaoHabilitacaoAudio` |
| lookup table (non-monotonic) | inline | `SituacaoComparecimentoEleitor` ↔ `ESituacaoComparecimento`: tables @1135428 `{1,4,2,3}` and @1135444 `{0,2,3,1}` |

Several `Desconverte` helpers are a `switch` with an explicit `case -1:` that throws from a different
source line than the default (e.g. `CConversorFase` lines 42/44, `CConversorAplicativo` 88/92). The
same pattern exists in uenux2 (u03). It is probably an extra "invalid" enumerator of the generated
`NamedNumber`; its name is not in the binary.

### 3.2 Error families and codes

All messages of this unit, with codes (the enumerator names are unknown): see the reconstructed
sources. Code ranges used: EDadosError 1951, 2014, 2023–2030; EAsnError 2235–2268;
EAsnMidiasError 2485–2489; EAsnParametrizacaoUrnaError 2560–2577; EAsnResultadoUrnaCadastroError
2635–2660; EDadosFederacoesError 2940; EDadosMidiasError 3040–3044; EDadosProcessoEleitoralError
3145–3146; EDadosResultadoUrnaCadastroError 3267–3283. The constructors are 18-byte thunks
(9025, 9026, 9041, 9046, 9107, 9158, 9229, 9266) over the generic `CBaseError` body func 1011.

---

## 4. Voter identifiers

`IIdentificadorEleitor` (24 bytes: `+4 TTipoIdentificadorEleitor m_tipo`, `+8 std::string m_numero`,
`+20 uebyte m_tamanho`) is always handled through `std::shared_ptr` (`TSharedIdentificadorEleitor`).

Constructor rules (funcs 9256 and the merged body 3939):

1. `m_numero` = the number **left-padded with `'0'`** to the maximum size (func 753 copies the string and
   inserts the zeros at position 0). A longer number is kept as is.
2. The padded number must not be all zeros and must contain only `'0'..'9'`, else
   `EDadosError 2023 "O número de identificação de eleitor deve ser numérico ({})."` (line 28).
3. The derived class then rejects an input longer than its size: título 12 (2024), CPF 11 (2025),
   livre 12 (2026), message `"O tamanho do número de … não pode ser maior do que {} ({})."`.

These classes validate no check digit (neither the título's nor the CPF's). The check digits are verified
earlier, where the number is typed, by the VOTA input rules `comum::md::CRegraTitulo` / `comum::md::CRegraCPF`
(vf4, funcs 11272 / 11270: weighted sums `% 11`), so this is not a gap on the normal input path. The display form (vtable slot 2,
name inferred `GetNumeroFormatado`) inserts separators: título `1234 5678 9012`, CPF
`123.456.789-01`, livre unchanged.

`CRegistroIdentificacaoEleitor` (20 bytes) = *identificação utilizada* (the document used to enable the
voter; `m_identificadorHabilitacao`, required, codes 2029/2030/2028) + optional *identificação principal*
(code 2027). The two-argument constructor engages the optional even when the second pointer is empty.

`CConversorIdentificadorEleitor` maps the CHOICE alternative to the class: `numeroInscricao` →
`make_shared<CNumeroInscricaoEleitoral>`, `numeroCPF` → `CNumeroCPF`, `identificacaoLivre` →
`CNumeroIdentificacaoLivre`; an unselected CHOICE throws 2268. The inverse copies `GetNumero()` into
the selected alternative.

---

## 5. Media information (`infomidia.dat`)

`CInformacaoMidia` (116 bytes): `+0 ETipoMidia` (MR 0, FC 1, FV 2), `+4 fase`, `+8 idPE`
(`CBaseType<unsigned,0,99999>` "ProcessoEleitoralID"), `+12 uf`, `+24 turno`, `+32 CDadosGeracaoMidia`
(serial, user, generator id, date), `+104 vector<CAplicativo>`.

* Only an **MR** (result medium) carries `aplicativos` (the programs it may start — vota, sa, red, vpp,
  ste, adh, atue — each with an optional `CAutenticacao`: validity period, password size, number of
  attempts, password hash). `GetAplicativos()` on anything else throws 3044; the non-MR constructor
  rejects MR (3043).
* `CConversorInformacaoMidia::DoDeconverte` decides the type from the presence of `aplicativos`: if
  present, the object is built as **MR regardless of `tipoMidia`** (func 9033); if absent, with the
  file's `tipoMidia`.
* `CAutenticacao::GetDataHoraInicial/Final` both require **both** dates (codes 3041/3042).
* `CSerialMidia` accepts exactly 8 hex digits (`std::regex_match(s, std::regex("([A-Fa-f0-9]{8})"))`,
  func 9258, built on every call) and stores them as 4 bytes; otherwise 1951
  `"Número de série '{}' inválido."`.

Each of the 8 scenarios ships `estatico/infomidia-fv-1-t.dat` (1st-round scenarios) or
`infomidia-fv-2-t.dat` (2nd round), 172 bytes each, all different. Decoded (municipal-t1): `tipoMidia = 2 (fc)`,
`fase = 3 (treinamento)`, `idPE = 2400`, `uf = "ac"`, `turno = 1`, `dadosGeracaoMidia = {serialMidia
"A1B2C3DB", usuario "simulador-votacao-ng", identificadorGeradorMidia {nome "simulador-votacao-ng",
serialCertificadoTPM 64×'0', serialInstalacao "A1B2C3DA"}, data "20260910T185602"}`, no `aplicativos`.
The other files differ only in `idPE` (2400 municipal, 2500 geral), `uf` (ac, df, zz), `turno` (1/2) and the
generation time; all are FC media of phase treinamento with the same serial.
Readers: `api::CFileASN::ReadFromFile<InformacaoMidia>` (func 3735, the only function that builds this
converter) from `comum::CGravadorBU`, `CGravadorRDV` and `IGravadorEnvelope` (vf7, GravaResultado).
`comum::impl::CValidaMidia` (func 11172) only uses the names `infomidia.dat`/`infomidia.vsc` (signature
check); it does not decode the file through this unit.

---

## 6. Urna parameters (`…-pu.dat`)

`CParametrosUrna` (402 bytes) is embedded at **+88 of `comum::CConfiguracaoEleicao`**, which is why other
units see `CConfiguracaoEleicao+88/+92` (suspension rules), `+484` (apresentarPartido) and `+486`
(imprimirQrCodeNoBU). Field offsets are in `src/ecourna/app/dados/parametrizacaourna/cparametrosurna.h`.
Correction to u02: the vectors at +100… are **six `vector<CTituloRelatorio>`** (+100..+160), not three
`SRotulo` vectors.

`CConversorParametrosUrna::DoDeconverte` (func 9156, observed) converts the two suspension rules
(0-based enums; codes 2567/2568 and 2570/2571), copies the 24 scalar fields that follow (22 integers and the
two booleans `criptografarBU`/`criptografarJUFA`), converts the six title lists with
`CConversorParametrosUrna::DesconverterTitulos` (func 5102, observed), copies `telaEmissaoBU`, converts the four
labels, copies the last 10 booleans, and calls the 47-argument constructor (func 5093). `DoConverte` (func 9164)
is the mirror image (`ConverterTitulos`, func 9161). The two title helpers are const **member** functions (name
inferred): their wasm signature keeps an unused `this` that the callers fill with a junk value, exactly as for the
srcloc-named members `ConverterFormaSuspender{Sem,Com}Voto`; a file-local function would have lost the parameter.

Values in every simulator scenario. The 16 `-pu.dat` files (`t00000br-pu.dat` + the state file in each of the 8
scenarios) are 4 distinct files, one per scenario family, that differ only in the header (`dataGeracao`, pleito
2400/2500); the `ParametrosUrna` body is byte-identical in all 16:

| field | value | field | value |
|---|---|---|---|
| formaSuspensaoSemVoto | 1 tornaNaoVotou | formaSuspensaoComVoto | 2 tornaOutrosNulo |
| numZeresimas | 1 | numBUVotaObrigatorios / Adicionais | **5 / 5** |
| numBUFinalRED Obrigat/Adic | 2 / 5 | numBUParcialRED Obrigat/Adic | 2 / 3 |
| numBUFinalSA Obrigat/Adic | 2 / 5 | numBUParcialSAObrigat | 1 |
| numTentativasHabilitacao / Verificacao | 4 / 2 | numRelatorio Estado/Eleitores/VersoesDados/PU | 5/1/1/1 |
| criptografarBU | **FALSE** | criptografarJUFA | **TRUE** |
| numDigitosPartido | 2 | tempoDispararSuspensao / TE | 45 / 45 |
| tempoConfirmacaoVoto | 1000 | tempoDesligamentoAutomatico / Aviso | 1800 / 60 |
| imprimirZeradosBU / gravarZeradosBU | FALSE / FALSE | aceitarJustificativa, aceitarBrancoNulo, apresentarPartido, permitirHabManualAudio, imprimirQrCodeNoBU, exibirScoreBiometria, registrarMesarios, pedeAnoNascimentoEleitor | all TRUE |

Labels: `Município/Município/Municípios/Municípios` (masculino), `Zona Eleitoral/Zona/Zonas
Eleitorais/Zonas` (feminino), `Seção Eleitoral/Seção/Seções Eleitorais/Seções` (feminino),
`Partido/…` (masculino). Strings are ISO-8859-1 in the files.

---

## 7. Municipal schedule and federations

* `CConfiguracaoMunicipios(cabecalho, map<código, CConfiguracaoMunicipio>)` (func 9028, observed)
  rejects an empty map (3145) and any entry whose key differs from its `codigoMunicipio` (3146). In
  the scenarios every `-cfm.dat` (pleitos 2410/2420 municipal, 2510/2520 geral) has 3 municípios with the same
  schedule: 1st round (`t02410ac-cfm.dat` etc.) zerésima `20261004T070000`, start `…T080000`, end `…T170000`,
  término `20261005T040000`; 2nd round (`t02420ac-cfm.dat` etc.) the same times on 2026-10-25 (término
  `20261026T040000`).
* `CFederacao(id, sigla, nome, partidos)` rejects an empty party list (2940). The simulator's `-fe.dat`
  files contain only the header (`federacoes` absent). The converter writes `identificador`
  (INTEGER 100..999), sigla, nome and one `INTEGER(0..99)` per party.

---

## 8. The attendance result (*resultado urna cadastro*)

```
CResultadoUrnaCadastro (164)  = EntidadeResultadoUrnaCadastro
 ├ CCabecalhoEntidade cabecalho, CFaseID fase, string versaoVotacao, ESituacaoArquivo (final/parcial)
 ├ optional<CDadosComparecimento> (72)            ── infoDadosComparecimento [0]
 │   ├ vector<CIdentificacaoJustificativa>  {RegistroIdentificacaoEleitor, anoNascimento}
 │   ├ CIdentificacaoSecaoEleitoral
 │   ├ vector<CEstadoComparecimento> (96 each)
 │   │   ├ CRegistroIdentificacaoEleitor, ESituacaoComparecimento (faltou/semCargo/naoVotou/votou)
 │   │   ├ optional<CApresentacaoFotoEleitor>  {estado 0..2, resultado 0..8}
 │   │   ├ optional<ESituacaoHabilitacaoAudio> (automática / pelo mesário / não habilitado)
 │   │   └ optional<CHabilitacaoBiometrica> (48)
 │   │       ├ tentativas, ultimoScore (0..999, 0 = absent), dedo (0..10), erroLeitura (0..12)
 │   │       └ optional<CEstadoHabilitacaoPorCodigo> {situação do mesário, optional<registro do mesário>}
 │   └ optional<vector<CComparecimentoMesario>> abertura / encerramento {registro, dataHora, estadoColeta 0..4}
 └ optional<CDadosComparecimentoCifrado> (48)      ── infoDadosComparecimento [1]
     └ CDadosCifracao {chave, salt ≥16 B, informacaoAdicional ≥16 B} + conteudo
```

Consistency rules enforced while **reading** (Deconverte). The `CEstadoHabilitacaoPorCodigo` and
`CDadosCifracao` rules are data-class constructor checks, so they also apply when `CGravadorRCSecao` (func
11616) builds these objects for writing:

* `EstadoComparecimento`: audio or biometrics require the photo data (2637, "Eleitor que votou não possui
  informações sobre a apresentacao de fotos do eleitor."); photo or biometrics require the audio data
  (2638). Valid shapes: nothing, photo+audio, photo+audio+biometrics.
* `HabilitacaoBiometrica`: if `dedoHabilitado ≠ naoIdentificado` there must be no `habilitacaoPorCodigo`
  and `erroLeituraBiometria = semErro` (2645 "Estrutura de dados mal formada."); otherwise
  `habilitacaoPorCodigo` is mandatory (2646).
* `EstadoHabilitacaoPorCodigo`: mesário identification present ⇔ situation ≠ NaoReconhecido (3270/3271).
* `CDadosCifracao`: salt and informacaoAdicional at least 16 bytes (3282/3283); the key size is not checked.
* `EntidadeResultadoUrnaCadastro`: the CHOICE must be selected (2660).

Encryption: with `criptografarJUFA = TRUE`, `comum::CGravadorRCSecao::GravaResultado` (func 11616, named
`LeChavePublica` by the tool) encodes `DadosComparecimento`, encrypts it with CEPESC (`CPlainText`,
key file `jufa.pk1`) and stores `DadosComparecimentoCifrado`. The data links each voter's identifier to
attendance and enabling method (never to the vote).

---

## 9. What this unit contributes to the *boletim de urna* (BU)

The BU itself is built by `comum::CGravadorBU` / `CConversorEntidadeBU` (uenux2, see
`docs/bu/build-a-bu.md` and `docs/bu/qrcode.md`). This unit supplies, step by step:

1. **How many copies and what text** (from `…-pu.dat`, §6): the parameters give
   `numBUVotaObrigatorios = 5` mandatory copies (*vias*) and `numBUVotaAdicionais = 5` additional ones
   (the consumer of these two integers is in the VOTA encerramento code, not in this unit; the meaning
   is taken from the names and from the screen text below, which lists exactly 5 vias).
   The screen text `telaEmissaoBU` tells the mesários what to do with them:
   > "Após a impressão completa do BU, colha as assinaturas necessárias e distribua as vias do seguinte
   > modo: - Duas vias vão, juntamente com a mídia de resultado, para a Junta Eleitoral; - Uma via fica
   > com o Presidente da Mesa Receptora de Votos; - Uma via fica com a fiscalização partidária; - Uma via
   > deve ser afixada em lugar visível da seção eleitoral."
2. **Header and footer lines** (`CTituloRelatorio {alinhamento, estilo, texto}`):
   `cabecalho` = [centro/expandido "Justiça Eleitoral", centro/normal "Tribunal Regional Eleitoral
   [<uf>]"]; `rodapeBUVOTA` = [centro "O conteúdo deste BU poderá ser\nconferido no endereço\nresultados.tse.jus.br",
   centro "ASSINATURAS:", esquerdo "PRESIDENTE: … MESÁRIOS: … FISCAIS:"]; `rodapeZEVOTA` = the same
   without the first line (zerésima). For SA: `rodapeBUSA`/`rodapeBUParcial` sign by "PRESIDENTE DA JUNTA,
   COMPONENTES DA JUNTA, MINISTÉRIO PÚBLICO, FISCAIS", `rodapeZESA` by "SECRETÁRIO DA JUNTA, FISCAIS".
3. **Switches**: `imprimirQrCodeNoBU = TRUE` (the QR codes; read at CConfiguracaoEleicao+486 by the QR
   generator, `docs/bu/qrcode.md`), `imprimirZeradosBU` / `gravarZeradosBU = FALSE` (by their names:
   whether entries with zero votes are printed / recorded), `criptografarBU = FALSE`.
4. **The medium's serial number**: `CGravadorBU` reads `infomidia.dat` through this unit's
   `CConversorInformacaoMidia` and writes the hex of `CSerialMidia` (8 hex digits validated by regex) as
   `urna.numeroSerieFV`, defaulting to `"00000000"` when the file is missing (the web build only has
   `infomidia-fv-{1,2}-t.dat`, so the default is used; `docs/bu/build-a-bu.md` §fields).
5. **Headers of ecourna files** use `CConversorCabecalhoEntidade`: `dataGeracao` as DataHoraJE
   (`"{:04}{:02}{:02}T{:02}{:02}{:02}"`, 15 chars) and `idEleitoral` CHOICE (0 PE, 1 pleito, 2 eleição);
   an unselected CHOICE or index ≥ 3 throws 2236. (The BU's own header uses the uenux2 twin
   `comum::asn::CConversorCabecalhoEntidade`, func 11589.)
6. **Cargo codes** in the RDV/BU vote records go through `CConversorCargoID`: 1..13 →
   `cargoConstitucional` (presidente … vereador), 25..99 → `numeroCargoConsultaLivre`, anything else 2245.
7. **Alongside the BU**, at encerramento, the attendance file of §8 is written (with
   `registrarMesarios = TRUE`, the mesários of abertura/encerramento are included).

---

## 10. Web-build specifics

* Nothing in this unit is mocked. The behaviour differences come from the data and from which paths
  the web page reaches.
* The web page never runs the encerramento (no poll-worker thread, votes not kept; see
  `docs/bu/build-a-bu.md`). So the `resultadournacadastro` converters, the RC writer and the
  `infomidia.dat` reading of `CGravadorBU` are **dead in the web build** (none observed executing).
* The 16 `-pu.dat` files carry identical parameters (4 distinct files that differ only in the header);
  `-fe.dat` files are header-only; the `infomidia-fv-{1,2}-t.dat` files are FC media with a fake generator (`simulador-votacao-ng`, all-zero TPM serial) and is not signed (its `.vsc`
  is empty, per `docs/data-model/asn1-schemas.md`).

---

## 11. wasm / Emscripten observations

* **merge-similar-functions**: the three identifier constructors are 33-byte thunks into func 3939
  (8 parameters: srcloc, code, format string, vtable, size, type); six `ConverteLista` instantiations
  are 26/27-byte thunks into func 1970; the nine error-family constructors are 18-byte thunks into
  func 1011 (eight are in this unit; the EAsnMidiasError one, func 9181, is not in the u14 index list); enum helpers share 2306/6144/6148/6151 (§3.1).
* **ICF**: `IIdentificadorEleitor::GetNumero` is the same body as `api::CFixedText::vf2` (func 1139),
  hence "GUI" names in callers; the derived identifiers' destructors are ICF'd tiny bodies (3491/3492).
* **libc++ ABI v2**: `std::shared_ptr` is `trivial_abi`, so by-value shared_ptr parameters are released
  by the callee (visible in funcs 1675/1878); `std::regex` uses `ECMAScript = 512` (func 2416 flag).
* **Strings**: almost every literal of `ecourna/app/dados` is **ISO-8859-1** (e.g. `N\xFAmero de
  s\xE9rie`); `cconversorturno.cpp`'s `"Turno inválido."` is the only UTF-8 one. Some messages are
  `std::string` globals built by `__wasm_call_ctors` (func 14478): the CBaseType type names
  (`ProcessoEleitoralID` @1911992 … `TipoIdentificadorEleitor` @1912208, 19 names) and four converter
  messages @1912232..1912268, one of which ("Forma de validar título inválido.") is never used.
* **Decompiler artefacts** to ignore: integer codes annotated with unrelated strings (e.g. `2485 /*
  "Foto duplicada…" */`, `2650 /* "{}: instância…" */`), and table slots split in two (`671 /* … */4` =
  slot 6714, `604 … 8` = slot 6048, `677 … 3` = slot 6773, `683 … 1` = slot 6831). A switch table
  displayed as `(b << 2)[283861]` is really at 1135444 (check with `q.py wat`).
* `null` shared_ptr dereferences do not trap in wasm (address 4 reads 0), see §13 item 6.

---

## 12. Functions observed executing (recorded votes)

5102 `CConversorParametrosUrna::DesconverterTitulos` and 9156 `CConversorParametrosUrna::DoDeconverte` (loading `-pu.dat`), 9028
`CConfiguracaoMunicipios` (loading `-cfm.dat`), 9218 `CConversorCabecalhoEntidade::DoDeconverte`
(headers of those files), 9226 `CConversorFoto::DoDeconverte` (candidate photos), 9230
`CConversorCargoID::DoConverte` (cargo codes of vote records).

---

## 13. Weird or risky code

1. **Missing `throw`** in `CConversorResultadoUrnaCadastro::DoConverte` (func 9056, line 48): when a
   `CResultadoUrnaCadastro` has neither clear nor encrypted attendance data, the code constructs
   `CBaseError(2658, "Classe de dados de comparecimento mal formada.")` on the stack (CError ctor func
   1143, no `__cxa_allocate_exception`/`__cxa_throw`), destroys it and returns an entity with an
   unselected CHOICE. The error resurfaces only as a generic invalid-entity error from `Converte`.
2. **Swapped messages** in `CEstadoComparecimento` (funcs 9020/9021): the audio getter reports
   "Dados de apresentação de foto…" (3268) and the photo getter "Habilitação de áudio…" (3269).
3. **`tipoMidia` ignored** when `aplicativos` is present (func 9179): any medium file with applications
   becomes an MR (result medium) object.
4. `CAutenticacao::GetDataHoraInicial` throws "início não definido" when only the **end** date is missing
   (both getters test both optionals).
5. `ultimoScore = 0` is dropped by `CConversorHabilitacaoBiometrica::DoConverte` (0 means "absent").
6. `CRegistroIdentificacaoEleitor(hab, principal)` engages the optional even if `principal` is empty;
   `GetIdentificadorPrincipal` would then return an empty pointer, and `CConversorIdentificadorEleitor::DoConverte`
   (reached through `CConversorRegistroIdentificacaoEleitor::DoConverte`, func 9114, which only tests the engaged
   flag) dereferences it (`identificador->GetTipo()`): a crash on native, a silent read of address 4 in wasm.
   **Latent only**: both callers of func 1878 pass a non-null pointer (func 9112 passes a `make_shared` result of
   `CConversorIdentificadorEleitor::Deconverte`, func 11616 the result of `CriaIdentidadeEleitor`, func 5845; both
   throw instead of returning null).
7. `CConversorCabecalhoEntidade::DoConverte` throws only for id type 3; types ≥ 4 (and negative values, the
   `br_table` index being unsigned) fall through silently and leave the CHOICE unselected.
8. Mixed encodings (ISO-8859-1 vs UTF-8) in exception texts; typos in messages ("biomoetria",
   "habiliação", "apresentacao"); one unused message constant.

---

## 14. Complete mapping table (136 functions)

`run` = observed executing during the recorded votes. Original file paths marked *(path inferred)* have
no `source_location`. "library" rows are libc++/ASN.1-runtime instantiations or wasm-opt shared bodies
that are only summarised in a comment of the listed source.

| func | size | run | reconstructed symbol | original file | src | conf. |
|---:|---:|:-:|---|---|---|---|
| 778 | 12 |  | `ecourna::app::dados::IIdentificadorEleitor::~IIdentificadorEleitor` | ecourna-lib/ecourna/app/dados/iidentificadoreleitor.cpp | src/ecourna/app/dados/iidentificadoreleitor.cpp | high |
| 1162 | 273 |  | `ecourna::app::dados::CDadosComparecimento::~CDadosComparecimento (implicit)` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cdadoscomparecimento.cpp (path inferred) | src/ecourna/app/dados/resultadournacadastro/cdadoscomparecimento.cpp | medium |
| 1241 | 33 |  | `ecourna::app::dados::CNumeroInscricaoEleitoral::CNumeroInscricaoEleitoral (thunk -> 3939)` | ecourna-lib/ecourna/app/dados/cnumeroinscricaoeleitoral.cpp | src/ecourna/app/dados/cnumeroinscricaoeleitoral.cpp | high |
| 1673 | 105 |  | `std::vector<ecourna::app::dados::CTituloRelatorio>::~vector` | library (libc++ instantiation) | src/ecourna/app/dados/parametrizacaourna/cparametrosurna.cpp (comment) | high |
| 1675 | 401 |  | `ecourna::app::dados::CRegistroIdentificacaoEleitor::CRegistroIdentificacaoEleitor(TSharedIdentificadorEleitor)` | ecourna-lib/ecourna/app/dados/cregistroidentificacaoeleitor.cpp | src/ecourna/app/dados/cregistroidentificacaoeleitor.cpp | high |
| 1878 | 480 |  | `ecourna::app::dados::CRegistroIdentificacaoEleitor::CRegistroIdentificacaoEleitor(TSharedIdentificadorEleitor, TSharedIdentificadorEleitor)` | ecourna-lib/ecourna/app/dados/cregistroidentificacaoeleitor.cpp | src/ecourna/app/dados/cregistroidentificacaoeleitor.cpp | high |
| 1943 | 201 |  | `std::optional<TVectorComparecimentoMesario>::~optional` | library (libc++ instantiation) | src/ecourna/app/dados/resultadournacadastro/cdadoscomparecimento.cpp (comment) | medium |
| 1970 | 267 |  | `ecourna::api::asn::ConverteLista<TConversor> (shared merged body, 6 instantiations)` | ecourna/api/asn template (inferred) | src/ecourna/app/dados/asn/midias/cconversorinformacaomidia.cpp | medium |
| 2306 | 268 |  | `merged body of 5 enum Converte helpers (0-based -> ASN value+1, count 3)` | wasm-opt merge-similar | (expanded in each converter) | high |
| 2659 | 88 |  | `ecourna::app::dados::CDadosCifracao::~CDadosCifracao (implicit)` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cdadoscifracao.cpp (path inferred) | src/ecourna/app/dados/resultadournacadastro/cdadoscifracao.cpp (comment) | medium |
| 2663 | 102 |  | `ecourna::app::dados::CLabelParametrizado::~CLabelParametrizado (implicit)` | ecourna-lib/ecourna/app/dados/parametrizacaourna/clabelparametrizado.h (path inferred) | src/ecourna/app/dados/parametrizacaourna/cparametrosurna.cpp (comment) | medium |
| 2664 | 123 |  | `std::vector<ecourna::app::dados::CAplicativo>::~vector` | library (libc++ instantiation) | src/ecourna/app/dados/midias/cinformacaomidia.cpp (comment) | medium |
| 2668 | 1745 |  | `ecourna::app::dados::CErroLeituraBiometria::CErroLeituraBiometria` | ecourna-lib/ecourna/app/dados/tiposbasicos.cpp | src/ecourna/app/dados/tiposbasicos.cpp | high |
| 2669 | 294 |  | `ecourna::app::dados::CRegistroIdentificacaoEleitor::GetIdentificadorHabilitacao` | ecourna-lib/ecourna/app/dados/cregistroidentificacaoeleitor.cpp | src/ecourna/app/dados/cregistroidentificacaoeleitor.cpp | high |
| 2671 | 55 |  | `std::__exception_guard<vector::__destroy_vector>::~__exception_guard (ICF, also boost)` | library (libc++) | - | medium |
| 3486 | 141 |  | `ecourna::app::dados::CComparecimentoMesario::CComparecimentoMesario` | ecourna-lib/ecourna/app/dados/resultadournacadastro/ccomparecimentomesario.cpp (path inferred) | src/ecourna/app/dados/resultadournacadastro/ccomparecimentomesario.cpp | medium |
| 3487 | 116 |  | `ecourna::app::dados::CDadosComparecimentoCifrado::~CDadosComparecimentoCifrado (implicit)` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cdadoscomparecimentocifrado.h (path inferred) | src/ecourna/app/dados/resultadournacadastro/cdadoscifracao.cpp (comment) | medium |
| 3489 | 464 |  | `std::__tree<map<TMunicipioID,CConfiguracaoMunicipio>>::__find_equal(hint, ...)` | library (libc++ instantiation) | src/ecourna/app/dados/processoeleitoral/cconfiguracaomunicipios.cpp (comment) | medium |
| 3490 | 132 |  | `ecourna::app::dados::CDadosGeracaoMidia::~CDadosGeracaoMidia (implicit)` | ecourna-lib/ecourna/app/dados/midias/cdadosgeracaomidia.h (path inferred) | src/ecourna/app/dados/midias/cinformacaomidia.cpp (comment) | medium |
| 3939 | 883 |  | `merged ctor body of CNumeroInscricaoEleitoral / CNumeroCPF / CNumeroIdentificacaoLivre` | wasm-opt merge-similar | src/ecourna/app/dados/cnumeroinscricaoeleitoral.cpp, cnumerocpf.cpp, cnumeroidentificacaolivre.cpp | high |
| 5088 | 389 |  | `ecourna::app::dados::CEstadoHabilitacaoPorCodigo::CEstadoHabilitacaoPorCodigo(ESituacao, const CRegistroIdentificacaoEleitor&)` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cestadohabilitacaoporcodigo.cpp | src/ecourna/app/dados/resultadournacadastro/cestadohabilitacaoporcodigo.cpp | high |
| 5089 | 124 |  | `std::optional<ecourna::app::dados::CRegistroIdentificacaoEleitor>::~optional` | library (libc++ instantiation) | src/ecourna/app/dados/resultadournacadastro/cestadohabilitacaoporcodigo.cpp (comment) | medium |
| 5090 | 283 |  | `ecourna::app::dados::CEstadoHabilitacaoPorCodigo::CEstadoHabilitacaoPorCodigo(ESituacao)` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cestadohabilitacaoporcodigo.cpp | src/ecourna/app/dados/resultadournacadastro/cestadohabilitacaoporcodigo.cpp | high |
| 5091 | 746 |  | `ecourna::app::dados::CDadosCifracao::CDadosCifracao` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cdadoscifracao.cpp | src/ecourna/app/dados/resultadournacadastro/cdadoscifracao.cpp | high |
| 5093 | 1288 |  | `ecourna::app::dados::CParametrosUrna::CParametrosUrna (47 args)` | ecourna-lib/ecourna/app/dados/parametrizacaourna/cparametrosurna.cpp (path inferred) | src/ecourna/app/dados/parametrizacaourna/cparametrosurna.cpp | medium |
| 5102 | 345 | * | `ecourna::app::dados::asn::CConversorParametrosUrna::DesconverterTitulos` (name inferred; member: dead `this` kept in the signature) | ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrosurna.cpp | src/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrosurna.cpp | medium |
| 5104 | 137 |  | `std::__exception_guard<vector<CAplicativo>::__destroy_vector>::~__exception_guard` | library (libc++ instantiation) | src/ecourna/app/dados/asn/midias/cconversorinformacaomidia.cpp (comment) | medium |
| 5105 | 64 |  | `std::optional<TSharedIdentificadorEleitor>::~optional` | library (libc++ instantiation) | src/ecourna/app/dados/cregistroidentificacaoeleitor.cpp (comment) | high |
| 5106 | 33 |  | `ecourna::app::dados::CNumeroIdentificacaoLivre::CNumeroIdentificacaoLivre (thunk -> 3939)` | ecourna-lib/ecourna/app/dados/cnumeroidentificacaolivre.cpp | src/ecourna/app/dados/cnumeroidentificacaolivre.cpp | high |
| 5107 | 33 |  | `ecourna::app::dados::CNumeroCPF::CNumeroCPF (thunk -> 3939)` | ecourna-lib/ecourna/app/dados/cnumerocpf.cpp | src/ecourna/app/dados/cnumerocpf.cpp | high |
| 5832 | 284 |  | `std::optional<TVectorComparecimentoMesario>::optional(const optional&)` | library (libc++ instantiation) | src/ecourna/app/dados/resultadournacadastro/cdadoscomparecimento.cpp (comment) | medium |
| 5846 | 397 |  | `ecourna::app::dados::CDadosComparecimento::CDadosComparecimento(const CDadosComparecimento&) (implicit)` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cdadoscomparecimento.cpp (path inferred) | src/ecourna/app/dados/resultadournacadastro/cdadoscomparecimento.cpp (comment) | medium |
| 9014 | 262 |  | `ecourna::app::dados::CResultadoUrnaCadastro::GetDadosComparecimentoCifrado` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cresultadournacadastro.cpp | src/ecourna/app/dados/resultadournacadastro/cresultadournacadastro.cpp | high |
| 9015 | 260 |  | `ecourna::app::dados::CResultadoUrnaCadastro::GetDadosComparecimento` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cresultadournacadastro.cpp | src/ecourna/app/dados/resultadournacadastro/cresultadournacadastro.cpp | high |
| 9016 | 260 |  | `ecourna::app::dados::CHabilitacaoBiometrica::GetEstadoHabilitacaoPorCodigo` | ecourna-lib/ecourna/app/dados/resultadournacadastro/chabilitacaobiometrica.cpp | src/ecourna/app/dados/resultadournacadastro/chabilitacaobiometrica.cpp | high |
| 9017 | 260 |  | `ecourna::app::dados::CEstadoHabilitacaoPorCodigo::GetIdentificacaoMesario` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cestadohabilitacaoporcodigo.cpp | src/ecourna/app/dados/resultadournacadastro/cestadohabilitacaoporcodigo.cpp | high |
| 9019 | 260 |  | `ecourna::app::dados::CEstadoComparecimento::GetHabilitacaoBiometrica` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cestadocomparecimento.cpp | src/ecourna/app/dados/resultadournacadastro/cestadocomparecimento.cpp | high |
| 9020 | 260 |  | `ecourna::app::dados::CEstadoComparecimento::GetSituacaoHabilitacaoAudio` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cestadocomparecimento.cpp | src/ecourna/app/dados/resultadournacadastro/cestadocomparecimento.cpp | high |
| 9021 | 260 |  | `ecourna::app::dados::CEstadoComparecimento::GetApresentacaoFoto` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cestadocomparecimento.cpp | src/ecourna/app/dados/resultadournacadastro/cestadocomparecimento.cpp | high |
| 9023 | 260 |  | `ecourna::app::dados::CDadosComparecimento::GetMesariosEncerramento` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cdadoscomparecimento.cpp | src/ecourna/app/dados/resultadournacadastro/cdadoscomparecimento.cpp | high |
| 9024 | 260 |  | `ecourna::app::dados::CDadosComparecimento::GetMesariosAbertura` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cdadoscomparecimento.cpp | src/ecourna/app/dados/resultadournacadastro/cdadoscomparecimento.cpp | high |
| 9025 | 18 |  | `CBaseError<EDadosResultadoUrnaCadastroError,{3265,3315}>::CBaseError (thunk -> 1011)` | exception thunk | src/ecourna/app/dados/dadoserros.h | high |
| 9026 | 18 |  | `CBaseError<EDadosProcessoEleitoralError,{3140,3190}>::CBaseError (thunk -> 1011)` | exception thunk | src/ecourna/app/dados/dadoserros.h | high |
| 9027 | 268 |  | `std::map<TMunicipioID,CConfiguracaoMunicipio>::insert(first, last)` | library (libc++ instantiation) | src/ecourna/app/dados/processoeleitoral/cconfiguracaomunicipios.cpp (comment) | medium |
| 9028 | 672 | * | `ecourna::app::dados::CConfiguracaoMunicipios::CConfiguracaoMunicipios` | ecourna-lib/ecourna/app/dados/processoeleitoral/cconfiguracaomunicipios.cpp | src/ecourna/app/dados/processoeleitoral/cconfiguracaomunicipios.cpp | high |
| 9030 | 260 |  | `ecourna::app::dados::CInformacaoMidia::GetAplicativos` | ecourna-lib/ecourna/app/dados/midias/cinformacaomidia.cpp | src/ecourna/app/dados/midias/cinformacaomidia.cpp | high |
| 9031 | 457 |  | `ecourna::app::dados::CInformacaoMidia::CInformacaoMidia(ETipoMidia, ...)` | ecourna-lib/ecourna/app/dados/midias/cinformacaomidia.cpp | src/ecourna/app/dados/midias/cinformacaomidia.cpp | high |
| 9032 | 477 |  | `ecourna::app::dados::CDadosGeracaoMidia::CDadosGeracaoMidia(const CDadosGeracaoMidia&) (implicit)` | ecourna-lib/ecourna/app/dados/midias/cdadosgeracaomidia.h (path inferred) | src/ecourna/app/dados/midias/cinformacaomidia.cpp (comment) | medium |
| 9033 | 474 |  | `ecourna::app::dados::CInformacaoMidia::CInformacaoMidia(fase, idPE, uf, turno, dadosGeracao, aplicativos) [MR]` | ecourna-lib/ecourna/app/dados/midias/cinformacaomidia.cpp (path inferred) | src/ecourna/app/dados/midias/cinformacaomidia.cpp | medium |
| 9035 | 272 |  | `ecourna::app::dados::CAutenticacao::GetDataHoraFinal` | ecourna-lib/ecourna/app/dados/midias/cautenticacao.cpp | src/ecourna/app/dados/midias/cautenticacao.cpp | high |
| 9036 | 269 |  | `ecourna::app::dados::CAutenticacao::GetDataHoraInicial` | ecourna-lib/ecourna/app/dados/midias/cautenticacao.cpp | src/ecourna/app/dados/midias/cautenticacao.cpp | high |
| 9041 | 18 |  | `CBaseError<EDadosMidiasError,{3040,3065}>::CBaseError (thunk -> 1011)` | exception thunk | src/ecourna/app/dados/dadoserros.h | high |
| 9042 | 263 |  | `ecourna::app::dados::CAplicativo::GetAutenticacao` | ecourna-lib/ecourna/app/dados/midias/caplicativo.cpp | src/ecourna/app/dados/midias/caplicativo.cpp | high |
| 9043 | 297 |  | `ecourna::app::dados::CAplicativo::CAplicativo(ETipoAplicativo, const CAutenticacao&)` | ecourna-lib/ecourna/app/dados/midias/caplicativo.cpp (path inferred) | src/ecourna/app/dados/midias/caplicativo.cpp | medium |
| 9044 | 467 |  | `ecourna::app::dados::CFederacao::CFederacao(const CFederacao&) (implicit)` | ecourna-lib/ecourna/app/dados/federacoes/cfederacao.cpp (path inferred) | src/ecourna/app/dados/federacoes/cfederacao.h | medium |
| 9046 | 18 |  | `CBaseError<EDadosFederacoesError,{2940,2965}>::CBaseError (thunk -> 1011)` | exception thunk | src/ecourna/app/dados/dadoserros.h | high |
| 9047 | 723 |  | `ecourna::app::dados::CFederacao::CFederacao` | ecourna-lib/ecourna/app/dados/federacoes/cfederacao.cpp | src/ecourna/app/dados/federacoes/cfederacao.cpp | high |
| 9051 | 489 |  | `ecourna::app::dados::asn::CConversorResultadoUrnaCadastro::DeconverteSituacaoArquivo` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorresultadournacadastro.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorresultadournacadastro.cpp | high |
| 9052 | 2100 |  | `ecourna::app::dados::asn::CConversorResultadoUrnaCadastro::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorresultadournacadastro.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorresultadournacadastro.cpp | high |
| 9054 | 734 |  | `ecourna::app::dados::CDadosCifracao::CDadosCifracao(const CDadosCifracao&) (implicit)` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cdadoscifracao.cpp (path inferred) | src/ecourna/app/dados/resultadournacadastro/cdadoscifracao.cpp (comment) | medium |
| 9055 | 297 |  | `ecourna::app::dados::asn::CConversorResultadoUrnaCadastro::ConverteSituacaoArquivo` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorresultadournacadastro.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorresultadournacadastro.cpp | high |
| 9056 | 2089 |  | `ecourna::app::dados::asn::CConversorResultadoUrnaCadastro::DoConverte` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorresultadournacadastro.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorresultadournacadastro.cpp | high |
| 9066 | 2002 |  | `ecourna::app::dados::asn::CConversorHabilitacaoBiometrica::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorhabilitacaobiometrica.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorhabilitacaobiometrica.cpp | high |
| 9069 | 281 |  | `ecourna::app::dados::asn::CConversorHabilitacaoBiometrica::ConverteErroLeituraBiometria` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorhabilitacaobiometrica.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorhabilitacaobiometrica.cpp | high |
| 9070 | 286 |  | `ecourna::app::dados::asn::CConversorHabilitacaoBiometrica::ConverteTipoDedo` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorhabilitacaobiometrica.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorhabilitacaobiometrica.cpp | high |
| 9071 | 704 |  | `ecourna::app::dados::asn::CConversorHabilitacaoBiometrica::DoConverte` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorhabilitacaobiometrica.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorhabilitacaobiometrica.cpp | high |
| 9072 | 21 |  | `ecourna::app::dados::asn::CConversorEstadoHabilitacaoPorCodigo::DeconverteSituacaoReconhecimentoMesario (thunk -> 6144)` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorestadohabilitacaoporcodigo.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorestadohabilitacaoporcodigo.cpp | high |
| 9073 | 503 |  | `ecourna::app::dados::asn::CConversorEstadoHabilitacaoPorCodigo::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorestadohabilitacaoporcodigo.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorestadohabilitacaoporcodigo.cpp | high |
| 9074 | 41 |  | `ecourna::app::dados::asn::CConversorEstadoHabilitacaoPorCodigo::ConverteSituacaoReconhecimentoMesario (thunk -> 2306)` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorestadohabilitacaoporcodigo.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorestadohabilitacaoporcodigo.cpp | high |
| 9075 | 481 |  | `ecourna::app::dados::asn::CConversorEstadoHabilitacaoPorCodigo::DoConverte` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorestadohabilitacaoporcodigo.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorestadohabilitacaoporcodigo.cpp | high |
| 9077 | 21 |  | `ecourna::app::dados::asn::CConversorEstadoComparecimento::DeconverteSituacaoHabilitacaoAudio (thunk -> 6144)` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorestadocomparecimento.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorestadocomparecimento.cpp | high |
| 9080 | 272 |  | `ecourna::app::dados::asn::CConversorEstadoComparecimento::DeconverteEstadoComparecimento` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorestadocomparecimento.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorestadocomparecimento.cpp | high |
| 9081 | 1946 |  | `ecourna::app::dados::asn::CConversorEstadoComparecimento::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorestadocomparecimento.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorestadocomparecimento.cpp | high |
| 9083 | 41 |  | `ecourna::app::dados::asn::CConversorEstadoComparecimento::ConverteSituacaoHabilitacaoAudio (thunk -> 2306)` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorestadocomparecimento.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorestadocomparecimento.cpp | high |
| 9085 | 289 |  | `ecourna::app::dados::asn::CConversorEstadoComparecimento::ConverteEstadoComparecimento` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorestadocomparecimento.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorestadocomparecimento.cpp | high |
| 9086 | 1127 |  | `ecourna::app::dados::asn::CConversorEstadoComparecimento::DoConverte` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorestadocomparecimento.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorestadocomparecimento.cpp | high |
| 9087 | 607 |  | `ecourna::app::dados::asn::CConversorApresentacaoFotoEleitor::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorapresentacaofotoeleitor.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorapresentacaofotoeleitor.cpp | high |
| 9088 | 281 |  | `ecourna::app::dados::asn::CConversorApresentacaoFotoEleitor::ConverteResultado` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorapresentacaofotoeleitor.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorapresentacaofotoeleitor.cpp | high |
| 9089 | 284 |  | `ecourna::app::dados::asn::CConversorApresentacaoFotoEleitor::ConverteEstado` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorapresentacaofotoeleitor.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorapresentacaofotoeleitor.cpp | high |
| 9091 | 191 |  | `ecourna::app::dados::asn::CConversorApresentacaoFotoEleitor::DoConverte` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorapresentacaofotoeleitor.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorapresentacaofotoeleitor.cpp | high |
| 9098 | 26 |  | `ConverteLista<CConversorComparecimentoMesario> (thunk -> 1970)` | template instantiation | src/ecourna/app/dados/asn/midias/cconversorinformacaomidia.cpp (comment) | medium |
| 9100 | 26 |  | `ConverteLista<CConversorIdentificacaoJustificativa> (thunk -> 1970)` | template instantiation | src/ecourna/app/dados/asn/midias/cconversorinformacaomidia.cpp (comment) | medium |
| 9103 | 264 |  | `ecourna::app::dados::asn::CConversorComparecimentoMesario::DeconverteEstadoColetaDigital` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorcomparecimentomesario.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorcomparecimentomesario.cpp | high |
| 9106 | 522 |  | `ecourna::app::dados::asn::CConversorComparecimentoMesario::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorcomparecimentomesario.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorcomparecimentomesario.cpp | high |
| 9107 | 18 |  | `CBaseError<asn::EAsnResultadoUrnaCadastroError,{2635,2665}>::CBaseError (thunk -> 1011)` | exception thunk | src/ecourna/app/dados/dadoserros.h | high |
| 9108 | 43 |  | `ecourna::app::dados::asn::CConversorComparecimentoMesario::ConverteEstadoColetaDigital (thunk -> 6151)` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorcomparecimentomesario.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorcomparecimentomesario.cpp | high |
| 9110 | 903 |  | `ecourna::app::dados::asn::CConversorComparecimentoMesario::DoConverte` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorcomparecimentomesario.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorcomparecimentomesario.cpp | high |
| 9116 | 652 |  | `ecourna::app::dados::asn::CConversorIdentificadorEleitor::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/cconversoridentificadoreleitor.cpp | src/ecourna/app/dados/asn/cconversoridentificadoreleitor.cpp | high |
| 9117 | 1268 |  | `ecourna::app::dados::asn::CConversorIdentificadorEleitor::DoConverte` | ecourna-lib/ecourna/app/dados/asn/cconversoridentificadoreleitor.cpp | src/ecourna/app/dados/asn/cconversoridentificadoreleitor.cpp | high |
| 9123 | 27 |  | `ConverteLista<CConversorEstadoComparecimento> (thunk -> 1970)` | template instantiation | src/ecourna/app/dados/asn/midias/cconversorinformacaomidia.cpp (comment) | medium |
| 9140 | 26 |  | `ConverteLista<CConversorConfiguracaoMunicipio> (thunk -> 1970)` | template instantiation | src/ecourna/app/dados/asn/midias/cconversorinformacaomidia.cpp (comment) | medium |
| 9145 | 1314 |  | `ecourna::app::dados::asn::CConversorTituloRelatorio::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversortitulorelatorio.cpp | src/ecourna/app/dados/asn/parametrizacaourna/cconversortitulorelatorio.cpp | high |
| 9147 | 325 |  | `ecourna::app::dados::asn::CConversorTituloRelatorio::ConverterEstiloTitulo` | ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversortitulorelatorio.cpp | src/ecourna/app/dados/asn/parametrizacaourna/cconversortitulorelatorio.cpp | high |
| 9148 | 37 |  | `ecourna::app::dados::asn::CConversorTituloRelatorio::ConverterAlinhamentoTitulo (thunk -> 6148)` | ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversortitulorelatorio.cpp | src/ecourna/app/dados/asn/parametrizacaourna/cconversortitulorelatorio.cpp | high |
| 9149 | 385 |  | `ecourna::app::dados::asn::CConversorTituloRelatorio::DoConverte` | ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversortitulorelatorio.cpp | src/ecourna/app/dados/asn/parametrizacaourna/cconversortitulorelatorio.cpp | high |
| 9152 | 370 |  | `std::vector<ecourna::app::dados::CTituloRelatorio>::__push_back_slow_path(CTituloRelatorio&&)` | library (libc++ instantiation) | src/ecourna/app/dados/parametrizacaourna/cparametrosurna.cpp (comment) | high |
| 9156 | 3569 | * | `ecourna::app::dados::asn::CConversorParametrosUrna::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrosurna.cpp | src/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrosurna.cpp | high |
| 9158 | 18 |  | `CBaseError<asn::EAsnParametrizacaoUrnaError,{2560,2585}>::CBaseError (thunk -> 1011)` | exception thunk | src/ecourna/app/dados/dadoserros.h | high |
| 9160 | 26 |  | `ASN1::SEQUENCE_OF<ModuloParametrizacaoUrna::TituloRelatorio>::SEQUENCE_OF(first, last) (thunk -> 2926)` | ASN.1 generated/template code | src/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrosurna.cpp (comment) | medium |
| 9161 | 309 |  | `ecourna::app::dados::asn::CConversorParametrosUrna::ConverterTitulos` (name inferred; member: dead `this` kept in the signature) | ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrosurna.cpp | src/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrosurna.cpp | medium |
| 9162 | 41 |  | `ecourna::app::dados::asn::CConversorParametrosUrna::ConverterFormaSuspenderComVoto (thunk -> 2306)` | ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrosurna.cpp | src/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrosurna.cpp | high |
| 9163 | 41 |  | `ecourna::app::dados::asn::CConversorParametrosUrna::ConverterFormaSuspenderSemVoto (thunk -> 2306)` | ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrosurna.cpp | src/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrosurna.cpp | high |
| 9164 | 3821 |  | `ecourna::app::dados::asn::CConversorParametrosUrna::DoConverte` | ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrosurna.cpp | src/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrosurna.cpp | high |
| 9167 | 1131 |  | `ecourna::app::dados::asn::CConversorLabelParametrizado::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversorlabelparametrizado.cpp | src/ecourna/app/dados/asn/parametrizacaourna/cconversorlabelparametrizado.cpp | high |
| 9168 | 37 |  | `ecourna::app::dados::asn::CConversorLabelParametrizado::ConverterGeneroLabel (thunk -> 6148)` | ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversorlabelparametrizado.cpp | src/ecourna/app/dados/asn/parametrizacaourna/cconversorlabelparametrizado.cpp | high |
| 9169 | 949 |  | `ecourna::app::dados::asn::CConversorLabelParametrizado::DoConverte` | ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversorlabelparametrizado.cpp | src/ecourna/app/dados/asn/parametrizacaourna/cconversorlabelparametrizado.cpp | high |
| 9179 | 2247 |  | `ecourna::app::dados::asn::CConversorInformacaoMidia::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/midias/cconversorinformacaomidia.cpp | src/ecourna/app/dados/asn/midias/cconversorinformacaomidia.cpp | high |
| 9182 | 492 |  | `ModuloInformacaoMidia::InformacaoMidia::set_aplicativos (generated setter)` | ASN.1 generated code | src/ecourna/app/dados/asn/midias/cconversorinformacaomidia.cpp (comment) | medium |
| 9183 | 27 |  | `ConverteLista<CConversorAplicativo> (thunk -> 1970)` | template instantiation | src/ecourna/app/dados/asn/midias/cconversorinformacaomidia.cpp | medium |
| 9185 | 560 |  | `std::__uninitialized_allocator_copy<CAplicativo> (vector<CAplicativo> copy)` | library (libc++ instantiation) | src/ecourna/app/dados/midias/cinformacaomidia.cpp (comment) | medium |
| 9189 | 41 |  | `ecourna::app::dados::asn::CConversorInformacaoMidia::ConverteTipoMidia (thunk -> 2306)` | ecourna-lib/ecourna/app/dados/asn/midias/cconversorinformacaomidia.cpp | src/ecourna/app/dados/asn/midias/cconversorinformacaomidia.cpp | high |
| 9190 | 1731 |  | `ecourna::app::dados::asn::CConversorInformacaoMidia::DoConverte` | ecourna-lib/ecourna/app/dados/asn/midias/cconversorinformacaomidia.cpp | src/ecourna/app/dados/asn/midias/cconversorinformacaomidia.cpp | high |
| 9191 | 274 |  | `ecourna::app::dados::asn::CConversorTurno::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/cconversorturno.cpp | src/ecourna/app/dados/asn/cconversorturno.cpp | high |
| 9192 | 301 |  | `ecourna::app::dados::asn::CConversorTurno::DoConverte` | ecourna-lib/ecourna/app/dados/asn/cconversorturno.cpp | src/ecourna/app/dados/asn/cconversorturno.cpp | high |
| 9193 | 478 |  | `ecourna::app::dados::asn::CConversorFase::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/cconversorfase.cpp | src/ecourna/app/dados/asn/cconversorfase.cpp | high |
| 9195 | 501 |  | `ecourna::app::dados::asn::CConversorFase::DoConverte` | ecourna-lib/ecourna/app/dados/asn/cconversorfase.cpp | src/ecourna/app/dados/asn/cconversorfase.cpp | high |
| 9196 | 1224 |  | `ecourna::app::dados::asn::CConversorAplicativo::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/midias/cconversoraplicativo.cpp | src/ecourna/app/dados/asn/midias/cconversoraplicativo.cpp | high |
| 9198 | 43 |  | `ecourna::app::dados::asn::CConversorAplicativo::ConverteTipoAplicativo (thunk -> 6151)` | ecourna-lib/ecourna/app/dados/asn/midias/cconversoraplicativo.cpp | src/ecourna/app/dados/asn/midias/cconversoraplicativo.cpp | high |
| 9199 | 434 |  | `ecourna::app::dados::asn::CConversorAplicativo::DoConverte` | ecourna-lib/ecourna/app/dados/asn/midias/cconversoraplicativo.cpp | src/ecourna/app/dados/asn/midias/cconversoraplicativo.cpp | high |
| 9210 | 26 |  | `ConverteLista<CConversorFederacao> (thunk -> 1970)` | template instantiation | src/ecourna/app/dados/asn/midias/cconversorinformacaomidia.cpp (comment) | medium |
| 9217 | 1048 |  | `ecourna::app::dados::asn::CConversorFederacao::DoConverte` | ecourna-lib/ecourna/app/dados/asn/federacoes/cconversorfederacao.cpp (path inferred) | src/ecourna/app/dados/asn/federacoes/cconversorfederacao.cpp | high |
| 9218 | 311 | * | `ecourna::app::dados::asn::CConversorCabecalhoEntidade::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/cconversorcabecalhoentidade.cpp | src/ecourna/app/dados/asn/cconversorcabecalhoentidade.cpp | high |
| 9219 | 953 |  | `ecourna::app::dados::asn::CConversorCabecalhoEntidade::DoConverte` | ecourna-lib/ecourna/app/dados/asn/cconversorcabecalhoentidade.cpp | src/ecourna/app/dados/asn/cconversorcabecalhoentidade.cpp | high |
| 9226 | 631 | * | `ecourna::app::dados::asn::CConversorFoto::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/cconversorfoto.cpp | src/ecourna/app/dados/asn/cconversorfoto.cpp | high |
| 9227 | 451 |  | `ecourna::app::dados::asn::CConversorFoto::DoConverte` | ecourna-lib/ecourna/app/dados/asn/cconversorfoto.cpp | src/ecourna/app/dados/asn/cconversorfoto.cpp | high |
| 9228 | 273 |  | `ecourna::app::dados::asn::CConversorCargoID::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/cconversorcargoid.cpp | src/ecourna/app/dados/asn/cconversorcargoid.cpp | high |
| 9229 | 18 |  | `CBaseError<asn::EAsnError,{2235,2335}>::CBaseError (thunk -> 1011)` | exception thunk | src/ecourna/app/dados/dadoserros.h | high |
| 9230 | 1788 | * | `ecourna::app::dados::asn::CConversorCargoID::DoConverte` | ecourna-lib/ecourna/app/dados/asn/cconversorcargoid.cpp | src/ecourna/app/dados/asn/cconversorcargoid.cpp | high |
| 9256 | 1295 |  | `ecourna::app::dados::IIdentificadorEleitor::IIdentificadorEleitor` | ecourna-lib/ecourna/app/dados/iidentificadoreleitor.cpp | src/ecourna/app/dados/iidentificadoreleitor.cpp | high |
| 9258 | 319 |  | `ecourna::app::dados::(anon)::SerialValido` | ecourna-lib/ecourna/app/dados/cserialmidia.cpp | src/ecourna/app/dados/cserialmidia.cpp | medium |
| 9259 | 1057 |  | `ecourna::app::dados::CSerialMidia::CSerialMidia` | ecourna-lib/ecourna/app/dados/cserialmidia.cpp | src/ecourna/app/dados/cserialmidia.cpp | high |
| 9260 | 295 |  | `ecourna::app::dados::CRegistroIdentificacaoEleitor::GetIdentificadorPrincipal` | ecourna-lib/ecourna/app/dados/cregistroidentificacaoeleitor.cpp | src/ecourna/app/dados/cregistroidentificacaoeleitor.cpp | high |
| 9262 | 289 |  | `ecourna::app::dados::CNumeroInscricaoEleitoral::GetNumeroFormatado` | ecourna-lib/ecourna/app/dados/cnumeroinscricaoeleitoral.cpp | src/ecourna/app/dados/cnumeroinscricaoeleitoral.cpp | medium |
| 9263 | 9 |  | `ecourna::app::dados::CNumeroIdentificacaoLivre::GetNumeroFormatado` | ecourna-lib/ecourna/app/dados/cnumeroidentificacaolivre.cpp | src/ecourna/app/dados/cnumeroidentificacaolivre.cpp | medium |
| 9264 | 339 |  | `ecourna::app::dados::CNumeroCPF::GetNumeroFormatado` | ecourna-lib/ecourna/app/dados/cnumerocpf.cpp | src/ecourna/app/dados/cnumerocpf.cpp | medium |
| 9266 | 18 |  | `CBaseError<EDadosError,{1935,2135}>::CBaseError (thunk -> 1011)` | exception thunk | src/ecourna/app/dados/dadoserros.h | high |

---

## 15. Open questions

* The name of the `case -1` enumerator in the enum `switch`es (and of the `case 0` / `case 3` sentinels
  of `CFaseID` / `CCabecalhoEntidade::ETipoId`).
* The exact declaration of `CIdentificadorEleitor` and `CFaseID` (distinct RTTI names; both 4–8 bytes).
* The RC attendance file name and the full `CGravadorRCSecao` flow (unit u23).
* The name of vtable slot 2 of `IIdentificadorEleitor` (`GetNumeroFormatado` is inferred from behaviour).
* The names of the enumerators of `ESituacaoComparecimento` (the C++ order faltou, semCargo, naoVotou,
  votou comes from the lookup tables; the names are inferred).
