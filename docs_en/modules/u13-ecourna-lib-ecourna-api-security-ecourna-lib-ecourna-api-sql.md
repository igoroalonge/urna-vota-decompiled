# u13: `ecourna::api::sql` (SQLite wrapper), `ecourna::api::util` (CStringUtils, CBitArray) and `ecourna::api::security` (SHA-512, SipHash MAC, CKey, ISymmetricCipher)

Unit u13 has 100 wasm functions. Most come from the TSE library **ecourna**
(`/home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/api/…`). The unit builder also filed a few
functions from other places here, because they call or inline ecourna helpers:

| group | functions | what |
|---|---:|---|
| `api/sql/sqlite/*` | 50 | the only database layer of the voting application: `CSqlConnection`, `CSqlStatement`, `CSqlResultSet` over SQLite 3.50.4 |
| `api/util/*` | 16 | `CStringUtils` (the number parsers used everywhere: typed candidate numbers, ASN.1 dates, DAO columns, hex) and `CBitArray` (bit reader for biometric templates) |
| `api/security/*` | 10 | `CSha`/`CSha512` (SHA-512 of the BU QR codes and of the RDV key derivation), `CSiphashMac` (the MAC behind the BU's *Código Verificador*), `CKey`, `ISymmetricCipher` |
| uenux2 code filed here | 9 | the RDV consistency check (`CIntegridadeReferencial`, 6.9 KB), `CCalculaCV::Calcula` (the *Código Verificador*), DataJE date conversion, `CDate` validation, the voter's year of birth, the null-vote predicate, the candidacy map key |
| library code | 15 | libc++ map/charconv/`<format>` 128-bit integers, boost::date_time, musl `isxdigit`, two error-constructor thunks |

Reconstructed sources (all new, written by u13):

* `src/ecourna/api/sql/{esqlerror.hpp,isqlconnection.hpp}`, `src/ecourna/api/sql/sqlite/csql{connection,statement,resultset}.{hpp,cpp}`
* `src/ecourna/api/util/cstringutils.{hpp,cpp}`, `src/ecourna/api/util/cbitarray.{hpp,cpp}`
* `src/ecourna/api/security/{csha,ckey,isymmetriccipher,imacalgorithm}.{hpp,cpp}`, `src/ecourna/api/security/csiphashmac.cpp`
* fragments of files owned by other units, named `*.u13.cpp`: `uenux2/src/app/comum/dados/cintegridadereferencial.u13.cpp`,
  `…/comum/relatorios/ccalculacv.u13.cpp`, `…/comum/asn/util.u13.cpp`, `…/comum/dados/asn/processoeleitoral/cconversorpleito.u13.cpp`,
  `…/comum/dados/md/rdv/cvoto.u13.cpp`, `…/comum/dados/md/eleitor/celeitor.u13.cpp`, `…/comum/dados/ccandidaturas.u13.cpp`,
  `uenux2/src/api/util/cdate.u13.cpp`.

24 of the 100 functions were seen executing in the recorded votes (`analysis/runtime/*.functions.tsv`):
1142, 1523, 1879, 1939, 2201, 2202, 2276, 3506, 3508, 3515, 3518, 3519, 5155, 5166, 5177, 5479, 6153, 9406,
9417, 9429, 9430, 9449, 9460 and 11365. Almost all of them run during `votaInit`. The large start-up
function 7787 (`vota::CInformacaoEleitor::Inicializar`, u02; formerly shown by the tools as `CHKDFSeed::GetSeed`,
one of the many functions inlined into it) opens `uenux.db`
(about 390 profiler samples in the `CSqlConnection` constructor and 500 in `Prepare`), hashes with SHA-512
for the RDV key, and decodes dates from the election files. During voting only the number parsers run:
the `votaTick`/`votaGetStateJson` host (func 5500) calls func 1939 and `ToWord`, and `ToDWord` was sampled once,
called by `vota::CPedeMajoritario::ProcessInputAudio` (func 11683), in the municipal vote.

Portuguese terms: *urna* = voting machine; *seção* = polling section; *mesário* = poll worker;
*comparecimento* = attendance; *justificativa* = justification of absence; *título* (de eleitor) = voter
registration number; *BU* (*boletim de urna*) = the tally the urna prints and records at the close;
*RDV* (*registro digital do voto*) = the shuffled record of every vote; *cargo* = office; *legenda* =
party-only vote; *nulo*/*branco* = null/blank vote; *candidatura apta/inapta* = eligible/ineligible candidacy;
*consulta* = referendum, whose *respostas* are the options; *pleito* = election round; *encerramento* = the
close of the vote.

---

## 1. Classes and how they relate (RTTI)

```
ecourna::api::pattern::NonCopyable
 ├─ sql::ISqlConnection                 typeinfo @1116204
 │    └─ sql::sqlite::CSqlConnection    typeinfo @1116148  vtable @1115936   24 bytes
 ├─ security::IHash                     typeinfo @1113416
 │    └─ security::CSha                 typeinfo @1113372  vtable @1113348   28 bytes
 │         └─ security::CSha512         typeinfo @1113472  vtable @1113316   (ctor func 2684: "SHA2-512", 64)
 ├─ security::IMacAlgorithm             typeinfo @1114792  vtable @1114744
 │    └─ security::CSiphashMac          typeinfo @1113620  vtable @1113604   (no data members)
 └─ security::ISymmetricCipher          typeinfo @1114024  vtable @1114100   32 bytes (vptr + CAesKey)
      └─ CBlockCipher<CAesCipher, CTrng>  (unit u01)
sql::ISqlStatement (no base)            typeinfo @1117204
 └─ sql::sqlite::CSqlStatement          typeinfo @1117148  vtable @1116948   20 bytes
sql::ISqlResultSet (no base)            typeinfo @1116896
 └─ sql::sqlite::CSqlResultSet          typeinfo @1116840  vtable @1116620   32 bytes
util::CBitArray (no base)               typeinfo @1566540  vtable @1566532   40 bytes
non-polymorphic: util::CStringUtils (static members only), security::CKey (48 bytes), security::CKeyData (12 bytes)
error types:     CBaseError<sql::ESqlError,  SErrorLimits{1725,1775}>  typeinfo @1115992, thunk func 9461
                 CBaseError<util::EUtilError, SErrorLimits{1875,1900}> typeinfo @1117592, thunk func 9410
                 CBaseError<security::ESecurityError, {1325,1725}>     typeinfo @1112704, thunk func 9505 (u01)
```

Ownership: every SQL object lives in a `std::shared_ptr` of its interface type. Libc++ emits one
`__shared_ptr_pointer` control block per pair: `…<CSqlConnection*, shared_ptr<ISqlConnection>>` @1559008
(held by the DAOs), `…<CSqlStatement*>` @1116292 and `…<CSqlResultSet*>` @1117256. The statement and the
result set keep **raw** `sqlite3*`/`sqlite3_stmt*` pointers. Nothing ties their lifetime to the connection.

### 1.1 Layouts

| class | layout (wasm32) |
|---|---|
| `CSqlConnection` | `+0 vptr`, `+4 std::string m_arquivo` (DB path), `+16 sqlite3* m_db`, `+20 bool m_fechada` (closed) |
| `CSqlStatement` | `+0 vptr`, `+4 sqlite3* m_db`, `+8 sqlite3_stmt* m_comando`, `+12 bool m_fechado`, `+16 int m_qtdParametros` |
| `CSqlResultSet` | `+0 vptr`, `+4 sqlite3*`, `+8 sqlite3_stmt*`, `+12 bool m_temLinha`, `+13 bool m_aguardandoNext`, `+16 int m_qtdColunas`, `+20 std::map<std::string,int> m_colunas` |
| `CSha` | `+0 vptr`, `+4 std::string m_algoritmo` ("SHA2-512"), `+16 size_t m_tamanho` (64), `+20 unique_ptr<EVP_MD_CTX, void(*)(EVP_MD_CTX*)>` (ptr +20, deleter `EVP_MD_CTX_free` +24) |
| `CKey` | `+0 std::string titular`, `+12 uedword idParChaves`, `+16 KeyType tipo`, `+20 vector<uebyte> bytes`, `+32 std::string tag`, `+44 uedword` (always 0) |
| `CKeyData` | `+0 vector<uebyte> bytes` |
| `ISymmetricCipher` | `+0 vptr`, `+4 CAesKey{ +4 key, +16 keySize (bits), +20 iv }` |
| `CBitArray` | `+0 vptr`, `+4 shared_ptr<boost::dynamic_bitset<uebyte>>` (+8 control), `+12 vector<uebyte>` (?), `+24 bool leitura` (=1), `+25 bool` (=0), `+32 ueqword posição` (bits) |

### 1.2 Virtual slot orders and how the names were recovered

The SQL interfaces have no srcloc of their own. The slot order comes from the vtables. The names come from the
srclocs of the implementation methods (`Prepare`, `Close`, `SetInt`…`SetNull`, `Reset`, `GetInt`…`GetBlob`),
from the SQL text each slot runs (`begin`/`commit`/`rollback transaction`, `PRAGMA integrity_check(1)`),
and from the error text "Chame o método 'Next()'…".

| class | slots |
|---|---|
| `ISqlConnection` | 0 ~, 1 ~D0, 2 `BeginTransaction`\*, 3 `Commit`\*, 4 `Rollback`\*, 5 `Prepare`, 6 `IsClosed`\*, 7 `Close`, 8 `CheckIntegrity`\*, 9 `GetPath`\* (ICF body 5322 "return this+4") |
| `ISqlStatement` | 0 ~, 1 ~D0, 2 `SetInt`, 3 `SetInt64`, 4 `SetDouble`, 5 `SetText`, 6 `SetDateTime`\*, 7 `SetBool`\*, 8 `SetBlob`, 9 `SetNull`, 10 `Execute`\*, 11 `Reset`, 12 `Close`, 13 `IsClosed`\* |
| `ISqlResultSet` | 0 ~, 1 ~D0, then pairs (by index, by column name): 2/3 `GetInt`, 4/5 `GetInt64`, 6/7 `GetDouble`, 8/9 `GetText`, 10/11 `IsNull`, 12/13 `GetDateTime`\*, 14/15 `GetBool`\*, 16/17 `GetBlob`, 18 `Next`\* |
| `IHash`/`CSha` | 0 ~, 1 ~D0, 2 `Reset`, 3 `Update`, 4 `Finish`, 5 `GetName`\* (ICF body 3874 "copy the string at this+4") |
| `IMacAlgorithm` | 0 ~ (trivial), 1 ~D0, 2 `DoMac`, 3 `DoVerify` (srcloc names); the public non-virtual `Mac` is inlined into its caller |

\* name inferred from behaviour.

---

## 2. The SQL layer (`ecourna::api::sql::sqlite`)

### 2.1 Where it is used

The voting application stores its **dynamic data** in SQLite files named `uenux.db`, one per flash
memory in the work directory: `/dsk/fi/dinamico/trab1/uenux.db` (internal flash) and `/dsk/fe/dinamico/trab1/uenux.db`
(external flash/memory card). The DAOs of other units build a `CSqlConnection` over those paths
(`operator new(24)` in func 3768 `CEleitorDinamicoDAO::CEleitorDinamicoDAO` and in the start-up function 7787)
and issue their own SQL through `Prepare`. After `votaInit`, the snapshot DBs (`analysis/runtime/memfs-after-*/…/uenux.db`)
contain exactly these tables, both empty after the recorded votes:

```sql
CREATE TABLE comparecimento_mesario (numero_titulo BIGINT NOT NULL,
  tipo_identificador INTEGER CHECK(tipo_identificador IN (1,2,3)) NOT NULL, periodo_presente INTEGER NOT NULL,
  pertence_secao INTEGER NOT NULL, estado_biometria INTEGER NOT NULL, dedo_habilitacao INTEGER NOT NULL,
  data_hora_registro DATETIME NOT NULL, id_arquivo INTEGER, UNIQUE (numero_titulo, periodo_presente));
CREATE TABLE registro_justificativa (numero_titulo BIGINT PRIMARY KEY UNIQUE NOT NULL, ano_nascimento SMALLINT);
```

A third table, used by `CEleitorDinamicoDAO`, holds the per-voter attendance state: `( titulo BIGINT PRIMARY
KEY UNIQUE NOT NULL, tipo_identificador …)` @449289, with its name in a global string. That DAO did not run in
the recorded sessions. The DAOs' SELECT/INSERT texts are at @442244 and @448347–449081. `integrity_check`
returns `ok` on both snapshots, and `foreign_keys` is off in a fresh CLI session because it is a per-connection pragma.

### 2.2 Connection life cycle (func 3515, 5167, 9458)

```
CSqlConnection(path):  m_fechada = true
   sqlite3_open(path, &db)            (default flags: READWRITE|CREATE, a missing file is created)
     ≠ OK  → msg = errmsg(db); sqlite3_close(db); throw 1725 msg                 (line 29)
   m_fechada = false
   try   Prepare("PRAGMA foreign_keys = ON")->Execute(); stmt->Close()
   catch(...) → msg = errmsg(db); sqlite3_close(db); m_fechada = true; throw 1726 msg   (line 44)
~CSqlConnection:      if (!m_fechada) try Close() catch(...) {}
Close():              sqlite3_close(db) ≠ OK → throw 1728 errmsg (line 103); m_fechada = true
BeginTransaction/Commit/Rollback: Prepare("begin|commit|rollback transaction")->Execute(); Close()
CheckIntegrity:       try Prepare("PRAGMA integrity_check(1)")->Execute(); Close(); return true
                      catch(...) return false                    ← the result row is never read (§8)
Prepare(sql):         sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) ≠ OK → throw 1727
                      "Falha ao executar operação no banco de dados.\nMensagem do banco de dados: '{}'\nOperação: '{}'"
                      return shared_ptr<ISqlStatement>(new CSqlStatement(db, stmt))
