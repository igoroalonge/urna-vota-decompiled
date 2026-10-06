# u37: funções de `app:vota` sem arquivo-fonte conhecido (encanamento da máquina de estados, singletons lazy, o menu "Mais informações", registros de log, helpers de impressão do BU)

A unidade u37 reúne **89 funções wasm** que as ferramentas colocaram no componente `app:vota`, mas não conseguiram associar a um
arquivo original: não há registro `std::source_location` e, muitas vezes, também não há RTTI. Elas não formam um subsistema. São a
cola sobre a qual o resto da aplicação de votação (VOTA) é construído. Cerca de um terço são funções minúsculas cujos corpos
o wasm-opt **mesclou** com outros ("merge-similar-functions": as constantes que diferiam viraram parâmetros) ou
**dobrou** (identical-code folding). As funções restantes são pequenos membros de classes que outras unidades já
reconstruíram. Todas as funções foram nomeadas, e cada uma foi escrita no arquivo de onde mais provavelmente
vem. Alguns caminhos são **(caminho inferido)**: nenhum registro nomeia esses arquivos, então eles vêm da convenção de nomes do TSE
(classe `CFooBar` → `cfoobar.cpp`, no diretório das classes aparentadas). Cada arquivo reconstruído diz isso nas suas primeiras
linhas.

**13 das 89 funções executaram** durante os votos gravados (`analysis/runtime/*.functions.tsv`):
`CAppState::NeedChangeState` (7480), os helpers do contexto de estados `ProcessMessage`/`ProcessInput`/`ProcessTick`/
`AceitaTeclado` (3843, 5911, 5910, 5909), o poller de ticks `CTickManager::GetTicksExpirados` (5450, a cada
`votaTick`), `CEleitorVotando::GetInst` (3229), o construtor de `IConfereVotoEmCargo` (1165, a cada voto), o
par cópia/remoção do contexto de aplicação (1841, 5553), o corpo do construtor de `IForm` (6024, toda tela) e duas
funções do menu "Mais informações" que `CTelasVota` constrói na inicialização (`CItemMenu::~CItemMenu` 2937,
`CItemImprimeEstadoUrna::Disponivel` 11669). Uma 14ª função certamente executou também, embora o profiler nunca
a tenha amostrado: o corpo mesclado de singleton 764. Seu thunk `CAguardaMensagem::GetInst` (1337, chamado por `main`) foi
amostrado, e esse thunk chama a 764 incondicionalmente.

| tópico | funções | § |
|---|---|---|
| encanamento da máquina de estados: `comum::CAppState`, `comum::CAppStateContext`, o poller de ticks | 7480, 3842, 3843, 5908-5911, 5450, 2125 | 3 |
| singletons lazy de estados (corpo mesclado 764 + 7 thunks, 8 construtores inlinados) | 764, 3864, 3879, 5342, 5983-5985, 6037, 1256, 1279, 1897, 2888, 3229, 5947, 5954, 1903 | 4 |
| destrutores at-exit dos statics dos singletons (código morto) | 7116 … 7798 (17 funções), 1286 | 4.3 |
| menu "Mais informações": `CMenuBase`, `CItemMenu` e quatro itens de relatório | 12235, 6188, 2937, 11666-11669 | 5 |
| telas de votação: temporização da conferência, contexto do ciclo do eleitor, telas | 1165, 3229, 1841, 5553, 6024, 5547, 3890, 6587, 2902 | 6 |
| registros do log de eventos (`CLogVota`, `CControladorRegistraMesariosVota`) | 2282, 3279, 4550, 4556, 5881, 5885, 3887, 3888, 6017, 5895 | 7 |
| persistência: `eg.bin`, contadores do `vota.bin`, o marcador "imprimindo", MR, diretórios WSQ | 3333, 3705, 3701, 1487, 1488, 2863, 3795, 5818, 5819, 1381 | 8 |
| BU: peças do fluxo de encerramento | 3879, 5983-5985, 6037, 2888, 3701, 1487/1488, 1540, 4556, 5885, 2863, 5547, 5918 | 9 |
| membros diversos de outras classes | 677, 2687, 2803, 1393, 5903, 5918, 5948, 5951, 3667, 2875 | 10 |

**Arquivos-fonte reconstruídos** (arquivos novos estão marcados com *new*; arquivos `*.u37.*` são fragmentos a mesclar no arquivo nomeado):

```
src/uenux2/src/app/comum/cappstate.u37.h                        new   comum::CAppState (class, layout, slots)
src/uenux2/src/app/comum/cappstate.u37.cpp                      new   NeedChangeState + 6 CAppStateContext helpers
src/uenux2/src/app/comum/citemmenu.h                            new   comum::CItemMenu
src/uenux2/src/app/comum/citemimprimeestadourna.{h,cpp}         new   comum::CItemImprimeEstadoUrna
src/uenux2/src/app/comum/citemimprimelistaeleitores.{h,cpp}     new   comum::CItemImprimeListaEleitores
src/uenux2/src/app/comum/citemparametrosurna.{h,cpp}            new   comum::CItemParametrosUrna
src/uenux2/src/app/comum/citemversoespacotes.{h,cpp}            new   comum::CItemVersoesPacotes
src/uenux2/src/app/comum/cmenubase.h                            new   comum::CMenuBase (declaration)
src/uenux2/src/app/comum/cmenubase.u37.cpp                            ~CMenuBase (u02 has AdicionaItem/Monta)
src/uenux2/src/app/comum/appinfo/cappinfo.u37.cpp                     comum::GravaEstadoGeral (eg.bin)
src/uenux2/src/app/comum/cinfomtlcd.u37.cpp                           CInfoMTLCD::ExibeBateria
src/uenux2/src/app/comum/iinterfaceinit.u37.cpp                       LogInfo, IInterfaceInit::HabilitarMR
src/uenux2/src/app/comum/informacao/cinformacaoeleicao.u37.cpp        ImprimeBoletimJustificativa
src/uenux2/src/app/comum/dados/celeitores.u37.cpp                     CEleitores::GetBiometriaCorrente
src/uenux2/src/app/comum/dados/md/cvalidadoridentidade.u37.cpp        CValidadorIdentidade::EhValida
src/uenux2/src/app/comum/dados/md/estadoaplicacao/estadoaplicacao.u37.cpp   IncrementaQtdBU, SetAjusteDataHora
src/uenux2/src/app/comum/relatorios/cgeradorrelpu.h                   shared with unit u35 (see note in the file)
src/uenux2/src/app/comum/relatorios/cgeradorrelpu.u37.cpp             CGeradorRelPU::AdicionaLinha
src/uenux2/src/api/util/cdatetime.u37.cpp                             CDateTime::ToTimeT
src/uenux2/src/api/util/ctickmanager.u37.cpp                          CTickManager::GetTicksExpirados
src/uenux2/src/api/gui/capplicationcontextstack.u37.cpp               CApplicationContext copy ctor, Stack::Remove
src/uenux2/src/api/gui/u37-template-instances.cpp                     IForm ctor body, builder Add bodies
src/uenux2/src/app/vota/u37-foreign-fragments.cpp                     singletons, small members, compiler-generated code
src/uenux2/src/app/vota/log/clogvota.u37.cpp                          5 CLogVota records + IEventosLog::LogaErro
src/uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.u37.cpp  8 mesário-registration log slots
src/uenux2/src/app/vota/comum/crelvotautil.u37.cpp                    the "dinamico/imprimindo" marker
src/uenux2/src/app/vota/eleitor/cconferevotoemcargo.u37.cpp           IConfereVotoEmCargo constructor
src/uenux2/src/app/vota/operador/outrasopcoes/cimpressaopu.u37.cpp    FormataData (PU report)
```

Glossário: *terminal do mesário* / MT = o microterminal do mesário; *título (de eleitor)* número de inscrição do eleitor; *habilitação* liberação de um eleitor para votar;
*conferência* a tela curta de "confira seu voto" exibida depois do último dígito; *zerésima* o relatório zerado impresso antes da
votação; *BU (boletim de urna)* o relatório e o arquivo de resultado de cada urna; *via* cópia impressa; *encerramento* fechamento da
votação; *MI / MV* flash interna (`/dsk/fi`) / flash removível da mídia de votação (`/dsk/fe`); *MR (mídia de resultado)*
o pendrive que leva os arquivos de resultado à Junta Eleitoral; *PU (parâmetros de urna)* os parâmetros de urna
de cada eleição (`-pu.dat`, `ecourna::app::dados::CParametrosUrna` em `CConfiguracaoEleicao +88`); *WSQ* o
formato de imagem de impressões digitais.

---

## 1. Onde este código fica no processo de votação

* **Toda tela da urna é um estado** (`comum::CAppState`). O terminal do eleitor (`vota::CThreadEleitor`) e
  o terminal do mesário (`vota::CThreadOperador`) conduzem cada um uma máquina de estados por meio de um
  `comum::CAppStateContext`. A cada iteração do laço, uma thread (1) repassa as mensagens entre threads enfileiradas ao
  estado corrente, (2) repassa um pressionamento de tecla, (3) repassa os ticks de timer expirados e (4) troca de estado quando
  `NeedChangeState()` assim indica. No build web, a thread do eleitor não tem uma thread própria: `votaTick` chama
  `CThreadEleitor::Processar` (4349, antes exibida pelas ferramentas como `ProcessarEntrada`), e é ali que 3843,
  5909-5911, 5450 e 7480 foram vistas executando.
