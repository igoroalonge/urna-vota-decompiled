# u10 — `uenux2/src/app/vota/operador`: identificação e habilitação do eleitor no terminal do mesário

A unidade u10 cobre 87 funções wasm do **lado do operador** da aplicação VOTA: os
estados que executam no *terminal do mesário* enquanto um eleitor é identificado, tem sua digital verificada
e é habilitado a votar, o estado mostrado enquanto ele vota, a suspensão automática do
modo de treinamento de eleitores, e o quadro-negro compartilhado da thread do operador, `IInformacaoThreadOperador`.
**Nenhuma das 87 funções executou durante os votos web gravados**: o simulador público nunca executa a
thread do operador (§2). Em vez disso, o fluxo foi verificado com o harness de wasm modificado do projeto,
`tools/bu/operator_harness.mjs` (sua execução gravada `samples/bu-real/run-full`, mais uma execução feita para esta
unidade, §2; o próprio wasm modificado é `work/bu-real/vota_web_wasm.opstep.wasm`).

Arquivos-fonte reconstruídos (`src/uenux2/src/app/vota/operador/`):
`comum/iinformacaothreadoperador.h`, `comum/cinformacaothreadoperador.{h,cpp}`,
`aguardaeleitor/cmostraeleitorvotando.{h,cpp}`, `aguardaeleitor/csuspensaoautomaticaeleitor.{h,cpp}`,
`confirmaidentidade/ccontrolareconhecimento.{h,cpp}`, `confirmaidentidade/cpededigital.{h,cpp}`,
`confirmaidentidade/cdigitalreconhecida.{h,cpp}`, `confirmaidentidade/cdigitalnaoreconhecida.{h,cpp}`,
`confirmaidentidade/cnomeeleitor.{h,cpp}` e `u10-foreign-fragments.cpp`. O arquivo de fragmentos contém
as funções que as ferramentas colocaram nesta unidade mas que pertencem a outros arquivos: acessores de singleton de
estados vizinhos, um método de estado, construtores de dados, thunks e instâncias de template do construtor de formulários.

## 1. Glossário

| termo | significado |
|---|---|
| mesário / operador | quem opera o *terminal do mesário* ("micro-terminal", **MT**: LCD 4×40, teclado, LED, buzzer, sensor de digital) |
| eleitor | a pessoa que vota |
| habilitar / habilitação | liberar a urna para um eleitor identificado. A urna registra **como** ele foi habilitado (`md::ETipoHabilitacao`) |
| título (de eleitor) / CPF | número do título (12 dígitos) / número do CPF (11 dígitos); "identificador livre" é o terceiro tipo de identidade |
| identidade principal | o tipo de identidade que serve de chave do cadastro da seção (`CConfiguracaoEleicao` +668) |
| biometria / digital / dedo | biometria por impressão digital / uma impressão digital / um dedo |
| reconhecimento 1x4 | a digital capturada é comparada com o template armazenado de até 4 dedos: polegar direito, polegar esquerdo, indicador direito, indicador esquerdo (códigos de dedo 1, 6, 2, 7) |
| tentativa | tentativa de captura, limitada por `ParametrosUrna.numTentativasHabilitacao` |
| ano de nascimento | a verificação biográfica usada quando a biometria está ausente ou falha |
| caderno de votação | o caderno de papel que os eleitores assinam. Um eleitor reconhecido pela biometria é instruído a **não** assiná-lo |
| WSQ | Wavelet Scalar Quantization, o formato de imagem de impressão digital do FBI; a urna guarda imagens WSQ cifradas das capturas |
| cabina | cabine de votação ("CABINA: LIVRE / OCUPADA" no MT) |
| treinamento de eleitores | modo de treinamento de eleitores de uma urna em fase de treinamento (`CEstadoGeralVota.treinamentoEleitor`, +72). O simulador web executa neste modo |
| treinamento (de mesários) | fase de treinamento sem treinamento de eleitores: o fluxo biométrico é **simulado** (§4.4) |
| demonstração | modo de demonstração (`IInterfaceInit::GetDemoMode`): a biometria e a verificação do ano de nascimento são substituídas por avisos |
| suspensão | terminar a sessão de um eleitor que não conclui o voto (u06 §4.7) |
| inspeção | inspeção da cabina que o mesário é solicitado a fazer em intervalos aleatórios |

## 2. Onde este código executa (e por que nunca executou no simulador web)

O VOTA tem duas threads de máquina de estados (u06 §2): a thread do eleitor `CThreadEleitor` e a thread do operador
`CThreadOperador` (singleton de 132 bytes em @1911708, construído pela func 270; as ferramentas chamam essa
função de `CPriorityMessageQueue<SMessage>::CPriorityMessageQueue@270`). A thread do operador possui uma
tabela de ticks `CThreadVota` em +20, a fila de mensagens `vota::CMessageOperador` em +36 e o texto do
cargo em votação em +108. Seus estados são singletons `comum::CAppState`, com o protocolo de slots de
u06 §2 (2 StartState, 3 NeedChangeState, 4 GetNextState, 5 FinishState, 6 ProcessMessage,
7 ProcessInput, 8 ProcessTick).

**Build web.** `main` registra `CExecucaoVotaCooperativa`, e `votaTick` só faz avançar
`CThreadEleitor` (u06/u07). `CThreadOperador::Run` (func 10204) nunca é chamada. Os estados do eleitor
ainda postam suas mensagens ao operador (6 no início, 9 por cargo, 13 e 1 no fim). Elas ficam
sem leitura na fila de `CThreadOperador`: depois de um voto municipal a fila continha 5 entradas (16 bytes cada)
`6, 9, 9, 13, 1`, e os singletons de `CPedeIdentidade`, `CPedeDigital` e `CMostraEleitorVotando`
nunca foram criados (sonda headless lendo @1911708/@1909064/@1909316 na saída). A página é recarregada para
cada eleitor, então o acúmulo continua pequeno.

**Evidência do harness.** `tools/bu/operator_harness.mjs` acrescenta um export `opStep` (uma iteração de
`CThreadOperador::Run`) a uma cópia do wasm e conduz as duas threads. Sua execução "full" gravada
(`samples/bu-real/run-full/states.txt`, `mt.txt`) passa pelo código desta unidade:
`CPedeIdentidade → CProcuraEleitor → CEleitorEncontrado → CNomeEleitor → CPedeAnoNascimentoSemBiometria
→ CInformaEleitorPodeVotar → CMostraEleitorVotando (VOTANDO PARA: Vereador / Prefeito) →
CSincronismoOperador`. A linha que ela deixou em `eleitor_dinamico` (`tipo_habilitacao` 0,
`tipo_ativacao_audio` 2) e o BU (`detalhamentoComparecimento.qtdEleitoresCompareceramSemBiometria = 1`,
QR `HBSB:1`) conferem com `SalvaHabilitacaoEleitor` (§4.5). Uma segunda execução feita para esta unidade
(`--script "geradin zeresima ot:20 om:8 ot:50 o:XXXXXXXXXXXXC ot:50 o:C ot:50 o:AAAAC ot:50 o:C ot:50
treino:1 t:20 clock:2026-10-04T16=59=00-03=00 t:60 t:60"`) definiu `treinamentoEleitor = 1` depois da
habilitação e adiantou o relógio falso em 9 minutos. Ela chegou a
`CSuspensaoAutomaticaEleitor::StartState` e morreu com `thread constructor failed: Not supported`
vindo da func 5392 (§6.1). Reexecutada pela revisão de fidelidade (2026-09-23): o harness reporta
`[throw] thread constructor failed: Not supported @ 1223 <- 5392 <- 10409`, e o estado do operador continua
`CSuspensaoAutomaticaEleitor`. O acúmulo da fila acima também foi medido de novo com uma sonda de saída `node --import`
em torno de `tools/run/headless.mjs` (`91001C  C  12C  C  `): `*(1911708)+40..+44` contém 80 bytes = mensagens
`6, 9, 9, 13, 1`; @1909064 e @1909316 são 0.

## 3. Classes e hierarquia (RTTI)

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

