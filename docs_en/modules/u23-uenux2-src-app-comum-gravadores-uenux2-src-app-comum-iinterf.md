# u23 — `comum/gravadores` (result-file writers), `IInterfaceInit`, `IInterfaceSavd`, `CInformacaoEleicao`

Unit u23 of the reverse-engineering of `vota_web_wasm.wasm` (TSE voting application VOTA, `uenux2` + `ecourna`,
Emscripten build of the public training simulator). 107 functions, 10 of them seen executing during the recorded
votes. Reconstructed sources: `src/uenux2/src/app/comum/gravadores/**`, `src/uenux2/src/app/comum/iinterfaceinit.*`,
`src/uenux2/src/app/comum/iinterfacesavd.*`, `src/uenux2/src/app/comum/informacao/cinformacaoeleicao.cpp`.
Function indices are wasm function indices (`python3 tools/wasmmap/q.py f <n>`).

Related chapters: docs/10-boletim-de-urna.md (what a BU is),
docs/bu/codepath.md (the encerramento, run for real with a harness),
[u07](u07-uenux2-src-app-vota-eleitor.md) (`vota::CGravaResultado`, which constructs and runs every writer of this
unit), [u14](u14-ecourna-lib-ecourna-app-dados.md) (the `ecourna::app::dados` attendance classes written by
`CGravadorRCSecao`), [u12](u12-ecourna-lib-ecourna-api-compression-ecourna-lib-ecourna-api-.md) (`CGravadorWSQ`
packing), [u05](u05-uenux2-src-app-comum-dados.md) (the RDV the BU is read from).

## 0. Words

| term | meaning |
|---|---|
| *gravador* | "writer": one class per result file (`CGravadorBU`, `CGravadorRCSecao`, …) |
| *resultado* / *arquivos de resultado* | the files the urna produces at the end of the day and copies to the *mídia de resultado* (MR, a USB stick) for transmission to the TSE |
| *BU*, *boletim de urna* | the section's tally (`…-bu.dat`) |
| *RC*, *JUFA* | attendance record of the section: who voted, justifications of absence (*justificativas*), mesários present (`…-jufa.dat`) |
| *WSQ* | fingerprint image format; `wsq*.jez` are ZIP packages of the day's fingerprint captures |
| MI / MV / MR | *memória interna* (`/dsk/fi`), *memória de votação* (external flash, `/dsk/fe`), *mídia de resultado* (`/dsk/mr`) |
| *trab* / *resultado* dirs | per-flash work directory (`dinamico/trab1/`) and result directory (`dinamico/res…`) |
| SAVD | the urna's signing/validation daemon (keys, HSM). The application talks to it with a small binary protocol |
| *init* | the urna's init/hardware service (power off, boot device, MR port, demonstration flag) |
| *modo demonstração* | demonstration mode of the urna (fixed report counts, no mesário registration) |
| *fase* | `o` oficial, `s` simulado, `t` treinamento (file prefix); ASN.1/`EUrnaFase` `'1'`/`'2'`/`'3'` |
| *pleito* / *eleição* | the election event (e.g. 2410) / one ballot inside it (e.g. 2411 municipal) |
| *abrangência* | scope of an election (municipal / estadual / federal); the eligible-voter counts are per abrangência |
| *hash encadeado* | the BU's chained SHA-512 over its vote lines |
| CEPESC | the TSE's encryption library (`ecourna::api::cepesc`), used to encrypt BU/JUFA contents when the parameters ask for it |

## 1. What the subsystem does

At the *encerramento* (end of voting), `vota::CGravaResultado::StartState` (func 12098, unit u07) builds one
`comum::IGravador` per result file, and a `comum::CGravacaoResultados` runs them. Each writer:

1. computes its file name from the section (`CGravadorUtil::DeterminaNomeArquivo`, func 3798):
   `"{fase:c}{pleito:05}{uf}{municipio:05}{zona:04}{secao:04}-" + suffix`, e.g. `t02410ac0000100010001-bu.dat`
   (the UF is the lower-case `CLocal::GetUF()`; the suffix table is `CArquivosResultado`);
2. writes the file in the MI work directory (`IGravador::Grava`, slot 4 → its own `GravaResultado(CFile&)`,
   slot 7), copies it to the MI result directory (slot 2), to the MV work directory (slot 3) and to the MV result
   directory (slot 6);
3. is then signed through SAVD (`CAssinador::AssinaArquivosResultado`, u07) into `…-vota.vsc`.

The unit contains four of those writers — **BU**, **RC da seção (jufa)**, **hashes**, **WSQ** — with their ASN.1
converters and C++ data models (`md::`), plus three service clients used all over the application:

* `IInterfaceInit`: requests to the init service (MR mounting and serial, power-off, demonstration flag);
* `IInterfaceSavd` + `CAssinador::Assina`: every signature of a dynamic file (`vota.bin`, `rdv.dat`, reports,
  `uenux.db` …) goes through `CAssinador::Assina` (func 1277) → SAVD. This path runs many times per vote;
* `CInformacaoEleicao`: parameter getters that answer fixed values in demonstration mode.

## 2. Classes (RTTI) and layouts

```
comum::IResultado (vtable @1541028)                      iresultado.cpp
  └ comum::IGravador (@1553656)  slots: 2 CopiaParaResultado 3 CopiaParaMV 4 Grava 5 GravaMV
      │                                 6 CopiaResultadoParaMV 7 GravaResultado(api::CFile&) [pure]
      ├ comum::CGravadorBU        (@1554140, 304 B)  bu.dat     SAVD file 35   cgravadorbu.cpp
      ├ comum::CGravadorRCSecao   (@1554748,  60 B)  jufa.dat   SAVD file 39   cgravadorrcsecao.cpp
      ├ comum::CGravadorHashes    (@1554600,  96 B)  hash.dat   SAVD file 46   cgravadorhashes.cpp
      ├ comum::CGravadorWSQ       (@1557300,  88 B)  wsq*.jez   SAVD 48/49/50  cgravadorwsq.cpp
      ├ comum::CGravadorRDV, CGravadorLog, CGravadorVersoesArquivos, IGravadorEnvelope─CGravadorEnvelopeArquivo
      │     (other units: u05/u07/u12/u21)
comum::CAssinador (@1553260) └ vota::CAssinadorVota (@1532312)                     cassinador.cpp
comum::IInterfaceInit (@1552160) └ simulador::CWasmInit (@1528772, web mock)        iinterfaceinit.cpp
comum::IInterfaceSavd (@1526736) └ (anonymous)::CWasmSavd (@1526688, web mock)     iinterfacesavd.cpp
comum::asn::CConversorEntidadeBU (@1597536)       IConversorASN<EntidadeBoletimUrna, md::CEntidadeBU>
comum::asn::CConversorEnvelopeGenerico            IConversorASN<EntidadeEnvelopeGenerico, md::CEnvelopeGenerico>
comum::asn::CConversorUrna (@1596460)             IConversorASN<Urna, md::CUrna>
comum::asn::CConversorCorrespResultado (@1596224) IConversorASN<CorrespondenciaResultado, md::CCorrespondenciaResultado>
comum::asn::CConversorTipoApuracaoSA (@1596864)   IConversorASN<TipoApuracaoSA, md::CTipoApuracaoSA>
comum::asn::CConversorVersoesArquivos (@1599296)  IConversorASN<EntidadeVersaoArquivos, md::CVersoesArquivos>
```

`IResultado` layout: `+4` município, `+8` zona (u16), `+12` local, `+16` seção (u16), `+18` fase char,
`+20` std::string nome, `+32` `EExtensaoArquivoResultado`, `+36` `ESavdArquivoUE`. The only out-of-line
constructor is `IGravador`'s (func 1396, IResultado's inlined). It validates the fase (`'o'`, `'s'`, `'t'`, else
8657 "Fase inválida: {}") **after** computing the name.

`CGravadorBU` (built inline by func 12098; see `src/.../gravadores/cgravadorbu.h` for every field):
`+40` dhGeração, `+52` dhEmissão, `+64` fase (`'1'..'3'`), `+68` tipo de arquivo `'1'` (votacaoUE), `+72`
`CDadoCorrespondencia` (carga: nº interno da urna, serial MC, data da carga, código da carga, gerador; município
/ zona / seção), `+168` histórico de códigos de carga, `+180` map abrangência → {aptos seção, aptos TTE},
`+192` urna biométrica, `+194` comparecimento, `+196` `IRdv*` (the in-memory RDV), `+200` `"bu.pk1"`, `+212`
cifra permitida (true), `+216` optional `CDadosBUVota` {abertura, encerramento, optional desligamento do voto
impresso} (flag +256), `+260` optional `CTipoApuracaoSA` (flag +268), `+272` optional `CDadosBUSA` (flag +280),
`+284` map cargo → ordem de impressão, `+296` habilitações (sem biometria, por biometria, por biografia).

Error codes: the writers throw `CBaseError<comum::EUeComumGravadoresError, SErrorLimits{8600, 8800}>`
(constructor thunk func 283); the service clients throw `CBaseError<comum::EUeComumError, {7200, 7600}>` (func
480). Codes used here: 8601–8698 and 7231–7340 (listed next to each `throw` in the reconstruction).

## 3. The BU file, step by step (`CGravadorBU::GravaResultado`, func 11629, and `CConversorEntidadeBU::DoConverte`, func 10273)

