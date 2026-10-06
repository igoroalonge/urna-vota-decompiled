# u18 — uenux2 `api/io` (arquivos ASN.1, arquivo RDV cifrado, chaves INI, tabelas em memória), `api/ipc` (threads, filas de mensagens, locks POSIX), `api/hwil/ipower.h` + funções avulsas do VOTA

A unidade u18 tem 95 funções. Elas vêm de três grupos:

1. **Infraestrutura `src/api` do uenux2**, sobre a qual toda a aplicação de votação se apoia:
   * `api/io/asn/cfileasn.h`: leitura e gravação de arquivos BER (`CFileASN`);
   * `api/io/asn/cpartialfileasn.h`: o seeder baseado em arquivo do decodificador BER em streaming (`CFileSeeder`);
   * `api/io/cdatamap.h`: as tabelas em memória com cursor (`CDataMap`);
   * `api/io/cencryptedfile.cpp`: a cópia de trabalho cifrada do RDV (`CEncryptedFile`);
   * `api/io/cinikey.cpp`: chaves INI/properties (`CIniKey`);
   * `api/ipc/*`: threads (`CThread`), filas de mensagens entre threads (`CPriorityMessageQueue`) e wrappers POSIX de mutex, rw-lock e semáforo;
   * `api/hwil/ipower.h`: os corpos padrão da interface de energia/bateria (`IPower`).
2. **Funções nomeadas segundo um template inlinado.** O analisador nomeou 20 funções da aplicação segundo um srcloc de `cfileasn.h` ou `cdatamap.h` porque o template foi inlinado nelas. Seus donos reais são os gravadores de resultado (`CGravadorRDV`, `IGravadorEnvelope`), os carregadores de eg.bin, `-lo.dat` e `-cp.dat`, o registro de comparecimento dos mesários e alguns helpers atribuídos erroneamente (construtores de `CUrna`, codificador hex).
3. **Cola de estados e threads do VOTA** que foi parar na unidade por proximidade no grafo de chamadas. São em sua maioria métodos `StartState`/`ProcessMessage` de uma linha que postam mensagens entre o terminal do eleitor e o terminal do mesário, mais as políticas de execução da urna e da web.

Arquivos-fonte reconstruídos (todos escritos pela u18):

| arquivo | conteúdo |
|---|---|
| `src/uenux2/src/api/io/asn/cfileasn.h` | `api::CFileASN` (todos os membros template, códigos de erro, lista de instâncias) |
| `src/uenux2/src/api/io/asn/cfileasn.instances.cpp` | as instâncias que sobrevivem como funções (instanciações explícitas) |
| `src/uenux2/src/api/io/asn/cpartialfileasn.h` | `api::CFileSeeder` (completo; a u12 tinha 3 slots em `cpartialfileasn.u12-fragment.h`) |
| `src/uenux2/src/api/io/cdatamap.h` | `api::CDataMap<K, R>` |
| `src/uenux2/src/api/io/cencryptedfile.h/.cpp` | `api::CEncryptedFile` (Load, MemRead, MemWrite; Save está no fragmento da u12) |
| `src/uenux2/src/api/io/cinikey.h/.cpp` | `api::CIniKey` |
| `src/uenux2/src/api/ipc/cmessagequeue.h` | `api::SMessage`, `api::CMessageInterface`, `api::CPriorityMessageQueue<T>` |
| `src/uenux2/src/api/ipc/cthread.h/.cpp` | `api::CThread` |
| `src/uenux2/src/api/ipc/posix/cposix{mutex,rwmutex,semaphore}.h/.cpp` | os três wrappers POSIX |
| `src/uenux2/src/api/hwil/ipower.h` | `api::IPower` |
| `src/uenux2/src/app/comum/u18-foreign-fragments.cpp` | `GravaResultado` de CGravadorRDV / IGravadorEnvelope, ctors/dtor de `md::CUrna`, cópia de `CCarga`, CLocal::Carrega, carregador do eg.bin, carregadores de processo eleitoral / pleito, CRegistradorMesario, a fonte de texto DSNumero, codificador hex do ecourna |
| `src/uenux2/src/app/vota/u18-foreign-fragments.cpp` | singletons de thread, políticas de execução, os enviadores de mensagens do eleitor/operador |

---

## 1. Classes e como se relacionam (RTTI)

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

Classes de erro usadas pela unidade (os thunks 210, 1889 e 1229 as constroem; são construtores de exceção do TSE mesclados
que recebem a vtable como parâmetro, ver `docs/libraries/libcxx-core.md` §1.2):

| alias | classe (RTTI) | limites |
|---|---|---|
| `api::CUeIoError` | `CBaseError<api::EUeIoError, SErrorLimits{5950, 6150}>` (typeinfo @1528172) | io |
| `api::CUeIpcError` | `CBaseError<api::EUeIpcError, SErrorLimits{6150, 6350}>` (@1528380) | ipc |
| `api::CUeHwilError` | `CBaseError<api::EUeHwilError, SErrorLimits{5150, 5950}>` (@1531040) | hwil |

---

## 2. Subsistemas e fluxo de controle

### 2.1 `api::CFileASN`: arquivos ASN.1 (`cfileasn.h`)

Templates só em header. Tudo foi inlinado, mas os registros `std::source_location` preservam os números de linha:

| linha | membro | código | mensagem |
|---|---|---|---|
| 48 | `ReadFromFile(const CFile&)` | 5950 | `O arquivo [{}] não estava aberto` |
| 60 | idem | 5951 | `O arquivo [{}] é muito grande para ser lido` (tamanho > 5 MiB = 5 242 880) |
| 78 | `ReadFromFile(const std::string&)` | 5952 | `O arquivo [{}] não existe` (`CSystem::IsRegularFile`, func 412) |
| 135 | `DecodeObjectFunction` | 5953 | `Conteúdo não foi decodificado para {}: {}` |
| 143 | idem | 5954 | `Conteúdo inválido para {}: {}` (exige `isValid() && isStrictlyValid()`) |
| 161 | `CodeObjectFunction` | 5955 | `Objeto com conteúdo inválido para {}: {}` (apenas `isValid()`) |
| 171 | idem | 5956 | `Arquivo não foi codificado para ` + contexto |

O segundo `{}` é a saída de `ASN1::trace_invalid` (o InvalidTracer em português do TSE). O primeiro é uma
string de contexto: `"ReadFromFile de " + file name`, `"DecodeObject de " + typeid(T).name()`,
`"CodeObject de " + typeid name` ou `"WriteToFile de " + file name`. O nome de tipo mangled aparece nas
mensagens de erro (por exemplo `DecodeObject de N25ModuloRegistroDigitalVoto27EntidadeRegistroDigitalVotoE`).

Caminho de leitura: `IsRegularFile` → `CFile(name, "rb")` → `Position` / `Seek(END)` / `Position` / `Seek(back)` → verificação
de tamanho → `std::vector<char>(size)` → `RawRead` (o resultado é ignorado) → `CoderEnv{ber}.decode` → validade →
`Close`. Caminho de gravação: `isValid` → `CoderEnv{ber}.encode` (ou `encodeBER`) para um vetor novo → swap →
`CFile::RawWrite`.

Dois helpers no nível do conversor não têm srcloc próprio. Seus nomes são inferidos:
`ReadDataFromFile<TConversor>(name)` (lê a entidade e depois chama `Desconverte`) e
`WriteDataToFile<TConversor>(CFile&, dado)` (`Converte` e depois grava). Ambos criam o conversor como variável local, e
é por isso que a vtable do conversor fica armazenada no frame da função chamada. O
`Converte`/`Desconverte` de um conversor do comum acrescenta EUeComumAsnError 7653 `Entidade deixada em estado inválido: {}` (iconversorasn.h:56)
ou 7654 `Entidade está inválida: {}` (:71). Um conversor do ecourna acrescenta EApiAsnError 1900/1902 (iconversorasn.hpp:49/66).

Quais arquivos ASN.1 passam pelas funções desta unidade:

| wasm | direção | entidade | arquivo |
|---|---|---|---|
| 9673 / 9695 / 9702 / 9801 (→ 2927) | gravação | EstadoGeralVota / SA / Gap / Urna | `dinamico/trab{1,2}/vota.bin`, `sa.bin`, `gap.bin`, `eg.bin` (fixtures gravadas no votaInit, u12) |
| 2724 (→ 6006) | gravação | EntidadeEnvelopeGenerico | `-bu.dat` (CGravadorBU 11629), `-imgbu.dat`, `-imgze.dat` (11626), func 2725 |
| 5367 (→ 6006) | gravação | EntidadeHashes | `-hash.dat` (CGravadorHashes 11620) |
| 5366 | gravação | EntidadeResultadoUrnaCadastro (ecourna) | `-jufa.dat` (CGravadorRCSecao 11616) |
| 11587 (inlinada) | gravação | EntidadeResultadoRDV | `-rdv.dat` |
| 5825 | decodificação | EntidadeRegistroDigitalVoto | o BER do RDV (conteúdo de rdv.dat, e decodificado novamente pela 11587) |
| 3791 | leitura | EstadoGeralUrna → CEstadoGeral | `eg.bin` (IServicoEstado<CEstadoGeral>) |
| 3771 | leitura | EntidadeProcessoEleitoral → CProcessoEleitoralDTO | `<fase><processo:05>-cp.dat` (por exemplo `t02400-cp.dat`) |
| 5778 | leitura | EntidadeSituacoesEleicoes, EntidadeEleicao | `<fase><pleito:05><uf>-ste.dat`, `<fase><eleição:05><uf>-ce.dat` |
| 5706 | leitura | CabecalhoPacote → md::CCabecalhoPacote | `...-ce.pid` (cabeçalho do pacote: a versão dos dados de cada eleição) e o carregador 7787 |
| 5740 | leitura | ModuloLocal::Local → md::CLocal | `<município:05><zona:04><seção:04>-lo.dat` |
| 3735 | leitura | InformacaoMidia (ecourna) | `/dsk/fe/estatico/infomidia.dat` (serial da MV), **ausente no simulador** |

### 2.2 `api::CFileSeeder` (`cpartialfileasn.h`)

É o `ASN1::ISeeder` sobre um `CFile` aberto usado pelo decodificador BER "parcial" (regra 2 do CoderEnv). Esse decodificador
lê o cadastro de eleitores `-el.dat` um eleitor por vez. Membros: `+4 const CFile*`, `+8 size_t m_tamanho` (fim
dos dados legíveis). Slots: `fim()` = `CFile::Eof()` (`feof` do stdio); `decodeByte` (:52, 5957); `decodeBlock`
(:75, 5958, limita a `m_tamanho`); `seek` (:88 5959 além do fim, :94 5960 se `fseek` falha); `skip` (:106,
5961, depois `seek`); `restante()`, `posicao()`. decodeByte, decodeBlock e seek foram **observadas em execução** durante
o votaInit.

### 2.3 `api::CDataMap<K, R>` (`cdatamap.h`)

28 bytes: `+0 std::map<K,R>`, `+12 iterator m_atual` (== end quando não há registro corrente), `+16 std::string
m_nome`. Membros atestados por srcloc: `GetCurrent()` (:98, 5976 `O registro corrente estava inválido {}`), `Next()`
(:117, 5977 `Operação inválida {}`), `Add(value_type)` (:143, 5978 `Duplicidade de registro {}`; o novo registro
se torna o corrente), `Update(key, record)` (:158, 5979 `Registro não encontrado {}`; o registro se torna o corrente).
O wasm-opt mesclou GetCurrent na 2919 e Next na 3892. A única coisa que difere entre as instâncias é o offset
do valor dentro do nó da árvore (cabeçalho de 16 bytes + tamanho da chave) e o srcloc. As instâncias estão listadas em §6.

`comum::CRegistradorMesario` (a func 815 constrói seu singleton de 52 bytes) é a tabela de comparecimento de mesários
(o registro de quais mesários estiveram presentes). Ela contém
`CDataMap<CComparecimentoMesarioPK, CComparecimentoMesario>` (+0), um segundo map indexado pela
identidade do mesário (+28) e um `servico::CComparecimentoMesarioServico` (+40) que contém o DAO do SQLite. A func 5378 é sua
etapa de gravação: ela indexa o registro pela identidade e depois chama `Update` se a PK existe ou `Add` se não existe. O chamador então executa
`SaveCurrentInternal()` (cregistradormesario.cpp:85, "O contêiner estava vazio" se a tabela estiver vazia), que
grava o registro corrente na tabela SQLite `comparecimento_mesario` de `uenux.db`.

### 2.4 `api::CEncryptedFile` (`cencryptedfile.cpp`): a cópia de trabalho do RDV

O RDV (*Registro Digital do Voto*, o registro digital de cada voto depositado) é mantido nas duas memórias flash
(MI = interna, MV = a memória de votação) como `dinamico/trab{1,2}/rdv.dat`. Esse arquivo é cifrado:
`MemWrite(CRdvVota::Converte())` → `Save(".tmp")` (u12: padding semelhante a PKCS#7 até 16 bytes, depois
`ISymmetricCipher::Encrypt`). Depois da gravação, ele é lido de volta e comparado: `Load(".tmp")` → `MemRead` →
`CRdvVota::ConfereConteudo` (u07 §4.2). Na reinicialização ele é lido de novo: `CEleitores::CompleteLoad` (u04) → `Load` →
`MemRead` → `CRdvVota::Desconverte`.

Load (func 3653, linhas 36-73):

1. nome vazio → lança 5987; `!IsRegularFile` → lança 5988 `Arquivo [{}] não existe`;
2. `tamanho = GetFileSize`; `tamanho == 0 || tamanho % 16` → **5989 construído mas não lançado**;
3. `CFile(name, "rb")`, `RawRead(tamanho)`; leitura curta → **5990 construído mas não lançado**. (Um arquivo de 0 bytes ainda é
   rejeitado, uma camada abaixo: o vetor vazio tem um buffer nulo e `CFile::RawRead(nullptr, 0)` lança
   `ecourna::api::io::CIoError` 1192 `buffer invalido`.)
4. `m_cifrador->Decrypt(cifrado, claro)` (slot 3 da vtable);
5. `pad = claro.back()`; `pad > 16` → lança 5991; `pad > tamanho` (o tamanho do arquivo cifrado) → lança 5992;
6. cada um dos `pad-1` bytes antes do último deve ser igual a `pad`, senão **5993 construído mas não lançado**;
7. `m_dados = claro[0 .. size-pad)`, `m_posicao = 0`. Um byte de pad igual a 0 é aceito.

MemWrite/MemRead (linhas 142-167) copiam para dentro e para fora de `m_dados` na posição do cursor. As sobrecargas com vetor (2766 e 3652)
são as funções wasm. MemWrite ignora um vetor de entrada vazio. MemRead não faz nada quando o cursor já está no
fim; caso contrário, acrescenta os bytes restantes ao vetor de saída (o novo espaço é pré-preenchido com 0xFF e depois
sobrescrito).

### 2.5 `api::CIniKey` (`cinikey.cpp`)

`{+0 nome, +12 valor}`. `operator=` (5483, :24) se recusa a mudar o nome (6005 `O nome de uma chave não pode
ser modificado.`). É usado pelo parser de INI (u12) para `/etc/dependencias.properties` e `/etc/versoes.properties`.

### 2.6 Threads e mensagens (`api/ipc`)

* **`CThread`** (3598): `+4 m_estado` (0 parada, 1 em execução, 3 terminada), `+8 m_bParar`, `+9 m_bDormindo`,
  `+12 unique_ptr<IThreadImpl>` vindo de `IGenericFactory<IThreadImpl>` (cthread.cpp:31, CWasmThread no build web)
  e `+16 unique_ptr<ISyncCtl>` (:32, CPosixMutex). `Start` (1684, :68) define o estado 1 e chama
  `IThreadImpl::Create(ThreadProc = func 10255, this)`. Lança 6231 `Nao foi especificado o tipo de implementação`
  quando não há implementação. `Wait` (1900) faz o join e reinicia o estado para 0. `ThreadProc` executa `Run()` e
  define o estado 3.
* **`CPriorityMessageQueue<SMessage>`**: em `+4`, uma `std::priority_queue` de entradas de 16 bytes `{SMessage (8), int
  prioridade, unsigned sequencia}`. Ordem: maior prioridade primeiro, FIFO dentro de uma mesma prioridade. Depois `+20
  unique_ptr<ISemaphore>` (cmessagequeue.h:113) e `+24 unique_ptr<ISyncCtl>` (:114), e em `+28` o contador de
  sequência, que é zerado quando a fila se esvazia. `Add` (rhvoice_f501, atribuída erroneamente) trava, insere e sinaliza o
  semáforo. `Remove` (2073, :153) trava e lança 6228 `A fila estava vazia.` se a fila estiver vazia; caso contrário,
  retira e destrava. `Recebe(timeout)` (slot 2, 4343) espera no semáforo e, com resultado 0, chama `Remove()`.
  Armazena a mensagem e o resultado na parte `CMessageInterface` (+36/+44).
