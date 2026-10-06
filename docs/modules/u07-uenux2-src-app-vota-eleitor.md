# u07: `uenux2/src/app/vota/eleitor` — a thread do eleitor, a persistência do voto (sincronização do RDV) e as telas de votação

A unidade u07 cobre 65 funções wasm. Três arquivos originais detêm a maior parte delas:

| arquivo original | o que é |
|---|---|
| `uenux2/src/app/vota/eleitor/cthreadeleitor.cpp` | `vota::CThreadEleitor`, a **thread do eleitor**: é dona do terminal do eleitor (tela, teclado, áudio) e executa a máquina de estados do lado do eleitor |
| `uenux2/src/app/vota/eleitor/csincronismovotoeleitor.cpp` | `vota::impl::CSincronismoVotoEleitor`, a etapa que torna **duráveis** os votos de um eleitor: grava o RDV cifrado e o estado da votação nas duas memórias flash e os assina novamente |
| `uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp` | `vota::CTelasVota`, a **fábrica de todas as telas de votação**: nome do cargo, dígitos digitados, nome/foto do candidato, "VOTO NULO", "VOTO DE LEGENDA", perguntas de consulta… |

A ferramenta de unidades também anexou (por proximidade no grafo de chamadas) funções cujo código-fonte está em outro lugar: helpers de GUI de
`uenux2/src/api/gui` (`CFormBuilder::Add`, `CProgressBar`, `CGrayedFramedText`), dois acessores de `comum::md::CCargo`,
três setters de `vota::CInformacaoEleitor`, os estados de reinício/zerésima (`CReinicioVotacao`,
`CQuerImprimirZeresima`, `CGeraZeresima*`, `testeteclado::CRetomada`), `CProgressoEncerramento` e dois
corpos mesclados pelo wasm-opt de `GetTelaCargoAtual`. Eles são reconstruídos em arquivos de *fragmento* chamados
`<original>.u07.cpp` ao lado do caminho do seu arquivo original, para que não sobrescrevam os arquivos das
unidades que os detêm (mesma convenção da u02).

27 das 65 funções foram executadas durante os dois votos gravados (`analysis/runtime/*.functions.tsv`).

**Fontes reconstruídos** (todos em `src/`):

* arquivos próprios: `uenux2/src/app/vota/eleitor/cthreadeleitor.{h,cpp}`, `uenux2/src/app/vota/eleitor/csincronismovotoeleitor.{h,cpp}`,
  `uenux2/src/app/vota/eleitor/comum/ctelasvota.{h,cpp}`
* fragmentos: `uenux2/src/app/vota/comum/csincronizavota.u07.cpp`, `uenux2/src/api/gui/{cformbuilder,cprogressbar,cgrayedframedtext}.u07.cpp`,
  `uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.u07.cpp`, `uenux2/src/app/vota/eleitor/comum/cinformacaoeleitor.u07.cpp`,
  `uenux2/src/app/vota/eleitor/iniciovotacao/{creiniciovotacao,cquerimprimirzeresima,cquerreimprimirzeresima,cgerazeresima}.u07.cpp`,
  `uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cretomada.u07.cpp`, `uenux2/src/app/vota/eleitor/fimvotacao/cprogressoencerramento.u07.cpp`,
  `uenux2/src/app/vota/eleitor/votamajoritario/cpedemajoritario.u07.cpp`

---

## 1. Onde este código se encaixa no processo de votação

Glossário: *mesário* = quem opera o terminal do mesário; *cargo* = cargo em votação (Prefeito, Vereador, Senador…);
*voto nominal / de legenda / branco / nulo* = voto em um candidato / apenas em um partido / em branco / nulo; *consulta* =
pergunta no estilo de plebiscito cujos "candidatos" são respostas; *suplente / vice* = companheiros de chapa; *RDV* (*Registro Digital do Voto*) = a lista anonimizada e cifrada de
todos os votos (`rdv.dat`); *MI* (*memória interna*) = a flash interna `/dsk/fi`; *MV* (*memória de votação*) = o
cartão flash removível `/dsk/fe`; *zerésima* = o relatório impresso quando a votação é aberta, que prova que todo candidato
começa com zero; *reinício* = reinício da aplicação durante o dia da eleição; *treinamento eleitor* = modo de
treinamento do eleitor (os cenários do simulador estão nesse modo).

A aplicação VOTA da urna executa três "threads" cooperativas (veja `docs/modules/u06-…` para os estados):

```
 operator terminal                        voter terminal (this unit)
 CThreadOperador  --msg 1 (inicia) ---->  CThreadEleitor.m_fila (vota::CMessageEleitor)
 (CHabilitaAudio*) --msg 8/9/10 ------->      |  Processar(): messages -> keyboard -> ticks
                                              v
                         comum::CAppStateContext: CAguardaMensagem -> CEleitorVotando -> ... ->
                         CSincronismoEleitor ("Gravando") -> CFimVotoEleitor ("FIM") -> CAguardaMensagem
 CMostraEleitorVotando <--msg 13--------  CSincronismoEleitor::StartState
 -> CSincronismoOperador::StartState (func 10210)
    ---msg 5--------------------------->  CSincronismoEleitor::ProcessMessage(5)
                                              -> ISincronismoVotoEleitor::GetInst().SincronizaVoto()
                                                 = CSincronismoVotoEleitor (urna) / CSincronismoVotoEleitorWeb (simulator: "return true")
```

Toda tela que o eleitor vê durante esses estados é um `CFormInterativoTelaVota` obtido de `CTelasVota`.

## 2. Classes (RTTI) e layouts

```
api::CThread                                   typeinfo @1600028  (cthread.cpp)
 └─ vota::CThreadVota                          typeinfo @1532484  slots 2 and 5 pure
     ├─ vota::CThreadEleitor                   typeinfo @1534664, vtable @1534624  (this unit)
     ├─ vota::CThreadOperador                  vtable @1601208  (Run = func 10204, same loop inlined)
     └─ vota::CThreadMonitor                   vtable @1600768
api::CPriorityMessageQueue<api::SMessage> ─┐
api::CMessageInterface ────────────────────┴─ vota::CMessageEleitor   typeinfo @1534716 (vmi), member of CThreadEleitor
vota::impl::ISincronismoVotoEleitor
 ├─ vota::impl::CSincronismoVotoEleitor        typeinfo @1534176, vtable @1534164   (urna implementation)
 └─ (anonymous)::CSincronismoVotoEleitorWeb    typeinfo @1527264, vtable @1527252   (vota_web_wasm.cpp, returns true)
vota::CTelasVota                               not polymorphic, 252 bytes, singleton @1833396
comum::CAppState ─ vota::CReinicioVotacao (@1546636), CQuerImprimirZeresima (@1544848), CQuerReimprimirZeresima (@1546348),
                   CGeraZeresimaBase (@1544392) ─ CGeraZeresima (@1544584), CRegerarZeresima (@1546500)
                   CGeraResumoZeresimaBase (@1544456) ─ CGeraResumoZeresima (@1544520), CRegeraResumoZeresima (@1546436)
                   CEstadoComDesligamentoAutomatico ─ testeteclado::CBase ─ testeteclado::CRetomada (@1546708)
comum::CAbstractTelaProgresso ─ vota::CProgressoEncerramento (@1540596)
```

Slots virtuais usados por esta unidade (nomes coerentes com a u06):

