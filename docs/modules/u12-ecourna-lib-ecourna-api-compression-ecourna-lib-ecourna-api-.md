# u12: `ecourna::api::io` (CFile), `ecourna::api::compression` (ZIP/.jez, 7-Zip) e `ecourna::api::pattern::CBaseType`

A unidade u12 tem **98 funções wasm** que o analisador atribuiu a seis arquivos da biblioteca **ecourna** do TSE
(compilada a partir do cache do Conan `/home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/…`):

| arquivo original | o que é |
|---|---|
| `ecourna/api/io/cfile.cpp` | `CFile`, o wrapper RAII sobre um `FILE*` do stdio. Quase todo arquivo que o aplicativo de votação manipula passa por ele: arquivos de dados ASN.1, o RDV (*Registro Digital do Voto*), os arquivos de estado `eg.bin`/`vota.bin`/`gap.bin`/`sa.bin`, imagens de digitais (WSQ), os arquivos de resultado, envelopes de chave |
| `ecourna/api/io/asn/serialization.hpp` | helpers de (des)serialização BER. Sobrevive uma instância: ela lê um envelope de chave `EntidadeChave` |
| `ecourna/api/compression/icompressor.cpp` | `ICompressor`, a interface de arquivo compactado, mais a expansão de glob (`globToFileList`) |
| `ecourna/api/compression/czip.cpp` | `CZip`: gravador de ZIP sobre minizip/zlib. Nesta base de código, os arquivos ZIP usam a extensão **`.jez`** |
| `ecourna/api/compression/clzmacompress.cpp` | `CLzmaCompress`: gravador de `.7z` sobre o 7-Zip 19.00. **Código morto** (nunca construído) |
| `ecourna/api/pattern/cbasetype.hpp` | `CBaseType<T, MIN, MAX, ID>`, o tipo de valor com verificação de intervalo por trás de `ZonaID`, `SecaoID`, `MunicipioID`, `CargoID`, … |

Cerca de metade dos 98 índices **não** é código da ecourna. Alguns são chamadores que inlinam funções da ecourna: o
analisador nomeia uma função a partir do primeiro `std::source_location` que ela contém, então a função hospedeira recebe o
nome da chamada inlinada. Outros são instâncias de template da libc++/7-Zip encontradas ao lado das funções da ecourna. A §8 e
a tabela de mapeamento (§13) dizem a que lugar cada uma realmente pertence. As atribuições erradas mais importantes são:

* a func **5480** "CFile::ReadLine" é na verdade o **leitor de INI/properties** da uenux2 (`api::CIniStrings`), com
  `CFile::ReadLine` inlinado. Outros 10 helpers dele seguem o mesmo caminho.
* a func **2767** "CFile::Flush" é **`api::CEncryptedFile::Save`** (grava o RDV cifrado), com `CFile::Flush` inlinado.
* a func **5821** "globToFileList" é **a rotina de compactação de `comum::CGravadorWSQ`** (empacota as imagens de digitais em
  `wsqbio.jez`/`wsqman.jez`/`wsqmes.jez`), com `ICompressor::Add(path)` e `globToFileList` inlinados.
* a func **2280** "DeserializeFromBuffer" recebe um **nome de arquivo**: ela é `DeserializeFromFile`, que lê o arquivo inteiro e depois o decodifica.

Arquivos-fonte reconstruídos (todos novos):

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

23 das 98 funções executaram durante os votos gravados (`analysis/runtime/*.functions.tsv`, ✓ na §13). Todas
elas são métodos de `CFile`, o gravador de arquivos de estado `CFileASN::WriteToFile<EstadoGeral*>` (2892 e os thunks
9985/10008/10040/10168, chamados pelo mock web `cappinfobuilder.cpp` durante `votaInit`), `CEncryptedFile::Save`
(RDV), os helpers de `stat` da libc++, `CFileSeeder` e os dois leitores do arquivo de eleitores (11517/11523). **Nenhum
código de compressão executa durante um voto.** `CZip` só é usado quando os arquivos de resultado são gravados no fim do
dia da eleição (*encerramento*). As sessões normais do simulador web nunca chegam lá.

---

## 1. Classes e como se relacionam (RTTI)

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

| classe | layout (wasm32) |
|---|---|
| `CFile` | `+0 FILE* m_file`, `+4 std::string m_name`, `+16 bool m_syncOnClose` |
| `ICompressor` | `+0 vptr`, `+4 signal` (`+4` vptr do signal, `+8/+12 shared_ptr<signal_impl>`) |
| `CZip` | `ICompressor`, `+16 path m_arquivo`, `+28 zipFile m_zip`, `+32 ECompressLevel m_nivel`, `+36 EOpenMode m_modo` |
| `CLzmaCompress` | `ICompressor`, `+16 path m_arquivo`, `+32 ECompressLevel m_nivel` |
| `CFileSeeder` | `+0 vptr`, `+4 const CFile*`, `+8 size_t` tamanho total |
| `api::CEncryptedFile` (u18) | `+0 shared_ptr<ISymmetricCipher>`, `+8 vector<uebyte>` texto claro |
| `api::CIniStrings` (leitor de INI) | `+0 map<string, CIniSection>` seções, `+12 map<string, CIniKey>` chaves fora de qualquer seção |

Vtable de `ICompressor`: `[0]` D1 (= 1526), `[1]` D0 (325, `unreachable`: abstrata), `[2] Close()`,
`[3] ExpandeGlob() const` (nome inferido: `CZip` retorna true pelo corpo compartilhado 434 `return 1`, `CLzmaCompress`
retorna false pelo 340; o código do 7-Zip expande os curingas sozinho), `[4] DoAdd(const path& src, const path& dst)`.

---

## 2. `CFile` (cfile.cpp): comportamento

### 2.1 Abertura (func 517 ctor [u18] → func 9508 `Open`, srcloc 60/74/117)

`CFile(name, mode, FileMode = 0)` zera o objeto e chama `Open` somente quando `name` não está vazio.
`Open`:

1. `fopen(name, mode)`. Em caso de falha: **1175** `"nao foi possivel abrir " + name`, `EFileOperation::Open`, `errno`.
2. Fecha o arquivo anterior, se havia um aberto, e armazena `m_name`, `m_file`.
3. Se a string de modo contém `w`, `a` ou `+` → `m_syncOnClose = true` (nunca é zerado depois).
4. `SetFileMode(fileMode)` (inlinado; srcloc 117). `FileMode` é uma máscara de bits: bit 0 → `O_SYNC` (`0x101000`),
   bit 1 → `O_NOATIME` (`0x40000`). O código chama **`fcntl(fd, 1 /*F_GETFD*/)` e
   `fcntl(fd, 2 /*F_SETFD*/, flags)`**, os comandos de flags do descritor. Os comandos de status do arquivo são 3/4 (`F_GETFL`/`F_SETFL`; o
   `fcntl` da musl (func 2322) adiciona `O_LARGEFILE` somente para o cmd 4, o que confirma a numeração). O kernel ignora
   todos os bits exceto `FD_CLOEXEC` em `F_SETFD`, então os modos pedidos nunca são aplicados. Neste binário, apenas
   `FileMode 2` (`O_NOATIME`) é pedido: 30 chamadas do construtor passam 0, 2 passam 2 (os dois gravadores de WSQ 2725
   e 5371). O glue do Emscripten retorna 0 para os cmds 1 e 2. Em caso de falha: `fclose`, **1176**
   `"nao foi possivel alterar modo " + name` (`Mode`). Sua verificação de "não aberto" (**1178**, inalcançável a partir de `Open`)
   é a única em `CFile` que passa `errno` em vez de 0.
5. Retorna `true`.

### 2.2 Fechamento, flush, sync (336, `Flush` inlinado, 5181)

* `Close()`: nada se estiver fechado; `Sync()` se `m_syncOnClose`; `fclose`. Em caso de falha, lança **1179**
  `"nao foi possivel fechar " + name` **sem zerar `m_file`** (ver "suspicious"). Em caso de sucesso, zera `m_name` e `m_file`.
  Se `Sync()` lançar, `fclose` nunca é alcançado: `m_file` continua aberto, `~CFile` tenta `Close()` de novo e, se `Sync()`
  falhar de novo, essa exceção é engolida e o `FILE*` (e seu descritor) vaza.