This is the code that turns the in-memory RDV into `…-bu.dat`. It runs once, at the encerramento, inside
`vota::CGravaResultado` (u07) — only when `treinamentoEleitor` is 0, so **never in the web page** (the page stays
in voter-training mode and never reaches the encerramento; see codepath.md §1.3). The harness of
codepath.md did run it and produced a 711-byte BU that decodes with the recovered schema and passes the TSE's own
hash-chain check. Everything below is read from the two functions; line numbers are the `std::source_location`
lines of the original files.

### 3.1 Inputs (members of `CGravadorBU`, filled by `CGravaResultado`)

| input | origin (u07) |
|---|---|
| section: município, zona, local, seção, fase | `CLocal`, `CEstadoGeral` |
| dhGeração / dhEmissão | "now" / `EstadoGeralVota.dtHrEmissaoBU` (set by `CGeraBU`) |
| `CDadoCorrespondencia` | `EstadoGeral` +60: nº interno da urna, serial MC, data/hora e código da carga, gerador da mídia |
| histórico de códigos de carga | `CEstadoGeralGap`: the carga history of the section's voting, not every load of this urna. On a contingency urna it starts with the original section urna's carga, can include intermediate contingency urnas and ends with this urna's own carga; a re-load restarts it (2026 urna data: [investigation/README.md](../../investigation/README.md), findings D6, B15) |
| aptos per abrangência | `CEleitores::GetQtdAptos()` (`SQtdeAptos` = {seção, TTE}) |
| comparecimento | `CEleitores` +104 (asserted equal to `rdv.Comparecimento()`) |
| urna biométrica + 3 habilitação counters | `CLocal`, `CEleitores` (sem biometria / por biometria / por biografia) |
| `CDadosBUVota` | início / fim da aquisição de votos (`EstadoGeralVota`) |
| ordem dos cargos | first position of each cargo in `CCargos` (1-based) |
| `IRdv*` | `CRdvVota::GetInst()`: the **only** source of vote counts (no separate tally) |

### 3.2 Context values read by `GravaResultado`

1. `tipoUrna` = `CEstadoGeral` +36 (1º turno) or +40 (2º turno) — `'1'` seção, `'2'` contingência, `'3'`, `'4'`.
2. Serial of the voting flash (`numeroSerieFV`): if `/dsk/fe/estatico/infomidia.dat` exists
   (`CPath` estático of the **external** flash), `CFileASN::ReadFromFile<ModuloInformacaoMidia::InformacaoMidia>`
   and the hex of its serial bytes (func 1243); otherwise `"00000000"`. No scenario ships a plain `infomidia.dat`
   (they ship `infomidia-fv-1-t.dat` for the 1st-round scenarios and `infomidia-fv-2-t.dat` for the 2nd-round ones,
   in `/dsk/fi/estatico/`), so the web data gives `"00000000"`.
3. `cfg = CConfiguracaoEleicao::GetInst()`: pleito (+28), the elections (+52..+56, 52-byte items), parameters
   (+88 = `CParametrosUrna`; `criptografarBU` at cfg +160).

### 3.3 Vote lines (`MontaResultados`, inlined; `MontaVotosCargos`, func 3819; `AcrescentaVotoVotavel`, func 3820)

For **each election** of the pleito, in configuration order:

1. `cargos = cfg.GetCargos(eleição, true)` (func 3775); split with `copy_if` (func 3818) into majoritários
   (`CCargo::m_tipo` 0), proporcionais (1) and consultas (2).
2. Attendance per group = `rdv.Cargo(first cargo)` (IRdv slot 10, total of the office) **÷** `qtdEscolhas` of that
   cargo (integer division; 2 for a 2-seat Senate race). Majoritário and consulta share the majoritário figure
   (the consulta figure only when there is no majoritário); the proportional group has its own.
3. For each cargo of a group (`MontaVotosCargos`):
   * **cargo without candidates** (not a consulta and `CCandidaturas::PossuiCandidatos` false): a single line
     `NULO_CARGO_SEM_CANDIDATO` (8) with `rdv.Cargo(cargo)` votes, no identification;
   * otherwise, in this order:
     1. nominal lines: for each candidate number of the cargo (`CCandidaturas`, func 2272) — or each answer of a
        consulta (`CDetalheConsulta` respostas) — `AcrescentaVotoVotavel(NOMINAL, n, rdv.Candidato(cargo, n,
        nDígitos))` (IRdv slot 3);
     2. `BRANCO` (3) with `rdv.Brancos(cargo)` (slot 9) — only if > 0;
     3. `NULO` (4) with `rdv.Nulos(cargo)` (slot 8) — only if > 0;
     4. for a proportional cargo that has candidate details: `AcrescentaVotoVotavel(LEGENDA, p,
        rdv.Legenda(cargo, p))` (slot 4) for **every party of `CPartidos`**, in ascending party number.
   * `AcrescentaVotoVotavel` drops zero counts (unconditionally: the PU parameter `gravarZeradosBU`,
     `CParametrosUrna` +393, is never read by the BU writer, so a candidate or party with 0 votes never appears
     in `bu.dat`); blank/null/sem-candidato lines have no identification; a consulta
     line gets `{codigo, partido 0}`; otherwise the party is the candidate's (`CCandidaturas::Busca`), or, if the
     number is not a candidate, the number must exist in `CPartidos` (legenda) — else
     **8638 "Candidato {} / cargo {} não existe"** (cgravadorbu.cpp:221).
   * the cargo record is `{codigo, ordemImpressao = ordemCargos.at(codigo), linhas}` (`std::out_of_range
     "map::at:  key not found"` if the cargo is not in `CCargos`).
4. `CResultadoVotacao{tipo, comparecimento, cargos}` are appended in the order **proporcional, majoritário,
   consulta**, each only if the group has at least one cargo.
5. `CResultadoVotacaoPorEleicao{idEleição, aptosSeção, aptosTTE, resultados}` with the aptos of the election's
   abrangência (`map::at`).

### 3.4 Header, urna and entity (`MontaEntidadeBU`, cgravadorbu.cpp:636, inlined)

* `CCabecalhoEntidade(dhGeração, pleito, tipoId = 1 /*pleito*/)`.
* `CCorrespondenciaResultado(município, zona, seção, CCarga(...), seção ≠ 0 ? '1' : '2')`: município/zona/seção come
  from the `CDadoCorrespondencia` (+112/+116/+118), the `'1'`/`'2'` choice from the writer's own seção
  (`IResultado` +16); `CCarga`'s constructor (func 2802) runs `CCarga::ValidaCriacao`.
* `CUrna(tipoUrna, "10.23.0.1 - DESENVOLVIMENTO", correspondência, tipoArquivo '1' /*votacaoUE*/, serialMV
  [, motivo SA])` → `CUrna::ValidaCriacao` (func 5862): tipo ≠ '0' (8689), tipo de arquivo ≠ '0' (8690), serial of
  exactly 8 hex digits (8691/8692 "Serial da MV inválido [..]").
* `CEntidadeBU`: with `CDadosBUVota` (VOTA) or `CDadosBUSA` (Sistema de Apuração); neither →
  **8641 "Dados específicos do aplicativo não presentes"**. `detalhamentoComparecimento` (the 3 habilitação
  counters) only on a **biometric** urna. `CEntidadeBU::ValidaCriacao` (centidadebu.cpp:159–177): fase `== '0'` or
  `>= '4'` (signed compare; a value below `'0'` would pass) → 8665, município < 100000 (8666), zona < 10000 (8667),
  local < 10000 (8668), seção < 10000 (8669).

### 3.5 ASN.1 conversion and the hash chain (`CConversorEntidadeBU::DoConverte`, func 10273)

`EntidadeBoletimUrna` fields, in the order they are set:

| field | value |
|---|---|
| `cabecalho` | `CConversorCabecalhoEntidade` (dataGeração, idEleitoral = pleito) |
| `fase` | `Utils::ConverteFase` |
| `urna` | `CConversorUrna::DoConverte` (func 10287): tipoUrna `'1','2','3','4'` → 1, 3, 4, 6 (8627); versão; correspondência (func 10289: identificação de seção `{município, zona, local = constant 1, seção}` or of contingência when seção 0 / tipo `'2'`; `'0'` → 8601; tipo > `'4'` leaves it unset); tipoArquivo `'1'..'6'` → 1..6 (8629); `numeroSerieFV` = 4 bytes from the 8 hex chars (func 3605; 8631/8632); motivo SA optional |
| `identificacaoSecao` | município, zona, local, seção |
| `dataHoraEmissao` | `"YYYYMMDDThhmmss"` (comum_f1080) |
| `dadosSecaoSA` | CHOICE `[0] dadosSecao {abertura, encerramento, desligamentoVotoImpresso OPTIONAL}` (VOTA) or `[1] dadosSA {junta, turma, numeroInternoUrnaOrigem OPTIONAL (omitted when 0)}`; neither → **8608 "Objeto de dados do BU mal formado"** (:232) |
| `qtdEleitoresCompareceram` | CEntidadeBU +174 |
| `detalhamentoComparecimento` | optional (biometric urnas) |
| `resultadosVotacaoPorEleicao` | below |
| `historicoCodigosCarga` | the carga history of the section's voting (§3.1), as `GeneralString` (2026 urna data: [investigation/README.md](../../investigation/README.md), findings D6, B15) |
| `historicoVotoImpresso` | optional, only when non-empty (never in VOTA: printed-vote hardware history) |

The header must identify a **pleito** (tipo 1), else **8607 "Erro no hash encadeado: Tipo de identificador do
cabeçalho da entidade BU precisa ser um pleito."** (:175). For each `ResultadoVotacaoPorEleicao`:

1. **initial hash** = SHA-512 of `"{pleito:05}|{eleição:05}|{município:05}|{zona:04}|{seção:04}|{códigoCarga:24}"`
   (format string @8483; `CalculaHash`, func 3602, over `ecourna::api::security::CSha512`). The running value is a
   static `std::vector` @1910004 of the (1-byte) `CAssinaVotavelBu` singleton (func 3603);
2. `idEleicao`; `qtdEleitoresAptos = aptosSeção + aptosTTE`; `qtdEleitoresAptosSecao`; `qtdEleitoresAptosTTE`;
3. for each `ResultadoVotacao`: `tipoCargo` = C++ tipo + 1 (majoritário 1, proporcional 2, consulta 3; ≥ 3 → 8610),
   `qtdComparecimento`, then for each cargo `TotalVotosCargo{codigoCargo (CConversorCodigoCargoConsulta),
   ordemImpressao, votosVotaveis}`; the per-cargo counter `ordemGeracaoHash` restarts at 1;
4. each `TotalVotosVotavel`: `tipoVoto` through the table @546352 (RDV 1..9 → BU `legenda 4, nominal 1, branco 2,
   nulo 3, branco 2, nulo 3, nulo 3, cargoSemCandidato 5, cargoSemCandidato 5`; out of range → 8609),
   `quantidadeVotos`, optional `identificacaoVotavel{partido, codigo}`, `ordemGeracaoHash`, and
   **hash(n) = SHA-512(`"{HEX(hash(n−1))}|{ordem}|{cargo}|{tipoVoto}|{quantidade}|{codigo}|{partido}"`)**
   (@1120; without identification the 5-field form @1126). HEX is upper case, the cargo is the C++ cargo code byte,
   `tipoVoto` is the **BU** value. This is exactly TSE's `calcula_hash_votacao` (`bu_assinatura_tuplas.py`);
5. `ultimoHashVotosVotavel` = the last hash (64 bytes) and `assinaturaUltimoHashVotosVotavel` =
   `IPkcs11` slot 23 (open session) → slot 6 (sign, given the raw 64-byte last hash) → slot 24 (close)
   (`CAssinaVotavelBu::Assinar`, cassinavotavelbu.cpp:89). No `IPkcs11` is linked in the web build (the lookup
   throws "PolySingleton - solicitada uma instancia nao criada N3api6pkcs117IPkcs11E").

Every sub-object is checked with `isValid() && isStrictlyValid()`; failures throw **8603..8606 "Entidade
inválida"** (lines 87, 110, 132, 164), and the converter frame throws 7653 "Entidade deixada em estado inválido:
{}" (iconversorasn.h:56) with the asn1 trace.

### 3.6 Encoding, optional encryption, envelope, file

1. `CFileASN::CodeObjectFunction` encodes the `EntidadeBoletimUrna` (5955 "Objeto com conteúdo inválido para {}: {}",
   5956 "Arquivo não foi codificado para …").
2. **Encryption** only if `CGravadorBU +212` (always true) **and** the PU parameter `criptografarBU` (False in all
   scenarios): 1024-byte crypto table from `api::IUrna` slot 4, 32 random bytes from `ecourna::api::security::IRng`
   slot 4, the public key `/dsk/fi/estatico/chave/bu.pk1` (an `EntidadeChave` whose content is deciphered with a
   key-encryption key from `api::IKernelHSM` slot 3 through `CSymmetricCipherFactory`; **8639 "O arquivo … não
   existe"**, **8640 "O arquivo … está vazio"**), `CPlainText(zona, seção, tabela, aleatório, chave, bytes)` (func
   5168) → `CCepescCipher::vf2` → `CSeguranca(0, 1, …)`. (In the web build CEPESC is a mock that does not encrypt,
   u01/u11.)
3. **Envelope** `EntidadeEnvelopeGenerico{cabecalho (same), fase, identificação (seção, or contingência), tipoEnvelope
   = envelopeBoletimUrna (1), [seguranca], conteudo = the BER bytes (or the ciphertext)}` — without the optional
   `urna` (constructors comum_f5861 / comum_f3821, both followed by `CEnvelopeGenerico::ValidaCriacao`, func 3822:
   8678..8684). It is written with `CFileASN::CodeObjectFunction` into the `api::CFile` opened by `IGravador::Grava`
   (`trab1/<nome>-bu.dat` on the MI).
4. `IGravador` then copies it to the MI result directory, to the MV work and result directories, and
   `CAssinador::AssinaArquivosResultado` has SAVD sign it into `…-vota.vsc` (u07).

### 3.7 What protects the BU, as far as this code goes

* hash chain + PKCS#11 signature of the last hash (per election) — *inside* the file;
* the SAVD signature of the whole file (`vota.vsc`) — outside;
* the optional CEPESC encryption (off in the scenarios);
* in this build: no PKCS#11, a SAVD mock that signs nothing, a CEPESC mock (see §9).

The printed BU, its QR codes and its check codes are produced elsewhere (`CGeraBU`, u08; `CGeradorBUQRCode`, u04).

## 4. Attendance record of the section: `CGravadorRCSecao` (`…-jufa.dat`, func 11616)

`EntidadeResultadoUrnaCadastro` (the `ecourna::app::dados` classes are reconstructed by u14):

1. header: the generation date/time is formatted `"{:04}{:02}{:02}T{:02}{:02}{:02}"` (@8913), parsed back with
   `boost::posix_time::from_iso_string`, and given to `CCabecalhoEntidade(ptime, pleito, 1)`;
