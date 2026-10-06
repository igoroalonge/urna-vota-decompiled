# u27 — `uenux2/src/app/vota/operador`: a thread do operador, a identificação do eleitor, as "outras opções" e o início do encerramento

A unidade u27 cobre **97 funções wasm** que as ferramentas atribuíram a nove arquivos originais do
**lado do operador** da aplicação VOTA:

```
uenux2/src/app/vota/operador/cthreadoperador.cpp
uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp
uenux2/src/app/vota/operador/justificativa/{iconfirmajustificativa,iiniciajustificativa}.cpp
uenux2/src/app/vota/operador/leidentidade/{celeitorencontrado,cpedeidentidade}.cpp
uenux2/src/app/vota/operador/outrasopcoes/{caguardaeleitoresvotarem,cencerramentohorarioinvalido,cescolheopcao}.cpp
```

É o próprio corpo da thread do operador (`CThreadOperador::Run`), a tela ociosa do terminal do
mesário (`CPedeIdentidade`, "Digite o Título ou o CPF"), a busca da identidade digitada no cadastro da seção
e a decisão sobre impedimentos e justificativas (`CProcuraEleitor`, `CEleitorEncontrado`,
`IEleitorImpedidoVotar`, `IIniciaJustificativa`, `IConfirmaJustificativa`), o menu "outras opções"
(`CEscolheOpcao`: áudio, **encerrar votação**, registrar mesários, contadores biométricos), os primeiros estados do
**encerramento** pelo operador e a liberação de último recurso de um eleitor com a
**própria impressão digital do mesário** (`CRegistraDigitalOperador`). Cerca de um terço das 97 funções são instâncias
de template do form builder, detalhes internos de `std::sort`/`std::map` da libc++ e pequenas chamadas postas fora de linha.

**1 das 97 funções rodou durante os votos registrados**: `api_f1055` (`std::filesystem::__status`, alcançada
a partir de código de inicialização: `std::filesystem::create_directories` (func 2548) e
`CPacoteArquivos::ValidarChaveEAplicacaoValida` (func 4625), segundo os arquivos de arestas de runtime). Todo o resto roda na thread do operador, que o build web nunca inicia (§2).

Arquivos-fonte reconstruídos (todos em `src/uenux2/src/app/vota/operador/`):

| arquivo | conteúdo |
|---|---|
| `cthreadoperador.{h,cpp}` | `CThreadOperador` (Run, dtor, FinalizaExecucao, LogDebug), `CMessageOperador` |
| `leidentidade/cpedeidentidade.{h,cpp}` | `CPedeIdentidade` (ctor da u17 incorporado, StartState, FinishState, ProcessInput, ProcessTick) |
| `leidentidade/celeitorencontrado.{h,cpp}` | `CEleitorEncontrado::StartState`, log de impedimento, tabela de decisão |
| `leidentidade/ieleitorimpedidovotar.{h,cpp}` *(caminho inferido)* | classe base das seis telas de "não pode votar aqui" + seus singletons |
| `justificativa/iiniciajustificativa.{h,cpp}`, `justificativa/iconfirmajustificativa.{h,cpp}` | os estados de entrada e de confirmação da justificativa |
| `outrasopcoes/cescolheopcao.{h,cpp}` | o menu "outras opções" (o fragmento da u02 `cescolheopcao.u02.cpp` contém os textos dos contadores) |
| `outrasopcoes/cencerramentohorarioinvalido.{h,cpp}` | encerramento solicitado cedo demais (+ salto do relógio em demonstração/treinamento) e o corpo compartilhado de "sair com uma tecla" |
| `outrasopcoes/caguardaeleitoresvotarem.cpp` | "Aguarde até todos os eleitores presentes votarem" |
| `confirmaidentidade/cregistradigitaloperador.{h,cpp}` | liberação pela impressão digital do mesário (ctor da u17 incorporado) |
| `u27-foreign-fragments.cpp` | funções desta unidade cujo arquivo original é outro: `CAguardaInicio::GetInst`, `CProcuraEleitor::StartState`, `CIniciaFinalizacao::StartState`, `CPedeAnoNascimentoSemBiometria::ProcessInput`, vários `GetInst`, `comum::EhTreinamentoEleitor`, `CRegistrarMesarios::GetInst`, `CControladorReconhecimentoMesario::GetInst`, resumos de templates do form builder |

Unidades relacionadas: u10 (o resto de `operador/`: habilitação por digital, `CMostraEleitorVotando`,
`IInformacaoThreadOperador`), u17 e u19 (fragmentos de estados vizinhos, p. ex. `CPedeTituloEncerramento`,
`CValidaIdentidade`), u22 (registro de mesários), u06/u07 (a thread do eleitor), `docs/bu/codepath.md`
(todo o caminho encerramento → BU).

## 1. Glossário (acréscimos ao §1 da u10)

| termo | significado |
|---|---|
| operador / mesário / terminal do mesário (MT) | membro da mesa; seu microterminal: LCD 4×40, teclado numérico com CORRIGE / CONFIRMA, LED, buzzer, sensor de impressão digital. A partir do modelo 2020, o LCD do MT é gráfico e mostra a foto do eleitor |
| identificador / identidade | o número digitado para encontrar o eleitor: título de eleitor (12 dígitos), CPF (11) ou "identificador" |
| seção / caderno | seção eleitoral / o caderno de papel que os eleitores assinam |
| impedimento | motivo pelo qual um eleitor do cadastro não pode votar nesta seção neste turno (`ModuloImpedidos.TipoImpedimento`, por turno P1/P2) |
| justificativa (de ausência) | declaração de que o eleitor não pôde votar na sua própria seção; a urna a registra (`CJustificador`, `jufa.dat`/BUJ) no lugar de um voto |
| trânsito / voto em trânsito | votar temporariamente em outro local (solicitado com antecedência). "TTE" = transferência temporária de eleitor |
| encerramento / encerrar votação | fechamento da votação no fim do dia, feito pelo presidente da mesa no MT; termina no BU |
| HorariosUrna | os quatro horários configurados: `emissaoZeresima`, `inicioVotacao`, `encerramentoVotacao` (horário mais cedo para o encerramento, p. ex. 17:00), `terminoVotacao` (depois dele, sem ninguém votando por 5 min, a votação é bloqueada) |
| inspeção | inspeção aleatória da cabine a cada 60–90 min (u10 §3.1) |
| habilitação por código do mesário | liberação de um eleitor cuja biometria falhou, autorizada pela própria impressão digital do mesário (`ETipoHabilitacao` 2, QR `HBBG`) |
| demonstração / treinamento | modo de demonstração (`IInterfaceInit::GetDemoMode`) / fase de treinamento (fase `'3'` do CEstadoGeral). "Treinamento de eleitores" = fase de treinamento + flag `treinamentoEleitor` (o simulador público); "treinamento de mesários" = fase de treinamento sem essa flag |

## 2. Onde este código roda (build web vs. urna) e como foi verificado

O VOTA tem duas threads de máquina de estados (u06 §2, u10 §2). `CThreadEleitor` conduz o terminal do eleitor e é
avançada por `votaTick` no build web. `CThreadOperador` (esta unidade) conduz o MT. No build web,
nada chama `CThreadOperador::Run` (func 10204): `main` registra `CExecucaoVotaCooperativa`, que só
avança a thread do eleitor. Assim, todo estado desta unidade é **código morto no simulador**; o eleitor é liberado
por `votaInit` (modo de treinamento do eleitor), e as mensagens que a thread do eleitor envia ao operador (6, 9, 13, 1 …)
ficam sem leitura em `CThreadOperador +36` (u10 §2).

**Verificações com harness feitas para esta unidade.** `tools/bu/operator_harness.mjs` roda uma cópia modificada do wasm com
um export `opStep` (uma iteração de `Run`), um relógio falso (padrão 2026-10-04 16:50 -03:00) e
`emscripten_sleep` substituído por stub. Duas execuções (saídas em um diretório temporário; comandos reproduzíveis):

```
node tools/bu/operator_harness.mjs --out <dir> --script "geradin zeresima ot:20 om:8 ot:50 o:D ot:20 o:4C ot:20 \
     o:D ot:20 o:2C ot:30 o:D ot:30 o:123C ot:30 o:D ot:30 o:XXXXXXXXXXXXC ot:50 o:D ot:30"
node tools/bu/operator_harness.mjs --out <dir> --script "geradin zeresima ot:20 om:8 ot:50 \
     clock:2026-10-04T17=05=00-03=00 ot:20 o:D ot:20 o:2C ot:30 o:D ot:30 o:D ot:30 o:1C ot:30 o:D ot:30 o:D ot:30 o:3C ot:30 o:D ot:30"
```

Estados do operador e texto do MT observados (todos conforme reconstruído):

```
CAguardaInicio        "VOTA: 10.23.0.1 / DESENVOLVIMENTO / Siga as instruções na tela do eleitor / 04/10/2026 16:50:00"
  --msg 8-->  CPedeIdentidade  "Digite o Título ou o CPF   16:50 / ____________   0000/0001"
  --D-->      CEscolheOpcao    "Selecione a opção: _  16:50 / 1-Ativar áudio  3-Registrar mesários /
                                2-Encerrar votação  4-Exibir contadores / CORRIGE: retornar  CONFIRMA: prosseguir"
  --4C-->     CContadoresBiometria "Habilitação biométrica: 0000 / Habilitação biográfica: 0000 /
                                    Habilitação sem biometria: 0000 / CORRIGE: retornar"
  --2C at 16:50--> CIniciaFinalizacao -> CEncerramentoHorarioInvalido "Encerramento de votação inválido / Antes do horário"
                   log(2) "Encerramento só pode ser solicitado após as 17:00:00 horas"
                   then THROW 8071 "DeltaT pode ser adicionado apenas para eAlterarDataSistema 0" (§6.1);
                   back on CPedeIdentidade the MT clock reads 16:59 (= 17:00:00 − 10 s)
  --2C at 17:05--> CPerguntaFilaEleitorVazia "Todas as pessoas presentes já votaram?" --D--> CAguardaEleitoresVotarem
                   "Aguarde até todos os / eleitores presentes votarem" -> CPedeIdentidade
  --1C-->     CConfirmaAudio   "Deseja realmente ativar o áudio?  CORRIGE: não  CONFIRMA: sim"
  --123C-->   CValidaIdentidade -> CIdentidadeInvalida "Identidade: 000000000123 / Número errado / CORRIGE: retornar"
  --XXXXXXXXXXXXC--> CValidaIdentidade -> CProcuraEleitor -> CEleitorEncontrado -> CNomeEleitor
                   "NOME OMITIDO / Título: XXXX XXXX XXXX  Seq: 0001 / Seção: 0001 / ..."
logd.dat: "Aguardando digitação do identificador do eleitor", "Operador selecionou: Exibir contadores",
          "Operador selecionou: Encerrar votação", "Identificador do eleitor digitado pelo mesário",
          "Identificador do eleitor digitado inválido", "Identificador digitado pelo mesário foi: (Título de eleitor)"
```

