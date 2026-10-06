# u18 — uenux2 `api/io` (ASN.1 files, encrypted RDV file, INI keys, in-memory tables), `api/ipc` (threads, message queues, POSIX locks), `api/hwil/ipower.h` + stray VOTA functions

Unit u18 has 95 functions. They come from three groups:

1. **uenux2 `src/api` infrastructure** that the whole voting application sits on:
   * `api/io/asn/cfileasn.h`: reading and writing BER files (`CFileASN`);
   * `api/io/asn/cpartialfileasn.h`: the file-backed seeder of the streaming BER decoder (`CFileSeeder`);
   * `api/io/cdatamap.h`: the in-memory tables with a cursor (`CDataMap`);
   * `api/io/cencryptedfile.cpp`: the encrypted working copy of the RDV (`CEncryptedFile`);
   * `api/io/cinikey.cpp`: INI/properties keys (`CIniKey`);
   * `api/ipc/*`: threads (`CThread`), inter-thread message queues (`CPriorityMessageQueue`), and POSIX mutex, rw-lock and semaphore wrappers;
   * `api/hwil/ipower.h`: the default bodies of the power/battery interface (`IPower`).
2. **Functions named after an inlined template.** The analyzer named 20 app functions after a `cfileasn.h` or `cdatamap.h` srcloc because the template was inlined into them. Their real owners are the result writers (`CGravadorRDV`, `IGravadorEnvelope`), the loaders of eg.bin, `-lo.dat` and `-cp.dat`, the mesários' attendance registry, and a few misattributed helpers (`CUrna` constructors, hex encoder).
3. **VOTA state and thread glue** that ended up in the unit by call-graph proximity. It is mostly one-line `StartState`/`ProcessMessage` methods that post messages between the voter terminal and the operator terminal, plus the urna and web execution policies.

Reconstructed sources (all written by u18):

