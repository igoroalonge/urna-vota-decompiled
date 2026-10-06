# u10 — `uenux2/src/app/vota/operador`: identifying and releasing the voter on the mesário terminal

Unit u10 covers 87 wasm functions of the **operator side** ("operador") of the VOTA application: the
states that run on the *terminal do mesário* while a voter is identified, has his fingerprint checked
and is released ("habilitado") to vote, the state shown while he votes, the automatic suspension of
voter-training mode, and the operator thread's shared blackboard `IInformacaoThreadOperador`.
**None of the 87 functions ran during the recorded web votes**: the public simulator never runs the
operator thread (§2). The flow was checked instead with the project's patched-wasm harness
`tools/bu/operator_harness.mjs` (its recorded run `samples/bu-real/run-full`, plus one run made for this
unit, §2; the patched wasm itself is `work/bu-real/vota_web_wasm.opstep.wasm`).

Reconstructed sources (`src/uenux2/src/app/vota/operador/`):
`comum/iinformacaothreadoperador.h`, `comum/cinformacaothreadoperador.{h,cpp}`,
`aguardaeleitor/cmostraeleitorvotando.{h,cpp}`, `aguardaeleitor/csuspensaoautomaticaeleitor.{h,cpp}`,
`confirmaidentidade/ccontrolareconhecimento.{h,cpp}`, `confirmaidentidade/cpededigital.{h,cpp}`,
`confirmaidentidade/cdigitalreconhecida.{h,cpp}`, `confirmaidentidade/cdigitalnaoreconhecida.{h,cpp}`,
`confirmaidentidade/cnomeeleitor.{h,cpp}` and `u10-foreign-fragments.cpp`. The fragments file holds
the functions that the tools put in this unit but that belong to other files: singleton accessors of
neighbouring states, one state method, data constructors, thunks and form-builder template instances.

## 1. Glossary

| term | meaning |
|---|---|
| mesário / operador | poll worker; uses the *terminal do mesário* ("micro-terminal", **MT**: 4×40 LCD, keypad, LED, buzzer, fingerprint sensor) |
| eleitor | voter |
| habilitar / habilitação | releasing the urna for one identified voter. The urna records **how** he was released (`md::ETipoHabilitacao`) |
| título (de eleitor) / CPF | voter-card number (12 digits) / taxpayer id (11 digits); "identificador livre" is the third identity type |
| identidade principal | the identity type that keys the section roll (`CConfiguracaoEleicao` +668) |
| biometria / digital / dedo | fingerprint biometrics / a fingerprint / a finger |
| reconhecimento 1x4 | the captured print is compared with the stored template of up to 4 fingers: right thumb, left thumb, right index, left index (finger codes 1, 6, 2, 7) |
| tentativa | capture attempt, limited by `ParametrosUrna.numTentativasHabilitacao` |
| ano de nascimento | birth year, the biographic check used when biometrics are missing or fail |
| caderno de votação | the paper roll that voters sign. A biometrically recognised voter is told **not** to sign it |
| WSQ | Wavelet Scalar Quantization, the FBI fingerprint-image format; the urna keeps encrypted WSQ images of the captures |
| cabina | voting booth ("CABINA: LIVRE / OCUPADA" on the MT) |
| treinamento de eleitores | voter-training mode of a training-phase urna (`CEstadoGeralVota.treinamentoEleitor`, +72). The web simulator runs in this mode |
| treinamento (de mesários) | training phase without voter training: the biometric flow is **simulated** (§4.4) |
| demonstração | demo mode (`IInterfaceInit::GetDemoMode`): biometrics and the birth-year check are replaced by notices |
| suspensão | ending the session of a voter who does not finish (u06 §4.7) |
| inspeção | booth inspection the mesário is asked to do at random intervals |

## 2. Where this code runs (and why it never ran in the web simulator)

VOTA has two state-machine threads (u06 §2): the voter thread `CThreadEleitor` and the operator
thread `CThreadOperador` (132-byte singleton at @1911708, built by func 270; the tools call that
function `CPriorityMessageQueue<SMessage>::CPriorityMessageQueue@270`). The operator thread owns a
`CThreadVota` tick table at +20, the message queue `vota::CMessageOperador` at +36 and the text of
the cargo being voted at +108. Its states are `comum::CAppState` singletons, with the slot protocol of
u06 §2 (2 StartState, 3 NeedChangeState, 4 GetNextState, 5 FinishState, 6 ProcessMessage,
7 ProcessInput, 8 ProcessTick).

**Web build.** `main` registers `CExecucaoVotaCooperativa`, and `votaTick` only steps
`CThreadEleitor` (u06/u07). `CThreadOperador::Run` (func 10204) is never called. The voter states
still post their operator messages (6 at the start, 9 per cargo, 13 and 1 at the end). They sit
unread in `CThreadOperador`'s queue: after one municipal vote the queue held 5 entries (16 bytes each)
`6, 9, 9, 13, 1`, and the singletons of `CPedeIdentidade`, `CPedeDigital` and `CMostraEleitorVotando`
were never created (headless probe reading @1911708/@1909064/@1909316 at exit). The page reloads for
every voter, so the backlog stays small.

**Harness evidence.** `tools/bu/operator_harness.mjs` appends an `opStep` export (one iteration of
`CThreadOperador::Run`) to a copy of the wasm and drives both threads. Its recorded "full" run
(`samples/bu-real/run-full/states.txt`, `mt.txt`) goes through this unit's code:
`CPedeIdentidade → CProcuraEleitor → CEleitorEncontrado → CNomeEleitor → CPedeAnoNascimentoSemBiometria
→ CInformaEleitorPodeVotar → CMostraEleitorVotando (VOTANDO PARA: Vereador / Prefeito) →
CSincronismoOperador`. The row it left in `eleitor_dinamico` (`tipo_habilitacao` 0,
`tipo_ativacao_audio` 2) and the BU (`detalhamentoComparecimento.qtdEleitoresCompareceramSemBiometria = 1`,
QR `HBSB:1`) match `SalvaHabilitacaoEleitor` (§4.5). A second run made for this unit
(`--script "geradin zeresima ot:20 om:8 ot:50 o:XXXXXXXXXXXXC ot:50 o:C ot:50 o:AAAAC ot:50 o:C ot:50
treino:1 t:20 clock:2026-10-04T16=59=00-03=00 t:60 t:60"`) set `treinamentoEleitor = 1` after the
habilitação and moved the fake clock forward 9 minutes. It reached
`CSuspensaoAutomaticaEleitor::StartState` and died with `thread constructor failed: Not supported`
from func 5392 (§6.1). Re-run by the fidelity review (2026-09-23): the harness reports
`[throw] thread constructor failed: Not supported @ 1223 <- 5392 <- 10409`, and the operator state stays
`CSuspensaoAutomaticaEleitor`. The queue backlog above was also re-measured with a `node --import` exit probe
around `tools/run/headless.mjs` (`91001C  C  12C  C  `): `*(1911708)+40..+44` holds 80 bytes = messages
`6, 9, 9, 13, 1`; @1909064 and @1909316 are 0.

## 3. Classes and hierarchy (RTTI)

```
api::CState
└─ comum::CAppState
   ├─ vota::CMostraEleitorVotando          (28 B)  "voter is voting" screen; persists the habilitação
   ├─ vota::CSuspensaoAutomaticaEleitor    (56 B)  10-second countdown, voter-training suspension
   ├─ vota::CControlaReconhecimento        (20 B)  entry of the biometric check; owns its statics
   ├─ vota::CPedeDigital                   (84 B)  fingerprint capture + 1x4 comparison
   ├─ vota::CDigitalReconhecida            (28 B)  recognised  -> release
   ├─ vota::CDigitalNaoReconhecida         (36 B)  not recognised -> retry / birth year
   ├─ vota::CDigitalNaoReconhecidaPorTempo (28 B)  capture timeout
   ├─ vota::CDigitalNaoReconhecidaDecBiometria (20 B) voter's biometric data could not be decrypted
   ├─ vota::CNomeEleitor                   (20 B)  "telaNomeEleitor": confirm the person, choose the path
   ├─ vota::CInformaBioDesabilitadaDemo / CInformaAnoDesabilitadoDemo (20 B)  demo-mode notices
   ├─ vota::CPedeAnoNascimentoSemBiometria (36 B)  birth year for voters without biometrics
   ├─ vota::CVerificaDadoEleitor           (20 B)  birth year after the last failed fingerprint
   ├─ vota::CEleitorDemorando / CEleitorVotouNaoVotou (36 B)  inactivity alert / end of voter
   └─ (other units) CPedeIdentidade, CProcuraEleitor, CEleitorEncontrado, CRegistraDigitalOperador,
      CHabilitaAudioEleitor, CCancelaHabilitacaoEleitor, CSincronismoOperador, CAguardaInspecao, ...

vota::impl::IInformacaoThreadOperador ─ vota::impl::CInformacaoThreadOperador (96 B, default impl)
```