* `Flush()` (só inlinado, em `CEncryptedFile::Save`): 1180 se fechado, `return fflush(m_file)`.
* `Sync()`: 1181 se fechado; `fflush` → 1184 em caso de erro (`"nao sincronizou <name> - erro = <strerror>"`,
  `Sync`); `fileno` → 1182 se −1 (`"nao recuperou o descritor de <name>, erro = <strerror>"`, mas marcado como
  `EFileOperation::Read`); depois **`fsync(fd)` somente se `ecourna::api::util::CSynchronizer::GetInst()` estiver habilitado**.
  O singleton @0x1D2CB4 é criado no primeiro uso com `true` (`CreateInst` inlinado, csynchronizer.cpp:59:
  1889 `"Instância já criada"` se for criado duas vezes). Falha de fsync → 1183. Resultado: todo arquivo gravado por
  `CFile` em um modo de escrita passa por flush e fsync quando é fechado.
* `~CFile()` (3564): `if (m_file) try { Close(); } catch (...) {}`.

### 2.3 Leitura e escrita (598, 1886, `ReadLine` inlinado, 419, 1885, 418)

| método | verificações (todas `EFileOperation::None`, errno 0) | E/S e falha |
|---|---|---|
| `RawWrite(p, n)` | 1185 fechado (`"arquivo nao estava aberto " + name`), 1186 `p == nullptr` "buffer invalido", 1187 `n == 0` "tamanho do buffer deve ser maior que zero" | `fwrite` e depois `fflush` **após cada escrita**. Sucesso se `fwrite` retornou valor diferente de zero **e** `fflush == 0`. Uma escrita parcial é retornada como sucesso. Caso contrário, **1188** `name + " size = <n>"` (`Write`, errno) |
| `RawRead(p, n)` | 1191 (`"arquivo nao estava aberto"`, **sem** o nome), 1192 "buffer invalido", 1193 "tamanho do buffer nao pode ser zero" | `fread`; `> 0` → retorna o valor; 0 e `ferror` → **1194**; 0 e `feof` → retorna 0; 0 nos demais casos → **1195** (ambos `name + " size = <n>"`, `Read`) |
| `ReadLine(line)` | 1205 | blocos de `fgets` de 128 bytes até que um bloco não esteja cheio ou termine em `'\n'`; acrescenta `'\n'` se faltar. Resultado vazio: no EOF → `line = ""`, retorna 0; caso contrário, **1206** (`name`, `Read`) |
| `Seek(off, whence)` | 1207 | `fseeko`; erro → **1208** (`name`, `Seek`) |
| `Eof()` | 1209 | `feof != 0` |
| `Position()` | 1210 | `ftell`; negativo → **1211** (`name`, `Tell`) |
| `RawWrite(vector)` (5180) | — | não faz nada para um vetor vazio |

Os helpers estáticos não usam o objeto:
`ReadFileContent(path)` (3522, texto via `fgets` de 256 bytes, erro 1213) e `ReadFileBinary(path)` (3521: `fopen "rb"`,
`fseek END`/`ftell`, `vector(size)`, `fread`, `fclose`; 1214 falha de abertura, 1215 leitura incompleta com errno 0).

### 2.4 `CIoError` (func 3520): como cada mensagem é montada

```
errno != 0 : "<msg> [<op>] - <errno> - <strerror(errno)>"
errno == 0 : "<msg> [<op>]"          op ∈ None, Open, Close, Read, Write, Seek, Tell, Sync, Mode  (table @1112652;
                                     other values: "Description(EFileOperation) - não reconhecida")
```

Códigos de erro usados (os nomes dos enumeradores em `cioerror.hpp` são inferidos):
1175, 1176, 1178, 1179 (Open/Mode/Close) · 1180–1184 (Flush/Sync) · 1185–1188 (RawWrite) ·
1191–1195 (RawRead) · 1205/1206 (ReadLine) · 1207/1208 (Seek) · 1209 (Eof) · 1210/1211 (Position) ·
1213 (ReadFileContent) · 1214/1215 (ReadFileBinary) · 1221/1222 (serialization.hpp). Os códigos não usados em
1175–1275 provavelmente pertencem a métodos de `CFile` que não são linkados neste binário. Um deles é conhecido pelo nome:
o segmento de dados ainda guarda dois registros de srcloc de `uedword CFile::WriteLine(const std::string&) const`
(cfile.cpp:211 @1112360 e :214 @1112376, entre os de `RawWrite` e os de `RawRead`), mas nenhuma função os referencia.
Pela posição, 1189/1190 provavelmente são os códigos dele.

---

## 3. Compressão: `ICompressor`, `CZip`, `CLzmaCompress`

### 3.1 `ICompressor` (icompressor.cpp)

* `ICompressor()` (5192): monta o signal de progresso (`new signal_impl`, 20 bytes) e define o vptr.
* `Add(const vector<path>&)` (9530): `DoAdd(p, p.filename())` para cada arquivo. **O diretório é descartado** dentro do arquivo compactado.
* `Add(const map<path,path>&)` (9529, u15): `DoAdd(src, dst)` para cada par. Retorna `*this`.
* `Add(const path&)` (inlinado em 5821): se `ExpandeGlob()` e o caminho contém `*` ou `?` →
  `Add(globToFileList(path))`, caso contrário `Add({path})`.