* **Tráfego de mensagens nesta unidade** (todas com prioridade 1). A fila do terminal do eleitor é `CThreadEleitor+36`. Seu
  primeiro estado, `CAguardaMensagem`, reage a 1, 7, 11, 12 e 14 (u02/u07). A fila do operador é
  `CThreadOperador+36`:

| enviador (wasm) | fila | id | significado |
|---|---|---|---|
| `CControladorRegistraMesariosVota` slot 10 (10792 → corpo mesclado 6018), chamado por `CEncerraRegistroMesarios` (10380) no estadoVota 55 | eleitor | 11 | registro dos mesários de abertura concluído → `CIniciodeCiclo` (urna pronta) |
| `CControladorRegistraMesariosVota` slot 15 (10787 → corpo mesclado 6018), chamado por `CEncerraRegistroMesarios` (10380) no estadoVota 58 | eleitor | **7** | registro dos mesários de encerramento concluído → **encerramento: estadoVota GERARBU, "Inicio do Encerramento", `CGeraBU`** |
| `CRegistroMesarioEncerrado::StartState` (10763) | eleitor | 14 | → `CMostraTelaContinuaVotacao` |
| `CMostraTelaContinuaVotacao::StartState` (7221) | operador | 12 | o terminal do eleitor mostra a tela "continuar votação" e depois espera |
| `CReinicioComparecimentoMesario::StartState` (11917 → corpo mesclado 6058) | operador | 7 | o eleitor espera enquanto o operador registra novamente os mesários |
| `CFinalizaAquisicao::StartState` (12113 → corpo mesclado 6058) | operador | 10 | fim da aquisição de votos; o terminal do eleitor espera |
| `CDefineRotaPreVotacao::NeedChangeState` (11994) | operador | 7 | modo de demonstração (e não *treinamento de eleitor*): o eleitor espera |
| `CUrnaInspecionada::ProcessMessage(13)` (11794) | — | — | mostra a tela de continuação, volta para `CAguardaMensagem` |

* **Políticas de execução** (`vota::IExecucaoVota`). O `CExecucaoVota` da urna inicia as threads do eleitor, do operador e
  de monitoramento (`Inicia` 10235), dorme 1 s e faz o join delas (`Aguarda` 10234, `Executa` 10236). O build web
  registra `CExecucaoVotaCooperativa` em seu lugar. Seus slots 2/3 (4713, observado) apenas instalam o primeiro estado em
  `CThreadEleitor`, e `votaTick` avança essa thread passo a passo (u07 §3.4).

### 2.7 Wrappers POSIX (`api/ipc/posix`)

`CPosixMutex` é recursivo (`pthread_mutexattr_settype(PTHREAD_MUTEX_RECURSIVE)`, 6150 `Falha ao criar mutex
recursivo: {}` com `strerror`). `CPosixSemaphore(0)` (`sem_init`, 6159 `Falha ao criar semáforo: {}`) tem
`Wait(timeoutMs)`: `clock_gettime(CLOCK_REALTIME)` (6160 `Falha ao obter tempo de relógio: {}`), prazo em
ms → s/ns com transporte, depois `sem_timedwait`, e retorna 0 quando sinalizado. `CPosixRWMutex` embrulha
`pthread_rwlock_t`.

O build web removeu a maioria dos caminhos de erro (os stubs de pthread/sem retornam 0), mas seus registros `std::source_location`
continuam no segmento de dados, sem referência (`analysis/srcloc.tsv`). Eles atestam os nomes dos métodos e as
linhas:

| arquivo | registros (linha) |
|---|---|
| cposixmutex.cpp | ctor (30, referenciado), `Lock()` (47), `Unlock()` (56) |
| cposixrwmutex.cpp | ctor (29, 35, 42: init dos atributos, `pthread_rwlock_init`, destroy dos atributos), `ReadLock()` (58), `WriteLock()` (69), `Unlock()` (80) |
| cposixsemaphore.cpp | ctor (23, referenciado), `Wait(size_t)` (48 referenciado, 76 não), `Lock()` (90), `Unlock()` (99), `GetValue() const` (114) |

As mensagens são strings sem referência do mesmo pool (`Falha ao bloquear mutex: {}`, `Falha ao desbloquear
semáforo: {}`, `Falha ao obter valor do semáforo: {}`, …). A numeração do EUeIpcError se encaixa: 6150..6152 são os
três pontos do CPosixMutex, 6153..6158 os seis pontos do CPosixRWMutex, e 6159/6160 são os dois primeiros pontos do semáforo.
Assim, os códigos mortos são inferidos como 6151/6152, 6153..6158 e 6161..6164. O registro da linha 76 mostra que
`Wait(size_t)` lança quando a espera falha por um motivo que não é timeout.

### 2.8 `api::IPower` (`ipower.h`)

Só existem os cinco corpos padrão: `BatIntMaxSocValue` (:354, 5172), `SetBatIntMaxSocValue(float)` (:361, 5173),
`BatIntStorageMode()` → `std::tuple<bool,float>` (:409, 5174), `EnableBatIntStorageMode(float)` (:417, 5175) e
`DisableBatIntStorageMode()` (:424, 5176). Cada um lança `CUeHwilError(..., "Not supported")`. Eles cobrem o
modo de armazenamento da bateria interna e o estado de carga máximo dos modelos de urna mais novos. `api::teste::CPowerMock` não
os sobrescreve (slots 8-12 da vtable). Nada os chama nas sessões gravadas.

---

## 3. Dados lidos e gravados

* Arquivos: listados em §2.1. A unidade também lê `infomidia.dat` da MV para obter `numeroSerieFV` e grava o
  `rdv.dat(.tmp)` cifrado (§2.4).
* Módulos ASN.1 envolvidos: ModuloEstadoGeral{Urna,Vota,Gap,SA}, ModuloEnvelopeGenerico, ModuloHashes,
  ModuloResultadoUrnaCadastro, ModuloRegistroDigitalVoto (`EntidadeRegistroDigitalVoto`, `EntidadeResultadoRDV`),
  ModuloTiposResultadosEcoUrna (`Urna`, `CorrespondenciaResultado`, `Carga`), ModuloProcessoEleitoral,
  ModuloSituacoesEleicoes, ModuloEleicao, ModuloTiposEleitorais (`CabecalhoPacote`), ModuloLocal e
  ModuloInformacaoMidia (ver `src/asn1/`).
* SQL: indiretamente, `comparecimento_mesario` por meio do DAO de `CRegistradorMesario` (u13/u05).

---

## 4. Particularidades do build web e observações sobre wasm/Emscripten

* **Sem pthreads.** `pthread_mutex_lock`/`unlock`, `sem_wait`/`post`/`timedwait` e `pthread_rwlock_*` são
  stubs que retornam 0 e foram eliminados por inlining. `CPosixMutex::Lock`/`Unlock` (observados em execução) apenas
  incrementam/decrementam um contador em +28. O semáforo nunca bloqueia, e seu `GetValue` (10246) sempre retorna 0.
  Os corpos cujo caminho de erro foi removido (Lock/Unlock do mutex, Wait/GetValue do semáforo, o corpo vazio compartilhado
  2219) mantêm um frame de pilha morto de 480-544 bytes, resto do caminho de erro com `std::format` que foi removido.
* **Sem Asyncify.** `CWasmThread::Wait` faz polling com `emscripten_sleep(100)` e `CExecucaoVota::Inicia` chama
  `emscripten_sleep(1000)` (protegido pelo byte `g_esperaHabilitada` @1584624 = 1). A cola implementa
  `emscripten_sleep` como `abort()`. Esses caminhos só estão mortos porque `main` registra
  `CExecucaoVotaCooperativa`.
* **Os objetos de thread ainda são construídos.** 316 e 270 executam no votaInit. A fila do operador vai enchendo, mas nada a
  lê (u10).
* **Fixtures.** `infomidia.dat` não existe em nenhum cenário, então todo arquivo de resultado do simulador tem
  `numeroSerieFV = 00000000` (confirmado em `samples/bu-real/run-full/analysis/tse_{bu,rdv}_dump.txt`).