* **Os estados são singletons lazy.** A seção 4 lista os acessores desta unidade: o ciclo do eleitor `CEleitorVotando`,
  a cadeia do encerramento/BU (`CInicioBU`, `CGravaResultado`, `CImprimirBUOutrasObrigatorias`, `CImprimindoBim`,
  `CEmitirMaisBU`, `CMostraQRCodeBU`), os estados do início do dia (`CVerificaHorarioZeresima`, o visualizador de candidatos)
  e estados do operador (`CControlaReconhecimento`, `CSincronismoOperador`, `CFinalizaOperador`,
  `CReinicioComparecimentoMesario`).
* **Antes da zerésima**, o menu "Mais informações" permite ao mesário imprimir um número limitado de vias extras de
  quatro relatórios (§5).
* **Durante o voto**, o construtor de `IConfereVotoEmCargo` define quanto tempo a tela de conferência permanece (§6.1).
  `CEleitorVotando` empilha um contexto de erro que declara o voto como *não registrado* se algo falhar enquanto o eleitor
  está escolhendo (§6.2).
* **No fim do dia**, a unidade fornece as peças que imprimem as vias do BU e as contam (§9).

---

## 2. Classes e hierarquia (RTTI)

```
api::CState (typeinfo @1551296, no vtable)
 └─ comum::CAppState                          vtable @1551248 (9 slots, table below)            cappstate.u37.h
     ├─ every vota:: and comum:: state (CAguardaMensagem, CEleitorVotando, CGeraBU, CInicioBU, ...)
     ├─ vota::CVotacaoStateAudio ─ vota::IConfereVotoEmCargo (@1547584) ─ CConfereVotoEmCargo<TConfirma, TELA> (10)
     └─ vota::CEstadoComDesligamentoAutomatico (@1541908) ─ testeteclado::CBase (@1545776, abstract) ─ CPreZeresima, CRetomada
                                                         ─ CVerificaHorarioZeresima, CMostraQRCodeBU, ...
api::CStateContext<comum::CAppState> (typeinfo @1551332) ─ comum::CAppStateContext (vtable @1551312)   (u35)

comum::CItemMenu (typeinfo @1539904, vtable @1539892: [0] 2937, [1] 325, [2] pure Disponivel)   citemmenu.h
 ├─ comum::CItemImprimeEstadoUrna     @1550788  [2] 11669 [3] pure ─ vota::CItemImprimeEstadoUrnaVota     @1539644 [3] 12267
 ├─ comum::CItemImprimeListaEleitores @1550824  [2] 11668 [3] pure ─ vota::CItemImprimeListaEleitoresVota @1539680 [3] 12266
 ├─ comum::CItemParametrosUrna        @1550860  [2] 11667 [3] pure ─ vota::CItemParametrosUrnaVota        @1539752 [3] 12255
 ├─ comum::CItemVersoesPacotes        @1550896  [2] 11666 [3] pure ─ vota::CItemVersoesPacotesVota        @1539716 [3] 12259
 └─ vota::CItemVisualizarCandidatosVota @1539788 [2] 12244 (u02)
comum::CMenuBase (typeinfo @1550944, vtable @1550932: [0] 12235 [1] 6188 [2] Monta 5913) ─ vota::CMenuMaisInformacoesVota @1539820
comum::CGeradorRelPU (typeinfo @1545296, vtable @1545288: [0] 11901 [1] 11900)     (u35 + u37)
```

As quatro vtables de itens do comum, `CMenuBase` e `api::COptionValidation` (usada por `CMenuBase::Monta`) são contíguas no
segmento de dados (@1550788 … @1550996). Isso se encaixa com uma unidade de tradução por classe compilada em ordem alfabética
(`citemimprimeestadourna` < `citemimprimelistaeleitores` < `citemparametrosurna` < `citemversoespacotes` <
`cmenubase`). O mesmo layout também apareceria se tudo estivesse em `cmenubase.cpp`.

Slots de `comum::CAppState` (nomes: 3 e 7 a partir de overrides nomeados por srcloc, o resto a partir do seu uso pelas threads):

| slot | função | corpo padrão |
|---:|---|---|
| 0 / 1 | `~CAppState()` / deleting | 174 (trivial) / 325 (inalcançável, abstrata) |
| 2 | `StartState()` | puro |
| 3 | `NeedChangeState()` | **7480** `return GetNextState() != this` |
| 4 | `GetNextState()` | 1661 `return m_proximoEstado` (+4) |
| 5 | `FinishState()` | no-op |
| 6 | `ProcessMessage(uebyte)` | no-op |
| 7 | `ProcessInput()` | no-op |
| 8 | `ProcessTick(uebyte)` | no-op |

Layout (12 bytes, construtor `shared_f224(this, flags)`): `+4 m_proximoEstado = this`, e os três flags de entrada
vindos da máscara de bits do construtor: `+8` mensagens (1), `+9` teclas (2), `+10` ticks (4).
`CEstadoComDesligamentoAutomatico` sempre acrescenta ticks (`flags | 4`).

---

## 3. Encanamento da máquina de estados

### 3.1 Helpers de `CAppStateContext` (3842, 3843, 5908, 5909, 5910, 5911)

Seis funções de uma linha, não inline, do contexto (`+4 CAppState* m_estado`). Cada uma tolera um contexto vazio:
`AceitaMensagens/AceitaTeclado/AceitaTicks` retornam o flag do estado (+8/+9/+10) ou false.
`ProcessMessage/ProcessInput/ProcessTick` repassam para os slots 6/7/8 quando há um estado. Os dois laços de thread as usam
(CThreadEleitor::Processar 4349, CThreadOperador::Run 10204). Elas também poderiam ser membros da base template
`api::CStateContext<CAppState>`, porque só tocam membros públicos do estado.

### 3.2 Ticks: `api::CTickManager::GetTicksExpirados` (5450)

Cada thread do VOTA possui um `std::map<uebyte, STick>` (unidade u20: `AddTick`, `AddStoppedTick`, `StartTick`,
`StopTick`). O poller:

1. lê o relógio de parede com `gettimeofday` (o relógio do JS no build web);
2. percorre o map em ordem de id e pula os ticks parados (`+24 == 1`) e os ticks cujo `proximo` ainda está no futuro;
3. para cada tick expirado, acrescenta seu id ao resultado e o **rearma**. A nova expiração é `proximo + k·intervalo`,
   em que k é o número de períodos inteiros decorridos mais um. O binário calcula isso em aritmética mista de double/inteiro de
   32 bits (reproduzida em `ctickmanager.u37.cpp`). **Períodos perdidos são colapsados**: uma thread que ficou ocupada por
   vários períodos recebe o tick uma única vez.

A thread então chama `ProcessTick(id)` para cada id (5910), desde que o estado aceite ticks.

### 3.3 `NeedChangeState` (7480)

Um estado pede para ser deixado escrevendo outro estado em `m_proximoEstado`. `NeedChangeState` compara
`GetNextState()` com `this`. Os estados que sobrescrevem o slot 3 (`CEleitorVotando::NeedChangeState` 7352,
`CDefineRotaPosReinicio::NeedChangeState` 11858) acrescentam suas próprias condições.

---

## 4. Singletons lazy

### 4.1 O corpo mesclado 764 e seus thunks

Todo estado é criado no primeiro uso. O padrão do código-fonte é
`static std::mutex m; static std::unique_ptr<T> p; lock_guard; if (!p) p.reset(new T); return *p;`. O build não tem
pthreads, então `lock()` desapareceu e só resta o stub de unlock (func 150). Para os estados de 12 bytes cujo
construtor é apenas `CAppState(flags)` mais um vptr, o wasm-opt mesclou todos os corpos de GetInst em
**764** `(mutex, &unique_ptr, vtable, flags)`. Ela tem 19 chamadores. Os desta unidade:

| wasm | acessor | statics (mutex / unique_ptr) | flags |
|---:|---|---|:-:|
| 3864 | `CReinicioComparecimentoMesario::GetInst` | @1834328 / @1834352 | 0 |
| 3879 | `CEmitirMaisBU::GetInst` | @1833600 / @1833624 | 2 |
| 5342 | `CFinalizaOperador::GetInst` | @1911628 / @1911652 | 0 |
| 5983 | `CInicioBU::GetInst` | @1833768 / @1833792 | 0 |
| 5984 | `CImprimirBUOutrasObrigatorias::GetInst` | @1833740 / @1833764 | 0 |
| 5985 | `CImprimindoBim::GetInst` | @1833684 / @1833708 | 0 |
| 6037 | `CGravaResultado::GetInst` | @1833572 / @1833596 | 0 |

### 4.2 Acessores com construtor inlinado

