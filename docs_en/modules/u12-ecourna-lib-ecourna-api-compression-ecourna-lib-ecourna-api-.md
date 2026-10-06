# u12: `ecourna::api::io` (CFile), `ecourna::api::compression` (ZIP/.jez, 7-Zip) and `ecourna::api::pattern::CBaseType`

Unit u12 has **98 wasm functions** that the analyzer attributed to six files of the TSE library **ecourna**
(built from the Conan cache `/home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/…`):

| original file | what it is |
|---|---|
| `ecourna/api/io/cfile.cpp` | `CFile`, the RAII wrapper over a stdio `FILE*`. Nearly every file the voting application touches goes through it: ASN.1 data files, the RDV (*Registro Digital do Voto*), the state files `eg.bin`/`vota.bin`/`gap.bin`/`sa.bin`, fingerprint images (WSQ), the result files, key envelopes |
| `ecourna/api/io/asn/serialization.hpp` | BER (de)serialisation helpers. One instance survives: it reads a key envelope `EntidadeChave` |
| `ecourna/api/compression/icompressor.cpp` | `ICompressor`, the archive interface, plus the glob expansion (`globToFileList`) |
| `ecourna/api/compression/czip.cpp` | `CZip`: ZIP writer over minizip/zlib. In this code base ZIP archives use the extension **`.jez`** |
| `ecourna/api/compression/clzmacompress.cpp` | `CLzmaCompress`: `.7z` writer over 7-Zip 19.00. **Dead code** (never constructed) |
| `ecourna/api/pattern/cbasetype.hpp` | `CBaseType<T, MIN, MAX, ID>`, the range-checked value type behind `ZonaID`, `SecaoID`, `MunicipioID`, `CargoID`, … |

About half of the 98 indices are **not** ecourna code. Some are callers that inline ecourna functions: the
analyzer names a function after the first `std::source_location` it contains, so the host function gets the
name of the inlined callee. Others are libc++/7-Zip template instances found next to the ecourna functions. §8 and
the mapping table (§13) say where each one really belongs. The most important misattributions are:

* func **5480** "CFile::ReadLine" is really uenux2's **INI/properties reader** (`api::CIniStrings`), with
  `CFile::ReadLine` inlined. 10 more helpers of it follow the same path.
* func **2767** "CFile::Flush" is **`api::CEncryptedFile::Save`** (writes the encrypted RDV), with `CFile::Flush` inlined.
* func **5821** "globToFileList" is **`comum::CGravadorWSQ`'s archive routine** (packs the fingerprint images into
  `wsqbio.jez`/`wsqman.jez`/`wsqmes.jez`), with `ICompressor::Add(path)` and `globToFileList` inlined.
* func **2280** "DeserializeFromBuffer" receives a **file name**: it is `DeserializeFromFile`, which reads the whole file and then decodes it.

Reconstructed sources (all new):

```
src/ecourna/api/io/cfile.hpp, cfile.cpp, cioerror.hpp, cioerror.cpp (path inferred)
src/ecourna/api/io/asn/serialization.hpp
src/ecourna/api/compression/icompressor.hpp, icompressor.cpp, czip.hpp, czip.cpp, clzmacompress.hpp, clzmacompress.cpp
src/ecourna/api/pattern/cbasetype.hpp, iobservableprogresswithdescription.hpp (path inferred)
fragments of other units' files that u12 owns:
src/uenux2/src/api/io/cinistrings.u12-fragment.cpp          (5480 + 10 helpers: INI reader)
src/uenux2/src/api/io/cencryptedfile.u12-fragment.cpp       (2767)
src/uenux2/src/api/io/asn/cfileasn.u12-fragment.h           (2892 + 4 thunks: WriteToFile<EstadoGeral*>)
src/uenux2/src/api/io/asn/cpartialfileasn.u12-fragment.h    (CFileSeeder slots 2/8/9)
src/uenux2/src/api/util/csystem.u12-fragment.cpp            (IsDirectory, EhDoTipo)
src/uenux2/src/app/comum/gravadores/u12-foreign-fragments.cpp        (CGravadorWSQ 5821, CGravadorLog 11584, CGravadorEnvelopeArquivo 11623)
src/uenux2/src/app/comum/reconhecimentobiometrico/u12-foreign-fragments.cpp  (5371, mesário WSQ writer)
```

23 of the 98 functions ran during the recorded votes (`analysis/runtime/*.functions.tsv`, ✓ in §13). All of
them are `CFile` methods, the state-file writer `CFileASN::WriteToFile<EstadoGeral*>` (2892 and the thunks
9985/10008/10040/10168, called by the web mock `cappinfobuilder.cpp` during `votaInit`), `CEncryptedFile::Save`
(RDV), the libc++ `stat` helpers, `CFileSeeder` and the two voter-file readers (11517/11523). **None of the
compression code runs during a vote.** `CZip` is used only when the result files are written at the end of the
election day (*encerramento*). The web simulator's normal sessions never get there.

---

## 1. Classes and how they relate (RTTI)

```
ecourna::api::io::CFile                         not polymorphic, 20 bytes (cfile.hpp)
    api::CFile  (uenux2)                        = the same type (srclocs of uenux2 say "api::CFile";
                                                  the code calls the ecourna functions directly)
ecourna::api::exception::CError
 └─ CBaseError<io::EIoError, SErrorLimits{1175,1275}>
     └─ ecourna::api::io::CIoError              typeinfo @1554360, vtable @1112640, ctor func 3520
ecourna::api::exception::CError
 └─ CBaseError<compression::ECompressionError, SErrorLimits{1000,1100}>   typeinfo @1108484, vtable @1110656
                                                  (ctor thunk func 9584 -> shared body 1011)
pattern::NonCopyable (empty)   pattern::IObservableProgressWithDescription (vtable @1111068: 1526 / 9528)
        \                              /           +4 boost::signals2::signal<void(ulong, ulong, const string&)>
         compression::ICompressor                   typeinfo @1110916 (vmi), vtable @1110896, ctor 5192, 16 bytes
          ├─ compression::CZip                      typeinfo @1110824, vtable @1110676, 40 bytes
          └─ compression::CLzmaCompress             typeinfo @1108612, vtable @1108448 (never stored: dead)
api::CFileSeeder : ASN1::ISeeder                    vtable @1564408 (slots 2/8/9 in u12)
CBaseType<TYPE, MIN, MAX, BASIC_TYPE>               not polymorphic, holds only the value
```

| class | layout (wasm32) |
|---|---|
| `CFile` | `+0 FILE* m_file`, `+4 std::string m_name`, `+16 bool m_syncOnClose` |
| `ICompressor` | `+0 vptr`, `+4 signal` (`+4` signal vptr, `+8/+12 shared_ptr<signal_impl>`) |
| `CZip` | `ICompressor`, `+16 path m_arquivo`, `+28 zipFile m_zip`, `+32 ECompressLevel m_nivel`, `+36 EOpenMode m_modo` |
| `CLzmaCompress` | `ICompressor`, `+16 path m_arquivo`, `+32 ECompressLevel m_nivel` |
| `CFileSeeder` | `+0 vptr`, `+4 const CFile*`, `+8 size_t` total size |
| `api::CEncryptedFile` (u18) | `+0 shared_ptr<ISymmetricCipher>`, `+8 vector<uebyte>` plaintext |
| `api::CIniStrings` (INI reader) | `+0 map<string, CIniSection>` sections, `+12 map<string, CIniKey>` keys outside any section |

`ICompressor` vtable: `[0]` D1 (= 1526), `[1]` D0 (325, `unreachable`: abstract), `[2] Close()`,
`[3] ExpandeGlob() const` (name inferred: `CZip` returns true through the shared body 434 `return 1`, `CLzmaCompress`
false through 340; the 7-Zip code expands wildcards itself), `[4] DoAdd(const path& src, const path& dst)`.

---

## 2. `CFile` (cfile.cpp): behaviour