Todo estado é um singleton criado de forma preguiçosa: um `unique_ptr` estático mais um mutex. O build
single-threaded mantém apenas o stub `mutex_unlock`. As redefinições na saída são as funcs 10414 e 10472. O acessor de uma
classe vizinha costuma ser inlinado no seu chamador junto com o construtor. Por exemplo,
`CDigitalReconhecida` e `CDigitalNaoReconhecida` são construídos dentro da func 5397, e
`CDigitalNaoReconhecidaPorTempo` dentro da func 10465. **O build usa LTO**: `CControlaReconhecimento::AvancaProximoDedo1x4`
(ccontrolareconhecimento.cpp:187) é inlinado em `CPedeDigital::ProcessTick` (cpededigital.cpp), e os
construtores de `CDigitalReconhecida` / `CDigitalNaoReconhecida` (de seus próprios arquivos .cpp) na func 5397. Portanto
o inlining não diz de qual unidade de tradução uma função veio. (TipoToStr, :40, inlinada na func 10586,
*não* é uma evidência desse tipo: as duas estão em cinformacaothreadoperador.cpp.)

### 3.1 `IInformacaoThreadOperador` (vtable @1590116, 31 slots)

O quadro-negro da thread do operador. Os estados gravam o que descobrem e `CMostraEleitorVotando` lê
de volta. Os nomes dos slots são inferidos, exceto 23–26 (srcloc).

| slot | método | corpo | usado por |
|---|---|---|---|
| 0/1 | dtor / dtor de exclusão | 10552 / 10551 | |
| 2 | `LimpaDadosHabilitacao()` foto, tipo de habilitação e áudio manual := 0 | 10581 | CCancelaHabilitacaoEleitor |
| 3 | `DeveHabilitarAudio()` = áudio manual ∨ `CEleitor::m_necessidadeEspecial == 1` | 10580 | caminhos de habilitação (via thunk 2746) |
| 4, 6 | `GetAudioAtivado()`, `GetAudioHabilitadoManualmente()` (+4) | icf 2683 | CEscolheOpcao, CHabilitaAudioManualmente… |
| 5 | `EleitorNecessitaAudio()` (eleitor atual +20 == 1) | 10579 | CHabilitaAudioEleitor |
| 7 | `SetAudioHabilitadoManualmente(bool)` | 10578 | menu de áudio |
| 8 | `GetTextoAudio()` → `"ÁUDIO ATIVADO"` ou `" "` | 10577 | fonte de texto do MT |
| 9/10/11 | tipo de habilitação == 2 / 1 / 0 | 10576/10574/10573 | |
| 12/13/14 | define tipo de habilitação 2 / 1 / 0 | 10572/10571/10570 | CRegistraDigitalOperador / CDigitalReconhecida / caminhos sem biometria |
| 15 | `GetTipoHabilitacao()` (+8) | icf 2587 | SalvaHabilitacaoEleitor |
| 16 | `GetTextoQtdVotaram()` → `"{:04}"` do comparecimento do RDV (treinamento de eleitores) ou `CEleitores::m_qtdVotaram` | 10569 | tela ociosa do MT ("0000/0001") |
| 17/18 | `SorteiaProximaInspecao()` / `GetDataHoraProximaInspecao()` (+12) | 10568/10567 | CPedeIdentidade → CAguardaInspecao |
| 19 | `VotacaoBloqueadaPorHorario()` | 10566 | via thunk 2226: CPedeIdentidade StartState / ProcessInput / ProcessTick (a cada minuto: "Votacao foi bloqueada por horario"), CEscolheOpcao::ProcessInput, CIniciaFinalizacao::StartState |
| 20 | `LimpaIdentidades()` | 10565 | CPedeIdentidade::StartState |
| 21/22 | `SetIdentidadeDigitada(string)` / `SetTipoIdentidadeDigitada(tipo)`, depois `AtualizaIdentidadeEleitor` (2225) | 10564/10563 | CPedeIdentidade, CValidaIdentidade |
| 23–26 | `GetIdentidadeDigitada` (:474, err 9401) / `GetTipoIdentidadeDigitada` (:484, 9402) / `GetIdentidadeEleitor` (:494, 9403) / `GetIdentidadePrincipalEleitor` (:504, 9404) | 10562–10559 | |
| 27/28/29 | foto: sem foto {0,0} / apresentada {1,0} / erro {2,resultado} | 10558/10557/10556 | `ApresentaFotoEleitor` (3616, u05) |
| 30 | `GetApresentacaoFoto()` → `CApresentacaoFotoEleitor{estado, resultado}` | 10554 | SalvaHabilitacaoEleitor |

Layout (96 bytes, construído dentro de `GetInst`, func 599): +4 áudio manual, +8 tipo de habilitação, +12
`CDateTime` da próxima inspeção, +24 `optional<string>` identidade digitada, +40 `optional<tipo>`,
+48 `optional<CEleitorIdentidade>` identidade, +68 `optional<CEleitorIdentidade>` identidade principal,
+88/+92 apresentação da foto.

`AtualizaIdentidadeEleitor` (2225) executa quando o número digitado e o seu tipo são ambos conhecidos e válidos
(`CValidadorIdentidade`, func 2803). Ela monta o `CEleitorIdentidade` normalizado (func 566). Se o
tipo é o tipo principal configurado, essa identidade é a identidade principal. Caso contrário, ela procura o
eleitor pelo índice secundário do cadastro (`CEleitores` +112) e pega a identidade principal dele. Como
efeito colateral, isso posiciona o item atual do cadastro.

**Próxima inspeção** (func 5412): `Now()` + de 60 a 90 minutos aleatórios (`std::mt19937` semeado uma vez a partir de
`std::random_device`, que é `crypto.getRandomValues` sob o Emscripten). **Votação bloqueada por horário**
(10566): nunca fica bloqueada na fase de treinamento. Caso contrário, fica bloqueada quando
`now ≥ cfg+580` (presumivelmente `HorariosUrna.terminoVotacao`, ver §7) e ou ninguém votou ou
o último voto (`CEstadoGeralVota.dtHrUltimoVoto`) foi há ≥ 300 s.

### 3.2 Estado estático da verificação biométrica (`CControlaReconhecimento`)

| endereço | membro (nome inferido) | significado |
|---|---|---|
| @1590924 | `s_tentativa` | tentativa atual (começando em 1). `GetTentativa()` a trunca para `uebyte` (os pontos de log/formatação usam um `load8`; a func 10425 usa um `i32.load` com máscara 255) |
| @1590928 | `s_verificacoesDadoEleitor` | verificações de ano de nascimento feitas (comparado com cfg +140 `numTentativasVerificacao`, func 5404) |
| @1590932 | `s_tentativaDigitalSalva` | tentativa cuja imagem é guardada |
| @1590944 | `DEDOS_1X4 = {1, 6, 2, 7}` | ordem dos dedos da comparação 1x4 |
| @1908668 | `s_indiceDedo` | índice em DEDOS_1X4. `AvancaTentativa` o redefine para 0 |
| @1908672 | `s_digitalCapturada` (`vector<uebyte>`) | a captura do eleitor (WSQ) |
| @1908684 | `s_digitalMesario` | a captura do mesário (gravada por CRegistraDigitalOperador) |
| @1908696 | `s_tituloMesario` (`std::string`) | título do mesário que habilitou o eleitor |
| @1908708 | `s_score` (uint16) | score do matcher para o dedo reconhecido |
| @1908712, @1908716..28 | `s_qtdEleitores`, `s_faixaTentativa[4]` | simulação de treinamento: tamanho do cadastro e N/6 |

## 4. Fluxo de controle

### 4.1 Da identidade digitada à habilitação (urna real)

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

A "habilitação" sempre segue o mesmo padrão (5400, 10481, 10511): se `DeveHabilitarAudio()`, vai para
`CHabilitaAudioEleitor` ("Este eleitor necessita de áudio / Coloque o fone de ouvido na urna").
Caso contrário, posta a mensagem **1** para a thread do eleitor (`CAguardaMensagem` → `CEleitorVotando`, u06) e
muda para `CMostraEleitorVotando`.

### 4.2 Verificação biométrica

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