Every state is a lazily created singleton: a static `unique_ptr` plus a mutex. The single-threaded
build keeps only the `mutex_unlock` stub. Exit-time resets are funcs 10414 and 10472. The accessor of a
neighbouring class is often inlined into its caller together with the constructor. For example
`CDigitalReconhecida` and `CDigitalNaoReconhecida` are built inside func 5397, and
`CDigitalNaoReconhecidaPorTempo` inside func 10465. **The build uses LTO**: `CControlaReconhecimento::AvancaProximoDedo1x4`
(ccontrolareconhecimento.cpp:187) is inlined into `CPedeDigital::ProcessTick` (cpededigital.cpp), and the
constructors of `CDigitalReconhecida` / `CDigitalNaoReconhecida` (their own .cpp files) into func 5397. So
inlining does not tell which translation unit a function came from. (TipoToStr, :40, inlined into func 10586
is *not* such evidence: both are in cinformacaothreadoperador.cpp.)

### 3.1 `IInformacaoThreadOperador` (vtable @1590116, 31 slots)

The operator thread's blackboard. States write what they learn and `CMostraEleitorVotando` reads it
back. Slot names are inferred except 23–26 (srcloc).

| slot | method | body | used by |
|---|---|---|---|
| 0/1 | dtor / deleting dtor | 10552 / 10551 | |
| 2 | `LimpaDadosHabilitacao()` photo, habilitação type and manual audio := 0 | 10581 | CCancelaHabilitacaoEleitor |
| 3 | `DeveHabilitarAudio()` = manual audio ∨ `CEleitor::m_necessidadeEspecial == 1` | 10580 | release paths (via thunk 2746) |
| 4, 6 | `GetAudioAtivado()`, `GetAudioHabilitadoManualmente()` (+4) | icf 2683 | CEscolheOpcao, CHabilitaAudioManualmente… |
| 5 | `EleitorNecessitaAudio()` (current voter +20 == 1) | 10579 | CHabilitaAudioEleitor |
| 7 | `SetAudioHabilitadoManualmente(bool)` | 10578 | audio menu |
| 8 | `GetTextoAudio()` → `"ÁUDIO ATIVADO"` or `" "` | 10577 | MT text source |
| 9/10/11 | habilitação type == 2 / 1 / 0 | 10576/10574/10573 | |
| 12/13/14 | set habilitação type 2 / 1 / 0 | 10572/10571/10570 | CRegistraDigitalOperador / CDigitalReconhecida / sem biometria paths |
| 15 | `GetTipoHabilitacao()` (+8) | icf 2587 | SalvaHabilitacaoEleitor |
| 16 | `GetTextoQtdVotaram()` → `"{:04}"` of the RDV comparecimento (voter training) or `CEleitores::m_qtdVotaram` | 10569 | idle MT screen ("0000/0001") |
| 17/18 | `SorteiaProximaInspecao()` / `GetDataHoraProximaInspecao()` (+12) | 10568/10567 | CPedeIdentidade → CAguardaInspecao |
| 19 | `VotacaoBloqueadaPorHorario()` | 10566 | via thunk 2226: CPedeIdentidade StartState / ProcessInput / ProcessTick (every minute: "Votacao foi bloqueada por horario"), CEscolheOpcao::ProcessInput, CIniciaFinalizacao::StartState |
| 20 | `LimpaIdentidades()` | 10565 | CPedeIdentidade::StartState |
| 21/22 | `SetIdentidadeDigitada(string)` / `SetTipoIdentidadeDigitada(tipo)`, then `AtualizaIdentidadeEleitor` (2225) | 10564/10563 | CPedeIdentidade, CValidaIdentidade |
| 23–26 | `GetIdentidadeDigitada` (:474, err 9401) / `GetTipoIdentidadeDigitada` (:484, 9402) / `GetIdentidadeEleitor` (:494, 9403) / `GetIdentidadePrincipalEleitor` (:504, 9404) | 10562–10559 | |
| 27/28/29 | photo: sem foto {0,0} / apresentada {1,0} / erro {2,resultado} | 10558/10557/10556 | `ApresentaFotoEleitor` (3616, u05) |
| 30 | `GetApresentacaoFoto()` → `CApresentacaoFotoEleitor{estado, resultado}` | 10554 | SalvaHabilitacaoEleitor |

Layout (96 bytes, built inside `GetInst`, func 599): +4 manual audio, +8 habilitação type, +12
`CDateTime` of the next inspection, +24 `optional<string>` typed identity, +40 `optional<tipo>`,
+48 `optional<CEleitorIdentidade>` identity, +68 `optional<CEleitorIdentidade>` principal identity,
+88/+92 photo presentation.

`AtualizaIdentidadeEleitor` (2225) runs once both the typed number and its type are known and valid
(`CValidadorIdentidade`, func 2803). It builds the normalised `CEleitorIdentidade` (func 566). If the
type is the configured principal one, that identity is the principal identity. Otherwise it looks the
voter up through the roll's secondary index (`CEleitores` +112) and takes his principal identity. As a
side effect this positions the roll's current item.

**Next inspection** (func 5412): `Now()` + a random 60–90 minutes (`std::mt19937` seeded once from
`std::random_device`, which is `crypto.getRandomValues` under Emscripten). **Voting blocked by time**
(10566): it is never blocked in the training phase. Otherwise it is blocked when
`now ≥ cfg+580` (presumably `HorariosUrna.terminoVotacao`, see §7) and either nobody has voted or
the last vote (`CEstadoGeralVota.dtHrUltimoVoto`) was ≥ 300 s ago.

### 3.2 Static state of the biometric check (`CControlaReconhecimento`)

| address | member (name inferred) | meaning |
|---|---|---|
| @1590924 | `s_tentativa` | current attempt (1-based). `GetTentativa()` truncates it to `uebyte` (the log/format sites use a `load8`; func 10425 uses an `i32.load` masked with 255) |
| @1590928 | `s_verificacoesDadoEleitor` | birth-year checks done (compared with cfg +140 `numTentativasVerificacao`, func 5404) |
| @1590932 | `s_tentativaDigitalSalva` | attempt whose image is kept |
| @1590944 | `DEDOS_1X4 = {1, 6, 2, 7}` | finger order of the 1x4 comparison |
| @1908668 | `s_indiceDedo` | index into DEDOS_1X4. `AvancaTentativa` resets it to 0 |
| @1908672 | `s_digitalCapturada` (`vector<uebyte>`) | the voter's capture (WSQ) |
| @1908684 | `s_digitalMesario` | the mesário's capture (written by CRegistraDigitalOperador) |
| @1908696 | `s_tituloMesario` (`std::string`) | título of the mesário who released the voter |
| @1908708 | `s_score` (uint16) | matcher score of the recognised finger |
| @1908712, @1908716..28 | `s_qtdEleitores`, `s_faixaTentativa[4]` | training simulation: roll size and N/6 |

## 4. Control flow

### 4.1 From the typed identity to the release (real urna)

```
CPedeIdentidade ("Digite o Título ou o CPF")          (u27)
  -> CProcuraEleitor / CEleitorEncontrado / impediments  (u27/u17)
  -> CNomeEleitor ("telaNomeEleitor": name, "Título: XXXX XXXX XXXX", Seq, Seção; photo on the LCD)
       StartState (10501): voter training -> release at once (HabilitaEleitorSemBiometria, 5400)
                           else ApresentaFotoEleitor (3616) + show
       ProcessInput (10500):
         CORRIGE  -> log "Habilitação cancelada durante confirmação de dado do eleitor" -> CCancelaHabilitacaoEleitor
         CONFIRMA -> if ParametrosUrna.pedeAnoNascimentoEleitor (+489):
                        biometria = urna biométrica ∧ CEleitor.possuiBiometria (+100) ∧ record has fingers
                        (training of mesários: the last sixth of the roll counts as "no biometrics")
                        biometria ∧ utilizaBiometria (+665) ? NavegaBiometrica : NavegaAnoNascimento
                     else utilizaBiometria ∧ urna biométrica ? NavegaBiometrica : HabilitaEleitorSemBiometria
         NavegaBiometrica   (:145): demo ? CInformaBioDesabilitadaDemo : CControlaReconhecimento
         NavegaAnoNascimento(:156): demo ? CInformaAnoDesabilitadoDemo : (type := 0) CPedeAnoNascimentoSemBiometria
```