* **Nomes mangled.** `typeid(T).name()` (por exemplo `N25ModuloRegistroDigitalVoto20EntidadeResultadoRDVE`) faz parte
  dos contextos de erro. O build mantém RTTI.
* **Corpos mesclados.** O wasm-opt produziu 2919/3892 (CDataMap), 2927/6006 (CFileASN), os três thunks de construtor de exceção
  210/1229/1889, que passam uma vtable para 2294/710, e muito provavelmente 6018 e 6058: cada um é chamado apenas por dois
  thunks que diferem em uma constante (slots 10/15 de `CControladorRegistraMesariosVota` com 11/7; o StartState de
  `CReinicioComparecimentoMesario` / `CFinalizaAquisicao` com 7/10). A 6018 mantém um primeiro parâmetro não usado, o
  `this` dos métodos referenciados pela tabela a partir dos quais foi mesclada, então não é um helper no nível do código-fonte. Várias funções são nomeadas segundo uma chamada inlinada
  (270/316 segundo o ctor da fila, 3735/3771/3791/5740/5778 segundo ReadFromFile, 5378 segundo CDataMap::Update).
  §6 dá os nomes reais.
* **Parâmetros mesclados.** A func 5378 recebe um argumento onde o srcloc mostra `Update(const KeyType&, const
  RecordType&)`, porque seu único chamador passa o mesmo registro como chave e como valor.

---

## 5. Boletim de Urna (BU): o que esta unidade contribui

O BU é construído e assinado por outras unidades (`docs/10-boletim-de-urna.md`, `docs/bu/codepath.md`). A u18 fornece
as seguintes peças, na ordem em que acontecem:

1. **Início do encerramento.**
   * `CFinalizaAquisicao::StartState` (12113) marca o fim da aquisição de votos no terminal do eleitor e posta
     a mensagem 10 para o operador.
   * O operador registra os mesários de encerramento. `comum::CEncerraRegistroMesarios::StartState` (func 10380;
     as ferramentas a chamam de `GetControlador`) pede a fase ao controlador (slot 6: estadoVota − 55 por meio de uma
     tabela). Na fase 3, estadoVota 58 `registromesariofinal`, ela define next = slot 12, chama o slot 29 e depois chama o
     **slot 15** (`CControladorRegistraMesariosVota` 10787), que posta a **mensagem 7** para o terminal do eleitor.
     Nas fases 0/1, estadoVota 55 `registromesarioinicial`, ela chama em vez disso o slot 10 (10792 → mensagem 11 →
     `CIniciodeCiclo`, a urna é aberta aos eleitores).
   * No terminal do eleitor, `CAguardaMensagem::ProcessMessage(7)` define `estadoVota = GERARBU`, salva `vota.bin`,
     registra no log `"Inicio do Encerramento"` e muda para `vota::CGeraBU`.
2. **O bloco `urna`** de `-bu.dat`, `-rdv.dat`, `-imgbu.dat` e `-imgze.dat` é `comum::md::CUrna`
   (construtores 2854/2855, destrutor 1394). Ele contém:
   * `tipoUrna`: DadoCarga do eg.bin, `tipoUrnaT1` quando o turno é '1', senão `tipoUrnaT2`;
   * `versaoVotacao` = `"10.23.0.1 - DESENVOLVIMENTO"`;
   * `correspondenciaResultado` = {município, zona, seção, `CCarga` (copiada pela 1557), tipo};
   * `tipoArquivo` vindo do gravador;
   * `numeroSerieFV` = hex do serial da MV vindo de `infomidia.dat` (via 3735 e 1243), padrão `"00000000"`;
   * `motivoUtilizacaoSA` opcional, quando o *sistema de apuração* foi usado.

   `CUrna::ValidaCriacao` rejeita `tipoUrna == '0'` (8689), `tipoArquivo == '0'` (8690) e um serial que não tenha
   exatamente 8 dígitos hex (8691/8692, `"Serial da MV inválido [...]"`).
3. **`-bu.dat`**: `CGravadorBU::GravaResultado` (11629, outra unidade) embrulha `EntidadeBoletimUrna` em
   `EntidadeEnvelopeGenerico{tipoEnvelope envelopeBoletimUrna}` e o grava com
   `CFileASN::WriteDataToFile<CConversorEnvelopeGenerico>` (thunk **2724** → **6006**). A cadeia é:
   `CConversorEnvelopeGenerico::DoConverte` (slot 2) → `isValid && isStrictlyValid`, senão 7653 → contexto
   `"WriteToFile de " + <MI work path>` → `isValid`, senão 5955 → `encodeBER`, senão 5956 → `CFile::RawWrite`.
4. **`-rdv.dat` (arquivo de resultado)**: `CGravadorRDV::GravaResultado` (**11587**, §2 do fragmento do comum).
   * `CRdvVota::Converte()` fornece o BER do RDV. Ele é decodificado novamente por `DecodeObject<EntidadeRegistroDigitalVoto>`
     (**5825**: 5953/5954 em caso de falha).
   * Um `EntidadeResultadoRDV{cabecalho = CCabecalhoEntidade(dataGeração, pleito, 1), urna = CUrna(...), rdv}`
     é construído.
   * O tipo da correspondência é `'1'` (seção) quando o número da seção ≠ 0, senão `'2'` (contingência).
   * A entidade é codificada em BER com o contexto `"CodeObject de N25ModuloRegistroDigitalVoto20EntidadeResultadoRDVE"`
     e gravada como BER simples. Ela **não é cifrada nem envelopada**. O arquivo cifrado é apenas a cópia de
     trabalho `trab/rdv.dat`.
5. **`-imgbu.dat` / `-imgze.dat`**: `IGravadorEnvelope::GravaResultado` (**11626**) pega os bytes crus da
   imagem impressa, `trab/bu.dat` ou `trab/ze.dat` (slot 8, `CGravadorEnvelopeArquivo::LeConteudo`). Ele constrói
   `CEnvelopeGenerico(cabecalho, tipoEnvelope, urna, município, zona, local, seção, …, conteudo)` (5860) e
   o grava por meio da 2724. Aqui o tipo da correspondência é o tipo de urna do eg.bin, e não a regra da seção do item 4.
6. **`-hash.dat`** e **`-jufa.dat`** usam o mesmo helper com outros conversores: 5367 (`CConversorEntidadeHashes`)
   e 5366 (`CConversorResultadoUrnaCadastro` do ecourna, EApiAsnError 1900 para uma entidade inválida).
7. **Durabilidade do RDV durante a votação** (antes do BU): a verificação de gravação/releitura do `CEncryptedFile` de §2.4. No
   build web o RDV não é persistido (`CSincronismoVotoEleitorWeb`), então `Load` nunca executa no simulador.

---

## 6. Tabela de mapeamento completa (95 funções)

"executou" = observada em execução durante os votos gravados. Os arquivos sob `src/` são relativos a `src/`.

