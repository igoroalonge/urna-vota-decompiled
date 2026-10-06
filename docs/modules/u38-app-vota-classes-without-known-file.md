# u38: `app:vota` sem arquivo conhecido (destrutores de saída dos singletons de estado e seis fontes de texto)

A unidade u38 tem **107 funções wasm** no componente `app:vota`. As ferramentas não conseguiram atribuir a elas um arquivo-fonte: nenhuma
delas referencia um registro `std::source_location`, nenhuma está em uma vtable e nenhuma tem chamador direto. Elas
estão apenas na tabela de funções. A leitura delas revela dois grupos:

| grupo | funções | o que são |
|---|---|---|
| **stubs de destrutor de saída** (chamados aqui de `__dtor_<variable>`; o símbolo do próprio clang é `__cxx_global_array_dtor[.N]`, §2.2) | 101 | Gerados pelo compilador. O clang emite um para cada objeto static com destrutor não trivial: 54 `std::mutex` static, 44 `std::unique_ptr<State>` static dos **singletons de estado** lazy das máquinas de estados do operador (mesário) e do eleitor, e 3 membros de dados static de `CControlaReconhecimento` (dois buffers de impressão digital e um título). **Código morto neste build**: nada os registra. |
| **fontes de texto** | 6 | Funções `std::string f()` que as telas e os relatórios chamam por meio de um ponteiro: 5 linhas do microterminal (10660, 10664, 10692, 10693, 10694) e o cabeçalho de coluna do relatório "Eleitores com habilitação biográfica" (11240). |

Nenhuma das 107 foi vista pelo profiler por amostragem durante os votos gravados. As seis fontes de texto pertencem à
thread do operador e aos relatórios do fim do dia, que a página web pública nunca executa. Os stubs nunca podem
executar. O ramo do CPF da func 10664 **foi** executado e observado com o harness do operador
(`tools/bu/operator_harness.mjs`, §5.2).

Arquivos-fonte reconstruídos (arquivos novos, fragmentos por arquivo original; cada seção nomeia seu caminho original):

```
src/uenux2/src/app/vota/u38-foreign-fragments.cpp            the atexit-stub pattern (read this first) +
                                                            iexecucaovota.cpp, monitor/cthreadmonitor.cpp
src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp   41 operator classes' statics (70 stubs), the
                                                            CControlaReconhecimento static buffers, funcs
                                                            10660/10664 (+ check of 10692-10694)
src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp    CConfereVotoEmCargo<> template statics, 18 voter
                                                            classes/instances' statics (29 stubs), func 11240 (BEHB header)
```

As funcs 10692/10693/10694 já tinham sido escritas pela unidade u27 em
`src/uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp` (linhas 64-66). A u38 as verificou byte a byte
e não as duplicou.

**Glossário**: *Terminal do mesário* / MT é o microterminal de 4 x 40 caracteres do mesário. *Título (de
eleitor)* é o número de inscrição do eleitor, com 12 dígitos, e *CPF* o número de contribuinte, com 11 dígitos. Ambos podem identificar
um eleitor. *Número livre* é um identificador livre. *Habilitação* é a liberação do eleitor para a cabine. Ela pode ser
*biométrica* (correspondência de impressão digital) ou *biográfica* (identificação por dados pessoais depois de uma digital que falhou ou
está ausente). *Justificativa* é o formulário que um eleitor preenche para justificar não ter votado na sua própria seção.
*Trânsito* é votar fora do domicílio, mediante pedido antecipado. *Encerramento* é o fechamento da votação no fim do
dia. *Zerésima* é o relatório zerado impresso antes da votação. *BU (boletim de urna)* é o resultado de cada urna.

---

## 1. Onde este código fica no processo de votação

A aplicação VOTA executa duas máquinas de estados. Cada uma roda em sua própria thread na urna:

* a **thread do operador** (`vota::CThreadOperador`, o microterminal do mesário). Ela cobre a identificação do
  eleitor (título/CPF digitado, busca no cadastro, impedimentos, justificativa), a biometria (`CPedeDigital`,
  `CRegistraDigitalOperador`, `CControlaReconhecimento`), a liberação da cabine, a espera enquanto o eleitor vota
  (`CMostraEleitorVotando`, `CEleitorDemorando`, `CSuspensaoAutomaticaEleitor`), o menu "outras opções"
  (`CEscolheOpcao`) e o início do **encerramento** (`CIniciaFinalizacao` → … → `CConfirmaEncerramento`);
* a **thread do eleitor** (`vota::CThreadEleitor`, a tela da cabine). Ela cobre a inicialização (zerésima, teste de
  teclado `testeteclado::*`, `CGeraDadosDinamicos`), o voto em si (`CPedeProporcional`, `CPedeMajoritario`,
  `CPedeNominal`, os estados de confirmação `CConfereVotoEmCargo<…>`) e os menus auxiliares (`CVisualizarCandidatos`,
  `CMaisInformacoes`, …).

Todo estado é um `comum::CAppState` e um **singleton lazy**. Uma transição é escrita como
`m_proximoEstado = &CFoo::GetInst();`. A instância é criada no primeiro uso e vive até o processo terminar.
Esta unidade contém as metades de saída desses singletons: as funções que destruiriam o mutex e
a instância em `exit()`. O build web nunca chama `exit()` (Emscripten `EXIT_RUNTIME=0`), então elas nunca executam.

As seis fontes de texto são o único código funcional da unidade:

* a func 10664 é a linha 2 da tela do MT **"eleitor não encontrado"**: `"CPF não encontrado. Digite o Título."`
  quando um CPF foi digitado e o identificador principal da eleição é o título, senão `"NÃO CADASTRADO nesta urna"`;
* a func 10660 é a linha 2 de **"optou por votar em trânsito"** (eleitor que pediu para votar em trânsito, em uma urna
  que não aceita justificativas);
* as funcs 10692-10694 são as entradas do menu "outras opções" **"Exibir contadores"**, **"Registrar mesários"** e
  **"Encerrar votação"**;
* a func 11240 é o cabeçalho de coluna `"Sequencial                      Título"` do relatório de fim do dia
  **"Eleitores com habilitação biográfica"** (arquivo `behb.dat`, impresso antes do BU).

---

## 2. O padrão de singleton lazy e seus stubs de saída

### 2.1 Padrão do código-fonte

```cpp
// in cfoo.cpp
std::mutex             CFoo::s_mutex;        // @X       (24 bytes: musl pthread_mutex_t on wasm32)
std::unique_ptr<CFoo>  CFoo::s_instancia;    // @X + 24

CFoo& CFoo::GetInst()
{
    std::lock_guard trava(s_mutex);          // lock(): removed entirely (no pthreads)
    if (!s_instancia)
        s_instancia.reset(new CFoo());       // "old = s; s = novo; if (old) delete old;"
    return *s_instancia;                     // unlock(): only the empty func-150 residue remains
}
```

A maioria dos acessores está **inlinada no estado que faz a transição** (LTO). É por isso que as ferramentas encontraram a
construção de, por exemplo, `CPerguntaEleitorVotando` dentro de `CEleitorDemorando::ProcessInput` (func 10439).
Alguns acessores continuam fora de linha (por exemplo, a func 1901 = `CEleitorVotouNaoVotou::GetInst`). Vários deles são thunks
para corpos *mesclados* pelo wasm-opt que recebem os endereços e a vtable como parâmetros: func 764
(`CFinalizaOperador`, `CReinicioComparecimentoMesario`) e 6051 (`CPedeProporcional`).

### 2.2 O que o clang emite para cada static, e o que resta disso

Para todo static com destrutor não trivial, o clang emite, na unidade de tradução que o define (ou, para um
static de template, que o instancia):

```cpp
static void __dtor_s_mutex(void*)     { CFoo::s_mutex.~mutex(); }          // real symbol: __cxx_global_array_dtor[.N]
static void __dtor_s_instancia(void*) { CFoo::s_instancia.~unique_ptr(); } // real symbol: __cxx_global_array_dtor[.N]
// and in the TU's global initialiser:
__cxa_atexit(&__dtor_s_mutex, nullptr, &__dso_handle);
__cxa_atexit(&__dtor_s_instancia, nullptr, &__dso_handle);
```

O helper existe porque, em WebAssembly, os destrutores retornam `this` (docs/libraries/libcxx-core.md §1.1) **e**
uma chamada indireta por meio de uma assinatura incompatível gera trap: o `WebAssemblyCXXABI` do clang sobrescreve
`canCallMismatchedFunctionType()` para `false`, então `EmitDeclDestroy` não pode registrar `&T::~T` e constrói um helper
com `generateDestroyHelper`. Esse helper é um `void __cxx_global_array_dtor(void*)` interno (o LLVM renomeia as
cópias como `.1`, `.2`, … dentro de uma TU), registrado com um argumento **nulo**, e é por isso que todo stub ignora seu
parâmetro e usa o endereço da variável como constante. A assinatura wasm observada `(i32)->void` confirma esse
tipo de helper: os stubs `__dtor_<mangled var>` do clang (`createAtExitStub`) são `void()` e só são usados com
`-fno-use-cxa-atexit`. Os nomes `__dtor_<variable>` usados nesta unidade (e na u39) são, portanto, rótulos descritivos,
e não os símbolos do compilador. Neste build:

* `__cxa_atexit` não faz nada (`EXIT_RUNTIME=0`, docs/libraries/libc-and-emscripten-runtime.md). O LTO e o wasm-opt
  apagaram todos os registros, e os inicializadores globais ficaram vazios (§8);
* os stubs sobrevivem apenas como **entradas da tabela de funções**. A tabela não é compactada. Nenhum código pega o número
  do slot deles: todos os 101 slots foram buscados como `i32.const`, e as poucas ocorrências são números não relacionados, como
  endereços de strings, códigos de controle do OpenSSL e aritmética;
* o stub **ignora seu argumento** e usa o endereço da variável como constante. Foi assim que cada stub foi
  associado ao seu acessor: o mesmo endereço aparece no acessor (`if (X+24[0]) …; X+24[0] = new …;
  mutex_unlock(X)`), ao lado da vtable da classe que ele instancia.

Corpos:

| stub de | corpo | significado |
|---|---|---|
| `s_mutex` | `mutex_unlock(X)` = func 150 | `~mutex()` → `pthread_mutex_destroy`, um no-op sem pthreads. O que resta é a verificação `__THREW__` vazia `noexcept` que o ICF compartilha com o resíduo do unlock. |
| `s_instancia` | `thunk(a, X+24)` → corpo mesclado | `~unique_ptr()`: `p = s; s = nullptr; if (p) { p->~T(); free(p); }`. O `delete` foi desvirtualizado (as classes presumivelmente são `final`, ou a desvirtualização de programa inteiro estava ativada; o binário não permite dizer), então ele chama o destrutor concreto diretamente. O merge-similar-functions do wasm-opt então dobrou os stubs com o mesmo destrutor em um único corpo que recebe o endereço como 2º argumento (abaixo). |