* `globToFileList(glob)` (srcloc icompressor.cpp:61, inlinado em 5821):
  1. `dir = glob.parent_path()`; se estiver vazio, `dir = "."` e `glob = "." / glob`;
  2. `!is_directory(dir)` → **1047** `"[<glob>] é um glob inválido"`;
  3. o **caminho inteiro do glob** vira uma regex ECMAScript: `*`→`.*`, `.`→`\.`, `?`→`.`, `\`→`\\`. Nenhum outro
     caractere é escapado;
  4. se o glob termina em **`"/*"`**, é usado `recursive_directory_iterator(dir)`; caso contrário,
     `directory_iterator(dir)`; cada entrada que satisfaz `is_regular_file()` e cujo `path().string()`
     casa com `regex_match` (func 9532) é acrescentada. O resultado fica na ordem do diretório, sem ordenação.

### 3.2 `CZip` (czip.cpp)

Máquina de estados: *construído* = `CreateZipFile()` abriu o arquivo compactado (`m_zip != nullptr`) → `DoAdd`\* → `Close()`
(`zipClose`, `m_zip = nullptr`). O destrutor chama `Close()` e engole exceções.

| passo | detalhes |
|---|---|
| `CZip(path, level)` (5193, u15) | `m_modo = Criar (0)`; `CreateZipFile()` (9543, u15): `fill_fopen64_filefunc` + `zipOpen3` inlinados, modo `APPEND_STATUS_CREATE`; `ConvertToOpen` mapeia 0→CREATE, 1→ADDINZIP, outro → **1040** "Modo de abertura do zip incorreto."; falha de abertura → **1041** "O arquivo {} não pode ser criado." |
| `CZip(path, level, map)` | só inlinado (em `CGravadorLog`, 11584): `Add(map)`; em caso de exceção, `FechaSemExcecao()` (9542: `try{Close();}catch(...){}`), relança |
| `ConvertToCompressLevel` (9539) | `{0, 1, -1 (Z_DEFAULT_COMPRESSION = 6), 9}[level]`; `level ≥ 4` → **1043** "Modo de compressão incorreto." |
| `DoAdd(src, dst)` (9538) | `AssertZipIsOpened` → **1042** "O arquivo {} está fechado."; horário da entrada = `localtime(file_clock::to_sys(last_write_time(src)))`; `size = file_size(src)`; `zipOpenNewFileInZip2_64(zf, dst, &zi, no extra, no comment, level ? Z_DEFLATED : 0 (stored), ConvertToCompressLevel(), raw 0, zip64 = size > 0xFFFFFFFE)` → erro **1044** "O arquivo {} não pode ser adicionado. {}"; `CFile(src,"rb")`; laço `while(!Eof())`: um novo buffer zerado de 1 KiB, `RawRead`, `zipWriteInFileInZip` (erro < 0 → **1045** "Falha ao adicionar o arquivo {}. {}"), progresso; `file.Close()`; `zipCloseFileInZip` → **1046** "A adição do arquivo {} não pode ser concluída. {}" |
| progresso (9537) | somente quando `processed % 1 MiB == 0`: `signal((processed << 10) / total, 1024, "Compactando arquivo <dst>...")` |
| `Close` (2691) | `zipClose(zf, NULL)` (inlinado: diretório central, registro zip64 de fim do diretório central e localizador quando necessário, registro `PK\5\6`); `m_zip = nullptr`; erro → **1039** "O arquivo {} não pode ser fechado. {}" |
| `ErrorToString` (9510) | 0 → "", −1 "Erro de arquivo.", −2 "Erro de leitura/escrita.", −3 "Erro de dados.", −4 "Memória insuficiente.", −5 "Erro de buffer.", −6 "Versão incompatível.", −100 "Arquivo não encontrado.", −102 "Parâmetro incorreto.", −103 "Arquivo inválido.", −104 "Erro interno.", −105 "Erro de CRC.", outro "Erro desconhecido." |

Nenhuma senha é passada (o código de criptografia PKWARE do minizip está linkado, mas não é usado). As entradas do ZIP não recebem campo extra nem comentário.

### 3.3 Quem grava arquivos `.jez`

| gravador | func | conteúdo | nível | nome dentro do arquivo compactado |
|---|---|---|---|---|
| `comum::CGravadorWSQ` (vf2 11579 → `CompactaWsq`, 5821) | 5821 | todo `*.wsq` em `<wsq>/habilitado/` (tipo 0 → `wsqbio.jez`), `<wsq>/nao-habilitado/` (1 → `wsqman.jez`) ou `<wsq>/operador/` (2 → `wsqmes.jez`); os diretórios da flash interna quando chamado a partir da vf2 (`externo = false`); vf3/vf6 passam por `comum_f6041`, que repassa a flag | **store** (WSQ já é comprimido) | só o nome do arquivo |
| o mesmo, quando todos os diretórios estão vazios | 5821 | **arquivo compactado vazio** (criado e fechado) | padrão | — |
| `comum::CGravadorLog::GravaResultado(api::CFile&)` | 11584 | o arquivo de log atual (+52) e todo arquivo regular do diretório de logs arquivados (+40), gravados em `<work>/temp.jez` e depois renomeados com `rename` para o nome do resultado (**7058** "Falha ao renomear [{}] para [{}]: {}"). O parâmetro `api::CFile&` é **ignorado** | padrão (6) | só o nome do arquivo |

Antes de empacotar, `CompactaWsq` cria cada diretório que falta (`mkdir` 0755 de cada prefixo). Um `mkdir` que falha interrompe
o processo **silenciosamente**.

### 3.4 `CLzmaCompress` (código morto)

A vtable @1108448 nunca é armazenada e `DoAdd` (9586) não tem chamador. A reconstrução (clzmacompress.cpp) mostra o que
ela faria: montar de forma preguiçosa um contexto do 7-Zip de 568 bytes, global ao processo (registra LZMA/LZMA2/BCJ/Copy e o formato
7z, monta tabelas de CRC, `new CCodecs` + `Load`), e então executar o equivalente a `7za a -m0=LZMA -mf=off`
(ou `-m0=Copy` para o nível 0) por meio de `UpdateArchive` (func 3414 `Executa7z`, func 8376 `Adiciona7z`), e uma
segunda passada `7za rn` quando o nome da entrada precisa mudar. Em caso de falha, lança **1013**
`"(0x{:X} - {}) Não foi possível incluir o arquivo {} no pacote {}."`. A eliminação de argumentos mortos removeu o
parâmetro `std::function` de progresso de `Executa7z` (os chamadores ainda o copiam e destroem), então o caminho 7z nunca
reportou progresso. Ver `docs/libraries/compression-7zip-lzma-zlib.md` §5 para o lado do 7-Zip.

---

## 4. Leitor de INI / properties (func 5480 e helpers; `api::CIniStrings` da uenux2)

Único usuário neste binário: `comum::CGeracaoVersoesContratos`, que é construído dentro de `vota::CGravaResultado::StartState` (12098)
e lê `<root>/etc/dependencias.properties` e `<root>/etc/versoes.properties`. Em seguida, ele compara a chave
`tag` com o literal `"20260601173148"`. No simulador, os dois arquivos são **stubs de 0 bytes**: o parser
retorna mapas vazios e a verificação da tag então lança 8662 (doc da u07).

Algoritmo (cinistrings.u12-fragment.cpp):

1. `CFile(path, "rb")`; **fase 1**: linhas lógicas. `ReadLine`, descarta `'\n'`, junta linhas que terminam em um número ímpar de
   `\` (o `\` é removido), depois `Trim(RemoveComentario(line))`; mantém a linha se ela não estiver vazia; para no EOF.
2. **Fase 2** (outro laço sobre o vetor, que parece ser um `Parse(vector<string>)` separado que foi inlinado):
   `[name]` abre uma seção (uma seção repetida é **mesclada** na primeira: cópia + `SetChave` das
   novas chaves + `CIniSection::operator=`, que lança 6007 "O nome de uma seção é constante." se os nomes não coincidirem);
   as outras linhas passam por `CIniStrings::ParseKeyLine` (primeiro `=` **não escapado**; nenhum → **6009**
   `"Linha inválida [{}]"`, chave vazia → **6010** `"Linha com nome inválido [{}]"`). O par chave/valor vai para a
   seção atual (`CIniSection::SetChave`, 5482), ou para o mapa global de chaves antes da primeira seção.
   Redefinir uma chave atribui o novo valor. `CIniKey::operator=` lança 6005 se os nomes forem diferentes.
3. Comentários: primeiro `#` ou `;` não escapado (ou NUL). Trim: `isspace` ou `' '`/`'\t'`, mantendo um espaço final
   escapado. Escapes (tabela @1839288, aplicados em ordem com substituição de todas as ocorrências): `\n \t \; \: \= \# "\ " \[ \] \\`.

---

## 5. Outras funções externas na unidade

* **`api::CEncryptedFile::Save(path)`** (2767, cencryptedfile.cpp:88): nome vazio → 5994 "Nome de arquivo vazio";
  preenche o texto claro no estilo PKCS#7 até um múltiplo de 16, chama `ISymmetricCipher::Encrypt` (AES-256-CBC, que adiciona
  seu próprio bloco PKCS#7; u01), depois `CFile(path,"wb")` → `RawWrite` → `Flush` → `Sync` → `Close`. Para o RDV, a
  cifra vem de `GetCifradorCryptoTable`: a chave **e o IV** são derivados da semente HKDF, então o IV é fixo e
  nada é acrescentado (u01 §2.6, §3.1). Um IV aleatório acrescentado só é usado por cifras construídas sem IV (os arquivos
  de chave). Chamadores: funcs 5736/5737 (a partir de 6737 `CGeraDadosDinamicos`, durante `votaInit`: elas criam `rdv.dat`;
  são as chamadas vistas nas sessões gravadas, `analysis/runtime/*.edges.tsv`) e o gravador de RDV por voto
  7174, que salva em `<rdv>.tmp` (ver docs/modules/u07-uenux2-src-app-vota-eleitor.md).
* **`api::CFileASN::WriteToFile<T>(path, obj)`** (2892 + thunks 9985/10008/10040/10168 para
  `EstadoGeralVota/SA/Gap/Urna`): `CFile(path,"wb")` + codificação BER (`"WriteToFile de " + name` como string de
  contexto dos erros do codificador). Chamada pelo mock web (`cappinfobuilder.cpp`, func 2894) para criar `vota.bin`,
  `sa.bin`, `gap.bin`, `eg.bin` durante `votaInit`. A 2892 nunca chama `Close()` ela mesma: `~CFile` fecha o arquivo e
  engole qualquer exceção, então uma falha de `fsync` (1183) ou `fclose` (1179) nesses arquivos de estado nunca é reportada.
* **`api::CFileSeeder`** slots 2/8/9 (11434/11429/11428): `Eof()`, `size − Position()`, `Position()`. São usados pelo
  decodificador BER em streaming do arquivo de eleitores.
