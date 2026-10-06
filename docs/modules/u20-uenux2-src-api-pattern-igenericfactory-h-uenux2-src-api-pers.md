# u20 — a base `api` (util, persistência, uelog, pattern) e `comum::CAppInfo`, o cache de estado da urna

A unidade u20 é a infraestrutura por baixo da aplicação VOTA (o programa da urna no dia da eleição). Ela contém 109 funções
wasm que as ferramentas atribuíram a 21 arquivos originais. A maioria delas são helpers pequenos e amplamente compartilhados:

| área | arquivos originais | o que faz |
|---|---|---|
| **`api/util`** | `csystem.cpp`, `cdirreader.cpp`, `cdate.cpp`, `ctime.cpp`, `cdatetime.cpp`, `cgenerictags.cpp`, `cstringutils.cpp`, `csynchronizer.cpp`, `ctickmanager.cpp`, `cwait.cpp`, `itimerscheduler.{h,cpp}` | helpers de sistema de arquivos (cópia com `O_SYNC`, substituição atômica com backup, "preenchimento com zeros" antes da exclusão), os tipos de valor de data/hora da urna, um codec TLV textual para o serviço de assinatura, o analisador de número de versão, a chave de sincronização de gravação, os temporizadores por thread ("ticks"), o prazo de espera ativa e o serviço de temporizador da GUI |
| **`api/persistencia`** | `cdaorepositorio.hpp`, `iuenuxgenericdao.h` | o registro que entrega DAOs (*Data Access Objects*) sobre o banco de dados SQLite `uenux.db`, e as operações padrão "não implementada" dos DAOs |
| **`api/uelog`** | `cloga.cpp`, `cescritorlog.cpp` | o front-end do log da aplicação (`CLoga::loga` → `CEscritorLog` → o gravador do logd; `simulador::CWasmLogd` no build web) |
| **`api/pattern`** | `igenericfactory.h` | a fábrica abstrata de primitivas do SO (semáforo, mutex, rw-lock, thread) |
| **`comum/appinfo`** | `cappinfo.cpp`, `servicos/cservicoestadogeral{sa,gap,vota}.cpp` | `CAppInfo`, o cache em memória dos arquivos de estado `eg.bin`, `gap.bin`, `vota.bin`, `sa.bin`, seus getters (`GetGeral`, `GetVota`, `TemVota`, `IndiceTurno`…), o gravador de `vota.bin` nas duas memórias flash (`SalvaEstado`) e o caminho de cada arquivo de estado |

A unidade também recebeu **26 funções de outros arquivos**, que as ferramentas agruparam aqui porque inlinam
ou chamam os helpers acima. São elas o cabeçalho dos **QR codes do BU**
(`CGeradorBUQRCodeVota::PreencheCabecalho`), o cache de presença de mesários e seu DAO
(`CRegistradorMesario`, `CJustificador`), a requisição "assinar/verificar arquivo" ao SAVD (`IInterfaceSavd`), duas
linhas de log de inicialização, e 13 funções `vota`: os estados dos fluxos de zerésima, registro de mesários, fim da
votação e BU, e dois getters de singleton. Mais quatro funções livres (`SalvaEstado`, a cópia da assinatura
para a MV e dois predicados de modo) não têm arquivo-fonte próprio; elas são mantidas em `cappinfo.cpp`.

31 das 109 funções executaram durante os votos gravados (`analysis/runtime/*.functions.tsv`): os getters de `CAppInfo`,
`SalvaEstado`, `CopyFile`, `ZeroFill`, datas e horas, o gerenciador de ticks, o gravador de log, o codec TLV do
SAVD e as duas linhas de log de inicialização. Estas últimas são os dois primeiros registros de `logd.dat` no snapshot
do MEMFS: `1|1|Iniciando aplicação - 1º turno` e `1|1|Versão da aplicação: 10.23.0.1 - DESENVOLVIMENTO`.

**Arquivos-fonte reconstruídos** (`// wasm func N` em toda função, `// name inferred` / `// ?` onde aplicável):

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

## 1. Glossário

| termo | significado |
|---|---|
| **MI / MV** | *mídia interna* (flash interna, `/dsk/fi`) / *mídia de votação* (cartão de flash externo, `/dsk/fe`). Todo arquivo dinâmico existe nas duas. |
| **trab1 / trab2** | diretório de trabalho por turno `dinamico/trab<turno>/` que contém `vota.bin`, `rdv.dat`, `uenux.db` e as suas assinaturas `.vsu` |
| **estado geral** | os arquivos de estado persistentes: `eg.bin` (urna), `gap.bin` (*GAP*, o lançador de aplicações), `vota.bin` (VOTA), `sa.bin` (*SA*, *Sistema de Apuração*, a aplicação de apuração) |
| **turno** | turno da eleição. No código, `EUrnaTurno`: `'0'` sem turno, `'1'`, `'2'`, `'3'` = "turno atual" (resolvido por meio de `CEstadoGeral`) |
| **fase** | `'1'` oficial, `'2'` simulado, `'3'` treinamento |
| **treinamento do eleitor / do mesário** | treinamento de eleitores vs treinamento de mesários (ambos fase `'3'`; `EstadoGeralVota.treinamentoEleitor` os distingue). O simulador executa em *treinamento do eleitor*. |
| **SAVD** | o serviço de assinatura/validação da urna (`comum::IInterfaceSavd`) |
| **pacote `.vsu`** | o arquivo de assinatura de um arquivo de dados (`vota.vsu`, `rdv.vsu`, `uenux.vsu`…); ids de `CArquivosSavd` 122–125 = `vota.vsu` (MI/MV × turno) |
| **DAO** | *Data Access Object* sobre o SQLite `uenux.db` (tabelas `comparecimento_mesario`, `registro_justificativa`, `eleitor_dinamico`) |
| **comparecimento de mesários** | presença dos mesários, registrada pela identificação de eleitor na abertura (*período* 1) e no encerramento |
| **zerésima** | relatório impresso antes da abertura da votação, que prova que todo candidato tem zero votos; seu resumo é `rze.dat` |
| **tick** | um temporizador periódico da máquina de estados de uma thread (`CTickManager`) |
| **logd** | o daemon de log da urna; registra `app|severity|text` em `log/logd.dat` (Latin-1) |

---

## 2. Classes e como se relacionam

RTTI (endereços de typeinfo / vtable) das classes polimórficas da unidade:

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

Classes de valor / auxiliares não polimórficas, com os layouts recuperados:

| classe | tamanho | layout |
|---|---|---|
| `comum::CAppInfo` | 532 | +0 `optional<CEstadoGeral>` (flag +180) · +184 `array<optional<CEstadoGeralGap>,2>` (48 B cada) · +280 `array<optional<CEstadoGeralVota>,2>` (104 B cada) · +488..532 não usado aqui. `unique_ptr` do singleton @1838664 (getter func 185) |
| `CEstadoGeralVota` (como usado aqui) | 100 | +0 estadoVota · +12/+16 `optional<uint32> urnaIdGerouZeresima` · +20/+32 `optional<CDateTime> dhIniAquisicao` · +36/+48 `dhFimAquisicao` · +72 treinamentoEleitor · +80/+92 `dhEmissao` · +96 tecladoTestadoPosConversao |
| `CEstadoGeral` (como usado aqui) | 180 | +20 município (QR MUNI) · +32 turno · +36/+40 tipoUrna T1/T2 · +48 fase · +60 `CDadoCorrespondencia` (seu primeiro int = id da urna) |
| `api::CDate` | 8 | 4 × `ueword`: dia, mes, ano, diaSemana |
| `api::CTime` | 4 | segundos desde a meia-noite |
| `api::CDateTime` | 12 | `CDate` + `CTime` |
| `api::CDirReader` | 20 | +0 `std::string` dir · +12 `DIR*` · +16 `dirent*` |
| `api::CGenericTags` | 20 | +0 largura do campo de tag (4) · +4 largura do nome da tag (3) · +8 `map<string,uebyte>` (o comprimento-do-comprimento n é gravado/lido como um byte em node+28) |
| `api::CTickManager` | 12 | `map<uebyte, STick>`; `STick` = {uint64 intervalo ms, timeval próximo, `parado` de 4 bytes (0/1, gravado e lido como i32, portanto não é um `bool`)} |
| `api::CWait` | 24 | +0 µs · +8 prazo `timeval` |
| `api::CSynchronizer` | 1 | `bool m_sincrono`; singleton @1839324 |
| `comum::CRegistradorMesario` *(nome tirado da string que ela armazena)* | 52 | +0 `map<CComparecimentoMesarioPK, CComparecimentoMesario>` · +12 iterador da última busca · +16 nome `"CRegistradorMesario"` · +28 segundo map · +40 `CComparecimentoMesarioServico` |
| `comum::CJustificador` *(idem)* | 40 | +0 map · +12 iterador · +16 `"CJustificador"` · +28 `CJustificadorServico` |

---

## 3. `comum::CAppInfo` e os arquivos de estado

### 3.1 Getters

Todos os getters seguem um mesmo template (srcloc `cappinfo.cpp:42` não const, `:57` const):

```cpp
template <typename ESTADO> ESTADO& GetEstado(const std::string& nome, std::optional<ESTADO>& estado)
{   if (!estado) throw CUeComumAppInfoError(7600 /*7601 const*/, format("O estado não foi carregado: {}", nome));
    return *estado; }
```

