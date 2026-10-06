# u39 — classes de `app:vota` sem arquivo conhecido: pequenos estados das duas threads, o controlador de registro de mesários, as políticas de execução e os stubs de saída

A unidade u39 tem **124 funções wasm** da aplicação VOTA (TSE `uenux2/src/app/vota`) cujas classes não carregam nenhum
registro de `std::source_location`, então as ferramentas não conseguiram associá-las a um arquivo-fonte. Elas pertencem a
46 classes (do modo como as ferramentas as agrupam; `CMessageEleitor` só empresta o nome ao destrutor compartilhado de
`CPriorityMessageQueue`), mais dois grupos de funções livres. A maioria são **métodos virtuais encontrados apenas por meio de
vtables** (`Class::vfN`), principalmente o `StartState` (slot 2 da vtable) de pequenos estados cujos outros métodos outras
unidades já reconstruíram. Também estão na unidade: 26 dos 37 slots do controlador de registro de mesários, as duas
políticas de execução de threads (urna vs. web), as mensagens de áudio de sete estados de votação, o embaralhador do teste
do teclado, cinco itens do menu "Mais informações", cinco fontes de dados de tela e **31 destrutores de saída gerados pelo
compilador** ("stubs atexit").

Apenas **4 das 124 funções executaram** durante os votos gravados (`analysis/runtime/*.functions.tsv`):

| func | o quê | por que executa na página web |
|---|---|---|
| 7823 | `CExecucaoVotaCooperativa::Processa` | todo `votaTick` (≈60/s) faz a thread do eleitor avançar por ela |
| 11790 | `IConfereVotoEmCargo::ProcessTickAudio` | o fim de cada tela "Confira o seu voto" (conferência) |
| 7178 | `CSincronismoEleitor::StartState` | fim de cada voto ("Gravando") |
| 12267 | `CItemImprimeEstadoUrnaVota::GetNumViasImpressas` | o menu "Mais informações" é montado na inicialização |