* **`api::CSystem::IsDirectory`** (5459) / `EhDoTipo` (6019): `stat` + teste de `S_IFMT` (`IsRegularFile` = func 412).
* **`comum::CGravadorEnvelopeArquivo::LeConteudo() const`** (11623): não recebe parâmetro (assinatura wasm `(sret, this)`;
  `IGravadorEnvelope::vf7` a chama como `call_indirect(out, this)`); o arquivo é o membro `m_arquivo` (+172).
  `GetFileSize` + `vector(size)` + `CFile(m_arquivo, "rb")` + `RawRead`. O número de bytes lidos é ignorado.
* **func 5371** (gravador de WSQ do mesário, nome desconhecido): deixa de salvar quando `statfs` informa < 5 MiB livres; escolhe o primeiro
  `me{:06}.wsq` livre (id vindo de uma `std::function`), grava-o com `CFile("w+b", FileMode 2)` e o copia para o diretório
  externo com `CSystem::CopyFile`.
* **11517 / 11523**: os corpos das `std::function` de `CEleitores::GetEleitoresImpedidos` / `GetEleitoresEstaticos`
  (leitura completa de `EntidadeImpedidos`; leitura BER parcial de `EntidadeEleitores` por meio de `CFileSeeder`). Elas são
  documentadas pelas u03/u05 e listadas aqui só porque inlinam `CFile`.
* **11905** `vota::CImpressaoVersaoPacotes::StartState`: imprime as versões dos pacotes a partir de `dadoscarga.dat`
  (`DadosDisponiveisCarga`), "{}ª via". **3691/11224**: destrutores de `CGeradorRelVersaoPacoteDados`.
* **9130 / 9221**: conversores de `ecourna::app::dados::asn` que constroem valores `CBaseType` (u14).

---

## 6. `CBaseType` (cbasetype.hpp:39)

`CBaseType(TYPE v)` armazena `v` **primeiro** e depois lança `CBaseError<EPatternError>` **1300**
`"O tipo '<name>' deve ter valores no intervalo [<MIN>,<MAX>]."` se `v` estiver fora do intervalo. Instâncias:

| func | TYPE, MIN..MAX, id | nome | corpo |
|---|---|---|---|
| 5682 | `ushort`, 1..99, 3 | CargoID | próprio |
| 9215 | `ushort`, 0..99, 4 | PartidoID | próprio |
| 2851 | `uint`, 0..99999, 6 | MunicipioID | → 6042 |
| 5850 | `ushort`, 0..9999, 7 | ZonaID | → 2299 |
| 5849 | `uint`, 0..9999, 8 | LocalID | → 6042 |
| 5848 | `ushort`, 0..9999, 9 | SecaoID | → 2299 |
| 1960 | `ushort`, 0..9999, 15 | QtdEleitor | → 2299 |
| 2277 | `ushort`, 0..9999, 36 | Ano | → 2299 |
| 2850 | `ushort`, 0..999, 38 | ScoreHabilitacao | → 2299 |
| 2666 | `ETipoIdentificadorEleitor`, 1..3, 40 | TipoIdentificadorEleitor | próprio |

2299/6042 são corpos de *merge-similar-functions* com os parâmetros `(this, v, &srcloc, &name.data, &name.size,
&name.flags, MAX, MAX+1)`. Só `v >= MAX+1` é testado, porque toda instância mesclada tem MIN 0 e um TYPE sem sinal.
As strings de nome são `std::string`s estáticas. Nomes com mais de 10 caracteres são montados na inicialização em
`__wasm_call_ctors`.

---

## 7. Dados lidos e gravados

| arquivo | por meio de | quem |
|---|---|---|
| `/dsk/{fi,fe}/dinamico/trab{1,2}/{eg,vota,gap,sa}.bin` | `CFileASN::WriteToFile` → `CFile "wb"` (+ `fsync` no fechamento) | mock web, `votaInit` (observado) |
| `<rdv>.tmp` / `rdv.dat` | `CEncryptedFile::Save` → `CFile "wb"` | gravador de RDV (observado na inicialização) |
| `etc/dependencias.properties`, `etc/versoes.properties` | leitor de INI (`CFile "rb"` + `ReadLine`) | `CGravaResultado` no encerramento |
| envelopes de chave (`/dsk/fi/estatico/chave/…`, p. ex. `cv.ber.pri`) | `DeserializeFromFile<EntidadeChave>` (2280) | carregadores de chave do BU, RC e WSQ (não incluídos no simulador, u01 §5) |
| `wsqbio.jez`, `wsqman.jez`, `wsqmes.jez` | `CZip` store | `CGravadorWSQ` no encerramento |
| `log.jez` (via `temp.jez` + rename) | `CZip` deflate | `CGravadorLog` no encerramento |
| `<wsq>/operador/me%06u.wsq` (+ cópia externa) | `CFile "w+b"` | func 5371 (biometria do mesário) |
| `dadoscarga.dat` | `CFileASN::ReadFromFile` | `CImpressaoVersaoPacotes` |

Tipos ASN.1 envolvidos: `ModuloEnvelopeChave::EntidadeChave` (lido), `ModuloEstadoGeral{Urna,Vota,Gap,SA}` (gravados),
`ModuloImpedidos::EntidadeImpedidos`, `ModuloEleitores::EntidadeEleitores`,
`ModuloDadosDisponiveisCarga::DadosDisponiveisCarga` (lidos, nas funções externas).

---

## 8. O que o analisador errou (para que leitores de outras unidades não sejam enganados)

| func | nome do analisador | realidade |
|---|---|---|
| 5480 | `CFile::ReadLine` | carregador de `api::CIniStrings` (uenux2) |
| 2767 | `CFile::Flush` | `api::CEncryptedFile::Save` |
| 5821 | `globToFileList` | rotina de compactação de `comum::CGravadorWSQ` |
| 2280 | `DeserializeFromBuffer` | `DeserializeFromFile` (função de buffer inlinada) |
| 5181 | `CFile::Sync` (srcloc também `CSynchronizer::CreateInst`) | correto; `CreateInst` está inlinado |
| 1397, 2234–2236, 3650, 3651, 5482, 5484, 5485, 6272 | "cfile.cpp" | helpers do leitor de INI / map da libc++ |
| 2154, 2573, 2574, 3372, 4787, 3334, 4679, 4685, 7745, 3527, 9531 | "icompressor.cpp"/"czip.cpp" | `<filesystem>` / vector da libc++ |
| 5459, 6019 | "icompressor.cpp" | helpers de `api::CSystem` |
| 3413, 4936, 4949, 5009, 2170 | "clzmacompress.cpp"/nenhum | internos do 7-Zip 19.00 |
| 11584 | `CGravadorLog::vf7` (candidato `CSystem::Rename`) | `CGravadorLog::GravaResultado(api::CFile&) const` |

---

## 9. BOLETIM DE URNA (BU): onde esta unidade está envolvida

Esta unidade não gera nem imprime o BU. Ela fornece:

1. **A camada de arquivos sob todo arquivo do BU.** `CGravadorBU::GravaResultado(api::CFile&) const` (func 11629,
   cgravadorbu.cpp:484) recebe, de acordo com sua assinatura, um `CFile` aberto no qual o envelope do BU é
   gravado. Toda escrita é um `RawWrite`
   (`fwrite` + `fflush`). Quando o arquivo é fechado, `Sync()` executa (`fflush` + `fsync`; o `CSynchronizer` vem ligado por padrão). Uma
   falha em qualquer ponto levanta um `CIoError` cujo texto informa a operação e `strerror(errno)`
   (p. ex. `"<name> size = 1234 [Write] - 28 - No space left on device"`).
2. **A leitura das chaves que protegem o BU.** `DeserializeFromFile<ModuloEnvelopeChave::EntidadeChave>` (2280) é
   o leitor usado por `vota::CGeraBU::StartState` (chave do *código verificador*, `cv.ber.pri`) e por
   `CGravadorBU::GravaResultado` (chave pública para a proteção CEPESC). Passos: `CFile(path, "rb")`, o arquivo
   inteiro em um `vector<char>`, `CoderEnv` com a regra **BER**, `decode` → **1221** "Falha ao fazer decode BER."
   se falhar, `isValid() && isStrictlyValid()` → **1222** "Foi lido conteúdo inválido." caso contrário. Um arquivo
   vazio falha antes, com 1192 "buffer invalido". A entidade então vai para `CKeyLoader::DecipherKeyIfNeeded` (u01 §5).
   Os arquivos de chave não são incluídos no simulador. A cópia do template no binário (2280) **não tem landing
   pads**: todas as suas chamadas são diretas, então em qualquer um desses erros o `CFile` nunca é fechado (seu `FILE*` vaza) e o
   buffer e o `CoderEnv` não são liberados. O linker manteve uma cópia COMDAT compilada em uma unidade de tradução da uenux2, que é
   compilada com a captura de exceções desabilitada (u11 §4.2).