O caminho da digital do mesário (`CRegistraDigitalOperador`) não pode ser executado: nenhuma interface de impressão digital está
registrada no build web, e o código de WSQ/template foi removido na compilação (u10 §6.4).

## 3. Classes e hierarquia (RTTI)

```
api::CThread
└─ vota::CThreadVota
   └─ vota::CThreadOperador            (132 B singleton @1911708; vtable @1601208)
        member vota::CMessageOperador : api::CPriorityMessageQueue<api::SMessage>, api::CMessageInterface  (+36)
api::CState
└─ comum::CAppState                    (+4 m_proximoEstado, +8/+9/+10 accepts messages/keys/ticks)
   ├─ vota::CAguardaInicio              (24 B) first operator state, waits for message 8
   ├─ vota::CPedeIdentidade             (32 B) idle identification screen
   ├─ vota::CValidaIdentidade           (32 B) type of the typed number (u17)
   ├─ vota::CIdentidadeInvalida         (20 B) "Número errado"
   ├─ vota::CProcuraEleitor             (12 B) roll lookup
   ├─ vota::CEleitorEncontrado          (12 B) decision on impediments / already voted / no cargo
   ├─ vota::CEleitorJaVotou             (20 B) "JÁ VOTOU"
   ├─ vota::IEleitorImpedidoVotar       (24 B) base of the "cannot vote here" screens:
   │    ├─ CEleitorNaoEncontrado, CEleitorOptouPorVotarEmTransito, CEleitorImpedidoJustificar,
   │    │  CEleitorNaoTemIdadeMinima, CEleitorNaoPossuiCargosParaVotar, CEleitorImpedidoJustificarVotoTransito
   ├─ vota::CEleitorImpedidoJustificarVotoCPF (20 B)
   ├─ vota::IIniciaJustificativa        (16 B) ─ CIniciaJustificativa / …Temporario / …Transito
   ├─ vota::IConfirmaJustificativa      (20 B) ─ CConfirmaJustificativa / …Temporario / …Transito
   ├─ vota::CJustificativaEfetuada      (32 B)
   ├─ vota::CEscolheOpcao               (32 B) "outras opções" menu
   ├─ vota::CHabilitacaoAudioNaoPermitida, CConfirmaAudio, CContadoresBiometria (20 B each)
   ├─ vota::CHorarioVotacaoTerminou     (20 B) "Horario de votacao terminou! / Favor encerrar a urna!"
   ├─ vota::CAguardaInspecao            (20 B) "Inspecione cabina e urna"
   ├─ vota::CIniciaFinalizacao          (12 B) start of the encerramento (transit state)
   ├─ vota::CEncerramentoHorarioInvalido(20 B)
   ├─ vota::CPerguntaFilaEleitorVazia   (20 B) "Todas as pessoas presentes já votaram?"
   ├─ vota::CAguardaEleitoresVotarem    (20 B, CAppState(0)) "Aguarde até todos os eleitores presentes votarem"
   ├─ vota::CPedeTituloEncerramento     (32 B) presidente's título (u17)
   ├─ vota::CRegistraDigitalOperador    (36 B) mesário fingerprint
   ├─ vota::CTentativaCapturaDigitalEsgotada, CDigitalNaoCapturada (20 B)
   ├─ vota::CPedeAnoNascimentoSemBiometria, CInformaEleitorPodeVotar, CInformaAnoNascimentoErrado (u10)
   └─ comum::CRegistrarMesarios         (28 B) "Registrar mesário?" (u22 flow)
```

Todo estado é um singleton criado sob demanda (`unique_ptr` estático + mutex; neste build de thread única resta apenas
o stub de `mutex::unlock` (func 150)). A maioria das funções `GetInst` e dos construtores está **inlinada no seu
primeiro chamador** (LTO), e é por isso que as ferramentas colocaram a construção de `CEncerramentoHorarioInvalido`,
`CConfirmaAudio`, `CContadoresBiometria`, `CHabilitacaoAudioNaoPermitida`, `CIniciaFinalizacao`,
`CAguardaInspecao`, `CEleitorJaVotou`, `CTentativaCapturaDigitalEsgotada`, `CDigitalNaoCapturada`,
`CInformaAnoNascimentoErrado` e de cinco subclasses de `IEleitorImpedidoVotar` dentro das funções desta unidade.

### 3.1 Layout de `CThreadOperador` (132 bytes, construído pela func 270)

| offset | membro |
|---|---|
| +0..+19 | `api::CThread`: vptr, `m_estado` (+4, 1 = iniciada), `m_bParar` (+8), `m_bDormindo` (+9), `m_pImpl` (+12, `IThreadImpl`), `m_pSync` (+16, `ISyncCtl`) |
| +20 | `CThreadVota::m_ticks` (`std::map<uebyte, STick>`, `CriaTick` = func 807, `StartTick` 700, `StopTick` 422) |
| +32 | `CThreadVota::m_pContexto` (`comum::CAppStateContext`) |
| +36..+83 | `CMessageOperador` (vector de `SMessage` de 16 bytes, semáforo +56, lock +60, `CMessageInterface` +68) |
| +84 | `std::string` ano de nascimento digitado na justificativa (`CPedeAnoNascimento`, func 10590) |
| +96 | `std::string` título digitado no registro de mesários (slots 17/18 da u22) |
| +108 | `std::string` texto do cargo "VOTANDO PARA: …", inicializado com `" "` (u10) |
| +120 | `std::string` título digitado pelo presidente para encerrar a votação (u17 `CPedeTituloEncerramento`) |

## 4. Fluxo de controle

### 4.1 A thread do operador (`CThreadOperador::Run`, func 10204, srcloc :87)

```
m_pContexto = new CAppStateContext(&CAguardaInicio::GetInst()); state->StartState()
teclado = CPolySingletonList::instance<IInputMT>()                      (:87)
while (!m_bParar):
   1. every queued message (CPriorityMessageQueue::Remove, func 2073):
        log "%s: msg[%d]" ("MensagemProcessadaPelaThread")
        id 0  -> MT form "URNA ELETRÔNICA INOPERANTE" (20,1 centred) / "Siga as instruções na tela do eleitor"
                 (20,2 centred); m_bParar = true; return            (posted with priority 100 by
                 CVerificaEleicaoPassou::StartState, func 11914: the election date has passed)
        id 16 -> m_bParar = true; return                            (sender not found in this unit)
        other -> log "%s: 1 msg[%d] context[%s]"; if the state accepts messages: ProcessMessage (slot 6),
                 VerificaTrocaEstado
   2. one key: if the state accepts keys and IInputMT::HasKey (slot 3): log "%s: 2 hasKey[%d] context[%s]",
      ProcessInput (slot 7), VerificaTrocaEstado
   3. expired ticks (func 5450): if any and the state accepts ticks: ProcessTick(each) (slot 8); VerificaTrocaEstado
   m_pImpl->Yield() (slot 5); if nothing was processed and m_estado == 1:
      lock; m_bDormindo = true; unlock; usleep(50 000); lock; m_bDormindo = false; unlock
```

`VerificaTrocaEstado` (inlinada 3×): `if (state->NeedChangeState()) { FinishState(); state = GetNextState();
if (state) StartState(); }`. `FinalizaExecucao` (slot 5, func 10203), chamada pelos handlers de exceção de `CThreadVota`,
define `m_bParar` de `CThreadEleitor` e da thread de monitoramento de energia `CThreadMonitor` (de
`vota::CThreadMonitor::GetInst`, func 1898, antes exibida pelas ferramentas como `CMonitoraAlimentacao::CreateInst`):
uma exceção que escape de qualquer estado do operador para a aplicação inteira.

### 4.2 Aguardando o início (`CAguardaInicio`, func 5343 = GetInst + ctor)

Tela (não interativa, `CriaForm("")`): linhas 1/2 = `format("{}: {}", CApplication::nome, versão)` dividida no
primeiro `-` e aparada ("VOTA: 10.23.0.1" / "DESENVOLVIMENTO"), linha 3 "Siga as instruções na tela do
eleitor", linha 4 data `DD/MM/YYYY` em (11,4) e hora `hh:mm:ss` em (22,4) (provedores de texto nos slots 1103/1104).
`CAppState(5)` (mensagens + ticks) e um tick de 500 ms. A mensagem 8 (de `CInicioVotacao`) leva a
`CPedeIdentidade` (func 10217, outra unidade). O slot 9 de `CControladorRegistraMesariosVota` (func 10794) também retorna
para cá após o registro inicial de mesários.

### 4.3 Identificação (`CPedeIdentidade`)

`StartState` (10680): log "Aguardando digitação do identificador do eleitor"; `LimpaIdentidades()`
(slot 20 de IInformacaoThreadOperador); mostra o formulário; em modelos de urna ≥ 2020 (`IUrna::GetModelo`, :62), chama o slot 19 de `IScreenMT` com `false` (:63)
(`CEscolheOpcao` passa `true`; o mock web apenas atualiza a tela – nome desconhecido); reanexa o LCD do MT ao
ícone de bateria (`CInfoMTLCD`, funcs 2284 + 5903, substitui a foto do último eleitor); inicia o tick de 60 s, exceto na
fase de treinamento ou se já estiver bloqueado por horário; inicia os ticks de 5 s e 1 s, exceto no modo de treinamento do eleitor.