2. `CIdentificacaoSecaoEleitoral(município ≤ 99999, zona, local, seção ≤ 9999)` (range-checked `CBaseType`s);
3. for every voter of the roll (`CEleitores`, map keyed by the voter's identity) that is *apto* in the current turn
   (`comum_f2266`: impedimento of the turn == 0), a `CEstadoComparecimento`:
   `CRegistroIdentificacaoEleitor(identidade estática, identidade de habilitação)` (`CriaIdentidadeEleitor`, func
   5845: título → `CNumeroInscricaoEleitoral`, CPF → `CNumeroCPF`, livre → `CNumeroIdentificacaoLivre`, else 8654
   "Tipo de identidade invalida: {}"), then by the dynamic situation: 0 → only the situation; 1 → + photo + audio;
   otherwise by enabling type: 0 (no biometrics) → + photo + audio; 1 (fingerprint) → + `CHabilitacaoBiometrica`
   (attempts, finger 0..999, score, read error); 2 (mesário code) → + biometria + `CEstadoHabilitacaoPorCodigo`
   (the mesário's identity — número and tipo — is looked up among the opening-period rows of the
   `comparecimento_mesario` table); other types are skipped;
4. justifications (`registro_justificativa`: título + birth year), read only if that repository singleton
   (@1839012) already exists; mesários of the opening (período 1) and closing (período 2), two separate filtered
   passes over the `comparecimento_mesario` repository (DAO rows of 52 bytes). Both mesário lists are **always
   present** in the entity (optional flags set unconditionally), empty when the persistence is not initialised;
5. `CDadosComparecimento`; if the PU parameter `criptografarJUFA` (cfg +161): BER-encode it, draw 32 random bytes
   (`IRng`), read `/dsk/fi/estatico/chave/jufa.pk1` (same scheme as `bu.pk1`; 8655/8656), encrypt with CEPESC and
   store `DadosComparecimentoCifrado{DadosCifracao{chave, salt, informação adicional}, conteúdo}`; else the clear
   data;
6. `CResultadoUrnaCadastro(cabeçalho, fase ecourna ('o'→2, 's'→1, 't'→3), "10.23.0.1 - DESENVOLVIMENTO",
   ArquivoFinal, dados)` written with `CFileASN`.

## 5. Installation hashes: `CGravadorHashes` (`…-hash.dat`, func 11620)

`md::CEntidadeHashes{cabeçalho, fase, UF (2 letters, 8673), identificação (contingência: município+zona, or seção:
município+zona+local+seção; neither → 8644 "Tipo de urna inválido"), versão, list of {caminho, hash}}`, validated by
func 5855. The list = `CMontadorHash::CalculaHashGeral(CPath root)` + `CalculaHashGeral("/dsk/fi/estatico/chave/")`,
each flattened by funcs 5358/5359. Excluded directories: `root/dev/`, `root/dsk/`, `root/proc/`, `root/sys/`,
`root/tmp/`, `MI/dinamico/`, `MV/dinamico/`, `MI/dinamico/tmp/`. So the file covers the **system files and the
keys**, not the election data under `/dsk/fi/estatico/` (only its `chave/` subdirectory is added back). In the web
build no `IGenericFactory<IHash>` is linked: this writer is where the harness run of codepath.md stops.

## 6. Fingerprint packages: `CGravadorWSQ` (`wsqbio.jez` / `wsqman.jez` / `wsqmes.jez`)

Constructor func 3796 (ValidaTipoBiometria inlined: tipo < 3, else 8651). The class overrides the *copy* slots and
leaves `Grava`/`GravaMV`/`GravaResultado` empty: `CopiaParaResultado` (11579) zips the MI images straight into the
MI result directory; `CopiaParaMV` (11577) / `CopiaResultadoParaMV` (11578) zip the MI / MV images into the MV result
directory, only when the flag `+84` is set (u07 passes `!EhFaseTreinamento()`). The zip itself is `CompactaWsq`
(func 5821, u12): level "store", `*.wsq` of `habilitado/`, `nao-habilitado/` or `operador/`. The member `+56 =
"wsq.pk1"` (a key file name) is never used by this code: the packages are not encrypted here.

## 7. Data models (`md::`) and converters: validation summary

| check | code | where |
|---|---|---|
| fase `'o'/'s'/'t'` | 8657 | IGravador ctor (iresultado.cpp:40) |
| município < 100000, zona < 10000, seção < 10000 (file name) | 8645/8646/8647 | cgravadorutil.cpp:28/33/38 |
| fase → `'1'/'2'/'3'` | 8649 "Fase inválida: {:#x}" | cgravadorutil.cpp:97 |
| serial flash 8 hex chars / 4 bytes | 8631/8632/8633 | asn/util.cpp:26/31/54 |
| envelope: fase, município, zona, local, tipo de urna, seção, tipo de envelope (< 5) | 8678..8684 | md/cenvelopegenerico.cpp:127..156 |
| envelope getters: "Não há informação de urna / de local / de criptografia" | 8675/8676/8677 | md/cenvelopegenerico.cpp:96/106/117 |
| urna: tipo, tipo de arquivo, serial MV | 8689..8692 | md/curna.cpp:58..69 |
| hashes: fase, UF | 8672/8673 | md/centidadehashes.cpp:59/62 |
| versões de arquivos: tag, list | 8694/8695 | md/cversoesarquivos.cpp:28/33 |
| properties lookup | 8663 / 8697 "Propriedade inexistente: {}" | md/cdependenciascontratos.cpp:59, md/cversoescontratos.cpp:42 |
| envelope ↔ ASN: tipo de urna, tipo de envelope, urna+criptografia together | 8611..8615 | asn/cconversorenvelopegenerico.cpp:72..191 |
| urna ↔ ASN: tipo urna / tipo arquivo | 8627..8630 | asn/cconversorurna.cpp:77..139 |
| apuração SA: tipo, motivos (manual 1–3/99, eletrônica 1–3/99, mista BU 1–5/99, mista MR 1–6/99) | 8621..8626, 8687/8688 | asn/cconversortipoapuracaosa.cpp, md/ctipoapuracaosa.cpp |
| correspondência ↔ ASN | 8601/8602 | asn/cconversorcorrespresultado.cpp:53/85 |

`TipoEnvelope` mapping (C++ 0..4 ↔ ASN.1): BU 1, RDV 2, BU impresso 4, imagem de biometria 5 (?), zerésima impressa 6
(tables @546580 / @546600).

## 8. `IInterfaceInit`: requests to the init service

Every method sends one command through the pure virtual `EnviarMensagem(cmd)` (slot 5) and reads a
`std::vector<int>`; an empty answer throws "Resposta nula".

| method (func) | command | failure | web answer (read from `simulador::CWasmInit::EnviarMensagem`, func 9405, which logs `"CWasmInit::{} {}"` to a sink that the headless run does not show) |
|---|---|---|---|
| `CurrentBootDevice` (11646) | 37 | 7231 | overridden: always 1 |
| `IsMRPresenteSemHabilitar` (2862) | 18 | 7237 | 1 (`"= no"`) → false |
| `IsMRMontadoSemHabilitar` (5897) | 17 | 7238 | 1 → false |
| `MontarMRSemHabilitar` (5896) | 34, then 10, **sleep 500 ms**, 12 | 7239/7240, 7282/7283, 7293/7294 | 0 / 0 / 0 (device 0: no serial) |
| `DesmontarMRSemDesabilitar` (3832) | 58 | 7241/7242 | 0 |
| `DesligarUrna` (5894) | logs "Urna desligada a pedido do aplicativo", 48 | 7268/7269 | 0 (nothing is powered off) |
| `EnviarMensagemThrowVoid` (3833) | any | 7293 "{}: Resposta nula", 7294 "Falha ao enviar mensagem {}, {}: {}" | — |
| `GetDemoMode` (729) | 67, cached in a `shared_ptr<bool>` | 7295/7296 | 0 (`"= 0"`) → not demo |

`MontarMRSemHabilitar` also reads the MR's size (`/sys/block/sdX/size × queue/logical_block_size`) and USB serial
from a model-specific sysfs path (`api::IUrna` slot 0 = model): 2009/2010/2011/2013 → `…/0000:00:1d.8/usb1/1-0` or
`1-8`; 2015 → `…/1d.7/usb1/1-0` or `1-7`; 2020/2022 → `…/15.0/usb1/1-3` or `usb2/2-3`; other models → no serial. It
logs "Tamanho da MR: {}" and (syslog/`DEBUG_UENUX`) "Serial da MR: %s" when a new MR is seen.

## 9. `IInterfaceSavd`: the signing service protocol

Request = 8-byte header `{0xFE, aplicação, u16 comando, u32 tamanho}` + payload; answer = 12-byte header `{0xFE,
erro, pad, u32 código, u32 tamanho}` + (on error) message `{0xFE, texto}`. Two exchange helpers: func 3830 (throws on
protocol errors 7305..7309, "Motivo não informado pelo serviço." when no text; the error code is written through an
explicit out-parameter, `&m_codigoErro` at every call site) and func 3829 (returns the error text).

| request | comando | payload | caller |
|---|---|---|---|
| sign a dynamic file (`AssinarUE`) | 0x0042 | `{0xFE, '=', arquivo}` | `CAssinador::Assina` (1277) ← `SalvaEstado` (491), `CSincronizaVota`, … — **observed at start-up and during the recorded votes** |
| sign a result file (`AssinarEcourna`) | 0x0080 | `{0xFE, '=', arquivo}` | `CAssinador::AssinaArquivosResultado` (u07) |
| HSM session open/close (`EnviarAcaoHSM`, 5889) | 0x1604 | `{0xFE, 0 or 1}` | same |
| validate a package's UE signature (`ValidarUE`, 5890) | 0x2021 | TLV from api_f3831 | `vota::VerificaAssinaturaMV/MI` |
| define the section "local" | 0x0404 | `{0xFE, 15 chars}` | u07 (inlined) |

Errors: 7336 "Falha ao assinar ({}-UE[{}])", 7340 "Falha ao enviar ação ao HSM. {}", 7323 "Falha ao validar
assinatura UE\npacote: ({})\nErro SAVD: ({})\n{}". `CAssinador::Assina` wraps the call in an
`api::CApplicationContextGuard(ReinicieOuFotografeQRCode, "Erro na assinatura", "", "Ocorreu um erro durante a
assinatura do arquivo: " + nome)` so a failure is shown to the operator with that text.

## 10. `CInformacaoEleicao`

A view over `CParametrosUrna` (cfg +88). In demonstration mode every count is 1 and mesário registration is off;
otherwise the PU values: `numBUVotaObrigatorios` (3847), `numBUVotaAdicionais` (5914), `numRelatorioEstado` (2865,
seen executing: the "Emissão do estado da urna" screen), `numRelatorioEleitores` (5917), `numRelatorioVersoesDados`
(5916), `numRelatorioPU` (5915), `registrarMesarios` (1950). `EhModoDemonstracao` (2286) is the static query itself.
The tools named all eight functions `EhModoDemonstracao` because that is the only srcloc (line 40) in them.

## 11. What is specific to the web build

* **SAVD is a mock** (`(anonymous)::CWasmSavd`, registered by `main`): `EnviaMensagem` does nothing and
  `RecebeMensagem` (func 10949) returns the 12-byte OK header for any request (and throws `std::invalid_argument`
  when asked for any length other than 12), and stores 0x0CABECA0 (the value slot 4 returns) in its `estado`
  out-parameter. Every `AssinarUE`, HSM action and `ValidarUE` therefore **succeeds without anything being
  signed or verified**; the `.vsu` files of the web build are the fake text written by `votaInit`.
* **Init service is a mock** (`simulador::CWasmInit`): no MR is ever present, the demonstration flag is off, power-off
  does nothing (§8).
* The BU / RC / hash writers are unreachable from the page (no encerramento; `CGravaResultado` needs
  `treinamentoEleitor == 0`). Even if reached: no `IPkcs11`, no `IKernelHSM`, no `IGenericFactory<IHash>`, no key
  files, 0-byte `/etc/*.properties` (docs/bu/codepath.md §6).
* Functions of this unit seen executing during the recorded votes: 1277 + 5891 + 3829 (`SalvaEstado` signing, ≈14
  samples), 729 + 2865 (the zerésima-time screen asks how many "estado da urna" copies are allowed), 3830 (a SAVD
  file check at start-up, via func 5892), 3604 (serial conversion while loading the state files, caller 11400) and 3605
  (while saving them, caller 11399 `CConversorDadoCorrespondencia::DoConverte`), 819
  (`CPartidos`), 276 (string assign).

## 12. wasm / Emscripten observations

* **LTO inlining hides whole functions.** `CGravadorBU::GravaResultado` (15.9 KB) contains `MontaEntidadeBU`,
  `CEntidadeBU::ValidaCriacao`, `LeChavePublica` and the vote-grouping loop; `CGravadorRCSecao::GravaResultado`
  (15.8 KB) contains `LeChavePublica` and the attendance builder; `CConversorEntidadeBU::DoConverte` (12.2 KB)
  contains six helpers of cconversorentidadebu.cpp and all of cassinavotavelbu.cpp.
* **Misleading tool names** corrected here: 11616 "LeChavePublica" and 11629 "vf7" are `GravaResultado`; 1396
  "IResultado::IResultado" is `IGravador`'s ctor; 1277 "AssinarUE" is `CAssinador::Assina`; 3830
  "DesconverteMensagem" is the SAVD request; 3796 "ValidaTipoBiometria" and 5867 "ValidaCriacao" are constructors;
  10284/10287 are `CConversorUrna::DoDesconverte/DoConverte`; the eight "EhModoDemonstracao" are different getters.
* **merge-similar-functions**: `CVersoesContratos::GetValor`/`CDependenciasContratos::GetValor` → one body (6044)
  taking srcloc and error code; `IsMRPresente`/`IsMRMontado` → 6046; the two error-class constructors → ecourna_f710.
* **ICF**: 12090 is the destructor of `IResultado`, `IGravador` and `CGravadorRCSecao`; 4694 is shared by
  `CAssinador`/`CAssinadorVota`; 2900 is the lazy creator of every 1-byte singleton.
* The 500 ms wait in `DispositivoMR` compiles to `if (byte@1584624 == 1) emscripten_sleep(500)`. It cannot be
  `std::this_thread::sleep_for` (libc++ routes that to `nanosleep`, a spin loop in this build). The byte @1584624 is a
  static set to 1 in the data segment and no code stores to it (24 functions only read it), so the sleep always runs,
  and without ASYNCIFY the glue's `_emscripten_sleep` aborts the module.
* `std::format` argument packing is visible in the constants: e.g. 3491 = (int, string_view, int) for "Falha ao
  enviar mensagem {}, {}: {}". Data addresses written `d_…[N]` in the decompiled text are `N + 1024` absolute.

## 13. Open questions

* Names of the `IPkcs11` slots (23/6/24) and of the SAVD command codes (0x42, 0x80, 0x404, 0x1604, 0x2021); the
  magic returned by `CWasmSavd` slot 4 (0x0CABECA0).
* Whether the PKCS#11 module pre-hashes the 64-byte last hash (TSE's reader verifies `SHA-512(ultimoHash)`, see
  docs/10-boletim-de-urna.md §4.2).