3. **Não é o BU: `.jez`.** Os arquivos ZIP são logs e imagens de digitais. O BU (`*-bu.dat`), o RDV,
   a *imgbu* e os outros arquivos de resultado não são comprimidos por `CZip`.

---

## 10. Particularidades do build web

* **Nenhuma compressão executa durante um voto.** `CZip` só é alcançado quando os resultados são gravados no fim do
  dia da eleição, e `CLzmaCompress` é inalcançável em qualquer build deste binário.
* **`fsync`/`fcntl` não fazem nada.** O `fsync` do MEMFS não faz nada. O `___syscall_fcntl64` do glue retorna 0 para os cmds
  1/2 (`F_GETFD`/`F_SETFD`), então `SetFileMode` sempre "tem sucesso".
* **A verificação de espaço livre da func 5371 é sempre satisfeita.** O `statfs` do Emscripten retorna valores fixos
  (`bsize 4096 × bavail 500000` ≈ 1,9 GiB), seja qual for o armazenamento real do navegador.
* **Arquivos de estado de fixture.** As quatro instâncias de `WriteToFile<EstadoGeral*>` são acionadas pelo mock
  `uenux2/mock/app/comum/cappinfobuilder.cpp` (func 2894) durante `votaInit`, então o "estado da urna" vem de
  dados fixos no código. Suas assinaturas `.vsu` são a falsa `"assinatura simulada para vota_web_wasm"`
  (docs/00-provenance.md).
* `etc/versoes.properties` / `etc/dependencias.properties` são stubs de 0 bytes (§4).
* **Os horários nos `.jez`** vêm de `localtime`, ou seja, do fuso horário do navegador (`__localtime_js`).

---

## 11. Observações sobre WebAssembly / Emscripten

* **O inlining desloca os nomes.** A maioria das atribuições erradas da §8 vem do inlining: 5480, 2767, 5821 e 2280 são
  nomeadas a partir do `source_location` de uma chamada.
* **merge-similar-functions**: `CBaseType` (2299/6042 + thunks de 35–37 bytes). `CFileASN::WriteToFile<T>` (2892
  + thunks 9985/10008/10040/10168, e 2891 + 9684/9701/9703/9820): a chamada específica do tipo é passada como argumento
  de **slot da tabela**.
* **Eliminação de argumentos mortos**: `Executa7z` (3414) perdeu seu parâmetro `std::function`, mas os chamadores ainda copiam
  e destroem o functor em torno de cada chamada.
* **Exceções JS**: toda construção de `CIoError` nos métodos de `CFile` é um `invoke_iiiiiii(6210, …)` dentro do
  protocolo `__THREW__`. A exceção é `DeserializeFromFile` (2280), uma cópia COMDAT da uenux2 sem landing pads que
  chama a func 3520 diretamente. As funções da uenux2 da unidade (5480, 5821, 5371, 11623, 2767) em geral também chamam diretamente,
  e mantêm `invoke_*` só no código inlinado da ecourna. `~CFile` e `~CZip` capturam tudo
  (`__cxa_begin_catch`/`__cxa_end_catch`).
* **Literais de string inline** no pseudo-código aparecem no endereço errado (`d_operator…[442013]` está na verdade
  em @443037). Leia o `i32.const` em `q.py wat` quando uma string decodificada parecer errada.
* **Argumentos de formatação**: as chamadas de `std::format` sobrevivem como `__try_constant_folding` + `__vformat_to`; a string
  de formato é a constante i64 `{data,size}` (p. ex. `38654709059` = `{3395, 9}` = `"size = {}"`).
* `std::regex` usa o valor de flag 512 (`ECMAScript`, diferente de zero porque a ABI v2 da libc++ define
  `_LIBCPP_ABI_REGEX_CONSTANTS_NONZERO`); `regex_match` aparece como o helper de busca chamado com as flags `4160`.
* O destrutor de exclusão de `ICompressor` é o corpo compartilhado `unreachable` (325), como em toda classe abstrata.

---

## 12. Código estranho ou arriscado (resumo; detalhes na lista estruturada "suspicious")