`AvancaTentativa` (5406) lança 9397 "Limite de tentativas atingido." se `t == numTentativasHabilitacao`
(os chamadores testam `EhUltimaTentativa` antes). Em seguida ela define `s_indiceDedo = 0` e incrementa `t`. Toda
tentativa, portanto, reinicia o percurso 1x4 no polegar direito, e toda tentativa exceto a primeira tem apenas
15 s.

### 4.3 Enquanto o eleitor vota: `CMostraEleitorVotando`

Tela: `<nome> / Título: … Seq: nnnn / Seção: nnnn / VOTANDO PARA: <cargo>` (construtor: func 1150,
u17). As teclas do mesário são lidas e descartadas (10426). `ProcessMessage` (10425):

| mensagem da thread do eleitor | ação |
|---|---|
| 2 EleitorNaoVotou | `CEleitorVotouNaoVotou` com `m_votou = false` |
| 3 / 4 EleitorDemorando sem / com voto | `CEleitorDemorando` (`m_votouParcialmente` 0/1). No treinamento de eleitores segue para `CSuspensaoAutomaticaEleitor` |
| 6 EleitorIniciouVotacao | **`SalvaHabilitacaoEleitor()`** (§4.5) |
| 9 AtualizaCargoAtual | copia `CThreadOperador +108` ("VOTANDO PARA: …") e redesenha |
| 13 SincronizaVoto | `CSincronismoOperador` (`m_suspensaoAutomatica = false`), texto do cargo := " " |
| outras (1, 5, 7, 8, 10–12) | ignoradas |

### 4.4 Treinamento de mesários: biometria simulada

Quando a urna está na fase de treinamento mas **não** no modo de treinamento de eleitores (`vota_f1823`,
`EhTreinamentoSemTreinamentoEleitor`), nada é capturado nem comparado. `CControlaReconhecimento`
e `CNomeEleitor` dividem o cadastro (N eleitores) pelo número sequencial:

| sequencial | resultado |
|---|---|
| (0, N/6] | reconhecido na tentativa 1 |
| (N/6, 2N/6] | reconhecido na tentativa 2 |
| (2N/6, 3N/6] | reconhecido na tentativa 3 |
| (3N/6, 4N/6] | reconhecido na tentativa 4 |
| (4N/6, 5N/6] | nunca reconhecido (caminho do ano de nascimento / do mesário) |
| (5N/6, N] | tratado como sem biometria |

`VerificaDigitalTreinamento` repassa a decisão ao matcher (`IFingerMatcher` slot 3 com
`(reconhecido, 20)`) e lê o score de volta. Isso dá aos treinandos todas as telas do fluxo real
sem impressões digitais. O modo é controlado apenas pela fase do `CEstadoGeral` = treinamento (ver §6.5).
Os sextos usam divisão inteira (`N/6`, depois `N/6*5`). Em uma seção pequena a maioria dos eleitores cai em
"sem biometria": com N = 10 o sexto é 1, então os eleitores 6–10 são tratados como sem biometria, e
com N < 6 todos os eleitores são. O cenário do harness `municipal-t1` tem um único eleitor.

### 4.5 `SalvaHabilitacaoEleitor` — o que é registrado quando o voto começa

Executa quando a thread do eleitor posta a mensagem 6. Não faz nada no modo de treinamento de eleitores. Caso contrário, conforme
`IInformacaoThreadOperador::GetTipoHabilitacao()`:

| tipo | imagens armazenadas (não na fase de treinamento) | `CEleitorDadosHabilitacaoBiometrica` |
|---|---|---|
| 0 SEM_BIOMETRIA | nenhuma | dedo 0, score 0, tentativas 0, erro 0 |
| 1 BIOMETRIA | `s_digitalCapturada` → `<trab>/wsq/habilitado/` (lança 9392 "Não há dados salvos de biometria do eleitor." se vazia) | dedo = DEDOS_1X4[s_indiceDedo], score = s_score, tentativas = s_tentativaDigitalSalva, erro 0 |
| 2 CODIGO_MESARIO | captura do eleitor (se houver) → `wsq/nao-habilitado/`; captura do mesário → `wsq/operador/` (lança 9393 "Não há dados salvos de biometria do mesário." se vazia) | dedo 0, score 0, tentativas = s_tentativa, erro = `CBiometriaEleitor.estadoDecifracao`, título do mesário (tipo TITULO) se conhecido |

Em seguida ela monta `comum::CEleitorDadosHabilitacao(identidade (slot 25), tipo, tipoAtivacaoAudio,
biometria, apresentaçãoFoto (slot 30))` (func 2733) e chama
`CEleitores::MarcaEleitorFoiHabilitado` (func 2825, u04). Isso grava a linha do eleitor em `eleitor_dinamico`.
`tipoAtivacaoAudio` = 0 se o modo de áudio do lado do eleitor é "conforme cadastro", 1 se habilitado, 2
caso contrário. As imagens passam por `CControlaArmazenamentoDeImagens` (func 2725, u24): pelo menos 5 MiB
livres, um nome aleatório único `NNNNNN.wsq` (o prefixo passado é vazio), cifragem com a chave pública
(`CifrarWsq`), gravadas nas duas áreas `<trab>` (flash interna `fi` e `fe`, `CPath::GetPathTrab(0|1)`).
O código também calcula um nome de arquivo descritivo (`<principal id padded to 12>-TT-DD`, `-TTErr`,
`_Operador`) e depois **o descarta** (§6.3).

### 4.6 Suspensão automática (treinamento de eleitores)

`CEleitorDemorando::StartState` (10440, u17) muda para `CSuspensaoAutomaticaEleitor` quando
`EhTreinamentoEleitor()`. A tela mostra `Suspensão automática em N segundos... / Votou parcialmente | Não votou /
CORRIGE: Cancelar`. StartState (10409) garante que o seu tick de 1 s existe e está parado (ela copia a
tabela de ticks da thread para um `map<id,bool>` local). Em seguida grava o texto de status, define 10 s, limpa a
flag de "enviado", formata o texto da contagem regressiva, emite um bipe por meio de `DisparaSinalizacaoSonora` (uma `std::thread` destacada
cuja lambda, 10410 em :89, chama o slot 6 de `IScreenMT` com `(50, 3)`), e só então inicia o tick, mostra
o formulário e permanece no estado. Como o bipe vem primeiro, o throw de §6.1 pula o início do tick e
o formulário. Cada tick (enquanto restam ≥ 2 s) faz a contagem regressiva, emite um bipe e redesenha. No último segundo o estado marca a
suspensão como enviada e posta **3** (SuspensaoAutomatica) para a thread do eleitor
(`CEleitorVotando::suspenderEleitor`, u06). Mensagens: 5 (o eleitor digitou de novo) → volta para
`CMostraEleitorVotando`. Depois da suspensão, 2 → `CPedeIdentidade` e 13 → `CSincronismoOperador`
com `m_suspensaoAutomatica = true`. CORRIGE antes do fim registra "Mesário abortou processo de
suspensão", posta 4 (continuar) e volta para `CMostraEleitorVotando`.

## 5. Telas do terminal do operador montadas por esta unidade

As coordenadas do MT são `SPoint{coluna, linha}` (começando em 1, 40×4). O alinhamento é 0 esquerda, 1 direita, 2 centro.
Vários textos centralizados usam x = 1, outros x = 20, e textos alinhados à direita normalmente usam x = 40. O único
`IScreenMT` deste binário, `simulador::CWasmScreenMT::Write` (func 8799, u31), ignora x nesses dois
alinhamentos: textos centralizados (≤ 39 caracteres) começam em `(40 − len) / 2`, os alinhados à direita em `40 − len`; só
os textos alinhados à esquerda usam `x − 1`. Os campos vêm de instâncias de template
do construtor de formulários do MT: `CLedFieldMT` (435), `CBuzzFieldMT` (941), `CBeepFieldMT` (1072),
`CTextFieldMT` com `CFixedText` (180), `CDataText<fn>` (619), `CDataTextFmt<fn>` (651),
`CDataTextFmt<CTextSource>` (1152), `CDataText<CTextSource>` (5409), o controle de entrada (395), a
entrada numérica de 4 dígitos (1151), e o formulário (301 = `CInteractiveForm<IScreenMT,IInputMT>` com
`CPreShowClearMT`).