| tick | ação |
|---|---|
| 60 s (+11) | se `VotacaoBloqueadaPorHorario()` (u10 §3.1: após `terminoVotacao` e ≥ 300 s desde o último voto, nunca em treinamento): para o tick, log(2) "Votacao foi bloqueada por horario" |
| 5 s (+12) | se agora ≥ `GetDataHoraProximaInspecao()`: sorteia a próxima (agora + 60..90 min), envia a mensagem **12** à thread do eleitor e vai para `CAguardaInspecao` ("Inspecione cabina e urna / Instruções no terminal do eleitor", buzzer 51/10) |
| 1 s (+13) | linha de status 4 = "CORRIGE: outras opções" enquanto o campo de entrada estiver vazio, `" "` caso contrário |

`ProcessInput` (10677):

* CORRIGE → `CEscolheOpcao` (§4.5).
* CONFIRMA no **modo de treinamento do eleitor** → o texto digitado é ignorado; `CEleitores` volta ao seu primeiro eleitor,
  cuja identidade principal passa a ser a identidade "digitada" → `CNomeEleitor`. No modo do simulador público, portanto, todo
  eleitor é o primeiro eleitor do cadastro.
* CONFIRMA nos demais casos: campo vazio → nada; log "Identificador do eleitor digitado pelo mesário"; votação
  bloqueada por horário → `CHorarioVotacaoTerminou` ("Horario de votacao terminou! / Favor encerrar a urna!",
  CORRIGE → volta); senão, o número é preenchido à esquerda com `'0'` até 12 dígitos, armazenado com
  `SetIdentidadeDigitada` e → `CValidaIdentidade` (u17: escolhe o tipo de identidade, menu quando ambíguo,
  `CIdentidadeInvalida` "Número errado" quando nenhum validador o aceita) → `CProcuraEleitor`.

### 4.4 Busca no cadastro, impedimentos e justificativa

`CProcuraEleitor::StartState` (10631): `CEleitores::Procura(Formata(identidade), tipo)` (func 2264, posiciona
o eleitor atual).

* encontrado → `CEleitorEncontrado`;
* não encontrado, `aceitarJustificativa` e o tipo digitado é **título** → `CIniciaJustificativa` com
  `CConfirmaJustificativa` ("não pertence <S|a|ao|à> <SCSN>" expandido por `CTradutorFrase::TraduzLabel`);
* caso contrário → `CEleitorNaoEncontrado` ("NÃO CADASTRADO nesta urna", ou "CPF não encontrado. Digite o Título."
  quando um CPF foi digitado e a identidade principal é o título; CONFIRMA: retornar).

`CEleitorEncontrado::StartState` (10635; lança 9394 "Eleitor não posicionado" (:102) se o cadastro não tiver
eleitor atual). Com `imp` = impedimento do turno atual (`CEleitorDetalhe` +196 1º turno / +200 2º
turno; enum md = `TipoImpedimento` do ASN.1 − 1):

| condição | próximo estado (linhas 2/3 do MT, tecla) |
|---|---|
| `imp == 0`, estado dinâmico `SEM_CARGO_PARA_VOTAR` (1) | `CEleitorNaoPossuiCargosParaVotar` "não está apto a votar nesta eleição." (CORRIGE) |
| `imp == 0`, estado dinâmico `VOTOU` (3) | `CEleitorJaVotou` nome / identidade / "JÁ VOTOU" (CONFIRMA: prosseguir) |
| `imp == 0`, caso contrário | `CNomeEleitor` (u10: nome, foto, habilitação) |
| `imp != 0`: log "Eleitor impedido - <motivo>" (`CLogVota::LogaEleitorImpedido` inlinado, clogvota.cpp:432; `imp == 15` lança 9388 "Tipo inválido", `imp ≥ 16` registra um motivo vazio) e então: | |
| justificativa NÃO aceita (`!aceitarJustificativa`, func 4578), `imp == 2` solicitou voto em trânsito | `CEleitorOptouPorVotarEmTransito` "Optou por votar em trânsito" (CONFIRMA) |
| justificativa não aceita, outro `imp` | `CEleitorNaoEncontrado` (a mesma tela de um eleitor de outra seção) |
| `imp` ∈ {4 suspenso, 5 cancelado, 6 não liberado} | `CEleitorImpedidoJustificar` "está impedido de votar ou justificar." / "Eleitor deve procurar cartório eleitoral" (CORRIGE) |
| `imp == 7` sem idade mínima | `CEleitorNaoTemIdadeMinima` "está impedido de votar ou justificar" / "por não ter idade mínima" |
| `imp == 2` e uma entrada diferente de zero no `std::set<int>` em `CConfiguracaoEleicao +604` (significado não identificado) | `CIniciaJustificativaTransito` → `CConfirmaJustificativaTransito` "optou por votar EM TRÂNSITO" |
| `imp == 2`, caso contrário | `CEleitorImpedidoJustificarVotoTransito` "está impedido de votar ou justificar." / "Eleitor solicitou voto em trânsito" |
| outro `imp` (1 vota na seção original, 3 preso provisório, 8 militar, 9–14 TTE …) | `CIniciaJustificativaTemporario` → `CConfirmaJustificativaTemporario` "está impedido de votar nesta seção" |

Estados de justificativa:

* `IIniciaJustificativa` (o ctor 3624 lança 9396 "Confirmação de justificativa nula" (:27) para uma
  confirmação nula). `StartState` (10587) procura o título do eleitor em `comum::CJustificador` (as justificativas
  já registradas nesta urna, chave `CNumeroInscricaoEleitoral`): se já estiver lá → `CJustificativaEfetuada` com
  `m_novaJustificativa = false` (variante "já justificou"); caso contrário → o seu `IConfirmaJustificativa`.
* `IConfirmaJustificativa` (ctor 3625; o rótulo deve caber no LCD de 40 colunas, senão lança 9395
  "Frase muito grande [<frase>]" (:34)): MT "<identidade digitada> / <frase> / CORRIGE: retornar  CONFIRMA: justificar".
  `StartState` (10589) o mostra junto com a foto do eleitor (`ApresentaFotoEleitor`, u05). `ProcessInput` (10588,
  outra unidade) segue para a pergunta do ano de nascimento (`CPedeAnoNascimento`, u17), que registra a justificativa
  (`CJustificador::Justifica`, `qtdJustificativas++`, sincronização MI/MV) e mostra `CJustificativaEfetuada`
  ("CONFIRMA: continuar"). Uma identidade digitada como CPF vai, em vez disso, para `CEleitorImpedidoJustificarVotoCPF`.

### 4.5 "Outras opções" (`CEscolheOpcao`)

Construção (2753): o menu é um `std::map<int, SOpcao>` indexado pelo número exibido; cada entrada tem uma ação
e uma fonte de texto `COpcaoDS{numero, std::function<string()>}` desenhada como `format("{}-{}")`:

| tecla | exibida quando | texto | ação em CONFIRMA (log "Operador selecionou: <texto>") |
|---|---|---|---|
| 1 | sempre | "Ativar áudio" / "Desativar áudio" (conforme `GetAudioHabilitadoManualmente`) | votação bloqueada por horário → limpa o campo, `CHorarioVotacaoTerminou`; `ParametrosUrna.permitirHabManualAudio` falso → `CHabilitacaoAudioNaoPermitida` ("Ativação de áudio não permitida.", 1 s, volta ao menu); senão `CConfirmaAudio` "Deseja realmente (des)ativar o áudio?" (CORRIGE: não / CONFIRMA: sim) |
| 2 | sempre | "Encerrar votação" | `CIniciaFinalizacao` (§4.6) |
| 3 | `DeveRegistrarMesarios()` (func 2520: não demonstração ∧ `registrarMesarios` ∧ não treinamento do eleitor) | "Registrar mesários" | `comum::CRegistrarMesarios` "Registrar mesário?" (u22) |
| 3 ou 4 | urna biométrica ∧ não treinamento do eleitor | "Exibir contadores" | `CContadoresBiometria`: "Habilitação biométrica / biográfica / sem biometria: NNNN" (ou "n.a."), CORRIGE: retornar |

Posições: 1 → (1,2), 2 → (1,3), 3 → (20,2), 4 → (20,3); entrada de um dígito em (20,1). CORRIGE → `CPedeIdentidade`.
Uma escolha que não é dígito, desconhecida ou indisponível deixa o menu inalterado. `StartState` (10690) também grava uma
linha de debug diretamente com `syslog(LOG_INFO, "%s:%d> %d", "StartState", 150, modelo > 2019)`.

### 4.6 Encerramento, lado do operador (a parte desta unidade)

É assim que o presidente da mesa inicia o fim do dia. As etapas desta unidade estão marcadas com **(u27)**; o
resto está documentado na u17 e em `docs/bu/codepath.md` §2.3.

1. **(u27)** `CPedeIdentidade` —CORRIGE→ `CEscolheOpcao` —"2" CONFIRMA→ log "Operador selecionou: Encerrar
   votação" → `CIniciaFinalizacao`.
2. **(u27)** `CIniciaFinalizacao::StartState` (10706), sem tela:
   * modo de treinamento do eleitor (`EhTreinamentoEleitor`, o modo do simulador) → `CPerguntaFilaEleitorVazia`;
   * agora ≥ `HorariosUrna.encerramentoVotacao` (cfg +568; o valor do cenário é 17:00:00):
     votação já bloqueada por horário (`VotacaoBloqueadaPorHorario`) → `CPedeTituloEncerramento` diretamente
     (ninguém pode estar esperando), senão → `CPerguntaFilaEleitorVazia`;
   * antes disso → log(2) "Encerramento só pode ser solicitado após as HH:MM:SS horas" →
     `CEncerramentoHorarioInvalido`.
3. **(u27)** `CEncerramentoHorarioInvalido::StartState` (10710): "Encerramento de votação inválido / Antes do
   horário / CORRIGE: retornar". **No modo de demonstração ou no treinamento de mesários**, ele também ajusta o relógio da urna para
   `encerramentoVotacao − 10 s` (16:59:50 para 17:00:00; slot 0 de `IAjusteDataHora`, :73), soma o salto a
   `CEstadoGeral.ajusteDataHora` (`AdicionaDeltaT`), salva o estado (`SalvaEstado`, func 491) e `eg.bin`
   (func 3333) — para que um treinando possa tentar de novo 10 segundos depois. CORRIGE → `CPedeIdentidade`.
