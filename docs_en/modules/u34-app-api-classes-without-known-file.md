# u34: `app:api` classes without a known source file (mesário states, start-of-day menus, BU QR header, support code)

Unit u34 holds **103 wasm functions** that the tools put in component `app:api` without an original source
file. There is no `std::source_location` record for them, or the one they carry belongs to an inlined
`api` header (`cinteractiveform.h:57`, `ctextsource.h:37`). That is why they were filed under `api`. Once
read, they fall into five groups:

| group | functions | what it is | § |
|---|---|---|---|
| **VOTA states** (`vota::C…::ProcessInput`, vtable slot 7) | 17 + 6 helpers | 11 poll-worker (*mesário*) microterminal states and 3 voter-screen states of the start of the election day, plus 3 singletons (5430, 5942, 5962), the helpers 6012, 6014, 10539, 12903 and the two merged `CLogVota` bodies 6113/6115 | 3, 4, 6 |
| **BU QR-code header** | 4 | `CCabecalhoQRCodeBuilder` setters `ZONA:` `SECA:` `IDUE:` `IDCA:` of the "BU digital" QR codes | 5 |
| **uenux2 `api` support code** | 46 | `CTime::IsValid`, `CDirReader::IsLink/Open`, `TrocaExtensao`, form-builder helpers, the status-header clock, the voter-screen form stack, the message-queue lock guard, merged destructors, poly-singleton registration thunks, atexit handlers | 7 |
| **comum / simulator code** | 7 | `CEleitores::GetEleitoresImpedidos`, `CCandidaturasDSNome`, `CTradutorFrase` labels, `CWasmLogBus` destructor | 6, 7 |
| **library code filed here by mistake** | 23 | 12 RHVoice 1.14.0 functions (TTS of the accessible vote) and 11 libc++/libc instances | 8 |

**22 of the 103 functions ran** during the recorded votes (`analysis/runtime/*.functions.tsv`). All of them
are support code that runs at `votaInit` or on every tick: the poly-singleton registrations of `main`, the
voter-roll loader (`GetEleitoresImpedidos` 5767, `TrocaExtensao` 5461, `CTime::IsValid` 5449), the voter-screen
form stack (`IForm<IScreen>::DoShow` 5541), the status-header clock (10870), the step progress bar (10971), the
message queue (7708, 5528) and a few library instances. **None of the VOTA states of this unit can run in the
simulator.** The mesário states belong to the operator thread, which the web build never runs (units u10,
u27). The voter-screen states belong to the start of the election day (restart, zerésima, "Mais
informações"), and the simulator starts the voter thread after that point. The BU header setters run only
during the *encerramento* (closing), which the page never reaches.

**Reconstructed sources** (all fragments, suffix `.u34`, to be merged into the named file; "(path inferred)"
means that no srcloc names the file):

```
src/uenux2/src/app/vota/operador/estadosoperador.u34.h                     declarations + layouts of the 11 MT states (paths inferred)
src/uenux2/src/app/vota/operador/aguardaeleitor/cdesabilitaaudioeleitor.u34.cpp   10435            (path inferred)
src/uenux2/src/app/vota/operador/aguardaeleitor/cconfirmainspecionada.u34.cpp     10527            (path inferred)
src/uenux2/src/app/vota/operador/aguardaeleitor/cmostraeleitorvotando.u34.cpp     10539            (file attested)
src/uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaocapturada.u34.cpp  10458            (path inferred)
src/uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.u34.cpp 6012          (file attested)
src/uenux2/src/app/vota/operador/justificativa/canoinformadoinvalido.u34.cpp      6014 10598 10601 (paths inferred)
src/uenux2/src/app/vota/operador/outrasopcoes/ccontadoresbiometria.u34.cpp        10698            (path inferred)
src/uenux2/src/app/vota/operador/outrasopcoes/cperguntafilaeleitorvazia.u34.cpp   10713 (+ CAguardaEleitoresVotarem ctor/GetInst)
src/uenux2/src/app/vota/operador/outrasopcoes/cconfirmaaudio.u34.cpp              10740 (+ CHabilitaAudioManualmente ctor/GetInst)
src/uenux2/src/app/vota/operador/outrasopcoes/chorariovotacaoterminou.u34.cpp     5430             (path inferred)
src/uenux2/src/app/vota/eleitor/iniciovotacao/estadosiniciovotacao.u34.h           declarations of the 3 voter-screen states
src/uenux2/src/app/vota/eleitor/iniciovotacao/creiniciovotacao.u34.cpp            11834            (path inferred)
src/uenux2/src/app/vota/eleitor/iniciovotacao/cconfirmaregerarzeresima.u34.cpp    11838 (+ CRegerarZeresima::GetInst)
src/uenux2/src/app/vota/eleitor/iniciovotacao/cmaisinformacoes.u34.cpp            11896            (path inferred)
src/uenux2/src/app/vota/eleitor/iniciovotacao/cgerazeresima.u34.cpp               5962 5942
src/uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.u34.cpp  12903            (file attested)
src/uenux2/src/app/vota/log/clogvota.u34.cpp                                      6113 6115        (file attested)
src/uenux2/src/app/comum/relatorios/ccabecalhoqrcodebuilder.u34.cpp               5618 5620 5621 5623 (file attested)
src/uenux2/src/app/comum/dados/celeitores.u34.cpp                                 5767 (+ notes 5379 6733)
src/uenux2/src/app/comum/dados/ccandidaturas.u34.cpp                              5807
src/uenux2/src/app/comum/dados/md/parametrizacaourna/ctradutorfrase.u34.cpp       5793 5794 5795 (as the source line)
src/uenux2/src/api/util/ctime.u34.cpp                                             5449
src/uenux2/src/api/util/cdirreader.u34.cpp                                        5469 5470
src/uenux2/src/api/util/cstringutils.u34.cpp                                      5461
src/uenux2/src/api/gui/cformbuilder.u34.cpp                                       10870
src/uenux2/src/api/gui/cformbuildermt.u34.cpp                                     5409             (path inferred)
src/uenux2/src/api/gui/cinteractiveformbuilder.u34.cpp                            5530
src/uenux2/src/api/ipc/clockguard.u34.h                                           5528             (path inferred)
src/uenux2/src/api/u34-foreign-fragments.cpp        merged dtors, factory body, Envia, registration thunks, atexit handlers, libc++ list
src/uenux2/src/api/audio/crhvoicetexttospeech.u34.cpp   index of the 12 RHVoice functions (library code, not rewritten)
src/uenux2/mock/app/simulador/wasm/cwasmlogbus.u34.cpp                            5154
```

Some bodies were already written by other units and are only referenced: `IForm<IScreen>::DoShow/DoShowOnTop/
Remove` (5541/5540/5987, `iform.h`, u17), `CabeLarguraProporcional` (10971, `cstepsprogressbar.cpp`, u16),
`CEncerraRegistroMesarios::GetInst` (5388, `cencerraregistromesarios.cpp`), `DefaultPolySingletonsInfo` (11265,
`cpolysingletonlist.h`, u19), and the three `ProcessInput`s of `CInformaEleitorPodeVotar /
CInformaAnoDesabilitadoDemo / CInformaBioDesabilitadaDemo` (10496/10504/10508,
`cinformaeleitorpodevotar.u33.cpp`, u33).

**Glossary.** *urna* voting machine; *eleitor* voter; *mesário* poll worker; *presidente (da mesa)* head poll
worker; *terminal do mesário* / **MT** the poll worker's microterminal (4 x 40 LCD, numeric keypad, LED,
buzzer, fingerprint reader); *habilitar o eleitor* release a voter to vote; *título (de eleitor)* voter
registration number; *áudio / fone de ouvido* the accessible vote read aloud through headphones;
*justificativa* justification of absence (a voter registered elsewhere declares he could not vote there);
*inspeção (da cabina)* periodic check of the voting booth; *encerramento* closing of the vote;
*zerésima* the "zero votes" report printed before voting starts; *reinício* restart of VOTA during the day;
*BU (boletim de urna)* per-machine result report; *via* printed copy; *QR code do BU / BU digital* the
signed QR codes at the bottom of the BU; *carga* the loading of the election data onto the urna (identified by
a 24-digit *código de carga*); *impedido* voter impeded from voting; *PU* parametrisation of the urna
(`*-pu.dat`); *CORRIGE / CONFIRMA / BRANCO* the orange / green / white keys.