* Exact semantic of the `CEleitorDinamico` situation values used by the jufa writer (0 absent, 1 …) and of the
  mesário-recognition search (the period filter is a guess).
* `IInterfaceInit` slots 3, 4, 6, 7 (only the web overrides are visible).
* The byte at `CUrna +108` and the exact `CDadoCorrespondencia` layout (u05/u07 disagree on minor fields).

## 14. Weird or risky code

| # | func | what | impact |
|---|---|---|---|
| 1 | 3829, 5891, 5889, 5890, 1277, 10949 | **Signing and signature validation are simulated**: `CWasmSavd` answers "OK" to every SAVD request, so `AssinarUE`, the HSM session, `ValidarUE` and the start-up file checks all succeed without any cryptography | simulator only; anyone studying the web build must not read these as real security checks |
| 2 | 5896 | `DispositivoMR` sleeps 500 ms through `emscripten_sleep` (guarded by a static byte that is always 1), which aborts the module (no ASYNCIFY) | unreachable in the page (the mock says no MR is present, and the encerramento never runs); would crash the simulator if reached |
| 3 | 5896 | MR size = `int` sectors × block size in 32 bits: overflows for media > 2 GiB; only the log line "Tamanho da MR" is wrong | real urna if the same code: wrong log value only |
| 4 | 3829 | the answer message is read without checking the received length (`msg[0]`, then `msg+1 .. msg+tamanho`): a SAVD that declares a longer message than it sends makes the client read past the buffer | real urna: heap over-read limited to a trusted local daemon; web: never triggered (the mock never reports an error) |
| 5 | 11620 | `hash.dat` excludes `root/dsk/` (and adds back only `/dsk/fi/estatico/chave/`), so the static election data of the MI and all of the MV are not in the installation hashes | informative: `hash.dat` covers the system and keys, not the election data files (those have their own signatures) |
| 6 | 11629 / 3819 | `std::map::at` on the cargo order and on the aptos per abrangência: a cargo missing from `CCargos` or an abrangência without aptos throws `std::out_of_range` ("map::at:  key not found"), not a TSE error, in the middle of the encerramento | real urna: only with inconsistent election data; would abort BU generation with a generic message |
| 7 | 11629 | attendance written in the BU is recomputed as `rdv.Cargo(1st cargo) / qtdEscolhas`, not taken from the roll; consultas reuse the majoritário figure | consistent in practice (asserted equal to the roll count before), but a tally-derived value |
| 8 | 10273 | the hash chain lives in a static vector (@1910004) shared by all conversions and reset per election; a second concurrent conversion would interleave chains | single-threaded in practice; no impact observed |
| 9 | 3604 | `DesconverteSerialFlash` formats all bytes before checking that there are exactly 4; the error text shows the formatted output, not the input | cosmetic |
| 10 | 1396 | the file name is built (with the raw fase char) before the fase is validated | cosmetic: the object is not created anyway |
| 11 | 10271 / 10289 | an unknown `tipoUrna` (> `'4'`) leaves the envelope/correspondência identification unset instead of throwing; the ASN.1 validity check catches it later with a less precise message | defensive gap only |
| 12 | 3796 / 6041 | `CGravadorWSQ` stores the key file name `"wsq.pk1"` but never uses it: fingerprint packages are only zipped (store level), not encrypted, in this code | informative (the images are biometric personal data; their protection relies on the MR/transmission layers) |
| 13 | 11629 | BU encryption depends on the PU parameter `criptografarBU` (False in every scenario); `CGravadorBU +212` is always true | informative |
| 14 | 5896 | MR serial read only for urna models 2009/2010/2011/2013/2015/2020/2022; other models log an empty serial | real urna: other models get no MR serial in the log |
| 15 | 11629 | attendance per group is `rdv.Cargo(cargo) / cargo.qtdEscolhas` (`i32.div_u` by the byte at `CCargo` +13) with no zero check: election data with a cargo whose `qtdEscolhas` is 0 makes the encerramento trap ("integer divide by zero") instead of throwing a TSE error | real urna (same source): undefined behaviour/crash on malformed election data only; web: unreachable |
| 16 | 3820 | zero-vote lines are dropped unconditionally; the PU parameter `gravarZeradosBU` (+393) is not consulted by this writer | informative: whoever reads the BU must treat a missing candidate/party as 0 votes |
| 17 | 10289 | the `urna.correspondenciaResultado` identification of a seção urna always carries the constant local `1` (the C++ model has no local); only the envelope carries the real local | informative: readers must take the local from the envelope/`identificacaoSecao`, not from the correspondência |
| 18 | 11629 / 3822 / 5855 | the fase checks of `CEntidadeBU`, `CEnvelopeGenerico` and `CEntidadeHashes` reject only `'0'` and values `>= '4'` (signed), not everything outside `'1'..'3'` | defensive gap only (the fase always comes from `ConverteFase`, which yields `'1'..'3'`) |

## 15. Mapping table: every function of the unit

`ran` = seen executing in the recorded votes (analysis/runtime). "library instantiation" rows are libc++ templates (vector growth, uninitialized copy, tree destroy) or compiler-generated code that belong to the file where they were instantiated; they carry no TSE logic and are only summarised in the reconstruction. Paths under `…/comum/` are `src/uenux2/src/app/comum/`.

