# u24: `comum/justificativa`, `comum/log`, `comum/md`, `comum/nomearquivo`, `comum/reconhecimentobiometrico`

Unit u24 covers 39 wasm functions from five small directories of `uenux2/src/app/comum/` ("comum" is the
code shared by the urna's applications). They are unrelated helpers used around the vote:

| directory | what it does in the voting process |
|---|---|
| `justificativa/` | **justificativa eleitoral**: a voter who is away from his electoral domicile goes to any urna and *justifies* his absence (título + year of birth). Keeps the list in memory (`CJustificador`) and in SQLite (`registro_justificativa`). |
| `log/` | fixed texts of the urna's **event log** (`logd.dat`): result generation at *encerramento* (closing), signature sessions, battery/power events, names of printed reports. |
| `md/` | "modelo de dados": small validated value classes used in result files: `CAbrangencia` (scope of a datum), `CCabecalhoEntidade` (header of every entity), `CIdentificacaoUrna` / `CIdentificacaoSecao` (who produced a result), `CSeguranca` (encryption block of an envelope). |
| `nomearquivo/` | names of the election **data files** the urna loads (`t02411ac00001-ca.dat` ...). |
| `reconhecimentobiometrico/` | storage of **fingerprint images** (WSQ): encrypt with the TSE public key, wrap in an envelope, write under a random name to both flashes. |

Six of the 39 functions ran in the recorded sessions (all at `votaInit`): 348, 1161, 1705, 3744, 3773, 5887.
Everything else belongs to the mesário terminal (justification, biometrics) or to the end of the day
(result files), which the public simulator never drives.

Reconstructed sources (all under `src/uenux2/src/app/comum/`):

```
justificativa/cjustificador.{h,cpp}           justificativa/dao/cjustificadordao.{h,cpp}
log/ieventoslog.{h,cpp}                       log/clogcomum.{h,cpp}
md/cabrangencia.{h,cpp}  md/ccabecalhoentidade.{h,cpp}  md/cidentificacaourna.{h,cpp}
md/cidentificacaosecao.{h,cpp}  md/cseguranca.{h,cpp}
nomearquivo/cnomearquivo.{h,cpp}              (plus cnomearquivo.u02.cpp from unit u02)
reconhecimentobiometrico/ccontrolaarmazenamentodeimagens.{h,cpp}
u24-foreign-fragments.cpp                     (348 CArquivosResultado::GetInst, 11412 CConversorSeguranca,
                                               10888 ~CFormPart, 11205 ~CParteEleitores)
```

## 1. Glossary

| term | meaning |
|---|---|
| justificativa (eleitoral) | declaration of absence by a voter outside his domicile; recorded instead of a vote |
| título / número de inscrição eleitoral | the voter's 12-digit registration number (`CNumeroInscricaoEleitoral`) |
| BUJ | *Boletim de Justificativa*, the printed list of justifications (report `ERelatoriosUE::BUJ`) |
| abrangência | geographic scope: municipal (UF + município), estadual (UF), federal (country) |
| pleito / eleição | the election day (all ballots) / one election of it (e.g. the municipal one) |
| fase | `'1'` oficial (`o`), `'2'` simulado (`s`), `'3'` treinamento (`t`); every simulator scenario is `t` |
| MI / FI, ME / FE, MV | mídia/flash interna (internal flash, `/dsk/fi/`), mídia/flash externa (`/dsk/fe/`, the removable voting card, "MV" = memória de votação) |
| MSD / MSE | the urna's security module (HSM), MSD on older models, MSE on UE2020+ (`IUrna::GetModelo() >= 2020`) |
| WSQ | FBI fingerprint image compression format; the urna stores the captured fingerprints as `.wsq` |
| CEPESC | the TSE/CEPESC encryption scheme used for result envelopes (`ecourna::api::cepesc`) |
| encerramento | closing of the section: result files (BU, RDV, ...) are generated, signed and copied |

## 2. Classes and hierarchy (from RTTI)

```
comum::IEventosLog                     (class, vtable @1552860 {174 D1, 144 D0}, 8 bytes: +4 ELogAplicativos)
  └─ vota::CLogVota                    (si, vtable @1532792; the only implementation, pushed in CPolySingletonList)
comum::CLogComum                       (no RTTI, empty class, lazy singleton = wasm 948)

comum::servico::CJustificadorServico   (class, vtable @1560936 {3740 D1, 11464 D0}, 12 bytes)
comum::CJustificador                   (no RTTI, 40 bytes) : api::CDataMap<CNumeroInscricaoEleitoral,
                                                             CJustificadorDetalhe>  + member CJustificadorServico
comum::CJustificadorDadoTitulo         (no RTTI; static Text())
api::persistencia::IDAO
  └─ IUenuxGenericDAO<ecourna::app::dados::CIdentificacaoJustificativa, std::string>
       └─ comum::dao::IJustificadorDAO
            └─ comum::dao::CJustificadorDAO   (vtable @1560768, 12 bytes: +4 shared_ptr<ISqlConnection>)

comum::md::CAbrangencia (20 B), CCabecalhoEntidade (20 B), CSeguranca (16 B)        (no RTTI)
comum::md::CIdentificacaoUrna (12 B) ─┬─ CIdentificacaoSecao (20 B, this unit)       (no RTTI)
                                      └─ CIdentificacaoUrnaContingencia (other unit, ctor 5888)
comum::CNomeArquivo                    (no RTTI; static functions + anonymous-namespace helpers)
comum::CControlaArmazenamentoDeImagens (no RTTI; static functions only)
```

### `CJustificador` layout (func 3742 builds it, 2810 destroys it)

| offset | member |
|---|---|
| +0 | `std::map<CNumeroInscricaoEleitoral, CJustificadorDetalhe>` (CDataMap container; node: key at +16, 24 bytes; year at +40) |
| +12 | `m_atual` cursor (== end() when not positioned) |
| +16 | `std::string m_nome = "CJustificador"` (CDataMap name used in its error messages) |
| +28 | `servico::CJustificadorServico m_servico` { vptr, `shared_ptr<IJustificadorDAO>` +32/+36 } |

`CJustificador::GetInst()` (func 1391, other unit) creates it lazily in the `unique_ptr` @1839012; the
constructor (3742) takes the DAO from `api::persistencia::CDAORepositorio::Entregar<IJustificadorDAO>()`,
which clones the prototype `CJustificadorDAO` registered at start-up by
`CInformacaoEleitor::InicializarPersistencia` (inlined in 7787).

## 3. Justificativa: control flow and data

1. **Start-up** (7787): `CJustificadorDAO(<trab MI>/uenux.db)` runs
   `CREATE TABLE IF NOT EXISTS registro_justificativa ( \nnumero_titulo     BIGINT PRIMARY KEY UNIQUE NOT NULL, \nano_nascimento    SMALLINT \n)`
   and is registered in `CDAORepositorio` under `"N5comum3dao16IJustificadorDAOE"`. Then
   `comum::CEleitores::CompleteLoad` (6734, celeitores.cpp:205..238, called by 7787 `CarregarDadosEstaticos` and by
   11865 `CGeraDadosDinamicos::StartState`) clears `CJustificador` (tree destroy 2809, cursor reset to `end()`) and
   refills it from `RecuperarTodos()` (DAO slot 8, reached through the service helper `comum_f5723`), one
   `CDataMap::Add` (5725) per row.
2. **Mesário enters a justification** (`vota::CPedeAnoNascimento`, 10590, unit u17), inlining
   `CJustificador::Justifica(titulo, ano)` (cjustificador.cpp:55): if the título is already in the map, the cursor
   is moved to it and `8800 "Já havia justificativa para o título"` is thrown; else `Add({titulo, {ano}})`.
3. **Saving** (inlined in 10590; cjustificador.cpp:83 `SaveCurrentInternal`): throws `8801 "Não há dados a serem
   salvos na MI"` when the map is empty; otherwise builds `CIdentificacaoJustificativa(CRegistroIdentificacaoEleitor(
   make_shared<CNumeroInscricaoEleitoral>(titulo)), ano)`, and calls `dao.Recuperar(numero)`; only if no row exists,
   `dao.Inserir(...)`. The surrounding code (same function 10590) guards with *"Gravando a justificativa na MI"*,
   saves `vota.bin` (SalvaVotaInterno), signs `uenux.db` (SAVD file 122/123), synchronises, then *"Gravando a
   justificativa na MV"*: SalvaVotaExterno and copy of the database to the external flash.
4. **Reports**: `CJustificadorDadoTitulo::Text()` (11475, slot 2949) returns the título under the cursor
   (`8802 "Justificativa inválida"` at end), used while the BUJ / justification screens walk the map with
   `CDataMap::Next()` (unit u08/u09). Slot 2948 (11474, unit u36) prints the count.

`CJustificadorDAO` SQL (exact text, including the leading blank and `" \n"`):

| slot | method | SQL / behaviour |
|---|---|---|
| 2 | `Clonar` 11468 | `new CJustificadorDAO(*this)` (shares the connection) |
| 3 | `Inserir` 11473 | ` INSERT INTO registro_justificativa (numero_titulo, ano_nascimento) VALUES (?, ?) \n` with `SetInt64(1, ToQWord(título))`, `SetInt(2, ano)` |
| 4 | `Excluir` 11467 (u20) | default: throws 6804 |
| 5 | `ExcluirID` 11472 | throws 8804 "Nao é esperado que o CJustificadorDAO exclua registros do banco" (:56) |
| 6 | `Atualizar` 11471 | throws 8805 "Nao é esperado que o CJustificadorDAO atualize registros do banco" (:63) |
| 7 | `Recuperar` 11470 | ` SELECT numero_titulo, ano_nascimento FROM registro_justificativa WHERE numero_titulo = ? \n` → `shared_ptr` or null |
| 8 | `RecuperarTodos` 11469 | ` SELECT numero_titulo, ano_nascimento FROM registro_justificativa \n` → `vector<CIdentificacaoJustificativa>` |

Rows are read as `std::format("{}", GetInt64(0))` (the 12-digit padding is restored by `CNumeroInscricaoEleitoral`)
and `GetInt(1)` truncated to 16 bits and range-checked 0..9999 by `CBaseType<0,9999>` (a NULL year reads as 0).
The table holds **no vote**: only who justified.

## 4. Log (`IEventosLog`, `CLogComum`)

All records go to `api::CLoga::loga(ELogAplicativos, ESeveridade, text)` through
`api::CPolySingletonList::instance<IEventosLog>()` (= `vota::CLogVota`, app id 1). The `srcloc` of every
`CLogComum` method is the line where it fetches that instance.

| method | line | wasm | text |
|---|---|---|---|
| `LogaIniciaSessaoMSD` / `FinalizaSessaoMSD` | 44 / 50 | inlined 12098 | "Inicia uma sessão no MSD" / "Finaliza a sessão no MSD" |
| `LogaIniciaSessaoMSE` / `FinalizaSessaoMSE` | 56 / 62 | inlined 12098 | "Inicia uma sessão no MSE" / "Finaliza a sessão no MSE" |
| `LogaCopiandoArqResParaFI(ext)` | 76 | 5878 → 6045 | "Copiando arquivo de resultado para MI: [{}]" |
| `LogaResultadoCopiadoResFE(ext)` | 84 | 3828 → 6045 | "Copiando arquivo de resultado para ME: [{}]" |
| `LogaGerandoResultados(ext, status)` | 92 | 5876 | "Gerando arquivo de resultado [{}] + [{}]" (name, "Início"/"Término") |
| `LogaNivelBateria(fonte, status)` | 106 | 5875 | "Carga da [{}]: [{}]" (severity 2 if CRÍTICA, else 1) |
| `LogaSuspensoMonitoramenteRepeticao(n)` | 136 | inlined 10226 | "Suspenso monitoramento de alimentação devido repetição de eventos. [ {} ] eventos" |
| `LogaTipoBateria` | 144/145/158 | inlined 10226 | "Urna operando na rede elétrica / bateria interna / bateria externa"; code 3 throws 8850 "Tipo de alimentação não implementado" |
| `LogaInicio/TerminoProcedimentoAssinatura` | 166 / 172 | inlined 12098 | "Início/Término do procedimento de assinatura dos arquivos de resultados" |
| `LogaPreparandoAssinaturaArquivosResultado` | 178 | inlined 12098 | "Preparando para assinatura dos arquivos de resultados" |
| `LogaScoreReconhecimentoMesario(score)` | 214 | inlined 5372 | "Batimento de digitais retornou o score {}" |

`ext` is turned into the file suffix by `CArquivosResultado::GetInst()[ext]` (func 348 + 347): `bu.dat`, `rdv.dat`,
`jufa.dat`, `imgbu.dat`, `hash.dat`, `log.jez`, `wsqbio.jez`, ... (see unit u21).

`IEventosLog` converters (texts are Latin-1 in the binary; out-of-range values throw `CUeComumLogError`
"Valor inválido"):

* `ConverteERelatoriosUE` (5879, :426, 8852): 0 ZERÉSIMA (Latin-1 `5A 45 52 C9 53 49 4D 41`, with the accent), 1 ZERÉSIMA DE APURAÇÃO, 2 ZERÉSIMA DE SEÇÃO, 3 BU, 4 BUJ,
  5 RDV, 6 AUTOTESTE, 7 COMPROVANTE DE CARGA, 8 HASHES ARQUIVOS, 9 BIM, 10 ARQUIVOS DE ELEITORES E CANDIDATOS,
  11 RESUMO DA ZERÉSIMA, 12 ELEITORES HABILITADOS BIOGRAFICAMENTE. Used by `CLogVota::LogaImpressaoRelatorio`:
  "Imprimindo relatório [BU] via nº [n]".
* `ConverteFonteAlim` (:440, 8853): 0 ALIMENTAÇÃO AC, 1 ALIMENTAÇÃO BATERIA INTERNA, 2 ALIMENTAÇÃO BATERIA EXTERNA.
* `ConverteStatusBateria` (:456, 8854): 0 PLENA, 1 PARCIAL, 2 CRÍTICA, 3 AUSENTE.

## 5. `md` value classes (validation rules)

Errors: `CBaseError<EUeComumMdError, {8900, 8950}>` (thunk `comum_f591`). Members are stored first, then checked.

| class (wasm) | fields | checks, in order (line, code, message) |
|---|---|---|
| `CAbrangencia(tipo, uf, município)` 3739 | +0 tipo, +4 uf, +16 município | :31 8900 uf size not 0/2 "UF inválida [uf]"; :34 8901 uf empty and not federal; :39 8902 municipal with município 0; :42 8903 município ≠ 0 and not municipal; :47 8904 federal with UF; :51 8905 tipo ≥ 3 "Tipo de abrangência inválido: {}" |
| `CCabecalhoEntidade(data, id, tipo)` 1945 | +0 CDateTime, +12 id, +16 tipo (0 processo, 1 pleito, 2 eleição) | :24 8906 id ≥ 100000 "ID inválido."; :27 8907 tipo ≥ 3 "Tipo de ID inválido." |
| `CIdentificacaoUrna(tipo, município, zona)` 5886 | +0 tipo, +4 município, +8 zona (u16) | :25 8918 tipo not '1'..'4' "Tipo inválido."; :28 8919 município ≥ 100000; :31 8920 zona ≥ 10000 |
| `CIdentificacaoSecao(tipo, mun, zona, local, seção)` 5887 | base + +12 local, +16 seção (u16) | (:33 tipo ≠ '1': folded away, see §10); :38 8916 local ≥ 10000 "Local inválido."; :41 8917 seção 0 or ≥ 10000 "Seção inválida." |
| `CSeguranca(tipoArquivo, idCripto, chave)` 3823 | +0 u8, +1 u8, +4 vector | :24 8925 tipo ≥ 3; :27 8926 id not 1..3; :30 8927 "Chave vazia." |

`CSeguranca` has no `idArquivoCD`: `CConversorSeguranca` (11412 read / 11413 write) drops it and writes 0.

## 6. File names (`CNomeArquivo`)

`<fase><id:05><uf>[<município:05>[<zona:04><seção:04>]]-<sufixo>.<extensão>`, with helpers
`FormataFase` ('1'/'2'/'3' → o/s/t, else 8950 "Fase inválida: {}", where the enum goes through the shared TSE enum formatter func 536 and prints its integer, e.g. `52`), `FormataNumero(n, casas)` (`std::format("{:0{}}")`; 8951 if longer),
`FormataUF` (2 letters else 8952, lower-cased by `CStringUtils::ToLower`).

* `MontaNome(fase, id, uf, sufixo, ext)` (1705, name inferred): processo eleitoral/UF level. At start-up 7787 builds
  `t02400ac-pu.dat`, `t00000ac-pu.dat` and `t00000br-pu.dat` (urna parametrization of the simulator's processo eleitoral
  02400, of the UF, national; parties are `-pa`, not `-pu`). Real 2026 and 2024 media carry only the national
  `o00000br-pu`; how 7787 handles the missing UF-level files is open (2026 urna data: investigation/README.md,
  findings E11, E12, E13).
* `MontaNomesEleicao(id, pleito, município, idEleição, sufixo, ext)` (3744, name inferred): copies the eleição
  (`CPleito::GetEleicao`, cpleito.cpp:139, throws 8162 "Eleição não encontrada: {}"), and if it has at least one elective office (a cargo
  with `DetalheCargo`), applies `ajustaAbrangenciaUFMunicipio` (:208: municipal keeps UF+município, estadual sets
  município 0, federal sets município 0 and UF "BR", else 8953) and returns `{abrangência → name}`:
  `t02411ac00001-ca.dat` (municipal), `t02512ac00000-ca.dat` (estadual), `t02511br00000-ca.dat` (federal).
  The per-eleição suffixes seen in the scenarios are `ca`, `co`, `fe`, `fo`, `le`, `pa`, `pi`, `rdj`.
* `MontaNome` with zona/seção (3772) and `NomesPorAbrangencia` (2811) are in `cnomearquivo.u02.cpp`.

## 7. Fingerprint image storage (`CControlaArmazenamentoDeImagens::Armazena`, 2725)

Called with (wsq, internal dir, external dir, prefix) by `CMostraEleitorVotando::SalvaHabilitacaoEleitor`
(prefix `""`, dirs `<trab>/wsq/habilitado|nao-habilitado|operador/`) and `CPedeDigitalMesario::GetControlador`
(prefix `"me"`). Steps:

1. `statfs("/dsk/fe/dinamico/")`: if it succeeds and `f_bavail * f_bsize < 5 MiB` → return 999999 (not stored).
2. `GerarCaminhosUnicos` (:73): repeat `id = (uint32)IRng::Gera() % 999999`, `nome = "{prefix}{id:06}.wsq"`,
   until `<internal dir>/nome` does not exist (the external path is not tested).
3. `imagem = <WSQ codec singleton 2742>(wsq)` (reduced to a copy here); empty → return 999999.
4. `CifrarWsq` (:149/:152): 1024-byte table from `IUrna` slot 4, 32 random bytes from `IRng` slot 4,
   `LeChavePublica()`, then `CPlainText(0, 1, zona, seção, tabela, aleatório, chave, imagem)` →
   `CCepescCipher::Cifra` → `CCipheredOut {chave, conteúdo}`.
   * `LeChavePublica` (:117..:135): `/dsk/fi/estatico/chave/<name>` must exist (else 9000 "O arquivo … não
     existe"). The name is **not recoverable from this binary**: it is formatted from a constant `string_view`
     `{data = 117, size = 7}` (`i64.const 30065124469`, format arg types 428 = `const char*` directory +
     `string_view` name). Address 117 is below the first data segment (1024) and holds 7 zero bytes at run time
     (read after `votaInit`), so the web build would look for `/dsk/fi/estatico/chave/` + seven NULs. `wsq.pk1`
     (7 chars, the name `CGravadorWSQ::ValidaTipoBiometria` 3796 uses) is an inference from this binary; the 2026 urna
     lists strongly support it, with `/dsk/fi/estatico/chave/wsq.pk1` for every UF (2026 urna data:
     investigation/README.md, finding A3). BER `EntidadeChave`; its
     `chave` field is deciphered with `CSymmetricCipherFactory::Cria(secret of IKernelHSM slot 3)`; empty result →
     9001 "O arquivo … está vazio". The `cifrado` flag is not consulted.
5. Open `<internal>` with `"w+b"`, `FM_NOATIME`; write `EntidadeEnvelopeGenerico` { cabecalho = (pleito date at
   **00:00:00**, pleito id, tipo pleito), fase, município, zona, local (optional), seção,
   `tipoEnvelope` = C++ 3 → ASN.1 5 `envelopeImagemBiometria`, `seguranca` = `CSeguranca(0, 1, out.chave)`,
   `conteudo` = `out.conteudo`, tipo de urna '1' } via `CFileASN::CodeObjectFunction`.
6. `utimes(internal, {0,0})`: access/modification times reset to 1970.
7. `CSystem::CopyFile(internal, external, preservaAtributos = true)` (the zero times are copied too); return id.

The random name, the midnight header date (step 5) and the zeroed file times (step 6) keep the capture time
out of the envelope and out of the file metadata, so the images cannot be sorted into the order in which voters
were enabled from those fields (directory order / inode numbers are outside this code).

## 8. Boletim de Urna (BU): where this unit is involved

This unit does not generate the BU. Its pieces used by the BU code (units u09/u23):

* **`CCabecalhoEntidade`** (1945) is the header of `EntidadeBoletimUrna` and of the envelopes around `bu.dat`,
  `rdv.dat`, `imgbu.dat`, `hash.dat`: built by `CGravadorBU`/`CGravadorRDV`/`CGravadorHashes`/`IGravadorEnvelope`
  with `(dhGeracao, CConfiguracaoEleicao pleito id, ETipoCabecalho::Pleito = 1)`; validated id < 100000, tipo < 3.
  ASN.1: `CabecalhoEntidade { dataGeracao DataHoraJE, idEleitoral CHOICE { [1] idProcessoEleitoral,
  [2] idPleito, [3] idEleicao } }`.
* **`CSeguranca`** (3823) is the `seguranca` of the encrypted BU envelope when the parameter "criptografar BU" is
  set: `CSeguranca(idTipoArquivo 0, idCriptografia 1, idArquivoChave = CEPESC out.chave)`; the BU flow reads
  `bu.pk1` with the same code as `LeChavePublica` above (cgravadorbu.cpp:542..560).
* **`CIdentificacaoSecao`** (5887) identifies the section in `CGravaResultado` (12098) and in the section data.
* **Logs of the encerramento** (`CGravacaoResultados::Executa`): per result file "Gerando arquivo de resultado
  [bu.dat] + [Início]" … "[Término]", "Copiando arquivo de resultado para MI: [bu.dat]"; then "Preparando para
  assinatura…", "Início do procedimento…", "Inicia uma sessão no MSE|MSD", "Finaliza a sessão…", "Término do
  procedimento…"; and "Copiando arquivo de resultado para ME: […]" for the copies to the voting card.
  Printing of the BU vias is logged as "Imprimindo relatório [BU] via nº [n]" using `ConverteERelatoriosUE`.
* The justifications (`registro_justificativa`) become the `jufa.dat` result file and the printed BUJ; they are
  stored separately from votes.

## 9. Web-build specifics

* The simulator drives only the voter terminal: justifications, biometric storage, encerramento logs and
  battery logs never run. `registro_justificativa` exists (empty) in the MEMFS `uenux.db`.
* `CControlaArmazenamentoDeImagens` cannot store anything: the WSQ pipeline is compiled out (empty images,
  u10), no `/dsk/fi/estatico/chave/` is shipped and no `api::IKernelHSM` is registered; `CCepescCipher` is an
  identity cipher (u01). `IRng` is `CPrng(seed 0)`, re-seeded at every call, so the "random" name is constant.
* Emscripten's MEMFS `statfs` returns fixed values (always far above 5 MiB).
* `IPower` is `api::teste::CPowerMock`; `IEventosLog` is `vota::CLogVota` writing `dsk/fi/dinamico/log/logd.dat`.

## 10. Wasm / Emscripten observations

* **merge-similar-functions**: 6045 is the body of two `CLogComum` methods with the format string (begin, end)
  and srcloc as parameters; 2899 is the destructor body of four unrelated `{vptr, shared_ptr}` classes with the
  vtable as a parameter; 2900 serves four empty singletons (348, 948, 2742, 3603); error-constructor thunks 2260,
  2827, 2861, 5369 all call ecourna 710 with a vtable.
* **Dead-argument elimination**: `CLogComum` methods and `IEventosLog::Convert*` are non-static in the source
  (no "static" in the srcloc pretty names) but take no `this` in wasm.
* **Constant propagation**: `CIdentificacaoSecao` lost its `EUrnaTipo` parameter (all callers pass '1'); the
  line-33 check vanished, leaving the unreferenced "Tipo inválido para urna de seção." (@361994) and an unused
  error code 8915.
* **Inlining misnames**: 2725 carries six srclocs of three inlined helpers (:73 `GerarCaminhosUnicos`,
  :117/:127/:135 `LeChavePublica`, :149/:152 `CifrarWsq`), hence the tools' name `LeChavePublica`;
  5875 (really `CLogComum::LogaNivelBateria`) was named after its inlined `ConverteFonteAlim`; 3744 after its
  inlined `ajustaAbrangenciaUFMunicipio`.
* `std::format` packs argument types with 5 bits each (e.g. 205 = string_view, unsigned).
* `std::mutex` locks of the lazy singletons are no-ops (no pthreads); only `mutex::unlock` (150) remains.

## 11. Error codes of this unit

| family (limits) | code | where | message |
|---|---|---|---|
| Justificativa {8800,8850} | 8800 / 8801 / 8802 / 8804 / 8805 | cjustificador :55 / :83 / :104, dao :56 / :63 | see §3 |
| Log {8850,8900} | 8850 / 8852 / 8853 / 8854 (8851 unused: the two `i32.const 8851` in the module are a table slot) | clogcomum :158, ieventoslog :426 / :440 / :456 | "Tipo de alimentação não implementado" / "Valor inválido" |
| Md {8900,8950} | 8900–8907, 8916–8920, 8925–8927 | §5 | §5 |
| NomeArquivo {8950,9000} | 8950 / 8951 / 8952 / 8953 | cnomearquivo :38 / :48 / :73 / :208 | Fase / Número / UF / Abrangência inválida |
| ReconhecimentoBiometrico {9000,9050} | 9000 / 9001 | ccontrolaarmazenamentodeimagens :117 / :135 | "O arquivo " + path + " não existe" / " está vazio" (concatenation, not `std::format`) |

## 12. Weird or risky code (details in the returned "suspicious" list)

1. `GerarCaminhosUnicos` loops `while (file exists)` with no bound; with the build's deterministic `CPrng(0)` the
   same id comes back forever once one image exists → hang (latent: unreachable in the web build).
2. `LeChavePublica` ignores `EntidadeChave.cifrado` and frees the HSM secret (the returned `vector` and its
   `std::string` copy passed to `Cria`) without wiping it.
3. `SaveCurrentInternal` (inlined in 10590, not in 2810) tests `empty()` but dereferences the cursor (safe in the
   only call path: `CDataMap::Add` 5725 sets the cursor to the inserted node just before).
4. `LogaTipoBateria` (inlined in `CThreadMonitor::Run` 10226, not in 5875) throws (8850) for power-source code 3
   instead of logging. The throw is a direct `__cxa_throw` in 10226 (no enclosing try there) and the thread
   trampoline 10255 calls `Run` without `invoke`, so nothing in the monitor catches it. In the web build the
   monitor thread never starts (`CThread::Start` is only reached through `CExecucaoVota`, see u07), and 5875
   itself cannot throw (its fonte is the constant 1 or 2 and its status is a 2-bit field).
5. `MontaNomesEleicao` copies the whole eleição (all cargos) to read two fields (performance only).
6. The key-file name of `LeChavePublica` in 2725 is a `string_view {117, 7}` pointing below the data segments
   (7 zero bytes at run time, see §7 step 4); the other key readers (`CGravadorBU` `bu.pk1`, `CGravadorRCSecao`
   `jufa.pk1`, `CConversorBiometriaEleitorCifrada` `bio.sk1`) append a real string literal. Unexplained; harmless
   in the web build because the function never gets that far.

## 13. Complete mapping table (39 functions)

Paths in the last column are relative to `src/uenux2/src/app/comum/`; `run` = observed executing in the recorded votes.

| wasm | size | run | reconstructed symbol | source file |
|---|---|---|---|---|
| 348 | 15 | * | `comum::CArquivosResultado::GetInst()` | u24-foreign-fragments.cpp (orig. carquivosresultado.cpp) |
| 1161 | 893 | * | `comum::(anon)::FormataNumero(uedword, uedword)` (:48) | nomearquivo/cnomearquivo.cpp |
| 1705 | 787 | * | `comum::CNomeArquivo::MontaNome(EUrnaFase, uedword, uf, sufixo, ext)` (name inferred) | nomearquivo/cnomearquivo.cpp |
| 1945 | 157 | | `comum::md::CCabecalhoEntidade::CCabecalhoEntidade` (:24/:27) | md/ccabecalhoentidade.cpp |
| 2260 | 18 | | `CUeComumJustificativaError` ctor thunk (CBaseError<EUeComumJustificativaError,{8800,8850}>) | justificativa/cjustificador.cpp (comment) |
| 2725 | 5103 | | `comum::CControlaArmazenamentoDeImagens::Armazena` (+ inlined GerarCaminhosUnicos :73, LeChavePublica :117–135, CifrarWsq :149/152) | reconhecimentobiometrico/ccontrolaarmazenamentodeimagens.cpp |
| 2810 | 46 | | `comum::CJustificador::~CJustificador()` | justificativa/cjustificador.cpp |
| 2827 | 18 | | `CUeComumNomeArquivoError` ctor thunk ({8950,9000}) | nomearquivo/cnomearquivo.cpp (comment) |
| 2828 | 530 | | `comum::(anon)::FormataFase(EUrnaFase)` (:38) | nomearquivo/cnomearquivo.cpp |
| 2861 | 18 | | `CBaseError<comum::EUeComumLogError,{8850,8900}>` ctor thunk | log/ieventoslog.cpp (comment) |
| 2899 | 63 | | ICF destructor body `{vptr, shared_ptr}` (~CJustificadorServico / ~CComparecimentoMesarioServico / ~CFormPart / ~CParteEleitores) | justificativa/cjustificador.cpp (comment) |
| 3739 | 865 | | `comum::md::CAbrangencia::CAbrangencia` (:31–:51) | md/cabrangencia.cpp |
| 3740 | 12 | | `comum::servico::CJustificadorServico::~CJustificadorServico()` (D1, slot 0) | justificativa/cjustificador.cpp |
| 3744 | 2521 | * | `comum::CNomeArquivo::MontaNomesEleicao` (name inferred) + inlined `ajustaAbrangenciaUFMunicipio` (:208) | nomearquivo/cnomearquivo.cpp |
| 3773 | 540 | * | `comum::(anon)::FormataUF(const std::string&)` (:73) | nomearquivo/cnomearquivo.cpp |
| 3823 | 305 | | `comum::md::CSeguranca::CSeguranca` (:24/:27/:30) | md/cseguranca.cpp |
| 3828 | 20 | | `comum::CLogComum::LogaResultadoCopiadoResFE` (:84) | log/clogcomum.cpp |
| 5369 | 18 | | `CUeComumReconhecimentoBiometricoError` ctor thunk ({9000,9050}) | reconhecimentobiometrico/ccontrolaarmazenamentodeimagens.cpp (comment) |
| 5831 | 259 | | library: `std::__uninitialized_allocator_relocate` for `vector<CIdentificacaoJustificativa>` | justificativa/dao/cjustificadordao.cpp (comment) |
| 5875 | 1137 | | `comum::CLogComum::LogaNivelBateria(uebyte, uebyte)` (:106; inlined ConverteFonteAlim :440, ConverteStatusBateria :456) | log/clogcomum.cpp (+ ieventoslog.cpp) |
| 5876 | 642 | | `comum::CLogComum::LogaGerandoResultados` (:92) | log/clogcomum.cpp |
| 5878 | 20 | | `comum::CLogComum::LogaCopiandoArqResParaFI` (:76) | log/clogcomum.cpp |
| 5879 | 885 | | `comum::IEventosLog::ConverteERelatoriosUE` (:426) | log/ieventoslog.cpp |
| 5886 | 199 | | `comum::md::CIdentificacaoUrna::CIdentificacaoUrna` (:25/:28/:31) | md/cidentificacaourna.cpp |
| 5887 | 160 | * | `comum::md::CIdentificacaoSecao::CIdentificacaoSecao` (:38/:41) | md/cidentificacaosecao.cpp |
| 6045 | 508 | | merged body of `LogaCopiandoArqResParaFI` / `LogaResultadoCopiadoResFE` | log/clogcomum.cpp (comment) |
| 6236 | 132 | | library: musl `utimes()` (→ `__futimesat` → `utimensat`) | reconhecimentobiometrico/ccontrolaarmazenamentodeimagens.cpp (comment) |
| 10888 | 12 | | `api::CFormPart::~CFormPart()` (thunk → 2899) | u24-foreign-fragments.cpp (orig. api/gui/cformpart.cpp, path inferred) |
| 11205 | 12 | | `comum::CParteEleitores::~CParteEleitores()` (thunk → 2899) | u24-foreign-fragments.cpp (orig. comum/relatorios/cparteeleitores.cpp, path inferred) |
| 11412 | 203 | | `comum::asn::CConversorSeguranca::DoDesconverte` (slot 3) | u24-foreign-fragments.cpp (orig. comum/asn/cconversorseguranca.cpp, path inferred) |
| 11464 | 13 | | `comum::servico::CJustificadorServico::~CJustificadorServico()` (D0, slot 1) | justificativa/cjustificador.cpp (comment) |
| 11468 | 12 | | `comum::dao::CJustificadorDAO::Clonar` (slot 2) | justificativa/dao/cjustificadordao.cpp |
| 11469 | 1662 | | `comum::dao::CJustificadorDAO::RecuperarTodos` (slot 8) | justificativa/dao/cjustificadordao.cpp |
| 11470 | 1399 | | `comum::dao::CJustificadorDAO::Recuperar` (slot 7) | justificativa/dao/cjustificadordao.cpp |
| 11471 | 51 | | `comum::dao::CJustificadorDAO::Atualizar` (slot 6, :63) | justificativa/dao/cjustificadordao.cpp |
| 11472 | 51 | | `comum::dao::CJustificadorDAO::ExcluirID` (slot 5, :56) | justificativa/dao/cjustificadordao.cpp |
| 11473 | 412 | | `comum::dao::CJustificadorDAO::Inserir` (slot 3) | justificativa/dao/cjustificadordao.cpp |
| 11475 | 164 | | `comum::CJustificadorDadoTitulo::Text()` (:104) | justificativa/cjustificador.cpp |
| 11477 | 37 | | atexit destructor of `CJustificador::s_pInstancia` (`~unique_ptr<CJustificador>`) | justificativa/cjustificador.cpp (comment) |

Related functions outside the unit identified while reading: 948 = `CLogComum::GetInst`, 1391 =
`CJustificador::GetInst`, 3742 = `CJustificador::CJustificador` (u20 agrees), 5888 = `CIdentificacaoUrnaContingencia`
ctor, 5168 = `CPlainText` delegating ctor (0, 1, …), 2742 = WSQ-codec empty singleton, 11640 / 11660 = atexit
resets of the `CLogComum` / `CArquivosResultado` singletons.

## 14. Open questions

* Exact names of 1705 / 3744 (`MontaNome` overload, `MontaNomesEleicao`) and of the first parameter struct of 3744
  (fase +0, uf +8; u02 calls the same shape `SIdentificacaoCarga`); why 3744 returns a map with at most one entry.
* The method of the WSQ codec singleton (2742) called in `Armazena` and whether it really is an identity in the
  urna build.
* The name of the function that wraps `SaveCurrentInternal` in 10590 (the MI/MV guards) and whether it belongs to
  `CJustificador` or to `CPedeAnoNascimento`.
* Error code 8803 (unused) and the code of the folded line-33 check of `CIdentificacaoSecao` (8915 assumed).
* Why the key-file name in 2725 is a `string_view` at address 117 (below `GLOBAL_BASE` 1024) instead of a
  `.rodata` literal. That the name is `wsq.pk1` is now strongly supported: `/dsk/fi/estatico/chave/wsq.pk1` is in
  every UF block of all four 2026 urna lists, and `bio.sk1` is the only other 7-character key name there (2026 urna
  data: investigation/README.md, finding A3).
* How 7787 handles the missing UF-level `-pu.dat` files on real media, which carry only the national `o00000br-pu`
  (2026 urna data: investigation/README.md, finding E13).
