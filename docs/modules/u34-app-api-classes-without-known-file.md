# u34: classes `app:api` sem arquivo-fonte conhecido (estados do mesário, menus do início do dia, cabeçalho do QR do BU, código de suporte)

A unidade u34 contém **103 funções wasm** que as ferramentas colocaram no componente `app:api` sem um arquivo-fonte
original. Não há registro de `std::source_location` para elas, ou o que elas carregam pertence a um header `api`
inlinado (`cinteractiveform.h:57`, `ctextsource.h:37`). É por isso que foram arquivadas em `api`. Depois de
lidas, elas se dividem em cinco grupos:

| grupo | funções | o que é | § |
|---|---|---|---|
| **Estados do VOTA** (`vota::C…::ProcessInput`, slot 7 da vtable) | 17 + 6 helpers | 11 estados do microterminal do mesário e 3 estados da tela do eleitor do início do dia da eleição, mais 3 singletons (5430, 5942, 5962), os helpers 6012, 6014, 10539, 12903 e os dois corpos mesclados de `CLogVota` 6113/6115 | 3, 4, 6 |
| **Cabeçalho do QR code do BU** | 4 | setters de `CCabecalhoQRCodeBuilder` `ZONA:` `SECA:` `IDUE:` `IDCA:` dos QR codes do "BU digital" | 5 |
| **Código de suporte `api` do uenux2** | 46 | `CTime::IsValid`, `CDirReader::IsLink/Open`, `TrocaExtensao`, helpers do form builder, o relógio do cabeçalho de status, a pilha de formulários da tela do eleitor, o lock guard da fila de mensagens, destrutores mesclados, thunks de registro de poly-singletons, handlers de atexit | 7 |
| **Código comum / do simulador** | 7 | `CEleitores::GetEleitoresImpedidos`, `CCandidaturasDSNome`, rótulos de `CTradutorFrase`, destrutor de `CWasmLogBus` | 6, 7 |
| **Código de biblioteca arquivado aqui por engano** | 23 | 12 funções do RHVoice 1.14.0 (TTS do voto acessível) e 11 instâncias de libc++/libc | 8 |