| estado | LED / som | linhas |
|---|---|---|
| CPedeDigital (captura) | apagado / buzz(52,5) | (20,1)c "Solicite que o(a) eleitor(a)"; (20,2)c nome `{:2}`; (1,3) "posicione POLEGAR ou INDICADOR no sensor"; (1,4) "CORRIGE: cancelar" |
| CPedeDigital (aguarde) | apagado | (1,2)c "Por favor, aguarde." |
| CDigitalReconhecida | apagado / buzz(52,5) | (1,1) "ELEITOR(A) RECONHECIDO(A)"; (40,1)r score; (1,2) "Não assinar o caderno de votação"; (40,4)r "CONFIRMA: prosseguir" |
| CDigitalNaoReconhecida | apagado / beep(1) | (1,1) nome; (1,2)c "Eleitor(a) não reconhecido(a)"; (1,3)c "Tentativa t de n"; (1,3)r score; (1,4) "CORRIGE: cancelar"; (40,4)r "CONFIRMA: prosseguir" |
| CDigitalNaoReconhecidaPorTempo | apagado / beep(1) | nome; (20,2)c "Eleitor(a) não reconhecido(a)"; (20,3)c tentativa; "CORRIGE: cancelar"; (40,4)r "CONFIRMA: retornar" |
| CDigitalNaoReconhecidaDecBiometria | apagado / beep(1) | nome; "Eleitor(a) não reconhecido(a)"; (1,3)c "Dados biométricos inválidos"; cancelar / prosseguir |
| CVerificaDadoEleitor, CPedeAnoNascimentoSemBiometria | — | nome; "Título: …" + "Seq:" + `{:04}`; (1,3) "Digite o ANO de nascimento:" + campo de 4 dígitos em (29,3); "CORRIGE: cancelar"; (40,4)r "CONFIRMA: habilitar". O segundo formulário de CPedeAno… mostra "ANO DE NASCIMENTO INCORRETO" / "CONFIRMA: tentar novamente" |
| CInformaBioDesabilitadaDemo / CInformaAnoDesabilitadoDemo | apagado | "Biometria desabilitada em demonstração" / "A validação do ano de nascimento" + "é desabilitada em demonstração"; (1,4) "CONFIRMA: prosseguir" (formulários chamados `telaInforma…Demo`) |
| CEleitorVotouNaoVotou | apagado | (1,1) identidade digitada; (1,2)/(1,3) textos de status; (40,4)r "CONFIRMA: prosseguir" |
| CEleitorDemorando | **aceso** / buzz(51,10) | "O eleitor está demorando"; status; nome; (40,4)r "CONFIRMA" |

A fonte de texto da identidade digitada é a func 10586: `std::format("{}: {}", TipoToStr(tipo), formatted)`
em que TipoToStr dá "Título" / "CPF" / "Identificador" (senão lança 9409) e o número é
exibido como `xxxx xxxx xxxx` (título) ou `xxx.xxx.xxx-xx` (CPF) por ecourna_f1924. A gravação do harness
mostra exatamente `Título: XXXX XXXX XXXX`.

## 6. Código estranho ou arriscado

1. **A suspensão automática lança exceção neste build.** A func 5392 é `DisparaSinalizacaoSonora`, com
   o construtor de `std::thread` inlinado. Sem pthreads, `pthread_create` é um stub, e o otimizador
   reduziu a chamada a um `__throw_system_error(138 /* ENOTSUP */, "thread constructor
   failed")` incondicional. Os dois chamadores (StartState 10409 e ProcessTick 10405) lançariam, portanto, a exceção. Reproduzido
   com o harness: o estado do operador continua `CSuspensaoAutomaticaEleitor`, nenhuma contagem regressiva é mostrada, nenhum
   tick é iniciado e o eleitor nunca é suspenso. Inalcançável no simulador público, onde a
   thread do operador não executa. Na urna, cada segundo cria uma thread destacada só para emitir um bipe.
2. **`emscripten_sleep(1000)`** no fim de `ObtemEstadoPosReconhecimentoBiometrico` (5397). Ele é
   protegido pelo byte @1584624, que vale 1 e nunca é gravado (notas de u06/u07/u08). O glue aborta
   nele (sem Asyncify). A chamada está morta no build web, e de qualquer forma as interfaces de impressão digital não
   estão registradas lá (`IFingerScanner`, `IFingerMatcher` e `IFingerDetection` não têm
   implementação, então as buscas em `CPolySingleton` lançariam exceção antes).
3. **Nomes descritivos de arquivos WSQ calculados e jogados fora** (10425). Todo ramo de
   `SalvaHabilitacaoEleitor` formata um nome que liga a imagem ao eleitor (`<principal id padded to
   12>-TT-DD`, `-TTErr`, `<id>_Operador`) com `std::format`. Em seguida chama a rotina de armazenamento com um prefixo
   **vazio**, de modo que os arquivos recebem nomes aleatórios (`NNNNNN.wsq`, 6 dígitos de um número aleatório %
   999999). O caminho de registro do mesário (`CPedeDigitalMesario::GetControlador`, 5382) faz o mesmo
   com o prefixo `"me"`. A ligação entre uma imagem e um eleitor é mantida apenas na ordem dos eventos. Se
   alguém "consertasse" a variável morta passando-a como prefixo, os arquivos de digitais na mídia de
   resultado nomeariam o título do eleitor. Efeito na privacidade hoje: positivo. Manutenibilidade: enganoso.
4. **Pipeline de impressão digital removido na compilação do binário web.** O codificador WSQ (singleton sem estado,
   func 2742) e o extrator de template (singleton de parâmetros func 1903, `{8, 500, …}`) não deixam nenhuma chamada
   no ramo fora do treinamento de `CPedeDigital::ProcessTick` (restam apenas os acessores de singleton e as
   leituras de largura/altura). Os resultados deles são vetores vazios (`shared_f1379` / `vector::clear`). Assim, mesmo
   um harness que registrasse um scanner falso armazenaria um WSQ vazio e compararia uma amostra vazia; fora
   da fase de treinamento um eleitor reconhecido faria então `SalvaHabilitacaoEleitor` lançar 9392 (na
   fase de treinamento o caminho simulado de §4.4 é usado e nenhuma imagem é armazenada). Isso mostra que o
   simulador não pode dizer nada sobre a comparação biométrica real. Info.
5. **Biometria simulada na fase de treinamento** (§4.4). O reconhecimento é decidido pelo número
   sequencial do eleitor e pelo número da tentativa, e o matcher recebe a resposta. É controlado apenas pela
   fase do `CEstadoGeral` = treinamento (`'3'`) e `treinamentoEleitor = 0`. Isso é legítimo para o treinamento de
   mesários, mas é "segurança simulada" no mesmo binário. Info.
6. **Contador de tentativas vs restrição do banco de dados.** `numTentativasHabilitacao` é `INTEGER (1..50)` em
   `ModuloParametrizacaoUrna`, e `s_tentativa` pode chegar a esse valor. A tabela `eleitor_dinamico`,
   criada por `CEleitorDinamicoDAO` a partir da string DDL @449289, declara
   `numero_tentativa INTEGER CHECK(numero_tentativa IN (0,1,2,3,4))`. Uma parametrização com mais de
   4 tentativas faria a atualização da linha de um eleitor habilitado na tentativa ≥ 5 violar a restrição quando
   os dados dinâmicos fossem gravados. Para um eleitor habilitado pelo mesário (tipo 2) o valor armazenado é
   `s_tentativa`, que a essa altura é igual a `numTentativasHabilitacao`, então com esse parâmetro ≥ 5 todo eleitor nessa situação
   cairia nela. Não verificado em tempo de execução (exige o fluxo biométrico). Baixo. O texto do DDL foi
   verificado no banco de dados do harness (`samples/bu-real/run-full/fs/dsk/fi/dinamico/trab1/uenux.db`). O mesmo
   DDL tem duas restrições copiadas e coladas na coluna errada:
   `resultado_decifracao_foto … CHECK(estado_apresentacao_foto IN (0..8))` e
   `tipo_identificador_mesario … CHECK(tipo_identificador IN (1,2,3))`. Elas não são aplicadas às
   colunas pretendidas.