---

## 1. Where this code sits in the voting process

VOTA runs two state machines of `comum::CAppState` objects (unit u06 §2):

* the **voter thread** (`CThreadEleitor`): the 640x480 voter screen and the voter keyboard. On election day it
  goes through the start-of-day states (keyboard test, zerésima, restart questions, "Mais informações"),
  then waits for the mesário to release each voter;
* the **operator thread** (`CThreadOperador`): the mesário's microterminal. It asks for the voter's título or
  CPF, checks the roll, captures the fingerprint, releases the voter, offers "outras opções" (audio, closing,
  mesário registration, counters) and periodically asks for an inspection of the booth.

The two threads talk through priority message queues (`api::CPriorityMessageQueue<api::SMessage>`, one per
thread at `+36`). `Envia` is `Add(msg, 1)` (func 7708). All `Add` calls of the binary use priority 1 except one:
`CVerificaEleicaoPassou::StartState` (11914) posts message 0 to the operator queue with priority 100.

This unit contains the **`ProcessInput()` (vtable slot 7) of 14 of these states**. The other slots are in
units u10, u17, u19, u27, u33 and u39. The functions were filed under `api` because
`api::CInteractiveForm<…>::Read()` is inlined into every one of them, with its `std::source_location`
record (`cinteractiveform.h:57`, the poly-singleton lookup of the keypad).

Next to them are the setters of the **header of the BU QR codes**, and some common support code of the `api`
library that runs at start-up.

---

## 2. Classes and hierarchy (RTTI)

```
api::CState
└─ comum::CAppState                         (+4 m_proximoEstado, +8/+9/+10 accepts messages/keys/ticks)
   ├─ MT states (operator thread), 20 bytes each unless noted, form at +12 = shared_ptr<CInteractiveForm<IScreenMT,IInputMT>>
   │   ├─ vota::CDesabilitaAudioEleitor     typeinfo @1592900  vtable @1592864   [2] 10436 (u39) [7] 10435
   │   ├─ vota::CConfirmaInspecionada       typeinfo @1590824  vtable @1590788   [7] 10527
   │   ├─ vota::CDigitalNaoCapturada        typeinfo @1592396  vtable @1592360   [7] 10458
   │   ├─ vota::CInformaEleitorPodeVotar    typeinfo @1591396  vtable @1591360   [7] 10496 ─┐
   │   ├─ vota::CInformaAnoDesabilitadoDemo typeinfo @1591220  vtable @1591184   [2] ICF 1718 [7] 10504  ├─ merged body 3883 (u33)
   │   ├─ vota::CInformaBioDesabilitadaDemo typeinfo @1591148  vtable @1591112   [2] ICF 1718 [7] 10508 ─┘
   │   ├─ vota::CAnoInformadoInvalido       typeinfo @1589704  vtable @1589668   [7] 10601 ─┐ merged body 6014
   │   ├─ vota::CEleitorMenor16Anos         typeinfo @1589776  vtable @1589740   [7] 10598 ─┘
   │   ├─ vota::CContadoresBiometria        typeinfo @1587968  vtable @1587932   [7] 10698
   │   ├─ vota::CPerguntaFilaEleitorVazia   typeinfo @1587736  vtable @1587700   [2] 10715 (u39) [7] 10713
   │   ├─ vota::CAguardaEleitoresVotarem    typeinfo @1587680  vtable @1587628   [2] 10718 (u27), [7] no-op 218, CAppState(0)
   │   ├─ vota::CConfirmaAudio              typeinfo @1587184  vtable @1587148   [7] 10740
   │   ├─ vota::CHabilitaAudioManualmente   typeinfo @1586996  vtable @1586960   [0] ICF 448 [1] ICF 765 [2] 10751 (u39) [7] no-op 218, 28 B, +20 shared_ptr<string>
   │   └─ vota::CHorarioVotacaoTerminou     typeinfo @1586868  vtable @1586832   [2] 10758 [7] 10757 (u27), GetInst 5430
   └─ voter-screen states (voter thread), form at +12 = CFormInterativoTelaVota (shared_ptr<CInteractiveForm<IScreen,IInputKbd>>)
       ├─ vota::CReinicioVotacao            typeinfo @1546672  vtable @1546636   [2] 11835 (u07) [7] 11834
       ├─ vota::CConfirmaRegerarZeresima    typeinfo @1546600  vtable @1546564   [2] ICF 5958    [7] 11838
       └─ vota::CMaisInformacoes            typeinfo @1545348  vtable @1545312   [2] 11897 (u39) [7] 11896, 24 B (+20 m_retorno)
```

Common vtable of these states: `[0]` destructor = ICF 244 (release the shared_ptr at +12), `[1]` deleting
destructor = ICF 387, `[2]` `StartState` (the ICF body 1070 `m_proximoEstado = this; m_form->Show();` unless
listed), `[3]` `NeedChangeState` 7480, `[4]` `GetNextState` 1661, `[5]` `FinishState` no-op, `[6]`
`ProcessMessage` no-op, `[7]` `ProcessInput`, `[8]` `ProcessTick` no-op. Every state is a lazily created
singleton (static `unique_ptr` + static mutex; in this single-threaded build only the `mutex::unlock` stub, func
150, remains). Most `GetInst` functions, and their constructors, are inlined into the state that switches to
them.

Other classes met in this unit: `comum::CCabecalhoQRCodeBuilder` (+0 `CCabecalhoQRCode`, 33 strings, 396
bytes), `api::CLockGuard` (4 bytes), `api::CDirReader` (20 bytes: +0 directory, +12 `DIR*`, +16 `dirent*`),
`comum::CCandidaturasDSNome` (1 byte: suplente index), `simulador::CWasmLogBus` (168 bytes, u29).

---

## 3. Microterminal (mesário) states

"`m_form->Read()`" below is the inlined `CInteractiveForm<IScreenMT, IInputMT>::Read()`. It gets the MT
keypad with `CPolySingletonList::instance<api::IInputMT>(loc)` (func 383) and lets the current input field
(`m_campos.at(m_indice)`, +72/+84, `std::out_of_range` past the end) read one key (field vtable slot 8). It
returns 5 for CORRIGE and 9 for CONFIRMA.

### 3.1 End of a vote with audio: `CDesabilitaAudioEleitor::ProcessInput` (10435)

The MT beeps and shows "Retire o fone de ouvido da urna / CONFIRMA". The operator thread goes there when the
voter who just finished had audio on (`CInformacaoEleitor::m_modoAudio != 2`). On CONFIRMA:

1. it posts **message 10** (`MSG_AUDIO_DESABILITADO`) to the voter thread's queue;
2. it **busy-waits** `while (CInformacaoEleitor::GetInst().m_modoAudio != 2) usleep(300);` until the voter
   thread has processed the message. There is no timeout (see §11);
3. log `"Áudio desativado pelo fim da votação"` (`CLogVota` func 4529);
4. next state `CPedeIdentidade` ("Digite o Título ou o CPF").

### 3.2 Inspection of the booth: `CConfirmaInspecionada::ProcessInput` (10527)

The periodic inspection works like this. Between voters, `CPedeIdentidade::ProcessTick` checks a random
deadline (now + 60..90 min, `SorteiaProximaInspecao`, func 3620). When it is due, the operator posts message
12 to the voter terminal and waits in `CAguardaInspecao` ("Inspecione cabina e urna"). The mesário confirms on
the voter keyboard (`CInspecionaUrna`, log "Inspeção da urna confirmada"). The voter thread posts **message
11** back, and `CAguardaInspecao::ProcessMessage(11)` (10530) builds `CConfirmaInspecionada` inline: clock at
{33,1}, "Inspeção completa" centred on line 2, "CONFIRMA: continuar a votação" right-aligned on line 4.

On CONFIRMA: log `"Inspeção da urna terminada"` (severity 1), **message 13** to the voter thread
(`CUrnaInspecionada` shows the "continue" screen and returns to `CAguardaMensagem`), draw the next inspection
time, then go to `CPedeIdentidade`.

### 3.3 Releasing a voter

* `CDigitalNaoCapturada::ProcessInput` (10458). If the voter's fingerprint is not recognised, the mesário
  can release the voter with **his own** fingerprint (`CRegistraDigitalOperador`). When no finger is read
  within 15 s and attempts remain, this screen shows "Digital não capturada / CONFIRMA: tentar novamente".
  CONFIRMA goes back to `CRegistraDigitalOperador::GetInst()` (5396).