Os estados da thread do operador (cerca de metade da unidade) nunca executam na página, porque o simulador não faz a
thread do mesário avançar (u10 §2). A **execução gravada do harness do operador** (`tools/bu/operator_harness.mjs`,
`samples/bu-real/run-full/{states,logd}.txt`) passa, sim, por vários deles. Ela confirma
`CAguardaInicio` → `CPedeIdentidade` com o registro de log "Urna pronta para receber votos" (func 10217),
`CPedeAnoNascimentoSemBiometria` (10490, sem registro de log), `CPerguntaFilaEleitorVazia` ("Operador indagado se
todas as pessoas presentes votaram", 10715) e `CFimAquisicaoVotos` → `CFinalizaOperador` + mensagem 7 do eleitor →
"Inicio do Encerramento" (10737). Em outras palavras, este é o início do BOLETIM DE URNA.

Todo caminho original nesta unidade é **inferido**, a menos que esteja marcado como *attested*. O diretório vem das
parentes mais próximas e dos caminhos que outras unidades já inferiram para a mesma classe (mantidos idênticos para que os
fragmentos se juntem). O nome da classe dá o nome do arquivo (convenção do TSE `CFooBar` → `cfoobar.{h,cpp}`).

## 1. Glossário

| termo | significado |
|---|---|
| urna | a urna eletrônica |
| eleitor / terminal do eleitor (TE) | quem vota / a tela + teclado do eleitor (thread do eleitor `CThreadEleitor`) |
| mesário / terminal do mesário (MT) | membro da mesa receptora / o seu microterminal LCD de 4 x 40 com teclado (thread do operador `CThreadOperador`) |
| habilitar / habilitação | o mesário identifica o eleitor e libera a urna para ele |
| título (de eleitor) | número de inscrição do eleitor (12 dígitos); também usado para identificar mesários |
| biometria / digital / dedo | dados de impressões digitais / uma impressão digital / dedo |
| decifrar | decriptar (os templates de impressão digital no cadastro de eleitores são cifrados) |
| justificativa (eleitoral) | declaração de ausência feita em qualquer urna por um eleitor fora da sua seção |
| suspender (a votação do eleitor) | encerrar a sessão de um eleitor que não termina de votar |
| cabina / inspeção | cabine de votação / a inspeção periódica da cabine que o mesário deve fazer |
| comparecimento de mesários | registro de presença dos mesários (SQLite `comparecimento_mesario`) |
| encerramento / fim da aquisição de votos | fechamento da urna / fim da coleta de votos |
| BU (boletim de urna), via | o relatório de resultado por urna; uma cópia impressa dele |
| BUJ, BIM, BEHB | boletins de justificativas, de identificação de mesários e de eleitores liberados sem impressão digital ("habilitados biograficamente") |
| zerésima | relatório impresso antes da votação mostrando zero votos para todos |
| RDV (registro digital do voto) | a tabela embaralhada dos votos dados, fonte do BU |
| MR (mídia de resultado) | a memória removível que leva os arquivos de resultado |
| teste do teclado | teste do teclado do terminal do eleitor, oferecido antes da zerésima e após um reinício |
| estadoVota | `EstadoGeralVota.estadoVota` em `vota.bin`: a fase da aplicação (valor C++ = valor ASN.1 + `'1'`) |
| áudio | o voto acessível: cada tela é lida em voz alta (RHVoice) nos fones de ouvido |

## 2. Conteúdo e arquivos-fonte reconstruídos

| assunto | classes | § |
|---|---|---|
| estados do operador (MT) | `CAguardaInicio`, `CAguardaInspecao` (+ `CConfirmaInspecionada` inlinado), `CPerguntaCodigoSuspensao`, `CDesabilitaAudioEleitor`, `CHabilitaAudioEleitor`, `CVerificaDadoEleitor`, `CDigitalNaoReconhecidaDecBiometria`, `CDigitalNaoReconhecidaPorTempo`, `CPedeAnoNascimentoSemBiometria`, `CCancelaHabilitacaoEleitor`, `CIdentidadeInvalida`, `CEleitorNaoEncontrado`, `CJustificativaEfetuada`, `CPerguntaFilaEleitorVazia`, `CHabilitaAudioManualmente`, `CHorarioVotacaoTerminou`, `CFimAquisicaoVotos` | 4 |
| controlador de registro de mesários | `CControladorRegistraMesariosVota` (26 de 37 slots) | 5 |
| políticas de execução | `CExecucaoVota` (urna), `CExecucaoVotaCooperativa` (web) | 6 |
| estados do eleitor | `CSincronismoEleitor`, `CInspecionaUrna`, `CAplicacaoEncerrada`, `CQuerImprimirZeresima`, `CImpressaoZeresimaTardia`, `CMaisInformacoes`, `testeteclado::CBase`, `testeteclado::CEsperaRetestar`, `CGeraZeresimaBase` (destrutor) | 3 |
| áudio das telas de voto | `IConfereVotoEmCargo` (4 slots); `GetMensagemAudio` de `CPedeNominal`, `CConfirmaVotoNominal`, `CConfirmaVotoLegenda`, `CCandidatoInexistente`, `CCandidatoInapto`, `CProporcionalBranco`, `CProporcionalNulo` | 3.2 |
| teste do teclado | `testeteclado::impl::CGeradorTeclasAleatorio` | 3.5 |
| telas | `CItem*Vota` (5 itens de menu), 4 fontes de dados de `CTelasVota`, "Via nº N", `CProgressoEncerramento`, `IQRCodeBUDS` (destrutor) | 3.6, 7 |
| destrutores de saída | 31 stubs `__dtor_…` + os destrutores da fila de mensagens (`CMessageEleitor::vf0/vf1`) | 8 |

**Arquivos-fonte reconstruídos** (todos novos, exceto os marcados como *fragment*; os fragmentos `.uNN` são as partes de um
arquivo que pertencem à classe de outra unidade ou cuja classe é declarada em outro lugar):

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

Escolhas de caminho novas (não tiradas de outra unidade): `cexecucaovotacooperativa` em **`uenux2/mock/app/vota/`**.
Essa classe exclusiva da web tem a sua vtable (@1532252) emitida logo depois da última classe de `mock/app/simulador/wasm`
(`simulador::CWasmTimerScheduler`) e logo antes da primeira classe de `src/app/vota` (`vota::CAssinadorVota`). A árvore
mock espelha `src/app` (`mock/app/comum/cappinfobuilder.cpp` é atestado). u18 propôs `mock/app/simulador/wasm/`.
Também são novos: `cfimaquisicaovotos` em `operador/outrasopcoes` (alcançado a partir de `CConfirmaEncerramento`, e a sua
vtable fica entre as classes desse diretório), `cimpressaozeresimatardia` em `eleitor/iniciovotacao` (ao lado de
`cinformacaozeresimatardia.cpp`), `cgeradorteclasaleatorio` em `testeteclado/`, e
`cfinalizaaquisicao.cpp` em `eleitor/fimvotacao` (para os seus stubs). As classes `CItem*Vota` foram colocadas **dentro de
ctelasvota.cpp** porque as vtables delas, a de `CMenuMaisInformacoesVota` e até a de `comum::CItemMenu` são emitidas nos
dados dessa unidade de tradução. Para `CControladorRegistraMesariosVota`, mantive `vota/comum/` (u20, u27); u18 usou
`operador/comparecimentomesario/`.

## 2.1 Hierarquia de classes (RTTI)

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

Todos os estados `comum::CAppState` são singletons lazy: um `std::mutex` estático mais um `std::unique_ptr` estático logo
depois dele. O wasm-opt mesclou muitos acessores em corpos compartilhados (`vota_f764(mutex, &ptr, vtable, flags)`: `CFimAquisicaoVotos`
5428, `CCancelaHabilitacaoEleitor` 1536, `CFinalizaOperador` 5342, …). A maioria está inlinada no primeiro estado que
precisa deles, por exemplo `CConfirmaInspecionada` dentro de `CAguardaInspecao::ProcessMessage`.

## 3. Peças da thread do eleitor

### 3.1 Fim de um voto: `CSincronismoEleitor::StartState` (7178, observada)

`CEleitorVotando::GravaVotos` (4454) coloca a cédula no RDV em memória e passa para `CSincronismoEleitor`
(12 B, `CAppState(1)`: só mensagens). `StartState`:

1. `CThreadOperador::GetInst().GetFila().Add(SMessage{13, &fila}, 1)`. Esta é a mensagem **13** do operador
   (`EMensagemOperadorRecebida::SincronizaVoto`).
2. `m_proximoEstado = this`.
3. `CTelasVota::m_barraProgresso` (+180, um `CProgressBar` de 4 passos) é reposto no seu mínimo (func 3667), e
   `m_telaProgressoRegistroVoto` (+172, "Gravando") é mostrada.

Na urna, a thread do operador (`CSincronismoOperador`) marca o eleitor como VOTOU e responde com a mensagem 5 do eleitor.
`ProcessMessage(5)` (7181, u19) então chama `ISincronismoVotoEleitor::SincronizaVoto()`, que grava o RDV e avança a
barra, e segue para "FIM". **Build web:** ninguém lê a mensagem 13 (a fila mantém `6, 9, 9, 13, 1` depois de um
voto, u10). O próprio `votaTick` posta a mensagem 5 (vota_web_wasm.u30.cpp), e `SincronizaVoto` é o no-op
`CSincronismoVotoEleitorWeb`, então o voto nunca é gravado no RDV.

### 3.2 Conferência e as frases de áudio

`IConfereVotoEmCargo` (u06) é a breve tela "Confira o seu voto" que segue o último dígito. Quatro dos seus slots estão
nesta unidade:

| slot | func | comportamento |
|---|---|---|
| 0 destrutor | 1717 | cancela uma espera pendente "depois do áudio atual" (`m_espera`, +32), depois `~CVotacaoStateAudio` |
| 11 `FinishStateAudio` | 11792 | o mesmo cancelamento (para que um callback tardio de "áudio terminou" não possa trocar de estado) |
| 12 `ProcessTickAudio(tick)` | 11790 | se `m_comTick` (+29) e `tick == m_tick` (+28): `StopTick`, depois `ExecutarAposAudioAtual([this]{ m_proximoEstado = GetProximoEstado(); })` (tela de confirmação quando o áudio termina) |
| 15 `GetMensagemAudio` | 11789 | `"Confira o seu voto."` |

Até o tick disparar, toda tecla é recusada ("Tecla indevida pressionada", u06). É por isso que as execuções headless
precisam de uma pausa antes de CONFIRMA (analysis/runtime/README.md).

O slot 15 dos estados proporcionais retorna a frase que o RHVoice lê para os eleitores que usam fones de ouvido. As tags são
expandidas por `CVotacaoStateAudio::FormataMensagem` (u08). Textos exatos, Latin-1 no binário:

| func | estado | frase |
|---|---|---|
| 11725 | `CPedeNominal` (dígitos do partido digitados, faltam os dígitos do candidato) | Você está votando para {cargo-atual}. Digite os demais dígitos do seu candidato, ou aperte confirma para prosseguir, ou corrige para reiniciar este voto. |
| 11728 | `CConfirmaVotoNominal` | Você está votando para {cargo-atual} n{o/a} candidat{o/a} {voto}: {candidato}. Aperte confirma ou corrige. |
| 11735 | `CConfirmaVotoLegenda` (voto de legenda) | Você está votando para {cargo-atual} no número {voto}. Aperte confirma para votar na legenda {legenda}, {nome-partido}, ou corrige para reiniciar o seu voto. |
| 11731 | `CCandidatoInexistente` | Você está votando para {cargo-atual} no número {voto}. Candidato inexistente. Aperte confirma para votar na legenda {legenda}, {nome-partido}, ou corrige para reiniciar este voto. |
| 11748 | `CCandidatoInapto` | Você está votando para {cargo-atual} n{o/a} candidat{o/a} {voto}. Candidat{o/a} não concorre. Se apertar confirma, este voto será nulo. Aperte confirma ou corrige. |
| 11704 | `CProporcionalBranco` | Você está votando em branco para {cargo-atual}. Aperte confirma ou corrige. |
| 11701 | `CProporcionalNulo` | Você está votando para {cargo-atual} no candidato {voto}. Número errado. Se apertar confirma, este voto será nulo. Aperte confirma ou corrige. |

Os construtores (corpo mesclado 1961, u06) fixam a tela e o tipo de voto no RDV: nominal (2) para `CConfirmaVotoNominal`,
**legenda (1)** para `CConfirmaVotoLegenda` *e* `CCandidatoInexistente` (um candidato inexistente de um partido existente
conta para o partido, como diz a frase), nulo (4) para `CCandidatoInapto`/`CProporcionalNulo`, branco (3).

### 3.3 Inspeção da cabine (lado do eleitor)

`CInspecionaUrna::StartState` (11798): mostra "Por favor, inspecione cabina e urna." e registra no log "Inspeção da urna
iniciada". A thread do operador inicia o procedimento (mensagem 12, §4.3), então isso nunca acontece na página web.

### 3.4 Antes da votação: perguntas da zerésima e "Mais informações"

* `CQuerImprimirZeresima::StartState` (11921): registra no log via `LogaMesarioIndagadoImprimirZeresima` (5882) e mostra
  `CTelasVota +228`. Só é alcançado no modo de treinamento do eleitor, em que imprimir a zerésima é opcional.
* `CImpressaoZeresimaTardia::StartState` (11932): "Mesário indagado se horário da urna está correto". Alcançado quando a
  zerésima é pedida depois da sua janela de horário: a urna pergunta se o seu próprio relógio está certo antes de imprimir
  (CORRIGE desliga a urna, u09 §3.3).
* `CMaisInformacoes::StartState` (11897): **reconstrói** a tela de menu toda vez
  (`CTelasVota::CriaTelaMaisInformacoes`, 6599), porque as contagens "(n/max)" mudam depois de cada impressão.
* `CGeraZeresimaBase::~CGeraZeresimaBase` (1720): thunk para o corpo compartilhado 2902 (vptr + liberação da tela).

### 3.5 Teste do teclado (`testeteclado`)

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

### 3.6 Fontes de dados de tela (ctelasvota.cpp e cimprimirbuoutrasobrigatorias.cpp)

São funções simples, armazenadas como ponteiros de função (slots de tabela) e chamadas sempre que o campo é desenhado:

| func | slot | usada por | valor |
|---|---|---|---|
| 13137 | 1097 | rodapé das telas de confirmação (`adicionaInstrucoesConfirmaCorrige`, 1102) | `" para REINICIAR este voto"` (depois de "CORRIGE") |
| 13176 | 1098 | telas de voto (6624, 3060) | `g_votoDigitado` (@1833288), os dígitos digitados até agora |
| 13441 | 1086 | campo de número de toda tela de voto vazia (`adicionaCampoNumero`, 2384 → 6689) | bytes de `:/resource/images/audioHabilitado.jpg` quando `CInformacaoEleitor::m_modoAudio != 2`, senão um BMP 1x1 **branco** de 66 bytes (um substituto invisível) |
| 12974 | 1105 | tela "telaNumeroCopiasErrado" ("Número de cópias / acima do limite permitido") | `"O número máximo permitido é " + to_string(GetQuantidadeMaximaBUsAdicionais())` |
| 13023 | 1101 | tela "telaDestinoBUs" de `CImprimirBUOutrasObrigatorias` | `format("Via nº {}", uebyte(qtdBU + 1))` |

Itens do menu "Mais informações" (`CItem*Vota`, criados por `CriaTelaMaisInformacoes`; o slot 3 `GetNumViasImpressas`
alimenta `comum::CItemImprime*::Disponivel`, u37):

| func | item | lê (vota.bin `EstadoGeralVota.numViasImpressasRelatorios`) |
|---|---|---|
| 12267 | `[1] Estado da urna (n/max)` | `numViasEstadoUrna` (byte +73) |
| 12266 | `[2] Lista de eleitores (n/max)` | `numViasEleitores` (+74) |
| 12259 | `[3] Versões de pacotes (n/max)` | `numViasVersoesDados` (+75) |
| 12255 | `[4] Parâmetros de urna (n/max)` | `numViasPU` (+76) |
| 12251 | `[5] Visualizar candidatos` | apenas o destrutor de deleção (o seu `Disponivel` 12244 está em u02) |

## 4. Peças da thread do operador (MT)

Tudo isto é **código morto na página web**. A tabela mostra a ação de entrada de cada estado (`StartState`, slot 2) e os
tratadores de mensagem reconstruídos aqui; o resto de cada classe está em u10, u17, u19, u27 ou u34.

### 4.1 Início do dia: `CAguardaInicio`

O primeiro estado do operador (MT: "VOTA: 10.23.0.1 / DESENVOLVIMENTO / Siga as instruções na tela do eleitor" + data e
hora). `StartState` (10220) inicia o seu tick de 500 ms e mostra o formulário, `ProcessTick` (10218) redesenha o formulário
(relógio), `FinishState` (10219) para o tick. `ProcessMessage` (10217) espera pela thread do eleitor:

| mensagem do operador | enviada por (thread do eleitor) | próximo estado do operador |
|---|---|---|
| 7 | `CDefineRotaPreVotacao` (11994), `CReinicioComparecimentoMesario` (11917) | `comum::CRegistrarMesarios` (registro de mesários antes da votação) |
| 8 | `CInicioVotacao` (5972) | log "Urna pronta para receber votos", `CPedeIdentidade` |
| 10 | `CFinalizaAquisicao` (12113, reinício durante o encerramento) | `CFimAquisicaoVotos` |
| 9 / outra | – | permanece |

### 4.2 Liberando um eleitor (habilitação)

| estado | StartState / tratador | registro de log (severidade) |
|---|---|---|
| `CIdentidadeInvalida` | 10669: mostra "Identidade: … / Número errado" | "Identificador do eleitor digitado inválido" (2) |
| `CEleitorNaoEncontrado` | 10663 = hook do slot 9, chamado por `IEleitorImpedidoVotar::StartState` depois de `Show()` | "Eleitor não encontrado para o identificador informado" (1) |
| `CDigitalNaoReconhecidaPorTempo` | 10486: "Tentativa {} de {}" (contador estático de tentativas @1590924, cfg +136) | "Timeout de reconhecimento do dedo. Tentativa [{}] de [{}]" (1) |
| `CDigitalNaoReconhecidaDecBiometria` | 10478: mostra; fora do treinamento, se `GetBiometria().GetEstadoDecifracao() > 0` | "Erro ao decifrar a biometria do eleitor - Código ({})" (3; o código é um enum com um `std::formatter` próprio) |
| `CVerificaDadoEleitor` (ano de nascimento depois da última tentativa de digital) | 10444 | "Solicitação de dado pessoal do eleitor para habilitação manual" (1) |
| `CPedeAnoNascimentoSemBiometria` (eleitor sem biometria utilizável) | 10490: `m_pedindoAno = true`, `m_erros = 0`, mostra | – |
| `CHabilitaAudioEleitor` (eleitor precisa de áudio) | 10524 | "Solicitado ao mesário que conecte o fone de ouvido" (1) |
| `CDesabilitaAudioEleitor` (depois de um eleitor com áudio) | 10436 | "Solicitado ao mesário que desconecte o fone de ouvido" (1) |
| `CCancelaHabilitacaoEleitor` | 10646: se o áudio foi ativado manualmente → "Áudio desativado pelo fim da votação"; `IInformacaoThreadOperador::LimpaDadosHabilitacao()`; → `CPedeIdentidade` | ver à esquerda |
| `CJustificativaEfetuada` | 10594: `m_novaJustificativa` ? "AUSÊNCIA JUSTIFICADA" : "JÁ JUSTIFICOU" no MT | "Justificativa recebida" (1) / "Eleitor já justificou" (2) |

### 4.3 Enquanto o eleitor vota

* **Suspensão** (`CPerguntaCodigoSuspensao`, "Informe seu título para / suspender a votação"). `StartState` (10421) registra
  no log "Solicitado título eleitoral para suspensão do eleitor", mostra o formulário e limpa `m_suspensaoEnviada` (+28).
  `ProcessMessage` (10419) trata o que a thread do eleitor informa nesse meio-tempo. **2** (sessão encerrada sem voto):
  → `CEleitorVotouNaoVotou` com `m_votou = false`. **5** (o eleitor voltou a digitar): → `CMostraEleitorVotando`, a
  pergunta é abandonada. **13** (o eleitor confirmou o último cargo): → `CSincronismoOperador` com
  `m_suspensaoAutomatica = true`, para que depois de "FIM" o MT diga que o eleitor votou.
* **Inspeção da cabine.** `CAguardaInspecao::ProcessMessage` (10530): mensagem **11** do teclado do eleitor
  (`CInspecionaUrna`: inspeção confirmada) → `CConfirmaInspecionada`, cujo construtor está inlinado aqui. Ele monta um relógio em
  {33,1}, "Inspeção completa" centralizado em {20,2}, "CONFIRMA: continuar a votação" em {40,4} e um controle de entrada.
* **Áudio manual** (`CHabilitaAudioManualmente`, 10751, "outras opções" → "Ativar/Desativar áudio" → CONFIRMA):
  inverte `IInformacaoThreadOperador::AudioHabilitadoManualmente`, escreve "ÁUDIO ATIVADO"/"ÁUDIO DESATIVADO"
  no texto do MT, registra no log "Áudio ativado pelo mesário"/"Áudio desativado pelo mesário", mostra o formulário por
  **1 s (`emscripten_sleep(1000)`)** e volta para `CPedeIdentidade`.
* `CHorarioVotacaoTerminou` (10758): "Horário de votação terminou" (1). O período de votação acabou e só
  o encerramento é possível.

### 4.4 Encerramento

`CPerguntaFilaEleitorVazia::StartState` (10715) registra no log "Operador indagado se todas as pessoas presentes votaram" e
mostra "Todas as pessoas presentes já votaram? / CORRIGE: não / CONFIRMA: sim". `CFimAquisicaoVotos::StartState` (10737)
é o ponto de articulação do encerramento (§7).

## 5. `CControladorRegistraMesariosVota`: o lado do VOTA no registro de mesários

As telas de comparecimento de mesários independem da aplicação (`comum/comparecimentomesario`, u22). O VOTA responde às
perguntas delas por meio de `comum::IControladorRegistraMesarios`. A única implementação é este objeto de 4 bytes,
registrado por `CAjusteInicial` (7160), que nunca executa no build web.

* **Ticks** (slots 2–4, 10800/10799/10798): os estados do comum rodam na thread do operador. `CriaTick(ms)` =
  `CThreadOperador +20 (CTickManager)::AddTick`, e `StopTick`/`StartTick` repassam para ele.
* **Período** (slot 6, 10796): `estadoVota` `'7'` REGISTROMESARIOINICIAL → INICIAL (1), `'8'` VOTAR → VOTACAO (2),
  `':'` REGISTROMESARIOFINAL → FINAL (3), qualquer outro → NENHUM (0). Compilado como `(estado − '7') <= 3u` mais a
  tabela @534720 = {1, 2, 0, 3}.
* **Limite** (slot 7, 10795): `CRegistradorMesario` (func 815, cache de `comparecimento_mesario`) conta as linhas do
  período: *abertura* (periodo 1) para `'7'`/`'8'`, *encerramento* (periodo 2) para `':'`. O limite é atingido quando
  a contagem é > 5, ou seja, no máximo **6 mesários** por período. Qualquer outro `estadoVota` informa "limite atingido".
* **Próximos estados**: slot 11 (10793) → `CRegistroMesarioEncerrado` (12 B, `CAppState(1)`, lazy @1904888, construtor
  inlinado; o seu `StartState` envia a mensagem 14 do eleitor). Slot 12 (10791) → `CFinalizaOperador`.
* **Busca do mesário**: os slots 13/14 (10790/10789) procuram no cadastro da seção o título digitado no MT
  (`CThreadOperador +96`) como uma identidade do tipo TÍTULO (`CEleitores::Busca`, 2264). "O mesário é eleitor desta
  seção?" orienta a comparação de digitais de `CPedeDigitalMesario`.
* **Registros de log** (slots 19–36, exceto 28), todos com severidade 1, salvo indicação:

| slot | func | registro |
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

## 6. Políticas de execução: urna vs. web

`vota::IExecucaoVota` decide como as três threads do VOTA rodam.

| slot | `CExecucaoVota` (urna, 4 B) | `CExecucaoVotaCooperativa` (web, 8 B) |
|---|---|---|
| 2 `Executa` / 3 `Inicia` | 3 (10235): inicia as threads do eleitor, do operador e de monitoramento, depois `Sleep(1000)`; 2 (10236): o mesmo, depois faz join das três threads (= `Inicia` + `Aguarda`) (u18) | os dois slots são o corpo 4713 (u18): instala o primeiro estado (`CAguardaMensagem`, passado ao construtor 7828) num novo `CAppStateContext` |
| 4 `Aguarda` | faz join das três threads | nop |
| 5 `IniciaOperador` | **10233**: `CThreadOperador::GetInst().Start()` | nop |
| 6 `FinalizaThreads` | **10232**: liga a flag de parada (`api::CThread +8`) das threads do operador e de monitoramento | nop |
| 7 `Processa` | `return false` (ICF 340) | **7823**: se a thread do eleitor tem um contexto (+32) e nenhum pedido de parada (+8): `return CThreadEleitor::GetInst().Processar()` (4349, antes exibida pelas ferramentas como `ProcessarEntrada`) |

O `main()` do build web registra `CExecucaoVotaCooperativa` antes que alguém chame `IExecucaoVota::GetInst()`, então a
política da urna nunca é criada. Nada no build web faz a thread do operador avançar. Essa única decisão de projeto explica
tudo o que esta unidade marca como código morto, inclusive por que a página nunca consegue produzir um BU (docs/10 §5.1).

## 7. BOLETIM DE URNA: o que esta unidade contribui, passo a passo

Esta unidade não contém nenhum gerador de BU. Ela contém os **pontos de articulação** do encerramento e várias peças que a
cadeia do BU usa. A geração em si está em u08/u09 (`CGeraBU`, `CImprimindoBU`, `CGravaResultado`…, docs/10 e
docs/bu/codepath.md). Os passos 1–4 abaixo são os registrados pelo harness do operador (`samples/bu-real/run-full`), com
as funções desta unidade em negrito. Essa execução para dentro do passo 4: `comum::CGravadorHashes::GravaResultado` lança
"PolySingleton - solicitada uma instancia nao criada" para `IGenericFactory<IHash>` ao gravar `-hash.dat`
(`exceptions.json`, último registro de `logd.txt` "Gerando arquivo de resultado [hash.dat] + [Início]"). O resto do passo
4 e os passos 5–8 vêm apenas do código:

1. **MT: "2 - Encerrar votação"** (`CEscolheOpcao`, u27) → `CIniciaFinalizacao` verifica o horário → **`CPerguntaFilaEleitorVazia::StartState` (10715)**.
   Ele registra no log "Operador indagado se todas as pessoas presentes votaram" e pergunta "Todas as pessoas presentes já votaram?".
   CORRIGE (não) → `CAguardaEleitoresVotarem` (3 s, volta para a identificação). CONFIRMA (sim) → log "Todas as pessoas presentes já votaram? SIM"
   → `CPedeTituloEncerramento` (o título do presidente da mesa: "Título digitado para encerramento: …")
   → `CConfirmaEncerramento` ("Procedimento de encerramento confirmado"). Esse estado marca `dhFimAquisicao`, define
   `estadoVota = FIMAQUISICAOVOTOS ('9')` e salva `vota.bin`.
2. **`CFimAquisicaoVotos::StartState` (10737)**:
   * se a eleição identifica mesários e esta não é uma urna de treinamento do eleitor (func 2520): → `comum::CRegistrarMesarios`
     para o registro final. `AtualizaEstadoRegistro` muda `'9'` para `':'` (REGISTROMESARIOFINAL). Quando a
     rodada termina, o **slot 12 (10791)** retorna `CFinalizaOperador` e o slot 15 (u18) posta a mesma mensagem 7 descrita abaixo;
   * caso contrário, posta a **mensagem 7 do eleitor** (`SMessage{7}` em `CThreadEleitor +36`, prioridade 1) e vai para `CFinalizaOperador`.
3. Thread do eleitor: `CAguardaMensagem::ProcessMessage(7)` → log "Inicio do Encerramento", `estadoVota = GERARBU` →
   `CGeraBU` (relatório do BU + QR codes + código verificador, `trab/bu.dat`) → `CGeraRelatorios` (BUJ, BIM, BEHB) →
   `CInicioBU` → [`CQuerImprimirBU` no treinamento do eleitor] → `CImprimindoBU` (via 1, "Mesário indagado sobre qualidade do
   Boletim de Urna") → `CGravaResultado`.
4. **`CGravaResultado` mostra `CProgressoEncerramento`** (nesta unidade: destrutor 3907/12104, `Inicia` 12103, `Finaliza`
   12102; `Avanca` 12101 em u07). A tela "telaProgressoEncerramento" diz "Preparando dados para encerramento / O processo pode
   levar alguns minutos / Por favor, aguarde…" sobre uma barra de 30 passos mostrada como porcentagem. `comum::CGravacaoResultados` chama `Inicia` (barra
   = mínimo, mostra), `Avanca` a cada passo e `Finaliza` (barra = máximo) enquanto grava e assina os arquivos de resultado:
   `…-bu.dat` (ASN.1 `EntidadeBoletimUrna` dentro de um `EntidadeEnvelopeGenerico`), `-rdv.dat`, `-jufa.dat`,
   `-imgbu.dat`, `-imgze.dat`, `-hash.dat`, `-log.jez`, `-vota.vsc`. Cada passo grava um registro de log "Gerando arquivo
   de resultado [x.dat] + [Início]/[Término]" (a execução do harness registrou bu, rdv, jufa, imgbu e imgze, e depois morreu
   no início de hash.dat).
