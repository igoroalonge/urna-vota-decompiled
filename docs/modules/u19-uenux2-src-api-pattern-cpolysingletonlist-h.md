# u19: `api::CPolySingletonList`, o registro de "poly-singletons", e o código que as ferramentas arquivaram junto com ele

A unidade u19 tem 184 funções wasm que as ferramentas atribuíram a um único arquivo original,
`uenux2/src/api/pattern/cpolysingletonlist.h`. Esse header é o **registro de serviços do aplicativo
da urna**. O código pede "o" `api::IScreen`, `api::IInputKbd`, `comum::IInterfaceSavd`,
`vota::IExecucaoVota` e assim por diante, e recebe a implementação que tiver sido registrada para aquela interface.
Essa é a costura que permite que o mesmo aplicativo de votação rode na urna (drivers reais) e no navegador
(os mocks `simulador::CWasm*` e `api::teste::*`).

A unidade se divide assim:

| grupo | funções | o que são |
|---|---:|---|
| `CPolySingleton<T>::instance(info, loc)` | 30 | uma por interface, com `CPolySingletonList::instance<T>` e o lock inlinados. As ferramentas as nomearam `api::CPolySingletonList::instance@N` |
| `CPolySingletonList::push<T>` / `replace<T>` | 18 | corpos de registro: 4 `push` isolados (5384, 5395, 5398, 5407) + seu corpo mesclado 3882; 9 wrappers de registrar-ou-substituir com `push` inlinado (7667, 8758…10090) e 1 sem (8728); o helper por valor 4162 e suas duas versões totalmente inlinadas 4879/4890 (§3.4) |
| `CPolySingletonList::exists<T>` | 37 | 6 corpos, 23 thunks e 8 instanciações únicas |
| outros membros do registro | 16 | `find`, `erase`, `log`, `trace`, o back end syslog/printf, o lock de leitura/escrita, o construtor do erro, os destrutores da entrada e do registro |
| helpers da libc++ | 48 | helpers instanciados para o registro (vector, `shared_ptr`, empacotamento de argumentos de `std::format`), dois para o map de ícones de bateria e alguns genéricos |
| **funções de outros arquivos** | 35 | funções de outras classes com um `GetInst()` inlinado nelas (o motivo pelo qual as ferramentas as colocaram aqui). Entre elas: o **bootstrap de hardware do simulador** (func 8302), a **fonte de dados do QR code do BU** (func 1956), o ícone de bateria, o front end de TTS e telas do operador (mesário) |

82 das 184 foram executadas durante os votos gravados (`analysis/runtime/*.functions.tsv`). Todos os registros
da inicialização, o ícone de bateria do cabeçalho de status, as buscas do lado do eleitor (tela, teclado, bipe, som,
log) e `CSincronismoEleitor::ProcessMessage` estão entre elas.

Arquivos-fonte reconstruídos:

```
src/uenux2/src/api/pattern/cpolysingletonlist.h          the registry, the lock, instance/exists/push/replace/erase/find (templates)
src/uenux2/src/api/pattern/cpolysingleton.h              CPolySingleton<T>::instance(info, loc) + list of the 30 instantiations
src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp    log() / LogUenux() bodies + index of every instantiation
src/uenux2/mock/app/simulador/wasm/csimuladorwasm.u19.cpp  func 8302: platform singletons of the web build (path inferred)
src/uenux2/mock/app/simulador/wasm/cwasmclp.u19.cpp      CLp::GetInst / ~CLp / static dtor (3876, 5978, 12016)
src/uenux2/src/app/vota/eleitor/fimvotacao/cmostraqrcodebu.u19.cpp   GetQRDSInst (BU QR parts), QRCodeAtual, TextoInstrucaoQRCode
src/uenux2/src/app/vota/iexecucaovota.u19.cpp            IExecucaoVota::GetInst (3594)
src/uenux2/src/app/comum/validamidia/cvalidamidia.u19.cpp  IValidaMidia::GetInst (5575)
src/uenux2/src/api/util/isystemdatetime.u19.cpp          ISystemDateTime::GetInst + CreateInst<T> (1155)
src/uenux2/src/api/util/iajustedatahora.u19.cpp          IAjusteDataHora::CreateInst (10876) (path inferred)
src/uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.u19.cpp  GetInst (599) + outlined calls (3623, 5415, 10584, 10670)
src/uenux2/src/app/vota/operador/u19-foreign-fragments.cpp   MT ProcessInput of 7 operator states (+ merged 6015, 1536)
src/uenux2/src/app/vota/eleitor/u19-foreign-fragments.cpp    CConfirmaVotoNominal slot 17 (5925), CSincronismoEleitor::ProcessMessage (7181)
src/uenux2/src/app/comum/comparecimentomesario/u19-foreign-fragments.cpp   merged ProcessInput 6011
src/uenux2/src/app/vota/monitor/cthreadmonitor.u19.cpp   std::async lambda of SaiPorVotacaoSuspensa (10224)
src/uenux2/src/api/gui/cpowerinformation.u19.cpp         battery icon table/state/timer (2773, 2774, 10981, 10987, 10991)
src/uenux2/src/api/audio/itexttospeech.u19.cpp           ITextToSpeech slot 0 (11530)
```

---

## 1. Propósito e os termos em português

* **Poly-singleton**: o nome que o TSE dá a "um singleton escolhido por interface" (singleton polimórfico). Todo
  serviço da plataforma é uma interface abstrata (`api::I…`). Uma implementação por interface é registrada na
  inicialização (`push`) e obtida em todos os outros lugares (`instance`). Muitas interfaces também têm um `GetInst()` que
  registra uma implementação padrão da urna na primeira vez em que é chamado (§3.4).
* **urna / UE (urna eletrônica)**: o equipamento de votação. **mesário**: membro da mesa receptora de votos. **MT (microterminal)**:
  o pequeno teclado com LCD do mesário. **eleitor**: a pessoa que vota.
* **BU (boletim de urna)**: o resultado assinado de cada urna. **BU digital**: o BU na forma de QR codes (§7).
  **RDV (registro digital do voto)**: o registro embaralhado dos votos.
* **SAVD**: a interface do módulo de segurança (`comum::IInterfaceSavd`). **MR (mídia de resultado)**: o
  cartão de memória dos resultados.

No processo de votação, esta unidade é o encanamento por baixo de tudo. Quando o eleitor pressiona uma tecla, a thread
a lê através de `CPolySingleton<IInputKbd>::instance`, desenha através de `…<IScreen>`, bipa através de
`…<IBeep>` e grava log através de `…<CEscritorLog>`. No fim do dia, a fonte de dados do QR do BU fica neste
registro.

## 2. Estruturas de dados e relações entre classes

### 2.1 O registro

```
TPolySingletonsInfo   (148 bytes, one static instance @0x1BF600 = 1832448, guard byte @1832596)
  +0   std::vector<SPolySingleton> lista
  +12  bool debug                     set to true by the accessor on every call (see §5)
  +16  CUpgradeMutex mutex            (132 bytes)

SPolySingleton  (24 bytes; the name of the struct is inferred)
  +0   std::string nome               typeid(INTERFACE).name(), e.g. "N3api7IScreenE"
  +12  const std::type_info* tipo     &typeid(INTERFACE)
  +16  std::shared_ptr<void> instancia  control block __shared_ptr_pointer<INTERFACE*, default_delete<INTERFACE>>

CUpgradeMutex  (name inferred; a writer-preferring read/write lock with "upgrade")
  +0   std::mutex                     +24 condition_variable (readers' gate)   +72 condition_variable (writers' gate)
  +120 int readers                    +124 int writers waiting                +128 bool writer active
```

* **Acessor.** O código nunca nomeia o registro estático diretamente. Ele chama um *ponteiro de função* armazenado nos dados
  em **@1526320** (slot 284 da tabela → func 11265). Assim, todo "`GetPolySingletonsInfo()`" no binário é um
  `call_indirect(d_…[0])`. A func 11265 retorna a variável estática local e **armazena `debug = true` a cada chamada**,
  fora da guarda de inicialização estática.
* A **busca** é linear. `std::find_if` (func 6779) percorre as entradas de 24 bytes comparando chaves `std::string`.
  Depois, `*it->tipo != typeid(T)` (a libc++ compara os ponteiros `__type_name`).
* **Posse.** As entradas guardam `shared_ptr`s, mas `instance()` retorna um `T&` cru. Os chamadores guardam referências
  simples, e nada as rastreia.

### 2.2 Classes de erro (RTTI)

```
ecourna::api::exception::CError
 ├─ CBaseError<api::EUePatternError, SErrorLimits{6750, 6800}>        typeinfo @1526364, vtable @1526448, ctor thunk 235
 │     6754 "{}: solicitada uma instância não criada de {}"               CPolySingletonList::instance  line 99
 │     6755 "{}: instância {} não corresponde a interface solicitada de {}"  instance                  line 105
 │     6756 "{}: instância já criada de {}"                                 push                      line 129
 └─ CBaseError<ecourna::api::pattern::EPatternError, SErrorLimits{1300, 1325}>  typeinfo @1526600, vtable @1526620, ctor thunk shared_f331
       1301 "PolySingleton - solicitada uma instancia nao criada <mangled> [<file>:<line>]"   cpolysingleton.h:78
std::runtime_error: "unlock_shared() called with no shared owner", "unlock() called when no writer is active",
                    "unlock_shared_and_lock() requires an active shared owner"
```

`<file>:<line>` na mensagem 1301 é o `std::source_location` do **chamador**, que todo `GetInst()` repassa.
Por exemplo, `IExecucaoVota::GetInst` passa `iexecucaovota.cpp:74`.

### 2.3 As interfaces registradas neste build (verificado em tempo de execução, §4)