| classe | slots |
|---|---|
| `api::CThread` / `CThreadVota` | 0 destrutor, 1 destrutor de deleção, 2 `Run`, 3 `TrataExcecao(const std::exception&)` (func 7710), 4 `TrataExcecaoDesconhecida` (7709), 5 `FinalizaExecucao` (puro em CThreadVota) |
| `api::IThreadImpl` (`simulador::CWasmThread`) | 2 `Create(void*(*)(void*), void*)` (9655), 5 `Yield` (9651) |
| `comum::CAppState` | 2 `StartState`, 3 `NeedChangeState` (= `GetNextState() != this`, func 7480), 4 `GetNextState` (retorna +4), 5 `FinishState`, 6 `ProcessMessage(uebyte)`, 7 `ProcessInput()`, 8 `ProcessTick(uebyte)` |
| `vota::IExecucaoVota` | 2/3 iniciam as threads do eleitor, do operador e do monitor de energia (2 depois faz join nelas), 4 join, 5 inicia apenas a thread do operador (`CExecucaoVota`, func 10233), 6 `FinalizaThreads` (nome inferido), 7 passo cooperativo (`CExecucaoVotaCooperativa::Processa`, func 7823, = `CThreadEleitor::GetInst().Processar()`), 8 `GetFilaEleitor()` (= `&CThreadEleitor::GetInst().m_fila`) |
| `ISincronismoVotoEleitor` | 0 destrutor, 1 destrutor de deleção, 2 `SincronizaVoto()` (nome inferido) |
| `testeteclado::CBase` | 10 `CriaTela`, 11 `GetEstadoPassouNoTeste` (nome atestado), 12 `GetEstadoSemTeste` (inferido) |
| `CGeraZeresimaBase` / `CGeraResumoZeresimaBase` | 9 hook pós-impressão (`CRegerarZeresima`: `CRelVotaUtil::CortaPapel`), 10 `GetEstadoResumo` (inferido) |
| `CAbstractTelaProgresso` | 2 `Inicia` (barra = mín., mostra), 3 `Finaliza` (barra = máx.), 4 `Avanca` |

`comum::CAppState` = `{+0 vptr, +4 m_proximoEstado (= this at StartState), +8 bool recebe mensagens, +9 bool recebe teclado, +10 bool recebe ticks}`;
o construtor (func 224) recebe as três flags como máscara de bits (1 = mensagens, 2 = teclado, 4 = ticks).
`comum::CAppStateContext` = `{+0 vptr, +4 CAppState* m_pEstado}`.

`CThreadEleitor` (84 bytes, `operator new(84)` na func 316):

| offset | membro |
|---|---|
| +0 | vptr |
| +4 | `int m_estado` (`CThread::Start` define 1) |
| +8 | `bool m_bParar` (parada solicitada) |
| +9 | `bool m_bDormindo` |
| +12 | `unique_ptr<api::IThreadImpl> m_pImpl` (de `IGenericFactory<IThreadImpl>`, cthread.cpp:31/32) |
| +16 | `unique_ptr<api::ISyncCtl> m_pSync` |
| +20 | `std::map<uebyte, STick> m_ticks` (CThreadVota; lista de expiração = func 5450, `gettimeofday`) |
| +32 | `unique_ptr<comum::CAppStateContext> m_pContexto` (CThreadVota) |
| +36 | `CMessageEleitor m_fila` (vector de `SMessage` +40..+48, semáforo +56, lock +60, 2º vptr +68) |

Membros de `CTelasVota` (252 bytes) referenciados por esta unidade: `+0 std::map<TCargoID, CTelasCargo>`, `+84` tela de
`CConfirmaRegerarZeresima`, `+124` instruções de acessibilidade, `+148` tela de reinício, `+172` "telaProgressoRegistroVoto"
("Gravando" + barra), `+180 shared_ptr<api::CProgressBar>` (máx. 4, retângulo (70,225)-(570,255)), `+212`/`+220` telas de
geração da zerésima, `+228` "quer imprimir a zerésima?", `+244` "quer reimprimir a zerésima?".

---

## 3. A thread do eleitor (`CThreadEleitor`)

### 3.1 Ciclo de vida

* **Criação**: `CThreadEleitor::GetInst()` (func 316, unidade u18, com o nome errado `CPriorityMessageQueue<SMessage>::ctor`)
  constrói o objeto de forma preguiçosa: `api::CThread()`, um mapa de ticks vazio, nenhum contexto e a fila `CMessageEleitor` (seu
  semáforo e seu lock vêm dos poly-singletons `IGenericFactory<ISemaphore>` / `IGenericFactory<ISyncCtl>`,
  cmessagequeue.h:113/114). O `unique_ptr` estático é liberado na saída pela func 7122.
* **Run** (slot 2, func 7061) — somente na urna:
  1. `IAjusteInicial::GetInst()` (cajusteinicial.cpp:57, inlinada): se a `CPolySingletonList` não tem
     `IAjusteInicial` (func 2450 `contains`), insere um `CAjusteInicial` (`CAppState` de 12 bytes com flags 0); depois
     `CPolySingleton<IAjusteInicial>::instance()`. Este é o padrão de *registro padrão* usado em todo o VOTA:
     uma plataforma pode pré-registrar a sua própria implementação; caso contrário, a da urna é criada.
  2. `m_pContexto = make_unique<CAppStateContext>(&ajuste)`, depois `StartState()` desse primeiro estado.
  3. `while (!m_bParar) { r = Processar(); if (m_bParar) break; m_pImpl->Yield(); if (!r) Sleep(50 ms); }` —
     o sleep é compilado como `if (flag@1584624) emscripten_sleep(50)`; a flag vale 1 no segmento de dados e
     nunca é escrita. Na build web, o próprio `Yield` é `simulador::CWasmThread::Yield` (func 9651), que chama
     `emscripten_sleep(0)`: sem Asyncify, as duas chamadas abortam, então `Run` não sobreviveria ao seu primeiro ciclo.
* **FinalizaExecucao** (slot 5, func 7030): `IExecucaoVota::GetInst().FinalizaThreads()` (slot 6). Os
  tratadores de exceção de `CThreadVota` (funcs 7710/7709) a chamam antes de mostrar um erro fatal (a menos que a exceção seja
  `api::CUeDesligandoError` ou que a flag de desligamento @1832936 esteja ligada), de modo que uma falha da thread do eleitor para a
  thread do operador e o monitor de energia (`CExecucaoVota::vf6`: `m_bParar = 1` em ambas). A política web cooperativa tem uma implementação vazia.
* **Destrutor** (func 2438, slot 0; 7070 = de deleção): apenas destruição de membros.

### 3.2 Um ciclo: `Processar()` (func 4349)

A função que as ferramentas primeiro chamaram de `ProcessarEntrada` (agora elas mostram `vota::CThreadEleitor::Processar`) é o
ciclo *externo*. Seu único registro srcloc (cthreadeleitor.cpp:134,
assinatura `bool ProcessarEntrada(comum::CAppStateContext&)`) pertence a um dos três helpers inlinados; as
strings de log de depuração dão o nome de um segundo (`"ProcessarMensagens"`). Ordem do trabalho, com cada parte pulada assim que
`m_bParar` é ligado:

1. **ProcessarMensagens**: enquanto a fila não está vazia (verificado sob o lock da fila), `Remove()` uma `SMessage`
   (func 2073) e despacha pelo seu id de 16 bits:

   | id | ação (na própria thread) |
   |---|---|
   | 0, 6 | `m_bParar = true` (para a thread) |
   | 8 | `CInformacaoEleitor::m_modoAudio = 0` — áudio "conforme cadastro" (postada pelo estado do operador `CHabilitaAudioEleitor` quando o cadastro do eleitor pede áudio) |
   | 9 | `m_modoAudio = 1` — áudio habilitado (`CHabilitaAudioEleitor`, e `votaInit` quando a opção de página `audioEleitorHabilitado` está ligada) |
   | 10 | `m_modoAudio = 2` — áudio desabilitado (`CDesabilitaAudioEleitor`; caso contrário, `votaInit` define 2 diretamente) |
   | qualquer outro | se o estado atual aceita mensagens (flag +8): `state->ProcessMessage(uebyte(id))`, depois a verificação de troca de estado. Ids conhecidos: 1 início (votaInit / CHabilitaAudioEleitor), 2/3 suspensão, 4 continuar (u06), 5 sincronizar o voto |

   Cada mensagem é registrada em LOG_DEBUG: `"%s: 1 msg[%d] context[%s]"` (ids tratados pela thread) ou
   `"%s: 2 msg[%d] context[%s]"` (encaminhados), com o nome `typeid` mangled do estado atual.