5. `CCopiaResultadoParaMR` (cópia para a MR, verificação do BU) → **`CImprimirBUOutrasObrigatorias`**, as vias obrigatórias
   restantes. A sua tela "telaDestinoBUs" mostra **"Via nº N" (13023)** com N = `EstadoGeralVota.qtdBU + 1`, avaliado a
   cada redesenho: `qtdBU` (vota.bin, byte +8) conta as vias já impressas. Depois BUJ, BIM, BEHB, `CRetirarMR`.
6. Vias extras (`CEmitirMaisBU`, u08): pedir mais do que o permitido mostra "telaNumeroCopiasErrado" com **"O número
   máximo permitido é N" (12974)**, onde N = `GetQuantidadeMaximaBUsAdicionais()` = obrigatórias + adicionais − já impressas
   (1 no treinamento do eleitor; votadefs.cpp:135).
7. `CMostraQRCodeBU` mostra o BU como QR codes, uma parte de cada vez (≤ 2500 caracteres por código na tela, ≤ 1100 no
   papel). Os textos deles ficam no poly-singleton **`IQRCodeBUDS`** (nesta unidade: destrutores 12050/12049). Eles são mantidos até
   a lista de singletons ser limpa. BRANCO mostra o certificado da urna (`CMostraQRCodeCertificado`).
