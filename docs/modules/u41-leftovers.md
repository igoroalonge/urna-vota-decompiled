# u41: sobras, sete funções reatribuídas ao código do TSE depois da primeira passada

A unidade u41 contém 7 funções wasm que não pertenciam a nenhuma unidade da primeira passada de reconstrução (u01–u40). Três
tinham sido arquivadas sob uma biblioteca ou sob `unknown` (`rhvoice_f501`, `shared_f10255`, `unknown_f7909`) e depois foram
reatribuídas ao código do TSE. As outras quatro tinham componente, mas não tinham nome (`api_f2741`, `api_f2744`, `api_f3884`,
`simulador_f3326`). Cinco das sete foram executadas durante os votos gravados (`analysis/runtime/vote_*.functions.tsv`).
A passada de consistência acrescentou depois uma oitava função, a func 5922 `vota::LegendaValida` (veja o Adendo no final),
então `analysis/units.json` agora lista 8 funções para a u41.

Depois de lidas, elas se dividem em cinco pequenos grupos:

| grupo | funcs | o que são |
|---|---|---|
| fila de mensagens entre threads | 501 | `api::CPriorityMessageQueue<api::SMessage>::Add`, a única forma de uma mensagem chegar à máquina de estados do eleitor ou do operador (mesário) |
| registro de poly-singletons | 3884, 2741, 2744 | um corpo mesclado de `CPolySingletonList::exists<T>` e dois dos seus thunks (`ISincronismoVotoEleitor`, `IPoliticaExecucaoEleitor`) |
| timer web | 7909 | `simulador::CWasmTimer::Dispara`, o callback de `setTimeout` por trás de todo timer periódico da GUI do simulador |
| ponto de entrada de thread | 10255 | `api::CThread::ThreadProc`, a rotina inicial entregue a `IThreadImpl::Create` (nunca roda no navegador) |
| **não é código do TSE** | 3326 | `std::string::insert(const_iterator, char)` da libc++. As ferramentas a chamaram de `simulador_f3326` porque `simulador::ResolveCaminho` é um dos seus três chamadores |

Três das sete (501, 7909, 10255) já tinham uma reconstrução escrita pela unidade dona do seu arquivo
(u18, u31). Essas reconstruções foram reverificadas aqui instrução por instrução. Elas são fiéis, e
esta unidade só acrescentou comentários (a ordem no nível dos bytes, os caminhos de exceção, uma afirmação errada corrigida). As outras quatro são
novas: 3884/2741/2744 agora têm um comentário no template `exists<>` e instanciações explícitas, e 3326 tem um
corpo de referência.

Arquivos-fonte reconstruídos tocados por esta unidade (todas as edições são aditivas e marcadas com "u41"):

```
src/uenux2/src/api/ipc/cmessagequeue.h                      Add (501): exception-path notes, prioridade-100 sender,
                                                            +16 = priority_queue comparator
src/uenux2/src/api/pattern/cpolysingletonlist.h             exists<INTERFACE>: merged body 3884 and its thunks
src/uenux2/src/api/pattern/cpolysingletonlist.u19.cpp       instantiation index: 3884, 2741, 2744
src/uenux2/src/app/vota/eleitor/csincronismovotoeleitor.cpp      template exists<ISincronismoVotoEleitor>  (2741)
src/uenux2/src/app/vota/eleitor/comum/cpoliticaexecucaoeleitor.cpp template exists<IPoliticaExecucaoEleitor> (2744)
src/uenux2/mock/app/simulador/wasm/cwasmresource.u29.cpp    call site + reference body of libc++ string::insert (3326)
src/uenux2/mock/app/simulador/wasm/cwasmtimer.cpp           Dispara (7909): re-check notes
src/uenux2/src/api/ipc/cthread.cpp                          ThreadProc (10255): re-check notes
```

## 0. Vocabulário

| termo | significado |
|---|---|
| urna / UE | a *urna eletrônica*, o equipamento de votação |
| eleitor / mesário | quem vota / membro da mesa receptora. Cada um tem uma thread com uma máquina de estados: `vota::CThreadEleitor`, `vota::CThreadOperador` |
| MT | o microterminal do mesário (teclado + LCD) |
| poly-singleton | o nome que o TSE dá a um singleton buscado por interface (`api::CPolySingletonList`, veja u19) |
| prioridade / sequência | prioridade / número de sequência de uma mensagem na fila |
| Dispara | "dispara" (o callback do timer) |