### 2.1 Opening (func 517 ctor [u18] → func 9508 `Open`, srcloc 60/74/117)

`CFile(name, mode, FileMode = 0)` zero-fills the object and calls `Open` only when `name` is not empty.
`Open`:

1. `fopen(name, mode)`. On failure: **1175** `"nao foi possivel abrir " + name`, `EFileOperation::Open`, `errno`.
2. Closes the previous file if one was open, stores `m_name`, `m_file`.
3. If the mode string contains `w`, `a` or `+` → `m_syncOnClose = true` (never reset afterwards).
4. `SetFileMode(fileMode)` (inlined; srcloc 117). `FileMode` is a bit mask: bit 0 → `O_SYNC` (`0x101000`),
   bit 1 → `O_NOATIME` (`0x40000`). The code calls **`fcntl(fd, 1 /*F_GETFD*/)` and
   `fcntl(fd, 2 /*F_SETFD*/, flags)`**, the descriptor-flag commands. The file-status commands are 3/4 (`F_GETFL`/`F_SETFL`; musl's
   `fcntl` (func 2322) adds `O_LARGEFILE` only for cmd 4, which confirms the numbering). The kernel ignores
   every bit except `FD_CLOEXEC` for `F_SETFD`, so the requested modes are never applied. In this binary only
   `FileMode 2` (`O_NOATIME`) is ever requested: 30 constructor calls pass 0, 2 pass 2 (the two WSQ writers 2725
   and 5371). The Emscripten glue returns 0 for cmds 1 and 2. On failure: `fclose`, **1176**
   `"nao foi possivel alterar modo " + name` (`Mode`). Its "not open" check (**1178**, unreachable from `Open`)
   is the only one in `CFile` that passes `errno` instead of 0.
5. Returns `true`.

### 2.2 Closing, flushing, syncing (336, inlined `Flush`, 5181)

* `Close()`: nothing if closed; `Sync()` if `m_syncOnClose`; `fclose`. On failure it throws **1179**
  `"nao foi possivel fechar " + name` **without clearing `m_file`** (see "suspicious"). On success it clears `m_name` and `m_file`.
  If `Sync()` throws, `fclose` is never reached: `m_file` stays open, `~CFile` retries `Close()`, and if `Sync()`
  fails again that exception is swallowed and the `FILE*` (and its descriptor) leaks.
* `Flush()` (only inlined, in `CEncryptedFile::Save`): 1180 if closed, `return fflush(m_file)`.
* `Sync()`: 1181 if closed; `fflush` → 1184 on error (`"nao sincronizou <name> - erro = <strerror>"`,
  `Sync`); `fileno` → 1182 on −1 (`"nao recuperou o descritor de <name>, erro = <strerror>"`, but tagged
  `EFileOperation::Read`); then **`fsync(fd)` only if `ecourna::api::util::CSynchronizer::GetInst()` is enabled**.
  The singleton @0x1D2CB4 is created on first use with `true` (inlined `CreateInst`, csynchronizer.cpp:59:
  1889 `"Instância já criada"` if created twice). fsync failure → 1183. The result: every file written through
  `CFile` in a writing mode is flushed and fsync'ed when it is closed.
* `~CFile()` (3564): `if (m_file) try { Close(); } catch (...) {}`.

### 2.3 Reading and writing (598, 1886, inlined `ReadLine`, 419, 1885, 418)

| method | checks (all `EFileOperation::None`, errno 0) | I/O and failure |
|---|---|---|
| `RawWrite(p, n)` | 1185 closed (`"arquivo nao estava aberto " + name`), 1186 `p == nullptr` "buffer invalido", 1187 `n == 0` "tamanho do buffer deve ser maior que zero" | `fwrite` then `fflush` **after every write**. Success if `fwrite` returned non-zero **and** `fflush == 0`. A partial write is returned as success. Otherwise **1188** `name + " size = <n>"` (`Write`, errno) |
| `RawRead(p, n)` | 1191 (`"arquivo nao estava aberto"`, **without** the name), 1192 "buffer invalido", 1193 "tamanho do buffer nao pode ser zero" | `fread`; `> 0` → return it; 0 and `ferror` → **1194**; 0 and `feof` → return 0; 0 otherwise → **1195** (both `name + " size = <n>"`, `Read`) |
| `ReadLine(line)` | 1205 | 128-byte `fgets` chunks until a chunk is not full or ends in `'\n'`; appends `'\n'` if missing. Empty result: at EOF → `line = ""`, return 0; otherwise **1206** (`name`, `Read`) |
| `Seek(off, whence)` | 1207 | `fseeko`; error → **1208** (`name`, `Seek`) |
| `Eof()` | 1209 | `feof != 0` |
| `Position()` | 1210 | `ftell`; negative → **1211** (`name`, `Tell`) |
| `RawWrite(vector)` (5180) | — | no-op for an empty vector |

The static helpers do not use the object:
`ReadFileContent(path)` (3522, text via 256-byte `fgets`, error 1213) and `ReadFileBinary(path)` (3521: `fopen "rb"`,
`fseek END`/`ftell`, `vector(size)`, `fread`, `fclose`; 1214 open failure, 1215 short read with errno 0).

### 2.4 `CIoError` (func 3520): how every message is built

```
errno != 0 : "<msg> [<op>] - <errno> - <strerror(errno)>"
errno == 0 : "<msg> [<op>]"          op ∈ None, Open, Close, Read, Write, Seek, Tell, Sync, Mode  (table @1112652;
                                     other values: "Description(EFileOperation) - não reconhecida")
```

Error codes used (enumerator names in `cioerror.hpp` are inferred):
1175, 1176, 1178, 1179 (Open/Mode/Close) · 1180–1184 (Flush/Sync) · 1185–1188 (RawWrite) ·
1191–1195 (RawRead) · 1205/1206 (ReadLine) · 1207/1208 (Seek) · 1209 (Eof) · 1210/1211 (Position) ·
1213 (ReadFileContent) · 1214/1215 (ReadFileBinary) · 1221/1222 (serialization.hpp). The unused codes in
1175–1275 probably belong to `CFile` methods that are not linked into this binary. One of them is known by name:
the data segment still holds two srcloc records of `uedword CFile::WriteLine(const std::string&) const`
(cfile.cpp:211 @1112360 and :214 @1112376, between `RawWrite`'s and `RawRead`'s), but no function references them.
By position, 1189/1190 are probably its codes.

---

## 3. Compression: `ICompressor`, `CZip`, `CLzmaCompress`

### 3.1 `ICompressor` (icompressor.cpp)

* `ICompressor()` (5192): builds the progress signal (`new signal_impl`, 20 bytes) and sets the vptr.
* `Add(const vector<path>&)` (9530): `DoAdd(p, p.filename())` for each file. **The directory is dropped** inside the archive.
* `Add(const map<path,path>&)` (9529, u15): `DoAdd(src, dst)` for each pair. Returns `*this`.
* `Add(const path&)` (inlined in 5821): if `ExpandeGlob()` and the path contains `*` or `?` →
  `Add(globToFileList(path))`, otherwise `Add({path})`.