| # | interface | implementação web | registrada por |
|---:|---|---|---|
| 0 | `api::ISystemDateTime` | `api::CSystemDateTime` (padrão, depois substituída) | primeiro `ISystemDateTime::GetInst()` (func 1155), antes de `main` registrar qualquer coisa |
| 1 | `comum::IInterfaceSavd` | `(anonymous)::CWasmSavd` | `main` → replace 10090 |
| 2 | `ecourna::api::security::IRng` | `ecourna::api::security::CPrng` | `main` → 9233 |
| 3 | `ecourna::api::security::ISymmetricCipherFactory` | `CSymmetricCipherFactory` | `main` → 9135 |
| 4 | `api::IFingerPrepare` | `simulador::CFingerPrepareSimulador` | `main` → 9039 |
| 5 | `api::IAjusteDataHora` | `api::CAjusteDataHora` | `main` → func 10876 |
| 6–9 | `IGenericFactory<IThreadImpl/ISyncCtl/IRWSyncCtl/ISemaphore>` | `CDefaultGenericFactory<…, CWasmThread / CPosixMutex / CPosixRWMutex / CPosixSemaphore>` | `main` → 8986/8911/8834/8758 |
| 10 | `vota::impl::IPoliticaExecucaoEleitor` | `(anonymous)::CPoliticaExecucaoEleitorWeb` | `main` → replace wasm_entry_f9507 (→ push 5407) |
| 11 | `vota::impl::ISincronismoVotoEleitor` | `(anonymous)::CSincronismoVotoEleitorWeb` | `main` → replace wasm_entry_f9437 (→ push 5398) |
| 12 | `vota::IExecucaoVota` | `vota::CExecucaoVotaCooperativa` | `main` → replace 8728 (→ push 5395) |
| 13 | `api::ITimerScheduler` | `simulador::CWasmTimerScheduler` | func 8302 |
| 14 | `comum::IInterfaceInit` | `simulador::CWasmInit` | 8302 |
| 15 | `api::IScreen` | `simulador::CWasmScreen` | 8302 |
| 16 | `api::IScreenMT` | `simulador::CWasmScreenMT` | 8302 |
| 17 | `api::IInputKbd` | `simulador::CWasmInputKbd` | 8302 |
| 18 | `api::IInputMT` | `simulador::CWasmInputMT` | 8302 |
| 19 | `api::IResource` | `simulador::CWasmResource` | 8302 |
| 20 | `api::IUrna` | `api::teste::CUrnaMock` (modelo 2020, serial 87654321) | 8302 |
| 21 | `api::IPower` | `api::teste::CPowerMock` (rede elétrica, bateria cheia) | 8302 |
| 22 | `api::IBeep` | `simulador::CWasmBeep` | 8302 |
| 23 | `api::ISound` | `simulador::CWasmNullSound`, **substituída** por `CWasmWebSound` | 8302, depois a lambda de `main` (replace libcxx_f10356 → push 5384) |
| 24 | `api::IPaperRelatorios` | `simulador::CWasmNullPaper` | 8302 |
| 25 | `api::IImpressoraRelatorios` | `simulador::CWasmNullPrinter` | 8302 |
| 26 | `api::ITextToSpeech` | `CWasmNullTextToSpeech`, **substituída** por `votaInit` (RHVoice ou Null) | 8302 (via 4162), `votaInit` (via 4162 → 7667) |
| – | `api::ISystemDateTime` | **substituída** por `simulador::CWasmSystemDateTime` | 8302 → 4890 |
| 27 | `api::CEscritorLog` | `simulador::CWasmLogd(<log dir>/logd.dat)` | 8302 |

Criadas sob demanda no primeiro uso (não vistas no trace da inicialização): `vota::IQRCodeBUDS` (1956),
`vota::impl::IInformacaoThreadOperador` (599), `comum::impl::IValidaMidia` (5575),
`comum::IControladorRegistraMesarios`, `comum::CCalculaCV`, `comum::CGeracaoVersoesContratos`,
`vota::IAjusteInicial`, `vota::testeteclado::impl::IGeradorTeclas`, `comum::IEventosLog` e as exclusivas da urna
`api::IKernelHSM`, `api::pkcs11::IPkcs11`, `api::IFingerScanner/Matcher/Detection`, `api::IPaper`,
`IGenericFactory<IHash/ITextEncoding>`. O último grupo não tem implementação no build web. As
funções `instance` existem, mas chamá-las lançaria 1301.

## 3. Fluxo de controle

### 3.1 `exists<T>(info)`

A função monta `std::string(typeid(T).name())`, pega um lock compartilhado, chama `find`, libera o lock e retorna
`found`. O wasm-opt mesclou as instanciações pelo **comprimento do nome mangled**, porque o nome é copiado
com loads fixos de 8 bytes: 6131 (12 caracteres), 6132 (16), 6057 (20), 2924 (23). 1166 é a versão genérica
out-of-line, e 3928 lê o nome através de `&typeid(T)`. `__wasm_call_ctors` faz 24 verificações de
existência (21 funções `exists<T>` distintas, 15 delas nesta unidade, mais 3 cópias inlinadas) e descarta
todos os resultados. Elas parecem inicializadores dinâmicos de dados estáticos que nunca são lidos.

### 3.2 `CPolySingletonList::instance<T>(info)` (cpolysingletonlist.h:99/105)

```
shared lock -> key = typeid(T).name() -> find
  not found            : log("instance", key, "find");   throw 6754 (line 99)
  entry.tipo != typeid : log("instance", key, "typeid"); throw 6755 (line 105)
return *static_pointer_cast<T>(entry.instancia)    // a shared_ptr copy is made and dropped; the raw T& escapes
```

### 3.3 `CPolySingleton<T>::instance(info, loc)` (cpolysingleton.h:78), as 30 funções "instance@N"

```
if (!exists<T>(GetPolySingletonsInfo()))   // note: re-reads the accessor, ignores `info`
    throw 1301 "PolySingleton - solicitada uma instancia nao criada " + typeid(T).name()
               + " [" + loc.file_name() + ":" + to_string(loc.line()) + "]"
return CPolySingletonList::instance<T>(info);
```

Por causa da verificação `exists`, os throws 6754/6755 internos são inalcançáveis em um programa single-threaded.

### 3.4 `push<T>(unique_ptr<T>&&, info)` (cpolysingletonlist.h:129) e o wrapper registrar-ou-**substituir**

O `push` em si apenas acrescenta. O passo "se existe, apague primeiro" pertence a um wrapper separado (seu nome
não está no binário; a reconstrução o chama de `replace`):

```
push(p, info):                                                  // srcloc :129
  exclusive lock (CUpgradeMutex::lock)
  if (find(name).found) { log("push", name, "find"); throw 6756 }   // unreachable single-threaded
  trace("push", name, format("sz[{}] ptr[{}]", lista.size(), (void*)&info))
  lista.emplace_back(name, &typeid(T), shared_ptr<T>(move(p)))

replace(p, info):                                               // name inferred
  if (exists<T>(info)) erase(typeid(T).name(), info);           // REPLACE semantics
  push(move(p), info)
```

Por que duas funções: as funções `GetInst()` 599, 3594, 5575, 5925 e 7181 chamam `exists` uma vez e depois executam
o corpo de `push` (inlinado, ou o out-of-line 5395/5407/5398) **sem** uma segunda chamada a `exists`. `exists` pega
o lock e passa por `invoke_*` do JS, então o compilador não consegue provar seu resultado nem eliminar uma segunda chamada.
Ele não a eliminou em 1956 (`GetQRDSInst`) e 10876 (`IAjusteDataHora::CreateInst`), que fazem o mesmo
teste `if (!exists)` e ainda chamam `exists` + `erase` de novo antes do corpo de `push`. Portanto, essas duas, o código de
inicialização (`main`, func 8302, `votaInit`) e `CreateInst<T>` chamam o wrapper. 8728, `wasm_entry_f9437/9507` e
`libcxx_f10356` são o wrapper com `push` deixado out-of-line (5395, 5398, 5407, 5384); 7667, 8758…10090 são o
wrapper com `push` inlinado.

O código de inicialização chega ao wrapper através de um helper por valor, `f(std::unique_ptr<T> p, info) { replace(move(p),
info); }` (func 4162 para `ITextToSpeech`, corpo mesclado 1562 para as chamadas de `main`). Com a ABI v2 da libc++,
`std::unique_ptr` é `[[clang::trivial_abi]]`: um `unique_ptr` passado por valor é passado como o ponteiro cru e a
função chamada o destrói. 4879 e 4890 são esse helper com `replace` e `push` inlinados, e é por isso que seu primeiro
parâmetro é o ponteiro cru do objeto.

`erase(nome, info)` (func 640) pega um lock compartilhado, encontra a entrada, faz o **upgrade** dele para exclusivo
(func 13621), guarda uma cópia do `shared_ptr` e apaga o elemento do vector. O objeto antigo é então
destruído enquanto o lock exclusivo ainda está tomado.

O padrão de `GetInst()` que aparece inlinado nesta unidade é:

```
[static mutex lock_guard]              // 599, 3594, 5575, 5925, 7181 (unlock residue only)
if (!CPolySingleton<I>::exists())  push<I>(make_unique<CDefault>())      // 599, 3594, 5575, 5925, 7181
                                   replace<I>(make_unique<...>())        // 1956, 10876
                                   CreateInst<T>() (throws "Tentativa    // 1155
                                   de recriar o singleton", then replace)
return CPolySingleton<I>::instance(info, source_location::current());   // not in 10876 (returns void)
```

Para `ISystemDateTime` (1155) e `ITimerScheduler` (1259/5444, fora da u19), `CreateInst<T>` lança
`CBaseError<api::EUeUtilError>` 7084/7085 "Tentativa de recriar o singleton" se a interface já existe;
ali, portanto, o re-registro é recusado, enquanto `replace` substitui e um `push` puro lançaria 6756.

### 3.5 O lock

`lock_shared` espera enquanto `writer || writersWaiting`. `lock` espera enquanto `writer || readers`. `unlock`
acorda primeiro um escritor que esteja esperando e, se não houver, todos os leitores. Neste build não há pthreads, então
`std::condition_variable::wait` (func 251) **retorna imediatamente**. Todo "espere até" é, portanto, um laço de espera ativa
que nunca termina se a condição for falsa (§11).

## 4. Inicialização, verificada em tempo de execução

Com uma cópia de `tools/run/headless.mjs` que define `ENV.DEBUG_UENUX = "1"` em `preRun`, as linhas de trace ficam
no buffer TTY do stdout, porque o formato não tem `\n`. Despejar esse buffer no fim de uma execução de municipal-t1
dá 31 linhas:

```
: CPolySingletonList::push(N3api15ISystemDateTimeE[api::ISystemDateTime])[sz[0] ptr[0x1bf600]]
: CPolySingletonList::push(N5comum14IInterfaceSavdE[comum::IInterfaceSavd])[sz[1] ptr[0x1bf600]]
...
bf60: CPolySingletonList::push(N3api15IAjusteDataHoraE[api::IAjusteDataHora])[sz[5] ptr[0x1bf600]]
...
À: CPolySingletonList::push(N3api13ITextToSpeechE[api::ITextToSpeech])[sz[26] ptr[0x1bf600]]
: CPolySingletonList::push(N3api15ISystemDateTimeE[api::ISystemDateTime])[sz[26] ptr[0x1bf600]]   <- replace (size unchanged)
: CPolySingletonList::push(N3api12CEscritorLogE[api::CEscritorLog])[sz[27] ptr[0x1bf600]]
: CPolySingletonList::push(N3api6ISoundE[api::ISound])[sz[27] ptr[0x1bf600]]                      <- main lambda: CWasmWebSound
: CPolySingletonList::push(N3api13ITextToSpeechE[api::ITextToSpeech])[sz[27] ptr[0x1bf600]]       <- votaInit
```

