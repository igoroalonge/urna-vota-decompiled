# u06 — `uenux2/src/app/vota/eleitor`: the voter's session (CEleitorVotando and friends)

Unit u06 covers 83 wasm functions of the **voter side** ("eleitor") of the VOTA application:
the state that runs while one voter is at the urna (`vota::CEleitorVotando`), the generic
"conferência" (check) and "confirmação" (confirm) states of a vote, the no-candidate cargo, the
accessibility instructions, the end-of-vote screen, the per-cargo screen registry
(`CTelasCargo`), and the start-up / restart routing states of the voter thread
(`CAjusteInicial`, `CDefineRotaPosReinicio`, `CIniciodeCiclo`, `CEstadoComDesligamentoAutomatico`).
34 of the 83 functions ran during the recorded votes (`analysis/runtime/`).

Reconstructed sources: `src/uenux2/src/app/vota/eleitor/` —
`celeitorvotando.{h,cpp}`, `cconferevotoemcargo.{h,cpp}`, `cconfirmavotoemcargo.{h,cpp}`,
`cconfirmavotosemcandidato.{h,cpp}`, `cinstrucaovotacaoacessibilidade.{h,cpp}`,
`cfimvotoeleitor.cpp`, `cajusteinicial.cpp`, `cdefinerotaposreinicio.cpp`, `ciniciodeciclo.cpp`,
`cestadocomdesligamentoautomatico.cpp`, `comum/ctelascargo.{h,cpp}`,
`comum/cpoliticaexecucaoeleitor.cpp`.

## 1. Glossary

| term | meaning |
|---|---|
| eleitor | voter |
| mesário | poll worker; operates the *terminal do mesário* (operator keypad/display, "TA") |
| habilitar / habilitação | the mesário identifies the voter and releases the urna for him/her |
| cargo | office being voted (Vereador, Prefeito, Senador…); a *consulta* is a referendum question treated as a cargo |
| escolha / vaga | one choice inside a cargo that elects several seats (e.g. 2 Senators → 2 escolhas) |
| voto nominal / de legenda / branco / nulo | vote for a candidate / for a party only / blank / null |
| conferência | the short screen right after the number is complete ("Confira o seu voto.") |
| confirmação | the screen with CONFIRMA / CORRIGE (confirm / correct) |
| suspender | the mesário (or the training mode) ends the session of a voter who did not finish |
| RDV | Registro Digital do Voto: the shuffled list of all votes, source of the BU |
| BU | Boletim de Urna: per-urna result report produced at the end of the day (encerramento) |
| zerésima | report printed before voting showing that the urna holds zero votes |
| MR | *mídia de resultado*: the USB stick that receives the result files |
| trânsito | voting outside one's own município (restricts which cargos can be voted) |
| treinamento eleitor | "voter training" mode of a training-phase urna |

## 2. Where this code runs

The VOTA application has two "threads": the **operator thread** (`CThreadOperador`, the mesário
terminal) and the **voter thread** (`CThreadEleitor`, the voter screen + keypad). Each is a state
machine of `comum::CAppState` objects, driven by events: keys, ticks (timers of the thread's
`api::CTickManager`) and messages (a `api::CPriorityMessageQueue<api::SMessage>` per thread).
`CThreadEleitor::Processar` (func 4349, unit u07; formerly shown by the tools as `ProcessarEntrada`, the
name of one of its inlined helpers) dispatches each event to the current state and then applies the
standard transition protocol:

```cpp
if (state->NeedChangeState()) { state->FinishState(); state = state->GetNextState(); if (state) state->StartState(); }
```

On a real urna the voter thread starts in `IAjusteInicial::GetInst()` (func 7061 →
`CAjusteInicial`), which re-derives the correct state from the persisted `EstadoGeralVota`. **In the
web build `main` instead starts a `CExecucaoVotaCooperativa` whose first state is
`CAguardaMensagem`** (`decompiled/app-wasm-entry/_by_index/10000.dcmp` line 475: `invoke_i(125 = func 1337)` then `CExecucaoVotaCooperativa::CExecucaoVotaCooperativa`), so `CAjusteInicial`,
`CDefineRotaPosReinicio` and the MR wipe never run in the simulator (none of them were observed).

### `comum::CAppState` virtual protocol (recovered slot names)

| slot | method | default (CAppState) | evidence |
|---|---|---|---|
| 0 / 1 | destructor / deleting destructor | | |
| 2 | `StartState()` | pure | srcloc `CFimVotoEleitor::StartState`, `CGeraBU::StartState`… |
| 3 | `bool NeedChangeState()` | `GetNextState() != this` (func 7480) | srcloc `CEleitorVotando::NeedChangeState` |
| 4 | `CAppState* GetNextState()` | returns `m_proximoEstado` (+4) (func 1661) | |
| 5 | `FinishState()` | no-op | `CPedeDigital::FinishState` |
| 6 | `ProcessMessage(uebyte)` *(name inferred)* | no-op | dispatch of queue messages (func 3843) |
| 7 | `ProcessInput()` | no-op | srcloc `CVotacaoStateAudio::ProcessInput` |
| 8 | `ProcessTick(uebyte)` | no-op | srcloc `CVotacaoStateAudio::ProcessTick(uebyte)` |

`CAppState(uebyte flags)` (func 224): +4 `m_proximoEstado = this`, +8 accepts messages (bit 0),
+9 accepts keys (bit 1), +10 accepts ticks (bit 2). Only the *top-level* state's flags matter:
`CEleitorVotando` (flags 7) forwards everything to its sub-state itself.

`vota::CVotacaoStateAudio` (unit u08) adds audio: slot 9 `ProcessInputAudio`, 10
`StartStateAudio`, 11 `FinishStateAudio` (inferred), 12 `ProcessTickAudio(uebyte)` (name from a
lambda type), 13 `EmiteEcoComInputField` (srcloc), 14 message-template expander (`{cargo-atual}`…),
15 `GetMensagemAudio()` (inferred: the text spoken 1.5 s after entering the state and 2 s after each
key). Its constructor is func 1785 (in this unit): `CAppState(flags)`, `m_audioHabilitado`=false,
two stopped ticks of 2000 ms (+12) and 1500 ms (+13). Slots 9, 10, 12 and 15 are pure
(`__cxa_pure_virtual`) in `CVotacaoStateAudio`, so every concrete state must override slot 12:
`CInstrucaoVotacaoAcessibilidade`, `CConfirmaVotoSemCandidato` and `CConfirmaVotoEmCargo` (hence
the ten confirmation states) override `ProcessTickAudio` with a no-op (func 425 through their own
table slots 944, 919, 1986); only `IConfereVotoEmCargo` has a real one (func 11790).

## 3. Classes and hierarchy (RTTI)