4. `CPerguntaFilaEleitorVazia` (ctor 5426 **(u27)**; métodos em outro lugar): "Todas as pessoas presentes já
   votaram?" — CONFIRMA: sim → `CPedeTituloEncerramento`; CORRIGE: não → **(u27)**
   `CAguardaEleitoresVotarem` (10718): "Aguarde até todos os / eleitores presentes votarem" por 3 s
   (`emscripten_sleep(3000)` neste build), `IInputMT::Flush()` (:47, descarta as teclas digitadas nesse meio-tempo), e volta a `CPedeIdentidade`.
5. `CPedeTituloEncerramento` (ctor 3631 **(u27)**, lógica na u17): "Informe seu título para / encerrar a votação",
   campo de 12 dígitos; ele guarda uma cópia de `encerramentoVotacao` em +20; o título é validado
   (`CValidadorIdentidade`) e armazenado em `CThreadOperador +120`, e um encerramento antecipado pergunta
   "Encerramento de votação antecipado?".
6. `CConfirmaEncerramento` → `CEstadoGeralVota` `estadoVota = 57` (fim da aquisição de votos), `CFimAquisicaoVotos`
   → (registro de mesários no final se `DeveRegistrarMesarios()`, u22) → mensagem **7** à thread do
   eleitor → `CGeraBU` (u09): o BU.

Correções a `docs/bu/codepath.md` §2.3 a partir do código desta unidade: o primeiro teste de `CIniciaFinalizacao` é o
**modo de treinamento do eleitor** (func 697), não a fase de treinamento; depois de `encerramentoVotacao`, a escolha entre
`CPedeTituloEncerramento` e `CPerguntaFilaEleitorVazia` depende de `VotacaoBloqueadaPorHorario`; e
`comum_f2520` (usada por `CFimAquisicaoVotos` e pelo menu) é `DeveRegistrarMesarios` = `IdentificaMesarios()`
(**não** demonstração ∧ `registrarMesarios`) ∧ não treinamento do eleitor — e não "modo de demonstração".

### 4.7 Liberação pela impressão digital do mesário (`CRegistraDigitalOperador`)

Alcançado a partir de `CVerificaDadoEleitor` (ano de nascimento confirmado após a última tentativa de digital que falhou) e de
`CDigitalNaoCapturada` ("tentar novamente").

```
StartState (10454): log "Solicita digital do mesário"; MT "MESÁRIO: / Posicione seu dedo POLEGAR ou INDICADOR /
   sobre o sensor / CORRIGE: não habilitar"; scanner LED 2 (:95); start 15 s and 50 ms ticks; scanner start (:98)
ProcessInput (10452): CORRIGE -> stop ticks, log "Habilitação cancelada durante a captura de digital do mesário",
   -> CPedeIdentidade
ProcessTick (10451):
   15 s:  stop ticks; --tentativasRestantes (starts at 3):
          0 -> reset to 3, CTentativaCapturaDigitalEsgotada "Digital do mesário não capturada / Eleitor não habilitado
               para votação / CONFIRMA: prosseguir" (-> CPedeIdentidade), log "Habilitação cancelada …"
          else CDigitalNaoCapturada "Digital não capturada / CONFIRMA: tentar novamente"
   50 ms: IFingerDetection (:143), IFingerScanner (:144), IUrna (:145); frame selection as CPedeDigital (u10 §4.2);
          no finger -> wait. Finger present:
          "Digital capturada / Por favor aguarde"; LED 1; invert (models ≤ 2019) + flip; WSQ (stub) ->
          CControlaReconhecimento::s_digitalMesario
          WHO IS IT? (only for the log and for s_tituloMesario):
            pass 1  registered mesários (CRegistradorMesario, by identity) whose finger matched at registration
            pass 2  registered mesários without a matched finger
            pass 3  comparecimento rows of mesários who are not voters of this section
            for each: IdentificaMesario(identidade, idArquivo or 999999)            (func 3614)
               a) BiometriaMesarioPresenteNosEleitores (:328/:371): fingers = the one matched at the opening
                  registration, or {1,6,2,7} if not registered at the opening, none (false) if registered without a
                  match; COPY of the whole voter roll; compare with the mesário's templates in the roll
                  (IFingerMatcher, threshold CControladorReconhecimentoMesario.m_limiarScore = 20)
                  -> log "Biometria do mesário {} encontrada em eleitores[ ({id})]", s_tituloMesario = título
               b) else <trab fi>/wsq/registrado/me{id:06}.wsq exists and ComparaDigitais -> log "… encontrada nos
                  arquivos coletados ({id})", s_tituloMesario = título
               c) else log "Biometria coletada não é do mesário {}[ ({id})]"
            first success -> log "Mesário {} habilitou o eleitor"
          nobody: compare with the 6 newest <trab fi>/wsq/nao-registrado/me*.wsq (sorted by name, descending):
            match -> log "Digital coletada bate com uma digital gravada após o registro inicial de mesário. Não é
                     possível associar a habilitação a um mesário."
            none  -> log "Capturada a digital do mesário", "Não encontrou digital coletada em nenhum dos arquivos, vai
                     salvar a digital em novo arquivo."; write me{++m_qtdArquivos:06}.wsq (raw, "w+b") into
                     wsq/nao-registrado/ of both flashes (func 5371, needs ≥ 5 MiB free)
          IN EVERY CASE: stop ticks; IInformacaoThreadOperador::SetHabilitacaoCodigoMesario() (tipo 2);
          -> CInformaEleitorPodeVotar ("ELEITOR(A) PODE VOTAR / Assinar o caderno de votação antes de votar")
```

A identificação nunca bloqueia a liberação: qualquer dedo detectado libera o eleitor. O que muda é o que é
registrado no log e se `s_tituloMesario` é preenchido; `CMostraEleitorVotando::SalvaHabilitacaoEleitor` (u10 §4.5)
depois armazena a captura (cifrada) em `wsq/operador/` e grava `tipo_habilitacao = 2` e o
título do mesário (se conhecido) em `eleitor_dinamico`.

### 4.8 Ano de nascimento sem biometria (`CPedeAnoNascimentoSemBiometria::ProcessInput`, 10489)

CORRIGE → log "Habilitação cancelada durante confirmação de dado do eleitor", `CCancelaHabilitacaoEleitor`.
CONFIRMA com ≥ 4 dígitos: `stoul` comparado com o ano de nascimento do cadastro: igual → log(1) "Ano de nascimento digitado é
igual ao do cadastro", `CInformaEleitorPodeVotar`; diferente → log(2) "… diferente do cadastro"; o primeiro erro
mostra "ANO DE NASCIMENTO INCORRETO / CONFIRMA: tentar novamente" (o segundo formulário), e o segundo vai para
`CInformaAnoNascimentoErrado` ("Por favor oriente o eleitor a procurar o / cartório eleitoral para consultar a data /
de nascimento dele no cadastro da urna / CONFIRMA: cancelar a habilitação", nome do formulário
`telaInformaAnoNascimentoErrado`).

## 5. Telas do MT construídas por esta unidade

| estado | LED / som | linhas (SPoint{col,line}, alinhamento) |
|---|---|---|
| mensagem 0 em `Run` | — | (20,1)c "URNA ELETRÔNICA INOPERANTE"; (20,2)c "Siga as instruções na tela do eleitor" |
| CAguardaInicio | — | (1,1)c "VOTA: 10.23.0.1"; (1,2)c "DESENVOLVIMENTO"; (1,3)c "Siga as instruções na tela do eleitor"; (11,4) data; (22,4) hora |
| CPedeIdentidade | desligado / buzz(51,10) | ver u17: "Digite o Título ou o CPF", relógio (33,1), entrada de 12 dígitos (1,2), "0000/0150", áudio (40,4)r, status (1,4). Variante de treinamento do eleitor: (1,1) "TREINAMENTO DE ELEITORES", (30,2) "Votos:", (37,2) contagem, (40,3)r áudio, (1,4) "CORRIGE: outras opções", (1,4)r "CONFIRMA: votar" |
| CAguardaInspecao | buzz(51,10) | relógio; (1,2) "Inspecione cabina e urna"; (1,3) "Instruções no terminal do eleitor" |
| CEscolheOpcao | — | relógio; (1,1) "Selecione a opção: "; opções (1,2)(1,3)(20,2)(20,3); (1,4) "CORRIGE: retornar"; (40,4)r "CONFIRMA: prosseguir"; entrada (20,1) |
| CHabilitacaoAudioNaoPermitida | beep(2) | (1,2)c "Ativação de áudio não permitida." |
| CConfirmaAudio | — | relógio; (1,2) "Deseja realmente ativar/desativar o áudio?"; (1,4) "CORRIGE: não"; (40,4)r "CONFIRMA: sim" |
| CContadoresBiometria | — | contadores em (35,1)r / (35,2)r / (35,3)r; (1,4) "CORRIGE: retornar" |
| CEncerramentoHorarioInvalido | desligado | relógio; (1,2)c "Encerramento de votação inválido"; (1,3)c "Antes do horário"; (1,4) "CORRIGE: retornar" |
| CPerguntaFilaEleitorVazia | — | relógio; (1,2) "Todas as pessoas presentes já votaram?"; (1,4) "CORRIGE: não"; (40,4)r "CONFIRMA: sim" |
| CPedeTituloEncerramento | — | relógio; (1,1) "Informe seu título para"; (1,2) "encerrar a votação"; entrada de 12 (1,3); (1,4) "CORRIGE: retornar"; (40,4)r "CONFIRMA: prosseguir" |
| família IEleitorImpedidoVotar | desligado | (1,1) identidade digitada; (1,2)/(1,3) textos; (40,4)r "CONFIRMA: retornar" ou (1,4) "CORRIGE: retornar" |
| CEleitorJaVotou | desligado | (1,1) nome "{:2}"; (1,2) identidade; (1,3) "JÁ VOTOU"; (40,4)r "CONFIRMA: prosseguir" |
| família IConfirmaJustificativa | desligado | (1,1) identidade; (1,2) frase; (1,4) "CORRIGE: retornar"; (40,4)r "CONFIRMA: justificar" |
| CJustificativaEfetuada | desligado | (1,1) identidade; (1,2) fonte de texto (placeholder "justificou/ja justificou"); (1,4)r "CONFIRMA: continuar" (posição (1,4) com alinhamento à direita no binário) |
| CRegistraDigitalOperador | desligado | "MESÁRIO:" / "Posicione seu dedo POLEGAR ou INDICADOR" / "sobre o sensor" / "CORRIGE: não habilitar"; depois beep(1) "Digital capturada" (20,1)c, "Por favor aguarde" (20,3)c |
| CTentativaCapturaDigitalEsgotada / CDigitalNaoCapturada | desligado / beep(1) | ver §4.7 |
| CInformaEleitorPodeVotar (func 5410, compartilhada com CControlaReconhecimento) | desligado / beep(1) | (1,1) "ELEITOR(A) PODE VOTAR" / (1,2) "Assinar o caderno de votação antes de" / (1,3) "votar" / (1,4)r "CONFIRMA: prosseguir" (posição (1,4) com alinhamento à direita, não (40,4)) |
| comum::CRegistrarMesarios | buzz(51,10) | (1,1) "Registrar mesário?"; (1,4) "CORRIGE: Cancelar  CONFIRMA: Prosseguir" |