* `CInformaEleitorPodeVotar` / `CInformaAnoDesabilitadoDemo` / `CInformaBioDesabilitadaDemo` (10496 /
  10504 / 10508) are one-line thunks into the merged body 3883 (u33). On CONFIRMA they either ask for the
  headphones (`CHabilitaAudioEleitor`) or post message 1 (`MSG_INICIA_ELEITOR`) to the voter thread and show
  `CMostraEleitorVotando`.
* `TextoAudioEleitor` (10539, table slot 4057) is line 3 of the `CMostraEleitorVotando` screen: it returns
  `"ÁUDIO ATIVADO"` when the voter thread exists and its audio mode is not 2, or when the mesário switched
  the audio on by hand (`GetAudioHabilitadoManualmente`, func 1687). Otherwise it returns `" "`.
* `FinalizaLeitorDigital` (6012) is the merged body of two `FinishState`s: `CRegistraDigitalOperador` (10453)
  and `comum::CPedeDigitalMesario` (10315). It turns the reader's LED off (`IFingerScanner` slot 7, 0) and
  ends the capture (slot 6). The two `std::source_location` records of the lookups are the parameters.

### 3.4 Justification errors: `CAnoInformadoInvalido` / `CEleitorMenor16Anos` (10601 / 10598 → 6014)

In the justification flow the mesário types the voter's year of birth (`CPedeAnoNascimento`, 10590, u17). An
impossible year leads to "Ano de nascimento inválido."; an age under 16 (counted by year) leads to "Eleitor
não pode votar ou justificar / por não ter idade mínima". Both screens say "CONFIRMA: tentar novamente".
Both `ProcessInput`s are the same merged body 6014, with the srcloc record as a parameter: CONFIRMA → back to
`CPedeAnoNascimento::GetInst()` (5418).

### 3.5 "Outras opções" menu (`CEscolheOpcao`, u27)

```
CEscolheOpcao  "Selecione a opção: _  1-Ativar áudio  2-Encerrar votação  [3-Registrar mesários] [3|4-Exibir contadores]"
 ├─1C, voting blocked by the clock ─> CHorarioVotacaoTerminou  (5430)  "Horario de votacao terminou! / Favor encerrar a urna! / CORRIGE"
 ├─1C, manual audio not allowed ───> CHabilitacaoAudioNaoPermitida
 ├─1C ─> CConfirmaAudio  "Deseja realmente ativar|desativar o áudio? / CORRIGE: não / CONFIRMA: sim"
 │         ProcessInput 10740: D ─> CEscolheOpcao ;  C ─> CHabilitaAudioManualmente (built inline)
 ├─2C ─> CIniciaFinalizacao ─> CPerguntaFilaEleitorVazia "Todas as pessoas presentes já votaram?"
 │         ProcessInput 10713: C ─> CPedeTituloEncerramento (presidente types his título)
 │                             D ─> CAguardaEleitoresVotarem (built inline) "Aguarde até todos os / eleitores presentes votarem"
 │                                  (StartState 10718: 3 s, then CPedeIdentidade)
 │                             log "Todas as pessoas presentes já votaram? SIM|NÃO"
 └─3C|4C ─> CContadoresBiometria  "Habilitação biométrica / biográfica / sem biometria: NNNN / CORRIGE: retornar"
           ProcessInput 10698: D ─> CEscolheOpcao
```

`CHorarioVotacaoTerminou` (GetInst + inlined ctor, 5430): `CAppState(2)`, `Beep(2)`, the three texts at {1,1},
{1,2}, {1,4} (left-aligned), a control input, `CriaFormInterativo("", true)`. The first text is stored
**without its accents** ("Horario de votacao", not "Horário de votação"); "Favor encerrar a urna!" needs none.
It is not the only unaccented MT text: `CVerificaDadoEleitor::ProcessInput` (10443) shows "CONFIRMA: cancelar
a habilitacao".

`CHabilitaAudioManualmente` (built inside 10740): 28 bytes, `CAppState(2)`, `m_texto =
shared_ptr<string>(new string("áudio habilitado/desabilitado"))` (+20), LED off, the text shown centred on
line 2 through `CDataTextFmt<CTextSource>` with format `"%s"`. The `CTextSource` constructor
(`ctextsource.h:37`) throws `CUeGuiError(4977, "Texto nulo")` on a null pointer, which cannot happen here. The
form has no control input: this state does not read keys (its StartState 10751 does the switching).

---

## 4. Voter-screen states of the start of the day

Before the vote starts, the mesário drives the urna from the **voter keyboard**. On these screens the BRANCO
key is labelled "Mais informações". The inlined `CInteractiveForm<IScreen, IInputKbd>::Read()` uses the
voter keyboard (`instance<IInputKbd>`, func 455) and field slot 10. It returns 3 for BRANCO, 5 for CORRIGE and
9 for CONFIRMA.

```
(VOTA restarted with votes/justifications recorded)
CReinicioVotacao  (StartState 11835, u07: log "Apresentada tela do reinício da votação")
   ProcessInput 11834:  C ─> log "Mesário confirmou o reinício da votação"  ─> CDefineRotaPosReinicio (5943)
                        B ─> log "Mesário selecionou outras opções"          ─> CMaisInformacoes::GetInst(this)
(no vote yet, zerésima generated by ANOTHER urna: EstadoGeralVota.urnaIdGerouZeresima != this urna)
CConfirmaRegerarZeresima  (StartState ICF 5958: log "Aguardando confirmação para emissão da zerésima")
   ProcessInput 11838:  C ─> CRegerarZeresima (lazy @1834912: CGeraZeresimaBase 5965 + vtable @1546500; prints the zerésima,
                                               then slot 9 cuts the paper, 11842)
                        B ─> CMaisInformacoes::GetInst(this)
CMaisInformacoes  (StartState 11897: builds CTelasVota::CriaTelaMaisInformacoes (6599) and shows it)
   "Mais informações: 1 Estado da urna (v/max)  2 Lista de eleitores (v/max)  3 Versões de pacotes (v/max)
    4 Parâmetros de urna (v/max)  5 Visualizar candidatos   CONFIRMA: Selecionar  CORRIGE: Retornar"
   ProcessInput 11896:  D ─> m_proximoEstado = m_retorno (+20); m_retorno = nullptr
                        C ─> text of input field 0; if non-empty and all digits: ToInt32 →
                             1 CImpressaoEstadoUrna  2 CImpressaoListaEleitores  3 CImpressaoVersaoPacotes
                             4 CImpressaoPU          5 CMenuVisualizarCandidatos ; anything else: stay
```

`CMaisInformacoes::GetInst(retorno)` (1280, u26) stores a non-null `retorno`. The five sub-states (and
`CMenuVisualizarCandidatos::StartState`) come back with `GetInst(nullptr)`, so the menu keeps its return state
until CORRIGE clears it. Every entry point into the menu (restart 11834, regenerate zerésima 11838,
`CVerificaHorarioZeresima` 11869, `CQuerReimprimirZeresima` 11848, `CConfirmaImpressaoZeresima` 11927) passes
`this`.

Two more singletons of the zerésima flow are in this unit. `CGeraZeresima::GetInst` (5962) is
`CGeraZeresimaBase` with vtable @1544584, used by `CConfirmaImpressaoZeresima` and `CImpressaoZeresimaTardia`.
`CReimprimindoResumoZeresima::GetInst` (5942) is a thunk into the merged lazy-singleton body 764.
`TextoFalhaTesteTeclado` (12903) is the text of the keyboard-test failure screen: `"Falha: Esperada {},
pressionada {}"` with `api::KeyName` of the expected and pressed keys stored in `CInformacaoEleitor` +9/+10.

---
## 5. Boletim de Urna (BU): the QR-code header fields ZONA, SECA, IDUE, IDCA

Four functions of this unit write fields of the **header of the BU QR codes**. These are the "BU digital"
printed under the BU and shown on the voter screen after the closing. See docs/bu/qrcode.md
for the whole payload, the hash chain and the signature. They are setters of
`comum::CCabecalhoQRCodeBuilder`, whose object starts with a `CCabecalhoQRCode`: 33 `std::string`, one per
tag, each holding the full `"TAG:valor "` text with its trailing blank. The builder is zero-filled by func
5624.