7. **`VotacaoBloqueadaPorHorario` pode lançar exceção dentro de um tick periódico** (10566). Se o cadastro diz que
   alguém votou (`CEleitores +104 > 0`) mas `CEstadoGeralVota.dtHrUltimoVoto` está vazio (estado
   inconsistente depois de uma restauração), `GetDtHrUltimoVoto` lança 8092 "Nenhum eleitor votou" a partir do
   tick de um minuto de `CPedeIdentidade`. Baixo.
8. **Coordenadas de campo inconsistentes (sem efeito visível aqui).** `CDigitalNaoReconhecida` centraliza
   "Eleitor(a) não reconhecido(a)" em x = 1 e declara tanto o texto centralizado da tentativa quanto o
   score alinhado à direita em (1,3), onde outras telas usam x = 20 (centro) e x = 40 (direita). A única
   implementação de MT deste binário, `CWasmScreenMT::Write` (func 8799), ignora x para texto centralizado e
   alinhado à direita (§5), então o texto da tentativa é centralizado e o score termina na coluna 40: eles não se
   sobrepõem, a menos que os dois textos juntos passem de cerca de 40 caracteres. Se o driver real do MT também
   ignora x não pode ser verificado a partir deste binário. Inconsistência cosmética de código, não um bug demonstrado.

## 7. Dados lidos e gravados

| dado | acesso |
|---|---|
| `CConfiguracaoEleicao` / `ParametrosUrna` (base +88, ordem ASN.1): +136 `numTentativasHabilitacao`, +140 `numTentativasVerificacao`, +487 `exibirScoreBiometria`, +489 `pedeAnoNascimentoEleitor` | lido (tentativas, exibição do score, caminho do ano de nascimento) |
| cfg +665 `utilizaBiometria` (ModuloProcessoEleitoral?), +668 tipo da identidade principal, +580 4º DataHoraJE de `HorariosUrna` (`terminoVotacao`, supondo +544/+556/+568/+580) | lido |
| `CEleitores` (cadastro): eleitor atual, +12 tamanho, +104 `m_qtdVotaram`, +112 índice secundário; `CEleitor` +0 sequencial, +20 necessidade especial, +100 possui biometria; `CBiometriaEleitor` dedos (+20/+32) e estado de decifração (+36) | lido; **gravado** por `MarcaEleitorFoiHabilitado` → `eleitor_dinamico` (tipo_habilitacao, dedo_habilitacao, score_habilitacao, numero_tentativa, erro_decifrar_biometria, tipo_ativacao_audio, identidade_habilitacao, apresentação da foto, titulo_mesario) |
| fase do `CEstadoGeral`, `CEstadoGeralVota` +72 treinamentoEleitor, dtHrUltimoVoto | lido |
| `<trab>/wsq/habilitado/`, `wsq/nao-habilitado/`, `wsq/operador/` em `/dsk/fi` e `/dsk/fe` | gravado (WSQ cifrado). No fim do dia `CGravadorWSQ` (5821) compacta os diretórios em `wsqbio.jez` / `wsqman.jez` / `wsqmes.jez` para a MR (o mapeamento de diretórios para pacotes provavelmente segue essa ordem) |
| `logd.dat` (CLogVota) | "Solicita digital. Tentativa [t] de [n]", "Capturada a digital…", "Número de tentativas de reconhecimento do dedo…", "Tipo de habilitação do eleitor [biométrica]", "O eleitor não possui biometria" (nível 2), "Habilitação cancelada durante …", "Mesário abortou processo de suspensão" |
| fila da thread do eleitor | mensagens 1 (habilitar), 3 (suspender), 4 (continuar) |

## 8. Boletim de Urna: o que esta unidade contribui

Esta unidade não monta o BU. Ela decide o **detalhamento do comparecimento por tipo de habilitação** que o
BU, o seu QR code e o arquivo de comparecimento carregam:

1. `SalvaHabilitacaoEleitor` armazena `ETipoHabilitacao` para todo eleitor habilitado: 0 = sem
   biometria, 1 = digital reconhecida, 2 = habilitado pelo mesário depois de falha da biometria
   (ASN.1 `habilitacaoPorCodigo`). Junto com os dados da digital ela armazena o dedo, o score, as tentativas e o
   erro de decifração, mais o título do mesário.
2. Depois do voto, `CSincronismoOperador` (u17/u20) marca o eleitor como `VOTOU`. No encerramento,
   `CEleitores::QtdVotaramPorTipoHabilitacao` (6034, u04) conta os eleitores por tipo. Essas contagens se tornam
   `ModuloBoletimUrna.DetalhamentoComparecimento` {`qtdEleitoresCompareceramSemBiometria` = tipo 0,
   `qtdEleitoresHabilitadosPorBiometria` = 1, `qtdEleitoresHabilitadosPorBiografia` = 2} e os campos do QR
   `HBSB` / `HBBM` / `HBBG` (`docs/bu/qrcode.md`: `HBBM + HBBG + HBSB = COMP`). A
   execução do harness confirma tipo 0 → `qtdEleitoresCompareceramSemBiometria: 1`, `HBSB:1`.
3. A mesma linha alimenta `ModuloResultadoUrnaCadastro.EstadoComparecimento` (`habilitacaoBiometrica`
   {tentativasHabilitacaoBiometrica, dedoHabilitado, erroLeituraBiometria, habilitacaoPorCodigo
   {situacaoReconhecimentoMesario, identificacaoMesario}, ultimoScore}, `apresentacaoFoto`,
   `situacaoHabilitacaoAudio`) e o relatório "ELEITORES HABILITADOS BIOGRAFICAMENTE" impresso no
   encerramento (`logd.txt` do harness).
4. As imagens de digitais salvas aqui seguem para a mídia de resultado (MR) como `wsqbio.jez` / `wsqman.jez` /
   `wsqmes.jez`. Os nomes dos arquivos são aleatórios (§6.3).
5. O próprio encerramento (`CEscolheOpcao` → "Todas as pessoas presentes já votaram?" →
   `CPedeTituloEncerramento` → `CConfirmaEncerramento`) pertence à u27. `VotacaoBloqueadaPorHorario`
   (slot 19) é a única ligação desta unidade com o horário de encerramento.

## 9. Particularidades do build web

* A thread do operador nunca é executada (§2): a unidade inteira é código morto no simulador. O eleitor é
  habilitado por `votaInit` (modo web, treinamento de eleitores), não por estes estados.
* Nenhum hardware de impressão digital: apenas `simulador::CFingerPrepareSimulador` (IFingerPrepare) é registrado.
  Buscas por `IFingerScanner`, `IFingerMatcher` e `IFingerDetection` lançariam "PolySingleton -
  solicitada uma instancia nao criada".
* Os corpos do codificador WSQ e do extrator de template estão vazios neste binário (§6.4).
* `std::thread` não pode ser criada (§6.1). `emscripten_sleep` aborta (§6.2).

## 10. Observações sobre wasm / Emscripten

* **Inlining entre arquivos por LTO**: srcloc de ccontrolareconhecimento.cpp dentro de `CPedeDigital::ProcessTick`
  e construtores inteiros de outras classes dentro de
  `ObtemEstadoPosReconhecimentoBiometrico` (5397), `CNomeEleitor::ProcessInput` (10500, nome nas ferramentas
  `NavegaBiometrica`) e `CDigitalNaoReconhecida::StartState` (10474). Por isso as ferramentas
  atribuíram singletons alheios (1901, 2734, 2735, 5402) aos arquivos desta unidade.
* **Chamadas virtuais extraídas (outlined)**: muitos thunks de uma linha `IInformacaoThreadOperador::GetInst().slotN()`
  (1535, 1687, 2226, 2745, 2746, 3619–3623, 5414–5416, 10583, 10584) são funções separadas no
  binário.
* **Leituras truncadas**: `CControlaReconhecimento::s_tentativa` é um `int` (gravado com `i32.store`),
  mas os pontos de log/formatação o leem com `i32.load8_u`. Isso é `static_cast<uebyte>` feito como uma leitura de byte
  em wasm little-endian.
* **libc com constantes dobradas**: `std::thread` → `system_error(138)` incondicional (ENOTSUP na numeração de
  errno do Emscripten). `std::random_device` + `mt19937` são inlinados na func 5412, com os limites da
  distribuição armazenados como a constante i64 `0x5A_0000003C` (60, 90).