```

`GetErrorMessage` (func 9411) returns `sqlite3_errmsg(db)`, or "Erro desconhecido do SQLite." when that is NULL.
Every SQL error message comes from it.

### 2.3 Statement → result set protocol

`Execute()` (func 9417) returns `shared_ptr<ISqlResultSet>(new CSqlResultSet(db, stmt))`. The result set's
**constructor steps once** (func 9449 → `StepStatement`). So `Execute()` alone runs DDL/DML, and every
DAO and the connection itself use it like that. Reading follows a small state machine:

```
             ctor: m_temLinha = StepStatement(); m_aguardandoNext = true
                     │
   GetX(...) ───────►│ m_aguardandoNext ⇒ throw 1729..1734 "Chame o método 'Next()' antes de buscar resultados."
                     ▼
   Next() #1:  m_aguardandoNext = false; return m_temLinha        (the pre-fetched step)
   Next() #n:  return StepStatement()                             (m_temLinha is not updated)
   GetX(i):    VerifyColumnIndex(i) (0 ≤ i < column count, else 1735); sqlite3_column_*(stmt, i)
   GetX(name): i = IndexForColumnName(name) → m_colunas.find(ToLower(name)) (else 1736 "Coluna {} não faz parte do resultado.")
```

`StepStatement` (line 185) runs `sqlite3_step`, refreshes `m_qtdColunas`, and **rebuilds** the name→index
map from `sqlite3_column_name`, lower-cased with the Latin-1-aware `CStringUtils::ToLower` (func 1879), on
every row. It returns `rc == SQLITE_ROW` for ROW/OK/DONE. For any other code it throws 1737 with the same
two-line message as `Prepare`: the SQLite message and `sqlite3_sql(stmt)`, or "INDISPONÍVEL" when that is NULL.

The getters do not check `m_temLinha`. After `Next()` returned false they return SQLite's defaults (0, "", {}).

| getter | implementation |
|---|---|
| `GetInt`/`GetInt64`/`GetDouble` | `sqlite3_column_int/int64/double` |
| `GetText` | `text = sqlite3_column_text`; `bytes != 0 && text` ? `std::string(text)` (strlen: stops at an embedded NUL) : `""` |
| `IsNull` | `sqlite3_column_text == NULL && sqlite3_column_bytes == 0`. Only SQL NULL gives a NULL text pointer: a zero-length TEXT or BLOB gives `""` (tested), so the answer is right, but the value is converted to text in place |
| `GetBlob` | `sqlite3_column_blob` + `bytes` → vector (empty when 0 bytes) |
| `GetBool` | `GetInt(i) == 1` (by name: `GetInt(IndexForColumnName(n)) == 1`) |
| `GetDateTime` | `GetText`, then replace `' '`→`"T"`, drop `'-'` and `':'` (func 9398) → `boost::date_time::parse_iso_time<ptime>(s, 'T')` |

Setters (1-based parameter indices; `VerifyParameterIndex` → 1746 "Índice do parâmetro fora do intervalo válido."
when `i ≤ 0 || i > sqlite3_bind_parameter_count`) call `sqlite3_bind_int/int64/double/text(SQLITE_TRANSIENT)/blob(SQLITE_TRANSIENT)/null`
and throw 1738…1743 with `GetErrorMessage`. `SetBool(i, b)` = `SetInt(i, b)`.
`SetDateTime(i, ptime)` = `to_iso_extended_string(t)` with `'T'` → `" "`, then `SetText`. This is the only
source of the text format `YYYY-MM-DD HH:MM:SS[.ffffff]` used for `comparecimento_mesario.data_hora_registro`.
Special values are stored as `not-a-date-time`/`±infinity`.
`Reset` → `sqlite3_reset` (1744), and bindings are kept. `Close` → `sqlite3_finalize` (1745) under a global
`std::mutex` (@1911796). The landing pads show an explicit `lock(); try {…} catch(...) { unlock(); throw; } unlock();`.

### 2.4 `ESqlError` codes (1725–1775 range)

The values are certain. They are numbered in source order, file by file.

| code | site (line) | message |
|---|---|---|
| 1725 | CSqlConnection ctor (29) | `sqlite3_errmsg` of the failed open |
| 1726 | CSqlConnection ctor (44) | `sqlite3_errmsg` after `PRAGMA foreign_keys = ON` failed |
| 1727 | Prepare (86) | "Falha ao executar operação no banco de dados.\nMensagem do banco de dados: '{}'\nOperação: '{}'" |
| 1728 | CSqlConnection::Close (103) | `sqlite3_errmsg` |
| 1729–1734 | GetInt (39), GetInt64 (50), GetDouble (61), GetText (72), IsNull (90), GetBlob (128) | "Chame o método 'Next()' antes de buscar resultados." |
| 1735 | VerifyColumnIndex (143) | "Índice da coluna fora do intervalo válido." |
| 1736 | IndexForColumnName (153) | "Coluna {} não faz parte do resultado." |
| 1737 | StepStatement (185) | same text as 1727 |
| 1738–1743 | SetInt (53), SetInt64 (63), SetDouble (73), SetText (83), SetBlob (109), SetNull (119) | `sqlite3_errmsg` |
| 1744 / 1745 | Reset (133) / CSqlStatement::Close (143) | `sqlite3_errmsg` |
| 1746 | VerifyParameterIndex (158) | "Índice do parâmetro fora do intervalo válido." |

---

## 3. `ecourna::api::util`

### 3.1 `CStringUtils` number parsers

All of them trim a copy of the argument (func 9406: removes every byte ≤ 0x20, control characters included,
at both ends), then check that it is not empty and contains only allowed characters, then call
`strtoull`/`strtoll` with `errno = 0`. They throw `CBaseError<EUtilError>` with the **caller's**
`std::source_location` (a defaulted argument of the shared body; that is why all the srclocs have column 12,
the column of `return ToUnsigned(`). The message is always `"valor invalido [" + original + "]"`:

| function (line) | body | base | allowed chars | range | result |
|---|---|---|---|---|---|
| `ToByte` (714) | 6153 → 2202 | 10 | `+-0123456789` | ≤ 255 | `& 0xFF` |
| `ToInt16` (719) | 5155 | 10 | same | −32768…32767 | sign-extended |
| `ToWord` (724) | 6153 → 2202 | 10 | same | ≤ 65535 | `& 0xFFFF` |
| `ToInt32` (729) | 5155 | 10 | same | int32 | |
| `ToQWord` (734) | 2202 | 10 | same | ≤ 2⁶⁴−1 (only ERANGE can fail) | |
| `ToDWord` (739) | 2202 | 10 | same | ≤ 2³²−1 | |
| `HexToInt` (744) | 2202 | 16 | `xXabcdefABCDEF0123456789` | ≤ 2³²−1 | |
| `HexToQWord` (754) | inlined in 3700 | 16 | same | ≤ 2⁶⁴−1 | |

Error codes: 1875 empty/invalid character, 1876 `*end != '\0'` (e.g. `"1-2"`), 1877 out of range or
`errno == ERANGE` (68 in Emscripten's WASI numbering). The unsigned parsers accept a leading `-`, which
`strtoull` negates modulo 2⁶⁴: `ToQWord("-5")` returns 18446744073709551611 (§8).

The allowed-character sets are local arrays: each wrapper (and the merged body 6153, and func 3700 for the inlined
`HexToQWord`) copies the 13- or 25-byte literal onto its stack frame and passes the copy.

`HexStringToBytes` (808): trim; empty → `{}`; any non-`isxdigit` → 1878 `"valor invalido [" + trimmed + "]"`;
then pairs of nibbles, `for (i = 0; i < size-1; i += 2)`. An odd final nibble is silently dropped.
Users: the BU QR-code signer (64 raw bytes from the 128-hex-digit SHA-512, §6.1), `CSerialMidia`, mock data (func 9963).

`ToLower(const std::string&)` (func 1879) is copy + table slot 6425 = func 5158. That is the in-place
lower-caser, which is Latin-1 aware: a lazily built static `std::set<int>` @1911836 of the 19 upper-case
accented letters `Ç Á É Í Ó Ú À È Ì Ò Ù Â Ê Î Ô Û Ã Õ Ñ` get `|= 0x20`, and other letters go through
`isalpha`/`tolower`. It is the lower-case twin of the upper-caser described by u02 (funcs 5156/3509).
Callers: SQL column names, `FormataUF` (file names use the lower-case UF), funcs 3753, 5726 and 5728.

### 3.2 `CBitArray` (func 1375)

`GetValor(n)`: needs the read flag (+24), otherwise it throws 1882 with the message "sem permissao de
**escrita** no array de bits" (the text says *write*). It returns `GetValor(posição, n)` and advances the cursor.
`GetValor(pos, n)`: 1883 if there is no bitset, 0 if `n == 0`, 1884 "busca no vetor em posicao invalida" if
`pos + n > size`. Otherwise it reads MSB-first: bit `pos+i` → bit `n−1−i`. Only user: `comum::asn::CConversorDedo`
(func 5702), which unpacks the fingerprint templates of the voter roll.

---

## 4. `ecourna::api::security`

* **`CSha` / `CSha512`** (funcs 5177, 3519, 3518, 5175, 5176). `Reset` = `EVP_DigestInit_ex2(ctx,
  EVP_get_digestbyname("SHA2-512"), NULL)` → 1417 "Falha ao iniciar o contexto.". `Update` rejects empty
  input (1418 "Não é possível calcular o hash de dados vazios."), `EVP_DigestUpdate` failure → 1419.
  `Finish` returns a `const std::vector<uebyte>` of `m_tamanho` bytes (64), `EVP_DigestFinal_ex` failure → 1420.
  All three ran at `votaInit` (RDV key derivation, host func 7787).
* **`IMacAlgorithm::Mac`** (imacalgorithm.cpp:32/35, only inlined): 1432 "Vetor de dados vazio.", 1433 "Chave vazia.",
  then virtual `DoMac`. **`CSiphashMac::DoMac`** (func 9498): `EVP_MAC_fetch("siphash")`, params
  `size=8, c-rounds=4, d-rounds=6` in a heap `std::vector<OSSL_PARAM>(4)`, `EVP_MAC_init/update/final`. Each failure
  throws 1423/1424/1425 `std::format("Falha ao autenticar. - {}", ERR_get_error())` with the error *number*.
  **`DoVerify`** (func 9497): key must be 16 bytes (1426 "O tamanho da chave deve ser igual 16 bytes."),
  recomputes, compares 8 bytes, and converts any `std::exception` into 1426 "Falha ao autenticar.". It has no caller.
* **`CKey`** (func 9504): copies (titular, idParChaves, tipo, bytes, tag), then 1412 "Titular da chave vazio.",
  1413 "Identificador do par de chaves igual a zero.", 1414 "Bytes da chave vazio.". `CKeyData(bytes)` (ckey.cpp:31,
  inlined into 12110) → 1411 "Bytes da chave vazio.". Both destructors zero the key bytes (inlined `memset`).
* **`ISymmetricCipher(const CAesKey&)`** (func 9474): copies the key. Then 1445 `std::format("Incompatibilidade entre o
  tamanho da chave e a quantidade de bits [{}/{}].", key.size(), (ueword)keySize)` if `keySize != 8·key.size()`, and
  1445 "Quantidade de bits nula." if `keySize == 0`. Destructor func 9484 is ICF-shared with `CBlockCipher` (u01).

`ESecurityError` codes used here: 1411–1414, 1417–1420, 1423–1426, 1432–1433 and 1445 (u01 lists the others).

---

## 5. uenux2 functions filed in this unit

* **`CIntegridadeReferencial::VerificaVotosRdv`** (func 11514, name inferred) checks every vote of the RDV
  against the static data and returns `{ok, erro}`. It is called by func 2543 → `Lanca` (func 2263, code 7871
  "Integridade referencial: {}"), after `CEleitores::CompleteLoad`, in `CSincronismoVotoEleitor::vf2` and in
  **`vota::CGravaResultado::vf2`**, i.e. before the results are written at the close. The rules, in order
  (RDV `TipoVoto`: legenda 1, nominal 2, branco 3, nulo 4, …, nuloAposSuspensao 6, nuloPorRepeticao 7):

  | vote | check | message "um voto do cargo ({}) no RDV está com …" |
  |---|---|---|
  | any | cargo in the cargo list | "o cargo ({}) no RDV não foi encontrado na lista de cargos" |
  | legenda | cargo has candidates and is proportional | "voto de legenda ({}) com cargo que não é proporcional" |
  | legenda | ≥ 2 digits | "… com conteúdo insuficiente" |
  | legenda | party = first 2 digits exists | "… para partido não encontrado" |
  | legenda | if it has the cargo's full number of digits, no candidacy with that number | "…, mas o candidato existe" |
  | nominal (consulta) | answer `cargo·10⁶+n` exists | "voto nominal ({}) para resposta não encontrada" |
  | nominal | candidacy exists / is apt (situação == 0) | "… para candidatura não encontrada" / "… para candidatura inapta" |
  | nulo 4/6 with full digits (consulta) | no such answer | "voto nulo ({}), mas a resposta existe" |
  | nulo 4/6 with full digits | no such apt candidacy | "voto nulo ({}) para candidatura apta" |

  Branco and the "cargo sem candidato" nulls (8, 9) are not checked. `nuloPorRepeticao` (7) is excluded
  explicitly. A nominal content that is not a number makes `ToDWord` throw `EUtilError` instead of producing an
  integrity error.
* **`CCalculaCV::Calcula`** (func 3700), the *Código Verificador* of the BU: see §6.2 and `docs/bu/codigo-verificador.md`.
* **`Utils::DesconverteDataJE`** (func 2276): `DataJE` "YYYYMMDD" → `api::CDate(ToByte(dd), ToByte(mm), ToWord(yyyy))`.
  `CDate(d, m, a)` (func 2765) reformats "{:02}{:02}{:04}" and parses it again with `strptime("%d%m%Y")`.
  Used by `CConversorPleito::DoDesconverte` (func 11365: `CPleitoDTO{id, nome, data}`), `DesconverteDataHoraJE`,
  `CConversorHorarioVerao`, `CConversorEstadoGeralGap`.
* **`CDate::IsValid`** (func 5479): "DDMMAA" or "DDMMAAAA", digits only, month 1–12, day 1…days-in-month (Gregorian leap
  rule). A 2-digit year gets +2000, so "010170" is 1 Jan **2070**. The 2000 is a 16-bit variable @1584256 that both
  `IsValid` and `CDate::CDate(const std::string&)` (func 3649) load from memory (`i32.load16_u`), not an immediate, so
  it is not a compile-time constant in that source file.
* **`CEleitor::GetAnoNascimento`** (func 5665): `ToDWord(dataNascimento.substr(0,4))`. The string at +68 is the
  `DataJE` birth date, which confirms u05's guess. The poll worker's "ano de nascimento" screens compare it with the typed year.
* **`CVoto::EhNulo`** (5645 static: tipo ∈ {4, 6, 7}; 11316 member, passed as a pointer-to-member to the
  `CVotosEleicoesVota::Nulos` counter) and **`CCandidaturas::Chave`** (1939: `cargo·1000000 + número`, the key
  of the candidacy and answer maps; the web build's `votaGetStateJson` uses it to list the candidate numbers).

---

## 6. BOLETIM DE URNA: what this unit contributes

This unit does not build the BU. It supplies four of the BU's integrity mechanisms. The other units
(`vota::CGeraBU`, `comum::CGeradorBUQRCode`, `CGravaResultado`) and `docs/bu/*.md` describe the whole flow.

### 6.1 QR codes: SHA-512 chain and signature input (`CSha512`, `HexStringToBytes`)

In `CGeradorBUQRCode::GeraQRCodes` (func 5604), for each QR slice `b` (docs/bu/qrcode.md §5):

1. `msg = b == 0 ? slice[0] : join(done, " ") + " " + slice[b]`;
2. `CSha512 sha;` (func 2684: `EVP_MD_CTX_new`, `Reset`), then `sha.Update(chunk)` for each 512-byte chunk of `msg`
   (`Update` refuses an empty chunk, and the chunks are never empty), `digest = sha.Finish()` (64 bytes);
3. `hex = ToHex(digest)` upper-case (func 1243) → `done.push_back(slice + " HASH:" + hex)`;
4. for the **last** slice only: `CStringUtils::HexStringToBytes(hex)` turns the hex back into the 64 raw digest bytes,
   which `api::pkcs11::IPkcs11` vtable slot 6 signs. The result is appended as ` ASSI:<hex>`. The algorithm, Ed25519,
   comes from the TSE manual and the official BUs; the wasm does not show it.

So `HASH_k = SHA-512(c1 " HASH:" h1 " " … " " ck)` and `ASSI = sign(bytes(HASH_last))`. No class in this binary
implements `IPkcs11` (docs/bu/qrcode.md §6, u05), so the web build cannot run the signing step.

### 6.2 *Código Verificador* (`CKey`, `IMacAlgorithm::Mac`, `CSiphashMac`, `HexToQWord`)

1. **Key** (inlined into `vota::CGeraBU::vf2`, func 12110): read `/dsk/fi/estatico/chave/cv.ber.pri`
   (`ModuloEnvelopeChave::EntidadeChave`), decipher `chave` with the AES cipher derived from the HSM secret (u01),
   build `CKey(descritor.nomeUsuario, descritor.serial, tipo 0/1/−1, bytes, tagChaves)` (func 9504, checks 1412–1414)
   → `CKeyData` (1411) → the first 16 bytes become `CCalculaCV::m_chave`. Both key objects are zeroed on destruction.
2. **Each code** (func 3700, `CCalculaCV::Calcula`):
   `m_dados += to_string(++m_contador)` → `CSiphashMac::DoMac(bytes(m_dados), tag, m_chave)` = SipHash-4-6, 8 bytes →
   `hex = Σ format("{:02x}", byte)` (these two steps are a separate function inlined with its own 416-byte stack frame) → `v = HexToQWord(hex)` (the tag read big-endian) → `Reinicia(to_string(v))`
   (the chain restarts from the full 64-bit value) → return `format("{:010}", v % 10¹⁰)`, printed as `d.ddd.ddd.ddd`.
3. What goes into `m_dados` for each BU block, and the tests against the 74 official codes: `docs/bu/codigo-verificador.md`.
   `DoVerify` is never called: VOTA has no code that verifies a CV.

In the web build `cv.ber.pri` and `api::IKernelHSM` are missing, so step 1 would throw before any code is computed (u01 §5).

### 6.3 RDV consistency before the results are written

`vota::CGravaResultado::vf2` (the *encerramento* step that writes the result files) calls func 2543 →
`VerificaVotosRdv` (func 11514, §5). Any inconsistent RDV vote aborts with `EUeComumDadosError` 7871
"Integridade referencial: um voto do cargo (…) no RDV está com …". This check does not run in the recorded
sessions, and the web build does not store votes in the RDV (`CSincronismoVotoEleitorWeb`).

### 6.4 Dates in the BU header

The pleito date that goes into the BU identification and into the CV EXTRAS (`YYYYMMDD`) comes from
`CConversorPleito::DoDesconverte` → `DesconverteDataJE` (funcs 11365/2276), which ran at start-up.

---

## 7. Web-build specifics and WebAssembly/Emscripten observations

* **SQLite on MEMFS.** `uenux.db` exists in both `/dsk/fi` and `/dsk/fe`, in the browser's memory only. Nothing is
  sent over the network. The `std::mutex` around `sqlite3_finalize` does nothing in this single-threaded build:
  `lock()` compiles to nothing and only the `unlock()` residue (func 150) remains. Func 9431 (`~mutex` at exit,
  table slot 6389) is registered but never referenced by any code.
* **Data-segment addressing in the pseudo-code.** Expressions like `d_operator0033s040504040x0505T[326061]:long@1`
  are relative to the start of a data segment, which is 1024 for that segment. Add 1024: 326061 → 327085
  "PRAGMA foreign_keys = ON". `d_ABCDEFGHIJKLMNOPQRSTUVWXYZab[660]:long` is 8-byte-scaled from the segment at 1112848
  (the Base64 alphabet), so it points to 1118128 "xXabcdefABCDEF0123456789". `q.py wat` shows the real addresses.
* **Misleading annotations.** Error codes that equal a string address get a string comment: `1730 /* "QRBU:{}:{} VRQR:{} {}" */`
  in `GetInt64` and `1411 /* "Quantidade de escolhas…" */` in func 12110 are the codes 1730 and 1411, not strings.
* **Packed `std::format` argument types.** 5 bits per argument: `6` = unsigned, `13` = string_view. `422` = (unsigned,
  string_view) in 11514, `198` = (unsigned, unsigned) in `ISymmetricCipher`, `13` = one string_view in `IndexForColumnName`.
  The format string is the other 8-byte constant: `399432394920` = `{data 436392, size 93}` is the "Falha ao executar
  operação…" text.
* **Latin-1 strings.** "Índice", "não", "INDISPONÍVEL" are stored as single bytes (`cd`, `e3`, `cd`). The TSE code is compiled
  with a Latin-1 execution character set.
* **merge-similar-functions.** `ToByte`/`ToWord` share body 6153 (mask and max are parameters). `ToLower`/`Trim`/`ToUpper` copies
  share body 3942 (the transform is a table slot).
* **ICF.** `ISymmetricCipher::~` = `CBlockCipher::~` (9484). `CSha::GetName` = `CSubReport` slot 2 = a `std::function`
  slot (3874). `CSqlConnection::GetPath` = an RHVoice `leaf_node` slot (5322).
* **Inlining hosts.** 3700 carries the srclocs of `imacalgorithm.cpp` and `cstringutils.cpp:754` but is `CCalculaCV`. Each
  inlined callee keeps its own stack frame (nested `stack_pointer -= n` … `+= n`), which shows that the MAC-and-hex step is a
  separate function (416 bytes) that contains `Mac` (32 bytes). 1375 is both `GetValor` overloads. 12110 contains `CKeyData`, `CKey::GetKeyData` and both key destructors.
* **Direct vs virtual calls.** `CSiphashMac::DoVerify` calls `DoMac` directly (table slot 6253). The by-name result-set getters
  call the by-index getter directly. Every call from the DAOs goes through the vtable.
* **Exceptions.** The SQL error paths build the message, then the exception, through `invoke_*`. `CSqlStatement::Close`'s
  `catch(...){unlock; throw;}` becomes `__cxa_begin_catch` + `__cxa_rethrow`.
* **errno** is musl's @1931660, and `ERANGE` = 68, `EOVERFLOW` = 61 (WASI numbering).
* **Library code filed here.** boost::date_time is used by `SetDateTime` (1376, 5182, 5183, 9414). libc++'s charconv
  (2866, 5919, 3846) serves `std::to_string(unsigned long long)`. The libc++ `<format>` 128-bit integer formatter
  (1949, 5912) is linked but has no TSE caller.

---

## 8. Weird or risky code

The ordering is by severity. "Latent" means no caller in this binary reaches the path. The same library code probably runs on the urna.

1. **`CheckIntegrity` never reads the result (func 9452, medium, latent).** It prepares and steps
   `PRAGMA integrity_check(1)`, closes the statement and returns `true`. SQLite reports integrity problems as
   an ordinary result row (the text of the first problem) with `SQLITE_ROW`, not as an error. So a damaged
   `uenux.db` passes this check unless SQLite fails outright. No static caller exists: the method is only
   reachable through the vtable, and it did not run in the recorded votes.
2. **Use-after-free when the API is misused (funcs 9460, 9458, 9417, latent).** `Close()` leaves the freed
   `sqlite3*` in the object, and `Prepare` does not check `m_fechada`. A result set keeps the raw `sqlite3_stmt*`
   after `CSqlStatement::Close` has finalized it. In wasm this reads freed linear memory; it does not trap.
3. **`CSiphashMac::DoVerify` (func 9497, low, latent).** It compares 8 bytes at `mac.data()` without checking
   `mac.size()`: an out-of-bounds read, or address 0 for an empty vector. The compare is not written as a
   constant-time one (no `CRYPTO_memcmp`), but the fixed length compiles to a single `i64.ne`, so this build has no
   timing leak. Both of its errors use the same code, 1426. Nothing calls it: VOTA never verifies a *Código Verificador*.
4. **The unsigned parsers accept negative input (func 2202; `ToQWord` func 1141, low).** `+`/`-` are in the
   allowed-character set, and `strtoull` negates modulo 2⁶⁴, so `ToQWord("-5")` = 18446744073709551611. This is
   no error, because the maximum is 2⁶⁴−1. The DAOs use `ToQWord` for `numero_titulo` values. `ToDWord("-1")`
   is rejected only because the wrapped value exceeds 2³²−1.
5. **`VerificaVotosRdv` does not validate content first (func 11514, low).** A nominal or null vote whose
   content is not a number makes `ToDWord` throw `EUtilError` 1875/1876, not the integrity error that
   `Lanca` would produce. Blank votes and the "cargo sem candidato" nulls (8, 9) are not checked, and
   `nuloPorRepeticao` (7) is excluded explicitly.
6. **`SetDateTime`/`GetDateTime` round trip (funcs 9422/5162, low).** A special `ptime` is stored as
   `not-a-date-time`/`+infinity`/`-infinity`. `parse_iso_time` (func 5851) runs boost's special-values parser when
   the text starts with `+`, `-`, `n` or `m`, so `+infinity` reads back. The other two do not: stripping `-` gives
   `notadatetime` and `infinity`, and `parse_iso_time` throws a boost exception, not `CSqlError`. Ordinary times
   round-trip.
7. **NULL handling (funcs 9420, 5163, info).** `SetBlob` with an empty vector that never allocated binds SQL NULL
   (`data() == nullptr`), so such an empty blob reads back as NULL. `IsNull` decides through `sqlite3_column_text`,
   which returns NULL only for SQL NULL (a zero-length BLOB or TEXT gives `""`: checked with `x''` and `zeroblob(0)`),
   so its answer is right, but it converts the value to text in place. `GetText` copies with `strlen`, so text
   with an embedded NUL is truncated.
8. **`CDate::IsValid` century rule (func 5479, info).** A 2-digit year always gets +2000, so "010170" is
   1 Jan 2070. Nothing in the unit shows which inputs are 6 digits long.
9. **`HexStringToBytes` drops an odd final nibble silently (func 3506, info).** Its callers (QR signing,
   media serial) pass even-length hex.
10. **`CBitArray::GetValor` (func 1375, info).** The read-mode check reports "sem permissao de **escrita**"
    (write). For more than 16 bits the result is truncated to `ueword`. A count above 32 is a C++ shift UB;
    wasm masks the count.
11. **`sqlite3_open` with default flags (func 3515, info).** A missing `uenux.db` is created empty without any
    notice. On an urna, a lost or replaced flash would look like a fresh database, and the DAOs'
    `CREATE TABLE IF NOT EXISTS` would recreate the tables.
12. **Other points (info).** `CSha::Update` refuses empty data, so the hash of an empty message cannot be
    computed; the BU chunks are never empty. `CKey` accepts `KeyType` −1 (an unknown ASN.1 `tipo`).
    `CSqlConnection`'s constructor discards the caught exception and reports `sqlite3_errmsg` instead. The global
    mutex in `CSqlStatement::Close` shows that the urna finalizes statements from several threads; in this
    build it does nothing.
13. **Privacy (info).** `uenux.db` holds poll-worker attendance (`comparecimento_mesario`: voter-registration
    numbers, biometric state, time) and justifications (`registro_justificativa`: registration number, year
    of birth). In the web build it lives only in MEMFS and nothing in this unit sends it anywhere.

## 9. Open questions

* The real names of the vtable-only methods: `BeginTransaction`/`Commit`/`Rollback`/`IsClosed`/`CheckIntegrity`/`GetPath`,
  `Execute`, `SetDateTime`/`GetDateTime`, `SetBool`/`GetBool`, `Next`, and `IHash` slot 5.
* The file of `GetErrorMessage` (func 9411): a shared internal header, or a `static` member of one of the classes.
* The `ESqlError`/`EUtilError`/`ESecurityError` enumerator names. The values are certain. Every throw site was
  searched: EUtilError uses 1875–1878 and 1882–1884 (this unit) plus 1889 (`csynchronizer.cpp:59`, inlined into
  `CFile::Sync`), and 1879–1881 never occur. ESecurityError 1415/1416, 1421/1422 and 1427–1431 never occur either;
  they probably belong to functions that are not linked.
* `CBitArray` +12 (a `std::vector`, freed by the destructor but unused by `GetValor`) and the +25 flag: probably
  the write side (a `SetValor`), which is not linked.
* `CKey` +44 (always 0).
* Who calls `ISqlConnection::CheckIntegrity` on the real urna. There is no static caller here.
* Whether the pointer passed to func 5665 is meant as `CEleitor`, `CEleitorDecorator` or `CEleitorDetalhe`. All three start
  at offset 0; the method is filed under `CEleitor`, which owns `m_dataNascimento` (+68).
* The +52 field of `md::CCandidatura`: u04 calls it situação/inapto (must be 0); u02 treats it as "no photo".
  The integrity messages of func 11514 ("candidatura apta/inapta") support u04's reading.

## 10. Mapping table (all 100 functions of u13)

`ran` = seen in `analysis/runtime/*.functions.tsv`. "confidence" is the confidence of the name.

| func | size | ran | reconstructed symbol | source file (or why none) | confidence |
|---|---:|:-:|---|---|---|
| 1141 | 62 |  | `ecourna::api::util::CStringUtils::ToQWord` | `src/ecourna/api/util/cstringutils.cpp` | high |
| 1142 | 18 | ✓ | `ecourna::api::util::CStringUtils::ToByte` | `src/ecourna/api/util/cstringutils.cpp` | high |
| 1375 | 993 |  | `ecourna::api::util::CBitArray::GetValor` | `src/ecourna/api/util/cbitarray.cpp` | high |
| 1376 | 552 |  | `boost::gregorian::gregorian_calendar::from_day_number` | library: boost::date_time (year/month/day from a day number, bad_year 1400..9999), instantiated for SetDateTime | high |
| 1523 | 67 | ✓ | `ecourna::api::util::CStringUtils::ToDWord` | `src/ecourna/api/util/cstringutils.cpp` | high |
| 1525 | 964 |  | `ecourna::api::sql::sqlite::CSqlResultSet::IndexForColumnName` | `src/ecourna/api/sql/sqlite/csqlresultset.cpp` | high |
| 1677 | 57 |  | `std::__tree<std::__value_type<std::string,int>>::destroy` | library: recursive node destruction of map<std::string,int> | high |
| 1879 | 12 | ✓ | `ecourna::api::util::CStringUtils::ToLower(const std::string&)` | `src/ecourna/api/util/cstringutils.cpp` | medium |
| 1882 | 267 |  | `ecourna::api::sql::sqlite::CSqlStatement::VerifyParameterIndex` | `src/ecourna/api/sql/sqlite/csqlstatement.cpp` | high |
| 1883 | 267 |  | `ecourna::api::sql::sqlite::CSqlResultSet::VerifyColumnIndex` | `src/ecourna/api/sql/sqlite/csqlresultset.cpp` | high |
| 1939 | 12 | ✓ | `comum::CCandidaturas::Chave` | `src/uenux2/src/app/comum/dados/ccandidaturas.u13.cpp` (path inferred) | low |
| 1949 | 2384 |  | `std::__formatter::__format_integer<unsigned __int128, char>` | library: libc++ <format> 128-bit integer formatting (sign, prefix, base 2/8/10/16, locale grouping) | high |
| 2200 | 71 |  | `ecourna::api::util::CStringUtils::ToInt32` | `src/ecourna/api/util/cstringutils.cpp` | high |
| 2201 | 20 | ✓ | `ecourna::api::util::CStringUtils::ToWord` | `src/ecourna/api/util/cstringutils.cpp` | high |
| 2202 | 1375 | ✓ | `ecourna::api::util::CStringUtils::ToUnsigned` | `src/ecourna/api/util/cstringutils.cpp` | medium |
| 2276 | 362 | ✓ | `comum::asn::Utils::DesconverteDataJE` | `src/uenux2/src/app/comum/asn/util.u13.cpp` | medium |
| 2678 | 261 |  | `std::map<std::string,int>::find` | library: libc++ __tree::find<std::string> (column-name map; also used by shared_f5149) | high |
| 2679 | 274 |  | `ecourna::api::sql::sqlite::CSqlResultSet::GetInt` | `src/ecourna/api/sql/sqlite/csqlresultset.cpp` | high |
| 2866 | 91 |  | `std::__to_chars_itoa<unsigned long long> (std::to_chars)` | library: libc++ <charconv> | high |
| 2965 | 23 |  | `isxdigit` | library: musl isxdigit (called by HexStringToBytes and libc++ num_put) | high |
| 3506 | 1265 | ✓ | `ecourna::api::util::CStringUtils::HexStringToBytes` | `src/ecourna/api/util/cstringutils.cpp` | high |
| 3507 | 95 |  | `ecourna::api::util::CStringUtils::HexToInt` | `src/ecourna/api/util/cstringutils.cpp` | high |
| 3508 | 68 | ✓ | `ecourna::api::util::CStringUtils::ToInt16` | `src/ecourna/api/util/cstringutils.cpp` | high |
| 3513 | 112 |  | `ecourna::api::sql::sqlite::CSqlStatement::~CSqlStatement` | `src/ecourna/api/sql/sqlite/csqlstatement.cpp` | high |
| 3514 | 439 |  | `ecourna::api::sql::sqlite::CSqlResultSet::GetText` | `src/ecourna/api/sql/sqlite/csqlresultset.cpp` | high |
| 3515 | 1562 | ✓ | `ecourna::api::sql::sqlite::CSqlConnection::CSqlConnection` | `src/ecourna/api/sql/sqlite/csqlconnection.cpp` | high |
| 3518 | 533 | ✓ | `ecourna::api::security::CSha::Finish` | `src/ecourna/api/security/csha.cpp` | high |
| 3519 | 489 | ✓ | `ecourna::api::security::CSha::Update` | `src/ecourna/api/security/csha.cpp` | high |
| 3700 | 1946 |  | `comum::CCalculaCV::Calcula` | `src/uenux2/src/app/comum/relatorios/ccalculacv.u13.cpp` (+ inlined IMacAlgorithm::Mac in src/ecourna/api/security/imacalgorithm.cpp, HexToQWord in cstringutils.cpp) | medium |
| 3846 | 51 |  | `std::__itoa::__append10<unsigned long long>` | library: libc++ <charconv> internals | high |
| 5155 | 1382 | ✓ | `ecourna::api::util::CStringUtils::ToSigned` | `src/ecourna/api/util/cstringutils.cpp` | medium |
| 5160 | 275 |  | `ecourna::api::sql::sqlite::CSqlStatement::SetInt` | `src/ecourna/api/sql/sqlite/csqlstatement.cpp` | high |
| 5161 | 476 |  | `ecourna::api::sql::sqlite::CSqlResultSet::GetBlob` | `src/ecourna/api/sql/sqlite/csqlresultset.cpp` | high |
| 5162 | 514 |  | `ecourna::api::sql::sqlite::CSqlResultSet::GetDateTime` | `src/ecourna/api/sql/sqlite/csqlresultset.cpp` | medium |
| 5163 | 290 |  | `ecourna::api::sql::sqlite::CSqlResultSet::IsNull` | `src/ecourna/api/sql/sqlite/csqlresultset.cpp` | high |
| 5164 | 276 |  | `ecourna::api::sql::sqlite::CSqlResultSet::GetDouble` | `src/ecourna/api/sql/sqlite/csqlresultset.cpp` | high |
| 5165 | 276 |  | `ecourna::api::sql::sqlite::CSqlResultSet::GetInt64` | `src/ecourna/api/sql/sqlite/csqlresultset.cpp` | high |
| 5166 | 1622 | ✓ | `ecourna::api::sql::sqlite::CSqlResultSet::StepStatement` | `src/ecourna/api/sql/sqlite/csqlresultset.cpp` | high |
| 5167 | 137 |  | `ecourna::api::sql::sqlite::CSqlConnection::~CSqlConnection` | `src/ecourna/api/sql/sqlite/csqlconnection.cpp` | high |
| 5175 | 114 |  | `ecourna::api::security::CSha::~CSha` | `src/ecourna/api/security/csha.hpp` | high |
| 5176 | 117 |  | `ecourna::api::security::CSha::~CSha [deleting]` | `src/ecourna/api/security/csha.hpp` | high |
| 5177 | 290 | ✓ | `ecourna::api::security::CSha::Reset` | `src/ecourna/api/security/csha.cpp` | high |
| 5182 | 299 |  | `boost::CV::simple_exception_policy<unsigned short,1,31,boost::gregorian::bad_day_of_month>::on_error` | library: boost::date_time range-check thrower | high |
| 5183 | 299 |  | `boost::CV::simple_exception_policy<unsigned short,1,12,boost::gregorian::bad_month>::on_error` | library: boost::date_time range-check thrower | high |
| 5479 | 723 | ✓ | `api::CDate::IsValid` | `src/uenux2/src/api/util/cdate.u13.cpp` | medium |
| 5645 | 16 |  | `comum::md::CVoto::EhNulo(ETipo)` | `src/uenux2/src/app/comum/dados/md/rdv/cvoto.u13.cpp` | medium |
| 5665 | 144 |  | `comum::md::CEleitor::GetAnoNascimento` | `src/uenux2/src/app/comum/dados/md/eleitor/celeitor.u13.cpp` | medium |
| 5912 | 408 |  | `std::__formatter::__format_integer<unsigned __int128> [type dispatch]` | library: libc++ <format>: the outer overload (value already made unsigned + `negative` flag by both the signed and the unsigned `__int128` callers in func 4027), switch on presentation type with prefixes 0b/0B/0/0x/0X | high |
| 5919 | 76 |  | `std::__itoa::__base_10_u64` | library: libc++ <charconv> internals | high |
| 6153 | 63 | ✓ | `ecourna::api::util::CStringUtils::ToByte/ToWord [merged body]` | `src/ecourna/api/util/cstringutils.cpp` | medium |
| 9406 | 326 | ✓ | `ecourna::api::util::CStringUtils::Trim(std::string&)` | `src/ecourna/api/util/cstringutils.cpp` | medium |
| 9410 | 18 |  | `ecourna::api::exception::CBaseError<util::EUtilError,{1875,1900}>::CBaseError(code, std::string&&, const source_location&) [thunk]` | library/inlined helper: 18-byte thunk -> CBaseError ctor body func 1011 with vtable @1117872 (ecourna exception header, not reconstructed) | high |
| 9411 | 222 |  | `ecourna::api::sql::sqlite::GetErrorMessage` | `src/ecourna/api/sql/sqlite/csqlconnection.cpp` (original file unknown) | medium |
| 9414 | 1175 |  | `boost::date_time::month_formatter<boost::gregorian::greg_month,boost::date_time::iso_extended_format<char>,char>::format_month` | library: boost::date_time ISO formatter (setw(2), setfill('0'), fill saver) | medium |
| 9416 | 264 |  | `ecourna::api::sql::sqlite::CSqlStatement::Reset` | `src/ecourna/api/sql/sqlite/csqlstatement.cpp` | high |
| 9417 | 228 | ✓ | `ecourna::api::sql::sqlite::CSqlStatement::Execute` | `src/ecourna/api/sql/sqlite/csqlstatement.cpp` | medium |
| 9419 | 273 |  | `ecourna::api::sql::sqlite::CSqlStatement::SetNull` | `src/ecourna/api/sql/sqlite/csqlstatement.cpp` | high |
| 9420 | 290 |  | `ecourna::api::sql::sqlite::CSqlStatement::SetBlob` | `src/ecourna/api/sql/sqlite/csqlstatement.cpp` | high |
| 9421 | 11 |  | `ecourna::api::sql::sqlite::CSqlStatement::SetBool` | `src/ecourna/api/sql/sqlite/csqlstatement.cpp` | medium |
| 9422 | 3006 |  | `ecourna::api::sql::sqlite::CSqlStatement::SetDateTime` | `src/ecourna/api/sql/sqlite/csqlstatement.cpp` | medium |
| 9423 | 305 |  | `ecourna::api::sql::sqlite::CSqlStatement::SetText` | `src/ecourna/api/sql/sqlite/csqlstatement.cpp` | high |
| 9424 | 275 |  | `ecourna::api::sql::sqlite::CSqlStatement::SetDouble` | `src/ecourna/api/sql/sqlite/csqlstatement.cpp` | high |
| 9425 | 275 |  | `ecourna::api::sql::sqlite::CSqlStatement::SetInt64` | `src/ecourna/api/sql/sqlite/csqlstatement.cpp` | high |
| 9426 | 111 |  | `ecourna::api::sql::sqlite::CSqlStatement::~CSqlStatement [deleting]` | `src/ecourna/api/sql/sqlite/csqlstatement.cpp` | high |
| 9428 | 7 |  | `ecourna::api::sql::sqlite::CSqlStatement::IsClosed` | `src/ecourna/api/sql/sqlite/csqlstatement.hpp` | medium |
| 9429 | 443 | ✓ | `ecourna::api::sql::sqlite::CSqlStatement::Close` | `src/ecourna/api/sql/sqlite/csqlstatement.cpp` | high |
| 9430 | 48 | ✓ | `ecourna::api::sql::sqlite::CSqlStatement::CSqlStatement` | `src/ecourna/api/sql/sqlite/csqlstatement.cpp` | high |
| 9432 | 16 |  | `ecourna::api::sql::sqlite::CSqlResultSet::GetBlob(const std::string&)` | `src/ecourna/api/sql/sqlite/csqlresultset.cpp` | medium |
| 9433 | 17 |  | `ecourna::api::sql::sqlite::CSqlResultSet::GetBool(const std::string&)` | `src/ecourna/api/sql/sqlite/csqlresultset.cpp` | medium |
| 9434 | 14 |  | `ecourna::api::sql::sqlite::CSqlResultSet::GetDateTime(const std::string&)` | `src/ecourna/api/sql/sqlite/csqlresultset.cpp` | medium |
| 9435 | 14 |  | `ecourna::api::sql::sqlite::CSqlResultSet::IsNull(const std::string&)` | `src/ecourna/api/sql/sqlite/csqlresultset.cpp` | medium |
| 9438 | 16 |  | `ecourna::api::sql::sqlite::CSqlResultSet::GetText(const std::string&)` | `src/ecourna/api/sql/sqlite/csqlresultset.cpp` | medium |
| 9439 | 14 |  | `ecourna::api::sql::sqlite::CSqlResultSet::GetDouble(const std::string&)` | `src/ecourna/api/sql/sqlite/csqlresultset.cpp` | medium |
| 9440 | 14 |  | `ecourna::api::sql::sqlite::CSqlResultSet::GetInt64(const std::string&)` | `src/ecourna/api/sql/sqlite/csqlresultset.cpp` | medium |
| 9441 | 14 |  | `ecourna::api::sql::sqlite::CSqlResultSet::GetInt(const std::string&)` | `src/ecourna/api/sql/sqlite/csqlresultset.cpp` | medium |
| 9442 | 20 |  | `ecourna::api::sql::sqlite::CSqlResultSet::~CSqlResultSet [deleting]` | `src/ecourna/api/sql/sqlite/csqlresultset.hpp` | high |
| 9443 | 17 |  | `ecourna::api::sql::sqlite::CSqlResultSet::~CSqlResultSet` | `src/ecourna/api/sql/sqlite/csqlresultset.hpp` | high |
| 9444 | 41 |  | `ecourna::api::sql::sqlite::CSqlResultSet::Next` | `src/ecourna/api/sql/sqlite/csqlresultset.cpp` | medium |
| 9446 | 12 |  | `ecourna::api::sql::sqlite::CSqlResultSet::GetBool` | `src/ecourna/api/sql/sqlite/csqlresultset.cpp` | medium |
| 9447 | 365 |  | `std::map<std::string,int>::__emplace_unique_key_args (operator[])` | library: libc++ __tree emplace with piecewise_construct (@463337) | high |
| 9449 | 135 | ✓ | `ecourna::api::sql::sqlite::CSqlResultSet::CSqlResultSet` | `src/ecourna/api/sql/sqlite/csqlresultset.cpp` | high |
| 9452 | 503 |  | `ecourna::api::sql::sqlite::CSqlConnection::CheckIntegrity` | `src/ecourna/api/sql/sqlite/csqlconnection.cpp` | medium |
| 9453 | 433 |  | `ecourna::api::sql::sqlite::CSqlConnection::Rollback` | `src/ecourna/api/sql/sqlite/csqlconnection.cpp` | medium |
| 9454 | 433 |  | `ecourna::api::sql::sqlite::CSqlConnection::Commit` | `src/ecourna/api/sql/sqlite/csqlconnection.cpp` | medium |
| 9455 | 433 |  | `ecourna::api::sql::sqlite::CSqlConnection::BeginTransaction` | `src/ecourna/api/sql/sqlite/csqlconnection.cpp` | medium |
| 9456 | 140 |  | `ecourna::api::sql::sqlite::CSqlConnection::~CSqlConnection [deleting]` | `src/ecourna/api/sql/sqlite/csqlconnection.cpp` | high |
| 9457 | 7 |  | `ecourna::api::sql::sqlite::CSqlConnection::IsClosed` | `src/ecourna/api/sql/sqlite/csqlconnection.hpp` | medium |
| 9458 | 271 |  | `ecourna::api::sql::sqlite::CSqlConnection::Close` | `src/ecourna/api/sql/sqlite/csqlconnection.cpp` | high |
| 9460 | 1136 | ✓ | `ecourna::api::sql::sqlite::CSqlConnection::Prepare` | `src/ecourna/api/sql/sqlite/csqlconnection.cpp` | high |
| 9461 | 18 |  | `ecourna::api::exception::CBaseError<sql::ESqlError,{1725,1775}>::CBaseError(code, std::string&&, const source_location&) [thunk]` | library/inlined helper: 18-byte thunk -> CBaseError ctor body func 1011 with vtable @1116272 (ecourna exception header, not reconstructed) | high |
| 9474 | 1721 |  | `ecourna::api::security::ISymmetricCipher::ISymmetricCipher` | `src/ecourna/api/security/isymmetriccipher.cpp` | high |
| 9484 | 72 |  | `ecourna::api::security::ISymmetricCipher::~ISymmetricCipher` | `src/ecourna/api/security/isymmetriccipher.hpp` | high |
| 9497 | 814 |  | `ecourna::api::security::CSiphashMac::DoVerify` | `src/ecourna/api/security/csiphashmac.cpp` | high |
| 9498 | 3654 |  | `ecourna::api::security::CSiphashMac::DoMac` | `src/ecourna/api/security/csiphashmac.cpp` | high |
| 9504 | 1189 |  | `ecourna::api::security::CKey::CKey` | `src/ecourna/api/security/ckey.cpp` | high |
| 11316 | 10 |  | `comum::md::CVoto::EhNulo` | `src/uenux2/src/app/comum/dados/md/rdv/cvoto.u13.cpp` | medium |
| 11365 | 214 | ✓ | `comum::asn::CConversorPleito::DoDesconverte` | `src/uenux2/src/app/comum/dados/asn/processoeleitoral/cconversorpleito.u13.cpp` (path inferred) | medium |
| 11414 | 97 |  | `ecourna::api::util::CBitArray::~CBitArray [deleting]` | `src/ecourna/api/util/cbitarray.hpp` | high |
| 11417 | 94 |  | `ecourna::api::util::CBitArray::~CBitArray` | `src/ecourna/api/util/cbitarray.hpp` | high |
| 11514 | 6868 |  | `comum::CIntegridadeReferencial::VerificaVotosRdv` | `src/uenux2/src/app/comum/dados/cintegridadereferencial.u13.cpp` | medium |