| corpo mesclado de `~unique_ptr` | destrutor que ele chama | classes (na u38) |
|---|---|---|
| 349 | nenhum (o destrutor virtual tem corpo vazio: slot 0 da vtable = `icf_ret_this_vf0` 174; apenas `free`) | CFinalizaOperador, CIniciaFinalizacao, CRegistroMesarioEncerrado, CGeraDadosDinamicos, CMenuFiltrarCandidatosPorCargo, CReinicioComparecimentoMesario |
| 389 | ICF 244 (libera o `shared_ptr` em +12/+16, o formulário MT do estado) | 20 estados do operador/eleitor |
| 763 | ICF 448 (dois `shared_ptr`s: +12, +20) | CMostraEleitorVotando, CDadoEleitorNaoConfere, CDigitalReconhecida, CDigitalNaoReconhecidaPorTempo |
| 2903 | ICF 1284 (três `shared_ptr`s: +12, +20, +28) | CEleitorVotouNaoVotou, CEleitorDemorando, CDigitalNaoReconhecida |
| 1564 | ICF 785 (`shared_ptr` em +28) | testeteclado::CEsperaRetestar, CVerificaHorarioZeresima |
| 1959 | `IEleitorImpedidoVotar::~` (1257) | CEleitorOptouPorVotarEmTransito, CEleitorNaoEncontrado |
| 1286 | `IConfereVotoEmCargo::~` (1717) | 3 instâncias de `CConfereVotoEmCargo<…>` |
| (inline) | `testeteclado::CBase::~` (1559) / `CGeraZeresimaBase::~` (1720) | CRetomada, CPreZeresima / CRegerarZeresima, CGeraZeresima (11833, 11864, 11844, 11938: stubs de 38 bytes que não foram mesclados) |

### 2.3 Onde os statics são definidos (evidência a partir das variáveis de guarda)

* **Classes simples.** Os acessores **não têm teste de variável de guarda**. Um static local de função com destrutor
  não trivial precisa de um (`__cxa_guard_acquire` + `__cxa_atexit` na primeira passagem). Os pares mutex/unique_ptr são,
  portanto, **de escopo de namespace ou membros de dados static** do `.cpp` que define a classe. O binário não permite
  dizer qual dos dois. Os fragmentos os escrevem como membros de dados static `s_mutex` / `s_instancia`, o
  estilo que a u19 usou para `CThreadMonitor`. Várias reconstruções de outras unidades os mostram como statics locais de
  função: o comportamento é o mesmo, mas não foi isso que foi compilado.
* **`CConfereVotoEmCargo<PROXIMO, TELA>`** (o estado de confirmação do voto) é diferente. Seu mutex e sua
  instância são **membros de dados static de template**. Esses são linkonce, e cada um tem uma guarda de 4 bytes ao lado
  (mutex @M, guarda @M+24, unique_ptr @M+28, guarda @M+32). `__wasm_call_ctors` (func 14478) ainda define as 20
  guardas das 10 instanciações como 1: `if (!(guard & 1)) guard = 1;` é o que resta do registro
  `__cxa_atexit` protegido. É por isso que o unique_ptr dessas instâncias fica em mutex **+28**, e não em +24.
  Por serem instanciações implícitas, seus helpers de destruição são emitidos nas TUs que as usam, e não no header.
  A ordem das funções concorda: 11674/11675 ficam ao lado dos corpos `CConfereVotoEmCargo<CMajoritario*>::vf16` e de
  `CPedeMajoritario` (cpedemajoritario.cpp), 11708/11709 entre as funções de `CPedeProporcional`, 11722 ao lado de
  `CPedeNominal::vf15/vf16`.
* **`IExecucaoVota`, `impl::IInformacaoThreadOperador` e `testeteclado::impl::IGeradorTeclas`** têm apenas um mutex
  static. O `GetInst()` deles (atestado por srcloc: iexecucaovota.cpp:74, cinformacaothreadoperador.cpp:554,
  ctesteteclado.cpp:121) o trava em torno de "inserir a implementação padrão no `CPolySingletonList` se nenhuma
  foi registrada, depois retornar `CPolySingleton<I>::instance()`". A instância pertence ao registro. Só resta o
  resíduo do unlock: `mutex_unlock(1911520)` no fim da func 3594, `mutex_unlock(1908504)` na func 599,
  `mutex_unlock(1837656)` na func 11805. O build web insere `CExecucaoVotaCooperativa` a partir de `main`, então a
  implementação padrão lazy de `IExecucaoVota` nunca é criada ali.

---

## 3. Classes e hierarquia (RTTI)

```
api::CState
└─ comum::CAppState                                  (vptr, +4 m_proximoEstado, +8/+9/+10 accepts msgs/keys/ticks)
   ├─ operator states (vota::, operador/…): CFinalizaOperador, CSuspensaoAutomaticaEleitor, CPerguntaEleitorVotando,
   │  CMostraEleitorVotando, CEleitorVotouNaoVotou, CEleitorDemorando, CVerificaDadoEleitor, CDadoEleitorNaoConfere,
   │  CTentativaCapturaDigitalEsgotada, CPedeDigital, CDigitalNaoReconhecida, CDigitalNaoReconhecidaDecBiometria,
   │  CDigitalReconhecida, CDigitalNaoReconhecidaPorTempo, CNomeEleitor, CInformaBioDesabilitadaDemo,
   │  CControlaReconhecimento, CHabilitaAudioEleitor, CConfirmaInspecionada, CAguardaInspecao, CValidaIdentidade,
   │  CEleitorJaVotou, CEscolheOpcao, CContadoresBiometria, CIniciaFinalizacao, CHabilitacaoAudioNaoPermitida,
   │  CEncerramentoHorarioInvalido, CPerguntaFilaEleitorVazia, CAguardaEleitoresVotarem, CPedeTituloEncerramento,
   │  CTituloEncerramentoInvalido, CEncerramentoAntecipado, CConfirmaEncerramento, CHorarioVotacaoTerminou,
   │  CRegistroMesarioEncerrado
   ├─ vota::IEleitorImpedidoVotar ─ CEleitorOptouPorVotarEmTransito, CEleitorNaoEncontrado (+4 siblings, u27)
   ├─ vota::IIniciaJustificativa   ─ CIniciaJustificativa (+2)
   ├─ vota::IConfirmaJustificativa ─ CConfirmaJustificativa (+2)
   ├─ voter states: CGeraDadosDinamicos, CVisualizarCandidatos, CMenuFiltrarCandidatosPorCargo,
   │  CMenuFiltrarCandidatosPorNumero, CMaisInformacoes, CReinicioComparecimentoMesario
   ├─ vota::CGeraZeresimaBase ─ CGeraZeresima, CRegerarZeresima
   ├─ vota::CEstadoComDesligamentoAutomatico ─ CVerificaHorarioZeresima, testeteclado::CTesteTeclado,
   │                                          testeteclado::CEsperaRetestar,
   │                                          testeteclado::CBase ─ testeteclado::CRetomada, testeteclado::CPreZeresima
   └─ vota::CVotacaoStateAudio ─ CPedeProporcional (+ CPedeMajoritario, CPedeNominal…)
                               └─ vota::IConfereVotoEmCargo ─ CConfereVotoEmCargo<PROXIMO, (ETelaVotacao)N> (10 instances)
api::CThread ─ vota::CThreadVota ─ CThreadOperador, CThreadMonitor
vota::IExecucaoVota (interface; default CExecucaoVota, web: CExecucaoVotaCooperativa)
vota::impl::IInformacaoThreadOperador (interface; CInformacaoThreadOperador)
vota::testeteclado::impl::IGeradorTeclas (interface; CGeradorTeclasAleatorio)
```

Objetos static destruídos por esta unidade, com seus endereços (todos em `.bss`). Dentro de um par, o mutex vem primeiro
(@X, @X+24), e entre unidades de tradução os endereços diminuem à medida que os índices de função dos stubs aumentam.
Em geral, o layout não pode ser amarrado à ordem do código-fonte: em `CControlaReconhecimento`, os stubs vêm na ordem
s_mutex, s_instancia, s_tituloMesario, s_digitalMesario, s_digitalCapturada (10514-10518), enquanto os endereços
crescem s_digitalCapturada < s_digitalMesario < s_tituloMesario < s_mutex < s_instancia.

* cada classe de estado do operador/eleitor: `s_mutex` @X e `s_instancia` @X+24 (por classe em §12);
* `CConfereVotoEmCargo<CMajoritarioValido, 2>` @1838452/@1838480, `<CProporcionalBranco, 4>` @1838112/@1838140,
  `<CConfirmaVotoLegenda, 10>` @1837976/@1838004;
* membros de dados static de `CControlaReconhecimento`: `std::vector<uebyte> s_digitalCapturada` @1908672 (a última
  captura de impressão digital do eleitor, WSQ), `std::vector<uebyte> s_digitalMesario` @1908684 (a captura do mesário) e
  `std::string s_tituloMesario` @1908696 (título do mesário que liberou o eleitor). Esses são os únicos
  statics da unidade que não são singletons. Seus vizinhos `s_indiceDedo` @1908668, `s_score` @1908708 e
  `s_qtdEleitores` @1908712 são trivialmente destrutíveis e não têm stub.

---

## 4. Fluxo de controle: onde os singletons são usados

Os stubs não fazem nada em tempo de execução. A informação útil que eles carregam é **qual estado vive em qual static,
e qual função o cria**. A tabela abaixo lista, por fluxo, as transições que instanciam os statics da u38
(acessor = a função que contém a criação lazy). Os detalhes de cada estado estão nas unidades citadas.

**Thread do operador** (u10, u17, u27, u33):

