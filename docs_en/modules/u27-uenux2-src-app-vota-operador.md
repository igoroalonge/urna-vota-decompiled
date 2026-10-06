# u27 — `uenux2/src/app/vota/operador`: the operator thread, voter identification, "outras opções" and the start of the encerramento

Unit u27 covers **97 wasm functions** that the tools attributed to nine original files of the
**operator side** ("operador") of the VOTA application:

```
uenux2/src/app/vota/operador/cthreadoperador.cpp
uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp
uenux2/src/app/vota/operador/justificativa/{iconfirmajustificativa,iiniciajustificativa}.cpp
uenux2/src/app/vota/operador/leidentidade/{celeitorencontrado,cpedeidentidade}.cpp
uenux2/src/app/vota/operador/outrasopcoes/{caguardaeleitoresvotarem,cencerramentohorarioinvalido,cescolheopcao}.cpp
```

It is the operator-thread body itself (`CThreadOperador::Run`), the idle screen of the poll worker's
terminal (`CPedeIdentidade`, "Digite o Título ou o CPF"), the lookup of the typed identity in the section
roll and the decision on impediments and justifications (`CProcuraEleitor`, `CEleitorEncontrado`,
`IEleitorImpedidoVotar`, `IIniciaJustificativa`, `IConfirmaJustificativa`), the "outras opções" menu
(`CEscolheOpcao`: audio, **encerrar votação**, register mesários, biometric counters), the first states of
the operator's **encerramento** (closing of the vote), and the last-resort release of a voter with the
**mesário's own fingerprint** (`CRegistraDigitalOperador`). About a third of the 97 functions are form-builder
template instances, libc++ `std::sort`/`std::map` internals and small outlined calls.

**1 of the 97 functions ran during the recorded votes**: `api_f1055` (`std::filesystem::__status`, reached
from start-up code: `std::filesystem::create_directories` (func 2548) and
`CPacoteArquivos::ValidarChaveEAplicacaoValida` (func 4625), per the runtime edge files). Everything else runs on the operator thread, which the web build never starts (§2).

Reconstructed sources (all under `src/uenux2/src/app/vota/operador/`):

| file | content |
|---|---|
| `cthreadoperador.{h,cpp}` | `CThreadOperador` (Run, dtor, FinalizaExecucao, LogDebug), `CMessageOperador` |
| `leidentidade/cpedeidentidade.{h,cpp}` | `CPedeIdentidade` (ctor from u17 merged in, StartState, FinishState, ProcessInput, ProcessTick) |
| `leidentidade/celeitorencontrado.{h,cpp}` | `CEleitorEncontrado::StartState`, impediment log, decision table |
| `leidentidade/ieleitorimpedidovotar.{h,cpp}` *(path inferred)* | base class of the six "cannot vote here" screens + their singletons |
| `justificativa/iiniciajustificativa.{h,cpp}`, `justificativa/iconfirmajustificativa.{h,cpp}` | the justification entry and confirmation states |
| `outrasopcoes/cescolheopcao.{h,cpp}` | the "outras opções" menu (the u02 fragment `cescolheopcao.u02.cpp` holds the counter texts) |
| `outrasopcoes/cencerramentohorarioinvalido.{h,cpp}` | closing requested too early (+ clock jump in demo/training) and the shared "leave on key" body |
| `outrasopcoes/caguardaeleitoresvotarem.cpp` | "Aguarde até todos os eleitores presentes votarem" |
| `confirmaidentidade/cregistradigitaloperador.{h,cpp}` | release by the mesário's fingerprint (ctor from u17 merged in) |
| `u27-foreign-fragments.cpp` | functions of this unit whose original file is another one: `CAguardaInicio::GetInst`, `CProcuraEleitor::StartState`, `CIniciaFinalizacao::StartState`, `CPedeAnoNascimentoSemBiometria::ProcessInput`, several `GetInst`, `comum::EhTreinamentoEleitor`, `CRegistrarMesarios::GetInst`, `CControladorReconhecimentoMesario::GetInst`, form-builder template summaries |

Related units: u10 (the rest of `operador/`: habilitação by fingerprint, `CMostraEleitorVotando`,
`IInformacaoThreadOperador`), u17 and u19 (fragments of neighbouring states, e.g. `CPedeTituloEncerramento`,
`CValidaIdentidade`), u22 (registration of mesários), u06/u07 (the voter thread), `docs/bu/codepath.md`
(the whole encerramento → BU path).

## 1. Glossary (additions to u10 §1)

| term | meaning |
|---|---|
| operador / mesário / terminal do mesário (MT) | poll worker; his micro-terminal: 4×40 LCD, numeric keypad with CORRIGE / CONFIRMA, LED, buzzer, fingerprint sensor. From model 2020 on the MT LCD is graphical and shows the voter's photo |
| identificador / identidade | the number typed to find the voter: título de eleitor (12 digits), CPF (11) or "identificador" |
| seção / caderno | polling section / the paper roll voters sign |
| impedimento | reason why a voter of the roll cannot vote in this section in this round (`ModuloImpedidos.TipoImpedimento`, per round P1/P2) |
| justificativa (de ausência) | statement that the voter could not vote in his own section; the urna records it (`CJustificador`, `jufa.dat`/BUJ) instead of a vote |
| trânsito / voto em trânsito | voting temporarily in another place (requested in advance). "TTE" = transferência temporária de eleitor |
| encerramento / encerrar votação | closing of the vote at the end of the day, done by the presidente da mesa on the MT; ends in the BU |
| HorariosUrna | the four configured times: `emissaoZeresima`, `inicioVotacao`, `encerramentoVotacao` (earliest closing, e.g. 17:00), `terminoVotacao` (after it, with nobody voting for 5 min, voting is blocked) |
| inspeção | random booth inspection every 60–90 min (u10 §3.1) |
| habilitação por código do mesário | release of a voter whose biometrics failed, authorised by the mesário's own fingerprint (`ETipoHabilitacao` 2, QR `HBBG`) |
| demonstração / treinamento | demo mode (`IInterfaceInit::GetDemoMode`) / training phase (CEstadoGeral fase `'3'`). "Treinamento de eleitores" = training phase + `treinamentoEleitor` flag (the public simulator); "treinamento de mesários" = training phase without that flag |

## 2. Where this code runs (web build vs. urna) and how it was checked

VOTA has two state-machine threads (u06 §2, u10 §2). `CThreadEleitor` drives the voter terminal and is
stepped by `votaTick` in the web build. `CThreadOperador` (this unit) drives the MT. In the web build
nothing calls `CThreadOperador::Run` (func 10204): `main` registers `CExecucaoVotaCooperativa`, which only
steps the voter thread. So every state of this unit is **dead code in the simulator**; the voter is released
by `votaInit` (voter-training mode), and the messages the voter thread posts to the operator (6, 9, 13, 1 …)
stay unread in `CThreadOperador +36` (u10 §2).

**Harness checks made for this unit.** `tools/bu/operator_harness.mjs` runs a patched copy of the wasm with
an `opStep` export (one iteration of `Run`), a fake clock (default 2026-10-04 16:50 -03:00) and stubbed
`emscripten_sleep`. Two runs (outputs in a temporary directory; commands reproducible):

```
node tools/bu/operator_harness.mjs --out <dir> --script "geradin zeresima ot:20 om:8 ot:50 o:D ot:20 o:4C ot:20 \
     o:D ot:20 o:2C ot:30 o:D ot:30 o:123C ot:30 o:D ot:30 o:XXXXXXXXXXXXC ot:50 o:D ot:30"
node tools/bu/operator_harness.mjs --out <dir> --script "geradin zeresima ot:20 om:8 ot:50 \
     clock:2026-10-04T17=05=00-03=00 ot:20 o:D ot:20 o:2C ot:30 o:D ot:30 o:D ot:30 o:1C ot:30 o:D ot:30 o:D ot:30 o:3C ot:30 o:D ot:30"
```

Observed operator states and MT text (all as reconstructed):

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

The mesário-fingerprint path (`CRegistraDigitalOperador`) cannot be run: no fingerprint interface is
registered in the web build and the WSQ/template code is compiled out (u10 §6.4).

## 3. Classes and hierarchy (RTTI)

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