| # | func | problema | severidade |
|---|---|---|---|
| 1 | 336 / 3564 | `CFile::Close` lança em caso de falha de `fclose` **sem zerar `m_file`**. `fclose` já liberou o `FILE`, então `~CFile` chama `Close` de novo: `fflush`/`fclose` sobre um `FILE*` liberado (use after free, double free) | média (só quando `fclose` falha) |
| 2 | 9508 | `SetFileMode` usa `F_GETFD`/`F_SETFD` em vez de `F_GETFL`/`F_SETFL`, então `O_SYNC`/`O_NOATIME` são descartados silenciosamente | baixa (só `O_NOATIME` é pedido; `Sync()` cobre a durabilidade) |
| 3 | 5480 (`ReadLine` inlinado) | uma última linha cujo comprimento é múltiplo de 127 bytes e que não tem `'\n'` final recebe os primeiros 126 bytes de seu último bloco acrescentados **uma segunda vez**: `fgets` deixa o buffer intocado no EOF, e `buffer[126]` foi zerado antes da chamada (o mesmo acontece se `fgets` falhar com um erro de leitura no meio de uma linha longa) | baixa |
| 4 | 5480 (fase 2) | o laço de parsing pode avançar seu iterador além de `end()` (continuação na última linha). Isso não pode acontecer com entrada vinda de arquivo: a fase 1 nunca produz uma linha que termine em um número ímpar de `\` | info |
| 4b | 2235 (`Trim`) | o trim à direita começa em `size() - 1` calculado sem sinal: para uma string vazia, lê o byte anterior ao buffer da string e continua varrendo para trás enquanto encontra espaços. O resultado ainda é `""`. Strings vazias chegam a ele em toda linha só de comentário | info |
| 5 | 5821 / icompressor.cpp:61 | a tradução glob→regex escapa apenas `* . ? \`. A parte do diretório fica dentro da regex. Um glob terminado em `/*` torna a expansão recursiva; a ordem dos arquivos é a ordem do diretório | baixa |
| 6 | 5821 | um `mkdir` dos diretórios de WSQ que falha é ignorado; quando todos os diretórios estão vazios, um `.jez` vazio ainda é produzido | baixa |
| 7 | 9537 | a divisão de progresso `(n << 10) / total` (`i64.div_u`) gera um trap (um `RuntimeError` do wasm, não uma exceção C++) se `total == 0` em uma fronteira de 1 MiB. `total` é o `file_size()` obtido antes de o arquivo ser aberto, então isso exige um arquivo que cresça de 0 bytes para ≥ 1 MiB enquanto está sendo adicionado. Um arquivo que cresce também faz a fração passar de 1024 | info |
| 8 | 11623, 2280 | os resultados de `RawRead` são ignorados (11623: uma leitura incompleta deixa bytes zero no final) / um arquivo de chave vazio é reportado como "buffer invalido" (2280), e a 2280 não tem landing pads, então todo caminho de erro vaza o `FILE*` aberto (§9) | baixa |
| 8b | 2892 | `WriteToFile(path, obj)` deixa o fechamento para `~CFile`, que engole exceções: falhas de `fsync`/`fclose` de `eg.bin`/`vota.bin`/`gap.bin`/`sa.bin` se perdem | baixa |
| 8c | 336 | se `Sync()` lançar dentro de `Close()`, `fclose` é pulado; a nova tentativa em `~CFile` engole uma segunda falha e o `FILE*` vaza | info |
| 9 | 2767 | preenchimento duplo (PKCS#7 manual + o da OpenSSL) em `CEncryptedFile::Save` | info |
| 10 | 5371 | a proteção de ≥ 5 MiB livres é sempre satisfeita no build web (`statfs` falso). Em uma urna real, um disco cheio faz com que a imagem da digital **não seja salva, silenciosamente** | baixa |
| 11 | 11584 | `CGravadorLog::GravaResultado` ignora o `api::CFile&` que recebe e renomeia `temp.jez` por cima do arquivo de resultado | info |
| 12 | 9586 et al. | cerca de 1.000 funções (344 KB) do 7-Zip estão linkadas, mas são inalcançáveis (`CLzmaCompress` nunca é construído) | info |

---

## 13. Tabela de mapeamento (todas as 98 funções da u12)

"✓" = vista executando nas sessões gravadas. Caminhos que começam com `src/` são os arquivos reconstruídos neste
repositório. Um caminho sem `src/` é um arquivo original que pertence a outra unidade.

| func | tamanho | executou | símbolo reconstruído | arquivo-fonte |
|---|---|---|---|---|
| 336 | 363 | ✓ | `ecourna::api::io::CFile::Close()` | src/ecourna/api/io/cfile.cpp |
| 418 | 385 | ✓ | `ecourna::api::io::CFile::Position() const` | src/ecourna/api/io/cfile.cpp |
| 419 | 385 | ✓ | `ecourna::api::io::CFile::Seek(int, int) const` | src/ecourna/api/io/cfile.cpp |
| 598 | 2641 | ✓ | `ecourna::api::io::CFile::RawRead(void*, uedword) const` | src/ecourna/api/io/cfile.cpp |
| 1397 | 107 |  | `std::__tree<pair<string, api::CIniKey>>::destroy(node)` (nós de 52 bytes) | biblioteca/helper inlinado: instância de `std::map` da libc++ do leitor de INI (ver src/uenux2/src/api/io/cinistrings.u12-fragment.cpp) |
| 1526 | 182 |  | `ecourna::api::pattern::IObservableProgressWithDescription::~IObservableProgressWithDescription()` (D1; também `~ICompressor`, slot 0) | src/ecourna/api/pattern/iobservableprogresswithdescription.hpp |
| 1885 | 269 | ✓ | `ecourna::api::io::CFile::Eof() const` | src/ecourna/api/io/cfile.cpp |
| 1886 | 1717 | ✓ | `ecourna::api::io::CFile::RawWrite(const void*, uedword) const` | src/ecourna/api/io/cfile.cpp |
| 1960 | 37 |  | `CBaseType<unsigned short, 0, 9999, 15>::CBaseType` ("QtdEleitor"; thunk -> 2299) | src/ecourna/api/pattern/cbasetype.hpp |
| 2154 | 63 |  | `std::shared_ptr<__dir_stream>::reset()` | biblioteca/helper inlinado: `<filesystem>` da libc++ (atribuído erroneamente a icompressor.cpp) |
| 2170 | 178 |  | `CObjectVector<CArcExtInfo / CProperty>::Add(const T&)` do 7-Zip ({UString, UString} de 24 bytes) | biblioteca/helper inlinado: 7-Zip 19.00 (alcançado só a partir do CLzmaCompress morto) |
| 2234 | 493 |  | `api::(anon)::FindNaoEscapado(const string&, const string&)` *(nome inferido)* | src/uenux2/src/api/io/cinistrings.u12-fragment.cpp (orig. uenux2/src/api/io/cinistrings.cpp) |
| 2235 | 403 |  | `api::(anon)::Trim(const string&)` (ciente de escapes) *(nome inferido)* | src/uenux2/src/api/io/cinistrings.u12-fragment.cpp |
| 2236 | 334 |  | `api::(anon)::RemoveComentario(const string&)` (corta em `#` `;` NUL não escapados) *(nome inferido)* | src/uenux2/src/api/io/cinistrings.u12-fragment.cpp |
| 2277 | 37 |  | `CBaseType<unsigned short, 0, 9999, 36>::CBaseType` ("Ano"; thunk -> 2299) | src/ecourna/api/pattern/cbasetype.hpp |
| 2280 | 624 |  | `ecourna::api::io::DeserializeFromFile<ModuloEnvelopeChave::EntidadeChave>(obj, fileName)` + `DeserializeFromBuffer<>` inlinado (linhas 44, 47) *(nome externo inferido)* | src/ecourna/api/io/asn/serialization.hpp |
| 2299 | 535 |  | corpo mesclado de `CBaseType<unsigned short, 0, MAX, ID>::CBaseType` (16 bits, MIN 0) | src/ecourna/api/pattern/cbasetype.hpp |
| 2573 | 50 |  | `std::deque<__dir_stream>::back()` (pilha do recursive_directory_iterator) | biblioteca/helper inlinado: `<filesystem>` da libc++ |
| 2574 | 10 |  | `std::filesystem::directory_iterator::__dereference()` | biblioteca/helper inlinado: `<filesystem>` da libc++ |
| 2666 | 870 |  | `CBaseType<ETipoIdentificadorEleitor, TipoIDPrimeiro, TipoIDNumeroLivre, 40>::CBaseType` ("TipoIdentificadorEleitor") | src/ecourna/api/pattern/cbasetype.hpp |
| 2690 | 145 |  | `ecourna::api::compression::CZip::~CZip()` (D1, slot 0) | src/ecourna/api/compression/czip.cpp |
| 2691 | 2393 |  | `ecourna::api::compression::CZip::Close()` (slot 2; `zipClose` inlinado) | src/ecourna/api/compression/czip.cpp |
| 2767 | 902 | ✓ | `api::CEncryptedFile::Save(const std::string&) const` (cencryptedfile.cpp:88) + `CFile::Flush` inlinado (cfile.cpp:145) | src/uenux2/src/api/io/cencryptedfile.u12-fragment.cpp (orig. uenux2/src/api/io/cencryptedfile.cpp); `CFile::Flush` em src/ecourna/api/io/cfile.cpp |
| 2850 | 35 |  | `CBaseType<unsigned short, 0, 999, 38>::CBaseType` ("ScoreHabilitacao"; thunk -> 2299) | src/ecourna/api/pattern/cbasetype.hpp |
| 2851 | 37 |  | `CBaseType<unsigned int, 0, 99999, 6>::CBaseType` ("MunicipioID"; thunk -> 6042) | src/ecourna/api/pattern/cbasetype.hpp |
| 2892 | 172 | ✓ | corpo mesclado de `api::CFileASN::WriteToFile<T>(const std::string&, const T&)` (CFile "wb" + callback do gravador) | src/uenux2/src/api/io/asn/cfileasn.u12-fragment.h (orig. uenux2/src/api/io/asn/cfileasn.h) |
| 3334 | 86 | ✓ | `std::filesystem::detail::posix_stat(const path&, StatT&, error_code*)` | biblioteca/helper inlinado: operations.cpp do `<filesystem>` da libc++ (atribuído erroneamente a czip.cpp) |
| 3372 | 244 |  | `std::filesystem::directory_iterator::__increment(error_code*)` | biblioteca/helper inlinado: `<filesystem>` da libc++ |
| 3413 | 126 |  | `CUpdateArchiveCommand::~CUpdateArchiveCommand()` do 7-Zip | biblioteca/helper inlinado: UI/Common do 7-Zip 19.00 (código morto) |
| 3414 | 3739 |  | `ecourna::api::compression::(anon)::Executa7z(ctx, arquivos, pacote, novosNomes, renomear, comprimir)` *(nome inferido)* | src/ecourna/api/compression/clzmacompress.cpp |
| 3520 | 1810 |  | `ecourna::api::io::CIoError::CIoError(EIoError, EFileOperation, const string&, int errnum, const source_location&)` | src/ecourna/api/io/cioerror.cpp (caminho inferido) |
| 3521 | 1180 |  | `static ecourna::api::io::CFile::ReadFileBinary(const std::filesystem::path&)` | src/ecourna/api/io/cfile.cpp |
| 3522 | 413 |  | `static ecourna::api::io::CFile::ReadFileContent(const std::string&)` | src/ecourna/api/io/cfile.cpp |
| 3527 | 104 |  | `std::vector<std::filesystem::path>::__destroy_vector::operator()` | biblioteca/helper inlinado: `std::vector` da libc++ |
| 3564 | 120 | ✓ | `ecourna::api::io::CFile::~CFile()` | src/ecourna/api/io/cfile.cpp |
| 3650 | 312 |  | `std::__tree<pair<string, api::CIniKey>>::__emplace_hint_unique_key_args` (map::insert(hint, v)) | biblioteca/helper inlinado: `std::map` da libc++ (cópia de CIniSection) |
| 3651 | 232 |  | `api::(anon)::Decodifica(const string&)` (comentário, trim, 10 escapes) *(nome inferido)* | src/uenux2/src/api/io/cinistrings.u12-fragment.cpp |
| 3691 | 168 |  | `comum::CGeradorRelVersaoPacoteDados::~CGeradorRelVersaoPacoteDados()` (D1) | uenux2/src/app/comum/relatorios/cgeradorrelversaopacotedados.cpp (outra unidade; trivial, não reconstruído) |
| 3797 | 82 |  | `std::__tree<pair<path, path>>::destroy(node)` (map de arquivos do CGravadorLog) | biblioteca/helper inlinado: `std::map` da libc++ |
| 3863 | 203 |  | `std::vector<(72-byte package record)>::__destroy_vector` (CDadosDisponiveisCarga) | biblioteca/helper inlinado: `std::vector` da libc++ |
| 4679 | 344 |  | `std::filesystem::detail::ErrorHandler<uintmax_t>::report(const error_code&)` | biblioteca/helper inlinado: `<filesystem>` da libc++ |
| 4685 | 270 | ✓ | `std::filesystem::detail::create_file_status(...)` | biblioteca/helper inlinado: `<filesystem>` da libc++ |
| 4787 | 13 |  | `std::filesystem::recursive_directory_iterator::__dereference()` | biblioteca/helper inlinado: `<filesystem>` da libc++ |
| 4936 | 165 |  | `NWildcard::CCensor::~CCensor()` do 7-Zip | biblioteca/helper inlinado: Common/Wildcard do 7-Zip 19.00 (código morto) |
| 4937 | 1038 |  | `std::default_delete<(anon)::CContexto7z>` (contexto do 7-Zip de 568 bytes: destrutor + free) *(nome inferido)* | src/ecourna/api/compression/clzmacompress.cpp |
| 4949 | 294 |  | `CPercentPrinter::~CPercentPrinter()` do 7-Zip (ClosePrint inlinado) | biblioteca/helper inlinado: UI/Console do 7-Zip 19.00 (código morto) |
| 5009 | 606 |  | `SplitString(const UString&, UStringVector&)` do 7-Zip (LoadCodecs.cpp) | biblioteca/helper inlinado: 7-Zip 19.00 (código morto) |
| 5180 | 36 |  | `ecourna::api::io::CFile::RawWrite(const std::vector<uebyte>&) const` *(nome inferido)* | src/ecourna/api/io/cfile.hpp |
| 5181 | 2349 | ✓ | `ecourna::api::io::CFile::Sync() const` (+ `util::CSynchronizer::GetInst/CreateInst` inlinados) | src/ecourna/api/io/cfile.cpp |
| 5192 | 205 |  | `ecourna::api::compression::ICompressor::ICompressor()` | src/ecourna/api/compression/icompressor.cpp |
| 5371 | 1566 |  | `comum::SalvaWsqMesario(...)` ("me{:06}.wsq", w+b, >= 5 MiB livres, int -> cópia ext) *(nome, classe, arquivo inferidos; baixa)* | src/uenux2/src/app/comum/reconhecimentobiometrico/u12-foreign-fragments.cpp |
| 5459 | 11 |  | `api::CSystem::IsDirectory(const std::string&)` *(nome inferido)* | src/uenux2/src/api/util/csystem.u12-fragment.cpp (orig. uenux2/src/api/util/csystem.cpp) |
| 5480 | 8780 |  | `api::CIniStrings::CIniStrings(const std::string&)` carregador de INI (+ `CFile::ReadLine` inlinado cfile.cpp:336/356, `CIniStrings::ParseKeyLine` cinistrings.cpp:183/187) *(nome inferido)* | src/uenux2/src/api/io/cinistrings.u12-fragment.cpp; `CFile::ReadLine` em src/ecourna/api/io/cfile.cpp |
| 5482 | 512 |  | `api::CIniSection::SetChave(const CIniKey&)` (insere ou atribui) *(nome inferido)* | src/uenux2/src/api/io/cinistrings.u12-fragment.cpp (orig. provavelmente uenux2/src/api/io/cinisection.cpp) |
| 5484 | 131 |  | `api::(anon)::TerminaComContinuacao(const string&)` *(nome inferido)* | src/uenux2/src/api/io/cinistrings.u12-fragment.cpp |
| 5485 | 110 |  | `api::(anon)::EhLinhaUtil(const string&)` *(nome inferido)* | src/uenux2/src/api/io/cinistrings.u12-fragment.cpp |
| 5573 | 16 |  | destrutor de `__exception_guard<vector<string>::__destroy_vector>` | biblioteca/helper inlinado: `std::vector` da libc++ |
| 5585 | 159 | ✓ | `std::vector<std::string>::__init_with_size(first, last, n)` | biblioteca/helper inlinado: `std::vector` da libc++ (também usado por código ativo) |
| 5682 | 558 |  | `CBaseType<unsigned short, 1, 99, 3>::CBaseType` ("CargoID") | src/ecourna/api/pattern/cbasetype.hpp |
| 5821 | 8686 |  | `comum::CGravadorWSQ::CompactaWsq(bool externo, const std::string&) const` (+ `GetCaminhoCorretoInternal/External` inlinados cgravadorwsq.cpp:175/192, `ICompressor::Add(path)`, `globToFileList` icompressor.cpp:61) *(nome inferido)* | src/uenux2/src/app/comum/gravadores/u12-foreign-fragments.cpp (orig. uenux2/src/app/comum/gravadores/cgravadorwsq.cpp); partes da ecourna em src/ecourna/api/compression/icompressor.cpp |
| 5848 | 37 |  | `CBaseType<unsigned short, 0, 9999, 9>::CBaseType` ("SecaoID"; thunk -> 2299) | src/ecourna/api/pattern/cbasetype.hpp |
| 5849 | 37 |  | `CBaseType<unsigned int, 0, 9999, 8>::CBaseType` ("LocalID"; thunk -> 6042) | src/ecourna/api/pattern/cbasetype.hpp |
| 5850 | 37 |  | `CBaseType<unsigned short, 0, 9999, 7>::CBaseType` ("ZonaID"; thunk -> 2299) | src/ecourna/api/pattern/cbasetype.hpp |
| 6019 | 43 | ✓ | `api::(anon)::EhDoTipo(const string&, unsigned)` *(nome inferido)* | src/uenux2/src/api/util/csystem.u12-fragment.cpp |
| 6042 | 535 |  | corpo mesclado de `CBaseType<unsigned int, 0, MAX, ID>::CBaseType` (32 bits, MIN 0) | src/ecourna/api/pattern/cbasetype.hpp |
| 6272 | 13 |  | `api::(anon)::EhBrancoExtra(int)` (espaço ou tab) *(nome inferido)* | src/uenux2/src/api/io/cinistrings.u12-fragment.cpp |
| 7745 | 11 |  | `std::filesystem::status(const path&, error_code*)` (-> 1055 -> 3334) | biblioteca/helper inlinado: `<filesystem>` da libc++ |
| 8376 | 1611 |  | `ecourna::api::compression::(anon)::Adiciona7z(...)` *(nome inferido)* | src/ecourna/api/compression/clzmacompress.cpp |
| 9130 | 62 |  | `ecourna::app::dados::asn::CConversorMunicipioZona::DoDeconverte` (slot 3) | ecourna-lib/ecourna/app/dados/asn/... (unidade u14; não reconstruído aqui) |
| 9215 | 869 |  | `CBaseType<unsigned short, 0, 99, 4>::CBaseType` ("PartidoID") | src/ecourna/api/pattern/cbasetype.hpp |
| 9221 | 104 |  | `ecourna::app::dados::asn::CConversorDetalhamentoComparecimento::DoDeconverte` (slot 3) | ecourna-lib/ecourna/app/dados/asn/... (unidade u14; não reconstruído aqui) |
| 9508 | 1152 | ✓ | `ecourna::api::io::CFile::Open(const string&, const string&, FileMode)` (+ `SetFileMode` inlinado, linha 117) | src/ecourna/api/io/cfile.cpp |
| 9510 | 974 |  | `ecourna::api::compression::CZip::ErrorToString(int)` *(nome inferido)* | src/ecourna/api/compression/czip.cpp |
| 9530 | 324 |  | `ecourna::api::compression::ICompressor::Add(const std::vector<std::filesystem::path>&)` | src/ecourna/api/compression/icompressor.cpp |
| 9531 | 593 |  | `std::vector<std::filesystem::path>::__init_with_size(first, last, n)` | biblioteca/helper inlinado: `std::vector` da libc++ |
| 9532 | 799 |  | lambda por entrada de `globToFileList` (is_regular_file && regex_match -> push_back) | src/ecourna/api/compression/icompressor.cpp |
| 9537 | 793 |  | `ecourna::api::compression::CZip::NotificaProgresso(uint64, uint64, const string&)` *(nome inferido)* | src/ecourna/api/compression/czip.cpp |
| 9538 | 6329 |  | `ecourna::api::compression::CZip::DoAdd(const path&, const path&)` (slot 4; + `AssertZipIsOpened` inlinado, linha 188) | src/ecourna/api/compression/czip.cpp |
| 9539 | 269 |  | `ecourna::api::compression::CZip::ConvertToCompressLevel() const` | src/ecourna/api/compression/czip.cpp |
| 9540 | 148 |  | `ecourna::api::compression::CZip::~CZip()` (D0, slot 1) | src/ecourna/api/compression/czip.cpp |
| 9542 | 52 |  | `ecourna::api::compression::CZip::FechaSemExcecao()` (`try { Close(); } catch (...) {}`) *(nome inferido)* | src/ecourna/api/compression/czip.cpp |
| 9581 | 35 |  | `ecourna::api::compression::CLzmaCompress::~CLzmaCompress()` (D0, slot 1) | src/ecourna/api/compression/clzmacompress.cpp |
| 9582 | 32 |  | `ecourna::api::compression::CLzmaCompress::~CLzmaCompress()` (D1, slot 0) | src/ecourna/api/compression/clzmacompress.cpp |
| 9584 | 18 |  | thunk `CBaseError<ECompressionError, {1000,1100}>::CBaseError(code, msg, loc)` (-> 1011, vtable @1110656) | biblioteca/helper inlinado: header de exceções da ecourna (não reconstruído) |
| 9586 | 6423 |  | `ecourna::api::compression::CLzmaCompress::DoAdd(const path&, const path&)` (slot 4, linha 114; inicialização do 7-Zip inlinada) | src/ecourna/api/compression/clzmacompress.cpp |
| 9985 | 12 | ✓ | thunk `api::CFileASN::WriteToFile<ModuloEstadoGeralVota::EstadoGeralVota>` | src/uenux2/src/api/io/asn/cfileasn.u12-fragment.h |
| 10008 | 12 | ✓ | thunk `api::CFileASN::WriteToFile<ModuloEstadoGeralSA::EstadoGeralSA>` | src/uenux2/src/api/io/asn/cfileasn.u12-fragment.h |
| 10040 | 12 | ✓ | thunk `api::CFileASN::WriteToFile<ModuloEstadoGeralGap::EstadoGeralGap>` | src/uenux2/src/api/io/asn/cfileasn.u12-fragment.h |
| 10168 | 12 | ✓ | thunk `api::CFileASN::WriteToFile<ModuloEstadoGeralUrna::EstadoGeralUrna>` | src/uenux2/src/api/io/asn/cfileasn.u12-fragment.h |
| 11224 | 13 |  | `comum::CGeradorRelVersaoPacoteDados::~CGeradorRelVersaoPacoteDados()` (D0) | uenux2/src/app/comum/relatorios/cgeradorrelversaopacotedados.cpp (outra unidade; trivial) |
| 11428 | 10 | ✓ | `api::CFileSeeder::posicao()` (slot 9) *(nome inferido)* | src/uenux2/src/api/io/asn/cpartialfileasn.u12-fragment.h (orig. uenux2/src/api/io/asn/cpartialfileasn.h) |
| 11429 | 16 |  | `api::CFileSeeder::restante()` (slot 8) *(nome inferido)* | src/uenux2/src/api/io/asn/cpartialfileasn.u12-fragment.h |
| 11434 | 10 | ✓ | `api::CFileSeeder::fim()` (slot 2) *(nome inferido)* | src/uenux2/src/api/io/asn/cpartialfileasn.u12-fragment.h |
| 11517 | 3826 | ✓ | `std::function<CEleitores::GetEleitoresImpedidos::$_0>::operator()` (CFileASN::ReadFromFile<EntidadeImpedidos> inlinado) | uenux2/src/app/comum/dados/celeitores.cpp (unidade u05: src/uenux2/src/app/comum/dados/u05-foreign-fragments.cpp) |
| 11523 | 4145 | ✓ | `std::function<CEleitores::GetEleitoresEstaticos::$_0>::operator()` (CPartialFileASN::ReadFromFile<EntidadeEleitores> inlinado) | uenux2/src/app/comum/dados/celeitores.cpp (unidades u03/u05) |
| 11584 | 4385 |  | `comum::CGravadorLog::GravaResultado(api::CFile&) const` (slot 7; ignora o CFile; temp.jez + CZip + rename) | src/uenux2/src/app/comum/gravadores/u12-foreign-fragments.cpp (orig. uenux2/src/app/comum/gravadores/cgravadorlog.cpp) |
| 11623 | 226 |  | `std::vector<uebyte> comum::CGravadorEnvelopeArquivo::LeConteudo() const` (slot 8; lê `m_arquivo` +172) *(nome inferido)* | src/uenux2/src/app/comum/gravadores/u12-foreign-fragments.cpp (orig. .../gravadores/cgravadorenvelopearquivo.cpp) |
| 11905 | 5098 |  | `vota::CImpressaoVersaoPacotes::StartState()` (slot 2; imprime as versões dos pacotes a partir de dadoscarga.dat) | uenux2/src/app/vota/.../cimpressaoversaopacotes.cpp (outra unidade; não reconstruído) |

Outras funções usadas neste doc, mas fora da u12: 517 (`CFile::CFile`, u18), 5193/9543/9529 (`CZip::CZip`,
`CreateZipFile`, `ICompressor::Add(map)`, u15), 1143 (ctor de `CBaseError<EIoError>`), 1011 (corpo compartilhado de `CBaseError`),
2891 + 9684/9701/9703/9820 (corpo mesclado de `CFileASN::WriteToFile(CFile&, T)` + thunks), 9673/9695/9702/9801
(`CodeObjectFunction<EstadoGeral*>`), 9585 (`CLzmaCompress::OnProgress`), 8118/8117/8119 (minizip),
8434 (`UpdateArchive`), 11579 (`CGravadorWSQ::vf2`), 6041, 11629 (`CGravadorBU::GravaResultado`),
12098 (`CGravaResultado::StartState`), 12110 (`CGeraBU::StartState`), 2725 (o outro gravador de WSQ), 412 (`CSystem::IsRegularFile`),
2894 (gravador de estado do mock).

---

## 14. Questões em aberto

* Os nomes reais dos enumeradores de `CFile::FileMode`, do slot 3 de `ICompressor` e de `EIoError` / `ECompressionError`.
  Só os números são conhecidos. Para o slot 3, a unidade u15 (`src/ecourna/api/compression/czip.u15.hpp`) propõe
  `IsOpen()`. Seu único ponto de chamada é o `ICompressor::Add(path)` inlinado, onde ele decide se um glob é
  expandido (CZip → true, CLzmaCompress → false). Um significado de "está aberto" tornaria esse desvio inútil, então esta
  unidade o chama de `ExpandeGlob()`. Ambos os nomes são inferidos.
* Se a func 5480 é o construtor de `CIniStrings` ou um `Load(path)` estático que retorna por valor (a ABI é
  a mesma). Onde fica `CIniSection::SetChave` (5482) (`cinisection.cpp` é provável).
* O dono da func 5371 (gravador de WSQ do mesário): `CControlaArmazenamentoDeImagens` (seu `GerarCaminhosUnicos`
  se parece com esse laço) ou uma classe de `comparecimentomesario`.
* O significado dos dois bytes de opção que `Adiciona7z` (8376) armazena no contexto +560 (`{1,0}`, `{2,0}`, `{2,0x32}`).
* Se o build da urna real linka o mesmo `CFile::SetFileMode`. Se linkar, `O_SYNC` também nunca tem efeito
  lá, e a durabilidade depende do `fsync` em `Sync()`.