| wasm | acessor | objeto | observações |
|---:|---|---|---|
| 3229 | `CEleitorVotando::GetInst` ✓ | 72 B, CAppState(7) | contexto `(11, "Erro inesperado durante a votação", "O voto do eleitor NÃO foi registrado", "Ocorreu um erro enquanto o eleitor registrava suas escolhas.")`, tick de inatividade = PU `tempoDispararSuspensaoTE` (+84) no treinamento de eleitor, senão `tempoDispararSuspensao` (+80), × 1000 ms |
| 5947 | `CVerificaHorarioZeresima::GetInst` | 52 B | formulário `CTelasVota +20` ("telaAntesHorarioZeresima"), data/hora da zerésima `cfg +544`, tick parado de 2 s |
| 2888 | `CMostraQRCodeBU::GetInst` | 36 B, `CEstadoComDesligamentoAutomatico(2)` | +28 formulário, construído em StartState |
| 1279 | `CVisualizarCandidatos::GetInst` | 40 B, CAppState(2) | três filtros opcionais (cargo +24, número +28, partido +36) |
| 5954 | `CMenuFiltrarCandidatosPorNumero::GetInst` | 20 B, CAppState(2) | +12 formulário |
| 1897 | `CSincronismoOperador::GetInst` | 12 B, CAppState(1) | +11 `m_suspensaoAutomatica = false` |
| 1256 | `CControlaReconhecimento::GetInst` | 20 B, CAppState(2) | +12 = `CriaFormEleitorPodeVotar()` (5410), depois toca a 1903 |
| 1903 | `ConfiguracaoExtrator::GetInst` (nome da u22) | 16 B | `{8, 500, 500/2.54}` = bits por pixel, ppi, pixels por cm. Configuração constante do extrator de templates de impressão digital. É criada logo antes de cada extração (stubada) |

### 4.3 Destrutores at-exit (código morto) e o singleton desconhecido

As 17 funções minúsculas 7116 … 7798 são os destrutores `__cxa_atexit` desses statics: `~mutex` é o stub de pthread
150, e `~unique_ptr` é `shared_f349` (free simples) ou `shared_f389` (dtor 244 + free). O Emscripten compila
sem `EXIT_RUNTIME`, então atexit é um no-op, as chamadas de registro sumiram e **nenhum código chama essas funções**.
Elas só sobrevivem porque a tabela de funções ainda as referencia. O dono de cada static aparece na
tabela de mapeamento e em `u37-foreign-fragments.cpp`. **7790/7798 (@1832908/@1832932)** pertencem a um singleton cujo
acessor não existe mais: nenhuma instrução carrega qualquer um dos dois endereços. O layout da tabela aponta para sua unidade de
tradução. Todos os outros pares at-exit desta unidade ficam **logo antes do primeiro slot de vtable da classe do seu próprio
arquivo**:

| slots at-exit | slot seguinte |
|---|---|
| 924 | 925 `CEleitorVotando` |
| 896/897 | 898 `CAguardaMensagem` |
| 908 | 909 `CConfirmaVotoSemCandidato` |
| 933/934 | 935 `CIniciodeCiclo` |
| 939 | 940 `CInstrucaoVotacaoAcessibilidade` |
| 947/948 | 949 `CMostraTelaContinuaVotacao` |
| 952/953 | 954 `CFimVotoEleitor` |
| 957/958 | 959 `CSincronismoEleitor` |
| 963 | 964 `impl::CSincronismoVotoEleitor` (seu slot [1]; o [0] reutiliza o slot 340) |
| 980 | 981 `CThreadEleitor` |

Os slots 866/867 são seguidos pelo slot 868, o primeiro slot de vtable de **`vota::CAssinadorVota`**. Os dados concordam: o
static seguinte, @1832936, é o flag "urna desligando" de `CSincronizaVota`. O singleton desconhecido, portanto, muito provavelmente
vive no arquivo que emite a vtable de `CAssinadorVota` e contém esse flag (`vota/comum/csincronizavota.cpp` ou
`cassinadorvota.cpp`). Ele não vem de `vota::CExecucaoVotaCooperativa`: os slots de vtable dessa classe
(856-865) vêm *antes* de 866/867, e não depois, o que é o oposto do padrão.

`vota_f1286` é um corpo at-exit mesclado do mesmo tipo, `(atexit argument, &unique_ptr)`. Os 10 stubs at-exit do
`s_inst` das instanciações de `CConfereVotoEmCargo<…>` na unidade u30 o usam. Ele deleta por meio da 1717, o
`~IConfereVotoEmCargo` que todas as 10 classes template herdam no slot 0 da vtable.

---

## 5. O menu "Mais informações" (`CMenuBase`, `CItemMenu`, quatro itens de relatório)

`CTelasVota::CriaTelaMaisInformacoes` (6599) constrói, uma única vez, na inicialização:

```
Mais informações
[1] - Estado da urna (n/max)          comum::CItemImprimeEstadoUrna     vias: EstadoGeralVota +73
[2] - Lista de eleitores (n/max)      comum::CItemImprimeListaEleitores vias: +74
[3] - Versões de pacotes (n/max)      comum::CItemVersoesPacotes        vias: +75
[4] - Parâmetros de urna (n/max)      comum::CItemParametrosUrna        vias: +76
[5] - Visualizar candidatos           vota::CItemVisualizarCandidatosVota
Escolha a sua opção: [_]
```

* `Disponivel()` (slot 2) retorna `GetNumViasImpressas() < max`, **sem sinal**. `GetNumViasImpressas()` (slot 3) é
  a subclasse do vota lendo o `vota.bin` (`EstadoGeralVota.numViasImpressasRelatorios`). `max` é o valor do PU
  `numRelatorioEstado / numRelatorioEleitores / numRelatorioVersoesDados / numRelatorioPU`, lido por meio de
  `CInformacaoEleicao`, e forçado a **1 no modo demonstração**.
* "Lista de eleitores" também exige, para o turno corrente, um tipo de urna vota ('1'), contingenciavota ('3') ou
  contingenciavotarecupera ('4') (`EstadoGeralUrna.dadoCarga.tipoUrnaT1/T2`, CEstadoGeral +36/+40). Uma urna de
  contingência simples ('2') ou uma urna sem tipo ('0') nunca oferece a lista de eleitores.
* **Correção à unidade u02** (`cmenubase.u02.cpp`): `AdicionaItem` (2285) acrescenta o id às opções aceitas somente
  quando `item->Disponivel()`. `Monta` (5913) desenha os itens indisponíveis na cor 5 (cinza) em vez de 2. Um relatório
  esgotado, portanto, continua listado, mas não pode ser selecionado. As arestas de tempo de execução confirmam que `2285 → 11669` e
  `5913 → 11669` executaram.
* Destrutores: `~CMenuBase` 12235/6188 (string `m_opcoes` +16, depois o `vector<shared_ptr<CItemMenu>>` +4), e
  `~CItemMenu` 2937 (um corpo mesclado que destrói a string em +8).

Selecionar um item imprime o relatório e incrementa o contador no `vota.bin`. Para "Parâmetros de urna", isso é
`vota::CImpressaoPU` (11902, unidade u25). Ele imprime os quatro instantes do dia de votação da configuração
(`boost::posix_time::ptime` em cfg +504…+528) por meio de `CGeradorRelPU::AdicionaLinha` (677: rótulo + valor
alinhado à direita na coluna 38), `FormataHorario` (2686, u25) e `FormataData` (2687, `"{:02}/{:02}/{:04}"`).

---

## 6. Telas de votação

### 6.1 Temporização da conferência: `IConfereVotoEmCargo(ETelaVotacao)` (1165) ✓

`CPede*` constrói o estado de cada conferência (`CConfereVotoEmCargo<TConfirma, TELA>`, 10 instanciações) com
`new(40)` + este construtor + o vptr do template:

* `CVotacaoStateAudio(6)`: teclas e ticks, sem mensagens;
* `m_tela` (+30) = o id da tela (o único parâmetro). O header da unidade u06 declarava um segundo parâmetro `bool comTick`;
  **ele não existe**;
* `m_tick` (+28) = um tick *parado* de `ParametrosUrna.tempoConfirmacaoVoto` ms (`CParametrosUrna +88`, cfg +176;
  1000 nos cenários publicados), criado na thread do eleitor apenas quando o valor é > 0. `m_comTick` (+29)
  registra se ele existe;
* `m_espera` (+32) vazio.

StartStateAudio inicia o tick. Quando ele dispara, vem a tela CONFIRMA/CORRIGE. As teclas pressionadas nesse intervalo são
rejeitadas e registradas no log como *"Tecla indevida pressionada"*: o eleitor precisa pressionar CONFIRMA de novo quando a tela de
confirmação aparece. Esta é a pausa de 1,5 s de que `tools/run/headless.mjs` precisa.

### 6.2 O contexto de erro do ciclo do eleitor (3229, 1841, 5553) ✓