* `globToFileList(glob)` (srcloc icompressor.cpp:61, inlined in 5821):
  1. `dir = glob.parent_path()`; if it is empty, `dir = "."` and `glob = "." / glob`;
  2. `!is_directory(dir)` → **1047** `"[<glob>] é um glob inválido"`;
  3. the **whole glob path** becomes an ECMAScript regex: `*`→`.*`, `.`→`\.`, `?`→`.`, `\`→`\\`. No other
     character is escaped;
  4. if the glob ends in **`"/*"`**, `recursive_directory_iterator(dir)` is used, otherwise
     `directory_iterator(dir)`; each entry that `is_regular_file()` and whose `path().string()`
     `regex_match`es (func 9532) is appended. The result is in directory order, not sorted.

### 3.2 `CZip` (czip.cpp)

State machine: *constructed* = `CreateZipFile()` opened the archive (`m_zip != nullptr`) → `DoAdd`\* → `Close()`
(`zipClose`, `m_zip = nullptr`). The destructor calls `Close()` and swallows exceptions.

| step | details |
|---|---|
| `CZip(path, level)` (5193, u15) | `m_modo = Criar (0)`; `CreateZipFile()` (9543, u15): `fill_fopen64_filefunc` + `zipOpen3` inlined, mode `APPEND_STATUS_CREATE`; `ConvertToOpen` maps 0→CREATE, 1→ADDINZIP, other → **1040** "Modo de abertura do zip incorreto."; open failure → **1041** "O arquivo {} não pode ser criado." |
| `CZip(path, level, map)` | only inlined (in `CGravadorLog`, 11584): `Add(map)`; on exception `FechaSemExcecao()` (9542: `try{Close();}catch(...){}`), rethrow |
| `ConvertToCompressLevel` (9539) | `{0, 1, -1 (Z_DEFAULT_COMPRESSION = 6), 9}[level]`; `level ≥ 4` → **1043** "Modo de compressão incorreto." |
| `DoAdd(src, dst)` (9538) | `AssertZipIsOpened` → **1042** "O arquivo {} está fechado."; entry time = `localtime(file_clock::to_sys(last_write_time(src)))`; `size = file_size(src)`; `zipOpenNewFileInZip2_64(zf, dst, &zi, no extra, no comment, level ? Z_DEFLATED : 0 (stored), ConvertToCompressLevel(), raw 0, zip64 = size > 0xFFFFFFFE)` → error **1044** "O arquivo {} não pode ser adicionado. {}"; `CFile(src,"rb")`; loop `while(!Eof())`: a new zeroed 1 KiB buffer, `RawRead`, `zipWriteInFileInZip` (error < 0 → **1045** "Falha ao adicionar o arquivo {}. {}"), progress; `file.Close()`; `zipCloseFileInZip` → **1046** "A adição do arquivo {} não pode ser concluída. {}" |
| progress (9537) | only when `processed % 1 MiB == 0`: `signal((processed << 10) / total, 1024, "Compactando arquivo <dst>...")` |
| `Close` (2691) | `zipClose(zf, NULL)` (inlined: central directory, zip64 end-of-central-directory record and locator when needed, `PK\5\6` record); `m_zip = nullptr`; error → **1039** "O arquivo {} não pode ser fechado. {}" |
| `ErrorToString` (9510) | 0 → "", −1 "Erro de arquivo.", −2 "Erro de leitura/escrita.", −3 "Erro de dados.", −4 "Memória insuficiente.", −5 "Erro de buffer.", −6 "Versão incompatível.", −100 "Arquivo não encontrado.", −102 "Parâmetro incorreto.", −103 "Arquivo inválido.", −104 "Erro interno.", −105 "Erro de CRC.", other "Erro desconhecido." |

No password is ever passed (minizip's PKWARE encryption code is linked but unused). ZIP entries get no extra field and no comment.

### 3.3 Who writes `.jez` archives

| writer | func | contents | level | name inside the archive |
|---|---|---|---|---|
| `comum::CGravadorWSQ` (vf2 11579 → `CompactaWsq`, 5821) | 5821 | every `*.wsq` in `<wsq>/habilitado/` (tipo 0 → `wsqbio.jez`), `<wsq>/nao-habilitado/` (1 → `wsqman.jez`) or `<wsq>/operador/` (2 → `wsqmes.jez`); the internal-flash directories when called from vf2 (`externo = false`); vf3/vf6 go through `comum_f6041`, which passes the flag on | **store** (WSQ is already compressed) | file name only |
| same, when every directory is empty | 5821 | **empty archive** (created and closed) | default | — |
| `comum::CGravadorLog::GravaResultado(api::CFile&)` | 11584 | the current log file (+52) and every regular file of the archived-logs directory (+40), written to `<work>/temp.jez` and then `rename`d to the result name (**7058** "Falha ao renomear [{}] para [{}]: {}"). The `api::CFile&` parameter is **ignored** | default (6) | file name only |

Before packing, `CompactaWsq` creates each missing directory (`mkdir` 0755 of each prefix). A failing `mkdir` stops
**silently**.

### 3.4 `CLzmaCompress` (dead code)

Vtable @1108448 is never stored and `DoAdd` (9586) has no caller. The reconstruction (clzmacompress.cpp) shows what
it would do: lazily build a process-wide 568-byte 7-Zip context (registers LZMA/LZMA2/BCJ/Copy and the 7z
format, builds CRC tables, `new CCodecs` + `Load`), then run the equivalent of `7za a -m0=LZMA -mf=off`
(or `-m0=Copy` for level 0) through `UpdateArchive` (func 3414 `Executa7z`, func 8376 `Adiciona7z`), and a
second `7za rn` pass when the entry name must change. On failure it throws **1013**
`"(0x{:X} - {}) Não foi possível incluir o arquivo {} no pacote {}."`. Dead-argument elimination removed the
progress `std::function` parameter of `Executa7z` (the callers still copy and destroy it), so the 7z path never
reported progress. See `docs/libraries/compression-7zip-lzma-zlib.md` §5 for the 7-Zip side.

---

## 4. INI / properties reader (func 5480 and helpers; uenux2 `api::CIniStrings`)

Only user in this binary: `comum::CGeracaoVersoesContratos`, which is built inside `vota::CGravaResultado::StartState` (12098)
and reads `<root>/etc/dependencias.properties` and `<root>/etc/versoes.properties`. It then checks the key
`tag` against the literal `"20260601173148"`. In the simulator both files are **0-byte stubs**: the parser
returns empty maps and the tag check then throws 8662 (u07 doc).

Algorithm (cinistrings.u12-fragment.cpp):

1. `CFile(path, "rb")`; **phase 1**: logical lines. `ReadLine`, drop `'\n'`, join lines that end in an odd number of
   `\` (the `\` is removed), then `Trim(RemoveComentario(line))`; keep the line if it is not empty; stop at EOF.
2. **Phase 2** (another loop over the vector, which looks like a separate `Parse(vector<string>)` that got inlined):
   `[name]` opens a section (a repeated section is **merged** into the first: copy + `SetChave` of the
   new keys + `CIniSection::operator=`, which throws 6007 "O nome de uma seção é constante." on a name mismatch);
   other lines go through `CIniStrings::ParseKeyLine` (first **unescaped** `=`; none → **6009**
   `"Linha inválida [{}]"`, empty key → **6010** `"Linha com nome inválido [{}]"`). The key/value pair goes to the
   current section (`CIniSection::SetChave`, 5482), or to the global key map before the first section.
   Redefining a key assigns the new value. `CIniKey::operator=` throws 6005 if the names differ.
3. Comments: first unescaped `#` or `;` (or NUL). Trim: `isspace` or `' '`/`'\t'`, keeping an escaped
   trailing blank. Escapes (table @1839288, applied in order with replace-all): `\n \t \; \: \= \# "\ " \[ \] \\`.

---

## 5. Other foreign functions in the unit

* **`api::CEncryptedFile::Save(path)`** (2767, cencryptedfile.cpp:88): empty name → 5994 "Nome de arquivo vazio";
  pads the plaintext PKCS#7-style to a multiple of 16, calls `ISymmetricCipher::Encrypt` (AES-256-CBC, which adds
  its own PKCS#7 block; u01), then `CFile(path,"wb")` → `RawWrite` → `Flush` → `Sync` → `Close`. For the RDV the
  cipher comes from `GetCifradorCryptoTable`: key **and IV** are derived from the HKDF seed, so the IV is fixed and
  nothing is appended (u01 §2.6, §3.1). A random appended IV is used only by ciphers built without an IV (the key
  files). Callers: funcs 5736/5737 (from 6737 `CGeraDadosDinamicos`, during `votaInit`: they create `rdv.dat`;
  these are the calls seen in the recorded sessions, `analysis/runtime/*.edges.tsv`) and the per-vote RDV writer
  7174, which saves to `<rdv>.tmp` (see docs/modules/u07-uenux2-src-app-vota-eleitor.md).
