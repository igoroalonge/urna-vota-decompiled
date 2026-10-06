# u39 — `app:vota` classes without a known file: small states of both threads, the mesário-registration controller, the execution policies and the exit-time stubs

Unit u39 has **124 wasm functions** of the VOTA application (TSE `uenux2/src/app/vota`) whose classes carry no
`std::source_location` record, so the tools could not attach them to a source file. They belong to 46 classes
(as the tools group them; `CMessageEleitor` only lends its name to the shared `CPriorityMessageQueue` destructor)
plus two groups of free functions. Most are **virtual methods found only through vtables** (`Class::vfN`), mainly the
`StartState` (vtable slot 2) of small states whose other methods other units already reconstructed.
Also in the unit: 26 of the 37 slots of the mesário-registration controller, the two thread-execution
policies (urna vs. web), the audio messages of seven voting states, the keypad-test shuffler, five
"Mais informações" menu items, five screen data sources, and **31 compiler-generated exit-time
destructors** ("atexit stubs").

Only **4 of the 124 functions ran** during the recorded votes (`analysis/runtime/*.functions.tsv`):

| func | what | why it runs in the web page |
|---|---|---|
| 7823 | `CExecucaoVotaCooperativa::Processa` | every `votaTick` (≈60/s) steps the voter thread through it |
| 11790 | `IConfereVotoEmCargo::ProcessTickAudio` | the end of each "Confira o seu voto" (conferência) screen |
| 7178 | `CSincronismoEleitor::StartState` | end of every vote ("Gravando") |
| 12267 | `CItemImprimeEstadoUrnaVota::GetNumViasImpressas` | the "Mais informações" menu is built at start-up |