| func | tamanho | executou | nome nas ferramentas | símbolo reconstruído | reconstruído em | arquivo original | conf. |
|---|---|---|---|---|---|---|---|
| 210 | 21 |  | api_f210 | thunk de ctor de `CUeIoError` (`CBaseError<EUeIoError,{5950,6150}>(code, std::string&&, source_location)` → 2294 mesclado) | helper de biblioteca/inlinado (thunk de ctor de exceção do TSE) | uenux2/src/api/io/euioerror.h (caminho inferido) | média |
| 270 | 353 | ✓ | CPriorityMessageQueue<SMessage>::ctor@270 | `vota::CThreadOperador::GetInst()` (inlina CThread + ctor da fila, cmessagequeue.h:113/114) | uenux2/src/app/vota/u18-foreign-fragments.cpp | uenux2/src/app/vota/operador/cthreadoperador.cpp | alta |
| 316 | 303 | ✓ | CPriorityMessageQueue<SMessage>::ctor@316 | `vota::CThreadEleitor::GetInst()` | uenux2/src/app/vota/eleitor/cthreadeleitor.cpp (u07) | uenux2/src/app/vota/eleitor/cthreadeleitor.cpp | alta |
| 412 | 11 | ✓ | api_f412 | `api::CSystem::IsRegularFile(const std::string&)` = EhDoTipo(path, S_IFREG) (nome inferido) | uenux2/src/api/util/csystem.u12-fragment.cpp | uenux2/src/api/util/csystem.cpp | média |
| 517 | 137 | ✓ | ecourna_f517 | `ecourna::api::io::CFile::CFile(const std::string&, const std::string&, FileMode)` | ecourna/api/io/cfile.cpp (u12) | ecourna-lib/ecourna/api/io/cfile.cpp | alta |
| 656 | 14 | ✓ | CDataMap<CEleitorIdentidade,CEleitorDetalhe>::GetCurrent | idem (thunk → 2919, valor em node+32) | uenux2/src/api/io/cdatamap.h | uenux2/src/api/io/cdatamap.h | alta |
| 1200 | 14 |  | CDataMap<unsigned,CCandidatura>::GetCurrent | idem (thunk → 2919) | uenux2/src/api/io/cdatamap.h | uenux2/src/api/io/cdatamap.h | alta |
| 1229 | 18 |  | api_f1229 | thunk de ctor de `CUeHwilError` (→ 710 com vtable @1531060) | helper de biblioteca/inlinado (thunk de ctor de exceção do TSE) | uenux2/src/api/hwil/euhwilerror.h (caminho inferido) | média |
| 1243 | 283 | ✓ | comum_f1243 | `ecourna::api::util::CStringUtils::BytesToHexString(const std::vector<uebyte>&)` (nome inferido) | uenux2/src/app/comum/u18-foreign-fragments.cpp | ecourna-lib/ecourna/api/util/cstringutils.cpp | média |
| 1283 | 14 |  | CDataMap<unsigned short,CPartido>::GetCurrent | idem (thunk → 2919) | uenux2/src/api/io/cdatamap.h | uenux2/src/api/io/cdatamap.h | alta |
| 1337 | 22 | ✓ | vota_f1337 | `vota::CAguardaMensagem::GetInst()` (→ helper de singleton lazy 764) | uenux2/src/app/vota/u18-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/caguardamensagem.cpp (caminho inferido) | alta |
| 1394 | 179 |  | comum_f1394 | `comum::md::CUrna::~CUrna()` | uenux2/src/app/comum/u18-foreign-fragments.cpp | uenux2/src/app/comum/gravadores/md/curna.cpp | média |
| 1557 | 325 |  | comum_f1557 | `comum::md::CCarga::CCarga(const CCarga&)` (implícito) | uenux2/src/app/comum/u18-foreign-fragments.cpp | uenux2/src/app/comum/dados/md/correspondencia/ccarga.h | média |
| 1684 | 116 |  | api::CThread::Start | `api::CThread::Start()` (:68) | uenux2/src/api/ipc/cthread.cpp | uenux2/src/api/ipc/cthread.cpp | alta |
| 1889 | 18 |  | api_f1889 | thunk de ctor de `CUeIpcError` (→ 710 com vtable @1528452) | helper de biblioteca/inlinado (thunk de ctor de exceção do TSE) | uenux2/src/api/ipc/euipcerror.h (caminho inferido) | média |
| 1900 | 40 |  | vota_f1900 | `api::CThread::Wait()` (nome inferido) | uenux2/src/api/ipc/cthread.cpp | uenux2/src/api/ipc/cthread.cpp | média |
| 2073 | 674 |  | CPriorityMessageQueue<SMessage>::Remove | idem (:153) | uenux2/src/api/ipc/cmessagequeue.h | uenux2/src/api/ipc/cmessagequeue.h | alta |
| 2721 | 80 |  | api::CThread::vf0 | `api::CThread::~CThread()` | uenux2/src/api/ipc/cthread.cpp | uenux2/src/api/ipc/cthread.cpp | alta |
| 2724 | 33 |  | CFileASN::CodeObjectFunction@2724 | `api::CFileASN::WriteDataToFile<comum::asn::CConversorEnvelopeGenerico>` (thunk → 6006; nome inferido) | uenux2/src/api/io/asn/cfileasn.instances.cpp | uenux2/src/api/io/asn/cfileasn.h | média |
| 2766 | 383 |  | api::CEncryptedFile::MemWrite | `MemWrite(const std::vector<uebyte>&)` com `MemWrite(const void*, uedword)` (:142/:145) inlinada | uenux2/src/api/io/cencryptedfile.cpp | uenux2/src/api/io/cencryptedfile.cpp | alta |
| 2854 | 197 |  | comum_f2854 | `comum::md::CUrna::CUrna(tipo, versão, correspondência, tipoArquivo, serialFV)` | uenux2/src/app/comum/u18-foreign-fragments.cpp | uenux2/src/app/comum/gravadores/md/curna.cpp | média |
| 2855 | 206 |  | comum_f2855 | `comum::md::CUrna::CUrna(..., motivoUtilizacaoSA)` | uenux2/src/app/comum/u18-foreign-fragments.cpp | uenux2/src/app/comum/gravadores/md/curna.cpp | média |
| 2919 | 527 |  | api_f2919 | corpo mesclado de `api::CDataMap<K,R>::GetCurrent()` (this, offset do valor, srcloc) | uenux2/src/api/io/cdatamap.h | uenux2/src/api/io/cdatamap.h | alta |
| 2927 | 1275 | ✓ | api_f2927 | corpo mesclado de `api::CFileASN::CodeObjectFunction<T>` (arquivos de estado) | uenux2/src/api/io/asn/cfileasn.h | uenux2/src/api/io/asn/cfileasn.h | alta |
| 3125 | 14 |  | CDataMap<unsigned,CRespostaConsulta>::GetCurrent | idem (thunk → 2919) | uenux2/src/api/io/cdatamap.h | uenux2/src/api/io/cdatamap.h | alta |
| 3598 | 1956 | ✓ | api::CThread::CThread | `api::CThread::CThread()` (:31/:32; CPolySingletonList::instance inlinada) | uenux2/src/api/ipc/cthread.cpp | uenux2/src/api/ipc/cthread.cpp | alta |
| 3652 | 514 |  | api::CEncryptedFile::MemRead | `MemRead(std::vector<uebyte>&)` com `MemRead(void*, uedword)` (:161-167) inlinada | uenux2/src/api/io/cencryptedfile.cpp | uenux2/src/api/io/cencryptedfile.cpp | alta |
| 3653 | 3845 |  | api::CEncryptedFile::Load | `api::CEncryptedFile::Load(const std::string&)` (:36-73) | uenux2/src/api/io/cencryptedfile.cpp | uenux2/src/api/io/cencryptedfile.cpp | alta |
| 3697 | 12 |  | CDataMap<unsigned short,CPartido>::Next | idem (thunk → 3892) | uenux2/src/api/io/cdatamap.h | uenux2/src/api/io/cdatamap.h | alta |
| 3735 | 3405 |  | CFileASN::ReadFromFile@3735 | `api::CFileASN::ReadDataFromFile<ecourna::app::dados::asn::CConversorInformacaoMidia>(const std::string&)` (nome inferido) | uenux2/src/api/io/asn/cfileasn.instances.cpp | uenux2/src/api/io/asn/cfileasn.h | média |
| 3771 | 6814 | ✓ | CFileASN::ReadFromFile@3771 | `comum::asn::LeProcessoEleitoral(const CEstadoGeral&)` (nome como na u04, inferido) | uenux2/src/app/comum/u18-foreign-fragments.cpp | uenux2/src/app/comum/dados/cpe.cpp (caminho inferido) | média |
| 3791 | 3956 | ✓ | CFileASN::ReadFromFile@3791 | `comum::IServicoEstado<CEstadoGeral, CConversorEstadoGeral>::Carrega()` (nome inferido) | uenux2/src/app/comum/u18-foreign-fragments.cpp | uenux2/src/app/comum/appinfo/servicos/iservicoestado.h (caminho inferido) | média |
| 3892 | 577 |  | api_f3892 | corpo mesclado de `api::CDataMap<K,R>::Next()` | uenux2/src/api/io/cdatamap.h | uenux2/src/api/io/cdatamap.h | alta |
| 4713 | 89 | ✓ | vota::CExecucaoVotaCooperativa::Inicia (antes exibida pelas ferramentas como `vf2`) | `CExecucaoVotaCooperativa::Inicia/Executa` (slots 2 e 3; nomes inferidos) | uenux2/src/app/vota/u18-foreign-fragments.cpp | uenux2/mock/app/vota/cexecucaovotacooperativa.cpp (caminho inferido, u39; componente app:mock) | média |
| 5353 | 14 |  | api::CPosixRWMutex::vf0 | `api::CPosixRWMutex::~CPosixRWMutex()` | uenux2/src/api/ipc/posix/cposixrwmutex.cpp | uenux2/src/api/ipc/posix/cposixrwmutex.cpp | alta |
| 5354 | 14 |  | api::CPosixSemaphore::vf0 | `api::CPosixSemaphore::~CPosixSemaphore()` | uenux2/src/api/ipc/posix/cposixsemaphore.cpp | uenux2/src/api/ipc/posix/cposixsemaphore.cpp | alta |
| 5355 | 14 |  | api::CPosixMutex::vf0 | `api::CPosixMutex::~CPosixMutex()` | uenux2/src/api/ipc/posix/cposixmutex.cpp | uenux2/src/api/ipc/posix/cposixmutex.cpp | alta |
| 5366 | 1244 |  | CFileASN::CodeObjectFunction@5366 | `api::CFileASN::WriteDataToFile<ecourna CConversorResultadoUrnaCadastro>` (nome inferido) | uenux2/src/api/io/asn/cfileasn.instances.cpp | uenux2/src/api/io/asn/cfileasn.h | média |
| 5367 | 33 |  | CFileASN::CodeObjectFunction@5367 | `api::CFileASN::WriteDataToFile<comum::asn::CConversorEntidadeHashes>` (thunk → 6006) | uenux2/src/api/io/asn/cfileasn.instances.cpp | uenux2/src/api/io/asn/cfileasn.h | média |
| 5377 | 1492 |  | CDataMap<PK,CComparecimentoMesario>::Add | `CDataMap<...>::Add(const KeyType&, const RecordType&)` + `Add(value_type)` inlinada (:143) | uenux2/src/api/io/cdatamap.h | uenux2/src/api/io/cdatamap.h | alta |
| 5378 | 1239 |  | CDataMap<PK,CComparecimentoMesario>::Update | `comum::CRegistradorMesario::AtualizaInternal(const md::CComparecimentoMesario&)` (nome inferido; inlina Update :158) | uenux2/src/app/comum/u18-foreign-fragments.cpp | uenux2/src/app/comum/comparecimentomesario/cregistradormesario.cpp | média |
| 5483 | 256 |  | api::CIniKey::operator= | idem (:24) | uenux2/src/api/io/cinikey.cpp | uenux2/src/api/io/cinikey.cpp | alta |
| 5601 | 12 |  | CDataMap<CEleitorIdentidade,CEleitorDetalhe>::Next | idem (thunk → 3892) | uenux2/src/api/io/cdatamap.h | uenux2/src/api/io/cdatamap.h | alta |
| 5611 | 12 |  | CDataMap<unsigned,CCandidatura>::Next | idem (thunk → 3892) | uenux2/src/api/io/cdatamap.h | uenux2/src/api/io/cdatamap.h | alta |
| 5706 | 3258 | ✓ | CFileASN::ReadFromFile@5706 | `api::CFileASN::ReadDataFromFile<comum::asn::CConversorCabecalhoPacote>(const std::string&)` (nome inferido) | uenux2/src/api/io/asn/cfileasn.instances.cpp | uenux2/src/api/io/asn/cfileasn.h | média |
| 5725 | 1085 |  | CDataMap<CNumeroInscricaoEleitoral,CJustificadorDetalhe>::Add | idem, `Add(const value_type&)` (:143) | uenux2/src/api/io/cdatamap.h | uenux2/src/api/io/cdatamap.h | alta |
| 5740 | 5775 | ✓ | CFileASN::ReadFromFile@5740 | `comum::CLocal::Carrega()` (nome inferido) | uenux2/src/app/comum/u18-foreign-fragments.cpp | uenux2/src/app/comum/dados/clocal.cpp | média |
| 5778 | 10735 | ✓ | CFileASN::ReadFromFile@5778 | `comum::asn::LePleito(fase, uf, dir, const CPleitoDTO&)` (nome inferido) | uenux2/src/app/comum/u18-foreign-fragments.cpp | uenux2/src/app/comum/dados/cpe.cpp (caminho inferido) | baixa |
| 5825 | 1308 |  | api::CFileASN::DecodeObjectFunction | `api::CFileASN::DecodeObject<EntidadeRegistroDigitalVoto>` (DecodeObjectFunction :135/:143 inlinada) | uenux2/src/api/io/asn/cfileasn.instances.cpp | uenux2/src/api/io/asn/cfileasn.h | alta |
| 6006 | 1651 |  | api_f6006 | corpo mesclado de `api::CFileASN::WriteDataToFile<TConversor>` (conversores do comum) | uenux2/src/api/io/asn/cfileasn.h | uenux2/src/api/io/asn/cfileasn.h | média |
| 6018 | 52 |  | vota_f6018 | corpo mesclado (wasm-opt merge-similar) dos slots 10/15 de `CControladorRegistraMesariosVota`: `(this /*unused*/, id)` → posta `{id, &fila}` na fila do eleitor. Rótulo `CControladorRegistraMesariosVota::EnviaMensagemEleitor`; provavelmente não há função no código-fonte | inline nos slots 10/15, uenux2/src/app/vota/u18-foreign-fragments.cpp | uenux2/src/app/vota/operador/comparecimentomesario/ccontroladorregistramesariosvota.cpp (caminho inferido) | baixa |
| 6058 | 60 |  | vota_f6058 | corpo mesclado (wasm-opt merge-similar) de `CReinicioComparecimentoMesario::StartState` / `CFinalizaAquisicao::StartState`: `(state, id)` → next = CAguardaMensagem, posta `{id, &fila}` na fila do operador. Rótulo `vota::AguardaOperador`; provavelmente não há função no código-fonte | inline em 11917/12113, uenux2/src/app/vota/u18-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/ (caminho inferido) | baixa |
| 7221 | 85 |  | vota::CMostraTelaContinuaVotacao::vf2 | `CMostraTelaContinuaVotacao::StartState()` | uenux2/src/app/vota/u18-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/cmostratelacontinuavotacao.cpp (caminho inferido) | alta |
| 8074 | 50 |  | api::IPower::DisableBatIntStorageMode | idem (:424) | uenux2/src/api/hwil/ipower.h | uenux2/src/api/hwil/ipower.h | alta |
| 8083 | 50 |  | api::IPower::EnableBatIntStorageMode | idem (:417) | uenux2/src/api/hwil/ipower.h | uenux2/src/api/hwil/ipower.h | alta |
| 8091 | 50 |  | api::IPower::BatIntStorageMode | idem (:409) | uenux2/src/api/hwil/ipower.h | uenux2/src/api/hwil/ipower.h | alta |
| 8098 | 50 |  | api::IPower::SetBatIntMaxSocValue | idem (:361) | uenux2/src/api/hwil/ipower.h | uenux2/src/api/hwil/ipower.h | alta |
| 8105 | 50 |  | api::IPower::BatIntMaxSocValue | idem (:354) | uenux2/src/api/hwil/ipower.h | uenux2/src/api/hwil/ipower.h | alta |
| 9257 | 9 |  | ecourna_f9257 | thunk para 1243 (segunda sobrecarga mesclada; `CStringUtils::BytesToHexString`) | uenux2/src/app/comum/u18-foreign-fragments.cpp | ecourna-lib/ecourna/api/util/cstringutils.cpp | baixa |
| 9673 | 21 | ✓ | CFileASN::CodeObjectFunction@9673 | `CFileASN::CodeObjectFunction<ModuloEstadoGeralVota::EstadoGeralVota>` (thunk → 2927) | uenux2/src/api/io/asn/cfileasn.instances.cpp | uenux2/src/api/io/asn/cfileasn.h | alta |
| 9695 | 21 |  | CFileASN::CodeObjectFunction@9695 | `...<ModuloEstadoGeralSA::EstadoGeralSA>` | uenux2/src/api/io/asn/cfileasn.instances.cpp | uenux2/src/api/io/asn/cfileasn.h | alta |
| 9702 | 21 | ✓ | CFileASN::CodeObjectFunction@9702 | `...<ModuloEstadoGeralGap::EstadoGeralGap>` | uenux2/src/api/io/asn/cfileasn.instances.cpp | uenux2/src/api/io/asn/cfileasn.h | alta |
| 9790 | 130 |  | api_f9790 | ctor de `std::__format_arg_store<format_context, const string&, string&>` (make_format_args) | helper de biblioteca/inlinado (libc++ `<format>`) | libc++ | média |
| 9801 | 21 | ✓ | CFileASN::CodeObjectFunction@9801 | `...<ModuloEstadoGeralUrna::EstadoGeralUrna>` | uenux2/src/api/io/asn/cfileasn.instances.cpp | uenux2/src/api/io/asn/cfileasn.h | alta |
| 10234 | 20 |  | vota::CExecucaoVota::vf4 | `vota::CExecucaoVota::Aguarda()` (nome inferido) | uenux2/src/app/vota/u18-foreign-fragments.cpp | uenux2/src/app/vota/cexecucaovota.cpp (caminho inferido) | média |
| 10235 | 39 |  | vota::CExecucaoVota::vf3 | `vota::CExecucaoVota::Inicia()` (nome inferido) | uenux2/src/app/vota/u18-foreign-fragments.cpp | uenux2/src/app/vota/cexecucaovota.cpp (caminho inferido) | média |
| 10236 | 57 |  | vota::CExecucaoVota::vf2 | `vota::CExecucaoVota::Executa()` (nome inferido) | uenux2/src/app/vota/u18-foreign-fragments.cpp | uenux2/src/app/vota/cexecucaovota.cpp (caminho inferido) | média |
| 10244 | 13 |  | api::CPosixRWMutex::vf1 | destrutor deleting | uenux2/src/api/ipc/posix/cposixrwmutex.cpp | uenux2/src/api/ipc/posix/cposixrwmutex.cpp | alta |
| 10245 | 42 |  | api::CPosixRWMutex::CPosixRWMutex | idem (srclocs sem referência :29/:35/:42 dos seus caminhos de erro removidos) | uenux2/src/api/ipc/posix/cposixrwmutex.cpp | uenux2/src/api/ipc/posix/cposixrwmutex.cpp | alta |
| 10246 | 44 |  | api::CPosixSemaphore::vf6 | `CPosixSemaphore::GetValue() const` (nome tirado do registro srcloc sem referência :114; slot por eliminação) | uenux2/src/api/ipc/posix/cposixsemaphore.cpp | uenux2/src/api/ipc/posix/cposixsemaphore.cpp | média |
| 10247 | 623 |  | api::CPosixSemaphore::Wait | `Wait(size_t)` (:48; o throw de :76 foi removido) | uenux2/src/api/ipc/posix/cposixsemaphore.cpp | uenux2/src/api/ipc/posix/cposixsemaphore.cpp | alta |
| 10248 | 15 |  | api::CPosixSemaphore::vf4 | `CPosixSemaphore::Wait()` → slot 2 (nome inferido) | uenux2/src/api/ipc/posix/cposixsemaphore.cpp | uenux2/src/api/ipc/posix/cposixsemaphore.cpp | baixa |
| 10249 | 13 |  | api::CPosixSemaphore::vf1 | destrutor deleting | uenux2/src/api/ipc/posix/cposixsemaphore.cpp | uenux2/src/api/ipc/posix/cposixsemaphore.cpp | alta |
| 10250 | 552 |  | api::CPosixSemaphore::CPosixSemaphore | `CPosixSemaphore(unsigned)` (:23) | uenux2/src/api/ipc/posix/cposixsemaphore.cpp | uenux2/src/api/ipc/posix/cposixsemaphore.cpp | alta |
| 10251 | 35 | ✓ | api::CPosixMutex::vf3 | `CPosixMutex::Unlock()` (nome tirado do registro srcloc sem referência :56) | uenux2/src/api/ipc/posix/cposixmutex.cpp | uenux2/src/api/ipc/posix/cposixmutex.cpp | alta |
| 10252 | 35 | ✓ | api::CPosixMutex::vf2 | `CPosixMutex::Lock()` (nome tirado do registro srcloc sem referência :47) | uenux2/src/api/ipc/posix/cposixmutex.cpp | uenux2/src/api/ipc/posix/cposixmutex.cpp | alta |
| 10253 | 13 |  | api::CPosixMutex::vf1 | destrutor deleting | uenux2/src/api/ipc/posix/cposixmutex.cpp | uenux2/src/api/ipc/posix/cposixmutex.cpp | alta |
| 10254 | 505 |  | api::CPosixMutex::CPosixMutex | idem (:30) | uenux2/src/api/ipc/posix/cposixmutex.cpp | uenux2/src/api/ipc/posix/cposixmutex.cpp | alta |
| 10728 | 411 |  | vota_f10728 | fonte de texto `std::format("{:<12s}", CThreadOperador +120)` (nome inferido) | uenux2/src/app/vota/u18-foreign-fragments.cpp | uenux2/src/app/vota/operador/ (tela CTituloEncerramentoInvalido; caminho inferido) | baixa |
| 10763 | 59 |  | vota::CRegistroMesarioEncerrado::vf2 | `CRegistroMesarioEncerrado::StartState()` | uenux2/src/app/vota/u18-foreign-fragments.cpp | uenux2/src/app/vota/operador/comparecimentomesario/cregistromesarioencerrado.cpp (caminho inferido) | média |
| 10785 | 116 |  | CControladorRegistraMesariosVota::vf18 | `SetTituloDigitado(const std::string&)` (nome inferido) | uenux2/src/app/vota/u18-foreign-fragments.cpp | .../ccontroladorregistramesariosvota.cpp (caminho inferido) | baixa |
| 10787 | 9 |  | CControladorRegistraMesariosVota::vf15 | `IniciaEncerramento()` → msg 7 para o eleitor (nome inferido) | uenux2/src/app/vota/u18-foreign-fragments.cpp | .../ccontroladorregistramesariosvota.cpp (caminho inferido) | baixa |
| 10788 | 9 |  | CControladorRegistraMesariosVota::vf17 | `GetTituloDigitado()` (nome inferido) | uenux2/src/app/vota/u18-foreign-fragments.cpp | .../ccontroladorregistramesariosvota.cpp (caminho inferido) | baixa |
| 10792 | 9 |  | CControladorRegistraMesariosVota::vf10 | `LiberaTerminalEleitor()` → msg 11 para o eleitor (nome inferido) | uenux2/src/app/vota/u18-foreign-fragments.cpp | .../ccontroladorregistramesariosvota.cpp (caminho inferido) | baixa |
| 11430 | 564 |  | api::CFileSeeder::skip | idem (:106) | uenux2/src/api/io/asn/cpartialfileasn.h | uenux2/src/api/io/asn/cpartialfileasn.h | alta |
| 11431 | 1082 | ✓ | api::CFileSeeder::seek | idem (:88/:94) | uenux2/src/api/io/asn/cpartialfileasn.h | uenux2/src/api/io/asn/cpartialfileasn.h | alta |
| 11432 | 578 | ✓ | api::CFileSeeder::decodeBlock | idem (:75) | uenux2/src/api/io/asn/cpartialfileasn.h | uenux2/src/api/io/asn/cpartialfileasn.h | alta |
| 11433 | 552 | ✓ | api::CFileSeeder::decodeByte | idem (:52) | uenux2/src/api/io/asn/cpartialfileasn.h | uenux2/src/api/io/asn/cpartialfileasn.h | alta |
| 11587 | 2707 |  | comum::CGravadorRDV::vf7 | `comum::CGravadorRDV::GravaResultado(api::CFile&) const` | uenux2/src/app/comum/u18-foreign-fragments.cpp | uenux2/src/app/comum/gravadores/cgravadorrdv.cpp (caminho inferido) | alta |
| 11626 | 1508 |  | comum::IGravadorEnvelope::vf7 | `comum::IGravadorEnvelope::GravaResultado(api::CFile&) const` | uenux2/src/app/comum/u18-foreign-fragments.cpp | uenux2/src/app/comum/gravadores/igravadorenvelope.cpp (caminho inferido) | alta |
| 11794 | 36 |  | vota::CUrnaInspecionada::vf6 | `CUrnaInspecionada::ProcessMessage(int)` | uenux2/src/app/vota/u18-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/curnainspecionada.cpp (caminho inferido) | média |
| 11917 | 9 |  | vota::CReinicioComparecimentoMesario::vf2 | `CReinicioComparecimentoMesario::StartState()` | uenux2/src/app/vota/u18-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/creiniciocomparecimentomesario.cpp (caminho inferido) | média |
| 11994 | 118 |  | vota::CDefineRotaPreVotacao::vf3 | `CDefineRotaPreVotacao::NeedChangeState()` | uenux2/src/app/vota/u18-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/iniciovotacao/cdefinerotaprevotacao.cpp (caminho inferido) | média |
| 12113 | 9 |  | vota::CFinalizaAquisicao::vf2 | `CFinalizaAquisicao::StartState()` | uenux2/src/app/vota/u18-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/cfinalizaaquisicao.cpp (caminho inferido) | média |
| 12621 | 442 |  | api::CDataText<CRespostasDSNumero>::vf2 | `api::CDataText<comum::CRespostasDSNumero>::GetText()` (`"{:0{}}"`, CRespostas::GetInst inlinada) | uenux2/src/app/comum/u18-foreign-fragments.cpp | região de uenux2/src/app/comum/dados/crespostas.cpp (caminho inferido) | média |