8. **`CAplicacaoEncerrada::StartState` (12058)**: log "Votação encerrada", tela final. A urna então se desliga sozinha quando
   expira o time-out da bateria (`CEstadoComDesligamentoAutomatico`).

Os stubs atexit desta unidade (§8) incluem os singletons de todos os estados desta cadeia (`CGeraBU` 12111/12112,
`CGeraRelatorios`, `CImprimindoBU`, `CQuerImprimirBU`, `CGravaResultado`, `CCopiaResultadoParaMR`,
`CImprimirBUOutrasObrigatorias`, `CImprimirBJust`, `CImprimindoBEHB`, `CVerificaQtdBUsAdicionais`,
`CLimiteCopiasBUAtingido`, `CMostraQRCodeCertificado`, `CFinalizaAquisicao`). Eles dão os endereços desses
singletons (úteis para sondar um módulo em execução) e são código morto.

## 8. Os destrutores de saída (31 funções)

Todos seguem o padrão descrito em `src/uenux2/src/app/vota/u38-foreign-fragments.cpp`. O `std::mutex` estático +
o `std::unique_ptr<T>` de cada singleton lazy recebem, cada um, um stub `__dtor_<var>` para `__cxa_atexit`. Os registros
foram eliminados pela otimização (`EXIT_RUNTIME=0`), e os stubs sobrevivem apenas pelos seus slots de tabela. Corpos: `~mutex` = o stub ICF
no-op 150; `~unique_ptr` = um dos corpos "delete" mesclados 349 (destrutor trivial), 389 (ICF 244), 763
(ICF 448), 1564 (ICF 785). Os donos e os endereços estão listados em `src/uenux2/src/app/vota/eleitor/u39-foreign-fragments.cpp`.
Três stubs não são de singletons:

* 13593 / 13588: `g_votoDigitado` (`std::string` @1833288) e `g_votosEleitor`
  (`std::vector<pair<TCargoID, CVoto>>` @1833300), a cédula em composição (celeitorvotando.cpp);
* 14464: o `static const std::regex("\\B\\d")` local de função (@1833216, byte de guarda @1833256) com o qual
  `CVotacaoStateAudio::FormataMensagem` separa `{voto}` em dígitos individuais para o sintetizador de voz. O seu corpo 6096 é o
  `~basic_regex` da libc++.

## 9. Dados lidos e gravados

| dado | acesso | onde |
|---|---|---|
| `dinamico/log/logd.dat` (log de eventos da urna, registros Latin-1 `1\|sev\|text`) | gravação | todo registro citado nas §4–§5 (via `CLogVota`/`IEventosLog`, `api::CLoga::loga`) |
| `vota.bin` `EstadoGeralVota` (ModuloEstadoGeralVota) | leitura | `estadoVota` (+0) nos slots 6/7; `qtdBU` (+8) em 13023; `numViasImpressasRelatorios` (+73..+76) nos itens de menu; `treinamentoEleitor` (+72) via as funcs 1823/2520 |
| tabela `comparecimento_mesario` de `uenux.db` | leitura (contagens) | slot 7 via `CRegistradorMesario` (as gravações estão nos estados do comum) |
| cadastro de eleitores (`CEleitores`) | leitura | slots 13/14 (título do mesário), 10478 (estado de decifração da biometria) |
| `CConfiguracaoEleicao` +136 | leitura | número de tentativas de digital (10486) |
| recurso `:/resource/images/audioHabilitado.jpg` | leitura | 13441 |
| token `/dev/urandom` (`std::random_device`, atendido por `crypto.getRandomValues` no Emscripten) | leitura | 11803 |
| filas de mensagens do operador/eleitor | gravação | 7178 (op 13), 10737 (eleitor 7), mais as reações em 10217, 10419, 10530 |