* **Endereçamento `@2` no pseudo-código**: `x[24]:int@2` indexa de 2 em 2 bytes (endereço x+48). As coordenadas
  do MT (`65556` = `{20,1}`) são pares de 16 bits armazenados dessa forma.
* Símbolos de base de dados: `d_operator0033s…[k]` é o endereço `k + 1024`, e `d_dGGNE4dNci36…[k]` é
  `1581188 + k` (×4 para `:int`).

## 11. Questões em aberto

* Nomes reais dos métodos das interfaces de impressão digital (`IFingerScanner` slots 0–7, `IFingerMatcher` 2–4,
  `IFingerDetection` 2, `IUrna` 0). Nenhuma delas tem RTTI ou srcloc neste build. Os modos de LED 0–3
  são palpites.
* Significado do `bool` passado a `CInteractiveForm` (true para a maioria dos formulários, false para os avisos de demonstração).
* Qual de cfg +544/+556/+568/+580 é qual campo de `HorariosUrna` (a ordem acima é uma suposição).
* `CPedeAnoNascimentoSemBiometria` +28/+32 (int 0, bool true) e `CEleitorVotouNaoVotou` +20 (segundo
  texto) são definidos por métodos de outras unidades.
* Mapeamento exato de `wsq/{habilitado,nao-habilitado,operador}` para `wsq{bio,man,mes}.jez`.

## 12. Tabela de mapeamento (todas as 87 funções da u10)

"ran" = observada nos votos web gravados (nenhuma). "reconstruído em" é relativo a
`src/uenux2/src/app/vota/operador/`. "(comentário)" significa que a função é descrita lá como
comentário, porque é uma instância de biblioteca ou de template, um thunk ou um stub de atexit.