"Release" always follows the same pattern (5400, 10481, 10511): if `DeveHabilitarAudio()` go to
`CHabilitaAudioEleitor` ("Este eleitor necessita de áudio / Coloque o fone de ouvido na urna").
Otherwise post message **1** to the voter thread (`CAguardaMensagem` → `CEleitorVotando`, u06) and
switch to `CMostraEleitorVotando`.

### 4.2 Biometric check

```
CControlaReconhecimento::StartState (10512)   resets the statics of §3.2
   training of mesários: seq in (5N/6, N] -> show "ELEITOR(A) PODE VOTAR / Assinar o caderno de votação antes de votar"
                          else -> CPedeDigital
   otherwise: voter has biometrics with fingers -> CPedeDigital
              else log(2) "O eleitor não possui biometria" + same screen
   ProcessInput (10511): CONFIRMA -> type := SEM_BIOMETRIA (0), release

CPedeDigital (ticks created at construction: 30 000 ms, 15 000 ms, 50 ms)
   StartState (10468): push CApplicationContext(2, "Captura da digital do eleitor",
        "Falha na captura da digital", "Ocorreu um erro durante captura da digital do eleitor.");
        log "Solicita digital. Tentativa [t] de [n]"; throw 9400 if !utilizaBiometria (:115);
        screen "Solicite que o(a) eleitor(a) / <nome> / posicione POLEGAR ou INDICADOR no sensor / CORRIGE: cancelar";
        scanner LED mode 2, start the 3 ticks, scanner start (:125)
   ProcessTick (10465):
      30 s tick on attempt 1, or 15 s tick later  -> stop ticks, CDigitalNaoReconhecidaPorTempo
      50 ms tick: grab frames (IFingerScanner slot 4) until usable:
          frame size != expected                  -> wait for next tick
          urna model 2009/2010/2020/2022          -> first frame is used
          other models: 3 frames each with grey-level entropy > 4 bits; keep the highest-entropy one
      IFingerDetection::DedoPresente(frame) (slot 2) false -> wait
      show "Por favor, aguarde."; log "Capturada a digital. Tentativa [t] de [n]"
      training of mesários -> VerificaDigitalTreinamento (:279), else (in ProcessTick, using the IUrna of :164)
          model <= 2019: InvertColors; VerticalFlip; WSQ-encode -> s_digitalCapturada, s_tentativaDigitalSalva = t;
          then VerificaDigital (:262): extract template; stop the ticks; for each finger from s_indiceDedo (1,6,2,7): stored template (or empty)
             IFingerMatcher::Compara(amostra, modelo, 20) -> {true, GetScore()}; else next finger
      ObtemEstadoPosReconhecimentoBiometrico(reconhecido) (5397, :234):
          LED 1 + CDigitalReconhecida  |  LED 3 + CDigitalNaoReconhecida;  then sleep 1 s
      s_score = score
   ProcessInput (10466): CORRIGE -> log(2) "Habilitação cancelada durante reconhecimento biométrico" -> CCancelaHabilitacaoEleitor
   FinishState (10467): LED 0, scanner stop (:132), pop the application context

CDigitalReconhecida  "ELEITOR(A) RECONHECIDO(A) [Score: n] / Não assinar o caderno de votação / CONFIRMA: prosseguir"
   CONFIRMA (10481): release; type := BIOMETRIA (1); log "Tipo de habilitação do eleitor [biométrica]"
CDigitalNaoReconhecida  "<nome> / Eleitor(a) não reconhecido(a) / Tentativa t de n [Score: n] / CORRIGE: cancelar  CONFIRMA: prosseguir"
   StartState (10474): outside the mesário-training simulation only (func 1823 false), a decryption error of
                       the voter's biometrics (CBiometriaEleitor +36 > 0) -> CDigitalNaoReconhecidaDecBiometria
                       log "Número de tentativas de reconhecimento do dedo. Tentativa [t] de [n]"
   CONFIRMA (10473): last attempt ? CVerificaDadoEleitor (birth year) : AvancaTentativa + CPedeDigital
   CORRIGE: cancel
CDigitalNaoReconhecidaPorTempo  (same choices, 10485; "CONFIRMA: retornar")
CVerificaDadoEleitor (u17) -> ... -> CRegistraDigitalOperador (u27: the mesário's own fingerprint;
   type := 2, s_digitalMesario, s_tituloMesario) -> release
```

`AvancaTentativa` (5406) throws 9397 "Limite de tentativas atingido." if `t == numTentativasHabilitacao`
(the callers test `EhUltimaTentativa` first). It then sets `s_indiceDedo = 0` and increments `t`. Every
attempt therefore restarts the 1x4 walk at the right thumb, and every attempt except the first has only
15 s.

### 4.3 While the voter votes: `CMostraEleitorVotando`

Screen: `<nome> / Título: … Seq: nnnn / Seção: nnnn / VOTANDO PARA: <cargo>` (constructor: func 1150,
u17). The mesário's keys are read and discarded (10426). `ProcessMessage` (10425):

| msg from the voter thread | action |
|---|---|
| 2 EleitorNaoVotou | `CEleitorVotouNaoVotou` with `m_votou = false` |
| 3 / 4 EleitorDemorando sem / com voto | `CEleitorDemorando` (`m_votouParcialmente` 0/1). In voter training it goes on to `CSuspensaoAutomaticaEleitor` |
| 6 EleitorIniciouVotacao | **`SalvaHabilitacaoEleitor()`** (§4.5) |
| 9 AtualizaCargoAtual | copy `CThreadOperador +108` ("VOTANDO PARA: …") and redraw |
| 13 SincronizaVoto | `CSincronismoOperador` (`m_suspensaoAutomatica = false`), cargo text := " " |
| others (1, 5, 7, 8, 10–12) | ignored |

### 4.4 Training of mesários: simulated biometrics

When the urna is in the training phase but **not** in voter-training mode (`vota_f1823`,
`EhTreinamentoSemTreinamentoEleitor`), nothing is captured or compared. `CControlaReconhecimento`
and `CNomeEleitor` split the roll (N voters) by sequential number:

| sequencial | outcome |
|---|---|
| (0, N/6] | recognised at attempt 1 |
| (N/6, 2N/6] | recognised at attempt 2 |
| (2N/6, 3N/6] | recognised at attempt 3 |
| (3N/6, 4N/6] | recognised at attempt 4 |
| (4N/6, 5N/6] | never recognised (birth year / mesário path) |
| (5N/6, N] | treated as having no biometrics |

`VerificaDigitalTreinamento` passes the decision to the matcher (`IFingerMatcher` slot 3 with
`(reconhecido, 20)`) and reads its score back. This gives trainees every screen of the real flow
without fingerprints. The mode is gated only by `CEstadoGeral` fase = training (see §6.5).
The sixths use integer division (`N/6`, then `N/6*5`). In a small section most voters fall into
"no biometrics": with N = 10 the sixth is 1, so voters 6–10 are treated as having no biometrics, and
with N < 6 every voter is. The harness scenario `municipal-t1` has a single voter.

### 4.5 `SalvaHabilitacaoEleitor` — what is recorded when the vote starts

Runs when the voter thread posts message 6. It does nothing in voter-training mode. Otherwise, by
`IInformacaoThreadOperador::GetTipoHabilitacao()`:

| type | images stored (not in the training phase) | `CEleitorDadosHabilitacaoBiometrica` |
|---|---|---|
| 0 SEM_BIOMETRIA | none | dedo 0, score 0, tentativas 0, erro 0 |
| 1 BIOMETRIA | `s_digitalCapturada` → `<trab>/wsq/habilitado/` (throws 9392 "Não há dados salvos de biometria do eleitor." if empty) | dedo = DEDOS_1X4[s_indiceDedo], score = s_score, tentativas = s_tentativaDigitalSalva, erro 0 |
| 2 CODIGO_MESARIO | voter's capture (if any) → `wsq/nao-habilitado/`; mesário's capture → `wsq/operador/` (throws 9393 "Não há dados salvos de biometria do mesário." if empty) | dedo 0, score 0, tentativas = s_tentativa, erro = `CBiometriaEleitor.estadoDecifracao`, título do mesário (type TITULO) if known |