## 1. Onde estas funções se encaixam

```
                 JavaScript (page)                                 urna (not the web build)
   votaInit / votaTick            setTimeout(838, arg, ms)           IThreadImpl::Create(4527, this)
          │                                  │                                   │
   3903 Envia(id) ─► 7708 ─► 501 Add    7909 CWasmTimer::Dispara          10255 CThread::ThreadProc
          │               (lock, heap,       │  (std::function,                   │  Run(); m_estado = 3
          │                sem_post)         │   re-arm)                          │
   votaTick: CurrentStateName, BuildStateJson ──► IExecucaoVota::GetInst ─► 2736 ─┐
   CSincronismoEleitor::ProcessMessage ─► ISincronismoVotoEleitor::GetInst ─► 2741 ─┼─► 3884 exists body
   CConfirmaVotoNominal::PosTecla ─► IPoliticaExecucaoEleitor::GetInst ─► 2744 ─┘     (lock_shared, find)
   simulador::ResolveCaminho ─► 3326 std::string::insert(begin(), '/')   (also ASN.1 runtime, Boost.Regex)
```

## 2. `api::CPriorityMessageQueue<api::SMessage>::Add` (func 501)

A classe é descrita na u18 (`cmessagequeue.h`, srclocs :113, :114, :153). Cada thread de máquina de estados é dona de
uma fila (`CThreadEleitor+36`, `CThreadOperador+36`). Outras threads fazem `Add` de mensagens e a dona as retira com `Remove`
(func 2073) em ordem de prioridade, FIFO dentro de uma mesma prioridade.

Layout usado por `Add` (`this` = a fila):

| offset | membro |
|---:|---|
| +4 | `std::priority_queue<SEntrada, vector<SEntrada>, CMenorPrioridade>`: o vector (begin/end/cap) |
| +16 | o comparador vazio `comp` de `std::priority_queue`. A libc++ o declara como um membro comum, então ele ocupa 1 byte com padding até 4. A u18 tinha deixado essa palavra sem explicação |
| +20 | `std::unique_ptr<ISemaphore> m_pSemaforo` (web: `CPosixSemaphore`) |
| +24 | `std::unique_ptr<ISyncCtl> m_pLock` (web: `CPosixMutex`) |
| +28 | `unsigned m_sequencia` |

Corpo, na ordem em que o wasm o executa:

1. `CLockGuard lock(*m_pLock)`: slot 2 de `ISyncCtl` (`Lock`) como um `call_indirect` simples. Ele roda antes de o
   guard existir, então nada precisa ser desfeito se lançar exceção.
2. `seq = m_sequencia++`. O contador é gravado de volta antes do push, então um push que falha ainda consome um número.
3. `m_fila.push({mensagem, prioridade, seq})` via `invoke_vii` da func 11076 (`push_back` com o
   slow path `__swap_out_circular_buffer`, depois o sift-up de `push_heap`). O `SMessage` de 8 bytes é copiado como um
   único i64. O comparador compara `prioridade` **com sinal** e `sequencia` **sem sinal** (a mais antiga primeiro em caso de empate).
4. `m_pSemaforo->Unlock()`: slot 3 de `ISemaphore` = `CPosixSemaphore::Unlock` (srcloc :99, `sem_post`), via `invoke_vi`.
5. `~CLockGuard` (func 5528, slot 3 de `ISyncCtl`). O landing pad compartilhado pelos passos 3 e 4 executa o mesmo destrutor
   e relança (`__resumeException`).