## 6. Código estranho ou arriscado

1. **Salto de relógio antes de uma verificação que pode lançar exceção** (`CEncerramentoHorarioInvalido::StartState`, 10710). No modo
   de demonstração ou no treinamento de mesários, o relógio é ajustado para `encerramentoVotacao − 10 s` **primeiro**, e só então
   `CAjusteDataHora::AdicionaDeltaT` é chamado; ele lança 8071 a menos que o modo de ajuste seja `eAlterarDataSistema`
   (2). No harness (modo 0 no seu `eg.bin` semeado), a exceção escapou do estado depois que o relógio do MT já
   tinha sido alterado (o relógio do MT marcava 16:50 antes e 16:59 depois) e `eg.bin` não foi atualizado. Em uma urna real, uma exceção que escape de um
   estado do operador chega ao handler de `CThreadVota`, cujo `FinalizaExecucao` também para a thread do eleitor. Se uma
   urna real de treinamento pode estar em modo ≠ 2 nesse ponto depende dos estados de início do dia (`CAjusteInicial`,
   `CIniciodeCiclo`, u06), que o harness pula.
2. **`emscripten_sleep` em dois estados desta unidade**: `CAguardaEleitoresVotarem::StartState` (3000 ms) e
   `CHabilitacaoAudioNaoPermitida::StartState` (1000 ms), protegidos pelo byte @1584624 (= 1, nunca gravado). O
   `_emscripten_sleep` do glue aborta (sem ASYNCIFY). Inalcançável no simulador (a thread do operador não roda); o
   harness o substitui por stub. Qualquer build web futuro que rode a thread do operador abortaria ali.
3. **Digitais de mesários gravadas sem cifragem.** A captura do "mesário não identificado" é gravada por
   `comum_f5371` com `CFile` "w+b" + `RawWrite` + `CSystem::CopyFile` em `wsq/nao-registrado/` das duas flashes
   (e o caminho de registro da u22 grava `wsq/registrado/` da mesma forma). As imagens dos eleitores passam por
   `CControlaArmazenamentoDeImagens` com `CifrarWsq` (u10/u24). Neste build, o codificador WSQ é um stub, então os
   arquivos estariam vazios; na urna, eles conteriam o WSQ do mesário em claro. Observação de privacidade para o sistema
   real (não verificável aqui).
4. **Qualquer dedo libera o eleitor** (`CRegistraDigitalOperador`). A busca pelo mesário só afeta o log e
   `s_tituloMesario`; um dedo não identificado (mesmo de quem não é mesário) ainda define `tipo_habilitacao = 2` e
   libera o eleitor. O próprio log do código admite "Não é possível associar a habilitação a um mesário". No
   build web, toda a cadeia biométrica está ausente. Info (escolha de projeto; o mesário está fisicamente presente).
5. **Cadastro de eleitores inteiro copiado a cada verificação de mesário** (`BiometriaMesarioPresenteNosEleitores`, dentro de 3614): o
   `std::map<CEleitorIdentidade, CEleitorDetalhe>` da seção (nós de 236 bytes incluindo os dados biométricos
   opcionais, copiados pela func 5760) é copiado antes de um único `find`, uma vez por mesário tentado (até três
   passadas sobre todos os mesários registrados), enquanto o eleitor espera no MT. Os únicos mesários que pulam a cópia
   são aqueles com registro no período de abertura cujo dedo não coincidiu (a função retorna `false`
   antes de copiar). Desempenho / memória, baixo.
6. **`m_idArquivo` obsoleto após uma liberação não identificada.** Salvar uma captura "nao-registrado" passa por
   `comum_f5371`, que define `CControladorReconhecimentoMesario::m_idArquivo` (singleton). Nada nesta unidade
   o limpa; a u22 observa que o registro de mesários copia esse campo para a próxima linha de `comparecimento_mesario`
   (`id_arquivo`) quando nenhuma imagem nova é armazenada, então um registro posterior poderia apontar para um arquivo do
   diretório `nao-registrado`. Baixo (entre unidades; ver u22).
7. **Iteração de diretório sem código de erro** (`ListaDigitaisNaoRegistradas`, 5374): `directory_iterator(path)`
   lança `filesystem_error` se `<trab>/wsq/nao-registrado/` não existir. Ela também é chamada a partir do construtor
   de `CControladorReconhecimentoMesario` (1149), ou seja, no primeiro uso desse singleton pelo registro de
   mesários ou por este estado. Nem o MEMFS web nem o harness têm qualquer diretório `wsq/`, e nenhum código
   que o crie foi encontrado nesta unidade. Baixo.
8. **O modo de treinamento do eleitor identifica o primeiro eleitor do cadastro** (`CPedeIdentidade::ProcessInput`): o número
   digitado é descartado e `CEleitores` volta ao seu primeiro elemento. Inofensivo no treinamento, mas é uma
   substituição silenciosa de identidade conduzida apenas pela flag `treinamentoEleitor` em `vota.bin`. Info.
9. **O impedimento 15 lança exceção, ≥ 16 é registrado como "Eleitor impedido - "** (`LogaEleitorImpedido` inlinado): um
   impedimento fora do intervalo vindo dos dados do cadastro (`-imp.dat`) ou para a thread do operador (15) ou
   produz um motivo vazio. A enumeração ASN.1 termina em 15 (= md 14), então 15 é o primeiro valor inválido. Baixo.
10. **`syslog` de debug deixado em `CEscolheOpcao::StartState`** ("%s:%d> %d", `__func__`, 150, modelo > 2019), gravado
    diretamente com `vsyslog`, contornando `CLogVota`. Info.
11. **`CRegistraDigitalOperador` verifica `me999999.wsq`**: quando um mesário não tem id de imagem, 999999 ainda é usado para
    montar o nome do arquivo na etapa b) de `IdentificaMesario`. Inofensivo, a menos que esse arquivo exista. Info.

## 7. Dados lidos e gravados

| dado | acesso |
|---|---|
| `CConfiguracaoEleicao`: `ParametrosUrna.aceitarJustificativa` (+482), `permitirHabManualAudio` (+485), `registrarMesarios` (+488, via `IdentificaMesarios`); `HorariosUrna.encerramentoVotacao` (+568 data, +576 hora); tipos de identidade permitidos; tipo de identidade principal (+668); o `std::set<int>` em +604 | lido |
| `CEstadoGeral` (fase, turno, `ajusteDataHora` +52), `CEstadoGeralVota` (`treinamentoEleitor` +72) | lido; **gravado** pelo salto de relógio (§4.6 etapa 3): `eg.bin` / `vota.bin` salvos |
| `CEleitores` (cadastro, eleitor atual, impedimentos P1/P2, estado dinâmico, templates biométricos) | lido; o eleitor atual é posicionado (busca / volta ao início) |
| `comum::CJustificador` (justificativas já registradas) | lido (`IIniciaJustificativa`) |
| `comum::CRegistradorMesario` (linhas de `comparecimento_mesario`, por PK e por identidade) | lido (`CRegistraDigitalOperador`) |
| `<trab>/wsq/registrado/me%06u.wsq` (flash interna) | lido |
| `<trab>/wsq/nao-registrado/me%06u.wsq` (flash interna + externa) | listado, lido, **gravado** (bruto) |
| `CControlaReconhecimento::s_digitalMesario`, `s_tituloMesario` | gravado (consumido por `SalvaHabilitacaoEleitor` da u10 → `eleitor_dinamico`) |
| `IInformacaoThreadOperador` | identidade/tipo digitados, `LimpaIdentidades`, hora da inspeção, `SetHabilitacaoCodigoMesario` |
| relógio do sistema (`IAjusteDataHora`) | **ajustado** em demonstração/treinamento (§4.6) |
| `logd.dat` (CLogVota) | todas as mensagens citadas no §4 |
| fila da thread do eleitor | mensagem 12 (inspeção) |

## 8. Boletim de urna (BU)

Esta unidade não monta nem assina o BU. Suas contribuições são:

* **o início do encerramento** (§4.6): o menu "outras opções", a verificação de horário contra
  `HorariosUrna.encerramentoVotacao`, o diálogo "ainda há alguém na fila?" e a entrada em
  `CPedeTituloEncerramento`; o caminho termina com a mensagem 7 à thread do eleitor, que roda `CGeraBU`
  (`docs/bu/codepath.md` §2.4, u08/u09);
* **o tipo de habilitação 2** (`CODIGO_MESARIO`) definido por `CRegistraDigitalOperador`, que é contado no BU como
  `DetalhamentoComparecimento.qtdEleitoresHabilitadosPorBiografia` e no QR code como `HBBG` (u10 §8), e a
  identificação do mesário (`s_tituloMesario`) que vai para `ModuloResultadoUrnaCadastro`
  (`habilitacaoPorCodigo.identificacaoMesario`);
