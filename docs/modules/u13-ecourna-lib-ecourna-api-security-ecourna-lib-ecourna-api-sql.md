# u13: `ecourna::api::sql` (wrapper de SQLite), `ecourna::api::util` (CStringUtils, CBitArray) e `ecourna::api::security` (SHA-512, MAC SipHash, CKey, ISymmetricCipher)

A unidade u13 tem 100 funções wasm. A maioria vem da biblioteca do TSE **ecourna**
(`/home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/api/…`). O montador de unidades também arquivou aqui algumas
funções de outros lugares, porque elas chamam ou inlinam helpers do ecourna:

| grupo | funções | o quê |
|---|---:|---|
| `api/sql/sqlite/*` | 50 | a única camada de banco de dados do aplicativo de votação: `CSqlConnection`, `CSqlStatement`, `CSqlResultSet` sobre o SQLite 3.50.4 |
| `api/util/*` | 16 | `CStringUtils` (os parsers de números usados em todo lugar: números de candidatos digitados, datas ASN.1, colunas de DAO, hex) e `CBitArray` (leitor de bits para templates biométricos) |
| `api/security/*` | 10 | `CSha`/`CSha512` (SHA-512 dos QR codes do BU e da derivação da chave do RDV), `CSiphashMac` (o MAC por trás do *Código Verificador* do BU), `CKey`, `ISymmetricCipher` |
| código do uenux2 arquivado aqui | 9 | a verificação de consistência do RDV (`CIntegridadeReferencial`, 6,9 KB), `CCalculaCV::Calcula` (o *Código Verificador*), conversão de datas DataJE, validação de `CDate`, o ano de nascimento do eleitor, o predicado de voto nulo, a chave do mapa de candidaturas |
| código de biblioteca | 15 | map/charconv/inteiros de 128 bits do `<format>` da libc++, boost::date_time, `isxdigit` da musl, dois thunks de construtores de erro |

Arquivos-fonte reconstruídos (todos novos, escritos pela u13):

* `src/ecourna/api/sql/{esqlerror.hpp,isqlconnection.hpp}`, `src/ecourna/api/sql/sqlite/csql{connection,statement,resultset}.{hpp,cpp}`
* `src/ecourna/api/util/cstringutils.{hpp,cpp}`, `src/ecourna/api/util/cbitarray.{hpp,cpp}`
* `src/ecourna/api/security/{csha,ckey,isymmetriccipher,imacalgorithm}.{hpp,cpp}`, `src/ecourna/api/security/csiphashmac.cpp`
* fragmentos de arquivos de outras unidades, chamados `*.u13.cpp`: `uenux2/src/app/comum/dados/cintegridadereferencial.u13.cpp`,
  `…/comum/relatorios/ccalculacv.u13.cpp`, `…/comum/asn/util.u13.cpp`, `…/comum/dados/asn/processoeleitoral/cconversorpleito.u13.cpp`,
  `…/comum/dados/md/rdv/cvoto.u13.cpp`, `…/comum/dados/md/eleitor/celeitor.u13.cpp`, `…/comum/dados/ccandidaturas.u13.cpp`,
  `uenux2/src/api/util/cdate.u13.cpp`.

24 das 100 funções foram vistas executando nos votos gravados (`analysis/runtime/*.functions.tsv`):
1142, 1523, 1879, 1939, 2201, 2202, 2276, 3506, 3508, 3515, 3518, 3519, 5155, 5166, 5177, 5479, 6153, 9406,
9417, 9429, 9430, 9449, 9460 e 11365. Quase todas executam durante `votaInit`. A grande função de
inicialização 7787 (`vota::CInformacaoEleitor::Inicializar`, u02; antes mostrada pelas ferramentas como `CHKDFSeed::GetSeed`,
uma das muitas funções inlinadas nela) abre `uenux.db`
(cerca de 390 amostras do profiler no construtor de `CSqlConnection` e 500 em `Prepare`), calcula hashes SHA-512
para a chave do RDV e decodifica datas dos arquivos da eleição. Durante a votação só os parsers de números executam:
o host de `votaTick`/`votaGetStateJson` (func 5500) chama a func 1939 e `ToWord`, e `ToDWord` foi amostrada uma vez,
chamada por `vota::CPedeMajoritario::ProcessInputAudio` (func 11683), no voto municipal.

Termos do domínio: *urna* = a máquina de votação; *seção* = seção eleitoral; *mesário* = membro da mesa receptora;
*comparecimento* = presença; *justificativa* = justificativa de ausência; *título* (de eleitor) = número de
inscrição do eleitor; *BU* (*boletim de urna*) = a apuração que a urna imprime e grava no encerramento;
*RDV* (*registro digital do voto*) = o registro embaralhado de todos os votos; *cargo* = cargo em disputa; *legenda* =
voto só no partido; *nulo*/*branco* = voto nulo/em branco; *candidatura apta/inapta* = candidatura elegível/inelegível;
*consulta* = referendo, cujas *respostas* são as opções; *pleito* = rodada da eleição; *encerramento* = o
fim da votação.

---

## 1. Classes e como elas se relacionam (RTTI)

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

Posse: todo objeto SQL vive em um `std::shared_ptr` do seu tipo de interface. A libc++ emite um bloco de controle
`__shared_ptr_pointer` por par: `…<CSqlConnection*, shared_ptr<ISqlConnection>>` @1559008
(mantido pelos DAOs), `…<CSqlStatement*>` @1116292 e `…<CSqlResultSet*>` @1117256. O statement e o
result set guardam ponteiros **crus** `sqlite3*`/`sqlite3_stmt*`. Nada amarra o tempo de vida deles ao da conexão.

### 1.1 Layouts

| classe | layout (wasm32) |
|---|---|
| `CSqlConnection` | `+0 vptr`, `+4 std::string m_arquivo` (caminho do BD), `+16 sqlite3* m_db`, `+20 bool m_fechada` |
| `CSqlStatement` | `+0 vptr`, `+4 sqlite3* m_db`, `+8 sqlite3_stmt* m_comando`, `+12 bool m_fechado`, `+16 int m_qtdParametros` |
| `CSqlResultSet` | `+0 vptr`, `+4 sqlite3*`, `+8 sqlite3_stmt*`, `+12 bool m_temLinha`, `+13 bool m_aguardandoNext`, `+16 int m_qtdColunas`, `+20 std::map<std::string,int> m_colunas` |
| `CSha` | `+0 vptr`, `+4 std::string m_algoritmo` ("SHA2-512"), `+16 size_t m_tamanho` (64), `+20 unique_ptr<EVP_MD_CTX, void(*)(EVP_MD_CTX*)>` (ptr +20, deleter `EVP_MD_CTX_free` +24) |
| `CKey` | `+0 std::string titular`, `+12 uedword idParChaves`, `+16 KeyType tipo`, `+20 vector<uebyte> bytes`, `+32 std::string tag`, `+44 uedword` (sempre 0) |
| `CKeyData` | `+0 vector<uebyte> bytes` |
| `ISymmetricCipher` | `+0 vptr`, `+4 CAesKey{ +4 key, +16 keySize (bits), +20 iv }` |
| `CBitArray` | `+0 vptr`, `+4 shared_ptr<boost::dynamic_bitset<uebyte>>` (+8 controle), `+12 vector<uebyte>` (?), `+24 bool leitura` (=1), `+25 bool` (=0), `+32 ueqword posição` (bits) |

### 1.2 Ordem dos slots virtuais e como os nomes foram recuperados

As interfaces SQL não têm srcloc próprio. A ordem dos slots vem das vtables. Os nomes vêm dos
srclocs dos métodos de implementação (`Prepare`, `Close`, `SetInt`…`SetNull`, `Reset`, `GetInt`…`GetBlob`),
do texto SQL que cada slot executa (`begin`/`commit`/`rollback transaction`, `PRAGMA integrity_check(1)`)
e do texto de erro "Chame o método 'Next()'…".

| classe | slots |
|---|---|
| `ISqlConnection` | 0 ~, 1 ~D0, 2 `BeginTransaction`\*, 3 `Commit`\*, 4 `Rollback`\*, 5 `Prepare`, 6 `IsClosed`\*, 7 `Close`, 8 `CheckIntegrity`\*, 9 `GetPath`\* (corpo ICF 5322 "return this+4") |
| `ISqlStatement` | 0 ~, 1 ~D0, 2 `SetInt`, 3 `SetInt64`, 4 `SetDouble`, 5 `SetText`, 6 `SetDateTime`\*, 7 `SetBool`\*, 8 `SetBlob`, 9 `SetNull`, 10 `Execute`\*, 11 `Reset`, 12 `Close`, 13 `IsClosed`\* |
| `ISqlResultSet` | 0 ~, 1 ~D0, depois pares (por índice, por nome de coluna): 2/3 `GetInt`, 4/5 `GetInt64`, 6/7 `GetDouble`, 8/9 `GetText`, 10/11 `IsNull`, 12/13 `GetDateTime`\*, 14/15 `GetBool`\*, 16/17 `GetBlob`, 18 `Next`\* |
| `IHash`/`CSha` | 0 ~, 1 ~D0, 2 `Reset`, 3 `Update`, 4 `Finish`, 5 `GetName`\* (corpo ICF 3874 "copia a string em this+4") |
| `IMacAlgorithm` | 0 ~ (trivial), 1 ~D0, 2 `DoMac`, 3 `DoVerify` (nomes vindos do srcloc); o `Mac` público não virtual está inlinado no seu chamador |

\* nome inferido a partir do comportamento.

---

## 2. A camada SQL (`ecourna::api::sql::sqlite`)

### 2.1 Onde ela é usada

O aplicativo de votação guarda os seus **dados dinâmicos** em arquivos SQLite chamados `uenux.db`, um por memória
flash no diretório de trabalho: `/dsk/fi/dinamico/trab1/uenux.db` (flash interna) e `/dsk/fe/dinamico/trab1/uenux.db`
(flash externa/cartão de memória). Os DAOs de outras unidades constroem uma `CSqlConnection` sobre esses caminhos
(`operator new(24)` na func 3768 `CEleitorDinamicoDAO::CEleitorDinamicoDAO` e na função de inicialização 7787)
e emitem o seu próprio SQL por meio de `Prepare`. Depois de `votaInit`, os BDs dos snapshots (`analysis/runtime/memfs-after-*/…/uenux.db`)
contêm exatamente estas tabelas, ambas vazias depois dos votos gravados:

```sql
CREATE TABLE comparecimento_mesario (numero_titulo BIGINT NOT NULL,
  tipo_identificador INTEGER CHECK(tipo_identificador IN (1,2,3)) NOT NULL, periodo_presente INTEGER NOT NULL,
  pertence_secao INTEGER NOT NULL, estado_biometria INTEGER NOT NULL, dedo_habilitacao INTEGER NOT NULL,
  data_hora_registro DATETIME NOT NULL, id_arquivo INTEGER, UNIQUE (numero_titulo, periodo_presente));