| func | bytes | ran | reconstructed symbol | original file | reconstruction | conf. |
|---:|---:|:-:|---|---|---|---|
| 276 | 14 | ✔ | `std::string::assign(const char*)` | library (libc++ <string>) | - | medium |
| 283 | 18 |  | `ecourna::api::exception::CBaseError<comum::EUeComumGravadoresError, SErrorLimits{8600,8800}>::CBaseError` | library/inlined helper: ecourna-lib/ecourna/api/exception/cbaseerror.hpp (merged ctor thunk) | …/comum/gravadores/u23-foreign-fragments.cpp | high |
| 480 | 18 |  | `ecourna::api::exception::CBaseError<comum::EUeComumError, SErrorLimits{7200,7600}>::CBaseError` | library/inlined helper: ecourna-lib/ecourna/api/exception/cbaseerror.hpp (merged ctor thunk) | …/comum/gravadores/u23-foreign-fragments.cpp | high |
| 729 | 736 | ✔ | `comum::IInterfaceInit::GetDemoMode` | uenux2/src/app/comum/iinterfaceinit.cpp | …/comum/iinterfaceinit.cpp | high |
| 819 | 84 | ✔ | `comum::CPartidos::GetInst` | uenux2/src/app/comum/dados/cpartidos.cpp (path inferred) | …/comum/gravadores/u23-foreign-fragments.cpp | medium |
| 1007 | 89 |  | `comum::CAssinador::~CAssinador` | uenux2/src/app/comum/gravadores/cassinador.cpp | …/comum/gravadores/cassinador.cpp | high |
| 1274 | 18 |  | `comum::CPath::GetPathResult(EFlashOrigem) [current turn]` | uenux2/src/app/comum/cpath.cpp | …/comum/gravadores/u23-foreign-fragments.cpp | medium |
| 1277 | 870 | ✔ | `comum::CAssinador::Assina` | uenux2/src/app/comum/gravadores/cassinador.cpp | …/comum/gravadores/cassinador.cpp | high |
| 1396 | 572 |  | `comum::IGravador::IGravador` | uenux2/src/app/comum/gravadores/iresultado.cpp (IResultado ctor inlined; IGravador ctor, file of IGravador inferred) | …/comum/gravadores/iresultado.cpp | medium |
| 1556 | 224 |  | `std::vector<comum::md::CVotosCargo>::vector(first,last,n)` | library instantiation (cgravadorbu.cpp) | …/comum/gravadores/cgravadorbu.cpp (comment) | medium |
| 1944 | 282 |  | `std::vector<comum::md::CVotosVotavel>::push_back` | library instantiation (cgravadorbu.cpp) | …/comum/gravadores/cgravadorbu.cpp (comment) | medium |
| 1950 | 72 |  | `comum::CInformacaoEleicao::IdentificaMesarios` | uenux2/src/app/comum/informacao/cinformacaoeleicao.cpp | …/comum/informacao/cinformacaoeleicao.cpp | medium |
| 2192 | 379 |  | `ecourna::app::dados::CEstadoComparecimento::CEstadoComparecimento(id,situacao,foto,audio,biometria)` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cestadocomparecimento.cpp (path inferred) | …/comum/gravadores/u23-foreign-fragments.cpp | high |
| 2193 | 176 |  | `ecourna::app::dados::CEstadoComparecimento::CEstadoComparecimento(id,situacao,foto,audio)` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cestadocomparecimento.cpp (path inferred) | …/comum/gravadores/u23-foreign-fragments.cpp | high |
| 2274 | 506 |  | `comum::CGravadorUtil::ConverteFase` | uenux2/src/app/comum/gravadores/cgravadorutil.cpp | …/comum/gravadores/cgravadorutil.cpp | high |
| 2278 | 423 |  | `std::__uninitialized_allocator_copy<ecourna::app::dados::CEstadoComparecimento>` | library instantiation | …/comum/gravadores/cgravadorrcsecao.cpp (comment) | medium |
| 2286 | 52 |  | `comum::CInformacaoEleicao::EhModoDemonstracao` | uenux2/src/app/comum/informacao/cinformacaoeleicao.cpp | …/comum/informacao/cinformacaoeleicao.cpp | high |
| 2802 | 329 |  | `comum::md::CCarga::CCarga` | uenux2/src/app/comum/dados/md/correspondencia/ccarga.cpp | …/comum/gravadores/cgravadorbu.cpp (comment) | medium |
| 2845 | 394 |  | `std::__uninitialized_allocator_move_if_noexcept<ecourna::app::dados::CEstadoComparecimento>` | library instantiation | …/comum/gravadores/cgravadorrcsecao.cpp (comment) | medium |
| 2853 | 423 |  | `std::__uninitialized_allocator_copy<comum::md::CResultadoVotacaoPorEleicao>` | library instantiation | …/comum/gravadores/cgravadorbu.cpp (comment) | medium |
| 2856 | 39 |  | `comum::md::CVotosVotavel::CVotosVotavel(tipo,codigo,quantidade)` | uenux2/src/app/comum/gravadores/md/cvotosvotavel.h (path inferred) | …/comum/gravadores/cgravadorbu.h | medium |
| 2862 | 17 |  | `comum::IInterfaceInit::IsMRPresenteSemHabilitar` | uenux2/src/app/comum/iinterfaceinit.cpp | …/comum/iinterfaceinit.cpp | high |
| 2865 | 72 | ✔ | `comum::CInformacaoEleicao::GetNumRelatorioEstado` | uenux2/src/app/comum/informacao/cinformacaoeleicao.cpp | …/comum/informacao/cinformacaoeleicao.cpp | medium |
| 2900 | 61 |  | `comum::(anonymous)::GetInstLazy<1-byte T> (ICF)` | uenux2/src/app/comum/gravadores/asn/cconversorentidadebu.cpp (ICF body shared with comum_f348/948/2742) | …/comum/gravadores/asn/cconversorentidadebu.cpp (comment) | low |
| 3602 | 290 |  | `comum::asn::(anonymous namespace)::CalculaHash` | uenux2/src/app/comum/gravadores/asn/cconversorentidadebu.cpp | …/comum/gravadores/asn/cconversorentidadebu.cpp | medium |
| 3603 | 15 |  | `comum::asn::CAssinaVotavelBu::GetInst` | uenux2/src/app/comum/gravadores/asn/cassinavotavelbu.cpp (path inferred) | …/comum/gravadores/asn/cconversorentidadebu.cpp | low |
| 3604 | 585 | ✔ | `comum::asn::DesconverteSerialFlash` | uenux2/src/app/comum/gravadores/asn/util.cpp | …/comum/gravadores/asn/util.cpp | high |
| 3605 | 748 | ✔ | `comum::asn::ConverteSerialFlash` | uenux2/src/app/comum/gravadores/asn/util.cpp | …/comum/gravadores/asn/util.cpp | high |
| 3751 | 57 |  | `comum::CPartidos::CPartidos` | uenux2/src/app/comum/dados/cpartidos.cpp (path inferred) | …/comum/gravadores/u23-foreign-fragments.cpp | medium |
| 3796 | 268 |  | `comum::CGravadorWSQ::CGravadorWSQ` | uenux2/src/app/comum/gravadores/cgravadorwsq.cpp | …/comum/gravadores/cgravadorwsq.cpp | high |
| 3798 | 2070 |  | `comum::CGravadorUtil::DeterminaNomeArquivo` | uenux2/src/app/comum/gravadores/cgravadorutil.cpp | …/comum/gravadores/cgravadorutil.cpp | medium |
| 3814 | 32 |  | `std::__tree<std::pair<TCargoID,uebyte>>::destroy` | library instantiation (cgravadorbu.cpp) | …/comum/gravadores/cgravadorbu.cpp (comment) | medium |
| 3815 | 278 |  | `ecourna::app::dados::CInformacaoMidia::~CInformacaoMidia` | library/inlined helper (implicit dtor of the infomidia.dat model) | …/comum/gravadores/cgravadorbu.cpp (comment) | low |
| 3816 | 560 |  | `std::__uninitialized_allocator_move<comum::md::CCargo>` | library instantiation | …/comum/gravadores/cgravadorbu.cpp (comment) | medium |
| 3817 | 796 |  | `std::vector<comum::md::CResultadoVotacao>::__push_back_slow_path` | library instantiation | …/comum/gravadores/cgravadorbu.cpp (comment) | medium |
| 3818 | 659 |  | `comum::(anonymous)::FiltraCargosPorTipo (std::copy_if)` | uenux2/src/app/comum/gravadores/cgravadorbu.cpp | …/comum/gravadores/cgravadorbu.cpp | medium |
| 3819 | 3993 |  | `comum::CGravadorBU::MontaVotosCargos` | uenux2/src/app/comum/gravadores/cgravadorbu.cpp | …/comum/gravadores/cgravadorbu.cpp | medium |
| 3820 | 749 |  | `comum::CGravadorBU::AcrescentaVotoVotavel` | uenux2/src/app/comum/gravadores/cgravadorbu.cpp | …/comum/gravadores/cgravadorbu.cpp | high |
| 3822 | 432 |  | `comum::md::CEnvelopeGenerico::ValidaCriacao` | uenux2/src/app/comum/gravadores/md/cenvelopegenerico.cpp | …/comum/gravadores/md/cenvelopegenerico.cpp | high |
| 3824 | 62 |  | `comum::md::CResultadoVotacao::CResultadoVotacao` | uenux2/src/app/comum/gravadores/md/cresultadovotacao.h (path inferred) | …/comum/gravadores/cgravadorbu.h | medium |
| 3825 | 20 |  | `comum::md::CVersoesContratos::GetValor` | uenux2/src/app/comum/gravadores/md/cversoescontratos.cpp | …/comum/gravadores/md/cversoescontratos.cpp | high |
| 3826 | 20 |  | `comum::md::CDependenciasContratos::GetValor` | uenux2/src/app/comum/gravadores/md/cdependenciascontratos.cpp | …/comum/gravadores/md/cdependenciascontratos.cpp | high |
| 3829 | 1112 | ✔ | `comum::IInterfaceSavd::EnviaComando` | uenux2/src/app/comum/iinterfacesavd.cpp | …/comum/iinterfacesavd.cpp | medium |
| 3830 | 2671 | ✔ | `comum::IInterfaceSavd::EnviaRequisicao` | uenux2/src/app/comum/iinterfacesavd.cpp | …/comum/iinterfacesavd.cpp | medium |
| 3832 | 581 |  | `comum::IInterfaceInit::DesmontarMRSemDesabilitar` | uenux2/src/app/comum/iinterfaceinit.cpp | …/comum/iinterfaceinit.cpp | high |
| 3833 | 1100 |  | `comum::IInterfaceInit::EnviarMensagemThrowVoid` | uenux2/src/app/comum/iinterfaceinit.cpp | …/comum/iinterfaceinit.cpp | high |
| 3847 | 72 |  | `comum::CInformacaoEleicao::GetNumBUVotaObrigatorios` | uenux2/src/app/comum/informacao/cinformacaoeleicao.cpp | …/comum/informacao/cinformacaoeleicao.cpp | medium |
| 4694 | 13 |  | `comum::CAssinador::~CAssinador [deleting]` | uenux2/src/app/comum/gravadores/cassinador.cpp | …/comum/gravadores/cassinador.cpp | high |
| 5092 | 313 |  | `ecourna::app::dados::CComparecimentoSecao::CComparecimentoSecao` | ecourna-lib/ecourna/app/dados/resultadournacadastro/ccomparecimentosecao.cpp (path inferred) | …/comum/gravadores/u23-foreign-fragments.cpp | high |
| 5358 | 1227 |  | `comum::(anonymous namespace)::Achata (hash tree -> list)` | uenux2/src/app/comum/gravadores/cgravadorhashes.cpp | …/comum/gravadores/cgravadorhashes.cpp | low |
| 5359 | 47 |  | `comum::(anonymous namespace)::ListaHashes` | uenux2/src/app/comum/gravadores/cgravadorhashes.cpp | …/comum/gravadores/cgravadorhashes.cpp | low |
| 5375 | 459 |  | `std::vector<mesário DAO row (52 bytes)>::__push_back_slow_path` | library instantiation (cgravadorrcsecao.cpp) | …/comum/gravadores/cgravadorrcsecao.cpp (comment) | low |
| 5503 | 23 |  | `comum::IInterfaceSavd::~IInterfaceSavd` | uenux2/src/app/comum/iinterfacesavd.cpp | …/comum/iinterfacesavd.cpp | high |
| 5841 | 600 |  | `comum::(anonymous)::CriaComparecimentoMesario` | uenux2/src/app/comum/gravadores/cgravadorrcsecao.cpp | …/comum/gravadores/cgravadorrcsecao.cpp (comment) | low |
| 5843 | 239 |  | `std::vector<CEstadoComparecimento>::__emplace_back_slow_path(...biometria)` | library instantiation | …/comum/gravadores/cgravadorrcsecao.cpp (comment) | medium |
| 5844 | 237 |  | `std::vector<CEstadoComparecimento>::__emplace_back_slow_path(...audio)` | library instantiation | …/comum/gravadores/cgravadorrcsecao.cpp (comment) | medium |
| 5845 | 938 |  | `comum::CGravadorRCSecao::CriaIdentidadeEleitor` | uenux2/src/app/comum/gravadores/cgravadorrcsecao.cpp | …/comum/gravadores/cgravadorrcsecao.cpp | high |
| 5852 | 340 |  | `std::copy<md::CHashArquivo> (pair of strings)` | library instantiation | …/comum/gravadores/cgravadorhashes.cpp (comment) | medium |
| 5853 | 201 |  | `comum::CGravadorHashes::~CGravadorHashes` | uenux2/src/app/comum/gravadores/cgravadorhashes.cpp | …/comum/gravadores/cgravadorhashes.cpp | high |
| 5854 | 1843 |  | `std::vector<md::CHashArquivo>::insert(pos,first,last,n)` | library instantiation | …/comum/gravadores/cgravadorhashes.cpp (comment) | medium |
| 5855 | 598 |  | `comum::md::CEntidadeHashes::ValidaCriacao` | uenux2/src/app/comum/gravadores/md/centidadehashes.cpp | …/comum/gravadores/md/centidadehashes.cpp | high |
| 5857 | 224 |  | `comum::CGravadorBU::~CGravadorBU` | uenux2/src/app/comum/gravadores/cgravadorbu.cpp | …/comum/gravadores/cgravadorbu.cpp | high |
| 5859 | 232 |  | `std::vector<md::CResultadoVotacaoPorEleicao>::__base_destruct_at_end` | library instantiation | …/comum/gravadores/cgravadorbu.cpp (comment) | medium |
| 5860 | 488 |  | `comum::md::CEnvelopeGenerico::CEnvelopeGenerico(cabecalho,fase,CUrna,...)` | uenux2/src/app/comum/gravadores/md/cenvelopegenerico.cpp | …/comum/gravadores/asn/cconversorenvelopegenerico.cpp | medium |
| 5862 | 435 |  | `comum::md::CUrna::ValidaCriacao` | uenux2/src/app/comum/gravadores/md/curna.cpp | …/comum/gravadores/md/curna.cpp | high |
| 5863 | 48 |  | `comum::md::CVotosVotavel::CVotosVotavel(tipo,codigo,quantidade,CIdentificacaoVotavel)` | uenux2/src/app/comum/gravadores/md/cvotosvotavel.h (path inferred) | …/comum/gravadores/cgravadorbu.h | medium |
| 5864 | 18 |  | `comum::md::CIdentificacaoVotavel::CIdentificacaoVotavel` | uenux2/src/app/comum/gravadores/md/cidentificacaovotavel.h (path inferred) | …/comum/gravadores/cgravadorbu.h | medium |
| 5867 | 327 |  | `comum::md::CVersoesArquivos::CVersoesArquivos` | uenux2/src/app/comum/gravadores/md/cversoesarquivos.cpp | …/comum/gravadores/md/cversoesarquivos.cpp | medium |
| 5889 | 761 |  | `comum::EnviarAcaoHSM` | uenux2/src/app/comum/iinterfacesavd.cpp | …/comum/iinterfacesavd.cpp | high |
| 5890 | 717 |  | `comum::ValidarUE` | uenux2/src/app/comum/iinterfacesavd.cpp | …/comum/iinterfacesavd.cpp | high |
| 5891 | 257 | ✔ | `comum::IInterfaceSavd::AssinaArquivo` | uenux2/src/app/comum/iinterfacesavd.cpp | …/comum/iinterfacesavd.cpp | medium |
| 5894 | 741 |  | `comum::IInterfaceInit::DesligarUrna` | uenux2/src/app/comum/iinterfaceinit.cpp | …/comum/iinterfaceinit.cpp | high |
| 5896 | 5233 |  | `comum::IInterfaceInit::MontarMRSemHabilitar` | uenux2/src/app/comum/iinterfaceinit.cpp | …/comum/iinterfaceinit.cpp | high |
| 5897 | 17 |  | `comum::IInterfaceInit::IsMRMontadoSemHabilitar` | uenux2/src/app/comum/iinterfaceinit.cpp | …/comum/iinterfaceinit.cpp | high |
| 5914 | 72 |  | `comum::CInformacaoEleicao::GetNumBUVotaAdicionais` | uenux2/src/app/comum/informacao/cinformacaoeleicao.cpp | …/comum/informacao/cinformacaoeleicao.cpp | medium |
| 5915 | 72 |  | `comum::CInformacaoEleicao::GetNumRelatorioPU` | uenux2/src/app/comum/informacao/cinformacaoeleicao.cpp | …/comum/informacao/cinformacaoeleicao.cpp | medium |
| 5916 | 72 |  | `comum::CInformacaoEleicao::GetNumRelatorioVersoesDados` | uenux2/src/app/comum/informacao/cinformacaoeleicao.cpp | …/comum/informacao/cinformacaoeleicao.cpp | medium |
| 5917 | 72 |  | `comum::CInformacaoEleicao::GetNumRelatorioEleitores` | uenux2/src/app/comum/informacao/cinformacaoeleicao.cpp | …/comum/informacao/cinformacaoeleicao.cpp | medium |
| 6041 | 331 |  | `comum::CGravadorWSQ::CompactaNaMV` | uenux2/src/app/comum/gravadores/cgravadorwsq.cpp | …/comum/gravadores/cgravadorwsq.cpp | medium |
| 6046 | 115 |  | `comum::IInterfaceInit::ConsultaFlag` | uenux2/src/app/comum/iinterfaceinit.cpp | …/comum/iinterfaceinit.cpp | medium |
| 7766 | 7 |  | `vota::CAssinadorVota::~CAssinadorVota` | uenux2/src/app/vota/comum/cassinadorvota.h (path inferred) | …/comum/gravadores/u23-foreign-fragments.cpp | high |
| 10263 | 1029 |  | `comum::asn::CConversorVersoesArquivos::DoDesconverte` | uenux2/src/app/comum/gravadores/asn/cconversorversoesarquivos.cpp (path inferred) | …/comum/gravadores/asn/cconversorversoesarquivos.u23.cpp | high |
| 10270 | 1851 |  | `comum::asn::CConversorEnvelopeGenerico::DoDesconverte` | uenux2/src/app/comum/gravadores/asn/cconversorenvelopegenerico.cpp | …/comum/gravadores/asn/cconversorenvelopegenerico.cpp | high |
| 10271 | 2568 |  | `comum::asn::CConversorEnvelopeGenerico::DoConverte` | uenux2/src/app/comum/gravadores/asn/cconversorenvelopegenerico.cpp | …/comum/gravadores/asn/cconversorenvelopegenerico.cpp | high |
| 10273 | 12196 |  | `comum::asn::CConversorEntidadeBU::DoConverte` | uenux2/src/app/comum/gravadores/asn/cconversorentidadebu.cpp | …/comum/gravadores/asn/cconversorentidadebu.cpp | high |
| 10282 | 369 |  | `comum::asn::CConversorTipoApuracaoSA::DoDesconverte` | uenux2/src/app/comum/gravadores/asn/cconversortipoapuracaosa.cpp | …/comum/gravadores/asn/cconversortipoapuracaosa.cpp | high |
| 10283 | 3122 |  | `comum::asn::CConversorTipoApuracaoSA::DoConverte` | uenux2/src/app/comum/gravadores/asn/cconversortipoapuracaosa.cpp | …/comum/gravadores/asn/cconversortipoapuracaosa.cpp | high |
| 10284 | 2592 |  | `comum::asn::CConversorUrna::DoDesconverte` | uenux2/src/app/comum/gravadores/asn/cconversorurna.cpp | …/comum/gravadores/asn/cconversorurna.cpp | high |
| 10287 | 2690 |  | `comum::asn::CConversorUrna::DoConverte` | uenux2/src/app/comum/gravadores/asn/cconversorurna.cpp | …/comum/gravadores/asn/cconversorurna.cpp | high |
| 10288 | 910 |  | `comum::asn::CConversorCorrespResultado::DoDesconverte` | uenux2/src/app/comum/gravadores/asn/cconversorcorrespresultado.cpp | …/comum/gravadores/asn/cconversorcorrespresultado.cpp | high |
| 10289 | 1501 |  | `comum::asn::CConversorCorrespResultado::DoConverte` | uenux2/src/app/comum/gravadores/asn/cconversorcorrespresultado.cpp | …/comum/gravadores/asn/cconversorcorrespresultado.cpp | high |
| 10431 | 101 |  | `vota::CEleitorVotouNaoVotou::StartState` | uenux2/src/app/vota/operador/aguardaeleitor/celeitorvotounaovotou.cpp (path inferred) | …/comum/gravadores/u23-foreign-fragments.cpp | high |
| 10842 | 23822 |  | `api::CRHVoiceTextToSpeech::CRHVoiceTextToSpeech` | uenux2/src/api/audio/crhvoicetexttospeech.cpp (path inferred) | docs/libraries/rhvoice.md (not reconstructed here) | high |
| 10965 | 10 |  | `(anonymous namespace)::CWasmSavd::~CWasmSavd [deleting]` | uenux2/wasm/vota_web/vota_web_wasm.cpp (path inferred: its other methods 10930/10949 are app:wasm-entry; registered by `main`) | …/comum/gravadores/u23-foreign-fragments.cpp | high |
| 11575 | 92 |  | `comum::CGravadorWSQ::~CGravadorWSQ [deleting]` | uenux2/src/app/comum/gravadores/cgravadorwsq.cpp | …/comum/gravadores/cgravadorwsq.cpp | high |
| 11576 | 89 |  | `comum::CGravadorWSQ::~CGravadorWSQ` | uenux2/src/app/comum/gravadores/cgravadorwsq.cpp | …/comum/gravadores/cgravadorwsq.cpp | high |
| 11577 | 9 |  | `comum::CGravadorWSQ::CopiaParaMV` | uenux2/src/app/comum/gravadores/cgravadorwsq.cpp | …/comum/gravadores/cgravadorwsq.cpp | medium |
| 11578 | 9 |  | `comum::CGravadorWSQ::CopiaResultadoParaMV` | uenux2/src/app/comum/gravadores/cgravadorwsq.cpp | …/comum/gravadores/cgravadorwsq.cpp | medium |
| 11579 | 319 |  | `comum::CGravadorWSQ::CopiaParaResultado` | uenux2/src/app/comum/gravadores/cgravadorwsq.cpp | …/comum/gravadores/cgravadorwsq.cpp | medium |
| 11615 | 42 |  | `comum::CGravadorRCSecao::~CGravadorRCSecao [deleting]` | uenux2/src/app/comum/gravadores/cgravadorrcsecao.cpp | …/comum/gravadores/cgravadorrcsecao.cpp | high |
| 11616 | 15849 |  | `comum::CGravadorRCSecao::GravaResultado` | uenux2/src/app/comum/gravadores/cgravadorrcsecao.cpp | …/comum/gravadores/cgravadorrcsecao.cpp | high |
| 11619 | 10 |  | `comum::CGravadorHashes::~CGravadorHashes [deleting]` | uenux2/src/app/comum/gravadores/cgravadorhashes.cpp | …/comum/gravadores/cgravadorhashes.cpp | high |
| 11620 | 4622 |  | `comum::CGravadorHashes::GravaResultado` | uenux2/src/app/comum/gravadores/cgravadorhashes.cpp | …/comum/gravadores/cgravadorhashes.cpp | high |
| 11628 | 10 |  | `comum::CGravadorBU::~CGravadorBU [deleting]` | uenux2/src/app/comum/gravadores/cgravadorbu.cpp | …/comum/gravadores/cgravadorbu.cpp | high |
| 11629 | 15903 |  | `comum::CGravadorBU::GravaResultado` | uenux2/src/app/comum/gravadores/cgravadorbu.cpp | …/comum/gravadores/cgravadorbu.cpp | high |
| 11646 | 138 |  | `comum::IInterfaceInit::CurrentBootDevice` | uenux2/src/app/comum/iinterfaceinit.cpp | …/comum/iinterfaceinit.cpp | high |
| 12090 | 39 |  | `comum::IResultado::~IResultado` | uenux2/src/app/comum/gravadores/iresultado.cpp | …/comum/gravadores/iresultado.cpp | high |