Chamadores: 34 pontos de chamada diretos em 30 funções, quase todos estados do eleitor e do mesário (`CEleitorVotando`,
`CSincronismoEleitor`, `CFimVotoEleitor`, `CSincronismoOperador`, `CHabilitaAudioEleitor`, …) e o wrapper de uma linha
`Envia` 7708 usado pelos pontos de entrada web (u33: 3903 ← 11026/11100/10376). **Todos passam prioridade 1,
exceto um.** `vota::CVerificaEleicaoPassou::StartState` (func 11914, u09) posta a mensagem 0 (parar) na fila do
*operador* com **prioridade 100** quando o limite da data da eleição já passou fora do modo de treinamento ("Data
da eleição inválida"). O comentário da u18 e a string de evidência das ferramentas ("every sender posts SMessage with
priority 1") estavam errados neste ponto. O comentário do header agora está corrigido.

Observada em execução nos dois votos gravados (arestas 7708 → 501 e 4464 → 501).

## 3. `CPolySingletonList::exists<T>`: corpo 3884, thunks 2741 e 2744

`exists<INTERFACE>(info)` (u19; outros capítulos a chamam de `contains`) responde "foi registrada uma implementação de
INTERFACE?":

```cpp
template <typename INTERFACE>
static bool exists(TPolySingletonsInfo& info)
{
    const std::string nome = typeid(INTERFACE).name();
    std::shared_lock leitura(info.mutex);
    return find(nome, info).first;
}
```

O *merge-similar-functions* do wasm-opt reduziu suas ~37 instanciações a alguns poucos corpos que diferem apenas em como
o nome chega até eles. Os thunks passam a parte que varia como uma constante (u19 §3.1, `cpolysingletonlist.u19.cpp`):

| corpo | argumento de nome | thunks |
|---|---|---|
| 1166 | a própria string do nome mangled (`"N3api13ITextToSpeechE"`) | 11 |
| **3884** | **o endereço do campo de nome do `type_info`** (`&typeid(T) + 4`), desreferenciado pelo corpo | **2736 `vota::IExecucaoVota`, 2741 `vota::impl::ISincronismoVotoEleitor`, 2744 `vota::impl::IPoliticaExecucaoEleitor`** |
| 3928 | igual a 3884, mas com o construtor `std::string(const char*)` inlinado (`strlen` + `memcpy`) | 3 |
| 6131/6132/6057/2924 | nenhum: o nome é montado inline a partir de pedaços de 8 bytes | 10 |

3884 é, instrução por instrução, a func 1166 mais um `i32.load` (verificado com um diff do WAT). As
constantes dos thunks são os campos `__type_name` dos objetos `type_info` das três interfaces:

| thunk | constante | `type_info` em | `*constant` |
|---|---|---|---|
| 2736 | 1600568 | 1600564 (vtable `__class_type_info` @1525564) | `"N4vota13IExecucaoVotaE"` |
| 2741 | 1534152 | 1534148 | `"N4vota4impl23ISincronismoVotoEleitorE"` |
| 2744 | 1536364 | 1536360 | `"N4vota4impl24IPoliticaExecucaoEleitorE"` |

Seus chamadores confirmam os tipos. `replace<ISincronismoVotoEleitor>` (9437, chamada a partir de `main`) executa
`if (2741(info)) erase(string(*1534152), info); push 5398`. O
`IPoliticaExecucaoEleitor::GetInst()` inlinado em 5925 executa `if (!2744(info)) push 5407(new CPoliticaExecucaoEleitor)`
e depois testa 2744 de novo para `CPolySingleton<>::instance` (cpolysingleton.h:78, "PolySingleton - solicitada uma
instancia nao criada N4vota4impl24IPoliticaExecucaoEleitorE"). `__wasm_call_ctors` chama os dois thunks e
descarta o resultado (u07 §8).

Por que essas três (e os três thunks de 3928) leem o nome em tempo de execução: nas unidades de tradução que
as emitiram, `typeid(T).name()` não foi dobrado em uma constante de string, mas mantido como um load a partir do objeto
`type_info`. Uma razão provável é que o `type_info` foi definido em outra TU (a interface tem uma key function
out-of-line, enquanto os headers da u06/u07 declaram esses destrutores inline). O wasm-opt então reescreveu
`i32.const ti; i32.load offset=4` como `i32.const ti+4; i32.load`, e a mesclagem transformou essa constante em
parâmetro.

Corpo 3884, passo a passo (frame de 32 bytes relativo ao sp):

1. `std::string nome(*pNome)`: func 148, em sp+20. Roda antes do lock, como uma chamada simples.
2. `std::shared_lock leitura(info.mutex)`: `{&info + 16, owns = 1}` em sp+12, depois `CUpgradeMutex::lock_shared`
   (func 3095) através de `invoke_vi`. Se isso lançar exceção, só `nome` é destruído, porque o construtor do `shared_lock`
   não terminou.
3. `find(nome, info)`: func 608 através de `invoke_viii`, com o par retornado em sp+4. O resultado é `.first`
   (o byte em sp+4).
4. `~shared_lock` (func 1322 → `unlock_shared`) e `~string` (func 149). Se `find` lançar exceção, o landing pad executa
   os dois e relança.

Em tempo de execução: `IExecucaoVota::GetInst()` (func 3594) é chamada repetidamente pelo adaptador web. A cada
`votaTick`, `CurrentStateName` (5408) e `CVotaWebEngine::BuildStateJson` (5500) pedem a ela o estado corrente.
A aresta 3594 → 2736 tem 1450 amostras em `vote_geral_t1` (2736 → 3884: 1424), cerca de 1 % do `votaTick`. Uma
string é alocada e um `find_if` linear roda toda vez que a política de execução é buscada. 2741 foi executada a partir de
`__wasm_call_ctors`, `main` (9437) e `CSincronismoEleitor::ProcessMessage` (7181). 2744 foi executada a partir de `main`
(9507).

## 4. A func 3326 é `std::string::insert(const_iterator, char)` da libc++, não código do simulador

O corpo corresponde exatamente a `basic_string<char>::insert(const_iterator __pos, value_type __c)` da libc++:
`ip = pos - data()`, um teste de capacidade (`(cap_word & 0x7fffffff) - 1` para uma string longa, 10 para uma curta), depois
ou `__grow_by_without_replace(cap, 1, sz, ip, 0, 1)` (func 1716) ou um `memmove` da cauda em um byte.
Em seguida armazena `c` e o NUL terminador, define o tamanho (longa: palavra +4, curta: byte +11 `& 0x7f`) e retorna
`begin() + ip`.

Seus três chamadores vêm de três componentes sem relação entre si:

* `simulador::ResolveCaminho` (2626, u29): `s.insert(s.begin(), '/')` torna absoluto um nome de recurso depois que o
  prefixo `':'` do Qt foi removido. Não foi executada nos votos gravados. Todos os 36 literais de nome de recurso no
  binário são `":/resource/…"`, e eles já começam com `'/'` depois que o `':'` sai.
* o decodificador AVN do runtime ASN.1 (5055, para `OCTET STRING` / `BIT STRING`);
* `basic_regex_creator<char>::append_set` do Boost.Regex (5139), através do slot 6483 da tabela e de `invoke_iiii`.

A atribuição correta é **lib:libcxx** (uma instanciação de `<string>`). O corpo de referência está escrito
como comentário ao lado do seu ponto de chamada em `cwasmresource.u29.cpp`.

## 5. `simulador::CWasmTimer::Dispara` (func 7909)

O simulador não tem pthreads, então `api::CTimer` (uma `std::thread`) não funciona. `simulador::CWasmTimer` (u31)
se rearma com `emscripten_async_call(838, arg, ms)`, isto é, um `setTimeout` do JavaScript, e o slot 838 da tabela é
esta função. `arg` é um `SAgendamento {shared_ptr<State> estado; uint64 geracao;}` no heap (16 bytes). O `State`
compartilhado (48 bytes) contém `ativo` (+0), `geracao` (+8), `intervalo` em ms (+16) e a `std::function<void()>` (+24,
`__f_` em +40).

```cpp
void CWasmTimer::Dispara(void* arg)
{
    auto* ag = static_cast<SAgendamento*>(arg);
    State& st = *ag->estado;
    if (st.ativo && st.geracao == ag->geracao) {
        st.callback();                               // __f_ null -> __throw_bad_function_call (648); else slot 6
        if (st.ativo && st.geracao == ag->geracao)   // the callback may have stopped/restarted the timer
            emscripten_async_call(&Dispara, new SAgendamento{ag->estado, ag->geracao}, st.intervalo);
    }
    delete ag;                                       // ~shared_ptr (ctrl slot 2 + __release_weak) + free
}
```

A reconstrução da u31 corresponde ao wasm, incluindo a verificação dupla, a cópia do `shared_ptr` (contador de uso em
ctrl+4, pulado para um bloco de controle nulo) e a geração armazenada, que é o valor corrente do State e
igual a `ag->geracao` nesse ponto. Cada chamada pendente é dona de uma referência ao `State`. Se o callback destruir
o `CWasmTimer` (seu destrutor chama `Stop()`, que incrementa `geracao`), o `State` e a
`std::function` em execução continuam vivos até `Dispara` retornar. A reverificação então falha e nada é rearmado.
Observada em execução: todo tick do relógio da barra de status, campo piscando e atualização do ícone de bateria (u31: 179 armações em um
voto municipal).

## 6. `api::CThread::ThreadProc` (func 10255)

```cpp
void* CThread::ThreadProc(void* parametro)        // table slot 4527
{
    auto* thread = static_cast<CThread*>(parametro);
    thread->Run();                                 // vtable slot 2, plain call_indirect (no try)
    thread->m_estado = TERMINADA;                  // +4 = 3
    return nullptr;
}
```

`CThread::Start` (1684, cthread.cpp:68) a passa para `IThreadImpl::Create` (slot 2) com `this`. Os únicos
chamadores de Start são a política de execução da urna `vota::CExecucaoVota` (10233 `IniciaOperador`, 10235 `Inicia`,
10236 `Executa`). O build web usa a política cooperativa e nunca inicia uma thread real, então 10255 é código morto no
simulador (não aparece nos perfis). A reconstrução da u18 é exata.

## 7. Observações sobre wasm / Emscripten

* **merge-similar-functions com uma constante ponteiro-para-campo** (3884). Corpos mesclados normalmente recebem uma string ou uma
  vtable como parâmetro. Aqui é `&typeid(T) + 4`, que só existe porque `OptimizeInstructions` dobrou
  o offset do load no endereço constante antes da mesclagem.
* **Código de biblioteca nomeado a partir de um chamador** (3326). Uma instanciação da libc++ compartilhada pelo simulador, pelo
  runtime ASN.1 e pelo Boost.Regex ganhou um nome `simulador_` porque o classificador escolheu um dos chamadores. Ela também está na
  tabela de funções (slot 6483) porque o Boost a chama através de `invoke_iiii`.
* **Callbacks de setTimeout rodam fora do `votaTick`** (7909). Uma exceção vinda de um callback de timer não é capturada pelo
  `try` da API C e sobe até `callUserCallback`. Veja o comentário em `cwasmtimer.cpp` (u31).
* **`invoke_*` revela os escopos RAII**. Em 501 e 3884, as chamadas feitas *antes* de o objeto RAII estar completo
  (`Lock`, construtor de `std::string`) são chamadas simples. As feitas enquanto ele está vivo passam por `invoke_*`,
  e seus landing pads executam os destrutores. Foi assim que os escopos do guard e do lock acima foram recuperados.

## 8. Código estranho ou arriscado

| # | func | o quê | impacto |
|---|---|---|---|
| 1 | 501 | o número de sequência é consumido antes do push. Se o push lançar exceção (`bad_alloc`), um número é pulado | inofensivo: os números só ordenam as mensagens |
| 2 | 501 | se `ISemaphore::Unlock` lançar exceção, a mensagem fica na fila sem a contagem correspondente no semáforo, então o consumidor pode nunca acordar para ela | inalcançável no build web (`sem_post` é um stub) |
| 3 | 501 / 11914 | uma mensagem de "parar" com prioridade 100 passa à frente de todas as mensagens pendentes de prioridade 1 na fila do operador | presumivelmente intencional (é a única prioridade diferente de 1) |
| 4 | 3884 | todo `IExecucaoVota::GetInst()` aloca uma `std::string`, pega o lock compartilhado e faz uma busca linear. O adaptador web faz isso várias vezes por tick | cerca de 1 % do tempo do `votaTick` |
| 5 | 7909 | uma exceção vinda do callback vaza o `SAgendamento` e sua referência ao `State`, e para o timer de vez | veja u31 |

## 9. Questões em aberto

* Os nomes de membros `Add`, `exists`, `Dispara` e `ThreadProc` são inferidos. Nenhuma string ou srcloc os nomeia.
* Ainda está em aberto por que `__wasm_call_ctors` chama `exists<ISincronismoVotoEleitor>` / `exists<IPoliticaExecucaoEleitor>` e
  descarta o resultado (a u07 sugere verificações de registro do tipo `static const bool`).
* O banco de dados da análise ainda tem os nomes antigos de 2741, 2744, 3884 e 3326 (`api_fNNNN`, `simulador_f3326`).
  Os nomes da tabela abaixo são os que devem ir para `analysis/names.override.json`.

## 10. Tabela de mapeamento (as 7 funções da primeira passada; a 5922 está no Adendo)

`ran` = vista em execução nos votos gravados (`analysis/runtime/vote_*.functions.tsv`).

| func | bytes | ran | nome nas ferramentas | símbolo reconstruído | arquivo original | reconstrução | conf. |
|---|---|---|---|---|---|---|---|
| 501 | 217 | ✔ | `api::CPriorityMessageQueue<api::SMessage>::Add` (era `rhvoice_f501`) | `api::CPriorityMessageQueue<api::SMessage>::Add(const SMessage&, int prioridade)` | uenux2/src/api/ipc/cmessagequeue.h (arquivo atestado pelos srclocs :113/:114/:153; nome inferido) | src/uenux2/src/api/ipc/cmessagequeue.h | alta (código), média (nome) |
| 2741 | 12 | ✔ | `api_f2741` | `api::CPolySingletonList::exists<vota::impl::ISincronismoVotoEleitor>(TPolySingletonsInfo&)` (thunk → 3884) | uenux2/src/api/pattern/cpolysingletonlist.h | src/uenux2/src/app/vota/eleitor/csincronismovotoeleitor.cpp (instanciação explícita); template em cpolysingletonlist.h | alta (tipo), média (nome do membro) |
| 2744 | 12 | ✔ | `api_f2744` | `api::CPolySingletonList::exists<vota::impl::IPoliticaExecucaoEleitor>(TPolySingletonsInfo&)` (thunk → 3884) | uenux2/src/api/pattern/cpolysingletonlist.h | src/uenux2/src/app/vota/eleitor/comum/cpoliticaexecucaoeleitor.cpp (instanciação explícita); template em cpolysingletonlist.h | alta (tipo), média (nome do membro) |
| 3326 | 208 |  | `simulador_f3326` | `std::basic_string<char>::insert(const_iterator, char)` (libc++; componente **lib:libcxx**, não é do TSE) | libc++ `<string>` (instanciação) | src/uenux2/mock/app/simulador/wasm/cwasmresource.u29.cpp (ponto de chamada + corpo de referência, em comentário) | alta |
| 3884 | 201 | ✔ | `api_f3884` | corpo mesclado de `api::CPolySingletonList::exists<T>` `(info, const char* const* pNome)` para `IExecucaoVota` / `ISincronismoVotoEleitor` / `IPoliticaExecucaoEleitor` | uenux2/src/api/pattern/cpolysingletonlist.h | src/uenux2/src/api/pattern/cpolysingletonlist.h (template + comentário), cpolysingletonlist.u19.cpp (índice) | alta |
| 7909 | 231 | ✔ | `simulador::CWasmTimer::Dispara` (era `unknown_f7909`) | `static void simulador::CWasmTimer::Dispara(void*)` (slot 838 da tabela) | uenux2/mock/app/simulador/wasm/cwasmtimer.cpp (caminho inferido) | src/uenux2/mock/app/simulador/wasm/cwasmtimer.cpp | alta (código), média (nome) |
| 10255 | 24 |  | `api::CThread::ThreadProc` (era `shared_f10255`) | `static void* api::CThread::ThreadProc(void*)` (slot 4527 da tabela) | uenux2/src/api/ipc/cthread.cpp (arquivo atestado pelos srclocs :31/:32/:68; nome inferido) | src/uenux2/src/api/ipc/cthread.cpp | alta (código), média (nome) |

Funções de outras unidades referenciadas acima (para navegação, não fazem parte da u41): 1166/3928/6131/6132/6057/2924
(outros corpos de `exists`), 2736 (`exists<IExecucaoVota>`), 3594 (`IExecucaoVota::GetInst`), 608/6161 (`find`), 3095
(`lock_shared`), 1322 (`~shared_lock`), 148/149 (ctor/dtor de `std::string`), 5398/5407 (`push<>`), 9437/9507
(`replace<>` a partir de `main`), 640 (`erase`), 5925 (`CConfirmaVotoNominal::PosTecla`), 7181
(`CSincronismoEleitor::ProcessMessage`), 11076 (push no heap), 5528 (`~CLockGuard`), 2073 (`Remove`), 7708/3903
(`Envia`), 11914 (`CVerificaEleicaoPassou::StartState`), 1716 (`__grow_by_without_replace`), 2626
(`ResolveCaminho`), 5055 (decodificador AVN do ASN.1), 5139 (`append_set` do Boost), 7918/7934/3347 (Start/Stop/dtor de
`CWasmTimer`), 648 (`__throw_bad_function_call`), 1684 (`CThread::Start`), 10233/10235/10236 (`CExecucaoVota`).

## 11. Revisão de fidelidade (2026-09-23)

Cada função foi comparada com seu corpo descompilado e seu WAT. As vizinhas em que o texto se apoia também foram verificadas:
1166 (diff do WAT com 3884), 3928, 9437, 5925, 11076, 11914, 1684, 7918 e os arquivos de arestas de execução.
Constatações e correções:

* **501**: a reconstrução da u18 corresponde (lock → seq++ → push no heap → `Unlock` do semáforo → dtor do guard, com o
  landing pad cobrindo os passos 3–4). Corrigido: a afirmação "todos os remetentes usam prioridade 1" (a func 11914 usa 100). Acrescentado:
  a palavra em +16 é o comparador da `priority_queue`, `prioridade` é comparada com sinal e `sequencia` sem sinal
  (`i32.lt_s` / `i32.gt_u` em 11076), e o que os caminhos de exceção deixam para trás.
* **3884 / 2741 / 2744**: novas. A identidade de cada thunk foi verificada de três formas: a constante aponta para o
  campo `__type_name` do `type_info` correspondente, `replace<>` (9437/9507) passa o mesmo nome para `erase`, e
  `CPolySingleton<>::instance` em 5925 formata o mesmo nome mangled na sua mensagem de erro.
* **3326**: reatribuída à libc++. O corpo é o `insert(const_iterator, char)` da libc++, até na constante de capacidade 10
  da string curta e no byte de tamanho `& 0x7f`.
* **7909**: a reconstrução da u31 corresponde (as duas verificações de `ativo`/`geracao`, verificação de nulo da `std::function` → func 648,
  chamada do slot 6, cópia e liberação do `shared_ptr`, `free`). Nenhuma mudança no código. Notas acrescentadas.
* **10255**: a reconstrução da u18 corresponde (chamada do slot 2, `+4 = 3`, `return 0`). Nenhuma mudança no código. Notas acrescentadas.


## Adendo: func 5922 `vota::LegendaValida` (acrescentada na passada de consistência)

A func 5922 (243 bytes, `(i32 cargo, i32 partido) -> i32`) ficou sem componente porque só dois
chamadores a usam: `CVotaWebEngine::BuildStateJson` (func 5500, o `legendaValida` do JSON de estado web) e
`vota::CPedeProporcional::ProcessInputAudio` (func 11711). Ela busca o número do partido no map
`comum::CPartidos` (func 819). Se o partido existe, percorre `comum::CCandidaturas` (func 521) procurando uma
candidatura daquele cargo e partido, aceitando também uma federação através de `comum::CFederacoes` (func 2832).
O resultado decide se os dois primeiros dígitos digitados para um cargo proporcional são um **voto de
legenda** válido. O nome é inferido, como na u26 e na u29. Ela é reconstruída como `LegendaValida` ao lado do seu
chamador em `src/uenux2/src/app/vota/eleitor/votaproporcional/cpedeproporcional.cpp`, e usada em
`src/uenux2/wasm/vota_web/vota_web_wasm.u29.cpp`.

| func wasm | símbolo | src |
|---|---|---|
| 5922 | `vota::LegendaValida` (nome inferido) | `src/uenux2/src/app/vota/eleitor/votaproporcional/cpedeproporcional.cpp` |