2. **ProcessarEntrada** (o srcloc :134 é a chamada `api::IInputKbd::GetInst()`): se o estado aceita teclado
   (flag +9) e `IInputKbd::HasKey()` (slot 3), registra `"%s: hasKey[%d] context[%s]"`, `state->ProcessInput()`,
   verificação de troca de estado. O próprio estado lê a tecla (através de `CInteractiveForm::Read`).
3. **ProcessarTicks** (nome inferido): os ids de tick expirados de `CThreadVota` (func 5450); se houver algum e o estado
   aceitar ticks (flag +10), `state->ProcessTick(id)` para cada um; depois a verificação de troca de estado.

A **verificação de troca de estado** (inlinada em todo lugar) é
`if (s && s->NeedChangeState()) { s->FinishState(); s = s->GetNextState(); if (s) s->StartState(); }`.
Os estados são singletons; o antigo não é deletado. Valor de retorno: "algo foi processado" (usado por `Run` para
decidir se deve dormir).

### 3.3 Helper de logging (funcs 2072 / 2921)

`LogDebug(fmt, ...)` → `LogSistema(LOG_DEBUG, s_urnaReal, fmt, ap)`: a primeira chamada guarda em cache
`access("/dev/urna", F_OK) == 0` (−1 = desconhecido). Numa urna, chama o `vsyslog` da musl; em outros lugares, imprime somente
se a variável de ambiente `DEBUG_UENUX` existir, como `printf("%s: ", <local buffer>)` + `vprintf`. O helper
vem de um header com linkage interno; o wasm-opt mesclou as cópias de quatro unidades de tradução na func 2921
(caches @1534908 cthreadeleitor, @1601364 cthreadoperador, @1577092 cvalidamidia, @1552472 func 5895). No
simulador os dois ramos são silenciosos (sem `/dev/urna`, sem `DEBUG_UENUX`, e o syslog não consegue alcançar `/dev/log`, veja
`docs/libraries/libc-and-emscripten-runtime.md` §12).

### 3.4 Build web: a thread nunca roda

`main()` registra `vota::CExecucaoVotaCooperativa` como `IExecucaoVota` (func 7828). Seu slot 2 apenas instala o
primeiro estado (um novo `CAppStateContext` na thread do eleitor, func 4713) e seu slot 7 (func 7823) chama
`CThreadEleitor::GetInst().Processar()` uma vez. `votaTick` (func 10619) chama o slot 7 a cada tick. `CThread::Start`
(func 1684) só é chamado pela política da urna `CExecucaoVota` (funcs 10233/10235/10236), então `Run`, o `Yield`
e o `emscripten_sleep(50)` são código morto no simulador. A thread do operador não é avançada de forma alguma.
(Mesmo que uma thread fosse iniciada, `simulador::CWasmThread::Create` (func 9655) apenas acrescenta o ponto de entrada a uma
fila, e `CWasmThread::Yield`/`vf3` (Wait) chamam `emscripten_sleep`, que o glue implementa como
`abort("Please compile your program with async support…")`.) `CThread::Start` também é o único lugar que
limpa `m_bParar` (+8); a política cooperativa nunca faz isso.

---

## 4. Tornando um voto durável: `CSincronismoVotoEleitor::SincronizaVoto`

### 4.1 Gatilho (urna)

1. `CEleitorVotando` registra os votos confirmados no RDV em memória (func 4454, unidade u22) e passa para
   `CSincronismoEleitor`.
2. `CSincronismoEleitor::StartState` (func 7178): posta a mensagem **13** na fila do operador
   (`CThreadOperador` +36), reinicia `CTelasVota::m_barraProgresso` no seu mínimo (func 3667) e mostra
   `m_telaProgressoRegistroVoto` ("Gravando" + uma barra de 4 passos).
3. Na thread do operador, a mensagem 13 leva `CMostraEleitorVotando` (func 10425) a `CSincronismoOperador`. Seu
   `StartState` (func 10210) marca o eleitor como tendo votado (`CEleitores::MarcaVotou`, inlinada, pulada em
   treinamento eleitor) e posta a mensagem **5** de volta na fila do eleitor (`CThreadEleitor` +36).
   `CSincronismoOperador::ProcessMessage` (func 10209) só trata as mensagens 1, 14 e 15.
4. `CSincronismoEleitor::ProcessMessage(5)` (func 7181) chama `ISincronismoVotoEleitor::GetInst().SincronizaVoto()`
   (o `GetInst` de csincronismovotoeleitor.cpp:67 está inlinado ali: registra por padrão `CSincronismoVotoEleitor`,
   4 bytes) e, quando ela retorna true, passa para `CFimVotoEleitor` ("FIM").

### 4.2 `SincronizaVoto` (func 7174), passo a passo

A função avança a barra de progresso quatro vezes em torno de duas funções inlinadas de `csincronizavota.cpp`
(namespace anônimo; as linhas srcloc 290 e 303 são os seus pontos de `throw`):

```
AvancaBarraProgresso()                                 (1/4)
SincronizaRDVInternoSeguro()     ── MI
AvancaBarraProgresso()                                 (2/4)
CLogVota::Loga("O voto do eleitor foi computado")      (log level 1)
AvancaBarraProgresso()                                 (3/4)
SincronizaRDVExternoSeguro()     ── MV
AvancaBarraProgresso()                                 (4/4)
return true
```

**SincronizaRDVInternoSeguro** (MI):

1. `CSincronizaVota::VerificaUrnaDesligando()`: se a flag de desligamento @1832936 está ligada, lança
   `api::CUeDesligandoError` (csincronizavota.cpp:47).
2. `rdv = comum::CRdvVota::GetInst()`; caminho `P = <trab dir MI>/rdv.dat` (func 1551).
3. Gravação: `CEncryptedFile f(rdv.m_cifrador)` (func 1692; o `shared_ptr` da cifra está em `CRdvVota+8`),
   `f.MemWrite(rdv.Converte())` (slot 11 de CRdv = BER de `ModuloRegistroDigitalVoto`), `f.Save(P + ".tmp")`
   (func 2767, cencryptedfile.cpp:88).
4. Releitura: novo `CEncryptedFile`, `Load(P + ".tmp")`, `MemRead(buf)`, `rdv.ConfereConteudo(buf)` (slot 13).
   Em caso de divergência: lança `CBaseError<vota::EUeVotaError>` **9300 "Falha na gravação do RDV na MI"** (:290).
   Nesse caso, o arquivo `.tmp` é deixado para trás.
5. `comum::VerificaIntegridadeReferencial()` (func 2543: verifica cruzadamente CRdvVota, CCargos, CRespostas).
6. `api::CSystem::ReplaceFile(P + ".tmp", P, "")` (func 5455, "CSystem::ReplaceFile").
7. `CAppInfo::SalvaVotaInterno()` (func 3790): `vota.bin` (`ModuloEstadoGeralVota`) do turno atual.
8. A menos que `EhTreinamentoEleitor()` (func 697: fase `'3'` treinamento **e** `EstadoGeralVota.treinamentoEleitor`):
   grava o registro dinâmico do eleitor atual em `<trab MI>/uenux.db` (`CEleitorDinamicoDAO`, SQLite), sync, depois
   `CAssinador(pkg).Assina(110 = uenux.db)`, sync.