| fluxo | transição (acessor) | statics (stubs) |
|---|---|---|
| objetos de thread | `CThreadOperador::GetInst` (270), `CThreadMonitor::GetInst` (1898), `IExecucaoVota::GetInst` (3594) | 10207, 10228, 10237 |
| identificação | `CPedeIdentidade::ProcessInput` (10677) → `CValidaIdentidade`; `CPedeIdentidade::ProcessTick` (10676) → `CAguardaInspecao` (inspeção a cada 60-90 min) → `CAguardaInspecao::ProcessMessage` (10530) → `CConfirmaInspecionada` | 10629; 10531/10532; 10528/10529 |
| busca no cadastro | `CProcuraEleitor::StartState` (10631) → `CIniciaJustificativa` + `CConfirmaJustificativa`, ou `CEleitorNaoEncontrado` (5424) | 10616, 10621, 10665/10666 |
| impedimentos | `CEleitorEncontrado::StartState` (10635) → `CEleitorJaVotou`, `CEleitorOptouPorVotarEmTransito` | 10644/10645, 10661/10662 |
| biometria | `CNomeEleitor` (5401), `CInformaBioDesabilitadaDemo` (5402), `CPedeDigital` (2740), `CPedeDigital::ObtemEstadoPosReconhecimentoBiometrico` (5397) → `CDigitalReconhecida` / `CDigitalNaoReconhecida`; `CDigitalNaoReconhecida::StartState` (10474) → `CDigitalNaoReconhecidaDecBiometria`; `CPedeDigital::ProcessTick` (10465) → `CDigitalNaoReconhecidaPorTempo`; `CRegistraDigitalOperador::ProcessTick` (10451) → `CTentativaCapturaDigitalEsgotada`; `CControlaReconhecimento` (1256) e seus buffers static | 10502/10503, 10509/10510, 10471, 10483/10484, 10475/10476, 10479/10480, 10487/10488, 10463, 10514-10518 |
| áudio | `CHabilitaAudioEleitor` (2743) | 10525/10526 |
| eleitor na cabine | `CMostraEleitorVotando` (1150), `CEleitorVotouNaoVotou` (1901), `CEleitorDemorando` (2734) → `CEleitorDemorando::StartState` (10440) → `CSuspensaoAutomaticaEleitor`, `::ProcessInput` (10439) → `CPerguntaEleitorVotando`; `CVerificaDadoEleitor` (2735) → `::ProcessInput` (10443) → `CDadoEleitorNaoConfere` | 10428/10429, 10433/10434, 10441/10442, 10412, 10417/10418, 10445/10446, 10449/10450 |
| outras opções | `CEscolheOpcao` (2753) → `::ProcessInput` (10689) → `CContadoresBiometria`, `CIniciaFinalizacao`, `CHabilitacaoAudioNaoPermitida`; `CHorarioVotacaoTerminou` (5430) | 10696, 10704/10705, 10707/10708, 10755/10756, 10760/10761 |
| encerramento | `CIniciaFinalizacao::StartState` (10706) → `CEncerramentoHorarioInvalido`; `CPerguntaFilaEleitorVazia` (5426) → `::ProcessInput` (10713) → `CAguardaEleitoresVotarem`; `CPedeTituloEncerramento` (3631) → `::StartState` (10722) → `CEncerramentoAntecipado`, `::ProcessInput` (10721) → `CTituloEncerramentoInvalido`; `CConfirmaEncerramento` (3632) | 10711/10712, 10716, 10719/10720, 10724/10725, 10733, 10729, 10735 |
| registro de mesários e fim | `CControladorRegistraMesariosVota::GetEstadoAposRegistroVotacao` (vf11, 10793) → `CRegistroMesarioEncerrado`; `GetEstadoAposRegistroFinal` (vf12) → `CFinalizaOperador` (5342) | 10765, 10216 |
| informação compartilhada | `impl::IInformacaoThreadOperador::GetInst` (599) | 10553 |

**Thread do eleitor** (u06, u07, u09, u26, u30):

| fluxo | transição (acessor) | statics (stubs) |
|---|---|---|
| inicialização / teste de teclado | `CAjusteInicial::ValidaTemposDesligamento` (7160) → `testeteclado::CRetomada`; `CVerificaEleicaoPassou::StartState` (11914) → `testeteclado::CPreZeresima` → `::GetEstadoPassouNoTeste` (5945) → `CGeraDadosDinamicos`; `CTesteTeclado` (3855) e, dentro do seu StartState (11805), `IGeradorTeclas::GetInst` (sequência aleatória de teclas); `CEsperaRetestar` (5936); `CReinicioComparecimentoMesario` (3864) | 11832/11833, 11863/11864, 11866/11867, 11807, 11802, 11827/11828, 11918/11919 |
| zerésima | `CVerificaHorarioZeresima` (5947), `CGeraZeresima` (5962), `CConfirmaRegerarZeresima::ProcessInput` (11838) → `CRegerarZeresima` | 11872/11873, 11938, 11844 |
| menus de consulta de candidatos | `CVisualizarCandidatos` (1279), `CMenuVisualizarCandidatos::StartState` (11881) → `CMenuFiltrarCandidatosPorCargo`, `CMenuFiltrarCandidatosPorNumero` (5954), `CMaisInformacoes` (1280) | 11878, 11889/11890, 11893/11894, 11898/11899 |
| o voto | `CPedeProporcional` (3849); `CPedeMajoritario::ProcessInputAudio` (11683) → `CConfereVotoEmCargo<CMajoritarioValido,2>` (voto majoritário válido); `CPedeProporcional::ProcessInputAudio` (11711) → `<CProporcionalBranco,4>` (voto proporcional em branco); `CPedeNominal::GetProximoEstado` (slot 16, 11724) → `<CConfirmaVotoLegenda,10>` (voto de legenda) | 11713, 11674/11675, 11708/11709, 11722 |

Nos votos gravados (profiler por amostragem), os acessores 3594, 270, 11683, 11711 e 11724 foram amostrados. Um
acessor que executou não necessariamente seguiu o ramo que cria um singleton da u38. O que as transcrições mostram:
`CThreadOperador` (270) existe; `CConfereVotoEmCargo<CMajoritarioValido,2>` (Governador/Presidente válido) e
`<CConfirmaVotoLegenda,10>` (o voto de legenda para Deputado Federal) foram criados em `vote_geral_t1`;
`<CProporcionalBranco,4>` **não** foi criado em nenhuma das sessões gravadas (nenhum voto proporcional em branco). Uma
execução headless com um Vereador em branco (`--scenario municipal-t1 --keys "B  C  12C  C  "`) o mostra
(`substate: CConfereVotoEmCargo<vota::CProporcionalBranco, (vota::ETelaVotacao)4>`). `IExecucaoVota::GetInst`
(3594) executa, mas sua implementação padrão lazy nunca é criada (§7). `CPedeProporcional` (3849) é o primeiro estado de votação
de toda transcrição, embora o profiler não tenha amostrado seu pequeno acessor. Os destrutores deles (estes stubs)
nunca executam.

---

## 5. As fontes de texto

Todas as seis são chamadas por meio de um ponteiro, nunca diretamente:

* 10660, 10664 e 10692-10694 são armazenadas como o callable de uma `std::function<std::string()>` cujo tipo-alvo é
  `std::string (*)()` (vtable @1542376 `__func<std::string(*)(), std::string()>`). Uma lambda teria sua própria
  vtable `__func<lambda>`, então estas são **funções nomeadas**, muito provavelmente em um namespace anônimo;
* 11240 é armazenada em um `api::CDataText<std::string (*)()>` (a func 604 o embrulha em um `CTextFieldPaper`).

### 5.1 Func 10660 (`TextoOptouTransito`, slot 3876)

Retorna `"Optou por votar em trânsito"`. O construtor de `CEleitorOptouPorVotarEmTransito`, inlinado em
`CEleitorEncontrado::StartState` (10635), a usa como linha 2, com `TextoVazio` (" ", func 3628) como linha 3 e
`CONFIRMA: retornar`. Ela é alcançada quando o impedimento do eleitor é 2 (pediu para votar em trânsito) e a urna não
aceita justificativas (u27 §4.4).

### 5.2 Func 10664 (`TextoNaoEncontrado`, slot 3867)

```cpp
const auto digitado = impl::IInformacaoThreadOperador::GetInst().GetTipoIdentidadeDigitada();  // func 5416 (slot 24)
const auto& cfg     = comum::CConfiguracaoEleicao::GetInst();                                // func 187
if (digitado == CPF && cfg.GetTipoIdentificadorPrincipal() == TITULO)                       // +668
    return "CPF não encontrado. Digite o Título.";
return "NÃO CADASTRADO nesta urna";
```

Linha 2 de `CEleitorNaoEncontrado` (construtor na func 5424). Verificado com o harness do operador:

```
node tools/bu/operator_harness.mjs --flow full --out <dir> \
     --script "geradin zeresima ot:20 om:8 ot:50 o:XXXXXXXXXXXC ot:50 o:C ot:50 o:XXXXXXXXXXXXC ot:50"
```

* um **CPF** desconhecido (XXX.XXX.XXX-XX): `CPedeIdentidade → CProcuraEleitor → CEleitorNaoEncontrado`, o MT mostra
  `CPF: XXX.XXX.XXX-XX / CPF não encontrado. Digite o Título. / CONFIRMA: retornar`;
* um **título** desconhecido (XXXX XXXX XXXX): `CProcuraEleitor → CIniciaJustificativa → CConfirmaJustificativa`
  (um eleitor de outra seção pode justificar). Reproduzido por esta revisão (mesmo comando, mesmas linhas do MT e mesmos
  estados). O roteamento está em `CProcuraEleitor::StartState` (10631): um identificador não encontrado vai para
  `CEleitorNaoEncontrado` quando a justificativa não é aceita (func 4578) **ou** quando o tipo digitado não é o
  título (`!= 1`). Assim, `"NÃO CADASTRADO nesta urna"` aparece para um título quando a justificativa não é aceita, para um
  CPF quando o identificador principal da eleição não é o título, e para qualquer outro tipo digitado (número livre).

### 5.3 Funcs 10692 / 10693 / 10694 (menu "outras opções")

`"Exibir contadores"`, `"Registrar mesários"`, `"Encerrar votação"`. Estas são as fontes de texto das entradas `COpcaoDS`
4 (ou 3), 3 e 2 de `CEscolheOpcao` (func 2753; u27 §4.5). Os mesmos literais também são usados por
`CEscolheOpcao::ProcessInput` (10689) na linha de log `"Operador selecionou: {}"`.

### 5.4 Func 11240 (`CabecalhoSequencialIdentificador`, slot 2961): cabeçalho do relatório BEHB

```cpp
constexpr std::size_t LARGURA_LINHA = 38;
std::string AlinhaEmColunas(const std::string& esq, const std::string& dir)   // inlined
{   const auto n = esq.size() + dir.size();
    const std::string espacos(n < LARGURA_LINHA ? LARGURA_LINHA - n : 0, ' ');
    return esq + espacos + dir; }   // (const&, const&) concat inlined, then append (func 160)

std::string CabecalhoSequencialIdentificador()
{   return AlinhaEmColunas("Sequencial",
        NomeTipoIdentificador(CConfiguracaoEleicao::GetInst().GetTipoIdentificadorPrincipal())); }
// NomeTipoIdentificador: 1 "Título", 2 "CPF", 3 "Número livre", anything else "" (inlined switch).
// Not the same table as vota::TipoToStr (func 10586), which says "Identificador" for 3 and throws 9409 otherwise.
```