Every state is a lazily created singleton (static `unique_ptr` + mutex; in this single-threaded build only
the `mutex::unlock` stub (func 150) remains). Most `GetInst` functions and constructors are **inlined into
their first caller** (LTO), which is why the tools placed the construction of `CEncerramentoHorarioInvalido`,
`CConfirmaAudio`, `CContadoresBiometria`, `CHabilitacaoAudioNaoPermitida`, `CIniciaFinalizacao`,
`CAguardaInspecao`, `CEleitorJaVotou`, `CTentativaCapturaDigitalEsgotada`, `CDigitalNaoCapturada`,
`CInformaAnoNascimentoErrado` and five `IEleitorImpedidoVotar` subclasses inside the functions of this unit.

### 3.1 `CThreadOperador` layout (132 bytes, built by func 270)

| offset | member |
|---|---|
| +0..+19 | `api::CThread`: vptr, `m_estado` (+4, 1 = started), `m_bParar` (+8), `m_bDormindo` (+9), `m_pImpl` (+12, `IThreadImpl`), `m_pSync` (+16, `ISyncCtl`) |
| +20 | `CThreadVota::m_ticks` (`std::map<uebyte, STick>`, `CriaTick` = func 807, `StartTick` 700, `StopTick` 422) |
| +32 | `CThreadVota::m_pContexto` (`comum::CAppStateContext`) |
| +36..+83 | `CMessageOperador` (vector of 16-byte `SMessage`, semaphore +56, lock +60, `CMessageInterface` +68) |
| +84 | `std::string` year of birth typed in the justification (`CPedeAnoNascimento`, func 10590) |
| +96 | `std::string` título typed in the mesário registration (u22 slots 17/18) |
| +108 | `std::string` "VOTANDO PARA: …" cargo text, initialised to `" "` (u10) |
| +120 | `std::string` título typed by the presidente to close the vote (u17 `CPedeTituloEncerramento`) |

## 4. Control flow

### 4.1 The operator thread (`CThreadOperador::Run`, func 10204, srcloc :87)

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

`VerificaTrocaEstado` (inlined 3×): `if (state->NeedChangeState()) { FinishState(); state = GetNextState();
if (state) StartState(); }`. `FinalizaExecucao` (slot 5, func 10203), called by `CThreadVota`'s exception
handlers, sets `m_bParar` of `CThreadEleitor` and of the power monitor thread `CThreadMonitor` (from
`vota::CThreadMonitor::GetInst`, func 1898, formerly shown by the tools as `CMonitoraAlimentacao::CreateInst`):
an exception escaping any operator state stops the whole application.

### 4.2 Waiting for the start (`CAguardaInicio`, func 5343 = GetInst + ctor)

Screen (non-interactive, `CriaForm("")`): line 1/2 = `format("{}: {}", CApplication::nome, versão)` split at
the first `-` and trimmed ("VOTA: 10.23.0.1" / "DESENVOLVIMENTO"), line 3 "Siga as instruções na tela do
eleitor", line 4 date `DD/MM/YYYY` at (11,4) and time `hh:mm:ss` at (22,4) (text providers slots 1103/1104).
`CAppState(5)` (messages + ticks) and a 500 ms tick. Message 8 (from `CInicioVotacao`) leads to
`CPedeIdentidade` (func 10217, other unit). `CControladorRegistraMesariosVota` slot 9 (func 10794) also returns
here after the initial registration of mesários.

### 4.3 Identification (`CPedeIdentidade`)

`StartState` (10680): log "Aguardando digitação do identificador do eleitor"; `LimpaIdentidades()`
(IInformacaoThreadOperador slot 20); show the form; on urna models ≥ 2020 (`IUrna::GetModelo`, :62) call `IScreenMT` slot 19 with `false` (:63)
(`CEscolheOpcao` passes `true`; the web mock only refreshes – name unknown); re-attach the MT LCD to the battery
icon (`CInfoMTLCD`, funcs 2284 + 5903, replaces the last voter's photo); start the 60 s tick unless in the
training phase or already blocked by time; start the 5 s and 1 s ticks unless in voter-training mode.

| tick | action |
|---|---|
| 60 s (+11) | if `VotacaoBloqueadaPorHorario()` (u10 §3.1: after `terminoVotacao` and ≥ 300 s since the last vote, never in training): stop the tick, log(2) "Votacao foi bloqueada por horario" |
| 5 s (+12) | if now ≥ `GetDataHoraProximaInspecao()`: draw the next one (now + 60..90 min), post message **12** to the voter thread and go to `CAguardaInspecao` ("Inspecione cabina e urna / Instruções no terminal do eleitor", buzzer 51/10) |
| 1 s (+13) | status line 4 = "CORRIGE: outras opções" while the input field is empty, `" "` otherwise |

`ProcessInput` (10677):

* CORRIGE → `CEscolheOpcao` (§4.5).
* CONFIRMA in **voter-training mode** → the typed text is ignored; `CEleitores` is rewound to its first voter,
  whose principal identity becomes the "typed" identity → `CNomeEleitor`. In the public simulator's mode every
  voter is therefore the first voter of the roll.
* CONFIRMA otherwise: empty field → nothing; log "Identificador do eleitor digitado pelo mesário"; voting
  blocked by time → `CHorarioVotacaoTerminou` ("Horario de votacao terminou! / Favor encerrar a urna!",
  CORRIGE → back); else the number is left-padded with `'0'` to 12 digits, stored with
  `SetIdentidadeDigitada` and → `CValidaIdentidade` (u17: chooses the identity type, menu when ambiguous,
  `CIdentidadeInvalida` "Número errado" when no validator accepts it) → `CProcuraEleitor`.

### 4.4 Roll lookup, impediments and justification

`CProcuraEleitor::StartState` (10631): `CEleitores::Procura(Formata(identidade), tipo)` (func 2264, positions
the current voter).

* found → `CEleitorEncontrado`;
* not found, `aceitarJustificativa` and the typed type is **título** → `CIniciaJustificativa` with
  `CConfirmaJustificativa` ("não pertence <S|a|ao|à> <SCSN>" expanded by `CTradutorFrase::TraduzLabel`);
* otherwise → `CEleitorNaoEncontrado` ("NÃO CADASTRADO nesta urna", or "CPF não encontrado. Digite o Título."
  when a CPF was typed and the principal identity is the título; CONFIRMA: retornar).

`CEleitorEncontrado::StartState` (10635; throws 9394 "Eleitor não posicionado" (:102) if the roll has no
current voter). With `imp` = impediment of the current round (`CEleitorDetalhe` +196 1st round / +200 2nd
round; md enum = ASN.1 `TipoImpedimento` − 1):

| condition | next state (MT lines 2/3, key) |
|---|---|
| `imp == 0`, dynamic state `SEM_CARGO_PARA_VOTAR` (1) | `CEleitorNaoPossuiCargosParaVotar` "não está apto a votar nesta eleição." (CORRIGE) |
| `imp == 0`, dynamic state `VOTOU` (3) | `CEleitorJaVotou` name / identity / "JÁ VOTOU" (CONFIRMA: prosseguir) |
| `imp == 0`, otherwise | `CNomeEleitor` (u10: name, photo, habilitação) |
| `imp != 0`: log "Eleitor impedido - <motivo>" (inlined `CLogVota::LogaEleitorImpedido`, clogvota.cpp:432; `imp == 15` throws 9388 "Tipo inválido", `imp ≥ 16` logs an empty reason) and then: | |
| justification NOT accepted (`!aceitarJustificativa`, func 4578), `imp == 2` solicitou voto em trânsito | `CEleitorOptouPorVotarEmTransito` "Optou por votar em trânsito" (CONFIRMA) |
| justification not accepted, other `imp` | `CEleitorNaoEncontrado` (same screen as a voter of another section) |
| `imp` ∈ {4 suspenso, 5 cancelado, 6 não liberado} | `CEleitorImpedidoJustificar` "está impedido de votar ou justificar." / "Eleitor deve procurar cartório eleitoral" (CORRIGE) |
| `imp == 7` sem idade mínima | `CEleitorNaoTemIdadeMinima` "está impedido de votar ou justificar" / "por não ter idade mínima" |
| `imp == 2` and a non-zero entry in the `std::set<int>` at `CConfiguracaoEleicao +604` (meaning not identified) | `CIniciaJustificativaTransito` → `CConfirmaJustificativaTransito` "optou por votar EM TRÂNSITO" |
| `imp == 2` otherwise | `CEleitorImpedidoJustificarVotoTransito` "está impedido de votar ou justificar." / "Eleitor solicitou voto em trânsito" |
| other `imp` (1 vota na seção original, 3 preso provisório, 8 militar, 9–14 TTE …) | `CIniciaJustificativaTemporario` → `CConfirmaJustificativaTemporario` "está impedido de votar nesta seção" |

Justification states:

* `IIniciaJustificativa` (ctor 3624 throws 9396 "Confirmação de justificativa nula" (:27) for a null
  confirmation). `StartState` (10587) looks the voter's título up in `comum::CJustificador` (the justifications
  already recorded in this urna, `CNumeroInscricaoEleitoral` key): already there → `CJustificativaEfetuada` with
  `m_novaJustificativa = false` ("já justificou" variant); otherwise → its `IConfirmaJustificativa`.