Funções de fora da unidade que foram reconstruídas nos arquivos da u18 por completude: 10255 `CThread::ThreadProc`,
rhvoice_f501 `CPriorityMessageQueue::Add`, 4343/4337 `Recebe`, 3162/3157 destrutores da fila, 11434/11429/11428
(slots do CFileSeeder da u12), 2891/2892 (`WriteToFile` da u12).

---

## 7. Código estranho ou arriscado

1. **`CEncryptedFile::Load` nunca lança três de suas verificações (func 3653).** Em cencryptedfile.cpp:44 (tamanho do arquivo 0
   ou não múltiplo de 16), :51 (leitura curta) e :73 (um byte de padding ≠ comprimento do pad), o código constrói um
   `CBaseError<EUeIoError>` em um temporário na pilha (`ecourna_f1143` com código 5989/5990/5993) e o destrói
   imediatamente. Não há `__cxa_allocate_exception` nem `__cxa_throw` (verificado no WAT). Em C++ isso é
   `CUeIoError(...);` sem `throw`. As outras quatro verificações da mesma função lançam de fato. Como resultado, um
   RDV cifrado truncado ou com padding incorreto não é rejeitado nesta camada. Uma leitura curta deixa zeros no buffer
   que é decifrado. Os testes de tamanho múltiplo de 16 e de padding secundário não têm efeito. Um arquivo de 0 bytes ainda é
   recusado, mas apenas porque `CFile::RawRead` lança `CIoError` 1192 (`buffer invalido`) para o buffer nulo do
   vetor vazio. Outras salvaguardas permanecem: a verificação pad > 16, as verificações da própria cifra (não verificadas aqui), a decodificação
   BER do RDV e a comparação de releitura `ConfereConteudo`. Mesmo código na urna (não é um mock web).