A string de preenchimento é um lvalue nomeado: o primeiro `+` é compilado como o `operator+(const string&, const
string&)` da libc++ (buffer novo, dois `memcpy`s inlinados). Um `std::string(n, ' ')` temporário selecionaria
`operator+(const string&, string&&)`, isto é, `rhs.insert(0, lhs)`, e não existe nenhuma chamada a `insert` na 11240.

Ela é usada uma única vez, por `CGeraRelatorios::StartState` (func 12105, `cgerarelatorios.cpp:51`). Essa função imprime,
no fim do dia e antes do BU, o relatório **"Eleitores com habilitação biográfica"** (`behb.dat`,
gravado em `CPath::GetPathTrab(INTERNA)` e espelhado por `CSincronizaVota::SincronizaRelatorios`). O relatório só é
impresso quando a urna é biométrica e não está em modo demonstração. Para cada seção com eleitores liberados biograficamente,
ele imprime este cabeçalho e depois uma linha por eleitor, construída com o **mesmo código de preenchimento inlinado**:
`AlinhaEmColunas(std::format("{:04}", sequencial), identidade)`. `identidade` é
`CEleitorDecorator::GetIdentidadePorTipo(eleitor, <voter's principal type, +104>)`. A largura 38 é a
linha da impressora na fonte normal. Ela corresponde a `CompletaDireita(rótulo, 28)` + uma data de 10 caracteres, e aos
separadores `"======"` de 38 caracteres no `samples/bu-real/run-full/reports/behb.txt` gravado. Nessa execução nenhum
eleitor foi liberado biograficamente ("Nenhum eleitor passou por habilitação biográfica"), então o cabeçalho em si
não foi impresso.

---

## 6. Dados lidos e gravados

* **Lidos** (apenas pelas fontes de texto): `comum::CConfiguracaoEleicao +668` = `tipoIdentificadorPrincipalEleitor` do
  ASN.1 `ModuloProcessoEleitoral` (`TipoIdentificadorEleitor`: 1 numeroInscricaoEleitoral, 2 numeroCPF,
  3 numeroLivre), e `IInformacaoThreadOperador::GetTipoIdentidadeDigitada()` (o tipo de número que o mesário
  digitou).
* **Gravados**: nada. Os stubs apenas liberariam memória. Não há acesso a arquivo, SQL ou ASN.1 nesta unidade.
* Memória static que os stubs liberam: os buffers de impressão digital e o título de `CControlaReconhecimento` (§3).
  No build web eles ficam vazios, porque o codificador WSQ é um stub (cregistradigitaloperador.cpp).

---

## 7. Particularidades do build web

* Emscripten `EXIT_RUNTIME=0`: `atexit`/`__cxa_atexit` são no-ops e a página nunca chama `exit()`. **Nenhum dos
  101 destrutores pode executar**, e nenhum singleton é destruído em momento algum. No build web nada se perde com isso: esses
  destrutores só liberam memória.
* Sem pthreads: o `std::lock_guard` de todo `GetInst` compila para nada (lock) mais o resíduo vazio `noexcept`
  (unlock, func 150). Esse mesmo corpo vazio é o que os stubs de `s_mutex` contêm.
* A thread do operador nunca executa no simulador público (u27 §2): os acessores do lado do operador e as fontes de
  texto 10660/10664/10692-10694 são inalcançáveis a partir da página. Elas foram exercitadas apenas por meio do harness
  modificado `tools/bu/operator_harness.mjs`.
* O ponto de entrada web insere `CExecucaoVotaCooperativa` antes que `IExecucaoVota::GetInst` (3594) execute, então o
  `CExecucaoVota` padrão protegido pelo mutex do stub 10237 nunca é criado. O resíduo do mutex ainda executa a cada
  chamada.

---

## 8. Observações sobre wasm / Emscripten

1. **Stubs de saída de um programa que nunca termina.** 101 funções (1.286 bytes de código) e 101 slots de tabela são peso morto.
   O wasm-opt não pode removê-los, porque a tabela é exportada (`__indirect_function_table`, export `Fb`, usada pelos
   trampolins `invoke_*` da cola por meio de `getWasmTableEntry`) e, portanto, não é considerada fechada.
2. **Os helpers de destruição só existem por causa da ABI C++ do WebAssembly**: os destrutores retornam `this` e
   `WebAssemblyCXXABI::canCallMismatchedFunctionType()` é `false`. No ARM, onde os destrutores também retornam `this`,
   esse hook mantém o padrão `true`, então o clang registra `&T::~T` diretamente, como faz no x86-64. Os helpers são
   `__cxx_global_array_dtor[.N]` (`void(void*)`, argumento nulo), e não os stubs `__dtor_` `void()` do clang (§2.2).
3. **merge-similar-functions nos stubs atexit.** Os stubs que diferem apenas pelo endereço da variável viraram thunks de 12 bytes
   `f(a) { body(a, @var) }` sobre 7 corpos compartilhados (349, 389, 763, 1286, 1564, 1959, 2903). Quatro stubs de 38 bytes
   mantiveram seu corpo inline (11833, 11844, 11864, 11938). Os stubs de mutex têm 10 bytes: `call 150` com o endereço.
4. **`delete` desvirtualizado.** Todo `~unique_ptr<State>` chama o destrutor concreto diretamente
   (por exemplo, ICF 1284) em vez do destrutor deleting virtual (slot 1 da vtable). Os próprios destrutores são
   mesclados por ICF entre estados com o mesmo layout de membros (o 244 é compartilhado por 49 classes).
5. **Variáveis de guarda de statics de template.** O único vestígio dos registros atexit das 10 instanciações de
   `CConfereVotoEmCargo<…>` são 20 `if (!(g & 1)) g = 1;` em `__wasm_call_ctors`. Os statics simples não deixaram nada ali.
   Isso dá uma forma de distinguir statics de template/inline de statics de TU neste binário.
6. **A ordem da tabela segue as unidades de tradução.** Os dois stubs de um par são adjacentes tanto no índice de função quanto no
   slot da tabela (por exemplo, 10433/10434 nos slots 4216/4215; os slots diminuem à medida que os índices aumentam), e os stubs de uma
   unidade de tradução são consecutivos, ao lado das funções dessa TU. Não é possível mostrar se essa é a ordem de definição no
   código-fonte (ver os membros de `CControlaReconhecimento` em §3).
7. As strings estão em Latin-1 no binário (`"Título"` tem 6 bytes, `"Optou por votar em trânsito"` 27). Os literais
   longos são construídos inline (loads de 8 bytes do segmento de dados para um buffer novo de `operator new`), e não
   por meio de `std::string(const char*)`. Uma exceção: na 11240, `"Número livre"` passa por
   `string::assign(const char*)` (func 276).

---

## 9. Boletim de urna (BU) e encerramento

Esta unidade não gera, não assina nem imprime o BU. O que ela toca:

* os **destrutores dos estados de encerramento** da thread do operador. São eles `CIniciaFinalizacao` (10707/10708),
  `CEncerramentoHorarioInvalido` (10711/10712), `CPerguntaFilaEleitorVazia` (10716),
  `CAguardaEleitoresVotarem` (10719/10720), `CPedeTituloEncerramento` (10724/10725),
  `CTituloEncerramentoInvalido` (10729), `CEncerramentoAntecipado` (10733), `CConfirmaEncerramento` (10735),
  `CRegistroMesarioEncerrado` (10765) e `CFinalizaOperador` (10216). O fluxo em si (opção 2 do menu → verificação
  de horário → "Todas as pessoas presentes já votaram?" → título do presidente → confirmação → `estadoVota = 57` →
  mensagem 7 → `CGeraBU`) está documentado em u27 §4.6, u17 e `docs/bu/codepath.md` §2.3;
* o texto **"Encerrar votação"** dessa opção do menu (10694);
* o cabeçalho do relatório BEHB (11240). `CGeraRelatorios::StartState` imprime o BEHB depois dos relatórios BUJ
  (justificativas) e BIM (mesários) e antes de passar para `CInicioBU` (u08 §4.1). Formato, em
  linhas de 38 colunas: `"Sequencial"` + espaços + o rótulo do identificador principal (`"Título"` / `"CPF"` /
  `"Número livre"`), depois uma linha por eleitor: número sequencial de 4 dígitos, espaços, identificador.

---

## 10. Código estranho ou arriscado

1. **Dados biométricos mantidos em armazenamento static e nunca apagados** (10516-10518). `CControlaReconhecimento::s_digitalCapturada`
   (a imagem WSQ da impressão digital do eleitor), `s_digitalMesario` (a do mesário) e `s_tituloMesario` são statics que duram
   todo o processo. `CControlaReconhecimento::StartState` (10512) apenas faz `clear()` nos dois vetores (`end = begin`):
   os bytes permanecem na capacidade alocada até que a próxima captura os sobrescreva. Uma nova captura é armazenada com
   `vector::assign` (func 1681, chamada a partir da 10465 e da 10451). Quando ela é maior que a capacidade, o assign libera o
   buffer antigo (func 5493) sem apagá-lo, e uma menor deixa o final da imagem anterior no lugar.
   A única outra liberação é o destrutor de saída, e ele também libera a memória sem zerá-la. Em uma urna real compilada a partir do mesmo código, as
   imagens de impressão digital mais recentes, portanto, permanecem na RAM entre eleitores e no heap liberado após a saída. No simulador,
   os buffers estão vazios (stub de WSQ) e os destrutores nunca executam.
2. **Singletons nunca destruídos / ordem de desligamento.** Neste build, nenhum singleton de estado ou de thread é destruído
   (§7). Em uma urna nativa, esses 101 destrutores executariam em `exit()` na ordem inversa de registro: ordem inversa de
   definição dentro de uma unidade de tradução, e, entre unidades de tradução, uma ordem que segue a ordem de link
   dos seus inicializadores, que a linguagem deixa não especificada. `GetInst()` entrega referências cruas (`return *s_instancia;`). Uma thread ainda em execução durante `exit()`,
   ou um destrutor de um estado que toque outro estado, usaria um objeto destruído. Isso não pode ser verificado
   a partir deste build.
3. **Stubs que liberariam singletons vivos.** Os 101 stubs continuam chamáveis por meio da tabela de funções exportada.
   Qualquer código ou JS que chame um slot da tabela pelo número, por exemplo um `dynCall` com índice errado, poderia executar
   `s_instancia.reset()` em um estado vivo (um use-after-free na próxima referência de `GetInst()`). Nenhuma chamada desse tipo existe
   na cola nem no wasm.