O trace confirma estes pontos:
* a ordem de registro da §2.3;
* a semântica de substituição (`sz` continua igual depois de um erase);
* `ptr[...]` é o endereço do registro (0x1BF600);
* o **prefixo de lixo** antes de `": "` (`bf60`, `À`, `ã`, `bf600§`): bytes não inicializados da pilha impressos por
  `printf("%s: ", buf)` (§11).

## 5. Diagnóstico: `log`, `trace` e o back end syslog/printf

* `log(func, type, detail)` (func 212) faz o demangle de `type` com `abi::__cxa_demangle` (`"..."` em caso de falha). Se
  `func` não começa com `"list"`, acrescenta um texto de pilha de chamadas. Esse texto é sempre vazio neste build.
  Em seguida chama a func 13156 com `"CPolySingletonList::%s(%s[%s])[%s] %s%s"`.
* Func 13156: `static int naUrna = access("/dev/urna", F_OK) == 0`. Numa urna real, a linha vai para o
  **syslog** (`LOG_INFO`). Em outros lugares, vai para o stdout, e só se `getenv("DEBUG_UENUX")`. O MEMFS não tem
  `/dev/urna` e a página não define nenhum ambiente, então nada é impresso no navegador.
* `trace()` (func 14476, e suas cópias inlinadas em todo `push`) chama `log` só se
  `GetPolySingletonsInfo().debug`: ela chama o acessor de novo e ignora o `info` passado ao `push`. O
  acessor define `debug = true` a cada chamada, então o trace está **sempre ligado**. Todo registro paga um
  `std::format` e um demangle, e as funcs 212 e 13156 foram executadas nas sessões gravadas.

## 6. As funções de outros arquivos

### 6.1 Inicialização do simulador: func 8302 (`simulador::CSimuladorWasm::Executa`, nomes inferidos)

`main` monta `{int 10, std::string "VOTA Web", short width, short height}` (ctor func 8311) e invoca
a func 8302 (através do seu slot 128 na tabela) com a lambda de `main` como `std::function`. A func 8302 faz o seguinte, nesta ordem:

1. Define as raízes de armazenamento de `comum::CPath`: raiz `"/"`, flash interna `/dsk/fi/`, flash externa `/dsk/fe/`
   (estáticos @1838600/@1838576/@1838588).
2. Chama `create_directories("/uenux")`, `setenv("PROJECT_DIR", ".")` e `setenv("BUILD_DIR", "/")`.
3. Executa `ITimerScheduler::CreateInst<CWasmTimerScheduler>()` (itimerscheduler.h:40). Depois registra, cada um
   através do wrapper registrar-ou-substituir da §3.4 (inlinado, ou 5384/4162/4890),
   `CWasmInit(10)`, `CWasmScreen(w, h)`, `CWasmScreenMT` (4 linhas de texto, cada uma `std::string(40, ' ')`), `CWasmInputKbd`,
   `CWasmInputMT`, `CWasmResource`, `CUrnaMock`, `CPowerMock`, `CWasmBeep`, `CWasmNullSound(9)`,
   `CWasmNullPaper`, `CWasmNullPrinter`, `CWasmNullTextToSpeech`, `CWasmSystemDateTime` e
   `CWasmLogd(logdir/"logd.dat")` como `CEscritorLog`.
   * `CWasmScreen(w, h)` escala por `sx = w/640`, `sy = h/480`, chama `js_init(w, h)` e `Clear(1)`, e grava
     `"CWasmScreen this=<address>"` no log através de `js_log`.
   * O construtor de `CWasmBeep` chama `js_wasm_beep_init()` (o objeto é alocado e seu vptr armazenado antes).
4. Chama a lambda. Se a lambda estiver vazia, lança `bad_function_call`.

Os valores dos mocks que ela instala:
* `CUrnaMock`: +4 = **2020** (slot 0 de IUrna, o ano do modelo: `CTesteTeclado`/`CPedeIdentidade` testam
  `<= 2019` / `>= 2020`), +8 = 87654321, +12 = 255.
* `CPowerMock`: status `{0x840, 2, 0, 100}`. Isso decodifica como alimentação pela rede elétrica com a bateria interna cheia,
  então a chave do ícone de bateria é 0.

### 6.2 Funções `GetInst()` com padrão criado sob demanda

| func | função | implementação padrão | srcloc |
|---:|---|---|---|
| 599 | `vota::impl::IInformacaoThreadOperador::GetInst` | `CInformacaoThreadOperador` (96 B) | cinformacaothreadoperador.cpp:554 |
| 1155 | `api::ISystemDateTime::GetInst` | `CreateInst<CSystemDateTime>` (lança 7084 se recriado) | isystemdatetime.cpp:23, .h:46 |
| 1956 | `vota::(anon)::GetQRDSInst` | `IQRCodeBUDS(GeraPartesQRCodeBU())`, veja a §7 | cmostraqrcodebu.cpp:31/38 |
| 3594 | `vota::IExecucaoVota::GetInst` | `CExecucaoVota` (threads da urna) | iexecucaovota.cpp:74 |
| 3876 | `api::CLp::GetInst` (`unique_ptr` estático, não é um poly-singleton) | `CLp()` busca `IImpressoraRelatorios` | cwasmclp.cpp:10 |
| 5575 | `comum::impl::IValidaMidia::GetInst` | `CValidaMidia` | cvalidamidia.cpp:182 |
| 10876 | `api::IAjusteDataHora::CreateInst` (nome inferido) | `CAjusteDataHora` (sem verificação de recriação) | – |
| 5925 | `IPoliticaExecucaoEleitor::GetInst` inlinado | `CPoliticaExecucaoEleitor` | cpoliticaexecucaoeleitor.cpp:45 |
| 7181 | `ISincronismoVotoEleitor::GetInst` inlinado | `CSincronismoVotoEleitor` | csincronismovotoeleitor.cpp:67 |

No build web, `main` registra primeiro as versões web de IExecucaoVota, IPoliticaExecucaoEleitor e
ISincronismoVotoEleitor, então os padrões da urna delas nunca são criados.

### 6.3 Lado do eleitor

* **5925**, slot 17 da vtable de `CConfirmaVotoNominal` e `CMajoritarioValido` (identical code folding). A
  base `CConfirmaVotoEmCargo` tem um no-op ali. `EmiteEcoCorrigeConfirma` a chama com a tecla que acabou de
  tratar. Em CORRIGE (5), grava no log "Eleitor corrigiu na tela de confirmação de candidato" e chama
  `IPoliticaExecucaoEleitor::LimpaBufferInput()`. Isso esvazia o teclado, de modo que teclas digitadas na tela de
  confirmação não são tomadas como o próximo voto. A política web esvazia uma vez, e a da urna esvazia de 3 a 6 vezes com
  pausas aleatórias (u06).
* **7181** `CSincronismoEleitor::ProcessMessage`: na mensagem 5, chama
  `ISincronismoVotoEleitor::GetInst().SincronizaVoto()`, e se isso der certo vai para `CFimVotoEleitor` (a
  tela "FIM"). Ela foi executada nos votos gravados.

### 6.4 Lado do operador (MT): nada disso roda no navegador

Estes são corpos de `ProcessInput` (slot 7) construídos sobre `CInteractiveForm<IScreenMT, IInputMT>::Read()`
(cinteractiveform.h:57). Os códigos de tecla são 5 = CORRIGE e 9 = CONFIRMA.

| func | estado | comportamento |
|---:|---|---|
| 10493, 10641 (→ 6015) | `CInformaAnoNascimentoErrado`, `CEleitorJaVotou` | CONFIRMA → `CCancelaHabilitacaoEleitor` (1536) |
| 10593 | `CJustificativaEfetuada` | CONFIRMA → `CCancelaHabilitacaoEleitor` |
| 10727 | `CTituloEncerramentoInvalido` | CORRIGE → estado vindo de vota_f3631 |
| 10477 | `CDigitalNaoReconhecidaDecBiometria` | CORRIGE → log + cancela a habilitação; CONFIRMA → `CVerificaDadoEleitor` |
| 10523 | `CHabilitaAudioEleitor` | CONFIRMA: limpa o áudio manual, posta a msg 8 (eleitor precisa de áudio) ou 9, depois a msg 1 ("habilitado") na fila do eleitor, vai para `CMostraEleitorVotando` e, só se o eleitor precisa de áudio, grava no log "Áudio ativado conforme cadastro". "Precisa de áudio" (slot 5) é lido antes de a tecla ser testada |
| 10420 | `CPerguntaCodigoSuspensao` | Suspende a sessão do eleitor. O mesário digita um título. Ele deve ser um título válido e diferente da identidade digitada do eleitor. Válido: grava no log "Título {} é válido para suspender a votação" e posta a msg 2. Inválido: grava no log "… inválido …", mostra a tela de erro, **`emscripten_sleep(3000)`**, redesenha. CORRIGE: log, posta a msg 4, volta para `CMostraEleitorVotando` |
| 6011 | `comum::CMesarioRegistrado` / `CRegistrarMesarios` | comparecimento de mesários: CORRIGE → slot 25 do controlador + "Finalizar registro de mesários?"; CONFIRMA → slot 24 + próximo estado |
| 3623, 5415, 10584, 10670 | `IInformacaoThreadOperador::GetInst().slotN()` outlined | GetIdentidadeDigitada, SetAudioHabilitadoManualmente, fonte de dados GetTextoAudio |

### 6.5 Outras

* **Ícone de bateria** (2773, 2774, 10981, 10987, 10991, 2238). A chave do ícone 0–9 corresponde a um par de imagens
  (horizontal para a tela, vertical para o LCD do MT). A chave 10 significa sem ícone:

  | chave | estado | imagem |
  |---:|---|---|
  | 0 / 1 / 2 | na rede elétrica, bateria cheia / parcial / crítica | `img-ac-bateria-full`, `…-parcial`, `…-critical` (hor: `-critical-h.jpg`) |
  | 3 | sem bateria | `img-ac-sem-bateria` |
  | 4 / 5 / 6 | na bateria: cheia / parcial / crítica | `img-bateria-full`, `…-parcial`, `…-critical` |
  | 7 / 8 / 9 | bateria externa: cheia / parcial / crítica | `img-bateria-ext-full`, `…-ext-parc(ial)`, `…-ext-crit(ical)` |

  `GetEstadoIcone` relê o status (slot 15 de `IPower`) antes de cada teste de bit. Lê a fonte nos bits
  1–2, a bateria interna nos bits 3–4 e a bateria externa nos bits 5–6. Uma lambda de timer de 1 segundo
  compara a chave com a anterior e notifica os observadores.