2. **`claro.back()` sem verificação de vazio (3653).** Se a decifração retornar um vetor vazio, o byte de pad
   é lido de antes do buffer (em wasm, um ponteiro de dados nulo dá o endereço 0xFFFFFFFF e um trap). Um arquivo
   de entrada vazio não consegue chegar lá (ver 1), então isso exige uma cifra que não retorne nada para uma entrada não vazia. A
   verificação `pad > tamanho` compara com o tamanho do arquivo cifrado, e não com o tamanho decifrado, e pad 0 é aceito.
3. **`CPriorityMessageQueue::Remove` lança com o lock adquirido (2073).** O lock é obtido com um `Lock()`
   explícito. Com a fila vazia, a função lança 6228 sem `Unlock()`: não há código de limpeza nesse caminho.
   O mutex é recursivo, então a mesma thread pode continuar, mas qualquer outra thread que precise da fila ficaria bloqueada
   para sempre. (No build web o "mutex" é só um contador, então nada bloqueia; o contador apenas fica uma unidade
   acima do correto.) Os chamadores normalmente testam `Vazia()` antes. `Recebe` (4343) confia no semáforo em vez disso. No build
   web o semáforo é um no-op que sempre informa sucesso, então `Recebe` com a fila vazia cairia nesse caminho
   (código morto no simulador: a thread do operador nunca executa).