| func | field (offset) | format string | argument | source of the value |
|---|---|---|---|---|
| 5618 | `zona` (+108) | `"ZONA:{} "` @440346 | `unsigned` (libc++ format arg type 6) | `CEstadoGeral` (eg.bin) zona, 16-bit field at +24 |
| 5620 | `seca` (+120) | `"SECA:{} "` @440364 | `unsigned` | `CEstadoGeral` seção, 16-bit field at +26 |
| 5621 | `idue` (+144) | `"IDUE:{} "` @440220 | `unsigned` | `carga.numeroInternoUrna` (eg.bin +60) |
| 5623 | `idca` (+156) | `"IDCA:{:.24s} "` @440443 | `const std::string&` (stored as format type 13, string_view) | `carga.codigoCarga` (eg.bin +88, a `std::string`) |

Step by step, in `vota::CGeraBU::StartState` (func 12110, `cgerabu.cpp`, the inlined `comum::CriaQRCode` of
`cgeradorbu.cpp:124`) and in the on-screen version `vota::(anonymous)::GetQRDSInst` (1956):

1. `CConfiguracaoEleicao` +486 bit 0 (`imprimirQrCodeNoBU` of `EntidadeParametrizacaoUrna`) must be set.
   Otherwise no QR code is printed. In the pre-election demonstration mode the BU prints "NÃO HÁ QR CODE"
   instead (qrcode.md §7).
2. `comparecimento = CRdvVota::GetComparecimento()` (maximum over the elections, `shared_f1269`).
3. `CCabecalhoQRCode` constructed (5624, 396 zero bytes).
4. `SetZona(zona)`, `SetSecao(secao)`, `SetIdUrna(numeroInternoUrna)`, `SetCodigoCarga(codigoCarga)` (this
   unit), then `SetHistoricoCargas(...)` (5622: `"HIQT:{} "` and one `"HICA:{}:{:.24s} "` per
   correspondência of `gap.bin`).
5. `CGeradorBUQRCodeVota(cabecalho, comparecimento, dhEmissao)` (5603) fills ORIG, AGRE, LOCA, APTO…
   (11242), and `GeraQRCodes(1100)` (5604; 2500 for the screen version) adds ORLC, PROC, DTPL, PLEI, TURN,
   FASE, UNFE and VERS.
6. **Check.** `CCabecalhoQRCodeBuilder::preBuild` (lambda 2791, `ccabecalhoqrcodebuilder.cpp:284`) throws
   `CRelatoriosError(9050, "Campo (Zona) não informado.")` (or `Secao`, `IdUrna`, `CodigoCarga`, …) if a
   required field is still empty.
7. The header is concatenated in this order: `ORIG ORLC PROC DTPL PLEI TURN FASE UNFE MUNI ZONA SECA [AGRE]
   IDUE IDCA HIQT HICA… VERS`. The payload is split into slices, chained by hash, and the last slice is signed
   (`ASSI:`).

Formats. The numbers are written in decimal **without zero padding**: `ZONA:1 SECA:51`, as in the official
2024 BU QR codes. The carga code goes through `{:.24s}`: a code longer than 24 characters is **silently
truncated**. The code is 24 digits, so this only matters for malformed data (see §11). The only example that
the real code produced in this project is the simulator-fixture BU of
docs/10-boletim-de-urna.md (`ZONA:1 SECA:1 IDUE:87654321
IDCA:123456789012345678901234`).

Other BU-related pieces of this unit:

* **Vias (copies).** `CLogVota::LogaAviso55` (6115) is the merged body of the 55-character warnings (severity
  2). One of them is `"Quantidade de vias adicionais excede o máximo permitido"` (thunk 4556), logged by
  `CLimiteCopiasBUAtingido::StartState` and `CEmitirMaisBU::ProcessInput` when the mesário asks for more
  extra BU copies than the parametrisation allows.
* `IForm<IScreen>::Remove` (5987) is called by `CEmitirMaisBU::ProcessInput` (12081) to take the "emitir mais
  vias" screen off the voter display before printing the extra copies (body in `iform.h`, u17).
* **Zerésima.** `CConfirmaRegerarZeresima` (11838) regenerates the zero-votes report when the zerésima of this
  election was generated by another urna. `CGeraZeresima::GetInst` (5962) and
  `CReimprimindoResumoZeresima::GetInst` (5942) are the singletons of the print states (§4).

In the web build none of this runs: the page never reaches the encerramento.

---

## 6. Data read and written

**Log records** (`/dsk/fi/dinamico/log/logd.dat`, `"<aplicativo>|<severidade>|<texto>"`, Latin-1), written by
this unit's code:

| text | severity | where |
|---|---|---|
| Áudio desativado pelo fim da votação | 1 | 10435 (via 4529) |
| Inspeção da urna terminada | 1 | 10527 |
| Todas as pessoas presentes já votaram? SIM / NÃO | 1 | 10713 |
| Mesário confirmou o reinício da votação | 1 | 11834 |
| Mesário selecionou outras opções | 1 | 11834 |
| Operador selecionou: {opção} / Mesário {nome} habilitou o eleitor | 1 | 6113 (thunks 2502, 3258) |
| Habilitação cancelada durante reconhecimento biométrico | 2 | 6115 (thunk 2495) |
| Quantidade de vias adicionais excede o máximo permitido | 2 | 6115 (thunk 4556) |

**Inter-thread messages** (`SMessage{id, &fila}`, priority 1): operator → voter 10 (switch audio off), 13
(inspection finished), 1 (voter released, body 3883); voter → operator 11 (inspection confirmed, received by
10530).

**Files.**

* `*-imp.dat` (ModuloImpedidos, BER). `CEleitores::GetEleitoresImpedidos` (5767) reads every impedidos file
  of the section through `CFileASN::ReadFromFile<EntidadeImpedidos>` and `CConversorImpedido`, and
  concatenates the 32-byte `md::CImpedido` records. This runs at start-up (observed).
* `*-ce.dat` → `*-ce.pid`. `CStringUtils::TrocaExtensao` (5461) builds the name of the package-version
  header of each election file (used by `LePleito`, 5778).
* Directory walks. `CDirReader::Open/IsLink` (5470/5469) serve the media hash (`CMontadorHash`, hash.dat) and
  the recursive delete of `CSystem`. Symbolic links are detected with `lstat`.
* Labels of the parametrisation (`*-pu.dat`). `CTradutorFrase::s_labels` receives the Município, Zona, Seção
  and Partido labels (5795/5794/5793). The texts use them through placeholders such as `<SCSN>`.
* Times. `CTime::IsValid` (5449) validates every `hhmm[ss]` read from the election data.

---

## 7. `api` support code

| func | reconstruction | notes |
|---|---|---|
| 5449 | `CTime::IsValid(hora)` | 4 or 6 digits (mask 0x03FF… = '0'..'9'), hh<24, mm<60, ss<60 |
| 5469 / 5470 | `CDirReader::IsLink()` / `Open(dir)` | `lstat` of `dir + "/" + d_name`, `S_IFLNK` / `opendir` + replace the old handle |
| 5461 | `CStringUtils::TrocaExtensao(caminho, ext)` | keeps the dot, returns the input when the name has no dot |
| 5409 | `CFormBuilderMT::AddTextoCompartilhado(pos, CTextSource)` | MT text field over a shared `std::string` (`CDataText<CTextSource>`, left-aligned); objects made with `new` + `shared_ptr`, not `make_shared` |
| 5530 | `CInteractiveFormBuilder::AddControlInput(builder, teclas)` | invisible `CInputFieldControl<IScreen>` (key mask bit 0 BRANCO, 1 CORRIGE, 2 CONFIRMA) |
| 10870 | `DataHoraAtual(formato)` (slot 3117) | the clock of the voter-screen status header, "A DD/MM/YYYY hh:mm:ss"; time placeholders are expanded before date placeholders |
| 5540 / 5541 / 5987 | `IForm<IScreen>::DoShowOnTop / DoShow / Remove` | the static form stack of the voter display @1832632 (u17) |
| 5528 | `CLockGuard::~CLockGuard()` | `ISyncCtl::Unlock` (slot 3) under `invoke_vi`; terminate on exception (noexcept) |
| 7708 | `CPriorityMessageQueue<SMessage>::Envia(msg)` | `Add(msg, 1)` |
| 6023 | merged `CDefaultGenericFactory<I,C>::Create` | `new` + constructor by table slot, free + rethrow on exception |
| 6021/6022, 6062/6063, 6026 | merged destructors | `CTextRectField`/`CTextFieldDoubleLine`, `CMaskedTextField<…>`, `CFormPart`/`comum::CParteEleitores` (vtable as parameter) |
| 9372 … 9986 | `CPolySingletonList::push<I>(I*, info)` wrappers | the 8 registrations of `main` (semaphore, mutex, rwmutex, thread factories, fingerprint preparer, cipher factory, RNG, IExecucaoVota) |
| 10424 / 10464 / 10507, 11548 | `vector<SPolySingleton>::emplace_back` slow-path thunks, `__split_buffer` | for IExecucaoVota, ISincronismoVotoEleitor, IPoliticaExecucaoEleitor |
| 11265 | `DefaultPolySingletonsInfo()` | the registry @1832448 |
| atexit stubs | 8177, 8187, 8641, 8727, 10866, 10867, 10879, 10901, 11003, 11162, 11655, 11656, 11662, 12108, 12109 | destructors of statics: IForm stacks/mutexes/observables, CSynchronizer, CIniStrings escapes, CInputMenuField regex and default text, CApplication description, CInfoMTLCD |