`CEleitorVotando` possui um `api::CApplicationContext` (+20). `IniciaCiclo` empilha uma cópia dele (1841 = construtor
de cópia), e `FinishState` o remove (5553, `CApplicationContextStack::Remove`: busca reversa por um contexto
igual e depois erase). Enquanto ele está na pilha, qualquer erro fatal durante o voto mostra *"O voto do eleitor NÃO foi
registrado / Ocorreu um erro enquanto o eleitor registrava suas escolhas."* com a ação 11 (reiniciar a urna; se
o erro persistir, fotografar o QR code). `~CApplicationContextGuard` (675) contém o mesmo código de remoção, mas
o pula quando `std::uncaught_exceptions() != 0`, para que o contexto de uma operação que está falhando permaneça no topo
para a tela de erro fatal.

### 6.3 Telas

* `IForm<MEDIA>::IForm` (6024, mesclado para IScreen 5548 e IScreenMT 5522) ✓: copia os campos e o hook
  pré-exibição, aloca o `FormControlBlock` de 128 bytes e chama `SetForm(this)` em cada campo.
* Corpos "adicionar um campo" do `CFormBuilder`: 3890 (um campo de 28 bytes com um argumento: nova linha no papel, LED, bipe) e
  5547, que adiciona um `CImageField` para as imagens de QR code de `CMostraQRCodeBU` / `CMostraQRCodeCertificado`. Os
  parâmetros wasm da 5547 são `(sret, builder, image, position)`:
  * a imagem vem antes da posição;
  * a imagem é um `shared_ptr` passado **por valor**, e a função chamada o libera (a ABI v2 da libc++ torna `shared_ptr`
    trivial_abi);
  * a âncora `1` é uma constante no corpo.

  Portanto, a 5547 não é uma instância simples do encaminhamento variádico `Add<FIELD>(ARGS&&...)` de `cformbuilder.u07.cpp`,
  que encaminharia `(position, image, 1)` por referência. É uma sobrecarga dedicada, ou uma instância por valor
  cujo argumento constante o LTO removeu.
* `CTelasVota::CriaTelaAguarde` (6587): `CriaTelaNeutra("Por favor, aguarde...", 35, false)`, exibida enquanto a
  zerésima é impressa.
* 2902: destrutor mesclado (vptr + liberação da tela `shared_ptr` em +12) de `IEleitorImpedidoVotar`,
  `IConfirmaJustificativa`, `CGeraZeresimaBase` e `CGeraResumoZeresimaBase`.
* `testeteclado::CBase::CBase` (5948): base do teste de teclado antes da zerésima,
  `CEstadoComDesligamentoAutomatico(2)` mais uma tela vazia.

---

## 7. Registros do log de eventos

Todos os registros vão para `/dsk/fi/dinamico/log/logd.dat` (`"<app>|<severity>|<text>"`, Latin-1, app 1 = VOTA, severidade
1 info / 2 aviso / 3 erro).

| wasm | método | severidade | texto |
|---:|---|:-:|---|
| 2282 | `IEventosLog::LogaErro(msg)` | 3 | (argumento) |
| 3279 | `CLogVota::LogaErroEstadoDesconhecido` | 3 | Erro estado do aplicativo não conhecido |
| 4550 | `CLogVota::LogaErroPartidoNaoEncontrado` | 3 | Erro partido não encontrado |
| 4556 | `CLogVota::LogaQtdViasExcedeMaximo` (corpo 6115) | 2 | Quantidade de vias adicionais excede o máximo permitido |
| 5881 | `CLogVota::LogaEstadoNaoEsperado` (corpo 3902) | 3 | Erro estado do aplicativo não esperado |
| 5885 | `CLogVota::LogaMRNaoPresente` (corpo 3902) | 3 | Mídia de resultado não estava presente |
| 3888 | slots 29/34/36 de `CControladorRegistraMesariosVota` | 1 | Operador encerrou ciclo de registro de mesários / Realizada a conferência da biometria do mesário / Operador indagado se finaliza registro mesários |
| 3887 | slots 31/32/33 | 1 | Pedido de leitura da biometria do mesário {título} / Mesário {título} é eleitor da seção / Mesário {título} não é eleitor da seção |
| 6017 | slots 19/27 | 1 | Mesário {título} registrado / Mesário {título} já registrado |

Os corpos com o mesmo comprimento de mensagem foram mesclados pelo wasm-opt: os pedaços de 8 bytes do literal viraram
parâmetros. Isso explica por que 3888, 3902 e 6115 recebem de 5 a 7 ponteiros para dentro de uma mesma string. A 5895 não é um registro do log
de eventos: é a cópia `LOG_INFO` do helper de log do sistema `LogSistema` (2921) em `iinterfaceinit.cpp`. Ela grava
no syslog em uma urna real (`/dev/urna` existe) e no stdout quando `DEBUG_UENUX` está definido.

---

## 8. Persistência e arquivos

| o quê | função | detalhes |
|---|---|---|
| `eg.bin` (EstadoGeralUrna) na MI e na MV | `comum::GravaEstadoGeral` 3333 | verifica o flag de desligamento @1832936 (lança `CUeDesligandoError`). **MI**: sob a proteção (2, "Gravando o estado geral da urna na MI", "Ocorreu um erro durante a sincronização do estado geral da urna na MI.") grava `dinamico/eg.bin` (CServicoEstadoGeral(0).Salva), `vota::CAssinadorVota(120).Assina(25)` o assina em `/dsk/fi/dinamico/eg.vsu` (a 1501 armazena o vptr de `CAssinadorVota`), depois sync. **MV**: sob a proteção (4, "... na MV", "... na MV.") grava o eg.bin, copia `eg.vsu` MI → MV, depois sync. Chamada depois que o relógio é ajustado: `CAjusteInicial`, `CIniciodeCiclo::AjustaDataHora`, `CEncerramentoHorarioInvalido`. O corpo usa classes exclusivas do VOTA (`CAssinadorVota`, o flag de `CSincronizaVota`), como faz sua gêmea `SalvaEstado` (491), então `comum`/`cappinfo.cpp` é incerto: `vota/comum/csincronizavota.cpp` é um lar igualmente bom |
| registro de ajuste do relógio | `CEstadoGeral::SetAjusteDataHora` 3705 | `EstadoGeralUrna.ajusteDataHora` (+52: tipo, valor) |
| hora local → time_t | `CDateTime::ToTimeT` 1381 | `timegm` da data/hora civil (a urna mantém a hora local "como UTC") |
| número de vias do BU | `CEstadoGeralVota::IncrementaQtdBU` 3701 | `qtdBU` do `vota.bin` (+8, um byte, embora o ASN.1 permita 0..999) |
| marcador de impressão | `CRelVotaUtil::MarcaImpressaoEmAndamento` 1488 / `RemoveMarcaImpressaoEmAndamento` 1487 | cria (`CFile "wb"`) / remove de forma segura `/dsk/fi/dinamico/imprimindo` em torno de cada trabalho de impressão, cada um seguido de `sync`. Só 1487/1488 referenciam o caminho, e a única leitura é o próprio teste de existência da 1487 antes da remoção |
| mídia de resultado | `IInterfaceInit::HabilitarMR` 2863 | comando 10 do init-daemon "habilitando MR", depois `sleep 500 ms` |
| diretórios WSQ | 5819 / 5818 / 3795 | `<MI>/dinamico/trab<turno>/wsq/{habilitado,nao-habilitado,operador}/`: imagens de impressões digitais de eleitores habilitados, de eleitores não habilitados e de mesários, empacotadas depois em `wsqbio.jez`, `wsqman.jez`, `wsqmes.jez` |
| parâmetros do PU lidos | 1165, 3229, 5918, 11666-11669 | `tempoConfirmacaoVoto` (+88), `tempoDispararSuspensao(TE)` (+80/+84), `aceitarJustificativa` (+394), `numRelatorio*` (+56…+68) |

---

## 9. BOLETIM DE URNA: as partes do encerramento que vivem nesta unidade

O fluxo do BU em si (geração `CGeraBU`, impressão `CImprimindoBU::ImprimeBU`, arquivos de resultado `CGravaResultado`, cópia
para a MR, QR codes) está documentado passo a passo nas unidades u08/u09 (`docs/modules/u09-…md` §4-5) e em
`docs/10-boletim-de-urna.md`. Esta unidade contribui com as seguintes peças, na ordem do fluxo:

1. **Roteamento na reinicialização.** Depois de um reboot, `CAjusteInicial::InicioVota` mapeia `EstadoGeralVota.estadoVota` para um estado:
   60 → `CGeraRelatorios`, **61 → `CInicioBU::GetInst` (5983)**, **62 → `CGravaResultado::GetInst` (6037)**,
   63 → `CCopiaResultadoParaMR`. Para 64 (encerrada), olha `estadoEncerramento`: '1'/'2' →
   **`CImprimirBUOutrasObrigatorias::GetInst` (5984)**, '3' → `CRetirarMR`, '4' → `CVerificaQtdBUsAdicionais`.
   Qualquer outro valor registra no log **"Erro estado do aplicativo não conhecido" (3279)** e lança 9303/9304.