Nenhum SQL é gravado aqui e nenhum ASN.1 é codificado aqui. O único elo desta unidade com os arquivos do BU é a tela de progresso.

## 10. Particularidades do build web

* `CExecucaoVotaCooperativa` (7828/7823) é o substituto web do executor de threads. É o que `votaTick` executa
  (723 amostras do profiler no voto municipal).
* Inalcançáveis na página: todos os estados do operador (§4), o controlador (§5), a política da urna (§6) e, portanto, todo o
  encerramento (§7). `CSincronismoEleitor` ainda posta a mensagem 13 numa fila que ninguém lê.
* O simulador começa diretamente em `estadoVota = VOTAR`, então as perguntas da zerésima, o teste do teclado e "Mais informações" nunca
  aparecem. A tela "Mais informações" ainda assim é **montada** na inicialização (12267 observada).
* As frases de áudio (§3.2) só são usadas quando o eleitor ativa o áudio (`votaSetAudioEnabled`), e só se o RHVoice estiver
  disponível na página.

## 11. Observações de wasm / Emscripten

* **Singletons lazy sem guards.** Os corpos de `GetInst` dos estados do operador não contêm `__cxa_guard`, então os estáticos
  são de escopo de namespace / membros de classe (o argumento de u38). As exceções são o `std::mt19937` de `GeraSequencia`, um verdadeiro
  estático local de função com um guard de um byte (@1837652), e o `std::regex` de `FormataMensagem` destruído pelo
  stub 14464 (byte de guarda @1833256). Num build de thread única, `__cxa_guard_acquire` é inlinado
  como um teste de byte.
* **Corpos de log mesclados.** As funções de uma linha de `CLogVota` foram mescladas pela forma (`merge-similar-functions`). A func 3888 é
  "registrar no log um literal de 47 bytes" e recebe os seis blocos de 8 bytes do literal como *parâmetros*; o anotador os
  imprime como seis strings sobrepostas (`/* "Operador indagado …" */, /* " indagado …" */…`). 3887 formata o título do
  mesário, e 6017 formata um argumento de título. A string de formato chega a 3887/6017 como um par `[begin, end)`.
* **Constantes `SPoint` empacotadas parecem strings.** Em 10530, as posições `{33,1}`, `{20,2}`, `{40,4}` são as
  constantes i32 65569, 131092, 262184. O anotador as decodifica como endereços de string ("dadeVersaoArquivos", "tuloRelatorio"…).
* **Tipos de argumento de `std::format`** lidos do código: `198` = dois `unsigned` (10486), `0x18C63` = quatro `int`
  (10772), `6` = um `unsigned` (13023), `13` = uma string (3887/6017). `15` = um *handle*: o enum de 10478
  é formatado por uma especialização de `std::formatter` do TSE cujo corpo é a lambda compartilhada 536 da libc++ (slot de tabela 4091).
* **Switches compilados**: 10796 é uma verificação de limites mais uma leitura de tabela; 10217 e 10419 são `br_table`s sobre `message − 7` / `message − 2`.
* **ICF**: os três destrutores de `CPriorityMessageQueue<SMessage>` (base, `CMessageEleitor`, `CMessageOperador`)
  são um único corpo, 3162/3157. As gravações de vptr das classes derivadas eram mortas e foram removidas.