Other code that is not `api`: `comum::CCandidaturasDSNome::operator()` (5807) returns the name of the current
candidate, or of suplente/vice n: `GetCandidaturaAtual("CCandidaturasDSNome")`, then `GetSuplente(n)` or the
titular (+8), then the name (+12). `simulador::CWasmLogBus::~CWasmLogBus` (5154) is the destructor of the
simulator's in-memory log bus (u29).

---

## 8. Library code filed in this unit

**RHVoice 1.14.0** (the TTS of the accessible vote). The main caller of these functions is
`CRHVoiceTextToSpeech::Sintetiza` (10841, 75 KB with the whole pipeline inlined), so the tools put them under
`app:api`. They were checked against the upstream sources of tag 1.14.0 (events.hpp, speech_processor.cpp,
equalizer.cpp, pitch.cpp, fst.hpp, language.cpp, userdict.cpp, voice_profile.hpp, utf.hpp):
`word_event`/`sentence_event` constructors (5212/5213), `speech_processor::finish` (5255),
`equalizer::read_coefs` (5254), `pitch::editor::reset` (5277), `vector<pitch::target_t>::push_back` (5280),
`fst::append_input_symbol(const item&)` (5298), `language::decode_as_character` (5302, with
`decode_as_unknown_character` inlined), `language::decode_as_digit_string` (5303), `userdict::dict::
simple_search` (5263), `voice_profile::voice_for_text` (5340), `std::distance` over `utf::text_iterator` (5432).
Index with details: `src/uenux2/src/api/audio/crhvoicetexttospeech.u34.cpp`.

**libc++ / libc.** `do_strerror_r` (4656), `unique_ptr<char[]>::reset` (4680: the `getcwd` buffer of the
inlined `__current_path` in `__do_absolute`; on musl libc++ takes the `pathconf` + `new char[size + 1]` branch,
not the glibc/Apple `free`-deleter one),
`filesystem::status(p, ec) noexcept` (4683), `ErrorHandler<bool>::report(errc)` (4684),
`_PathCVT<char>::__append_source` (7747), `filesystem::detail::vformat_string` (8095), `std::map` / `std::set`
instances (5379, 5793, 5794, 5795, 6733), `__split_buffer` (11548), an out-of-line `string::operator=(const
char*)` kept in the table (12979), and a `strerror` thunk kept in the table (12293).

---

## 9. What is specific to the web build

* **Dead states.** No `ProcessInput` of this unit can run in the browser. The operator thread is never
  stepped: `main` registers `CExecucaoVotaCooperativa`, which only feeds the voter thread. The voter thread
  starts in `CAguardaMensagem`, after the start-of-day states. So the periodic booth inspection, the audio
  switch of the MT, the fingerprint release of a voter, the justification flow, the closing question and the
  "Mais informações" menu do not exist in the simulator. `votaInit` posts messages 1 and 9 itself (unit u30).
* **Would hang or abort if reached.** `CDesabilitaAudioEleitor::ProcessInput` loops on `usleep`, which is a
  busy wait on `performance.now()` in this build (func 6265), with no other thread to change the flag.
  `CAguardaEleitoresVotarem::StartState` (reached from 10713) calls `emscripten_sleep`, which the glue turns
  into `abort()` (no Asyncify).
* **Simulator replacements visible here.** `CWasmSystemDateTime` feeds the status-header clock (10870). The
  registration wrappers (9372…) install `CWasmThread` / `CPosixMutex` / `CPosixSemaphore` factories and
  `CFingerPrepareSimulador`. The `CWasmLogBus` (5154) is a simulator-only object.

---

## 10. wasm / Emscripten observations

* **Merged bodies (wasm-opt merge-similar-functions).** Functions that differ only in a constant become one
  body plus one-line thunks. In this unit the constant is an srcloc record (6014, 3883, 6012), a format
  string (6113), a string literal cut into 8-byte pieces (6115), a vtable (6021/6022, 6062/6063, 6026), a
  table slot and a size (6023), or a push function slot (1562 ← 9372…9986). One C++ function therefore maps to
  a "thunk + body" pair.
* **Inlined singletons.** The `GetInst` and the constructor of `CAguardaEleitoresVotarem`,
  `CHabilitaAudioManualmente`, `CRegerarZeresima`, `CConfirmaInspecionada`, the four `CImpressao*` and others
  exist only inside the `ProcessInput` of the state that goes to them. Their static `unique_ptr` and mutex
  addresses are the only identity left.
* **Inlined `CInteractiveForm::Read`.** The srcloc record of `cinteractiveform.h:57` inside each
  `ProcessInput` made the tools file all these vota states under `api`. The MT version calls field slot 8
  with `IInputMT` (383); the voter-screen version calls slot 10 with `IInputKbd` (455).
* **noexcept through JS exceptions.** A `noexcept` function that calls something that may throw keeps the call
  inside `invoke_*` and has a landing pad that calls `__clang_call_terminate` (4683, 5528).
* **Functions kept only for the table.** 12293 (`strerror`) and 12979 (`string::operator=`) exist because
  some code calls them through `invoke_*`, which needs a table slot.
* **Constant propagation of statics.** 5794 is `map::operator=` specialised to the one static map it assigns
  (@1839056 hard-coded). 5767 has no `this` parameter (static member, or `this` removed as unused).
* **Busy-wait `usleep`.** musl's `nanosleep` is compiled to a spin on `emscripten_get_now` (6265).

---
## 11. Weird or risky code

1. **Unbounded busy wait for another thread (10435, `CDesabilitaAudioEleitor::ProcessInput`).** After posting
   message 10 the operator thread polls `CInformacaoEleitor::m_modoAudio` every 300 µs, with no timeout. On
   the urna, if the voter thread has stopped (an exception makes `CThreadVota` stop the application's
   threads, see u27 §4.1) or does not consume its queue, the mesário terminal freezes on "Retire o fone de
   ouvido". The field is a plain 4-byte `int` (`i32.load offset=4`, initialised to 2 by `GetInst` 509) that
   another thread writes. `GetInst` locks the singleton mutex only around the pointer, not around the field,
   so this is a data race in C++ terms, although the call re-reads the field each time. On the urna `usleep`
   yields the CPU, so the loop is a sleep-poll. In this build `usleep` spins on `performance.now()`, so
   reaching it would freeze the browser tab. The code is dead in the simulator.
2. **Unsynchronised cross-thread read in a text source (10539, `TextoAudioEleitor`).** The test of
   `CThreadEleitor`'s static `unique_ptr` is done under its singleton mutex @1833188 (the `lock` is compiled
   away in this build, and the `mutex::unlock` residue right after the read shows that a `lock_guard` was
   there). The voter's audio mode (`CInformacaoEleitor` +4) is then read after `CInformacaoEleitor::GetInst`
   has released its own mutex, as in 10435. On the urna the operator thread (which draws the MT) and the voter
   thread run at the same time, so this read races with the voter thread's write. The effect can only be
   cosmetic: "ÁUDIO ATIVADO" shown one refresh late.