2. **Primeira via.** `CGeraRelatorios` → `CInicioBU` → `CImprimindoBU`. Em torno de cada trabalho de impressão:
   `MarcaImpressaoEmAndamento` (1488) cria `dinamico/imprimindo`, `CSigVerifier(trab, "bu.dat", "bu.vsu")`
   (1540) permite que o serviço de impressão verifique a assinatura da imagem antes de imprimir, depois
   `RemoveMarcaImpressaoEmAndamento` (1487). Quando o mesário aceita a qualidade, `qtdBU` é incrementado
   (**3701**) e o `vota.bin` é salvo. `CImprimindoBU` então vai para `CEmitirMaisBU::GetInst` (3879, treinamento
   de eleitor) ou `CGravaResultado::GetInst` (6037).
3. **Arquivos de resultado → MR.** `CCopiaResultadoParaMR::CopiaResultado` chama **`HabilitarMR` (2863)**: comando 10
   "habilitando MR" para o init daemon, depois um sleep de 500 ms. Se o pendrive estiver ausente, registra no log **"Mídia de resultado
   não estava presente" (5885)**. Continua para **`CImprimirBUOutrasObrigatorias` (5984)**.
4. **Vias obrigatórias.** Para cada via restante até `numBUVotaObrigatorios` (1 no modo demonstração), executa 1488 →
   impressão → 1487 e **3701**. Depois **`ImprimeBoletimJustificativa` (5918)** (PU `aceitarJustificativa`, *sem* override
   de demonstração) decide se o "boletim de justificativa" (`buj.dat`) é impresso. Em seguida vem o relatório dos
   mesários `CImprimindoBim::GetInst` (**5985**, quando mesários foram identificados), depois `CRetirarMR`.
5. **Vias extras.** `CEmitirMaisBU` imprime vias adicionais até `numBUVotaObrigatorios + numBUVotaAdicionais`
   (**3701** para cada uma). Além disso, `CLimiteCopiasBUAtingido` / `CEmitirMaisBU` registram no log **"Quantidade de vias
   adicionais excede o máximo permitido" (4556, aviso)**.
6. **QR codes na tela.** **`CMostraQRCodeBU::GetInst` (2888)** mostra os QR codes do BU como `CImageField`s criados
   pela **5547** (parâmetros wasm `(image by value, position)`, âncora 1 constante). `CMostraQRCodeCertificado` usa o mesmo builder para
   o QR code do certificado.

Dados exatos: `qtdBU` é `EstadoGeralVota.qtdBU` (`vota.bin`, byte +8 em memória, INTEGER 0..999 em ASN.1). O
marcador é `/dsk/fi/dinamico/imprimindo` (arquivo vazio). O par de assinatura é `trab/bu.dat` + `trab/bu.vsu`
(`CSigVerifier` +4 ESavdAplic 1, +8 diretório, +20 arquivo, +32 assinatura). O id do comando da MR é 10, com o texto
"habilitando MR" (11 "desabilitando MR" está inlinado em `CAjusteInicial`).

---

## 10. Outros membros reconstruídos aqui

* `comum::md::CValidadorIdentidade::EhValida(tipo, identidade)` (2803): a gêmea que não lança de `Valida`. Usa
  a mesma busca de regra (a primeira regra da lista que seja a regra de "identificador livre", tipo 3, ou do
  tipo pedido) e retorna false quando não existe
  regra. Usada para verificar os títulos digitados para suspender um voto, para encerrar a votação e para registrar um mesário.
* `comum::CEleitores::GetBiometriaCorrente()` (1393): uma cópia não inline de `GetCurrent().GetBiometria()`, usada por
  seis estados do operador.
* `comum::CInfoMTLCD::ExibeBateria()` (5903): reanexa o LCD do MT ao observable do ícone de bateria somente se ele
  ainda não estiver anexado, e envia o ícone corrente imediatamente.
* `api::CProgressBar::SetValor` (3667): reconstruída pela unidade u16 (`cprogressbar.cpp`), listada aqui para o mapa.
* Código de biblioteca: 2125 (deleter de nós do `std::map` da tabela de ticks), 2875 (insert de intervalo em `vector<shared_ptr<T>>`,
  dobrado por identical-code folding com instanciações do RHVoice), 1286 (`unique_ptr<IConfereVotoEmCargo>::reset`).

---

## 11. Particularidades do build web

* A página web nunca sai do laço de votação do eleitor, então nenhuma das funções de encerramento, zerésima, menu, mesário ou
  impressão digital acima executa no simulador. As duas exceções são a construção, na inicialização, do formulário "Mais
  informações" e o encanamento do laço de votação (§3, §6).
* `HabilitarMR` (2863) é compilada como `if (byte @1584624 == 1) emscripten_sleep(500)`. O byte é 1 e o módulo
  não tem Asyncify, então alcançá-la **aborta** o programa. Ela é inalcançável a partir da página.
* `GravaEstadoGeral` chamaria o mock web do SAVD: as "assinaturas" são o texto `assinatura simulada para
  vota_web_wasm`.
* O tempo vem do navegador: `gettimeofday` no poller de ticks, `timegm` em `ToTimeT`. O
  `CWasmSystemDateTime` web não consegue mover o relógio do JS, então os saltos de relógio da urna real não acontecem.

## 12. Observações sobre Wasm / Emscripten

* O **merge-similar-functions** criou os corpos parametrizados 764 (vtable + flags), 2902 / 6024 / 3890 (vtables),
  3887 / 3888 / 6017 / 3902 / 6115 (pedaços de string ou strings de formatação) e 2921 (prioridade + cache por TU).
* **Identical code folding**: a 2875 é compartilhada com instanciações do RHVoice, o que explica os chamadores "rhvoice".
  O corpo da 2937, `shared_f1727`, é compartilhado por toda classe cujo destrutor só destrói uma string em +8.
* **Entradas órfãs da tabela**: os 17 destrutores at-exit são inalcançáveis. Seus registros desapareceram com o
  `__cxa_atexit` no-op de um build sem `EXIT_RUNTIME`, mas os slots da tabela foram mantidos. A ordem deles na tabela ainda é
  útil: os stubs at-exit de cada arquivo ficam logo antes do primeiro slot de vtable da classe desse arquivo. É
  assim que §4.3 situa 7790/7798. Os próprios corpos at-exit são mesclados, com o endereço do static como parâmetro:
  `shared_f349` / `shared_f389` / `vota_f1286`.
* **`shared_ptr` é trivial_abi** (ABI v2 da libc++): um `shared_ptr` passado por valor vai como ponteiro para a cópia do
  chamador e é **liberado pela função chamada**. Exemplos: 5547, que libera seu argumento de imagem, e 426
  (`CFormBuilder::Add`).
* Os **mutexes static locais de função** sobrevivem apenas como o stub de unlock `std::mutex::unlock` (func 150), chamado sobre
  o endereço do static.
* Três casos de **computação morta deixada pelo código-fonte**: a string de detalhe não usada de `GravaEstadoGeral`, o
  resultado não usado de `ConfiguracaoExtrator::GetInst()` em `CControlaReconhecimento` e a
  ida e volta `std::to_string`/`std::stoi` do ano em `FormataData`.

---

## 13. Correções às reconstruções de outras unidades

| onde | correção |
|---|---|
| `src/uenux2/src/app/comum/cmenubase.u02.cpp` (2285) | o id da opção só é acrescentado quando `item->Disponivel()` (slot 2), e não sempre que `item` é não nulo; `Monta` acinzenta os itens indisponíveis (cor 5) |
| `src/uenux2/src/app/vota/eleitor/cconferevotoemcargo.h` | o construtor recebe apenas `ETelaVotacao`; `m_comTick` = `tempoConfirmacaoVoto > 0` |
| chamadores de `comum::GravaEstadoGeral` (u06/u10) | as proteções da 3333 recebem uma string de detalhe **vazia** (diferente de `SalvaEstado`, 491) |
| `src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp` | `CSincronismoOperador` está mais provavelmente em `operador/` do que em `operador/aguardaeleitor/` (layout dos statics) |
| `src/uenux2/src/app/comum/relatorios/cgeradorrelpu.h` | a u37 sobrescreveu por acidente a primeira versão deste header feita pela u35, porque as duas unidades rodaram em paralelo. O arquivo agora contém uma versão mesclada, que mantém a declaração do destrutor não inline da u35 |
| `src/uenux2/src/app/vota/eleitor/cajusteinicial.cpp` (7160) | (revisão da u37, aplicada) `CAjusteInicial::StartState` empilha `CControladorRegistraMesariosVota` quando `CInformacaoEleicao::IdentificaMesarios()` (func 1950 = `!demo && PU.registrarMesarios`, +400) é verdadeiro, e não quando `EhModoDemonstracao()` é verdadeiro. O controlador de registro de mesários do VOTA, portanto, executa em urnas reais e nunca no modo demonstração |
| `src/uenux2/src/api/gui/capplicationcontextstack.u15.h` | (revisão da u37, não aplicada) `CApplicationContext +48` (lá `bool m_generico`) é armazenado (5557), copiado (1841) e comparado (5556) como um valor de 32 bits: mais provavelmente um `int`/enum do que um `bool` |
| `src/uenux2/src/app/comum/citemmenu.h` / `cmenubase.u02.cpp` | (revisão da u37, aplicada a citemmenu.h) `CItemMenu +24` é um único `api::SFont {20, 0}` (8 bytes, como o "SFonte" da u02), e não dois ints; o texto é copiado (referência const) |