9. `CAssinador(pkg).Assina(83 = rdv.dat); Assina(31 = vota.bin)`; sync. `pkg` = ESavdPacote 122 (turno 1) ou 123
   (turno 2) = `vota.vsu`; o turno é `CEstadoGeral +32` (`'1'`/`'2'`), e cada turno tem o seu próprio diretório
   de trabalho (`dinamico/trab1`, `dinamico/trab2`) nas duas memórias.
10. `VerificaAssinaturaMI("vota.vsu")`, `("rdv.vsu")` e `("uenux.vsu")`, a menos que seja treinamento eleitor
    (votadefs.cpp:103: `IInterfaceSavd::ValidarUE(<trab MI>/<name>)`).

**SincronizaRDVExternoSeguro** (MV): os passos 1–7 novamente com `<trab MV>/rdv.dat` (func 1701), erro **9301
"Falha na gravação do RDV na MV"** (:303) e `SalvaVotaExterno` (func 3789). Depois, em vez de assinar novamente:

* a menos que seja treinamento eleitor: copia `uenux.db` MI → MV (`CArquivosSavd` ids 110 → 111) e chama
  `CopiaAssinaturaBancoDadosParaMV()` (func 4682: sob `CApplicationContextGuard(4, "", "Gravando o banco de
  dados na MV", "Ocorreu um erro durante a persitência dos dados na MV.")` copia `uenux.vsu` 199 → 201 (turno 1)
  ou 200 → 202 (turno 2));
* copia os pacotes de assinatura da MI para a MV: `vota.vsu` 122 → 124 e `rdv.vsu` 134 → 136 (turno 1) ou 123 → 125
  e 135 → 137 (turno 2);
* sync; `VerificaAssinaturaMV` de `vota.vsu`, `rdv.vsu` e (a menos que seja treinamento eleitor) `uenux.vsu`.

Como as cópias de `rdv.dat`/`vota.bin` na MV são gravadas novamente mas as suas assinaturas são copiadas da MI,
a verificação da MV só pode passar se as duas gravações produzirem bytes idênticos, isto é, se a cifragem do RDV for
determinística (inferência; a `CAesKey` da u01 carrega o seu IV).

Os ids de `ESavdArquivoUE`/`ESavdPacote` vêm das tabelas construídas em `CArquivosSavd::GetInst` (func 1164):
31 `vota.bin`, 83 `rdv.dat`, 110/111 `uenux.db`; 120/121 `eg.vsu`; 122–125 `vota.vsu`; 126–129 `bu.vsu`;
130–133 `buj.vsu`; 134–137 `rdv.vsu`; 142–145 `rze.vsu`; 146–149 `gap.vsu`; 152–155 `red.vsu`; 191–194 `bim.vsu`;
195–198 `behb.vsu`; 199–202 `uenux.vsu`; 203 `dadoscarga.vsu`.

**Relação com o BU.** Esta unidade nunca constrói o Boletim de Urna. Ela é a metade por eleitor da cadeia de resultado:
todo voto termina no RDV cifrado (`rdv.dat`) e nos contadores de `vota.bin`, nas duas memórias, assinados
(`rdv.vsu`, `vota.vsu`). O BU gerado no *encerramento* (unidades u08/u09) é calculado a partir desse RDV/estado.

### 4.3 No simulador

`main()` insere `CSincronismoVotoEleitorWeb` primeiro, então `SincronizaVoto` é `return true` (func ICF 434) e nada
do que está acima é executado: nenhum `rdv.dat`, `vota.bin`, `uenux.db` ou `.vsu` muda após um voto (confirmado pelos snapshots do MEMFS,
`analysis/runtime/README.md`). A thread do operador também não é avançada, então o próprio `votaTick` faz o papel dos
passos 3–4 do §4.1: enquanto o nome do estado contém `"CSincronismoEleitor"`, ele chama
`CTelasVota::GetInst().AvancaBarraProgresso()` (func 2369) em t+120 ms, +280, +440, +600 ms (a cada 160 ms, quatro
vezes), depois posta a mensagem 5 através do slot 8 de `IExecucaoVota` (func 10376). A barra "Gravando" que o eleitor vê é uma
animação temporizada; as transcrições mostram `CSincronismoEleitor` → 5 atualizações de tela → `CAguardaMensagem` ("fim").

`CopiaAssinaturaBancoDadosParaMV` (func 4682) *é* executada no simulador, mas somente durante `votaInit`: a
inicialização da persistência no arranque (func 4662 → func 4657, "Gravando o banco de dados na MI": assina `uenux.db`,
copia-o para a MV) a chama para copiar `uenux.vsu` para a MV (amostras do profiler nas duas sessões gravadas). No
snapshot do MEMFS após `votaInit`, `/dsk/fi/dinamico/trab{1,2}/uenux.vsu` e `/dsk/fe/dinamico/trab{1,2}/uenux.vsu`
contêm todos o texto de assinatura simulada "assinatura simulada para vota_web_wasm".

---

## 5. As telas de votação (`CTelasVota`)

### 5.1 Como as telas são construídas