**22 das 103 funções foram executadas** durante os votos gravados (`analysis/runtime/*.functions.tsv`). Todas elas
são código de suporte que roda no `votaInit` ou a cada tick: os registros de poly-singletons de `main`, o
carregador do cadastro de eleitores (`GetEleitoresImpedidos` 5767, `TrocaExtensao` 5461, `CTime::IsValid` 5449), a pilha
de formulários da tela do eleitor (`IForm<IScreen>::DoShow` 5541), o relógio do cabeçalho de status (10870), a barra de progresso por etapas (10971), a
fila de mensagens (7708, 5528) e algumas instâncias de biblioteca. **Nenhum dos estados do VOTA desta unidade pode rodar no
simulador.** Os estados do mesário pertencem à thread do operador, que o build web nunca executa (unidades u10,
u27). Os estados da tela do eleitor pertencem ao início do dia da eleição (reinício, zerésima, "Mais
informações"), e o simulador inicia a thread do eleitor depois desse ponto. Os setters do cabeçalho do BU só rodam
durante o *encerramento*, que a página nunca alcança.

**Arquivos-fonte reconstruídos** (todos fragmentos, sufixo `.u34`, a serem mesclados no arquivo indicado; "(path inferred)"
significa que nenhum srcloc nomeia o arquivo):

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

Alguns corpos já foram escritos por outras unidades e são apenas referenciados: `IForm<IScreen>::DoShow/DoShowOnTop/
Remove` (5541/5540/5987, `iform.h`, u17), `CabeLarguraProporcional` (10971, `cstepsprogressbar.cpp`, u16),
`CEncerraRegistroMesarios::GetInst` (5388, `cencerraregistromesarios.cpp`), `DefaultPolySingletonsInfo` (11265,
`cpolysingletonlist.h`, u19) e os três `ProcessInput`s de `CInformaEleitorPodeVotar /
CInformaAnoDesabilitadoDemo / CInformaBioDesabilitadaDemo` (10496/10504/10508,
`cinformaeleitorpodevotar.u33.cpp`, u33).

**Glossário.** *urna*: o equipamento de votação; *eleitor*: quem vota; *mesário*: membro da mesa receptora; *presidente (da mesa)*: o mesário
que preside a mesa; *terminal do mesário* / **MT**: o microterminal do mesário (LCD de 4 x 40, teclado numérico, LED,
buzzer, leitor de impressão digital); *habilitar o eleitor*: liberar um eleitor para votar; *título (de eleitor)*: número de
inscrição do eleitor; *áudio / fone de ouvido*: o voto acessível lido em voz alta pelos fones de ouvido;
*justificativa*: justificativa de ausência (um eleitor inscrito em outro lugar declara que não pôde votar lá);
*inspeção (da cabina)*: verificação periódica da cabina de votação; *encerramento*: fechamento da votação;
*zerésima*: o relatório de "zero votos" impresso antes do início da votação; *reinício*: reinício do VOTA durante o dia;
*BU (boletim de urna)*: relatório de resultado de cada urna; *via*: cópia impressa; *QR code do BU / BU digital*: os
QR codes assinados no fim do BU; *carga*: o carregamento dos dados da eleição na urna (identificado por
um *código de carga* de 24 dígitos); *impedido*: eleitor impedido de votar; *PU*: parametrização da urna
(`*-pu.dat`); *CORRIGE / CONFIRMA / BRANCO*: as teclas laranja / verde / branca.

---

## 1. Onde este código se encaixa no processo de votação

O VOTA executa duas máquinas de estados de objetos `comum::CAppState` (unidade u06 §2):

* a **thread do eleitor** (`CThreadEleitor`): a tela do eleitor de 640x480 e o teclado do eleitor. No dia da eleição, ela
  passa pelos estados do início do dia (teste de teclado, zerésima, perguntas de reinício, "Mais informações") e
  depois espera o mesário liberar cada eleitor;
* a **thread do operador** (`CThreadOperador`): o microterminal do mesário. Ela pede o título ou o
  CPF do eleitor, consulta o cadastro, captura a impressão digital, libera o eleitor, oferece "outras opções" (áudio, encerramento,
  registro de mesários, contadores) e periodicamente pede uma inspeção da cabina.

As duas threads se comunicam por filas de mensagens com prioridade (`api::CPriorityMessageQueue<api::SMessage>`, uma por
thread em `+36`). `Envia` é `Add(msg, 1)` (func 7708). Todas as chamadas `Add` do binário usam prioridade 1, exceto uma:
`CVerificaEleicaoPassou::StartState` (11914) posta a mensagem 0 na fila do operador com prioridade 100.

Esta unidade contém o **`ProcessInput()` (slot 7 da vtable) de 14 desses estados**. Os outros slots estão nas
unidades u10, u17, u19, u27, u33 e u39. As funções foram arquivadas em `api` porque
`api::CInteractiveForm<…>::Read()` está inlinado em cada uma delas, com seu registro de `std::source_location`
(`cinteractiveform.h:57`, a busca do teclado via poly-singleton).

Ao lado delas estão os setters do **cabeçalho dos QR codes do BU** e algum código de suporte comum da biblioteca `api`
que roda na inicialização.

---

## 2. Classes e hierarquia (RTTI)

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

Vtable comum desses estados: `[0]` destrutor = ICF 244 (libera o shared_ptr em +12), `[1]` destrutor
com delete = ICF 387, `[2]` `StartState` (o corpo ICF 1070 `m_proximoEstado = this; m_form->Show();`, salvo
indicação em contrário), `[3]` `NeedChangeState` 7480, `[4]` `GetNextState` 1661, `[5]` `FinishState` no-op, `[6]`
`ProcessMessage` no-op, `[7]` `ProcessInput`, `[8]` `ProcessTick` no-op. Todo estado é um singleton criado sob demanda
(`unique_ptr` estático + mutex estático; neste build single-threaded só resta o stub `mutex::unlock`, func
150). A maioria das funções `GetInst`, e seus construtores, está inlinada no estado que muda para
elas.

Outras classes encontradas nesta unidade: `comum::CCabecalhoQRCodeBuilder` (+0 `CCabecalhoQRCode`, 33 strings, 396
bytes), `api::CLockGuard` (4 bytes), `api::CDirReader` (20 bytes: +0 diretório, +12 `DIR*`, +16 `dirent*`),
`comum::CCandidaturasDSNome` (1 byte: índice do suplente), `simulador::CWasmLogBus` (168 bytes, u29).

---

## 3. Estados do microterminal (mesário)

"`m_form->Read()`" abaixo é o `CInteractiveForm<IScreenMT, IInputMT>::Read()` inlinado. Ele obtém o teclado do
MT com `CPolySingletonList::instance<api::IInputMT>(loc)` (func 383) e deixa o campo de entrada corrente
(`m_campos.at(m_indice)`, +72/+84, `std::out_of_range` além do fim) ler uma tecla (slot 8 da vtable do campo).
Retorna 5 para CORRIGE e 9 para CONFIRMA.

### 3.1 Fim de um voto com áudio: `CDesabilitaAudioEleitor::ProcessInput` (10435)

O MT bipa e mostra "Retire o fone de ouvido da urna / CONFIRMA". A thread do operador vai para lá quando o
eleitor que acabou de votar estava com o áudio ligado (`CInformacaoEleitor::m_modoAudio != 2`). Em CONFIRMA:

1. posta a **mensagem 10** (`MSG_AUDIO_DESABILITADO`) na fila da thread do eleitor;
2. faz **espera ativa** `while (CInformacaoEleitor::GetInst().m_modoAudio != 2) usleep(300);` até a thread do eleitor
   processar a mensagem. Não há timeout (veja a §11);
3. grava no log `"Áudio desativado pelo fim da votação"` (`CLogVota` func 4529);
4. próximo estado `CPedeIdentidade` ("Digite o Título ou o CPF").

### 3.2 Inspeção da cabina: `CConfirmaInspecionada::ProcessInput` (10527)

A inspeção periódica funciona assim. Entre um eleitor e outro, `CPedeIdentidade::ProcessTick` verifica um prazo
aleatório (agora + 60..90 min, `SorteiaProximaInspecao`, func 3620). Quando ele vence, o operador posta a mensagem
12 para o terminal do eleitor e espera em `CAguardaInspecao` ("Inspecione cabina e urna"). O mesário confirma no
teclado do eleitor (`CInspecionaUrna`, log "Inspeção da urna confirmada"). A thread do eleitor posta de volta a **mensagem
11**, e `CAguardaInspecao::ProcessMessage(11)` (10530) constrói `CConfirmaInspecionada` inline: relógio em
{33,1}, "Inspeção completa" centralizado na linha 2, "CONFIRMA: continuar a votação" alinhado à direita na linha 4.

Em CONFIRMA: log `"Inspeção da urna terminada"` (severidade 1), **mensagem 13** para a thread do eleitor
(`CUrnaInspecionada` mostra a tela de "continuar" e volta para `CAguardaMensagem`), sorteia o horário da próxima
inspeção e então vai para `CPedeIdentidade`.

### 3.3 Liberando um eleitor

* `CDigitalNaoCapturada::ProcessInput` (10458). Se a impressão digital do eleitor não é reconhecida, o mesário
  pode liberar o eleitor com a **própria** impressão digital (`CRegistraDigitalOperador`). Quando nenhum dedo é lido
  em 15 s e ainda restam tentativas, esta tela mostra "Digital não capturada / CONFIRMA: tentar novamente".
  CONFIRMA volta para `CRegistraDigitalOperador::GetInst()` (5396).
* `CInformaEleitorPodeVotar` / `CInformaAnoDesabilitadoDemo` / `CInformaBioDesabilitadaDemo` (10496 /
  10504 / 10508) são thunks de uma linha para o corpo mesclado 3883 (u33). Em CONFIRMA, ou pedem os
  fones de ouvido (`CHabilitaAudioEleitor`) ou postam a mensagem 1 (`MSG_INICIA_ELEITOR`) para a thread do eleitor e mostram
  `CMostraEleitorVotando`.
* `TextoAudioEleitor` (10539, slot 4057 da tabela) é a linha 3 da tela `CMostraEleitorVotando`: retorna
  `"ÁUDIO ATIVADO"` quando a thread do eleitor existe e seu modo de áudio não é 2, ou quando o mesário ligou
  o áudio manualmente (`GetAudioHabilitadoManualmente`, func 1687). Caso contrário, retorna `" "`.
* `FinalizaLeitorDigital` (6012) é o corpo mesclado de dois `FinishState`s: `CRegistraDigitalOperador` (10453)
  e `comum::CPedeDigitalMesario` (10315). Desliga o LED do leitor (slot 7 de `IFingerScanner`, 0) e
  encerra a captura (slot 6). Os dois registros de `std::source_location` das buscas são os parâmetros.

### 3.4 Erros da justificativa: `CAnoInformadoInvalido` / `CEleitorMenor16Anos` (10601 / 10598 → 6014)

No fluxo de justificativa, o mesário digita o ano de nascimento do eleitor (`CPedeAnoNascimento`, 10590, u17). Um
ano impossível leva a "Ano de nascimento inválido."; uma idade abaixo de 16 (contada pelo ano) leva a "Eleitor
não pode votar ou justificar / por não ter idade mínima". As duas telas dizem "CONFIRMA: tentar novamente".
Os dois `ProcessInput`s são o mesmo corpo mesclado 6014, com o registro de srcloc como parâmetro: CONFIRMA → volta para
`CPedeAnoNascimento::GetInst()` (5418).

### 3.5 Menu "Outras opções" (`CEscolheOpcao`, u27)

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

`CHorarioVotacaoTerminou` (GetInst + ctor inlinado, 5430): `CAppState(2)`, `Beep(2)`, os três textos em {1,1},
{1,2}, {1,4} (alinhados à esquerda), um input de controle, `CriaFormInterativo("", true)`. O primeiro texto está armazenado
**sem os acentos** ("Horario de votacao", não "Horário de votação"); "Favor encerrar a urna!" não precisa de nenhum.
Não é o único texto do MT sem acentos: `CVerificaDadoEleitor::ProcessInput` (10443) mostra "CONFIRMA: cancelar
a habilitacao".

`CHabilitaAudioManualmente` (construído dentro de 10740): 28 bytes, `CAppState(2)`, `m_texto =
shared_ptr<string>(new string("áudio habilitado/desabilitado"))` (+20), LED apagado, o texto mostrado centralizado na
linha 2 através de `CDataTextFmt<CTextSource>` com formato `"%s"`. O construtor de `CTextSource`
(`ctextsource.h:37`) lança `CUeGuiError(4977, "Texto nulo")` com ponteiro nulo, o que não pode acontecer aqui. O
formulário não tem input de controle: este estado não lê teclas (seu StartState 10751 faz a troca).

---

## 4. Estados da tela do eleitor do início do dia

Antes do início da votação, o mesário opera a urna pelo **teclado do eleitor**. Nessas telas, a tecla BRANCO
tem o rótulo "Mais informações". O `CInteractiveForm<IScreen, IInputKbd>::Read()` inlinado usa o
teclado do eleitor (`instance<IInputKbd>`, func 455) e o slot 10 do campo. Retorna 3 para BRANCO, 5 para CORRIGE e
9 para CONFIRMA.

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

`CMaisInformacoes::GetInst(retorno)` (1280, u26) armazena um `retorno` não nulo. Os cinco subestados (e
`CMenuVisualizarCandidatos::StartState`) voltam com `GetInst(nullptr)`, então o menu mantém seu estado de retorno
até que CORRIGE o limpe. Todo ponto de entrada no menu (reinício 11834, regerar zerésima 11838,
`CVerificaHorarioZeresima` 11869, `CQuerReimprimirZeresima` 11848, `CConfirmaImpressaoZeresima` 11927) passa
`this`.

Mais dois singletons do fluxo da zerésima estão nesta unidade. `CGeraZeresima::GetInst` (5962) é
`CGeraZeresimaBase` com vtable @1544584, usado por `CConfirmaImpressaoZeresima` e `CImpressaoZeresimaTardia`.
`CReimprimindoResumoZeresima::GetInst` (5942) é um thunk para o corpo mesclado de singleton lazy 764.
`TextoFalhaTesteTeclado` (12903) é o texto da tela de falha do teste de teclado: `"Falha: Esperada {},
pressionada {}"` com `api::KeyName` das teclas esperada e pressionada, armazenadas em `CInformacaoEleitor` +9/+10.

---
## 5. Boletim de Urna (BU): os campos ZONA, SECA, IDUE, IDCA do cabeçalho do QR code

Quatro funções desta unidade escrevem campos do **cabeçalho dos QR codes do BU**. Esses são o "BU digital"
impresso abaixo do BU e mostrado na tela do eleitor depois do encerramento. Veja docs/bu/qrcode.md
para o payload completo, a cadeia de hashes e a assinatura. São setters de
`comum::CCabecalhoQRCodeBuilder`, cujo objeto começa com um `CCabecalhoQRCode`: 33 `std::string`, uma por
tag, cada uma com o texto completo `"TAG:valor "` incluindo o espaço final. O builder é preenchido com zeros pela func
5624.

| func | campo (offset) | string de formato | argumento | origem do valor |
|---|---|---|---|---|
| 5618 | `zona` (+108) | `"ZONA:{} "` @440346 | `unsigned` (tipo de argumento de formato 6 da libc++) | zona de `CEstadoGeral` (eg.bin), campo de 16 bits em +24 |
| 5620 | `seca` (+120) | `"SECA:{} "` @440364 | `unsigned` | seção de `CEstadoGeral`, campo de 16 bits em +26 |
| 5621 | `idue` (+144) | `"IDUE:{} "` @440220 | `unsigned` | `carga.numeroInternoUrna` (eg.bin +60) |
| 5623 | `idca` (+156) | `"IDCA:{:.24s} "` @440443 | `const std::string&` (armazenado como tipo de formato 13, string_view) | `carga.codigoCarga` (eg.bin +88, uma `std::string`) |

Passo a passo, em `vota::CGeraBU::StartState` (func 12110, `cgerabu.cpp`, o `comum::CriaQRCode` inlinado de
`cgeradorbu.cpp:124`) e na versão de tela `vota::(anonymous)::GetQRDSInst` (1956):

1. O bit 0 de `CConfiguracaoEleicao` +486 (`imprimirQrCodeNoBU` de `EntidadeParametrizacaoUrna`) precisa estar ligado.
   Caso contrário, nenhum QR code é impresso. No modo de demonstração pré-eleição, o BU imprime "NÃO HÁ QR CODE"
   no lugar (qrcode.md §7).
2. `comparecimento = CRdvVota::GetComparecimento()` (máximo entre as eleições, `shared_f1269`).
3. `CCabecalhoQRCode` construído (5624, 396 bytes zerados).
4. `SetZona(zona)`, `SetSecao(secao)`, `SetIdUrna(numeroInternoUrna)`, `SetCodigoCarga(codigoCarga)` (esta
   unidade), depois `SetHistoricoCargas(...)` (5622: `"HIQT:{} "` e um `"HICA:{}:{:.24s} "` por
   correspondência de `gap.bin`).
5. `CGeradorBUQRCodeVota(cabecalho, comparecimento, dhEmissao)` (5603) preenche ORIG, AGRE, LOCA, APTO…
   (11242), e `GeraQRCodes(1100)` (5604; 2500 para a versão de tela) acrescenta ORLC, PROC, DTPL, PLEI, TURN,
   FASE, UNFE e VERS.
6. **Verificação.** `CCabecalhoQRCodeBuilder::preBuild` (lambda 2791, `ccabecalhoqrcodebuilder.cpp:284`) lança
   `CRelatoriosError(9050, "Campo (Zona) não informado.")` (ou `Secao`, `IdUrna`, `CodigoCarga`, …) se um
   campo obrigatório ainda estiver vazio.
7. O cabeçalho é concatenado nesta ordem: `ORIG ORLC PROC DTPL PLEI TURN FASE UNFE MUNI ZONA SECA [AGRE]
   IDUE IDCA HIQT HICA… VERS`. O payload é dividido em fatias, encadeadas por hash, e a última fatia é assinada
   (`ASSI:`).

Formatos. Os números são escritos em decimal **sem preenchimento com zeros**: `ZONA:1 SECA:51`, como nos QR codes
oficiais de BU de 2024. O código de carga passa por `{:.24s}`: um código com mais de 24 caracteres é **truncado
silenciosamente**. O código tem 24 dígitos, então isso só importa para dados malformados (veja a §11). O único exemplo que
o código real produziu neste projeto é o BU com as fixtures do simulador de
docs/10-boletim-de-urna.md (`ZONA:1 SECA:1 IDUE:87654321
IDCA:123456789012345678901234`).

Outras peças desta unidade relacionadas ao BU:

* **Vias.** `CLogVota::LogaAviso55` (6115) é o corpo mesclado dos avisos de 55 caracteres (severidade
  2). Um deles é `"Quantidade de vias adicionais excede o máximo permitido"` (thunk 4556), registrado por
  `CLimiteCopiasBUAtingido::StartState` e `CEmitirMaisBU::ProcessInput` quando o mesário pede mais
  vias extras do BU do que a parametrização permite.
* `IForm<IScreen>::Remove` (5987) é chamada por `CEmitirMaisBU::ProcessInput` (12081) para tirar a tela "emitir mais
  vias" da tela do eleitor antes de imprimir as vias extras (corpo em `iform.h`, u17).
* **Zerésima.** `CConfirmaRegerarZeresima` (11838) regera o relatório de zero votos quando a zerésima desta
  eleição foi gerada por outra urna. `CGeraZeresima::GetInst` (5962) e
  `CReimprimindoResumoZeresima::GetInst` (5942) são os singletons dos estados de impressão (§4).

No build web nada disso roda: a página nunca chega ao encerramento.

---

## 6. Dados lidos e escritos

**Registros de log** (`/dsk/fi/dinamico/log/logd.dat`, `"<aplicativo>|<severidade>|<texto>"`, Latin-1), escritos pelo
código desta unidade:

| texto | severidade | onde |
|---|---|---|
| Áudio desativado pelo fim da votação | 1 | 10435 (via 4529) |
| Inspeção da urna terminada | 1 | 10527 |
| Todas as pessoas presentes já votaram? SIM / NÃO | 1 | 10713 |
| Mesário confirmou o reinício da votação | 1 | 11834 |
| Mesário selecionou outras opções | 1 | 11834 |
| Operador selecionou: {opção} / Mesário {nome} habilitou o eleitor | 1 | 6113 (thunks 2502, 3258) |
| Habilitação cancelada durante reconhecimento biométrico | 2 | 6115 (thunk 2495) |
| Quantidade de vias adicionais excede o máximo permitido | 2 | 6115 (thunk 4556) |

**Mensagens entre threads** (`SMessage{id, &fila}`, prioridade 1): operador → eleitor 10 (desligar o áudio), 13
(inspeção concluída), 1 (eleitor liberado, corpo 3883); eleitor → operador 11 (inspeção confirmada, recebida por
10530).

**Arquivos.**

* `*-imp.dat` (ModuloImpedidos, BER). `CEleitores::GetEleitoresImpedidos` (5767) lê todos os arquivos de impedidos
  da seção através de `CFileASN::ReadFromFile<EntidadeImpedidos>` e `CConversorImpedido`, e
  concatena os registros `md::CImpedido` de 32 bytes. Isso roda na inicialização (observado).
* `*-ce.dat` → `*-ce.pid`. `CStringUtils::TrocaExtensao` (5461) monta o nome do cabeçalho de versão de pacote
  de cada arquivo de eleição (usado por `LePleito`, 5778).
* Percursos de diretórios. `CDirReader::Open/IsLink` (5470/5469) servem ao hash da mídia (`CMontadorHash`, hash.dat) e
  à exclusão recursiva de `CSystem`. Links simbólicos são detectados com `lstat`.
* Rótulos da parametrização (`*-pu.dat`). `CTradutorFrase::s_labels` recebe os rótulos Município, Zona, Seção
  e Partido (5795/5794/5793). Os textos os usam através de placeholders como `<SCSN>`.
* Horários. `CTime::IsValid` (5449) valida todo `hhmm[ss]` lido dos dados da eleição.

---

## 7. Código de suporte `api`

| func | reconstrução | observações |
|---|---|---|
| 5449 | `CTime::IsValid(hora)` | 4 ou 6 dígitos (máscara 0x03FF… = '0'..'9'), hh<24, mm<60, ss<60 |
| 5469 / 5470 | `CDirReader::IsLink()` / `Open(dir)` | `lstat` de `dir + "/" + d_name`, `S_IFLNK` / `opendir` + substitui o handle antigo |
| 5461 | `CStringUtils::TrocaExtensao(caminho, ext)` | mantém o ponto, retorna a entrada quando o nome não tem ponto |
| 5409 | `CFormBuilderMT::AddTextoCompartilhado(pos, CTextSource)` | campo de texto do MT sobre uma `std::string` compartilhada (`CDataText<CTextSource>`, alinhado à esquerda); objetos criados com `new` + `shared_ptr`, não `make_shared` |
| 5530 | `CInteractiveFormBuilder::AddControlInput(builder, teclas)` | `CInputFieldControl<IScreen>` invisível (máscara de teclas: bit 0 BRANCO, 1 CORRIGE, 2 CONFIRMA) |
| 10870 | `DataHoraAtual(formato)` (slot 3117) | o relógio do cabeçalho de status da tela do eleitor, "A DD/MM/YYYY hh:mm:ss"; os placeholders de hora são expandidos antes dos de data |
| 5540 / 5541 / 5987 | `IForm<IScreen>::DoShowOnTop / DoShow / Remove` | a pilha estática de formulários da tela do eleitor @1832632 (u17) |
| 5528 | `CLockGuard::~CLockGuard()` | `ISyncCtl::Unlock` (slot 3) sob `invoke_vi`; terminate em caso de exceção (noexcept) |
| 7708 | `CPriorityMessageQueue<SMessage>::Envia(msg)` | `Add(msg, 1)` |
| 6023 | `CDefaultGenericFactory<I,C>::Create` mesclado | `new` + construtor por slot da tabela, free + rethrow em caso de exceção |
| 6021/6022, 6062/6063, 6026 | destrutores mesclados | `CTextRectField`/`CTextFieldDoubleLine`, `CMaskedTextField<…>`, `CFormPart`/`comum::CParteEleitores` (vtable como parâmetro) |
| 9372 … 9986 | wrappers `CPolySingletonList::push<I>(I*, info)` | os 8 registros de `main` (factories de semaphore, mutex, rwmutex e thread, preparador de impressão digital, factory de cifradores, RNG, IExecucaoVota) |
| 10424 / 10464 / 10507, 11548 | thunks de slow path de `vector<SPolySingleton>::emplace_back`, `__split_buffer` | para IExecucaoVota, ISincronismoVotoEleitor, IPoliticaExecucaoEleitor |
| 11265 | `DefaultPolySingletonsInfo()` | o registro @1832448 |
| stubs de atexit | 8177, 8187, 8641, 8727, 10866, 10867, 10879, 10901, 11003, 11162, 11655, 11656, 11662, 12108, 12109 | destrutores de estáticos: pilhas/mutexes/observables de IForm, CSynchronizer, escapes de CIniStrings, regex e texto padrão de CInputMenuField, descrição de CApplication, CInfoMTLCD |

Outro código que não é `api`: `comum::CCandidaturasDSNome::operator()` (5807) retorna o nome do candidato
corrente, ou do suplente/vice n: `GetCandidaturaAtual("CCandidaturasDSNome")`, depois `GetSuplente(n)` ou o
titular (+8), depois o nome (+12). `simulador::CWasmLogBus::~CWasmLogBus` (5154) é o destrutor do
barramento de log em memória do simulador (u29).

---

## 8. Código de biblioteca arquivado nesta unidade

**RHVoice 1.14.0** (o TTS do voto acessível). O principal chamador dessas funções é
`CRHVoiceTextToSpeech::Sintetiza` (10841, 75 KB com todo o pipeline inlinado), por isso as ferramentas as colocaram em
`app:api`. Elas foram conferidas com os fontes upstream da tag 1.14.0 (events.hpp, speech_processor.cpp,
equalizer.cpp, pitch.cpp, fst.hpp, language.cpp, userdict.cpp, voice_profile.hpp, utf.hpp):
construtores de `word_event`/`sentence_event` (5212/5213), `speech_processor::finish` (5255),
`equalizer::read_coefs` (5254), `pitch::editor::reset` (5277), `vector<pitch::target_t>::push_back` (5280),
`fst::append_input_symbol(const item&)` (5298), `language::decode_as_character` (5302, com
`decode_as_unknown_character` inlinado), `language::decode_as_digit_string` (5303), `userdict::dict::
simple_search` (5263), `voice_profile::voice_for_text` (5340), `std::distance` sobre `utf::text_iterator` (5432).
Índice com detalhes: `src/uenux2/src/api/audio/crhvoicetexttospeech.u34.cpp`.

**libc++ / libc.** `do_strerror_r` (4656), `unique_ptr<char[]>::reset` (4680: o buffer de `getcwd` do
`__current_path` inlinado em `__do_absolute`; no musl, a libc++ toma o ramo `pathconf` + `new char[size + 1]`,
não o do deleter `free` da glibc/Apple),
`filesystem::status(p, ec) noexcept` (4683), `ErrorHandler<bool>::report(errc)` (4684),
`_PathCVT<char>::__append_source` (7747), `filesystem::detail::vformat_string` (8095), instâncias de `std::map` / `std::set`
(5379, 5793, 5794, 5795, 6733), `__split_buffer` (11548), um `string::operator=(const
char*)` out-of-line mantido na tabela (12979) e um thunk de `strerror` mantido na tabela (12293).

---

## 9. O que é específico do build web

* **Estados mortos.** Nenhum `ProcessInput` desta unidade pode rodar no navegador. A thread do operador nunca
  avança: `main` registra `CExecucaoVotaCooperativa`, que só alimenta a thread do eleitor. A thread do eleitor
  começa em `CAguardaMensagem`, depois dos estados do início do dia. Assim, a inspeção periódica da cabina, a chave de
  áudio do MT, a liberação de um eleitor por impressão digital, o fluxo de justificativa, a pergunta do encerramento e o
  menu "Mais informações" não existem no simulador. O próprio `votaInit` posta as mensagens 1 e 9 (unidade u30).
* **Travaria ou abortaria se alcançado.** `CDesabilitaAudioEleitor::ProcessInput` fica em laço em `usleep`, que neste
  build é uma espera ativa sobre `performance.now()` (func 6265), sem outra thread para mudar a flag.
  `CAguardaEleitoresVotarem::StartState` (alcançado a partir de 10713) chama `emscripten_sleep`, que o glue transforma
  em `abort()` (sem Asyncify).
* **Substitutos do simulador visíveis aqui.** `CWasmSystemDateTime` alimenta o relógio do cabeçalho de status (10870). Os
  wrappers de registro (9372…) instalam as factories `CWasmThread` / `CPosixMutex` / `CPosixSemaphore` e
  `CFingerPrepareSimulador`. O `CWasmLogBus` (5154) é um objeto exclusivo do simulador.

---

## 10. Observações sobre wasm / Emscripten

* **Corpos mesclados (merge-similar-functions do wasm-opt).** Funções que diferem apenas em uma constante viram um
  corpo mais thunks de uma linha. Nesta unidade, a constante é um registro de srcloc (6014, 3883, 6012), uma string
  de formato (6113), um literal de string cortado em pedaços de 8 bytes (6115), uma vtable (6021/6022, 6062/6063, 6026), um
  slot de tabela e um tamanho (6023), ou um slot de função push (1562 ← 9372…9986). Uma função C++ corresponde, portanto, a
  um par "thunk + corpo".
* **Singletons inlinados.** O `GetInst` e o construtor de `CAguardaEleitoresVotarem`,
  `CHabilitaAudioManualmente`, `CRegerarZeresima`, `CConfirmaInspecionada`, dos quatro `CImpressao*` e de outros
  só existem dentro do `ProcessInput` do estado que vai para eles. Os endereços do `unique_ptr` estático e do mutex
  são a única identidade que resta.
* **`CInteractiveForm::Read` inlinado.** O registro de srcloc de `cinteractiveform.h:57` dentro de cada
  `ProcessInput` fez as ferramentas arquivarem todos esses estados de vota em `api`. A versão do MT chama o slot 8 do campo
  com `IInputMT` (383); a versão da tela do eleitor chama o slot 10 com `IInputKbd` (455).
* **noexcept com exceções via JS.** Uma função `noexcept` que chama algo que pode lançar exceção mantém a chamada
  dentro de `invoke_*` e tem um landing pad que chama `__clang_call_terminate` (4683, 5528).
* **Funções mantidas só por causa da tabela.** 12293 (`strerror`) e 12979 (`string::operator=`) existem porque
  algum código as chama através de `invoke_*`, que precisa de um slot na tabela.
* **Propagação de constantes de estáticos.** 5794 é `map::operator=` especializado para o único map estático que ele atribui
  (@1839056 fixo no código). 5767 não tem parâmetro `this` (membro estático, ou `this` removido por não ser usado).
* **`usleep` com espera ativa.** O `nanosleep` do musl é compilado como um giro sobre `emscripten_get_now` (6265).

---
## 11. Código estranho ou arriscado

1. **Espera ativa sem limite por outra thread (10435, `CDesabilitaAudioEleitor::ProcessInput`).** Depois de postar
   a mensagem 10, a thread do operador consulta `CInformacaoEleitor::m_modoAudio` a cada 300 µs, sem timeout. Na
   urna, se a thread do eleitor tiver parado (uma exceção faz `CThreadVota` parar as threads do
   aplicativo, veja u27 §4.1) ou não consumir sua fila, o terminal do mesário congela em "Retire o fone de
   ouvido". O campo é um `int` simples de 4 bytes (`i32.load offset=4`, inicializado com 2 por `GetInst` 509) que
   outra thread escreve. `GetInst` trava o mutex do singleton só em torno do ponteiro, não do campo,
   então isto é uma condição de corrida (data race) nos termos do C++, embora a chamada releia o campo a cada vez. Na urna, `usleep`
   cede a CPU, então o laço é um poll com sleep. Neste build, `usleep` gira sobre `performance.now()`, então
   alcançá-lo congelaria a aba do navegador. O código é morto no simulador.
2. **Leitura entre threads sem sincronização em uma fonte de texto (10539, `TextoAudioEleitor`).** O teste do
   `unique_ptr` estático de `CThreadEleitor` é feito sob seu mutex de singleton @1833188 (o `lock` some
   na compilação deste build, e o resíduo de `mutex::unlock` logo depois da leitura mostra que havia um `lock_guard`
   ali). O modo de áudio do eleitor (`CInformacaoEleitor` +4) é então lido depois que `CInformacaoEleitor::GetInst`
   liberou seu próprio mutex, como em 10435. Na urna, a thread do operador (que desenha o MT) e a thread do eleitor
   rodam ao mesmo tempo, então essa leitura concorre com a escrita da thread do eleitor. O efeito só pode ser
   cosmético: "ÁUDIO ATIVADO" mostrado com uma atualização de atraso.
3. **Truncamento silencioso do código de carga no cabeçalho do QR do BU (5623).** `"IDCA:{:.24s} "` corta qualquer
   `codigoCarga` mais longo para 24 caracteres sem erro, enquanto `preBuild` só verifica que o campo não está
   vazio. Com um `eg.bin` malformado, o QR code assinado traria um IDCA diferente do armazenado.
   Cargas válidas têm exatamente 24 dígitos, então isto é latente.
4. **`TrocaExtensao` perde a barra da raiz (5461).** Para um caminho diretamente sob `/` (`"/x.dat"`), a parte do
   diretório é vazia e o resultado é `"x.pid"`, um caminho relativo. Todo caminho que o VOTA passa está sob
   `/dsk/fi/estatico/`, então isto é latente.
5. **O lock guard noexcept termina o programa em falha de unlock (5528).** Se `ISyncCtl::Unlock` lançar exceção (por exemplo,
   o wrapper POSIX reporta um erro de pthread na urna), `~CLockGuard` chama `std::terminate` em vez de
   reportar. Essa é a regra normal do C++ para destrutores; está listado porque é um caminho de abort de todo
   envio de mensagem.
6. **Abortaria no navegador se alcançado (10713 → `CAguardaEleitoresVotarem::StartState`).** O ramo "não"
   de "Todas as pessoas presentes já votaram?" leva a um `api::CSystem::Sleep` de 3 s, compilado como
   `if (byte@1584624 == 1) emscripten_sleep(3000)`. O byte vale 1 e nada o escreve, e o
   `_emscripten_sleep` do glue chama `abort()` sem Asyncify (já reportado pela u27). Código morto no simulador.
7. **Inspeção, justificativa, liberação por impressão digital e áudio manual não são simulados.** Esses fluxos
   do operador fazem parte do procedimento real do dia da votação (por exemplo, as inspeções aleatórias da cabina a cada 60–90 min), mas
   a thread do operador nunca roda no build web. Quem treina com o simulador público nunca os vê.
   Isto é uma nota de fidelidade, não um bug.
8. **Texto do MT sem acentos (5430).** "Horario de votacao terminou!" está sem os acentos ("Favor encerrar a
   urna!" não precisa de nenhum). Outros textos do MT também estão sem acentos (por exemplo, "CONFIRMA: cancelar a habilitacao" de
   `CVerificaDadoEleitor`, 10443), enquanto a maioria dos textos do MT tem acentos. Isto é cosmético.

---

## 12. Tabela de mapeamento (todas as 103 funções)

"Observada" = amostrada enquanto os votos gravados eram executados (`analysis/runtime/vote_*.functions.tsv`). As linhas "biblioteca"
são código de terceiros (libc++, musl, RHVoice 1.14.0) que as ferramentas arquivaram aqui. Elas são listadas no
fragmento indicado na linha, não reescritas.

| func | tamanho | observada | nome nas ferramentas | símbolo reconstruído | arquivo-fonte | confiança |
|---|---|---|---|---|---|---|
| 4656 | 151 |  | `api_f4656` | `std::(anonymous namespace)::do_strerror_r` | biblioteca (libc++), listado em src/uenux2/src/api/u34-foreign-fragments.cpp | alta |
| 4680 | 28 |  | `api_f4680` | `std::unique_ptr<char[]>::reset` (buffer de `__current_path`, `delete[]` = free) | biblioteca (libc++) | média |
| 4683 | 57 | ✓ | `api_f4683` | `std::filesystem::status(const path&, error_code&) noexcept` | biblioteca (libc++) | média |
| 4684 | 52 |  | `api_f4684` | `std::filesystem::detail::ErrorHandler<bool>::report(const std::errc&)` | biblioteca (libc++) | média |
| 5154 | 387 |  | `api_f5154` | `simulador::CWasmLogBus::~CWasmLogBus` | src/uenux2/mock/app/simulador/wasm/cwasmlogbus.u34.cpp | média |
| 5212 | 327 |  | `api_f5212` | `RHVoice::word_event::word_event` | biblioteca (RHVoice 1.14.0), índice em src/uenux2/src/api/audio/crhvoicetexttospeech.u34.cpp | alta |
| 5213 | 646 |  | `api_f5213` | `RHVoice::sentence_event::sentence_event` | biblioteca (RHVoice 1.14.0) | alta |
| 5254 | 1114 |  | `api_f5254` | `RHVoice::equalizer::read_coefs` | biblioteca (RHVoice 1.14.0) | alta |
| 5255 | 372 |  | `api_f5255` | `RHVoice::speech_processor::finish` | biblioteca (RHVoice 1.14.0) | alta |
| 5263 | 85 |  | `api_f5263` | `RHVoice::userdict::dict::simple_search` | biblioteca (RHVoice 1.14.0) | alta |
| 5277 | 370 |  | `api_f5277` | `RHVoice::pitch::editor::reset` | biblioteca (RHVoice 1.14.0) | alta |
| 5280 | 230 |  | `api_f5280` | `std::vector<RHVoice::pitch::target_t>::push_back` | biblioteca (instanciação da libc++) | média |
| 5298 | 270 |  | `api_f5298` | `RHVoice::fst::append_input_symbol(const item&, input_symbols&)` | biblioteca (RHVoice 1.14.0) | alta |
| 5302 | 1687 |  | `api_f5302` | `RHVoice::language::decode_as_character` | biblioteca (RHVoice 1.14.0) | alta |
| 5303 | 1384 |  | `api_f5303` | `RHVoice::language::decode_as_digit_string` | biblioteca (RHVoice 1.14.0) | alta |
| 5340 | 415 |  | `api_f5340` | `RHVoice::voice_profile::voice_for_text` | biblioteca (RHVoice 1.14.0) | alta |
| 5379 | 309 |  | `api_f5379` | `std::map<std::string, V>::insert(value_type&&) (__emplace_unique_key_args)` | biblioteca (instanciação da libc++), anotado em src/uenux2/src/app/comum/dados/celeitores.u34.cpp | média |
| 5388 | 111 |  | `api_f5388` | `comum::CEncerraRegistroMesarios::GetInst` | src/uenux2/src/app/comum/comparecimentomesario/estados/cencerraregistromesarios.cpp (escrito por outra unidade) | alta |
| 5409 | 356 |  | `api_f5409` | `api::CFormBuilderMT::AddTextoCompartilhado (Add<CTextFieldMT> with CDataText<CTextSource>)` | src/uenux2/src/api/gui/cformbuildermt.u34.cpp | média |
| 5430 | 948 |  | `api_f5430` | `vota::CHorarioVotacaoTerminou::GetInst` | src/uenux2/src/app/vota/operador/outrasopcoes/chorariovotacaoterminou.u34.cpp | alta |
| 5432 | 1278 |  | `api_f5432` | `std::distance<RHVoice::utf::text_iterator<std::string::const_iterator>>` | biblioteca (instanciação RHVoice/libc++) | média |
| 5449 | 495 | ✓ | `api_f5449` | `api::CTime::IsValid` | src/uenux2/src/api/util/ctime.u34.cpp | média |
| 5461 | 1341 | ✓ | `api_f5461` | `api::CStringUtils::TrocaExtensao` | src/uenux2/src/api/util/cstringutils.u34.cpp | baixa |
| 5469 | 352 |  | `api_f5469` | `api::CDirReader::IsLink` | src/uenux2/src/api/util/cdirreader.u34.cpp | alta |
| 5470 | 224 |  | `api_f5470` | `api::CDirReader::Open` | src/uenux2/src/api/util/cdirreader.u34.cpp | alta |
| 5528 | 64 | ✓ | `api_f5528` | `api::CLockGuard::~CLockGuard` | src/uenux2/src/api/ipc/clockguard.u34.h | média |
| 5530 | 121 |  | `api_f5530` | `api::CInteractiveFormBuilder::AddControlInput(CFormBuilder&, unsigned)` | src/uenux2/src/api/gui/cinteractiveformbuilder.u34.cpp | média |
| 5540 | 289 |  | `api_f5540` | `api::IForm<api::IScreen>::DoShowOnTop` | src/uenux2/src/api/gui/iform.h (escrito pela unidade u17) | média |
| 5541 | 212 | ✓ | `api_f5541` | `api::IForm<api::IScreen>::DoShow` | src/uenux2/src/api/gui/iform.h (escrito pela unidade u17) | média |
| 5618 | 414 |  | `api_f5618` | `comum::CCabecalhoQRCodeBuilder::SetZona` | src/uenux2/src/app/comum/relatorios/ccabecalhoqrcodebuilder.u34.cpp | média |
| 5620 | 417 |  | `api_f5620` | `comum::CCabecalhoQRCodeBuilder::SetSecao` | src/uenux2/src/app/comum/relatorios/ccabecalhoqrcodebuilder.u34.cpp | média |
| 5621 | 419 |  | `api_f5621` | `comum::CCabecalhoQRCodeBuilder::SetIdUrna` | src/uenux2/src/app/comum/relatorios/ccabecalhoqrcodebuilder.u34.cpp | média |
| 5623 | 463 |  | `api_f5623` | `comum::CCabecalhoQRCodeBuilder::SetCodigoCarga` | src/uenux2/src/app/comum/relatorios/ccabecalhoqrcodebuilder.u34.cpp | média |
| 5767 | 2553 | ✓ | `api_f5767` | `comum::CEleitores::GetEleitoresImpedidos` | src/uenux2/src/app/comum/dados/celeitores.u34.cpp | alta |
| 5793 | 256 | ✓ | `api_f5793` | `std::pair<const char, ecourna::app::dados::CLabelParametrizado>::pair(const pair&)` | biblioteca (instanciação da libc++), anotado em src/uenux2/src/app/comum/dados/md/parametrizacaourna/ctradutorfrase.u34.cpp | média |
| 5794 | 1483 |  | `api_f5794` | `std::map<char, CLabelParametrizado>::operator=(const map&) on CTradutorFrase::s_labels` | biblioteca (instanciação da libc++), anotado em ctradutorfrase.u34.cpp | média |
| 5795 | 345 | ✓ | `api_f5795` | `std::map<char, CLabelParametrizado>::map(std::initializer_list)` | biblioteca (instanciação da libc++), anotado em ctradutorfrase.u34.cpp | média |
| 5807 | 204 |  | `api_f5807` | `comum::CCandidaturasDSNome::operator()` | src/uenux2/src/app/comum/dados/ccandidaturas.u34.cpp | média |
| 5942 | 22 |  | `api_f5942` | `vota::CReimprimindoResumoZeresima::GetInst` | src/uenux2/src/app/vota/eleitor/iniciovotacao/cgerazeresima.u34.cpp | alta |
| 5962 | 93 |  | `api_f5962` | `vota::CGeraZeresima::GetInst` | src/uenux2/src/app/vota/eleitor/iniciovotacao/cgerazeresima.u34.cpp | alta |
| 5987 | 411 |  | `api_f5987` | `api::IForm<api::IScreen>::Remove` | src/uenux2/src/api/gui/iform.h (escrito pela unidade u17) | média |
| 6012 | 100 |  | `api_f6012` | `vota::(anonymous)::FinalizaLeitorDigital (merged FinishState body)` | src/uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.u34.cpp | média |
| 6014 | 127 |  | `api_f6014` | `vota::(anonymous)::VoltaParaAnoNascimentoSeConfirma (merged ProcessInput body)` | src/uenux2/src/app/vota/operador/justificativa/canoinformadoinvalido.u34.cpp | média |
| 6021 | 101 |  | `api_f6021` | `api::CTextRectField / CTextFieldDoubleLine deleting destructor (merged body)` | src/uenux2/src/api/u34-foreign-fragments.cpp | alta |
| 6022 | 98 |  | `api_f6022` | `api::CTextRectField / CTextFieldDoubleLine complete destructor (merged body)` | src/uenux2/src/api/u34-foreign-fragments.cpp | alta |
| 6023 | 64 | ✓ | `api_f6023` | `api::CDefaultGenericFactory<I,C>::Create (merged body)` | src/uenux2/src/api/u34-foreign-fragments.cpp | alta |
| 6026 | 66 |  | `api_f6026` | `api::CFormPart / comum::CParteEleitores deleting destructor (merged body)` | src/uenux2/src/api/u34-foreign-fragments.cpp | alta |
| 6062 | 101 |  | `api_f6062` | `api::CMaskedTextField<MASK> deleting destructor (merged body)` | src/uenux2/src/api/u34-foreign-fragments.cpp | alta |
| 6063 | 98 |  | `api_f6063` | `api::CMaskedTextField<MASK> complete destructor (merged body)` | src/uenux2/src/api/u34-foreign-fragments.cpp | alta |
| 6113 | 446 |  | `api_f6113` | `vota::CLogVota::LogaFormatado (merged body)` | src/uenux2/src/app/vota/log/clogvota.u34.cpp | média |
| 6115 | 160 |  | `api_f6115` | `vota::CLogVota::LogaAviso55 (merged body)` | src/uenux2/src/app/vota/log/clogvota.u34.cpp | média |
| 6733 | 361 |  | `api_f6733` | `std::set<int>::set(first, last) (range insert with end hint)` | biblioteca (instanciação da libc++), anotado em celeitores.u34.cpp | média |
| 7708 | 11 | ✓ | `api_f7708` | `api::CPriorityMessageQueue<api::SMessage>::Envia(const SMessage&)` | src/uenux2/src/api/u34-foreign-fragments.cpp | média |
| 7747 | 20 |  | `api_f7747` | `std::filesystem::_PathCVT<char>::__append_source<const char*>` | biblioteca (libc++) | média |
| 8095 | 221 |  | `api_f8095` | `std::filesystem::detail::vformat_string` | biblioteca (libc++) | alta |
| 8177 | 39 |  | `api_f8177` | `atexit: ~std::vector<IForm<IScreenMT>*> IForm<IScreenMT>::ms_pilha` | src/uenux2/src/api/u34-foreign-fragments.cpp | alta |
| 8187 | 10 |  | `api_f8187` | `atexit: ~std::mutex IForm<IScreenMT>::ms_mutex` | src/uenux2/src/api/u34-foreign-fragments.cpp | alta |
| 8641 | 10 |  | `api_f8641` | `atexit: ~std::mutex IForm<IScreen>::ms_mutex` | src/uenux2/src/api/u34-foreign-fragments.cpp | alta |
| 8727 | 11 |  | `api_f8727` | `atexit: ~IObservable IForm<IScreenMT>::ms_observavel` | src/uenux2/src/api/u34-foreign-fragments.cpp | alta |
| 9372 | 12 | ✓ | `api_f9372` | `api::CPolySingletonList::push<vota::IExecucaoVota>(I*, info) wrapper` | src/uenux2/src/api/u34-foreign-fragments.cpp | alta |
| 9574 | 12 | ✓ | `api_f9574` | `api::CPolySingletonList::push<IGenericFactory<ISemaphore>>(I*, info) wrapper` | src/uenux2/src/api/u34-foreign-fragments.cpp | alta |
| 9650 | 12 | ✓ | `api_f9650` | `api::CPolySingletonList::push<IGenericFactory<IRWSyncCtl>>(I*, info) wrapper` | src/uenux2/src/api/u34-foreign-fragments.cpp | alta |
| 9685 | 12 | ✓ | `api_f9685` | `api::CPolySingletonList::push<IGenericFactory<ISyncCtl>>(I*, info) wrapper` | src/uenux2/src/api/u34-foreign-fragments.cpp | alta |
| 9731 | 12 | ✓ | `api_f9731` | `api::CPolySingletonList::push<IGenericFactory<IThreadImpl>>(I*, info) wrapper` | src/uenux2/src/api/u34-foreign-fragments.cpp | alta |
| 9831 | 12 | ✓ | `api_f9831` | `api::CPolySingletonList::push<api::IFingerPrepare>(I*, info) wrapper` | src/uenux2/src/api/u34-foreign-fragments.cpp | alta |
| 9897 | 12 | ✓ | `api_f9897` | `api::CPolySingletonList::push<ecourna::api::security::ISymmetricCipherFactory>(I*, info) wrapper` | src/uenux2/src/api/u34-foreign-fragments.cpp | alta |
| 9986 | 12 | ✓ | `api_f9986` | `api::CPolySingletonList::push<ecourna::api::security::IRng>(I*, info) wrapper` | src/uenux2/src/api/u34-foreign-fragments.cpp | alta |
| 10424 | 16 |  | `api_f10424` | `std::vector<api::SPolySingleton>::emplace_back slow-path thunk (push<vota::IExecucaoVota>)` | src/uenux2/src/api/u34-foreign-fragments.cpp | alta |
| 10435 | 200 |  | `vota::CDesabilitaAudioEleitor::vf7` | `vota::CDesabilitaAudioEleitor::ProcessInput` | src/uenux2/src/app/vota/operador/aguardaeleitor/cdesabilitaaudioeleitor.u34.cpp | alta |
| 10458 | 130 |  | `vota::CDigitalNaoCapturada::vf7` | `vota::CDigitalNaoCapturada::ProcessInput` | src/uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaocapturada.u34.cpp | alta |
| 10464 | 16 |  | `api_f10464` | `std::vector<api::SPolySingleton>::emplace_back slow-path thunk (push<vota::impl::ISincronismoVotoEleitor>)` | src/uenux2/src/api/u34-foreign-fragments.cpp | alta |
| 10496 | 12 |  | `vota::CInformaEleitorPodeVotar::vf7` | `vota::CInformaEleitorPodeVotar::ProcessInput` | src/uenux2/src/app/vota/operador/confirmaidentidade/cinformaeleitorpodevotar.u33.cpp (escrito pela unidade u33) | alta |
| 10504 | 12 |  | `vota::CInformaAnoDesabilitadoDemo::vf7` | `vota::CInformaAnoDesabilitadoDemo::ProcessInput` | src/uenux2/src/app/vota/operador/confirmaidentidade/cinformaeleitorpodevotar.u33.cpp (escrito pela unidade u33) | alta |
| 10507 | 16 |  | `api_f10507` | `std::vector<api::SPolySingleton>::emplace_back slow-path thunk (push<vota::impl::IPoliticaExecucaoEleitor>)` | src/uenux2/src/api/u34-foreign-fragments.cpp | alta |
| 10508 | 12 |  | `vota::CInformaBioDesabilitadaDemo::vf7` | `vota::CInformaBioDesabilitadaDemo::ProcessInput` | src/uenux2/src/app/vota/operador/confirmaidentidade/cinformaeleitorpodevotar.u33.cpp (escrito pela unidade u33) | alta |
| 10527 | 300 |  | `vota::CConfirmaInspecionada::vf7` | `vota::CConfirmaInspecionada::ProcessInput` | src/uenux2/src/app/vota/operador/aguardaeleitor/cconfirmainspecionada.u34.cpp | alta |
| 10539 | 124 |  | `api_f10539` | `vota::(anonymous)::TextoAudioEleitor` | src/uenux2/src/app/vota/operador/aguardaeleitor/cmostraeleitorvotando.u34.cpp | média |
| 10598 | 12 |  | `vota::CEleitorMenor16Anos::vf7` | `vota::CEleitorMenor16Anos::ProcessInput` | src/uenux2/src/app/vota/operador/justificativa/canoinformadoinvalido.u34.cpp | alta |
| 10601 | 12 |  | `vota::CAnoInformadoInvalido::vf7` | `vota::CAnoInformadoInvalido::ProcessInput` | src/uenux2/src/app/vota/operador/justificativa/canoinformadoinvalido.u34.cpp | alta |
| 10698 | 130 |  | `vota::CContadoresBiometria::vf7` | `vota::CContadoresBiometria::ProcessInput` | src/uenux2/src/app/vota/operador/outrasopcoes/ccontadoresbiometria.u34.cpp | alta |
| 10713 | 1389 |  | `vota::CPerguntaFilaEleitorVazia::vf7` | `vota::CPerguntaFilaEleitorVazia::ProcessInput` | src/uenux2/src/app/vota/operador/outrasopcoes/cperguntafilaeleitorvazia.u34.cpp | alta |
| 10740 | 944 |  | `vota::CConfirmaAudio::vf7` | `vota::CConfirmaAudio::ProcessInput` | src/uenux2/src/app/vota/operador/outrasopcoes/cconfirmaaudio.u34.cpp | alta |
| 10866 | 10 |  | `api_f10866` | `atexit: ~std::mutex @1839300 (CSynchronizer instance mutex)` | src/uenux2/src/api/u34-foreign-fragments.cpp | baixa |
| 10867 | 12 |  | `api_f10867` | `atexit: ~std::unique_ptr<api::CSynchronizer> s_instancia` | src/uenux2/src/api/u34-foreign-fragments.cpp | alta |
| 10870 | 98 | ✓ | `api_f10870` | `api::(anonymous)::DataHoraAtual` | src/uenux2/src/api/gui/cformbuilder.u34.cpp | média |
| 10879 | 28 |  | `api_f10879` | `atexit: ~std::vector<std::pair<std::string,char>> CIniStrings ESCAPES` | src/uenux2/src/api/u34-foreign-fragments.cpp | alta |
| 10901 | 17 |  | `api_f10901` | `atexit: ~std::regex CInputMenuField marcadores("%[ST]")` | src/uenux2/src/api/u34-foreign-fragments.cpp | alta |
| 10971 | 2127 | ✓ | `api_f10971` | `api::CabeLarguraProporcional` | src/uenux2/src/api/gui/cstepsprogressbar.cpp (escrito pela unidade u16) | média |
| 11003 | 11 |  | `api_f11003` | `atexit: ~IObservable IForm<IPaper>::ms_observavel` | src/uenux2/src/api/u34-foreign-fragments.cpp | alta |
| 11162 | 36 |  | `api_f11162` | `atexit: ~std::string api::CApplication::ms_descricao` | src/uenux2/src/api/u34-foreign-fragments.cpp | alta |
| 11265 | 39 | ✓ | `api_f11265` | `api::DefaultPolySingletonsInfo` | src/uenux2/src/api/pattern/cpolysingletonlist.h (declarado pela unidade u19) | média |
| 11548 | 93 |  | `api_f11548` | `std::__split_buffer<api::SPolySingleton, allocator&>::__split_buffer` | biblioteca (instanciação da libc++), anotado em src/uenux2/src/api/u34-foreign-fragments.cpp | alta |
| 11655 | 10 |  | `api_f11655` | `atexit: ~std::mutex comum::CInfoMTLCD instance mutex` | src/uenux2/src/api/u34-foreign-fragments.cpp | alta |
| 11656 | 40 |  | `api_f11656` | `atexit: ~std::unique_ptr<comum::CInfoMTLCD> s_instancia` | src/uenux2/src/api/u34-foreign-fragments.cpp | alta |
| 11662 | 11 |  | `api_f11662` | `atexit: ~std::string api::CInputMenuField::ms_textoInstrucaoPadrao` | src/uenux2/src/api/u34-foreign-fragments.cpp | alta |
| 11834 | 448 |  | `vota::CReinicioVotacao::vf7` | `vota::CReinicioVotacao::ProcessInput` | src/uenux2/src/app/vota/eleitor/iniciovotacao/creiniciovotacao.u34.cpp | alta |
| 11838 | 242 |  | `vota::CConfirmaRegerarZeresima::vf7` | `vota::CConfirmaRegerarZeresima::ProcessInput` | src/uenux2/src/app/vota/eleitor/iniciovotacao/cconfirmaregerarzeresima.u34.cpp | alta |
| 11896 | 717 |  | `vota::CMaisInformacoes::vf7` | `vota::CMaisInformacoes::ProcessInput` | src/uenux2/src/app/vota/eleitor/iniciovotacao/cmaisinformacoes.u34.cpp | alta |
| 12108 | 39 |  | `api_f12108` | `atexit: ~std::vector<IForm<IPaper>*> IForm<IPaper>::ms_pilha` | src/uenux2/src/api/u34-foreign-fragments.cpp | alta |
| 12109 | 10 |  | `api_f12109` | `atexit: ~std::mutex IForm<IPaper>::ms_mutex` | src/uenux2/src/api/u34-foreign-fragments.cpp | alta |
| 12293 | 7 |  | `api_f12293` | `strerror thunk (table slot 6211)` | biblioteca (thunk da libc), listado em src/uenux2/src/api/u34-foreign-fragments.cpp | baixa |
| 12903 | 542 |  | `api_f12903` | `vota::testeteclado::(anonymous)::TextoFalhaTesteTeclado` | src/uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.u34.cpp | média |
| 12979 | 9 | ✓ | `api_f12979` | `std::string::operator=(const char*) (out-of-line, table slot 168)` | biblioteca (libc++), listado em src/uenux2/src/api/u34-foreign-fragments.cpp | média |

---

## 13. Questões em aberto

* Os nomes exatos do TSE dos helpers mesclados (6014, 6012, 6113, 6115), dos setters do cabeçalho do BU
  (5618/5620/5621/5623: só as strings de formato e a ordem dos campos são certas) e de `TrocaExtensao` /
  `DataHoraAtual` / `TextoAudioEleitor` / `TextoFalhaTesteTeclado` são inferidos.
* O arquivo do form builder do MT (`cformbuildermt.h`?) e o de `CLockGuard` (`clockguard.h`?) são desconhecidos.
* `CGeraBU` verifica `imprimirQrCodeNoBU`, mas a verificação de que `codigoCarga` tem 24 dígitos (se existir)
  acontece em outro lugar, talvez no carregador de `CEstadoGeral`. Ela não foi encontrada nesta unidade.
* O thunk de `strerror` 12293: o ponto de `invoke_ii(6211, …)` não foi localizado (não há `i32.const 6211` no wat).
  Seus vizinhos na tabela sugerem as mensagens de erro de `ecourna::api::io::CFile`.