---

## 14. Código suspeito ou estranho

| # | função | constatação | quem é afetado | gravidade |
|---:|---|---|---|:-:|
| 1 | 2863 `IInterfaceInit::HabilitarMR` | `emscripten_sleep(500)` atrás do flag sempre verdadeiro @1584624 em um build sem Asyncify: alcançá-lo aborta o wasm. Chamadores: a limpeza da MR de `CAjusteInicial` (fase de treinamento) e a cópia do encerramento para a MR | apenas o simulador, e só se uma página futura acionasse o encerramento (inalcançável hoje); uma urna real apenas espera 500 ms | baixa |
| 2 | 5450 `CTickManager::GetTicksExpirados` | o período é convertido para microssegundos em um `int` de 32 bits (`intervalo * 1000`): um tick mais longo que 2 147 s (35,8 min) estoura (UB; dá a volta em wasm) e recebe uma próxima expiração errada. Períodos perdidos são colapsados em um único `ProcessTick`, e o relógio é o relógio de parede (`gettimeofday`), não um monotônico, então uma mudança do relógio para trás atrasa todo tick em execução até o relógio alcançar | urna real (ajustes de relógio por `CAjusteInicial`/`CIniciodeCiclo` em treinamento/demonstração). Nenhum parâmetro válido chega ao estouro. Os ticks criados são de 1 s, 2 s, `tempoConfirmacaoVoto` (ASN.1 0..99 999 ms; 1 s nos cenários) e `tempoDispararSuspensao(TE)` (15..360 s; 45 s nos cenários). Todos estão muito abaixo de 2 147 s | baixa |
| 3 | 3333 `GravaEstadoGeral` | calcula o texto de detalhe do contexto de erro corrente (ou "Erro de sincronização") e nunca o usa: as duas proteções recebem um detalhe vazio, diferente de sua gêmea `SalvaEstado` (491). Uma falha de gravação do eg.bin, portanto, mostra uma tela de erro fatal sem a linha de detalhe | urna real, apenas no caminho de erro | info |
| 4 | 3887 / 6017 | o **título de eleitor** do mesário vai em claro para o log de eventos `logd.dat` ("Mesário {} registrado", "... é eleitor da seção", "Pedido de leitura da biometria do mesário {}"). Os logs da urna estão entre os arquivos que saem da urna (log.jez na MR, `CGravadorLog`). Não foi verificado se o TSE os filtra antes de publicar | privacidade dos mesários em urnas **reais**. `CAjusteInicial` instala este controlador somente quando `IdentificaMesarios()` é verdadeiro (func 1950: fora do modo demonstração e com o PU `registrarMesarios`, que é TRUE nos cenários publicados). Ele nunca executa no modo demonstração nem no build web | baixa |
| 5 | 3701 `IncrementaQtdBU` | `qtdBU` é `INTEGER (0..999)` em ASN.1, mas um byte sem verificação em C++ | ninguém na prática (as vias são limitadas pelos parâmetros do PU) | info |
| 6 | 5918 `ImprimeBoletimJustificativa` | o único getter de `CInformacaoEleicao` sem o override do modo demonstração: uma urna de demonstração imprime o boletim de justificativa conforme o valor real do PU | urnas de demonstração; provavelmente intencional | info |
| 7 | 2687 `FormataData` | um `ptime` especial (not_a_date_time/±infinity, por exemplo uma janela de horário ausente na configuração) chega a `from_day_number`, que lança `boost::gregorian::bad_year`, não capturada em `CImpressaoPU`. O ano também passa por uma ida e volta `to_string`/`stoi` inútil | relatório "Parâmetros de urna" de uma urna real com configuração malformada | info |
| 8 | 1165 `IConfereVotoEmCargo` | por projeto, CONFIRMA pressionado durante a conferência (PU `tempoConfirmacaoVoto`, 1 s) é descartado como "Tecla indevida pressionada". O descarte não é silencioso: `PlayKey(0)` emite o bipe de erro e grava o registro de log (unidade u06). Um eleitor que pressiona CONFIRMA uma vez, cedo demais, vê a tela de confirmação e precisa pressioná-la de novo; nada é registrado até lá. Reverificado com `headless.mjs --keys "91001C  C  12  C  "`: o primeiro C deixa o estado em `CConfereVotoEmCargo<…>`, e o C depois da pausa confirma | eleitores (apenas UX: nenhum voto é registrado sem o segundo CONFIRMA) | info |
| 9 | 7790 / 7798 | destrutores at-exit de um singleton cujo acessor não existe mais (statics @1832908/@1832932 nunca referenciados; pelo layout da tabela e dos dados, provavelmente na unidade de tradução de `CAssinadorVota`/`CSincronizaVota`, ver §4.3); todos os 17 destrutores at-exit da unidade são inalcançáveis | ninguém (código morto) | info |
| 10 | 1487 / 1488 | o marcador `dinamico/imprimindo` é gravado e removido pelo VOTA. Nenhum outro código deste binário o lê: a única leitura é o próprio teste de existência da 1487 antes da remoção. Seu consumidor, presumivelmente a detecção de uma impressão interrompida por falta de energia, está fora deste módulo | ninguém no simulador | info |

---

## 15. Tabela de mapeamento (todas as 89 funções)

✓ = observada em execução nos votos gravados; (✓) = não amostrada, mas deve ter executado porque um chamador amostrado
a chama incondicionalmente. "helper de biblioteca/inlinado" = não é código-fonte do TSE (gerado pelo compilador ou
uma instanciação da libc++), com o motivo.