* **`api::CFileASN::WriteToFile<T>(path, obj)`** (2892 + thunks 9985/10008/10040/10168 for
  `EstadoGeralVota/SA/Gap/Urna`): `CFile(path,"wb")` + BER encoding (`"WriteToFile de " + name` as the context
  string of the encoder errors). Called by the web mock (`cappinfobuilder.cpp`, func 2894) to create `vota.bin`,
  `sa.bin`, `gap.bin`, `eg.bin` during `votaInit`. 2892 never calls `Close()` itself: `~CFile` closes the file and
  swallows any exception, so an `fsync` (1183) or `fclose` (1179) failure on these state files is never reported.
* **`api::CFileSeeder`** slots 2/8/9 (11434/11429/11428): `Eof()`, `size − Position()`, `Position()`. They are used by the
  streaming BER decoder of the voter file.
* **`api::CSystem::IsDirectory`** (5459) / `EhDoTipo` (6019): `stat` + `S_IFMT` test (`IsRegularFile` = func 412).
* **`comum::CGravadorEnvelopeArquivo::LeConteudo() const`** (11623): takes no parameter (wasm sig `(sret, this)`;
  `IGravadorEnvelope::vf7` calls it as `call_indirect(out, this)`); the file is the member `m_arquivo` (+172).
  `GetFileSize` + `vector(size)` + `CFile(m_arquivo, "rb")` + `RawRead`. The number of bytes read is ignored.
* **func 5371** (mesário WSQ writer, name unknown): skips saving when `statfs` reports < 5 MiB free; picks the first free
  `me{:06}.wsq` (id from a `std::function`), writes it with `CFile("w+b", FileMode 2)`, and copies it to the external
  directory with `CSystem::CopyFile`.
* **11517 / 11523**: the `std::function` bodies of `CEleitores::GetEleitoresImpedidos` / `GetEleitoresEstaticos`
  (full reading of `EntidadeImpedidos`; partial BER reading of `EntidadeEleitores` through `CFileSeeder`). They are
  documented by u03/u05 and listed here only because they inline `CFile`.
* **11905** `vota::CImpressaoVersaoPacotes::StartState`: prints the package versions from `dadoscarga.dat`
  (`DadosDisponiveisCarga`), "{}ª via". **3691/11224**: `CGeradorRelVersaoPacoteDados` destructors.
* **9130 / 9221**: `ecourna::app::dados::asn` converters that build `CBaseType` values (u14).

---

## 6. `CBaseType` (cbasetype.hpp:39)

`CBaseType(TYPE v)` stores `v` **first**, then throws `CBaseError<EPatternError>` **1300**
`"O tipo '<name>' deve ter valores no intervalo [<MIN>,<MAX>]."` if `v` is out of range. Instances:

| func | TYPE, MIN..MAX, id | name | body |
|---|---|---|---|
| 5682 | `ushort`, 1..99, 3 | CargoID | own |
| 9215 | `ushort`, 0..99, 4 | PartidoID | own |
| 2851 | `uint`, 0..99999, 6 | MunicipioID | → 6042 |
| 5850 | `ushort`, 0..9999, 7 | ZonaID | → 2299 |
| 5849 | `uint`, 0..9999, 8 | LocalID | → 6042 |
| 5848 | `ushort`, 0..9999, 9 | SecaoID | → 2299 |
| 1960 | `ushort`, 0..9999, 15 | QtdEleitor | → 2299 |
| 2277 | `ushort`, 0..9999, 36 | Ano | → 2299 |
| 2850 | `ushort`, 0..999, 38 | ScoreHabilitacao | → 2299 |
| 2666 | `ETipoIdentificadorEleitor`, 1..3, 40 | TipoIdentificadorEleitor | own |

2299/6042 are *merge-similar-functions* bodies with the parameters `(this, v, &srcloc, &name.data, &name.size,
&name.flags, MAX, MAX+1)`. Only `v >= MAX+1` is tested, because every merged instance has MIN 0 and an unsigned TYPE.
The name strings are static `std::string`s. Names longer than 10 characters are built at start-up in
`__wasm_call_ctors`.

---

## 7. Data read and written

| file | through | who |
|---|---|---|
| `/dsk/{fi,fe}/dinamico/trab{1,2}/{eg,vota,gap,sa}.bin` | `CFileASN::WriteToFile` → `CFile "wb"` (+ `fsync` at close) | web mock, `votaInit` (observed) |
| `<rdv>.tmp` / `rdv.dat` | `CEncryptedFile::Save` → `CFile "wb"` | RDV writer (observed at init) |
| `etc/dependencias.properties`, `etc/versoes.properties` | INI reader (`CFile "rb"` + `ReadLine`) | `CGravaResultado` at encerramento |
| key envelopes (`/dsk/fi/estatico/chave/…`, e.g. `cv.ber.pri`) | `DeserializeFromFile<EntidadeChave>` (2280) | BU, RC, WSQ key loaders (not shipped in the simulator, u01 §5) |
| `wsqbio.jez`, `wsqman.jez`, `wsqmes.jez` | `CZip` store | `CGravadorWSQ` at encerramento |
| `log.jez` (via `temp.jez` + rename) | `CZip` deflate | `CGravadorLog` at encerramento |
| `<wsq>/operador/me%06u.wsq` (+ external copy) | `CFile "w+b"` | func 5371 (mesário biometrics) |
| `dadoscarga.dat` | `CFileASN::ReadFromFile` | `CImpressaoVersaoPacotes` |

ASN.1 types touched: `ModuloEnvelopeChave::EntidadeChave` (read), `ModuloEstadoGeral{Urna,Vota,Gap,SA}` (written),
`ModuloImpedidos::EntidadeImpedidos`, `ModuloEleitores::EntidadeEleitores`,
`ModuloDadosDisponiveisCarga::DadosDisponiveisCarga` (read, in the foreign functions).

---

## 8. What the analyzer got wrong (so readers of other units are not misled)

| func | analyzer name | reality |
|---|---|---|
| 5480 | `CFile::ReadLine` | `api::CIniStrings` loader (uenux2) |
| 2767 | `CFile::Flush` | `api::CEncryptedFile::Save` |
| 5821 | `globToFileList` | `comum::CGravadorWSQ` archive routine |
| 2280 | `DeserializeFromBuffer` | `DeserializeFromFile` (buffer function inlined) |
| 5181 | `CFile::Sync` (srcloc also `CSynchronizer::CreateInst`) | correct; `CreateInst` is inlined |
| 1397, 2234–2236, 3650, 3651, 5482, 5484, 5485, 6272 | "cfile.cpp" | INI reader helpers / libc++ map |
| 2154, 2573, 2574, 3372, 4787, 3334, 4679, 4685, 7745, 3527, 9531 | "icompressor.cpp"/"czip.cpp" | libc++ `<filesystem>` / vector |
| 5459, 6019 | "icompressor.cpp" | `api::CSystem` helpers |
| 3413, 4936, 4949, 5009, 2170 | "clzmacompress.cpp"/none | 7-Zip 19.00 internals |
| 11584 | `CGravadorLog::vf7` (candidate `CSystem::Rename`) | `CGravadorLog::GravaResultado(api::CFile&) const` |

---

## 9. BOLETIM DE URNA (BU): where this unit is involved

This unit neither generates nor prints the BU. It supplies:

1. **The file layer under every BU file.** `CGravadorBU::GravaResultado(api::CFile&) const` (func 11629,
   cgravadorbu.cpp:484) receives, according to its signature, an open `CFile` into which the BU envelope is
   written. Every write is a `RawWrite`
   (`fwrite` + `fflush`). When the file is closed, `Sync()` runs (`fflush` + `fsync`, the `CSynchronizer` is on by default). A
   failure anywhere raises a `CIoError` whose text says the operation and `strerror(errno)`
   (e.g. `"<name> size = 1234 [Write] - 28 - No space left on device"`).