3. **Silent truncation of the carga code in the BU QR header (5623).** `"IDCA:{:.24s} "` cuts any longer
   `codigoCarga` to 24 characters without an error, while `preBuild` only checks that the field is not
   empty. With a malformed `eg.bin` the signed QR code would carry a different IDCA from the stored one.
   Valid cargas have exactly 24 digits, so this is latent.
4. **`TrocaExtensao` loses the root slash (5461).** For a path directly under `/` (`"/x.dat"`) the directory
   part is empty and the result is `"x.pid"`, a relative path. Every path VOTA passes is under
   `/dsk/fi/estatico/`, so this is latent.
5. **The noexcept lock guard terminates on unlock failure (5528).** If `ISyncCtl::Unlock` throws (for example
   the POSIX wrapper reports a pthread error on the urna), `~CLockGuard` calls `std::terminate` instead of
   reporting. This is the normal C++ rule for destructors; it is listed because it is an abort path of every
   message send.
6. **Would abort in the browser if reached (10713 → `CAguardaEleitoresVotarem::StartState`).** The "não" branch
   of "Todas as pessoas presentes já votaram?" leads to a 3 s `api::CSystem::Sleep`, compiled as
   `if (byte@1584624 == 1) emscripten_sleep(3000)`. The byte is 1 and nothing writes it, and the glue's
   `_emscripten_sleep` calls `abort()` without Asyncify (already reported by u27). Dead code in the simulator.
7. **Inspection, justification, fingerprint release and manual audio are not simulated.** These operator
   flows are part of the real voting-day procedure (for example the random 60–90 min booth inspections), but
   the operator thread never runs in the web build. A trainee using the public simulator never sees them.
   This is a fidelity note, not a bug.
8. **Unaccented MT text (5430).** "Horario de votacao terminou!" lacks its accents ("Favor encerrar a
   urna!" needs none). Other MT texts are unaccented too (for example "CONFIRMA: cancelar a habilitacao" of
   `CVerificaDadoEleitor`, 10443), while most MT texts are accented. This is cosmetic.

---

## 12. Mapping table (all 103 functions)

"Observed" = sampled while the recorded votes ran (`analysis/runtime/vote_*.functions.tsv`). "Library"
rows are third-party code (libc++, musl, RHVoice 1.14.0) that the tools filed here. They are listed in the
fragment named in the row, not rewritten.