* **as justificativas** iniciadas aqui (`IIniciaJustificativa` / `IConfirmaJustificativa`) se tornam o BUJ /
  `jufa.dat` no fim do dia (u17 `CPedeAnoNascimento`, u09);
* a tela "Exibir contadores" mostra ao vivo os mesmos contadores de habilitação (fontes `HBBM`/`HBBG`/`HBSB`).

## 9. Particularidades do build web

* A thread do operador nunca roda (§2); todo estado aqui é código morto no navegador.
* Nenhuma implementação de scanner/matcher/detecção de digitais está registrada; o primeiro
  `CPolySingletonList::instance<IFingerScanner>()` lançaria "PolySingleton - solicitada uma instancia nao criada".
* O slot 19 de `IScreenMT` é um refresh em `simulador::CWasmScreenMT` (o significado no MT real é desconhecido).
* `emscripten_sleep` aborta (§6.2); o harness o substitui.
* O simulador roda no modo de treinamento do eleitor, em que `CPedeIdentidade` ignora o número digitado e
  `CIniciaFinalizacao` sempre pergunta "Todas as pessoas presentes já votaram?".

## 10. Observações sobre wasm / Emscripten

* **Nomes enganosos das ferramentas por causa do inlining**: a func 3614 carrega os srclocs de
  `BiometriaMesarioPresenteNosEleitores` inlinada (um parâmetro), mas recebe dois (identidade + id da imagem) e faz o log
  em volta dela; a func 2753 (`CEscolheOpcao::GetInst`) e as 2749/1906 (`CValidaIdentidade`) foram arquivadas em
  `cpedeidentidade.cpp` porque `CPedeIdentidade::ProcessInput` as chama/inlina; a func 5343
  (`CAguardaInicio::GetInst`), em `cthreadoperador.cpp` porque `Run` a chama.
* **Corpos compartilhados**: a func 2295 é o `ProcessInput` de cinco estados (o código da tecla e o registro de srcloc do chamador
  são parâmetros); a func 2902 é o destrutor de todo estado "vptr + um shared_ptr em +12" (1257 e 1688 a chamam
  com a sua vtable); a func 1689 é o destrutor de deleção de sete classes; a func 6012 é o corpo de `FinishState`
  de `CRegistraDigitalOperador` e de outro estado (srclocs passados como argumento).
* **Ordenação com um comparador por valor**: `std::sort(..., [](std::string a, std::string b){ return a > b; })`
  instancia `__introsort`/`__insertion_sort_incomplete`/`__sort4` (5373/5370/2726; `__sort3` está inlinado em 2726) em torno do comparador 310, que
  copia as duas strings (alocações no heap para nomes com ≥ 11 caracteres) a cada comparação.
* **Alinhamento do valor do map do menu**: os nós de `std::map<int, SOpcao>` têm 64 bytes (chave em +16, valor em +24 porque
  `std::function` força alinhamento de 8 bytes), e é por isso que a chave e o id da ação parecem duplicados no
  pseudocódigo de 2753.
* **Testes com/sem sinal**: o teste de intervalo do impedimento é `(imp - 7) >=u -3` (ou seja, 4..6), e o teste de dígito no
  menu, `(c - 58) <u -10`; o pseudocódigo imprime ambos como `>=`/`<` simples.
* A func 3641 (nome nas ferramentas `CTime::operator+=`) contém tanto `CTime::operator+=(int)` (srcloc ctime.cpp:148, overflow)
  quanto `operator-=(int)` (:161, underflow) e trabalha sobre uma CÓPIA do valor: para um argumento não negativo, ela retorna
  `time − n`. `CEncerramentoHorarioInvalido` a chama com 10, então o relógio vai para `encerramentoVotacao − 10 s`
  (não + 10 s); a configuração não é modificada. Confirmado no harness (relógio do MT em 16:59 após o salto).
  O mesmo corpo é chamado por `CIniciodeCiclo::AjustaDataHora` (func 7306, argumento 10) e
  `CAjusteInicial::ValidaTemposDesligamento` (func 7160, argumento 30), então os "+ 10 s" / "+ 30 s" da u06 devem ser
  reverificados (provavelmente também são − 10 s / − 30 s).

## 11. Questões em aberto

* Significado do `std::set<int>` em `CConfiguracaoEleicao +604` que habilita a justificativa de eleitores que
  solicitaram voto em trânsito.
* Quem envia a mensagem 16 do operador (parada silenciosa) e o significado exato do slot 19 de `IScreenMT` (`bool`) no MT real.
* Nome real da func 3614 (pode ser uma lambda ou um método privado como `ProcuraMesario`).
* Se o modo de `CAjusteDataHora` é sempre `eAlterarDataSistema` quando `CEncerramentoHorarioInvalido` roda em uma
  urna real de demonstração/treinamento (§6.1).
* Onde os diretórios `<trab>/wsq/{registrado,nao-registrado}/` são criados.
* Texto em +12 de `CJustificativaEfetuada`: o placeholder "justificou/ja justificou" é substituído pelo seu StartState
  (outra unidade) — os textos finais exatos não são vistos aqui.

## 12. Tabela de mapeamento (todas as 97 funções da u27)

Colunas: índice wasm, tamanho em bytes, `*` = observada em execução nos votos registrados, nome na ferramenta, símbolo
reconstruído, onde ela é reconstruída, arquivo original, confiança.