| func | método | notas |
|---|---|---|
| 291 / 457 → 6040 | `GetGeral()` / `GetGeral() const` | 6040 é o corpo mesclado; os thunks passam o registro de srcloc e o código de erro |
| 261 | `GetVota(EUrnaTurno)` | `m_vota[IndiceTurno("GetVota", turno)]` |
| 903 | `GetVota() const` | turno atual |
| 2841 | `TemVota(turno)` | `m_vota[...].has_value()` |
| 3788 | `GetHistoricoCargas() const` | de `gap.bin`: o código da carga (+28) de cada um dos ≤10 `CDadoCorrespondencia` (96 B). O BU os imprime como o *histórico de cargas* (QR `HIQT`/`HICA`) |
| 1553 | `static IndiceTurno(funcao, turno)` | `'3'` → `GetGeral().turno`; `'1'`→0, `'2'`→1, senão erro 7602 `"Turno {} invalido: {}"` (srcloc :472). A mensagem mostra o **argumento** turno, não o resolvido: `IndiceTurno(f, '3')` em uma urna sem turno reporta `Turno 3 invalido` |

Predicados auxiliares (funções livres, caminho inferido):
`EhTreinamentoSemTreinamentoEleitor()` (1823) = fase `'3'` && !treinamentoEleitor (treinamento de mesários);
`EhModoDemonstracaoSemTreinamentoEleitor()` (2520) = `CInformacaoEleicao::EhModoDemonstracao()` &&
!(fase `'3'` && treinamentoEleitor). A unidade u09 chama a 2520 de `DeveRegistrarMesarios`, porque ela decide se
os estados de registro de mesários executam.

### 3.2 Gravação de `vota.bin`: `comum::SalvaEstado()` (func 491)

Toda mudança de `EstadoGeralVota.estadoVota` em toda a aplicação (32 chamadores: init, zerésima, registro de
mesários, fim da votação, cópias do BU, cópia do resultado…) é seguida por esta função:

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

A MV **não** é assinada de novo: o seu `vota.vsu` é uma cópia da assinatura da MI, então só confere se as duas
codificações de `vota.bin` forem idênticas byte a byte (e são, já que ambas vêm do mesmo objeto em memória).
`SalvaVotaInterno/Externo` (3790/3789) compartilham o corpo 6039, que não faz nada se o estado do turno atual não
estiver carregado. O caminho do serviço de estado é `CPath::GetPathTrab(midia, turno) / "vota.bin"` (11568; `gap.bin`
11567, `sa.bin` 11565). Pedi-lo com turno `'0'` lança 7605–7607 "Urna sem turno em contexto onde
turno era esperado".

No build web esta função executa (observada durante `votaInit`), mas toda gravação vai para o MEMFS. As
assinaturas são o texto fixo `assinatura simulada para vota_web_wasm`, e `Sync()` é vazio (§5).

---

## 4. Os helpers de `api`

### 4.1 `api::CSystem` e `CDirReader` (csystem.cpp, cdirreader.cpp)

Todos os erros são `CBaseError<api::EUeUtilError>` (códigos 7000–7099, thunk func 346). Os principais pontos de entrada:

| função | comportamento |
|---|---|
| `ExistResource(stat&, path)` (1690) | `stat()`; `ENOENT`/`ENOTDIR` → false; outros erros → 7031 "Falha ao validar a existência do recurso" |
| `CopyFile(orig, dest, preserva)` (378) | não faz nada se os nomes forem iguais. Caso contrário: abre a origem (7042), abre o destino `O_WRONLY\|O_CREAT\|O_TRUNC` + `O_SYNC` quando `CSynchronizer` é síncrono, modo 0644 (7043), copia em blocos de 32 KiB (7044). Se `preserva` estiver ativo: `lstat` da origem (7045), `utime` (7046), dono (7047), modo (7048). O dono e o modo são alterados por `fchown`/`fchmod` sobre um fd (com `opendir`+`dirfd` para diretórios) e recorrem a `chown`/`chmod` em caso de EPERM/EACCES. Sempre termina com `CSynchronizer::Sync()` |
| `ReplaceFile(novo, destino, backup)` (5455) | substituição atômica usada para `rdv.dat` depois de cada voto na urna real. `PrepareReplace`: backup = `backup` ou um nome `mkstemp(novo + ".XXXXXX")` (7034/7035, o arquivo temporário é excluído imediatamente); `novo` deve ser um arquivo regular (7064), diferente de `destino` (7065), e o backup não pode existir (7066). `ReplaceResource`: `destino`→backup (7067), `novo`→`destino` (7068, revertendo antes o primeiro passo), depois o backup é excluído: um diretório recursivamente (5458), um arquivo com `ZeroFill`+`remove` |
| `ZeroFill(path)` (2761) | apenas arquivos regulares: `open(O_WRONLY)` (7069), `ftruncate(0)` e depois `ftruncate(size)` (7070). Ela **não** grava zeros (§10) |
| `IsEmptyDir(dir)` (2762) | deve ser um diretório existente (7036). Está vazio quando nenhuma entrada é um arquivo regular ou um subdiretório diferente de `.`/`..` |
| `GetFileSize(path)` (2759) | `stat().st_size` truncado para 32 bits (7038) |
| `CDirReader` (1914/1260/1913/2232/6020) | wrapper de `opendir`; erro do ctor 7009 "Não foi possível ler o diretório …", `NextEntry` em um leitor fechado 7010; `IsDirectory/IsFile/IsLink` fazem `stat`/`lstat` de dir + "/" + `d_name` |

### 4.2 Datas e horas (cdate.cpp, ctime.cpp, cdatetime.cpp)

* `CDate(const string&)` (3649): aceita `DDMMAA` ou `DDMMAAAA` (`IsValid`, u13). Um ano de 2 dígitos recebe
  2000 somados. A data é então analisada com `strptime("%d%m%Y")`, que neste build é **a implementação
  JavaScript do glue do Emscripten** (`env.strptime`). Erros 7001/7002 "Data inválida [...]".
  `CDate(uebyte dia, uebyte mes, ueint16 ano)` (2765) formata `"{:02}{:02}{:04}"` e delega (os tipos dos
  argumentos de formato empacotados são unsigned, unsigned, int).
* `CDate::Format` (706): `A` → `DOM SEG TER QUA QUI SEX SAB` (tabela @1584308; 7003 se o dia da semana for ≥ 7),
  `DD`/`D`, `MM`/`M`, `YYYY`, `YY`, `Y` (ano % 100); todo outro caractere é copiado.
* `CTime` = segundos desde a meia-noite: `CTime(h,m,s)` (3642, erro 7079 "Hora inválida" se h>23, m>59 ou
  s≥60), `CTime("hhmm[ss]")` (3643, 7080), `operator-(int)` (3641: `operator-=` com `operator+=` inlinado;
  7081 "Overflow" além de 86 399 s, 7082 "Underflow" abaixo de 0).
* `CDateTime("DDMMAAAAhhmm[ss]")` (5475, 7005), `ConvertFromLocalTime(time_t)` (5476, usa **gmtime_r**;
  7006), `ConvertFromTimestamp("AAAAMMDDhhmmss")` (2764, 7008; reordena para `DDMMAAAA…`).

### 4.3 `CGenericTags`, o codec de mensagens do SAVD (cgenerictags.cpp)

Um TLV textual. Cada elemento é `<tag><L><length in 2n hex digits><value>`, em que `L = ';' + n`
(`<` `=` `>` `?` para n = 1..4). `insert` (1071: 7013/7014/7015, n deve ser 1..4), `TagSize` (5468: 7016),
`EncodeTLV` (5467: 7017; 7018 se o valor for mais longo que 2^(8n)−1), `AppendTLV` (5466: 7019), além de
`WalkTreeTLV`/`DecodeTLV` (7020/7021) inlinados no seu único chamador. O único usuário é o cliente do SAVD,
com as tags `pkg`, `key`, `fil` (n=1) e `sup` (n=3):

```
MontaMensagemPacote(pacote, chave)            (3831)  → "sup>" hex6 [ "pkg<" hex2 pacote  "key<" hex2 "<chave>" ]
AssinarVerificarArquivo(app, chave, pkg, fil) (5892)  → the same, with "fil<" hex2 <file> appended inside "sup",
                                                        sent to the SAVD with header 0x10010000 (byte 1 = app)
```

`comum::ValidarUE` (5890, outra unidade) envia a forma `sup{pkg,key}` com o header 0x20210000.
`CPacoteArquivos::ValidarChaveEAplicacaoValida` e `CSigVerifier` usam a 5892. Ambas executaram durante `votaInit`.

### 4.4 Outros helpers

* `CStringUtils::GetVersionNumber` (1539): o Boost.Regex `^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+` deve casar na
  posição 0 (7025). Devolve o trecho casado: `"10.23.0.1 - DESENVOLVIMENTO"` → `"10.23.0.1"` (a linha "Ver:"
  do BU/zerésima e o campo `VERS` do QR).