```
api::CState
└─ comum::CAppState
   ├─ vota::CEleitorVotando                     (72 B)  voter session, owns the sub-state machine
   ├─ vota::CFimVotoEleitor                     (12 B)  "FIM / VOTOU"
   ├─ vota::IAjusteInicial ─ vota::CAjusteInicial        first state on a real urna
   ├─ vota::CDefineRotaPosReinicio                       routing after a restart
   ├─ vota::CIniciodeCiclo                               start of the voting period
   ├─ vota::CEstadoComDesligamentoAutomatico             battery power-off watchdog (base)
   │    ├─ CVerificaHorarioZeresima, CAplicacaoEncerrada, CMostraQRCodeBU, CMostraQRCodeCertificado
   └─ vota::CVotacaoStateAudio  (u08)
        ├─ vota::CInstrucaoVotacaoAcessibilidade   (36 B)  audio instructions
        ├─ vota::CConfirmaVotoSemCandidato        (32 B)  cargo without candidates
        ├─ vota::IConfereVotoEmCargo              (40 B)  conferência (abstract)
        │    └─ CConfereVotoEmCargo<TConfirma, ETelaVotacao>  ×10 instances
        ├─ vota::CConfirmaVotoEmCargo             (36 B)  confirmação (abstract)
        │    ├─ CConfirmaProporcional ─ CConfirmaVotoNominal, CConfirmaVotoLegenda,
        │    │                          CCandidatoInexistente, CCandidatoInapto,
        │    │                          CProporcionalBranco, CProporcionalNulo
        │    └─ CConfirmaMajoritario  ─ CMajoritarioValido, CMajoritarioRepetido,
        │                               CMajoritarioNulo, CMajoritarioBranco
        └─ CPedeMajoritario, CPedeProporcional, CPedeNominal, CCompletaProporcional, CPedeNulo (u26)

vota::impl::IPoliticaExecucaoEleitor ─ CPoliticaExecucaoEleitor (urna) / (anon)::CPoliticaExecucaoEleitorWeb (web)
vota::CTelasCargo (plain class, 16 B): map<ETelaVotacao, CFormInterativoTelaVota> of one cargo
```

Every state is a lazily created singleton (`static unique_ptr` + mutex; the mutex lock vanished
in the single-threaded build, only `mutex_unlock` residues remain; exit-time resets are funcs 7287,
7416). wasm-opt merged several of these accessors into shared bodies with the vtable and storage as
parameters (funcs 1961, 2901, 6051, 764).

## 4. The voter session

### 4.1 Top-level flow

```
CAguardaMensagem --msg 1 "Eleitor foi habilitado"--> CEleitorVotando
CEleitorVotando  --all cargos done: GravaVotos (func 4454)--> CSincronismoEleitor (u07/u19)
                 --suspension with discard--> CAguardaMensagem          (screen "FIM / NÃO VOTOU")
CSincronismoEleitor --> CFimVotoEleitor ("FIM / VOTOU", 4 beeps, "fim") --> CAguardaMensagem
```

The recorded municipal vote shows exactly this sequence
(`CEleitorVotando/CPedeProporcional → CPedeNominal → CConfereVotoEmCargo<CConfirmaVotoNominal,2>
→ CConfirmaVotoNominal → CPedeMajoritario → CConfereVotoEmCargo<CMajoritarioNulo,7> →
CMajoritarioNulo → CSincronismoEleitor → CAguardaMensagem`).

### 4.2 `CEleitorVotando::StartState` (func 7377, `IniciaCiclo` inlined)