* `emscripten_sleep` é protegido pelo byte @1584624 (= 1, nunca gravado). O `_emscripten_sleep` do glue aborta
  ("Please compile your program with async support…").

## 12. Código suspeito / notável

1. **10751 `CHabilitaAudioManualmente::StartState` chama `emscripten_sleep(1000)`**, e `CExecucaoVota` (10235/10236, u18)
   faz o mesmo. Neste build, essa chamada aborta o módulo inteiro. Ela é inalcançável na página, mas qualquer versão web
   futura que faça a thread do operador avançar (como faz o harness, com um sleep substituído por stub) quebraria no momento em que um
   mesário alternasse o áudio. Numa urna real, ela congela o MT por 1 s, por projeto.
2. **Títulos de mesários gravados no log de eventos** (10769/10770/10771/10775/10784; 3887/6017). "Mesário 012345678901
   é eleitor da seção", "Pedido de leitura da biometria do mesário …", "Mesário … registrado". O título eleitoral do
   mesário é dado pessoal. `logd.dat` vai para os arquivos de resultado (`-log.jez`), e o TSE publica os
   logs das urnas. Os títulos dos eleitores não são registrados no log por essas funções.
3. **Scores biométricos no log, possivelmente sob o dedo errado** (10772). Os quatro scores são sempre rotulados
   Polegar Direito, Polegar Esquerdo, Indicador Direito, Indicador Esquerdo. O chamador (`CPedeDigitalMesario`,
   u22) preenche o vetor de forma *compacta*, apenas para os dedos que o mesário tem no cadastro (`scores[n++]`), e zero
   para o resto. Com um dedo ausente, os scores se deslocam e são atribuídos aos dedos errados no log.
   O vetor sempre tem 4 elementos, então não há acesso fora dos limites.
4. **`LimiteMesariosAtingido` (10795) retorna true para qualquer `estadoVota` inesperado.** É seguro por projeto (registro
   recusado), mas depende de o slot 5 ter mudado `'9'` para `':'` antes da verificação.
5. **Fila do operador não lida no build web** (7178 + os outros estados do eleitor). Cada voto deixa 2 + (número de
   cargos) mensagens de 16 bytes que ninguém remove (6 no início, 9 por cargo, 13 e 1 no fim: 5 no voto
   municipal de 2 cargos, u10 §2). O crescimento é limitado na prática: depois de "FIM", a thread do eleitor fica ociosa em
   `CAguardaMensagem` (`done: true` nas transcrições; `node tools/run/headless.mjs --scenario municipal-t1 --auto
   blank --voters 2` informa `NEXT-VOTER {"started":false}` depois de 15 s de ticks), porque só a thread do operador
   poderia liberar o próximo eleitor, e os dois botões do painel final da página saem da página (`restartVoting` → `window.location.reload()`,
   `chooseScenario` → `index.html`, `votaWasmAdapter.js`). Só viraria um vazamento numa variante que liberasse
   vários eleitores por carregamento da página.
6. **`CEsperaRetestar::StartState` (11825)**: a espera antes de repetir o teste do teclado cresce 5 s a cada
   tentativa e nunca é zerada enquanto a aplicação roda. Depois de muitas falhas, o mesário espera minutos. É um
   problema de usabilidade, não de segurança.
7. **`CExecucaoVotaCooperativa::Processa` (7823)** retorna false silenciosamente se a flag de parada da thread do eleitor (+8) estiver ligada
   (só `CThreadOperador::FinalizaExecucao` faz isso). Uma variante web que rodasse a thread do operador e
   chegasse a isso congelaria a tela do eleitor sem nenhum erro. Informativo.
8. **Caminho morto de decifração biométrica no treinamento** (10478). O registro de log é suprimido em
   `EhTreinamentoSemTreinamentoEleitor`, e a função copia o `CBiometriaEleitor` inteiro (vetor + map) duas vezes por
   valor para ler um enum. Ineficiente, não é um bug.
9. **Nome enganoso no banco de análise.** A func 1950, nomeada pelo srcloc como `CInformacaoEleicao::EhModoDemonstracao`, retorna
   `!IInterfaceInit::GetDemoMode() && cfg[400]`. Ela se comporta como "identifica mesários (fora do modo de demonstração)", com
   `EhModoDemonstracao()` inlinada nela. Chamadores como `CFimAquisicaoVotos` (via 2520) parecem dizer "modo de demonstração → registrar
   mesários" se se confiar no nome da ferramenta.

## 13. Questões em aberto

* Os nomes dos slots 5/6 de `IExecucaoVota` (`IniciaOperador`, `FinalizaThreads`) e dos slots de `CAbstractTelaProgresso`
  (`Inicia`/`Finaliza`/`Avanca`) são inferidos a partir do comportamento. Nenhuma string ou srcloc os nomeia.
* Slot 9 de `IEleitorImpedidoVotar`: chamado por `StartState` depois de `Show()`. Aqui ele se chama `LogaEntrada` e,
  no header de u27, `AoRetornar`. Os dois nomes devem ser unificados.
* `CExecucaoVotaCooperativa` fica em `mock/app/vota/` ou em `mock/app/simulador/wasm/`? Ambos são compatíveis com o
  layout dos dados. O banco de dados agora registra as três funções dela (7828, 4713, 7823) em
  `uenux2/mock/app/vota/cexecucaovotacooperativa.cpp`, componente `app:mock`, seguindo a escolha desta unidade.
  `CControladorRegistraMesariosVota` fica em `vota/comum/` ou em `vota/operador/comparecimentomesario/`?
* `CItem*Vota`: definidas em ctelasvota.cpp ou num header incluído apenas por ele? As vtables emitidas só provam que
  ctelasvota.cpp é o (primeiro) usuário.
* O slot 8 de `CControladorRegistraMesariosVota` (sempre `true`, ICF 434) não tem chamador nos estados do comum reconstruídos até
  agora.

## 14. Tabela de mapeamento completa (124 funções)

Coluna "run" = observada executando nos votos gravados. O arquivo src é onde a reconstrução fica. O
caminho original vem em seguida, entre parênteses (inferido, a menos que a §2 diga que é atestado). "Nome nas ferramentas"
é o nome atual no banco de análise.