* `CSynchronizer` (600): o singleton é criado síncrono (`CreateInst(true)`, 7030 "Instância já
  criada"). `Sync()` (620) tem corpo vazio neste build.
* `CTickManager` (3644/5451/700/5452): ids 0..255 (7077 "Não há mais id disponível para tick"), período
  mínimo de 30 ms (7072), `StartTick` rearma a partir de *agora* (7075), `StopTick` (7074). `AddStoppedTick` (5452) é
  o helper de tempo de construção que os estados usam para criar temporizadores que armam depois.
* `CWait(ms)` (5447): prazo = agora + ms, no máximo 4 000 000 ms (7083). A espera ativa em si é `Aguarda`
  (u09, laço de `usleep(200)`, sem `emscripten_sleep`).
* `ITimerScheduler::GetInst/CreateInst<CTimerScheduler>` (1259/5444): registro em `CPolySingletonList`,
  7085 "Tentativa de recriar o singleton".
* `IGenericFactory<AP>` (10839…10872 + 1561): as duas sobrecargas de `Create` que `CDefaultGenericFactory`
  não sobrescreve lançam `EUePatternError` 6761/6762 "Create(...) não sobrecarregado.".

### 4.5 Log (cloga.cpp, cescritorlog.cpp)

`CLoga::loga(app, severidade, texto)` (433) → `CPolySingletonList::instance<CEscritorLog>().loga` (10260):
`app` deve ser < 61 (6950 "{} - O código da aplicação deve ser menor que {} [{}]") → `Escreve` (slot 4,
`simulador::CWasmLogd` → `log/logd.dat`). `fazOperacao(op)` (10262) aceita operações do logd 230..240
(6951/6952) e as envia como um registro com app = op, severidade 1 e texto vazio.
`IEventosLog::LogaInicioAplicacao(turno)` (11641: "Iniciando aplicação - {Sem turno|1º turno|2º turno|Padrão|
Inválido}") e `LogaVersaoAplicacao()` (11642: "Versão da aplicação: 10.23.0.1 - DESENVOLVIMENTO") são
chamadas por `votaInit` através da tabela de funções (slots de invoke 37/38). Os textos são literais Latin-1.

### 4.6 Persistência (cdaorepositorio.hpp, iuenuxgenericdao.h)

`CDAORepositorio` mantém `map<string, shared_ptr<IDAO>>` @1909964, com chave `typeid(I).name()`, e o preenche
durante `votaInit` (inlinado na func 7787) com `CComparecimentoMesarioDAO` e `CJustificadorDAO`.
`Entregar<I>()` procura a chave, chama `IDAO::Clonar()` (slot 2: um novo DAO que compartilha a conexão SQLite
do protótipo), converte-o com `dynamic_cast` e devolve um novo `shared_ptr<I>`. Erro 6802 "Classe DAO {} não
foi registrada." (cdaorepositorio.hpp:79). As duas instanciações são inlinadas nos construtores dos seus
caches: `CRegistradorMesario::GetInst()` (815, singleton de 52 bytes @1909932) e `CJustificador()` (3742,
40 bytes, singleton @1839012 via func 1391).

Padrões de `IUenuxGenericDAO` (2297 = corpo de throw compartilhado): `Excluir` 6804, `ExcluirID` 6805, `Atualizar` 6806
`"<op> não implementada para entidade " + typeid(DADO).name()`. Eles permanecem para
`CComparecimentoMesarioDAO` (sem exclusão/atualização de linhas de presença de mesários), `CJustificadorDAO::Excluir` e
`CEleitorDinamicoDAO::Excluir`.

---

## 5. Particularidades do build web

* **MEMFS**: o `O_SYNC` de `CopyFile`, `chown`/`fchown`, `utime` e `ZeroFill` atuam apenas sobre o sistema de arquivos
  em memória do Emscripten. `CSynchronizer::Sync()` compila para um corpo vazio.
* **`strptime` é JavaScript** (`env.strptime` no glue): `CDate(const string&)` depende da implementação com
  expressões regulares do glue. Para as entradas de 8 dígitos que `CDate::IsValid` deixa passar
  (`%d` = `0[1-9]|[1-9](?!\d)|1\d|2\d|30|31`) ela dá o mesmo dia, mês e ano que um `strptime` em C, exceto para
  anos de 4 dígitos 0000–0099 (§10.5). Ela também preenche `tm_wday` (`new Date(...).getDay()`), que `CDate` guarda como o
  dia da semana usado por `Format("A")`. O `strptime` da musl deixaria o `tm_wday` zerado em 0 (`DOM` para toda data);
  a glibc o calcula.
* `IGenericFactory<IThreadImpl>` cria `simulador::CWasmThread` em vez da thread POSIX.
* O gravador de log é `simulador::CWasmLogd`. As requisições ao SAVD de §4.3 vão para o mock web, e os
  arquivos `.vsu` contêm `assinatura simulada para vota_web_wasm`.
* `CSystem::ReplaceFile` (a substituição de `rdv.dat` a cada voto) nunca executa: a política web
  `CSincronismoVotoEleitorWeb` pula a persistência do voto (u07). `SalvaEstado` executa (init, mudanças de estado).

## 6. Os estados vota / comum desta unidade

(Nomes de slots do protocolo `comum::CAppState` de u06. Os valores de estadoVota são chars, `'1'` + o valor ASN.1.)

| func | estado / método | o que faz |
|---|---|---|
| 10210 | `CSincronismoOperador::StartState` | lado do operador depois de um voto: a menos que seja *treinamento do eleitor*, `CEleitores::MarcaVotou()` (inlinada; 7845 "Item inexistente", 7846 "Dados dinâmicos não carregados"; situação := 3 VOTOU, `++votaram`, `ConfereDadosDinamicos("MarcaVotou")`), depois a mensagem **5** para a thread do eleitor (`CThreadEleitor` +36, prioridade 1) |
| 10734 | `CConfirmaEncerramento::ProcessInput` | CORRIGE: log (severidade 2) "Procedimento de encerramento abortado" → `CPedeIdentidade`. CONFIRMA: log "Procedimento de encerramento confirmado", `CEstadoGeralVota::MarcaFimAquisicao()` (8084 "O fim da aquisição já havia sido registrado", 8085 "O início da aquisição não havia sido registrado"; dhFimAquisicao := agora), estadoVota := `'9'` FIMAQUISICAOVOTOS, `SalvaEstado`, → `CFimAquisicaoVotos` |
| 10797 | `CControladorRegistraMesariosVota` slot 5 (`IniciaRegistro`, nome inferido) | ZERESIMAIMPRESSA → `'7'` REGISTROMESARIOINICIAL; VOTAR → `'7'` somente se o RDV não tem comparecimento, não há justificativa e fase ≠ treinamento; FIMAQUISICAOVOTOS → `':'` REGISTROMESARIOFINAL; `SalvaEstado` depois de uma mudança |
| 11831 / 11830 | `testeteclado::CRetomada` `CriaTela` / `GetEstadoPassouNoTeste` | teste de teclado no reinício. "Não testar" é oferecido se esta urna gerou a zerésima (`urnaIdGerouZeresima == id da urna`) ou se o teclado já foi testado. Em uma urna de *contingência-vota* (tipoUrna `'3'` do turno) a primeira passagem define `tecladoTestadoPosConversao` e salva; em seguida `CReinicioVotacao` |
| 11924 | `CInicioZeresima::StartState` | estadoVota := `'3'` AGUARDAHORAZERESIMA, `SalvaEstado`; → `CQuerImprimirZeresima` no treinamento de eleitores, senão `CConfirmaImpressaoZeresima` (5959: singleton preguiçoso, prazo = uma data-hora configurada (config +544) + 10 800 s) |
| 11920 | `CQuerImprimirZeresima::ProcessInput` | CORRIGE: estadoVota := `'8'` VOTAR, `SalvaEstado`, → `CInicioVotacao` (a votação abre **sem** imprimir a zerésima; apenas no treinamento de eleitores). CONFIRMA → `CConfirmaImpressaoZeresima`. Alcançado a partir de `CInicioZeresima` (11924) e de `CReinicioVotacao::StartState` (11835, no reinício sem comparecimento/justificativa e sem `ze.dat`), ambos somente quando `EhTreinamentoEleitor()` |
| 11931 | `CImpressaoZeresimaTardia::ProcessInput` | "zerésima tardia": CORRIGE: log "Mesário confirma que o horário da urna está errado" → `CInformacaoZeresimaTardia` (CWait 1000 ms). CONFIRMA: estadoVota := `'4'` GERARZE, `SalvaEstado`, log "... está correto" → `CGeraZeresima` |
| 11943 / 11940 | `CGeraResumoZeresimaBase::StartState` / `CGeraResumoZeresima::PosGeracao` | grava `trab/rze.dat` "Resumo da Zerésima" (cabeçalho 5977, extrato do RDV 5971, rodapé 5579), `SincronizaRelatorios("rze.dat")`, `urnaIdGerouZeresima := id da urna`, `SalvaEstado`, log do relatório 11 [INÍCIO]/[TÉRMINO]; depois slot 9 (estadoVota := `'5'` ZERESIMAGERADA + `SalvaEstado`) e slot 10 (próximo estado) |
| 12062 | `CInicioBU::StartState` | primeiro estado do BU: treinamento de eleitores → `CQuerImprimirBU` ("Quer imprimir o BU?"), caso contrário → `CImprimindoBU` |
| 5428 | `CFimAquisicaoVotos::GetInst` | thunk de singleton |

Nenhum destes estados executou nas sessões gravadas: a página web só conduz sessões de eleitor (u06 §10).


## 7. BOLETIM DE URNA: o que esta unidade contribui, passo a passo

A unidade não monta o corpo do BU (u08 `CGeraBU`, geradores de u04/u25). Ela fornece o controle de estado
em torno dele e o **cabeçalho dos QR codes do BU** específico do VOTA. O fluxo de encerramento, em ordem:

1. **O mesário confirma o fim da votação** (tela do MT): `CConfirmaEncerramento::ProcessInput` (10734).
   CONFIRMA: log "Procedimento de encerramento confirmado", `MarcaFimAquisicao()` define
   `EstadoGeralVota.dhFimAquisicao` = agora. Falha se o fim já tiver sido marcado (8084) ou se o início
   (`dhIniAquisicao`, definido quando a votação abriu) estiver ausente (8085). Em seguida estadoVota := FIMAQUISICAOVOTOS `'9'`,
   `SalvaEstado()` (vota.bin + vota.vsu na MI, depois na MV), próximo estado `CFimAquisicaoVotos`.
2. **Registro de mesários no encerramento** (quando o registro de presença está ativo): o slot 5 do controlador (10797)
   passa de FIMAQUISICAOVOTOS → REGISTROMESARIOFINAL `':'` e salva. A contagem mostrada durante o registro
   ("Quantidade de mesários registrados:") vem de `CComparecimentoMesariosDS` (5387 abertura / 10368
   encerramento) sobre `CRegistradorMesario` (815), isto é, a tabela `comparecimento_mesario` de `uenux.db`.
   Essas linhas também alimentam a lista de mesários presentes do BU (u08).
3. **Geração do BU** (`CGeraBU`, 12110, u08) usa desta unidade:
   * `CAppInfo::GetVota()`: define `dhEmissao` (+80) se não definido (outra unidade), `TemVota(turno)` (2841);
   * `CAppInfo::GetHistoricoCargas()` (3788): códigos de carga das correspondências de `gap.bin` = o *histórico
     de cargas* do cabeçalho;
   * `CStringUtils::GetVersionNumber` (1539): "Ver: 10.23.0.1";
   * `CDate::Format` (706) e os construtores de `CDate`/`CTime`/`CDateTime` (3649, 3642/3643, 5475) para as datas;
   * `SalvaEstado()` (491) depois de cada mudança de estadoVota (GERARBU → GERARRELATORIOS …).
4. **QR codes do BU**. O gerador `CGeradorBUQRCodeVota` (ctor 5603, u04) é construído com o
   comparecimento (+416), `m_origemRED` (+418), `m_incluiEmissao` (+419) e a data de emissão (+420).
   O seu `PreencheCabecalho(cab)` virtual (**11242**) preenche estes campos do cabeçalho, cada um como `"TAG:value "`, nesta
   ordem:

   | campo | valor | origem |
   |---|---|---|
   | `ORIG` | `VOTA` (ou `RED` se +418) | flag do gerador |
   | `AGRE` | seções agregadas unidas por `.` (somente se houver) | `CLocal::GetAgregadas()` (chamada duas vezes, a primeira chamada inlinada) |
   | `LOCA` | número do local de votação | `CLocal::GetLocalID()` (1933) |
   | `APTO` | aptos da seção + aptos em TTE (16 bits) | `CEleitores::ContaAptos()` (2823) |
   | `APTS` / `APTT` | aptos da seção / transferidos (TTE) | idem |
   | `COMP` | comparecimento | gerador +416 |
   | `MUNI` | município | `CEstadoGeral` +20 |
   | `HBBM` `HBBG` `HBSB` | eleitores habilitados pela digital / pelo ano de nascimento depois de falha da digital / eleitores sem biometria que votaram | somente se `CLocal::UrnaBiometrica()` (820); contadores de `CEleitores` 2821/1935/2822 |
   | `FALT` | aptos − comparecimento (estouro circular de 16 bits) | |
   | `DTAB` `HRAB` | início da recepção de votos, `YYYYMMDD` / `hhmmss` | `vota.bin dhIniAquisicao` (8090 "Início da aquisição não marcado" se ausente) |
   | `DTFC` `HRFC` | fim da recepção de votos | `vota.bin dhFimAquisicao` (8091 "Fim da aquisição não marcado") |
   | `DTEM` `HREM` | data/hora de emissão | somente se +419 |

   Os quatro campos DT/HR só são gravados quando `TemVota(turno)` é verdadeiro. Os outros campos do cabeçalho, o
   corpo, a divisão em 823 caracteres, a cadeia de hashes SHA-512 `HASH` e a assinatura `ASSI` são de
   `CGeradorBUQRCode::GeraQRCodes` (5604) da u04; ver `docs/bu/qrcode.md`.
5. **Impressão do BU**: `CInicioBU::StartState` (12062) ramifica. No *treinamento do eleitor* o mesário
   recebe a pergunta "Quer imprimir o BU?" (`CQuerImprimirBU`). Caso contrário o BU vai direto para `CImprimindoBU`
   (u09), que imprime as vias e incrementa `qtdBU` com `SalvaEstado()` depois de cada cópia.
6. **Arquivos de resultado e cópia para a MR** (u09 `CGravaResultado`, `CCopiaResultadoParaMR`): usam `CSystem::CopyFile`
   (378), `GetFileSize` (2759), `IsEmptyDir` (2762) e `CAppInfo::GetHistoricoCargas` (3788) desta unidade.

**Zerésima (antes da votação).** `CInicioZeresima` (11924) → estadoVota AGUARDAHORAZERESIMA. Se a zerésima
estiver atrasada, `CImpressaoZeresimaTardia` (11931) pergunta se o relógio está certo (CONFIRMA → GERARZE →
`CGeraZeresima`). `CGeraResumoZeresimaBase::StartState` (11943) grava `rze.dat`, registra o id da urna em
`urnaIdGerouZeresima`, e `CGeraResumoZeresima::PosGeracao` (11940) define ZERESIMAGERADA. No modo de treinamento
de eleitores, `CQuerImprimirZeresima` (11920) permite ao operador pular a impressão e ir direto para VOTAR (ele é alcançado a partir de
`CInicioZeresima` e, depois de um reinício antes de qualquer atividade, a partir de `CReinicioVotacao` 11835).

**RDV (antes do BU).** Depois de cada voto em uma urna real, `CSincronismoVotoEleitor` (u07) substitui `rdv.dat`
com `CSystem::ReplaceFile(rdv.dat.tmp, rdv.dat, "")` (5455): o RDV antigo é renomeado para um nome `mkstemp` e
depois "preenchido com zeros" e removido. Quando o banco de dados dinâmico é regenerado, os antigos `rdv.dat`/`uenux.db` também
são excluídos com `ZeroFill` + `remove` (via func 2760). Ver §10 para entender por que isso não apaga os dados.

---

## 8. Dados lidos e gravados

| dado | formato | quem |
|---|---|---|
| `dinamico/trab<t>/vota.bin` na MI e na MV | BER `ModuloEstadoGeralVota` | `SalvaEstado` (491) → `SalvaVotaInterno/Externo` (3790/3789, `CServicoEstadoGeralVota`) |
| `vota.vsu` (ESavdPacote 122/123 MI, 124/125 MV) | pacote de assinatura do SAVD | assinado na MI (`CAssinador(...).Assina(31)`), copiado para a MV (4687) |
| `gap.bin`, `sa.bin`, `eg.bin` | BER | caminhos por `GetPathArquivo` (11565/11567/11568); correspondências de `gap.bin` lidas por 3788 |
| `trab/rze.dat` | relatório (resumo da zerésima) | 11943 |
| `log/logd.dat` | Latin-1 `app\|sev\|text` | `CLoga` → `CEscritorLog` → `CWasmLogd` |
| tabelas `comparecimento_mesario`, `registro_justificativa` de `uenux.db` | SQLite | DAOs entregues por `CDAORepositorio` (815/3742) |
| requisições ao SAVD | TLV `sup{pkg,key[,fil]}` + header de 8 bytes | 3831/5892 |
| qualquer arquivo | cópia / substituição / exclusão | `CSystem` |

---

## 9. Observações sobre wasm / Emscripten

* **Corpos mesclados (merge-similar-functions do wasm-opt).** A 6040 atende às duas sobrecargas de `GetGeral()` (o registro
  de srcloc e o código de erro são parâmetros). A 6039 atende às duas `SalvaVota*`: o nome de função de 16 bytes é passado
  como duas metades de 8 bytes. A 3896 é o corpo das três `GetPathArquivo` (nome de arquivo como um par `[begin,end)`).
  1561 e 2297 são os corpos de throw dos padrões de pattern / DAO. 1912/2231 são thunks de `IsType`.
* **Eliminação de argumentos mortos** removeu o 4º `bool` de `CopyFile` e o `size_t` de `AppendTLV`,
  cujas assinaturas sobrevivem apenas nos registros de srcloc.
* **O inlining engana os nomes baseados em srcloc.** 261/903/3788 foram nomeadas `GetEstado`, mas são
  getters de `CAppInfo` que inlinam o template `GetEstado`. 815/3742 são `Entregar`, inlinadas em
  construtores. 3641 é `operator-` com `operator-=`/`+=` inlinados. 3642/3643 são construtores de `CTime`
  com `EncodeTime` inlinado. 600 é `GetInst` com `CreateInst` inlinado. 700 é o wrapper `StartTick` da thread
  (`this+20`). 5892 é uma rotina de `IInterfaceSavd` com `WalkTreeTLV`/`DecodeTLV` inlinados.
  10260 é `CEscritorLog::loga` alcançada pela vtable de `CWasmLogd`.
* **Structs de um único int são passadas por valor.** `CTime` (4 bytes) é devolvido como um i32 (func 3641).
* **Verificação de intervalo sem sinal compilada como `(x-5) <=u -5`** em `CGenericTags::insert`. O descompilador a mostra
  como uma comparação com sinal, mas ela significa `x ∉ [1,4]`.
* **Literais de string inline.** Literais curtos são reconstruídos com stores de 8 bytes a partir de uma base de dados (por ex.
  `d_operator…[121855]` + 1024 = "Erro de sincronização"). Os textos de log da 11641 são armazenados em Latin-1
  (`"1\xBA turno"`, `"Inv\xE1lido"`), o que confere com o `logd.dat` em Latin-1.
* **Código da musl na unidade:** `closedir` (957), `utime` (6238), e `mkstemp`/`__randname` e
  `exp2` inlinados (em `EncodeTLV`). Não há `emscripten_sleep` em nenhum lugar da unidade.

---

## 10. Código estranho ou arriscado

1. **`CSystem::ZeroFill` não sobrescreve nada** (func 2761, mesmo código-fonte da urna real). O nome
   diz "preencher com zeros", mas o corpo é `ftruncate(fd, 0); ftruncate(fd, size)`. Em um sistema de arquivos Linux
   isso produz um arquivo esparso de zeros e apenas **libera** os blocos antigos; os bytes antigos permanecem na mídia
   até serem reutilizados, e o nivelamento de desgaste da flash os mantém por ainda mais tempo. O mesmo "ZeroFill + remove" é a forma como a
   aplicação exclui: o `rdv.dat` anterior depois de cada voto (`ReplaceFile`, 5455, via
   `CSincronismoVotoEleitor`), os antigos `rdv.dat`/`uenux.db` quando os dados dinâmicos são regenerados
   (func 2760), e os backups de diretório (5458). Se a urna real compila este código da mesma forma, versões anteriores do RDV
   (uma por voto) podem sobreviver na flash não alocada. Quem conseguir ler a mídia bruta e
   decifrar o RDV poderia comparar versões consecutivas e descobrir a ordem dos votos. Isso é uma
   preocupação com o sigilo do voto, não um vazamento comprovado (o sistema de arquivos e a proteção da chave do RDV não foram
   examinados). Sem efeito no simulador (MEMFS).
2. **`CWait` estoura em builds de 32 bits** (5447). `m_microssegundos = ms * 1000` é um `size_t` de 32 bits em
   wasm32 e é somado ao `tv_usec` de 32 bits. Para `ms` acima de cerca de 2 146 484–2 147 483 (~36 min, o
   limite exato depende do `tv_usec` atual), que ainda está abaixo do máximo permitido de 4 000 000 ms
   (verificado com `i32.ge_u`), a soma excede `INT_MAX`. Ela fica negativa, a normalização (com sinal,
   `> 1000000`, `i32.ge_s`) é pulada, e o prazo cai no passado, então `Aguarda` (5446: para quando
   `now.sec > limite.sec` ou segundos iguais e `now.usec >= limite.usec`) termina imediatamente.
   Os chamadores atuais usam 1000 e 2000 ms, então nada é afetado hoje. Um build em que `long`/
   `suseconds_t` seja de 64 bits não teria o problema.
3. **`FALT` (eleitores faltosos) é aritmética de 16 bits** no cabeçalho do QR do BU (11242):
   `(ueword)(aptosSeção + aptosTTE − comparecimento)`. Se o comparecimento algum dia exceder os eleitores contados como
   *aptos*, o QR code mostra um número próximo de 65 535 em vez de um valor negativo ou zero. Não existe verificação.
4. **No treinamento de eleitores, a zerésima pode ser pulada e a votação aberta** (11920: CORRIGE em "Quer imprimir
   a zerésima?" → estadoVota VOTAR → `CInicioVotacao`). Dois estados levam até lá: `CInicioZeresima` (11924) e
   `CReinicioVotacao::StartState` (11835: reinício sem comparecimento no RDV, sem justificativa e sem
   `trab/ze.dat`). Ambos fazem isso somente quando `EhTreinamentoEleitor()` (func 697) é verdadeiro (fase `'3'` e
   `vota.bin.treinamentoEleitor`), então uma urna oficial não pode seguir esse caminho a menos que o seu `vota.bin` assinado
   indique treinamento de eleitores. Da mesma forma, `CInicioBU` (12062)
   torna a impressão do BU opcional somente nesse modo.
5. **`strptime` é JavaScript no build web** (`CDate(const string&)`, 3649). O glue monta o resultado
   com `new Date(year, …)`, que mapeia os anos 0–99 para 1900–1999, enquanto a musl os manteria. Anos de quatro dígitos
   abaixo de 100 só vêm de arquivos de dados malformados, então esta é uma divergência exclusiva do web com pouco
   efeito prático.
6. **`ReplaceFile` usa o padrão mktemp** (5455): `mkstemp` cria o nome do backup, exclui o arquivo
   imediatamente e reutiliza o nome depois de uma verificação com `stat` (time-of-check/time-of-use). Isso é inofensivo na
   urna de aplicação única, mas a proteção `O_EXCL` de `mkstemp` é jogada fora.
7. **Texto de erro de `IsEmptyDir`** (2762): quando o caminho existe mas não é um diretório, a mensagem é
   `strerror(errno)` com um `errno` antigo, então o motivo informado está errado. Afeta apenas o diagnóstico.
8. **Segurança simulada nos fluxos desta unidade (build web).** `SalvaEstado` assina `vota.bin` por meio de
   `CAssinador`, e o `.vsu` gravado é o literal `assinatura simulada para vota_web_wasm`. As requisições TLV ao SAVD
   (3831/5892) são respondidas pelo mock web, e `CSynchronizer::Sync()` é vazio. Isso é
   esperado em um simulador; está listado para que os fluxos não sejam confundidos com verificações de integridade reais.

## 11. Questões em aberto

* Nomes reais das classes armazenadas em @1909932 / @1839012. Aqui elas são chamadas de `CRegistradorMesario` e
  `CJustificador` por causa das strings que armazenam, o que sugere uma base comum (um cache nomeado sobre um
  serviço/DAO) cuja RTTI está ausente porque ela não é polimórfica.
* O slot 5 de `comum::IControladorRegistraMesarios` (10797, chamado aqui de `IniciaRegistro`) e as duas codificações
  de período: `vf6` mapeia estadoVota `'7' '8' '9' ':'` para 1, 2, 0, 3, enquanto as contagens de presença usam
  os períodos 1 e 2.
* O arquivo de `SalvaEstado()` (491), `EhTreinamentoSemTreinamentoEleitor` (1823) e
  `EhModoDemonstracaoSemTreinamentoEleitor` (2520). Elas foram colocadas em `cappinfo.cpp`, mas poderiam estar em
  um arquivo utilitário de `comum`.
* `CGenericTags` +4 (sempre 3 = largura do nome da tag) duplica `m_tamanhoTag - 1`. É um argumento separado do
  construtor ou um membro derivado?
* O header de 8 bytes das requisições ao SAVD (`0x10010000` com byte 1 = aplicação) deveria ser decodificado com base em
  `IInterfaceSavd` (unidades u01/u02).
* Bytes +488..+532 de `CAppInfo` (um `optional<CEstadoGeralSA>`?).

---

## 12. Tabela de mapeamento completa (109 funções)

`ran` = vista em execução nas sessões gravadas. As linhas "biblioteca/helper inlinado" são código da musl / libc++.

| func | tamanho | ran | nome nas ferramentas | símbolo reconstruído | reconstruído em | arquivo original | conf. |
|---:|---:|:-:|---|---|---|---|---|
| 261 | 609 | ✓ | `comum::GetEstado@261` | `comum::CAppInfo::GetVota(EUrnaTurno) [GetEstado<CEstadoGeralVota> inlined]` | src/uenux2/src/app/comum/appinfo/cappinfo.cpp | uenux2/src/app/comum/appinfo/cappinfo.cpp | alta |
| 291 | 15 | ✓ | `comum::GetEstado@291` | `comum::CAppInfo::GetGeral()` | src/uenux2/src/app/comum/appinfo/cappinfo.cpp | uenux2/src/app/comum/appinfo/cappinfo.cpp | alta |
| 346 | 18 |  | `api_f346` | `CBaseError<api::EUeUtilError> constructor thunk (CUeUtilError)` | src/uenux2/src/api/util/csystem.cpp (comentário) | helper de exceção do ecourna (instância de EUeUtilError) | alta |
| 378 | 4665 | ✓ | `api::CSystem::CopyFile` | `api::CSystem::CopyFile` | src/uenux2/src/api/util/csystem.cpp | uenux2/src/api/util/csystem.cpp | alta |
| 433 | 68 | ✓ | `api::CLoga::loga` | `api::CLoga::loga` | src/uenux2/src/api/uelog/cloga.cpp | uenux2/src/api/uelog/cloga.cpp | alta |
| 457 | 15 | ✓ | `comum::GetEstado@457` | `comum::CAppInfo::GetGeral() const` | src/uenux2/src/app/comum/appinfo/cappinfo.cpp | uenux2/src/app/comum/appinfo/cappinfo.cpp | alta |
| 491 | 1329 | ✓ | `comum_f491` | `comum::SalvaEstado()` | src/uenux2/src/app/comum/appinfo/cappinfo.cpp | uenux2/src/app/comum/appinfo/cappinfo.cpp (caminho inferido) | média |
| 600 | 143 |  | `api::CSynchronizer::CreateInst` | `api::CSynchronizer::GetInst() [CreateInst(true) inlined]` | src/uenux2/src/api/util/csynchronizer.cpp | uenux2/src/api/util/csynchronizer.cpp | alta |
| 700 | 639 | ✓ | `api::CTickManager::StartTick` | `api::CTickManager::StartTick [via CThreadVota::StartTick, +20]` | src/uenux2/src/api/util/ctickmanager.cpp | uenux2/src/api/util/ctickmanager.cpp | alta |
| 706 | 1982 | ✓ | `api::CDate::Format` | `api::CDate::Format` | src/uenux2/src/api/util/cdate.cpp | uenux2/src/api/util/cdate.cpp | alta |
| 815 | 1261 |  | `api::persistencia::CDAORepositorio::Entregar@815` | `comum::CRegistradorMesario::GetInst() [CDAORepositorio::Entregar<IComparecimentoMesarioDAO> inlined]` | src/uenux2/src/app/comum/u20-foreign-fragments.cpp | uenux2/src/app/comum/comparecimentomesario/cregistradormesario.cpp (caminho inferido) | média |
| 903 | 609 |  | `comum::GetEstado@903` | `comum::CAppInfo::GetVota() const` | src/uenux2/src/app/comum/appinfo/cappinfo.cpp | uenux2/src/app/comum/appinfo/cappinfo.cpp | alta |
| 957 | 17 |  | `api_f957` | `musl closedir(DIR*)` | biblioteca/helper inlinado | musl libc (closedir.c) | alta |
| 1071 | 2048 |  | `api::CGenericTags::insert` | `api::CGenericTags::insert` | src/uenux2/src/api/util/cgenerictags.cpp | uenux2/src/api/util/cgenerictags.cpp | alta |
| 1259 | 70 | ✓ | `api::ITimerScheduler::GetInst` | `api::ITimerScheduler::GetInst` | src/uenux2/src/api/util/itimerscheduler.cpp | uenux2/src/api/util/itimerscheduler.cpp | alta |
| 1260 | 87 |  | `api::CDirReader::NextEntry` | `api::CDirReader::NextEntry` | src/uenux2/src/api/util/cdirreader.cpp | uenux2/src/api/util/cdirreader.cpp | alta |
| 1539 | 803 |  | `api::CStringUtils::GetVersionNumber` | `api::CStringUtils::GetVersionNumber` | src/uenux2/src/api/util/cstringutils.cpp | uenux2/src/api/util/cstringutils.cpp | alta |
| 1553 | 578 |  | `comum::CAppInfo::IndiceTurno` | `comum::CAppInfo::IndiceTurno` | src/uenux2/src/app/comum/appinfo/cappinfo.cpp | uenux2/src/app/comum/appinfo/cappinfo.cpp | alta |
| 1561 | 215 |  | `api_f1561` | `api::detail::LancaNaoSobrecarregado (shared throw body of IGenericFactory defaults)` | src/uenux2/src/api/pattern/igenericfactory.h | uenux2/src/api/pattern/igenericfactory.h | média |
| 1690 | 591 | ✓ | `api::ExistResource` | `api::ExistResource` | src/uenux2/src/api/util/csystem.cpp | uenux2/src/api/util/csystem.cpp | alta |
| 1710 | 18 |  | `comum_f1710` | `CBaseError<comum::EUeComumAppInfoError> constructor thunk` | src/uenux2/src/app/comum/appinfo/cappinfo.h (comentário) | helper de exceção do ecourna (instância de EUeComumAppInfoError) | alta |
| 1715 | 18 |  | `api_f1715` | `CBaseError<api::EUePersistenciaError> constructor thunk` | src/uenux2/src/api/persistencia/cdaorepositorio.hpp (comentário) | helper de exceção do ecourna (instância de EUePersistenciaError) | alta |
| 1823 | 39 |  | `vota_f1823` | `comum::EhTreinamentoSemTreinamentoEleitor()` | src/uenux2/src/app/comum/appinfo/cappinfo.cpp | uenux2/src/app/comum/appinfo/cappinfo.cpp (caminho inferido) | média |
| 1912 | 11 |  | `ecourna_f1912` | `api::CDirReader::IsDirectory() [IsType(S_IFDIR)]` | src/uenux2/src/api/util/cdirreader.h | uenux2/src/api/util/cdirreader.h | média |
| 1913 | 101 |  | `api_f1913` | `api::CDirReader::~CDirReader` | src/uenux2/src/api/util/cdirreader.cpp | uenux2/src/api/util/cdirreader.cpp | média |
| 1914 | 127 |  | `api::CDirReader::CDirReader` | `api::CDirReader::CDirReader` | src/uenux2/src/api/util/cdirreader.cpp | uenux2/src/api/util/cdirreader.cpp | alta |
| 2130 | 250 |  | `api_f2130` | `std::string::insert(size_t pos, size_t n, char c)` | biblioteca/helper inlinado | libc++ <string> (instanciação) | alta |
| 2231 | 11 |  | `ecourna_f2231` | `api::CDirReader::IsFile() [IsType(S_IFREG)]` | src/uenux2/src/api/util/cdirreader.h | uenux2/src/api/util/cdirreader.h | média |
| 2232 | 74 |  | `api_f2232` | `api::CDirReader::Close` | src/uenux2/src/api/util/cdirreader.cpp | uenux2/src/api/util/cdirreader.cpp | média |
| 2283 | 57 |  | `api_f2283` | `std::__tree<std::__value_type<std::string, uebyte>>::destroy (~CGenericTags map)` | biblioteca/helper inlinado | libc++ <__tree> (instanciação para CGenericTags::m_tags) | alta |
| 2297 | 64 |  | `api_f2297` | `api::persistencia::detail::NaoImplementada (shared throw body of IUenuxGenericDAO defaults)` | src/uenux2/src/api/persistencia/iuenuxgenericdao.h | uenux2/src/api/persistencia/iuenuxgenericdao.h | média |
| 2520 | 81 |  | `comum_f2520` | `comum::EhModoDemonstracaoSemTreinamentoEleitor() (u09: DeveRegistrarMesarios)` | src/uenux2/src/app/comum/appinfo/cappinfo.cpp | uenux2/src/app/comum/appinfo/cappinfo.cpp (caminho inferido) | média |
| 2759 | 573 |  | `api::CSystem::GetFileSize` | `api::CSystem::GetFileSize` | src/uenux2/src/api/util/csystem.cpp | uenux2/src/api/util/csystem.cpp | alta |
| 2761 | 841 | ✓ | `api::CSystem::ZeroFill` | `api::CSystem::ZeroFill` | src/uenux2/src/api/util/csystem.cpp | uenux2/src/api/util/csystem.cpp | alta |
| 2762 | 919 |  | `api::CSystem::IsEmptyDir` | `api::CSystem::IsEmptyDir` | src/uenux2/src/api/util/csystem.cpp | uenux2/src/api/util/csystem.cpp | alta |
| 2764 | 1283 | ✓ | `api::CDateTime::ConvertFromTimestamp` | `api::CDateTime::ConvertFromTimestamp` | src/uenux2/src/api/util/cdatetime.cpp | uenux2/src/api/util/cdatetime.cpp | alta |
| 2765 | 424 | ✓ | `api_f2765` | `api::CDate::CDate(uebyte dia, uebyte mes, ueint16 ano)` | src/uenux2/src/api/util/cdate.cpp | uenux2/src/api/util/cdate.cpp | média |
| 2841 | 107 |  | `comum_f2841` | `comum::CAppInfo::TemVota(EUrnaTurno) const` | src/uenux2/src/app/comum/appinfo/cappinfo.cpp | uenux2/src/app/comum/appinfo/cappinfo.cpp | média |
| 3601 | 18 |  | `api_f3601` | `CBaseError<api::EUeUeLogError> constructor thunk` | src/uenux2/src/api/uelog/cescritorlog.cpp (comentário) | helper de exceção do ecourna (instância de EUeUeLogError) | alta |
| 3607 | 12 |  | `comum::servico::CComparecimentoMesarioServico::vf0` | `comum::servico::CComparecimentoMesarioServico::~CComparecimentoMesarioServico` | src/uenux2/src/app/comum/u20-foreign-fragments.cpp | uenux2/src/app/comum/servico/ccomparecimentomesarioservico.cpp (caminho inferido) | média |
| 3641 | 286 |  | `api::CTime::operator+=` | `api::CTime::operator-(int) const [operator-= / operator+= inlined]` | src/uenux2/src/api/util/ctime.cpp | uenux2/src/api/util/ctime.cpp | média |
| 3642 | 103 |  | `api::EncodeTime@3642` | `api::CTime::CTime(uebyte, uebyte, uebyte) [EncodeTime inlined]` | src/uenux2/src/api/util/ctime.cpp | uenux2/src/api/util/ctime.cpp | média |
| 3643 | 452 | ✓ | `api::EncodeTime@3643` | `api::CTime::CTime(const std::string&) [EncodeTime inlined]` | src/uenux2/src/api/util/ctime.cpp | uenux2/src/api/util/ctime.cpp | média |
| 3644 | 939 | ✓ | `api::CTickManager::AddTick` | `api::CTickManager::AddTick [SearchNextId inlined]` | src/uenux2/src/api/util/ctickmanager.cpp | uenux2/src/api/util/ctickmanager.cpp | alta |
| 3645 | 608 |  | `api_f3645` | `api::(anonymous)::RenameResource` | src/uenux2/src/api/util/csystem.cpp | uenux2/src/api/util/csystem.cpp | média |
| 3647 | 20 | ✓ | `api_f3647` | `api::CGenericTags::EncodeTLV(tag, valor) [= EncodeTLV(tag, valor, TagSize(tag))]` | src/uenux2/src/api/util/cgenerictags.h | uenux2/src/api/util/cgenerictags.cpp | média |
| 3649 | 769 | ✓ | `api::CDate::CDate` | `api::CDate::CDate(const std::string&)` | src/uenux2/src/api/util/cdate.cpp | uenux2/src/api/util/cdate.cpp | alta |
| 3742 | 1096 |  | `api::persistencia::CDAORepositorio::Entregar@3742` | `comum::CJustificador::CJustificador() [CDAORepositorio::Entregar<IJustificadorDAO> inlined]` | src/uenux2/src/app/comum/u20-foreign-fragments.cpp | uenux2/src/app/comum/justificativa/cjustificador.cpp (caminho inferido) | média |
| 3788 | 825 |  | `comum::GetEstado@3788` | `comum::CAppInfo::GetHistoricoCargas() const [GetEstado<CEstadoGeralGap> inlined]` | src/uenux2/src/app/comum/appinfo/cappinfo.cpp | uenux2/src/app/comum/appinfo/cappinfo.cpp | média |
| 3831 | 691 | ✓ | `api_f3831` | `comum::MontaMensagemPacote(pacote, chave) (SAVD 'sup' TLV)` | src/uenux2/src/app/comum/u20-foreign-fragments.cpp | uenux2/src/app/comum/iinterfacesavd.cpp (caminho inferido) | média |
| 4687 | 221 | ✓ | `comum_f4687` | `comum::CopiaAssinaturaEstadoVotaParaMV()` | src/uenux2/src/app/comum/appinfo/cappinfo.cpp | uenux2/src/app/comum/appinfo/cappinfo.cpp (caminho inferido) | média |
| 5380 | 59 |  | `api_f5380` | `comum::CRegistradorMesario::~CRegistradorMesario` | src/uenux2/src/app/comum/u20-foreign-fragments.cpp | uenux2/src/app/comum/comparecimentomesario/cregistradormesario.cpp (caminho inferido) | média |
| 5387 | 454 |  | `api::CDataTextFmt<comum::(anonymous namespace)::CComparecimentoMesariosDS>::vf2@5387` | `api::CDataTextFmt<comum::(anon)::CComparecimentoMesariosDS>::Text (periodo 1)` | src/uenux2/src/app/comum/u20-foreign-fragments.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cpedetitulomesario.cpp (caminho inferido) | média |
| 5428 | 22 |  | `vota_f5428` | `vota::CFimAquisicaoVotos::GetInst() (thunk of merged singleton body 764)` | src/uenux2/src/app/vota/u20-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/fimvotacao/cfimaquisicaovotos.cpp (caminho inferido) | média |
| 5444 | 157 |  | `api::ITimerScheduler::CreateInst` | `api::ITimerScheduler::CreateInst<api::CTimerScheduler>` | src/uenux2/src/api/util/itimerscheduler.h | uenux2/src/api/util/itimerscheduler.h | alta |
| 5447 | 573 |  | `api::CWait::CWait` | `api::CWait::CWait(size_t)` | src/uenux2/src/api/util/cwait.cpp | uenux2/src/api/util/cwait.cpp | alta |
| 5451 | 548 |  | `api::CTickManager::StopTick` | `api::CTickManager::StopTick` | src/uenux2/src/api/util/ctickmanager.cpp | uenux2/src/api/util/ctickmanager.cpp | alta |
| 5452 | 18 | ✓ | `vota_f5452` | `api::CTickManager::AddStoppedTick(size_t)` | src/uenux2/src/api/util/ctickmanager.cpp | uenux2/src/api/util/ctickmanager.cpp | média |
| 5455 | 4258 |  | `api::(anonymous namespace)::PrepareReplace` | `api::CSystem::ReplaceFile [PrepareReplace, ReplaceResource, TemporaryPath, mkstemp inlined]` | src/uenux2/src/api/util/csystem.cpp | uenux2/src/api/util/csystem.cpp | média |
| 5456 | 105 |  | `api_f5456` | `std::make_format_args(string, string, const char*)` | biblioteca/helper inlinado | libc++ <format> (instanciação) | alta |
| 5457 | 543 |  | `api::truncate` | `api::truncate(int, off_t, const std::string&)` (estático de arquivo) | src/uenux2/src/api/util/csystem.cpp | uenux2/src/api/util/csystem.cpp | alta |
| 5458 | 1367 |  | `api_f5458` | `api::(anonymous)::RemoveConteudoDiretorio` | src/uenux2/src/api/util/csystem.cpp | uenux2/src/api/util/csystem.cpp | média |
| 5460 | 73 |  | `api_f5460` | `std::operator+(std::string&&, const std::string&)` | biblioteca/helper inlinado | libc++ <string> (instanciação) | alta |
| 5466 | 627 | ✓ | `api::CGenericTags::AppendTLV` | `api::CGenericTags::AppendTLV` | src/uenux2/src/api/util/cgenerictags.cpp | uenux2/src/api/util/cgenerictags.cpp | alta |
| 5467 | 2505 | ✓ | `api::CGenericTags::EncodeTLV` | `api::CGenericTags::EncodeTLV(tag, valor, tamanho)` | src/uenux2/src/api/util/cgenerictags.cpp | uenux2/src/api/util/cgenerictags.cpp | alta |
| 5468 | 534 |  | `api::CGenericTags::TagSize` | `api::CGenericTags::TagSize` | src/uenux2/src/api/util/cgenerictags.cpp | uenux2/src/api/util/cgenerictags.cpp | alta |
| 5475 | 909 | ✓ | `api::CDateTime::CDateTime` | `api::CDateTime::CDateTime(const std::string&)` | src/uenux2/src/api/util/cdatetime.cpp | uenux2/src/api/util/cdatetime.cpp | alta |
| 5476 | 201 | ✓ | `api::CDateTime::ConvertFromLocalTime` | `api::CDateTime::ConvertFromLocalTime` | src/uenux2/src/api/util/cdatetime.cpp | uenux2/src/api/util/cdatetime.cpp | alta |
| 5892 | 3294 | ✓ | `api::CGenericTags::WalkTreeTLV` | `comum::IInterfaceSavd::AssinarVerificarArquivo [CGenericTags::WalkTreeTLV/DecodeTLV inlined]` | src/uenux2/src/app/comum/u20-foreign-fragments.cpp | uenux2/src/app/comum/iinterfacesavd.cpp (caminho inferido) | baixa |
| 5959 | 177 |  | `vota_f5959` | `vota::CConfirmaImpressaoZeresima::GetInst()` | src/uenux2/src/app/vota/u20-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/iniciovotacao/cconfirmaimpressaozeresima.cpp (caminho inferido) | média |
| 6020 | 350 |  | `api_f6020` | `api::CDirReader::IsType(mode_t) const` | src/uenux2/src/api/util/cdirreader.cpp | uenux2/src/api/util/cdirreader.cpp | média |
| 6039 | 150 | ✓ | `comum_f6039` | `comum::CAppInfo::SalvaVota(funcao, midia) (merged body of SalvaVotaInterno/Externo)` | src/uenux2/src/app/comum/appinfo/cappinfo.cpp | uenux2/src/app/comum/appinfo/cappinfo.cpp | média |
| 6040 | 516 | ✓ | `comum_f6040` | `comum::CAppInfo::GetGeral merged body [GetEstado<CEstadoGeral>]` | src/uenux2/src/app/comum/appinfo/cappinfo.cpp | uenux2/src/app/comum/appinfo/cappinfo.cpp | alta |
| 6238 | 79 |  | `comum_f6238` | `musl utime(path, const utimbuf*)` | biblioteca/helper inlinado | musl libc (utime.c) | alta |
| 10210 | 334 |  | `vota::CSincronismoOperador::vf2` | `vota::CSincronismoOperador::StartState [comum::CEleitores::MarcaVotou inlined]` | src/uenux2/src/app/vota/u20-foreign-fragments.cpp | uenux2/src/app/vota/operador/csincronismooperador.cpp (caminho inferido) | média |
| 10260 | 579 | ✓ | `simulador::CWasmLogd::vf3` | `api::CEscritorLog::loga [confereAppValida inlined]` | src/uenux2/src/api/uelog/cescritorlog.cpp | uenux2/src/api/uelog/cescritorlog.cpp | média |
| 10262 | 1069 |  | `api::CEscritorLog::fazOperacao` | `api::CEscritorLog::fazOperacao` | src/uenux2/src/api/uelog/cescritorlog.cpp | uenux2/src/api/uelog/cescritorlog.cpp | alta |
| 10310 | 37 |  | `api_f10310` | `atexit destructor of CRegistradorMesario::s_instancia (@1909932)` | src/uenux2/src/app/comum/u20-foreign-fragments.cpp (comentário) | gerado pelo compilador (unique_ptr estático) | alta |
| 10312 | 13 |  | `comum::servico::CComparecimentoMesarioServico::vf1` | `comum::servico::CComparecimentoMesarioServico deleting destructor` | src/uenux2/src/app/comum/u20-foreign-fragments.cpp | uenux2/src/app/comum/servico/ccomparecimentomesarioservico.cpp (caminho inferido) | média |
| 10368 | 454 |  | `api::CDataTextFmt<comum::(anonymous namespace)::CComparecimentoMesariosDS>::vf2@10368` | `api::CDataTextFmt<comum::(anon)::CComparecimentoMesariosDS>::Text (periodo 2)` | src/uenux2/src/app/comum/u20-foreign-fragments.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cpedetitulomesario.cpp (caminho inferido) | média |
| 10396 | 25 |  | `api::persistencia::IUenuxGenericDAO<comum::md::CComparecimentoMesario, comum::md::CComparecimentoMesarioPK>::Atualizar` | `api::persistencia::IUenuxGenericDAO<CComparecimentoMesario, CComparecimentoMesarioPK>::Atualizar` | src/uenux2/src/api/persistencia/iuenuxgenericdao.h | uenux2/src/api/persistencia/iuenuxgenericdao.h | alta |
| 10397 | 25 |  | `api::persistencia::IUenuxGenericDAO<comum::md::CComparecimentoMesario, comum::md::CComparecimentoMesarioPK>::ExcluirID` | `api::persistencia::IUenuxGenericDAO<CComparecimentoMesario, CComparecimentoMesarioPK>::ExcluirID` | src/uenux2/src/api/persistencia/iuenuxgenericdao.h | uenux2/src/api/persistencia/iuenuxgenericdao.h | alta |
| 10398 | 25 |  | `api::persistencia::IUenuxGenericDAO<comum::md::CComparecimentoMesario, comum::md::CComparecimentoMesarioPK>::Excluir` | `api::persistencia::IUenuxGenericDAO<CComparecimentoMesario, CComparecimentoMesarioPK>::Excluir` | src/uenux2/src/api/persistencia/iuenuxgenericdao.h | uenux2/src/api/persistencia/iuenuxgenericdao.h | alta |
| 10734 | 648 |  | `vota::CConfirmaEncerramento::vf7` | `vota::CConfirmaEncerramento::ProcessInput [CEstadoGeralVota::MarcaFimAquisicao inlined]` | src/uenux2/src/app/vota/u20-foreign-fragments.cpp | uenux2/src/app/vota/operador/outrasopcoes/cconfirmaencerramento.cpp (caminho inferido) | média |
| 10797 | 105 |  | `vota::CControladorRegistraMesariosVota::vf5` | `vota::CControladorRegistraMesariosVota::IniciaRegistro (slot 5)` | src/uenux2/src/app/vota/u20-foreign-fragments.cpp | uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp (caminho inferido) | baixa |
| 10839 | 21 |  | `api::IGenericFactory<api::ISemaphore>::Create@10839` | `api::IGenericFactory<api::ISemaphore>::Create(size_t)` | src/uenux2/src/api/pattern/igenericfactory.h | uenux2/src/api/pattern/igenericfactory.h | alta |
| 10845 | 21 |  | `api::IGenericFactory<api::ISemaphore>::Create@10845` | `api::IGenericFactory<api::ISemaphore>::Create(const std::string&)` | src/uenux2/src/api/pattern/igenericfactory.h | uenux2/src/api/pattern/igenericfactory.h | alta |
| 10856 | 21 |  | `api::IGenericFactory<api::IRWSyncCtl>::Create@10856` | `api::IGenericFactory<api::IRWSyncCtl>::Create(size_t)` | src/uenux2/src/api/pattern/igenericfactory.h | uenux2/src/api/pattern/igenericfactory.h | alta |
| 10863 | 21 |  | `api::IGenericFactory<api::IRWSyncCtl>::Create@10863` | `api::IGenericFactory<api::IRWSyncCtl>::Create(const std::string&)` | src/uenux2/src/api/pattern/igenericfactory.h | uenux2/src/api/pattern/igenericfactory.h | alta |
| 10865 | 21 |  | `api::IGenericFactory<api::ISyncCtl>::Create@10865` | `api::IGenericFactory<api::ISyncCtl>::Create(size_t)` | src/uenux2/src/api/pattern/igenericfactory.h | uenux2/src/api/pattern/igenericfactory.h | alta |
| 10868 | 21 |  | `api::IGenericFactory<api::ISyncCtl>::Create@10868` | `api::IGenericFactory<api::ISyncCtl>::Create(const std::string&)` | src/uenux2/src/api/pattern/igenericfactory.h | uenux2/src/api/pattern/igenericfactory.h | alta |
| 10871 | 21 |  | `api::IGenericFactory<api::IThreadImpl>::Create@10871` | `api::IGenericFactory<api::IThreadImpl>::Create(size_t)` | src/uenux2/src/api/pattern/igenericfactory.h | uenux2/src/api/pattern/igenericfactory.h | alta |
| 10872 | 21 |  | `api::IGenericFactory<api::IThreadImpl>::Create@10872` | `api::IGenericFactory<api::IThreadImpl>::Create(const std::string&)` | src/uenux2/src/api/pattern/igenericfactory.h | uenux2/src/api/pattern/igenericfactory.h | alta |
| 11242 | 10017 |  | `comum::CGeradorBUQRCodeVota::vf2` | `comum::CGeradorBUQRCodeVota::PreencheCabecalho(CCabecalhoQRCode&) const` | src/uenux2/src/app/comum/u20-foreign-fragments.cpp | uenux2/src/app/comum/relatorios/cgeradorbuqrcodevota.cpp (caminho inferido) | média |
| 11467 | 25 |  | `api::persistencia::IUenuxGenericDAO<ecourna::app::dados::CIdentificacaoJustificativa, std::string>::Excluir` | `api::persistencia::IUenuxGenericDAO<ecourna::app::dados::CIdentificacaoJustificativa, std::string>::Excluir` | src/uenux2/src/api/persistencia/iuenuxgenericdao.h | uenux2/src/api/persistencia/iuenuxgenericdao.h | alta |
| 11540 | 25 |  | `api::persistencia::IUenuxGenericDAO<comum::md::CEleitorDinamico, std::string>::Excluir` | `api::persistencia::IUenuxGenericDAO<comum::md::CEleitorDinamico, std::string>::Excluir` | src/uenux2/src/api/persistencia/iuenuxgenericdao.h | uenux2/src/api/persistencia/iuenuxgenericdao.h | alta |
| 11565 | 25 |  | `comum::CServicoEstadoGeralSA::GetPathArquivo` | `comum::CServicoEstadoGeralSA::GetPathArquivo` | src/uenux2/src/app/comum/appinfo/servicos/cservicoestadogeralsa.cpp | uenux2/src/app/comum/appinfo/servicos/cservicoestadogeralsa.cpp | alta |
| 11567 | 25 | ✓ | `comum::CServicoEstadoGeralGap::GetPathArquivo` | `comum::CServicoEstadoGeralGap::GetPathArquivo` | src/uenux2/src/app/comum/appinfo/servicos/cservicoestadogeralgap.cpp | uenux2/src/app/comum/appinfo/servicos/cservicoestadogeralgap.cpp | alta |
| 11568 | 25 |  | `comum::CServicoEstadoGeralVota::GetPathArquivo` | `comum::CServicoEstadoGeralVota::GetPathArquivo` | src/uenux2/src/app/comum/appinfo/servicos/cservicoestadogeralvota.cpp | uenux2/src/app/comum/appinfo/servicos/cservicoestadogeralvota.cpp | alta |
| 11641 | 653 | ✓ | `comum_f11641` | `comum::IEventosLog::LogaInicioAplicacao(EUrnaTurno)` | src/uenux2/src/app/comum/u20-foreign-fragments.cpp | uenux2/src/app/comum/log/ieventoslog.cpp (caminho inferido) | média |
| 11642 | 414 | ✓ | `api_f11642` | `comum::IEventosLog::LogaVersaoAplicacao()` | src/uenux2/src/app/comum/u20-foreign-fragments.cpp | uenux2/src/app/comum/log/ieventoslog.cpp (caminho inferido) | média |
| 11830 | 74 |  | `vota::testeteclado::CRetomada::vf11` | `vota::testeteclado::CRetomada::GetEstadoPassouNoTeste (slot 11)` | src/uenux2/src/app/vota/u20-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cretomada.cpp (caminho inferido) | média |
| 11831 | 92 |  | `vota::testeteclado::CRetomada::vf10` | `vota::testeteclado::CRetomada::CriaTela (slot 10)` | src/uenux2/src/app/vota/u20-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cretomada.cpp (caminho inferido) | média |
| 11920 | 168 |  | `vota::CQuerImprimirZeresima::vf7` | `vota::CQuerImprimirZeresima::ProcessInput` | src/uenux2/src/app/vota/u20-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/iniciovotacao/cquerimprimirzeresima.cpp (caminho inferido) | média |
| 11924 | 41 |  | `vota::CInicioZeresima::vf2` | `vota::CInicioZeresima::StartState` | src/uenux2/src/app/vota/u20-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/iniciovotacao/ciniciozeresima.cpp (caminho inferido) | média |
| 11931 | 655 |  | `vota::CImpressaoZeresimaTardia::vf7` | `vota::CImpressaoZeresimaTardia::ProcessInput` | src/uenux2/src/app/vota/u20-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/iniciovotacao/cimpressaozeresimatardia.cpp (caminho inferido) | média |
| 11940 | 18 |  | `vota::CGeraResumoZeresima::vf9` | `vota::CGeraResumoZeresima::PosGeracao (slot 9)` | src/uenux2/src/app/vota/u20-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/iniciovotacao/cgeraresumozeresima.cpp (caminho inferido) | baixa |
| 11943 | 1731 |  | `vota::CGeraResumoZeresimaBase::vf2` | `vota::CGeraResumoZeresimaBase::StartState` | src/uenux2/src/app/vota/u20-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp (leitura da u09; caminho inferido) | média |
| 12062 | 189 |  | `vota::CInicioBU::vf2` | `vota::CInicioBU::StartState` | src/uenux2/src/app/vota/u20-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/fimvotacao/ciniciobu.cpp (caminho inferido) | média |