2. **Reading the keys that protect the BU.** `DeserializeFromFile<ModuloEnvelopeChave::EntidadeChave>` (2280) is
   the reader used by `vota::CGeraBU::StartState` (key of the *código verificador*, `cv.ber.pri`) and by
   `CGravadorBU::GravaResultado` (public key for the CEPESC protection). Steps: `CFile(path, "rb")`, whole
   file into a `vector<char>`, `CoderEnv` with rule **BER**, `decode` → **1221** "Falha ao fazer decode BER."
   if it fails, `isValid() && isStrictlyValid()` → **1222** "Foi lido conteúdo inválido." otherwise. An empty
   file fails earlier with 1192 "buffer invalido". The entity then goes to `CKeyLoader::DecipherKeyIfNeeded` (u01 §5).
   The key files are not shipped in the simulator. The copy of the template in the binary (2280) has **no landing
   pads**: all its calls are direct, so on any of these errors the `CFile` is never closed (its `FILE*` leaks) and the
   buffer and `CoderEnv` are not freed. The linker kept a COMDAT copy compiled in a uenux2 translation unit, which is
   built with exception catching disabled (u11 §4.2).
3. **Not the BU: `.jez`.** The ZIP archives are logs and fingerprint images. The BU (`*-bu.dat`), RDV,
   *imgbu* and the other result files are not compressed by `CZip`.

---

## 10. Web-build specifics

* **No compression runs during a vote.** `CZip` is reached only when the results are written at the end of the
  election day, and `CLzmaCompress` is unreachable in any build of this binary.
* **`fsync`/`fcntl` are no-ops.** MEMFS `fsync` does nothing. The glue's `___syscall_fcntl64` returns 0 for cmds
  1/2 (`F_GETFD`/`F_SETFD`), so `SetFileMode` always "succeeds".
* **The free-space check of func 5371 is always satisfied.** Emscripten's `statfs` returns fixed values
  (`bsize 4096 × bavail 500000` ≈ 1.9 GiB), whatever the real browser storage.
* **Fixture state files.** The four `WriteToFile<EstadoGeral*>` instances are driven by the mock
  `uenux2/mock/app/comum/cappinfobuilder.cpp` (func 2894) during `votaInit`, so the "state of the urna" comes from
  hard-coded data. Their `.vsu` signatures are the fake `"assinatura simulada para vota_web_wasm"`
  (docs/00-provenance.md).
* `etc/versoes.properties` / `etc/dependencias.properties` are 0-byte stubs (§4).
* **Timestamps in `.jez`** come from `localtime`, i.e. the browser's time zone (`__localtime_js`).

---

## 11. WebAssembly / Emscripten observations

* **Inlining moves the names.** Most of the misattributions in §8 come from inlining: 5480, 2767, 5821 and 2280 are
  named after a callee's `source_location`.
* **merge-similar-functions**: `CBaseType` (2299/6042 + 35–37-byte thunks). `CFileASN::WriteToFile<T>` (2892
  + thunks 9985/10008/10040/10168, and 2891 + 9684/9701/9703/9820): the type-specific callee is passed as a
  **table slot** argument.
* **Dead-argument elimination**: `Executa7z` (3414) lost its `std::function` parameter, but the callers still copy
  and destroy the functor around each call.
* **JS exceptions**: every `CIoError` construction in the `CFile` methods is an `invoke_iiiiiii(6210, …)` inside the
  `__THREW__` protocol. The exception is `DeserializeFromFile` (2280), a uenux2 COMDAT copy without landing pads that
  calls func 3520 directly. The uenux2 functions of the unit (5480, 5821, 5371, 11623, 2767) mostly call directly
  too, and keep `invoke_*` only in the inlined ecourna code. `~CFile` and `~CZip` catch everything
  (`__cxa_begin_catch`/`__cxa_end_catch`).
* **Inline string literals** in the pseudo-code are shown at the wrong address (`d_operator…[442013]` is really
  @443037). Read the `i32.const` in `q.py wat` when a decoded string looks wrong.
* **Format arguments**: `std::format` calls survive as `__try_constant_folding` + `__vformat_to`; the format
  string is the `{data,size}` i64 constant (e.g. `38654709059` = `{3395, 9}` = `"size = {}"`).
* `std::regex` uses flag value 512 (`ECMAScript`, non-zero because libc++ ABI v2 sets
  `_LIBCPP_ABI_REGEX_CONSTANTS_NONZERO`); `regex_match` shows up as the search helper called with flags `4160`.
* `ICompressor`'s deleting destructor is the shared `unreachable` body (325), as for every abstract class.

---

## 12. Weird or risky code (summary; details in the structured "suspicious" list)