O construtor de `CTelasVota` (ctelasvota.cpp:3502, inlinado na função de inicialização 7787 da u02) constrói, para cada
cargo, um `CTelasCargo` que contém um `CFormInterativoTelaVota` por `ETelaVotacao` (18 tipos, veja u06 §5), além das
telas fixas. Cada tela é um `api::CFormBuilder` (um `vector<shared_ptr<IFormField<IScreen>>>`) transformado em um
`api::CInteractiveForm<IScreen, IInputKbd>` (func 554) com um hook `CPreShowFormVota`/`CPreShowProgressBar`.
`CFormBuilder::Add` (func 426) nomeia cada campo como `<ClassName><n>` (`"CTextField1"`, …) usando o slot 7 de
`IFormFieldBase`. As fábricas por `ETelaVotacao` que chamam os builders abaixo (funcs 1591, 1767, 3063, 3067, 6616, 6626,
6639, 6681: `CriaTela…(cargo)`, cada uma passando `__func__` como `nomeTela`), os rodapés ("Aperte a tecla: CONFIRMA para …
/ CORRIGE para REINICIAR este voto", func 1102) e o construtor são reconstruídos pela unidade u02 em
`src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp`. Os textos são **fontes de dados**, avaliadas no momento do desenho:

| fonte | usada para |
|---|---|
| `DS_NomeCargoNeutroComEscolha{copy of CCargo}` (func 6627 / 12587) | linha superior: nome do cargo em gênero neutro + `" - " + "{}ª vaga"` quando o cargo tem várias vagas (Senado) |
| slot 1088 → func 12811 | nome do cargo no *gênero do candidato* (`CCargoDSNomeSexoCandidato{0,0}`: "Prefeita") |
| slot 1089 → func 11501 (`CDataTextFmt`, formato `"{}"`) | texto do partido do candidato atual |
| slots 1090/1092, 1093 | nomes do companheiro de chapa 1 / 2;  slots 1091/1095, 1094: os títulos dos seus cargos (legendas das fotos) |
| slot 1096 → resposta atual de `CRespostas`; `CRespostasDSNumero` | texto / número da resposta da consulta |
| slot 1098 → `g_votoDigitado` (@1833288) | os dígitos digitados até o momento (caixas cinzas) |
| slot 1099 → `DS_CandidatoNaoConcorre` (func 13101) | "CANDIDATO NÃO CONCORRE" / "CANDIDATA NÃO CONCORRE" (sexo 2 = feminino); lança 9333 se nenhum candidato estiver selecionado |
| `CCandidaturasDSNumero`, `CCandidaturasDSNome{0}`, `CCandidaturasDSFoto{i}` | número, nome e foto *i* (0 = titular) do candidato atual |

Tela lógica de 640×480. Fontes: {40} caixas de dígitos / mensagens grandes, {30 negrito} nome do cargo, {25} nomes, {20} textos
pequenos, {35} pergunta da consulta, {13} legendas das fotos.

### 5.2 Catálogo de telas (as funções desta unidade)

| builder (linha) | usado para | conteúdo (posições x,y) |
|---|---|---|
| `adicionaNomeCargo` (func 1192) | todas as telas | nome do cargo em (10,60); duas linhas (10,60)/(10,95) até x=639 quando o cargo mostra fotos |
| `adicionaNumeroDigitado` (3060) | nulo/legenda/inexistente/inapto | caixas cinzas com os dígitos digitados em (10,115) |
| `adicionaPartido` (2028) | legenda, inexistente, tela completa | linha do partido em (10,250), somente se `CConfiguracaoEleicao +484` |
| `adicionaDadosCandidato` (4185) | tela completa | cargo (gênero do candidato) em (10,60)[/(10,95)], caixas do número (10,115), nome do candidato (10,190)/(10,215) |
| `adicionaFotoCandidato` (4184) | tela completa | foto 161×225, canto superior direito em (640,36); legenda opcional (cargo no gênero do candidato) centralizada abaixo |
| `adicionaFotoSuplente` (4181) | tela completa com vice/suplentes | foto 111×155, canto superior direito em (x+1,288), legenda centralizada abaixo (fonte 13) |
| `adicionaPerguntaConsulta` (3074) | consultas | a pergunta (`'|'` = nova linha) centralizada em (10,50)-(629,255); retorna base+23 |
| `adicionaBaseTelaCompletaCandidatoCom0` (:822, 6678) | candidato encontrado, 0 companheiros de chapa | cabeçalho + partido (exceto código de cargo 25) + foto sem legenda |
| `…Com1` (:853, 6671) | 1 companheiro de chapa (vice) | + nome do companheiro de chapa (10,320)/(10,340) + fotos com legenda |
| `…Com2` (:890, 6660) | 2 companheiros de chapa (Senado) | + nomes (10,310)/(10,330) e (10,355)/(10,375) + até duas fotos pequenas, da direita para a esquerda |
| `adicionaBaseTelaCompletaConsulta` (:942, 6652) | resposta de consulta encontrada | pergunta + caixas do número em (100−40·dígitos, y) + texto da resposta em (100, y+3) |
| `adicionaBaseTelaVotoBrancoCandidato` (:964, 6646) | voto branco | cargo + **"VOTO EM BRANCO"** piscante em (195,200) |
| `adicionaBaseTelaVotoBrancoConsulta` (:993, 6641) | consulta em branco | pergunta + "VOTO EM BRANCO" piscante em (320, y) |
| `adicionaBaseTelaVotoNuloConsulta` (:1014, 6624) | consulta nula | pergunta + caixas + texto + **"VOTO NULO"** piscante em (320,345) |
| `adicionaBaseTelaVotoNuloCandidato` (:1049, 6608) | voto nulo | cargo + caixas (dígitos − n) + texto em (10,190) + "VOTO NULO" piscante em (195,345) |
| `adicionaBaseTelaCandidatoInexistente` (:1075, 6605) | proporcional: número desconhecido, partido válido | cargo + caixas + **"CANDIDATO INEXISTENTE"** + partido + **"VOTO DE LEGENDA"** piscante |
| `adicionaBaseTelaCandidatoInapto` (:1101, 6603) | proporcional: candidato que não concorre | cargo + caixas + "CANDIDATO/A NÃO CONCORRE" + "VOTO NULO" piscante em (320,345) |
| `adicionaBaseTelaVotoLegenda` (:1126, 6602) | proporcional: voto apenas no partido | cargo + caixas + partido + "VOTO DE LEGENDA" piscante |
| `CriaTelaVotoCargoSemCandidato` (:1585, 3064) | cargo sem candidatos | cargo + **"NÃO HÁ CANDIDATOS CONCORRENDO"** piscante em (320,200), linha (0,400)-(639,401), "Aperte a tecla:" (1,405), "CONFIRMA" alinhado à direita + " para continuar" alinhado à esquerda em (130,430); nome do formulário "telaVotoCargoSemCandidato" |
| lambda `$_2` de `CriaTelaVisualizacaoCandidato` (:3366, 12272) | "visualizar candidatos" do operador | para cada companheiro de chapa: foto 111×155, legenda, linha "<cargo>: <nome>", linha "Gênero: masculino/feminino/não informado"; lança 9356 "Sem suporte a mais de 2 suplentes" |

Os builders `adicionaBase*` verificam primeiro o tipo de cargo e lançam `CBaseError<vota::EUeVotaError, {9300,9500}>`
com a mensagem `"<tela> - cargo <código> (<nome>) - não é de candidato | não é de proporcional | não é de
consulta | com número de suplentes incompatível"` (func 690). Códigos: 9333 (:294) … 9357 (:3502), listados em
`ctelasvota.h`; 9343 não é usado porque `adicionaBaseTelaVotoNuloConsulta` lança 9344, assim como
`adicionaBaseTelaVotoNuloCandidato`.

Os textos piscantes são `api::CTextFieldBlinking` (cores 2/3 sobre 1, timer de 500 ms de `ITimerScheduler`). As strings são
armazenadas em Latin-1 (por exemplo, `"NÃO HÁ"` = `4E C3 4F 20 48 C1`), portanto os fontes originais são codificados em ISO-8859-1.

---

## 6. Estados de reinício e de zerésima (fragmentos)

**`CReinicioVotacao::StartState`** (func 11835) decide o que mostrar após um reinício:

```
if RDV comparecimento (max over eleições, func 1269) != 0  or  CJustificador count (+8) != 0:
    log "Apresentada tela do reinício da votação"; show CTelasVota+148   (mesário confirms: ProcessInput 11834
                                                                          logs "Mesário confirmou o reinício da votação")
elif EhTreinamentoEleitor():
    next = exists(<trab MI>/ze.dat) ? CQuerReimprimirZeresima : CQuerImprimirZeresima
elif EstadoGeralVota.urnaIdGerouZeresima present and != CEstadoGeral id (+60):
    next = CConfirmaRegerarZeresima          (the zerésima was produced by another urna)
else:
    next = CQuerReimprimirZeresima
```

`testeteclado::CRetomada` (teste de teclado antes da retomada) retorna `CReinicioVotacao` a partir do slot 12 quando pular o
teste era permitido. `CGeraZeresima`/`CRegerarZeresima` retornam o estado "resumo" correspondente a partir do slot 10 (corpo
singleton mesclado, func 6056). Todos esses estados obtêm a sua tela de `CTelasVota` nos seus construtores, e é
por isso que as ferramentas os agruparam com esta unidade. `CProgressoEncerramento::Avanca` (func 12101) avança a barra de progresso
da tela de *encerramento* sob o seu mutex (o mesmo `CProgressBar::Incrementa`, func 5508).

---

## 7. O que é específico da build web

* **Thread do eleitor não iniciada.** `CExecucaoVotaCooperativa` substitui o início/join das threads; `votaTick` → slot 7 de IExecucaoVota
  → `CThreadEleitor::Processar()` uma vez por tick. `Run`/`emscripten_sleep` estão mortos (§3.4).
* **Persistência do voto substituída.** `CSincronismoVotoEleitorWeb::SincronizaVoto()` retorna `true`; a gravação real
  de RDV/`vota.bin`/`uenux.db`, a assinatura e a verificação do §4.2 estão compiladas, mas são inalcançáveis.
* **Barra "Gravando" animada por timers** em `votaTick` (4 × `AvancaBarraProgresso`, com 120 ms e depois 160 ms de intervalo),
  e então a mensagem 5 é postada pelo adaptador em vez de pela thread do operador.
* **Modo de áudio** definido por `votaInit` a partir da opção de página `audioEleitorHabilitado` (mensagem 9 ou modo 2 direto).
* **Treinamento eleitor**: os cenários rodam na fase `'3'` com `treinamentoEleitor = TRUE`, então
  `EhTreinamentoEleitor()` (func 697) é verdadeiro e todo ramo do banco de dados de eleitores (uenux.db, `uenux.vsu`) seria
  pulado mesmo pelo código real.
* O logging via syslog/`DEBUG_UENUX` deste arquivo é silencioso no navegador.

## 8. Observações notáveis sobre wasm / Emscripten

* **Srcloc inlinado engana nos nomes.** A func 4349 carrega o srcloc da `ProcessarEntrada(CAppStateContext&)` inlinada;
  a função externa não tem esse parâmetro (assinatura wasm `(i32)->i32`). As funcs 4181/4184 carregam o srcloc da
  `adicionaFotoEmoldurada<CCandidaturasDSFoto>` inlinada (:487, a chamada `IScreen::GetInst()`), mas são os chamadores
  que adicionam as legendas das fotos. A func 7061 foi nomeada apenas pela vtable (`vf2`, agora curada como `CThreadEleitor::Run`),
  enquanto os seus srclocs pertencem a `IAjusteInicial::GetInst` (cajusteinicial.cpp:57) e aos templates de CPolySingletonList.
  A func 7787 (antes mostrada pelas ferramentas como `CHKDFSeed::GetSeed`, agora `vota::CInformacaoEleitor::Inicializar`, nome
  inferido na u02) é a função de inicialização que contém todo o construtor de `CTelasVota`.
* **merge-similar-functions**: func 2921 (helper de logging de 4 unidades de tradução, prioridade + cache como parâmetros), func 3921
  (3 × `GetTelaCargoAtual(const std::string&)`, srcloc + código de erro como parâmetros), func 6050 (2 × `GetTelaCargoAtual()`),
  func 6056 (2 getters de singleton, mutex + ponteiro + vtable como parâmetros), func 253 → 710 (todo
  construtor `CBaseError<E>` recebe a sua vtable como parâmetro), func 2902 (destrutores de estados que contêm uma tela).
* **A propagação de constantes removeu parâmetros**: `CriaTelaVotoCargoSemCandidato(const CCargo&, TPosition)` tem um
  parâmetro wasm além do ponteiro de resultado (o `TPosition` sumiu); `adicionaBaseTelaVotoNuloCandidato` perdeu os seus 3 `TPosition` e o `bool`; `adicionaBaseTelaVotoBrancoCandidato`
  perdeu o seu `TPosition`; `adicionaFotoEmoldurada<CCandidaturasDSFoto>` foi especializada duas vezes (constantes x/y/w/h).
  O construtor de `CTextFieldDoubleLine` (func 3659) é chamado com uma cor a menos do que a sua assinatura no srcloc (a sua primeira
  cor é armazenada como a constante 2), e `CTextFieldMultiLine` (func 5498, wasm `(i32,i32,i32,i32)`) com as duas
  cores removidas.
* **Resíduo de thread única**: `std::lock_guard` aparece apenas como o stub `mutex::unlock` (func 150); `CSynchronizer::Sync()`
  (func 620) foi compilado como uma leitura morta; laços de `condition_variable::wait` no código inlinado de poly-singleton.
* **Chamadas de inicializadores estáticos**: `__wasm_call_ctors` chama `CPolySingletonList::contains<IAjusteInicial>` (2450) e
  `contains<ISincronismoVotoEleitor>` (2741) e descarta os resultados (provavelmente verificações de registro `static const bool`
  em cajusteinicial.cpp / csincronismovotoeleitor.cpp).
* Slots da tabela de funções são usados como fontes de dados (`CDataText<std::string(*)()>` armazena o número do slot: 1088–1100).

## 9. Código suspeito ou digno de nota (detalhes na lista "suspicious" do StructuredOutput)

1. `CThreadEleitor::Run` (7061) chama `emscripten_sleep(50)` em todo ciclo ocioso, e em todo ciclo o seu
   `m_pImpl->Yield()` chega a `CWasmThread::Yield` (9651), que chama `emscripten_sleep(0)`. Esta build não tem
   Asyncify (o `_emscripten_sleep` do glue é um `abort`), então `Run` abortaria no seu primeiro ciclo. Ele só é inalcançável
   na página distribuída porque `main()` registra a política cooperativa.
2. A tela "Gravando" do simulador é uma animação: `SincronizaVoto` é `return true`; nada é gravado ou assinado.
   Verificado novamente em 2026-09-23 com `headless.mjs --dump-fs` (municipal-t1, Vereador 91001 + Prefeito 12): o único arquivo
   que difere de uma execução sem voto é `dsk/fi/dinamico/log/logd.dat`, e ele não tem nenhum registro "O voto do eleitor foi
   computado" (a linha de log que o `SincronizaVoto` real grava).
3. `LogSistema` (2921) imprime como prefixo um buffer de pilha não inicializado de 140 bytes (sp+16..sp+155) quando `DEBUG_UENUX` está
   definida e `/dev/urna` está ausente. Latente: a página e o glue nunca definem `DEBUG_UENUX`.
4. O código de erro 9344 é usado por dois builders diferentes (`…VotoNuloConsulta` :1014 e `…VotoNuloCandidato` :1049), e
   9343 não aparece em lugar nenhum como código de erro. Só o número é ambíguo: a linha do srcloc e a mensagem ("não é de
   consulta" / "não é de candidato") ainda identificam o builder.
5. As posições de legenda de fallback em 4181/4184 estão do lado errado das fotos ancoradas à direita (x + w/2 em vez de x − w/2;
   (719,261) fica fora da tela). Morto na prática: `CWasmImageSurfaceOps::CalcRect` (func 9289) sempre retorna um
   retângulo de largura w+1, então o ramo `largura <= 1` nunca é executado.
6. As assinaturas da MV são copiadas da MI em vez de recalculadas (§4.2); a correção depende de a cifragem do RDV ser
   determinística (não verificado aqui).
7. Não há rollback entre as metades MI e MV de `SincronizaVoto`: uma falha 9301 deixa o voto efetivado apenas na MI.
8. As mensagens 0 e 6 da fila do eleitor param a thread do eleitor; na build cooperativa nada a reinicia (`CThread::Start`
   é o único código que limpa `m_bParar`, e todo passo posterior de `votaTick` retornaria `false`). Latente: na build
   web, apenas as mensagens 1, 5 e 9 são postadas (funcs 11026, 10376, 11100 através da func 3903).
9. `adicionaDadosCandidato` (4185) dá ao título de cargo em duas linhas um limite direito de `xLimiteNome + 237`: 707 quando
   `…Com1`/`…Com2` mostram a foto do titular (elas passam 470) e 876 quando `…Com0` o faz (ela sempre passa 639). Os dois
   limites ficam além da borda direita da tela (639), então o título nunca é quebrado antes da foto de 161 px em
   x 479..640. Outros campos de linha dupla usam limites dentro da tela.
10. As verificações da quantidade de companheiros de chapa discordam: `…Com2` (6660) só rejeita `qtd <= 1`, então aceitaria 3 ou mais,
    enquanto a lambda de `CriaTelaVisualizacaoCandidato` (12272) lança 9356 "Sem suporte a mais de 2 suplentes".
    Inofensivo nesta build: o único chamador (o construtor na func 7787) despacha exatamente em 0, 1 ou 2 e lança
    9357 "Tipo de cargo nao identificado" para qualquer outra quantidade.

## 10. Tabela de mapeamento completa (todas as 65 funções da u07)

"executou" = observada em execução durante os votos gravados.

| # | func wasm | tamanho | executou | símbolo reconstruído | arquivo-fonte / destino |
|---|---|---|---|---|---|
| 1 | 253 | 18 |  | `ecourna::api::exception::CBaseError<vota::EUeVotaError, SErrorLimits{9300,9500}>::CBaseError(int, std::string&&, const std::source_location&)` (= `vota::CUeVotaError`) | biblioteca/helper inlinado: thunk de merge-similar para o corpo compartilhado do construtor 710 (vtable @1532440 como argumento extra); lançado por todos os caminhos de erro desta unidade |
| 2 | 407 | 24 |  | `vota::CTelasVota::GetInst()` (ctelasvota.cpp:3434) | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 3 | 426 | 658 | sim | `api::CFormBuilder::Add(std::shared_ptr<IFormField<IScreen>>)` (nome único "<Classe><n>" + push_back) | src/uenux2/src/api/gui/cformbuilder.u07.cpp |
| 4 | 690 | 735 |  | `vota::(anon)::MensagemErroCargo(nomeTela, cargo, motivo)` | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 5 | 1157 | 36 |  | `comum::md::CCargo::GetQtdSuplentes() const` | src/uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.u07.cpp |
| 6 | 1191 | 292 | sim | instanciação de `api::CFormBuilder::Add<CTextField>(SPoint, make_shared<CDataText<std::string(*)()>>, SFont, 2, 1)` | biblioteca/helper inlinado (template de cformbuilder.h); resumido em src/uenux2/src/api/gui/cformbuilder.u07.cpp |
| 7 | 1192 | 1027 | sim | `vota::(anon)::adicionaNomeCargo(const CCargo&, CFormBuilder&)` | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 8 | 1546 | 16 |  | `comum::md::CCargo::PossuiFoto() const` | src/uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.u07.cpp |
| 9 | 1952 | 12 |  | `vota::CGeraResumoZeresimaBase::~CGeraResumoZeresimaBase()` (D1, via corpo compartilhado 2902) | src/uenux2/src/app/vota/eleitor/iniciovotacao/cgerazeresima.u07.cpp (caminho inferido) |
| 10 | 2028 | 502 | sim | `vota::(anon)::adicionaPartido(CFormBuilder&)` | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 11 | 2072 | 16 | sim | `vota::(anon)::LogDebug(const char*, ...)` (LOG_DEBUG, cache @1534908) | src/uenux2/src/app/vota/eleitor/cthreadeleitor.cpp |
| 12 | 2369 | 11 | sim | `vota::CTelasVota::AvancaBarraProgresso()` | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 13 | 2438 | 166 |  | `vota::CThreadEleitor::~CThreadEleitor()` (slot 0 da vtable) | src/uenux2/src/app/vota/eleitor/cthreadeleitor.cpp |
| 14 | 2450 | 19 | sim | `api::CPolySingletonList::contains<vota::IAjusteInicial>(TPolySingletonsInfo&)` | biblioteca/helper inlinado: instância de template (cpolysingletonlist.h) usada por IAjusteInicial::GetInst inlinada em Run; também chamada (com o resultado descartado) por __wasm_call_ctors |
| 15 | 2782 | 343 | sim | instanciação de `api::CFormBuilder::Add<CTextFieldMultiLine>(SRect, make_shared<CFixedText>, SFont)` | biblioteca/helper inlinado (template); resumido em src/uenux2/src/api/gui/cformbuilder.u07.cpp |
| 16 | 2921 | 118 | sim | `LogSistema(prioridade, int& urnaReal, fmt, va_list)` - corpo mesclado de um helper de header (access("/dev/urna") -> vsyslog, senão DEBUG_UENUX -> printf) | src/uenux2/src/app/vota/eleitor/cthreadeleitor.cpp (header de origem desconhecido; também atende cthreadoperador.cpp, cvalidamidia.cpp, 5895) |
| 17 | 3060 | 294 | sim | `vota::(anon)::adicionaNumeroDigitado(form, digitos, pos)` (= Add<CMaskedTextField<CGrayedFramedText>>) | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 18 | 3064 | 1307 | sim | `vota::CTelasVota::CriaTelaVotoCargoSemCandidato(const CCargo&, TPosition)` (:1585) | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 19 | 3068 | 294 |  | instanciação de `api::CFormBuilder::Add<CTextFieldDoubleLine>(SPoint, SPoint, TPosition, make_shared<CDataText<std::string(*)()>>, SFont, TColor)` | biblioteca/helper inlinado (template); resumido em src/uenux2/src/api/gui/cformbuilder.u07.cpp |
| 20 | 3074 | 428 |  | `vota::(anon)::adicionaPerguntaConsulta(form, cargo)` -> TPosition | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 21 | 3921 | 97 | sim | corpo mesclado de `CConfirmaVotoEmCargo::GetTelaCargoAtual(const std::string&)` / `CConfirmaVotoSemCandidato::` / `CCompletaProporcional::` (srcloc + código como argumentos extras) | src/uenux2/src/app/vota/eleitor/cconfirmavotoemcargo.cpp (escrito pela u06) |
| 22 | 4129 | 280 |  | instanciação de `api::CFormBuilder::Add<CImageField>(SPoint, make_shared<CDataImage<comum::CCandidaturasDSFoto>>, anchor 1)` | biblioteca/helper inlinado (template); resumido em src/uenux2/src/api/gui/cformbuilder.u07.cpp |
| 23 | 4181 | 427 | sim | `vota::(anon)::adicionaFotoSuplente(form, legenda, x, indice)` -> SRect (contém `adicionaFotoEmoldurada<CCandidaturasDSFoto>` :487 inlinada) | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 24 | 4184 | 610 | sim | `vota::(anon)::adicionaFotoCandidato(form, comLegenda)` (contém `adicionaFotoEmoldurada<CCandidaturasDSFoto>` :487 inlinada) | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 25 | 4185 | 937 | sim | `vota::(anon)::adicionaDadosCandidato(form, cargo, xLimiteNome)` | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 26 | 4349 | 1427 | sim | `vota::CThreadEleitor::Processar()` (externa; inlina ProcessarMensagens, ProcessarEntrada(CAppStateContext&) :134, ProcessarTicks) | src/uenux2/src/app/vota/eleitor/cthreadeleitor.cpp |
| 27 | 4682 | 549 | sim | `vota::CopiaAssinaturaBancoDadosParaMV()` (uenux.vsu MI->MV sob um CApplicationContextGuard) | src/uenux2/src/app/vota/comum/csincronizavota.u07.cpp |
| 28 | 5508 | 110 | sim | `api::CProgressBar::Incrementa()` | src/uenux2/src/api/gui/cprogressbar.u07.cpp |
| 29 | 5535 | 25 | sim | `api::CGrayedFramedText::CGrayedFramedText(size_t, const SPoint&)` | src/uenux2/src/api/gui/cgrayedframedtext.u07.cpp |
| 30 | 5938 | 142 |  | `vota::CReinicioVotacao::GetInst()` (construtor inlinado) | src/uenux2/src/app/vota/eleitor/iniciovotacao/creiniciovotacao.u07.cpp (caminho inferido) |
| 31 | 5940 | 142 |  | `vota::CQuerReimprimirZeresima::GetInst()` (construtor inlinado) | src/uenux2/src/app/vota/eleitor/iniciovotacao/cquerreimprimirzeresima.u07.cpp |
| 32 | 5957 | 142 |  | `vota::CQuerImprimirZeresima::GetInst()` (construtor inlinado) | src/uenux2/src/app/vota/eleitor/iniciovotacao/cquerimprimirzeresima.u07.cpp (caminho inferido) |
| 33 | 5965 | 68 |  | `vota::CGeraZeresimaBase::CGeraZeresimaBase()` | src/uenux2/src/app/vota/eleitor/iniciovotacao/cgerazeresima.u07.cpp (caminho inferido) |
| 34 | 6050 | 88 | sim | corpo mesclado de `CPedeMajoritario::GetTelaCargoAtual()` / `CPedeProporcional::GetTelaCargoAtual()` | src/uenux2/src/app/vota/eleitor/votamajoritario/cpedemajoritario.u07.cpp |
| 35 | 6056 | 75 |  | corpo mesclado de `CGeraResumoZeresima::GetInst()` / `CRegeraResumoZeresima::GetInst()` (mutex, instância, vtable como argumentos) | src/uenux2/src/app/vota/eleitor/iniciovotacao/cgerazeresima.u07.cpp (caminho inferido) |
| 36 | 6528 | 295 |  | instanciação de `api::CFormBuilder::Add<CTextField>(SPoint, make_shared<CDataText<comum::CCargoDSNomeSexoCandidato>>, SFont, 2, 1)` | biblioteca/helper inlinado (template); resumido em src/uenux2/src/api/gui/cformbuilder.u07.cpp |
| 37 | 6561 | 1320 |  | `vota::(anon)::adicionaFotoEmoldurada<std::__bind<lambda@3323&, const CDadosCandidato&>>(form, x, y, w, h, fonte)` (:487) | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 38 | 6602 | 374 | sim | `vota::(anon)::adicionaBaseTelaVotoLegenda` (:1126) | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 39 | 6603 | 436 |  | `vota::(anon)::adicionaBaseTelaCandidatoInapto` (:1101) | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 40 | 6605 | 557 | sim | `vota::(anon)::adicionaBaseTelaCandidatoInexistente` (:1075) | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 41 | 6608 | 473 | sim | `vota::(anon)::adicionaBaseTelaVotoNuloCandidato` (:1049) | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 42 | 6624 | 729 |  | `vota::(anon)::adicionaBaseTelaVotoNuloConsulta` (:1014) | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 43 | 6641 | 272 |  | `vota::(anon)::adicionaBaseTelaVotoBrancoConsulta` (:993) | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 44 | 6646 | 264 | sim | `vota::(anon)::adicionaBaseTelaVotoBrancoCandidato` (:964) | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 45 | 6652 | 577 |  | `vota::(anon)::adicionaBaseTelaCompletaConsulta` (:942) | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 46 | 6660 | 514 | sim | `vota::(anon)::adicionaBaseTelaCompletaCandidatoCom2` (:890/:895) | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 47 | 6671 | 363 | sim | `vota::(anon)::adicionaBaseTelaCompletaCandidatoCom1` (:853/:858) | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 48 | 6678 | 192 | sim | `vota::(anon)::adicionaBaseTelaCompletaCandidatoCom0` (:822/:827) | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |
| 49 | 6743 | 9 |  | `vota::CInformacaoEleitor::HabilitaAudio()` (m_modoAudio = 1) | src/uenux2/src/app/vota/eleitor/comum/cinformacaoeleitor.u07.cpp |
| 50 | 6745 | 9 |  | `vota::CInformacaoEleitor::HabilitaAudioConformeCadastro()` (m_modoAudio = 0) | src/uenux2/src/app/vota/eleitor/comum/cinformacaoeleitor.u07.cpp |
| 51 | 7030 | 18 |  | `vota::CThreadEleitor::FinalizaExecucao()` (slot 5 da vtable) | src/uenux2/src/app/vota/eleitor/cthreadeleitor.cpp |
| 52 | 7061 | 4119 |  | `vota::CThreadEleitor::Run()` (slot 2 da vtable; IAjusteInicial::GetInst cajusteinicial.cpp:57 inlinada) | src/uenux2/src/app/vota/eleitor/cthreadeleitor.cpp |
| 53 | 7070 | 13 |  | destrutor de deleção de `vota::CThreadEleitor::~CThreadEleitor()` (slot 1) | src/uenux2/src/app/vota/eleitor/cthreadeleitor.cpp |
| 54 | 7122 | 38 |  | destrutor atexit do `std::unique_ptr` estático de `CThreadEleitor` (@1833212) | src/uenux2/src/app/vota/eleitor/cthreadeleitor.cpp |
| 55 | 7174 | 3288 |  | `vota::impl::CSincronismoVotoEleitor::SincronizaVoto()` (slot 2; csincronizavota.cpp:290/:303 inlinadas) | src/uenux2/src/app/vota/eleitor/csincronismovotoeleitor.cpp (+ comum/csincronizavota.u07.cpp para as partes inlinadas) |
| 56 | 11829 | 19 |  | `vota::testeteclado::CRetomada::GetEstadoSemTeste()` (slot 12) | src/uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cretomada.u07.cpp (caminho inferido) |
| 57 | 11835 | 712 |  | `vota::CReinicioVotacao::StartState()` (slot 2) | src/uenux2/src/app/vota/eleitor/iniciovotacao/creiniciovotacao.u07.cpp (caminho inferido) |
| 58 | 11841 | 22 |  | `vota::CRegerarZeresima::GetEstadoResumo()` (slot 10) | src/uenux2/src/app/vota/eleitor/iniciovotacao/cgerazeresima.u07.cpp (caminho inferido) |
| 59 | 11847 | 38 |  | destrutor atexit da instância de `CRegeraResumoZeresima` (@1834884) | src/uenux2/src/app/vota/eleitor/iniciovotacao/cgerazeresima.u07.cpp (caminho inferido) |
| 60 | 11935 | 22 |  | `vota::CGeraZeresima::GetEstadoResumo()` (slot 10) | src/uenux2/src/app/vota/eleitor/iniciovotacao/cgerazeresima.u07.cpp (caminho inferido) |
| 61 | 11942 | 38 |  | destrutor atexit da instância de `CGeraResumoZeresima` (@1834184) | src/uenux2/src/app/vota/eleitor/iniciovotacao/cgerazeresima.u07.cpp (caminho inferido) |
| 62 | 11945 | 68 |  | `vota::CGeraResumoZeresimaBase::CGeraResumoZeresimaBase()` | src/uenux2/src/app/vota/eleitor/iniciovotacao/cgerazeresima.u07.cpp (caminho inferido) |
| 63 | 12101 | 18 |  | `vota::CProgressoEncerramento::Avanca()` (slot 4) | src/uenux2/src/app/vota/eleitor/fimvotacao/cprogressoencerramento.u07.cpp (caminho inferido) |
| 64 | 12272 | 1126 |  | operator() da lambda `$_2` em `CTelasVota::CriaTelaVisualizacaoCandidato` (:3366; std::function recursiva sobre os suplentes) | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp (como trecho comentado; a função envolvente é a func 6569 da u09) |
| 65 | 13101 | 217 |  | `vota::(anon)::DS_CandidatoNaoConcorre()` (:294; slot de tabela 1099) | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp |

Funções destes arquivos que **não** estão na u07, mas pertencem a eles: func 316 `CThreadEleitor::GetInst` (u18,
reproduzida em cthreadeleitor.cpp), `ISincronismoVotoEleitor::GetInst` (inlinada na func 7181, u19), o
construtor/`CreateInst`/`CriaTelaInputVazio*`/`CriaTelaPartido`/`CriaTelaZeresimaTardia` de `CTelasVota`
(inlinados na func 7787, u02), `CTelasVota::GetTelaCargo` (inlinada na func 4135, u06),
`CriaTelaVisualizacaoCandidato` (func 6569, u09) e `adicionaLinha` (func 1588, u09).

## 11. Questões em aberto

* Nomes exatos do ciclo externo (func 4349), de `SincronizaVoto` e dos slots de `IExecucaoVota` (sem srcloc).
* Significado das mensagens 0 vs 6 da fila do eleitor (ambas param a thread) e da mensagem 13 do lado do operador.
* O conteúdo do buffer de prefixo de 140 bytes de `LogSistema` no Linux (nome da thread? `prctl(PR_GET_NAME)`?).
* Qual campo de `CConfiguracaoEleicao` é o +484 (linha do partido exibida) e por que o código de cargo 25 oculta o partido em `…Com0`.
* Semântica de `CDetalheSuplente +1` (flag de foto) e das duas strings de `CDadosCandidato` (+0, +12).
* Se a cifragem do RDV é determinística (necessário para que a cópia da assinatura da MV seja validada).