| wasm | tamanho | executou | símbolo reconstruído | arquivo original | reconstruído em |
|---:|---:|:-:|---|---|---|
| 677 | 223 |  | `comum::CGeradorRelPU::AdicionaLinha` | uenux2/src/app/comum/relatorios/cgeradorrelpu.cpp (caminho inferido) | src/uenux2/src/app/comum/relatorios/cgeradorrelpu.u37.cpp |
| 764 | 71 | (✓) | `vota::ObtemEstadoSimples<T> [merged GetInst body of 12-byte states]` | corpo mesclado: o GetInst() de cada estado de 12 bytes, um por arquivo de estado sob uenux2/src/app/vota/ | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 1165 | 91 | ✓ | `vota::IConfereVotoEmCargo::IConfereVotoEmCargo` | uenux2/src/app/vota/eleitor/cconferevotoemcargo.cpp | src/uenux2/src/app/vota/eleitor/cconferevotoemcargo.u37.cpp |
| 1256 | 109 |  | `vota::CControlaReconhecimento::GetInst` | uenux2/src/app/vota/operador/confirmaidentidade/ccontrolareconhecimento.cpp | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 1279 | 139 |  | `vota::CVisualizarCandidatos::GetInst` | uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cvisualizarcandidatos.cpp | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 1286 | 29 |  | `std::unique_ptr<vota::IConfereVotoEmCargo>::reset [at-exit body]` | helper de biblioteca/inlinado (unique_ptr da libc++, instanciado em uenux2/src/app/vota/eleitor/votamajoritario, votaproporcional) | src/uenux2/src/app/vota/eleitor/cconferevotoemcargo.u37.cpp (comentário) |
| 1381 | 132 |  | `api::CDateTime::ToTimeT` | uenux2/src/api/util/cdatetime.cpp | src/uenux2/src/api/util/cdatetime.u37.cpp |
| 1393 | 15 |  | `comum::CEleitores::GetBiometriaCorrente` | uenux2/src/app/comum/dados/celeitores.cpp | src/uenux2/src/app/comum/dados/celeitores.u37.cpp |
| 1487 | 587 |  | `vota::CRelVotaUtil::RemoveMarcaImpressaoEmAndamento` | uenux2/src/app/vota/comum/crelvotautil.cpp (caminho inferido) | src/uenux2/src/app/vota/comum/crelvotautil.u37.cpp |
| 1488 | 405 |  | `vota::CRelVotaUtil::MarcaImpressaoEmAndamento` | uenux2/src/app/vota/comum/crelvotautil.cpp (caminho inferido) | src/uenux2/src/app/vota/comum/crelvotautil.u37.cpp |
| 1540 | 123 |  | `comum::CSigVerifier::CSigVerifier` | uenux2/src/app/comum/relatorios/csigverifier.cpp | src/uenux2/src/app/comum/relatorios/csigverifier.h (unidade u25, inline) |
| 1841 | 351 | ✓ | `api::CApplicationContext::CApplicationContext(const CApplicationContext&)` | uenux2/src/api/gui/capplicationcontextstack.cpp | src/uenux2/src/api/gui/capplicationcontextstack.u37.cpp |
| 1897 | 98 |  | `vota::CSincronismoOperador::GetInst` | uenux2/src/app/vota/operador/csincronismooperador.cpp (caminho inferido) | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 1903 | 102 |  | `ConfiguracaoExtrator::GetInst` | uenux2/src/app/vota/operador/confirmaidentidade/ccontrolareconhecimento.cpp (caminho inferido) | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 2125 | 32 |  | `std::__tree<std::__value_type<uebyte, api::STick>>::destroy` | helper de biblioteca/inlinado (<map> da libc++ para api::CTickManager) | src/uenux2/src/app/vota/u37-foreign-fragments.cpp (comentário) |
| 2282 | 14 |  | `comum::IEventosLog::LogaErro` | uenux2/src/app/comum/log/ieventoslog.h | src/uenux2/src/app/vota/log/clogvota.u37.cpp |
| 2687 | 983 |  | `vota::(anonymous namespace)::FormataData` | uenux2/src/app/vota/operador/outrasopcoes/cimpressaopu.cpp (caminho inferido) | src/uenux2/src/app/vota/operador/outrasopcoes/cimpressaopu.u37.cpp |
| 2803 | 114 |  | `comum::md::CValidadorIdentidade::EhValida` | uenux2/src/app/comum/dados/md/cvalidadoridentidade.cpp | src/uenux2/src/app/comum/dados/md/cvalidadoridentidade.u37.cpp |
| 2863 | 135 |  | `comum::IInterfaceInit::HabilitarMR` | uenux2/src/app/comum/iinterfaceinit.cpp (caminho inferido) | src/uenux2/src/app/comum/iinterfaceinit.u37.cpp |
| 2875 | 1093 |  | `std::vector<std::shared_ptr<T>>::__insert_with_size` | helper de biblioteca/inlinado (<vector> da libc++, dobrado por identical-code folding para vários T) | src/uenux2/src/app/vota/u37-foreign-fragments.cpp (comentário) |
| 2888 | 104 |  | `vota::CMostraQRCodeBU::GetInst` | uenux2/src/app/vota/eleitor/fimvotacao/cmostraqrcodebu.cpp | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 2902 | 63 |  | `[merged destructor body: vptr + shared_ptr<+12> release]` | helper de biblioteca/inlinado (~X() gerado pelo compilador de IEleitorImpedidoVotar, IConfirmaJustificativa, CGeraZeresimaBase, CGeraResumoZeresimaBase) | src/uenux2/src/app/vota/u37-foreign-fragments.cpp (comentário) |
| 2937 | 12 | ✓ | `comum::CItemMenu::~CItemMenu` | uenux2/src/app/comum/citemmenu.h (caminho inferido) | src/uenux2/src/app/comum/citemmenu.h |
| 3229 | 586 | ✓ | `vota::CEleitorVotando::GetInst` | uenux2/src/app/vota/eleitor/celeitorvotando.cpp | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 3279 | 150 |  | `vota::CLogVota::LogaErroEstadoDesconhecido` | uenux2/src/app/vota/log/clogvota.cpp | src/uenux2/src/app/vota/log/clogvota.u37.cpp |
| 3333 | 1260 |  | `comum::GravaEstadoGeral` | uenux2/src/app/comum/appinfo/cappinfo.cpp (caminho inferido) | src/uenux2/src/app/comum/appinfo/cappinfo.u37.cpp |
| 3667 | 112 |  | `api::CProgressBar::SetValor` | uenux2/src/api/gui/cprogressbar.cpp | src/uenux2/src/api/gui/cprogressbar.cpp (unidade u16) |
| 3701 | 15 |  | `comum::md::estadoaplicacao::CEstadoGeralVota::IncrementaQtdBU` | uenux2/src/app/comum/dados/md/estadoaplicacao/cestadogeralvota.cpp | src/uenux2/src/app/comum/dados/md/estadoaplicacao/estadoaplicacao.u37.cpp |
| 3705 | 12 |  | `comum::md::estadoaplicacao::CEstadoGeral::SetAjusteDataHora` | uenux2/src/app/comum/dados/md/estadoaplicacao/cestadogeral.cpp | src/uenux2/src/app/comum/dados/md/estadoaplicacao/estadoaplicacao.u37.cpp |
| 3795 | 15 |  | `vota::DiretorioWsqOperador` | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp (caminho inferido) | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 3842 | 23 |  | `comum::CAppStateContext::AceitaMensagens` | uenux2/src/app/comum/cappstate.cpp (caminho inferido) | src/uenux2/src/app/comum/cappstate.u37.cpp |
| 3843 | 27 | ✓ | `comum::CAppStateContext::ProcessMessage` | uenux2/src/app/comum/cappstate.cpp (caminho inferido) | src/uenux2/src/app/comum/cappstate.u37.cpp |
| 3864 | 22 |  | `vota::CReinicioComparecimentoMesario::GetInst` | uenux2/src/app/vota/eleitor/creiniciocomparecimentomesario.cpp (caminho inferido) | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 3879 | 22 |  | `vota::CEmitirMaisBU::GetInst` | uenux2/src/app/vota/eleitor/fimvotacao/cemitirmaisbu.cpp | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 3887 | 458 |  | `vota::CControladorRegistraMesariosVota::LogaTituloMesario [merged body of slots 31-33]` | uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp (caminho inferido) | src/uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.u37.cpp |
| 3888 | 151 |  | `vota::CControladorRegistraMesariosVota::Loga47 [merged body of slots 29, 34, 36]` | uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp (caminho inferido) | src/uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.u37.cpp |
| 3890 | 372 |  | `api::CFormBuilderBase<MEDIA>::AddCampo<FIELD>(arg) [merged body]` | helper de biblioteca/inlinado (template de uenux2/src/api/gui/cformbuilder.h) | src/uenux2/src/api/gui/u37-template-instances.cpp |
| 4550 | 138 |  | `vota::CLogVota::LogaErroPartidoNaoEncontrado` | uenux2/src/app/vota/log/clogvota.cpp | src/uenux2/src/app/vota/log/clogvota.u37.cpp |
| 4556 | 35 |  | `vota::CLogVota::LogaQtdViasExcedeMaximo` | uenux2/src/app/vota/log/clogvota.cpp | src/uenux2/src/app/vota/log/clogvota.u37.cpp |
| 5342 | 22 |  | `vota::CFinalizaOperador::GetInst` | uenux2/src/app/vota/operador/cfinalizaoperador.cpp (caminho inferido) | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 5450 | 625 | ✓ | `api::CTickManager::GetTicksExpirados` | uenux2/src/api/util/ctickmanager.cpp | src/uenux2/src/api/util/ctickmanager.u37.cpp |
| 5547 | 166 |  | `api::CFormBuilder::Add<api::CImageField>(shared_ptr<CQRCodeImage> [by value], const SPoint&) [anchor 1 constant]` | helper de biblioteca/inlinado (template de uenux2/src/api/gui/cformbuilder.h) | src/uenux2/src/api/gui/u37-template-instances.cpp |
| 5553 | 204 | ✓ | `api::CApplicationContextStack::Remove` | uenux2/src/api/gui/capplicationcontextstack.cpp | src/uenux2/src/api/gui/capplicationcontextstack.u37.cpp |
| 5818 | 15 |  | `vota::DiretorioWsqNaoHabilitado` | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp (caminho inferido) | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 5819 | 15 |  | `vota::DiretorioWsqHabilitado` | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp (caminho inferido) | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 5881 | 29 |  | `vota::CLogVota::LogaEstadoNaoEsperado` | uenux2/src/app/vota/log/clogvota.cpp | src/uenux2/src/app/vota/log/clogvota.u37.cpp |
| 5885 | 29 |  | `vota::CLogVota::LogaMRNaoPresente` | uenux2/src/app/vota/log/clogvota.cpp | src/uenux2/src/app/vota/log/clogvota.u37.cpp |
| 5895 | 16 |  | `comum::(anonymous namespace)::LogInfo` | uenux2/src/app/comum/iinterfaceinit.cpp | src/uenux2/src/app/comum/iinterfaceinit.u37.cpp |
| 5903 | 112 |  | `comum::CInfoMTLCD::ExibeBateria` | uenux2/src/app/comum/cinfomtlcd.cpp | src/uenux2/src/app/comum/cinfomtlcd.u37.cpp |
| 5908 | 23 |  | `comum::CAppStateContext::AceitaTicks` | uenux2/src/app/comum/cappstate.cpp (caminho inferido) | src/uenux2/src/app/comum/cappstate.u37.cpp |
| 5909 | 23 | ✓ | `comum::CAppStateContext::AceitaTeclado` | uenux2/src/app/comum/cappstate.cpp (caminho inferido) | src/uenux2/src/app/comum/cappstate.u37.cpp |
| 5910 | 27 | ✓ | `comum::CAppStateContext::ProcessTick` | uenux2/src/app/comum/cappstate.cpp (caminho inferido) | src/uenux2/src/app/comum/cappstate.u37.cpp |
| 5911 | 25 | ✓ | `comum::CAppStateContext::ProcessInput` | uenux2/src/app/comum/cappstate.cpp (caminho inferido) | src/uenux2/src/app/comum/cappstate.u37.cpp |
| 5918 | 11 |  | `comum::CInformacaoEleicao::ImprimeBoletimJustificativa` | uenux2/src/app/comum/informacao/cinformacaoeleicao.cpp | src/uenux2/src/app/comum/informacao/cinformacaoeleicao.u37.cpp |
| 5947 | 212 |  | `vota::CVerificaHorarioZeresima::GetInst` | uenux2/src/app/vota/eleitor/iniciovotacao/cverificahorariozeresima.cpp | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 5948 | 28 |  | `vota::testeteclado::CBase::CBase` | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cbase.cpp (caminho inferido) | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 5951 | 19 |  | `vota::CVisualizarCandidatos::GetCargo` | uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cvisualizarcandidatos.h | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 5954 | 104 |  | `vota::CMenuFiltrarCandidatosPorNumero::GetInst` | uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatospornumero.cpp (caminho inferido) | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 5983 | 22 |  | `vota::CInicioBU::GetInst` | uenux2/src/app/vota/eleitor/fimvotacao/ciniciobu.cpp (caminho inferido) | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 5984 | 22 |  | `vota::CImprimirBUOutrasObrigatorias::GetInst` | uenux2/src/app/vota/eleitor/fimvotacao/cimprimirbuoutrasobrigatorias.cpp | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 5985 | 22 |  | `vota::CImprimindoBim::GetInst` | uenux2/src/app/vota/eleitor/fimvotacao/cimprimindobim.cpp | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 6017 | 460 |  | `vota::CControladorRegistraMesariosVota::LogaComTitulo [merged body of slots 19, 27]` | uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp (caminho inferido) | src/uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.u37.cpp |
| 6024 | 365 | ✓ | `api::IForm<MEDIA>::IForm(campos, preShow) [merged body]` | helper de biblioteca/inlinado (template de uenux2/src/api/gui/iform.h) | src/uenux2/src/api/gui/u37-template-instances.cpp |
| 6037 | 22 |  | `vota::CGravaResultado::GetInst` | uenux2/src/app/vota/eleitor/fimvotacao/cgravaresultado.cpp | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 6188 | 162 |  | `comum::CMenuBase::~CMenuBase (deleting)` | uenux2/src/app/comum/cmenubase.cpp (caminho inferido) | src/uenux2/src/app/comum/cmenubase.u37.cpp |
| 6587 | 130 |  | `vota::CTelasVota::CriaTelaAguarde` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/u37-foreign-fragments.cpp |
| 7116 | 10 |  | `vota::CThreadEleitor::GetInst()::s_mutex [atexit ~mutex]` | uenux2/src/app/vota/eleitor/cthreadeleitor.cpp | src/uenux2/src/app/vota/u37-foreign-fragments.cpp (tabela) |
| 7175 | 10 |  | `vota::impl::ISincronismoVotoEleitor::GetInst()::s_mutex [atexit ~mutex]` | uenux2/src/app/vota/eleitor/csincronismovotoeleitor.cpp | src/uenux2/src/app/vota/u37-foreign-fragments.cpp (tabela) |
| 7187 | 10 |  | `vota::CSincronismoEleitor::GetInst()::s_mutex [atexit ~mutex]` | uenux2/src/app/vota/eleitor/csincronismoeleitor.cpp (caminho inferido) | src/uenux2/src/app/vota/u37-foreign-fragments.cpp (tabela) |
| 7195 | 12 |  | `vota::CSincronismoEleitor::GetInst()::s_inst [atexit ~unique_ptr]` | uenux2/src/app/vota/eleitor/csincronismoeleitor.cpp (caminho inferido) | src/uenux2/src/app/vota/u37-foreign-fragments.cpp (tabela) |
| 7202 | 10 |  | `vota::CFimVotoEleitor::GetInst()::s_mutex [atexit ~mutex]` | uenux2/src/app/vota/eleitor/cfimvotoeleitor.cpp | src/uenux2/src/app/vota/u37-foreign-fragments.cpp (tabela) |
| 7212 | 12 |  | `vota::CFimVotoEleitor::GetInst()::s_inst [atexit ~unique_ptr]` | uenux2/src/app/vota/eleitor/cfimvotoeleitor.cpp | src/uenux2/src/app/vota/u37-foreign-fragments.cpp (tabela) |
| 7232 | 10 |  | `vota::CMostraTelaContinuaVotacao::GetInst()::s_mutex [atexit ~mutex]` | uenux2/src/app/vota/eleitor/cmostratelacontinuavotacao.cpp (caminho inferido) | src/uenux2/src/app/vota/u37-foreign-fragments.cpp (tabela) |
| 7239 | 12 |  | `vota::CMostraTelaContinuaVotacao::GetInst()::s_inst [atexit ~unique_ptr]` | uenux2/src/app/vota/eleitor/cmostratelacontinuavotacao.cpp (caminho inferido) | src/uenux2/src/app/vota/u37-foreign-fragments.cpp (tabela) |
| 7284 | 10 |  | `vota::CInstrucaoVotacaoAcessibilidade::GetInst()::s_mutex [atexit ~mutex]` | uenux2/src/app/vota/eleitor/cinstrucaovotacaoacessibilidade.cpp | src/uenux2/src/app/vota/u37-foreign-fragments.cpp (tabela) |
| 7323 | 10 |  | `vota::CIniciodeCiclo::GetInst()::s_mutex [atexit ~mutex]` | uenux2/src/app/vota/eleitor/ciniciodeciclo.cpp | src/uenux2/src/app/vota/u37-foreign-fragments.cpp (tabela) |
| 7333 | 12 |  | `vota::CIniciodeCiclo::GetInst()::s_inst [atexit ~unique_ptr]` | uenux2/src/app/vota/eleitor/ciniciodeciclo.cpp | src/uenux2/src/app/vota/u37-foreign-fragments.cpp (tabela) |
| 7405 | 10 |  | `vota::CEleitorVotando::GetInst()::s_mutex [atexit ~mutex]` | uenux2/src/app/vota/eleitor/celeitorvotando.cpp | src/uenux2/src/app/vota/u37-foreign-fragments.cpp (tabela) |
| 7470 | 10 |  | `vota::CConfirmaVotoSemCandidato::GetInst()::s_mutex [atexit ~mutex]` | uenux2/src/app/vota/eleitor/cconfirmavotosemcandidato.cpp | src/uenux2/src/app/vota/u37-foreign-fragments.cpp (tabela) |
| 7480 | 18 | ✓ | `comum::CAppState::NeedChangeState` | uenux2/src/app/comum/cappstate.cpp (caminho inferido) | src/uenux2/src/app/comum/cappstate.u37.cpp |
| 7482 | 10 |  | `vota::CAguardaMensagem::GetInst()::s_mutex [atexit ~mutex]` | uenux2/src/app/vota/eleitor/caguardamensagem.cpp (caminho inferido) | src/uenux2/src/app/vota/u37-foreign-fragments.cpp (tabela) |
| 7485 | 12 |  | `vota::CAguardaMensagem::GetInst()::s_inst [atexit ~unique_ptr]` | uenux2/src/app/vota/eleitor/caguardamensagem.cpp (caminho inferido) | src/uenux2/src/app/vota/u37-foreign-fragments.cpp (tabela) |
| 7790 | 10 |  | `vota::<unknown>::GetInst()::s_mutex [atexit ~mutex]` | desconhecido (a TU que emite a vtable de vota::CAssinadorVota e contém o flag de CSincronizaVota @1832936, por exemplo uenux2/src/app/vota/comum/csincronizavota.cpp?) | src/uenux2/src/app/vota/u37-foreign-fragments.cpp (tabela) |
| 7798 | 12 |  | `vota::<unknown>::GetInst()::s_inst [atexit ~unique_ptr]` | desconhecido (a TU que emite a vtable de vota::CAssinadorVota e contém o flag de CSincronizaVota @1832936, por exemplo uenux2/src/app/vota/comum/csincronizavota.cpp?) | src/uenux2/src/app/vota/u37-foreign-fragments.cpp (tabela) |
| 11666 | 56 |  | `comum::CItemVersoesPacotes::Disponivel` | uenux2/src/app/comum/citemversoespacotes.cpp (caminho inferido) | src/uenux2/src/app/comum/citemversoespacotes.cpp |
| 11667 | 56 |  | `comum::CItemParametrosUrna::Disponivel` | uenux2/src/app/comum/citemparametrosurna.cpp (caminho inferido) | src/uenux2/src/app/comum/citemparametrosurna.cpp |
| 11668 | 106 |  | `comum::CItemImprimeListaEleitores::Disponivel` | uenux2/src/app/comum/citemimprimelistaeleitores.cpp (caminho inferido) | src/uenux2/src/app/comum/citemimprimelistaeleitores.cpp |
| 11669 | 56 | ✓ | `comum::CItemImprimeEstadoUrna::Disponivel` | uenux2/src/app/comum/citemimprimeestadourna.cpp (caminho inferido) | src/uenux2/src/app/comum/citemimprimeestadourna.cpp |
| 12235 | 159 |  | `comum::CMenuBase::~CMenuBase` | uenux2/src/app/comum/cmenubase.cpp (caminho inferido) | src/uenux2/src/app/comum/cmenubase.u37.cpp |