* **11530** slot 0 de `ITextToSpeech`: o front end de síntese com o cache LRU (veja u02 e rhvoice.md).
* **10224**: o corpo de `std::async` em `CThreadMonitor::SaiPorVotacaoSuspensa`, que emite um bipe (slot 4 de `IBeep`).
* **3876/5978/12016** `api::CLp`: o front end da impressora de relatórios do mock (`cwasmclp.cpp`).

## 7. BU: a fonte de dados do "BU digital" na tela (func 1956, `GetQRDSInst`)

No fim do dia, `vota::CMostraQRCodeBU` (u09) mostra o BU como QR codes na tela da urna, uma parte de cada
vez (teclas 3/9). Seus dados vêm do poly-singleton `vota::IQRCodeBUDS`. Esse objeto é construído **uma vez**,
na primeira vez em que `GetQRDSInst()` é executada:

1. `CPolySingleton<IQRCodeBUDS>::exists()` (func 2886). Se for true, pula para o passo 9.
2. `eg = GetEstado<CEstadoGeral>()` (func 291, `eg.bin`).
3. `gap = GetEstado<CEstadoGeralGap>()` (inlinada). Se esse estado não foi carregado, lança
   `CUeComumAppInfoError 7600 "O estado não foi carregado: GetGap"` (cappinfo.cpp:42).
4. `comparecimento = CRdvVota::GetInst()` → máximo, entre as eleições, de `CVotosEleicoesVota::Comparecimento`
   (shared_f1269).
5. `vota = GetEstado<CEstadoGeralVota>()` (func 261, `vota.bin`). `dhEmissao = vota.GetDtHrEmissaoBU()`
   (optional em +80). Se estiver vazio, lança `CUeComumDadosError 8093 "A data/hora da emissão do BU não foi
   registrada"` (cestadogeralvota.h:159). A data é registrada quando o BU impresso é gerado, então o
   BU de tela só pode existir depois dele.
6. Cabeçalho `CCabecalhoQRCode` (func 5624, 33 strings `TAG:value ` vazias):
   * `ZONA` = eg+24 (func 5618)
   * `SECA` = eg+26 (5620)
   * `IDUE` = carga `numeroInternoUrna` eg+60 (5621)
   * `IDCA` = carga `codigoCarga` eg+88 (5623)
   * `HIQT`/`HICA` = o `codigoCarga` de cada entrada de `gap.correspondencias` (entradas de 96 bytes, string em +28)
     (5622)
7. `CGeradorBUQRCodeVota(cabecalho, comparecimento, dhEmissao)` (func 5603). Depois,
   **`GeraQRCodes(2500)`** (func 5604). O BU impresso (`CGeraBU`, func 12110) usa exatamente o mesmo cabeçalho
   e gerador com **1100**. Os QR codes de tela, portanto, contêm no máximo 2500 caracteres cada (fatias de
   2500 − 277 = 2223 caracteres de payload, se a reserva de 277 caracteres descrita em `docs/bu/qrcode.md` §4
   se aplicar), então um BU precisa de menos códigos, mais densos, na tela do que no papel. O resultado é
   `{vector<string> conteudos, string assinatura}`. A string de assinatura é descartada, porque o campo `ASSI:`
   já está dentro do último payload.
8. `IQRCodeBUDS(partes)` (vtable @1542092): `{vector<string> m_qrcodes (+4), size_t m_indice = 0 (+16)}`.
   Lança `CUeVotaError 9372 "Vetor de partes vazio"` (cmostraqrcodebu.cpp:38) se o vector estiver vazio. Depois
   `replace<IQRCodeBUDS>` (§3.4, inlinado: um segundo `exists` + `erase`, depois o corpo de `push`). As partes são
   geradas antes de o acessor do registro ser chamado.
9. `return CPolySingleton<IQRCodeBUDS>::instance(info, loc = cmostraqrcodebu.cpp:31)`.

A tela liga duas funções de fonte de dados desta unidade a campos `std::function<std::string()>`:
* `QRCodeAtual()` (func 12053, slot 1506 da tabela) retorna `m_qrcodes.at(m_indice)`. Essa string é o
  payload de QR mostrado por `CQRCodeImage`.
* `TextoInstrucaoQRCode()` (func 12054, slot 1505) retorna "O QR code ao lado contém o resultado da votação
  para esta urna." quando há 1 parte. Com várias partes, acrescenta " Use as teclas 3 e 9 para navegar pelas
  partes do BU.".

O formato do payload, a cadeia de hashes e a assinatura `ASSI` estão em `docs/bu/qrcode.md`. Nada aqui foi executado no
simulador, porque o build web nunca chega ao encerramento (`docs/bu/codepath.md` §1.3). Os payloads de
tela são gerados independentemente dos impressos. São assinados pelo mesmo gerador com o mesmo cabeçalho,
mas as fatias, e portanto a cadeia de hashes e a assinatura, diferem dos QR codes em papel do mesmo BU.

## 8. Particularidades do build web

* Os serviços da plataforma são todos mocks registrados pela func 8302 e por `main`, e alguns padrões da urna são
  substituídos (`ISystemDateTime`, `ISound`, `ITextToSpeech`) (§2.3).
* `CUrnaMock` informa o ano de modelo 2020 e o serial fixo 87654321. `CPowerMock` informa rede elétrica com bateria
  cheia.
* O trace está compilado e sempre "habilitado", mas invisível: não há `/dev/urna` nem
  `DEBUG_UENUX`.
* A thread do operador nunca roda, então a §6.4 e a tela do BU (§7) são código morto no navegador.

## 9. Observações sobre wasm / Emscripten

* **Armadilha de nomes.** 30 funções levam o nome `CPolySingletonList::instance@N`. Na verdade são
  `CPolySingleton<T>::instance` (assinatura `(info, loc)`) ou um `GetInst()` inteiro (assinatura `()`). A func
  8302 aparecia como "push", mas é a inicialização do simulador (§6.1); as ferramentas agora mostram o nome inferido
  `simulador::CSimuladorWasm::Executa`.
* O **merge-similar-functions** mesclou corpos pelo comprimento em bytes do nome de tipo inlinado (exists: 6131, 6132,
  6057, 2924; instance: 6013), por vtables constantes (`shared_ptr` a partir de `unique_ptr`: 1010) e por slots de
  tabela constantes (`emplace_back`: 1009; `find`: 6161; lock guards: 6147; exception guards: 3895).
* **Duas funções de registro, não uma dividida.** As "cabeças" 8728, wasm_entry_f9437, wasm_entry_f9507 e
  libcxx_f10356 são o wrapper registrar-ou-substituir (`if (exists) erase`), e as "caudas" 5395/5398/5407 (→ corpo
  mesclado 3882) e 5384 são o próprio `push`, deixado out-of-line. As funções `GetInst()` chamam `push` diretamente, e é
  por isso que não têm um segundo `exists` (§3.4). O compilador não removeu nada: `exists` pega lock e chama o JS,
  e 1956/10876 mantêm a segunda chamada a `exists` depois do mesmo teste `if (!exists)`.
* **Dois modelos de exceção.** Só 26 das 184 funções usam `invoke_*` (por exemplo, o `instance` de
  IInputKbd/ISound, os corpos de `push`/`replace` chamados de `main`/`votaInit`, `erase`, as primitivas de lock,
  `ITextToSpeech::GetAudio`). As cópias inlinadas na maior parte do código de aplicação do uenux2 **não têm landing pads**.
  Se lançarem exceção enquanto seguram o lock do registro, nunca o liberam. Isso bate com a constatação da u11/rhvoice
  de que a maioria das unidades de tradução do uenux2 é compilada sem captura de exceções. Não vale para todas:
  as cópias out-of-line de `instance` 455/1091, que estados de vota:: chamam, e `GetAudio` (11530) têm landing
  pads.
* **O acessor é um ponteiro de função** (@1526320), não uma chamada direta. Isso custa um `call_indirect`
  por busca, e todo `instance` o chama duas vezes.
* **Inicializadores estáticos com stores mortos.** 24 verificações de existência em `__wasm_call_ctors` (21 funções `exists<T>`
  distintas) têm os resultados descartados.
* O destrutor do registro estático (10620) é o slot 2 da tabela. As ferramentas o rotularam erroneamente como o vf0 de
  `IObservableProgressWithDescription`. Ele nunca é executado, por causa de `noExitRuntime`.

## 10. Tabela de mapeamento completa (184 funções)

"executou" = observada em execução em `analysis/runtime/*.functions.tsv`. "Reconstruído em" é relativo à
raiz do projeto. Os helpers de biblioteca/inlinados são listados com o motivo.