| idx | tamanho | executou | nome na ferramenta | símbolo | reconstruído em | arquivo original | conf. |
|---|---|---|---|---|---|---|---|
| 310 | 286 |  | `vota_f310` | `std::__sort helper: comparator lambda [](std::string a, std::string b){return a > b;} (strings by value)` | src/uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp (comentário) | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | média |
| 395 | 327 |  | `vota_f395` | `api::CFormBuilderMT::AddInputControl` | helper de biblioteca/inlinado (resumido em src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp) | uenux2/src/api/gui/cformbuildermt.h (instância de template) | média |
| 697 | 36 |  | `comum_f697` | `comum::EhTreinamentoEleitor` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp | uenux2/src/app/comum/appinfo/cappinfo.cpp | alta |
| 728 | 381 |  | `vota_f728` | `api::CFormBuilderMT::Add<api::CClockFieldMT>` | helper de biblioteca/inlinado (resumido em src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp) | uenux2/src/api/gui/cformbuildermt.h (instância de template) | média |
| 1055 | 33 | * | `api_f1055` | `std::filesystem::__status(const path&, error_code*)` (libc++, por trás de `status()`) | biblioteca | libcxx filesystem (operations.cpp) | média |
| 1149 | 252 |  | `comum_f1149` | `comum::CControladorReconhecimentoMesario::GetInst` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp | uenux2/src/app/comum/comparecimentomesario/ccontroladorreconhecimetomesario.cpp | alta |
| 1151 | 330 |  | `vota_f1151` | `api::CFormBuilderMT::Add<api::CInputFieldMT>` | helper de biblioteca/inlinado (resumido em src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp) | uenux2/src/api/gui/cformbuildermt.h (instância de template) | média |
| 1257 | 12 |  | `vota::IEleitorImpedidoVotar::vf0` | `vota::IEleitorImpedidoVotar::~IEleitorImpedidoVotar` | src/uenux2/src/app/vota/operador/leidentidade/ieleitorimpedidovotar.cpp | uenux2/src/app/vota/operador/leidentidade/ieleitorimpedidovotar.cpp (caminho inferido) | alta |
| 1687 | 20 |  | `api_f1687` | `vota::(anon)::AudioHabilitadoManualmente [IInformacaoThreadOperador::GetInst().GetAudioHabilitadoManualmente()]` | src/uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | média |
| 1688 | 12 |  | `vota::IConfirmaJustificativa::vf0` | `vota::IConfirmaJustificativa::~IConfirmaJustificativa` | src/uenux2/src/app/vota/operador/justificativa/iconfirmajustificativa.cpp | uenux2/src/app/vota/operador/justificativa/iconfirmajustificativa.cpp | alta |
| 1689 | 13 |  | `vota::IEleitorImpedidoVotar::vf1` | `vota::IEleitorImpedidoVotar::~IEleitorImpedidoVotar (deleting)` | src/uenux2/src/app/vota/operador/leidentidade/ieleitorimpedidovotar.cpp | uenux2/src/app/vota/operador/leidentidade/ieleitorimpedidovotar.cpp (caminho inferido) | alta |
| 1694 | 423 |  | `vota_f1694` | `api::CFormBuilderMT::CriaForm` | helper de biblioteca/inlinado (resumido em src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp) | uenux2/src/api/gui/cformbuildermt.h (instância de template) | média |
| 1902 | 82 |  | `vota_f1902` | `std::__tree<map<CEleitorIdentidade, CComparecimentoMesario>>::destroy` | helper de biblioteca/inlinado | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp (instância de template) | média |
| 1905 | 1239 |  | `vota_f1905` | `vota::IEleitorImpedidoVotar::IEleitorImpedidoVotar` | src/uenux2/src/app/vota/operador/leidentidade/ieleitorimpedidovotar.cpp | uenux2/src/app/vota/operador/leidentidade/ieleitorimpedidovotar.cpp (caminho inferido) | alta |
| 1906 | 57 |  | `vota_f1906` | `std::__tree<map<int, SOpcaoIdentidade>>::destroy (CValidaIdentidade::m_opcoes)` | helper de biblioteca/inlinado | uenux2/src/app/vota/operador/leidentidade/cvalidaidentidade.cpp (caminho inferido) | média |
| 2226 | 20 |  | `api_f2226` | `vota::impl::IInformacaoThreadOperador thunk: GetInst().VotacaoBloqueadaPorHorario()` | src/uenux2/src/app/vota/operador/leidentidade/cpedeidentidade.cpp (pontos de chamada) | uenux2/src/app/vota/operador/comum/iinformacaothreadoperador.h (chamada inline posta fora de linha) | média |
| 2295 | 127 |  | `api_f2295` | `vota::RetornaParaPedeIdentidadeSeTecla (shared ProcessInput body)` | src/uenux2/src/app/vota/operador/outrasopcoes/cencerramentohorarioinvalido.cpp | uenux2/src/app/vota/operador/outrasopcoes/cencerramentohorarioinvalido.cpp | média |
| 2298 | 182 |  | `vota_f2298` | `vota::(anon)::DiretorioWsq(subdir) [<trab fi>/wsq/<subdir>]` | src/uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp (helper compartilhado) | média |
| 2502 | 15 |  | `api_f2502` | `vota::(anon)::LogaOpcaoSelecionada ["Operador selecionou: {}"]` | src/uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | alta |
| 2717 | 268 |  | `vota::CThreadOperador::vf0` | `vota::CThreadOperador::~CThreadOperador` | src/uenux2/src/app/vota/operador/cthreadoperador.cpp | uenux2/src/app/vota/operador/cthreadoperador.cpp | alta |
| 2726 | 594 |  | `vota_f2726` | `std::__sort4<..., lambda a>b>(vector<string>)` (quatro iteradores; __sort3 inlinado) | helper de biblioteca/inlinado | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp (instância de template) | média |
| 2747 | 908 |  | `vota_f2747` | `vota::CJustificativaEfetuada::GetInst` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp | uenux2/src/app/vota/operador/justificativa/cjustificativaefetuada.cpp (caminho inferido) | alta |
| 2748 | 13 |  | `vota::IConfirmaJustificativa::vf1` | `vota::IConfirmaJustificativa::~IConfirmaJustificativa (deleting)` | src/uenux2/src/app/vota/operador/justificativa/iconfirmajustificativa.cpp | uenux2/src/app/vota/operador/justificativa/iconfirmajustificativa.cpp | alta |
| 2749 | 69 |  | `vota::CValidaIdentidade::vf0` | `vota::CValidaIdentidade::~CValidaIdentidade` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp | uenux2/src/app/vota/operador/leidentidade/cvalidaidentidade.cpp (caminho inferido) | alta |
| 2750 | 96 |  | `api_f2750` | `vota::CEscolheOpcao::SOpcao::SOpcao(EAcao, COpcaoDS&&)` | src/uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | média |
| 2751 | 83 |  | `api_f2751` | `vota::CEscolheOpcao::COpcaoDS::COpcaoDS(int, std::function&&)` | src/uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | média |
| 2752 | 69 |  | `vota::CEscolheOpcao::vf0` | `vota::CEscolheOpcao::~CEscolheOpcao` | src/uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | alta |
| 2753 | 3646 |  | `api_f2753` | `vota::CEscolheOpcao::GetInst` | src/uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | alta |
| 2805 | 658 |  | `vota_f2805` | `std::__tree<map<CEleitorIdentidade, ...>>::__find_equal (hint)` | helper de biblioteca/inlinado | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp (instância de template) | baixa |
| 2864 | 175 |  | `comum_f2864` | `comum::CPath::GetPathWsq(flash, turno) [GetPathTrab/"wsq/"]` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp | uenux2/src/app/comum/cpath.cpp (?) | média |
| 3258 | 17 |  | `vota_f3258` | `vota::(anon)::LogaMesarioHabilitou ["Mesário {} habilitou o eleitor"]` | src/uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | alta |
| 3593 | 16 |  | `vota_f3593` | `vota::(anon)::LogDebug (cthreadoperador.cpp copy)` | src/uenux2/src/app/vota/operador/cthreadoperador.cpp | uenux2/src/app/vota/operador/cthreadoperador.cpp | alta |
| 3610 | 971 |  | `comum_f3610` | `comum::CRegistrarMesarios::GetInst` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cregistrarmesarios.cpp (caminho inferido) | alta |
| 3614 | 4765 |  | `vota::CRegistraDigitalOperador::BiometriaMesarioPresenteNosEleitores` | `vota::CRegistraDigitalOperador::IdentificaMesario (inlines BiometriaMesarioPresenteNosEleitores)` | src/uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | média |
| 3619 | 22 |  | `api_f3619` | `vota::impl::IInformacaoThreadOperador thunk: GetInst().SetTipoIdentidadeDigitada(t)` | src/uenux2/src/app/vota/operador/leidentidade/cpedeidentidade.cpp (pontos de chamada) | uenux2/src/app/vota/operador/comum/iinformacaothreadoperador.h (chamada inline posta fora de linha) | média |
| 3624 | 92 |  | `vota::IIniciaJustificativa::IIniciaJustificativa` | `vota::IIniciaJustificativa::IIniciaJustificativa` | src/uenux2/src/app/vota/operador/justificativa/iiniciajustificativa.cpp | uenux2/src/app/vota/operador/justificativa/iiniciajustificativa.cpp | alta |
| 3625 | 965 |  | `vota::IConfirmaJustificativa::IConfirmaJustificativa` | `vota::IConfirmaJustificativa::IConfirmaJustificativa` | src/uenux2/src/app/vota/operador/justificativa/iconfirmajustificativa.cpp | uenux2/src/app/vota/operador/justificativa/iconfirmajustificativa.cpp | alta |
| 3630 | 81 |  | `vota_f3630` | `std::__tree<map<int, CEscolheOpcao::SOpcao>>::destroy` | helper de biblioteca/inlinado | uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp (instância de template) | média |
| 3631 | 1120 |  | `vota_f3631` | `vota::CPedeTituloEncerramento::GetInst` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp | uenux2/src/app/vota/operador/outrasopcoes/cpedetituloencerramento.cpp (caminho inferido) | alta |
| 3673 | 441 |  | `vota_f3673` | `api::IInputField<api::IScreenMT>::IInputField(validation, tamanho, flag)` | helper de biblioteca/inlinado (resumido em src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp) | uenux2/src/api/gui/iinputfield.h (instância de template) | média |
| 3674 | 273 |  | `vota_f3674` | `std::vector<std::shared_ptr<api::IFormField<api::IScreenMT>>>::push_back(&&)` | helper de biblioteca/inlinado | libcxx vector (instância de template) | média |
| 3769 | 27 |  | `vota_f3769` | `vota::(anon)::ImpedimentoTurnoAtual [CEleitorDetalhe +196/+200 by turno]` | src/uenux2/src/app/vota/operador/leidentidade/celeitorencontrado.cpp | uenux2/src/app/vota/operador/leidentidade/celeitorencontrado.cpp | média |
| 3793 | 15 |  | `vota_f3793` | `vota::(anon)::DiretorioNaoRegistrado` | src/uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | média |
| 3885 | 32 |  | `vota_f3885` | `std::unique_ptr<vota::IConfirmaJustificativa>::reset (static slot)` | src/uenux2/src/app/vota/operador/justificativa/iconfirmajustificativa.cpp (comentário) | uenux2/src/app/vota/operador/justificativa/iconfirmajustificativa.cpp | média |
| 4522 | 187 |  | `vota_f4522` | `vota::(anon)::LogaHabilitacaoCanceladaCapturaMesario` | src/uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | alta |
| 4578 | 41 |  | `vota_f4578` | `vota::(anon)::JustificativaNaoAceita` | src/uenux2/src/app/vota/operador/leidentidade/celeitorencontrado.cpp | uenux2/src/app/vota/operador/leidentidade/celeitorencontrado.cpp | média |
| 5343 | 2105 |  | `vota_f5343` | `vota::CAguardaInicio::GetInst` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp | uenux2/src/app/vota/operador/caguardainicio.cpp (caminho inferido) | alta |
| 5370 | 1469 |  | `vota_f5370` | `std::__insertion_sort_incomplete<lambda a>b, string*>` | helper de biblioteca/inlinado | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp (instância de template) | média |
| 5373 | 7522 |  | `vota_f5373` | `std::__introsort<lambda a>b, string*>` | helper de biblioteca/inlinado | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp (instância de template) | média |
| 5374 | 988 |  | `vota_f5374` | `vota::(anon)::ListaDigitaisNaoRegistradas` | src/uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | média |
| 5376 | 343 |  | `vota_f5376` | `std::map<CEleitorIdentidade, CComparecimentoMesario>::__emplace_hint_unique` | helper de biblioteca/inlinado | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp (instância de template) | média |
| 5399 | 105 |  | `vota_f5399` | `vota::CInformaEleitorPodeVotar::GetInst` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp | uenux2/src/app/vota/operador/confirmaidentidade/cinformaeleitorpodevotar.cpp (caminho inferido) | alta |
| 5410 | 1258 |  | `vota_f5410` | `vota::CriaFormEleitorPodeVotar` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp | uenux2/src/app/vota/operador/confirmaidentidade/cinformaeleitorpodevotar.cpp (caminho inferido) | média |
| 5414 | 22 |  | `api_f5414` | `vota::impl::IInformacaoThreadOperador thunk: GetInst().SetIdentidadeDigitada(s)` | src/uenux2/src/app/vota/operador/leidentidade/cpedeidentidade.cpp (pontos de chamada) | uenux2/src/app/vota/operador/comum/iinformacaothreadoperador.h (chamada inline posta fora de linha) | média |
| 5419 | 423 |  | `vota_f5419` | `api::CFormBuilderMT::Add<CTextFieldMT>(pos, CDataText<std::function<std::string()>>)` | helper de biblioteca/inlinado (resumido em src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp) | uenux2/src/api/gui/cformbuildermt.h (instância de template) | média |
| 5424 | 263 |  | `vota_f5424` | `vota::CEleitorNaoEncontrado::GetInst` | src/uenux2/src/app/vota/operador/leidentidade/ieleitorimpedidovotar.cpp | uenux2/src/app/vota/operador/leidentidade/celeitornaoencontrado.cpp (caminho inferido) | alta |
| 5426 | 851 |  | `vota_f5426` | `vota::CPerguntaFilaEleitorVazia::GetInst` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp | uenux2/src/app/vota/operador/outrasopcoes/cperguntafilaeleitorvazia.cpp (caminho inferido) | alta |
| 5522 | 16 |  | `vota_f5522` | `api::IForm<api::IScreenMT>::IForm(campos, preShow)` | helper de biblioteca/inlinado | uenux2/src/api/gui/iform.h (instância de template) | média |
| 5815 | 15 |  | `vota_f5815` | `vota::(anon)::DiretorioRegistrado` | src/uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | média |
| 10203 | 18 |  | `vota::CThreadOperador::vf5` | `vota::CThreadOperador::FinalizaExecucao` | src/uenux2/src/app/vota/operador/cthreadoperador.cpp | uenux2/src/app/vota/operador/cthreadoperador.cpp | média |
| 10204 | 1873 |  | `vota::CThreadOperador::Run` | `vota::CThreadOperador::Run` | src/uenux2/src/app/vota/operador/cthreadoperador.cpp | uenux2/src/app/vota/operador/cthreadoperador.cpp | alta |
| 10206 | 13 |  | `vota::CThreadOperador::vf1` | `vota::CThreadOperador::~CThreadOperador (deleting)` | src/uenux2/src/app/vota/operador/cthreadoperador.cpp | uenux2/src/app/vota/operador/cthreadoperador.cpp | alta |
| 10208 | 38 |  | `vota_f10208` | `atexit: CThreadOperador static instance reset (@1911708)` | src/uenux2/src/app/vota/operador/cthreadoperador.cpp (comentário) | uenux2/src/app/vota/operador/cthreadoperador.cpp | alta |
| 10451 | 6996 |  | `vota::CRegistraDigitalOperador::ProcessTick` | `vota::CRegistraDigitalOperador::ProcessTick` | src/uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | alta |
| 10452 | 158 |  | `vota::CRegistraDigitalOperador::vf7` | `vota::CRegistraDigitalOperador::ProcessInput` | src/uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | alta |
| 10453 | 17 |  | `vota::CRegistraDigitalOperador::FinishState` | `vota::CRegistraDigitalOperador::FinishState` | src/uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | alta |
| 10454 | 288 |  | `vota::CRegistraDigitalOperador::StartState` | `vota::CRegistraDigitalOperador::StartState` | src/uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | alta |
| 10461 | 14 |  | `vota::CTentativaCapturaDigitalEsgotada::vf7` | `vota::CTentativaCapturaDigitalEsgotada::ProcessInput` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp | uenux2/src/app/vota/operador/confirmaidentidade/ctentativacapturadigitalesgotada.cpp (caminho inferido) | alta |
| 10489 | 2040 |  | `vota::CPedeAnoNascimentoSemBiometria::vf7` | `vota::CPedeAnoNascimentoSemBiometria::ProcessInput` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp | uenux2/src/app/vota/operador/confirmaidentidade/cpedeanonascimentosembiometria.cpp (caminho inferido) | alta |
| 10587 | 210 |  | `vota::IIniciaJustificativa::vf2` | `vota::IIniciaJustificativa::StartState` | src/uenux2/src/app/vota/operador/justificativa/iiniciajustificativa.cpp | uenux2/src/app/vota/operador/justificativa/iiniciajustificativa.cpp | alta |
| 10589 | 32 |  | `vota::IConfirmaJustificativa::vf2` | `vota::IConfirmaJustificativa::StartState` | src/uenux2/src/app/vota/operador/justificativa/iconfirmajustificativa.cpp | uenux2/src/app/vota/operador/justificativa/iconfirmajustificativa.cpp | alta |
| 10604 | 14 |  | `vota::CEleitorImpedidoJustificarVotoCPF::vf7` | `vota::CEleitorImpedidoJustificarVotoCPF::ProcessInput` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp | uenux2/src/app/vota/operador/justificativa/celeitorimpedidojustificarvotocpf.cpp (caminho inferido) | alta |
| 10611 | 12 |  | `vota_f10611` | `atexit: CConfirmaJustificativaTransito instance reset (@1905812)` | src/uenux2/src/app/vota/operador/justificativa/iconfirmajustificativa.cpp (comentário) | uenux2/src/app/vota/operador/justificativa/cconfirmajustificativatransito.cpp (caminho inferido) | média |
| 10615 | 12 |  | `vota_f10615` | `atexit: CConfirmaJustificativaTemporario instance reset (@1905756)` | src/uenux2/src/app/vota/operador/justificativa/iconfirmajustificativa.cpp (comentário) | uenux2/src/app/vota/operador/justificativa/cconfirmajustificativatemporario.cpp (caminho inferido) | média |
| 10622 | 12 |  | `vota_f10622` | `atexit: CConfirmaJustificativa instance reset (@1905700)` | src/uenux2/src/app/vota/operador/justificativa/iconfirmajustificativa.cpp (comentário) | uenux2/src/app/vota/operador/justificativa/cconfirmajustificativa.cpp (caminho inferido) | média |
| 10628 | 13 |  | `vota::CValidaIdentidade::vf1` | `vota::CValidaIdentidade::~CValidaIdentidade (deleting)` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp | uenux2/src/app/vota/operador/leidentidade/cvalidaidentidade.cpp (caminho inferido) | alta |
| 10630 | 38 |  | `vota_f10630` | `atexit: CValidaIdentidade instance reset (@1905672)` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp (comentário) | uenux2/src/app/vota/operador/leidentidade/cvalidaidentidade.cpp (caminho inferido) | alta |
| 10631 | 657 |  | `vota::CProcuraEleitor::vf2` | `vota::CProcuraEleitor::StartState` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp | uenux2/src/app/vota/operador/leidentidade/cprocuraeleitor.cpp (caminho inferido) | alta |
| 10635 | 3870 |  | `vota::CEleitorEncontrado::vf2` | `vota::CEleitorEncontrado::StartState` | src/uenux2/src/app/vota/operador/leidentidade/celeitorencontrado.cpp | uenux2/src/app/vota/operador/leidentidade/celeitorencontrado.cpp | alta |
| 10667 | 14 |  | `vota::CIdentidadeInvalida::vf7` | `vota::CIdentidadeInvalida::ProcessInput` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp | uenux2/src/app/vota/operador/leidentidade/cidentidadeinvalida.cpp (caminho inferido) | alta |
| 10676 | 1154 |  | `vota::CPedeIdentidade::vf8` | `vota::CPedeIdentidade::ProcessTick` | src/uenux2/src/app/vota/operador/leidentidade/cpedeidentidade.cpp | uenux2/src/app/vota/operador/leidentidade/cpedeidentidade.cpp | alta |
| 10677 | 756 |  | `vota::CPedeIdentidade::vf7` | `vota::CPedeIdentidade::ProcessInput` | src/uenux2/src/app/vota/operador/leidentidade/cpedeidentidade.cpp | uenux2/src/app/vota/operador/leidentidade/cpedeidentidade.cpp | alta |
| 10678 | 35 |  | `vota::CPedeIdentidade::vf5` | `vota::CPedeIdentidade::FinishState` | src/uenux2/src/app/vota/operador/leidentidade/cpedeidentidade.cpp | uenux2/src/app/vota/operador/leidentidade/cpedeidentidade.cpp | alta |
| 10680 | 377 |  | `vota::CPedeIdentidade::StartState` | `vota::CPedeIdentidade::StartState` | src/uenux2/src/app/vota/operador/leidentidade/cpedeidentidade.cpp | uenux2/src/app/vota/operador/leidentidade/cpedeidentidade.cpp | alta |
| 10689 | 3059 |  | `vota::CEscolheOpcao::vf7` | `vota::CEscolheOpcao::ProcessInput` | src/uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | alta |
| 10690 | 240 |  | `vota::CEscolheOpcao::StartState` | `vota::CEscolheOpcao::StartState` | src/uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | alta |
| 10691 | 13 |  | `vota::CEscolheOpcao::vf1` | `vota::CEscolheOpcao::~CEscolheOpcao (deleting)` | src/uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | alta |
| 10695 | 113 |  | `vota_f10695` | `vota::(anon)::TextoOpcaoAudio` | src/uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | alta |
| 10697 | 38 |  | `vota_f10697` | `atexit: CEscolheOpcao instance reset (@1905308)` | src/uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp (comentário) | uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | alta |
| 10706 | 1498 |  | `vota::CIniciaFinalizacao::vf2` | `vota::CIniciaFinalizacao::StartState` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp | uenux2/src/app/vota/operador/outrasopcoes/ciniciafinalizacao.cpp (caminho inferido) | alta |
| 10709 | 14 |  | `vota::CEncerramentoHorarioInvalido::vf7` | `vota::CEncerramentoHorarioInvalido::ProcessInput` | src/uenux2/src/app/vota/operador/outrasopcoes/cencerramentohorarioinvalido.cpp | uenux2/src/app/vota/operador/outrasopcoes/cencerramentohorarioinvalido.cpp | alta |
| 10710 | 267 |  | `vota::CEncerramentoHorarioInvalido::StartState` | `vota::CEncerramentoHorarioInvalido::StartState` | src/uenux2/src/app/vota/operador/outrasopcoes/cencerramentohorarioinvalido.cpp | uenux2/src/app/vota/operador/outrasopcoes/cencerramentohorarioinvalido.cpp | alta |
| 10718 | 107 |  | `vota::CAguardaEleitoresVotarem::StartState` | `vota::CAguardaEleitoresVotarem::StartState` | src/uenux2/src/app/vota/operador/outrasopcoes/caguardaeleitoresvotarem.cpp | uenux2/src/app/vota/operador/outrasopcoes/caguardaeleitoresvotarem.cpp | alta |
| 10742 | 173 |  | `api_f10742` | `vota::(anon)::TextoConfirmaAudio` | src/uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | uenux2/src/app/vota/operador/outrasopcoes/cconfirmaaudio.cpp (caminho inferido) | alta |
| 10754 | 49 |  | `vota::CHabilitacaoAudioNaoPermitida::vf2` | `vota::CHabilitacaoAudioNaoPermitida::StartState` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp | uenux2/src/app/vota/operador/outrasopcoes/chabilitacaoaudionaopermitida.cpp (caminho inferido) | alta |
| 10757 | 14 |  | `vota::CHorarioVotacaoTerminou::vf7` | `vota::CHorarioVotacaoTerminou::ProcessInput` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp | uenux2/src/app/vota/operador/outrasopcoes/chorariovotacaoterminou.cpp (caminho inferido) | alta |
| 10794 | 5 |  | `vota::CControladorRegistraMesariosVota::vf9` | `vota::CControladorRegistraMesariosVota::GetEstadoAposRegistroInicial` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp | uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp (caminho inferido) | média |