## 16. Reconstructed files

* [`src/uenux2/src/app/comum/gravadores/iresultado.h`](../../src/uenux2/src/app/comum/gravadores/iresultado.h)
* [`src/uenux2/src/app/comum/gravadores/iresultado.cpp`](../../src/uenux2/src/app/comum/gravadores/iresultado.cpp)
* [`src/uenux2/src/app/comum/gravadores/cgravadorutil.h`](../../src/uenux2/src/app/comum/gravadores/cgravadorutil.h)
* [`src/uenux2/src/app/comum/gravadores/cgravadorutil.cpp`](../../src/uenux2/src/app/comum/gravadores/cgravadorutil.cpp)
* [`src/uenux2/src/app/comum/gravadores/cgravadorbu.h`](../../src/uenux2/src/app/comum/gravadores/cgravadorbu.h)
* [`src/uenux2/src/app/comum/gravadores/cgravadorbu.cpp`](../../src/uenux2/src/app/comum/gravadores/cgravadorbu.cpp)
* [`src/uenux2/src/app/comum/gravadores/cgravadorrcsecao.cpp`](../../src/uenux2/src/app/comum/gravadores/cgravadorrcsecao.cpp)
* [`src/uenux2/src/app/comum/gravadores/cgravadorhashes.cpp`](../../src/uenux2/src/app/comum/gravadores/cgravadorhashes.cpp)
* [`src/uenux2/src/app/comum/gravadores/cgravadorwsq.cpp`](../../src/uenux2/src/app/comum/gravadores/cgravadorwsq.cpp)
* [`src/uenux2/src/app/comum/gravadores/cassinador.cpp`](../../src/uenux2/src/app/comum/gravadores/cassinador.cpp)
* [`src/uenux2/src/app/comum/gravadores/u23-foreign-fragments.cpp`](../../src/uenux2/src/app/comum/gravadores/u23-foreign-fragments.cpp)
* [`src/uenux2/src/app/comum/gravadores/asn/cconversorentidadebu.cpp`](../../src/uenux2/src/app/comum/gravadores/asn/cconversorentidadebu.cpp)
* [`src/uenux2/src/app/comum/gravadores/asn/cconversorenvelopegenerico.cpp`](../../src/uenux2/src/app/comum/gravadores/asn/cconversorenvelopegenerico.cpp)
* [`src/uenux2/src/app/comum/gravadores/asn/cconversorurna.cpp`](../../src/uenux2/src/app/comum/gravadores/asn/cconversorurna.cpp)
* [`src/uenux2/src/app/comum/gravadores/asn/cconversorcorrespresultado.cpp`](../../src/uenux2/src/app/comum/gravadores/asn/cconversorcorrespresultado.cpp)
* [`src/uenux2/src/app/comum/gravadores/asn/cconversortipoapuracaosa.cpp`](../../src/uenux2/src/app/comum/gravadores/asn/cconversortipoapuracaosa.cpp)
* [`src/uenux2/src/app/comum/gravadores/asn/cconversorversoesarquivos.u23.cpp`](../../src/uenux2/src/app/comum/gravadores/asn/cconversorversoesarquivos.u23.cpp)
* [`src/uenux2/src/app/comum/gravadores/asn/util.cpp`](../../src/uenux2/src/app/comum/gravadores/asn/util.cpp)
* [`src/uenux2/src/app/comum/gravadores/md/cenvelopegenerico.cpp`](../../src/uenux2/src/app/comum/gravadores/md/cenvelopegenerico.cpp)
* [`src/uenux2/src/app/comum/gravadores/md/curna.cpp`](../../src/uenux2/src/app/comum/gravadores/md/curna.cpp)
* [`src/uenux2/src/app/comum/gravadores/md/centidadehashes.cpp`](../../src/uenux2/src/app/comum/gravadores/md/centidadehashes.cpp)
* [`src/uenux2/src/app/comum/gravadores/md/cversoesarquivos.cpp`](../../src/uenux2/src/app/comum/gravadores/md/cversoesarquivos.cpp)
* [`src/uenux2/src/app/comum/gravadores/md/cversoescontratos.cpp`](../../src/uenux2/src/app/comum/gravadores/md/cversoescontratos.cpp)
* [`src/uenux2/src/app/comum/gravadores/md/cdependenciascontratos.cpp`](../../src/uenux2/src/app/comum/gravadores/md/cdependenciascontratos.cpp)
* [`src/uenux2/src/app/comum/iinterfaceinit.h`](../../src/uenux2/src/app/comum/iinterfaceinit.h)
* [`src/uenux2/src/app/comum/iinterfaceinit.cpp`](../../src/uenux2/src/app/comum/iinterfaceinit.cpp)
* [`src/uenux2/src/app/comum/iinterfacesavd.h`](../../src/uenux2/src/app/comum/iinterfacesavd.h)
* [`src/uenux2/src/app/comum/iinterfacesavd.cpp`](../../src/uenux2/src/app/comum/iinterfacesavd.cpp)
* [`src/uenux2/src/app/comum/informacao/cinformacaoeleicao.cpp`](../../src/uenux2/src/app/comum/informacao/cinformacaoeleicao.cpp)

The other files of `comum/gravadores` (`cgravadorrdv`, `cgravadorlog`, `cgravadorenvelopearquivo`, `cgravadorversoesarquivos`, `igravador`, `md/centidadebu`, `md/cdadosbuvota`, `asn/cassinavotavelbu`, `asn/cconversorentidadehashes`) belong to other units or exist only inlined; their inlined bodies are reconstructed where they appear (see the comments in the files above).