Then it builds `comum::CEleitorDadosHabilitacao(identidade (slot 25), tipo, tipoAtivacaoAudio,
biometria, apresentaçãoFoto (slot 30))` (func 2733) and calls
`CEleitores::MarcaEleitorFoiHabilitado` (func 2825, u04). That writes the voter's `eleitor_dinamico`
row. `tipoAtivacaoAudio` = 0 if the voter-side audio mode is "conforme cadastro", 1 if enabled, 2
otherwise. The images go through `CControlaArmazenamentoDeImagens` (func 2725, u24): at least 5 MiB
free, a unique random name `NNNNNN.wsq` (the prefix passed is empty), encryption with the public key
(`CifrarWsq`), written under both `<trab>` areas (internal flash `fi` and `fe`, `CPath::GetPathTrab(0|1)`).
The code also computes a descriptive file name (`<principal id padded to 12>-TT-DD`, `-TTErr`,
`_Operador`) and then **discards it** (§6.3).

### 4.6 Automatic suspension (voter training)

`CEleitorDemorando::StartState` (10440, u17) switches to `CSuspensaoAutomaticaEleitor` when
`EhTreinamentoEleitor()`. The screen reads `Suspensão automática em N segundos... / Votou parcialmente | Não votou /
CORRIGE: Cancelar`. StartState (10409) makes sure its 1 s tick exists and is stopped (it copies the
thread's tick table into a local `map<id,bool>`). It then writes the status text, sets 10 s, clears the
"sent" flag, formats the countdown text, beeps through `DisparaSinalizacaoSonora` (a detached `std::thread`
whose lambda, 10410 at :89, calls `IScreenMT` slot 6 with `(50, 3)`), and only then starts the tick, shows
the form and stays in the state. Because the beep comes first, the throw of §6.1 skips the tick start and
the form. Each tick (while ≥ 2 s remain) counts down, beeps and redraws. At the last second the state marks the
suspension as sent and posts **3** (SuspensaoAutomatica) to the voter thread
(`CEleitorVotando::suspenderEleitor`, u06). Messages: 5 (the voter typed again) → back to
`CMostraEleitorVotando`. After the suspension, 2 → `CPedeIdentidade` and 13 → `CSincronismoOperador`
with `m_suspensaoAutomatica = true`. CORRIGE before the end logs "Mesário abortou processo de
suspensão", posts 4 (continue) and returns to `CMostraEleitorVotando`.

## 5. Operator-terminal screens built by this unit

MT coordinates are `SPoint{coluna, linha}` (1-based, 40×4). Alignment is 0 left, 1 right, 2 centre.
Several centred texts use x = 1, others x = 20, and right-aligned texts normally use x = 40. The only
`IScreenMT` in this binary, `simulador::CWasmScreenMT::Write` (func 8799, u31), ignores x for those two
alignments: centred texts (≤ 39 chars) start at `(40 − len) / 2`, right-aligned ones at `40 − len`; only
left-aligned texts use `x − 1`. Fields come from template instances
of the MT form builder: `CLedFieldMT` (435), `CBuzzFieldMT` (941), `CBeepFieldMT` (1072),
`CTextFieldMT` with `CFixedText` (180), `CDataText<fn>` (619), `CDataTextFmt<fn>` (651),
`CDataTextFmt<CTextSource>` (1152), `CDataText<CTextSource>` (5409), the input control (395), the
4-digit number input (1151), and the form (301 = `CInteractiveForm<IScreenMT,IInputMT>` with
`CPreShowClearMT`).

| state | LED / sound | lines |
|---|---|---|
| CPedeDigital (captura) | off / buzz(52,5) | (20,1)c "Solicite que o(a) eleitor(a)"; (20,2)c name `{:2}`; (1,3) "posicione POLEGAR ou INDICADOR no sensor"; (1,4) "CORRIGE: cancelar" |
| CPedeDigital (aguarde) | off | (1,2)c "Por favor, aguarde." |
| CDigitalReconhecida | off / buzz(52,5) | (1,1) "ELEITOR(A) RECONHECIDO(A)"; (40,1)r score; (1,2) "Não assinar o caderno de votação"; (40,4)r "CONFIRMA: prosseguir" |
| CDigitalNaoReconhecida | off / beep(1) | (1,1) name; (1,2)c "Eleitor(a) não reconhecido(a)"; (1,3)c "Tentativa t de n"; (1,3)r score; (1,4) "CORRIGE: cancelar"; (40,4)r "CONFIRMA: prosseguir" |
| CDigitalNaoReconhecidaPorTempo | off / beep(1) | name; (20,2)c "Eleitor(a) não reconhecido(a)"; (20,3)c tentativa; "CORRIGE: cancelar"; (40,4)r "CONFIRMA: retornar" |
| CDigitalNaoReconhecidaDecBiometria | off / beep(1) | name; "Eleitor(a) não reconhecido(a)"; (1,3)c "Dados biométricos inválidos"; cancel / prosseguir |
| CVerificaDadoEleitor, CPedeAnoNascimentoSemBiometria | — | name; "Título: …" + "Seq:" + `{:04}`; (1,3) "Digite o ANO de nascimento:" + 4-digit field at (29,3); "CORRIGE: cancelar"; (40,4)r "CONFIRMA: habilitar". The second form of CPedeAno… reads "ANO DE NASCIMENTO INCORRETO" / "CONFIRMA: tentar novamente" |
| CInformaBioDesabilitadaDemo / CInformaAnoDesabilitadoDemo | off | "Biometria desabilitada em demonstração" / "A validação do ano de nascimento" + "é desabilitada em demonstração"; (1,4) "CONFIRMA: prosseguir" (forms named `telaInforma…Demo`) |
| CEleitorVotouNaoVotou | off | (1,1) typed identity; (1,2)/(1,3) status texts; (40,4)r "CONFIRMA: prosseguir" |
| CEleitorDemorando | **on** / buzz(51,10) | "O eleitor está demorando"; status; name; (40,4)r "CONFIRMA" |

The typed-identity text source is func 10586: `std::format("{}: {}", TipoToStr(tipo), formatted)`
where TipoToStr gives "Título" / "CPF" / "Identificador" (else throws 9409) and the number is
displayed as `xxxx xxxx xxxx` (título) or `xxx.xxx.xxx-xx` (CPF) by ecourna_f1924. The harness
recording shows exactly `Título: XXXX XXXX XXXX`.

## 6. Weird or risky code

1. **Automatic suspension throws in this build.** Func 5392 is `DisparaSinalizacaoSonora`, with
   `std::thread`'s constructor inlined. Without pthreads, `pthread_create` is a stub, and the optimizer
   folded the call into an unconditional `__throw_system_error(138 /* ENOTSUP */, "thread constructor
   failed")`. Both callers (StartState 10409 and ProcessTick 10405) would therefore throw. Reproduced
   with the harness: the operator state stays `CSuspensaoAutomaticaEleitor`, no countdown is shown, no
   tick is started and the voter is never suspended. Unreachable in the public simulator, where the
   operator thread does not run. On the urna each second creates a detached thread just to beep.
2. **`emscripten_sleep(1000)`** at the end of `ObtemEstadoPosReconhecimentoBiometrico` (5397). It is
   guarded by the byte @1584624, which is 1 and never written (u06/u07/u08 notes). The glue aborts on
   it (no Asyncify). The call is dead in the web build, and the fingerprint interfaces are not
   registered there anyway (`IFingerScanner`, `IFingerMatcher` and `IFingerDetection` have no
   implementation, so the `CPolySingleton` lookups would throw first).
3. **Descriptive WSQ file names computed and thrown away** (10425). Every branch of
   `SalvaHabilitacaoEleitor` formats a name that ties the image to the voter (`<principal id padded to
   12>-TT-DD`, `-TTErr`, `<id>_Operador`) with `std::format`. It then calls the storage routine with an
   **empty** prefix, so the files get random names (`NNNNNN.wsq`, 6 digits from a random number %
   999999). The mesário-registration path (`CPedeDigitalMesario::GetControlador`, 5382) does the same
   with prefix `"me"`. The link between an image and a voter is kept only in the order of events. If
   someone "fixed" the dead variable by passing it as the prefix, the fingerprint files on the results
   media would name the voter's título. Effect on privacy today: positive. Maintainability: misleading.
4. **Fingerprint pipeline compiled out of the web binary.** The WSQ encoder (stateless singleton,
   func 2742) and the template extractor (parameter singleton func 1903, `{8, 500, …}`) leave no call
   in the non-training branch of `CPedeDigital::ProcessTick` (only the singleton accessors and the
   width/height reads remain). Their results are empty vectors (`shared_f1379` / `vector::clear`). So even
   a harness that registered a fake scanner would store an empty WSQ and compare an empty sample; outside
   the training phase a recognised voter would then make `SalvaHabilitacaoEleitor` throw 9392 (in the
   training phase the simulated path of §4.4 is used and no image is stored). This shows that the
   simulator cannot say anything about the real biometric matching. Info.
5. **Simulated biometrics in the training phase** (§4.4). Recognition is decided by the voter's
   sequential number and the attempt number, and the matcher is told the answer. It is gated only by
   `CEstadoGeral` fase = training (`'3'`) and `treinamentoEleitor = 0`. This is legitimate for mesário
   training, but it is "security that is simulated" in the same binary. Info.
6. **Attempt counter vs database constraint.** `numTentativasHabilitacao` is `INTEGER (1..50)` in
   `ModuloParametrizacaoUrna`, and `s_tentativa` can reach that value. The `eleitor_dinamico` table,
   created by `CEleitorDinamicoDAO` from the DDL string @449289, declares
   `numero_tentativa INTEGER CHECK(numero_tentativa IN (0,1,2,3,4))`. A parametrisation with more than
   4 attempts would make the row update of a voter released at attempt ≥ 5 violate the constraint when
   the dynamic data are written. For a voter released by the mesário (type 2) the stored value is
   `s_tentativa`, which by then equals `numTentativasHabilitacao`, so with that parameter ≥ 5 every such
   voter would hit it. Not verified at runtime (needs the biometric flow). Low. The DDL text was
   checked in the harness database (`samples/bu-real/run-full/fs/dsk/fi/dinamico/trab1/uenux.db`). The same
   DDL has two copy-paste constraints on the wrong column:
   `resultado_decifracao_foto … CHECK(estado_apresentacao_foto IN (0..8))` and
   `tipo_identificador_mesario … CHECK(tipo_identificador IN (1,2,3))`. They are not enforced on the
   intended columns.
7. **`VotacaoBloqueadaPorHorario` can throw inside a periodic tick** (10566). If the roll says
   somebody voted (`CEleitores +104 > 0`) but `CEstadoGeralVota.dtHrUltimoVoto` is empty (inconsistent
   state after a restore), `GetDtHrUltimoVoto` throws 8092 "Nenhum eleitor votou" from
   `CPedeIdentidade`'s one-minute tick. Low.
8. **Inconsistent field coordinates (no visible effect here).** `CDigitalNaoReconhecida` centres
   "Eleitor(a) não reconhecido(a)" at x = 1 and declares both the centred attempt text and the
   right-aligned score at (1,3), where other screens use x = 20 (centre) and x = 40 (right). The only
   MT implementation in this binary, `CWasmScreenMT::Write` (func 8799), ignores x for centred and
   right-aligned text (§5), so the attempt text is centred and the score ends at column 40: they do not
   overlap unless the two texts together exceed about 40 characters. Whether the real MT driver also
   ignores x cannot be checked from this binary. Cosmetic code inconsistency, not a demonstrated bug.

## 7. Data read and written

| data | access |
|---|---|
| `CConfiguracaoEleicao` / `ParametrosUrna` (+88 base, ASN.1 order): +136 `numTentativasHabilitacao`, +140 `numTentativasVerificacao`, +487 `exibirScoreBiometria`, +489 `pedeAnoNascimentoEleitor` | read (attempts, score display, birth-year path) |
| cfg +665 `utilizaBiometria` (ModuloProcessoEleitoral?), +668 principal identity type, +580 4th `HorariosUrna` DataHoraJE (`terminoVotacao`, assuming +544/+556/+568/+580) | read |
| `CEleitores` (roll): current voter, +12 size, +104 `m_qtdVotaram`, +112 secondary index; `CEleitor` +0 sequencial, +20 necessidade especial, +100 possui biometria; `CBiometriaEleitor` dedos (+20/+32) and estado de decifração (+36) | read; **written** by `MarcaEleitorFoiHabilitado` → `eleitor_dinamico` (tipo_habilitacao, dedo_habilitacao, score_habilitacao, numero_tentativa, erro_decifrar_biometria, tipo_ativacao_audio, identidade_habilitacao, apresentação da foto, titulo_mesario) |
| `CEstadoGeral` fase, `CEstadoGeralVota` +72 treinamentoEleitor, dtHrUltimoVoto | read |
| `<trab>/wsq/habilitado/`, `wsq/nao-habilitado/`, `wsq/operador/` in `/dsk/fi` and `/dsk/fe` | written (encrypted WSQ). At the end of the day `CGravadorWSQ` (5821) zips the directories into `wsqbio.jez` / `wsqman.jez` / `wsqmes.jez` for the MR (the mapping of directories to packages is probably in that order) |
| `logd.dat` (CLogVota) | "Solicita digital. Tentativa [t] de [n]", "Capturada a digital…", "Número de tentativas de reconhecimento do dedo…", "Tipo de habilitação do eleitor [biométrica]", "O eleitor não possui biometria" (level 2), "Habilitação cancelada durante …", "Mesário abortou processo de suspensão" |
| voter thread queue | messages 1 (release), 3 (suspend), 4 (continue) |

## 8. Boletim de Urna: what this unit contributes

This unit does not build the BU. It decides the **attendance breakdown by habilitação type** that the
BU, its QR code and the attendance file carry:

1. `SalvaHabilitacaoEleitor` stores `ETipoHabilitacao` for every released voter: 0 = without
   biometrics, 1 = fingerprint recognised, 2 = released by the mesário after failed biometrics
   (ASN.1 `habilitacaoPorCodigo`). With the fingerprint data it stores the finger, score, attempts and
   decryption error, plus the mesário's título.
2. After the vote `CSincronismoOperador` (u17/u20) marks the voter `VOTOU`. At encerramento
   `CEleitores::QtdVotaramPorTipoHabilitacao` (6034, u04) counts voters by type. These counts become
   `ModuloBoletimUrna.DetalhamentoComparecimento` {`qtdEleitoresCompareceramSemBiometria` = type 0,
   `qtdEleitoresHabilitadosPorBiometria` = 1, `qtdEleitoresHabilitadosPorBiografia` = 2} and the QR
   fields `HBSB` / `HBBM` / `HBBG` (`docs/bu/qrcode.md`: `HBBM + HBBG + HBSB = COMP`). The
   harness run confirms type 0 → `qtdEleitoresCompareceramSemBiometria: 1`, `HBSB:1`.
3. The same row feeds `ModuloResultadoUrnaCadastro.EstadoComparecimento` (`habilitacaoBiometrica`
   {tentativasHabilitacaoBiometrica, dedoHabilitado, erroLeituraBiometria, habilitacaoPorCodigo
   {situacaoReconhecimentoMesario, identificacaoMesario}, ultimoScore}, `apresentacaoFoto`,
   `situacaoHabilitacaoAudio`) and the report "ELEITORES HABILITADOS BIOGRAFICAMENTE" printed at
   encerramento (harness `logd.txt`).
4. The fingerprint images saved here travel to the results media (MR) as `wsqbio.jez` / `wsqman.jez` /
   `wsqmes.jez`. The file names are random (§6.3).
5. The encerramento itself (`CEscolheOpcao` → "Todas as pessoas presentes já votaram?" →
   `CPedeTituloEncerramento` → `CConfirmaEncerramento`) belongs to u27. `VotacaoBloqueadaPorHorario`
   (slot 19) is this unit's only link to the closing time.

## 9. Web build specifics

* The operator thread is never run (§2): the whole unit is dead code in the simulator. The voter is
  released by `votaInit` (web mode, voter training), not by these states.
* No fingerprint hardware: only `simulador::CFingerPrepareSimulador` (IFingerPrepare) is registered.
  `IFingerScanner`, `IFingerMatcher` and `IFingerDetection` lookups would throw "PolySingleton -
  solicitada uma instancia nao criada".
* The WSQ encoder and template extractor bodies are empty in this binary (§6.4).
* `std::thread` cannot be created (§6.1). `emscripten_sleep` aborts (§6.2).

## 10. wasm / Emscripten observations

* **LTO cross-file inlining**: srcloc of ccontrolareconhecimento.cpp inside `CPedeDigital::ProcessTick`
  and whole constructors of other classes inside
  `ObtemEstadoPosReconhecimentoBiometrico` (5397), `CNomeEleitor::ProcessInput` (10500, tools name
  `NavegaBiometrica`) and `CDigitalNaoReconhecida::StartState` (10474). The tools therefore
  attributed foreign singletons (1901, 2734, 2735, 5402) to this unit's files.
* **Outlined virtual calls**: many one-line thunks `IInformacaoThreadOperador::GetInst().slotN()`
  (1535, 1687, 2226, 2745, 2746, 3619–3623, 5414–5416, 10583, 10584) are separate functions in the
  binary.
* **Truncating loads**: `CControlaReconhecimento::s_tentativa` is an `int` (written with `i32.store`),
  but log/format sites read it with `i32.load8_u`. This is `static_cast<uebyte>` done as a byte load
  on little-endian wasm.
* **Constant-folded libc**: `std::thread` → unconditional `system_error(138)` (ENOTSUP in Emscripten's
  errno numbering). `std::random_device` + `mt19937` are inlined into func 5412, with the distribution
  bounds stored as the i64 constant `0x5A_0000003C` (60, 90).
* **`@2` addressing in the pseudo-code**: `x[24]:int@2` indexes by 2 bytes (address x+48). The MT
  coordinates (`65556` = `{20,1}`) are 16-bit pairs stored this way.
* Data base symbols: `d_operator0033s…[k]` is address `k + 1024`, and `d_dGGNE4dNci36…[k]` is
  `1581188 + k` (×4 for `:int`).

## 11. Open questions

* Real method names of the fingerprint interfaces (`IFingerScanner` slots 0–7, `IFingerMatcher` 2–4,
  `IFingerDetection` 2, `IUrna` 0). None of them has RTTI or a srcloc in this build. The LED modes 0–3
  are guesses.
* Meaning of the `bool` passed to `CInteractiveForm` (true for most forms, false for the demo notices).
* Which of cfg +544/+556/+568/+580 is which `HorariosUrna` field (the order above is an assumption).
* `CPedeAnoNascimentoSemBiometria` +28/+32 (int 0, bool true) and `CEleitorVotouNaoVotou` +20 (second
  text) are set by methods in other units.
* Exact mapping of `wsq/{habilitado,nao-habilitado,operador}` to `wsq{bio,man,mes}.jez`.

## 12. Mapping table (all 87 functions of u10)

"ran" = observed in the recorded web votes (none). "reconstructed in" is relative to
`src/uenux2/src/app/vota/operador/`. "(comment)" means that the function is described there as a
comment, because it is a library or template instance, a thunk, or an atexit stub.

| func | size | ran | analyzer name | reconstructed symbol | original file | reconstructed in | conf. |
|---:|---:|:-:|---|---|---|---|---|
| 422 | 12 |  | `vota_f422` | `vota::CThreadVota::StopTick` | uenux2/src/app/vota/comum/cthreadvota.cpp (path inferred) | u10-foreign-fragments.cpp (comment) | medium |
| 651 | 265 |  | `api_f651` | `api::CFormBuilderMT::Add<api::CTextFieldMT, api::CDataTextFmt<std::string(*)(const std::string&)>>` | uenux2/src/api/gui/cformbuilder.h (template instance) | u10-foreign-fragments.cpp (comment) | medium |
| 1072 | 19 |  | `vota_f1072` | `api::CFormBuilderMT::Add<api::CBeepFieldMT>` | uenux2/src/api/gui/cformbuilder.h (template instance) | u10-foreign-fragments.cpp (comment) | medium |
| 1152 | 444 |  | `api_f1152` | `api::CFormBuilderMT::Add<api::CTextFieldMT, api::CDataTextFmt<api::CTextSource>>` | uenux2/src/api/gui/cformbuilder.h (template instance) | u10-foreign-fragments.cpp (comment) | medium |
| 1901 | 1197 |  | `vota_f1901` | `vota::CEleitorVotouNaoVotou::GetInst` | uenux2/src/app/vota/operador/aguardaeleitor/celeitorvotounaovotou.cpp (path inferred) | u10-foreign-fragments.cpp | high |
| 2097 | 187 |  | `vota_f2097` | `vota::CLogVota::LogaHabilitacaoCanceladaConfirmacaoDado` | uenux2/src/app/vota/log/clogvota.cpp (path inferred) | confirmaidentidade/cnomeeleitor.cpp (comment) | low |
| 2225 | 757 |  | `vota_f2225` | `vota::impl::CInformacaoThreadOperador::AtualizaIdentidadeEleitor` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | medium |
| 2233 | 222 |  | `api_f2233` | `api::CDateTime::AdicionaSegundos` | uenux2/src/api/util/cdatetime.cpp | u10-foreign-fragments.cpp | medium |
| 2495 | 35 |  | `api_f2495` | `vota::CLogVota::LogaHabilitacaoCanceladaReconhecimento` | uenux2/src/app/vota/log/clogvota.cpp (path inferred) | confirmaidentidade/cpededigital.cpp (call sites) | low |
| 2499 | 57 |  | `api_f2499` | `std::__tree<std::__value_type<K,std::string>>::destroy` | - (library instance) | u10-foreign-fragments.cpp (comment) | medium |
| 2732 | 156 |  | `vota::CSuspensaoAutomaticaEleitor::vf0` | `vota::CSuspensaoAutomaticaEleitor::~CSuspensaoAutomaticaEleitor` | uenux2/src/app/vota/operador/aguardaeleitor/csuspensaoautomaticaeleitor.cpp | aguardaeleitor/csuspensaoautomaticaeleitor.cpp | high |
| 2733 | 217 |  | `vota_f2733` | `comum::CEleitorDadosHabilitacao::CEleitorDadosHabilitacao` | uenux2/src/app/comum/dados/celeitordadoshabilitacao.cpp (path inferred) | u10-foreign-fragments.cpp | medium |
| 2734 | 1228 |  | `vota_f2734` | `vota::CEleitorDemorando::GetInst` | uenux2/src/app/vota/operador/aguardaeleitor/celeitordemorando.cpp (path inferred) | u10-foreign-fragments.cpp | high |
| 2735 | 1413 |  | `api_f2735` | `vota::CVerificaDadoEleitor::GetInst` | uenux2/src/app/vota/operador/confirmaidentidade/cverificadadoeleitor.cpp (path inferred) | u10-foreign-fragments.cpp | high |
| 2739 | 115 |  | `vota::CPedeDigital::vf0` | `vota::CPedeDigital::~CPedeDigital` | uenux2/src/app/vota/operador/confirmaidentidade/cpededigital.cpp | confirmaidentidade/cpededigital.cpp | high |
| 2740 | 1815 |  | `vota_f2740` | `vota::CPedeDigital::GetInst` | uenux2/src/app/vota/operador/confirmaidentidade/cpededigital.cpp | confirmaidentidade/cpededigital.cpp | high |
| 2745 | 20 |  | `vota_f2745` | `vota::impl::IInformacaoThreadOperador::GetApresentacaoFoto (outlined call)` | uenux2/src/app/vota/operador/aguardaeleitor/cmostraeleitorvotando.cpp | u10-foreign-fragments.cpp (comment) | medium |
| 3078 | 10 |  | `vota_f3078` | `vota::CInformacaoEleitor::AudioHabilitado` | uenux2/src/app/vota/eleitor/comum/cinformacaoeleitor.h (inline) | u10-foreign-fragments.cpp (comment) | low |
| 3612 | 373 |  | `vota_f3612` | `vota::CSuspensaoAutomaticaEleitor::AtualizaTextoContagem` | uenux2/src/app/vota/operador/aguardaeleitor/csuspensaoautomaticaeleitor.cpp | aguardaeleitor/csuspensaoautomaticaeleitor.cpp | medium |
| 3613 | 32 |  | `vota_f3613` | `std::__tree<std::__value_type<uebyte,bool>>::destroy` | - (library instance) | aguardaeleitor/csuspensaoautomaticaeleitor.cpp (comment) | medium |
| 3621 | 20 |  | `api_f3621` | `vota::impl::IInformacaoThreadOperador::SetHabilitacaoSemBiometria (outlined call)` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | u10-foreign-fragments.cpp (comment) | medium |
| 3718 | 44 |  | `vota_f3718` | `comum::md::CEleitorDadosHabilitacaoBiometrica::CEleitorDadosHabilitacaoBiometrica` | uenux2/src/app/comum/dados/md/celeitordadoshabilitacaobiometrica.cpp (path inferred) | u10-foreign-fragments.cpp | medium |
| 4535 | 150 |  | `api_f4535` | `vota::CLogVota::LogaMesarioAbortouSuspensao` | uenux2/src/app/vota/log/clogvota.cpp (path inferred) | aguardaeleitor/csuspensaoautomaticaeleitor.cpp (comment) | low |
| 5392 | 42 |  | `vota_f5392` | `vota::CSuspensaoAutomaticaEleitor::DisparaSinalizacaoSonora` | uenux2/src/app/vota/operador/aguardaeleitor/csuspensaoautomaticaeleitor.cpp | aguardaeleitor/csuspensaoautomaticaeleitor.cpp | high |
| 5397 | 2997 |  | `vota::CPedeDigital::ObtemEstadoPosReconhecimentoBiometrico` | `vota::CPedeDigital::ObtemEstadoPosReconhecimentoBiometrico` | uenux2/src/app/vota/operador/confirmaidentidade/cpededigital.cpp | confirmaidentidade/cpededigital.cpp | high |
| 5400 | 82 |  | `vota_f5400` | `vota::CNomeEleitor::HabilitaEleitorSemBiometria` | uenux2/src/app/vota/operador/confirmaidentidade/cnomeeleitor.cpp | confirmaidentidade/cnomeeleitor.cpp | low |
| 5402 | 754 |  | `vota_f5402` | `vota::CInformaBioDesabilitadaDemo::GetInst` | uenux2/src/app/vota/operador/confirmaidentidade/cinformabiodesabilitadademo.cpp (path inferred) | u10-foreign-fragments.cpp | high |
| 5403 | 13 |  | `vota_f5403` | `vota::CControlaReconhecimento::EhPrimeiraTentativa` | uenux2/src/app/vota/operador/confirmaidentidade/ccontrolareconhecimento.h (inline) | confirmaidentidade/ccontrolareconhecimento.h | medium |
| 5405 | 18 |  | `api_f5405` | `vota::CControlaReconhecimento::EhUltimaTentativa` | uenux2/src/app/vota/operador/confirmaidentidade/ccontrolareconhecimento.cpp | confirmaidentidade/ccontrolareconhecimento.cpp | medium |
| 5406 | 105 |  | `vota::CControlaReconhecimento::AvancaTentativa` | `vota::CControlaReconhecimento::AvancaTentativa` | uenux2/src/app/vota/operador/confirmaidentidade/ccontrolareconhecimento.cpp | confirmaidentidade/ccontrolareconhecimento.cpp | high |
| 5412 | 766 |  | `api_f5412` | `vota::(anonymous namespace)::CalculaHorarioProximaInspecao` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | medium |
| 10404 | 103 |  | `vota::CSuspensaoAutomaticaEleitor::vf6` | `vota::CSuspensaoAutomaticaEleitor::ProcessMessage` | uenux2/src/app/vota/operador/aguardaeleitor/csuspensaoautomaticaeleitor.cpp | aguardaeleitor/csuspensaoautomaticaeleitor.cpp | medium |
| 10405 | 173 |  | `vota::CSuspensaoAutomaticaEleitor::ProcessTick` | `vota::CSuspensaoAutomaticaEleitor::ProcessTick` | uenux2/src/app/vota/operador/aguardaeleitor/csuspensaoautomaticaeleitor.cpp | aguardaeleitor/csuspensaoautomaticaeleitor.cpp | high |
| 10407 | 191 |  | `vota::CSuspensaoAutomaticaEleitor::vf7` | `vota::CSuspensaoAutomaticaEleitor::ProcessInput` | uenux2/src/app/vota/operador/aguardaeleitor/csuspensaoautomaticaeleitor.cpp | aguardaeleitor/csuspensaoautomaticaeleitor.cpp | medium |
| 10408 | 20 |  | `vota::CSuspensaoAutomaticaEleitor::vf5` | `vota::CSuspensaoAutomaticaEleitor::FinishState` | uenux2/src/app/vota/operador/aguardaeleitor/csuspensaoautomaticaeleitor.cpp | aguardaeleitor/csuspensaoautomaticaeleitor.cpp | medium |
| 10409 | 537 |  | `vota::CSuspensaoAutomaticaEleitor::vf2` | `vota::CSuspensaoAutomaticaEleitor::StartState` | uenux2/src/app/vota/operador/aguardaeleitor/csuspensaoautomaticaeleitor.cpp | aguardaeleitor/csuspensaoautomaticaeleitor.cpp | medium |
| 10410 | 126 |  | `vota::CSuspensaoAutomaticaEleitor::DisparaSinalizacaoSonora()::(lambda)::operator()` | `vota::CSuspensaoAutomaticaEleitor::DisparaSinalizacaoSonora()::$_0::operator()` | uenux2/src/app/vota/operador/aguardaeleitor/csuspensaoautomaticaeleitor.cpp | aguardaeleitor/csuspensaoautomaticaeleitor.cpp | high |
| 10411 | 13 |  | `vota::CSuspensaoAutomaticaEleitor::vf1` | `vota::CSuspensaoAutomaticaEleitor::~CSuspensaoAutomaticaEleitor (deleting)` | uenux2/src/app/vota/operador/aguardaeleitor/csuspensaoautomaticaeleitor.cpp | aguardaeleitor/csuspensaoautomaticaeleitor.cpp | high |
| 10414 | 38 |  | `vota_f10414` | `vota::CSuspensaoAutomaticaEleitor::GetInst()::s_inst (atexit destructor)` | uenux2/src/app/vota/operador/aguardaeleitor/csuspensaoautomaticaeleitor.cpp | aguardaeleitor/csuspensaoautomaticaeleitor.cpp (comment) | medium |
| 10425 | 4809 |  | `vota::CMostraEleitorVotando::SalvaHabilitacaoEleitor` | `vota::CMostraEleitorVotando::ProcessMessage` | uenux2/src/app/vota/operador/aguardaeleitor/cmostraeleitorvotando.cpp | aguardaeleitor/cmostraeleitorvotando.cpp | high |
| 10426 | 155 |  | `vota::CMostraEleitorVotando::ProcessInput` | `vota::CMostraEleitorVotando::ProcessInput` | uenux2/src/app/vota/operador/aguardaeleitor/cmostraeleitorvotando.cpp | aguardaeleitor/cmostraeleitorvotando.cpp | high |
| 10427 | 158 |  | `vota::CMostraEleitorVotando::vf2` | `vota::CMostraEleitorVotando::StartState` | uenux2/src/app/vota/operador/aguardaeleitor/cmostraeleitorvotando.cpp | aguardaeleitor/cmostraeleitorvotando.cpp | medium |
| 10465 | 4463 |  | `vota::CPedeDigital::ProcessTick` | `vota::CPedeDigital::ProcessTick` | uenux2/src/app/vota/operador/confirmaidentidade/cpededigital.cpp | confirmaidentidade/cpededigital.cpp | high |
| 10466 | 136 |  | `vota::CPedeDigital::vf7` | `vota::CPedeDigital::ProcessInput` | uenux2/src/app/vota/operador/confirmaidentidade/cpededigital.cpp | confirmaidentidade/cpededigital.cpp | medium |
| 10467 | 114 |  | `vota::CPedeDigital::FinishState` | `vota::CPedeDigital::FinishState` | uenux2/src/app/vota/operador/confirmaidentidade/cpededigital.cpp | confirmaidentidade/cpededigital.cpp | high |
| 10468 | 851 |  | `vota::CPedeDigital::StartState` | `vota::CPedeDigital::StartState` | uenux2/src/app/vota/operador/confirmaidentidade/cpededigital.cpp | confirmaidentidade/cpededigital.cpp | high |
| 10469 | 13 |  | `vota::CPedeDigital::vf1` | `vota::CPedeDigital::~CPedeDigital (deleting)` | uenux2/src/app/vota/operador/confirmaidentidade/cpededigital.cpp | confirmaidentidade/cpededigital.cpp | high |
| 10472 | 38 |  | `vota_f10472` | `vota::CPedeDigital::GetInst()::s_inst (atexit destructor)` | uenux2/src/app/vota/operador/confirmaidentidade/cpededigital.cpp | confirmaidentidade/cpededigital.cpp (comment) | medium |
| 10473 | 172 |  | `vota::CDigitalNaoReconhecida::vf7` | `vota::CDigitalNaoReconhecida::ProcessInput` | uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaoreconhecida.cpp | confirmaidentidade/cdigitalnaoreconhecida.cpp | medium |
| 10474 | 2650 |  | `vota::CDigitalNaoReconhecida::StartState` | `vota::CDigitalNaoReconhecida::StartState` | uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaoreconhecida.cpp | confirmaidentidade/cdigitalnaoreconhecida.cpp | high |
| 10481 | 926 |  | `vota::CDigitalReconhecida::vf7` | `vota::CDigitalReconhecida::ProcessInput` | uenux2/src/app/vota/operador/confirmaidentidade/cdigitalreconhecida.cpp | confirmaidentidade/cdigitalreconhecida.cpp | medium |
| 10482 | 496 |  | `vota::CDigitalReconhecida::StartState` | `vota::CDigitalReconhecida::StartState` | uenux2/src/app/vota/operador/confirmaidentidade/cdigitalreconhecida.cpp | confirmaidentidade/cdigitalreconhecida.cpp | high |
| 10485 | 172 |  | `vota::CDigitalNaoReconhecidaPorTempo::vf7` | `vota::CDigitalNaoReconhecidaPorTempo::ProcessInput` | uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaoreconhecidaportempo.cpp (path inferred) | u10-foreign-fragments.cpp | medium |
| 10500 | 3978 |  | `vota::CNomeEleitor::NavegaBiometrica` | `vota::CNomeEleitor::ProcessInput` | uenux2/src/app/vota/operador/confirmaidentidade/cnomeeleitor.cpp | confirmaidentidade/cnomeeleitor.cpp | high |
| 10501 | 44 |  | `vota::CNomeEleitor::vf2` | `vota::CNomeEleitor::StartState` | uenux2/src/app/vota/operador/confirmaidentidade/cnomeeleitor.cpp | confirmaidentidade/cnomeeleitor.cpp | medium |
| 10511 | 179 |  | `vota::CControlaReconhecimento::vf7` | `vota::CControlaReconhecimento::ProcessInput` | uenux2/src/app/vota/operador/confirmaidentidade/ccontrolareconhecimento.cpp | confirmaidentidade/ccontrolareconhecimento.cpp | medium |
| 10512 | 514 |  | `vota::CControlaReconhecimento::vf2` | `vota::CControlaReconhecimento::StartState` | uenux2/src/app/vota/operador/confirmaidentidade/ccontrolareconhecimento.cpp | confirmaidentidade/ccontrolareconhecimento.cpp | medium |
| 10551 | 118 |  | `vota::impl::CInformacaoThreadOperador::vf1` | `vota::impl::CInformacaoThreadOperador::~CInformacaoThreadOperador (deleting)` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | high |
| 10552 | 115 |  | `vota::impl::CInformacaoThreadOperador::vf0` | `vota::impl::CInformacaoThreadOperador::~CInformacaoThreadOperador` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | high |
| 10554 | 18 |  | `vota::impl::CInformacaoThreadOperador::vf30` | `vota::impl::CInformacaoThreadOperador::GetApresentacaoFoto` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | medium |
| 10556 | 16 |  | `vota::impl::CInformacaoThreadOperador::vf29` | `vota::impl::CInformacaoThreadOperador::SetFotoNaoApresentadaPorErro` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | medium |
| 10557 | 9 |  | `vota::impl::CInformacaoThreadOperador::vf28` | `vota::impl::CInformacaoThreadOperador::SetFotoApresentada` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | medium |
| 10558 | 9 |  | `vota::impl::CInformacaoThreadOperador::vf27` | `vota::impl::CInformacaoThreadOperador::SetEleitorSemFoto` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | medium |
| 10559 | 130 |  | `vota::impl::CInformacaoThreadOperador::GetIdentidadePrincipalEleitor` | `vota::impl::CInformacaoThreadOperador::GetIdentidadePrincipalEleitor` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | high |
| 10560 | 130 |  | `vota::impl::CInformacaoThreadOperador::GetIdentidadeEleitor` | `vota::impl::CInformacaoThreadOperador::GetIdentidadeEleitor` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | high |
| 10561 | 74 |  | `vota::impl::CInformacaoThreadOperador::GetTipoIdentidadeDigitada` | `vota::impl::CInformacaoThreadOperador::GetTipoIdentidadeDigitada` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | high |
| 10562 | 120 |  | `vota::impl::CInformacaoThreadOperador::GetIdentidadeDigitada` | `vota::impl::CInformacaoThreadOperador::GetIdentidadeDigitada` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | high |
| 10563 | 21 |  | `vota::impl::CInformacaoThreadOperador::vf22` | `vota::impl::CInformacaoThreadOperador::SetTipoIdentidadeDigitada` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | medium |
| 10564 | 206 |  | `vota::impl::CInformacaoThreadOperador::vf21` | `vota::impl::CInformacaoThreadOperador::SetIdentidadeDigitada` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | medium |
| 10565 | 149 |  | `vota::impl::CInformacaoThreadOperador::vf20` | `vota::impl::CInformacaoThreadOperador::LimpaIdentidades` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | medium |
| 10566 | 179 |  | `vota::impl::CInformacaoThreadOperador::vf19` | `vota::impl::CInformacaoThreadOperador::VotacaoBloqueadaPorHorario` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | medium |
| 10567 | 22 |  | `vota::impl::CInformacaoThreadOperador::vf18` | `vota::impl::CInformacaoThreadOperador::GetDataHoraProximaInspecao` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | medium |
| 10568 | 48 |  | `vota::impl::CInformacaoThreadOperador::vf17` | `vota::impl::CInformacaoThreadOperador::SorteiaProximaInspecao` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | medium |
| 10569 | 380 |  | `vota::impl::CInformacaoThreadOperador::vf16` | `vota::impl::CInformacaoThreadOperador::GetTextoQtdVotaram` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | medium |
| 10570 | 9 |  | `vota::impl::CInformacaoThreadOperador::vf14` | `vota::impl::CInformacaoThreadOperador::SetHabilitacaoSemBiometria` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | medium |
| 10571 | 9 |  | `vota::impl::CInformacaoThreadOperador::vf13` | `vota::impl::CInformacaoThreadOperador::SetHabilitacaoBiometrica` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | medium |
| 10572 | 9 |  | `vota::impl::CInformacaoThreadOperador::vf12` | `vota::impl::CInformacaoThreadOperador::SetHabilitacaoCodigoMesario` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | medium |
| 10573 | 8 |  | `vota::impl::CInformacaoThreadOperador::vf11` | `vota::impl::CInformacaoThreadOperador::HabilitadoSemBiometria` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | low |
| 10574 | 10 |  | `vota::impl::CInformacaoThreadOperador::vf10` | `vota::impl::CInformacaoThreadOperador::HabilitadoPorBiometria` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | low |
| 10576 | 10 |  | `vota::impl::CInformacaoThreadOperador::vf9` | `vota::impl::CInformacaoThreadOperador::HabilitadoPorCodigoMesario` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | low |
| 10577 | 102 |  | `vota::impl::CInformacaoThreadOperador::vf8` | `vota::impl::CInformacaoThreadOperador::GetTextoAudio` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | medium |
| 10578 | 9 |  | `vota::impl::CInformacaoThreadOperador::vf7` | `vota::impl::CInformacaoThreadOperador::SetAudioHabilitadoManualmente` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | medium |
| 10579 | 17 |  | `vota::impl::CInformacaoThreadOperador::vf5` | `vota::impl::CInformacaoThreadOperador::EleitorNecessitaAudio` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | medium |
| 10580 | 34 |  | `vota::impl::CInformacaoThreadOperador::vf3` | `vota::impl::CInformacaoThreadOperador::DeveHabilitarAudio` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | medium |
| 10581 | 23 |  | `vota::impl::CInformacaoThreadOperador::vf2` | `vota::impl::CInformacaoThreadOperador::LimpaDadosHabilitacao` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | medium |
| 10586 | 837 |  | `vota::TipoToStr` | `vota::TextoIdentidadeDigitada` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | medium |
| 13471 | 18 |  | `api_f13471` | `api::getResourceMovie()::s_filmes (atexit destructor)` | uenux2/src/api/gui/iresource.h | u10-foreign-fragments.cpp (comment) | medium |