* `IConfirmaJustificativa` (ctor 3625, the label must fit the 40-column LCD, else throws 9395
  "Frase muito grande [<frase>]" (:34)): MT "<typed identity> / <frase> / CORRIGE: retornar  CONFIRMA: justificar".
  `StartState` (10589) shows it and the voter's photo (`ApresentaFotoEleitor`, u05). `ProcessInput` (10588,
  other unit) continues to the birth-year question (`CPedeAnoNascimento`, u17), which records the justification
  (`CJustificador::Justifica`, `qtdJustificativas++`, MI/MV sync) and shows `CJustificativaEfetuada`
  ("CONFIRMA: continuar"). A CPF-typed identity goes to `CEleitorImpedidoJustificarVotoCPF` instead.

### 4.5 "Outras opções" (`CEscolheOpcao`)

Construction (2753): the menu is a `std::map<int, SOpcao>` keyed by the number shown; each entry has an action
and a text source `COpcaoDS{numero, std::function<string()>}` drawn as `format("{}-{}")`:

| key | shown when | text | action on CONFIRMA (log "Operador selecionou: <texto>") |
|---|---|---|---|
| 1 | always | "Ativar áudio" / "Desativar áudio" (by `GetAudioHabilitadoManualmente`) | voting blocked by time → clear the field, `CHorarioVotacaoTerminou`; `ParametrosUrna.permitirHabManualAudio` false → `CHabilitacaoAudioNaoPermitida` ("Ativação de áudio não permitida.", 1 s, back to the menu); else `CConfirmaAudio` "Deseja realmente (des)ativar o áudio?" (CORRIGE: não / CONFIRMA: sim) |
| 2 | always | "Encerrar votação" | `CIniciaFinalizacao` (§4.6) |
| 3 | `DeveRegistrarMesarios()` (func 2520: not demo ∧ `registrarMesarios` ∧ not voter training) | "Registrar mesários" | `comum::CRegistrarMesarios` "Registrar mesário?" (u22) |
| 3 or 4 | urna biométrica ∧ not voter training | "Exibir contadores" | `CContadoresBiometria`: "Habilitação biométrica / biográfica / sem biometria: NNNN" (or "n.a."), CORRIGE: retornar |

Positions: 1 → (1,2), 2 → (1,3), 3 → (20,2), 4 → (20,3); one-digit input at (20,1). CORRIGE → `CPedeIdentidade`.
A non-digit, unknown or unavailable choice leaves the menu unchanged. `StartState` (10690) also writes a
debug line directly with `syslog(LOG_INFO, "%s:%d> %d", "StartState", 150, modelo > 2019)`.

### 4.6 Encerramento, operator side (the part in this unit)

This is how the presidente da mesa starts the end of the day. Steps in this unit are marked **(u27)**; the
rest is documented in u17 and `docs/bu/codepath.md` §2.3.

1. **(u27)** `CPedeIdentidade` —CORRIGE→ `CEscolheOpcao` —"2" CONFIRMA→ log "Operador selecionou: Encerrar
   votação" → `CIniciaFinalizacao`.