| func | tamanho | ran | nome no analisador | símbolo reconstruído | arquivo original | reconstruído em | conf. |
|---:|---:|:-:|---|---|---|---|---|
| 422 | 12 |  | `vota_f422` | `vota::CThreadVota::StopTick` | uenux2/src/app/vota/comum/cthreadvota.cpp (caminho inferido) | u10-foreign-fragments.cpp (comentário) | média |
| 651 | 265 |  | `api_f651` | `api::CFormBuilderMT::Add<api::CTextFieldMT, api::CDataTextFmt<std::string(*)(const std::string&)>>` | uenux2/src/api/gui/cformbuilder.h (instância de template) | u10-foreign-fragments.cpp (comentário) | média |
| 1072 | 19 |  | `vota_f1072` | `api::CFormBuilderMT::Add<api::CBeepFieldMT>` | uenux2/src/api/gui/cformbuilder.h (instância de template) | u10-foreign-fragments.cpp (comentário) | média |
| 1152 | 444 |  | `api_f1152` | `api::CFormBuilderMT::Add<api::CTextFieldMT, api::CDataTextFmt<api::CTextSource>>` | uenux2/src/api/gui/cformbuilder.h (instância de template) | u10-foreign-fragments.cpp (comentário) | média |
| 1901 | 1197 |  | `vota_f1901` | `vota::CEleitorVotouNaoVotou::GetInst` | uenux2/src/app/vota/operador/aguardaeleitor/celeitorvotounaovotou.cpp (caminho inferido) | u10-foreign-fragments.cpp | alta |
| 2097 | 187 |  | `vota_f2097` | `vota::CLogVota::LogaHabilitacaoCanceladaConfirmacaoDado` | uenux2/src/app/vota/log/clogvota.cpp (caminho inferido) | confirmaidentidade/cnomeeleitor.cpp (comentário) | baixa |
| 2225 | 757 |  | `vota_f2225` | `vota::impl::CInformacaoThreadOperador::AtualizaIdentidadeEleitor` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | média |
| 2233 | 222 |  | `api_f2233` | `api::CDateTime::AdicionaSegundos` | uenux2/src/api/util/cdatetime.cpp | u10-foreign-fragments.cpp | média |
| 2495 | 35 |  | `api_f2495` | `vota::CLogVota::LogaHabilitacaoCanceladaReconhecimento` | uenux2/src/app/vota/log/clogvota.cpp (caminho inferido) | confirmaidentidade/cpededigital.cpp (pontos de chamada) | baixa |
| 2499 | 57 |  | `api_f2499` | `std::__tree<std::__value_type<K,std::string>>::destroy` | - (instância de biblioteca) | u10-foreign-fragments.cpp (comentário) | média |
| 2732 | 156 |  | `vota::CSuspensaoAutomaticaEleitor::vf0` | `vota::CSuspensaoAutomaticaEleitor::~CSuspensaoAutomaticaEleitor` | uenux2/src/app/vota/operador/aguardaeleitor/csuspensaoautomaticaeleitor.cpp | aguardaeleitor/csuspensaoautomaticaeleitor.cpp | alta |
| 2733 | 217 |  | `vota_f2733` | `comum::CEleitorDadosHabilitacao::CEleitorDadosHabilitacao` | uenux2/src/app/comum/dados/celeitordadoshabilitacao.cpp (caminho inferido) | u10-foreign-fragments.cpp | média |
| 2734 | 1228 |  | `vota_f2734` | `vota::CEleitorDemorando::GetInst` | uenux2/src/app/vota/operador/aguardaeleitor/celeitordemorando.cpp (caminho inferido) | u10-foreign-fragments.cpp | alta |
| 2735 | 1413 |  | `api_f2735` | `vota::CVerificaDadoEleitor::GetInst` | uenux2/src/app/vota/operador/confirmaidentidade/cverificadadoeleitor.cpp (caminho inferido) | u10-foreign-fragments.cpp | alta |
| 2739 | 115 |  | `vota::CPedeDigital::vf0` | `vota::CPedeDigital::~CPedeDigital` | uenux2/src/app/vota/operador/confirmaidentidade/cpededigital.cpp | confirmaidentidade/cpededigital.cpp | alta |
| 2740 | 1815 |  | `vota_f2740` | `vota::CPedeDigital::GetInst` | uenux2/src/app/vota/operador/confirmaidentidade/cpededigital.cpp | confirmaidentidade/cpededigital.cpp | alta |
| 2745 | 20 |  | `vota_f2745` | `vota::impl::IInformacaoThreadOperador::GetApresentacaoFoto (outlined call)` | uenux2/src/app/vota/operador/aguardaeleitor/cmostraeleitorvotando.cpp | u10-foreign-fragments.cpp (comentário) | média |
| 3078 | 10 |  | `vota_f3078` | `vota::CInformacaoEleitor::AudioHabilitado` | uenux2/src/app/vota/eleitor/comum/cinformacaoeleitor.h (inline) | u10-foreign-fragments.cpp (comentário) | baixa |
| 3612 | 373 |  | `vota_f3612` | `vota::CSuspensaoAutomaticaEleitor::AtualizaTextoContagem` | uenux2/src/app/vota/operador/aguardaeleitor/csuspensaoautomaticaeleitor.cpp | aguardaeleitor/csuspensaoautomaticaeleitor.cpp | média |
| 3613 | 32 |  | `vota_f3613` | `std::__tree<std::__value_type<uebyte,bool>>::destroy` | - (instância de biblioteca) | aguardaeleitor/csuspensaoautomaticaeleitor.cpp (comentário) | média |
| 3621 | 20 |  | `api_f3621` | `vota::impl::IInformacaoThreadOperador::SetHabilitacaoSemBiometria (outlined call)` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | u10-foreign-fragments.cpp (comentário) | média |
| 3718 | 44 |  | `vota_f3718` | `comum::md::CEleitorDadosHabilitacaoBiometrica::CEleitorDadosHabilitacaoBiometrica` | uenux2/src/app/comum/dados/md/celeitordadoshabilitacaobiometrica.cpp (caminho inferido) | u10-foreign-fragments.cpp | média |
| 4535 | 150 |  | `api_f4535` | `vota::CLogVota::LogaMesarioAbortouSuspensao` | uenux2/src/app/vota/log/clogvota.cpp (caminho inferido) | aguardaeleitor/csuspensaoautomaticaeleitor.cpp (comentário) | baixa |
| 5392 | 42 |  | `vota_f5392` | `vota::CSuspensaoAutomaticaEleitor::DisparaSinalizacaoSonora` | uenux2/src/app/vota/operador/aguardaeleitor/csuspensaoautomaticaeleitor.cpp | aguardaeleitor/csuspensaoautomaticaeleitor.cpp | alta |
| 5397 | 2997 |  | `vota::CPedeDigital::ObtemEstadoPosReconhecimentoBiometrico` | `vota::CPedeDigital::ObtemEstadoPosReconhecimentoBiometrico` | uenux2/src/app/vota/operador/confirmaidentidade/cpededigital.cpp | confirmaidentidade/cpededigital.cpp | alta |
| 5400 | 82 |  | `vota_f5400` | `vota::CNomeEleitor::HabilitaEleitorSemBiometria` | uenux2/src/app/vota/operador/confirmaidentidade/cnomeeleitor.cpp | confirmaidentidade/cnomeeleitor.cpp | baixa |
| 5402 | 754 |  | `vota_f5402` | `vota::CInformaBioDesabilitadaDemo::GetInst` | uenux2/src/app/vota/operador/confirmaidentidade/cinformabiodesabilitadademo.cpp (caminho inferido) | u10-foreign-fragments.cpp | alta |
| 5403 | 13 |  | `vota_f5403` | `vota::CControlaReconhecimento::EhPrimeiraTentativa` | uenux2/src/app/vota/operador/confirmaidentidade/ccontrolareconhecimento.h (inline) | confirmaidentidade/ccontrolareconhecimento.h | média |
| 5405 | 18 |  | `api_f5405` | `vota::CControlaReconhecimento::EhUltimaTentativa` | uenux2/src/app/vota/operador/confirmaidentidade/ccontrolareconhecimento.cpp | confirmaidentidade/ccontrolareconhecimento.cpp | média |
| 5406 | 105 |  | `vota::CControlaReconhecimento::AvancaTentativa` | `vota::CControlaReconhecimento::AvancaTentativa` | uenux2/src/app/vota/operador/confirmaidentidade/ccontrolareconhecimento.cpp | confirmaidentidade/ccontrolareconhecimento.cpp | alta |
| 5412 | 766 |  | `api_f5412` | `vota::(anonymous namespace)::CalculaHorarioProximaInspecao` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | média |
| 10404 | 103 |  | `vota::CSuspensaoAutomaticaEleitor::vf6` | `vota::CSuspensaoAutomaticaEleitor::ProcessMessage` | uenux2/src/app/vota/operador/aguardaeleitor/csuspensaoautomaticaeleitor.cpp | aguardaeleitor/csuspensaoautomaticaeleitor.cpp | média |
| 10405 | 173 |  | `vota::CSuspensaoAutomaticaEleitor::ProcessTick` | `vota::CSuspensaoAutomaticaEleitor::ProcessTick` | uenux2/src/app/vota/operador/aguardaeleitor/csuspensaoautomaticaeleitor.cpp | aguardaeleitor/csuspensaoautomaticaeleitor.cpp | alta |
| 10407 | 191 |  | `vota::CSuspensaoAutomaticaEleitor::vf7` | `vota::CSuspensaoAutomaticaEleitor::ProcessInput` | uenux2/src/app/vota/operador/aguardaeleitor/csuspensaoautomaticaeleitor.cpp | aguardaeleitor/csuspensaoautomaticaeleitor.cpp | média |
| 10408 | 20 |  | `vota::CSuspensaoAutomaticaEleitor::vf5` | `vota::CSuspensaoAutomaticaEleitor::FinishState` | uenux2/src/app/vota/operador/aguardaeleitor/csuspensaoautomaticaeleitor.cpp | aguardaeleitor/csuspensaoautomaticaeleitor.cpp | média |
| 10409 | 537 |  | `vota::CSuspensaoAutomaticaEleitor::vf2` | `vota::CSuspensaoAutomaticaEleitor::StartState` | uenux2/src/app/vota/operador/aguardaeleitor/csuspensaoautomaticaeleitor.cpp | aguardaeleitor/csuspensaoautomaticaeleitor.cpp | média |
| 10410 | 126 |  | `vota::CSuspensaoAutomaticaEleitor::DisparaSinalizacaoSonora()::(lambda)::operator()` | `vota::CSuspensaoAutomaticaEleitor::DisparaSinalizacaoSonora()::$_0::operator()` | uenux2/src/app/vota/operador/aguardaeleitor/csuspensaoautomaticaeleitor.cpp | aguardaeleitor/csuspensaoautomaticaeleitor.cpp | alta |
| 10411 | 13 |  | `vota::CSuspensaoAutomaticaEleitor::vf1` | `vota::CSuspensaoAutomaticaEleitor::~CSuspensaoAutomaticaEleitor (deleting)` | uenux2/src/app/vota/operador/aguardaeleitor/csuspensaoautomaticaeleitor.cpp | aguardaeleitor/csuspensaoautomaticaeleitor.cpp | alta |
| 10414 | 38 |  | `vota_f10414` | `vota::CSuspensaoAutomaticaEleitor::GetInst()::s_inst (atexit destructor)` | uenux2/src/app/vota/operador/aguardaeleitor/csuspensaoautomaticaeleitor.cpp | aguardaeleitor/csuspensaoautomaticaeleitor.cpp (comentário) | média |
| 10425 | 4809 |  | `vota::CMostraEleitorVotando::SalvaHabilitacaoEleitor` | `vota::CMostraEleitorVotando::ProcessMessage` | uenux2/src/app/vota/operador/aguardaeleitor/cmostraeleitorvotando.cpp | aguardaeleitor/cmostraeleitorvotando.cpp | alta |
| 10426 | 155 |  | `vota::CMostraEleitorVotando::ProcessInput` | `vota::CMostraEleitorVotando::ProcessInput` | uenux2/src/app/vota/operador/aguardaeleitor/cmostraeleitorvotando.cpp | aguardaeleitor/cmostraeleitorvotando.cpp | alta |
| 10427 | 158 |  | `vota::CMostraEleitorVotando::vf2` | `vota::CMostraEleitorVotando::StartState` | uenux2/src/app/vota/operador/aguardaeleitor/cmostraeleitorvotando.cpp | aguardaeleitor/cmostraeleitorvotando.cpp | média |
| 10465 | 4463 |  | `vota::CPedeDigital::ProcessTick` | `vota::CPedeDigital::ProcessTick` | uenux2/src/app/vota/operador/confirmaidentidade/cpededigital.cpp | confirmaidentidade/cpededigital.cpp | alta |
| 10466 | 136 |  | `vota::CPedeDigital::vf7` | `vota::CPedeDigital::ProcessInput` | uenux2/src/app/vota/operador/confirmaidentidade/cpededigital.cpp | confirmaidentidade/cpededigital.cpp | média |
| 10467 | 114 |  | `vota::CPedeDigital::FinishState` | `vota::CPedeDigital::FinishState` | uenux2/src/app/vota/operador/confirmaidentidade/cpededigital.cpp | confirmaidentidade/cpededigital.cpp | alta |
| 10468 | 851 |  | `vota::CPedeDigital::StartState` | `vota::CPedeDigital::StartState` | uenux2/src/app/vota/operador/confirmaidentidade/cpededigital.cpp | confirmaidentidade/cpededigital.cpp | alta |
| 10469 | 13 |  | `vota::CPedeDigital::vf1` | `vota::CPedeDigital::~CPedeDigital (deleting)` | uenux2/src/app/vota/operador/confirmaidentidade/cpededigital.cpp | confirmaidentidade/cpededigital.cpp | alta |
| 10472 | 38 |  | `vota_f10472` | `vota::CPedeDigital::GetInst()::s_inst (atexit destructor)` | uenux2/src/app/vota/operador/confirmaidentidade/cpededigital.cpp | confirmaidentidade/cpededigital.cpp (comentário) | média |
| 10473 | 172 |  | `vota::CDigitalNaoReconhecida::vf7` | `vota::CDigitalNaoReconhecida::ProcessInput` | uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaoreconhecida.cpp | confirmaidentidade/cdigitalnaoreconhecida.cpp | média |
| 10474 | 2650 |  | `vota::CDigitalNaoReconhecida::StartState` | `vota::CDigitalNaoReconhecida::StartState` | uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaoreconhecida.cpp | confirmaidentidade/cdigitalnaoreconhecida.cpp | alta |
| 10481 | 926 |  | `vota::CDigitalReconhecida::vf7` | `vota::CDigitalReconhecida::ProcessInput` | uenux2/src/app/vota/operador/confirmaidentidade/cdigitalreconhecida.cpp | confirmaidentidade/cdigitalreconhecida.cpp | média |
| 10482 | 496 |  | `vota::CDigitalReconhecida::StartState` | `vota::CDigitalReconhecida::StartState` | uenux2/src/app/vota/operador/confirmaidentidade/cdigitalreconhecida.cpp | confirmaidentidade/cdigitalreconhecida.cpp | alta |
| 10485 | 172 |  | `vota::CDigitalNaoReconhecidaPorTempo::vf7` | `vota::CDigitalNaoReconhecidaPorTempo::ProcessInput` | uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaoreconhecidaportempo.cpp (caminho inferido) | u10-foreign-fragments.cpp | média |
| 10500 | 3978 |  | `vota::CNomeEleitor::NavegaBiometrica` | `vota::CNomeEleitor::ProcessInput` | uenux2/src/app/vota/operador/confirmaidentidade/cnomeeleitor.cpp | confirmaidentidade/cnomeeleitor.cpp | alta |
| 10501 | 44 |  | `vota::CNomeEleitor::vf2` | `vota::CNomeEleitor::StartState` | uenux2/src/app/vota/operador/confirmaidentidade/cnomeeleitor.cpp | confirmaidentidade/cnomeeleitor.cpp | média |
| 10511 | 179 |  | `vota::CControlaReconhecimento::vf7` | `vota::CControlaReconhecimento::ProcessInput` | uenux2/src/app/vota/operador/confirmaidentidade/ccontrolareconhecimento.cpp | confirmaidentidade/ccontrolareconhecimento.cpp | média |
| 10512 | 514 |  | `vota::CControlaReconhecimento::vf2` | `vota::CControlaReconhecimento::StartState` | uenux2/src/app/vota/operador/confirmaidentidade/ccontrolareconhecimento.cpp | confirmaidentidade/ccontrolareconhecimento.cpp | média |
| 10551 | 118 |  | `vota::impl::CInformacaoThreadOperador::vf1` | `vota::impl::CInformacaoThreadOperador::~CInformacaoThreadOperador (deleting)` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | alta |
| 10552 | 115 |  | `vota::impl::CInformacaoThreadOperador::vf0` | `vota::impl::CInformacaoThreadOperador::~CInformacaoThreadOperador` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | alta |
| 10554 | 18 |  | `vota::impl::CInformacaoThreadOperador::vf30` | `vota::impl::CInformacaoThreadOperador::GetApresentacaoFoto` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | média |
| 10556 | 16 |  | `vota::impl::CInformacaoThreadOperador::vf29` | `vota::impl::CInformacaoThreadOperador::SetFotoNaoApresentadaPorErro` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | média |
| 10557 | 9 |  | `vota::impl::CInformacaoThreadOperador::vf28` | `vota::impl::CInformacaoThreadOperador::SetFotoApresentada` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | média |
| 10558 | 9 |  | `vota::impl::CInformacaoThreadOperador::vf27` | `vota::impl::CInformacaoThreadOperador::SetEleitorSemFoto` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | média |
| 10559 | 130 |  | `vota::impl::CInformacaoThreadOperador::GetIdentidadePrincipalEleitor` | `vota::impl::CInformacaoThreadOperador::GetIdentidadePrincipalEleitor` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | alta |
| 10560 | 130 |  | `vota::impl::CInformacaoThreadOperador::GetIdentidadeEleitor` | `vota::impl::CInformacaoThreadOperador::GetIdentidadeEleitor` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | alta |
| 10561 | 74 |  | `vota::impl::CInformacaoThreadOperador::GetTipoIdentidadeDigitada` | `vota::impl::CInformacaoThreadOperador::GetTipoIdentidadeDigitada` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | alta |
| 10562 | 120 |  | `vota::impl::CInformacaoThreadOperador::GetIdentidadeDigitada` | `vota::impl::CInformacaoThreadOperador::GetIdentidadeDigitada` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | alta |
| 10563 | 21 |  | `vota::impl::CInformacaoThreadOperador::vf22` | `vota::impl::CInformacaoThreadOperador::SetTipoIdentidadeDigitada` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | média |
| 10564 | 206 |  | `vota::impl::CInformacaoThreadOperador::vf21` | `vota::impl::CInformacaoThreadOperador::SetIdentidadeDigitada` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | média |
| 10565 | 149 |  | `vota::impl::CInformacaoThreadOperador::vf20` | `vota::impl::CInformacaoThreadOperador::LimpaIdentidades` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | média |
| 10566 | 179 |  | `vota::impl::CInformacaoThreadOperador::vf19` | `vota::impl::CInformacaoThreadOperador::VotacaoBloqueadaPorHorario` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | média |
| 10567 | 22 |  | `vota::impl::CInformacaoThreadOperador::vf18` | `vota::impl::CInformacaoThreadOperador::GetDataHoraProximaInspecao` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | média |
| 10568 | 48 |  | `vota::impl::CInformacaoThreadOperador::vf17` | `vota::impl::CInformacaoThreadOperador::SorteiaProximaInspecao` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | média |
| 10569 | 380 |  | `vota::impl::CInformacaoThreadOperador::vf16` | `vota::impl::CInformacaoThreadOperador::GetTextoQtdVotaram` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | média |
| 10570 | 9 |  | `vota::impl::CInformacaoThreadOperador::vf14` | `vota::impl::CInformacaoThreadOperador::SetHabilitacaoSemBiometria` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | média |
| 10571 | 9 |  | `vota::impl::CInformacaoThreadOperador::vf13` | `vota::impl::CInformacaoThreadOperador::SetHabilitacaoBiometrica` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | média |
| 10572 | 9 |  | `vota::impl::CInformacaoThreadOperador::vf12` | `vota::impl::CInformacaoThreadOperador::SetHabilitacaoCodigoMesario` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | média |
| 10573 | 8 |  | `vota::impl::CInformacaoThreadOperador::vf11` | `vota::impl::CInformacaoThreadOperador::HabilitadoSemBiometria` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | baixa |
| 10574 | 10 |  | `vota::impl::CInformacaoThreadOperador::vf10` | `vota::impl::CInformacaoThreadOperador::HabilitadoPorBiometria` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | baixa |
| 10576 | 10 |  | `vota::impl::CInformacaoThreadOperador::vf9` | `vota::impl::CInformacaoThreadOperador::HabilitadoPorCodigoMesario` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | baixa |
| 10577 | 102 |  | `vota::impl::CInformacaoThreadOperador::vf8` | `vota::impl::CInformacaoThreadOperador::GetTextoAudio` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | média |
| 10578 | 9 |  | `vota::impl::CInformacaoThreadOperador::vf7` | `vota::impl::CInformacaoThreadOperador::SetAudioHabilitadoManualmente` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | média |
| 10579 | 17 |  | `vota::impl::CInformacaoThreadOperador::vf5` | `vota::impl::CInformacaoThreadOperador::EleitorNecessitaAudio` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | média |
| 10580 | 34 |  | `vota::impl::CInformacaoThreadOperador::vf3` | `vota::impl::CInformacaoThreadOperador::DeveHabilitarAudio` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | média |
| 10581 | 23 |  | `vota::impl::CInformacaoThreadOperador::vf2` | `vota::impl::CInformacaoThreadOperador::LimpaDadosHabilitacao` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | média |
| 10586 | 837 |  | `vota::TipoToStr` | `vota::TextoIdentidadeDigitada` | uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp | comum/cinformacaothreadoperador.cpp | média |
| 13471 | 18 |  | `api_f13471` | `api::getResourceMovie()::s_filmes (atexit destructor)` | uenux2/src/api/gui/iresource.h | u10-foreign-fragments.cpp (comentário) | média |