| func | tamanho | executou | nome nas ferramentas | símbolo reconstruído | reconstruído em | arquivo original | conf. |
|---:|---:|:-:|---|---|---|---|---|
| 212 | 725 | ✓ | `api_f212` | `api::CPolySingletonList::log(funcao, tipo, detalhe)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 235 | 21 |  | `api_f235` | `api::CUePatternError::CUePatternError(code, msg, loc) (thunk)` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | alta |
| 305 | 14 | ✓ | `api_f305` | `api::CPolySingletonList::find (thunk, lambda slot 156)` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 321 | 32 |  | `api_f321` | `std::__format::__allocating_buffer<char>::~__allocating_buffer` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: destrutor do buffer de std::format da libc++ | média |
| 327 | 53 | ✓ | `api_f327` | `std::to_string(unsigned)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: to_string da libc++ para loc.line() | alta |
| 356 | 1808 |  | `api::CPolySingletonList::instance@356` | `api::CPolySingleton<comum::IControladorRegistraMesarios>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | alta |
| 383 | 1772 |  | `api::CPolySingletonList::instance@383` | `api::CPolySingleton<api::IInputMT>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | alta |
| 388 | 55 | ✓ | `api_f388` | `std::shared_ptr<T>::~shared_ptr()` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: destrutor de shared_ptr da libc++, compartilhado por muitos tipos (ICF) | alta |
| 429 | 135 |  | `api_f429` | `std::make_format_args("instance", const std::string&, const std::string&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: empacotamento de argumentos de std::format da libc++ | média |
| 430 | 93 |  | `api_f430` | `std::make_format_args("instance", const std::string&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: empacotamento de argumentos de std::format da libc++ | média |
| 455 | 2898 | ✓ | `api::CPolySingletonList::instance@455` | `api::CPolySingleton<api::IInputKbd>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | alta |
| 512 | 1772 | ✓ | `api::CPolySingletonList::instance@512` | `api::CPolySingleton<api::IScreen>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | alta |
| 599 | 4114 |  | `api::CPolySingletonList::instance@599` | `vota::impl::IInformacaoThreadOperador::GetInst()` | src/uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.u19.cpp | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | alta |
| 608 | 14 | ✓ | `api_f608` | `api::CPolySingletonList::find (thunk, lambda slot 162)` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 611 | 1846 | ✓ | `api::CPolySingletonList::instance@611` | `api::CPolySingleton<comum::IInterfaceInit>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | alta |
| 631 | 536 | ✓ | `api_f631` | `std::vector<api::SPolySingleton>::__emplace_back_slow_path(const char*&, const type_info*, const shared_ptr&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: crescimento de vector da libc++ para o registro | média |
| 640 | 327 | ✓ | `api_f640` | `api::CPolySingletonList::erase(const std::string&, TPolySingletonsInfo&)` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 642 | 10 |  | `api_f642` | `std::unique_lock<api::CUpgradeMutex>::~unique_lock()` | src/uenux2/src/api/pattern/cpolysingletonlist.h | biblioteca/helper inlinado: destrutor de lock guard da libc++ para o lock do registro | média |
| 816 | 36 |  | `api::CPolySingletonList::instance@816` | `api::CPolySingleton<api::IFingerScanner>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | alta |
| 837 | 1784 | ✓ | `api::CPolySingletonList::instance@837` | `api::CPolySingleton<comum::IEventosLog>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | alta |
| 862 | 1772 | ✓ | `api::CPolySingletonList::instance@862` | `api::CPolySingleton<api::IPower>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | alta |
| 905 | 1784 |  | `api::CPolySingletonList::instance@905` | `api::CPolySingleton<api::IPaperRelatorios>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | alta |
| 923 | 1772 | ✓ | `api::CPolySingletonList::instance@923` | `api::CPolySingleton<api::IUrna>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | alta |
| 925 | 1772 | ✓ | `api::CPolySingletonList::instance@925` | `api::CPolySingleton<api::IBeep>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | alta |
| 1010 | 63 |  | `api_f1010` | `std::shared_ptr<T>::shared_ptr(std::unique_ptr<T>&&) (merged body)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: shared_ptr a partir de unique_ptr da libc++ (corpo mesclado) | média |
| 1091 | 2898 | ✓ | `api::CPolySingletonList::instance@1091` | `api::CPolySingleton<api::ISound>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | alta |
| 1113 | 227 |  | `api_f1113` | `std::make_format_args(size_t, const void*)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: empacotamento de argumentos de std::format da libc++ | média |
| 1119 | 395 |  | `api_f1119` | `std::make_format_args("push", const char*)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: empacotamento de argumentos de std::format da libc++ | média |
| 1126 | 141 | ✓ | `api_f1126` | `api::CUpgradeMutex::lock()` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 1155 | 1944 | ✓ | `api::CPolySingletonList::instance@1155` | `api::ISystemDateTime::GetInst()` | src/uenux2/src/api/util/isystemdatetime.u19.cpp | uenux2/src/api/util/isystemdatetime.cpp | alta |
| 1166 | 198 | ✓ | `api_f1166` | `api::CPolySingletonList::exists<T> generic body (string key, out-of-line lock_shared/find)` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 1282 | 1784 |  | `api::CPolySingletonList::instance@1282` | `api::CPolySingleton<comum::CCalculaCV>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | alta |
| 1322 | 10 | ✓ | `api_f1322` | `std::shared_lock<api::CUpgradeMutex>::~shared_lock()` | src/uenux2/src/api/pattern/cpolysingletonlist.h | biblioteca/helper inlinado: destrutor de lock guard da libc++ para o lock do registro | média |
| 1536 | 22 |  | `api_f1536` | `vota::CCancelaHabilitacaoEleitor::GetInst()` | src/uenux2/src/app/vota/operador/u19-foreign-fragments.cpp | uenux2/src/app/vota/operador/comum/ccancelahabilitacaoeleitor.cpp (caminho inferido) | média |
| 1549 | 200 |  | `api_f1549` | `std::__format::__create_packed_storage lambda for const std::string&` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: empacotamento de argumentos de std::format da libc++ (compartilhado com outros chamadores) | média |
| 1654 | 19 | ✓ | `api_f1654` | `api::CPolySingletonList::exists<api::ITimerScheduler>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 1686 | 36 |  | `api::CPolySingletonList::instance@1686` | `api::CPolySingleton<api::IFingerMatcher>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | alta |
| 1714 | 1772 |  | `api::CPolySingletonList::instance@1714` | `api::CPolySingleton<api::IScreenMT>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | alta |
| 1822 | 1784 | ✓ | `api::CPolySingletonList::instance@1822` | `api::CPolySingleton<comum::IInterfaceSavd>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | alta |
| 1956 | 5663 |  | `api::CPolySingletonList::instance@1956` | `vota::(anonymous namespace)::GetQRDSInst()` | src/uenux2/src/app/vota/eleitor/fimvotacao/cmostraqrcodebu.u19.cpp | uenux2/src/app/vota/eleitor/fimvotacao/cmostraqrcodebu.cpp | alta |
| 2030 | 1796 |  | `api::CPolySingletonList::instance@2030` | `api::CPolySingleton<ecourna::api::security::IRng>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | alta |
| 2079 | 1784 |  | `api::CPolySingletonList::instance@2079` | `api::CPolySingleton<api::ITextToSpeech>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | alta |
| 2160 | 19 | ✓ | `api_f2160` | `api::CPolySingletonList::exists<api::ISystemDateTime>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 2238 | 152 |  | `api_f2238` | `std::__tree<pair<int, vector<shared_ptr<api::IImage>>>>::destroy(node)` | src/uenux2/src/api/gui/cpowerinformation.u19.cpp | biblioteca/helper inlinado (para CPowerInformation::ms_icones) | média |
| 2279 | 2126 |  | `api::CPolySingletonList::instance@2279` | `api::CPolySingleton<api::IKernelHSM>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | alta |
| 2446 | 381 |  | `api_f2446` | `api::CPolySingletonList::exists<comum::IControladorRegistraMesarios>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 2469 | 19 | ✓ | `api_f2469` | `api::CPolySingletonList::exists<api::IAjusteDataHora>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 2509 | 357 | ✓ | `api_f2509` | `api::CPolySingletonList::exists<comum::IEventosLog>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 2584 | 16 |  | `ecourna_f2584` | `api::SPolySingleton::~SPolySingleton()` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 2736 | 12 | ✓ | `api_f2736` | `api::CPolySingletonList::exists<vota::IExecucaoVota>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 2773 | 5642 | ✓ | `api_f2773` | `api::CPowerInformation::CarregaIcones()` | src/uenux2/src/api/gui/cpowerinformation.u19.cpp | uenux2/src/api/gui/cpowerinformation.cpp | média |
| 2774 | 302 | ✓ | `api_f2774` | `api::CPowerInformation::GetEstadoIcone(api::IPower&)` | src/uenux2/src/api/gui/cpowerinformation.u19.cpp | uenux2/src/api/gui/cpowerinformation.cpp | média |
| 2886 | 19 | ✓ | `api_f2886` | `api::CPolySingletonList::exists<vota::IQRCodeBUDS>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 2924 | 351 | ✓ | `api_f2924` | `api::CPolySingletonList::exists<T> merged body, 23-char type names` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 3075 | 1772 | ✓ | `api::CPolySingletonList::instance@3075` | `api::CPolySingleton<api::IResource>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | alta |
| 3095 | 119 | ✓ | `api_f3095` | `api::CUpgradeMutex::lock_shared()` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 3164 | 1808 | ✓ | `api::CPolySingletonList::instance@3164` | `api::CPolySingleton<api::IGenericFactory<api::ISyncCtl>>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | alta |
| 3204 | 1784 |  | `api::CPolySingletonList::instance@3204` | `api::CPolySingleton<api::IAjusteDataHora>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | alta |
| 3281 | 11 | ✓ | `api_f3281` | `api::CPolySingletonList::exists<api::ITextToSpeech>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 3343 | 11 | ✓ | `api_f3343` | `api::CPolySingletonList::exists<api::ISound>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 3386 | 15 |  | `api_f3386` | `api::CPolySingletonList::exists<api::IBeep>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 3387 | 345 | ✓ | `api_f3387` | `api::CPolySingletonList::exists<api::IScreen>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 3391 | 12 |  | `api_f3391` | `api::CPolySingletonList::exists<api::CEscritorLog>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 3392 | 12 |  | `api_f3392` | `api::CPolySingletonList::exists<api::IImpressoraRelatorios>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 3394 | 12 | ✓ | `api_f3394` | `api::CPolySingletonList::exists<comum::IInterfaceInit>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 3594 | 1923 | ✓ | `api::CPolySingletonList::instance@3594` | `vota::IExecucaoVota::GetInst()` | src/uenux2/src/app/vota/iexecucaovota.u19.cpp | uenux2/src/app/vota/iexecucaovota.cpp | alta |
| 3615 | 2126 |  | `api::CPolySingletonList::instance@3615` | `api::CPolySingleton<api::IFingerDetection>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | alta |
| 3618 | 11 | ✓ | `api_f3618` | `api::CPolySingletonList::exists<api::IGenericFactory<api::ISemaphore>>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 3622 | 381 |  | `api_f3622` | `api::CPolySingletonList::exists<vota::impl::IInformacaoThreadOperador>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 3623 | 20 |  | `api_f3623` | `IInformacaoThreadOperador::GetInst().GetIdentidadeDigitada() (outlined)` | src/uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.u19.cpp | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.h (chamada inline, outlined) | média |
| 3627 | 11 | ✓ | `api_f3627` | `api::CPolySingletonList::exists<api::IGenericFactory<api::ISyncCtl>>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 3668 | 803 |  | `api_f3668` | `std::vector<std::shared_ptr<T>>::__assign_with_size(first, last, n)` | src/uenux2/src/api/gui/cpowerinformation.u19.cpp | biblioteca/helper inlinado (ICF com rhvoice_f1245 do RHVoice) | média |
| 3686 | 369 |  | `api_f3686` | `api::CPolySingletonList::exists<comum::impl::IValidaMidia>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 3704 | 2126 |  | `api::CPolySingletonList::instance@3704` | `api::CPolySingleton<api::pkcs11::IPkcs11>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | alta |
| 3876 | 2002 |  | `api::CPolySingletonList::instance@3876` | `api::CLp::GetInst()` | src/uenux2/mock/app/simulador/wasm/cwasmclp.u19.cpp | uenux2/mock/app/simulador/wasm/cwasmclp.cpp | média |
| 3882 | 2072 | ✓ | `api_f3882` | `api::CPolySingletonList::push<T>` corpo mesclado (IExecucaoVota/ISincronismoVotoEleitor/IPoliticaExecucaoEleitor) | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | alta |
| 3895 | 62 |  | `api_f3895` | `std::__exception_guard_exceptions<Rollback>::~__exception_guard_exceptions (merged body)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: exception guard da libc++ (corpo mesclado) | média |
| 3928 | 413 |  | `api_f3928` | `api::CPolySingletonList::exists<T> body taking &typeid(T).__type_name` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 4162 | 143 | ✓ | `api_f4162` | helper por valor `f(std::unique_ptr<api::ITextToSpeech>, info)` → `replace<api::ITextToSpeech>` 7667 | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 4180 | 84 |  | `api_f4180` | `std::shared_ptr<T>::operator=(std::shared_ptr<T>&&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: atribuição por movimento de shared_ptr da libc++ (ICF) | média |
| 4356 | 1820 |  | `api::CPolySingletonList::instance@4356` | `api::CPolySingleton<api::IGenericFactory<api::ISemaphore>>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | alta |
| 4852 | 42 |  | `ecourna_f4852` | `std::vector<api::SPolySingleton>::__base_destruct_at_end` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: destroy-at-end de vector da libc++ | média |
| 4853 | 357 |  | `api_f4853` | `api::CPolySingletonList::exists<api::IPaperRelatorios>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 4867 | 345 | ✓ | `api_f4867` | `api::CPolySingletonList::exists<api::IPower>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 4871 | 15 | ✓ | `api_f4871` | `api::CPolySingletonList::exists<api::IUrna>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 4872 | 15 | ✓ | `api_f4872` | `api::CPolySingletonList::exists<api::IResource>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 4873 | 345 |  | `api_f4873` | `api::CPolySingletonList::exists<api::IInputMT>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 4877 | 15 |  | `api_f4877` | `api::CPolySingletonList::exists<api::IScreenMT>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 4879 | 2097 | ✓ | `api::CPolySingletonList::push@4879` | helper por valor → `replace<api::ITimerScheduler>` → `push` (tudo inlinado) | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 4890 | 2097 | ✓ | `api::CPolySingletonList::push@4890` | helper por valor → `replace<api::ISystemDateTime>` → `push` (tudo inlinado) | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 5310 | 10 |  | `api_f5310` | `std::__exception_guard_exceptions<...>::~ (rollback slot 473 = comum_f9873)` | - | biblioteca/helper inlinado (vector não relacionado de comum; chamador unknown_f9896) | baixa |
| 5361 | 2248 |  | `api::CPolySingletonList::instance@5361` | `api::CPolySingleton<api::IGenericFactory<ecourna::api::security::ITextEncoding>>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | alta |
| 5362 | 2222 |  | `api::CPolySingletonList::instance@5362` | `api::CPolySingleton<api::IGenericFactory<ecourna::api::security::IHash>>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | alta |
| 5384 | 2079 | ✓ | `api::CPolySingletonList::push@5384` | `api::CPolySingletonList::push<api::ISound>` (sem prefixo exists/erase) | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | alta |
| 5395 | 30 | ✓ | `api::CPolySingletonList::push@5395` | `api::CPolySingletonList::push<vota::IExecucaoVota>` (thunk de 3882) | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | alta |
| 5398 | 30 | ✓ | `api::CPolySingletonList::push@5398` | `api::CPolySingletonList::push<vota::impl::ISincronismoVotoEleitor>` (thunk de 3882) | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | alta |
| 5407 | 30 | ✓ | `api::CPolySingletonList::push@5407` | `api::CPolySingletonList::push<vota::impl::IPoliticaExecucaoEleitor>` (thunk de 3882) | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | alta |
| 5415 | 22 |  | `api_f5415` | `IInformacaoThreadOperador::GetInst().SetAudioHabilitadoManualmente(bool) (outlined)` | src/uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.u19.cpp | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.h (chamada inline, outlined) | média |
| 5417 | 11 |  | `api_f5417` | `api::CPolySingletonList::exists<api::IGenericFactory<api::IRWSyncCtl>>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 5423 | 11 |  | `api_f5423` | `api::CPolySingletonList::exists<api::IGenericFactory<api::IThreadImpl>>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 5425 | 11 | ✓ | `api_f5425` | `api::CPolySingletonList::exists<api::IFingerPrepare>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 5427 | 11 | ✓ | `api_f5427` | `api::CPolySingletonList::exists<ecourna::api::security::ISymmetricCipherFactory>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 5431 | 11 |  | `api_f5431` | `api::CPolySingletonList::exists<ecourna::api::security::IRng>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 5434 | 11 | ✓ | `api_f5434` | `api::CPolySingletonList::exists<comum::IInterfaceSavd>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 5436 | 11 | ✓ | `api_f5436` | `api::CPolySingletonList::exists<api::IInputKbd>` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 5443 | 1784 | ✓ | `api::CPolySingletonList::instance@5443` | `api::CPolySingleton<api::ITimerScheduler>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | alta |
| 5562 | 10 |  | `api_f5562` | `std::__exception_guard_exceptions<...>::~ (rollback slot 235 = libcxx_f11146)` | - | biblioteca/helper inlinado (vector de strings não relacionado; chamador unknown_f11158) | baixa |
| 5575 | 3863 |  | `api::CPolySingletonList::instance@5575` | `comum::impl::IValidaMidia::GetInst()` | src/uenux2/src/app/comum/validamidia/cvalidamidia.u19.cpp | uenux2/src/app/comum/validamidia/cvalidamidia.cpp | alta |
| 5646 | 178 |  | `api_f5646` | `std::__format::__create_packed_storage lambda for const char(&)[9]` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: empacotamento de argumentos de std::format da libc++ | média |
| 5650 | 168 |  | `api_f5650` | `std::make_format_args("instance", const std::string&, const std::string&) (non-inlined packing)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: empacotamento de argumentos de std::format da libc++ | média |
| 5656 | 132 |  | `api_f5656` | `std::make_format_args("instance", const std::string&) (non-inlined packing)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: empacotamento de argumentos de std::format da libc++ | média |
| 5774 | 10 |  | `api_f5774` | `std::__exception_guard_exceptions<_AllocatorDestroyRangeReverse<...SPolySingleton>>::~__exception_guard_exceptions` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: exception guard da libc++ | média |
| 5779 | 68 |  | `api_f5779` | `std::__split_buffer<api::SPolySingleton>::~__split_buffer` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: destrutor de split_buffer da libc++ | média |
| 5786 | 382 | ✓ | `api_f5786` | `std::vector<api::SPolySingleton>::__swap_out_circular_buffer` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: realocação de vector da libc++ | média |
| 5925 | 2141 |  | `vota::CConfirmaVotoNominal::vf17` | `vota::CConfirmaVotoNominal::PosTecla(int) [= CMajoritarioValido slot 17]` | src/uenux2/src/app/vota/eleitor/u19-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/votaproporcional/cconfirmavotonominal.cpp (caminho inferido) | média |
| 5978 | 97 |  | `api_f5978` | `api::CLp::~CLp()` | src/uenux2/mock/app/simulador/wasm/cwasmclp.u19.cpp | uenux2/mock/app/simulador/wasm/cwasmclp.cpp | baixa |
| 6011 | 227 |  | `comum_f6011` | `ProcessInput merged body (comum::CMesarioRegistrado / comum::CRegistrarMesarios)` | src/uenux2/src/app/comum/comparecimentomesario/u19-foreign-fragments.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cmesarioregistrado.cpp + cregistrarmesarios.cpp | média |
| 6013 | 2101 |  | `api_f6013` | `api::CPolySingleton<T>::instance (merged body for IFingerScanner/IFingerMatcher)` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | alta |
| 6015 | 127 |  | `api_f6015` | `ProcessInput merged body (CInformaAnoNascimentoErrado / CEleitorJaVotou)` | src/uenux2/src/app/vota/operador/u19-foreign-fragments.cpp | uenux2/src/app/vota/operador/confirmaidentidade/cinformaanonascimentoerrado.cpp + celeitorjavotou.cpp (caminho inferido) | média |
| 6057 | 351 | ✓ | `api_f6057` | `api::CPolySingletonList::exists<T> merged body, 20-char type names` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 6131 | 341 | ✓ | `api_f6131` | `api::CPolySingletonList::exists<T> merged body, 12-char type names` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 6132 | 341 | ✓ | `api_f6132` | `api::CPolySingletonList::exists<T> merged body, 16-char type names` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 6147 | 71 | ✓ | `api_f6147` | `lock-guard destructor merged body: if (owns) invoke(unlock slot, m)` | src/uenux2/src/api/pattern/cpolysingletonlist.h | biblioteca/helper inlinado: corpo mesclado dos dois destrutores de lock guard | média |
| 6161 | 138 | ✓ | `api_f6161` | `api::CPolySingletonList::find (merged body)` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 7181 | 2072 | ✓ | `vota::CSincronismoEleitor::vf6` | `vota::CSincronismoEleitor::ProcessMessage(short)` | src/uenux2/src/app/vota/eleitor/u19-foreign-fragments.cpp | uenux2/src/app/vota/eleitor/csincronismoeleitor.cpp (caminho inferido) | alta |
| 7667 | 2186 | ✓ | `api::CPolySingletonList::push@7667` | `api::CPolySingletonList::replace<api::ITextToSpeech>` (push inlinado) | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 8302 | 28435 | ✓ | `simulador::CSimuladorWasm::Executa` (antes mostrado pelas ferramentas como `api::CPolySingletonList::push@8302`) | `simulador::CSimuladorWasm::Executa(const std::function<void()>&)` | src/uenux2/mock/app/simulador/wasm/csimuladorwasm.u19.cpp | uenux2/mock/app/simulador/wasm/csimuladorwasm.cpp (caminho inferido) | baixa |
| 8728 | 118 | ✓ | `api_f8728` | `api::CPolySingletonList::replace<vota::IExecucaoVota>` (exists/erase, depois push 5395) | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 8758 | 2186 | ✓ | `api::CPolySingletonList::push@8758` | `api::CPolySingletonList::replace<api::IGenericFactory<api::ISemaphore>>` (push inlinado) | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 8834 | 2186 | ✓ | `api::CPolySingletonList::push@8834` | `api::CPolySingletonList::replace<api::IGenericFactory<api::IRWSyncCtl>>` (push inlinado) | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 8911 | 2186 | ✓ | `api::CPolySingletonList::push@8911` | `api::CPolySingletonList::replace<api::IGenericFactory<api::ISyncCtl>>` (push inlinado) | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 8986 | 2186 | ✓ | `api::CPolySingletonList::push@8986` | `api::CPolySingletonList::replace<api::IGenericFactory<api::IThreadImpl>>` (push inlinado) | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 9039 | 2186 | ✓ | `api::CPolySingletonList::push@9039` | `api::CPolySingletonList::replace<api::IFingerPrepare>` (push inlinado) | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 9135 | 2186 | ✓ | `api::CPolySingletonList::push@9135` | `api::CPolySingletonList::replace<ecourna::api::security::ISymmetricCipherFactory>` (push inlinado) | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 9233 | 2186 | ✓ | `api::CPolySingletonList::push@9233` | `api::CPolySingletonList::replace<ecourna::api::security::IRng>` (push inlinado) | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 10090 | 2186 | ✓ | `api::CPolySingletonList::push@10090` | `api::CPolySingletonList::replace<comum::IInterfaceSavd>` (push inlinado) | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 10224 | 67 |  | `std::__async_assoc_state<void, std::__async_func<vota::CThreadMonitor::SaiPorVotacaoSuspensa()::$_0>>::vf3` | `std::__async_assoc_state<void, __async_func<CThreadMonitor::SaiPorVotacaoSuspensa()::$_0>>::__execute()` | src/uenux2/src/app/vota/monitor/cthreadmonitor.u19.cpp | uenux2/src/app/vota/monitor/cthreadmonitor.cpp | alta |
| 10325 | 16 |  | `api_f10325` | `std::vector<api::SPolySingleton>::emplace_back(const char*&, &typeid(api::ISound), shared_ptr<api::ISound>&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: shared_ptr a partir de unique_ptr da libc++ (thunk de 1010) | média |
| 10335 | 14 |  | `api_f10335` | `std::shared_ptr<api::ISound>::shared_ptr(std::unique_ptr<api::ISound>&&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: shared_ptr a partir de unique_ptr da libc++ (thunk de 1010) | média |
| 10420 | 1917 |  | `vota::CPerguntaCodigoSuspensao::vf7` | `vota::CPerguntaCodigoSuspensao::ProcessInput()` | src/uenux2/src/app/vota/operador/u19-foreign-fragments.cpp | uenux2/src/app/vota/operador/aguardaeleitor/cperguntacodigosuspensao.cpp (caminho inferido) | alta |
| 10432 | 14 |  | `api_f10432` | `std::shared_ptr<vota::IExecucaoVota>::shared_ptr(std::unique_ptr<vota::IExecucaoVota>&&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: shared_ptr a partir de unique_ptr da libc++ (thunk de 1010) | média |
| 10470 | 14 |  | `api_f10470` | `std::shared_ptr<vota::impl::ISincronismoVotoEleitor>::shared_ptr(std::unique_ptr<vota::impl::ISincronismoVotoEleitor>&&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: shared_ptr a partir de unique_ptr da libc++ (thunk de 1010) | média |
| 10477 | 158 |  | `vota::CDigitalNaoReconhecidaDecBiometria::vf7` | `vota::CDigitalNaoReconhecidaDecBiometria::ProcessInput()` | src/uenux2/src/app/vota/operador/u19-foreign-fragments.cpp | uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaoreconhecidadecbiometria.cpp (caminho inferido) | alta |
| 10493 | 12 |  | `vota::CInformaAnoNascimentoErrado::vf7` | `vota::CInformaAnoNascimentoErrado::ProcessInput()` | src/uenux2/src/app/vota/operador/u19-foreign-fragments.cpp | uenux2/src/app/vota/operador/confirmaidentidade/cinformaanonascimentoerrado.cpp (caminho inferido) | alta |
| 10513 | 14 |  | `api_f10513` | `std::shared_ptr<vota::impl::IPoliticaExecucaoEleitor>::shared_ptr(std::unique_ptr<vota::impl::IPoliticaExecucaoEleitor>&&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: shared_ptr a partir de unique_ptr da libc++ (thunk de 1010) | média |
| 10523 | 400 |  | `vota::CHabilitaAudioEleitor::vf7` | `vota::CHabilitaAudioEleitor::ProcessInput()` | src/uenux2/src/app/vota/operador/u19-foreign-fragments.cpp | uenux2/src/app/vota/operador/confirmaidentidade/chabilitaaudioeleitor.cpp (caminho inferido) | alta |
| 10547 | 16 |  | `api_f10547` | `std::vector<api::SPolySingleton>::emplace_back(const char*&, &typeid(api::IGenericFactory<api::ISemaphore>), shared_ptr<api::IGenericFactory<api::ISemaphore>>&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: shared_ptr a partir de unique_ptr da libc++ (thunk de 1010) | média |
| 10555 | 14 |  | `api_f10555` | `std::shared_ptr<api::IGenericFactory<api::ISemaphore>>::shared_ptr(std::unique_ptr<api::IGenericFactory<api::ISemaphore>>&&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: shared_ptr a partir de unique_ptr da libc++ (thunk de 1010) | média |
| 10582 | 16 |  | `api_f10582` | `std::vector<api::SPolySingleton>::emplace_back(const char*&, &typeid(api::IGenericFactory<api::IRWSyncCtl>), shared_ptr<api::IGenericFactory<api::IRWSyncCtl>>&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: shared_ptr a partir de unique_ptr da libc++ (thunk de 1010) | média |
| 10584 | 20 |  | `api_f10584` | `vota::DS_TextoAudio() -> IInformacaoThreadOperador slot 8 GetTextoAudio` | src/uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.u19.cpp | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp (caminho inferido) | média |
| 10585 | 14 |  | `api_f10585` | `std::shared_ptr<api::IGenericFactory<api::IRWSyncCtl>>::shared_ptr(std::unique_ptr<api::IGenericFactory<api::IRWSyncCtl>>&&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: shared_ptr a partir de unique_ptr da libc++ (thunk de 1010) | média |
| 10593 | 130 |  | `vota::CJustificativaEfetuada::vf7` | `vota::CJustificativaEfetuada::ProcessInput()` | src/uenux2/src/app/vota/operador/u19-foreign-fragments.cpp | uenux2/src/app/vota/operador/justificativa/cjustificativaefetuada.cpp (caminho inferido) | alta |
| 10607 | 16 |  | `api_f10607` | `std::vector<api::SPolySingleton>::emplace_back(const char*&, &typeid(api::IGenericFactory<api::ISyncCtl>), shared_ptr<api::IGenericFactory<api::ISyncCtl>>&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: shared_ptr a partir de unique_ptr da libc++ (thunk de 1010) | média |
| 10618 | 14 |  | `api_f10618` | `std::shared_ptr<api::IGenericFactory<api::ISyncCtl>>::shared_ptr(std::unique_ptr<api::IGenericFactory<api::ISyncCtl>>&&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: shared_ptr a partir de unique_ptr da libc++ (thunk de 1010) | média |
| 10620 | 96 |  | `ecourna::api::pattern::IObservableProgressWithDescription::vf0@10620` | `api::TPolySingletonsInfo::~TPolySingletonsInfo() (static @1832448)` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 10641 | 12 |  | `vota::CEleitorJaVotou::vf7` | `vota::CEleitorJaVotou::ProcessInput()` | src/uenux2/src/app/vota/operador/u19-foreign-fragments.cpp | uenux2/src/app/vota/operador/confirmaidentidade/celeitorjavotou.cpp (caminho inferido) | alta |
| 10643 | 16 |  | `api_f10643` | `std::vector<api::SPolySingleton>::emplace_back(const char*&, &typeid(api::IGenericFactory<api::IThreadImpl>), shared_ptr<api::IGenericFactory<api::IThreadImpl>>&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: shared_ptr a partir de unique_ptr da libc++ (thunk de 1010) | média |
| 10652 | 14 |  | `api_f10652` | `std::shared_ptr<api::IGenericFactory<api::IThreadImpl>>::shared_ptr(std::unique_ptr<api::IGenericFactory<api::IThreadImpl>>&&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: shared_ptr a partir de unique_ptr da libc++ (thunk de 1010) | média |
| 10670 | 7 |  | `api_f10670` | `vota::DS_IdentidadeDigitada() -> 3623` | src/uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.u19.cpp | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp (caminho inferido) | baixa |
| 10679 | 16 |  | `api_f10679` | `std::vector<api::SPolySingleton>::emplace_back(const char*&, &typeid(api::IFingerPrepare), shared_ptr<api::IFingerPrepare>&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: shared_ptr a partir de unique_ptr da libc++ (thunk de 1010) | média |
| 10687 | 14 |  | `api_f10687` | `std::shared_ptr<api::IFingerPrepare>::shared_ptr(std::unique_ptr<api::IFingerPrepare>&&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: shared_ptr a partir de unique_ptr da libc++ (thunk de 1010) | média |
| 10714 | 16 |  | `api_f10714` | `std::vector<api::SPolySingleton>::emplace_back(const char*&, &typeid(ecourna::api::security::ISymmetricCipherFactory), shared_ptr<ecourna::api::security::ISymmetricCipherFactory>&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: shared_ptr a partir de unique_ptr da libc++ (thunk de 1010) | média |
| 10723 | 14 |  | `api_f10723` | `std::shared_ptr<ecourna::api::security::ISymmetricCipherFactory>::shared_ptr(std::unique_ptr<ecourna::api::security::ISymmetricCipherFactory>&&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: shared_ptr a partir de unique_ptr da libc++ (thunk de 1010) | média |
| 10727 | 130 |  | `vota::CTituloEncerramentoInvalido::vf7` | `vota::CTituloEncerramentoInvalido::ProcessInput()` | src/uenux2/src/app/vota/operador/u19-foreign-fragments.cpp | uenux2/src/app/vota/operador/outrasopcoes/ctituloencerramentoinvalido.cpp (caminho inferido) | alta |
| 10750 | 16 |  | `api_f10750` | `std::vector<api::SPolySingleton>::emplace_back(const char*&, &typeid(ecourna::api::security::IRng), shared_ptr<ecourna::api::security::IRng>&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: shared_ptr a partir de unique_ptr da libc++ (thunk de 1010) | média |
| 10759 | 14 |  | `api_f10759` | `std::shared_ptr<ecourna::api::security::IRng>::shared_ptr(std::unique_ptr<ecourna::api::security::IRng>&&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: shared_ptr a partir de unique_ptr da libc++ (thunk de 1010) | média |
| 10801 | 16 | ✓ | `api_f10801` | `std::vector<api::SPolySingleton>::emplace_back(const char*&, &typeid(comum::IInterfaceSavd), shared_ptr<comum::IInterfaceSavd>&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: shared_ptr a partir de unique_ptr da libc++ (thunk de 1010) | média |
| 10807 | 14 | ✓ | `api_f10807` | `std::shared_ptr<comum::IInterfaceSavd>::shared_ptr(std::unique_ptr<comum::IInterfaceSavd>&&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: shared_ptr a partir de unique_ptr da libc++ (thunk de 1010) | média |
| 10876 | 2140 | ✓ | `api::CPolySingletonList::push@10876` | `api::IAjusteDataHora::CreateInst()` | src/uenux2/src/api/util/iajustedatahora.u19.cpp | uenux2/src/api/util/iajustedatahora.h (caminho inferido) | baixa |
| 10981 | 433 | ✓ | `std::function<api::BatteryIconDataSource<0>::BatteryIconDataSource::'lambda'>::operator()` | `api::BatteryIconDataSource<Horizontal>::UpdateBatteryIcon (timer lambda operator())` | src/uenux2/src/api/gui/cpowerinformation.u19.cpp | uenux2/src/api/gui/cpowerinformation.cpp | alta |
| 10987 | 433 | ✓ | `std::function<api::BatteryIconDataSource<1>::BatteryIconDataSource::'lambda'>::operator()` | `api::BatteryIconDataSource<Vertical>::UpdateBatteryIcon (timer lambda operator())` | src/uenux2/src/api/gui/cpowerinformation.u19.cpp | uenux2/src/api/gui/cpowerinformation.cpp | alta |
| 10991 | 18 |  | `api_f10991` | `exit-time destructor of api::CPowerInformation::ms_icones` | src/uenux2/src/api/gui/cpowerinformation.u19.cpp | uenux2/src/api/gui/cpowerinformation.cpp | média |
| 11157 | 1846 |  | `api::CPolySingletonList::instance@11157` | `api::CPolySingleton<api::CEscritorLog>::instance` | src/uenux2/src/api/pattern/cpolysingleton.h | uenux2/src/api/pattern/cpolysingleton.h | alta |
| 11530 | 415 |  | `api::ITextToSpeech::vf0` | `api::ITextToSpeech::GetAudio(const std::string&) [vtable slot 0]` | src/uenux2/src/api/audio/itexttospeech.u19.cpp | uenux2/src/api/audio/itexttospeech.cpp (caminho inferido) | média |
| 11533 | 53 |  | `ecourna_f11533` | `std::_AllocatorDestroyRangeReverse<allocator<SPolySingleton>>::operator()` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: functor de rollback da libc++ | média |
| 11544 | 26 |  | `api_f11544` | `std::__allocator_destroy(first, last) for SPolySingleton` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: destruição de intervalo da libc++ | média |
| 12016 | 37 |  | `api_f12016` | `exit-time destructor of the static std::unique_ptr<api::CLp>` | src/uenux2/mock/app/simulador/wasm/cwasmclp.u19.cpp | uenux2/mock/app/simulador/wasm/cwasmclp.cpp | média |
| 12053 | 94 |  | `api_f12053` | `vota::(anonymous namespace)::QRCodeAtual()` | src/uenux2/src/app/vota/eleitor/fimvotacao/cmostraqrcodebu.u19.cpp | uenux2/src/app/vota/eleitor/fimvotacao/cmostraqrcodebu.cpp | média |
| 12054 | 208 |  | `api_f12054` | `vota::(anonymous namespace)::TextoInstrucaoQRCode()` | src/uenux2/src/app/vota/eleitor/fimvotacao/cmostraqrcodebu.u19.cpp | uenux2/src/app/vota/eleitor/fimvotacao/cmostraqrcodebu.cpp | média |
| 13156 | 129 | ✓ | `api_f13156` | `api::(anonymous)::LogUenux(int, const char*, ...)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | uenux2/src/api/pattern/cpolysingletonlist.h | baixa |
| 13621 | 318 | ✓ | `api_f13621` | `api::CUpgradeMutex::upgrade(std::shared_lock&) / unlock_shared_and_lock()` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |
| 14423 | 16 | ✓ | `api_f14423` | `std::vector<api::SPolySingleton>::emplace_back(const char*&, &typeid(api::ITextToSpeech), shared_ptr<api::ITextToSpeech>&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: shared_ptr a partir de unique_ptr da libc++ (thunk de 1010) | média |
| 14456 | 14 |  | `api_f14456` | `std::shared_ptr<api::ITextToSpeech>::shared_ptr(std::unique_ptr<api::ITextToSpeech>&&)` | src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp | biblioteca/helper inlinado: shared_ptr a partir de unique_ptr da libc++ (thunk de 1010) | média |
| 14476 | 31 | ✓ | `api_f14476` | `api::CPolySingletonList::trace(funcao, tipo, detalhe)` | src/uenux2/src/api/pattern/cpolysingletonlist.h | uenux2/src/api/pattern/cpolysingletonlist.h | média |

## 11. Código estranho ou arriscado

1. **Locks com espera ativa que podem travar a aba** (`CUpgradeMutex`, funcs 1126/3095/13621 e todas as cópias
   inlinadas). `condition_variable::wait` é um no-op sem pthreads, então qualquer pedido de lock que não possa ser
   concedido gira para sempre. Duas formas de chegar lá:
   * `erase` (640) destrói o objeto substituído **enquanto segura o lock exclusivo**. Se esse destrutor
     chamar qualquer `GetInst()`/`instance()`, entra em laço em `lock_shared`.
   * As cópias inlinadas de `instance`/`push` não têm landing pads (§9). Um throw nas linhas 99/105 deixa um leitor
     contado (o próximo escritor gira); um throw na linha 129 deixa o lock de escrita tomado (o próximo leitor ou escritor
     gira).

   Nenhum dos dois é alcançável hoje: os objetos substituídos têm destrutores triviais, e os throws são protegidos por
   `exists`. O código é frágil, porém, porque uma mudança futura congelaria a página sem nenhum erro.
   Impacto: simulador (travamento). Na urna, o mesmo código bloqueia threads em vez de girar. Baixo.
2. **Substituir um singleton destrói o objeto por trás de referências ainda em uso.** `instance()` retorna um
   `T&` cru, e o `replace` de uma interface já registrada apaga e destrói a instância antiga (`ISound`,
   `ITextToSpeech` e `ISystemDateTime` são substituídas na inicialização, e `ITextToSpeech` de novo a cada
   `votaInit`). Uma classe que tivesse guardado a referência antiga ficaria com uma referência pendente. Nenhum cache desse tipo foi encontrado
   nos caminhos que foram executados. Impacto: simulador (e urna, se substituições acontecessem lá). Baixo.
3. **Buffer da pilha não inicializado impresso** (func 13156). Com `DEBUG_UENUX` definido, `printf("%s: ", buf)` imprime
   140 bytes da pilha que nada escreveu. Isso foi verificado: prefixos como `bf60`, `À`, `bf600§`. Pode
   ler além do buffer até um byte NUL (vazamento de conteúdo da pilha para o stdout e potencial
   leitura além dos limites). Impacto: simulador, só quando um desenvolvedor define a variável. A urna usa syslog. Baixo.
4. **Trace sempre ligado** (a func 11265 define `debug = true`). Todo `push` executa `std::format` + `__cxa_demangle`
   + `access("/dev/urna")`/`getenv`. O custo é pequeno, mas mostra que a chave de diagnóstico está fixa no código.
   Info.
5. **`emscripten_sleep(3000)` em `CPerguntaCodigoSuspensao::ProcessInput`** (10420), atrás da flag
   @1584624 (= 1). Num build sem Asyncify, abortaria o módulo quando um mesário digitasse um título inválido
   para suspender um voto. É inalcançável no navegador (thread do operador não iniciada). Na urna, é
   uma pausa de 3 s. Info.
6. **`std::async` sem threads** (10224, `CThreadMonitor::SaiPorVotacaoSuspensa`). `launch::async` precisa de
   `pthread_create`. Neste build sem pthreads, lançaria `system_error`. O caminho é inalcançável (a
   thread de monitoramento não é iniciada). Info.
7. **BU de tela vs BU impresso** (1956). Os QR codes de tela são regenerados com limite de 2500 caracteres
   em vez de 1100. Carregam os mesmos dados, mas com fatiamento, cadeia de hashes e assinatura diferentes. Quem
   comparar QR codes da tela e do papel não deve esperar payloads idênticos. Trata-se também de uma segunda geração, independente,
   a partir do estado em memória (RDV e `vota.bin`) no momento da exibição, não de uma cópia do que foi impresso.
   Info.
8. **Verificação de `CPerguntaCodigoSuspensao`** (10420): o título digitado deve ser válido e diferente da identidade
   digitada do eleitor que está sendo suspenso. Nada mais limita quem pode suspender (nesta função, não há verificação de que o título
   pertença a um mesário registrado). Mesmo código da urna. Info.

## 12. Questões em aberto

* Os nomes reais da struct do registro (`SPolySingleton`), da classe do lock (`CUpgradeMutex`), da
  flag `debug`, do acessor via ponteiro de função, do wrapper registrar-ou-substituir (`replace`) e do seu
  helper por valor (4162/1562), e a escolha entre `exists` e `contains`. Nenhum deles tem string ou srcloc.
  As reconstruções de outras unidades (por exemplo, `vota_web_wasm.u29/u30.cpp`) ainda escrevem os registros da inicialização como
  `push<T>`; no binário eles passam pelo wrapper (§3.4).
* O que o buffer não inicializado de 140 bytes da func 13156 deveria conter (um timestamp ou um nome de
  thread/processo, preenchido por uma chamada que o build web compilou para nada).
* Por que o acessor (func 11265) escreve `debug = true` a cada chamada. Pode ser uma sobrescrita do build web
  (a variável do ponteiro fica ao lado dos dados de `vota_web_wasm.cpp`) ou o comportamento padrão do header.
* O significado do int `10` que `main` passa para `CWasmInit` (slot 4 de IInterfaceInit), e os nomes dos slots
  24/25 de `IControladorRegistraMesarios` e 4 de `IBeep`.
* O arquivo exato da func 8302 (`csimuladorwasm.cpp` é um palpite) e dos estados do operador da §6.4.