4. **Rótulo do cabeçalho do BEHB vs. conteúdo das linhas** (11240). O rótulo do cabeçalho vem do identificador principal
   de toda a eleição (config +668). Cada linha imprime a identidade do eleitor pelo tipo principal do próprio eleitor (+104). Espera-se
   que eles concordem. Se diferirem, os números são impressos sob o rótulo errado (por exemplo, CPFs sob
   "Título"). Um tipo desconhecido dá um rótulo vazio silenciosamente, enquanto `vota::TipoToStr` lança 9409.
5. **Seleção da mensagem na func 10664.** `"CPF não encontrado. Digite o Título."` aparece apenas quando o identificador
   principal da eleição é o título. Em uma eleição cujo identificador principal é o CPF, um CPF que não é
   encontrado recebe o genérico `"NÃO CADASTRADO nesta urna"`, sem nenhuma dica para tentar o outro identificador. À parte disso,
   um título que não está no cadastro desta seção vai para o fluxo de justificativa quando as justificativas são
   aceitas, e não para esta tela (verificado com o harness). Ambos são coerentes com a intenção do código. Apenas
   o primeiro pode confundir o mesário.

---

## 11. Questões em aberto

* Membro de dados static (`CFoo::s_mutex`) ou variável de escopo de namespace no `.cpp` da classe: o binário não permite dizer.
  Ele só prova que não são statics locais de função (§2.3).
* O arquivo original da func 11240. Ela é referenciada apenas por `CGeraRelatorios::StartState`, então
  `cgerarelatorios.cpp` é o lar natural. Com LTO, ela também poderia ser um helper de `comum/relatorios` (o
  `AlinhaEmColunas` inlinado pode ser um helper nomeado de `CRelUtil`).
* Caminhos inferidos: `CAguardaInspecao` / `CConfirmaInspecionada` foram colocados em `operador/leidentidade/`, ao lado de
  `CPedeIdentidade`, seu criador. Eles poderiam ficar em `operador/aguardaeleitor/`.
  `CEleitorOptouPorVotarEmTransito`/`CEleitorNaoEncontrado` seguem o único `ieleitorimpedidovotar.cpp` da u27.
* Por que o merge-similar-functions deixou os quatro stubs `~unique_ptr` de 38 bytes (11833/11864, 11844/11938) como dois
  pares de formato idêntico em vez de thunks.

---

## 12. Tabela de mapeamento (todas as 107 funções da u38)

Colunas: *static* = endereço do objeto destruído (`s_instancia` = mutex + 24, ou + 28 para os statics de template de
`CConfereVotoEmCargo`); *corpo* = o que o stub chama; *arquivo original* = a unidade de tradução
que define o static (onde o clang emite o stub; para os statics de template de `CConfereVotoEmCargo`, a TU que
os instancia); *acessor / usuário* = a função que cria o objeto ou usa a fonte de texto. Os símbolos
dos stubs seguem a convenção descritiva do projeto `__dtor_<variable>` (compartilhada com a u39). O símbolo real do clang para
cada um deles é um `__cxx_global_array_dtor[.N]` interno (§2.2).