CREATE TABLE registro_justificativa (numero_titulo BIGINT PRIMARY KEY UNIQUE NOT NULL, ano_nascimento SMALLINT);
```

Uma terceira tabela, usada por `CEleitorDinamicoDAO`, guarda o estado de comparecimento por eleitor: `( titulo BIGINT PRIMARY
KEY UNIQUE NOT NULL, tipo_identificador …)` @449289, com o seu nome em uma string global. Esse DAO não executou nas
sessões gravadas. Os textos de SELECT/INSERT dos DAOs estão em @442244 e @448347–449081. `integrity_check`
retorna `ok` nos dois snapshots, e `foreign_keys` fica desligado em uma sessão nova da CLI porque é um pragma por conexão.

### 2.2 Ciclo de vida da conexão (func 3515, 5167, 9458)

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

`GetErrorMessage` (func 9411) retorna `sqlite3_errmsg(db)`, ou "Erro desconhecido do SQLite." quando ele é NULL.
Toda mensagem de erro de SQL vem dela.

### 2.3 Protocolo statement → result set

`Execute()` (func 9417) retorna `shared_ptr<ISqlResultSet>(new CSqlResultSet(db, stmt))`. O **construtor do result
set dá um passo** (func 9449 → `StepStatement`). Assim, `Execute()` sozinho executa DDL/DML, e todos os
DAOs e a própria conexão o usam desse jeito. A leitura segue uma pequena máquina de estados:

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

`StepStatement` (linha 185) executa `sqlite3_step`, atualiza `m_qtdColunas` e **reconstrói** o mapa nome→índice
a partir de `sqlite3_column_name`, em minúsculas com o `CStringUtils::ToLower` ciente de Latin-1 (func 1879), a
cada linha. Ele retorna `rc == SQLITE_ROW` para ROW/OK/DONE. Para qualquer outro código ele lança 1737 com a mesma
mensagem de duas linhas de `Prepare`: a mensagem do SQLite e `sqlite3_sql(stmt)`, ou "INDISPONÍVEL" quando esta é NULL.

Os getters não verificam `m_temLinha`. Depois que `Next()` retornou false, eles retornam os padrões do SQLite (0, "", {}).

| getter | implementação |
|---|---|
| `GetInt`/`GetInt64`/`GetDouble` | `sqlite3_column_int/int64/double` |
| `GetText` | `text = sqlite3_column_text`; `bytes != 0 && text` ? `std::string(text)` (strlen: para em um NUL embutido) : `""` |
| `IsNull` | `sqlite3_column_text == NULL && sqlite3_column_bytes == 0`. Só o NULL do SQL dá um ponteiro de texto NULL: um TEXT ou BLOB de comprimento zero dá `""` (testado), então a resposta está certa, mas o valor é convertido para texto no lugar |
| `GetBlob` | `sqlite3_column_blob` + `bytes` → vector (vazio quando há 0 bytes) |
| `GetBool` | `GetInt(i) == 1` (por nome: `GetInt(IndexForColumnName(n)) == 1`) |
| `GetDateTime` | `GetText`, depois substitui `' '`→`"T"`, remove `'-'` e `':'` (func 9398) → `boost::date_time::parse_iso_time<ptime>(s, 'T')` |

Os setters (índices de parâmetro a partir de 1; `VerifyParameterIndex` → 1746 "Índice do parâmetro fora do intervalo válido."
quando `i ≤ 0 || i > sqlite3_bind_parameter_count`) chamam `sqlite3_bind_int/int64/double/text(SQLITE_TRANSIENT)/blob(SQLITE_TRANSIENT)/null`
e lançam 1738…1743 com `GetErrorMessage`. `SetBool(i, b)` = `SetInt(i, b)`.
`SetDateTime(i, ptime)` = `to_iso_extended_string(t)` com `'T'` → `" "`, depois `SetText`. Esta é a única
origem do formato de texto `YYYY-MM-DD HH:MM:SS[.ffffff]` usado em `comparecimento_mesario.data_hora_registro`.
Os valores especiais são guardados como `not-a-date-time`/`±infinity`.
`Reset` → `sqlite3_reset` (1744), e os bindings são mantidos. `Close` → `sqlite3_finalize` (1745) sob um
`std::mutex` global (@1911796). Os landing pads mostram um `lock(); try {…} catch(...) { unlock(); throw; } unlock();` explícito.

### 2.4 Códigos de `ESqlError` (faixa 1725–1775)

Os valores são certos. Eles são numerados na ordem do fonte, arquivo por arquivo.

| código | local (linha) | mensagem |
|---|---|---|
| 1725 | ctor de CSqlConnection (29) | `sqlite3_errmsg` da abertura que falhou |
| 1726 | ctor de CSqlConnection (44) | `sqlite3_errmsg` depois que `PRAGMA foreign_keys = ON` falhou |
| 1727 | Prepare (86) | "Falha ao executar operação no banco de dados.\nMensagem do banco de dados: '{}'\nOperação: '{}'" |
| 1728 | CSqlConnection::Close (103) | `sqlite3_errmsg` |
| 1729–1734 | GetInt (39), GetInt64 (50), GetDouble (61), GetText (72), IsNull (90), GetBlob (128) | "Chame o método 'Next()' antes de buscar resultados." |
| 1735 | VerifyColumnIndex (143) | "Índice da coluna fora do intervalo válido." |
| 1736 | IndexForColumnName (153) | "Coluna {} não faz parte do resultado." |
| 1737 | StepStatement (185) | mesmo texto de 1727 |
| 1738–1743 | SetInt (53), SetInt64 (63), SetDouble (73), SetText (83), SetBlob (109), SetNull (119) | `sqlite3_errmsg` |
| 1744 / 1745 | Reset (133) / CSqlStatement::Close (143) | `sqlite3_errmsg` |
| 1746 | VerifyParameterIndex (158) | "Índice do parâmetro fora do intervalo válido." |

---

## 3. `ecourna::api::util`

### 3.1 Parsers de números de `CStringUtils`

Todos eles aparam uma cópia do argumento (func 9406: remove todo byte ≤ 0x20, incluindo caracteres de controle,
nas duas pontas), depois verificam que ela não está vazia e contém só caracteres permitidos, e então chamam
`strtoull`/`strtoll` com `errno = 0`. Eles lançam `CBaseError<EUtilError>` com o `std::source_location` do
**chamador** (um argumento com valor padrão do corpo compartilhado; é por isso que todos os srclocs têm coluna 12,
a coluna de `return ToUnsigned(`). A mensagem é sempre `"valor invalido [" + original + "]"`:

| função (linha) | corpo | base | caracteres permitidos | faixa | resultado |
|---|---|---|---|---|---|
| `ToByte` (714) | 6153 → 2202 | 10 | `+-0123456789` | ≤ 255 | `& 0xFF` |
| `ToInt16` (719) | 5155 | 10 | os mesmos | −32768…32767 | com extensão de sinal |
| `ToWord` (724) | 6153 → 2202 | 10 | os mesmos | ≤ 65535 | `& 0xFFFF` |
| `ToInt32` (729) | 5155 | 10 | os mesmos | int32 | |
| `ToQWord` (734) | 2202 | 10 | os mesmos | ≤ 2⁶⁴−1 (só ERANGE pode falhar) | |
| `ToDWord` (739) | 2202 | 10 | os mesmos | ≤ 2³²−1 | |
| `HexToInt` (744) | 2202 | 16 | `xXabcdefABCDEF0123456789` | ≤ 2³²−1 | |
| `HexToQWord` (754) | inlinada na 3700 | 16 | os mesmos | ≤ 2⁶⁴−1 | |

Códigos de erro: 1875 vazio/caractere inválido, 1876 `*end != '\0'` (por exemplo `"1-2"`), 1877 fora da faixa ou
`errno == ERANGE` (68 na numeração WASI do Emscripten). Os parsers sem sinal aceitam um `-` inicial, que
`strtoull` nega módulo 2⁶⁴: `ToQWord("-5")` retorna 18446744073709551611 (§8).

Os conjuntos de caracteres permitidos são arrays locais: cada wrapper (e o corpo mesclado 6153, e a func 3700 para o
`HexToQWord` inlinado) copia o literal de 13 ou 25 bytes para o seu frame de pilha e passa a cópia.

`HexStringToBytes` (808): apara; vazio → `{}`; qualquer caractere que não passe em `isxdigit` → 1878 `"valor invalido [" + trimmed + "]"`;
depois pares de nibbles, `for (i = 0; i < size-1; i += 2)`. Um nibble final ímpar é descartado em silêncio.
Usuários: o assinador do QR code do BU (64 bytes crus a partir do SHA-512 de 128 dígitos hex, §6.1), `CSerialMidia`, dados de mock (func 9963).

`ToLower(const std::string&)` (func 1879) é cópia + slot 6425 da tabela = func 5158. Esse é o conversor para
minúsculas no lugar, que é ciente de Latin-1: um `std::set<int>` estático @1911836, construído sob demanda, com as 19 letras
maiúsculas acentuadas `Ç Á É Í Ó Ú À È Ì Ò Ù Â Ê Î Ô Û Ã Õ Ñ` recebe `|= 0x20`, e as outras letras passam por
`isalpha`/`tolower`. É o gêmeo em minúsculas do conversor para maiúsculas descrito pela u02 (funcs 5156/3509).
Chamadores: nomes de colunas SQL, `FormataUF` (os nomes de arquivos usam a UF em minúsculas), funcs 3753, 5726 e 5728.

### 3.2 `CBitArray` (func 1375)

`GetValor(n)`: precisa da flag de leitura (+24); caso contrário lança 1882 com a mensagem "sem permissao de
**escrita** no array de bits" (o texto diz *escrita*). Ela retorna `GetValor(posição, n)` e avança o cursor.
`GetValor(pos, n)`: 1883 se não há bitset, 0 se `n == 0`, 1884 "busca no vetor em posicao invalida" se
`pos + n > size`. Caso contrário, lê com o MSB primeiro: bit `pos+i` → bit `n−1−i`. Único usuário: `comum::asn::CConversorDedo`
(func 5702), que desempacota os templates de impressões digitais do cadastro de eleitores.

---

## 4. `ecourna::api::security`

* **`CSha` / `CSha512`** (funcs 5177, 3519, 3518, 5175, 5176). `Reset` = `EVP_DigestInit_ex2(ctx,
  EVP_get_digestbyname("SHA2-512"), NULL)` → 1417 "Falha ao iniciar o contexto.". `Update` rejeita entrada
  vazia (1418 "Não é possível calcular o hash de dados vazios."), falha de `EVP_DigestUpdate` → 1419.
  `Finish` retorna um `const std::vector<uebyte>` de `m_tamanho` bytes (64), falha de `EVP_DigestFinal_ex` → 1420.
  Os três executaram em `votaInit` (derivação da chave do RDV, func hospedeira 7787).
* **`IMacAlgorithm::Mac`** (imacalgorithm.cpp:32/35, apenas inlinada): 1432 "Vetor de dados vazio.", 1433 "Chave vazia.",
  depois o `DoMac` virtual. **`CSiphashMac::DoMac`** (func 9498): `EVP_MAC_fetch("siphash")`, parâmetros
  `size=8, c-rounds=4, d-rounds=6` em um `std::vector<OSSL_PARAM>(4)` no heap, `EVP_MAC_init/update/final`. Cada falha
  lança 1423/1424/1425 `std::format("Falha ao autenticar. - {}", ERR_get_error())` com o *número* do erro.
  **`DoVerify`** (func 9497): a chave precisa ter 16 bytes (1426 "O tamanho da chave deve ser igual 16 bytes."),
  recalcula, compara 8 bytes e converte qualquer `std::exception` em 1426 "Falha ao autenticar.". Ela não tem chamador.
* **`CKey`** (func 9504): copia (titular, idParChaves, tipo, bytes, tag), depois 1412 "Titular da chave vazio.",
  1413 "Identificador do par de chaves igual a zero.", 1414 "Bytes da chave vazio.". `CKeyData(bytes)` (ckey.cpp:31,
  inlinado na 12110) → 1411 "Bytes da chave vazio.". Os dois destrutores zeram os bytes da chave (`memset` inlinado).
* **`ISymmetricCipher(const CAesKey&)`** (func 9474): copia a chave. Depois 1445 `std::format("Incompatibilidade entre o
  tamanho da chave e a quantidade de bits [{}/{}].", key.size(), (ueword)keySize)` se `keySize != 8·key.size()`, e
  1445 "Quantidade de bits nula." se `keySize == 0`. O destrutor func 9484 é compartilhado por ICF com `CBlockCipher` (u01).

Códigos de `ESecurityError` usados aqui: 1411–1414, 1417–1420, 1423–1426, 1432–1433 e 1445 (a u01 lista os demais).

---

## 5. Funções do uenux2 arquivadas nesta unidade

* **`CIntegridadeReferencial::VerificaVotosRdv`** (func 11514, nome inferido) verifica cada voto do RDV
  contra os dados estáticos e retorna `{ok, erro}`. Ela é chamada pela func 2543 → `Lanca` (func 2263, código 7871
  "Integridade referencial: {}"), depois de `CEleitores::CompleteLoad`, em `CSincronismoVotoEleitor::vf2` e em
  **`vota::CGravaResultado::vf2`**, ou seja, antes que os resultados sejam gravados no encerramento. As regras, em ordem
  (`TipoVoto` do RDV: legenda 1, nominal 2, branco 3, nulo 4, …, nuloAposSuspensao 6, nuloPorRepeticao 7):

  | voto | verificação | mensagem "um voto do cargo ({}) no RDV está com …" |
  |---|---|---|
  | qualquer | cargo na lista de cargos | "o cargo ({}) no RDV não foi encontrado na lista de cargos" |
  | legenda | o cargo tem candidatos e é proporcional | "voto de legenda ({}) com cargo que não é proporcional" |
  | legenda | ≥ 2 dígitos | "… com conteúdo insuficiente" |
  | legenda | o partido = 2 primeiros dígitos existe | "… para partido não encontrado" |
  | legenda | se tem o número completo de dígitos do cargo, nenhuma candidatura com esse número | "…, mas o candidato existe" |
  | nominal (consulta) | a resposta `cargo·10⁶+n` existe | "voto nominal ({}) para resposta não encontrada" |
  | nominal | a candidatura existe / é apta (situação == 0) | "… para candidatura não encontrada" / "… para candidatura inapta" |
  | nulo 4/6 com todos os dígitos (consulta) | essa resposta não existe | "voto nulo ({}), mas a resposta existe" |
  | nulo 4/6 com todos os dígitos | essa candidatura apta não existe | "voto nulo ({}) para candidatura apta" |

  O branco e os nulos de "cargo sem candidato" (8, 9) não são verificados. `nuloPorRepeticao` (7) é excluído
  explicitamente. Um conteúdo nominal que não é número faz `ToDWord` lançar `EUtilError` em vez de produzir um
  erro de integridade.
* **`CCalculaCV::Calcula`** (func 3700), o *Código Verificador* do BU: veja §6.2 e `docs/bu/codigo-verificador.md`.
* **`Utils::DesconverteDataJE`** (func 2276): `DataJE` "YYYYMMDD" → `api::CDate(ToByte(dd), ToByte(mm), ToWord(yyyy))`.
  `CDate(d, m, a)` (func 2765) reformata "{:02}{:02}{:04}" e analisa de novo com `strptime("%d%m%Y")`.
  Usada por `CConversorPleito::DoDesconverte` (func 11365: `CPleitoDTO{id, nome, data}`), `DesconverteDataHoraJE`,
  `CConversorHorarioVerao`, `CConversorEstadoGeralGap`.
* **`CDate::IsValid`** (func 5479): "DDMMAA" ou "DDMMAAAA", só dígitos, mês 1–12, dia 1…dias-do-mês (regra de ano
  bissexto gregoriana). Um ano de 2 dígitos ganha +2000, então "010170" é 1º de jan. de **2070**. O 2000 é uma variável de 16 bits @1584256 que tanto
  `IsValid` quanto `CDate::CDate(const std::string&)` (func 3649) carregam da memória (`i32.load16_u`), não um imediato, então
  ele não é uma constante de tempo de compilação nesse arquivo-fonte.
* **`CEleitor::GetAnoNascimento`** (func 5665): `ToDWord(dataNascimento.substr(0,4))`. A string em +68 é a
  data de nascimento `DataJE`, o que confirma o palpite da u05. As telas de "ano de nascimento" do mesário a comparam com o ano digitado.
* **`CVoto::EhNulo`** (5645 estática: tipo ∈ {4, 6, 7}; 11316 membro, passada como ponteiro para membro ao
  contador `CVotosEleicoesVota::Nulos`) e **`CCandidaturas::Chave`** (1939: `cargo·1000000 + número`, a chave
  dos mapas de candidaturas e de respostas; o `votaGetStateJson` do build web a usa para listar os números dos candidatos).

---

## 6. BOLETIM DE URNA: o que esta unidade contribui

Esta unidade não monta o BU. Ela fornece quatro dos mecanismos de integridade do BU. As outras unidades
(`vota::CGeraBU`, `comum::CGeradorBUQRCode`, `CGravaResultado`) e `docs/bu/*.md` descrevem o fluxo completo.

### 6.1 QR codes: cadeia de SHA-512 e entrada da assinatura (`CSha512`, `HexStringToBytes`)

Em `CGeradorBUQRCode::GeraQRCodes` (func 5604), para cada fatia de QR `b` (docs/bu/qrcode.md §5):

1. `msg = b == 0 ? slice[0] : join(done, " ") + " " + slice[b]`;
2. `CSha512 sha;` (func 2684: `EVP_MD_CTX_new`, `Reset`), depois `sha.Update(chunk)` para cada bloco de 512 bytes de `msg`
   (`Update` recusa um bloco vazio, e os blocos nunca são vazios), `digest = sha.Finish()` (64 bytes);
3. `hex = ToHex(digest)` em maiúsculas (func 1243) → `done.push_back(slice + " HASH:" + hex)`;
4. apenas para a **última** fatia: `CStringUtils::HexStringToBytes(hex)` transforma o hex de volta nos 64 bytes crus do digest,
   que o slot 6 da vtable de `api::pkcs11::IPkcs11` assina. O resultado é acrescentado como ` ASSI:<hex>`. O algoritmo, Ed25519,
   vem do manual do TSE e dos BUs oficiais; o wasm não o mostra.

Assim, `HASH_k = SHA-512(c1 " HASH:" h1 " " … " " ck)` e `ASSI = sign(bytes(HASH_last))`. Nenhuma classe neste binário
implementa `IPkcs11` (docs/bu/qrcode.md §6, u05), então o build web não consegue executar o passo de assinatura.

### 6.2 *Código Verificador* (`CKey`, `IMacAlgorithm::Mac`, `CSiphashMac`, `HexToQWord`)

1. **Chave** (inlinado em `vota::CGeraBU::vf2`, func 12110): lê `/dsk/fi/estatico/chave/cv.ber.pri`
   (`ModuloEnvelopeChave::EntidadeChave`), decifra `chave` com o cifrador AES derivado do segredo do HSM (u01),
   monta `CKey(descritor.nomeUsuario, descritor.serial, tipo 0/1/−1, bytes, tagChaves)` (func 9504, verificações 1412–1414)
   → `CKeyData` (1411) → os primeiros 16 bytes viram `CCalculaCV::m_chave`. Os dois objetos de chave são zerados na destruição.
2. **Cada código** (func 3700, `CCalculaCV::Calcula`):
   `m_dados += to_string(++m_contador)` → `CSiphashMac::DoMac(bytes(m_dados), tag, m_chave)` = SipHash-4-6, 8 bytes →
   `hex = Σ format("{:02x}", byte)` (estes dois passos são uma função separada, inlinada com o seu próprio frame de pilha de 416 bytes) → `v = HexToQWord(hex)` (a tag lida em big-endian) → `Reinicia(to_string(v))`
   (a cadeia recomeça a partir do valor completo de 64 bits) → retorna `format("{:010}", v % 10¹⁰)`, impresso como `d.ddd.ddd.ddd`.
3. O que entra em `m_dados` para cada bloco do BU, e os testes contra os 74 códigos oficiais: `docs/bu/codigo-verificador.md`.
   `DoVerify` nunca é chamada: o VOTA não tem código que verifique um CV.

No build web faltam `cv.ber.pri` e `api::IKernelHSM`, então o passo 1 lançaria uma exceção antes que qualquer código fosse calculado (u01 §5).

### 6.3 Consistência do RDV antes de os resultados serem gravados

`vota::CGravaResultado::vf2` (o passo do *encerramento* que grava os arquivos de resultado) chama a func 2543 →
`VerificaVotosRdv` (func 11514, §5). Qualquer voto inconsistente do RDV aborta com `EUeComumDadosError` 7871
"Integridade referencial: um voto do cargo (…) no RDV está com …". Esta verificação não executa nas sessões
gravadas, e o build web não guarda votos no RDV (`CSincronismoVotoEleitorWeb`).

### 6.4 Datas no cabeçalho do BU

A data do pleito que entra na identificação do BU e nos EXTRAS do CV (`YYYYMMDD`) vem de
`CConversorPleito::DoDesconverte` → `DesconverteDataJE` (funcs 11365/2276), que executaram na inicialização.

---

## 7. Particularidades do build web e observações sobre WebAssembly/Emscripten

* **SQLite sobre MEMFS.** `uenux.db` existe tanto em `/dsk/fi` quanto em `/dsk/fe`, apenas na memória do navegador. Nada é
  enviado pela rede. O `std::mutex` em torno de `sqlite3_finalize` não faz nada neste build de thread única:
  `lock()` compila para nada e só resta o resíduo de `unlock()` (func 150). A func 9431 (`~mutex` na saída,
  slot 6389 da tabela) é registrada, mas nunca referenciada por nenhum código.
* **Endereçamento de segmentos de dados no pseudocódigo.** Expressões como `d_operator0033s040504040x0505T[326061]:long@1`
  são relativas ao início de um segmento de dados, que é 1024 para esse segmento. Some 1024: 326061 → 327085
  "PRAGMA foreign_keys = ON". `d_ABCDEFGHIJKLMNOPQRSTUVWXYZab[660]:long` é escalado em 8 bytes a partir do segmento em 1112848
  (o alfabeto Base64), então aponta para 1118128 "xXabcdefABCDEF0123456789". `q.py wat` mostra os endereços reais.
* **Anotações enganosas.** Códigos de erro iguais a um endereço de string ganham um comentário de string: `1730 /* "QRBU:{}:{} VRQR:{} {}" */`
  em `GetInt64` e `1411 /* "Quantidade de escolhas…" */` na func 12110 são os códigos 1730 e 1411, não strings.
* **Tipos de argumento compactados de `std::format`.** 5 bits por argumento: `6` = unsigned, `13` = string_view. `422` = (unsigned,
  string_view) na 11514, `198` = (unsigned, unsigned) em `ISymmetricCipher`, `13` = um string_view em `IndexForColumnName`.
  A string de formato é a outra constante de 8 bytes: `399432394920` = `{data 436392, size 93}` é o texto "Falha ao executar
  operação…".
* **Strings Latin-1.** "Índice", "não", "INDISPONÍVEL" são guardadas como bytes únicos (`cd`, `e3`, `cd`). O código do TSE é compilado
  com um conjunto de caracteres de execução Latin-1.
* **merge-similar-functions.** `ToByte`/`ToWord` compartilham o corpo 6153 (máscara e máximo são parâmetros). As cópias de `ToLower`/`Trim`/`ToUpper`
  compartilham o corpo 3942 (a transformação é um slot da tabela).
* **ICF.** `ISymmetricCipher::~` = `CBlockCipher::~` (9484). `CSha::GetName` = slot 2 de `CSubReport` = um slot de
  `std::function` (3874). `CSqlConnection::GetPath` = um slot de `leaf_node` do RHVoice (5322).
* **Hospedeiros de inlining.** A 3700 carrega os srclocs de `imacalgorithm.cpp` e `cstringutils.cpp:754`, mas é `CCalculaCV`. Cada
  chamada inlinada mantém o seu próprio frame de pilha (`stack_pointer -= n` … `+= n` aninhados), o que mostra que o passo de MAC e hex é uma
  função separada (416 bytes) que contém `Mac` (32 bytes). A 1375 são as duas sobrecargas de `GetValor`. A 12110 contém `CKeyData`, `CKey::GetKeyData` e os dois destrutores de chave.
* **Chamadas diretas vs virtuais.** `CSiphashMac::DoVerify` chama `DoMac` diretamente (slot 6253 da tabela). Os getters por nome do result set
  chamam diretamente o getter por índice. Toda chamada vinda dos DAOs passa pela vtable.
* **Exceções.** Os caminhos de erro de SQL montam a mensagem, depois a exceção, por meio de `invoke_*`. O
  `catch(...){unlock; throw;}` de `CSqlStatement::Close` vira `__cxa_begin_catch` + `__cxa_rethrow`.
* **errno** é o da musl @1931660, e `ERANGE` = 68, `EOVERFLOW` = 61 (numeração WASI).
* **Código de biblioteca arquivado aqui.** boost::date_time é usada por `SetDateTime` (1376, 5182, 5183, 9414). O charconv da libc++
  (2866, 5919, 3846) atende `std::to_string(unsigned long long)`. O formatador de inteiros de 128 bits do `<format>` da libc++
  (1949, 5912) está linkado, mas não tem chamador do TSE.

---

## 8. Código estranho ou arriscado

A ordem é por gravidade. "Latente" significa que nenhum chamador neste binário alcança o caminho. O mesmo código de biblioteca provavelmente roda na urna.

1. **`CheckIntegrity` nunca lê o resultado (func 9452, média, latente).** Ela prepara e dá um passo em
   `PRAGMA integrity_check(1)`, fecha o statement e retorna `true`. O SQLite informa problemas de integridade como
   uma linha de resultado comum (o texto do primeiro problema) com `SQLITE_ROW`, não como um erro. Assim, um
   `uenux.db` danificado passa nesta verificação, a menos que o SQLite falhe de vez. Não existe chamador estático: o método só é
   alcançável pela vtable, e ele não executou nos votos gravados.
2. **Use-after-free quando a API é mal usada (funcs 9460, 9458, 9417, latente).** `Close()` deixa o `sqlite3*`
   liberado no objeto, e `Prepare` não verifica `m_fechada`. Um result set mantém o `sqlite3_stmt*` cru
   depois que `CSqlStatement::Close` o finalizou. Em wasm isso lê memória linear liberada; não gera trap.
3. **`CSiphashMac::DoVerify` (func 9497, baixa, latente).** Ela compara 8 bytes em `mac.data()` sem verificar
   `mac.size()`: uma leitura fora dos limites, ou do endereço 0 para um vector vazio. A comparação não é escrita como
   uma comparação de tempo constante (sem `CRYPTO_memcmp`), mas o comprimento fixo compila para um único `i64.ne`, então este build não tem
   vazamento por tempo. Os seus dois erros usam o mesmo código, 1426. Nada a chama: o VOTA nunca verifica um *Código Verificador*.
4. **Os parsers sem sinal aceitam entrada negativa (func 2202; `ToQWord` func 1141, baixa).** `+`/`-` estão no
   conjunto de caracteres permitidos, e `strtoull` nega módulo 2⁶⁴, então `ToQWord("-5")` = 18446744073709551611. Isto
   não é erro, porque o máximo é 2⁶⁴−1. Os DAOs usam `ToQWord` para valores de `numero_titulo`. `ToDWord("-1")`
   só é rejeitado porque o valor que dá a volta excede 2³²−1.
5. **`VerificaVotosRdv` não valida o conteúdo antes (func 11514, baixa).** Um voto nominal ou nulo cujo
   conteúdo não é um número faz `ToDWord` lançar `EUtilError` 1875/1876, e não o erro de integridade que
   `Lanca` produziria. Os votos em branco e os nulos de "cargo sem candidato" (8, 9) não são verificados, e
   `nuloPorRepeticao` (7) é excluído explicitamente.
6. **Ida e volta de `SetDateTime`/`GetDateTime` (funcs 9422/5162, baixa).** Um `ptime` especial é guardado como
   `not-a-date-time`/`+infinity`/`-infinity`. `parse_iso_time` (func 5851) executa o parser de valores especiais do boost quando
   o texto começa com `+`, `-`, `n` ou `m`, então `+infinity` é lido de volta. Os outros dois não: remover `-` dá
   `notadatetime` e `infinity`, e `parse_iso_time` lança uma exceção do boost, não `CSqlError`. Horários comuns
   fazem a ida e volta.
7. **Tratamento de NULL (funcs 9420, 5163, info).** `SetBlob` com um vector vazio que nunca alocou faz bind de SQL NULL
   (`data() == nullptr`), então um blob vazio assim é lido de volta como NULL. `IsNull` decide por meio de `sqlite3_column_text`,
   que retorna NULL apenas para SQL NULL (um BLOB ou TEXT de comprimento zero dá `""`: verificado com `x''` e `zeroblob(0)`),
   então a sua resposta está certa, mas ela converte o valor para texto no lugar. `GetText` copia com `strlen`, então um texto
   com um NUL embutido é truncado.
8. **Regra de século de `CDate::IsValid` (func 5479, info).** Um ano de 2 dígitos sempre ganha +2000, então "010170" é
   1º de jan. de 2070. Nada na unidade mostra quais entradas têm 6 dígitos.
9. **`HexStringToBytes` descarta em silêncio um nibble final ímpar (func 3506, info).** Os seus chamadores (assinatura do QR,
   serial da mídia) passam hex de comprimento par.
10. **`CBitArray::GetValor` (func 1375, info).** A verificação do modo de leitura informa "sem permissao de **escrita**".
    Para mais de 16 bits o resultado é truncado para `ueword`. Uma contagem acima de 32 é UB de deslocamento em C++;
    o wasm aplica uma máscara à contagem.
11. **`sqlite3_open` com flags padrão (func 3515, info).** Um `uenux.db` ausente é criado vazio sem nenhum
    aviso. Em uma urna, uma flash perdida ou substituída pareceria um banco de dados novo, e o
    `CREATE TABLE IF NOT EXISTS` dos DAOs recriaria as tabelas.
12. **Outros pontos (info).** `CSha::Update` recusa dados vazios, então o hash de uma mensagem vazia não pode ser
    calculado; os blocos do BU nunca são vazios. `CKey` aceita `KeyType` −1 (um `tipo` ASN.1 desconhecido).
    O construtor de `CSqlConnection` descarta a exceção capturada e informa `sqlite3_errmsg` no lugar. O mutex
    global em `CSqlStatement::Close` mostra que a urna finaliza statements a partir de várias threads; neste
    build ele não faz nada.
13. **Privacidade (info).** `uenux.db` guarda o comparecimento dos mesários (`comparecimento_mesario`: números de
    inscrição eleitoral, estado biométrico, horário) e as justificativas (`registro_justificativa`: número de inscrição, ano
    de nascimento). No build web ele vive apenas no MEMFS e nada nesta unidade o envia para lugar algum.

## 9. Questões em aberto

* Os nomes reais dos métodos que só aparecem na vtable: `BeginTransaction`/`Commit`/`Rollback`/`IsClosed`/`CheckIntegrity`/`GetPath`,
  `Execute`, `SetDateTime`/`GetDateTime`, `SetBool`/`GetBool`, `Next` e o slot 5 de `IHash`.
* O arquivo de `GetErrorMessage` (func 9411): um header interno compartilhado, ou um membro `static` de uma das classes.
* Os nomes dos enumeradores de `ESqlError`/`EUtilError`/`ESecurityError`. Os valores são certos. Todos os locais de throw foram
  pesquisados: EUtilError usa 1875–1878 e 1882–1884 (esta unidade) mais 1889 (`csynchronizer.cpp:59`, inlinado em
  `CFile::Sync`), e 1879–1881 nunca ocorrem. ESecurityError 1415/1416, 1421/1422 e 1427–1431 também nunca ocorrem;
  provavelmente pertencem a funções que não estão linkadas.
* `CBitArray` +12 (um `std::vector`, liberado pelo destrutor, mas não usado por `GetValor`) e a flag +25: provavelmente
  o lado de escrita (um `SetValor`), que não está linkado.
* `CKey` +44 (sempre 0).
* Quem chama `ISqlConnection::CheckIntegrity` na urna real. Aqui não há chamador estático.
* Se o ponteiro passado à func 5665 se destina a `CEleitor`, `CEleitorDecorator` ou `CEleitorDetalhe`. Os três começam
  no offset 0; o método está arquivado sob `CEleitor`, que é dono de `m_dataNascimento` (+68).
* O campo +52 de `md::CCandidatura`: a u04 o chama de situação/inapto (deve ser 0); a u02 o trata como "sem foto".
  As mensagens de integridade da func 11514 ("candidatura apta/inapta") apoiam a leitura da u04.

## 10. Tabela de mapeamento (todas as 100 funções da u13)

`ran` = visto em `analysis/runtime/*.functions.tsv`. "confiança" é a confiança do nome.

| func | tamanho | ran | símbolo reconstruído | arquivo-fonte (ou por que nenhum) | confiança |
|---|---:|:-:|---|---|---|
| 1141 | 62 |  | `ecourna::api::util::CStringUtils::ToQWord` | `src/ecourna/api/util/cstringutils.cpp` | alta |
| 1142 | 18 | ✓ | `ecourna::api::util::CStringUtils::ToByte` | `src/ecourna/api/util/cstringutils.cpp` | alta |
| 1375 | 993 |  | `ecourna::api::util::CBitArray::GetValor` | `src/ecourna/api/util/cbitarray.cpp` | alta |
| 1376 | 552 |  | `boost::gregorian::gregorian_calendar::from_day_number` | biblioteca: boost::date_time (ano/mês/dia a partir de um número de dia, bad_year 1400..9999), instanciada para SetDateTime | alta |
| 1523 | 67 | ✓ | `ecourna::api::util::CStringUtils::ToDWord` | `src/ecourna/api/util/cstringutils.cpp` | alta |
| 1525 | 964 |  | `ecourna::api::sql::sqlite::CSqlResultSet::IndexForColumnName` | `src/ecourna/api/sql/sqlite/csqlresultset.cpp` | alta |
| 1677 | 57 |  | `std::__tree<std::__value_type<std::string,int>>::destroy` | biblioteca: destruição recursiva dos nós de map<std::string,int> | alta |
| 1879 | 12 | ✓ | `ecourna::api::util::CStringUtils::ToLower(const std::string&)` | `src/ecourna/api/util/cstringutils.cpp` | média |
| 1882 | 267 |  | `ecourna::api::sql::sqlite::CSqlStatement::VerifyParameterIndex` | `src/ecourna/api/sql/sqlite/csqlstatement.cpp` | alta |
| 1883 | 267 |  | `ecourna::api::sql::sqlite::CSqlResultSet::VerifyColumnIndex` | `src/ecourna/api/sql/sqlite/csqlresultset.cpp` | alta |
| 1939 | 12 | ✓ | `comum::CCandidaturas::Chave` | `src/uenux2/src/app/comum/dados/ccandidaturas.u13.cpp` (caminho inferido) | baixa |
| 1949 | 2384 |  | `std::__formatter::__format_integer<unsigned __int128, char>` | biblioteca: formatação de inteiros de 128 bits do <format> da libc++ (sinal, prefixo, base 2/8/10/16, agrupamento por locale) | alta |
| 2200 | 71 |  | `ecourna::api::util::CStringUtils::ToInt32` | `src/ecourna/api/util/cstringutils.cpp` | alta |
| 2201 | 20 | ✓ | `ecourna::api::util::CStringUtils::ToWord` | `src/ecourna/api/util/cstringutils.cpp` | alta |
| 2202 | 1375 | ✓ | `ecourna::api::util::CStringUtils::ToUnsigned` | `src/ecourna/api/util/cstringutils.cpp` | média |
| 2276 | 362 | ✓ | `comum::asn::Utils::DesconverteDataJE` | `src/uenux2/src/app/comum/asn/util.u13.cpp` | média |
| 2678 | 261 |  | `std::map<std::string,int>::find` | biblioteca: __tree::find<std::string> da libc++ (mapa de nomes de colunas; também usado por shared_f5149) | alta |
| 2679 | 274 |  | `ecourna::api::sql::sqlite::CSqlResultSet::GetInt` | `src/ecourna/api/sql/sqlite/csqlresultset.cpp` | alta |
| 2866 | 91 |  | `std::__to_chars_itoa<unsigned long long> (std::to_chars)` | biblioteca: <charconv> da libc++ | alta |
| 2965 | 23 |  | `isxdigit` | biblioteca: isxdigit da musl (chamada por HexStringToBytes e pelo num_put da libc++) | alta |
| 3506 | 1265 | ✓ | `ecourna::api::util::CStringUtils::HexStringToBytes` | `src/ecourna/api/util/cstringutils.cpp` | alta |
| 3507 | 95 |  | `ecourna::api::util::CStringUtils::HexToInt` | `src/ecourna/api/util/cstringutils.cpp` | alta |
| 3508 | 68 | ✓ | `ecourna::api::util::CStringUtils::ToInt16` | `src/ecourna/api/util/cstringutils.cpp` | alta |
| 3513 | 112 |  | `ecourna::api::sql::sqlite::CSqlStatement::~CSqlStatement` | `src/ecourna/api/sql/sqlite/csqlstatement.cpp` | alta |
| 3514 | 439 |  | `ecourna::api::sql::sqlite::CSqlResultSet::GetText` | `src/ecourna/api/sql/sqlite/csqlresultset.cpp` | alta |
| 3515 | 1562 | ✓ | `ecourna::api::sql::sqlite::CSqlConnection::CSqlConnection` | `src/ecourna/api/sql/sqlite/csqlconnection.cpp` | alta |
| 3518 | 533 | ✓ | `ecourna::api::security::CSha::Finish` | `src/ecourna/api/security/csha.cpp` | alta |
| 3519 | 489 | ✓ | `ecourna::api::security::CSha::Update` | `src/ecourna/api/security/csha.cpp` | alta |
| 3700 | 1946 |  | `comum::CCalculaCV::Calcula` | `src/uenux2/src/app/comum/relatorios/ccalculacv.u13.cpp` (+ IMacAlgorithm::Mac inlinada, de src/ecourna/api/security/imacalgorithm.cpp, e HexToQWord, de cstringutils.cpp) | média |
| 3846 | 51 |  | `std::__itoa::__append10<unsigned long long>` | biblioteca: detalhes internos do <charconv> da libc++ | alta |
| 5155 | 1382 | ✓ | `ecourna::api::util::CStringUtils::ToSigned` | `src/ecourna/api/util/cstringutils.cpp` | média |
| 5160 | 275 |  | `ecourna::api::sql::sqlite::CSqlStatement::SetInt` | `src/ecourna/api/sql/sqlite/csqlstatement.cpp` | alta |
| 5161 | 476 |  | `ecourna::api::sql::sqlite::CSqlResultSet::GetBlob` | `src/ecourna/api/sql/sqlite/csqlresultset.cpp` | alta |
| 5162 | 514 |  | `ecourna::api::sql::sqlite::CSqlResultSet::GetDateTime` | `src/ecourna/api/sql/sqlite/csqlresultset.cpp` | média |
| 5163 | 290 |  | `ecourna::api::sql::sqlite::CSqlResultSet::IsNull` | `src/ecourna/api/sql/sqlite/csqlresultset.cpp` | alta |
| 5164 | 276 |  | `ecourna::api::sql::sqlite::CSqlResultSet::GetDouble` | `src/ecourna/api/sql/sqlite/csqlresultset.cpp` | alta |
| 5165 | 276 |  | `ecourna::api::sql::sqlite::CSqlResultSet::GetInt64` | `src/ecourna/api/sql/sqlite/csqlresultset.cpp` | alta |
| 5166 | 1622 | ✓ | `ecourna::api::sql::sqlite::CSqlResultSet::StepStatement` | `src/ecourna/api/sql/sqlite/csqlresultset.cpp` | alta |
| 5167 | 137 |  | `ecourna::api::sql::sqlite::CSqlConnection::~CSqlConnection` | `src/ecourna/api/sql/sqlite/csqlconnection.cpp` | alta |
| 5175 | 114 |  | `ecourna::api::security::CSha::~CSha` | `src/ecourna/api/security/csha.hpp` | alta |
| 5176 | 117 |  | `ecourna::api::security::CSha::~CSha [deleting]` | `src/ecourna/api/security/csha.hpp` | alta |
| 5177 | 290 | ✓ | `ecourna::api::security::CSha::Reset` | `src/ecourna/api/security/csha.cpp` | alta |
| 5182 | 299 |  | `boost::CV::simple_exception_policy<unsigned short,1,31,boost::gregorian::bad_day_of_month>::on_error` | biblioteca: lançador da verificação de faixa do boost::date_time | alta |
| 5183 | 299 |  | `boost::CV::simple_exception_policy<unsigned short,1,12,boost::gregorian::bad_month>::on_error` | biblioteca: lançador da verificação de faixa do boost::date_time | alta |
| 5479 | 723 | ✓ | `api::CDate::IsValid` | `src/uenux2/src/api/util/cdate.u13.cpp` | média |
| 5645 | 16 |  | `comum::md::CVoto::EhNulo(ETipo)` | `src/uenux2/src/app/comum/dados/md/rdv/cvoto.u13.cpp` | média |
| 5665 | 144 |  | `comum::md::CEleitor::GetAnoNascimento` | `src/uenux2/src/app/comum/dados/md/eleitor/celeitor.u13.cpp` | média |
| 5912 | 408 |  | `std::__formatter::__format_integer<unsigned __int128> [type dispatch]` | biblioteca: <format> da libc++: a sobrecarga externa (o valor já foi tornado unsigned + flag `negative` pelos dois chamadores, o com sinal e o sem sinal, de `__int128` na func 4027), switch sobre o tipo de apresentação com os prefixos 0b/0B/0/0x/0X | alta |
| 5919 | 76 |  | `std::__itoa::__base_10_u64` | biblioteca: detalhes internos do <charconv> da libc++ | alta |
| 6153 | 63 | ✓ | `ecourna::api::util::CStringUtils::ToByte/ToWord [merged body]` | `src/ecourna/api/util/cstringutils.cpp` | média |
| 9406 | 326 | ✓ | `ecourna::api::util::CStringUtils::Trim(std::string&)` | `src/ecourna/api/util/cstringutils.cpp` | média |
| 9410 | 18 |  | `ecourna::api::exception::CBaseError<util::EUtilError,{1875,1900}>::CBaseError(code, std::string&&, const source_location&) [thunk]` | helper de biblioteca/inlinado: thunk de 18 bytes -> corpo do ctor de CBaseError func 1011 com vtable @1117872 (header de exceções do ecourna, não reconstruído) | alta |
| 9411 | 222 |  | `ecourna::api::sql::sqlite::GetErrorMessage` | `src/ecourna/api/sql/sqlite/csqlconnection.cpp` (arquivo original desconhecido) | média |
| 9414 | 1175 |  | `boost::date_time::month_formatter<boost::gregorian::greg_month,boost::date_time::iso_extended_format<char>,char>::format_month` | biblioteca: formatador ISO do boost::date_time (setw(2), setfill('0'), fill saver) | média |
| 9416 | 264 |  | `ecourna::api::sql::sqlite::CSqlStatement::Reset` | `src/ecourna/api/sql/sqlite/csqlstatement.cpp` | alta |
| 9417 | 228 | ✓ | `ecourna::api::sql::sqlite::CSqlStatement::Execute` | `src/ecourna/api/sql/sqlite/csqlstatement.cpp` | média |
| 9419 | 273 |  | `ecourna::api::sql::sqlite::CSqlStatement::SetNull` | `src/ecourna/api/sql/sqlite/csqlstatement.cpp` | alta |
| 9420 | 290 |  | `ecourna::api::sql::sqlite::CSqlStatement::SetBlob` | `src/ecourna/api/sql/sqlite/csqlstatement.cpp` | alta |
| 9421 | 11 |  | `ecourna::api::sql::sqlite::CSqlStatement::SetBool` | `src/ecourna/api/sql/sqlite/csqlstatement.cpp` | média |
| 9422 | 3006 |  | `ecourna::api::sql::sqlite::CSqlStatement::SetDateTime` | `src/ecourna/api/sql/sqlite/csqlstatement.cpp` | média |
| 9423 | 305 |  | `ecourna::api::sql::sqlite::CSqlStatement::SetText` | `src/ecourna/api/sql/sqlite/csqlstatement.cpp` | alta |
| 9424 | 275 |  | `ecourna::api::sql::sqlite::CSqlStatement::SetDouble` | `src/ecourna/api/sql/sqlite/csqlstatement.cpp` | alta |
| 9425 | 275 |  | `ecourna::api::sql::sqlite::CSqlStatement::SetInt64` | `src/ecourna/api/sql/sqlite/csqlstatement.cpp` | alta |
| 9426 | 111 |  | `ecourna::api::sql::sqlite::CSqlStatement::~CSqlStatement [deleting]` | `src/ecourna/api/sql/sqlite/csqlstatement.cpp` | alta |
| 9428 | 7 |  | `ecourna::api::sql::sqlite::CSqlStatement::IsClosed` | `src/ecourna/api/sql/sqlite/csqlstatement.hpp` | média |
| 9429 | 443 | ✓ | `ecourna::api::sql::sqlite::CSqlStatement::Close` | `src/ecourna/api/sql/sqlite/csqlstatement.cpp` | alta |
| 9430 | 48 | ✓ | `ecourna::api::sql::sqlite::CSqlStatement::CSqlStatement` | `src/ecourna/api/sql/sqlite/csqlstatement.cpp` | alta |
| 9432 | 16 |  | `ecourna::api::sql::sqlite::CSqlResultSet::GetBlob(const std::string&)` | `src/ecourna/api/sql/sqlite/csqlresultset.cpp` | média |
| 9433 | 17 |  | `ecourna::api::sql::sqlite::CSqlResultSet::GetBool(const std::string&)` | `src/ecourna/api/sql/sqlite/csqlresultset.cpp` | média |
| 9434 | 14 |  | `ecourna::api::sql::sqlite::CSqlResultSet::GetDateTime(const std::string&)` | `src/ecourna/api/sql/sqlite/csqlresultset.cpp` | média |
| 9435 | 14 |  | `ecourna::api::sql::sqlite::CSqlResultSet::IsNull(const std::string&)` | `src/ecourna/api/sql/sqlite/csqlresultset.cpp` | média |
| 9438 | 16 |  | `ecourna::api::sql::sqlite::CSqlResultSet::GetText(const std::string&)` | `src/ecourna/api/sql/sqlite/csqlresultset.cpp` | média |
| 9439 | 14 |  | `ecourna::api::sql::sqlite::CSqlResultSet::GetDouble(const std::string&)` | `src/ecourna/api/sql/sqlite/csqlresultset.cpp` | média |
| 9440 | 14 |  | `ecourna::api::sql::sqlite::CSqlResultSet::GetInt64(const std::string&)` | `src/ecourna/api/sql/sqlite/csqlresultset.cpp` | média |
| 9441 | 14 |  | `ecourna::api::sql::sqlite::CSqlResultSet::GetInt(const std::string&)` | `src/ecourna/api/sql/sqlite/csqlresultset.cpp` | média |
| 9442 | 20 |  | `ecourna::api::sql::sqlite::CSqlResultSet::~CSqlResultSet [deleting]` | `src/ecourna/api/sql/sqlite/csqlresultset.hpp` | alta |
| 9443 | 17 |  | `ecourna::api::sql::sqlite::CSqlResultSet::~CSqlResultSet` | `src/ecourna/api/sql/sqlite/csqlresultset.hpp` | alta |
| 9444 | 41 |  | `ecourna::api::sql::sqlite::CSqlResultSet::Next` | `src/ecourna/api/sql/sqlite/csqlresultset.cpp` | média |
| 9446 | 12 |  | `ecourna::api::sql::sqlite::CSqlResultSet::GetBool` | `src/ecourna/api/sql/sqlite/csqlresultset.cpp` | média |
| 9447 | 365 |  | `std::map<std::string,int>::__emplace_unique_key_args (operator[])` | biblioteca: emplace do __tree da libc++ com piecewise_construct (@463337) | alta |
| 9449 | 135 | ✓ | `ecourna::api::sql::sqlite::CSqlResultSet::CSqlResultSet` | `src/ecourna/api/sql/sqlite/csqlresultset.cpp` | alta |
| 9452 | 503 |  | `ecourna::api::sql::sqlite::CSqlConnection::CheckIntegrity` | `src/ecourna/api/sql/sqlite/csqlconnection.cpp` | média |
| 9453 | 433 |  | `ecourna::api::sql::sqlite::CSqlConnection::Rollback` | `src/ecourna/api/sql/sqlite/csqlconnection.cpp` | média |
| 9454 | 433 |  | `ecourna::api::sql::sqlite::CSqlConnection::Commit` | `src/ecourna/api/sql/sqlite/csqlconnection.cpp` | média |
| 9455 | 433 |  | `ecourna::api::sql::sqlite::CSqlConnection::BeginTransaction` | `src/ecourna/api/sql/sqlite/csqlconnection.cpp` | média |
| 9456 | 140 |  | `ecourna::api::sql::sqlite::CSqlConnection::~CSqlConnection [deleting]` | `src/ecourna/api/sql/sqlite/csqlconnection.cpp` | alta |
| 9457 | 7 |  | `ecourna::api::sql::sqlite::CSqlConnection::IsClosed` | `src/ecourna/api/sql/sqlite/csqlconnection.hpp` | média |
| 9458 | 271 |  | `ecourna::api::sql::sqlite::CSqlConnection::Close` | `src/ecourna/api/sql/sqlite/csqlconnection.cpp` | alta |
| 9460 | 1136 | ✓ | `ecourna::api::sql::sqlite::CSqlConnection::Prepare` | `src/ecourna/api/sql/sqlite/csqlconnection.cpp` | alta |
| 9461 | 18 |  | `ecourna::api::exception::CBaseError<sql::ESqlError,{1725,1775}>::CBaseError(code, std::string&&, const source_location&) [thunk]` | helper de biblioteca/inlinado: thunk de 18 bytes -> corpo do ctor de CBaseError func 1011 com vtable @1116272 (header de exceções do ecourna, não reconstruído) | alta |
| 9474 | 1721 |  | `ecourna::api::security::ISymmetricCipher::ISymmetricCipher` | `src/ecourna/api/security/isymmetriccipher.cpp` | alta |
| 9484 | 72 |  | `ecourna::api::security::ISymmetricCipher::~ISymmetricCipher` | `src/ecourna/api/security/isymmetriccipher.hpp` | alta |
| 9497 | 814 |  | `ecourna::api::security::CSiphashMac::DoVerify` | `src/ecourna/api/security/csiphashmac.cpp` | alta |
| 9498 | 3654 |  | `ecourna::api::security::CSiphashMac::DoMac` | `src/ecourna/api/security/csiphashmac.cpp` | alta |
| 9504 | 1189 |  | `ecourna::api::security::CKey::CKey` | `src/ecourna/api/security/ckey.cpp` | alta |
| 11316 | 10 |  | `comum::md::CVoto::EhNulo` | `src/uenux2/src/app/comum/dados/md/rdv/cvoto.u13.cpp` | média |
| 11365 | 214 | ✓ | `comum::asn::CConversorPleito::DoDesconverte` | `src/uenux2/src/app/comum/dados/asn/processoeleitoral/cconversorpleito.u13.cpp` (caminho inferido) | média |
| 11414 | 97 |  | `ecourna::api::util::CBitArray::~CBitArray [deleting]` | `src/ecourna/api/util/cbitarray.hpp` | alta |
| 11417 | 94 |  | `ecourna::api::util::CBitArray::~CBitArray` | `src/ecourna/api/util/cbitarray.hpp` | alta |
| 11514 | 6868 |  | `comum::CIntegridadeReferencial::VerificaVotosRdv` | `src/uenux2/src/app/comum/dados/cintegridadereferencial.u13.cpp` | média |