| # | func | tamanho | run | nome nas ferramentas | símbolo reconstruído | arquivo src (caminho original) |
|---|---|---|---|---|---|---|
| 1 | 1559 | 66 |  | `vota::testeteclado::CBase::vf0` | `vota::testeteclado::CBase::~CBase` | `src/uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cbase.cpp` (`uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cbase.cpp`) |
| 2 | 1717 | 151 |  | `vota::IConfereVotoEmCargo::vf0` | `vota::IConfereVotoEmCargo::~IConfereVotoEmCargo` | `src/uenux2/src/app/vota/eleitor/cconferevotoemcargo.u39.cpp` (`uenux2/src/app/vota/eleitor/cconferevotoemcargo.cpp`) |
| 3 | 1720 | 12 |  | `vota::CGeraZeresimaBase::vf0` | `vota::CGeraZeresimaBase::~CGeraZeresimaBase` | `src/uenux2/src/app/vota/eleitor/iniciovotacao/cgerazeresima.u39.cpp` (`uenux2/src/app/vota/eleitor/iniciovotacao/cgerazeresima.cpp`) |
| 4 | 3157 | 111 |  | `vota::CMessageEleitor::vf1` | `api::CPriorityMessageQueue<api::SMessage>::~CPriorityMessageQueue (deleting)` | `src/uenux2/src/api/ipc/cmessagequeue.h` (u18) + `src/uenux2/src/app/vota/eleitor/u39-foreign-fragments.cpp` (`uenux2/src/api/ipc/cmessagequeue.h`) |
| 5 | 3162 | 108 |  | `vota::CMessageEleitor::vf0` | `api::CPriorityMessageQueue<api::SMessage>::~CPriorityMessageQueue` | `src/uenux2/src/api/ipc/cmessagequeue.h` (u18) + `src/uenux2/src/app/vota/eleitor/u39-foreign-fragments.cpp` (`uenux2/src/api/ipc/cmessagequeue.h`) |
| 6 | 3907 | 114 |  | `vota::CProgressoEncerramento::vf0` | `vota::CProgressoEncerramento::~CProgressoEncerramento` | `src/uenux2/src/app/vota/eleitor/fimvotacao/cprogressoencerramento.cpp` (`uenux2/src/app/vota/eleitor/fimvotacao/cprogressoencerramento.cpp`) |
| 7 | 7178 | 97 | ✓ | `vota::CSincronismoEleitor::vf2` | `vota::CSincronismoEleitor::StartState` | `src/uenux2/src/app/vota/eleitor/csincronismoeleitor.cpp` (`uenux2/src/app/vota/eleitor/csincronismoeleitor.cpp`) |
| 8 | 7823 | 38 | ✓ | `vota::CExecucaoVotaCooperativa::Processa` (antes exibido pelas ferramentas como `vf7`) | `vota::CExecucaoVotaCooperativa::Processa` | `src/uenux2/mock/app/vota/cexecucaovotacooperativa.cpp` (`uenux2/mock/app/vota/cexecucaovotacooperativa.cpp`) |
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

## 15. Revisão de fidelidade (2026-09-23)

Comparado linha a linha com o código descompilado (e com o WAT onde o pseudo-código era ambíguo): 7178, 7823, 7828,
11790, 11792, 1717, 11789, 12267/12266/12259/12255/12251, 10486, 10478, 11803, 10530, 10217–10220, 10419, 10421,
10436, 10444, 10490, 10524, 10594, 10646, 10663, 10669, 10715, 10737, 10751, 10758, todos os 26 slots do controlador, 11874,
11875, 1559, 11824, 11825, 11897, 11921, 11932, 11798, 12058, 3907/12102–12104, 13023, 12974, 13137, 13176, 13441,
3157/3162, 12049/12050, 1720, os sete textos de `GetMensagemAudio` e todos os 31 stubs atexit (cada endereço foi rastreado
até o acessor que armazena a vtable do dono). As afirmações vfN → nome de método batem com as vtables
(`q.py cls`) e com os headers de interface de u08/u22/u37. Condições de desvio, constantes, strings, offsets e a ordem
das chamadas batem. A tabela de mapeamento lista todos os 124 índices, e os seus nomes de ferramenta e tamanhos batem com `functions.tsv`.
Correções:

* Introdução: a unidade agrupa 46 classes, não 44.
* §6: o slot 3 de `CExecucaoVota` (`Inicia`, 10235) não faz join das threads. Só o slot 2 (`Executa`, 10236 = Inicia +
  Aguarda) faz.
* §7: a execução do harness não cobre todo o encerramento. Ela morre em `CGravaResultado` ao gravar `-hash.dat`
  (`IGenericFactory<IHash>` não registrado). Os passos 5–8 e os últimos arquivos de resultado vêm apenas do código.
* §12.5: o acúmulo por voto é 2 + número de cargos, não 5 fixos. Crescimento ilimitado não é alcançável com
  a página atual. Reconferido com `headless.mjs --voters 2`: nenhum segundo eleitor começa sem recarregar a página.
* §11: a regex de 14464 é um segundo estático local de função com guard.
* Fontes: `cfimaquisicaovotos.cpp` chamava `comum::DeveRegistrarMesarios()`, mas o `cappinfo.h` que ele inclui
  declara a func 2520 como `EhModoDemonstracaoSemTreinamentoEleitor`. `ccancelahabilitacaoeleitor.cpp` e
  `chabilitaaudiomanualmente.u39.cpp` declaravam helpers em namespace anônimo que nada define, então não
  linkariam. Agora eles chamam diretamente os slots 6/7 de `IInformacaoThreadOperador` (`GetAudioHabilitadoManualmente` /
  `SetAudioHabilitadoManualmente`, como em `iinformacaothreadoperador.h`). Em `ctelasvota.u39.cpp`, `VotoDigitado`
  foi movido para o namespace anônimo onde o `ctelasvota.cpp` de u07 o declara.
* Comentários: o header de `CPerguntaCodigoSuspensao` dizia que um título válido posta a mensagem 4 do eleitor. Ele posta a
  mensagem 2; a mensagem 4 é o abort por CORRIGE. O no-op da base do slot 9 de `IEleitorImpedidoVotar` é o ICF 218, não o 425. Em 10486,
  as chamadas descartadas de `CControlaReconhecimento::GetInst()` vêm apenas antes das duas primeiras leituras. A func 5946 é o
  destrutor de deleção de `CPreZeresima`/`CRetomada`, não um reset de saída.

Achados suspeitos (§12): 1, 3, 4, 6 e 7 foram confirmados no código. Para o 3, o chamador 10316 preenche `scores[j++]`
apenas para os dedos presentes no cadastro, na ordem de dedos {1, 6, 2, 7} = PD, PE, ID, IE. Para o 1, o byte de guarda
@1584624 não tem nenhuma gravação no módulo, e o `_emscripten_sleep` do glue chama `abort(...)`. Para o 2, o código de fato
registra no log o título do mesário em claro. As afirmações de que `logd.dat` vai para `-log.jez` e de que o TSE publica os
logs das urnas não puderam ser conferidas com os artefatos desta unidade. O 5 foi confirmado apenas na sua forma fraca, como
corrigido acima. O 9 foi confirmado: a func 1950 retorna `!GetDemoMode() && cfg[400]`.
