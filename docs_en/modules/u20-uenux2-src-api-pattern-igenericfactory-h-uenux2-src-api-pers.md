# u20 — the `api` foundation (util, persistência, uelog, pattern) and `comum::CAppInfo`, the urna's state cache

Unit u20 is the plumbing under the VOTA application (the urna's election-day program). It holds 109 wasm
functions that the tools attributed to 21 original files. Most of them are small, widely shared helpers:

| area | original files | what it does |
|---|---|---|
| **`api/util`** | `csystem.cpp`, `cdirreader.cpp`, `cdate.cpp`, `ctime.cpp`, `cdatetime.cpp`, `cgenerictags.cpp`, `cstringutils.cpp`, `csynchronizer.cpp`, `ctickmanager.cpp`, `cwait.cpp`, `itimerscheduler.{h,cpp}` | file-system helpers (copy with `O_SYNC`, atomic replace with backup, "zero-fill" before deletion), the urna's date/time value types, a textual TLV codec for the signing service, the version-number parser, the write-synchronisation switch, the per-thread timers ("ticks"), the busy-wait deadline and the GUI timer service |
| **`api/persistencia`** | `cdaorepositorio.hpp`, `iuenuxgenericdao.h` | the registry that hands out DAOs (*Data Access Objects*) over the SQLite database `uenux.db`, and the default "not implemented" DAO operations |
| **`api/uelog`** | `cloga.cpp`, `cescritorlog.cpp` | the application log front-end (`CLoga::loga` → `CEscritorLog` → the logd writer; `simulador::CWasmLogd` in the web build) |
| **`api/pattern`** | `igenericfactory.h` | the abstract factory of OS primitives (semaphore, mutex, rw-lock, thread) |
| **`comum/appinfo`** | `cappinfo.cpp`, `servicos/cservicoestadogeral{sa,gap,vota}.cpp` | `CAppInfo`, the in-memory cache of the state files `eg.bin`, `gap.bin`, `vota.bin`, `sa.bin`, its getters (`GetGeral`, `GetVota`, `TemVota`, `IndiceTurno`…), the writer of `vota.bin` on both flash memories (`SalvaEstado`) and the path of each state file |

The unit also received **26 functions of other files**, which the tools grouped here because they inline
or call the helpers above. They are the header of the **BU QR codes**
(`CGeradorBUQRCodeVota::PreencheCabecalho`), the mesário-attendance cache and its DAO
(`CRegistradorMesario`, `CJustificador`), the SAVD "sign/verify file" request (`IInterfaceSavd`), two
start-up log lines, and 13 `vota` functions: the states of the zerésima, mesário registration, end of
voting and BU flows, and two singleton getters. Four more free functions (`SalvaEstado`, the MV signature
copy and two mode predicates) have no source file of their own; they are kept in `cappinfo.cpp`.

31 of the 109 functions ran during the recorded votes (`analysis/runtime/*.functions.tsv`): the `CAppInfo`
getters, `SalvaEstado`, `CopyFile`, `ZeroFill`, dates and times, the tick manager, the log writer, the SAVD
TLV codec and the two start-up log lines. The latter are the first two records of `logd.dat` in the MEMFS
snapshot: `1|1|Iniciando aplicação - 1º turno` and `1|1|Versão da aplicação: 10.23.0.1 - DESENVOLVIMENTO`.

**Reconstructed sources** (`// wasm func N` on every function, `// name inferred` / `// ?` where applicable):

```
src/uenux2/src/api/pattern/igenericfactory.h
src/uenux2/src/api/persistencia/cdaorepositorio.hpp, iuenuxgenericdao.h
src/uenux2/src/api/uelog/cloga.cpp, cescritorlog.cpp
src/uenux2/src/api/util/csystem.cpp          (+ existing fragment csystem.u12-fragment.cpp)
src/uenux2/src/api/util/cdirreader.{h,cpp}
src/uenux2/src/api/util/cdate.cpp            (+ fragments cdate.u13.cpp, u05-foreign-fragments.cpp)
src/uenux2/src/api/util/ctime.cpp, cdatetime.cpp (+ cdatetime.u02.cpp)
src/uenux2/src/api/util/cgenerictags.{h,cpp}, cstringutils.cpp, csynchronizer.cpp
src/uenux2/src/api/util/ctickmanager.cpp, cwait.cpp (+ cwait.u09.cpp), itimerscheduler.{h,cpp}
src/uenux2/src/app/comum/appinfo/cappinfo.{h,cpp}
src/uenux2/src/app/comum/appinfo/servicos/cservicoestadogeral{sa,gap,vota}.cpp
src/uenux2/src/app/comum/u20-foreign-fragments.cpp   (QR header, CRegistradorMesario, CJustificador,
                                                      mesário-count data source, SAVD TLV, IEventosLog)
src/uenux2/src/app/vota/u20-foreign-fragments.cpp    (13 vota functions, see §6)
```

---

## 1. Glossary

| term | meaning |
|---|---|
| **MI / MV** | *mídia interna* (internal flash, `/dsk/fi`) / *mídia de votação* (external flash card, `/dsk/fe`). Every dynamic file exists on both. |
| **trab1 / trab2** | per-turno work directory `dinamico/trab<turno>/` holding `vota.bin`, `rdv.dat`, `uenux.db` and their `.vsu` signatures |
| **estado geral** | the persistent state files: `eg.bin` (urna), `gap.bin` (*GAP*, the application launcher), `vota.bin` (VOTA), `sa.bin` (*SA*, *Sistema de Apuração*, the counting application) |
| **turno** | election round. In code `EUrnaTurno`: `'0'` sem turno, `'1'`, `'2'`, `'3'` = "turno atual" (resolved through `CEstadoGeral`) |
| **fase** | `'1'` oficial, `'2'` simulado, `'3'` treinamento (training) |
| **treinamento do eleitor / do mesário** | voter training vs poll-worker training (both fase `'3'`; `EstadoGeralVota.treinamentoEleitor` tells them apart). The simulator runs in *treinamento do eleitor*. |
| **SAVD** | the urna's signing/validation service (`comum::IInterfaceSavd`) |
| **`.vsu` package** | the signature file of one data file (`vota.vsu`, `rdv.vsu`, `uenux.vsu`…); `CArquivosSavd` ids 122–125 = `vota.vsu` (MI/MV × turno) |
| **DAO** | *Data Access Object* over SQLite `uenux.db` (tables `comparecimento_mesario`, `registro_justificativa`, `eleitor_dinamico`) |
| **comparecimento de mesários** | attendance of the poll workers, registered by voter-id at opening (*período* 1) and at closing |
| **zerésima** | report printed before voting opens, proving every candidate has zero votes; its summary is `rze.dat` |
| **tick** | a periodic timer of a thread's state machine (`CTickManager`) |
| **logd** | the urna's log daemon; records `app|severity|text` in `log/logd.dat` (Latin-1) |

---

## 2. Classes and how they relate

RTTI (typeinfo / vtable addresses) of the polymorphic classes of the unit:

```
api::IGenericFactory<AP>                    (abstract, vtable [0,1] dtors [2] Create() [3] Create(string) [4] Create(size_t))
 ├─ CDefaultGenericFactory<ISemaphore,  CPosixSemaphore>        typeinfo 1527096, vtable @1527076
 ├─ CDefaultGenericFactory<IRWSyncCtl,  CPosixRWMutex>          typeinfo 1527016
 ├─ CDefaultGenericFactory<ISyncCtl,    CPosixMutex>            typeinfo 1526936
 └─ CDefaultGenericFactory<IThreadImpl, simulador::CWasmThread> typeinfo 1526856   (web build: cooperative threads)

api::persistencia::IDAO                     (typeinfo 1558992; slot 2 = Clonar())
 └─ IUenuxGenericDAO<DADO, ID>              (slots 4 Excluir, 5 ExcluirID, 6 Atualizar; defaults throw)
     ├─ comum::dao::IComparecimentoMesarioDAO ── CComparecimentoMesarioDAO  (vtable @1593360)
     ├─ comum::dao::IJustificadorDAO          ── CJustificadorDAO           (vtable @1560768)
     └─ comum::dao::IEleitorDinamicoDAO       ── CEleitorDinamicoDAO        (vtable @1558904)

api::CEscritorLog                           (typeinfo 1599616, abstract: no vtable of its own)
 └─ simulador::CWasmLogd                    (vtable @1530808: [2] fazOperacao 10262 [3] loga 10260 [4] Escreve 8327)

api::ITimerScheduler ── api::CTimerScheduler (vtable @1585404, 4 bytes)

comum::IServicoEstado<ESTADO, CONVERSOR>
 ├─ CServicoEstadoGeral      (eg.bin)       vtable @1558024
 ├─ CServicoEstadoGeralVota  (vota.bin)     vtable @1558064  [2] GetPathArquivo = 11568
 ├─ CServicoEstadoGeralGap   (gap.bin)      vtable @1558120  [2] GetPathArquivo = 11567
 └─ CServicoEstadoGeralSA    (sa.bin)       vtable @1558176  [2] GetPathArquivo = 11565

comum::servico::CComparecimentoMesarioServico (vtable @1595676: [0] 3607 [1] 10312)
comum::servico::CJustificadorServico          (vtable @1560936)
comum::CGeradorBUQRCode (abstract) ── CGeradorBUQRCodeVota (vtable @1575984: [2] PreencheCabecalho = 11242)
api::CDataTextFmt<comum::(anon)::CComparecimentoMesariosDS> ×3 (vtables @1594036/@1594188/@1594340: [2] Text)
std::__function::__func<CSystem::ZeroFill(...)::$_0, void()>  (vtable @1585036; operator() = 10860 = close(fd))
```

Non-polymorphic value / helper classes, with the layouts recovered:

| class | size | layout |
|---|---|---|
| `comum::CAppInfo` | 532 | +0 `optional<CEstadoGeral>` (flag +180) · +184 `array<optional<CEstadoGeralGap>,2>` (48 B each) · +280 `array<optional<CEstadoGeralVota>,2>` (104 B each) · +488..532 not used here. Singleton `unique_ptr` @1838664 (getter func 185) |
| `CEstadoGeralVota` (as used here) | 100 | +0 estadoVota · +12/+16 `optional<uint32> urnaIdGerouZeresima` · +20/+32 `optional<CDateTime> dhIniAquisicao` · +36/+48 `dhFimAquisicao` · +72 treinamentoEleitor · +80/+92 `dhEmissao` · +96 tecladoTestadoPosConversao |
| `CEstadoGeral` (as used here) | 180 | +20 município (QR MUNI) · +32 turno · +36/+40 tipoUrna T1/T2 · +48 fase · +60 `CDadoCorrespondencia` (its first int = id of the urna) |
| `api::CDate` | 8 | 4 × `ueword`: dia, mes, ano, diaSemana |
| `api::CTime` | 4 | seconds since midnight |
| `api::CDateTime` | 12 | `CDate` + `CTime` |
| `api::CDirReader` | 20 | +0 `std::string` dir · +12 `DIR*` · +16 `dirent*` |
| `api::CGenericTags` | 20 | +0 tag field width (4) · +4 tag name width (3) · +8 `map<string,uebyte>` (the length-of-length n is stored/loaded as one byte at node+28) |
| `api::CTickManager` | 12 | `map<uebyte, STick>`; `STick` = {uint64 intervalo ms, timeval próximo, 4-byte `parado` (0/1, written and read as i32, so not a `bool`)} |
| `api::CWait` | 24 | +0 µs · +8 `timeval` deadline |
| `api::CSynchronizer` | 1 | `bool m_sincrono`; singleton @1839324 |
| `comum::CRegistradorMesario` *(name from the string it stores)* | 52 | +0 `map<CComparecimentoMesarioPK, CComparecimentoMesario>` · +12 last-lookup iterator · +16 name `"CRegistradorMesario"` · +28 second map · +40 `CComparecimentoMesarioServico` |
| `comum::CJustificador` *(idem)* | 40 | +0 map · +12 iterator · +16 `"CJustificador"` · +28 `CJustificadorServico` |

---

## 3. `comum::CAppInfo` and the state files

### 3.1 Getters

All getters follow one template (srcloc `cappinfo.cpp:42` non-const, `:57` const):

```cpp
template <typename ESTADO> ESTADO& GetEstado(const std::string& nome, std::optional<ESTADO>& estado)
{   if (!estado) throw CUeComumAppInfoError(7600 /*7601 const*/, format("O estado não foi carregado: {}", nome));
    return *estado; }
```

| func | method | notes |
|---|---|---|
| 291 / 457 → 6040 | `GetGeral()` / `GetGeral() const` | 6040 is the merged body; the thunks pass the srcloc record and the error code |
| 261 | `GetVota(EUrnaTurno)` | `m_vota[IndiceTurno("GetVota", turno)]` |
| 903 | `GetVota() const` | turno atual |
| 2841 | `TemVota(turno)` | `m_vota[...].has_value()` |
| 3788 | `GetHistoricoCargas() const` | from `gap.bin`: the carga code (+28) of each of the ≤10 `CDadoCorrespondencia` (96 B). The BU prints them as the *histórico de cargas* (QR `HIQT`/`HICA`) |
| 1553 | `static IndiceTurno(funcao, turno)` | `'3'` → `GetGeral().turno`; `'1'`→0, `'2'`→1, else error 7602 `"Turno {} invalido: {}"` (srcloc :472). The message shows the turno **argument**, not the resolved one: `IndiceTurno(f, '3')` on an urna without turno reports `Turno 3 invalido` |

Helper predicates (free functions, path inferred):
`EhTreinamentoSemTreinamentoEleitor()` (1823) = fase `'3'` && !treinamentoEleitor (mesário training);
`EhModoDemonstracaoSemTreinamentoEleitor()` (2520) = `CInformacaoEleicao::EhModoDemonstracao()` &&
!(fase `'3'` && treinamentoEleitor). Unit u09 calls 2520 `DeveRegistrarMesarios`, because it decides whether
the mesário-registration states run.

### 3.2 Writing `vota.bin`: `comum::SalvaEstado()` (func 491)

Every change of `EstadoGeralVota.estadoVota` in the whole application (32 callers: init, zerésima, mesário
registration, end of voting, BU copies, result copy…) is followed by this function:

```
detalhe = top of CApplicationContextStack ? its m_detalhe : "Erro de sincronização"
┌ CApplicationContextGuard(Actions 2 "…substitua a urna", detalhe, "Gravando o estado da urna",
│                          "Ocorreu um erro durante a sincronização do estado geral do VOTA.")
│   if urna desligando (@1832936) → throw CUeDesligandoError 4201 "Urna desligando"
│   CAppInfo::SalvaVotaInterno()            → CServicoEstadoGeralVota(MI, atual).Salva → trab<t>/vota.bin (MI)
│   CAssinador(turno 1 ? pkg 122 : 123).Assina(31 = vota.bin)   → vota.vsu on the MI
│   CSynchronizer::Sync()
└ ~guard
┌ CApplicationContextGuard(Actions 4 "…substitua a mídia de votação", same texts)
│   CAppInfo::SalvaVotaExterno()            → vota.bin on the MV
│   CopiaAssinaturaEstadoVotaParaMV()       → CopyFile(vota.vsu 122→124 | 123→125)   (func 4687)
│   CSynchronizer::Sync()
└ ~guard
```

The MV is **not** re-signed: its `vota.vsu` is a copy of the MI signature, so it only verifies if both
`vota.bin` encodings are byte-identical (they are, since both come from the same in-memory object).
`SalvaVotaInterno/Externo` (3790/3789) share body 6039, which does nothing if the current turno's state is
not loaded. The state service path is `CPath::GetPathTrab(midia, turno) / "vota.bin"` (11568; `gap.bin`
11567, `sa.bin` 11565). Asking it with turno `'0'` throws 7605–7607 "Urna sem turno em contexto onde
turno era esperado".

In the web build this function runs (observed during `votaInit`), but every write goes to MEMFS. The
signatures are the fixed text `assinatura simulada para vota_web_wasm`, and `Sync()` is empty (§5).

---

## 4. The `api` helpers

### 4.1 `api::CSystem` and `CDirReader` (csystem.cpp, cdirreader.cpp)

All errors are `CBaseError<api::EUeUtilError>` (codes 7000–7099, thunk func 346). The main entry points:

| function | behaviour |
|---|---|
| `ExistResource(stat&, path)` (1690) | `stat()`; `ENOENT`/`ENOTDIR` → false; other errors → 7031 "Falha ao validar a existência do recurso" |
| `CopyFile(orig, dest, preserva)` (378) | no-op if the names are equal. Otherwise: open source (7042), open destination `O_WRONLY\|O_CREAT\|O_TRUNC` + `O_SYNC` when `CSynchronizer` is synchronous, mode 0644 (7043), copy in 32 KiB blocks (7044). If `preserva` is set: `lstat` source (7045), `utime` (7046), owner (7047), mode (7048). The owner and mode are changed through `fchown`/`fchmod` on an fd (with `opendir`+`dirfd` for directories) and fall back to `chown`/`chmod` on EPERM/EACCES. Always ends with `CSynchronizer::Sync()` |
| `ReplaceFile(novo, destino, backup)` (5455) | atomic replace used for `rdv.dat` after each vote on the real urna. `PrepareReplace`: backup = `backup` or a `mkstemp(novo + ".XXXXXX")` name (7034/7035, the temp file is deleted at once); `novo` must be a regular file (7064), different from `destino` (7065), and the backup must not exist (7066). `ReplaceResource`: `destino`→backup (7067), `novo`→`destino` (7068, rolling back first), then the backup is deleted: a directory recursively (5458), a file with `ZeroFill`+`remove` |
| `ZeroFill(path)` (2761) | regular files only: `open(O_WRONLY)` (7069), `ftruncate(0)` then `ftruncate(size)` (7070). It does **not** write zeros (§10) |
| `IsEmptyDir(dir)` (2762) | must be an existing directory (7036). It is empty when no entry is a regular file or a sub-directory other than `.`/`..` |
| `GetFileSize(path)` (2759) | `stat().st_size` truncated to 32 bits (7038) |
| `CDirReader` (1914/1260/1913/2232/6020) | `opendir` wrapper; ctor error 7009 "Não foi possível ler o diretório …", `NextEntry` on a closed reader 7010; `IsDirectory/IsFile/IsLink` `stat`/`lstat` dir + "/" + `d_name` |

### 4.2 Dates and times (cdate.cpp, ctime.cpp, cdatetime.cpp)

* `CDate(const string&)` (3649): accepts `DDMMAA` or `DDMMAAAA` (`IsValid`, u13). A 2-digit year gets
  2000 added. The date is then parsed with `strptime("%d%m%Y")`, which in this build is **the JavaScript
  implementation of the Emscripten glue** (`env.strptime`). Errors 7001/7002 "Data inválida [...]".
  `CDate(uebyte dia, uebyte mes, ueint16 ano)` (2765) formats `"{:02}{:02}{:04}"` and delegates (the packed
  format arg types are unsigned, unsigned, int).
* `CDate::Format` (706): `A` → `DOM SEG TER QUA QUI SEX SAB` (table @1584308; 7003 if the weekday is ≥ 7),
  `DD`/`D`, `MM`/`M`, `YYYY`, `YY`, `Y` (year % 100); every other character is copied.
* `CTime` = seconds since midnight: `CTime(h,m,s)` (3642, error 7079 "Hora inválida" if h>23, m>59 or
  s≥60), `CTime("hhmm[ss]")` (3643, 7080), `operator-(int)` (3641: `operator-=` with `operator+=` inlined;
  7081 "Overflow" past 86 399 s, 7082 "Underflow" below 0).
* `CDateTime("DDMMAAAAhhmm[ss]")` (5475, 7005), `ConvertFromLocalTime(time_t)` (5476, uses **gmtime_r**;
  7006), `ConvertFromTimestamp("AAAAMMDDhhmmss")` (2764, 7008; reorders to `DDMMAAAA…`).

### 4.3 `CGenericTags`, the SAVD message codec (cgenerictags.cpp)

A textual TLV. Each element is `<tag><L><length in 2n hex digits><value>`, where `L = ';' + n`
(`<` `=` `>` `?` for n = 1..4). `insert` (1071: 7013/7014/7015, n must be 1..4), `TagSize` (5468: 7016),
`EncodeTLV` (5467: 7017; 7018 if the value is longer than 2^(8n)−1), `AppendTLV` (5466: 7019), plus
`WalkTreeTLV`/`DecodeTLV` (7020/7021) inlined into their only caller. The only user is the SAVD client,
with tags `pkg`, `key`, `fil` (n=1) and `sup` (n=3):

```
MontaMensagemPacote(pacote, chave)            (3831)  → "sup>" hex6 [ "pkg<" hex2 pacote  "key<" hex2 "<chave>" ]
AssinarVerificarArquivo(app, chave, pkg, fil) (5892)  → the same, with "fil<" hex2 <file> appended inside "sup",
                                                        sent to the SAVD with header 0x10010000 (byte 1 = app)
```

`comum::ValidarUE` (5890, other unit) sends the `sup{pkg,key}` form with header 0x20210000.
`CPacoteArquivos::ValidarChaveEAplicacaoValida` and `CSigVerifier` use 5892. Both ran during `votaInit`.

### 4.4 Other helpers

* `CStringUtils::GetVersionNumber` (1539): Boost.Regex `^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+` must match at
  position 0 (7025). Returns the match: `"10.23.0.1 - DESENVOLVIMENTO"` → `"10.23.0.1"` (the "Ver:"
  line of the BU/zerésima and QR field `VERS`).
* `CSynchronizer` (600): the singleton is created synchronous (`CreateInst(true)`, 7030 "Instância já
  criada"). `Sync()` (620) is an empty body in this build.
* `CTickManager` (3644/5451/700/5452): ids 0..255 (7077 "Não há mais id disponível para tick"), minimum
  period 30 ms (7072), `StartTick` re-arms from *now* (7075), `StopTick` (7074). `AddStoppedTick` (5452) is
  the constructor-time helper that states use to create timers they arm later.
* `CWait(ms)` (5447): deadline = now + ms, at most 4 000 000 ms (7083). The busy wait itself is `Aguarda`
  (u09, `usleep(200)` loop, no `emscripten_sleep`).
* `ITimerScheduler::GetInst/CreateInst<CTimerScheduler>` (1259/5444): `CPolySingletonList` registration,
  7085 "Tentativa de recriar o singleton".
* `IGenericFactory<AP>` (10839…10872 + 1561): the two `Create` overloads that `CDefaultGenericFactory`
  does not override throw `EUePatternError` 6761/6762 "Create(...) não sobrecarregado.".

### 4.5 Logging (cloga.cpp, cescritorlog.cpp)

`CLoga::loga(app, severidade, texto)` (433) → `CPolySingletonList::instance<CEscritorLog>().loga` (10260):
`app` must be < 61 (6950 "{} - O código da aplicação deve ser menor que {} [{}]") → `Escreve` (slot 4,
`simulador::CWasmLogd` → `log/logd.dat`). `fazOperacao(op)` (10262) accepts logd operations 230..240
(6951/6952) and sends them as a record with app = op, severity 1 and an empty text.
`IEventosLog::LogaInicioAplicacao(turno)` (11641: "Iniciando aplicação - {Sem turno|1º turno|2º turno|Padrão|
Inválido}") and `LogaVersaoAplicacao()` (11642: "Versão da aplicação: 10.23.0.1 - DESENVOLVIMENTO") are
called by `votaInit` through the function table (invoke slots 37/38). The texts are Latin-1 literals.

### 4.6 Persistence (cdaorepositorio.hpp, iuenuxgenericdao.h)

`CDAORepositorio` keeps `map<string, shared_ptr<IDAO>>` @1909964, keyed by `typeid(I).name()`, and fills it
during `votaInit` (inlined into func 7787) with `CComparecimentoMesarioDAO` and `CJustificadorDAO`.
`Entregar<I>()` looks the key up, calls `IDAO::Clonar()` (slot 2: a new DAO sharing the prototype's SQLite
connection), casts it with `dynamic_cast` and returns a new `shared_ptr<I>`. Error 6802 "Classe DAO {} não
foi registrada." (cdaorepositorio.hpp:79). The two instantiations are inlined into the constructors of their
caches: `CRegistradorMesario::GetInst()` (815, 52-byte singleton @1909932) and `CJustificador()` (3742,
40 bytes, singleton @1839012 via func 1391).

`IUenuxGenericDAO` defaults (2297 = shared throw body): `Excluir` 6804, `ExcluirID` 6805, `Atualizar` 6806
`"<op> não implementada para entidade " + typeid(DADO).name()`. They survive for
`CComparecimentoMesarioDAO` (no delete/update of mesário attendance rows), `CJustificadorDAO::Excluir` and
`CEleitorDinamicoDAO::Excluir`.

---

## 5. Web-build specifics

* **MEMFS**: `CopyFile`'s `O_SYNC`, `chown`/`fchown`, `utime` and `ZeroFill` act only on Emscripten's
  in-memory file system. `CSynchronizer::Sync()` compiles to an empty body.
* **`strptime` is JavaScript** (`env.strptime` in the glue): `CDate(const string&)` depends on the glue's
  regular-expression implementation. For the 8-digit inputs that `CDate::IsValid` lets through
  (`%d` = `0[1-9]|[1-9](?!\d)|1\d|2\d|30|31`) it gives the same day, month and year as a C `strptime`, except for
  4-digit years 0000–0099 (§10.5). It also fills `tm_wday` (`new Date(...).getDay()`), which `CDate` keeps as the
  weekday used by `Format("A")`. musl's `strptime` would leave the zeroed `tm_wday` at 0 (`DOM` for every date);
  glibc computes it.
* `IGenericFactory<IThreadImpl>` creates `simulador::CWasmThread` instead of the POSIX thread.
* The log writer is `simulador::CWasmLogd`. The SAVD requests of §4.3 go to the web mock, and the
  `.vsu` files contain `assinatura simulada para vota_web_wasm`.
* `CSystem::ReplaceFile` (the per-vote replacement of `rdv.dat`) never runs: the web policy
  `CSincronismoVotoEleitorWeb` skips vote persistence (u07). `SalvaEstado` does run (init, state changes).

## 6. The vota / comum states in this unit

(Slot names from the `comum::CAppState` protocol of u06. The estadoVota values are chars, `'1'` + the ASN.1 value.)

| func | state / method | what it does |
|---|---|---|
| 10210 | `CSincronismoOperador::StartState` | operator side after a vote: unless *treinamento do eleitor*, `CEleitores::MarcaVotou()` (inlined; 7845 "Item inexistente", 7846 "Dados dinâmicos não carregados"; situação := 3 VOTOU, `++votaram`, `ConfereDadosDinamicos("MarcaVotou")`), then message **5** to the voter thread (`CThreadEleitor` +36, priority 1) |
| 10734 | `CConfirmaEncerramento::ProcessInput` | CORRIGE: log (severity 2) "Procedimento de encerramento abortado" → `CPedeIdentidade`. CONFIRMA: log "Procedimento de encerramento confirmado", `CEstadoGeralVota::MarcaFimAquisicao()` (8084 "O fim da aquisição já havia sido registrado", 8085 "O início da aquisição não havia sido registrado"; dhFimAquisicao := now), estadoVota := `'9'` FIMAQUISICAOVOTOS, `SalvaEstado`, → `CFimAquisicaoVotos` |
| 10797 | `CControladorRegistraMesariosVota` slot 5 (`IniciaRegistro`, name inferred) | ZERESIMAIMPRESSA → `'7'` REGISTROMESARIOINICIAL; VOTAR → `'7'` only if the RDV has no attendance, there is no justification and fase ≠ treinamento; FIMAQUISICAOVOTOS → `':'` REGISTROMESARIOFINAL; `SalvaEstado` after a change |
| 11831 / 11830 | `testeteclado::CRetomada` `CriaTela` / `GetEstadoPassouNoTeste` | keyboard test on restart. "Não testar" is offered if this urna generated the zerésima (`urnaIdGerouZeresima == id da urna`) or the keyboard was already tested. On a *contingência-vota* urna (tipoUrna `'3'` of the turno) the first pass sets `tecladoTestadoPosConversao` and saves; next `CReinicioVotacao` |
| 11924 | `CInicioZeresima::StartState` | estadoVota := `'3'` AGUARDAHORAZERESIMA, `SalvaEstado`; → `CQuerImprimirZeresima` in voter training, else `CConfirmaImpressaoZeresima` (5959: lazy singleton, deadline = a configured date-time (config +544) + 10 800 s) |
| 11920 | `CQuerImprimirZeresima::ProcessInput` | CORRIGE: estadoVota := `'8'` VOTAR, `SalvaEstado`, → `CInicioVotacao` (voting opens **without** printing the zerésima; voter-training only). CONFIRMA → `CConfirmaImpressaoZeresima`. Reached from `CInicioZeresima` (11924) and from `CReinicioVotacao::StartState` (11835, on restart with no attendance/justification and no `ze.dat`), both only when `EhTreinamentoEleitor()` |
| 11931 | `CImpressaoZeresimaTardia::ProcessInput` | "late zerésima": CORRIGE: log "Mesário confirma que o horário da urna está errado" → `CInformacaoZeresimaTardia` (CWait 1000 ms). CONFIRMA: estadoVota := `'4'` GERARZE, `SalvaEstado`, log "... está correto" → `CGeraZeresima` |
| 11943 / 11940 | `CGeraResumoZeresimaBase::StartState` / `CGeraResumoZeresima::PosGeracao` | writes `trab/rze.dat` "Resumo da Zerésima" (header 5977, RDV extract 5971, trailer 5579), `SincronizaRelatorios("rze.dat")`, `urnaIdGerouZeresima := id da urna`, `SalvaEstado`, log report 11 [INÍCIO]/[TÉRMINO]; then slot 9 (estadoVota := `'5'` ZERESIMAGERADA + `SalvaEstado`) and slot 10 (next state) |
| 12062 | `CInicioBU::StartState` | first BU state: voter training → `CQuerImprimirBU` ("Quer imprimir o BU?"), otherwise → `CImprimindoBU` |
| 5428 | `CFimAquisicaoVotos::GetInst` | singleton thunk |

None of these states ran in the recorded sessions: the web page only drives voter sessions (u06 §10).


## 7. BOLETIM DE URNA: what this unit contributes, step by step

The unit does not build the BU body (u08 `CGeraBU`, u04/u25 generators). It supplies the state bookkeeping
around it and the VOTA-specific **header of the BU QR codes**. The encerramento (closing) flow, in order:

1. **Poll worker confirms the end of voting** (MT screen): `CConfirmaEncerramento::ProcessInput` (10734).
   CONFIRMA: log "Procedimento de encerramento confirmado", `MarcaFimAquisicao()` sets
   `EstadoGeralVota.dhFimAquisicao` = now. It fails if the end was already marked (8084) or if the start
   (`dhIniAquisicao`, set when voting opened) is missing (8085). Then estadoVota := FIMAQUISICAOVOTOS `'9'`,
   `SalvaEstado()` (vota.bin + vota.vsu on MI, then MV), next state `CFimAquisicaoVotos`.
2. **Closing mesário registration** (when attendance registration is on): the controller's slot 5 (10797)
   moves FIMAQUISICAOVOTOS → REGISTROMESARIOFINAL `':'` and saves. The count shown during registration
   ("Quantidade de mesários registrados:") comes from `CComparecimentoMesariosDS` (5387 opening / 10368
   closing) over `CRegistradorMesario` (815), i.e. the `comparecimento_mesario` table of `uenux.db`.
   These rows also feed the BU's list of mesários present (u08).
3. **BU generation** (`CGeraBU`, 12110, u08) uses from this unit:
   * `CAppInfo::GetVota()`: sets `dhEmissao` (+80) if unset (other unit), `TemVota(turno)` (2841);
   * `CAppInfo::GetHistoricoCargas()` (3788): carga codes of `gap.bin`'s correspondences = the *histórico
     de cargas* of the header;
   * `CStringUtils::GetVersionNumber` (1539): "Ver: 10.23.0.1";
   * `CDate::Format` (706) and the `CDate`/`CTime`/`CDateTime` constructors (3649, 3642/3643, 5475) for the dates;
   * `SalvaEstado()` (491) after each estadoVota change (GERARBU → GERARRELATORIOS …).
4. **QR codes of the BU**. The generator `CGeradorBUQRCodeVota` (ctor 5603, u04) is built with the
   comparecimento (+416), `m_origemRED` (+418), `m_incluiEmissao` (+419) and the emission date (+420).
   Its virtual `PreencheCabecalho(cab)` (**11242**) fills these header fields, each as `"TAG:value "`, in
   this order:

   | field | value | source |
   |---|---|---|
   | `ORIG` | `VOTA` (or `RED` if +418) | generator flag |
   | `AGRE` | aggregated sections joined by `.` (only if any) | `CLocal::GetAgregadas()` (called twice, the first call inlined) |
   | `LOCA` | polling-place number | `CLocal::GetLocalID()` (1933) |
   | `APTO` | aptos da seção + aptos em TTE (16-bit) | `CEleitores::ContaAptos()` (2823) |
   | `APTS` / `APTT` | aptos da seção / transferidos (TTE) | same |
   | `COMP` | comparecimento | generator +416 |
   | `MUNI` | município | `CEstadoGeral` +20 |
   | `HBBM` `HBBG` `HBSB` | voters enabled by fingerprint / by birth-year after fingerprint failure / voters without biometrics who voted | only if `CLocal::UrnaBiometrica()` (820); `CEleitores` counters 2821/1935/2822 |
   | `FALT` | aptos − comparecimento (16-bit wrap-around) | |
   | `DTAB` `HRAB` | start of vote acquisition, `YYYYMMDD` / `hhmmss` | `vota.bin dhIniAquisicao` (8090 "Início da aquisição não marcado" if absent) |
   | `DTFC` `HRFC` | end of vote acquisition | `vota.bin dhFimAquisicao` (8091 "Fim da aquisição não marcado") |
   | `DTEM` `HREM` | emission date/time | only if +419 |

   The four DT/HR fields are written only when `TemVota(turno)` is true. The other header fields, the
   body, the 823-character split, the SHA-512 `HASH` chain and the `ASSI` signature are u04's
   `CGeradorBUQRCode::GeraQRCodes` (5604); see `docs/bu/qrcode.md`.
5. **BU printing**: `CInicioBU::StartState` (12062) branches. In *treinamento do eleitor* the poll worker
   is asked "Quer imprimir o BU?" (`CQuerImprimirBU`). Otherwise the BU goes straight to `CImprimindoBU`
   (u09), which prints the vias and increments `qtdBU` with `SalvaEstado()` after each copy.
6. **Result files and MR copy** (u09 `CGravaResultado`, `CCopiaResultadoParaMR`): use `CSystem::CopyFile`
   (378), `GetFileSize` (2759), `IsEmptyDir` (2762) and `CAppInfo::GetHistoricoCargas` (3788) from this unit.

**Zerésima (before voting).** `CInicioZeresima` (11924) → estadoVota AGUARDAHORAZERESIMA. If the zerésima
is late, `CImpressaoZeresimaTardia` (11931) asks whether the clock is right (CONFIRMA → GERARZE →
`CGeraZeresima`). `CGeraResumoZeresimaBase::StartState` (11943) writes `rze.dat`, records the urna id in
`urnaIdGerouZeresima`, and `CGeraResumoZeresima::PosGeracao` (11940) sets ZERESIMAGERADA. In voter-training
mode, `CQuerImprimirZeresima` (11920) lets the operator skip printing it and go straight to VOTAR (it is reached from
`CInicioZeresima` and, after a restart before any activity, from `CReinicioVotacao` 11835).

**RDV (before the BU).** After every vote on a real urna, `CSincronismoVotoEleitor` (u07) replaces `rdv.dat`
with `CSystem::ReplaceFile(rdv.dat.tmp, rdv.dat, "")` (5455): the old RDV is renamed to a `mkstemp` name and
then "zero-filled" and removed. When the dynamic database is regenerated, the old `rdv.dat`/`uenux.db` are
also deleted with `ZeroFill` + `remove` (via func 2760). See §10 for why this does not erase the data.

---

## 8. Data read and written

| data | format | who |
|---|---|---|
| `dinamico/trab<t>/vota.bin` on MI and MV | BER `ModuloEstadoGeralVota` | `SalvaEstado` (491) → `SalvaVotaInterno/Externo` (3790/3789, `CServicoEstadoGeralVota`) |
| `vota.vsu` (ESavdPacote 122/123 MI, 124/125 MV) | SAVD signature package | signed on MI (`CAssinador(...).Assina(31)`), copied to MV (4687) |
| `gap.bin`, `sa.bin`, `eg.bin` | BER | paths by `GetPathArquivo` (11565/11567/11568); `gap.bin` correspondences read by 3788 |
| `trab/rze.dat` | report (zerésima summary) | 11943 |
| `log/logd.dat` | Latin-1 `app\|sev\|text` | `CLoga` → `CEscritorLog` → `CWasmLogd` |
| `uenux.db` tables `comparecimento_mesario`, `registro_justificativa` | SQLite | DAOs delivered by `CDAORepositorio` (815/3742) |
| SAVD requests | TLV `sup{pkg,key[,fil]}` + 8-byte header | 3831/5892 |
| any file | copy / replace / delete | `CSystem` |

---

## 9. wasm / Emscripten observations

* **Merged bodies (wasm-opt merge-similar-functions).** 6040 serves both `GetGeral()` overloads (the srcloc
  record and error code are parameters). 6039 serves both `SalvaVota*`: the 16-byte function name is passed
  as two 8-byte halves. 3896 is the body of the three `GetPathArquivo` (file name as a `[begin,end)` pair).
  1561 and 2297 are the throw bodies of the pattern / DAO defaults. 1912/2231 are thunks of `IsType`.
* **Dead-argument elimination** removed the 4th `bool` of `CopyFile` and the `size_t` of `AppendTLV`,
  whose signatures survive only in the srcloc records.
* **Inlining misleads the srcloc-based names.** 261/903/3788 were named `GetEstado`, but they are
  `CAppInfo` getters that inline the `GetEstado` template. 815/3742 are `Entregar`, inlined into
  constructors. 3641 is `operator-` with `operator-=`/`+=` inlined. 3642/3643 are `CTime` constructors
  with `EncodeTime` inlined. 600 is `GetInst` with `CreateInst` inlined. 700 is the thread's `StartTick`
  wrapper (`this+20`). 5892 is an `IInterfaceSavd` routine with `WalkTreeTLV`/`DecodeTLV` inlined.
  10260 is `CEscritorLog::loga` reached through `CWasmLogd`'s vtable.
* **Single-int structs are passed by value.** `CTime` (4 bytes) is returned as an i32 (func 3641).
* **Unsigned range check compiled as `(x-5) <=u -5`** in `CGenericTags::insert`. The decompiler shows it
  as a signed compare, but it means `x ∉ [1,4]`.
* **Inline string literals.** Short literals are rebuilt with 8-byte stores from a data base (e.g.
  `d_operator…[121855]` + 1024 = "Erro de sincronização"). The log texts of 11641 are stored Latin-1
  (`"1\xBA turno"`, `"Inv\xE1lido"`), which matches the Latin-1 `logd.dat`.
* **musl code in the unit:** `closedir` (957), `utime` (6238), and inlined `mkstemp`/`__randname` and
  `exp2` (in `EncodeTLV`). There is no `emscripten_sleep` anywhere in the unit.

---

## 10. Weird or risky code

1. **`CSystem::ZeroFill` does not overwrite anything** (func 2761, same source as the real urna). The name
   says "fill with zeros", but the body is `ftruncate(fd, 0); ftruncate(fd, size)`. On a Linux file system
   this makes a sparse file of zeros and only **frees** the old blocks; the old bytes stay on the medium
   until reused, and flash wear-levelling keeps them even longer. The same "ZeroFill + remove" is how the
   application deletes: the previous `rdv.dat` after every vote (`ReplaceFile`, 5455, via
   `CSincronismoVotoEleitor`), the old `rdv.dat`/`uenux.db` when the dynamic data is regenerated
   (func 2760), and directory backups (5458). If the real urna builds this code the same way, earlier RDV
   versions (one per vote) may survive in unallocated flash. Anyone who can read the raw medium and
   decrypt the RDV could compare consecutive versions and learn the order of the votes. That is a
   ballot-secrecy concern, not a proven leak (the file system and the RDV key protection were not
   examined). No effect in the simulator (MEMFS).
2. **`CWait` overflows in 32-bit builds** (5447). `m_microssegundos = ms * 1000` is a 32-bit `size_t` in
   wasm32 and it is added to the 32-bit `tv_usec`. For `ms` above about 2 146 484–2 147 483 (~36 min, the
   exact bound depends on the current `tv_usec`), which is still under the allowed maximum of 4 000 000 ms
   (checked with `i32.ge_u`), the sum exceeds `INT_MAX`. It becomes negative, the normalisation (signed
   `> 1000000`, `i32.ge_s`) is skipped, and the deadline lands in the past, so `Aguarda` (5446: stop when
   `now.sec > limite.sec` or equal seconds and `now.usec >= limite.usec`) ends at once.
   The current callers use 1000 and 2000 ms, so nothing is affected today. A build where `long`/
   `suseconds_t` is 64-bit would not have the problem.
3. **`FALT` (absent voters) is 16-bit arithmetic** in the BU QR header (11242):
   `(ueword)(aptosSeção + aptosTTE − comparecimento)`. If the turnout ever exceeds the voters counted as
   *aptos*, the QR code shows a number near 65 535 instead of a negative or zero value. No check exists.
4. **In voter training, the zerésima can be skipped and voting opened** (11920: CORRIGE on "Quer imprimir
   a zerésima?" → estadoVota VOTAR → `CInicioVotacao`). Two states lead there: `CInicioZeresima` (11924) and
   `CReinicioVotacao::StartState` (11835: restart with no attendance in the RDV, no justification and no
   `trab/ze.dat`). Both do so only when `EhTreinamentoEleitor()` (func 697) is true (fase `'3'` and
   `vota.bin.treinamentoEleitor`), so an official urna cannot take this path unless its signed `vota.bin`
   says voter training. Likewise `CInicioBU` (12062)
   makes BU printing optional only in that mode.
5. **`strptime` is JavaScript in the web build** (`CDate(const string&)`, 3649). The glue builds the result
   with `new Date(year, …)`, which maps years 0–99 to 1900–1999, while musl would keep them. Four-digit
   years below 100 only come from malformed data files, so this is a web-only divergence of little
   practical effect.
6. **`ReplaceFile` uses the mktemp pattern** (5455): `mkstemp` creates the backup name, deletes the file
   at once and reuses the name after a `stat` check (time-of-check/time-of-use). It is harmless on the
   single-application urna, but the `O_EXCL` protection of `mkstemp` is thrown away.
7. **Error text of `IsEmptyDir`** (2762): when the path exists but is not a directory, the message is
   `strerror(errno)` with a stale `errno`, so the stated reason is wrong. It only affects diagnostics.
8. **Simulated security in this unit's flows (web build).** `SalvaEstado` signs `vota.bin` through
   `CAssinador`, and the `.vsu` written is the literal `assinatura simulada para vota_web_wasm`. The SAVD
   TLV requests (3831/5892) are answered by the web mock, and `CSynchronizer::Sync()` is empty. These are
   expected in a simulator; they are listed so the flows are not mistaken for real integrity checks.

## 11. Open questions

* Real names of the classes stored at @1909932 / @1839012. They are called here `CRegistradorMesario` and
  `CJustificador` after the strings they store, which suggests a common base (a named cache over a
  service/DAO) whose RTTI is absent because it is not polymorphic.
* Slot 5 of `comum::IControladorRegistraMesarios` (10797, called `IniciaRegistro` here) and the two period
  encodings: `vf6` maps estadoVota `'7' '8' '9' ':'` to 1, 2, 0, 3, while the attendance counts use
  periods 1 and 2.
* The file of `SalvaEstado()` (491), `EhTreinamentoSemTreinamentoEleitor` (1823) and
  `EhModoDemonstracaoSemTreinamentoEleitor` (2520). They are placed in `cappinfo.cpp`, but they could live in
  a `comum` utility file.
* `CGenericTags` +4 (always 3 = tag-name width) duplicates `m_tamanhoTag - 1`. Is it a separate
  constructor argument or a derived member?
* The 8-byte SAVD request header (`0x10010000` with byte 1 = application) should be decoded against
  `IInterfaceSavd` (units u01/u02).
* `CAppInfo` bytes +488..+532 (an `optional<CEstadoGeralSA>`?).

---

## 12. Complete mapping table (109 functions)

`ran` = seen executing in the recorded sessions. "library/inlined helper" rows are musl / libc++ code.

| func | size | ran | tools name | reconstructed symbol | reconstructed in | original file | conf. |
|---:|---:|:-:|---|---|---|---|---|
| 261 | 609 | ✓ | `comum::GetEstado@261` | `comum::CAppInfo::GetVota(EUrnaTurno) [GetEstado<CEstadoGeralVota> inlined]` | src/uenux2/src/app/comum/appinfo/cappinfo.cpp | uenux2/src/app/comum/appinfo/cappinfo.cpp | high |
| 291 | 15 | ✓ | `comum::GetEstado@291` | `comum::CAppInfo::GetGeral()` | src/uenux2/src/app/comum/appinfo/cappinfo.cpp | uenux2/src/app/comum/appinfo/cappinfo.cpp | high |
| 346 | 18 |  | `api_f346` | `CBaseError<api::EUeUtilError> constructor thunk (CUeUtilError)` | src/uenux2/src/api/util/csystem.cpp (comment) | ecourna exception helper (EUeUtilError instance) | high |
| 378 | 4665 | ✓ | `api::CSystem::CopyFile` | `api::CSystem::CopyFile` | src/uenux2/src/api/util/csystem.cpp | uenux2/src/api/util/csystem.cpp | high |
| 433 | 68 | ✓ | `api::CLoga::loga` | `api::CLoga::loga` | src/uenux2/src/api/uelog/cloga.cpp | uenux2/src/api/uelog/cloga.cpp | high |
| 457 | 15 | ✓ | `comum::GetEstado@457` | `comum::CAppInfo::GetGeral() const` | src/uenux2/src/app/comum/appinfo/cappinfo.cpp | uenux2/src/app/comum/appinfo/cappinfo.cpp | high |
| 491 | 1329 | ✓ | `comum_f491` | `comum::SalvaEstado()` | src/uenux2/src/app/comum/appinfo/cappinfo.cpp | uenux2/src/app/comum/appinfo/cappinfo.cpp (path inferred) | medium |
| 600 | 143 |  | `api::CSynchronizer::CreateInst` | `api::CSynchronizer::GetInst() [CreateInst(true) inlined]` | src/uenux2/src/api/util/csynchronizer.cpp | uenux2/src/api/util/csynchronizer.cpp | high |
| 700 | 639 | ✓ | `api::CTickManager::StartTick` | `api::CTickManager::StartTick [via CThreadVota::StartTick, +20]` | src/uenux2/src/api/util/ctickmanager.cpp | uenux2/src/api/util/ctickmanager.cpp | high |
| 706 | 1982 | ✓ | `api::CDate::Format` | `api::CDate::Format` | src/uenux2/src/api/util/cdate.cpp | uenux2/src/api/util/cdate.cpp | high |
| 815 | 1261 |  | `api::persistencia::CDAORepositorio::Entregar@815` | `comum::CRegistradorMesario::GetInst() [CDAORepositorio::Entregar<IComparecimentoMesarioDAO> inlined]` | src/uenux2/src/app/comum/u20-foreign-fragments.cpp | uenux2/src/app/comum/comparecimentomesario/cregistradormesario.cpp (path inferred) | medium |
| 903 | 609 |  | `comum::GetEstado@903` | `comum::CAppInfo::GetVota() const` | src/uenux2/src/app/comum/appinfo/cappinfo.cpp | uenux2/src/app/comum/appinfo/cappinfo.cpp | high |
| 957 | 17 |  | `api_f957` | `musl closedir(DIR*)` | library/inlined helper | musl libc (closedir.c) | high |
| 1071 | 2048 |  | `api::CGenericTags::insert` | `api::CGenericTags::insert` | src/uenux2/src/api/util/cgenerictags.cpp | uenux2/src/api/util/cgenerictags.cpp | high |
| 1259 | 70 | ✓ | `api::ITimerScheduler::GetInst` | `api::ITimerScheduler::GetInst` | src/uenux2/src/api/util/itimerscheduler.cpp | uenux2/src/api/util/itimerscheduler.cpp | high |
| 1260 | 87 |  | `api::CDirReader::NextEntry` | `api::CDirReader::NextEntry` | src/uenux2/src/api/util/cdirreader.cpp | uenux2/src/api/util/cdirreader.cpp | high |
| 1539 | 803 |  | `api::CStringUtils::GetVersionNumber` | `api::CStringUtils::GetVersionNumber` | src/uenux2/src/api/util/cstringutils.cpp | uenux2/src/api/util/cstringutils.cpp | high |
| 1553 | 578 |  | `comum::CAppInfo::IndiceTurno` | `comum::CAppInfo::IndiceTurno` | src/uenux2/src/app/comum/appinfo/cappinfo.cpp | uenux2/src/app/comum/appinfo/cappinfo.cpp | high |
| 1561 | 215 |  | `api_f1561` | `api::detail::LancaNaoSobrecarregado (shared throw body of IGenericFactory defaults)` | src/uenux2/src/api/pattern/igenericfactory.h | uenux2/src/api/pattern/igenericfactory.h | medium |
| 1690 | 591 | ✓ | `api::ExistResource` | `api::ExistResource` | src/uenux2/src/api/util/csystem.cpp | uenux2/src/api/util/csystem.cpp | high |
| 1710 | 18 |  | `comum_f1710` | `CBaseError<comum::EUeComumAppInfoError> constructor thunk` | src/uenux2/src/app/comum/appinfo/cappinfo.h (comment) | ecourna exception helper (EUeComumAppInfoError instance) | high |
| 1715 | 18 |  | `api_f1715` | `CBaseError<api::EUePersistenciaError> constructor thunk` | src/uenux2/src/api/persistencia/cdaorepositorio.hpp (comment) | ecourna exception helper (EUePersistenciaError instance) | high |
| 1823 | 39 |  | `vota_f1823` | `comum::EhTreinamentoSemTreinamentoEleitor()` | src/uenux2/src/app/comum/appinfo/cappinfo.cpp | uenux2/src/app/comum/appinfo/cappinfo.cpp (path inferred) | medium |
| 1912 | 11 |  | `ecourna_f1912` | `api::CDirReader::IsDirectory() [IsType(S_IFDIR)]` | src/uenux2/src/api/util/cdirreader.h | uenux2/src/api/util/cdirreader.h | medium |
| 1913 | 101 |  | `api_f1913` | `api::CDirReader::~CDirReader` | src/uenux2/src/api/util/cdirreader.cpp | uenux2/src/api/util/cdirreader.cpp | medium |
| 1914 | 127 |  | `api::CDirReader::CDirReader` | `api::CDirReader::CDirReader` | src/uenux2/src/api/util/cdirreader.cpp | uenux2/src/api/util/cdirreader.cpp | high |
| 2130 | 250 |  | `api_f2130` | `std::string::insert(size_t pos, size_t n, char c)` | library/inlined helper | libc++ <string> (instantiation) | high |
| 2231 | 11 |  | `ecourna_f2231` | `api::CDirReader::IsFile() [IsType(S_IFREG)]` | src/uenux2/src/api/util/cdirreader.h | uenux2/src/api/util/cdirreader.h | medium |
| 2232 | 74 |  | `api_f2232` | `api::CDirReader::Close` | src/uenux2/src/api/util/cdirreader.cpp | uenux2/src/api/util/cdirreader.cpp | medium |
| 2283 | 57 |  | `api_f2283` | `std::__tree<std::__value_type<std::string, uebyte>>::destroy (~CGenericTags map)` | library/inlined helper | libc++ <__tree> (instantiation for CGenericTags::m_tags) | high |
| 2297 | 64 |  | `api_f2297` | `api::persistencia::detail::NaoImplementada (shared throw body of IUenuxGenericDAO defaults)` | src/uenux2/src/api/persistencia/iuenuxgenericdao.h | uenux2/src/api/persistencia/iuenuxgenericdao.h | medium |
| 2520 | 81 |  | `comum_f2520` | `comum::EhModoDemonstracaoSemTreinamentoEleitor() (u09: DeveRegistrarMesarios)` | src/uenux2/src/app/comum/appinfo/cappinfo.cpp | uenux2/src/app/comum/appinfo/cappinfo.cpp (path inferred) | medium |
| 2759 | 573 |  | `api::CSystem::GetFileSize` | `api::CSystem::GetFileSize` | src/uenux2/src/api/util/csystem.cpp | uenux2/src/api/util/csystem.cpp | high |
| 2761 | 841 | ✓ | `api::CSystem::ZeroFill` | `api::CSystem::ZeroFill` | src/uenux2/src/api/util/csystem.cpp | uenux2/src/api/util/csystem.cpp | high |
| 2762 | 919 |  | `api::CSystem::IsEmptyDir` | `api::CSystem::IsEmptyDir` | src/uenux2/src/api/util/csystem.cpp | uenux2/src/api/util/csystem.cpp | high |
| 2764 | 1283 | ✓ | `api::CDateTime::ConvertFromTimestamp` | `api::CDateTime::ConvertFromTimestamp` | src/uenux2/src/api/util/cdatetime.cpp | uenux2/src/api/util/cdatetime.cpp | high |
| 2765 | 424 | ✓ | `api_f2765` | `api::CDate::CDate(uebyte dia, uebyte mes, ueint16 ano)` | src/uenux2/src/api/util/cdate.cpp | uenux2/src/api/util/cdate.cpp | medium |
| 2841 | 107 |  | `comum_f2841` | `comum::CAppInfo::TemVota(EUrnaTurno) const` | src/uenux2/src/app/comum/appinfo/cappinfo.cpp | uenux2/src/app/comum/appinfo/cappinfo.cpp | medium |
| 3601 | 18 |  | `api_f3601` | `CBaseError<api::EUeUeLogError> constructor thunk` | src/uenux2/src/api/uelog/cescritorlog.cpp (comment) | ecourna exception helper (EUeUeLogError instance) | high |
| 3607 | 12 |  | `comum::servico::CComparecimentoMesarioServico::vf0` | `comum::servico::CComparecimentoMesarioServico::~CComparecimentoMesarioServico` | src/uenux2/src/app/comum/u20-foreign-fragments.cpp | uenux2/src/app/comum/servico/ccomparecimentomesarioservico.cpp (path inferred) | medium |
| 3641 | 286 |  | `api::CTime::operator+=` | `api::CTime::operator-(int) const [operator-= / operator+= inlined]` | src/uenux2/src/api/util/ctime.cpp | uenux2/src/api/util/ctime.cpp | medium |
| 3642 | 103 |  | `api::EncodeTime@3642` | `api::CTime::CTime(uebyte, uebyte, uebyte) [EncodeTime inlined]` | src/uenux2/src/api/util/ctime.cpp | uenux2/src/api/util/ctime.cpp | medium |
| 3643 | 452 | ✓ | `api::EncodeTime@3643` | `api::CTime::CTime(const std::string&) [EncodeTime inlined]` | src/uenux2/src/api/util/ctime.cpp | uenux2/src/api/util/ctime.cpp | medium |
| 3644 | 939 | ✓ | `api::CTickManager::AddTick` | `api::CTickManager::AddTick [SearchNextId inlined]` | src/uenux2/src/api/util/ctickmanager.cpp | uenux2/src/api/util/ctickmanager.cpp | high |
| 3645 | 608 |  | `api_f3645` | `api::(anonymous)::RenameResource` | src/uenux2/src/api/util/csystem.cpp | uenux2/src/api/util/csystem.cpp | medium |
| 3647 | 20 | ✓ | `api_f3647` | `api::CGenericTags::EncodeTLV(tag, valor) [= EncodeTLV(tag, valor, TagSize(tag))]` | src/uenux2/src/api/util/cgenerictags.h | uenux2/src/api/util/cgenerictags.cpp | medium |
| 3649 | 769 | ✓ | `api::CDate::CDate` | `api::CDate::CDate(const std::string&)` | src/uenux2/src/api/util/cdate.cpp | uenux2/src/api/util/cdate.cpp | high |
| 3742 | 1096 |  | `api::persistencia::CDAORepositorio::Entregar@3742` | `comum::CJustificador::CJustificador() [CDAORepositorio::Entregar<IJustificadorDAO> inlined]` | src/uenux2/src/app/comum/u20-foreign-fragments.cpp | uenux2/src/app/comum/justificativa/cjustificador.cpp (path inferred) | medium |
| 3788 | 825 |  | `comum::GetEstado@3788` | `comum::CAppInfo::GetHistoricoCargas() const [GetEstado<CEstadoGeralGap> inlined]` | src/uenux2/src/app/comum/appinfo/cappinfo.cpp | uenux2/src/app/comum/appinfo/cappinfo.cpp | medium |
| 3831 | 691 | ✓ | `api_f3831` | `comum::MontaMensagemPacote(pacote, chave) (SAVD 'sup' TLV)` | src/uenux2/src/app/comum/u20-foreign-fragments.cpp | uenux2/src/app/comum/iinterfacesavd.cpp (path inferred) | medium |
| 4687 | 221 | ✓ | `comum_f4687` | `comum::CopiaAssinaturaEstadoVotaParaMV()` | src/uenux2/src/app/comum/appinfo/cappinfo.cpp | uenux2/src/app/comum/appinfo/cappinfo.cpp (path inferred) | medium |
| 5380 | 59 |  | `api_f5380` | `comum::CRegistradorMesario::~CRegistradorMesario` | src/uenux2/src/app/comum/u20-foreign-fragments.cpp | uenux2/src/app/comum/comparecimentomesario/cregistradormesario.cpp (path inferred) | medium |
| 5387 | 454 |  | `api::CDataTextFmt<comum::(anonymous namespace)::CComparecimentoMesariosDS>::vf2@5387` | `api::CDataTextFmt<comum::(anon)::CComparecimentoMesariosDS>::Text (periodo 1)` | src/uenux2/src/app/comum/u20-foreign-fragments.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cpedetitulomesario.cpp (path inferred) | medium |
| 5428 | 22 |  | `vota_f5428` | `vota::CFimAquisicaoVotos::GetInst() (thunk of merged singleton body 764)` | src/uenux2/src/app/vota/u20-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/fimvotacao/cfimaquisicaovotos.cpp (path inferred) | medium |
| 5444 | 157 |  | `api::ITimerScheduler::CreateInst` | `api::ITimerScheduler::CreateInst<api::CTimerScheduler>` | src/uenux2/src/api/util/itimerscheduler.h | uenux2/src/api/util/itimerscheduler.h | high |
| 5447 | 573 |  | `api::CWait::CWait` | `api::CWait::CWait(size_t)` | src/uenux2/src/api/util/cwait.cpp | uenux2/src/api/util/cwait.cpp | high |
| 5451 | 548 |  | `api::CTickManager::StopTick` | `api::CTickManager::StopTick` | src/uenux2/src/api/util/ctickmanager.cpp | uenux2/src/api/util/ctickmanager.cpp | high |
| 5452 | 18 | ✓ | `vota_f5452` | `api::CTickManager::AddStoppedTick(size_t)` | src/uenux2/src/api/util/ctickmanager.cpp | uenux2/src/api/util/ctickmanager.cpp | medium |
| 5455 | 4258 |  | `api::(anonymous namespace)::PrepareReplace` | `api::CSystem::ReplaceFile [PrepareReplace, ReplaceResource, TemporaryPath, mkstemp inlined]` | src/uenux2/src/api/util/csystem.cpp | uenux2/src/api/util/csystem.cpp | medium |
| 5456 | 105 |  | `api_f5456` | `std::make_format_args(string, string, const char*)` | library/inlined helper | libc++ <format> (instantiation) | high |
| 5457 | 543 |  | `api::truncate` | `api::truncate(int, off_t, const std::string&)` (file-static) | src/uenux2/src/api/util/csystem.cpp | uenux2/src/api/util/csystem.cpp | high |
| 5458 | 1367 |  | `api_f5458` | `api::(anonymous)::RemoveConteudoDiretorio` | src/uenux2/src/api/util/csystem.cpp | uenux2/src/api/util/csystem.cpp | medium |
| 5460 | 73 |  | `api_f5460` | `std::operator+(std::string&&, const std::string&)` | library/inlined helper | libc++ <string> (instantiation) | high |
| 5466 | 627 | ✓ | `api::CGenericTags::AppendTLV` | `api::CGenericTags::AppendTLV` | src/uenux2/src/api/util/cgenerictags.cpp | uenux2/src/api/util/cgenerictags.cpp | high |
| 5467 | 2505 | ✓ | `api::CGenericTags::EncodeTLV` | `api::CGenericTags::EncodeTLV(tag, valor, tamanho)` | src/uenux2/src/api/util/cgenerictags.cpp | uenux2/src/api/util/cgenerictags.cpp | high |
| 5468 | 534 |  | `api::CGenericTags::TagSize` | `api::CGenericTags::TagSize` | src/uenux2/src/api/util/cgenerictags.cpp | uenux2/src/api/util/cgenerictags.cpp | high |
| 5475 | 909 | ✓ | `api::CDateTime::CDateTime` | `api::CDateTime::CDateTime(const std::string&)` | src/uenux2/src/api/util/cdatetime.cpp | uenux2/src/api/util/cdatetime.cpp | high |
| 5476 | 201 | ✓ | `api::CDateTime::ConvertFromLocalTime` | `api::CDateTime::ConvertFromLocalTime` | src/uenux2/src/api/util/cdatetime.cpp | uenux2/src/api/util/cdatetime.cpp | high |
| 5892 | 3294 | ✓ | `api::CGenericTags::WalkTreeTLV` | `comum::IInterfaceSavd::AssinarVerificarArquivo [CGenericTags::WalkTreeTLV/DecodeTLV inlined]` | src/uenux2/src/app/comum/u20-foreign-fragments.cpp | uenux2/src/app/comum/iinterfacesavd.cpp (path inferred) | low |
| 5959 | 177 |  | `vota_f5959` | `vota::CConfirmaImpressaoZeresima::GetInst()` | src/uenux2/src/app/vota/u20-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/iniciovotacao/cconfirmaimpressaozeresima.cpp (path inferred) | medium |
| 6020 | 350 |  | `api_f6020` | `api::CDirReader::IsType(mode_t) const` | src/uenux2/src/api/util/cdirreader.cpp | uenux2/src/api/util/cdirreader.cpp | medium |
| 6039 | 150 | ✓ | `comum_f6039` | `comum::CAppInfo::SalvaVota(funcao, midia) (merged body of SalvaVotaInterno/Externo)` | src/uenux2/src/app/comum/appinfo/cappinfo.cpp | uenux2/src/app/comum/appinfo/cappinfo.cpp | medium |
| 6040 | 516 | ✓ | `comum_f6040` | `comum::CAppInfo::GetGeral merged body [GetEstado<CEstadoGeral>]` | src/uenux2/src/app/comum/appinfo/cappinfo.cpp | uenux2/src/app/comum/appinfo/cappinfo.cpp | high |
| 6238 | 79 |  | `comum_f6238` | `musl utime(path, const utimbuf*)` | library/inlined helper | musl libc (utime.c) | high |
| 10210 | 334 |  | `vota::CSincronismoOperador::vf2` | `vota::CSincronismoOperador::StartState [comum::CEleitores::MarcaVotou inlined]` | src/uenux2/src/app/vota/u20-foreign-fragments.cpp | uenux2/src/app/vota/operador/csincronismooperador.cpp (path inferred) | medium |
| 10260 | 579 | ✓ | `simulador::CWasmLogd::vf3` | `api::CEscritorLog::loga [confereAppValida inlined]` | src/uenux2/src/api/uelog/cescritorlog.cpp | uenux2/src/api/uelog/cescritorlog.cpp | medium |
| 10262 | 1069 |  | `api::CEscritorLog::fazOperacao` | `api::CEscritorLog::fazOperacao` | src/uenux2/src/api/uelog/cescritorlog.cpp | uenux2/src/api/uelog/cescritorlog.cpp | high |
| 10310 | 37 |  | `api_f10310` | `atexit destructor of CRegistradorMesario::s_instancia (@1909932)` | src/uenux2/src/app/comum/u20-foreign-fragments.cpp (comment) | compiler-generated (static unique_ptr) | high |
| 10312 | 13 |  | `comum::servico::CComparecimentoMesarioServico::vf1` | `comum::servico::CComparecimentoMesarioServico deleting destructor` | src/uenux2/src/app/comum/u20-foreign-fragments.cpp | uenux2/src/app/comum/servico/ccomparecimentomesarioservico.cpp (path inferred) | medium |
| 10368 | 454 |  | `api::CDataTextFmt<comum::(anonymous namespace)::CComparecimentoMesariosDS>::vf2@10368` | `api::CDataTextFmt<comum::(anon)::CComparecimentoMesariosDS>::Text (periodo 2)` | src/uenux2/src/app/comum/u20-foreign-fragments.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cpedetitulomesario.cpp (path inferred) | medium |
| 10396 | 25 |  | `api::persistencia::IUenuxGenericDAO<comum::md::CComparecimentoMesario, comum::md::CComparecimentoMesarioPK>::Atualizar` | `api::persistencia::IUenuxGenericDAO<CComparecimentoMesario, CComparecimentoMesarioPK>::Atualizar` | src/uenux2/src/api/persistencia/iuenuxgenericdao.h | uenux2/src/api/persistencia/iuenuxgenericdao.h | high |
| 10397 | 25 |  | `api::persistencia::IUenuxGenericDAO<comum::md::CComparecimentoMesario, comum::md::CComparecimentoMesarioPK>::ExcluirID` | `api::persistencia::IUenuxGenericDAO<CComparecimentoMesario, CComparecimentoMesarioPK>::ExcluirID` | src/uenux2/src/api/persistencia/iuenuxgenericdao.h | uenux2/src/api/persistencia/iuenuxgenericdao.h | high |
| 10398 | 25 |  | `api::persistencia::IUenuxGenericDAO<comum::md::CComparecimentoMesario, comum::md::CComparecimentoMesarioPK>::Excluir` | `api::persistencia::IUenuxGenericDAO<CComparecimentoMesario, CComparecimentoMesarioPK>::Excluir` | src/uenux2/src/api/persistencia/iuenuxgenericdao.h | uenux2/src/api/persistencia/iuenuxgenericdao.h | high |
| 10734 | 648 |  | `vota::CConfirmaEncerramento::vf7` | `vota::CConfirmaEncerramento::ProcessInput [CEstadoGeralVota::MarcaFimAquisicao inlined]` | src/uenux2/src/app/vota/u20-foreign-fragments.cpp | uenux2/src/app/vota/operador/outrasopcoes/cconfirmaencerramento.cpp (path inferred) | medium |
| 10797 | 105 |  | `vota::CControladorRegistraMesariosVota::vf5` | `vota::CControladorRegistraMesariosVota::IniciaRegistro (slot 5)` | src/uenux2/src/app/vota/u20-foreign-fragments.cpp | uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp (path inferred) | low |
| 10839 | 21 |  | `api::IGenericFactory<api::ISemaphore>::Create@10839` | `api::IGenericFactory<api::ISemaphore>::Create(size_t)` | src/uenux2/src/api/pattern/igenericfactory.h | uenux2/src/api/pattern/igenericfactory.h | high |
| 10845 | 21 |  | `api::IGenericFactory<api::ISemaphore>::Create@10845` | `api::IGenericFactory<api::ISemaphore>::Create(const std::string&)` | src/uenux2/src/api/pattern/igenericfactory.h | uenux2/src/api/pattern/igenericfactory.h | high |
| 10856 | 21 |  | `api::IGenericFactory<api::IRWSyncCtl>::Create@10856` | `api::IGenericFactory<api::IRWSyncCtl>::Create(size_t)` | src/uenux2/src/api/pattern/igenericfactory.h | uenux2/src/api/pattern/igenericfactory.h | high |
| 10863 | 21 |  | `api::IGenericFactory<api::IRWSyncCtl>::Create@10863` | `api::IGenericFactory<api::IRWSyncCtl>::Create(const std::string&)` | src/uenux2/src/api/pattern/igenericfactory.h | uenux2/src/api/pattern/igenericfactory.h | high |
| 10865 | 21 |  | `api::IGenericFactory<api::ISyncCtl>::Create@10865` | `api::IGenericFactory<api::ISyncCtl>::Create(size_t)` | src/uenux2/src/api/pattern/igenericfactory.h | uenux2/src/api/pattern/igenericfactory.h | high |
| 10868 | 21 |  | `api::IGenericFactory<api::ISyncCtl>::Create@10868` | `api::IGenericFactory<api::ISyncCtl>::Create(const std::string&)` | src/uenux2/src/api/pattern/igenericfactory.h | uenux2/src/api/pattern/igenericfactory.h | high |
| 10871 | 21 |  | `api::IGenericFactory<api::IThreadImpl>::Create@10871` | `api::IGenericFactory<api::IThreadImpl>::Create(size_t)` | src/uenux2/src/api/pattern/igenericfactory.h | uenux2/src/api/pattern/igenericfactory.h | high |
| 10872 | 21 |  | `api::IGenericFactory<api::IThreadImpl>::Create@10872` | `api::IGenericFactory<api::IThreadImpl>::Create(const std::string&)` | src/uenux2/src/api/pattern/igenericfactory.h | uenux2/src/api/pattern/igenericfactory.h | high |
| 11242 | 10017 |  | `comum::CGeradorBUQRCodeVota::vf2` | `comum::CGeradorBUQRCodeVota::PreencheCabecalho(CCabecalhoQRCode&) const` | src/uenux2/src/app/comum/u20-foreign-fragments.cpp | uenux2/src/app/comum/relatorios/cgeradorbuqrcodevota.cpp (path inferred) | medium |
| 11467 | 25 |  | `api::persistencia::IUenuxGenericDAO<ecourna::app::dados::CIdentificacaoJustificativa, std::string>::Excluir` | `api::persistencia::IUenuxGenericDAO<ecourna::app::dados::CIdentificacaoJustificativa, std::string>::Excluir` | src/uenux2/src/api/persistencia/iuenuxgenericdao.h | uenux2/src/api/persistencia/iuenuxgenericdao.h | high |
| 11540 | 25 |  | `api::persistencia::IUenuxGenericDAO<comum::md::CEleitorDinamico, std::string>::Excluir` | `api::persistencia::IUenuxGenericDAO<comum::md::CEleitorDinamico, std::string>::Excluir` | src/uenux2/src/api/persistencia/iuenuxgenericdao.h | uenux2/src/api/persistencia/iuenuxgenericdao.h | high |
| 11565 | 25 |  | `comum::CServicoEstadoGeralSA::GetPathArquivo` | `comum::CServicoEstadoGeralSA::GetPathArquivo` | src/uenux2/src/app/comum/appinfo/servicos/cservicoestadogeralsa.cpp | uenux2/src/app/comum/appinfo/servicos/cservicoestadogeralsa.cpp | high |
| 11567 | 25 | ✓ | `comum::CServicoEstadoGeralGap::GetPathArquivo` | `comum::CServicoEstadoGeralGap::GetPathArquivo` | src/uenux2/src/app/comum/appinfo/servicos/cservicoestadogeralgap.cpp | uenux2/src/app/comum/appinfo/servicos/cservicoestadogeralgap.cpp | high |
| 11568 | 25 |  | `comum::CServicoEstadoGeralVota::GetPathArquivo` | `comum::CServicoEstadoGeralVota::GetPathArquivo` | src/uenux2/src/app/comum/appinfo/servicos/cservicoestadogeralvota.cpp | uenux2/src/app/comum/appinfo/servicos/cservicoestadogeralvota.cpp | high |
| 11641 | 653 | ✓ | `comum_f11641` | `comum::IEventosLog::LogaInicioAplicacao(EUrnaTurno)` | src/uenux2/src/app/comum/u20-foreign-fragments.cpp | uenux2/src/app/comum/log/ieventoslog.cpp (path inferred) | medium |
| 11642 | 414 | ✓ | `api_f11642` | `comum::IEventosLog::LogaVersaoAplicacao()` | src/uenux2/src/app/comum/u20-foreign-fragments.cpp | uenux2/src/app/comum/log/ieventoslog.cpp (path inferred) | medium |
| 11830 | 74 |  | `vota::testeteclado::CRetomada::vf11` | `vota::testeteclado::CRetomada::GetEstadoPassouNoTeste (slot 11)` | src/uenux2/src/app/vota/u20-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cretomada.cpp (path inferred) | medium |
| 11831 | 92 |  | `vota::testeteclado::CRetomada::vf10` | `vota::testeteclado::CRetomada::CriaTela (slot 10)` | src/uenux2/src/app/vota/u20-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cretomada.cpp (path inferred) | medium |
| 11920 | 168 |  | `vota::CQuerImprimirZeresima::vf7` | `vota::CQuerImprimirZeresima::ProcessInput` | src/uenux2/src/app/vota/u20-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/iniciovotacao/cquerimprimirzeresima.cpp (path inferred) | medium |
| 11924 | 41 |  | `vota::CInicioZeresima::vf2` | `vota::CInicioZeresima::StartState` | src/uenux2/src/app/vota/u20-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/iniciovotacao/ciniciozeresima.cpp (path inferred) | medium |
| 11931 | 655 |  | `vota::CImpressaoZeresimaTardia::vf7` | `vota::CImpressaoZeresimaTardia::ProcessInput` | src/uenux2/src/app/vota/u20-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/iniciovotacao/cimpressaozeresimatardia.cpp (path inferred) | medium |
| 11940 | 18 |  | `vota::CGeraResumoZeresima::vf9` | `vota::CGeraResumoZeresima::PosGeracao (slot 9)` | src/uenux2/src/app/vota/u20-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/iniciovotacao/cgeraresumozeresima.cpp (path inferred) | low |
| 11943 | 1731 |  | `vota::CGeraResumoZeresimaBase::vf2` | `vota::CGeraResumoZeresimaBase::StartState` | src/uenux2/src/app/vota/u20-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp (u09's reading; path inferred) | medium |
| 12062 | 189 |  | `vota::CInicioBU::vf2` | `vota::CInicioBU::StartState` | src/uenux2/src/app/vota/u20-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/fimvotacao/ciniciobu.cpp (path inferred) | medium |