4. **Semáforos e mutexes não fazem nada no build web.** `sem_*` e `pthread_*` são stubs. `CPosixSemaphore::Wait`
   sempre retorna "sinalizado", `GetValue` retorna 0 e `CPosixMutex` apenas conta. Isso é inofensivo enquanto apenas
   `votaTick` executa, mas qualquer código que use essas primitivas para esperar não esperaria.
5. **Caminhos com `emscripten_sleep` abortam (sem Asyncify).** `CExecucaoVota::Inicia`/`Executa` (10235/10236) chamam
   `emscripten_sleep(1000)`, e `CExecucaoVota::Aguarda` → `CThread::Wait` → `CWasmThread::Wait` faz polling com
   `emscripten_sleep(100)`. A cola transforma ambos em `abort("Please compile your program with async support…")`.
   Eles só são inalcançáveis porque `main` registra `CExecucaoVotaCooperativa`: o getter de `IExecucaoVota`
   (3594, iexecucaovota.cpp:74) cria um `CExecucaoVota` por padrão quando nenhuma política está registrada. Uma mudança de
   política ou uma chamada direta derrubaria o simulador.
6. **`CFileSeeder::decodeByte` pode retornar um byte não inicializado (11433).** `fim()` é o `feof` do stdio, que
   só se torna verdadeiro depois que uma leitura falhou. Exatamente no fim do arquivo, `RawRead` retorna 0 sem lançar
   (cfile.cpp), e `decodeByte` retorna o byte não inicializado da pilha em vez de lançar 5957. Um
   `-el.dat` truncado poderia entregar um byte de lixo ao decodificador BER parcial. As verificações de comprimento do decodificador normalmente
   detectam o truncamento antes. Mesmo código na urna.
7. **`CFileASN::ReadFromFile` ignora o resultado de `RawRead`.** `CFile::RawRead` retorna uma contagem menor sem
   lançar, então uma leitura curta decodifica um buffer completado com zeros. Normalmente a decodificação então falha com "não foi decodificado"
   (a mensagem aponta para o conteúdo em vez de para a E/S). Se os bytes faltantes caírem dentro de um valor primitivo
   (uma OCTET STRING, por exemplo), os zeros podem ser aceitos como conteúdo. Uma leitura curta exige um erro de E/S ou um arquivo
   que encolha entre a sondagem do tamanho e a leitura; não observado.
8. **O tipo da correspondência é calculado de duas maneiras diferentes.** `CGravadorRDV` (11587) deriva o
   tipo da correspondência de `seção != 0 ? '1' : '2'`. `IGravadorEnvelope` (11626) usa o tipo de urna do eg.bin
   para o mesmo campo. Para uma urna de contingência com número de seção, esses valores poderiam diferir entre `-rdv.dat` e
   `-imgbu.dat`. A diferença no código é certa. Se ela muda os arquivos codificados não foi verificado: isso depende
   de como `CConversorUrna` (10287) mapeia os dois conjuntos de valores. Não observado. Baixa.
9. **`numeroSerieFV` é sempre `00000000` no simulador.** `infomidia.dat` só é procurado em
   `/dsk/fe/estatico/`, e nenhum cenário o distribui. O valor recebe o padrão silenciosamente, sem ser reportado. Todo
   BU/RDV/envelope simulado carrega o serial padrão. Informativo (é esperado para uma fixture, mas quem lê os arquivos
   deve saber que não é um serial real).

---

## 8. Questões em aberto

* Os nomes exatos dos helpers de `CFileASN` no nível do conversor (aqui `ReadDataFromFile`/`WriteDataToFile`), dos
  slots de `IExecucaoVota` e de `IControladorRegistraMesarios`, e de `CThreadOperador` +84/+96/+120 são inferidos.
* `SMessage` +4 é sempre gravado com o endereço da fila de destino. Não estabelecemos se isso é
  um membro real (remetente/destino) ou um artefato do otimizador a partir de um membro não inicializado.
* `md::CUrna` +108 (um bool definido como true pelos dois construtores, nunca testado por `ValidaCriacao`).
* Slot 4 de `CPosixSemaphore` (10248, chama o slot 2 = `Lock`): nenhum srcloc o nomeia; `Wait()` é um palpite. Os códigos de erro
  e as mensagens dos caminhos de erro POSIX removidos (§2.7) são inferidos a partir da numeração e do pool de strings.
* O significado, do lado do operador, das mensagens 7, 10 e 12 (consumidas pelos estados de `CThreadOperador`, u10) não é rastreado aqui.