| # | func | issue | severity |
|---|---|---|---|
| 1 | 336 / 3564 | `CFile::Close` throws on `fclose` failure **without clearing `m_file`**. `fclose` has already freed the `FILE`, so `~CFile` calls `Close` again: `fflush`/`fclose` on a freed `FILE*` (use after free, double free) | medium (only when `fclose` fails) |
| 2 | 9508 | `SetFileMode` uses `F_GETFD`/`F_SETFD` instead of `F_GETFL`/`F_SETFL`, so `O_SYNC`/`O_NOATIME` are silently dropped | low (only `O_NOATIME` is ever requested; `Sync()` covers durability) |
| 3 | 5480 (inlined `ReadLine`) | a last line whose length is a multiple of 127 bytes and that has no final `'\n'` gets the first 126 bytes of its last chunk appended **a second time**: `fgets` leaves the buffer untouched at EOF, and `buffer[126]` was zeroed before the call (the same happens if `fgets` fails with a read error in the middle of a long line) | low |
| 4 | 5480 (phase 2) | the parse loop can advance its iterator past `end()` (continuation on the last line). It cannot happen with file input: phase 1 never outputs a line that ends in an odd number of `\` | info |
| 4b | 2235 (`Trim`) | the right-trim starts at `size() - 1` computed unsigned: for an empty string it reads the byte before the string buffer and keeps scanning backwards while it finds blanks. The result is still `""`. Empty strings reach it on every comment-only line | info |
| 5 | 5821 / icompressor.cpp:61 | the glob→regex translation escapes only `* . ? \`. The directory part is inside the regex. A glob ending in `/*` makes the expansion recursive; the file order is the directory order | low |
| 6 | 5821 | a failing `mkdir` of the WSQ directories is ignored; when every directory is empty an empty `.jez` is still produced | low |
| 7 | 9537 | the progress division `(n << 10) / total` (`i64.div_u`) traps (a wasm `RuntimeError`, not a C++ exception) if `total == 0` at a 1 MiB boundary. `total` is `file_size()` taken before the file is opened, so this needs a file that grows from 0 bytes to ≥ 1 MiB while it is being added. A file that grows also makes the fraction exceed 1024 | info |
| 8 | 11623, 2280 | results of `RawRead` are ignored (11623: a short read leaves zero bytes at the end) / an empty key file is reported as "buffer invalido" (2280), and 2280 has no landing pads, so every error path leaks the open `FILE*` (§9) | low |
| 8b | 2892 | `WriteToFile(path, obj)` leaves the close to `~CFile`, which swallows exceptions: `fsync`/`fclose` failures of `eg.bin`/`vota.bin`/`gap.bin`/`sa.bin` are lost | low |
| 8c | 336 | if `Sync()` throws inside `Close()`, `fclose` is skipped; the retry in `~CFile` swallows a second failure and the `FILE*` leaks | info |
| 9 | 2767 | double padding (manual PKCS#7 + OpenSSL's) in `CEncryptedFile::Save` | info |
| 10 | 5371 | the ≥ 5 MiB free-space guard is always satisfied in the web build (fake `statfs`). On a real urna a full disk makes the fingerprint image **silently not saved** | low |
| 11 | 11584 | `CGravadorLog::GravaResultado` ignores the `api::CFile&` it is given and renames `temp.jez` over the result file | info |
| 12 | 9586 et al. | about 1,000 functions (344 KB) of 7-Zip are linked but unreachable (`CLzmaCompress` is never constructed) | info |

---

## 13. Mapping table (all 98 functions of u12)

"✓" = seen executing in the recorded sessions. Paths starting with `src/` are the reconstructed files in this
repository. A path without `src/` is an original file owned by another unit.

| func | size | ran | reconstructed symbol | source file |
|---|---|---|---|---|
| 336 | 363 | ✓ | `ecourna::api::io::CFile::Close()` | src/ecourna/api/io/cfile.cpp |
| 418 | 385 | ✓ | `ecourna::api::io::CFile::Position() const` | src/ecourna/api/io/cfile.cpp |
| 419 | 385 | ✓ | `ecourna::api::io::CFile::Seek(int, int) const` | src/ecourna/api/io/cfile.cpp |
| 598 | 2641 | ✓ | `ecourna::api::io::CFile::RawRead(void*, uedword) const` | src/ecourna/api/io/cfile.cpp |
| 1397 | 107 |  | `std::__tree<pair<string, api::CIniKey>>::destroy(node)` (52-byte nodes) | library/inlined helper: libc++ `std::map` instance of the INI reader (see src/uenux2/src/api/io/cinistrings.u12-fragment.cpp) |
| 1526 | 182 |  | `ecourna::api::pattern::IObservableProgressWithDescription::~IObservableProgressWithDescription()` (D1; also `~ICompressor`, slot 0) | src/ecourna/api/pattern/iobservableprogresswithdescription.hpp |
| 1885 | 269 | ✓ | `ecourna::api::io::CFile::Eof() const` | src/ecourna/api/io/cfile.cpp |
| 1886 | 1717 | ✓ | `ecourna::api::io::CFile::RawWrite(const void*, uedword) const` | src/ecourna/api/io/cfile.cpp |
| 1960 | 37 |  | `CBaseType<unsigned short, 0, 9999, 15>::CBaseType` ("QtdEleitor"; thunk -> 2299) | src/ecourna/api/pattern/cbasetype.hpp |
| 2154 | 63 |  | `std::shared_ptr<__dir_stream>::reset()` | library/inlined helper: libc++ `<filesystem>` (misattributed to icompressor.cpp) |
| 2170 | 178 |  | 7-Zip `CObjectVector<CArcExtInfo / CProperty>::Add(const T&)` (24-byte {UString, UString}) | library/inlined helper: 7-Zip 19.00 (reached only from the dead CLzmaCompress) |
| 2234 | 493 |  | `api::(anon)::FindNaoEscapado(const string&, const string&)` *(name inferred)* | src/uenux2/src/api/io/cinistrings.u12-fragment.cpp (orig. uenux2/src/api/io/cinistrings.cpp) |
| 2235 | 403 |  | `api::(anon)::Trim(const string&)` (escape-aware) *(name inferred)* | src/uenux2/src/api/io/cinistrings.u12-fragment.cpp |
| 2236 | 334 |  | `api::(anon)::RemoveComentario(const string&)` (cut at unescaped `#` `;` NUL) *(name inferred)* | src/uenux2/src/api/io/cinistrings.u12-fragment.cpp |
| 2277 | 37 |  | `CBaseType<unsigned short, 0, 9999, 36>::CBaseType` ("Ano"; thunk -> 2299) | src/ecourna/api/pattern/cbasetype.hpp |
| 2280 | 624 |  | `ecourna::api::io::DeserializeFromFile<ModuloEnvelopeChave::EntidadeChave>(obj, fileName)` + inlined `DeserializeFromBuffer<>` (lines 44, 47) *(outer name inferred)* | src/ecourna/api/io/asn/serialization.hpp |
| 2299 | 535 |  | `CBaseType<unsigned short, 0, MAX, ID>::CBaseType` merged body (16-bit, MIN 0) | src/ecourna/api/pattern/cbasetype.hpp |
| 2573 | 50 |  | `std::deque<__dir_stream>::back()` (recursive_directory_iterator stack) | library/inlined helper: libc++ `<filesystem>` |
| 2574 | 10 |  | `std::filesystem::directory_iterator::__dereference()` | library/inlined helper: libc++ `<filesystem>` |
| 2666 | 870 |  | `CBaseType<ETipoIdentificadorEleitor, TipoIDPrimeiro, TipoIDNumeroLivre, 40>::CBaseType` ("TipoIdentificadorEleitor") | src/ecourna/api/pattern/cbasetype.hpp |
| 2690 | 145 |  | `ecourna::api::compression::CZip::~CZip()` (D1, slot 0) | src/ecourna/api/compression/czip.cpp |
| 2691 | 2393 |  | `ecourna::api::compression::CZip::Close()` (slot 2; `zipClose` inlined) | src/ecourna/api/compression/czip.cpp |
| 2767 | 902 | ✓ | `api::CEncryptedFile::Save(const std::string&) const` (cencryptedfile.cpp:88) + inlined `CFile::Flush` (cfile.cpp:145) | src/uenux2/src/api/io/cencryptedfile.u12-fragment.cpp (orig. uenux2/src/api/io/cencryptedfile.cpp); `CFile::Flush` in src/ecourna/api/io/cfile.cpp |
| 2850 | 35 |  | `CBaseType<unsigned short, 0, 999, 38>::CBaseType` ("ScoreHabilitacao"; thunk -> 2299) | src/ecourna/api/pattern/cbasetype.hpp |
| 2851 | 37 |  | `CBaseType<unsigned int, 0, 99999, 6>::CBaseType` ("MunicipioID"; thunk -> 6042) | src/ecourna/api/pattern/cbasetype.hpp |
| 2892 | 172 | ✓ | `api::CFileASN::WriteToFile<T>(const std::string&, const T&)` merged body (CFile "wb" + writer callback) | src/uenux2/src/api/io/asn/cfileasn.u12-fragment.h (orig. uenux2/src/api/io/asn/cfileasn.h) |
| 3334 | 86 | ✓ | `std::filesystem::detail::posix_stat(const path&, StatT&, error_code*)` | library/inlined helper: libc++ `<filesystem>` operations.cpp (misattributed to czip.cpp) |
| 3372 | 244 |  | `std::filesystem::directory_iterator::__increment(error_code*)` | library/inlined helper: libc++ `<filesystem>` |
| 3413 | 126 |  | 7-Zip `CUpdateArchiveCommand::~CUpdateArchiveCommand()` | library/inlined helper: 7-Zip 19.00 UI/Common (dead code) |
| 3414 | 3739 |  | `ecourna::api::compression::(anon)::Executa7z(ctx, arquivos, pacote, novosNomes, renomear, comprimir)` *(name inferred)* | src/ecourna/api/compression/clzmacompress.cpp |
| 3520 | 1810 |  | `ecourna::api::io::CIoError::CIoError(EIoError, EFileOperation, const string&, int errnum, const source_location&)` | src/ecourna/api/io/cioerror.cpp (path inferred) |
| 3521 | 1180 |  | `static ecourna::api::io::CFile::ReadFileBinary(const std::filesystem::path&)` | src/ecourna/api/io/cfile.cpp |
| 3522 | 413 |  | `static ecourna::api::io::CFile::ReadFileContent(const std::string&)` | src/ecourna/api/io/cfile.cpp |
| 3527 | 104 |  | `std::vector<std::filesystem::path>::__destroy_vector::operator()` | library/inlined helper: libc++ `std::vector` |
| 3564 | 120 | ✓ | `ecourna::api::io::CFile::~CFile()` | src/ecourna/api/io/cfile.cpp |
| 3650 | 312 |  | `std::__tree<pair<string, api::CIniKey>>::__emplace_hint_unique_key_args` (map::insert(hint, v)) | library/inlined helper: libc++ `std::map` (CIniSection copy) |
| 3651 | 232 |  | `api::(anon)::Decodifica(const string&)` (comment, trim, 10 escapes) *(name inferred)* | src/uenux2/src/api/io/cinistrings.u12-fragment.cpp |
| 3691 | 168 |  | `comum::CGeradorRelVersaoPacoteDados::~CGeradorRelVersaoPacoteDados()` (D1) | uenux2/src/app/comum/relatorios/cgeradorrelversaopacotedados.cpp (other unit; trivial, not reconstructed) |
| 3797 | 82 |  | `std::__tree<pair<path, path>>::destroy(node)` (CGravadorLog file map) | library/inlined helper: libc++ `std::map` |
| 3863 | 203 |  | `std::vector<(72-byte package record)>::__destroy_vector` (CDadosDisponiveisCarga) | library/inlined helper: libc++ `std::vector` |
| 4679 | 344 |  | `std::filesystem::detail::ErrorHandler<uintmax_t>::report(const error_code&)` | library/inlined helper: libc++ `<filesystem>` |
| 4685 | 270 | ✓ | `std::filesystem::detail::create_file_status(...)` | library/inlined helper: libc++ `<filesystem>` |
| 4787 | 13 |  | `std::filesystem::recursive_directory_iterator::__dereference()` | library/inlined helper: libc++ `<filesystem>` |
| 4936 | 165 |  | 7-Zip `NWildcard::CCensor::~CCensor()` | library/inlined helper: 7-Zip 19.00 Common/Wildcard (dead code) |
| 4937 | 1038 |  | `std::default_delete<(anon)::CContexto7z>` (568-byte 7-Zip context: dtor + free) *(name inferred)* | src/ecourna/api/compression/clzmacompress.cpp |
| 4949 | 294 |  | 7-Zip `CPercentPrinter::~CPercentPrinter()` (ClosePrint inlined) | library/inlined helper: 7-Zip 19.00 UI/Console (dead code) |
| 5009 | 606 |  | 7-Zip `SplitString(const UString&, UStringVector&)` (LoadCodecs.cpp) | library/inlined helper: 7-Zip 19.00 (dead code) |
| 5180 | 36 |  | `ecourna::api::io::CFile::RawWrite(const std::vector<uebyte>&) const` *(name inferred)* | src/ecourna/api/io/cfile.hpp |
| 5181 | 2349 | ✓ | `ecourna::api::io::CFile::Sync() const` (+ inlined `util::CSynchronizer::GetInst/CreateInst`) | src/ecourna/api/io/cfile.cpp |
| 5192 | 205 |  | `ecourna::api::compression::ICompressor::ICompressor()` | src/ecourna/api/compression/icompressor.cpp |
| 5371 | 1566 |  | `comum::SalvaWsqMesario(...)` ("me{:06}.wsq", w+b, >= 5 MiB free, int -> ext copy) *(name, class, file inferred; low)* | src/uenux2/src/app/comum/reconhecimentobiometrico/u12-foreign-fragments.cpp |
| 5459 | 11 |  | `api::CSystem::IsDirectory(const std::string&)` *(name inferred)* | src/uenux2/src/api/util/csystem.u12-fragment.cpp (orig. uenux2/src/api/util/csystem.cpp) |
| 5480 | 8780 |  | `api::CIniStrings::CIniStrings(const std::string&)` INI loader (+ inlined `CFile::ReadLine` cfile.cpp:336/356, `CIniStrings::ParseKeyLine` cinistrings.cpp:183/187) *(name inferred)* | src/uenux2/src/api/io/cinistrings.u12-fragment.cpp; `CFile::ReadLine` in src/ecourna/api/io/cfile.cpp |
| 5482 | 512 |  | `api::CIniSection::SetChave(const CIniKey&)` (insert-or-assign) *(name inferred)* | src/uenux2/src/api/io/cinistrings.u12-fragment.cpp (orig. probably uenux2/src/api/io/cinisection.cpp) |
| 5484 | 131 |  | `api::(anon)::TerminaComContinuacao(const string&)` *(name inferred)* | src/uenux2/src/api/io/cinistrings.u12-fragment.cpp |
| 5485 | 110 |  | `api::(anon)::EhLinhaUtil(const string&)` *(name inferred)* | src/uenux2/src/api/io/cinistrings.u12-fragment.cpp |
| 5573 | 16 |  | `__exception_guard<vector<string>::__destroy_vector>` destructor | library/inlined helper: libc++ `std::vector` |
| 5585 | 159 | ✓ | `std::vector<std::string>::__init_with_size(first, last, n)` | library/inlined helper: libc++ `std::vector` (also used by live code) |
| 5682 | 558 |  | `CBaseType<unsigned short, 1, 99, 3>::CBaseType` ("CargoID") | src/ecourna/api/pattern/cbasetype.hpp |
| 5821 | 8686 |  | `comum::CGravadorWSQ::CompactaWsq(bool externo, const std::string&) const` (+ inlined `GetCaminhoCorretoInternal/External` cgravadorwsq.cpp:175/192, `ICompressor::Add(path)`, `globToFileList` icompressor.cpp:61) *(name inferred)* | src/uenux2/src/app/comum/gravadores/u12-foreign-fragments.cpp (orig. uenux2/src/app/comum/gravadores/cgravadorwsq.cpp); ecourna parts in src/ecourna/api/compression/icompressor.cpp |
| 5848 | 37 |  | `CBaseType<unsigned short, 0, 9999, 9>::CBaseType` ("SecaoID"; thunk -> 2299) | src/ecourna/api/pattern/cbasetype.hpp |
| 5849 | 37 |  | `CBaseType<unsigned int, 0, 9999, 8>::CBaseType` ("LocalID"; thunk -> 6042) | src/ecourna/api/pattern/cbasetype.hpp |
| 5850 | 37 |  | `CBaseType<unsigned short, 0, 9999, 7>::CBaseType` ("ZonaID"; thunk -> 2299) | src/ecourna/api/pattern/cbasetype.hpp |
| 6019 | 43 | ✓ | `api::(anon)::EhDoTipo(const string&, unsigned)` *(name inferred)* | src/uenux2/src/api/util/csystem.u12-fragment.cpp |
| 6042 | 535 |  | `CBaseType<unsigned int, 0, MAX, ID>::CBaseType` merged body (32-bit, MIN 0) | src/ecourna/api/pattern/cbasetype.hpp |
| 6272 | 13 |  | `api::(anon)::EhBrancoExtra(int)` (space or tab) *(name inferred)* | src/uenux2/src/api/io/cinistrings.u12-fragment.cpp |
| 7745 | 11 |  | `std::filesystem::status(const path&, error_code*)` (-> 1055 -> 3334) | library/inlined helper: libc++ `<filesystem>` |
| 8376 | 1611 |  | `ecourna::api::compression::(anon)::Adiciona7z(...)` *(name inferred)* | src/ecourna/api/compression/clzmacompress.cpp |
| 9130 | 62 |  | `ecourna::app::dados::asn::CConversorMunicipioZona::DoDeconverte` (slot 3) | ecourna-lib/ecourna/app/dados/asn/... (unit u14; not reconstructed here) |
| 9215 | 869 |  | `CBaseType<unsigned short, 0, 99, 4>::CBaseType` ("PartidoID") | src/ecourna/api/pattern/cbasetype.hpp |
| 9221 | 104 |  | `ecourna::app::dados::asn::CConversorDetalhamentoComparecimento::DoDeconverte` (slot 3) | ecourna-lib/ecourna/app/dados/asn/... (unit u14; not reconstructed here) |
| 9508 | 1152 | ✓ | `ecourna::api::io::CFile::Open(const string&, const string&, FileMode)` (+ inlined `SetFileMode`, line 117) | src/ecourna/api/io/cfile.cpp |
| 9510 | 974 |  | `ecourna::api::compression::CZip::ErrorToString(int)` *(name inferred)* | src/ecourna/api/compression/czip.cpp |
| 9530 | 324 |  | `ecourna::api::compression::ICompressor::Add(const std::vector<std::filesystem::path>&)` | src/ecourna/api/compression/icompressor.cpp |
| 9531 | 593 |  | `std::vector<std::filesystem::path>::__init_with_size(first, last, n)` | library/inlined helper: libc++ `std::vector` |
| 9532 | 799 |  | `globToFileList` per-entry lambda (is_regular_file && regex_match -> push_back) | src/ecourna/api/compression/icompressor.cpp |
| 9537 | 793 |  | `ecourna::api::compression::CZip::NotificaProgresso(uint64, uint64, const string&)` *(name inferred)* | src/ecourna/api/compression/czip.cpp |
| 9538 | 6329 |  | `ecourna::api::compression::CZip::DoAdd(const path&, const path&)` (slot 4; + inlined `AssertZipIsOpened` line 188) | src/ecourna/api/compression/czip.cpp |
| 9539 | 269 |  | `ecourna::api::compression::CZip::ConvertToCompressLevel() const` | src/ecourna/api/compression/czip.cpp |
| 9540 | 148 |  | `ecourna::api::compression::CZip::~CZip()` (D0, slot 1) | src/ecourna/api/compression/czip.cpp |
| 9542 | 52 |  | `ecourna::api::compression::CZip::FechaSemExcecao()` (`try { Close(); } catch (...) {}`) *(name inferred)* | src/ecourna/api/compression/czip.cpp |
| 9581 | 35 |  | `ecourna::api::compression::CLzmaCompress::~CLzmaCompress()` (D0, slot 1) | src/ecourna/api/compression/clzmacompress.cpp |
| 9582 | 32 |  | `ecourna::api::compression::CLzmaCompress::~CLzmaCompress()` (D1, slot 0) | src/ecourna/api/compression/clzmacompress.cpp |
| 9584 | 18 |  | `CBaseError<ECompressionError, {1000,1100}>::CBaseError(code, msg, loc)` thunk (-> 1011, vtable @1110656) | library/inlined helper: ecourna exception header (not reconstructed) |
| 9586 | 6423 |  | `ecourna::api::compression::CLzmaCompress::DoAdd(const path&, const path&)` (slot 4, line 114; 7-Zip init inlined) | src/ecourna/api/compression/clzmacompress.cpp |
| 9985 | 12 | ✓ | `api::CFileASN::WriteToFile<ModuloEstadoGeralVota::EstadoGeralVota>` thunk | src/uenux2/src/api/io/asn/cfileasn.u12-fragment.h |
| 10008 | 12 | ✓ | `api::CFileASN::WriteToFile<ModuloEstadoGeralSA::EstadoGeralSA>` thunk | src/uenux2/src/api/io/asn/cfileasn.u12-fragment.h |
| 10040 | 12 | ✓ | `api::CFileASN::WriteToFile<ModuloEstadoGeralGap::EstadoGeralGap>` thunk | src/uenux2/src/api/io/asn/cfileasn.u12-fragment.h |
| 10168 | 12 | ✓ | `api::CFileASN::WriteToFile<ModuloEstadoGeralUrna::EstadoGeralUrna>` thunk | src/uenux2/src/api/io/asn/cfileasn.u12-fragment.h |
| 11224 | 13 |  | `comum::CGeradorRelVersaoPacoteDados::~CGeradorRelVersaoPacoteDados()` (D0) | uenux2/src/app/comum/relatorios/cgeradorrelversaopacotedados.cpp (other unit; trivial) |
| 11428 | 10 | ✓ | `api::CFileSeeder::posicao()` (slot 9) *(name inferred)* | src/uenux2/src/api/io/asn/cpartialfileasn.u12-fragment.h (orig. uenux2/src/api/io/asn/cpartialfileasn.h) |
| 11429 | 16 |  | `api::CFileSeeder::restante()` (slot 8) *(name inferred)* | src/uenux2/src/api/io/asn/cpartialfileasn.u12-fragment.h |
| 11434 | 10 | ✓ | `api::CFileSeeder::fim()` (slot 2) *(name inferred)* | src/uenux2/src/api/io/asn/cpartialfileasn.u12-fragment.h |
| 11517 | 3826 | ✓ | `std::function<CEleitores::GetEleitoresImpedidos::$_0>::operator()` (CFileASN::ReadFromFile<EntidadeImpedidos> inlined) | uenux2/src/app/comum/dados/celeitores.cpp (unit u05: src/uenux2/src/app/comum/dados/u05-foreign-fragments.cpp) |
| 11523 | 4145 | ✓ | `std::function<CEleitores::GetEleitoresEstaticos::$_0>::operator()` (CPartialFileASN::ReadFromFile<EntidadeEleitores> inlined) | uenux2/src/app/comum/dados/celeitores.cpp (units u03/u05) |
| 11584 | 4385 |  | `comum::CGravadorLog::GravaResultado(api::CFile&) const` (slot 7; ignores the CFile; temp.jez + CZip + rename) | src/uenux2/src/app/comum/gravadores/u12-foreign-fragments.cpp (orig. uenux2/src/app/comum/gravadores/cgravadorlog.cpp) |
| 11623 | 226 |  | `std::vector<uebyte> comum::CGravadorEnvelopeArquivo::LeConteudo() const` (slot 8; reads `m_arquivo` +172) *(name inferred)* | src/uenux2/src/app/comum/gravadores/u12-foreign-fragments.cpp (orig. .../gravadores/cgravadorenvelopearquivo.cpp) |
| 11905 | 5098 |  | `vota::CImpressaoVersaoPacotes::StartState()` (slot 2; prints package versions from dadoscarga.dat) | uenux2/src/app/vota/.../cimpressaoversaopacotes.cpp (other unit; not reconstructed) |

Other functions used in this doc but outside u12: 517 (`CFile::CFile`, u18), 5193/9543/9529 (`CZip::CZip`,
`CreateZipFile`, `ICompressor::Add(map)`, u15), 1143 (`CBaseError<EIoError>` ctor), 1011 (shared `CBaseError` body),
2891 + 9684/9701/9703/9820 (`CFileASN::WriteToFile(CFile&, T)` merged body + thunks), 9673/9695/9702/9801
(`CodeObjectFunction<EstadoGeral*>`), 9585 (`CLzmaCompress::OnProgress`), 8118/8117/8119 (minizip),
8434 (`UpdateArchive`), 11579 (`CGravadorWSQ::vf2`), 6041, 11629 (`CGravadorBU::GravaResultado`),
12098 (`CGravaResultado::StartState`), 12110 (`CGeraBU::StartState`), 2725 (the other WSQ writer), 412 (`CSystem::IsRegularFile`),
2894 (mock state writer).

---

## 14. Open questions

* The real names of `CFile::FileMode`'s enumerators, `ICompressor` slot 3 and `EIoError` / `ECompressionError`.
  Only the numbers are known. For slot 3, unit u15 (`src/ecourna/api/compression/czip.u15.hpp`) proposes
  `IsOpen()`. Its only call site is the inlined `ICompressor::Add(path)`, where it decides whether a glob is
  expanded (CZip → true, CLzmaCompress → false). An "is open" meaning would make that branch pointless, so this
  unit calls it `ExpandeGlob()`. Both names are inferred.
* Whether func 5480 is `CIniStrings`' constructor or a static `Load(path)` that returns by value (the ABI is
  the same). Where `CIniSection::SetChave` (5482) lives (`cinisection.cpp` is likely).
* The owner of func 5371 (mesário WSQ writer): `CControlaArmazenamentoDeImagens` (its `GerarCaminhosUnicos`
  looks like this loop) or a class of `comparecimentomesario`.
* The meaning of the two option bytes that `Adiciona7z` (8376) stores at context +560 (`{1,0}`, `{2,0}`, `{2,0x32}`).
* Whether the real urna build links the same `CFile::SetFileMode`. If it does, `O_SYNC` never takes effect
  there either, and durability depends on the `fsync` in `Sync()`.