| # | func | tamanho | slot | símbolo reconstruído | o que faz | static | corpo | arquivo original | reconstruído em | acessor / usuário |
|---|---|---|---|---|---|---|---|---|---|---|
| 1 | 10207 | 10 | 4593 | `__dtor_vota::CThreadOperador::s_mutex` | ~mutex de s_mutex | @1911684 | func 150 (~mutex vazio) | uenux2/src/app/vota/operador/cthreadoperador.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CThreadOperador::GetInst (func 270) |
| 2 | 10216 | 12 | 4581 | `__dtor_vota::CFinalizaOperador::s_instancia` | ~unique_ptr de s_instancia | @1911652 | thunk→349 (apenas free) | uenux2/src/app/vota/operador/cfinalizaoperador.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CFinalizaOperador::GetInst (func 5342 -> corpo mesclado 764) |
| 3 | 10228 | 10 | 4565 | `__dtor_vota::CThreadMonitor::s_mutex` | ~mutex de s_mutex | @1911572 | func 150 (~mutex vazio) | uenux2/src/app/vota/monitor/cthreadmonitor.cpp | src/uenux2/src/app/vota/u38-foreign-fragments.cpp | CThreadMonitor::GetInst (func 1898) |
| 4 | 10237 | 10 | 4549 | `__dtor_vota::IExecucaoVota::s_mutex` | ~mutex de s_mutex | @1911520 | func 150 (~mutex vazio) | uenux2/src/app/vota/iexecucaovota.cpp | src/uenux2/src/app/vota/u38-foreign-fragments.cpp | IExecucaoVota::GetInst (func 3594, srcloc iexecucaovota.cpp:74) |
| 5 | 10412 | 10 | 4244 | `__dtor_vota::CSuspensaoAutomaticaEleitor::s_mutex` | ~mutex de s_mutex | @1909376 | func 150 (~mutex vazio) | uenux2/src/app/vota/operador/aguardaeleitor/csuspensaoautomaticaeleitor.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlinado em CEleitorDemorando::StartState (func 10440) |
| 6 | 10417 | 10 | 4236 | `__dtor_vota::CPerguntaEleitorVotando::s_mutex` | ~mutex de s_mutex | @1909348 | func 150 (~mutex vazio) | uenux2/src/app/vota/operador/aguardaeleitor/cperguntaeleitorvotando.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlinado em CEleitorDemorando::ProcessInput (func 10439) |
| 7 | 10418 | 12 | 4235 | `__dtor_vota::CPerguntaEleitorVotando::s_instancia` | ~unique_ptr de s_instancia | @1909372 | thunk→389 (~ICF 244 + free) | uenux2/src/app/vota/operador/aguardaeleitor/cperguntaeleitorvotando.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlinado em CEleitorDemorando::ProcessInput (func 10439) |
| 8 | 10428 | 10 | 4222 | `__dtor_vota::CMostraEleitorVotando::s_mutex` | ~mutex de s_mutex | @1909292 | func 150 (~mutex vazio) | uenux2/src/app/vota/operador/aguardaeleitor/cmostraeleitorvotando.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CMostraEleitorVotando::GetInst (func 1150) |
| 9 | 10429 | 12 | 4221 | `__dtor_vota::CMostraEleitorVotando::s_instancia` | ~unique_ptr de s_instancia | @1909316 | thunk→763 (~ICF 448 + free) | uenux2/src/app/vota/operador/aguardaeleitor/cmostraeleitorvotando.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CMostraEleitorVotando::GetInst (func 1150) |
| 10 | 10433 | 10 | 4216 | `__dtor_vota::CEleitorVotouNaoVotou::s_mutex` | ~mutex de s_mutex | @1909264 | func 150 (~mutex vazio) | uenux2/src/app/vota/operador/aguardaeleitor/celeitorvotounaovotou.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CEleitorVotouNaoVotou::GetInst (func 1901) |
| 11 | 10434 | 12 | 4215 | `__dtor_vota::CEleitorVotouNaoVotou::s_instancia` | ~unique_ptr de s_instancia | @1909288 | thunk→2903 (~ICF 1284 + free) | uenux2/src/app/vota/operador/aguardaeleitor/celeitorvotounaovotou.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CEleitorVotouNaoVotou::GetInst (func 1901) |
| 12 | 10441 | 10 | 4203 | `__dtor_vota::CEleitorDemorando::s_mutex` | ~mutex de s_mutex | @1909208 | func 150 (~mutex vazio) | uenux2/src/app/vota/operador/aguardaeleitor/celeitordemorando.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CEleitorDemorando::GetInst (func 2734) |
| 13 | 10442 | 12 | 4202 | `__dtor_vota::CEleitorDemorando::s_instancia` | ~unique_ptr de s_instancia | @1909232 | thunk→2903 (~ICF 1284 + free) | uenux2/src/app/vota/operador/aguardaeleitor/celeitordemorando.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CEleitorDemorando::GetInst (func 2734) |
| 14 | 10445 | 10 | 4197 | `__dtor_vota::CVerificaDadoEleitor::s_mutex` | ~mutex de s_mutex | @1909180 | func 150 (~mutex vazio) | uenux2/src/app/vota/operador/confirmaidentidade/cverificadadoeleitor.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CVerificaDadoEleitor::GetInst (func 2735) |
| 15 | 10446 | 12 | 4196 | `__dtor_vota::CVerificaDadoEleitor::s_instancia` | ~unique_ptr de s_instancia | @1909204 | thunk→389 (~ICF 244 + free) | uenux2/src/app/vota/operador/confirmaidentidade/cverificadadoeleitor.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CVerificaDadoEleitor::GetInst (func 2735) |
| 16 | 10449 | 10 | 4191 | `__dtor_vota::CDadoEleitorNaoConfere::s_mutex` | ~mutex de s_mutex | @1909152 | func 150 (~mutex vazio) | uenux2/src/app/vota/operador/confirmaidentidade/cdadoeleitornaoconfere.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlinado em CVerificaDadoEleitor::ProcessInput (func 10443) |
| 17 | 10450 | 12 | 4190 | `__dtor_vota::CDadoEleitorNaoConfere::s_instancia` | ~unique_ptr de s_instancia | @1909176 | thunk→763 (~ICF 448 + free) | uenux2/src/app/vota/operador/confirmaidentidade/cdadoeleitornaoconfere.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlinado em CVerificaDadoEleitor::ProcessInput (func 10443) |
| 18 | 10463 | 12 | 4170 | `__dtor_vota::CTentativaCapturaDigitalEsgotada::s_instancia` | ~unique_ptr de s_instancia | @1909092 | thunk→389 (~ICF 244 + free) | uenux2/src/app/vota/operador/confirmaidentidade/ctentativacapturadigitalesgotada.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlinado em CRegistraDigitalOperador::ProcessTick (func 10451) |
| 19 | 10471 | 10 | 4163 | `__dtor_vota::CPedeDigital::s_mutex` | ~mutex de s_mutex | @1909040 | func 150 (~mutex vazio) | uenux2/src/app/vota/operador/confirmaidentidade/cpededigital.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CPedeDigital::GetInst (func 2740) |
| 20 | 10475 | 10 | 4157 | `__dtor_vota::CDigitalNaoReconhecida::s_mutex` | ~mutex de s_mutex | @1909012 | func 150 (~mutex vazio) | uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaoreconhecida.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlinado em CPedeDigital::ObtemEstadoPosReconhecimentoBiometrico (func 5397) |
| 21 | 10476 | 12 | 4156 | `__dtor_vota::CDigitalNaoReconhecida::s_instancia` | ~unique_ptr de s_instancia | @1909036 | thunk→2903 (~ICF 1284 + free) | uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaoreconhecida.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlinado em CPedeDigital::ObtemEstadoPosReconhecimentoBiometrico (func 5397) |
| 22 | 10479 | 10 | 4151 | `__dtor_vota::CDigitalNaoReconhecidaDecBiometria::s_mutex` | ~mutex de s_mutex | @1908984 | func 150 (~mutex vazio) | uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaoreconhecidadecbiometria.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlinado em CDigitalNaoReconhecida::StartState (func 10474) |
| 23 | 10480 | 12 | 4150 | `__dtor_vota::CDigitalNaoReconhecidaDecBiometria::s_instancia` | ~unique_ptr de s_instancia | @1909008 | thunk→389 (~ICF 244 + free) | uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaoreconhecidadecbiometria.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlinado em CDigitalNaoReconhecida::StartState (func 10474) |
| 24 | 10483 | 10 | 4145 | `__dtor_vota::CDigitalReconhecida::s_mutex` | ~mutex de s_mutex | @1908956 | func 150 (~mutex vazio) | uenux2/src/app/vota/operador/confirmaidentidade/cdigitalreconhecida.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlinado em CPedeDigital::ObtemEstadoPosReconhecimentoBiometrico (func 5397) |
| 25 | 10484 | 12 | 4144 | `__dtor_vota::CDigitalReconhecida::s_instancia` | ~unique_ptr de s_instancia | @1908980 | thunk→763 (~ICF 448 + free) | uenux2/src/app/vota/operador/confirmaidentidade/cdigitalreconhecida.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlinado em CPedeDigital::ObtemEstadoPosReconhecimentoBiometrico (func 5397) |
| 26 | 10487 | 10 | 4139 | `__dtor_vota::CDigitalNaoReconhecidaPorTempo::s_mutex` | ~mutex de s_mutex | @1908928 | func 150 (~mutex vazio) | uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaoreconhecidaportempo.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlinado em CPedeDigital::ProcessTick (func 10465) |
| 27 | 10488 | 12 | 4138 | `__dtor_vota::CDigitalNaoReconhecidaPorTempo::s_instancia` | ~unique_ptr de s_instancia | @1908952 | thunk→763 (~ICF 448 + free) | uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaoreconhecidaportempo.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlinado em CPedeDigital::ProcessTick (func 10465) |
| 28 | 10502 | 10 | 4115 | `__dtor_vota::CNomeEleitor::s_mutex` | ~mutex de s_mutex | @1908816 | func 150 (~mutex vazio) | uenux2/src/app/vota/operador/confirmaidentidade/cnomeeleitor.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CNomeEleitor::GetInst (func 5401) |
| 29 | 10503 | 12 | 4114 | `__dtor_vota::CNomeEleitor::s_instancia` | ~unique_ptr de s_instancia | @1908840 | thunk→389 (~ICF 244 + free) | uenux2/src/app/vota/operador/confirmaidentidade/cnomeeleitor.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CNomeEleitor::GetInst (func 5401) |
| 30 | 10509 | 10 | 4102 | `__dtor_vota::CInformaBioDesabilitadaDemo::s_mutex` | ~mutex de s_mutex | @1908760 | func 150 (~mutex vazio) | uenux2/src/app/vota/operador/confirmaidentidade/cinformabiodesabilitadademo.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CInformaBioDesabilitadaDemo::GetInst (func 5402) |
| 31 | 10510 | 12 | 4101 | `__dtor_vota::CInformaBioDesabilitadaDemo::s_instancia` | ~unique_ptr de s_instancia | @1908784 | thunk→389 (~ICF 244 + free) | uenux2/src/app/vota/operador/confirmaidentidade/cinformabiodesabilitadademo.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CInformaBioDesabilitadaDemo::GetInst (func 5402) |
| 32 | 10514 | 10 | 4096 | `__dtor_vota::CControlaReconhecimento::s_mutex` | ~mutex de s_mutex | @1908732 | func 150 (~mutex vazio) | uenux2/src/app/vota/operador/confirmaidentidade/ccontrolareconhecimento.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CControlaReconhecimento::GetInst (func 1256); membros de dados static usados pelas funcs 3614, 10425, 10451, 10465, 10512 |
| 33 | 10515 | 12 | 4095 | `__dtor_vota::CControlaReconhecimento::s_instancia` | ~unique_ptr de s_instancia | @1908756 | thunk→389 (~ICF 244 + free) | uenux2/src/app/vota/operador/confirmaidentidade/ccontrolareconhecimento.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CControlaReconhecimento::GetInst (func 1256); membros de dados static usados pelas funcs 3614, 10425, 10451, 10465, 10512 |
| 34 | 10516 | 36 | 4094 | `__dtor_vota::CControlaReconhecimento::s_tituloMesario` | ~string de s_tituloMesario | @1908696 | inline ~string | uenux2/src/app/vota/operador/confirmaidentidade/ccontrolareconhecimento.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CControlaReconhecimento::GetInst (func 1256); membros de dados static usados pelas funcs 3614, 10425, 10451, 10465, 10512 |
| 35 | 10517 | 39 | 4093 | `__dtor_vota::CControlaReconhecimento::s_digitalMesario` | ~vector<uebyte> de s_digitalMesario | @1908684 | inline ~vector | uenux2/src/app/vota/operador/confirmaidentidade/ccontrolareconhecimento.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CControlaReconhecimento::GetInst (func 1256); membros de dados static usados pelas funcs 3614, 10425, 10451, 10465, 10512 |
| 36 | 10518 | 39 | 4092 | `__dtor_vota::CControlaReconhecimento::s_digitalCapturada` | ~vector<uebyte> de s_digitalCapturada | @1908672 | inline ~vector | uenux2/src/app/vota/operador/confirmaidentidade/ccontrolareconhecimento.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CControlaReconhecimento::GetInst (func 1256); membros de dados static usados pelas funcs 3614, 10425, 10451, 10465, 10512 |
| 37 | 10525 | 10 | 4082 | `__dtor_vota::CHabilitaAudioEleitor::s_mutex` | ~mutex de s_mutex | @1908584 | func 150 (~mutex vazio) | uenux2/src/app/vota/operador/confirmaidentidade/chabilitaaudioeleitor.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CHabilitaAudioEleitor::GetInst (func 2743) |
| 38 | 10526 | 12 | 4081 | `__dtor_vota::CHabilitaAudioEleitor::s_instancia` | ~unique_ptr de s_instancia | @1908608 | thunk→389 (~ICF 244 + free) | uenux2/src/app/vota/operador/confirmaidentidade/chabilitaaudioeleitor.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CHabilitaAudioEleitor::GetInst (func 2743) |
| 39 | 10528 | 10 | 4076 | `__dtor_vota::CConfirmaInspecionada::s_mutex` | ~mutex de s_mutex | @1908556 | func 150 (~mutex vazio) | uenux2/src/app/vota/operador/leidentidade/cconfirmainspecionada.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlinado em CAguardaInspecao::ProcessMessage (func 10530) |
| 40 | 10529 | 12 | 4075 | `__dtor_vota::CConfirmaInspecionada::s_instancia` | ~unique_ptr de s_instancia | @1908580 | thunk→389 (~ICF 244 + free) | uenux2/src/app/vota/operador/leidentidade/cconfirmainspecionada.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlinado em CAguardaInspecao::ProcessMessage (func 10530) |
| 41 | 10531 | 10 | 4070 | `__dtor_vota::CAguardaInspecao::s_mutex` | ~mutex de s_mutex | @1908528 | func 150 (~mutex vazio) | uenux2/src/app/vota/operador/leidentidade/caguardainspecao.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlinado em CPedeIdentidade::ProcessTick (func 10676) |
| 42 | 10532 | 12 | 4069 | `__dtor_vota::CAguardaInspecao::s_instancia` | ~unique_ptr de s_instancia | @1908552 | thunk→389 (~ICF 244 + free) | uenux2/src/app/vota/operador/leidentidade/caguardainspecao.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlinado em CPedeIdentidade::ProcessTick (func 10676) |
| 43 | 10553 | 10 | 4004 | `__dtor_vota::impl::IInformacaoThreadOperador::s_mutex` | ~mutex de s_mutex | @1908504 | func 150 (~mutex vazio) | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | IInformacaoThreadOperador::GetInst (func 599, srcloc :554) |
| 44 | 10616 | 10 | 3949 | `__dtor_vota::CIniciaJustificativa::s_mutex` | ~mutex de s_mutex | @1905704 | func 150 (~mutex vazio) | uenux2/src/app/vota/operador/justificativa/iiniciajustificativa.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlinado em CProcuraEleitor::StartState (func 10631) |
| 45 | 10621 | 10 | 3943 | `__dtor_vota::CConfirmaJustificativa::s_mutex` | ~mutex de s_mutex | @1905676 | func 150 (~mutex vazio) | uenux2/src/app/vota/operador/justificativa/iconfirmajustificativa.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlinado em CProcuraEleitor::StartState (func 10631) |
| 46 | 10629 | 10 | 3931 | `__dtor_vota::CValidaIdentidade::s_mutex` | ~mutex de s_mutex | @1905648 | func 150 (~mutex vazio) | uenux2/src/app/vota/operador/leidentidade/cvalidaidentidade.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlinado em CPedeIdentidade::ProcessInput (func 10677) |
| 47 | 10644 | 10 | 3909 | `__dtor_vota::CEleitorJaVotou::s_mutex` | ~mutex de s_mutex | @1905536 | func 150 (~mutex vazio) | uenux2/src/app/vota/operador/confirmaidentidade/celeitorjavotou.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlinado em CEleitorEncontrado::StartState (func 10635) |
| 48 | 10645 | 12 | 3908 | `__dtor_vota::CEleitorJaVotou::s_instancia` | ~unique_ptr de s_instancia | @1905560 | thunk→389 (~ICF 244 + free) | uenux2/src/app/vota/operador/confirmaidentidade/celeitorjavotou.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlinado em CEleitorEncontrado::StartState (func 10635) |
| 49 | 10660 | 87 | 3876 | `vota::(anonymous namespace)::TextoOptouTransito` | texto "Optou por votar em trânsito" | — |  | uenux2/src/app/vota/operador/leidentidade/ieleitorimpedidovotar.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | slot armazenado pelo ctor de CEleitorOptouPorVotarEmTransito inlinado em CEleitorEncontrado::StartState (func 10635) |
| 50 | 10661 | 10 | 3879 | `__dtor_vota::CEleitorOptouPorVotarEmTransito::s_mutex` | ~mutex de s_mutex | @1905396 | func 150 (~mutex vazio) | uenux2/src/app/vota/operador/leidentidade/ieleitorimpedidovotar.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlinado em CEleitorEncontrado::StartState (func 10635) |
| 51 | 10662 | 12 | 3878 | `__dtor_vota::CEleitorOptouPorVotarEmTransito::s_instancia` | ~unique_ptr de s_instancia | @1905420 | thunk→1959 (~IEleitorImpedidoVotar + free) | uenux2/src/app/vota/operador/leidentidade/ieleitorimpedidovotar.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlinado em CEleitorEncontrado::StartState (func 10635) |
| 52 | 10664 | 214 | 3867 | `vota::(anonymous namespace)::TextoNaoEncontrado` | texto "CPF não encontrado. Digite o Título." / "NÃO CADASTRADO nesta urna" | — |  | uenux2/src/app/vota/operador/leidentidade/ieleitorimpedidovotar.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | slot armazenado pelo ctor de CEleitorNaoEncontrado em CEleitorNaoEncontrado::GetInst (func 5424); chama 5416, 187 |
| 53 | 10665 | 10 | 3870 | `__dtor_vota::CEleitorNaoEncontrado::s_mutex` | ~mutex de s_mutex | @1905368 | func 150 (~mutex vazio) | uenux2/src/app/vota/operador/leidentidade/ieleitorimpedidovotar.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CEleitorNaoEncontrado::GetInst (func 5424) |
| 54 | 10666 | 12 | 3869 | `__dtor_vota::CEleitorNaoEncontrado::s_instancia` | ~unique_ptr de s_instancia | @1905392 | thunk→1959 (~IEleitorImpedidoVotar + free) | uenux2/src/app/vota/operador/leidentidade/ieleitorimpedidovotar.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CEleitorNaoEncontrado::GetInst (func 5424) |
| 55 | 10692 | 75 | 3830 | `vota::(anonymous namespace)::TextoOpcaoContadores` | texto "Exibir contadores" | — |  | uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | src/uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp (u27; verificado pela u38) | slot armazenado por CEscolheOpcao::GetInst/ctor (func 2753), entrada 4 (ou 3) do menu |
| 56 | 10693 | 75 | 3829 | `vota::(anonymous namespace)::TextoOpcaoRegistrarMesarios` | texto "Registrar mesários" | — |  | uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | src/uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp (u27; verificado pela u38) | slot armazenado por CEscolheOpcao::GetInst/ctor (func 2753), entrada 3 do menu |
| 57 | 10694 | 63 | 3828 | `vota::(anonymous namespace)::TextoOpcaoEncerrar` | texto "Encerrar votação" | — |  | uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | src/uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp (u27; verificado pela u38) | slot armazenado por CEscolheOpcao::GetInst/ctor (func 2753), entrada 2 do menu |
| 58 | 10696 | 10 | 3832 | `__dtor_vota::CEscolheOpcao::s_mutex` | ~mutex de s_mutex | @1905284 | func 150 (~mutex vazio) | uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CEscolheOpcao::GetInst (func 2753) |
| 59 | 10704 | 10 | 3822 | `__dtor_vota::CContadoresBiometria::s_mutex` | ~mutex de s_mutex | @1905256 | func 150 (~mutex vazio) | uenux2/src/app/vota/operador/outrasopcoes/ccontadoresbiometria.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlinado em CEscolheOpcao::ProcessInput (func 10689) |
| 60 | 10705 | 12 | 3821 | `__dtor_vota::CContadoresBiometria::s_instancia` | ~unique_ptr de s_instancia | @1905280 | thunk→389 (~ICF 244 + free) | uenux2/src/app/vota/operador/outrasopcoes/ccontadoresbiometria.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlinado em CEscolheOpcao::ProcessInput (func 10689) |
| 61 | 10707 | 10 | 3814 | `__dtor_vota::CIniciaFinalizacao::s_mutex` | ~mutex de s_mutex | @1905228 | func 150 (~mutex vazio) | uenux2/src/app/vota/operador/outrasopcoes/ciniciafinalizacao.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlinado em CEscolheOpcao::ProcessInput (func 10689) |
| 62 | 10708 | 12 | 3813 | `__dtor_vota::CIniciaFinalizacao::s_instancia` | ~unique_ptr de s_instancia | @1905252 | thunk→349 (apenas free) | uenux2/src/app/vota/operador/outrasopcoes/ciniciafinalizacao.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlinado em CEscolheOpcao::ProcessInput (func 10689) |
| 63 | 10711 | 10 | 3808 | `__dtor_vota::CEncerramentoHorarioInvalido::s_mutex` | ~mutex de s_mutex | @1905200 | func 150 (~mutex vazio) | uenux2/src/app/vota/operador/outrasopcoes/cencerramentohorarioinvalido.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlinado em CIniciaFinalizacao::StartState (func 10706) |
| 64 | 10712 | 12 | 3807 | `__dtor_vota::CEncerramentoHorarioInvalido::s_instancia` | ~unique_ptr de s_instancia | @1905224 | thunk→389 (~ICF 244 + free) | uenux2/src/app/vota/operador/outrasopcoes/cencerramentohorarioinvalido.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlinado em CIniciaFinalizacao::StartState (func 10706) |
| 65 | 10716 | 10 | 3802 | `__dtor_vota::CPerguntaFilaEleitorVazia::s_mutex` | ~mutex de s_mutex | @1905172 | func 150 (~mutex vazio) | uenux2/src/app/vota/operador/outrasopcoes/cperguntafilaeleitorvazia.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CPerguntaFilaEleitorVazia::GetInst (func 5426) |
| 66 | 10719 | 10 | 3797 | `__dtor_vota::CAguardaEleitoresVotarem::s_mutex` | ~mutex de s_mutex | @1905144 | func 150 (~mutex vazio) | uenux2/src/app/vota/operador/outrasopcoes/caguardaeleitoresvotarem.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlinado em CPerguntaFilaEleitorVazia::ProcessInput (func 10713) |
| 67 | 10720 | 12 | 3796 | `__dtor_vota::CAguardaEleitoresVotarem::s_instancia` | ~unique_ptr de s_instancia | @1905168 | thunk→389 (~ICF 244 + free) | uenux2/src/app/vota/operador/outrasopcoes/caguardaeleitoresvotarem.cpp | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlinado em CPerguntaFilaEleitorVazia::ProcessInput (func 10713) |
| 68 | 10724 | 10 | 3791 | `__dtor_vota::CPedeTituloEncerramento::s_mutex` | ~mutex de s_mutex | @1905116 | func 150 (~mutex vazio) | uenux2/src/app/vota/operador/outrasopcoes/cpedetituloencerramento.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CPedeTituloEncerramento::GetInst (func 3631) |
| 69 | 10725 | 12 | 3790 | `__dtor_vota::CPedeTituloEncerramento::s_instancia` | ~unique_ptr de s_instancia | @1905140 | thunk→389 (~ICF 244 + free) | uenux2/src/app/vota/operador/outrasopcoes/cpedetituloencerramento.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CPedeTituloEncerramento::GetInst (func 3631) |
| 70 | 10729 | 10 | 3783 | `__dtor_vota::CTituloEncerramentoInvalido::s_mutex` | ~mutex de s_mutex | @1905088 | func 150 (~mutex vazio) | uenux2/src/app/vota/operador/outrasopcoes/ctituloencerramentoinvalido.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlinado em CPedeTituloEncerramento::ProcessInput (func 10721) |
| 71 | 10733 | 12 | 3775 | `__dtor_vota::CEncerramentoAntecipado::s_instancia` | ~unique_ptr de s_instancia | @1905084 | thunk→389 (~ICF 244 + free) | uenux2/src/app/vota/operador/outrasopcoes/cencerramentoantecipado.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlinado em CPedeTituloEncerramento::StartState (func 10722) |
| 72 | 10735 | 10 | 3770 | `__dtor_vota::CConfirmaEncerramento::s_mutex` | ~mutex de s_mutex | @1905032 | func 150 (~mutex vazio) | uenux2/src/app/vota/operador/outrasopcoes/cconfirmaencerramento.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CConfirmaEncerramento::GetInst (func 3632) |
| 73 | 10755 | 10 | 3737 | `__dtor_vota::CHabilitacaoAudioNaoPermitida::s_mutex` | ~mutex de s_mutex | @1904920 | func 150 (~mutex vazio) | uenux2/src/app/vota/operador/outrasopcoes/chabilitacaoaudionaopermitida.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlinado em CEscolheOpcao::ProcessInput (func 10689) |
| 74 | 10756 | 12 | 3736 | `__dtor_vota::CHabilitacaoAudioNaoPermitida::s_instancia` | ~unique_ptr de s_instancia | @1904944 | thunk→389 (~ICF 244 + free) | uenux2/src/app/vota/operador/outrasopcoes/chabilitacaoaudionaopermitida.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlinado em CEscolheOpcao::ProcessInput (func 10689) |
| 75 | 10760 | 10 | 3731 | `__dtor_vota::CHorarioVotacaoTerminou::s_mutex` | ~mutex de s_mutex | @1904892 | func 150 (~mutex vazio) | uenux2/src/app/vota/operador/outrasopcoes/chorariovotacaoterminou.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CHorarioVotacaoTerminou::GetInst (func 5430) |
| 76 | 10761 | 12 | 3730 | `__dtor_vota::CHorarioVotacaoTerminou::s_instancia` | ~unique_ptr de s_instancia | @1904916 | thunk→389 (~ICF 244 + free) | uenux2/src/app/vota/operador/outrasopcoes/chorariovotacaoterminou.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | CHorarioVotacaoTerminou::GetInst (func 5430) |
| 77 | 10765 | 12 | 3724 | `__dtor_vota::CRegistroMesarioEncerrado::s_instancia` | ~unique_ptr de s_instancia | @1904888 | thunk→349 (apenas free) | uenux2/src/app/vota/operador/outrasopcoes/cregistromesarioencerrado.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/u38-foreign-fragments.cpp | GetInst inlinado em CControladorRegistraMesariosVota::GetEstadoAposRegistroVotacao (vf11, func 10793) |
| 78 | 11240 | 772 | 2961 | `vota::(anonymous namespace)::CabecalhoSequencialIdentificador` | cabeçalho do BEHB "Sequencial" + espaços + Título/CPF/Número livre (38 colunas) | — |  | uenux2/src/app/vota/eleitor/fimvotacao/cgerarelatorios.cpp | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | slot armazenado por CGeraRelatorios::StartState (func 12105) via comum_f604 (CDataText<std::string(*)()> em um CTextFieldPaper) |
| 79 | 11674 | 12 | 2095 | `__dtor_vota::CConfereVotoEmCargo<vota::CMajoritarioValido, (vota::ETelaVotacao)2>::s_instancia` | ~unique_ptr de s_instancia | @1838480 | thunk→1286 (~IConfereVotoEmCargo + free) | uenux2/src/app/vota/eleitor/votamajoritario/cpedemajoritario.cpp (TU que instancia; definição em cconferevotoemcargo.h, caminho inferido) | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | GetInst inlinado em CPedeMajoritario::ProcessInputAudio (func 11683) |
| 80 | 11675 | 10 | 2094 | `__dtor_vota::CConfereVotoEmCargo<vota::CMajoritarioValido, (vota::ETelaVotacao)2>::s_mutex` | ~mutex de s_mutex | @1838452 | func 150 (~mutex vazio) | uenux2/src/app/vota/eleitor/votamajoritario/cpedemajoritario.cpp (TU que instancia; definição em cconferevotoemcargo.h, caminho inferido) | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | GetInst inlinado em CPedeMajoritario::ProcessInputAudio (func 11683) |
| 81 | 11708 | 12 | 2044 | `__dtor_vota::CConfereVotoEmCargo<vota::CProporcionalBranco, (vota::ETelaVotacao)4>::s_instancia` | ~unique_ptr de s_instancia | @1838140 | thunk→1286 (~IConfereVotoEmCargo + free) | uenux2/src/app/vota/eleitor/votaproporcional/cpedeproporcional.cpp (TU que instancia; definição em cconferevotoemcargo.h, caminho inferido) | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | GetInst inlinado em CPedeProporcional::ProcessInputAudio (func 11711) |
| 82 | 11709 | 10 | 2043 | `__dtor_vota::CConfereVotoEmCargo<vota::CProporcionalBranco, (vota::ETelaVotacao)4>::s_mutex` | ~mutex de s_mutex | @1838112 | func 150 (~mutex vazio) | uenux2/src/app/vota/eleitor/votaproporcional/cpedeproporcional.cpp (TU que instancia; definição em cconferevotoemcargo.h, caminho inferido) | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | GetInst inlinado em CPedeProporcional::ProcessInputAudio (func 11711) |
| 83 | 11713 | 10 | 2046 | `__dtor_vota::CPedeProporcional::s_mutex` | ~mutex de s_mutex | @1838084 | func 150 (~mutex vazio) | uenux2/src/app/vota/eleitor/votaproporcional/cpedeproporcional.cpp | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | CPedeProporcional::GetInst (func 3849 -> corpo mesclado 6051) |
| 84 | 11722 | 12 | 2026 | `__dtor_vota::CConfereVotoEmCargo<vota::CConfirmaVotoLegenda, (vota::ETelaVotacao)10>::s_instancia` | ~unique_ptr de s_instancia | @1838004 | thunk→1286 (~IConfereVotoEmCargo + free) | uenux2/src/app/vota/eleitor/votaproporcional/cpedenominal.cpp (caminho inferido) (TU que instancia; definição em cconferevotoemcargo.h, caminho inferido) | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | GetInst inlinado em CPedeNominal::GetProximoEstado (vf16, func 11724) |
| 85 | 11802 | 10 | 1898 | `__dtor_vota::testeteclado::impl::IGeradorTeclas::s_mutex` | ~mutex de s_mutex | @1837656 | func 150 (~mutex vazio) | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.cpp | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | IGeradorTeclas::GetInst (srcloc ctesteteclado.cpp:121) inlinado em CTesteTeclado::StartState (func 11805) |
| 86 | 11807 | 10 | 1897 | `__dtor_vota::testeteclado::CTesteTeclado::s_mutex` | ~mutex de s_mutex | @1835124 | func 150 (~mutex vazio) | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.cpp | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | CTesteTeclado::GetInst (func 3855) |
| 87 | 11827 | 10 | 1864 | `__dtor_vota::testeteclado::CEsperaRetestar::s_mutex` | ~mutex de s_mutex | @1835012 | func 150 (~mutex vazio) | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cesperaretestar.cpp (caminho inferido) | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | CEsperaRetestar::GetInst (func 5936) |
| 88 | 11828 | 12 | 1863 | `__dtor_vota::testeteclado::CEsperaRetestar::s_instancia` | ~unique_ptr de s_instancia | @1835036 | thunk→1564 (~ICF 785 + free) | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cesperaretestar.cpp (caminho inferido) | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | CEsperaRetestar::GetInst (func 5936) |
| 89 | 11832 | 10 | 1856 | `__dtor_vota::testeteclado::CRetomada::s_mutex` | ~mutex de s_mutex | @1834972 | func 150 (~mutex vazio) | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cretomada.cpp (caminho inferido) | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | GetInst inlinado em CAjusteInicial::ValidaTemposDesligamento (func 7160) |
| 90 | 11833 | 38 | 1855 | `__dtor_vota::testeteclado::CRetomada::s_instancia` | ~unique_ptr de s_instancia | @1834996 | inline ~testeteclado::CBase (1559) + free | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cretomada.cpp (caminho inferido) | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | GetInst inlinado em CAjusteInicial::ValidaTemposDesligamento (func 7160) |
| 91 | 11844 | 38 | 1837 | `__dtor_vota::CRegerarZeresima::s_instancia` | ~unique_ptr de s_instancia | @1834912 | inline ~CGeraZeresimaBase (1720) + free | uenux2/src/app/vota/eleitor/iniciovotacao/cregerarzeresima.cpp (caminho inferido) | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | GetInst inlinado em CConfirmaRegerarZeresima::ProcessInput (func 11838) |
| 92 | 11863 | 10 | 1802 | `__dtor_vota::testeteclado::CPreZeresima::s_mutex` | ~mutex de s_mutex | @1834720 | func 150 (~mutex vazio) | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cprezeresima.cpp | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | GetInst inlinado em CVerificaEleicaoPassou::StartState (func 11914) |
| 93 | 11864 | 38 | 1801 | `__dtor_vota::testeteclado::CPreZeresima::s_instancia` | ~unique_ptr de s_instancia | @1834744 | inline ~testeteclado::CBase (1559) + free | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cprezeresima.cpp | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | GetInst inlinado em CVerificaEleicaoPassou::StartState (func 11914) |
| 94 | 11866 | 10 | 1797 | `__dtor_vota::CGeraDadosDinamicos::s_mutex` | ~mutex de s_mutex | @1834692 | func 150 (~mutex vazio) | uenux2/src/app/vota/eleitor/iniciovotacao/cgeradadosdinamicos.cpp | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | GetInst inlinado em testeteclado::CPreZeresima::GetEstadoPassouNoTeste (func 5945) |
| 95 | 11867 | 12 | 1796 | `__dtor_vota::CGeraDadosDinamicos::s_instancia` | ~unique_ptr de s_instancia | @1834716 | thunk→349 (apenas free) | uenux2/src/app/vota/eleitor/iniciovotacao/cgeradadosdinamicos.cpp | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | GetInst inlinado em testeteclado::CPreZeresima::GetEstadoPassouNoTeste (func 5945) |
| 96 | 11872 | 10 | 1789 | `__dtor_vota::CVerificaHorarioZeresima::s_mutex` | ~mutex de s_mutex | @1834664 | func 150 (~mutex vazio) | uenux2/src/app/vota/eleitor/iniciovotacao/cverificahorariozeresima.cpp | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | CVerificaHorarioZeresima::GetInst (func 5947) |
| 97 | 11873 | 12 | 1788 | `__dtor_vota::CVerificaHorarioZeresima::s_instancia` | ~unique_ptr de s_instancia | @1834688 | thunk→1564 (~ICF 785 + free) | uenux2/src/app/vota/eleitor/iniciovotacao/cverificahorariozeresima.cpp | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | CVerificaHorarioZeresima::GetInst (func 5947) |
| 98 | 11878 | 10 | 1780 | `__dtor_vota::CVisualizarCandidatos::s_mutex` | ~mutex de s_mutex | @1834636 | func 150 (~mutex vazio) | uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cvisualizarcandidatos.cpp | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | CVisualizarCandidatos::GetInst (func 1279) |
| 99 | 11889 | 10 | 1764 | `__dtor_vota::CMenuFiltrarCandidatosPorCargo::s_mutex` | ~mutex de s_mutex | @1834552 | func 150 (~mutex vazio) | uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatosporcargo.cpp | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | GetInst inlinado em CMenuVisualizarCandidatos::StartState (func 11881) |
| 100 | 11890 | 12 | 1763 | `__dtor_vota::CMenuFiltrarCandidatosPorCargo::s_instancia` | ~unique_ptr de s_instancia | @1834576 | thunk→349 (apenas free) | uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatosporcargo.cpp | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | GetInst inlinado em CMenuVisualizarCandidatos::StartState (func 11881) |
| 101 | 11893 | 10 | 1758 | `__dtor_vota::CMenuFiltrarCandidatosPorNumero::s_mutex` | ~mutex de s_mutex | @1834524 | func 150 (~mutex vazio) | uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatospornumero.cpp (caminho inferido) | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | CMenuFiltrarCandidatosPorNumero::GetInst (func 5954) |
| 102 | 11894 | 12 | 1757 | `__dtor_vota::CMenuFiltrarCandidatosPorNumero::s_instancia` | ~unique_ptr de s_instancia | @1834548 | thunk→389 (~ICF 244 + free) | uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatospornumero.cpp (caminho inferido) | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | CMenuFiltrarCandidatosPorNumero::GetInst (func 5954) |
| 103 | 11898 | 10 | 1752 | `__dtor_vota::CMaisInformacoes::s_mutex` | ~mutex de s_mutex | @1834496 | func 150 (~mutex vazio) | uenux2/src/app/vota/eleitor/iniciovotacao/cmaisinformacoes.cpp (caminho inferido) | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | CMaisInformacoes::GetInst (func 1280) |
| 104 | 11899 | 12 | 1751 | `__dtor_vota::CMaisInformacoes::s_instancia` | ~unique_ptr de s_instancia | @1834520 | thunk→389 (~ICF 244 + free) | uenux2/src/app/vota/eleitor/iniciovotacao/cmaisinformacoes.cpp (caminho inferido) | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | CMaisInformacoes::GetInst (func 1280) |
| 105 | 11918 | 10 | 1720 | `__dtor_vota::CReinicioComparecimentoMesario::s_mutex` | ~mutex de s_mutex | @1834328 | func 150 (~mutex vazio) | uenux2/src/app/vota/eleitor/creiniciocomparecimentomesario.cpp (caminho inferido) | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | CReinicioComparecimentoMesario::GetInst (func 3864 -> corpo mesclado 764) |
| 106 | 11919 | 12 | 1719 | `__dtor_vota::CReinicioComparecimentoMesario::s_instancia` | ~unique_ptr de s_instancia | @1834352 | thunk→349 (apenas free) | uenux2/src/app/vota/eleitor/creiniciocomparecimentomesario.cpp (caminho inferido) | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | CReinicioComparecimentoMesario::GetInst (func 3864 -> corpo mesclado 764) |
| 107 | 11938 | 38 | 1690 | `__dtor_vota::CGeraZeresima::s_instancia` | ~unique_ptr de s_instancia | @1834212 | inline ~CGeraZeresimaBase (1720) + free | uenux2/src/app/vota/eleitor/iniciovotacao/cgerazeresima.cpp (caminho inferido) | src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp | CGeraZeresima::GetInst (func 5962) |