2. **(u27)** `CIniciaFinalizacao::StartState` (10706), no screen:
   * voter-training mode (`EhTreinamentoEleitor`, the simulator's mode) → `CPerguntaFilaEleitorVazia`;
   * now ≥ `HorariosUrna.encerramentoVotacao` (cfg +568; the scenario's value is 17:00:00):
     voting already blocked by time (`VotacaoBloqueadaPorHorario`) → `CPedeTituloEncerramento` directly
     (nobody can be waiting), else → `CPerguntaFilaEleitorVazia`;
   * before it → log(2) "Encerramento só pode ser solicitado após as HH:MM:SS horas" →
     `CEncerramentoHorarioInvalido`.
3. **(u27)** `CEncerramentoHorarioInvalido::StartState` (10710): "Encerramento de votação inválido / Antes do
   horário / CORRIGE: retornar". **In demo mode or in mesário training** it also sets the urna clock to
   `encerramentoVotacao − 10 s` (16:59:50 for 17:00:00; `IAjusteDataHora` slot 0, :73), adds the jump to
   `CEstadoGeral.ajusteDataHora` (`AdicionaDeltaT`), saves the state (`SalvaEstado`, func 491) and `eg.bin`
   (func 3333) — so a trainee can retry 10 seconds later. CORRIGE → `CPedeIdentidade`.
4. `CPerguntaFilaEleitorVazia` (ctor 5426 **(u27)**; methods elsewhere): "Todas as pessoas presentes já
   votaram?" — CONFIRMA: sim → `CPedeTituloEncerramento`; CORRIGE: não → **(u27)**
   `CAguardaEleitoresVotarem` (10718): "Aguarde até todos os / eleitores presentes votarem" for 3 s
   (`emscripten_sleep(3000)` in this build), `IInputMT::Flush()` (:47, drops keys typed meanwhile), back to `CPedeIdentidade`.
5. `CPedeTituloEncerramento` (ctor 3631 **(u27)**, logic u17): "Informe seu título para / encerrar a votação",
   12-digit field; it keeps a copy of `encerramentoVotacao` at +20; the título is validated
   (`CValidadorIdentidade`), stored at `CThreadOperador +120`, an early closing asks
   "Encerramento de votação antecipado?".
6. `CConfirmaEncerramento` → `CEstadoGeralVota` `estadoVota = 57` (fim da aquisição de votos), `CFimAquisicaoVotos`
   → (registration of mesários at the end if `DeveRegistrarMesarios()`, u22) → message **7** to the voter
   thread → `CGeraBU` (u09): the BU.

Corrections to `docs/bu/codepath.md` §2.3 from this unit's code: the first test of `CIniciaFinalizacao` is
**voter-training mode** (func 697), not the training phase; after `encerramentoVotacao` the choice between
`CPedeTituloEncerramento` and `CPerguntaFilaEleitorVazia` depends on `VotacaoBloqueadaPorHorario`; and
`comum_f2520` (used by `CFimAquisicaoVotos` and by the menu) is `DeveRegistrarMesarios` = `IdentificaMesarios()`
(**not** demo ∧ `registrarMesarios`) ∧ not voter training — not "demo mode".

### 4.7 Release by the mesário's fingerprint (`CRegistraDigitalOperador`)

Reached from `CVerificaDadoEleitor` (birth year confirmed after the last failed fingerprint attempt) and from
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

The identification never blocks the release: any detected finger releases the voter. What changes is what is
logged and whether `s_tituloMesario` is filled; `CMostraEleitorVotando::SalvaHabilitacaoEleitor` (u10 §4.5)
later stores the capture (encrypted) under `wsq/operador/` and writes `tipo_habilitacao = 2` and the mesário's
título (if known) into `eleitor_dinamico`.

### 4.8 Birth year without biometrics (`CPedeAnoNascimentoSemBiometria::ProcessInput`, 10489)

CORRIGE → log "Habilitação cancelada durante confirmação de dado do eleitor", `CCancelaHabilitacaoEleitor`.
CONFIRMA with ≥ 4 digits: `stoul` compared with the roll's birth year: equal → log(1) "Ano de nascimento digitado é
igual ao do cadastro", `CInformaEleitorPodeVotar`; different → log(2) "… diferente do cadastro"; the first error
shows "ANO DE NASCIMENTO INCORRETO / CONFIRMA: tentar novamente" (the second form), the second one goes to
`CInformaAnoNascimentoErrado` ("Por favor oriente o eleitor a procurar o / cartório eleitoral para consultar a data /
de nascimento dele no cadastro da urna / CONFIRMA: cancelar a habilitação", form name
`telaInformaAnoNascimentoErrado`).

## 5. MT screens built by this unit

| state | LED / sound | lines (SPoint{col,line}, alignment) |
|---|---|---|
| message 0 in `Run` | — | (20,1)c "URNA ELETRÔNICA INOPERANTE"; (20,2)c "Siga as instruções na tela do eleitor" |
| CAguardaInicio | — | (1,1)c "VOTA: 10.23.0.1"; (1,2)c "DESENVOLVIMENTO"; (1,3)c "Siga as instruções na tela do eleitor"; (11,4) date; (22,4) time |
| CPedeIdentidade | off / buzz(51,10) | see u17: "Digite o Título ou o CPF", clock (33,1), 12-digit input (1,2), "0000/0150", audio (40,4)r, status (1,4). Voter-training variant: (1,1) "TREINAMENTO DE ELEITORES", (30,2) "Votos:", (37,2) count, (40,3)r audio, (1,4) "CORRIGE: outras opções", (1,4)r "CONFIRMA: votar" |
| CAguardaInspecao | buzz(51,10) | clock; (1,2) "Inspecione cabina e urna"; (1,3) "Instruções no terminal do eleitor" |
| CEscolheOpcao | — | clock; (1,1) "Selecione a opção: "; options (1,2)(1,3)(20,2)(20,3); (1,4) "CORRIGE: retornar"; (40,4)r "CONFIRMA: prosseguir"; input (20,1) |
| CHabilitacaoAudioNaoPermitida | beep(2) | (1,2)c "Ativação de áudio não permitida." |
| CConfirmaAudio | — | clock; (1,2) "Deseja realmente ativar/desativar o áudio?"; (1,4) "CORRIGE: não"; (40,4)r "CONFIRMA: sim" |
| CContadoresBiometria | — | (35,1)r / (35,2)r / (35,3)r counters; (1,4) "CORRIGE: retornar" |
| CEncerramentoHorarioInvalido | off | clock; (1,2)c "Encerramento de votação inválido"; (1,3)c "Antes do horário"; (1,4) "CORRIGE: retornar" |
| CPerguntaFilaEleitorVazia | — | clock; (1,2) "Todas as pessoas presentes já votaram?"; (1,4) "CORRIGE: não"; (40,4)r "CONFIRMA: sim" |
| CPedeTituloEncerramento | — | clock; (1,1) "Informe seu título para"; (1,2) "encerrar a votação"; input 12 (1,3); (1,4) "CORRIGE: retornar"; (40,4)r "CONFIRMA: prosseguir" |
| IEleitorImpedidoVotar family | off | (1,1) typed identity; (1,2)/(1,3) texts; (40,4)r "CONFIRMA: retornar" or (1,4) "CORRIGE: retornar" |
| CEleitorJaVotou | off | (1,1) name "{:2}"; (1,2) identity; (1,3) "JÁ VOTOU"; (40,4)r "CONFIRMA: prosseguir" |
| IConfirmaJustificativa family | off | (1,1) identity; (1,2) frase; (1,4) "CORRIGE: retornar"; (40,4)r "CONFIRMA: justificar" |
| CJustificativaEfetuada | off | (1,1) identity; (1,2) text source ("justificou/ja justificou" placeholder); (1,4)r "CONFIRMA: continuar" (position (1,4) with right alignment in the binary) |
| CRegistraDigitalOperador | off | "MESÁRIO:" / "Posicione seu dedo POLEGAR ou INDICADOR" / "sobre o sensor" / "CORRIGE: não habilitar"; then beep(1) "Digital capturada" (20,1)c, "Por favor aguarde" (20,3)c |
| CTentativaCapturaDigitalEsgotada / CDigitalNaoCapturada | off / beep(1) | see §4.7 |
| CInformaEleitorPodeVotar (func 5410, shared with CControlaReconhecimento) | off / beep(1) | (1,1) "ELEITOR(A) PODE VOTAR" / (1,2) "Assinar o caderno de votação antes de" / (1,3) "votar" / (1,4)r "CONFIRMA: prosseguir" (position (1,4) with right alignment, not (40,4)) |
| comum::CRegistrarMesarios | buzz(51,10) | (1,1) "Registrar mesário?"; (1,4) "CORRIGE: Cancelar  CONFIRMA: Prosseguir" |

## 6. Weird or risky code

1. **Clock jump before a check that can throw** (`CEncerramentoHorarioInvalido::StartState`, 10710). In demo
   mode or mesário training the clock is set to `encerramentoVotacao − 10 s` **first**, and only then
   `CAjusteDataHora::AdicionaDeltaT` is called; it throws 8071 unless the adjust mode is `eAlterarDataSistema`
   (2). In the harness (mode 0 in its seeded `eg.bin`) the exception escaped the state after the MT clock had
   already been changed (the MT clock read 16:50 before and 16:59 after) and `eg.bin` was not updated. On a real urna an exception escaping an
   operator state reaches `CThreadVota`'s handler, whose `FinalizaExecucao` stops the voter thread too. Whether a
   real training urna can be in mode ≠ 2 at that point depends on the start-of-day states (`CAjusteInicial`,
   `CIniciodeCiclo`, u06), which the harness skips.
2. **`emscripten_sleep` in two states of this unit**: `CAguardaEleitoresVotarem::StartState` (3000 ms) and
   `CHabilitacaoAudioNaoPermitida::StartState` (1000 ms), guarded by the byte @1584624 (= 1, never written). The
   glue's `_emscripten_sleep` aborts (no ASYNCIFY). Unreachable in the simulator (operator thread not run); the
   harness stubs it. Any future web build that runs the operator thread would abort there.
3. **Mesário fingerprints written unencrypted.** The "unidentified mesário" capture is written by
   `comum_f5371` with `CFile` "w+b" + `RawWrite` + `CSystem::CopyFile` into `wsq/nao-registrado/` of both flashes
   (and u22's registration path writes `wsq/registrado/` the same way). Voters' images go through
   `CControlaArmazenamentoDeImagens` with `CifrarWsq` (u10/u24). In this build the WSQ encoder is a stub, so the
   files would be empty; on the urna they would hold the mesário's WSQ in clear. Privacy observation for the real
   system (not verifiable here).
4. **Any finger releases the voter** (`CRegistraDigitalOperador`). The mesário search only affects the log and
   `s_tituloMesario`; an unidentified finger (even a non-mesário's) still sets `tipo_habilitacao = 2` and
   releases the voter. The code's own log admits "Não é possível associar a habilitação a um mesário". In the
   web build the whole biometric chain is absent. Info (design choice; the mesário is physically present).
5. **Whole voter roll copied per mesário check** (`BiometriaMesarioPresenteNosEleitores`, inside 3614): the
   `std::map<CEleitorIdentidade, CEleitorDetalhe>` of the section (236-byte nodes including the optional
   biometric data, copied by func 5760) is copied before a single `find`, once per mesário tried (up to three
   passes over all registered mesários) while the voter waits at the MT. The only mesários that skip the copy
   are those with an opening-period registration whose finger was not matched (the function returns `false`
   before copying). Performance / memory, low.
6. **Stale `m_idArquivo` after an unidentified release.** Saving a "nao-registrado" capture goes through
   `comum_f5371`, which sets `CControladorReconhecimentoMesario::m_idArquivo` (singleton). Nothing in this unit
   clears it; u22 notes that the registration of mesários copies this field into the next `comparecimento_mesario`
   row (`id_arquivo`) when no new image is stored, so a later registration could point to a file of the
   `nao-registrado` directory. Low (cross-unit; see u22).
7. **Directory iteration without error code** (`ListaDigitaisNaoRegistradas`, 5374): `directory_iterator(path)`
   throws `filesystem_error` if `<trab>/wsq/nao-registrado/` does not exist. It is also called from the constructor
   of `CControladorReconhecimentoMesario` (1149), i.e. on the first use of that singleton by the mesário
   registration or by this state. Neither the web MEMFS nor the harness has any `wsq/` directory, and no code
   creating it was found in this unit. Low.
8. **Voter-training mode identifies the first voter of the roll** (`CPedeIdentidade::ProcessInput`): the typed
   number is discarded and `CEleitores` is rewound to its first element. Harmless in training, but it is a
   silent substitution of identity driven only by the `treinamentoEleitor` flag in `vota.bin`. Info.
9. **Impediment 15 throws, ≥ 16 is logged as "Eleitor impedido - "** (inlined `LogaEleitorImpedido`): an
   out-of-range impediment coming from the roll data (`-imp.dat`) either stops the operator thread (15) or
   produces an empty reason. The ASN.1 enumeration ends at 15 (= md 14), so 15 is the first invalid value. Low.
10. **Debug `syslog` left in `CEscolheOpcao::StartState`** ("%s:%d> %d", `__func__`, 150, model > 2019), written
    with `vsyslog` directly, bypassing `CLogVota`. Info.
11. **`CRegistraDigitalOperador` checks `me999999.wsq`**: when a mesário has no image id, 999999 is still used to
    build the file name in step b) of `IdentificaMesario`. Harmless unless such a file exists. Info.

## 7. Data read and written

| data | access |
|---|---|
| `CConfiguracaoEleicao`: `ParametrosUrna.aceitarJustificativa` (+482), `permitirHabManualAudio` (+485), `registrarMesarios` (+488, via `IdentificaMesarios`); `HorariosUrna.encerramentoVotacao` (+568 date, +576 time); allowed identity types; principal identity type (+668); the `std::set<int>` at +604 | read |
| `CEstadoGeral` (fase, turno, `ajusteDataHora` +52), `CEstadoGeralVota` (`treinamentoEleitor` +72) | read; **written** by the clock jump (§4.6 step 3): `eg.bin` / `vota.bin` saved |
| `CEleitores` (roll, current voter, impediments P1/P2, dynamic state, biometric templates) | read; the current voter is positioned (lookup / rewind) |
| `comum::CJustificador` (justifications already recorded) | read (`IIniciaJustificativa`) |
| `comum::CRegistradorMesario` (`comparecimento_mesario` rows, by PK and by identity) | read (`CRegistraDigitalOperador`) |
| `<trab>/wsq/registrado/me%06u.wsq` (internal flash) | read |
| `<trab>/wsq/nao-registrado/me%06u.wsq` (internal + external flash) | listed, read, **written** (raw) |
| `CControlaReconhecimento::s_digitalMesario`, `s_tituloMesario` | written (consumed by u10 `SalvaHabilitacaoEleitor` → `eleitor_dinamico`) |
| `IInformacaoThreadOperador` | typed identity/type, `LimpaIdentidades`, inspection time, `SetHabilitacaoCodigoMesario` |
| system clock (`IAjusteDataHora`) | **set** in demo/training (§4.6) |
| `logd.dat` (CLogVota) | all messages quoted in §4 |
| voter thread queue | message 12 (inspection) |

## 8. Boletim de urna (BU)

This unit does not build or sign the BU. Its contributions are:

* **the start of the encerramento** (§4.6): the "outras opções" menu, the time check against
  `HorariosUrna.encerramentoVotacao`, the "is anybody still in line?" dialogue and the entry into
  `CPedeTituloEncerramento`; the path ends with message 7 to the voter thread, which runs `CGeraBU`
  (`docs/bu/codepath.md` §2.4, u08/u09);
* **the habilitação type 2** (`CODIGO_MESARIO`) set by `CRegistraDigitalOperador`, which is counted in the BU as
  `DetalhamentoComparecimento.qtdEleitoresHabilitadosPorBiografia` and in the QR code as `HBBG` (u10 §8), and the
  mesário identification (`s_tituloMesario`) that goes into `ModuloResultadoUrnaCadastro`
  (`habilitacaoPorCodigo.identificacaoMesario`);
* **the justifications** started here (`IIniciaJustificativa` / `IConfirmaJustificativa`) become the BUJ /
  `jufa.dat` at the end of the day (u17 `CPedeAnoNascimento`, u09);
* the "Exibir contadores" screen shows the same habilitação counters (`HBBM`/`HBBG`/`HBSB` sources) live.

## 9. Web build specifics

* The operator thread never runs (§2); every state here is dead code in the browser.
* No fingerprint scanner/matcher/detection implementation is registered; the first
  `CPolySingletonList::instance<IFingerScanner>()` would throw "PolySingleton - solicitada uma instancia nao criada".
* `IScreenMT` slot 19 is a refresh in `simulador::CWasmScreenMT` (the meaning on the real MT is unknown).
* `emscripten_sleep` aborts (§6.2); the harness replaces it.
* The simulator runs in voter-training mode, where `CPedeIdentidade` ignores the typed number and
  `CIniciaFinalizacao` always asks "Todas as pessoas presentes já votaram?".

## 10. wasm / Emscripten observations

* **Misleading tool names from inlining**: func 3614 carries the srclocs of the inlined
  `BiometriaMesarioPresenteNosEleitores` (one parameter) but takes two (identity + image id) and does the logging
  around it; func 2753 (`CEscolheOpcao::GetInst`) and 2749/1906 (`CValidaIdentidade`) were filed under
  `cpedeidentidade.cpp` because `CPedeIdentidade::ProcessInput` calls/inlines them; func 5343
  (`CAguardaInicio::GetInst`) under `cthreadoperador.cpp` because `Run` calls it.
* **Shared bodies**: func 2295 is the `ProcessInput` of five states (the key code and the caller's srcloc record
  are parameters); func 2902 is the destructor of every "vptr + one shared_ptr at +12" state (1257, 1688 call it
  with their vtable); func 1689 is the deleting destructor of seven classes; func 6012 is the `FinishState` body
  of `CRegistraDigitalOperador` and another state (srclocs passed in).
* **Sorting with a by-value comparator**: `std::sort(..., [](std::string a, std::string b){ return a > b; })`
  instantiates `__introsort`/`__insertion_sort_incomplete`/`__sort4` (5373/5370/2726; `__sort3` is inlined into 2726) around comparator 310, which
  copies both strings (heap allocations for names ≥ 11 chars) on every comparison.
* **Menu map value alignment**: `std::map<int, SOpcao>` nodes are 64 bytes (key at +16, value at +24 because
  `std::function` forces 8-byte alignment), which is why the key and the action id look duplicated in the
  pseudo-code of 2753.
* **Signed/unsigned tests**: the impediment range test is `(imp - 7) >=u -3` (i.e. 4..6), the digit test in the
  menu `(c - 58) <u -10`; the pseudo-code prints both as plain `>=`/`<`.
* Func 3641 (tools name `CTime::operator+=`) holds both `CTime::operator+=(int)` (srcloc ctime.cpp:148, overflow)
  and `operator-=(int)` (:161, underflow) and works on a COPY of the value: for a non-negative argument it returns
  `time − n`. `CEncerramentoHorarioInvalido` calls it with 10, so the clock goes to `encerramentoVotacao − 10 s`
  (not + 10 s); the configuration is not modified. Confirmed in the harness (MT clock 16:59 after the jump).
  The same body is called by `CIniciodeCiclo::AjustaDataHora` (func 7306, argument 10) and
  `CAjusteInicial::ValidaTemposDesligamento` (func 7160, argument 30), so u06's "+ 10 s" / "+ 30 s" should be
  re-checked (they are probably − 10 s / − 30 s as well).

## 11. Open questions

* Meaning of the `std::set<int>` at `CConfiguracaoEleicao +604` that enables the justification of voters who
  requested voto em trânsito.
* Who posts operator message 16 (silent stop) and the exact meaning of `IScreenMT` slot 19 (`bool`) on the real MT.
* Real name of func 3614 (it could be a lambda or a private method such as `ProcuraMesario`).
* Whether `CAjusteDataHora`'s mode is always `eAlterarDataSistema` when `CEncerramentoHorarioInvalido` runs on a
  real demo/training urna (§6.1).
* Where the `<trab>/wsq/{registrado,nao-registrado}/` directories are created.
* `CJustificativaEfetuada` +12 text: the placeholder "justificou/ja justificou" is replaced by its StartState
  (other unit) — exact final texts not seen here.

## 12. Mapping table (all 97 functions of u27)

Columns: wasm index, size in bytes, `*` = observed executing in the recorded votes, tool name, reconstructed
symbol, where it is reconstructed, original file, confidence.

| idx | size | run | tool name | symbol | reconstructed in | original file | conf. |
|---|---|---|---|---|---|---|---|
| 310 | 286 |  | `vota_f310` | `std::__sort helper: comparator lambda [](std::string a, std::string b){return a > b;} (strings by value)` | src/uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp (comment) | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | medium |
| 395 | 327 |  | `vota_f395` | `api::CFormBuilderMT::AddInputControl` | library/inlined helper (summarised in src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp) | uenux2/src/api/gui/cformbuildermt.h (template instance) | medium |
| 697 | 36 |  | `comum_f697` | `comum::EhTreinamentoEleitor` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp | uenux2/src/app/comum/appinfo/cappinfo.cpp | high |
| 728 | 381 |  | `vota_f728` | `api::CFormBuilderMT::Add<api::CClockFieldMT>` | library/inlined helper (summarised in src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp) | uenux2/src/api/gui/cformbuildermt.h (template instance) | medium |
| 1055 | 33 | * | `api_f1055` | `std::filesystem::__status(const path&, error_code*)` (libc++, behind `status()`) | library | libcxx filesystem (operations.cpp) | medium |
| 1149 | 252 |  | `comum_f1149` | `comum::CControladorReconhecimentoMesario::GetInst` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp | uenux2/src/app/comum/comparecimentomesario/ccontroladorreconhecimetomesario.cpp | high |
| 1151 | 330 |  | `vota_f1151` | `api::CFormBuilderMT::Add<api::CInputFieldMT>` | library/inlined helper (summarised in src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp) | uenux2/src/api/gui/cformbuildermt.h (template instance) | medium |
| 1257 | 12 |  | `vota::IEleitorImpedidoVotar::vf0` | `vota::IEleitorImpedidoVotar::~IEleitorImpedidoVotar` | src/uenux2/src/app/vota/operador/leidentidade/ieleitorimpedidovotar.cpp | uenux2/src/app/vota/operador/leidentidade/ieleitorimpedidovotar.cpp (path inferred) | high |
| 1687 | 20 |  | `api_f1687` | `vota::(anon)::AudioHabilitadoManualmente [IInformacaoThreadOperador::GetInst().GetAudioHabilitadoManualmente()]` | src/uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | medium |
| 1688 | 12 |  | `vota::IConfirmaJustificativa::vf0` | `vota::IConfirmaJustificativa::~IConfirmaJustificativa` | src/uenux2/src/app/vota/operador/justificativa/iconfirmajustificativa.cpp | uenux2/src/app/vota/operador/justificativa/iconfirmajustificativa.cpp | high |
| 1689 | 13 |  | `vota::IEleitorImpedidoVotar::vf1` | `vota::IEleitorImpedidoVotar::~IEleitorImpedidoVotar (deleting)` | src/uenux2/src/app/vota/operador/leidentidade/ieleitorimpedidovotar.cpp | uenux2/src/app/vota/operador/leidentidade/ieleitorimpedidovotar.cpp (path inferred) | high |
| 1694 | 423 |  | `vota_f1694` | `api::CFormBuilderMT::CriaForm` | library/inlined helper (summarised in src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp) | uenux2/src/api/gui/cformbuildermt.h (template instance) | medium |
| 1902 | 82 |  | `vota_f1902` | `std::__tree<map<CEleitorIdentidade, CComparecimentoMesario>>::destroy` | library/inlined helper | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp (template instance) | medium |
| 1905 | 1239 |  | `vota_f1905` | `vota::IEleitorImpedidoVotar::IEleitorImpedidoVotar` | src/uenux2/src/app/vota/operador/leidentidade/ieleitorimpedidovotar.cpp | uenux2/src/app/vota/operador/leidentidade/ieleitorimpedidovotar.cpp (path inferred) | high |
| 1906 | 57 |  | `vota_f1906` | `std::__tree<map<int, SOpcaoIdentidade>>::destroy (CValidaIdentidade::m_opcoes)` | library/inlined helper | uenux2/src/app/vota/operador/leidentidade/cvalidaidentidade.cpp (path inferred) | medium |
| 2226 | 20 |  | `api_f2226` | `vota::impl::IInformacaoThreadOperador thunk: GetInst().VotacaoBloqueadaPorHorario()` | src/uenux2/src/app/vota/operador/leidentidade/cpedeidentidade.cpp (call sites) | uenux2/src/app/vota/operador/comum/iinformacaothreadoperador.h (outlined inline call) | medium |
| 2295 | 127 |  | `api_f2295` | `vota::RetornaParaPedeIdentidadeSeTecla (shared ProcessInput body)` | src/uenux2/src/app/vota/operador/outrasopcoes/cencerramentohorarioinvalido.cpp | uenux2/src/app/vota/operador/outrasopcoes/cencerramentohorarioinvalido.cpp | medium |
| 2298 | 182 |  | `vota_f2298` | `vota::(anon)::DiretorioWsq(subdir) [<trab fi>/wsq/<subdir>]` | src/uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp (shared helper) | medium |
| 2502 | 15 |  | `api_f2502` | `vota::(anon)::LogaOpcaoSelecionada ["Operador selecionou: {}"]` | src/uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | high |
| 2717 | 268 |  | `vota::CThreadOperador::vf0` | `vota::CThreadOperador::~CThreadOperador` | src/uenux2/src/app/vota/operador/cthreadoperador.cpp | uenux2/src/app/vota/operador/cthreadoperador.cpp | high |
| 2726 | 594 |  | `vota_f2726` | `std::__sort4<..., lambda a>b>(vector<string>)` (four iterators; __sort3 inlined) | library/inlined helper | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp (template instance) | medium |
| 2747 | 908 |  | `vota_f2747` | `vota::CJustificativaEfetuada::GetInst` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp | uenux2/src/app/vota/operador/justificativa/cjustificativaefetuada.cpp (path inferred) | high |
| 2748 | 13 |  | `vota::IConfirmaJustificativa::vf1` | `vota::IConfirmaJustificativa::~IConfirmaJustificativa (deleting)` | src/uenux2/src/app/vota/operador/justificativa/iconfirmajustificativa.cpp | uenux2/src/app/vota/operador/justificativa/iconfirmajustificativa.cpp | high |
| 2749 | 69 |  | `vota::CValidaIdentidade::vf0` | `vota::CValidaIdentidade::~CValidaIdentidade` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp | uenux2/src/app/vota/operador/leidentidade/cvalidaidentidade.cpp (path inferred) | high |
| 2750 | 96 |  | `api_f2750` | `vota::CEscolheOpcao::SOpcao::SOpcao(EAcao, COpcaoDS&&)` | src/uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | medium |
| 2751 | 83 |  | `api_f2751` | `vota::CEscolheOpcao::COpcaoDS::COpcaoDS(int, std::function&&)` | src/uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | medium |
| 2752 | 69 |  | `vota::CEscolheOpcao::vf0` | `vota::CEscolheOpcao::~CEscolheOpcao` | src/uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | high |
| 2753 | 3646 |  | `api_f2753` | `vota::CEscolheOpcao::GetInst` | src/uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | high |
| 2805 | 658 |  | `vota_f2805` | `std::__tree<map<CEleitorIdentidade, ...>>::__find_equal (hint)` | library/inlined helper | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp (template instance) | low |
| 2864 | 175 |  | `comum_f2864` | `comum::CPath::GetPathWsq(flash, turno) [GetPathTrab/"wsq/"]` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp | uenux2/src/app/comum/cpath.cpp (?) | medium |
| 3258 | 17 |  | `vota_f3258` | `vota::(anon)::LogaMesarioHabilitou ["Mesário {} habilitou o eleitor"]` | src/uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | high |
| 3593 | 16 |  | `vota_f3593` | `vota::(anon)::LogDebug (cthreadoperador.cpp copy)` | src/uenux2/src/app/vota/operador/cthreadoperador.cpp | uenux2/src/app/vota/operador/cthreadoperador.cpp | high |
| 3610 | 971 |  | `comum_f3610` | `comum::CRegistrarMesarios::GetInst` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cregistrarmesarios.cpp (path inferred) | high |
| 3614 | 4765 |  | `vota::CRegistraDigitalOperador::BiometriaMesarioPresenteNosEleitores` | `vota::CRegistraDigitalOperador::IdentificaMesario (inlines BiometriaMesarioPresenteNosEleitores)` | src/uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | medium |
| 3619 | 22 |  | `api_f3619` | `vota::impl::IInformacaoThreadOperador thunk: GetInst().SetTipoIdentidadeDigitada(t)` | src/uenux2/src/app/vota/operador/leidentidade/cpedeidentidade.cpp (call sites) | uenux2/src/app/vota/operador/comum/iinformacaothreadoperador.h (outlined inline call) | medium |
| 3624 | 92 |  | `vota::IIniciaJustificativa::IIniciaJustificativa` | `vota::IIniciaJustificativa::IIniciaJustificativa` | src/uenux2/src/app/vota/operador/justificativa/iiniciajustificativa.cpp | uenux2/src/app/vota/operador/justificativa/iiniciajustificativa.cpp | high |
| 3625 | 965 |  | `vota::IConfirmaJustificativa::IConfirmaJustificativa` | `vota::IConfirmaJustificativa::IConfirmaJustificativa` | src/uenux2/src/app/vota/operador/justificativa/iconfirmajustificativa.cpp | uenux2/src/app/vota/operador/justificativa/iconfirmajustificativa.cpp | high |
| 3630 | 81 |  | `vota_f3630` | `std::__tree<map<int, CEscolheOpcao::SOpcao>>::destroy` | library/inlined helper | uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp (template instance) | medium |
| 3631 | 1120 |  | `vota_f3631` | `vota::CPedeTituloEncerramento::GetInst` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp | uenux2/src/app/vota/operador/outrasopcoes/cpedetituloencerramento.cpp (path inferred) | high |
| 3673 | 441 |  | `vota_f3673` | `api::IInputField<api::IScreenMT>::IInputField(validation, tamanho, flag)` | library/inlined helper (summarised in src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp) | uenux2/src/api/gui/iinputfield.h (template instance) | medium |
| 3674 | 273 |  | `vota_f3674` | `std::vector<std::shared_ptr<api::IFormField<api::IScreenMT>>>::push_back(&&)` | library/inlined helper | libcxx vector (template instance) | medium |
| 3769 | 27 |  | `vota_f3769` | `vota::(anon)::ImpedimentoTurnoAtual [CEleitorDetalhe +196/+200 by turno]` | src/uenux2/src/app/vota/operador/leidentidade/celeitorencontrado.cpp | uenux2/src/app/vota/operador/leidentidade/celeitorencontrado.cpp | medium |
| 3793 | 15 |  | `vota_f3793` | `vota::(anon)::DiretorioNaoRegistrado` | src/uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | medium |
| 3885 | 32 |  | `vota_f3885` | `std::unique_ptr<vota::IConfirmaJustificativa>::reset (static slot)` | src/uenux2/src/app/vota/operador/justificativa/iconfirmajustificativa.cpp (comment) | uenux2/src/app/vota/operador/justificativa/iconfirmajustificativa.cpp | medium |
| 4522 | 187 |  | `vota_f4522` | `vota::(anon)::LogaHabilitacaoCanceladaCapturaMesario` | src/uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | high |
| 4578 | 41 |  | `vota_f4578` | `vota::(anon)::JustificativaNaoAceita` | src/uenux2/src/app/vota/operador/leidentidade/celeitorencontrado.cpp | uenux2/src/app/vota/operador/leidentidade/celeitorencontrado.cpp | medium |
| 5343 | 2105 |  | `vota_f5343` | `vota::CAguardaInicio::GetInst` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp | uenux2/src/app/vota/operador/caguardainicio.cpp (path inferred) | high |
| 5370 | 1469 |  | `vota_f5370` | `std::__insertion_sort_incomplete<lambda a>b, string*>` | library/inlined helper | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp (template instance) | medium |
| 5373 | 7522 |  | `vota_f5373` | `std::__introsort<lambda a>b, string*>` | library/inlined helper | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp (template instance) | medium |
| 5374 | 988 |  | `vota_f5374` | `vota::(anon)::ListaDigitaisNaoRegistradas` | src/uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | medium |
| 5376 | 343 |  | `vota_f5376` | `std::map<CEleitorIdentidade, CComparecimentoMesario>::__emplace_hint_unique` | library/inlined helper | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp (template instance) | medium |
| 5399 | 105 |  | `vota_f5399` | `vota::CInformaEleitorPodeVotar::GetInst` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp | uenux2/src/app/vota/operador/confirmaidentidade/cinformaeleitorpodevotar.cpp (path inferred) | high |
| 5410 | 1258 |  | `vota_f5410` | `vota::CriaFormEleitorPodeVotar` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp | uenux2/src/app/vota/operador/confirmaidentidade/cinformaeleitorpodevotar.cpp (path inferred) | medium |
| 5414 | 22 |  | `api_f5414` | `vota::impl::IInformacaoThreadOperador thunk: GetInst().SetIdentidadeDigitada(s)` | src/uenux2/src/app/vota/operador/leidentidade/cpedeidentidade.cpp (call sites) | uenux2/src/app/vota/operador/comum/iinformacaothreadoperador.h (outlined inline call) | medium |
| 5419 | 423 |  | `vota_f5419` | `api::CFormBuilderMT::Add<CTextFieldMT>(pos, CDataText<std::function<std::string()>>)` | library/inlined helper (summarised in src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp) | uenux2/src/api/gui/cformbuildermt.h (template instance) | medium |
| 5424 | 263 |  | `vota_f5424` | `vota::CEleitorNaoEncontrado::GetInst` | src/uenux2/src/app/vota/operador/leidentidade/ieleitorimpedidovotar.cpp | uenux2/src/app/vota/operador/leidentidade/celeitornaoencontrado.cpp (path inferred) | high |
| 5426 | 851 |  | `vota_f5426` | `vota::CPerguntaFilaEleitorVazia::GetInst` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp | uenux2/src/app/vota/operador/outrasopcoes/cperguntafilaeleitorvazia.cpp (path inferred) | high |
| 5522 | 16 |  | `vota_f5522` | `api::IForm<api::IScreenMT>::IForm(campos, preShow)` | library/inlined helper | uenux2/src/api/gui/iform.h (template instance) | medium |
| 5815 | 15 |  | `vota_f5815` | `vota::(anon)::DiretorioRegistrado` | src/uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | medium |
| 10203 | 18 |  | `vota::CThreadOperador::vf5` | `vota::CThreadOperador::FinalizaExecucao` | src/uenux2/src/app/vota/operador/cthreadoperador.cpp | uenux2/src/app/vota/operador/cthreadoperador.cpp | medium |
| 10204 | 1873 |  | `vota::CThreadOperador::Run` | `vota::CThreadOperador::Run` | src/uenux2/src/app/vota/operador/cthreadoperador.cpp | uenux2/src/app/vota/operador/cthreadoperador.cpp | high |
| 10206 | 13 |  | `vota::CThreadOperador::vf1` | `vota::CThreadOperador::~CThreadOperador (deleting)` | src/uenux2/src/app/vota/operador/cthreadoperador.cpp | uenux2/src/app/vota/operador/cthreadoperador.cpp | high |
| 10208 | 38 |  | `vota_f10208` | `atexit: CThreadOperador static instance reset (@1911708)` | src/uenux2/src/app/vota/operador/cthreadoperador.cpp (comment) | uenux2/src/app/vota/operador/cthreadoperador.cpp | high |
| 10451 | 6996 |  | `vota::CRegistraDigitalOperador::ProcessTick` | `vota::CRegistraDigitalOperador::ProcessTick` | src/uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | high |
| 10452 | 158 |  | `vota::CRegistraDigitalOperador::vf7` | `vota::CRegistraDigitalOperador::ProcessInput` | src/uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | high |
| 10453 | 17 |  | `vota::CRegistraDigitalOperador::FinishState` | `vota::CRegistraDigitalOperador::FinishState` | src/uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | high |
| 10454 | 288 |  | `vota::CRegistraDigitalOperador::StartState` | `vota::CRegistraDigitalOperador::StartState` | src/uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp | high |
| 10461 | 14 |  | `vota::CTentativaCapturaDigitalEsgotada::vf7` | `vota::CTentativaCapturaDigitalEsgotada::ProcessInput` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp | uenux2/src/app/vota/operador/confirmaidentidade/ctentativacapturadigitalesgotada.cpp (path inferred) | high |
| 10489 | 2040 |  | `vota::CPedeAnoNascimentoSemBiometria::vf7` | `vota::CPedeAnoNascimentoSemBiometria::ProcessInput` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp | uenux2/src/app/vota/operador/confirmaidentidade/cpedeanonascimentosembiometria.cpp (path inferred) | high |
| 10587 | 210 |  | `vota::IIniciaJustificativa::vf2` | `vota::IIniciaJustificativa::StartState` | src/uenux2/src/app/vota/operador/justificativa/iiniciajustificativa.cpp | uenux2/src/app/vota/operador/justificativa/iiniciajustificativa.cpp | high |
| 10589 | 32 |  | `vota::IConfirmaJustificativa::vf2` | `vota::IConfirmaJustificativa::StartState` | src/uenux2/src/app/vota/operador/justificativa/iconfirmajustificativa.cpp | uenux2/src/app/vota/operador/justificativa/iconfirmajustificativa.cpp | high |
| 10604 | 14 |  | `vota::CEleitorImpedidoJustificarVotoCPF::vf7` | `vota::CEleitorImpedidoJustificarVotoCPF::ProcessInput` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp | uenux2/src/app/vota/operador/justificativa/celeitorimpedidojustificarvotocpf.cpp (path inferred) | high |
| 10611 | 12 |  | `vota_f10611` | `atexit: CConfirmaJustificativaTransito instance reset (@1905812)` | src/uenux2/src/app/vota/operador/justificativa/iconfirmajustificativa.cpp (comment) | uenux2/src/app/vota/operador/justificativa/cconfirmajustificativatransito.cpp (path inferred) | medium |
| 10615 | 12 |  | `vota_f10615` | `atexit: CConfirmaJustificativaTemporario instance reset (@1905756)` | src/uenux2/src/app/vota/operador/justificativa/iconfirmajustificativa.cpp (comment) | uenux2/src/app/vota/operador/justificativa/cconfirmajustificativatemporario.cpp (path inferred) | medium |
| 10622 | 12 |  | `vota_f10622` | `atexit: CConfirmaJustificativa instance reset (@1905700)` | src/uenux2/src/app/vota/operador/justificativa/iconfirmajustificativa.cpp (comment) | uenux2/src/app/vota/operador/justificativa/cconfirmajustificativa.cpp (path inferred) | medium |
| 10628 | 13 |  | `vota::CValidaIdentidade::vf1` | `vota::CValidaIdentidade::~CValidaIdentidade (deleting)` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp | uenux2/src/app/vota/operador/leidentidade/cvalidaidentidade.cpp (path inferred) | high |
| 10630 | 38 |  | `vota_f10630` | `atexit: CValidaIdentidade instance reset (@1905672)` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp (comment) | uenux2/src/app/vota/operador/leidentidade/cvalidaidentidade.cpp (path inferred) | high |
| 10631 | 657 |  | `vota::CProcuraEleitor::vf2` | `vota::CProcuraEleitor::StartState` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp | uenux2/src/app/vota/operador/leidentidade/cprocuraeleitor.cpp (path inferred) | high |
| 10635 | 3870 |  | `vota::CEleitorEncontrado::vf2` | `vota::CEleitorEncontrado::StartState` | src/uenux2/src/app/vota/operador/leidentidade/celeitorencontrado.cpp | uenux2/src/app/vota/operador/leidentidade/celeitorencontrado.cpp | high |
| 10667 | 14 |  | `vota::CIdentidadeInvalida::vf7` | `vota::CIdentidadeInvalida::ProcessInput` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp | uenux2/src/app/vota/operador/leidentidade/cidentidadeinvalida.cpp (path inferred) | high |
| 10676 | 1154 |  | `vota::CPedeIdentidade::vf8` | `vota::CPedeIdentidade::ProcessTick` | src/uenux2/src/app/vota/operador/leidentidade/cpedeidentidade.cpp | uenux2/src/app/vota/operador/leidentidade/cpedeidentidade.cpp | high |
| 10677 | 756 |  | `vota::CPedeIdentidade::vf7` | `vota::CPedeIdentidade::ProcessInput` | src/uenux2/src/app/vota/operador/leidentidade/cpedeidentidade.cpp | uenux2/src/app/vota/operador/leidentidade/cpedeidentidade.cpp | high |
| 10678 | 35 |  | `vota::CPedeIdentidade::vf5` | `vota::CPedeIdentidade::FinishState` | src/uenux2/src/app/vota/operador/leidentidade/cpedeidentidade.cpp | uenux2/src/app/vota/operador/leidentidade/cpedeidentidade.cpp | high |
| 10680 | 377 |  | `vota::CPedeIdentidade::StartState` | `vota::CPedeIdentidade::StartState` | src/uenux2/src/app/vota/operador/leidentidade/cpedeidentidade.cpp | uenux2/src/app/vota/operador/leidentidade/cpedeidentidade.cpp | high |
| 10689 | 3059 |  | `vota::CEscolheOpcao::vf7` | `vota::CEscolheOpcao::ProcessInput` | src/uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | high |
| 10690 | 240 |  | `vota::CEscolheOpcao::StartState` | `vota::CEscolheOpcao::StartState` | src/uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | high |
| 10691 | 13 |  | `vota::CEscolheOpcao::vf1` | `vota::CEscolheOpcao::~CEscolheOpcao (deleting)` | src/uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | high |
| 10695 | 113 |  | `vota_f10695` | `vota::(anon)::TextoOpcaoAudio` | src/uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | high |
| 10697 | 38 |  | `vota_f10697` | `atexit: CEscolheOpcao instance reset (@1905308)` | src/uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp (comment) | uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | high |
| 10706 | 1498 |  | `vota::CIniciaFinalizacao::vf2` | `vota::CIniciaFinalizacao::StartState` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp | uenux2/src/app/vota/operador/outrasopcoes/ciniciafinalizacao.cpp (path inferred) | high |
| 10709 | 14 |  | `vota::CEncerramentoHorarioInvalido::vf7` | `vota::CEncerramentoHorarioInvalido::ProcessInput` | src/uenux2/src/app/vota/operador/outrasopcoes/cencerramentohorarioinvalido.cpp | uenux2/src/app/vota/operador/outrasopcoes/cencerramentohorarioinvalido.cpp | high |
| 10710 | 267 |  | `vota::CEncerramentoHorarioInvalido::StartState` | `vota::CEncerramentoHorarioInvalido::StartState` | src/uenux2/src/app/vota/operador/outrasopcoes/cencerramentohorarioinvalido.cpp | uenux2/src/app/vota/operador/outrasopcoes/cencerramentohorarioinvalido.cpp | high |
| 10718 | 107 |  | `vota::CAguardaEleitoresVotarem::StartState` | `vota::CAguardaEleitoresVotarem::StartState` | src/uenux2/src/app/vota/operador/outrasopcoes/caguardaeleitoresvotarem.cpp | uenux2/src/app/vota/operador/outrasopcoes/caguardaeleitoresvotarem.cpp | high |
| 10742 | 173 |  | `api_f10742` | `vota::(anon)::TextoConfirmaAudio` | src/uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | uenux2/src/app/vota/operador/outrasopcoes/cconfirmaaudio.cpp (path inferred) | high |
| 10754 | 49 |  | `vota::CHabilitacaoAudioNaoPermitida::vf2` | `vota::CHabilitacaoAudioNaoPermitida::StartState` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp | uenux2/src/app/vota/operador/outrasopcoes/chabilitacaoaudionaopermitida.cpp (path inferred) | high |
| 10757 | 14 |  | `vota::CHorarioVotacaoTerminou::vf7` | `vota::CHorarioVotacaoTerminou::ProcessInput` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp | uenux2/src/app/vota/operador/outrasopcoes/chorariovotacaoterminou.cpp (path inferred) | high |
| 10794 | 5 |  | `vota::CControladorRegistraMesariosVota::vf9` | `vota::CControladorRegistraMesariosVota::GetEstadoAposRegistroInicial` | src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp | uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp (path inferred) | medium |