| func | size | observed | tools name | reconstructed symbol | source file | confidence |
|---|---|---|---|---|---|---|
| 4656 | 151 |  | `api_f4656` | `std::(anonymous namespace)::do_strerror_r` | library (libc++), listed in src/uenux2/src/api/u34-foreign-fragments.cpp | high |
| 4680 | 28 |  | `api_f4680` | `std::unique_ptr<char[]>::reset` (`__current_path` buffer, `delete[]` = free) | library (libc++) | medium |
| 4683 | 57 | ✓ | `api_f4683` | `std::filesystem::status(const path&, error_code&) noexcept` | library (libc++) | medium |
| 4684 | 52 |  | `api_f4684` | `std::filesystem::detail::ErrorHandler<bool>::report(const std::errc&)` | library (libc++) | medium |
| 5154 | 387 |  | `api_f5154` | `simulador::CWasmLogBus::~CWasmLogBus` | src/uenux2/mock/app/simulador/wasm/cwasmlogbus.u34.cpp | medium |
| 5212 | 327 |  | `api_f5212` | `RHVoice::word_event::word_event` | library (RHVoice 1.14.0), index in src/uenux2/src/api/audio/crhvoicetexttospeech.u34.cpp | high |
| 5213 | 646 |  | `api_f5213` | `RHVoice::sentence_event::sentence_event` | library (RHVoice 1.14.0) | high |
| 5254 | 1114 |  | `api_f5254` | `RHVoice::equalizer::read_coefs` | library (RHVoice 1.14.0) | high |
| 5255 | 372 |  | `api_f5255` | `RHVoice::speech_processor::finish` | library (RHVoice 1.14.0) | high |
| 5263 | 85 |  | `api_f5263` | `RHVoice::userdict::dict::simple_search` | library (RHVoice 1.14.0) | high |
| 5277 | 370 |  | `api_f5277` | `RHVoice::pitch::editor::reset` | library (RHVoice 1.14.0) | high |
| 5280 | 230 |  | `api_f5280` | `std::vector<RHVoice::pitch::target_t>::push_back` | library (libc++ instantiation) | medium |
| 5298 | 270 |  | `api_f5298` | `RHVoice::fst::append_input_symbol(const item&, input_symbols&)` | library (RHVoice 1.14.0) | high |
| 5302 | 1687 |  | `api_f5302` | `RHVoice::language::decode_as_character` | library (RHVoice 1.14.0) | high |
| 5303 | 1384 |  | `api_f5303` | `RHVoice::language::decode_as_digit_string` | library (RHVoice 1.14.0) | high |
| 5340 | 415 |  | `api_f5340` | `RHVoice::voice_profile::voice_for_text` | library (RHVoice 1.14.0) | high |
| 5379 | 309 |  | `api_f5379` | `std::map<std::string, V>::insert(value_type&&) (__emplace_unique_key_args)` | library (libc++ instantiation), noted in src/uenux2/src/app/comum/dados/celeitores.u34.cpp | medium |
| 5388 | 111 |  | `api_f5388` | `comum::CEncerraRegistroMesarios::GetInst` | src/uenux2/src/app/comum/comparecimentomesario/estados/cencerraregistromesarios.cpp (written by another unit) | high |
| 5409 | 356 |  | `api_f5409` | `api::CFormBuilderMT::AddTextoCompartilhado (Add<CTextFieldMT> with CDataText<CTextSource>)` | src/uenux2/src/api/gui/cformbuildermt.u34.cpp | medium |
| 5430 | 948 |  | `api_f5430` | `vota::CHorarioVotacaoTerminou::GetInst` | src/uenux2/src/app/vota/operador/outrasopcoes/chorariovotacaoterminou.u34.cpp | high |
| 5432 | 1278 |  | `api_f5432` | `std::distance<RHVoice::utf::text_iterator<std::string::const_iterator>>` | library (RHVoice/libc++ instantiation) | medium |
| 5449 | 495 | ✓ | `api_f5449` | `api::CTime::IsValid` | src/uenux2/src/api/util/ctime.u34.cpp | medium |
| 5461 | 1341 | ✓ | `api_f5461` | `api::CStringUtils::TrocaExtensao` | src/uenux2/src/api/util/cstringutils.u34.cpp | low |
| 5469 | 352 |  | `api_f5469` | `api::CDirReader::IsLink` | src/uenux2/src/api/util/cdirreader.u34.cpp | high |
| 5470 | 224 |  | `api_f5470` | `api::CDirReader::Open` | src/uenux2/src/api/util/cdirreader.u34.cpp | high |
| 5528 | 64 | ✓ | `api_f5528` | `api::CLockGuard::~CLockGuard` | src/uenux2/src/api/ipc/clockguard.u34.h | medium |
| 5530 | 121 |  | `api_f5530` | `api::CInteractiveFormBuilder::AddControlInput(CFormBuilder&, unsigned)` | src/uenux2/src/api/gui/cinteractiveformbuilder.u34.cpp | medium |
| 5540 | 289 |  | `api_f5540` | `api::IForm<api::IScreen>::DoShowOnTop` | src/uenux2/src/api/gui/iform.h (written by unit u17) | medium |
| 5541 | 212 | ✓ | `api_f5541` | `api::IForm<api::IScreen>::DoShow` | src/uenux2/src/api/gui/iform.h (written by unit u17) | medium |
| 5618 | 414 |  | `api_f5618` | `comum::CCabecalhoQRCodeBuilder::SetZona` | src/uenux2/src/app/comum/relatorios/ccabecalhoqrcodebuilder.u34.cpp | medium |
| 5620 | 417 |  | `api_f5620` | `comum::CCabecalhoQRCodeBuilder::SetSecao` | src/uenux2/src/app/comum/relatorios/ccabecalhoqrcodebuilder.u34.cpp | medium |
| 5621 | 419 |  | `api_f5621` | `comum::CCabecalhoQRCodeBuilder::SetIdUrna` | src/uenux2/src/app/comum/relatorios/ccabecalhoqrcodebuilder.u34.cpp | medium |
| 5623 | 463 |  | `api_f5623` | `comum::CCabecalhoQRCodeBuilder::SetCodigoCarga` | src/uenux2/src/app/comum/relatorios/ccabecalhoqrcodebuilder.u34.cpp | medium |
| 5767 | 2553 | ✓ | `api_f5767` | `comum::CEleitores::GetEleitoresImpedidos` | src/uenux2/src/app/comum/dados/celeitores.u34.cpp | high |
| 5793 | 256 | ✓ | `api_f5793` | `std::pair<const char, ecourna::app::dados::CLabelParametrizado>::pair(const pair&)` | library (libc++ instantiation), noted in src/uenux2/src/app/comum/dados/md/parametrizacaourna/ctradutorfrase.u34.cpp | medium |
| 5794 | 1483 |  | `api_f5794` | `std::map<char, CLabelParametrizado>::operator=(const map&) on CTradutorFrase::s_labels` | library (libc++ instantiation), noted in ctradutorfrase.u34.cpp | medium |
| 5795 | 345 | ✓ | `api_f5795` | `std::map<char, CLabelParametrizado>::map(std::initializer_list)` | library (libc++ instantiation), noted in ctradutorfrase.u34.cpp | medium |
| 5807 | 204 |  | `api_f5807` | `comum::CCandidaturasDSNome::operator()` | src/uenux2/src/app/comum/dados/ccandidaturas.u34.cpp | medium |
| 5942 | 22 |  | `api_f5942` | `vota::CReimprimindoResumoZeresima::GetInst` | src/uenux2/src/app/vota/eleitor/iniciovotacao/cgerazeresima.u34.cpp | high |
| 5962 | 93 |  | `api_f5962` | `vota::CGeraZeresima::GetInst` | src/uenux2/src/app/vota/eleitor/iniciovotacao/cgerazeresima.u34.cpp | high |
| 5987 | 411 |  | `api_f5987` | `api::IForm<api::IScreen>::Remove` | src/uenux2/src/api/gui/iform.h (written by unit u17) | medium |
| 6012 | 100 |  | `api_f6012` | `vota::(anonymous)::FinalizaLeitorDigital (merged FinishState body)` | src/uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.u34.cpp | medium |
| 6014 | 127 |  | `api_f6014` | `vota::(anonymous)::VoltaParaAnoNascimentoSeConfirma (merged ProcessInput body)` | src/uenux2/src/app/vota/operador/justificativa/canoinformadoinvalido.u34.cpp | medium |
| 6021 | 101 |  | `api_f6021` | `api::CTextRectField / CTextFieldDoubleLine deleting destructor (merged body)` | src/uenux2/src/api/u34-foreign-fragments.cpp | high |
| 6022 | 98 |  | `api_f6022` | `api::CTextRectField / CTextFieldDoubleLine complete destructor (merged body)` | src/uenux2/src/api/u34-foreign-fragments.cpp | high |
| 6023 | 64 | ✓ | `api_f6023` | `api::CDefaultGenericFactory<I,C>::Create (merged body)` | src/uenux2/src/api/u34-foreign-fragments.cpp | high |
| 6026 | 66 |  | `api_f6026` | `api::CFormPart / comum::CParteEleitores deleting destructor (merged body)` | src/uenux2/src/api/u34-foreign-fragments.cpp | high |
| 6062 | 101 |  | `api_f6062` | `api::CMaskedTextField<MASK> deleting destructor (merged body)` | src/uenux2/src/api/u34-foreign-fragments.cpp | high |
| 6063 | 98 |  | `api_f6063` | `api::CMaskedTextField<MASK> complete destructor (merged body)` | src/uenux2/src/api/u34-foreign-fragments.cpp | high |
| 6113 | 446 |  | `api_f6113` | `vota::CLogVota::LogaFormatado (merged body)` | src/uenux2/src/app/vota/log/clogvota.u34.cpp | medium |
| 6115 | 160 |  | `api_f6115` | `vota::CLogVota::LogaAviso55 (merged body)` | src/uenux2/src/app/vota/log/clogvota.u34.cpp | medium |
| 6733 | 361 |  | `api_f6733` | `std::set<int>::set(first, last) (range insert with end hint)` | library (libc++ instantiation), noted in celeitores.u34.cpp | medium |
| 7708 | 11 | ✓ | `api_f7708` | `api::CPriorityMessageQueue<api::SMessage>::Envia(const SMessage&)` | src/uenux2/src/api/u34-foreign-fragments.cpp | medium |
| 7747 | 20 |  | `api_f7747` | `std::filesystem::_PathCVT<char>::__append_source<const char*>` | library (libc++) | medium |
| 8095 | 221 |  | `api_f8095` | `std::filesystem::detail::vformat_string` | library (libc++) | high |
| 8177 | 39 |  | `api_f8177` | `atexit: ~std::vector<IForm<IScreenMT>*> IForm<IScreenMT>::ms_pilha` | src/uenux2/src/api/u34-foreign-fragments.cpp | high |
| 8187 | 10 |  | `api_f8187` | `atexit: ~std::mutex IForm<IScreenMT>::ms_mutex` | src/uenux2/src/api/u34-foreign-fragments.cpp | high |
| 8641 | 10 |  | `api_f8641` | `atexit: ~std::mutex IForm<IScreen>::ms_mutex` | src/uenux2/src/api/u34-foreign-fragments.cpp | high |
| 8727 | 11 |  | `api_f8727` | `atexit: ~IObservable IForm<IScreenMT>::ms_observavel` | src/uenux2/src/api/u34-foreign-fragments.cpp | high |
| 9372 | 12 | ✓ | `api_f9372` | `api::CPolySingletonList::push<vota::IExecucaoVota>(I*, info) wrapper` | src/uenux2/src/api/u34-foreign-fragments.cpp | high |
| 9574 | 12 | ✓ | `api_f9574` | `api::CPolySingletonList::push<IGenericFactory<ISemaphore>>(I*, info) wrapper` | src/uenux2/src/api/u34-foreign-fragments.cpp | high |
| 9650 | 12 | ✓ | `api_f9650` | `api::CPolySingletonList::push<IGenericFactory<IRWSyncCtl>>(I*, info) wrapper` | src/uenux2/src/api/u34-foreign-fragments.cpp | high |
| 9685 | 12 | ✓ | `api_f9685` | `api::CPolySingletonList::push<IGenericFactory<ISyncCtl>>(I*, info) wrapper` | src/uenux2/src/api/u34-foreign-fragments.cpp | high |
| 9731 | 12 | ✓ | `api_f9731` | `api::CPolySingletonList::push<IGenericFactory<IThreadImpl>>(I*, info) wrapper` | src/uenux2/src/api/u34-foreign-fragments.cpp | high |
| 9831 | 12 | ✓ | `api_f9831` | `api::CPolySingletonList::push<api::IFingerPrepare>(I*, info) wrapper` | src/uenux2/src/api/u34-foreign-fragments.cpp | high |
| 9897 | 12 | ✓ | `api_f9897` | `api::CPolySingletonList::push<ecourna::api::security::ISymmetricCipherFactory>(I*, info) wrapper` | src/uenux2/src/api/u34-foreign-fragments.cpp | high |
| 9986 | 12 | ✓ | `api_f9986` | `api::CPolySingletonList::push<ecourna::api::security::IRng>(I*, info) wrapper` | src/uenux2/src/api/u34-foreign-fragments.cpp | high |
| 10424 | 16 |  | `api_f10424` | `std::vector<api::SPolySingleton>::emplace_back slow-path thunk (push<vota::IExecucaoVota>)` | src/uenux2/src/api/u34-foreign-fragments.cpp | high |
| 10435 | 200 |  | `vota::CDesabilitaAudioEleitor::vf7` | `vota::CDesabilitaAudioEleitor::ProcessInput` | src/uenux2/src/app/vota/operador/aguardaeleitor/cdesabilitaaudioeleitor.u34.cpp | high |
| 10458 | 130 |  | `vota::CDigitalNaoCapturada::vf7` | `vota::CDigitalNaoCapturada::ProcessInput` | src/uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaocapturada.u34.cpp | high |
| 10464 | 16 |  | `api_f10464` | `std::vector<api::SPolySingleton>::emplace_back slow-path thunk (push<vota::impl::ISincronismoVotoEleitor>)` | src/uenux2/src/api/u34-foreign-fragments.cpp | high |
| 10496 | 12 |  | `vota::CInformaEleitorPodeVotar::vf7` | `vota::CInformaEleitorPodeVotar::ProcessInput` | src/uenux2/src/app/vota/operador/confirmaidentidade/cinformaeleitorpodevotar.u33.cpp (written by unit u33) | high |
| 10504 | 12 |  | `vota::CInformaAnoDesabilitadoDemo::vf7` | `vota::CInformaAnoDesabilitadoDemo::ProcessInput` | src/uenux2/src/app/vota/operador/confirmaidentidade/cinformaeleitorpodevotar.u33.cpp (written by unit u33) | high |
| 10507 | 16 |  | `api_f10507` | `std::vector<api::SPolySingleton>::emplace_back slow-path thunk (push<vota::impl::IPoliticaExecucaoEleitor>)` | src/uenux2/src/api/u34-foreign-fragments.cpp | high |
| 10508 | 12 |  | `vota::CInformaBioDesabilitadaDemo::vf7` | `vota::CInformaBioDesabilitadaDemo::ProcessInput` | src/uenux2/src/app/vota/operador/confirmaidentidade/cinformaeleitorpodevotar.u33.cpp (written by unit u33) | high |
| 10527 | 300 |  | `vota::CConfirmaInspecionada::vf7` | `vota::CConfirmaInspecionada::ProcessInput` | src/uenux2/src/app/vota/operador/aguardaeleitor/cconfirmainspecionada.u34.cpp | high |
| 10539 | 124 |  | `api_f10539` | `vota::(anonymous)::TextoAudioEleitor` | src/uenux2/src/app/vota/operador/aguardaeleitor/cmostraeleitorvotando.u34.cpp | medium |
| 10598 | 12 |  | `vota::CEleitorMenor16Anos::vf7` | `vota::CEleitorMenor16Anos::ProcessInput` | src/uenux2/src/app/vota/operador/justificativa/canoinformadoinvalido.u34.cpp | high |
| 10601 | 12 |  | `vota::CAnoInformadoInvalido::vf7` | `vota::CAnoInformadoInvalido::ProcessInput` | src/uenux2/src/app/vota/operador/justificativa/canoinformadoinvalido.u34.cpp | high |
| 10698 | 130 |  | `vota::CContadoresBiometria::vf7` | `vota::CContadoresBiometria::ProcessInput` | src/uenux2/src/app/vota/operador/outrasopcoes/ccontadoresbiometria.u34.cpp | high |
| 10713 | 1389 |  | `vota::CPerguntaFilaEleitorVazia::vf7` | `vota::CPerguntaFilaEleitorVazia::ProcessInput` | src/uenux2/src/app/vota/operador/outrasopcoes/cperguntafilaeleitorvazia.u34.cpp | high |
| 10740 | 944 |  | `vota::CConfirmaAudio::vf7` | `vota::CConfirmaAudio::ProcessInput` | src/uenux2/src/app/vota/operador/outrasopcoes/cconfirmaaudio.u34.cpp | high |
| 10866 | 10 |  | `api_f10866` | `atexit: ~std::mutex @1839300 (CSynchronizer instance mutex)` | src/uenux2/src/api/u34-foreign-fragments.cpp | low |
| 10867 | 12 |  | `api_f10867` | `atexit: ~std::unique_ptr<api::CSynchronizer> s_instancia` | src/uenux2/src/api/u34-foreign-fragments.cpp | high |
| 10870 | 98 | ✓ | `api_f10870` | `api::(anonymous)::DataHoraAtual` | src/uenux2/src/api/gui/cformbuilder.u34.cpp | medium |
| 10879 | 28 |  | `api_f10879` | `atexit: ~std::vector<std::pair<std::string,char>> CIniStrings ESCAPES` | src/uenux2/src/api/u34-foreign-fragments.cpp | high |
| 10901 | 17 |  | `api_f10901` | `atexit: ~std::regex CInputMenuField marcadores("%[ST]")` | src/uenux2/src/api/u34-foreign-fragments.cpp | high |
| 10971 | 2127 | ✓ | `api_f10971` | `api::CabeLarguraProporcional` | src/uenux2/src/api/gui/cstepsprogressbar.cpp (written by unit u16) | medium |
| 11003 | 11 |  | `api_f11003` | `atexit: ~IObservable IForm<IPaper>::ms_observavel` | src/uenux2/src/api/u34-foreign-fragments.cpp | high |
| 11162 | 36 |  | `api_f11162` | `atexit: ~std::string api::CApplication::ms_descricao` | src/uenux2/src/api/u34-foreign-fragments.cpp | high |
| 11265 | 39 | ✓ | `api_f11265` | `api::DefaultPolySingletonsInfo` | src/uenux2/src/api/pattern/cpolysingletonlist.h (declared by unit u19) | medium |
| 11548 | 93 |  | `api_f11548` | `std::__split_buffer<api::SPolySingleton, allocator&>::__split_buffer` | library (libc++ instantiation), noted in src/uenux2/src/api/u34-foreign-fragments.cpp | high |
| 11655 | 10 |  | `api_f11655` | `atexit: ~std::mutex comum::CInfoMTLCD instance mutex` | src/uenux2/src/api/u34-foreign-fragments.cpp | high |
| 11656 | 40 |  | `api_f11656` | `atexit: ~std::unique_ptr<comum::CInfoMTLCD> s_instancia` | src/uenux2/src/api/u34-foreign-fragments.cpp | high |
| 11662 | 11 |  | `api_f11662` | `atexit: ~std::string api::CInputMenuField::ms_textoInstrucaoPadrao` | src/uenux2/src/api/u34-foreign-fragments.cpp | high |
| 11834 | 448 |  | `vota::CReinicioVotacao::vf7` | `vota::CReinicioVotacao::ProcessInput` | src/uenux2/src/app/vota/eleitor/iniciovotacao/creiniciovotacao.u34.cpp | high |
| 11838 | 242 |  | `vota::CConfirmaRegerarZeresima::vf7` | `vota::CConfirmaRegerarZeresima::ProcessInput` | src/uenux2/src/app/vota/eleitor/iniciovotacao/cconfirmaregerarzeresima.u34.cpp | high |
| 11896 | 717 |  | `vota::CMaisInformacoes::vf7` | `vota::CMaisInformacoes::ProcessInput` | src/uenux2/src/app/vota/eleitor/iniciovotacao/cmaisinformacoes.u34.cpp | high |
| 12108 | 39 |  | `api_f12108` | `atexit: ~std::vector<IForm<IPaper>*> IForm<IPaper>::ms_pilha` | src/uenux2/src/api/u34-foreign-fragments.cpp | high |
| 12109 | 10 |  | `api_f12109` | `atexit: ~std::mutex IForm<IPaper>::ms_mutex` | src/uenux2/src/api/u34-foreign-fragments.cpp | high |
| 12293 | 7 |  | `api_f12293` | `strerror thunk (table slot 6211)` | library (libc thunk), listed in src/uenux2/src/api/u34-foreign-fragments.cpp | low |
| 12903 | 542 |  | `api_f12903` | `vota::testeteclado::(anonymous)::TextoFalhaTesteTeclado` | src/uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.u34.cpp | medium |
| 12979 | 9 | ✓ | `api_f12979` | `std::string::operator=(const char*) (out-of-line, table slot 168)` | library (libc++), listed in src/uenux2/src/api/u34-foreign-fragments.cpp | medium |

---

## 13. Open questions

* The exact TSE names of the merged helpers (6014, 6012, 6113, 6115), of the BU-header setters
  (5618/5620/5621/5623: only the format strings and the field order are certain) and of `TrocaExtensao` /
  `DataHoraAtual` / `TextoAudioEleitor` / `TextoFalhaTesteTeclado` are inferred.
* The file of the MT form builder (`cformbuildermt.h`?) and of `CLockGuard` (`clockguard.h`?) is unknown.
* `CGeraBU` checks `imprimirQrCodeNoBU`, but the check that `codigoCarga` has 24 digits (if one exists)
  happens elsewhere, maybe in `CEstadoGeral`'s loader. It was not found in this unit.
* The 12293 `strerror` thunk: the `invoke_ii(6211, …)` site was not located (no `i32.const 6211` in the wat).
  Its table neighbours suggest `ecourna::api::io::CFile`'s error messages.