1. pushes its `api::CApplicationContext` (error text shown if anything throws: *"Erro inesperado
   durante a votação" / "O voto do eleitor NÃO foi registrado" / "Ocorreu um erro enquanto o eleitor
   registrava suas escolhas."*, set by the constructor in func 3229);
2. sends `EleitorIniciouVotacao` (6) to the operator thread;
3. clears the screen (`IScreen::Clear(1)`, `Refresh()`), starts the **inactivity tick**
   (`cfg+172` s in voter-training mode, else `cfg+168` s);
4. resets the globals: typed number `g_votoDigitado` (@1833288), the list of confirmed votes
   `g_votosEleitor` (@1833300, entries `{TCargoID, CVoto{tipo, numero}}`), escolha index
   `g_numeroEscolha` (@1536340) = 1;
5. **filters the cargos by the voter's *abrangência*** (voto em trânsito, func 3721): 0 = voter of
   this município → all cargos; 1 = other município of the same UF → cargos of abrangência 1 or 2;
   2 = other UF → abrangência 2 only (inlined `CCargos::FiltraPorAbrangencia`, ccargos.cpp:249),
   then sorts them (comparator func 11559: eleição, then cargo order) and rewinds;
6. if audio is enabled (`CInformacaoEleitor::m_modoAudio` ≠ 2; func 509, naming by units u02/u07) the first sub-state is
   `CInstrucaoVotacaoAcessibilidade` and the operator line shows "Tela de instrução de
   acessibilidade"; otherwise the operator line shows `VOTANDO PARA: <cargo>` and the first cargo's
   sub-state is created (`ChamaEstadoProximoCargo`).

### 4.3 Choosing the sub-state of a cargo (`ChamaEstadoProximoCargo`, func 4459)

| condition | sub-state |
|---|---|
| no candidate for the cargo in this urna and not a consulta | `CConfirmaVotoSemCandidato` (screen 17, only CONFIRMA accepted → vote type 8) |
| eletivo and majoritário (`CCargo+4 == 0`), or consulta | `CPedeMajoritario` |
| eletivo and proporcional (`CCargo+4 == 1`) | `CPedeProporcional` |
| otherwise | log "Erro na identificação do tipo de cargo", throw 9310 "Tipo de cargo nao identificado" |

### 4.4 Advancing (`NeedChangeState`, func 7352)

If the sub-state wants to change and names a next state, CEleitorVotando swaps sub-states and stays.
If the next state is `nullptr` the cargo/escolha is finished: `g_numeroEscolha++`, or
`CCargos::Next()` and escolha = 1 when all vagas of the cargo are done (skipped once after the
accessibility instructions). Then one short beep (`IBeep` slot 2 with 1). When the list is
exhausted it calls **func 4454** (unit u22, analyzer name `CVotosCargos::ConfereCedula`, really a
CEleitorVotando method with the RDV code inlined): groups `g_votosEleitor` per eleição, checks the
ballot (`ConfereCedula`), inserts it into `CRdvVota` (`InsereCedula`/`RecebeCedula`), marks the
last-vote time in `EstadoGeralVota` (`MarcaUltimoVoto`) and sets the next state to
`CSincronismoEleitor`.

### 4.5 Conferência and confirmação

After the last digit (or BRANCO, or an invalid number) the `CPede*` state switches to a
`CConfereVotoEmCargo<TConfirma, TELA>` (one lazy singleton per instantiation, inlined into the
`CPede*` states; its constructor, func 1165, takes only the screen). The duration of the
conferência comes from the configuration: `ParametrosUrna.tempoConfirmacaoVoto` (`CConfiguracaoEleicao +176`, ms). If it is > 0 the
state creates a tick of that length, shows `TELA`, starts the tick, says "Confira o seu voto." and
refuses every key (`PlayKey(0)` = error beep + log "Tecla indevida pressionada"; then
`ProcessInputAudio` flushes the keypad, `IInputKbd` slot 4. Seen twice in the recorded
`logd.dat`). If it is ≤ 0 no tick is created and the screen is never shown: `StartStateAudio`
goes straight to `TConfirma` once the current audio ends. When the tick fires (after any audio still playing,
`ExecutarAposAudioAtual`, func 3851) it moves to the singleton `TConfirma`, whose
`StartStateAudio` shows the confirmation screen. On CONFIRMA (`EInputResult` 9) the vote is stored
with `CEleitorVotando::RegistraVoto(cargo, m_tipoVoto, g_votoDigitado)` (func 4471) and the state
returns `nullptr`; on CORRIGE (5) it returns to `CPedeProporcional`/`CPedeMajoritario`. Before
leaving, both paths call the slot-17 hook with the result (a no-op except in
`CConfirmaVotoNominal`/`CMajoritarioValido`, func 5925, which on CORRIGE logs "Eleitor corrigiu na
tela de confirmação de candidato" and flushes the keypad through `IPoliticaExecucaoEleitor`, §10).

| conferência template (slot 16 func) | confirm state | flags | confirm screen | RDV `TipoVoto` stored |
|---|---|---|---|---|
| `<CConfirmaVotoNominal, ConferenciaVotoNominal(2)>` (11715) | CConfirmaVotoNominal | 6 | 1 Completa | 2 nominal |
| `<CConfirmaVotoLegenda, ConferenciaVotoLegenda(10)>` (11717) | CConfirmaVotoLegenda | 6 | 9 Voto Legenda Incompleto | 1 legenda |
| `<CCandidatoInexistente, 12>` (11716) | CCandidatoInexistente | 6 | 11 Candidato Inexistente | **1 legenda** (party prefix exists) |
| `<CCandidatoInapto, 14>` (11739) | CCandidatoInapto | 6 | 13 Candidato Inapto | 4 nulo |
| `<CProporcionalBranco, 4>` (11707) | CProporcionalBranco | 6 | 3 Voto Branco | 3 branco |
| `<CProporcionalNulo, 7>` (11738) | CProporcionalNulo | 6 | 6 Voto Nulo | 4 nulo |
| `<CMajoritarioValido, 2>` (11670) | CMajoritarioValido | 2 | 1 Completa | 2 nominal |
| `<CMajoritarioRepetido, 16>` (11671) | CMajoritarioRepetido | 6 | 15 Candidato Repetido | 7 nuloPorRepeticao |
| `<CMajoritarioNulo, 7>` (11672) | CMajoritarioNulo | 6 | 6 Voto Nulo | 4 nulo |
| `<CMajoritarioBranco, 4>` (11673) | CMajoritarioBranco | 6 | 3 Voto Branco | 3 branco |

(`CMajoritarioRepetido`: the same candidate chosen twice for a multi-seat cargo, detected by
`CPedeMajoritario` scanning `g_votosEleitor`.)

### 4.6 Vote types produced by this unit

`comum::md::CVoto::ETipo` uses the RDV numbering (`ModuloRegistroDigitalVoto.TipoVoto`):
1 legenda, 2 nominal, 3 branco, 4 nulo, 5 brancoAposSuspensao, 6 nuloAposSuspensao,
7 nuloPorRepeticao, 8 nuloCargoSemCandidato, 9 nuloAposSuspensaoCargoSemCandidato.
The **BU** uses another enum (`ModuloBoletimUrna.TipoVoto`: nominal 1, branco 2, nulo 3, legenda 4,
cargoSemCandidato 5); the conversion happens in the BU generator (units u08/u22).
Each stored vote is logged with the cargo name only, e.g. `1|1|Voto confirmado para [Vereador]`
(`logd.dat`, Latin-1); the log never contains the number voted.

### 4.7 Operator messages, inactivity and suspension

Messages posted to `CThreadOperador` (priority 1): 1 fim, 2 não votou, 3/4 eleitor demorando
(without/with confirmed votes), 5 voltou a digitar, 6 iniciou votação, 9 new text in
`CThreadOperador+108` ("VOTANDO PARA: …", at most 40 chars; func 4464 falls back to the
abbreviated cargo name when the full one is too long).
When the inactivity tick fires, CEleitorVotando posts 3 or 4 and sets `m_eleitorDemorando`; the
next key restarts the timer and posts 5 ("Eleitor voltou a digitar enquanto estava sendo
suspenso"). Messages received: 2 = suspended by the mesário, 3 = automatic suspension (voter
training), 4 = continue (restart timer); others go to the sub-state.

`suspenderEleitor` (func 4442) applies the election configuration (`CInformacaoEleicao` at
`CConfiguracaoEleicao+88`):

| situation | config field | 0 | 1 | 2 | other |
|---|---|---|---|---|---|
| nothing confirmed yet (cargo 0, escolha 1) | `+88` forma suspensão *sem* voto | discard (DescartaVotos) | remaining escolhas = brancoAposSuspensao | = nuloAposSuspensao | throw 9312 |
| some vote confirmed | `+92` forma suspensão *com* voto | = brancoAposSuspensao | = nuloAposSuspensao | **discard all, including the confirmed votes** | throw 9313 |

When filling, cargos without candidates (and not consultas) get type 9. Then it logs "Eleitor votou
parcialmente e em seguida foi suspenso" and records the ballot (func 4454) exactly like a normal
end. `DescartaVotos` (func 4449) instead shows "FIM / NÃO VOTOU", posts 2 to the operator, plays the
suspension beep, stops audio, logs "Eleitor foi suspenso e não confirmou nenhum voto" and goes back to
`CAguardaMensagem` without touching the RDV.

## 5. Screens: `ETelaVotacao` and `CTelasCargo`

`NomeTela` (func 6718) gives the 18 screens: 0 Tela Inicial, 1 Tela Completa, 2 Conferência de Voto
Nominal, 3 Voto Branco, 4 Conferência de Voto Branco, 5 Voto Proporcional Nulo Incompleto, 6 Voto
Nulo, 7 Conferência de Voto Nulo, 8 Partido, 9 Voto Legenda Incompleto, 10 Conferência de Voto
Legenda, 11 Candidato Inexistente, 12 Conferência de Candidato Inexistente, 13 Candidato Inapto,
14 Conferência de Candidato Inapto, 15 Candidato Repetido, 16 Conferência de Candidato Repetido,
17 Cargo sem Candidato. `CTelasVota` (u07) builds, at start-up, one `CTelasCargo` per cargo
(func 2386 validates: at least one screen, no null screen) and `CTelasVota::GetTelaCargo`
(func 4135, with `CTelasCargo::GetTela` inlined) returns the `shared_ptr` of a screen or throws
9358/9330. Other `CTelasVota` members used here: +124 `telaInstrucoesAcessibilidade`, +156
`telaFim` "FIM/VOTOU", +164 `telaFim` "FIM/NÃO VOTOU" (Latin-1 "N\xC3O VOTOU").

## 6. Accessibility (voter audio)

When the mesário enables audio for the voter (voter-thread messages 8/9 set `CInformacaoEleitor::m_modoAudio` to 0 "conforme cadastro" / 1
"habilitado", 10 back to 2 = desabilitado), the session starts with `CInstrucaoVotacaoAcessibilidade`: volume reset to 7/10
and speech rate to 2, the text *"A urna está pronta para receber o seu voto. Use o teclado numérico
{abaixo | à direita} da urna para digitar o seu voto. Aperte a tecla 4 para fala mais lenta. Aperte a
tecla 6 para fala mais rápida. Aperte a tecla 3 para aumentar o volume. Aperte a tecla 9 para
diminuir o volume. Aperte confirma para iniciar o seu voto"* ("abaixo" when `IUrna` model > 2019).
Keys: 3/9 volume ±1 (0..10, "Volume máximo/mínimo" at the limits), 4/6 speech slower/faster
("Fala em velocidade mínima/máxima" at the limits), CONFIRMA starts the vote, every other key and
CORRIGE are refused. A refused key is **not spoken**: `PlayKey(0)` (func 1455) gives the 554 Hz error beep
(`IBeep::BeepErro`, `CWasmBeep` func 8494) and writes the log line "Tecla indevida pressionada". With the voice
on, the urna would also play the click file `:/resource/sounds/tecE.wav`, but `CWasmWebSound`'s play-file slot
(func 9580) plays nothing. Confirmed with `node tools/run/headless.mjs --audio` (state
`CInstrucaoVotacaoAcessibilidade` then `CPedeProporcional` after `C`; `logd.dat` shows the two
"Tecla indevida pressionada" lines for keys 5 and CORRIGE). All spoken texts are Latin-1 in the binary.

## 7. Start-up and restart routing (real urna only)

`CAjusteInicial::StartState` (func 7160) = `AjustaDataHora` + `ValidaTemposDesligamento` +
`LimpaMidiaResultado` + (demo mode) register `CControladorRegistraMesariosVota` + `InicioVota`.

* **AjustaDataHora** — only in demonstration mode or training phase (`CEstadoGeral+48 == '3'`) and
  before voting started: moves the system clock (via `IAjusteDataHora`, and records the delta in
  `EstadoGeralUrna.ajusteDataHora`, then rewrites `eg.bin`) to a configured date/time
  (`cfg+556/+564` in voter-training mode, else `cfg+544/+552` + 30 s). `CIniciodeCiclo`
  (func 7306) runs when the operator starts the voting period: if `estadoVota` ≤ 55 it moves the
  clock to `cfg+556/+564` + 10 s, but only in demonstration mode or on a training-phase urna that
  is **not** in voter training (func 1823), then sets `estadoVota = votar` (56) and saves; in all
  cases it goes to `CInicioVotacao`.
* **ValidaTemposDesligamento** — `cfg+180` (battery time limit) and `cfg+184` (warning time) must be
  > 0 and limit ≥ warning (errors 9405-9407).
* **LimpaMidiaResultado** — training phase, not voter training: mounts the MR (`/dsk/mr/`) and deletes
  every entry except `infomidia*` and `turno2*`, overwriting regular files with zeros first
  (func 2760, never through a symlink).
* **InicioVota** — the recovery table (C++ enum = ASN.1 `EstadoVota` + 49):

| estadoVota | ASN.1 name | next state |
|---|---|---|
| 49-51 | inicial, gerabasedinamica, aguardahorazeresima | CVerificaEleicaoPassou |
| 52 | gerarze | CInicioZeresima |
| 53 | zeresimagerada | CImprimindoZeresima |
| 54-56 | zeresimaimpressa, registromesarioinicial, votar | testeteclado::CRetomada (keypad test, then resume) |
| 57 | fimaquisicaovotos | CFinalizaAquisicao |
| 58 | registromesariofinal | CReinicioComparecimentoMesario |
| 59 | gerarbu | CGeraBU |
| 60 | gerarrelatorios | CGeraRelatorios |
| 61 | imprimirbu | CInicioBU |
| 62 | gravarresultados | CGravaResultado |
| 63 | copiaresultadosmr | CCopiaResultadoParaMR |
| 64 | encerrada → estadoEncerramento: inicial/imprimirobrigatoriabu → CImprimirBUOutrasObrigatorias, retirarmr → CRetirarMR, fimdostrabalhos → CVerificaQtdBUsAdicionais, else throw 9303 |
| other (incl. 65 exibealertadesligamento) | | throw 9304 "Estado nao conhecido" |

`CDefineRotaPosReinicio::NeedChangeState` (func 11858): estadoVota 54/55 → `CReinicioComparecimentoMesario`
when func 2520 holds (demonstration mode, unless the urna is a training-phase urna in voter
training; a non-demo urna never goes there) or `CInicioVotacao` otherwise; 56 → `CInicioVotacao`;
else log "Erro estado do aplicativo não conhecido" and throw 9309.

`CEstadoComDesligamentoAutomatico::ProcessTick` (func 12061): every second, if `IPower` reports the
internal battery (`status & 6 == 2`), arms a deadline `now + (limit − warning)`; when it passes, it
stores `deadline + warning` in `g_dataHoraDesligamento` (@1833312) and switches to
`CExibeAlertaDesligamento` (singleton func 5979). Back on external power the deadline is cleared.

## 8. Boletim de Urna (BU): what this unit contributes

This unit does not build the BU. Its contributions to "how a BU works":

1. **Content**: every vote that ends up in the BU/RDV is created here (`RegistraVoto`,
   `suspenderEleitor`) with the types of §4.6 and appended to `g_votosEleitor`; func 4454 (u22)
   turns the voter's list into one RDV *cédula* after `ConfereCedula` checks (vote count per cargo =
   number of escolhas, nominal-vote length = cargo digits, repetition only for multi-seat cargos).
   A discarded session (DescartaVotos) produces no cédula and no comparecimento.
2. **Encerramento entry**: the operator's "encerrar" arrives at `CAguardaMensagem` as message 7
   (`estadoVota = gerarbu (59)`, save, log "Inicio do Encerramento", next `CGeraBU`); the BU chain is
   CGeraBU (59) → CGeraRelatorios (60) → CInicioBU/printing (61) → CGravaResultado (62) →
   CCopiaResultadoParaMR (63) → encerrada (64: obligatory copies, remove MR, extra copies).
3. **Restart safety**: after a reboot during encerramento `CAjusteInicial::InicioVota` re-enters the
   exact BU step from `vota.bin` (table in §7).
4. **Screens of the BU states** built by the singleton accessors in this unit: `CGeraBU` (func 6129)
   "Votação encerrada" (`telaVotacaoEncerrada`) and "Preparando dados para encerramento"
   (`telaPreparandoDadosEncerramento`), `CGeraRelatorios` (6084) the latter, `CCopiaResultadoParaMR`
   (6174) "Gravando o resultado na mídia" (`telaCopiaResultadoParaMR`), each with
   "Por favor, aguarde..." (func 4152). On a training urna the MR is wiped at start-up except
   `infomidia*`/`turno2*` (§7).

## 9. Data read and written

| data | access |
|---|---|
| `CConfiguracaoEleicao` +88/+92 | forma de suspensão sem/com voto |
| +168/+172 | voter inactivity timeout (s): normal / voter-training |
| +176 | `ParametrosUrna.tempoConfirmacaoVoto`: duration of the conferência screen (ms; 1000 in the published scenarios per unit u37); ≤ 0 = no conferência screen (func 1165) |
| +180/+184 | battery time limit / power-off warning (s) |
| +544/+552, +556/+564 | configured date/times used to set the clock in demo/training |
| `CEstadoGeral` (eg.bin) +48 fase, +52 ajusteDataHora | read; ajuste written by the clock adjustments |
| `CEstadoGeralVota` (vota.bin) estadoVota, estadoEncerramento, treinamentoEleitor (+72) | read; estadoVota written by CIniciodeCiclo (56) |
| `CCargos`, `CCandidaturas`, `CEleitores` (current voter) | read (cargo list, candidates, trânsito) |
| `logd.dat` | via `CLogVota` (severity 1 info, 3 error) |
| `/dsk/mr/*` | deleted by LimpaMidiaResultado (training urna) |
| globals `g_votoDigitado`, `g_votosEleitor`, `g_numeroEscolha` | session scratch; also read by the web adapter (`votaGetStateJson` reads the first 2 typed digits to report `legendaValida`) |

## 10. Web build specifics

* Initial state `CAguardaMensagem` instead of `CAjusteInicial`: no clock adjustment, no MR wipe,
  no restart routing in the simulator.
* `IPoliticaExecucaoEleitor`: the web registers `CPoliticaExecucaoEleitorWeb` (vota_web_wasm.cpp:361,
  func 10835, installed by `main`, func 10307), which flushes the keypad **once**; the urna version
  (func 13564) flushes 3-6 times with random 50-149 ms pauses (IRng). Both run only after a CORRIGE
  on the candidate confirmation screens (`CConfirmaVotoNominal` / `CMajoritarioValido` slot 17,
  func 5925), not after every CORRIGE.
* Mock hardware reached from here: `CWasmScreen` (Clear/Refresh; slot 14 is a no-op),
  `CWasmInputKbd` (slot 7 no-op), `CWasmBeep` (tone sequences), `CWasmWebSound` (Web Audio),
  `ITextToSpeech` = RHVoice.
* The adapter reads `CEleitorVotando+12` (the sub-state) by comparing the object's vtable with
  1533152 and prints the sub-state's RTTI name as `substate`; it reports "vota:done" when the state
  name becomes `CAguardaMensagem` again (also after a discarded session).

## 11. wasm / Emscripten observations

* **Slot-name recovery through `__PRETTY_FUNCTION__`**: `GetTelaCargoAtual(const std::string&)` receives the
  caller's pretty name, which names funcs 7428, 7441, 11754, 11755 exactly.
* **merge-similar-functions**: 2302 (5 log thunks differ only by the format string), 3921
  (GetTelaCargoAtual with srcloc/error code as parameters), 1961/2901/6051/764 (singletons).
* **Inlining hides the virtual**: funcs 7160, 7306, 7377, 11793, 12061, 4407, 4135 were named by the
  analyzer after an inlined callee's srcloc; the table below gives the real virtual/outer function.
* `std::format` arguments are packed as type codes (6 = unsigned, 13 = string_view, 3 = int,
  15 = handle); a `uebyte` cargo id is formatted as a number ("Cargo 13 "). `NomeTela` casts the
  screen to `int` (code 3), but the `EEstadoVota`/`EEstadoEncerramento` values in the 9303/9304/9309
  messages are passed as enums (code 15, a user `std::formatter` whose ICF body, func 536, prints
  the unsigned value, e.g. "Estado nao conhecido = 65").
* `emscripten_sleep` behind the global flag @1584624 (initial value 1) is reachable from
  `LimpaMidiaResultado`/`HabilitaMR` and `CPoliticaExecucaoEleitor::LimpaBufferInput`; both are dead
  in the web flow (see suspicious list).

## 12. Weird or risky code

1. **`emscripten_sleep` on the MR path** (func 7160 `LimpaMidiaResultado`, via func 2863 and a
   direct call): `sleep(500)`/`sleep(100)` guarded by the flag @1584624 = 1. Without Asyncify the glue
   aborts (`upstream/site/wasm/vota_web_wasm.js`: `_emscripten_sleep=()=>{abort("Please compile your
   program with async support …")}`). Unreachable in the simulator because the web never enters `CAjusteInicial`; on the urna
   this is `sleep_for`. Info.
2. **`CPoliticaExecucaoEleitor::LimpaBufferInput`** (func 13564) sleeps 3-6 × 50-149 ms through the
   same flag: it would abort, but the web registers its own policy first. The web policy flushes
   once, so the simulator's post-CORRIGE key handling is not the urna's (keys typed during the
   ~0.15-0.9 s flush window are kept in the web). The remainders are signed (`i32.rem_s`): a negative
   random number gives 0-2 passes. Low.
3. **Empty cargo list → endless restart** (func 7377/7352): if the trânsito filter leaves no cargo,
   `StartState` sets `m_estadoCargo = nullptr` while `m_proximoEstado` stays `this`; the next event
   makes `NeedChangeState` return true and the thread re-runs `StartState` (new context push,
   message 6 to the operator, screen clear) forever; no vote and no comparecimento are recorded. Only
   a suspension by the mesário gets out. Low (needs a voter habilitated with no eligible cargo).
   The restart is event-driven, not a busy loop: `CThreadEleitor::Processar` (func 4349)
   calls `FinishState`, then `StartState` on `GetNextState()` (= `this`) after each key, tick or
   message. Established by reading the code (7377/7352/4349); not reproduced at runtime (no
   scenario has such a voter).
4. **Discarding confirmed votes is logged and shown as "não votou"** (func 4442 → 4449): with
   `formaSuspensaoComVoto == 2` the votes the voter already confirmed are dropped, the log says
   "Eleitor foi suspenso e não confirmou nenhum voto", the screen says "NÃO VOTOU" and the
   operator receives message 2. The screen and message 2 can be argued to be intended (the ballot is
   cancelled, so the voter is treated as not having voted); the log text is plainly wrong for that
   case (the preceding log line "Eleitor foi suspenso pelo mesário" is the only hint). The reverse
   also happens: with `formaSuspensaoSemVoto` 1/2 a voter who confirmed **nothing** gets all
   escolhas filled with branco/nulo and the log says "Eleitor votou parcialmente e em seguida foi
   suspenso". Config-driven. Low (audit-trail wording). Established by reading func 4442/4449; the
   simulator API (`votaPressKey` etc.) has no way to suspend a voter, so it was not run.
5. **`g_votosEleitor` keeps the last voter's choices in memory** until the next habilitação
   (not cleared by `GravaVotos`/`DescartaVotos`, only at the next `StartState`), and
   `g_votoDigitado` keeps the last typed number of the last cargo. Harmless in the simulator; on a
   real urna it is RAM-only state of the ballot secrecy perimeter. Info.
6. **`CCandidatoInexistente` stores a *legenda* vote** (RDV type 1) with the full typed number, and
   `CCandidatoInapto` stores a *nulo* with the number: correct per the electoral rules, but worth
   knowing when reading RDV/BU counts. Info. The types are constants in funcs 11716/11739 (tipo 1 and
   4). The path was run: `node tools/run/headless.mjs --keys "91999  C  B  C  "` goes
   `CPedeNominal → CConfereVotoEmCargo<CCandidatoInexistente,12> → CCandidatoInexistente` (adapter
   `voteMode: "legenda"`) and logs "Voto confirmado para [Vereador]". The stored type itself is not
   visible from outside, because the web build does not persist the RDV.
7. Cargo sub-states are singletons shared across voters; their per-voter state is reset only in
   `StartState*` (e.g. volume/speed in the accessibility state), so any field not reset there leaks
   between voters. Nothing concrete found. Info.

## 13. Open questions

* Exact names of the `CConfiguracaoEleicao` date/time fields at +544/+552 and +556/+564 (zerésima
  vs. start of voting?).
* Official names of `IScreen` slot 14 and `IInputKbd` slot 7 (no-ops in the web mocks), of the
  `IBeep`/`ISound` slots and of `api::EInputResult` values 5/9/13 (units u06/u08 use Corrige/Confirma/Tecla).
* Whether the `CLogVota::Loga*` helpers (2302 family, 4529) are CLogVota members or free helpers in
  celeitorvotando.cpp; their first argument is the CLogVota instance.
* `CDefineRotaPosReinicio +12` (set to 0 before routing): meaning unknown.
* Whether any real configuration sets `tempoConfirmacaoVoto` (`CConfiguracaoEleicao +176`) ≤ 0,
  which removes the conferência screen.
* Cross-unit: unit u07's `ctelasvota.h` does not declare the `+156`/`+164` "FIM" screens that
  `cfimvotoeleitor.cpp`/`celeitorvotando.cpp` use (`m_telaFimVotou`/`m_telaFimNaoVotou`), and unit
  u08's `cvotacaostateaudio.h` forward-declares `CFormInterativoTelaVota` as a class, although it is
  the `shared_ptr` alias of `ctelasvota.h`/`ctelascargo.h`.

## 14. Mapping table (all 83 functions of u06)

"ran" = observed executing in the recorded votes. "reconstructed in" is relative to
`src/uenux2/src/app/vota/eleitor/`; "(comment)" means the function is explained there as a comment
(merged bodies, library instantiations, accessors of classes owned by other units).

| func | size | ran | analyzer name | reconstructed symbol | original file | reconstructed in (src/…/vota/eleitor/) | conf. |
|---:|---:|:-:|---|---|---|---|---|
| 233 | 14 | ✓ | `api_f233` | `comum::IEventosLog::Loga` | uenux2/src/app/comum/log/ieventoslog.cpp | celeitorvotando.cpp (call sites) | low |
| 1785 | 70 | ✓ | `vota_f1785` | `vota::CVotacaoStateAudio::CVotacaoStateAudio` | …/eleitor/cvotacaostateaudio.cpp | celeitorvotando.cpp (appendix comment) | high |
| 1961 | 79 | ✓ | `vota_f1961` | `vota::merged_GetInst_CConfirmaProporcional` | …/eleitor/cconferevotoemcargo.cpp | cconferevotoemcargo.cpp (comment) | medium |
| 2302 | 483 | ✓ | `vota_f2302` | `vota::CLogVota::merged_LogaVotoCargo` | uenux2/src/app/vota/log/clogvota.cpp | celeitorvotando.cpp (comment) | medium |
| 2386 | 941 | ✓ | `vota::CTelasCargo::CTelasCargo` | `vota::CTelasCargo::CTelasCargo` | …/eleitor/comum/ctelascargo.cpp | comum/ctelascargo.cpp | high |
| 2466 | 59 |  | `vota::CInstrucaoVotacaoAcessibilidade::vf0` | `vota::CInstrucaoVotacaoAcessibilidade::~CInstrucaoVotacaoAcessibilidade` | …/eleitor/cinstrucaovotacaoacessibilidade.cpp | cinstrucaovotacaoacessibilidade.cpp | high |
| 2483 | 13 |  | `vota::CEleitorVotando::vf0` | `vota::CEleitorVotando::~CEleitorVotando` | …/eleitor/celeitorvotando.cpp | celeitorvotando.cpp | high |
| 2760 | 94 | ✓ | `vota_f2760` | `api::CSystem::SecureRemove` | uenux2/src/api/util/csystem.cpp | cajusteinicial.cpp (appendix) | low |
| 2901 | 81 | ✓ | `vota_f2901` | `vota::merged_GetInst_CConfirmaMajoritario` | …/eleitor/cconferevotoemcargo.cpp | cconferevotoemcargo.cpp (comment) | medium |
| 3251 | 466 | ✓ | `vota_f3251` | `std::__tree<std::__value_type<vota::ETelaVotacao,CFormInterativoTelaVota>>::__find_equal` | — (inline/merged helper) | library (comment in ctelascargo.cpp) | medium |
| 3272 | 17 |  | `vota_f3272` | `vota::CLogVota::LogaVotoCargoSemCandidatoSuspensao` | uenux2/src/app/vota/log/clogvota.cpp | celeitorvotando.cpp (comment) | low |
| 3783 | 378 | ✓ | `comum_f3783` | `std::vector<comum::CCargos::SItem>::__assign_with_size` | — (inline/merged helper) | library (comment in celeitorvotando.cpp) | medium |
| 3849 | 20 |  | `vota_f3849` | `vota::CPedeProporcional::GetInst` | …/eleitor/votaproporcional/cpedeproporcional.cpp | celeitorvotando.cpp (comment) | medium |
| 3851 | 781 | ✓ | `vota::IConfereVotoEmCargo::ExecutarAposAudioAtual` | `vota::IConfereVotoEmCargo::ExecutarAposAudioAtual` | …/eleitor/cconferevotoemcargo.cpp | cconferevotoemcargo.cpp | high |
| 4135 | 1190 | ✓ | `vota::CTelasCargo::GetTela` | `vota::CTelasVota::GetTelaCargo` | …/eleitor/comum/ctelasvota.cpp | comum/ctelascargo.cpp | high |
| 4152 | 297 |  | `vota_f4152` | `vota::CriaTelaStatusAguarde` | — (inline/merged helper) | cajusteinicial.cpp (comment) | low |
| 4192 | 704 |  | `vota_f4192` | `vota::CEleitorVotando::InsereVoto` | …/eleitor/celeitorvotando.cpp | celeitorvotando.cpp | low |
| 4407 | 408 |  | `vota::CInstrucaoVotacaoAcessibilidade::GetKeyboardPosition` | `vota::CInstrucaoVotacaoAcessibilidade::GetMensagemAudio` | …/eleitor/cinstrucaovotacaoacessibilidade.cpp | cinstrucaovotacaoacessibilidade.cpp | medium |
| 4420 | 141 | ✓ | `vota_f4420` | `vota::CInstrucaoVotacaoAcessibilidade::GetInst` | …/eleitor/cinstrucaovotacaoacessibilidade.cpp | cinstrucaovotacaoacessibilidade.cpp | medium |
| 4442 | 1183 |  | `vota::CEleitorVotando::suspenderEleitor` | `vota::CEleitorVotando::suspenderEleitor` | …/eleitor/celeitorvotando.cpp | celeitorvotando.cpp | high |
| 4449 | 352 |  | `vota::CEleitorVotando::DescartaVotos` | `vota::CEleitorVotando::DescartaVotos` | …/eleitor/celeitorvotando.cpp | celeitorvotando.cpp | high |
| 4459 | 604 | ✓ | `vota::CEleitorVotando::ChamaEstadoProximoCargo` | `vota::CEleitorVotando::ChamaEstadoProximoCargo` | …/eleitor/celeitorvotando.cpp | celeitorvotando.cpp | high |
| 4464 | 1272 | ✓ | `vota_f4464` | `vota::InformaCargoAoOperador` | …/eleitor/celeitorvotando.cpp | celeitorvotando.cpp | low |
| 4471 | 140 | ✓ | `vota_f4471` | `vota::CEleitorVotando::RegistraVoto` | …/eleitor/celeitorvotando.cpp | celeitorvotando.cpp | low |
| 4477 | 243 |  | `vota::CConfirmaVotoSemCandidato::EmiteEcoComInputField` | `vota::CConfirmaVotoSemCandidato::EmiteEcoComInputField` | …/eleitor/cconfirmavotosemcandidato.cpp | cconfirmavotosemcandidato.cpp | high |
| 4483 | 20 |  | `vota::CConfirmaVotoSemCandidato::GetTelaCargoAtual` | `vota::CConfirmaVotoSemCandidato::GetTelaCargoAtual` | …/eleitor/cconfirmavotosemcandidato.cpp | cconfirmavotosemcandidato.cpp | high |
| 4529 | 150 |  | `vota_f4529` | `vota::CLogVota::LogaAudioDesativadoFimVotacao` | uenux2/src/app/vota/log/clogvota.cpp | (doc only) | low |
| 4537 | 17 | ✓ | `vota_f4537` | `vota::CLogVota::LogaVotoConfirmado` | uenux2/src/app/vota/log/clogvota.cpp | celeitorvotando.cpp (comment) | low |
| 4541 | 17 |  | `vota_f4541` | `vota::CLogVota::LogaVotoNuloSuspensao` | uenux2/src/app/vota/log/clogvota.cpp | celeitorvotando.cpp (comment) | low |
| 4542 | 17 |  | `vota_f4542` | `vota::CLogVota::LogaVotoCargoSemCandidato` | uenux2/src/app/vota/log/clogvota.cpp | celeitorvotando.cpp (comment) | low |
| 4546 | 17 |  | `vota_f4546` | `vota::CLogVota::LogaVotoBrancoSuspensao` | uenux2/src/app/vota/log/clogvota.cpp | celeitorvotando.cpp (comment) | low |
| 4640 | 15 |  | `vota_f4640` | `api::CThread::AdicionaTick` | — (inline/merged helper) | cestadocomdesligamentoautomatico.cpp (comment) | low |
| 5474 | 12 |  | `vota_f5474` | `api::CDateTime::operator+=` | — (inline/merged helper) | cestadocomdesligamentoautomatico.cpp (comment) | low |
| 5921 | 20 |  | `vota_f5921` | `vota::CPedeMajoritario::GetInst` | …/eleitor/votamajoritario/cpedemajoritario.cpp | celeitorvotando.cpp (comment) | medium |
| 5928 | 20 | ✓ | `vota::CConfirmaVotoEmCargo::GetTelaCargoAtual` | `vota::CConfirmaVotoEmCargo::GetTelaCargoAtual` | …/eleitor/cconfirmavotoemcargo.cpp | cconfirmavotoemcargo.cpp | high |
| 5929 | 35 | ✓ | `vota_f5929` | `vota::CConfirmaVotoEmCargo::CConfirmaVotoEmCargo` | …/eleitor/cconfirmavotoemcargo.cpp | cconfirmavotoemcargo.cpp | high |
| 5973 | 22 |  | `vota_f5973` | `vota::CImprimindoZeresima::GetInst` | …/eleitor/iniciovotacao/cimprimindozeresima.cpp | cajusteinicial.cpp (comment) | medium |
| 5979 | 202 |  | `vota_f5979` | `vota::CExibeAlertaDesligamento::GetInst` | …/eleitor/iniciovotacao/cexibealertadesligamento.cpp | cestadocomdesligamentoautomatico.cpp (comment) | medium |
| 6051 | 77 |  | `vota_f6051` | `vota::merged_GetInst_CVotacaoStateAudio28` | — (inline/merged helper) | celeitorvotando.cpp (comment) | medium |
| 6084 | 105 |  | `vota_f6084` | `vota::CGeraRelatorios::GetInst` | …/eleitor/fimvotacao/cgerarelatorios.cpp | cajusteinicial.cpp (comment) | medium |
| 6129 | 466 |  | `vota_f6129` | `vota::CGeraBU::GetInst` | …/eleitor/fimvotacao/cgerabu.cpp | cajusteinicial.cpp (comment) | medium |
| 6174 | 470 |  | `vota_f6174` | `vota::CCopiaResultadoParaMR::GetInst` | …/eleitor/fimvotacao/ccopiaresultadoparamr.cpp | cajusteinicial.cpp (comment) | medium |
| 6583 | 398 |  | `vota_f6583` | `vota::CriaTelaPreparandoDadosEncerramento` | — (inline/merged helper) | cajusteinicial.cpp (comment) | low |
| 6718 | 2086 |  | `vota::NomeTela` | `vota::NomeTela` | …/eleitor/comum/ctelascargo.cpp | comum/ctelascargo.cpp | high |
| 7160 | 5220 |  | `vota::CAjusteInicial::ValidaTemposDesligamento` | `vota::CAjusteInicial::StartState` | …/eleitor/cajusteinicial.cpp | cajusteinicial.cpp | high |
| 7198 | 207 | ✓ | `vota::CFimVotoEleitor::StartState` | `vota::CFimVotoEleitor::StartState` | …/eleitor/cfimvotoeleitor.cpp | cfimvotoeleitor.cpp | high |
| 7242 | 123 |  | `vota::CInstrucaoVotacaoAcessibilidade::StartStateAudio` | `vota::CInstrucaoVotacaoAcessibilidade::StartStateAudio` | …/eleitor/cinstrucaovotacaoacessibilidade.cpp | cinstrucaovotacaoacessibilidade.cpp | high |
| 7248 | 1315 |  | `vota::CInstrucaoVotacaoAcessibilidade::ProcessInputAudio` | `vota::CInstrucaoVotacaoAcessibilidade::ProcessInputAudio` | …/eleitor/cinstrucaovotacaoacessibilidade.cpp | cinstrucaovotacaoacessibilidade.cpp | high |
| 7255 | 13 |  | `vota::CInstrucaoVotacaoAcessibilidade::vf1` | `vota::CInstrucaoVotacaoAcessibilidade::~CInstrucaoVotacaoAcessibilidade(deleting)` | …/eleitor/cinstrucaovotacaoacessibilidade.cpp | cinstrucaovotacaoacessibilidade.cpp | high |
| 7287 | 38 |  | `vota_f7287` | `vota::CInstrucaoVotacaoAcessibilidade::GetInst::__dtor_s_inst` | …/eleitor/cinstrucaovotacaoacessibilidade.cpp | cinstrucaovotacaoacessibilidade.cpp (comment) | medium |
| 7306 | 251 |  | `vota::CIniciodeCiclo::AjustaDataHora` | `vota::CIniciodeCiclo::StartState` | …/eleitor/ciniciodeciclo.cpp | ciniciodeciclo.cpp | high |
| 7337 | 96 |  | `vota::CEleitorVotando::vf6` | `vota::CEleitorVotando::ProcessMessage` | …/eleitor/celeitorvotando.cpp | celeitorvotando.cpp | medium |
| 7345 | 287 | ✓ | `vota::CEleitorVotando::vf7` | `vota::CEleitorVotando::ProcessInput` | …/eleitor/celeitorvotando.cpp | celeitorvotando.cpp | high |
| 7349 | 162 | ✓ | `vota::CEleitorVotando::vf8` | `vota::CEleitorVotando::ProcessTick` | …/eleitor/celeitorvotando.cpp | celeitorvotando.cpp | high |
| 7352 | 305 | ✓ | `vota::CEleitorVotando::NeedChangeState` | `vota::CEleitorVotando::NeedChangeState` | …/eleitor/celeitorvotando.cpp | celeitorvotando.cpp | high |
| 7370 | 61 | ✓ | `vota::CEleitorVotando::vf5` | `vota::CEleitorVotando::FinishState` | …/eleitor/celeitorvotando.cpp | celeitorvotando.cpp | high |
| 7377 | 2313 | ✓ | `vota::CEleitorVotando::IniciaCiclo` | `vota::CEleitorVotando::StartState` | …/eleitor/celeitorvotando.cpp | celeitorvotando.cpp | high |
| 7385 | 13 |  | `vota::CEleitorVotando::vf1` | `vota::CEleitorVotando::~CEleitorVotando(deleting)` | …/eleitor/celeitorvotando.cpp | celeitorvotando.cpp | high |
| 7416 | 38 |  | `vota_f7416` | `vota::CEleitorVotando::GetInst::__dtor_s_inst` | …/eleitor/celeitorvotando.cpp | celeitorvotando.cpp (comment) | medium |
| 7425 | 51 |  | `vota::CConfirmaVotoSemCandidato::vf15` | `vota::CConfirmaVotoSemCandidato::GetMensagemAudio` | …/eleitor/cconfirmavotosemcandidato.cpp | cconfirmavotosemcandidato.cpp | medium |
| 7428 | 278 |  | `vota::CConfirmaVotoSemCandidato::vf9` | `vota::CConfirmaVotoSemCandidato::ProcessInputAudio` | …/eleitor/cconfirmavotosemcandidato.cpp | cconfirmavotosemcandidato.cpp | high |
| 7441 | 267 |  | `vota::CConfirmaVotoSemCandidato::vf10` | `vota::CConfirmaVotoSemCandidato::StartStateAudio` | …/eleitor/cconfirmavotosemcandidato.cpp | cconfirmavotosemcandidato.cpp | high |
| 11670 | 28 | ✓ | `vota::CConfereVotoEmCargo<vota::CMajoritarioValido, (vota::ETelaVotacao)2>::vf16` | `vota::CConfereVotoEmCargo<vota::CMajoritarioValido, (vota::ETelaVotacao)2>::GetProximoEstado` | …/eleitor/cconferevotoemcargo.h | cconferevotoemcargo.h | medium |
| 11671 | 28 |  | `vota::CConfereVotoEmCargo<vota::CMajoritarioRepetido, (vota::ETelaVotacao)16>::vf16` | `vota::CConfereVotoEmCargo<vota::CMajoritarioRepetido, (vota::ETelaVotacao)16>::GetProximoEstado` | …/eleitor/cconferevotoemcargo.h | cconferevotoemcargo.h | medium |
| 11672 | 28 | ✓ | `vota::CConfereVotoEmCargo<vota::CMajoritarioNulo, (vota::ETelaVotacao)7>::vf16` | `vota::CConfereVotoEmCargo<vota::CMajoritarioNulo, (vota::ETelaVotacao)7>::GetProximoEstado` | …/eleitor/cconferevotoemcargo.h | cconferevotoemcargo.h | medium |
| 11673 | 28 |  | `vota::CConfereVotoEmCargo<vota::CMajoritarioBranco, (vota::ETelaVotacao)4>::vf16` | `vota::CConfereVotoEmCargo<vota::CMajoritarioBranco, (vota::ETelaVotacao)4>::GetProximoEstado` | …/eleitor/cconferevotoemcargo.h | cconferevotoemcargo.h | medium |
| 11699 | 5 |  | `vota::CConfirmaMajoritario::vf16` | `vota::CConfirmaMajoritario::GetEstadoCorrige` | …/eleitor/cconfirmavotoemcargo.cpp | cconfirmavotoemcargo.cpp | medium |
| 11700 | 25 | ✓ | `vota_f11700` | `vota::CConfirmaMajoritario::CConfirmaMajoritario` | …/eleitor/cconfirmavotoemcargo.cpp | cconfirmavotoemcargo.cpp | high |
| 11707 | 26 |  | `vota::CConfereVotoEmCargo<vota::CProporcionalBranco, (vota::ETelaVotacao)4>::vf16` | `vota::CConfereVotoEmCargo<vota::CProporcionalBranco, (vota::ETelaVotacao)4>::GetProximoEstado` | …/eleitor/cconferevotoemcargo.h | cconferevotoemcargo.h | medium |
| 11715 | 26 | ✓ | `vota::CConfereVotoEmCargo<vota::CConfirmaVotoNominal, (vota::ETelaVotacao)2>::vf16` | `vota::CConfereVotoEmCargo<vota::CConfirmaVotoNominal, (vota::ETelaVotacao)2>::GetProximoEstado` | …/eleitor/cconferevotoemcargo.h | cconferevotoemcargo.h | medium |
| 11716 | 26 |  | `vota::CConfereVotoEmCargo<vota::CCandidatoInexistente, (vota::ETelaVotacao)12>::vf16` | `vota::CConfereVotoEmCargo<vota::CCandidatoInexistente, (vota::ETelaVotacao)12>::GetProximoEstado` | …/eleitor/cconferevotoemcargo.h | cconferevotoemcargo.h | medium |
| 11717 | 26 | ✓ | `vota::CConfereVotoEmCargo<vota::CConfirmaVotoLegenda, (vota::ETelaVotacao)10>::vf16` | `vota::CConfereVotoEmCargo<vota::CConfirmaVotoLegenda, (vota::ETelaVotacao)10>::GetProximoEstado` | …/eleitor/cconferevotoemcargo.h | cconferevotoemcargo.h | medium |
| 11738 | 26 | ✓ | `vota::CConfereVotoEmCargo<vota::CProporcionalNulo, (vota::ETelaVotacao)7>::vf16` | `vota::CConfereVotoEmCargo<vota::CProporcionalNulo, (vota::ETelaVotacao)7>::GetProximoEstado` | …/eleitor/cconferevotoemcargo.h | cconferevotoemcargo.h | medium |
| 11739 | 26 |  | `vota::CConfereVotoEmCargo<vota::CCandidatoInapto, (vota::ETelaVotacao)14>::vf16` | `vota::CConfereVotoEmCargo<vota::CCandidatoInapto, (vota::ETelaVotacao)14>::GetProximoEstado` | …/eleitor/cconferevotoemcargo.h | cconferevotoemcargo.h | medium |
| 11751 | 5 |  | `vota::CConfirmaProporcional::vf16` | `vota::CConfirmaProporcional::GetEstadoCorrige` | …/eleitor/cconfirmavotoemcargo.cpp | cconfirmavotoemcargo.cpp | medium |
| 11752 | 25 | ✓ | `vota_f11752` | `vota::CConfirmaProporcional::CConfirmaProporcional` | …/eleitor/cconfirmavotoemcargo.cpp | cconfirmavotoemcargo.cpp | high |
| 11755 | 267 | ✓ | `vota::CConfirmaVotoEmCargo::vf10` | `vota::CConfirmaVotoEmCargo::StartStateAudio` | …/eleitor/cconfirmavotoemcargo.cpp | cconfirmavotoemcargo.cpp | high |
| 11791 | 71 | ✓ | `vota::IConfereVotoEmCargo::ProcessInputAudio` | `vota::IConfereVotoEmCargo::ProcessInputAudio` | …/eleitor/cconferevotoemcargo.cpp | cconferevotoemcargo.cpp | high |
| 11793 | 333 | ✓ | `vota::(anonymous namespace)::GetCargoID` | `vota::IConfereVotoEmCargo::StartStateAudio` | …/eleitor/cconferevotoemcargo.cpp | cconferevotoemcargo.cpp | high |
| 11858 | 562 |  | `vota::CDefineRotaPosReinicio::NeedChangeState` | `vota::CDefineRotaPosReinicio::NeedChangeState` | …/eleitor/cdefinerotaposreinicio.cpp | cdefinerotaposreinicio.cpp | high |
| 11939 | 5 |  | `vota::CGeraResumoZeresima::vf10` | `vota::CGeraResumoZeresima::GetProximoEstado` | …/eleitor/iniciovotacao/cgeradorresumozeresima.cpp | cajusteinicial.cpp (comment) | low |
| 12061 | 441 |  | `vota::CEstadoComDesligamentoAutomatico::ProcessTick(uebyte)::(lambda)::operator()` | `vota::CEstadoComDesligamentoAutomatico::ProcessTick` | …/eleitor/cestadocomdesligamentoautomatico.cpp | cestadocomdesligamentoautomatico.cpp | high |
| 13564 | 211 |  | `vota::impl::CPoliticaExecucaoEleitor::LimpaBufferInput` | `vota::impl::CPoliticaExecucaoEleitor::LimpaBufferInput` | …/eleitor/comum/cpoliticaexecucaoeleitor.cpp | comum/cpoliticaexecucaoeleitor.cpp | high |