| file | content |
|---|---|
| `src/uenux2/src/api/io/asn/cfileasn.h` | `api::CFileASN` (all template members, error codes, instance list) |
| `src/uenux2/src/api/io/asn/cfileasn.instances.cpp` | the instances that survive as functions (explicit instantiations) |
| `src/uenux2/src/api/io/asn/cpartialfileasn.h` | `api::CFileSeeder` (complete; u12 had 3 slots in `cpartialfileasn.u12-fragment.h`) |
| `src/uenux2/src/api/io/cdatamap.h` | `api::CDataMap<K, R>` |
| `src/uenux2/src/api/io/cencryptedfile.h/.cpp` | `api::CEncryptedFile` (Load, MemRead, MemWrite; Save is in u12's fragment) |
| `src/uenux2/src/api/io/cinikey.h/.cpp` | `api::CIniKey` |
| `src/uenux2/src/api/ipc/cmessagequeue.h` | `api::SMessage`, `api::CMessageInterface`, `api::CPriorityMessageQueue<T>` |
| `src/uenux2/src/api/ipc/cthread.h/.cpp` | `api::CThread` |
| `src/uenux2/src/api/ipc/posix/cposix{mutex,rwmutex,semaphore}.h/.cpp` | the three POSIX wrappers |
| `src/uenux2/src/api/hwil/ipower.h` | `api::IPower` |
| `src/uenux2/src/app/comum/u18-foreign-fragments.cpp` | CGravadorRDV / IGravadorEnvelope `GravaResultado`, `md::CUrna` ctors/dtor, `CCarga` copy, CLocal::Carrega, eg.bin loader, processo eleitoral / pleito loaders, CRegistradorMesario, the DSNumero text source, ecourna hex encoder |
| `src/uenux2/src/app/vota/u18-foreign-fragments.cpp` | thread singletons, execution policies, the voter/operator message senders |

---

## 1. Classes and how they relate (RTTI)

```
api::CThread (abstract, typeinfo @1600028, vtable @1599960: [0] ~ 2721, [1] 325 (unreachable: abstract),
│             [2] Run =0, [3] TrataExcecao =0, [4] TrataExcecaoDesconhecida =0)
└─ vota::CThreadVota ─┬─ vota::CThreadEleitor   (84 B, singleton 316)      unit u07
                      ├─ vota::CThreadOperador  (132 B, singleton 270)     unit u10
                      └─ vota::CThreadMonitor   (48 B, singleton 1898)     unit u26 (typeinfo @1600968)

api::IThreadImpl ─ simulador::CWasmThread (web; Create 9655, Wait 9652, Yield 9651)
api::IGenericFactory<T> ─ CDefaultGenericFactory<IThreadImpl, CWasmThread> / <ISyncCtl, CPosixMutex> /
                          <IRWSyncCtl, CPosixRWMutex> / <ISemaphore, CPosixSemaphore>
api::ISyncCtl ─┬─ api::CPosixMutex (32 B)          [2] Lock 10252  [3] Unlock 10251
               └─ api::ISemaphore ─ api::CPosixSemaphore (20 B)   [2] Lock [3] Unlock (both 2219, empty ICF body)
                                    [4] Wait() 10248 (name inferred) [5] Wait(size_t) 10247 [6] GetValue 10246
api::IRWSyncCtl ─ api::CPosixRWMutex (36 B)        [2] ReadLock [3] WriteLock [4] Unlock (all 2219)
(the POSIX method names are attested by srcloc records, most of them unreferenced: see §2.7)

api::CPriorityMessageQueue<api::SMessage> (vtable @1534772)   api::CMessageInterface
        └───────────────┬───────────────────────────────┘
          vota::CMessageEleitor [vmi]  (CThreadEleitor +36)     vtable [2] = Recebe 4343 (thunk 4337)
          vota::CMessageOperador [vmi] (CThreadOperador +36)

ASN1::ISeeder ─ api::CFileSeeder (vtable @1564408)
api::IPower ─ api::teste::CPowerMock (the only implementation in the web build)
api::CDataMap<K,R> (not polymorphic) ← comum::CEleitores, CCandidaturas, CPartidos, CRespostas, CJustificador,
                                       comum::CRegistradorMesario (name from cregistradormesario.cpp:85)
api::CFileASN, api::CEncryptedFile, api::CIniKey: not polymorphic
```

Error classes used by the unit (thunks 210, 1889, 1229 build them; they are merged TSE exception constructors
that take the vtable as a parameter, see `docs/libraries/libcxx-core.md` §1.2):

| alias | class (RTTI) | limits |
|---|---|---|
| `api::CUeIoError` | `CBaseError<api::EUeIoError, SErrorLimits{5950, 6150}>` (typeinfo @1528172) | io |
| `api::CUeIpcError` | `CBaseError<api::EUeIpcError, SErrorLimits{6150, 6350}>` (@1528380) | ipc |
| `api::CUeHwilError` | `CBaseError<api::EUeHwilError, SErrorLimits{5150, 5950}>` (@1531040) | hwil |

---

## 2. Subsystems and control flow

### 2.1 `api::CFileASN`: ASN.1 files (`cfileasn.h`)

Header-only templates. Everything was inlined, but the `std::source_location` records keep the line numbers:

| line | member | code | message |
|---|---|---|---|
| 48 | `ReadFromFile(const CFile&)` | 5950 | `O arquivo [{}] não estava aberto` |
| 60 | idem | 5951 | `O arquivo [{}] é muito grande para ser lido` (size > 5 MiB = 5 242 880) |
| 78 | `ReadFromFile(const std::string&)` | 5952 | `O arquivo [{}] não existe` (`CSystem::IsRegularFile`, func 412) |
| 135 | `DecodeObjectFunction` | 5953 | `Conteúdo não foi decodificado para {}: {}` |
| 143 | idem | 5954 | `Conteúdo inválido para {}: {}` (`isValid() && isStrictlyValid()` required) |
| 161 | `CodeObjectFunction` | 5955 | `Objeto com conteúdo inválido para {}: {}` (only `isValid()`) |
| 171 | idem | 5956 | `Arquivo não foi codificado para ` + contexto |

The second `{}` is the output of `ASN1::trace_invalid` (TSE's Portuguese InvalidTracer). The first is a
context string: `"ReadFromFile de " + file name`, `"DecodeObject de " + typeid(T).name()`,
`"CodeObject de " + typeid name`, or `"WriteToFile de " + file name`. The mangled type name appears in the
error messages (for example `DecodeObject de N25ModuloRegistroDigitalVoto27EntidadeRegistroDigitalVotoE`).

Read path: `IsRegularFile` → `CFile(name, "rb")` → `Position` / `Seek(END)` / `Position` / `Seek(back)` → size
check → `std::vector<char>(size)` → `RawRead` (the result is ignored) → `CoderEnv{ber}.decode` → validity →
`Close`. Write path: `isValid` → `CoderEnv{ber}.encode` (or `encodeBER`) into a new vector → swap →
`CFile::RawWrite`.

Two converter-level helpers have no srcloc of their own. Their names are inferred:
`ReadDataFromFile<TConversor>(name)` (read the entity, then `Desconverte`) and
`WriteDataToFile<TConversor>(CFile&, dado)` (`Converte`, then write). Both create the converter as a local, which
is why the converter vtable is stored in the callee's frame. A comum converter's
`Converte`/`Desconverte` adds EUeComumAsnError 7653 `Entidade deixada em estado inválido: {}` (iconversorasn.h:56)
or 7654 `Entidade está inválida: {}` (:71). An ecourna converter adds EApiAsnError 1900/1902 (iconversorasn.hpp:49/66).

Which ASN.1 files go through the functions of this unit:

| wasm | direction | entity | file |
|---|---|---|---|
| 9673 / 9695 / 9702 / 9801 (→ 2927) | write | EstadoGeralVota / SA / Gap / Urna | `dinamico/trab{1,2}/vota.bin`, `sa.bin`, `gap.bin`, `eg.bin` (fixtures written at votaInit, u12) |
| 2724 (→ 6006) | write | EntidadeEnvelopeGenerico | `-bu.dat` (CGravadorBU 11629), `-imgbu.dat`, `-imgze.dat` (11626), func 2725 |
| 5367 (→ 6006) | write | EntidadeHashes | `-hash.dat` (CGravadorHashes 11620) |
| 5366 | write | EntidadeResultadoUrnaCadastro (ecourna) | `-jufa.dat` (CGravadorRCSecao 11616) |
| 11587 (inlined) | write | EntidadeResultadoRDV | `-rdv.dat` |
| 5825 | decode | EntidadeRegistroDigitalVoto | the RDV BER (rdv.dat content, and re-decoded by 11587) |
| 3791 | read | EstadoGeralUrna → CEstadoGeral | `eg.bin` (IServicoEstado<CEstadoGeral>) |
| 3771 | read | EntidadeProcessoEleitoral → CProcessoEleitoralDTO | `<fase><processo:05>-cp.dat` (e.g. `t02400-cp.dat`) |
| 5778 | read | EntidadeSituacoesEleicoes, EntidadeEleicao | `<fase><pleito:05><uf>-ste.dat`, `<fase><eleição:05><uf>-ce.dat` |
| 5706 | read | CabecalhoPacote → md::CCabecalhoPacote | `...-ce.pid` (package header: the version of each eleição's data) and the loader 7787 |
| 5740 | read | ModuloLocal::Local → md::CLocal | `<município:05><zona:04><seção:04>-lo.dat` |
| 3735 | read | InformacaoMidia (ecourna) | `/dsk/fe/estatico/infomidia.dat` (MV serial), **absent in the simulator** |

### 2.2 `api::CFileSeeder` (`cpartialfileasn.h`)

This is the `ASN1::ISeeder` over an open `CFile` used by the "partial" BER decoder (CoderEnv rule 2). That decoder
reads the voter roll `-el.dat` one voter at a time. Members: `+4 const CFile*`, `+8 size_t m_tamanho` (end
of the readable data). Slots: `fim()` = `CFile::Eof()` (stdio `feof`); `decodeByte` (:52, 5957); `decodeBlock`
(:75, 5958, clamps to `m_tamanho`); `seek` (:88 5959 beyond the end, :94 5960 if `fseek` fails); `skip` (:106,
5961, then `seek`); `restante()`, `posicao()`. decodeByte, decodeBlock and seek were **observed executing** during
votaInit.

### 2.3 `api::CDataMap<K, R>` (`cdatamap.h`)

28 bytes: `+0 std::map<K,R>`, `+12 iterator m_atual` (== end when there is no current record), `+16 std::string
m_nome`. Srcloc-attested members: `GetCurrent()` (:98, 5976 `O registro corrente estava inválido {}`), `Next()`
(:117, 5977 `Operação inválida {}`), `Add(value_type)` (:143, 5978 `Duplicidade de registro {}`; the new record
becomes current), `Update(key, record)` (:158, 5979 `Registro não encontrado {}`; the record becomes current).
wasm-opt merged GetCurrent into 2919 and Next into 3892. The only thing that differs between instances is the offset
of the value inside the tree node (16-byte header + key size) and the srcloc. The instances are listed in §6.

`comum::CRegistradorMesario` (func 815 builds its 52-byte singleton) is the mesários' attendance table
(*comparecimento de mesários*, the record of which poll workers were present). It holds
`CDataMap<CComparecimentoMesarioPK, CComparecimentoMesario>` (+0), a second map indexed by the mesário's
identity (+28), and a `servico::CComparecimentoMesarioServico` (+40) that holds the SQLite DAO. Func 5378 is its
save step: it indexes the record by identity, then `Update` if the PK exists or `Add` if not. The caller then runs
`SaveCurrentInternal()` (cregistradormesario.cpp:85, "O contêiner estava vazio" if the table is empty), which
writes the current record to the SQLite table `comparecimento_mesario` of `uenux.db`.

### 2.4 `api::CEncryptedFile` (`cencryptedfile.cpp`): the working copy of the RDV

The RDV (*Registro Digital do Voto*, the digital record of every ballot cast) is kept on both flash memories
(MI = internal, MV = the voting memory card) as `dinamico/trab{1,2}/rdv.dat`. That file is encrypted:
`MemWrite(CRdvVota::Converte())` → `Save(".tmp")` (u12: PKCS#7-like padding to 16 bytes, then
`ISymmetricCipher::Encrypt`). After the write it is read back and compared: `Load(".tmp")` → `MemRead` →
`CRdvVota::ConfereConteudo` (u07 §4.2). On reboot it is read again: `CEleitores::CompleteLoad` (u04) → `Load` →
`MemRead` → `CRdvVota::Desconverte`.

Load (func 3653, lines 36-73):

1. empty name → throw 5987; `!IsRegularFile` → throw 5988 `Arquivo [{}] não existe`;
2. `tamanho = GetFileSize`; `tamanho == 0 || tamanho % 16` → **5989 built but not thrown**;
3. `CFile(name, "rb")`, `RawRead(tamanho)`; short read → **5990 built but not thrown**. (A 0-byte file is still
   rejected, one layer down: the empty vector has a null buffer and `CFile::RawRead(nullptr, 0)` throws
   `ecourna::api::io::CIoError` 1192 `buffer invalido`.)
4. `m_cifrador->Decrypt(cifrado, claro)` (vtable slot 3);
5. `pad = claro.back()`; `pad > 16` → throw 5991; `pad > tamanho` (the encrypted file size) → throw 5992;
6. each of the `pad-1` bytes before the last must equal `pad`, otherwise **5993 built but not thrown**;
7. `m_dados = claro[0 .. size-pad)`, `m_posicao = 0`. A pad byte of 0 is accepted.

MemWrite/MemRead (lines 142-167) copy into and out of `m_dados` at the cursor. The vector overloads (2766 and 3652)
are the wasm functions. MemWrite ignores an empty input vector. MemRead does nothing when the cursor is already at
the end; otherwise it appends the remaining bytes to the output vector (the new space is pre-filled with 0xFF, then
overwritten).

### 2.5 `api::CIniKey` (`cinikey.cpp`)

`{+0 nome, +12 valor}`. `operator=` (5483, :24) refuses to change the name (6005 `O nome de uma chave não pode
ser modificado.`). It is used by the INI parser (u12) for `/etc/dependencias.properties` and `/etc/versoes.properties`.

### 2.6 Threads and messages (`api/ipc`)

* **`CThread`** (3598): `+4 m_estado` (0 stopped, 1 running, 3 finished), `+8 m_bParar`, `+9 m_bDormindo`,
  `+12 unique_ptr<IThreadImpl>` from `IGenericFactory<IThreadImpl>` (cthread.cpp:31, CWasmThread in the web build),
  and `+16 unique_ptr<ISyncCtl>` (:32, CPosixMutex). `Start` (1684, :68) sets state 1 and calls
  `IThreadImpl::Create(ThreadProc = func 10255, this)`. It throws 6231 `Nao foi especificado o tipo de implementação`
  when there is no implementation. `Wait` (1900) joins and resets the state to 0. `ThreadProc` runs `Run()` and
  sets state 3.
* **`CPriorityMessageQueue<SMessage>`**: `+4` a `std::priority_queue` of 16-byte entries `{SMessage (8), int
  prioridade, unsigned sequencia}`. Order: highest priority first, FIFO within a priority. Then `+20
  unique_ptr<ISemaphore>` (cmessagequeue.h:113) and `+24 unique_ptr<ISyncCtl>` (:114), and `+28` the sequence
  counter, which resets when the queue empties. `Add` (rhvoice_f501, misattributed) locks, pushes and posts the
  semaphore. `Remove` (2073, :153) locks and throws 6228 `A fila estava vazia.` if the queue is empty; otherwise it
  pops and unlocks. `Recebe(timeout)` (slot 2, 4343) waits on the semaphore and, on result 0, calls `Remove()`.
  It stores the message and result in the `CMessageInterface` part (+36/+44).
* **Message traffic in this unit** (all with priority 1). The voter terminal's queue is `CThreadEleitor+36`. Its
  first state `CAguardaMensagem` reacts to 1, 7, 11, 12 and 14 (u02/u07). The operator's queue is
  `CThreadOperador+36`:

| sender (wasm) | queue | id | meaning |
|---|---|---|---|
| `CControladorRegistraMesariosVota` slot 10 (10792 → merged body 6018), called by `CEncerraRegistroMesarios` (10380) at estadoVota 55 | voter | 11 | registration of the opening mesários done → `CIniciodeCiclo` (urna ready) |
| `CControladorRegistraMesariosVota` slot 15 (10787 → merged body 6018), called by `CEncerraRegistroMesarios` (10380) at estadoVota 58 | voter | **7** | registration of the closing mesários done → **encerramento: estadoVota GERARBU, "Inicio do Encerramento", `CGeraBU`** |
| `CRegistroMesarioEncerrado::StartState` (10763) | voter | 14 | → `CMostraTelaContinuaVotacao` |
| `CMostraTelaContinuaVotacao::StartState` (7221) | operator | 12 | voter terminal shows the "continue voting" screen, then waits |
| `CReinicioComparecimentoMesario::StartState` (11917 → merged body 6058) | operator | 7 | voter waits while the operator re-registers mesários |
| `CFinalizaAquisicao::StartState` (12113 → merged body 6058) | operator | 10 | end of vote acquisition; voter terminal waits |
| `CDefineRotaPreVotacao::NeedChangeState` (11994) | operator | 7 | demonstration mode (and not *treinamento de eleitor*): voter waits |
| `CUrnaInspecionada::ProcessMessage(13)` (11794) | — | — | shows the continuation screen, back to `CAguardaMensagem` |

* **Execution policies** (`vota::IExecucaoVota`). The urna's `CExecucaoVota` starts the voter, operator and
  monitor threads (`Inicia` 10235), sleeps 1 s, and joins them (`Aguarda` 10234, `Executa` 10236). The web build
  registers `CExecucaoVotaCooperativa` instead. Its slots 2/3 (4713, observed) only install the first state in
  `CThreadEleitor`, and `votaTick` steps that thread (u07 §3.4).

### 2.7 POSIX wrappers (`api/ipc/posix`)

`CPosixMutex` is recursive (`pthread_mutexattr_settype(PTHREAD_MUTEX_RECURSIVE)`, 6150 `Falha ao criar mutex
recursivo: {}` with `strerror`). `CPosixSemaphore(0)` (`sem_init`, 6159 `Falha ao criar semáforo: {}`) has
`Wait(timeoutMs)`: `clock_gettime(CLOCK_REALTIME)` (6160 `Falha ao obter tempo de relógio: {}`), deadline in
ms → s/ns with carry, then `sem_timedwait`, and returns 0 when signalled. `CPosixRWMutex` wraps
`pthread_rwlock_t`.

The web build removed most error paths (the pthread/sem stubs return 0), but their `std::source_location`
records are still in the data segment, unreferenced (`analysis/srcloc.tsv`). They attest the method names and
lines:

| file | records (line) |
|---|---|
| cposixmutex.cpp | ctor (30, referenced), `Lock()` (47), `Unlock()` (56) |
| cposixrwmutex.cpp | ctor (29, 35, 42: attribute init, `pthread_rwlock_init`, attribute destroy), `ReadLock()` (58), `WriteLock()` (69), `Unlock()` (80) |
| cposixsemaphore.cpp | ctor (23, referenced), `Wait(size_t)` (48 referenced, 76 not), `Lock()` (90), `Unlock()` (99), `GetValue() const` (114) |

The messages are unreferenced strings of the same pool (`Falha ao bloquear mutex: {}`, `Falha ao desbloquear
semáforo: {}`, `Falha ao obter valor do semáforo: {}`, …). The EUeIpcError numbering fits: 6150..6152 are the
three CPosixMutex sites, 6153..6158 the six CPosixRWMutex sites, and 6159/6160 are the first two semaphore sites.
So the dead codes are inferred as 6151/6152, 6153..6158 and 6161..6164. The line-76 record shows that
`Wait(size_t)` throws when the wait fails for a reason other than a timeout.

### 2.8 `api::IPower` (`ipower.h`)

Only the five default bodies exist: `BatIntMaxSocValue` (:354, 5172), `SetBatIntMaxSocValue(float)` (:361, 5173),
`BatIntStorageMode()` → `std::tuple<bool,float>` (:409, 5174), `EnableBatIntStorageMode(float)` (:417, 5175), and
`DisableBatIntStorageMode()` (:424, 5176). Each throws `CUeHwilError(..., "Not supported")`. They cover the
internal-battery storage mode and maximum state of charge of newer urna models. `api::teste::CPowerMock` does not
override them (vtable slots 8-12). Nothing calls them in the recorded sessions.

---

## 3. Data read and written

* Files: listed in §2.1. The unit also reads `infomidia.dat` of the MV to get `numeroSerieFV`, and writes the
  encrypted `rdv.dat(.tmp)` (§2.4).
* ASN.1 modules touched: ModuloEstadoGeral{Urna,Vota,Gap,SA}, ModuloEnvelopeGenerico, ModuloHashes,
  ModuloResultadoUrnaCadastro, ModuloRegistroDigitalVoto (`EntidadeRegistroDigitalVoto`, `EntidadeResultadoRDV`),
  ModuloTiposResultadosEcoUrna (`Urna`, `CorrespondenciaResultado`, `Carga`), ModuloProcessoEleitoral,
  ModuloSituacoesEleicoes, ModuloEleicao, ModuloTiposEleitorais (`CabecalhoPacote`), ModuloLocal, and
  ModuloInformacaoMidia (see `src/asn1/`).
* SQL: indirectly, `comparecimento_mesario` through `CRegistradorMesario`'s DAO (u13/u05).

---

## 4. Web-build specifics and wasm/Emscripten observations

* **No pthreads.** `pthread_mutex_lock`/`unlock`, `sem_wait`/`post`/`timedwait` and `pthread_rwlock_*` are
  stubs that return 0 and were inlined away. `CPosixMutex::Lock`/`Unlock` (observed executing) only
  increment/decrement a counter at +28. The semaphore never blocks, and its `GetValue` (10246) always returns 0.
  The bodies whose error path was removed (mutex Lock/Unlock, semaphore Wait/GetValue, the shared empty body
  2219) keep a dead 480-544-byte stack frame, a leftover of the removed `std::format` error path.
* **No Asyncify.** `CWasmThread::Wait` polls with `emscripten_sleep(100)` and `CExecucaoVota::Inicia` calls
  `emscripten_sleep(1000)` (guarded by the byte `g_esperaHabilitada` @1584624 = 1). The glue implements
  `emscripten_sleep` as `abort()`. These paths are dead only because `main` registers
  `CExecucaoVotaCooperativa`.
* **Thread objects are still built.** 316 and 270 run at votaInit. The operator queue fills up but nothing reads
  it (u10).
* **Fixtures.** `infomidia.dat` does not exist in any scenario, so every result file of the simulator has
  `numeroSerieFV = 00000000` (confirmed in `samples/bu-real/run-full/analysis/tse_{bu,rdv}_dump.txt`).
* **Mangled names.** `typeid(T).name()` (for example `N25ModuloRegistroDigitalVoto20EntidadeResultadoRDVE`) is part
  of the error contexts. The build keeps RTTI.
* **Merged bodies.** wasm-opt produced 2919/3892 (CDataMap), 2927/6006 (CFileASN), the three exception-constructor
  thunks 210/1229/1889, which pass a vtable to 2294/710, and most likely 6018 and 6058: each is called only by two
  thunks that differ in one constant (slots 10/15 of `CControladorRegistraMesariosVota` with 11/7; the StartState of
  `CReinicioComparecimentoMesario` / `CFinalizaAquisicao` with 7/10). 6018 keeps an unused first parameter, the
  `this` of the table-referenced methods it was merged from, so it is not a source-level helper. Several functions are named after an inlined callee
  (270/316 after the queue ctor, 3735/3771/3791/5740/5778 after ReadFromFile, 5378 after CDataMap::Update).
  §6 gives the real names.
* **Merged parameters.** Func 5378 takes one argument where the srcloc shows `Update(const KeyType&, const
  RecordType&)`, because its only caller passes the same record as both key and value.

---

## 5. Boletim de Urna (BU): what this unit contributes

The BU is built and signed by other units (`docs/10-boletim-de-urna.md`, `docs/bu/codepath.md`). u18 provides
the following pieces, in the order they happen:

1. **Start of the encerramento (closing).**
   * `CFinalizaAquisicao::StartState` (12113) marks the end of vote acquisition on the voter terminal and posts
     message 10 to the operator.
   * The operator registers the closing mesários. `comum::CEncerraRegistroMesarios::StartState` (func 10380;
     the tools call it `GetControlador`) asks the controller for the phase (slot 6: estadoVota − 55 through a
     table). In phase 3, estadoVota 58 `registromesariofinal`, it sets next = slot 12, calls slot 29, then calls
     **slot 15** (`CControladorRegistraMesariosVota` 10787), which posts **message 7** to the voter terminal.
     In phases 0/1, estadoVota 55 `registromesarioinicial`, it calls slot 10 (10792 → message 11 →
     `CIniciodeCiclo`, the urna opens for voters) instead.
   * On the voter terminal, `CAguardaMensagem::ProcessMessage(7)` sets `estadoVota = GERARBU`, saves `vota.bin`,
     logs `"Inicio do Encerramento"`, and switches to `vota::CGeraBU`.
2. **The `urna` block** of `-bu.dat`, `-rdv.dat`, `-imgbu.dat` and `-imgze.dat` is `comum::md::CUrna`
   (constructors 2854/2855, destructor 1394). It holds:
   * `tipoUrna`: eg.bin DadoCarga, `tipoUrnaT1` when turno is '1', else `tipoUrnaT2`;
   * `versaoVotacao` = `"10.23.0.1 - DESENVOLVIMENTO"`;
   * `correspondenciaResultado` = {município, zona, seção, `CCarga` (copied by 1557), tipo};
   * `tipoArquivo` from the writer;
   * `numeroSerieFV` = hex of the MV serial from `infomidia.dat` (via 3735 and 1243), default `"00000000"`;
   * optional `motivoUtilizacaoSA` when the *sistema de apuração* was used.

   `CUrna::ValidaCriacao` rejects `tipoUrna == '0'` (8689), `tipoArquivo == '0'` (8690), and a serial that is not
   exactly 8 hex digits (8691/8692, `"Serial da MV inválido [...]"`).
3. **`-bu.dat`**: `CGravadorBU::GravaResultado` (11629, other unit) wraps `EntidadeBoletimUrna` in
   `EntidadeEnvelopeGenerico{tipoEnvelope envelopeBoletimUrna}` and writes it with
   `CFileASN::WriteDataToFile<CConversorEnvelopeGenerico>` (thunk **2724** → **6006**). The chain is:
   `CConversorEnvelopeGenerico::DoConverte` (slot 2) → `isValid && isStrictlyValid`, else 7653 → context
   `"WriteToFile de " + <MI work path>` → `isValid`, else 5955 → `encodeBER`, else 5956 → `CFile::RawWrite`.
4. **`-rdv.dat` (result file)**: `CGravadorRDV::GravaResultado` (**11587**, §2 of the comum fragment).
   * `CRdvVota::Converte()` gives the RDV BER. It is decoded again by `DecodeObject<EntidadeRegistroDigitalVoto>`
     (**5825**: 5953/5954 on failure).
   * An `EntidadeResultadoRDV{cabecalho = CCabecalhoEntidade(dataGeração, pleito, 1), urna = CUrna(...), rdv}`
     is built.
   * The correspondência tipo is `'1'` (seção) when the seção number ≠ 0, else `'2'` (contingência).
   * The entity is BER-encoded with context `"CodeObject de N25ModuloRegistroDigitalVoto20EntidadeResultadoRDVE"`
     and written as plain BER. It is **not encrypted and not enveloped**. The encrypted file is only the working
     copy `trab/rdv.dat`.
5. **`-imgbu.dat` / `-imgze.dat`**: `IGravadorEnvelope::GravaResultado` (**11626**) takes the raw bytes of the
   printed image, `trab/bu.dat` or `trab/ze.dat` (slot 8, `CGravadorEnvelopeArquivo::LeConteudo`). It builds
   `CEnvelopeGenerico(cabecalho, tipoEnvelope, urna, município, zona, local, seção, …, conteudo)` (5860) and
   writes it through 2724. Here the correspondência tipo is the eg.bin tipo de urna, not the seção rule of 4.
6. **`-hash.dat`** and **`-jufa.dat`** use the same helper with other converters: 5367 (`CConversorEntidadeHashes`)
   and 5366 (ecourna `CConversorResultadoUrnaCadastro`, EApiAsnError 1900 on an invalid entity).
7. **RDV durability during the vote** (before the BU): the `CEncryptedFile` write/read-back check of §2.4. In the
   web build the RDV is not persisted (`CSincronismoVotoEleitorWeb`), so `Load` never runs in the simulator.

---

## 6. Complete mapping table (95 functions)

"ran" = observed executing during the recorded votes. Files under `src/` are relative to `src/`.

| func | size | ran | tools name | reconstructed symbol | reconstructed in | original file | conf. |
|---|---|---|---|---|---|---|---|
| 210 | 21 |  | api_f210 | `CUeIoError` ctor thunk (`CBaseError<EUeIoError,{5950,6150}>(code, std::string&&, source_location)` → merged 2294) | library/inlined helper (TSE exception ctor thunk) | uenux2/src/api/io/euioerror.h (path inferred) | medium |
| 270 | 353 | ✓ | CPriorityMessageQueue<SMessage>::ctor@270 | `vota::CThreadOperador::GetInst()` (inlines CThread + queue ctor, cmessagequeue.h:113/114) | uenux2/src/app/vota/u18-foreign-fragments.cpp | uenux2/src/app/vota/operador/cthreadoperador.cpp | high |
| 316 | 303 | ✓ | CPriorityMessageQueue<SMessage>::ctor@316 | `vota::CThreadEleitor::GetInst()` | uenux2/src/app/vota/eleitor/cthreadeleitor.cpp (u07) | uenux2/src/app/vota/eleitor/cthreadeleitor.cpp | high |
| 412 | 11 | ✓ | api_f412 | `api::CSystem::IsRegularFile(const std::string&)` = EhDoTipo(path, S_IFREG) (name inferred) | uenux2/src/api/util/csystem.u12-fragment.cpp | uenux2/src/api/util/csystem.cpp | medium |
| 517 | 137 | ✓ | ecourna_f517 | `ecourna::api::io::CFile::CFile(const std::string&, const std::string&, FileMode)` | ecourna/api/io/cfile.cpp (u12) | ecourna-lib/ecourna/api/io/cfile.cpp | high |
| 656 | 14 | ✓ | CDataMap<CEleitorIdentidade,CEleitorDetalhe>::GetCurrent | same (thunk → 2919, value at node+32) | uenux2/src/api/io/cdatamap.h | uenux2/src/api/io/cdatamap.h | high |
| 1200 | 14 |  | CDataMap<unsigned,CCandidatura>::GetCurrent | same (thunk → 2919) | uenux2/src/api/io/cdatamap.h | uenux2/src/api/io/cdatamap.h | high |
| 1229 | 18 |  | api_f1229 | `CUeHwilError` ctor thunk (→ 710 with vtable @1531060) | library/inlined helper (TSE exception ctor thunk) | uenux2/src/api/hwil/euhwilerror.h (path inferred) | medium |
| 1243 | 283 | ✓ | comum_f1243 | `ecourna::api::util::CStringUtils::BytesToHexString(const std::vector<uebyte>&)` (name inferred) | uenux2/src/app/comum/u18-foreign-fragments.cpp | ecourna-lib/ecourna/api/util/cstringutils.cpp | medium |
| 1283 | 14 |  | CDataMap<unsigned short,CPartido>::GetCurrent | same (thunk → 2919) | uenux2/src/api/io/cdatamap.h | uenux2/src/api/io/cdatamap.h | high |
| 1337 | 22 | ✓ | vota_f1337 | `vota::CAguardaMensagem::GetInst()` (→ lazy-singleton helper 764) | uenux2/src/app/vota/u18-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/caguardamensagem.cpp (path inferred) | high |
| 1394 | 179 |  | comum_f1394 | `comum::md::CUrna::~CUrna()` | uenux2/src/app/comum/u18-foreign-fragments.cpp | uenux2/src/app/comum/gravadores/md/curna.cpp | medium |
| 1557 | 325 |  | comum_f1557 | `comum::md::CCarga::CCarga(const CCarga&)` (implicit) | uenux2/src/app/comum/u18-foreign-fragments.cpp | uenux2/src/app/comum/dados/md/correspondencia/ccarga.h | medium |
| 1684 | 116 |  | api::CThread::Start | `api::CThread::Start()` (:68) | uenux2/src/api/ipc/cthread.cpp | uenux2/src/api/ipc/cthread.cpp | high |
| 1889 | 18 |  | api_f1889 | `CUeIpcError` ctor thunk (→ 710 with vtable @1528452) | library/inlined helper (TSE exception ctor thunk) | uenux2/src/api/ipc/euipcerror.h (path inferred) | medium |
| 1900 | 40 |  | vota_f1900 | `api::CThread::Wait()` (name inferred) | uenux2/src/api/ipc/cthread.cpp | uenux2/src/api/ipc/cthread.cpp | medium |
| 2073 | 674 |  | CPriorityMessageQueue<SMessage>::Remove | same (:153) | uenux2/src/api/ipc/cmessagequeue.h | uenux2/src/api/ipc/cmessagequeue.h | high |
| 2721 | 80 |  | api::CThread::vf0 | `api::CThread::~CThread()` | uenux2/src/api/ipc/cthread.cpp | uenux2/src/api/ipc/cthread.cpp | high |
| 2724 | 33 |  | CFileASN::CodeObjectFunction@2724 | `api::CFileASN::WriteDataToFile<comum::asn::CConversorEnvelopeGenerico>` (thunk → 6006; name inferred) | uenux2/src/api/io/asn/cfileasn.instances.cpp | uenux2/src/api/io/asn/cfileasn.h | medium |
| 2766 | 383 |  | api::CEncryptedFile::MemWrite | `MemWrite(const std::vector<uebyte>&)` with `MemWrite(const void*, uedword)` (:142/:145) inlined | uenux2/src/api/io/cencryptedfile.cpp | uenux2/src/api/io/cencryptedfile.cpp | high |
| 2854 | 197 |  | comum_f2854 | `comum::md::CUrna::CUrna(tipo, versão, correspondência, tipoArquivo, serialFV)` | uenux2/src/app/comum/u18-foreign-fragments.cpp | uenux2/src/app/comum/gravadores/md/curna.cpp | medium |
| 2855 | 206 |  | comum_f2855 | `comum::md::CUrna::CUrna(..., motivoUtilizacaoSA)` | uenux2/src/app/comum/u18-foreign-fragments.cpp | uenux2/src/app/comum/gravadores/md/curna.cpp | medium |
| 2919 | 527 |  | api_f2919 | `api::CDataMap<K,R>::GetCurrent()` merged body (this, value offset, srcloc) | uenux2/src/api/io/cdatamap.h | uenux2/src/api/io/cdatamap.h | high |
| 2927 | 1275 | ✓ | api_f2927 | `api::CFileASN::CodeObjectFunction<T>` merged body (state files) | uenux2/src/api/io/asn/cfileasn.h | uenux2/src/api/io/asn/cfileasn.h | high |
| 3125 | 14 |  | CDataMap<unsigned,CRespostaConsulta>::GetCurrent | same (thunk → 2919) | uenux2/src/api/io/cdatamap.h | uenux2/src/api/io/cdatamap.h | high |
| 3598 | 1956 | ✓ | api::CThread::CThread | `api::CThread::CThread()` (:31/:32; CPolySingletonList::instance inlined) | uenux2/src/api/ipc/cthread.cpp | uenux2/src/api/ipc/cthread.cpp | high |
| 3652 | 514 |  | api::CEncryptedFile::MemRead | `MemRead(std::vector<uebyte>&)` with `MemRead(void*, uedword)` (:161-167) inlined | uenux2/src/api/io/cencryptedfile.cpp | uenux2/src/api/io/cencryptedfile.cpp | high |
| 3653 | 3845 |  | api::CEncryptedFile::Load | `api::CEncryptedFile::Load(const std::string&)` (:36-73) | uenux2/src/api/io/cencryptedfile.cpp | uenux2/src/api/io/cencryptedfile.cpp | high |
| 3697 | 12 |  | CDataMap<unsigned short,CPartido>::Next | same (thunk → 3892) | uenux2/src/api/io/cdatamap.h | uenux2/src/api/io/cdatamap.h | high |
| 3735 | 3405 |  | CFileASN::ReadFromFile@3735 | `api::CFileASN::ReadDataFromFile<ecourna::app::dados::asn::CConversorInformacaoMidia>(const std::string&)` (name inferred) | uenux2/src/api/io/asn/cfileasn.instances.cpp | uenux2/src/api/io/asn/cfileasn.h | medium |
| 3771 | 6814 | ✓ | CFileASN::ReadFromFile@3771 | `comum::asn::LeProcessoEleitoral(const CEstadoGeral&)` (name as u04, inferred) | uenux2/src/app/comum/u18-foreign-fragments.cpp | uenux2/src/app/comum/dados/cpe.cpp (path inferred) | medium |
| 3791 | 3956 | ✓ | CFileASN::ReadFromFile@3791 | `comum::IServicoEstado<CEstadoGeral, CConversorEstadoGeral>::Carrega()` (name inferred) | uenux2/src/app/comum/u18-foreign-fragments.cpp | uenux2/src/app/comum/appinfo/servicos/iservicoestado.h (path inferred) | medium |
| 3892 | 577 |  | api_f3892 | `api::CDataMap<K,R>::Next()` merged body | uenux2/src/api/io/cdatamap.h | uenux2/src/api/io/cdatamap.h | high |
| 4713 | 89 | ✓ | vota::CExecucaoVotaCooperativa::Inicia (formerly shown by the tools as `vf2`) | `CExecucaoVotaCooperativa::Inicia/Executa` (slots 2 and 3; names inferred) | uenux2/src/app/vota/u18-foreign-fragments.cpp | uenux2/mock/app/vota/cexecucaovotacooperativa.cpp (path inferred, u39; component app:mock) | medium |
| 5353 | 14 |  | api::CPosixRWMutex::vf0 | `api::CPosixRWMutex::~CPosixRWMutex()` | uenux2/src/api/ipc/posix/cposixrwmutex.cpp | uenux2/src/api/ipc/posix/cposixrwmutex.cpp | high |
| 5354 | 14 |  | api::CPosixSemaphore::vf0 | `api::CPosixSemaphore::~CPosixSemaphore()` | uenux2/src/api/ipc/posix/cposixsemaphore.cpp | uenux2/src/api/ipc/posix/cposixsemaphore.cpp | high |
| 5355 | 14 |  | api::CPosixMutex::vf0 | `api::CPosixMutex::~CPosixMutex()` | uenux2/src/api/ipc/posix/cposixmutex.cpp | uenux2/src/api/ipc/posix/cposixmutex.cpp | high |
| 5366 | 1244 |  | CFileASN::CodeObjectFunction@5366 | `api::CFileASN::WriteDataToFile<ecourna CConversorResultadoUrnaCadastro>` (name inferred) | uenux2/src/api/io/asn/cfileasn.instances.cpp | uenux2/src/api/io/asn/cfileasn.h | medium |
| 5367 | 33 |  | CFileASN::CodeObjectFunction@5367 | `api::CFileASN::WriteDataToFile<comum::asn::CConversorEntidadeHashes>` (thunk → 6006) | uenux2/src/api/io/asn/cfileasn.instances.cpp | uenux2/src/api/io/asn/cfileasn.h | medium |
| 5377 | 1492 |  | CDataMap<PK,CComparecimentoMesario>::Add | `CDataMap<...>::Add(const KeyType&, const RecordType&)` + inlined `Add(value_type)` (:143) | uenux2/src/api/io/cdatamap.h | uenux2/src/api/io/cdatamap.h | high |
| 5378 | 1239 |  | CDataMap<PK,CComparecimentoMesario>::Update | `comum::CRegistradorMesario::AtualizaInternal(const md::CComparecimentoMesario&)` (name inferred; inlines Update :158) | uenux2/src/app/comum/u18-foreign-fragments.cpp | uenux2/src/app/comum/comparecimentomesario/cregistradormesario.cpp | medium |
| 5483 | 256 |  | api::CIniKey::operator= | same (:24) | uenux2/src/api/io/cinikey.cpp | uenux2/src/api/io/cinikey.cpp | high |
| 5601 | 12 |  | CDataMap<CEleitorIdentidade,CEleitorDetalhe>::Next | same (thunk → 3892) | uenux2/src/api/io/cdatamap.h | uenux2/src/api/io/cdatamap.h | high |
| 5611 | 12 |  | CDataMap<unsigned,CCandidatura>::Next | same (thunk → 3892) | uenux2/src/api/io/cdatamap.h | uenux2/src/api/io/cdatamap.h | high |
| 5706 | 3258 | ✓ | CFileASN::ReadFromFile@5706 | `api::CFileASN::ReadDataFromFile<comum::asn::CConversorCabecalhoPacote>(const std::string&)` (name inferred) | uenux2/src/api/io/asn/cfileasn.instances.cpp | uenux2/src/api/io/asn/cfileasn.h | medium |
| 5725 | 1085 |  | CDataMap<CNumeroInscricaoEleitoral,CJustificadorDetalhe>::Add | same, `Add(const value_type&)` (:143) | uenux2/src/api/io/cdatamap.h | uenux2/src/api/io/cdatamap.h | high |
| 5740 | 5775 | ✓ | CFileASN::ReadFromFile@5740 | `comum::CLocal::Carrega()` (name inferred) | uenux2/src/app/comum/u18-foreign-fragments.cpp | uenux2/src/app/comum/dados/clocal.cpp | medium |
| 5778 | 10735 | ✓ | CFileASN::ReadFromFile@5778 | `comum::asn::LePleito(fase, uf, dir, const CPleitoDTO&)` (name inferred) | uenux2/src/app/comum/u18-foreign-fragments.cpp | uenux2/src/app/comum/dados/cpe.cpp (path inferred) | low |
| 5825 | 1308 |  | api::CFileASN::DecodeObjectFunction | `api::CFileASN::DecodeObject<EntidadeRegistroDigitalVoto>` (DecodeObjectFunction :135/:143 inlined) | uenux2/src/api/io/asn/cfileasn.instances.cpp | uenux2/src/api/io/asn/cfileasn.h | high |
| 6006 | 1651 |  | api_f6006 | `api::CFileASN::WriteDataToFile<TConversor>` merged body (comum converters) | uenux2/src/api/io/asn/cfileasn.h | uenux2/src/api/io/asn/cfileasn.h | medium |
| 6018 | 52 |  | vota_f6018 | merged body (wasm-opt merge-similar) of `CControladorRegistraMesariosVota` slots 10/15: `(this /*unused*/, id)` → post `{id, &fila}` to the voter queue. Label `CControladorRegistraMesariosVota::EnviaMensagemEleitor`; probably no source function | inline in slots 10/15, uenux2/src/app/vota/u18-foreign-fragments.cpp | uenux2/src/app/vota/operador/comparecimentomesario/ccontroladorregistramesariosvota.cpp (path inferred) | low |
| 6058 | 60 |  | vota_f6058 | merged body (wasm-opt merge-similar) of `CReinicioComparecimentoMesario::StartState` / `CFinalizaAquisicao::StartState`: `(state, id)` → next = CAguardaMensagem, post `{id, &fila}` to the operator queue. Label `vota::AguardaOperador`; probably no source function | inline in 11917/12113, uenux2/src/app/vota/u18-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/ (path inferred) | low |
| 7221 | 85 |  | vota::CMostraTelaContinuaVotacao::vf2 | `CMostraTelaContinuaVotacao::StartState()` | uenux2/src/app/vota/u18-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/cmostratelacontinuavotacao.cpp (path inferred) | high |
| 8074 | 50 |  | api::IPower::DisableBatIntStorageMode | same (:424) | uenux2/src/api/hwil/ipower.h | uenux2/src/api/hwil/ipower.h | high |
| 8083 | 50 |  | api::IPower::EnableBatIntStorageMode | same (:417) | uenux2/src/api/hwil/ipower.h | uenux2/src/api/hwil/ipower.h | high |
| 8091 | 50 |  | api::IPower::BatIntStorageMode | same (:409) | uenux2/src/api/hwil/ipower.h | uenux2/src/api/hwil/ipower.h | high |
| 8098 | 50 |  | api::IPower::SetBatIntMaxSocValue | same (:361) | uenux2/src/api/hwil/ipower.h | uenux2/src/api/hwil/ipower.h | high |
| 8105 | 50 |  | api::IPower::BatIntMaxSocValue | same (:354) | uenux2/src/api/hwil/ipower.h | uenux2/src/api/hwil/ipower.h | high |
| 9257 | 9 |  | ecourna_f9257 | thunk onto 1243 (second overload merged; `CStringUtils::BytesToHexString`) | uenux2/src/app/comum/u18-foreign-fragments.cpp | ecourna-lib/ecourna/api/util/cstringutils.cpp | low |
| 9673 | 21 | ✓ | CFileASN::CodeObjectFunction@9673 | `CFileASN::CodeObjectFunction<ModuloEstadoGeralVota::EstadoGeralVota>` (thunk → 2927) | uenux2/src/api/io/asn/cfileasn.instances.cpp | uenux2/src/api/io/asn/cfileasn.h | high |
| 9695 | 21 |  | CFileASN::CodeObjectFunction@9695 | `...<ModuloEstadoGeralSA::EstadoGeralSA>` | uenux2/src/api/io/asn/cfileasn.instances.cpp | uenux2/src/api/io/asn/cfileasn.h | high |
| 9702 | 21 | ✓ | CFileASN::CodeObjectFunction@9702 | `...<ModuloEstadoGeralGap::EstadoGeralGap>` | uenux2/src/api/io/asn/cfileasn.instances.cpp | uenux2/src/api/io/asn/cfileasn.h | high |
| 9790 | 130 |  | api_f9790 | `std::__format_arg_store<format_context, const string&, string&>` ctor (make_format_args) | library/inlined helper (libc++ `<format>`) | libc++ | medium |
| 9801 | 21 | ✓ | CFileASN::CodeObjectFunction@9801 | `...<ModuloEstadoGeralUrna::EstadoGeralUrna>` | uenux2/src/api/io/asn/cfileasn.instances.cpp | uenux2/src/api/io/asn/cfileasn.h | high |
| 10234 | 20 |  | vota::CExecucaoVota::vf4 | `vota::CExecucaoVota::Aguarda()` (name inferred) | uenux2/src/app/vota/u18-foreign-fragments.cpp | uenux2/src/app/vota/cexecucaovota.cpp (path inferred) | medium |
| 10235 | 39 |  | vota::CExecucaoVota::vf3 | `vota::CExecucaoVota::Inicia()` (name inferred) | uenux2/src/app/vota/u18-foreign-fragments.cpp | uenux2/src/app/vota/cexecucaovota.cpp (path inferred) | medium |
| 10236 | 57 |  | vota::CExecucaoVota::vf2 | `vota::CExecucaoVota::Executa()` (name inferred) | uenux2/src/app/vota/u18-foreign-fragments.cpp | uenux2/src/app/vota/cexecucaovota.cpp (path inferred) | medium |
| 10244 | 13 |  | api::CPosixRWMutex::vf1 | deleting destructor | uenux2/src/api/ipc/posix/cposixrwmutex.cpp | uenux2/src/api/ipc/posix/cposixrwmutex.cpp | high |
| 10245 | 42 |  | api::CPosixRWMutex::CPosixRWMutex | same (unreferenced srclocs :29/:35/:42 of its removed error paths) | uenux2/src/api/ipc/posix/cposixrwmutex.cpp | uenux2/src/api/ipc/posix/cposixrwmutex.cpp | high |
| 10246 | 44 |  | api::CPosixSemaphore::vf6 | `CPosixSemaphore::GetValue() const` (name from the unreferenced srcloc record :114; slot by elimination) | uenux2/src/api/ipc/posix/cposixsemaphore.cpp | uenux2/src/api/ipc/posix/cposixsemaphore.cpp | medium |
| 10247 | 623 |  | api::CPosixSemaphore::Wait | `Wait(size_t)` (:48; the :76 throw was removed) | uenux2/src/api/ipc/posix/cposixsemaphore.cpp | uenux2/src/api/ipc/posix/cposixsemaphore.cpp | high |
| 10248 | 15 |  | api::CPosixSemaphore::vf4 | `CPosixSemaphore::Wait()` → slot 2 (name inferred) | uenux2/src/api/ipc/posix/cposixsemaphore.cpp | uenux2/src/api/ipc/posix/cposixsemaphore.cpp | low |
| 10249 | 13 |  | api::CPosixSemaphore::vf1 | deleting destructor | uenux2/src/api/ipc/posix/cposixsemaphore.cpp | uenux2/src/api/ipc/posix/cposixsemaphore.cpp | high |
| 10250 | 552 |  | api::CPosixSemaphore::CPosixSemaphore | `CPosixSemaphore(unsigned)` (:23) | uenux2/src/api/ipc/posix/cposixsemaphore.cpp | uenux2/src/api/ipc/posix/cposixsemaphore.cpp | high |
| 10251 | 35 | ✓ | api::CPosixMutex::vf3 | `CPosixMutex::Unlock()` (name from the unreferenced srcloc record :56) | uenux2/src/api/ipc/posix/cposixmutex.cpp | uenux2/src/api/ipc/posix/cposixmutex.cpp | high |
| 10252 | 35 | ✓ | api::CPosixMutex::vf2 | `CPosixMutex::Lock()` (name from the unreferenced srcloc record :47) | uenux2/src/api/ipc/posix/cposixmutex.cpp | uenux2/src/api/ipc/posix/cposixmutex.cpp | high |
| 10253 | 13 |  | api::CPosixMutex::vf1 | deleting destructor | uenux2/src/api/ipc/posix/cposixmutex.cpp | uenux2/src/api/ipc/posix/cposixmutex.cpp | high |
| 10254 | 505 |  | api::CPosixMutex::CPosixMutex | same (:30) | uenux2/src/api/ipc/posix/cposixmutex.cpp | uenux2/src/api/ipc/posix/cposixmutex.cpp | high |
| 10728 | 411 |  | vota_f10728 | text source `std::format("{:<12s}", CThreadOperador +120)` (name inferred) | uenux2/src/app/vota/u18-foreign-fragments.cpp | uenux2/src/app/vota/operador/ (CTituloEncerramentoInvalido screen; path inferred) | low |
| 10763 | 59 |  | vota::CRegistroMesarioEncerrado::vf2 | `CRegistroMesarioEncerrado::StartState()` | uenux2/src/app/vota/u18-foreign-fragments.cpp | uenux2/src/app/vota/operador/comparecimentomesario/cregistromesarioencerrado.cpp (path inferred) | medium |
| 10785 | 116 |  | CControladorRegistraMesariosVota::vf18 | `SetTituloDigitado(const std::string&)` (name inferred) | uenux2/src/app/vota/u18-foreign-fragments.cpp | .../ccontroladorregistramesariosvota.cpp (path inferred) | low |
| 10787 | 9 |  | CControladorRegistraMesariosVota::vf15 | `IniciaEncerramento()` → voter msg 7 (name inferred) | uenux2/src/app/vota/u18-foreign-fragments.cpp | .../ccontroladorregistramesariosvota.cpp (path inferred) | low |
| 10788 | 9 |  | CControladorRegistraMesariosVota::vf17 | `GetTituloDigitado()` (name inferred) | uenux2/src/app/vota/u18-foreign-fragments.cpp | .../ccontroladorregistramesariosvota.cpp (path inferred) | low |
| 10792 | 9 |  | CControladorRegistraMesariosVota::vf10 | `LiberaTerminalEleitor()` → voter msg 11 (name inferred) | uenux2/src/app/vota/u18-foreign-fragments.cpp | .../ccontroladorregistramesariosvota.cpp (path inferred) | low |
| 11430 | 564 |  | api::CFileSeeder::skip | same (:106) | uenux2/src/api/io/asn/cpartialfileasn.h | uenux2/src/api/io/asn/cpartialfileasn.h | high |
| 11431 | 1082 | ✓ | api::CFileSeeder::seek | same (:88/:94) | uenux2/src/api/io/asn/cpartialfileasn.h | uenux2/src/api/io/asn/cpartialfileasn.h | high |
| 11432 | 578 | ✓ | api::CFileSeeder::decodeBlock | same (:75) | uenux2/src/api/io/asn/cpartialfileasn.h | uenux2/src/api/io/asn/cpartialfileasn.h | high |
| 11433 | 552 | ✓ | api::CFileSeeder::decodeByte | same (:52) | uenux2/src/api/io/asn/cpartialfileasn.h | uenux2/src/api/io/asn/cpartialfileasn.h | high |
| 11587 | 2707 |  | comum::CGravadorRDV::vf7 | `comum::CGravadorRDV::GravaResultado(api::CFile&) const` | uenux2/src/app/comum/u18-foreign-fragments.cpp | uenux2/src/app/comum/gravadores/cgravadorrdv.cpp (path inferred) | high |
| 11626 | 1508 |  | comum::IGravadorEnvelope::vf7 | `comum::IGravadorEnvelope::GravaResultado(api::CFile&) const` | uenux2/src/app/comum/u18-foreign-fragments.cpp | uenux2/src/app/comum/gravadores/igravadorenvelope.cpp (path inferred) | high |
| 11794 | 36 |  | vota::CUrnaInspecionada::vf6 | `CUrnaInspecionada::ProcessMessage(int)` | uenux2/src/app/vota/u18-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/curnainspecionada.cpp (path inferred) | medium |
| 11917 | 9 |  | vota::CReinicioComparecimentoMesario::vf2 | `CReinicioComparecimentoMesario::StartState()` | uenux2/src/app/vota/u18-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/creiniciocomparecimentomesario.cpp (path inferred) | medium |
| 11994 | 118 |  | vota::CDefineRotaPreVotacao::vf3 | `CDefineRotaPreVotacao::NeedChangeState()` | uenux2/src/app/vota/u18-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/iniciovotacao/cdefinerotaprevotacao.cpp (path inferred) | medium |
| 12113 | 9 |  | vota::CFinalizaAquisicao::vf2 | `CFinalizaAquisicao::StartState()` | uenux2/src/app/vota/u18-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/cfinalizaaquisicao.cpp (path inferred) | medium |
| 12621 | 442 |  | api::CDataText<CRespostasDSNumero>::vf2 | `api::CDataText<comum::CRespostasDSNumero>::GetText()` (`"{:0{}}"`, CRespostas::GetInst inlined) | uenux2/src/app/comum/u18-foreign-fragments.cpp | uenux2/src/app/comum/dados/crespostas.cpp area (path inferred) | medium |

Functions outside the unit that were reconstructed in u18's files for completeness: 10255 `CThread::ThreadProc`,
rhvoice_f501 `CPriorityMessageQueue::Add`, 4343/4337 `Recebe`, 3162/3157 queue destructors, 11434/11429/11428
(u12's CFileSeeder slots), 2891/2892 (u12's `WriteToFile`).

---

## 7. Weird or risky code

1. **`CEncryptedFile::Load` never throws three of its checks (func 3653).** At cencryptedfile.cpp:44 (file size 0
   or not a multiple of 16), :51 (short read) and :73 (a padding byte ≠ pad length), the code builds a
   `CBaseError<EUeIoError>` in a stack temporary (`ecourna_f1143` with code 5989/5990/5993) and destroys it
   immediately. There is no `__cxa_allocate_exception` and no `__cxa_throw` (checked in the WAT). In C++ this is
   `CUeIoError(...);` without `throw`. The other four checks in the same function do throw. As a result a
   truncated or badly padded encrypted RDV is not rejected at this layer. A short read leaves zeros in the buffer
   that is decrypted. The size-multiple-of-16 and secondary-padding tests have no effect. A 0-byte file is still
   refused, but only because `CFile::RawRead` throws `CIoError` 1192 (`buffer invalido`) for the null buffer of the
   empty vector. Other safeguards remain: the pad > 16 check, the cipher's own checks (not verified here), the RDV
   BER decode, and the read-back comparison `ConfereConteudo`. Same code on the urna (it is not a web mock).
2. **`claro.back()` without an emptiness check (3653).** If the decryption returns an empty vector, the pad byte
   is read from before the buffer (in wasm, a null data pointer gives address 0xFFFFFFFF and a trap). An empty
   input file cannot get there (see 1), so this needs a cipher that returns nothing for non-empty input. The
   `pad > tamanho` check compares with the encrypted file size, not the decrypted size, and pad 0 is accepted.
3. **`CPriorityMessageQueue::Remove` throws with the lock held (2073).** The lock is taken with an explicit
   `Lock()`. On an empty queue the function throws 6228 without `Unlock()`: there is no cleanup code on that path.
   The mutex is recursive, so the same thread can continue, but any other thread that needs the queue would block
   forever. (In the web build the "mutex" is only a counter, so nothing blocks; the counter just stays one too
   high.) Callers normally test `Vazia()` first. `Recebe` (4343) relies on the semaphore instead. In the web
   build the semaphore is a no-op that always reports success, so `Recebe` on an empty queue would hit this path
   (dead code in the simulator: the operator thread never runs).
4. **Semaphores and mutexes do nothing in the web build.** `sem_*` and `pthread_*` are stubs. `CPosixSemaphore::Wait`
   always returns "signalled", `GetValue` returns 0, and `CPosixMutex` only counts. This is harmless while only
   `votaTick` runs, but any code that uses these primitives to wait would not wait.
5. **`emscripten_sleep` paths abort (no Asyncify).** `CExecucaoVota::Inicia`/`Executa` (10235/10236) call
   `emscripten_sleep(1000)`, and `CExecucaoVota::Aguarda` → `CThread::Wait` → `CWasmThread::Wait` polls with
   `emscripten_sleep(100)`. The glue turns both into `abort("Please compile your program with async support…")`.
   They are unreachable only because `main` registers `CExecucaoVotaCooperativa`: the `IExecucaoVota` getter
   (3594, iexecucaovota.cpp:74) creates a `CExecucaoVota` by default when no policy is registered. A change of
   policy or a direct call would crash the simulator.
6. **`CFileSeeder::decodeByte` can return an uninitialised byte (11433).** `fim()` is stdio `feof`, which
   becomes true only after a read has failed. At the exact end of the file `RawRead` returns 0 without throwing
   (cfile.cpp), and `decodeByte` returns the uninitialised stack byte instead of throwing 5957. A truncated
   `-el.dat` could feed one garbage byte to the partial BER decoder. The length checks of the decoder normally
   catch truncation first. Same code on the urna.
7. **`CFileASN::ReadFromFile` ignores the `RawRead` result.** `CFile::RawRead` returns a short count without
   throwing, so a short read decodes a zero-padded buffer. Usually the decode then fails as "não foi decodificado"
   (the message points at the content instead of the I/O). If the missing bytes fall inside a primitive value
   (an OCTET STRING, for example), the zeros can be accepted as content. A short read needs an I/O error or a file
   that shrinks between the size probe and the read; not observed.
8. **The correspondência type is computed in two different ways.** `CGravadorRDV` (11587) derives the
   correspondência's tipo from `seção != 0 ? '1' : '2'`. `IGravadorEnvelope` (11626) uses eg.bin's tipo de urna
   for the same field. For a contingency urna with a section number these could differ between `-rdv.dat` and
   `-imgbu.dat`. The code difference is certain. Whether it changes the encoded files was not checked: that depends
   on how `CConversorUrna` (10287) maps the two value sets. Not observed. Low.
9. **`numeroSerieFV` is always `00000000` in the simulator.** `infomidia.dat` is looked up only under
   `/dsk/fe/estatico/`, and no scenario ships it. The value is silently defaulted, not reported. Every simulated
   BU/RDV/envelope carries the default serial. Info (it is expected for a fixture, but a reader of the files
   should know it is not a real serial).

---

## 8. Open questions

* The exact names of the converter-level `CFileASN` helpers (`ReadDataFromFile`/`WriteDataToFile` here), of the
  `IExecucaoVota` and `IControladorRegistraMesarios` slots, and of `CThreadOperador` +84/+96/+120 are inferred.
* `SMessage` +4 is always written with the destination queue's address. We have not established whether that is
  a real member (sender/destination) or an optimiser artefact of an uninitialised one.
* `md::CUrna` +108 (a bool set to true by both constructors, never tested by `ValidaCriacao`).
* `CPosixSemaphore` slot 4 (10248, calls slot 2 = `Lock`): no srcloc names it; `Wait()` is a guess. The error codes
  and messages of the removed POSIX error paths (§2.7) are inferred from the numbering and the string pool.
* The operator-side meaning of messages 7, 10 and 12 (consumed by `CThreadOperador` states, u10) is not traced here.