The operator-thread states (about half of the unit) never run in the page, because the simulator does not step the
mesário thread (u10 §2). The recorded **operator-harness run** (`tools/bu/operator_harness.mjs`,
`samples/bu-real/run-full/{states,logd}.txt`) does pass through several of them. It confirms
`CAguardaInicio` → `CPedeIdentidade` with the log record "Urna pronta para receber votos" (func 10217),
`CPedeAnoNascimentoSemBiometria` (10490, no log record), `CPerguntaFilaEleitorVazia` ("Operador indagado se
todas as pessoas presentes votaram", 10715) and `CFimAquisicaoVotos` → `CFinalizaOperador` + voter message 7 →
"Inicio do Encerramento" (10737). In other words, this is the start of the BOLETIM DE URNA.

Every original path in this unit is **(path inferred)** unless marked *attested*. The directory comes from the
closest relatives and from paths other units already inferred for the same class (kept identical so that the
fragments merge). The class name gives the file name (TSE convention `CFooBar` → `cfoobar.{h,cpp}`).

## 1. Glossary

| term | meaning |
|---|---|
| urna | the electronic voting machine |
| eleitor / terminal do eleitor (TE) | voter / the voter's screen + keypad (voter thread `CThreadEleitor`) |
| mesário / terminal do mesário (MT) | poll worker / his 4 x 40 LCD microterminal with keypad (operator thread `CThreadOperador`) |
| habilitar / habilitação | the mesário identifies the voter and releases the urna for him |
| título (de eleitor) | voter registration number (12 digits); also used to identify mesários |
| biometria / digital / dedo | fingerprint data / a fingerprint / finger |
| decifrar | decrypt (the fingerprint templates in the voter roll are encrypted) |
| justificativa (eleitoral) | declaration of absence made at any urna by a voter away from his section |
| suspender (a votação do eleitor) | end the session of a voter who does not finish |
| cabina / inspeção | voting booth / the periodic booth inspection the mesário must do |
| comparecimento de mesários | attendance record of the poll workers (SQLite `comparecimento_mesario`) |
| encerramento / fim da aquisição de votos | closing of the urna / end of vote collection |
| BU (boletim de urna), via | the per-urna result report; a printed copy of it |
| BUJ, BIM, BEHB | boletins of justifications, of mesário identification, of voters released without fingerprint ("habilitados biograficamente") |
| zerésima | report printed before voting showing zero votes for everyone |
| RDV (registro digital do voto) | the shuffled table of cast votes, source of the BU |
| MR (mídia de resultado) | the removable memory that carries the result files |
| teste do teclado | keypad test of the voter terminal, offered before the zerésima and after a restart |
| estadoVota | `EstadoGeralVota.estadoVota` in `vota.bin`: the application phase (C++ value = ASN.1 value + `'1'`) |
| áudio | the accessible vote: every screen is read aloud (RHVoice) in headphones |

## 2. Contents and reconstructed sources

| topic | classes | § |
|---|---|---|
| operator (MT) states | `CAguardaInicio`, `CAguardaInspecao` (+ inlined `CConfirmaInspecionada`), `CPerguntaCodigoSuspensao`, `CDesabilitaAudioEleitor`, `CHabilitaAudioEleitor`, `CVerificaDadoEleitor`, `CDigitalNaoReconhecidaDecBiometria`, `CDigitalNaoReconhecidaPorTempo`, `CPedeAnoNascimentoSemBiometria`, `CCancelaHabilitacaoEleitor`, `CIdentidadeInvalida`, `CEleitorNaoEncontrado`, `CJustificativaEfetuada`, `CPerguntaFilaEleitorVazia`, `CHabilitaAudioManualmente`, `CHorarioVotacaoTerminou`, `CFimAquisicaoVotos` | 4 |
| mesário registration controller | `CControladorRegistraMesariosVota` (26 of 37 slots) | 5 |
| execution policies | `CExecucaoVota` (urna), `CExecucaoVotaCooperativa` (web) | 6 |
| voter states | `CSincronismoEleitor`, `CInspecionaUrna`, `CAplicacaoEncerrada`, `CQuerImprimirZeresima`, `CImpressaoZeresimaTardia`, `CMaisInformacoes`, `testeteclado::CBase`, `testeteclado::CEsperaRetestar`, `CGeraZeresimaBase` (dtor) | 3 |
| audio of the vote screens | `IConfereVotoEmCargo` (4 slots); `GetMensagemAudio` of `CPedeNominal`, `CConfirmaVotoNominal`, `CConfirmaVotoLegenda`, `CCandidatoInexistente`, `CCandidatoInapto`, `CProporcionalBranco`, `CProporcionalNulo` | 3.2 |
| keypad test | `testeteclado::impl::CGeradorTeclasAleatorio` | 3.5 |
| screens | `CItem*Vota` (5 menu items), 4 `CTelasVota` data sources, "Via nº N", `CProgressoEncerramento`, `IQRCodeBUDS` (dtor) | 3.6, 7 |
| exit-time destructors | 31 `__dtor_…` stubs + the message-queue destructors (`CMessageEleitor::vf0/vf1`) | 8 |

**Reconstructed sources** (all new unless marked *fragment*; `.uNN` fragments are the parts of a file that belong to
another unit's class or whose class is declared elsewhere):

```
src/uenux2/src/app/vota/operador/caguardainicio.{h,cpp}
src/uenux2/src/app/vota/operador/aguardaeleitor/caguardainspecao.{h,cpp}      (+ CConfirmaInspecionada ctor/GetInst)
src/uenux2/src/app/vota/operador/aguardaeleitor/cperguntacodigosuspensao.{h,cpp}
src/uenux2/src/app/vota/operador/aguardaeleitor/cdesabilitaaudioeleitor.u39.cpp    fragment (class in estadosoperador.u34.h)
src/uenux2/src/app/vota/operador/confirmaidentidade/cverificadadoeleitor.{h,cpp}
src/uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaoreconhecidadecbiometria.{h,cpp}
src/uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaoreconhecidaportempo.{h,cpp}
src/uenux2/src/app/vota/operador/confirmaidentidade/cpedeanonascimentosembiometria.{h,cpp}
src/uenux2/src/app/vota/operador/confirmaidentidade/chabilitaaudioeleitor.{h,cpp}
src/uenux2/src/app/vota/operador/justificativa/cjustificativaefetuada.{h,cpp}
src/uenux2/src/app/vota/operador/comum/ccancelahabilitacaoeleitor.{h,cpp}
src/uenux2/src/app/vota/operador/leidentidade/cidentidadeinvalida.{h,cpp}
src/uenux2/src/app/vota/operador/leidentidade/celeitornaoencontrado.cpp          (class declared by u27's macro)
src/uenux2/src/app/vota/operador/outrasopcoes/cfimaquisicaovotos.{h,cpp}
src/uenux2/src/app/vota/operador/outrasopcoes/chabilitaaudiomanualmente.u39.cpp  fragment (class in estadosoperador.u34.h)
src/uenux2/src/app/vota/operador/outrasopcoes/chorariovotacaoterminou.u39.cpp    fragment (idem)
src/uenux2/src/app/vota/operador/outrasopcoes/cperguntafilaeleitorvazia.u39.cpp  fragment (idem)
src/uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.{h,cpp}
src/uenux2/src/app/vota/cexecucaovota.{h,cpp}
src/uenux2/mock/app/vota/cexecucaovotacooperativa.{h,cpp}
src/uenux2/src/app/vota/eleitor/csincronismoeleitor.{h,cpp}
src/uenux2/src/app/vota/eleitor/cinspecionaurna.{h,cpp}
src/uenux2/src/app/vota/eleitor/cconferevotoemcargo.u39.cpp                       fragment (file attested)
src/uenux2/src/app/vota/eleitor/fimvotacao/caplicacaoencerrada.{h,cpp}
src/uenux2/src/app/vota/eleitor/fimvotacao/cprogressoencerramento.{h,cpp}
src/uenux2/src/app/vota/eleitor/fimvotacao/cimprimirbuoutrasobrigatorias.u39.cpp  fragment (file attested)
src/uenux2/src/app/vota/eleitor/fimvotacao/cmostraqrcodebu.u39.cpp                fragment (file attested)
src/uenux2/src/app/vota/eleitor/iniciovotacao/cquerimprimirzeresima.{h,cpp}
src/uenux2/src/app/vota/eleitor/iniciovotacao/cimpressaozeresimatardia.{h,cpp}
src/uenux2/src/app/vota/eleitor/iniciovotacao/cmaisinformacoes.u39.cpp           fragment (u34 owns ProcessInput)
src/uenux2/src/app/vota/eleitor/iniciovotacao/cgerazeresima.u39.cpp              fragment
src/uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cbase.{h,cpp}
src/uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cesperaretestar.{h,cpp}
src/uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cgeradorteclasaleatorio.cpp
src/uenux2/src/app/vota/eleitor/votaproporcional/{cpedenominal,cconfirmavotonominal,cconfirmavotolegenda,
      ccandidatoinexistente,ccandidatoinapto,cproporcionalbranco,cproporcionalnulo}.{h,cpp}
src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u39.cpp                         fragment (file attested)
src/uenux2/src/app/vota/eleitor/u39-foreign-fragments.cpp                         the 31 atexit stubs + queue dtor
```

Path choices that are new (not taken from another unit): `cexecucaovotacooperativa` in **`uenux2/mock/app/vota/`**.
This web-only class has its vtable (@1532252) emitted right after the last `mock/app/simulador/wasm` class
(`simulador::CWasmTimerScheduler`) and right before the first `src/app/vota` class (`vota::CAssinadorVota`). The mock
tree mirrors `src/app` (`mock/app/comum/cappinfobuilder.cpp` is attested). u18 proposed `mock/app/simulador/wasm/`.
Also new: `cfimaquisicaovotos` in `operador/outrasopcoes` (reached from `CConfirmaEncerramento`, and its vtable
sits among that directory's classes), `cimpressaozeresimatardia` in `eleitor/iniciovotacao` (next to
`cinformacaozeresimatardia.cpp`), `cgeradorteclasaleatorio` in `testeteclado/`, and
`cfinalizaaquisicao.cpp` in `eleitor/fimvotacao` (for its stubs). The `CItem*Vota` classes are placed **inside ctelasvota.cpp** because their vtables,
`CMenuMaisInformacoesVota`'s and even `comum::CItemMenu`'s are emitted in that translation unit's data.
For `CControladorRegistraMesariosVota` I kept `vota/comum/` (u20, u27); u18 used `operador/comparecimentomesario/`.

## 2.1 Class hierarchy (RTTI)

```
api::CState
└─ comum::CAppState                                   (+4 m_proximoEstado, +8/+9/+10 accepts messages/keys/ticks)
   ├─ operator states (MT, CThreadOperador): CAguardaInicio (24 B), CAguardaInspecao (20), CConfirmaInspecionada (20),
   │    CPerguntaCodigoSuspensao (32), CDesabilitaAudioEleitor (20), CHabilitaAudioEleitor (20), CVerificaDadoEleitor (20),
   │    CDigitalNaoReconhecidaDecBiometria (20), CDigitalNaoReconhecidaPorTempo (28), CPedeAnoNascimentoSemBiometria (36),
   │    CCancelaHabilitacaoEleitor (12), CIdentidadeInvalida (20), CJustificativaEfetuada (32),
   │    CPerguntaFilaEleitorVazia (20), CHabilitaAudioManualmente (28), CHorarioVotacaoTerminou (20),
   │    CFimAquisicaoVotos (12), CRegistroMesarioEncerrado (12)
   │    └─ IEleitorImpedidoVotar (24) ─ CEleitorNaoEncontrado (+ 5 siblings, u27)
   ├─ voter states (CThreadEleitor): CSincronismoEleitor (12), CInspecionaUrna (20), CQuerImprimirZeresima (20),
   │    CImpressaoZeresimaTardia (20), CMaisInformacoes (24), CGeraZeresimaBase (20) ─ CGeraZeresima, CRegerarZeresima
   ├─ CEstadoComDesligamentoAutomatico (28, u06)       (1 s tick: power-off after the battery time-out)
   │    ├─ CAplicacaoEncerrada (36)
   │    ├─ testeteclado::CBase (36) ─ CPreZeresima (48), CRetomada (40)
   │    └─ testeteclado::CEsperaRetestar (44)
   └─ CVotacaoStateAudio (u08)
        ├─ IConfereVotoEmCargo (40) ─ CConfereVotoEmCargo<TConfirma, ETelaVotacao> x10 (u06)
        ├─ CConfirmaVotoEmCargo ─ CConfirmaProporcional ─ CConfirmaVotoNominal, CConfirmaVotoLegenda,
        │                                                 CCandidatoInexistente, CCandidatoInapto,
        │                                                 CProporcionalBranco, CProporcionalNulo (36 B each)
        └─ CCompletaProporcional ─ CPedeNominal (32), CPedeNulo
comum::IControladorRegistraMesarios ─ vota::CControladorRegistraMesariosVota (4 B)
vota::IExecucaoVota ─ vota::CExecucaoVota (4 B, urna) / vota::CExecucaoVotaCooperativa (8 B, web)
comum::CAbstractTelaProgresso ─ vota::CProgressoEncerramento (44 B)
comum::CItemMenu ─ comum::CItemImprimeEstadoUrna / …ListaEleitores / …VersoesPacotes / …ParametrosUrna ─ vota::CItem*Vota
                 └─ vota::CItemVisualizarCandidatosVota
vota::IQRCodeBUDS (no base, 20 B)
vota::testeteclado::impl::IGeradorTeclas ─ impl::CGeradorTeclasAleatorio (4 B)
api::CPriorityMessageQueue<api::SMessage>, api::CMessageInterface ─ vota::CMessageEleitor / vota::CMessageOperador
```

All the `comum::CAppState` states are lazy singletons: a static `std::mutex` plus a static `std::unique_ptr` right
after it. wasm-opt merged many accessors into shared bodies (`vota_f764(mutex, &ptr, vtable, flags)`: `CFimAquisicaoVotos`
5428, `CCancelaHabilitacaoEleitor` 1536, `CFinalizaOperador` 5342, …). Most are inlined into the first state that
needs them, e.g. `CConfirmaInspecionada` inside `CAguardaInspecao::ProcessMessage`.

## 3. Voter-thread pieces

### 3.1 End of a vote: `CSincronismoEleitor::StartState` (7178, observed)

`CEleitorVotando::GravaVotos` (4454) puts the ballot into the in-memory RDV and switches to `CSincronismoEleitor`
(12 B, `CAppState(1)`: messages only). `StartState`:

1. `CThreadOperador::GetInst().GetFila().Add(SMessage{13, &fila}, 1)`. This is operator message **13**
   (`EMensagemOperadorRecebida::SincronizaVoto`).
2. `m_proximoEstado = this`.
3. `CTelasVota::m_barraProgresso` (+180, a 4-step `CProgressBar`) is reset to its minimum (func 3667), and
   `m_telaProgressoRegistroVoto` (+172, "Gravando") is shown.

On the urna, the operator thread (`CSincronismoOperador`) marks the voter as VOTOU and answers with voter message 5.
`ProcessMessage(5)` (7181, u19) then calls `ISincronismoVotoEleitor::SincronizaVoto()`, which writes the RDV and
advances the bar, and moves on to "FIM". **Web build:** nobody reads message 13 (the queue keeps `6, 9, 9, 13, 1` after a
vote, u10). `votaTick` posts message 5 itself (vota_web_wasm.u30.cpp), and `SincronizaVoto` is the no-op
`CSincronismoVotoEleitorWeb`, so the vote is never written to the RDV.

### 3.2 Conferência and the audio sentences

`IConfereVotoEmCargo` (u06) is the short "Confira o seu voto" screen that follows the last digit. Four of its slots are in this
unit:

| slot | func | behaviour |
|---|---|---|
| 0 dtor | 1717 | cancels a pending "after the current audio" wait (`m_espera`, +32), then `~CVotacaoStateAudio` |
| 11 `FinishStateAudio` | 11792 | same cancel (so a late "audio ended" callback cannot switch state) |
| 12 `ProcessTickAudio(tick)` | 11790 | if `m_comTick` (+29) and `tick == m_tick` (+28): `StopTick`, then `ExecutarAposAudioAtual([this]{ m_proximoEstado = GetProximoEstado(); })` (confirmation screen once the audio ends) |
| 15 `GetMensagemAudio` | 11789 | `"Confira o seu voto."` |

Until the tick fires, every key is refused ("Tecla indevida pressionada", u06). That is why the headless runs need a pause before
CONFIRMA (analysis/runtime/README.md).

Slot 15 of the proportional states returns the sentence RHVoice reads to voters who use headphones. The tags are
expanded by `CVotacaoStateAudio::FormataMensagem` (u08). Exact texts, Latin-1 in the binary:

| func | state | sentence |
|---|---|---|
| 11725 | `CPedeNominal` (party digits typed, candidate digits missing) | Você está votando para {cargo-atual}. Digite os demais dígitos do seu candidato, ou aperte confirma para prosseguir, ou corrige para reiniciar este voto. |
| 11728 | `CConfirmaVotoNominal` | Você está votando para {cargo-atual} n{o/a} candidat{o/a} {voto}: {candidato}. Aperte confirma ou corrige. |
| 11735 | `CConfirmaVotoLegenda` (party vote) | Você está votando para {cargo-atual} no número {voto}. Aperte confirma para votar na legenda {legenda}, {nome-partido}, ou corrige para reiniciar o seu voto. |
| 11731 | `CCandidatoInexistente` | Você está votando para {cargo-atual} no número {voto}. Candidato inexistente. Aperte confirma para votar na legenda {legenda}, {nome-partido}, ou corrige para reiniciar este voto. |
| 11748 | `CCandidatoInapto` | Você está votando para {cargo-atual} n{o/a} candidat{o/a} {voto}. Candidat{o/a} não concorre. Se apertar confirma, este voto será nulo. Aperte confirma ou corrige. |
| 11704 | `CProporcionalBranco` | Você está votando em branco para {cargo-atual}. Aperte confirma ou corrige. |
| 11701 | `CProporcionalNulo` | Você está votando para {cargo-atual} no candidato {voto}. Número errado. Se apertar confirma, este voto será nulo. Aperte confirma ou corrige. |

The constructors (merged body 1961, u06) fix screen and RDV vote type: nominal (2) for `CConfirmaVotoNominal`,
**legenda (1)** for `CConfirmaVotoLegenda` *and* `CCandidatoInexistente` (a non-existent candidate of an existing party
counts for the party, as the sentence says), nulo (4) for `CCandidatoInapto`/`CProporcionalNulo`, branco (3).

### 3.3 Booth inspection (voter side)

`CInspecionaUrna::StartState` (11798): shows "Por favor, inspecione cabina e urna." and logs "Inspeção da urna
iniciada". The operator thread starts the procedure (message 12, §4.3), so it never happens in the web page.

### 3.4 Before voting: zerésima questions and "Mais informações"

* `CQuerImprimirZeresima::StartState` (11921): logs through `LogaMesarioIndagadoImprimirZeresima` (5882) and shows
  `CTelasVota +228`. Only reached in voter-training mode, where printing the zerésima is optional.
* `CImpressaoZeresimaTardia::StartState` (11932): "Mesário indagado se horário da urna está correto". Reached when the
  zerésima is requested after its time window: the urna asks whether its own clock is right before printing
  (CORRIGE switches the urna off, u09 §3.3).
* `CMaisInformacoes::StartState` (11897): **rebuilds** the menu screen every time
  (`CTelasVota::CriaTelaMaisInformacoes`, 6599), because the counts "(n/max)" change after each print.
* `CGeraZeresimaBase::~CGeraZeresimaBase` (1720): thunk to the shared body 2902 (vptr + release of the screen).

### 3.5 Keypad test (`testeteclado`)

```
CPreZeresima / CRetomada  (testeteclado::CBase)
   StartState 11875: m_tela = CriaTela() [slot 10]; m_tela->Show()
   ProcessInput 11874 (CInteractiveForm::Read inlined, srcloc cinteractiveform.h:57):
     CORRIGE  -> GetEstadoSemTeste() [slot 12]; if the state changes: log "Teste de Teclado do TE não executado"
     CONFIRMA -> CTesteTeclado::GetInst().m_estadoAposTeste = GetEstadoPassouNoTeste() [slot 11]; -> CTesteTeclado
CTesteTeclado (u26): the mesário presses the 13 keys in the order of
   CGeradorTeclasAleatorio::GeraSequencia 11803 = std::shuffle(teclas, static std::mt19937(std::random_device{}()))
   wrong key -> CTesteFalhou -> "Repetir teste" -> CEsperaRetestar
CEsperaRetestar  StartState 11825: m_segundosEspera += 5 (5, 10, 15 s …, never reset); g_horaRetestar = now + that;
                 start the 100 ms tick; show "Por favor, espere {}s para a / realização de uma nova tentativa …"
                 FinishState 11824: stop the tick, deactivate the screen (IForm::Deactivate inlined)
                 ProcessTickNaoDesligamento 11823 (u26): when the time has come -> CTesteTeclado
```

### 3.6 Screen data sources (ctelasvota.cpp and cimprimirbuoutrasobrigatorias.cpp)

These are plain functions stored as function pointers (table slots) and called whenever the field is drawn:

| func | slot | used by | value |
|---|---|---|---|
| 13137 | 1097 | footer of the confirmation screens (`adicionaInstrucoesConfirmaCorrige`, 1102) | `" para REINICIAR este voto"` (after "CORRIGE") |
| 13176 | 1098 | vote screens (6624, 3060) | `g_votoDigitado` (@1833288), the digits typed so far |
| 13441 | 1086 | number field of every empty vote screen (`adicionaCampoNumero`, 2384 → 6689) | bytes of `:/resource/images/audioHabilitado.jpg` when `CInformacaoEleitor::m_modoAudio != 2`, else a 66-byte 1x1 **white** BMP (an invisible placeholder) |
| 12974 | 1105 | screen "telaNumeroCopiasErrado" ("Número de cópias / acima do limite permitido") | `"O número máximo permitido é " + to_string(GetQuantidadeMaximaBUsAdicionais())` |
| 13023 | 1101 | screen "telaDestinoBUs" of `CImprimirBUOutrasObrigatorias` | `format("Via nº {}", uebyte(qtdBU + 1))` |

"Mais informações" menu items (`CItem*Vota`, created by `CriaTelaMaisInformacoes`; slot 3 `GetNumViasImpressas`
feeds `comum::CItemImprime*::Disponivel`, u37):

| func | item | reads (vota.bin `EstadoGeralVota.numViasImpressasRelatorios`) |
|---|---|---|
| 12267 | `[1] Estado da urna (n/max)` | `numViasEstadoUrna` (byte +73) |
| 12266 | `[2] Lista de eleitores (n/max)` | `numViasEleitores` (+74) |
| 12259 | `[3] Versões de pacotes (n/max)` | `numViasVersoesDados` (+75) |
| 12255 | `[4] Parâmetros de urna (n/max)` | `numViasPU` (+76) |
| 12251 | `[5] Visualizar candidatos` | deleting destructor only (its `Disponivel` 12244 is in u02) |

## 4. Operator-thread (MT) pieces

All of this is **dead code in the web page**. The table shows the entry action of each state (`StartState`, slot 2) and the
message handlers reconstructed here; the rest of each class is in u10, u17, u19, u27 or u34.

### 4.1 Start of the day: `CAguardaInicio`

The first operator state (MT: "VOTA: 10.23.0.1 / DESENVOLVIMENTO / Siga as instruções na tela do eleitor" + date and
time). `StartState` (10220) starts its 500 ms tick and shows the form, `ProcessTick` (10218) redraws the form
(clock), `FinishState` (10219) stops the tick. `ProcessMessage` (10217) waits for the voter thread:

| operator message | sent by (voter thread) | next operator state |
|---|---|---|
| 7 | `CDefineRotaPreVotacao` (11994), `CReinicioComparecimentoMesario` (11917) | `comum::CRegistrarMesarios` (mesário registration before voting) |
| 8 | `CInicioVotacao` (5972) | log "Urna pronta para receber votos", `CPedeIdentidade` |
| 10 | `CFinalizaAquisicao` (12113, restart during the closing) | `CFimAquisicaoVotos` |
| 9 / other | – | stay |

### 4.2 Releasing a voter (habilitação)

| state | StartState / handler | log record (severity) |
|---|---|---|
| `CIdentidadeInvalida` | 10669: show "Identidade: … / Número errado" | "Identificador do eleitor digitado inválido" (2) |
| `CEleitorNaoEncontrado` | 10663 = slot 9 hook, called by `IEleitorImpedidoVotar::StartState` after `Show()` | "Eleitor não encontrado para o identificador informado" (1) |
| `CDigitalNaoReconhecidaPorTempo` | 10486: "Tentativa {} de {}" (static attempt counter @1590924, cfg +136) | "Timeout de reconhecimento do dedo. Tentativa [{}] de [{}]" (1) |
| `CDigitalNaoReconhecidaDecBiometria` | 10478: show; unless training, if `GetBiometria().GetEstadoDecifracao() > 0` | "Erro ao decifrar a biometria do eleitor - Código ({})" (3; the code is an enum with a custom `std::formatter`) |
| `CVerificaDadoEleitor` (birth year after the last fingerprint attempt) | 10444 | "Solicitação de dado pessoal do eleitor para habilitação manual" (1) |
| `CPedeAnoNascimentoSemBiometria` (voter without usable biometrics) | 10490: `m_pedindoAno = true`, `m_erros = 0`, show | – |
| `CHabilitaAudioEleitor` (voter needs audio) | 10524 | "Solicitado ao mesário que conecte o fone de ouvido" (1) |
| `CDesabilitaAudioEleitor` (after an audio voter) | 10436 | "Solicitado ao mesário que desconecte o fone de ouvido" (1) |
| `CCancelaHabilitacaoEleitor` | 10646: if audio was enabled by hand → "Áudio desativado pelo fim da votação"; `IInformacaoThreadOperador::LimpaDadosHabilitacao()`; → `CPedeIdentidade` | see left |
| `CJustificativaEfetuada` | 10594: `m_novaJustificativa` ? "AUSÊNCIA JUSTIFICADA" : "JÁ JUSTIFICOU" on the MT | "Justificativa recebida" (1) / "Eleitor já justificou" (2) |

### 4.3 While the voter votes

* **Suspension** (`CPerguntaCodigoSuspensao`, "Informe seu título para / suspender a votação"). `StartState` (10421) logs
  "Solicitado título eleitoral para suspensão do eleitor", shows the form and clears `m_suspensaoEnviada` (+28).
  `ProcessMessage` (10419) handles what the voter thread reports meanwhile. **2** (session ended without a vote):
  → `CEleitorVotouNaoVotou` with `m_votou = false`. **5** (the voter typed again): → `CMostraEleitorVotando`, the
  question is dropped. **13** (the voter confirmed the last cargo): → `CSincronismoOperador` with
  `m_suspensaoAutomatica = true`, so that after "FIM" the MT says the voter did vote.
* **Booth inspection.** `CAguardaInspecao::ProcessMessage` (10530): message **11** from the voter keypad
  (`CInspecionaUrna`: inspection confirmed) → `CConfirmaInspecionada`, whose constructor is inlined here. It builds a clock at
  {33,1}, "Inspeção completa" centred at {20,2}, "CONFIRMA: continuar a votação" at {40,4}, and an input control.
* **Audio by hand** (`CHabilitaAudioManualmente`, 10751, "outras opções" → "Ativar/Desativar áudio" → CONFIRMA):
  flips `IInformacaoThreadOperador::AudioHabilitadoManualmente`, writes "ÁUDIO ATIVADO"/"ÁUDIO DESATIVADO"
  into the MT text, logs "Áudio ativado pelo mesário"/"Áudio desativado pelo mesário", shows the form for
  **1 s (`emscripten_sleep(1000)`)** and returns to `CPedeIdentidade`.
* `CHorarioVotacaoTerminou` (10758): "Horário de votação terminou" (1). The voting period is over and only
  the closing is possible.

### 4.4 Closing (encerramento)

`CPerguntaFilaEleitorVazia::StartState` (10715) logs "Operador indagado se todas as pessoas presentes votaram" and shows
"Todas as pessoas presentes já votaram? / CORRIGE: não / CONFIRMA: sim". `CFimAquisicaoVotos::StartState` (10737)
is the hinge of the closing (§7).

## 5. `CControladorRegistraMesariosVota`: VOTA side of the mesário registration

The mesário-attendance screens are application-independent (`comum/comparecimentomesario`, u22). VOTA answers
their questions through `comum::IControladorRegistraMesarios`. The only implementation is this 4-byte object,
pushed by `CAjusteInicial` (7160), which never runs in the web build.

* **Ticks** (slots 2–4, 10800/10799/10798): the comum states run on the operator thread. `CriaTick(ms)` =
  `CThreadOperador +20 (CTickManager)::AddTick`, and `StopTick`/`StartTick` forward to it.
* **Period** (slot 6, 10796): `estadoVota` `'7'` REGISTROMESARIOINICIAL → INICIAL (1), `'8'` VOTAR → VOTACAO (2),
  `':'` REGISTROMESARIOFINAL → FINAL (3), anything else → NENHUM (0). Compiled as `(estado − '7') <= 3u` plus the
  table @534720 = {1, 2, 0, 3}.
* **Limit** (slot 7, 10795): `CRegistradorMesario` (func 815, cache of `comparecimento_mesario`) counts the rows of the
  period: *abertura* (periodo 1) for `'7'`/`'8'`, *encerramento* (periodo 2) for `':'`. The limit is reached when
  the count is > 5, i.e. at most **6 mesários** per period. Any other `estadoVota` reports "limit reached".
* **Next states**: slot 11 (10793) → `CRegistroMesarioEncerrado` (12 B, `CAppState(1)`, lazy @1904888, constructor inlined;
  its `StartState` sends voter message 14). Slot 12 (10791) → `CFinalizaOperador`.
* **Mesário lookup**: slots 13/14 (10790/10789) search the section roll for the título typed on the MT
  (`CThreadOperador +96`) as a TÍTULO identity (`CEleitores::Busca`, 2264). "Is the mesário a voter of this
  section?" drives the fingerprint comparison of `CPedeDigitalMesario`.
* **Log records** (slots 19–36 except 28), all severity 1 unless stated:

| slot | func | record |
|---|---|---|
| 19 | 10784 | Mesário {título} registrado |
| 20 / 21 / 22 | 10783 / 10782 / 10781 | Registrando mesários antes da votação / durante a votação / após a votação |
| 23 | 10779 | Operador indagado se ocorrerá registro de mesários |
| 24 / 25 | 10778 / 10777 | Operador confirmou o registro de mesários / Operador cancelou o registro de mesários |
| 26 | 10776 | Digitado título inválido para o registro de mesário (**2**) |
| 27 | 10775 | Mesário {título} já registrado |
| 29 | 10773 | Operador encerrou ciclo de registro de mesários |
| 30 | 10772 | Digital capturada não corresponde a digital do eleitor: Polegar Direito [score {}], Polegar Esquerdo [score {}], Indicador Direito [score {}], Indicador Esquerdo [score {}] |
| 31 | 10771 | Pedido de leitura da biometria do mesário {título} |
| 32 / 33 | 10770 / 10769 | Mesário {título} é eleitor da seção / Mesário {título} não é eleitor da seção |
| 34 | 10768 | Realizada a conferência da biometria do mesário |
| 35 / 36 | 10767 / 10766 | Operador indagado se continua registrando mesários / Operador indagado se finaliza registro mesários |

## 6. Execution policies: urna vs. web

`vota::IExecucaoVota` decides how VOTA's three threads run.

| slot | `CExecucaoVota` (urna, 4 B) | `CExecucaoVotaCooperativa` (web, 8 B) |
|---|---|---|
| 2 `Executa` / 3 `Inicia` | 3 (10235): start the voter, operator and monitor threads, then `Sleep(1000)`; 2 (10236): the same, then join the three threads (= `Inicia` + `Aguarda`) (u18) | both slots are body 4713 (u18): install the first state (`CAguardaMensagem`, passed to the constructor 7828) in a new `CAppStateContext` |
| 4 `Aguarda` | join the three threads | nop |
| 5 `IniciaOperador` | **10233**: `CThreadOperador::GetInst().Start()` | nop |
| 6 `FinalizaThreads` | **10232**: sets the stop flag (`api::CThread +8`) of the operator and monitor threads | nop |
| 7 `Processa` | `return false` (ICF 340) | **7823**: if the voter thread has a context (+32) and no stop request (+8): `return CThreadEleitor::GetInst().Processar()` (4349, formerly shown by the tools as `ProcessarEntrada`) |

`main()` of the web build pushes `CExecucaoVotaCooperativa` before anyone calls `IExecucaoVota::GetInst()`, so the urna
policy is never created. Nothing in the web build steps the operator thread. That one design decision explains
everything this unit marks as dead code, including why the page can never produce a BU (docs/10 §5.1).

## 7. BOLETIM DE URNA: what this unit contributes, step by step

This unit holds no BU generator. It holds the **hinges** of the closing and several pieces the BU chain uses. The
generation itself is in u08/u09 (`CGeraBU`, `CImprimindoBU`, `CGravaResultado`…, docs/10 and docs/bu/codepath.md).
Steps 1–4 below are the ones recorded by the operator harness (`samples/bu-real/run-full`), with the functions of
this unit in bold. That run stops inside step 4: `comum::CGravadorHashes::GravaResultado` throws "PolySingleton -
solicitada uma instancia nao criada" for `IGenericFactory<IHash>` while writing `-hash.dat` (`exceptions.json`, last
`logd.txt` record "Gerando arquivo de resultado [hash.dat] + [Início]"). The rest of step 4 and steps 5–8 come from
the code only:

1. **MT: "2 - Encerrar votação"** (`CEscolheOpcao`, u27) → `CIniciaFinalizacao` checks the time → **`CPerguntaFilaEleitorVazia::StartState` (10715)**.
   It logs "Operador indagado se todas as pessoas presentes votaram" and asks "Todas as pessoas presentes já votaram?".
   CORRIGE (no) → `CAguardaEleitoresVotarem` (3 s, back to identification). CONFIRMA (yes) → log "Todas as pessoas presentes já votaram? SIM"
   → `CPedeTituloEncerramento` (the presiding mesário's título: "Título digitado para encerramento: …")
   → `CConfirmaEncerramento` ("Procedimento de encerramento confirmado"). That state marks `dhFimAquisicao`, sets
   `estadoVota = FIMAQUISICAOVOTOS ('9')` and saves `vota.bin`.
2. **`CFimAquisicaoVotos::StartState` (10737)**:
   * if the election identifies mesários and this is not a voter-training urna (func 2520): → `comum::CRegistrarMesarios`
     for the final registration. `AtualizaEstadoRegistro` changes `'9'` into `':'` (REGISTROMESARIOFINAL). When the
     round ends, **slot 12 (10791)** returns `CFinalizaOperador` and slot 15 (u18) posts the same message 7 as below;
   * otherwise it posts **voter message 7** (`SMessage{7}` on `CThreadEleitor +36`, priority 1) and goes to `CFinalizaOperador`.
3. Voter thread: `CAguardaMensagem::ProcessMessage(7)` → log "Inicio do Encerramento", `estadoVota = GERARBU` →
   `CGeraBU` (BU report + QR codes + verification code, `trab/bu.dat`) → `CGeraRelatorios` (BUJ, BIM, BEHB) →
   `CInicioBU` → [`CQuerImprimirBU` in voter training] → `CImprimindoBU` (via 1, "Mesário indagado sobre qualidade do
   Boletim de Urna") → `CGravaResultado`.
4. **`CGravaResultado` shows `CProgressoEncerramento`** (this unit: dtor 3907/12104, `Inicia` 12103, `Finaliza`
   12102; `Avanca` 12101 in u07). The screen "telaProgressoEncerramento" reads "Preparando dados para encerramento / O processo pode
   levar alguns minutos / Por favor, aguarde…" over a 30-step bar shown as a percentage. `comum::CGravacaoResultados` calls `Inicia` (bar
   = min, show), `Avanca` per step and `Finaliza` (bar = max) while it writes and signs the result files:
   `…-bu.dat` (ASN.1 `EntidadeBoletimUrna` in an `EntidadeEnvelopeGenerico`), `-rdv.dat`, `-jufa.dat`,
   `-imgbu.dat`, `-imgze.dat`, `-hash.dat`, `-log.jez`, `-vota.vsc`. Each step writes a log record "Gerando arquivo
   de resultado [x.dat] + [Início]/[Término]" (the harness run logged bu, rdv, jufa, imgbu and imgze, then died at
   the start of hash.dat).
5. `CCopiaResultadoParaMR` (copy to the MR, verify the BU) → **`CImprimirBUOutrasObrigatorias`**, the remaining mandatory
   copies. Its screen "telaDestinoBUs" shows **"Via nº N" (13023)** with N = `EstadoGeralVota.qtdBU + 1`, evaluated at
   each redraw: `qtdBU` (vota.bin, byte +8) counts the copies already printed. Then BUJ, BIM, BEHB, `CRetirarMR`.
6. Extra copies (`CEmitirMaisBU`, u08): asking for more than allowed shows "telaNumeroCopiasErrado" with **"O número
   máximo permitido é N" (12974)**, where N = `GetQuantidadeMaximaBUsAdicionais()` = obrigatórias + adicionais − já impressas
   (1 in voter training; votadefs.cpp:135).
7. `CMostraQRCodeBU` shows the BU as QR codes, one part at a time (≤ 2500 characters per code on screen, ≤ 1100 on
   paper). Their texts live in the poly-singleton **`IQRCodeBUDS`** (this unit: destructors 12050/12049). They are kept until the
   singleton list is cleared. BRANCO shows the urna certificate (`CMostraQRCodeCertificado`).
8. **`CAplicacaoEncerrada::StartState` (12058)**: log "Votação encerrada", final screen. The urna then switches itself off when the
   battery time-out expires (`CEstadoComDesligamentoAutomatico`).

The atexit stubs of this unit (§8) include the singletons of every state of this chain (`CGeraBU` 12111/12112,
`CGeraRelatorios`, `CImprimindoBU`, `CQuerImprimirBU`, `CGravaResultado`, `CCopiaResultadoParaMR`,
`CImprimirBUOutrasObrigatorias`, `CImprimirBJust`, `CImprimindoBEHB`, `CVerificaQtdBUsAdicionais`,
`CLimiteCopiasBUAtingido`, `CMostraQRCodeCertificado`, `CFinalizaAquisicao`). They give the addresses of
those singletons (useful for probing a running module), and they are dead code.

## 8. The exit-time destructors (31 functions)

All follow the pattern described in `src/uenux2/src/app/vota/u38-foreign-fragments.cpp`. The static `std::mutex` +
`std::unique_ptr<T>` of every lazy singleton each get a `__dtor_<var>` stub for `__cxa_atexit`. The registrations
were optimised away (`EXIT_RUNTIME=0`), and the stubs survive only through their table slots. Bodies: `~mutex` = the no-op ICF
stub 150; `~unique_ptr` = one of the merged "delete" bodies 349 (trivial destructor), 389 (ICF 244), 763
(ICF 448), 1564 (ICF 785). The owners and addresses are listed in `src/uenux2/src/app/vota/eleitor/u39-foreign-fragments.cpp`.
Three stubs are not singletons:

* 13593 / 13588: `g_votoDigitado` (`std::string` @1833288) and `g_votosEleitor`
  (`std::vector<pair<TCargoID, CVoto>>` @1833300), the ballot being composed (celeitorvotando.cpp);
* 14464: the function-local `static const std::regex("\\B\\d")` (@1833216, guard byte @1833256) with which
  `CVotacaoStateAudio::FormataMensagem` splits `{voto}` into single digits for the speech synthesiser. Its body 6096 is libc++'s
  `~basic_regex`.

## 9. Data read and written

| data | access | where |
|---|---|---|
| `dinamico/log/logd.dat` (urna event log, Latin-1 records `1\|sev\|text`) | write | every record quoted in §4–§5 (through `CLogVota`/`IEventosLog`, `api::CLoga::loga`) |
| `vota.bin` `EstadoGeralVota` (ModuloEstadoGeralVota) | read | `estadoVota` (+0) in slots 6/7; `qtdBU` (+8) in 13023; `numViasImpressasRelatorios` (+73..+76) in the menu items; `treinamentoEleitor` (+72) through funcs 1823/2520 |
| `uenux.db` table `comparecimento_mesario` | read (counts) | slot 7 via `CRegistradorMesario` (writes are in the comum states) |
| voter roll (`CEleitores`) | read | slots 13/14 (mesário título), 10478 (biometric decryption state) |
| `CConfiguracaoEleicao` +136 | read | number of fingerprint attempts (10486) |
| resource `:/resource/images/audioHabilitado.jpg` | read | 13441 |
| `/dev/urandom` token (`std::random_device`, served by `crypto.getRandomValues` in Emscripten) | read | 11803 |
| operator/voter message queues | write | 7178 (op 13), 10737 (voter 7), plus the reactions in 10217, 10419, 10530 |

No SQL is written here and no ASN.1 is encoded here. This unit's only link to the BU files is the progress screen.

## 10. Web-build specifics

* `CExecucaoVotaCooperativa` (7828/7823) is the web replacement of the thread runner. It is what `votaTick` executes
  (723 profiler samples in the municipal vote).
* Unreachable in the page: all operator states (§4), the controller (§5), the urna policy (§6) and, therefore, the whole
  closing (§7). `CSincronismoEleitor` still posts message 13 to a queue nobody reads.
* The simulator starts directly in `estadoVota = VOTAR`, so the zerésima questions, the keypad test and "Mais informações" never
  appear. The "Mais informações" screen is still **built** at start-up (12267 observed).
* The audio sentences (§3.2) are used only when the voter enables audio (`votaSetAudioEnabled`), and only if RHVoice is
  available in the page.

## 11. wasm / Emscripten observations

* **Lazy singletons without guards.** The operator states' `GetInst` bodies contain no `__cxa_guard`, so the statics are
  namespace-scope / class members (u38's argument). The exceptions are the `std::mt19937` of `GeraSequencia`, a true
  function-local static with a one-byte guard (@1837652), and the `std::regex` of `FormataMensagem` destroyed by
  stub 14464 (guard byte @1833256). In a single-threaded build `__cxa_guard_acquire` inlines
  to a byte test.
* **Merged log bodies.** `CLogVota` one-liners were merged by shape (`merge-similar-functions`). Func 3888 is "log a 47-byte
  literal" and takes the literal's six 8-byte chunks as *parameters*; the annotator prints them as six overlapping
  strings (`/* "Operador indagado …" */, /* " indagado …" */…`). 3887 formats the mesário título, and 6017 formats a
  título argument. The format string reaches 3887/6017 as a `[begin, end)` pair.
* **Packed `SPoint` constants look like strings.** In 10530 the positions `{33,1}`, `{20,2}`, `{40,4}` are the
  i32 constants 65569, 131092, 262184. The annotator decodes them as string addresses ("dadeVersaoArquivos", "tuloRelatorio"…).
* **`std::format` argument types** read from the code: `198` = two `unsigned` (10486), `0x18C63` = four `int`
  (10772), `6` = one `unsigned` (13023), `13` = one string (3887/6017). `15` = a *handle*: the enum of 10478
  is formatted by a TSE `std::formatter` specialisation whose body is libc++'s shared lambda 536 (table slot 4091).
* **Compiled switches**: 10796 is a bounds check plus a table load; 10217 and 10419 are `br_table`s on `message − 7` / `message − 2`.
* **ICF**: the three `CPriorityMessageQueue<SMessage>` destructors (base, `CMessageEleitor`, `CMessageOperador`)
  are one body, 3162/3157. The derived vptr stores were dead and were removed.
* `emscripten_sleep` is guarded by the byte @1584624 (= 1, never written). The glue's `_emscripten_sleep` aborts
  ("Please compile your program with async support…").

## 12. Suspicious / notable code

1. **10751 `CHabilitaAudioManualmente::StartState` calls `emscripten_sleep(1000)`**, and `CExecucaoVota` (10235/10236, u18)
   does the same. In this build that call aborts the whole module. It is unreachable in the page, but any future web
   version that steps the operator thread (as the harness does, with a stubbed sleep) would crash the moment a
   mesário toggles the audio. On a real urna it freezes the MT for 1 s by design.
2. **Mesários' títulos written to the event log** (10769/10770/10771/10775/10784; 3887/6017). "Mesário 012345678901
   é eleitor da seção", "Pedido de leitura da biometria do mesário …", "Mesário … registrado". The mesário
   título eleitoral is personal data. `logd.dat` goes into the result files (`-log.jez`), and TSE publishes
   urna logs. Voters' títulos are not logged by these functions.
3. **Biometric scores in the log, possibly under the wrong finger** (10772). The four scores are always labelled
   Polegar Direito, Polegar Esquerdo, Indicador Direito, Indicador Esquerdo. The caller (`CPedeDigitalMesario`,
   u22) fills the vector *compactly*, only for the fingers the mesário has in the roll (`scores[n++]`), and zero
   for the rest. With a missing finger the scores shift and are attributed to the wrong fingers in the log.
   The vector always has 4 elements, so there is no out-of-bounds access.
4. **`LimiteMesariosAtingido` (10795) returns true for any unexpected `estadoVota`.** It is safe by design (registration
   refused), but it relies on slot 5 having moved `'9'` to `':'` before the check.
5. **Unread operator queue in the web build** (7178 + the other voter states). Each vote leaves 2 + (number of
   cargos) messages of 16 bytes that nobody removes (6 at the start, 9 per cargo, 13 and 1 at the end: 5 in the
   2-cargo municipal vote, u10 §2). The growth is bounded in practice: after "FIM" the voter thread idles in
   `CAguardaMensagem` (`done: true` in the transcripts; `node tools/run/headless.mjs --scenario municipal-t1 --auto
   blank --voters 2` reports `NEXT-VOTER {"started":false}` after 15 s of ticks) because only the operator thread
   could release the next voter, and both buttons of the page's finish panel leave the page (`restartVoting` → `window.location.reload()`,
   `chooseScenario` → `index.html`, `votaWasmAdapter.js`). It would become a leak only in a variant that released
   several voters per page load.
6. **`CEsperaRetestar::StartState` (11825)**: the wait before repeating the keypad test grows by 5 s on every
   attempt and is never reset while the application runs. After many failures the mesário waits minutes. This is a
   usability issue, not a safety one.
7. **`CExecucaoVotaCooperativa::Processa` (7823)** returns false silently if the voter thread's stop flag (+8) is set
   (only `CThreadOperador::FinalizaExecucao` does that). A web variant that ran the operator thread and
   reached it would freeze the voter screen with no error. Info.
8. **Dead biometric decrypt path in training** (10478). The log record is suppressed in
   `EhTreinamentoSemTreinamentoEleitor`, and the function copies the whole `CBiometriaEleitor` (vector + map) twice by
   value to read one enum. Inefficient, not a bug.
9. **Misleading name in the analysis DB.** Func 1950, srcloc-named `CInformacaoEleicao::EhModoDemonstracao`, returns
   `!IInterfaceInit::GetDemoMode() && cfg[400]`. It behaves like "identifies mesários (not in demo mode)", with
   `EhModoDemonstracao()` inlined into it. Callers such as `CFimAquisicaoVotos` (via 2520) read as "demo mode → register
   mesários" if one trusts the tool name.

## 13. Open questions

* The name of `IExecucaoVota` slots 5/6 (`IniciaOperador`, `FinalizaThreads`) and of the `CAbstractTelaProgresso` slots
  (`Inicia`/`Finaliza`/`Avanca`) are inferred from behaviour. No string or srcloc names them.
* `IEleitorImpedidoVotar` slot 9: called by `StartState` after `Show()`. It is named `LogaEntrada` here and
  `AoRetornar` in u27's header. The two should be unified.
* Is `CExecucaoVotaCooperativa` in `mock/app/vota/` or `mock/app/simulador/wasm/`? Both are compatible with the
  data layout. The database now files all three of its functions (7828, 4713, 7823) under
  `uenux2/mock/app/vota/cexecucaovotacooperativa.cpp`, component `app:mock`, following this unit's choice.
  Is `CControladorRegistraMesariosVota` in `vota/comum/` or `vota/operador/comparecimentomesario/`?
* `CItem*Vota`: defined in ctelasvota.cpp or in a header included only by it? The emitted vtables only prove that
  ctelasvota.cpp is the (first) user.
* `CControladorRegistraMesariosVota` slot 8 (always `true`, ICF 434) has no caller in the comum states reconstructed so
  far.

## 14. Complete mapping table (124 functions)

Column "run" = observed executing in the recorded votes. The src file is where the reconstruction lives. The
original path follows in parentheses (inferred unless §2 says attested). "Tools name" is the current name in the
analysis DB.

| # | func | size | run | tools name | reconstructed symbol | src file (original path) |
|---|---|---|---|---|---|---|
| 1 | 1559 | 66 |  | `vota::testeteclado::CBase::vf0` | `vota::testeteclado::CBase::~CBase` | `src/uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cbase.cpp` (`uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cbase.cpp`) |
| 2 | 1717 | 151 |  | `vota::IConfereVotoEmCargo::vf0` | `vota::IConfereVotoEmCargo::~IConfereVotoEmCargo` | `src/uenux2/src/app/vota/eleitor/cconferevotoemcargo.u39.cpp` (`uenux2/src/app/vota/eleitor/cconferevotoemcargo.cpp`) |
| 3 | 1720 | 12 |  | `vota::CGeraZeresimaBase::vf0` | `vota::CGeraZeresimaBase::~CGeraZeresimaBase` | `src/uenux2/src/app/vota/eleitor/iniciovotacao/cgerazeresima.u39.cpp` (`uenux2/src/app/vota/eleitor/iniciovotacao/cgerazeresima.cpp`) |
| 4 | 3157 | 111 |  | `vota::CMessageEleitor::vf1` | `api::CPriorityMessageQueue<api::SMessage>::~CPriorityMessageQueue (deleting)` | `src/uenux2/src/api/ipc/cmessagequeue.h` (u18) + `src/uenux2/src/app/vota/eleitor/u39-foreign-fragments.cpp` (`uenux2/src/api/ipc/cmessagequeue.h`) |
| 5 | 3162 | 108 |  | `vota::CMessageEleitor::vf0` | `api::CPriorityMessageQueue<api::SMessage>::~CPriorityMessageQueue` | `src/uenux2/src/api/ipc/cmessagequeue.h` (u18) + `src/uenux2/src/app/vota/eleitor/u39-foreign-fragments.cpp` (`uenux2/src/api/ipc/cmessagequeue.h`) |
| 6 | 3907 | 114 |  | `vota::CProgressoEncerramento::vf0` | `vota::CProgressoEncerramento::~CProgressoEncerramento` | `src/uenux2/src/app/vota/eleitor/fimvotacao/cprogressoencerramento.cpp` (`uenux2/src/app/vota/eleitor/fimvotacao/cprogressoencerramento.cpp`) |
| 7 | 7178 | 97 | ✓ | `vota::CSincronismoEleitor::vf2` | `vota::CSincronismoEleitor::StartState` | `src/uenux2/src/app/vota/eleitor/csincronismoeleitor.cpp` (`uenux2/src/app/vota/eleitor/csincronismoeleitor.cpp`) |
| 8 | 7823 | 38 | ✓ | `vota::CExecucaoVotaCooperativa::Processa` (formerly shown by the tools as `vf7`) | `vota::CExecucaoVotaCooperativa::Processa` | `src/uenux2/mock/app/vota/cexecucaovotacooperativa.cpp` (`uenux2/mock/app/vota/cexecucaovotacooperativa.cpp`) |
| 9 | 7828 | 21 |  | `vota::CExecucaoVotaCooperativa::CExecucaoVotaCooperativa` | `vota::CExecucaoVotaCooperativa::CExecucaoVotaCooperativa` | `src/uenux2/mock/app/vota/cexecucaovotacooperativa.cpp` (`uenux2/mock/app/vota/cexecucaovotacooperativa.cpp`) |
| 10 | 10217 | 191 |  | `vota::CAguardaInicio::vf6` | `vota::CAguardaInicio::ProcessMessage` | `src/uenux2/src/app/vota/operador/caguardainicio.cpp` (`uenux2/src/app/vota/operador/caguardainicio.cpp`) |
| 11 | 10218 | 31 |  | `vota::CAguardaInicio::vf8` | `vota::CAguardaInicio::ProcessTick` | `src/uenux2/src/app/vota/operador/caguardainicio.cpp` (`uenux2/src/app/vota/operador/caguardainicio.cpp`) |
| 12 | 10219 | 13 |  | `vota::CAguardaInicio::vf5` | `vota::CAguardaInicio::FinishState` | `src/uenux2/src/app/vota/operador/caguardainicio.cpp` (`uenux2/src/app/vota/operador/caguardainicio.cpp`) |
| 13 | 10220 | 38 |  | `vota::CAguardaInicio::vf2` | `vota::CAguardaInicio::StartState` | `src/uenux2/src/app/vota/operador/caguardainicio.cpp` (`uenux2/src/app/vota/operador/caguardainicio.cpp`) |
| 14 | 10232 | 18 |  | `vota::CExecucaoVota::vf6` | `vota::CExecucaoVota::FinalizaThreads` | `src/uenux2/src/app/vota/cexecucaovota.cpp` (`uenux2/src/app/vota/cexecucaovota.cpp`) |
| 15 | 10233 | 8 |  | `vota::CExecucaoVota::vf5` | `vota::CExecucaoVota::IniciaOperador` | `src/uenux2/src/app/vota/cexecucaovota.cpp` (`uenux2/src/app/vota/cexecucaovota.cpp`) |
| 16 | 10419 | 85 |  | `vota::CPerguntaCodigoSuspensao::vf6` | `vota::CPerguntaCodigoSuspensao::ProcessMessage` | `src/uenux2/src/app/vota/operador/aguardaeleitor/cperguntacodigosuspensao.cpp` (`uenux2/src/app/vota/operador/aguardaeleitor/cperguntacodigosuspensao.cpp`) |
| 17 | 10421 | 207 |  | `vota::CPerguntaCodigoSuspensao::vf2` | `vota::CPerguntaCodigoSuspensao::StartState` | `src/uenux2/src/app/vota/operador/aguardaeleitor/cperguntacodigosuspensao.cpp` (`uenux2/src/app/vota/operador/aguardaeleitor/cperguntacodigosuspensao.cpp`) |
| 18 | 10436 | 200 |  | `vota::CDesabilitaAudioEleitor::vf2` | `vota::CDesabilitaAudioEleitor::StartState` | `src/uenux2/src/app/vota/operador/aguardaeleitor/cdesabilitaaudioeleitor.u39.cpp` (`uenux2/src/app/vota/operador/aguardaeleitor/cdesabilitaaudioeleitor.cpp`) |
| 19 | 10444 | 213 |  | `vota::CVerificaDadoEleitor::vf2` | `vota::CVerificaDadoEleitor::StartState` | `src/uenux2/src/app/vota/operador/confirmaidentidade/cverificadadoeleitor.cpp` (`uenux2/src/app/vota/operador/confirmaidentidade/cverificadadoeleitor.cpp`) |
| 20 | 10478 | 633 |  | `vota::CDigitalNaoReconhecidaDecBiometria::vf2` | `vota::CDigitalNaoReconhecidaDecBiometria::StartState` | `src/uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaoreconhecidadecbiometria.cpp` (`uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaoreconhecidadecbiometria.cpp`) |
| 21 | 10486 | 919 |  | `vota::CDigitalNaoReconhecidaPorTempo::vf2` | `vota::CDigitalNaoReconhecidaPorTempo::StartState` | `src/uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaoreconhecidaportempo.cpp` (`uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaoreconhecidaportempo.cpp`) |
| 22 | 10490 | 41 |  | `vota::CPedeAnoNascimentoSemBiometria::vf2` | `vota::CPedeAnoNascimentoSemBiometria::StartState` | `src/uenux2/src/app/vota/operador/confirmaidentidade/cpedeanonascimentosembiometria.cpp` (`uenux2/src/app/vota/operador/confirmaidentidade/cpedeanonascimentosembiometria.cpp`) |
| 23 | 10524 | 200 |  | `vota::CHabilitaAudioEleitor::vf2` | `vota::CHabilitaAudioEleitor::StartState` | `src/uenux2/src/app/vota/operador/confirmaidentidade/chabilitaaudioeleitor.cpp` (`uenux2/src/app/vota/operador/confirmaidentidade/chabilitaaudioeleitor.cpp`) |
| 24 | 10530 | 701 |  | `vota::CAguardaInspecao::vf6` | `vota::CAguardaInspecao::ProcessMessage` | `src/uenux2/src/app/vota/operador/aguardaeleitor/caguardainspecao.cpp` (`uenux2/src/app/vota/operador/aguardaeleitor/caguardainspecao.cpp`) |
| 25 | 10594 | 328 |  | `vota::CJustificativaEfetuada::vf2` | `vota::CJustificativaEfetuada::StartState` | `src/uenux2/src/app/vota/operador/justificativa/cjustificativaefetuada.cpp` (`uenux2/src/app/vota/operador/justificativa/cjustificativaefetuada.cpp`) |
| 26 | 10646 | 40 |  | `vota::CCancelaHabilitacaoEleitor::vf2` | `vota::CCancelaHabilitacaoEleitor::StartState` | `src/uenux2/src/app/vota/operador/comum/ccancelahabilitacaoeleitor.cpp` (`uenux2/src/app/vota/operador/comum/ccancelahabilitacaoeleitor.cpp`) |
| 27 | 10663 | 175 |  | `vota::CEleitorNaoEncontrado::vf9` | `vota::CEleitorNaoEncontrado::LogaEntrada` | `src/uenux2/src/app/vota/operador/leidentidade/celeitornaoencontrado.cpp` (`uenux2/src/app/vota/operador/leidentidade/celeitornaoencontrado.cpp`) |
| 28 | 10669 | 193 |  | `vota::CIdentidadeInvalida::vf2` | `vota::CIdentidadeInvalida::StartState` | `src/uenux2/src/app/vota/operador/leidentidade/cidentidadeinvalida.cpp` (`uenux2/src/app/vota/operador/leidentidade/cidentidadeinvalida.cpp`) |
| 29 | 10715 | 200 |  | `vota::CPerguntaFilaEleitorVazia::vf2` | `vota::CPerguntaFilaEleitorVazia::StartState` | `src/uenux2/src/app/vota/operador/outrasopcoes/cperguntafilaeleitorvazia.u39.cpp` (`uenux2/src/app/vota/operador/outrasopcoes/cperguntafilaeleitorvazia.cpp`) |
| 30 | 10737 | 79 |  | `vota::CFimAquisicaoVotos::vf2` | `vota::CFimAquisicaoVotos::StartState` | `src/uenux2/src/app/vota/operador/outrasopcoes/cfimaquisicaovotos.cpp` (`uenux2/src/app/vota/operador/outrasopcoes/cfimaquisicaovotos.cpp`) |
| 31 | 10751 | 370 |  | `vota::CHabilitaAudioManualmente::vf2` | `vota::CHabilitaAudioManualmente::StartState` | `src/uenux2/src/app/vota/operador/outrasopcoes/chabilitaaudiomanualmente.u39.cpp` (`uenux2/src/app/vota/operador/outrasopcoes/chabilitaaudiomanualmente.cpp`) |
| 32 | 10758 | 169 |  | `vota::CHorarioVotacaoTerminou::vf2` | `vota::CHorarioVotacaoTerminou::StartState` | `src/uenux2/src/app/vota/operador/outrasopcoes/chorariovotacaoterminou.u39.cpp` (`uenux2/src/app/vota/operador/outrasopcoes/chorariovotacaoterminou.cpp`) |
| 33 | 10766 | 31 |  | `vota::CControladorRegistraMesariosVota::vf36` | `vota::CControladorRegistraMesariosVota::LogaIndagadoFinalizarRegistro` | `src/uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp` (`uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp`) |
| 34 | 10767 | 175 |  | `vota::CControladorRegistraMesariosVota::vf35` | `vota::CControladorRegistraMesariosVota::LogaIndagadoContinuarRegistro` | `src/uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp` (`uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp`) |
| 35 | 10768 | 31 |  | `vota::CControladorRegistraMesariosVota::vf34` | `vota::CControladorRegistraMesariosVota::LogaConferenciaBiometria` | `src/uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp` (`uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp`) |
| 36 | 10769 | 15 |  | `vota::CControladorRegistraMesariosVota::vf33` | `vota::CControladorRegistraMesariosVota::LogaMesarioNaoEhEleitor` | `src/uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp` (`uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp`) |
| 37 | 10770 | 15 |  | `vota::CControladorRegistraMesariosVota::vf32` | `vota::CControladorRegistraMesariosVota::LogaMesarioEhEleitor` | `src/uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp` (`uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp`) |
| 38 | 10771 | 13 |  | `vota::CControladorRegistraMesariosVota::vf31` | `vota::CControladorRegistraMesariosVota::LogaPedidoLeituraBiometria` | `src/uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp` (`uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp`) |
| 39 | 10772 | 570 |  | `vota::CControladorRegistraMesariosVota::vf30` | `vota::CControladorRegistraMesariosVota::LogaDigitalNaoCorresponde` | `src/uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp` (`uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp`) |
| 40 | 10773 | 31 |  | `vota::CControladorRegistraMesariosVota::vf29` | `vota::CControladorRegistraMesariosVota::LogaEncerrouRegistro` | `src/uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp` (`uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp`) |
| 41 | 10775 | 17 |  | `vota::CControladorRegistraMesariosVota::vf27` | `vota::CControladorRegistraMesariosVota::LogaMesarioJaRegistrado` | `src/uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp` (`uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp`) |
| 42 | 10776 | 180 |  | `vota::CControladorRegistraMesariosVota::vf26` | `vota::CControladorRegistraMesariosVota::LogaTituloInvalido` | `src/uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp` (`uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp`) |
| 43 | 10777 | 151 |  | `vota::CControladorRegistraMesariosVota::vf25` | `vota::CControladorRegistraMesariosVota::LogaCancelouRegistro` | `src/uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp` (`uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp`) |
| 44 | 10778 | 163 |  | `vota::CControladorRegistraMesariosVota::vf24` | `vota::CControladorRegistraMesariosVota::LogaConfirmouRegistro` | `src/uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp` (`uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp`) |
| 45 | 10779 | 180 |  | `vota::CControladorRegistraMesariosVota::vf23` | `vota::CControladorRegistraMesariosVota::LogaIndagadoRegistro` | `src/uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp` (`uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp`) |
| 46 | 10781 | 151 |  | `vota::CControladorRegistraMesariosVota::vf22` | `vota::CControladorRegistraMesariosVota::LogaRegistroAposVotacao` | `src/uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp` (`uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp`) |
| 47 | 10782 | 151 |  | `vota::CControladorRegistraMesariosVota::vf21` | `vota::CControladorRegistraMesariosVota::LogaRegistroDuranteVotacao` | `src/uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp` (`uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp`) |
| 48 | 10783 | 151 |  | `vota::CControladorRegistraMesariosVota::vf20` | `vota::CControladorRegistraMesariosVota::LogaRegistroAntesVotacao` | `src/uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp` (`uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp`) |
| 49 | 10784 | 17 |  | `vota::CControladorRegistraMesariosVota::vf19` | `vota::CControladorRegistraMesariosVota::LogaMesarioRegistrado` | `src/uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp` (`uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp`) |
| 50 | 10789 | 17 |  | `vota::CControladorRegistraMesariosVota::vf14` | `vota::CControladorRegistraMesariosVota::GetEleitorMesario` | `src/uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp` (`uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp`) |
| 51 | 10790 | 20 |  | `vota::CControladorRegistraMesariosVota::vf13` | `vota::CControladorRegistraMesariosVota::MesarioEhEleitorDaSecao` | `src/uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp` (`uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp`) |
| 52 | 10791 | 5 |  | `vota::CControladorRegistraMesariosVota::vf12` | `vota::CControladorRegistraMesariosVota::GetEstadoAposRegistroFinal` | `src/uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp` (`uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp`) |
| 53 | 10793 | 89 |  | `vota::CControladorRegistraMesariosVota::vf11` | `vota::CControladorRegistraMesariosVota::GetEstadoAposRegistroVotacao` | `src/uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp` (`uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp`) |
| 54 | 10795 | 64 |  | `vota::CControladorRegistraMesariosVota::vf7` | `vota::CControladorRegistraMesariosVota::LimiteMesariosAtingido` | `src/uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp` (`uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp`) |
| 55 | 10796 | 46 |  | `vota::CControladorRegistraMesariosVota::vf6` | `vota::CControladorRegistraMesariosVota::GetPeriodoRegistro` | `src/uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp` (`uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp`) |
| 56 | 10798 | 10 |  | `vota::CControladorRegistraMesariosVota::vf4` | `vota::CControladorRegistraMesariosVota::StartTick` | `src/uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp` (`uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp`) |
| 57 | 10799 | 10 |  | `vota::CControladorRegistraMesariosVota::vf3` | `vota::CControladorRegistraMesariosVota::StopTick` | `src/uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp` (`uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp`) |
| 58 | 10800 | 13 |  | `vota::CControladorRegistraMesariosVota::vf2` | `vota::CControladorRegistraMesariosVota::CriaTick` | `src/uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp` (`uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.cpp`) |
| 59 | 11701 | 52 |  | `vota::CProporcionalNulo::vf15` | `vota::CProporcionalNulo::GetMensagemAudio` | `src/uenux2/src/app/vota/eleitor/votaproporcional/cproporcionalnulo.cpp` (`uenux2/src/app/vota/eleitor/votaproporcional/cproporcionalnulo.cpp`) |
| 60 | 11704 | 51 |  | `vota::CProporcionalBranco::vf15` | `vota::CProporcionalBranco::GetMensagemAudio` | `src/uenux2/src/app/vota/eleitor/votaproporcional/cproporcionalbranco.cpp` (`uenux2/src/app/vota/eleitor/votaproporcional/cproporcionalbranco.cpp`) |
| 61 | 11725 | 52 |  | `vota::CPedeNominal::vf15` | `vota::CPedeNominal::GetMensagemAudio` | `src/uenux2/src/app/vota/eleitor/votaproporcional/cpedenominal.cpp` (`uenux2/src/app/vota/eleitor/votaproporcional/cpedenominal.cpp`) |
| 62 | 11728 | 51 |  | `vota::CConfirmaVotoNominal::vf15` | `vota::CConfirmaVotoNominal::GetMensagemAudio` | `src/uenux2/src/app/vota/eleitor/votaproporcional/cconfirmavotonominal.cpp` (`uenux2/src/app/vota/eleitor/votaproporcional/cconfirmavotonominal.cpp`) |
| 63 | 11731 | 52 |  | `vota::CCandidatoInexistente::vf15` | `vota::CCandidatoInexistente::GetMensagemAudio` | `src/uenux2/src/app/vota/eleitor/votaproporcional/ccandidatoinexistente.cpp` (`uenux2/src/app/vota/eleitor/votaproporcional/ccandidatoinexistente.cpp`) |
| 64 | 11735 | 52 |  | `vota::CConfirmaVotoLegenda::vf15` | `vota::CConfirmaVotoLegenda::GetMensagemAudio` | `src/uenux2/src/app/vota/eleitor/votaproporcional/cconfirmavotolegenda.cpp` (`uenux2/src/app/vota/eleitor/votaproporcional/cconfirmavotolegenda.cpp`) |
| 65 | 11748 | 52 |  | `vota::CCandidatoInapto::vf15` | `vota::CCandidatoInapto::GetMensagemAudio` | `src/uenux2/src/app/vota/eleitor/votaproporcional/ccandidatoinapto.cpp` (`uenux2/src/app/vota/eleitor/votaproporcional/ccandidatoinapto.cpp`) |
| 66 | 11789 | 73 |  | `vota::IConfereVotoEmCargo::vf15` | `vota::IConfereVotoEmCargo::GetMensagemAudio` | `src/uenux2/src/app/vota/eleitor/cconferevotoemcargo.u39.cpp` (`uenux2/src/app/vota/eleitor/cconferevotoemcargo.cpp`) |
| 67 | 11790 | 131 | ✓ | `vota::IConfereVotoEmCargo::vf12` | `vota::IConfereVotoEmCargo::ProcessTickAudio` | `src/uenux2/src/app/vota/eleitor/cconferevotoemcargo.u39.cpp` (`uenux2/src/app/vota/eleitor/cconferevotoemcargo.cpp`) |
| 68 | 11792 | 86 |  | `vota::IConfereVotoEmCargo::vf11` | `vota::IConfereVotoEmCargo::FinishStateAudio` | `src/uenux2/src/app/vota/eleitor/cconferevotoemcargo.u39.cpp` (`uenux2/src/app/vota/eleitor/cconferevotoemcargo.cpp`) |
| 69 | 11798 | 164 |  | `vota::CInspecionaUrna::vf2` | `vota::CInspecionaUrna::StartState` | `src/uenux2/src/app/vota/eleitor/cinspecionaurna.cpp` (`uenux2/src/app/vota/eleitor/cinspecionaurna.cpp`) |
| 70 | 11803 | 958 |  | `vota::testeteclado::impl::CGeradorTeclasAleatorio::vf2` | `vota::testeteclado::impl::CGeradorTeclasAleatorio::GeraSequencia` | `src/uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cgeradorteclasaleatorio.cpp` (`uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cgeradorteclasaleatorio.cpp`) |
| 71 | 11824 | 86 |  | `vota::testeteclado::CEsperaRetestar::vf5` | `vota::testeteclado::CEsperaRetestar::FinishState` | `src/uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cesperaretestar.cpp` (`uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cesperaretestar.cpp`) |
| 72 | 11825 | 147 |  | `vota::testeteclado::CEsperaRetestar::vf2` | `vota::testeteclado::CEsperaRetestar::StartState` | `src/uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cesperaretestar.cpp` (`uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cesperaretestar.cpp`) |
| 73 | 11874 | 352 |  | `vota::testeteclado::CBase::vf7` | `vota::testeteclado::CBase::ProcessInput` | `src/uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cbase.cpp` (`uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cbase.cpp`) |
| 74 | 11875 | 188 |  | `vota::testeteclado::CBase::vf2` | `vota::testeteclado::CBase::StartState` | `src/uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cbase.cpp` (`uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cbase.cpp`) |
| 75 | 11897 | 182 |  | `vota::CMaisInformacoes::vf2` | `vota::CMaisInformacoes::StartState` | `src/uenux2/src/app/vota/eleitor/iniciovotacao/cmaisinformacoes.u39.cpp` (`uenux2/src/app/vota/eleitor/iniciovotacao/cmaisinformacoes.cpp`) |
| 76 | 11921 | 35 |  | `vota::CQuerImprimirZeresima::vf2` | `vota::CQuerImprimirZeresima::StartState` | `src/uenux2/src/app/vota/eleitor/iniciovotacao/cquerimprimirzeresima.cpp` (`uenux2/src/app/vota/eleitor/iniciovotacao/cquerimprimirzeresima.cpp`) |
| 77 | 11932 | 188 |  | `vota::CImpressaoZeresimaTardia::vf2` | `vota::CImpressaoZeresimaTardia::StartState` | `src/uenux2/src/app/vota/eleitor/iniciovotacao/cimpressaozeresimatardia.cpp` (`uenux2/src/app/vota/eleitor/iniciovotacao/cimpressaozeresimatardia.cpp`) |
| 78 | 12021 | 10 |  | `vota_f12021` | `__dtor_vota::CExibeAlertaDesligamento::s_mutex` | `src/uenux2/src/app/vota/eleitor/u39-foreign-fragments.cpp` (`uenux2/src/app/vota/eleitor/iniciovotacao/cexibealertadesligamento.cpp`) |
| 79 | 12024 | 10 |  | `vota_f12024` | `__dtor_vota::CVerificaQtdBUsAdicionais::s_mutex` | `src/uenux2/src/app/vota/eleitor/u39-foreign-fragments.cpp` (`uenux2/src/app/vota/eleitor/fimvotacao/cverificaqtdbusadicionais.cpp`) |
| 80 | 12025 | 12 |  | `vota_f12025` | `__dtor_vota::CVerificaQtdBUsAdicionais::s_instancia` | `src/uenux2/src/app/vota/eleitor/u39-foreign-fragments.cpp` (`uenux2/src/app/vota/eleitor/fimvotacao/cverificaqtdbusadicionais.cpp`) |
| 81 | 12028 | 10 |  | `vota_f12028` | `__dtor_vota::CLimiteCopiasBUAtingido::s_mutex` | `src/uenux2/src/app/vota/eleitor/u39-foreign-fragments.cpp` (`uenux2/src/app/vota/eleitor/fimvotacao/climitecopiasbuatingido.cpp`) |
| 82 | 12029 | 12 |  | `vota_f12029` | `__dtor_vota::CLimiteCopiasBUAtingido::s_instancia` | `src/uenux2/src/app/vota/eleitor/u39-foreign-fragments.cpp` (`uenux2/src/app/vota/eleitor/fimvotacao/climitecopiasbuatingido.cpp`) |
| 83 | 12037 | 12 |  | `vota_f12037` | `__dtor_vota::CQuerImprimirBU::s_instancia` | `src/uenux2/src/app/vota/eleitor/u39-foreign-fragments.cpp` (`uenux2/src/app/vota/eleitor/fimvotacao/cquerimprimirbu.cpp`) |
| 84 | 12041 | 10 |  | `vota_f12041` | `__dtor_vota::CMostraQRCodeCertificado::s_mutex` | `src/uenux2/src/app/vota/eleitor/u39-foreign-fragments.cpp` (`uenux2/src/app/vota/eleitor/fimvotacao/cmostraqrcodecertificado.cpp`) |
| 85 | 12042 | 12 |  | `vota_f12042` | `__dtor_vota::CMostraQRCodeCertificado::s_instancia` | `src/uenux2/src/app/vota/eleitor/u39-foreign-fragments.cpp` (`uenux2/src/app/vota/eleitor/fimvotacao/cmostraqrcodecertificado.cpp`) |
| 86 | 12049 | 119 |  | `vota::IQRCodeBUDS::vf1` | `vota::IQRCodeBUDS::~IQRCodeBUDS (deleting)` | `src/uenux2/src/app/vota/eleitor/fimvotacao/cmostraqrcodebu.u39.cpp` (`uenux2/src/app/vota/eleitor/fimvotacao/cmostraqrcodebu.cpp`) |
| 87 | 12050 | 116 |  | `vota::IQRCodeBUDS::vf0` | `vota::IQRCodeBUDS::~IQRCodeBUDS` | `src/uenux2/src/app/vota/eleitor/fimvotacao/cmostraqrcodebu.u39.cpp` (`uenux2/src/app/vota/eleitor/fimvotacao/cmostraqrcodebu.cpp`) |
| 88 | 12058 | 152 |  | `vota::CAplicacaoEncerrada::vf2` | `vota::CAplicacaoEncerrada::StartState` | `src/uenux2/src/app/vota/eleitor/fimvotacao/caplicacaoencerrada.cpp` (`uenux2/src/app/vota/eleitor/fimvotacao/caplicacaoencerrada.cpp`) |
| 89 | 12066 | 10 |  | `vota_f12066` | `__dtor_vota::CImprimirBUOutrasObrigatorias::s_mutex` | `src/uenux2/src/app/vota/eleitor/u39-foreign-fragments.cpp` (`uenux2/src/app/vota/eleitor/fimvotacao/cimprimirbuoutrasobrigatorias.cpp`) |
| 90 | 12067 | 12 |  | `vota_f12067` | `__dtor_vota::CImprimirBUOutrasObrigatorias::s_instancia` | `src/uenux2/src/app/vota/eleitor/u39-foreign-fragments.cpp` (`uenux2/src/app/vota/eleitor/fimvotacao/cimprimirbuoutrasobrigatorias.cpp`) |
| 91 | 12069 | 10 |  | `vota_f12069` | `__dtor_vota::CImprimirBJust::s_mutex` | `src/uenux2/src/app/vota/eleitor/u39-foreign-fragments.cpp` (`uenux2/src/app/vota/eleitor/fimvotacao/cimprimirbjust.cpp`) |
| 92 | 12070 | 12 |  | `vota_f12070` | `__dtor_vota::CImprimirBJust::s_instancia` | `src/uenux2/src/app/vota/eleitor/u39-foreign-fragments.cpp` (`uenux2/src/app/vota/eleitor/fimvotacao/cimprimirbjust.cpp`) |
| 93 | 12075 | 10 |  | `vota_f12075` | `__dtor_vota::CImprimindoBEHB::s_mutex` | `src/uenux2/src/app/vota/eleitor/u39-foreign-fragments.cpp` (`uenux2/src/app/vota/eleitor/fimvotacao/cimprimindobehb.cpp`) |
| 94 | 12076 | 12 |  | `vota_f12076` | `__dtor_vota::CImprimindoBEHB::s_instancia` | `src/uenux2/src/app/vota/eleitor/u39-foreign-fragments.cpp` (`uenux2/src/app/vota/eleitor/fimvotacao/cimprimindobehb.cpp`) |
| 95 | 12079 | 10 |  | `vota_f12079` | `__dtor_vota::CImprimindoBU::s_mutex` | `src/uenux2/src/app/vota/eleitor/u39-foreign-fragments.cpp` (`uenux2/src/app/vota/eleitor/fimvotacao/cimprimindobu.cpp`) |
| 96 | 12080 | 12 |  | `vota_f12080` | `__dtor_vota::CImprimindoBU::s_instancia` | `src/uenux2/src/app/vota/eleitor/u39-foreign-fragments.cpp` (`uenux2/src/app/vota/eleitor/fimvotacao/cimprimindobu.cpp`) |
| 97 | 12100 | 12 |  | `vota_f12100` | `__dtor_vota::CGravaResultado::s_instancia` | `src/uenux2/src/app/vota/eleitor/u39-foreign-fragments.cpp` (`uenux2/src/app/vota/eleitor/fimvotacao/cgravaresultado.cpp`) |
| 98 | 12102 | 27 |  | `vota::CProgressoEncerramento::vf3` | `vota::CProgressoEncerramento::Finaliza` | `src/uenux2/src/app/vota/eleitor/fimvotacao/cprogressoencerramento.cpp` (`uenux2/src/app/vota/eleitor/fimvotacao/cprogressoencerramento.cpp`) |
| 99 | 12103 | 45 |  | `vota::CProgressoEncerramento::vf2` | `vota::CProgressoEncerramento::Inicia` | `src/uenux2/src/app/vota/eleitor/fimvotacao/cprogressoencerramento.cpp` (`uenux2/src/app/vota/eleitor/fimvotacao/cprogressoencerramento.cpp`) |
| 100 | 12104 | 13 |  | `vota::CProgressoEncerramento::vf1` | `vota::CProgressoEncerramento::~CProgressoEncerramento (deleting)` | `src/uenux2/src/app/vota/eleitor/fimvotacao/cprogressoencerramento.cpp` (`uenux2/src/app/vota/eleitor/fimvotacao/cprogressoencerramento.cpp`) |
| 101 | 12106 | 10 |  | `vota_f12106` | `__dtor_vota::CGeraRelatorios::s_mutex` | `src/uenux2/src/app/vota/eleitor/u39-foreign-fragments.cpp` (`uenux2/src/app/vota/eleitor/fimvotacao/cgerarelatorios.cpp`) |
| 102 | 12107 | 12 |  | `vota_f12107` | `__dtor_vota::CGeraRelatorios::s_instancia` | `src/uenux2/src/app/vota/eleitor/u39-foreign-fragments.cpp` (`uenux2/src/app/vota/eleitor/fimvotacao/cgerarelatorios.cpp`) |
| 103 | 12111 | 10 |  | `vota_f12111` | `__dtor_vota::CGeraBU::s_mutex` | `src/uenux2/src/app/vota/eleitor/u39-foreign-fragments.cpp` (`uenux2/src/app/vota/eleitor/fimvotacao/cgerabu.cpp`) |
| 104 | 12112 | 12 |  | `vota_f12112` | `__dtor_vota::CGeraBU::s_instancia` | `src/uenux2/src/app/vota/eleitor/u39-foreign-fragments.cpp` (`uenux2/src/app/vota/eleitor/fimvotacao/cgerabu.cpp`) |
| 105 | 12114 | 10 |  | `vota_f12114` | `__dtor_vota::CFinalizaAquisicao::s_mutex` | `src/uenux2/src/app/vota/eleitor/u39-foreign-fragments.cpp` (`uenux2/src/app/vota/eleitor/fimvotacao/cfinalizaaquisicao.cpp`) |
| 106 | 12115 | 12 |  | `vota_f12115` | `__dtor_vota::CFinalizaAquisicao::s_instancia` | `src/uenux2/src/app/vota/eleitor/u39-foreign-fragments.cpp` (`uenux2/src/app/vota/eleitor/fimvotacao/cfinalizaaquisicao.cpp`) |
| 107 | 12145 | 10 |  | `vota_f12145` | `__dtor_vota::CCopiaResultadoParaMR::s_mutex` | `src/uenux2/src/app/vota/eleitor/u39-foreign-fragments.cpp` (`uenux2/src/app/vota/eleitor/fimvotacao/ccopiaresultadoparamr.cpp`) |
| 108 | 12152 | 12 |  | `vota_f12152` | `__dtor_vota::CCopiaResultadoParaMR::s_instancia` | `src/uenux2/src/app/vota/eleitor/u39-foreign-fragments.cpp` (`uenux2/src/app/vota/eleitor/fimvotacao/ccopiaresultadoparamr.cpp`) |
| 109 | 12251 | 13 |  | `vota::CItemVisualizarCandidatosVota::vf1` | `vota::CItemVisualizarCandidatosVota::~CItemVisualizarCandidatosVota (deleting)` | `src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u39.cpp` (`uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp`) |
| 110 | 12255 | 11 |  | `vota::CItemParametrosUrnaVota::vf3` | `vota::CItemParametrosUrnaVota::GetNumViasImpressas` | `src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u39.cpp` (`uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp`) |
| 111 | 12259 | 11 |  | `vota::CItemVersoesPacotesVota::vf3` | `vota::CItemVersoesPacotesVota::GetNumViasImpressas` | `src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u39.cpp` (`uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp`) |
| 112 | 12266 | 11 |  | `vota::CItemImprimeListaEleitoresVota::vf3` | `vota::CItemImprimeListaEleitoresVota::GetNumViasImpressas` | `src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u39.cpp` (`uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp`) |
| 113 | 12267 | 11 | ✓ | `vota::CItemImprimeEstadoUrnaVota::vf3` | `vota::CItemImprimeEstadoUrnaVota::GetNumViasImpressas` | `src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u39.cpp` (`uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp`) |
| 114 | 12974 | 105 |  | `vota_f12974` | `vota::(anonymous namespace)::DS_NumeroMaximoCopiasBU` | `src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u39.cpp` (`uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp`) |
| 115 | 13023 | 375 |  | `vota_f13023` | `vota::(anonymous namespace)::DS_ViaAtualBU` | `src/uenux2/src/app/vota/eleitor/fimvotacao/cimprimirbuoutrasobrigatorias.u39.cpp` (`uenux2/src/app/vota/eleitor/fimvotacao/cimprimirbuoutrasobrigatorias.cpp`) |
| 116 | 13137 | 87 |  | `vota_f13137` | `vota::(anonymous namespace)::DS_ParaReiniciarEsteVoto` | `src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u39.cpp` (`uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp`) |
| 117 | 13176 | 7 |  | `vota_f13176` | `vota::(anonymous namespace)::VotoDigitado` | `src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u39.cpp` (`uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp`) |
| 118 | 13441 | 386 |  | `vota_f13441` | `vota::(anonymous namespace)::DS_IconeAudio` | `src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u39.cpp` (`uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp`) |
| 119 | 13574 | 10 |  | `vota_f13574` | `__dtor_vota::impl::IPoliticaExecucaoEleitor::GetInst::s_mutex` | `src/uenux2/src/app/vota/eleitor/u39-foreign-fragments.cpp` (`uenux2/src/app/vota/eleitor/comum/cpoliticaexecucaoeleitor.cpp`) |
| 120 | 13588 | 120 |  | `vota_f13588` | `__dtor_vota::g_votosEleitor` | `src/uenux2/src/app/vota/eleitor/u39-foreign-fragments.cpp` (`uenux2/src/app/vota/eleitor/celeitorvotando.cpp`) |
| 121 | 13593 | 36 |  | `vota_f13593` | `__dtor_vota::g_votoDigitado` | `src/uenux2/src/app/vota/eleitor/u39-foreign-fragments.cpp` (`uenux2/src/app/vota/eleitor/celeitorvotando.cpp`) |
| 122 | 13685 | 10 |  | `vota_f13685` | `__dtor_vota::CInformacaoEleitor::s_mutex` | `src/uenux2/src/app/vota/eleitor/u39-foreign-fragments.cpp` (`uenux2/src/app/vota/eleitor/comum/cinformacaoeleitor.cpp`) |
| 123 | 13695 | 12 |  | `vota_f13695` | `__dtor_vota::CInformacaoEleitor::s_instancia` | `src/uenux2/src/app/vota/eleitor/u39-foreign-fragments.cpp` (`uenux2/src/app/vota/eleitor/comum/cinformacaoeleitor.cpp`) |
| 124 | 14464 | 17 |  | `vota_f14464` | `__dtor_vota::CVotacaoStateAudio::FormataMensagem::digitos` | `src/uenux2/src/app/vota/eleitor/u39-foreign-fragments.cpp` (`uenux2/src/app/vota/eleitor/cvotacaostateaudio.cpp`) |

## 15. Fidelity review (2026-09-23)

Compared line by line with the decompiled code (and the WAT where the pseudo-code was ambiguous): 7178, 7823, 7828,
11790, 11792, 1717, 11789, 12267/12266/12259/12255/12251, 10486, 10478, 11803, 10530, 10217–10220, 10419, 10421,
10436, 10444, 10490, 10524, 10594, 10646, 10663, 10669, 10715, 10737, 10751, 10758, all 26 controller slots, 11874,
11875, 1559, 11824, 11825, 11897, 11921, 11932, 11798, 12058, 3907/12102–12104, 13023, 12974, 13137, 13176, 13441,
3157/3162, 12049/12050, 1720, the seven `GetMensagemAudio` texts and all 31 atexit stubs (each address was traced
back to the accessor that stores the owner's vtable). The vfN → method-name claims agree with the vtables
(`q.py cls`) and with the interface headers of u08/u22/u37. Branch conditions, constants, strings, offsets and
callee order match. The mapping table lists all 124 indices, and its tool names and sizes match `functions.tsv`.
Corrections:

* Intro: the unit groups 46 classes, not 44.
* §6: `CExecucaoVota` slot 3 (`Inicia`, 10235) does not join the threads. Only slot 2 (`Executa`, 10236 = Inicia +
  Aguarda) does.
* §7: the harness run does not cover the whole closing. It dies in `CGravaResultado` while writing `-hash.dat`
  (`IGenericFactory<IHash>` not registered). Steps 5–8 and the last result files come from the code only.
* §12.5: the backlog per vote is 2 + number of cargos, not a fixed 5. Unbounded growth is not reachable with
  the current page. Re-checked with `headless.mjs --voters 2`: no second voter starts without a reload.
* §11: the regex of 14464 is a second guarded function-local static.
* Sources: `cfimaquisicaovotos.cpp` called `comum::DeveRegistrarMesarios()`, but the `cappinfo.h` it includes
  declares func 2520 as `EhModoDemonstracaoSemTreinamentoEleitor`. `ccancelahabilitacaoeleitor.cpp` and
  `chabilitaaudiomanualmente.u39.cpp` declared anonymous-namespace helpers that nothing defines, so they would not
  link. They now call `IInformacaoThreadOperador` slots 6/7 directly (`GetAudioHabilitadoManualmente` /
  `SetAudioHabilitadoManualmente`, as in `iinformacaothreadoperador.h`). In `ctelasvota.u39.cpp`, `VotoDigitado`
  moved into the anonymous namespace where u07's `ctelasvota.cpp` declares it.
* Comments: the header of `CPerguntaCodigoSuspensao` said a valid título posts voter message 4. It posts message 2;
  message 4 is the CORRIGE abort. The base no-op of `IEleitorImpedidoVotar` slot 9 is ICF 218, not 425. In 10486
  the discarded `CControlaReconhecimento::GetInst()` calls come before the first two reads only. Func 5946 is the
  deleting destructor of `CPreZeresima`/`CRetomada`, not an exit-time reset.

Suspicious findings (§12): 1, 3, 4, 6 and 7 were confirmed in the code. For 3, the caller 10316 fills `scores[j++]`
only for the fingers present in the roll, in the finger order {1, 6, 2, 7} = PD, PE, ID, IE. For 1, the guard byte
@1584624 has no store in the module, and the glue's `_emscripten_sleep` calls `abort(...)`. For 2, the code does
log the mesário título in clear. The claims that `logd.dat` goes into `-log.jez` and that the TSE publishes urna
logs could not be checked against this unit's artefacts. 5 was confirmed only in its weak form, as corrected above.
9 was confirmed: func 1950 returns `!GetDemoMode() && cfg[400]`.
