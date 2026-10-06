# u06 — `uenux2/src/app/vota/eleitor`: a sessão do eleitor (CEleitorVotando e companhia)

A unidade u06 cobre 83 funções wasm do **lado do eleitor** da aplicação VOTA:
o estado que roda enquanto um eleitor está na urna (`vota::CEleitorVotando`), os estados genéricos de
"conferência" e "confirmação" de um voto, o cargo sem candidato, as
instruções de acessibilidade, a tela de fim de voto, o registro de telas por cargo
(`CTelasCargo`) e os estados de roteamento de inicialização / reinício da thread do eleitor
(`CAjusteInicial`, `CDefineRotaPosReinicio`, `CIniciodeCiclo`, `CEstadoComDesligamentoAutomatico`).
34 das 83 funções rodaram durante os votos registrados (`analysis/runtime/`).

Arquivos-fonte reconstruídos: `src/uenux2/src/app/vota/eleitor/` —
`celeitorvotando.{h,cpp}`, `cconferevotoemcargo.{h,cpp}`, `cconfirmavotoemcargo.{h,cpp}`,
`cconfirmavotosemcandidato.{h,cpp}`, `cinstrucaovotacaoacessibilidade.{h,cpp}`,
`cfimvotoeleitor.cpp`, `cajusteinicial.cpp`, `cdefinerotaposreinicio.cpp`, `ciniciodeciclo.cpp`,
`cestadocomdesligamentoautomatico.cpp`, `comum/ctelascargo.{h,cpp}`,
`comum/cpoliticaexecucaoeleitor.cpp`.

## 1. Glossário

| termo | significado |
|---|---|
| eleitor | o cidadão que vota na urna |
| mesário | membro da mesa receptora; opera o *terminal do mesário* (teclado/visor do operador, "TA") |
| habilitar / habilitação | o mesário identifica o eleitor e libera a urna para ele/ela |
| cargo | cargo em votação (Vereador, Prefeito, Senador…); uma *consulta* é uma pergunta de referendo tratada como um cargo |
| escolha / vaga | uma escolha dentro de um cargo que elege várias vagas (p. ex., 2 Senadores → 2 escolhas) |
| voto nominal / de legenda / branco / nulo | voto em um candidato / apenas em um partido / em branco / nulo |
| conferência | a tela curta exibida logo depois que o número fica completo ("Confira o seu voto.") |
| confirmação | a tela com CONFIRMA / CORRIGE |
| suspender | o mesário (ou o modo de treinamento) encerra a sessão de um eleitor que não terminou |
| RDV | Registro Digital do Voto: a lista embaralhada de todos os votos, fonte do BU |
| BU | Boletim de Urna: relatório de resultado por urna produzido no fim do dia (encerramento) |
| zerésima | relatório impresso antes da votação mostrando que a urna não contém nenhum voto |
| MR | *mídia de resultado*: o pendrive USB que recebe os arquivos de resultado |
| trânsito | votar fora do próprio município (restringe quais cargos podem ser votados) |
| treinamento eleitor | modo de "treinamento do eleitor" de uma urna em fase de treinamento |

## 2. Onde este código roda

A aplicação VOTA tem duas "threads": a **thread do operador** (`CThreadOperador`, o terminal do
mesário) e a **thread do eleitor** (`CThreadEleitor`, a tela do eleitor + teclado). Cada uma é uma máquina
de estados de objetos `comum::CAppState`, conduzida por eventos: teclas, ticks (timers do
`api::CTickManager` da thread) e mensagens (uma `api::CPriorityMessageQueue<api::SMessage>` por thread).
`CThreadEleitor::Processar` (func 4349, unidade u07; antes exibida pelas ferramentas como `ProcessarEntrada`, o
nome de um de seus helpers inlinados) despacha cada evento para o estado atual e então aplica o
protocolo de transição padrão:

```cpp
if (state->NeedChangeState()) { state->FinishState(); state = state->GetNextState(); if (state) state->StartState(); }
```

Em uma urna real, a thread do eleitor começa em `IAjusteInicial::GetInst()` (func 7061 →
`CAjusteInicial`), que deduz novamente o estado correto a partir do `EstadoGeralVota` persistido. **No
build web, `main` em vez disso inicia um `CExecucaoVotaCooperativa` cujo primeiro estado é
`CAguardaMensagem`** (`decompiled/app-wasm-entry/_by_index/10000.dcmp` linha 475: `invoke_i(125 = func 1337)` e depois `CExecucaoVotaCooperativa::CExecucaoVotaCooperativa`), então `CAjusteInicial`,
`CDefineRotaPosReinicio` e a limpeza da MR nunca rodam no simulador (nenhum deles foi observado).

### Protocolo virtual de `comum::CAppState` (nomes de slot recuperados)

| slot | método | padrão (CAppState) | evidência |
|---|---|---|---|
| 0 / 1 | destrutor / destrutor de deleção | | |
| 2 | `StartState()` | puro | srcloc `CFimVotoEleitor::StartState`, `CGeraBU::StartState`… |
| 3 | `bool NeedChangeState()` | `GetNextState() != this` (func 7480) | srcloc `CEleitorVotando::NeedChangeState` |
| 4 | `CAppState* GetNextState()` | retorna `m_proximoEstado` (+4) (func 1661) | |
| 5 | `FinishState()` | no-op | `CPedeDigital::FinishState` |
| 6 | `ProcessMessage(uebyte)` *(nome inferido)* | no-op | despacho das mensagens da fila (func 3843) |
| 7 | `ProcessInput()` | no-op | srcloc `CVotacaoStateAudio::ProcessInput` |
| 8 | `ProcessTick(uebyte)` | no-op | srcloc `CVotacaoStateAudio::ProcessTick(uebyte)` |

`CAppState(uebyte flags)` (func 224): +4 `m_proximoEstado = this`, +8 aceita mensagens (bit 0),
+9 aceita teclas (bit 1), +10 aceita ticks (bit 2). Apenas as flags do estado de *nível superior* importam:
`CEleitorVotando` (flags 7) repassa tudo ao seu subestado por conta própria.

`vota::CVotacaoStateAudio` (unidade u08) acrescenta áudio: slot 9 `ProcessInputAudio`, 10
`StartStateAudio`, 11 `FinishStateAudio` (inferido), 12 `ProcessTickAudio(uebyte)` (nome tirado de um
tipo de lambda), 13 `EmiteEcoComInputField` (srcloc), 14 expansor de template de mensagem (`{cargo-atual}`…),
15 `GetMensagemAudio()` (inferido: o texto falado 1.5 s após entrar no estado e 2 s após cada
tecla). Seu construtor é a func 1785 (nesta unidade): `CAppState(flags)`, `m_audioHabilitado`=false,
dois ticks parados de 2000 ms (+12) e 1500 ms (+13). Os slots 9, 10, 12 e 15 são puros
(`__cxa_pure_virtual`) em `CVotacaoStateAudio`, então todo estado concreto precisa sobrescrever o slot 12:
`CInstrucaoVotacaoAcessibilidade`, `CConfirmaVotoSemCandidato` e `CConfirmaVotoEmCargo` (e, portanto,
os dez estados de confirmação) sobrescrevem `ProcessTickAudio` com um no-op (func 425 por meio de seus próprios
slots de tabela 944, 919, 1986); apenas `IConfereVotoEmCargo` tem um de verdade (func 11790).

## 3. Classes e hierarquia (RTTI)

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

Todo estado é um singleton criado sob demanda (`static unique_ptr` + mutex; o lock do mutex desapareceu
no build de thread única, restando apenas resíduos de `mutex_unlock`; os resets na saída do programa são as funcs 7287,
7416). O wasm-opt mesclou vários desses acessores em corpos compartilhados, com a vtable e o armazenamento como
parâmetros (funcs 1961, 2901, 6051, 764).

## 4. A sessão do eleitor

### 4.1 Fluxo de nível superior

```
CAguardaMensagem --msg 1 "Eleitor foi habilitado"--> CEleitorVotando
CEleitorVotando  --all cargos done: GravaVotos (func 4454)--> CSincronismoEleitor (u07/u19)
                 --suspension with discard--> CAguardaMensagem          (screen "FIM / NÃO VOTOU")
CSincronismoEleitor --> CFimVotoEleitor ("FIM / VOTOU", 4 beeps, "fim") --> CAguardaMensagem
```

O voto municipal registrado mostra exatamente esta sequência
(`CEleitorVotando/CPedeProporcional → CPedeNominal → CConfereVotoEmCargo<CConfirmaVotoNominal,2>
→ CConfirmaVotoNominal → CPedeMajoritario → CConfereVotoEmCargo<CMajoritarioNulo,7> →
CMajoritarioNulo → CSincronismoEleitor → CAguardaMensagem`).

### 4.2 `CEleitorVotando::StartState` (func 7377, `IniciaCiclo` inlinado)

1. empilha o seu `api::CApplicationContext` (texto de erro exibido se algo lançar exceção: *"Erro inesperado
   durante a votação" / "O voto do eleitor NÃO foi registrado" / "Ocorreu um erro enquanto o eleitor
   registrava suas escolhas."*, definido pelo construtor na func 3229);
2. envia `EleitorIniciouVotacao` (6) à thread do operador;
3. limpa a tela (`IScreen::Clear(1)`, `Refresh()`) e inicia o **tick de inatividade**
   (`cfg+172` s no modo de treinamento do eleitor, senão `cfg+168` s);
4. reinicia os globais: o número digitado `g_votoDigitado` (@1833288), a lista de votos confirmados
   `g_votosEleitor` (@1833300, entradas `{TCargoID, CVoto{tipo, numero}}`), o índice de escolha
   `g_numeroEscolha` (@1536340) = 1;
5. **filtra os cargos pela *abrangência* do eleitor** (voto em trânsito, func 3721): 0 = eleitor deste
   município → todos os cargos; 1 = outro município da mesma UF → cargos de abrangência 1 ou 2;
   2 = outra UF → apenas abrangência 2 (`CCargos::FiltraPorAbrangencia` inlinado, ccargos.cpp:249),
   depois os ordena (comparador func 11559: eleição, depois ordem do cargo) e volta ao início;
6. se o áudio estiver habilitado (`CInformacaoEleitor::m_modoAudio` ≠ 2; func 509, nomes conforme as unidades u02/u07), o primeiro subestado é
   `CInstrucaoVotacaoAcessibilidade` e a linha do operador mostra "Tela de instrução de
   acessibilidade"; caso contrário, a linha do operador mostra `VOTANDO PARA: <cargo>` e o subestado do
   primeiro cargo é criado (`ChamaEstadoProximoCargo`).

### 4.3 Escolhendo o subestado de um cargo (`ChamaEstadoProximoCargo`, func 4459)

| condição | subestado |
|---|---|
| nenhum candidato para o cargo nesta urna e não é consulta | `CConfirmaVotoSemCandidato` (tela 17, só aceita CONFIRMA → tipo de voto 8) |
| eletivo e majoritário (`CCargo+4 == 0`), ou consulta | `CPedeMajoritario` |
| eletivo e proporcional (`CCargo+4 == 1`) | `CPedeProporcional` |
| caso contrário | log "Erro na identificação do tipo de cargo", lança 9310 "Tipo de cargo nao identificado" |

### 4.4 Avançando (`NeedChangeState`, func 7352)

Se o subestado quer mudar e indica um próximo estado, o CEleitorVotando troca de subestado e permanece.
Se o próximo estado é `nullptr`, o cargo/escolha está concluído: `g_numeroEscolha++`, ou
`CCargos::Next()` e escolha = 1 quando todas as vagas do cargo estão concluídas (pulado uma vez após as
instruções de acessibilidade). Em seguida, um bipe curto (`IBeep` slot 2 com 1). Quando a lista se
esgota, ele chama a **func 4454** (unidade u22, nome do analisador `CVotosCargos::ConfereCedula`, na verdade um
método de CEleitorVotando com o código do RDV inlinado): agrupa `g_votosEleitor` por eleição, confere a
cédula (`ConfereCedula`), insere-a no `CRdvVota` (`InsereCedula`/`RecebeCedula`), marca a
hora do último voto em `EstadoGeralVota` (`MarcaUltimoVoto`) e define o próximo estado como
`CSincronismoEleitor`.

### 4.5 Conferência e confirmação

Após o último dígito (ou BRANCO, ou um número inválido), o estado `CPede*` muda para um
`CConfereVotoEmCargo<TConfirma, TELA>` (um singleton lazy por instanciação, inlinado nos
estados `CPede*`; seu construtor, func 1165, recebe apenas a tela). A duração da
conferência vem da configuração: `ParametrosUrna.tempoConfirmacaoVoto` (`CConfiguracaoEleicao +176`, ms). Se for > 0, o
estado cria um tick com essa duração, mostra `TELA`, inicia o tick, diz "Confira o seu voto." e
recusa todas as teclas (`PlayKey(0)` = bipe de erro + log "Tecla indevida pressionada"; depois
`ProcessInputAudio` esvazia o teclado, `IInputKbd` slot 4. Visto duas vezes no
`logd.dat` registrado). Se for ≤ 0, nenhum tick é criado e a tela nunca é exibida: `StartStateAudio`
vai direto para `TConfirma` assim que o áudio atual termina. Quando o tick dispara (depois de qualquer áudio ainda em reprodução,
`ExecutarAposAudioAtual`, func 3851), ele passa para o singleton `TConfirma`, cujo
`StartStateAudio` mostra a tela de confirmação. Em CONFIRMA (`EInputResult` 9), o voto é armazenado
com `CEleitorVotando::RegistraVoto(cargo, m_tipoVoto, g_votoDigitado)` (func 4471) e o estado
retorna `nullptr`; em CORRIGE (5), volta para `CPedeProporcional`/`CPedeMajoritario`. Antes de
sair, os dois caminhos chamam o hook do slot 17 com o resultado (um no-op, exceto em
`CConfirmaVotoNominal`/`CMajoritarioValido`, func 5925, que em CORRIGE registra no log "Eleitor corrigiu na
tela de confirmação de candidato" e esvazia o teclado por meio de `IPoliticaExecucaoEleitor`, §10).

| template de conferência (func do slot 16) | estado de confirmação | flags | tela de confirmação | `TipoVoto` do RDV armazenado |
|---|---|---|---|---|
| `<CConfirmaVotoNominal, ConferenciaVotoNominal(2)>` (11715) | CConfirmaVotoNominal | 6 | 1 Completa | 2 nominal |
| `<CConfirmaVotoLegenda, ConferenciaVotoLegenda(10)>` (11717) | CConfirmaVotoLegenda | 6 | 9 Voto Legenda Incompleto | 1 legenda |
| `<CCandidatoInexistente, 12>` (11716) | CCandidatoInexistente | 6 | 11 Candidato Inexistente | **1 legenda** (o prefixo do partido existe) |
| `<CCandidatoInapto, 14>` (11739) | CCandidatoInapto | 6 | 13 Candidato Inapto | 4 nulo |
| `<CProporcionalBranco, 4>` (11707) | CProporcionalBranco | 6 | 3 Voto Branco | 3 branco |
| `<CProporcionalNulo, 7>` (11738) | CProporcionalNulo | 6 | 6 Voto Nulo | 4 nulo |
| `<CMajoritarioValido, 2>` (11670) | CMajoritarioValido | 2 | 1 Completa | 2 nominal |
| `<CMajoritarioRepetido, 16>` (11671) | CMajoritarioRepetido | 6 | 15 Candidato Repetido | 7 nuloPorRepeticao |
| `<CMajoritarioNulo, 7>` (11672) | CMajoritarioNulo | 6 | 6 Voto Nulo | 4 nulo |
| `<CMajoritarioBranco, 4>` (11673) | CMajoritarioBranco | 6 | 3 Voto Branco | 3 branco |

(`CMajoritarioRepetido`: o mesmo candidato escolhido duas vezes para um cargo de várias vagas, detectado por
`CPedeMajoritario` ao percorrer `g_votosEleitor`.)

### 4.6 Tipos de voto produzidos por esta unidade

`comum::md::CVoto::ETipo` usa a numeração do RDV (`ModuloRegistroDigitalVoto.TipoVoto`):
1 legenda, 2 nominal, 3 branco, 4 nulo, 5 brancoAposSuspensao, 6 nuloAposSuspensao,
7 nuloPorRepeticao, 8 nuloCargoSemCandidato, 9 nuloAposSuspensaoCargoSemCandidato.
O **BU** usa outro enum (`ModuloBoletimUrna.TipoVoto`: nominal 1, branco 2, nulo 3, legenda 4,
cargoSemCandidato 5); a conversão acontece no gerador do BU (unidades u08/u22).
Cada voto armazenado é registrado no log apenas com o nome do cargo, p. ex. `1|1|Voto confirmado para [Vereador]`
(`logd.dat`, Latin-1); o log nunca contém o número votado.

### 4.7 Mensagens ao operador, inatividade e suspensão

Mensagens enviadas a `CThreadOperador` (prioridade 1): 1 fim, 2 não votou, 3/4 eleitor demorando
(sem/com votos confirmados), 5 voltou a digitar, 6 iniciou votação, 9 novo texto em
`CThreadOperador+108` ("VOTANDO PARA: …", no máximo 40 caracteres; a func 4464 recorre ao
nome abreviado do cargo quando o completo é longo demais).
Quando o tick de inatividade dispara, o CEleitorVotando envia 3 ou 4 e define `m_eleitorDemorando`; a
próxima tecla reinicia o timer e envia 5 ("Eleitor voltou a digitar enquanto estava sendo
suspenso"). Mensagens recebidas: 2 = suspenso pelo mesário, 3 = suspensão automática (treinamento
do eleitor), 4 = continuar (reiniciar o timer); as demais vão para o subestado.

`suspenderEleitor` (func 4442) aplica a configuração da eleição (`CInformacaoEleicao` em
`CConfiguracaoEleicao+88`):

| situação | campo de configuração | 0 | 1 | 2 | outro |
|---|---|---|---|---|---|
| nada confirmado ainda (cargo 0, escolha 1) | `+88` forma suspensão *sem* voto | descarta (DescartaVotos) | escolhas restantes = brancoAposSuspensao | = nuloAposSuspensao | lança 9312 |
| algum voto confirmado | `+92` forma suspensão *com* voto | = brancoAposSuspensao | = nuloAposSuspensao | **descarta tudo, inclusive os votos confirmados** | lança 9313 |

Ao preencher, cargos sem candidatos (e que não são consultas) recebem o tipo 9. Em seguida, registra no log "Eleitor votou
parcialmente e em seguida foi suspenso" e grava a cédula (func 4454) exatamente como em um fim
normal. `DescartaVotos` (func 4449), em vez disso, mostra "FIM / NÃO VOTOU", envia 2 ao operador, toca o
bipe de suspensão, para o áudio, registra no log "Eleitor foi suspenso e não confirmou nenhum voto" e volta para
`CAguardaMensagem` sem tocar no RDV.

## 5. Telas: `ETelaVotacao` e `CTelasCargo`

`NomeTela` (func 6718) dá as 18 telas: 0 Tela Inicial, 1 Tela Completa, 2 Conferência de Voto
Nominal, 3 Voto Branco, 4 Conferência de Voto Branco, 5 Voto Proporcional Nulo Incompleto, 6 Voto
Nulo, 7 Conferência de Voto Nulo, 8 Partido, 9 Voto Legenda Incompleto, 10 Conferência de Voto
Legenda, 11 Candidato Inexistente, 12 Conferência de Candidato Inexistente, 13 Candidato Inapto,
14 Conferência de Candidato Inapto, 15 Candidato Repetido, 16 Conferência de Candidato Repetido,
17 Cargo sem Candidato. `CTelasVota` (u07) constrói, na inicialização, um `CTelasCargo` por cargo
(a func 2386 valida: pelo menos uma tela, nenhuma tela nula) e `CTelasVota::GetTelaCargo`
(func 4135, com `CTelasCargo::GetTela` inlinado) retorna o `shared_ptr` de uma tela ou lança
9358/9330. Outros membros de `CTelasVota` usados aqui: +124 `telaInstrucoesAcessibilidade`, +156
`telaFim` "FIM/VOTOU", +164 `telaFim` "FIM/NÃO VOTOU" (Latin-1 "N\xC3O VOTOU").

## 6. Acessibilidade (áudio do eleitor)

Quando o mesário habilita o áudio para o eleitor (as mensagens 8/9 da thread do eleitor definem `CInformacaoEleitor::m_modoAudio` como 0 "conforme cadastro" / 1
"habilitado", e a 10 volta para 2 = desabilitado), a sessão começa com `CInstrucaoVotacaoAcessibilidade`: volume reiniciado para 7/10
e velocidade da fala para 2, e o texto *"A urna está pronta para receber o seu voto. Use o teclado numérico
{abaixo | à direita} da urna para digitar o seu voto. Aperte a tecla 4 para fala mais lenta. Aperte a
tecla 6 para fala mais rápida. Aperte a tecla 3 para aumentar o volume. Aperte a tecla 9 para
diminuir o volume. Aperte confirma para iniciar o seu voto"* ("abaixo" quando o modelo da `IUrna` é > 2019).
Teclas: 3/9 volume ±1 (0..10, "Volume máximo/mínimo" nos limites), 4/6 fala mais lenta/mais rápida
("Fala em velocidade mínima/máxima" nos limites), CONFIRMA inicia o voto, todas as outras teclas e
CORRIGE são recusadas. Uma tecla recusada **não é falada**: `PlayKey(0)` (func 1455) dá o bipe de erro de 554 Hz
(`IBeep::BeepErro`, `CWasmBeep` func 8494) e grava a linha de log "Tecla indevida pressionada". Com a voz
ligada, a urna também tocaria o arquivo de clique `:/resource/sounds/tecE.wav`, mas o slot de tocar arquivo de `CWasmWebSound`
(func 9580) não toca nada. Confirmado com `node tools/run/headless.mjs --audio` (estado
`CInstrucaoVotacaoAcessibilidade` e depois `CPedeProporcional` após `C`; `logd.dat` mostra as duas linhas
"Tecla indevida pressionada" para as teclas 5 e CORRIGE). Todos os textos falados estão em Latin-1 no binário.

## 7. Roteamento de inicialização e reinício (apenas urna real)

`CAjusteInicial::StartState` (func 7160) = `AjustaDataHora` + `ValidaTemposDesligamento` +
`LimpaMidiaResultado` + (modo de demonstração) registrar `CControladorRegistraMesariosVota` + `InicioVota`.

* **AjustaDataHora** — apenas no modo de demonstração ou na fase de treinamento (`CEstadoGeral+48 == '3'`) e
  antes de a votação começar: move o relógio do sistema (via `IAjusteDataHora`, registrando o delta em
  `EstadoGeralUrna.ajusteDataHora` e depois regravando `eg.bin`) para uma data/hora configurada
  (`cfg+556/+564` no modo de treinamento do eleitor, senão `cfg+544/+552` + 30 s). `CIniciodeCiclo`
  (func 7306) roda quando o operador inicia o período de votação: se `estadoVota` ≤ 55, move o
  relógio para `cfg+556/+564` + 10 s, mas apenas no modo de demonstração ou em uma urna em fase de treinamento que
  **não** esteja em treinamento do eleitor (func 1823), depois define `estadoVota = votar` (56) e salva; em todos os
  casos, vai para `CInicioVotacao`.
* **ValidaTemposDesligamento** — `cfg+180` (limite de tempo na bateria) e `cfg+184` (tempo de aviso) devem ser
  > 0 e limite ≥ aviso (erros 9405-9407).
* **LimpaMidiaResultado** — fase de treinamento, fora do treinamento do eleitor: monta a MR (`/dsk/mr/`) e apaga
  todas as entradas exceto `infomidia*` e `turno2*`, sobrescrevendo antes os arquivos regulares com zeros
  (func 2760, nunca através de um symlink).
* **InicioVota** — a tabela de recuperação (enum C++ = `EstadoVota` do ASN.1 + 49):

| estadoVota | nome ASN.1 | próximo estado |
|---|---|---|
| 49-51 | inicial, gerabasedinamica, aguardahorazeresima | CVerificaEleicaoPassou |
| 52 | gerarze | CInicioZeresima |
| 53 | zeresimagerada | CImprimindoZeresima |
| 54-56 | zeresimaimpressa, registromesarioinicial, votar | testeteclado::CRetomada (teste de teclado, depois retomada) |
| 57 | fimaquisicaovotos | CFinalizaAquisicao |
| 58 | registromesariofinal | CReinicioComparecimentoMesario |
| 59 | gerarbu | CGeraBU |
| 60 | gerarrelatorios | CGeraRelatorios |
| 61 | imprimirbu | CInicioBU |
| 62 | gravarresultados | CGravaResultado |
| 63 | copiaresultadosmr | CCopiaResultadoParaMR |
| 64 | encerrada → estadoEncerramento: inicial/imprimirobrigatoriabu → CImprimirBUOutrasObrigatorias, retirarmr → CRetirarMR, fimdostrabalhos → CVerificaQtdBUsAdicionais, senão lança 9303 |
| outro (incl. 65 exibealertadesligamento) | | lança 9304 "Estado nao conhecido" |

`CDefineRotaPosReinicio::NeedChangeState` (func 11858): estadoVota 54/55 → `CReinicioComparecimentoMesario`
quando a func 2520 é verdadeira (modo de demonstração, a menos que a urna seja uma urna em fase de treinamento em treinamento
do eleitor; uma urna fora do modo de demonstração nunca vai para lá) ou `CInicioVotacao` caso contrário; 56 → `CInicioVotacao`;
senão, registra no log "Erro estado do aplicativo não conhecido" e lança 9309.

`CEstadoComDesligamentoAutomatico::ProcessTick` (func 12061): a cada segundo, se `IPower` informar a
bateria interna (`status & 6 == 2`), arma um prazo `now + (limit − warning)`; quando ele passa, armazena
`deadline + warning` em `g_dataHoraDesligamento` (@1833312) e muda para
`CExibeAlertaDesligamento` (singleton func 5979). De volta à alimentação externa, o prazo é limpo.

## 8. Boletim de Urna (BU): o que esta unidade contribui

Esta unidade não monta o BU. Suas contribuições para "como um BU funciona":

1. **Conteúdo**: todo voto que acaba no BU/RDV é criado aqui (`RegistraVoto`,
   `suspenderEleitor`) com os tipos do §4.6 e anexado a `g_votosEleitor`; a func 4454 (u22)
   transforma a lista do eleitor em uma *cédula* do RDV após as verificações de `ConfereCedula` (contagem de votos por cargo =
   número de escolhas, tamanho do voto nominal = dígitos do cargo, repetição apenas para cargos de várias vagas).
   Uma sessão descartada (DescartaVotos) não produz cédula nem comparecimento.
2. **Entrada no encerramento**: o "encerrar" do operador chega a `CAguardaMensagem` como mensagem 7
   (`estadoVota = gerarbu (59)`, salva, log "Inicio do Encerramento", próximo `CGeraBU`); a cadeia do BU é
   CGeraBU (59) → CGeraRelatorios (60) → CInicioBU/impressão (61) → CGravaResultado (62) →
   CCopiaResultadoParaMR (63) → encerrada (64: vias obrigatórias, retirar a MR, vias extras).
3. **Segurança no reinício**: após uma reinicialização durante o encerramento, `CAjusteInicial::InicioVota` reentra
   exatamente na etapa do BU a partir de `vota.bin` (tabela no §7).
4. **Telas dos estados do BU** construídas pelos acessores singleton desta unidade: `CGeraBU` (func 6129)
   "Votação encerrada" (`telaVotacaoEncerrada`) e "Preparando dados para encerramento"
   (`telaPreparandoDadosEncerramento`), `CGeraRelatorios` (6084) esta última, `CCopiaResultadoParaMR`
   (6174) "Gravando o resultado na mídia" (`telaCopiaResultadoParaMR`), cada uma com
   "Por favor, aguarde..." (func 4152). Em uma urna de treinamento, a MR é limpa na inicialização, exceto
   `infomidia*`/`turno2*` (§7).

## 9. Dados lidos e gravados

| dado | acesso |
|---|---|
| `CConfiguracaoEleicao` +88/+92 | forma de suspensão sem/com voto |
| +168/+172 | timeout de inatividade do eleitor (s): normal / treinamento do eleitor |
| +176 | `ParametrosUrna.tempoConfirmacaoVoto`: duração da tela de conferência (ms; 1000 nos cenários publicados, segundo a unidade u37); ≤ 0 = sem tela de conferência (func 1165) |
| +180/+184 | limite de tempo na bateria / aviso de desligamento (s) |
| +544/+552, +556/+564 | datas/horas configuradas usadas para ajustar o relógio em demonstração/treinamento |
| `CEstadoGeral` (eg.bin) +48 fase, +52 ajusteDataHora | lido; ajuste gravado pelos ajustes de relógio |
| `CEstadoGeralVota` (vota.bin) estadoVota, estadoEncerramento, treinamentoEleitor (+72) | lido; estadoVota gravado por CIniciodeCiclo (56) |
| `CCargos`, `CCandidaturas`, `CEleitores` (eleitor atual) | lidos (lista de cargos, candidatos, trânsito) |
| `logd.dat` | via `CLogVota` (severidade 1 info, 3 erro) |
| `/dsk/mr/*` | apagado por LimpaMidiaResultado (urna de treinamento) |
| globais `g_votoDigitado`, `g_votosEleitor`, `g_numeroEscolha` | rascunho da sessão; também lidos pelo adaptador web (`votaGetStateJson` lê os 2 primeiros dígitos digitados para informar `legendaValida`) |

## 10. Particularidades do build web

* Estado inicial `CAguardaMensagem` em vez de `CAjusteInicial`: sem ajuste de relógio, sem limpeza da MR,
  sem roteamento de reinício no simulador.
* `IPoliticaExecucaoEleitor`: o build web registra `CPoliticaExecucaoEleitorWeb` (vota_web_wasm.cpp:361,
  func 10835, instalado por `main`, func 10307), que esvazia o teclado **uma vez**; a versão da urna
  (func 13564) esvazia 3-6 vezes com pausas aleatórias de 50-149 ms (IRng). Ambas rodam apenas após um CORRIGE
  nas telas de confirmação de candidato (`CConfirmaVotoNominal` / `CMajoritarioValido` slot 17,
  func 5925), não após todo CORRIGE.
* Hardware mock alcançado a partir daqui: `CWasmScreen` (Clear/Refresh; o slot 14 é um no-op),
  `CWasmInputKbd` (slot 7 no-op), `CWasmBeep` (sequências de tons), `CWasmWebSound` (Web Audio),
  `ITextToSpeech` = RHVoice.
* O adaptador lê `CEleitorVotando+12` (o subestado) comparando a vtable do objeto com
  1533152 e imprime o nome RTTI do subestado como `substate`; ele informa "vota:done" quando o nome do
  estado volta a ser `CAguardaMensagem` (também após uma sessão descartada).

## 11. Observações sobre wasm / Emscripten

* **Recuperação de nomes de slot via `__PRETTY_FUNCTION__`**: `GetTelaCargoAtual(const std::string&)` recebe o
  pretty name do chamador, que nomeia exatamente as funcs 7428, 7441, 11754, 11755.
* **merge-similar-functions**: 2302 (5 thunks de log que diferem apenas pela string de formato), 3921
  (GetTelaCargoAtual com srcloc/código de erro como parâmetros), 1961/2901/6051/764 (singletons).
* **O inlining esconde o virtual**: as funcs 7160, 7306, 7377, 11793, 12061, 4407, 4135 foram nomeadas pelo
  analisador a partir do srcloc de uma chamada inlinada; a tabela abaixo dá a função virtual/externa real.
* Os argumentos de `std::format` são empacotados como códigos de tipo (6 = unsigned, 13 = string_view, 3 = int,
  15 = handle); um id de cargo `uebyte` é formatado como número ("Cargo 13 "). `NomeTela` converte a
  tela para `int` (código 3), mas os valores `EEstadoVota`/`EEstadoEncerramento` nas mensagens 9303/9304/9309
  são passados como enums (código 15, um `std::formatter` de usuário cujo corpo ICF, func 536, imprime
  o valor sem sinal, p. ex. "Estado nao conhecido = 65").
* `emscripten_sleep` atrás da flag global @1584624 (valor inicial 1) é alcançável a partir de
  `LimpaMidiaResultado`/`HabilitaMR` e de `CPoliticaExecucaoEleitor::LimpaBufferInput`; ambos estão mortos
  no fluxo web (ver a lista de suspeitos).

## 12. Código estranho ou arriscado

1. **`emscripten_sleep` no caminho da MR** (func 7160 `LimpaMidiaResultado`, via func 2863 e uma
   chamada direta): `sleep(500)`/`sleep(100)` protegidos pela flag @1584624 = 1. Sem Asyncify, o glue
   aborta (`upstream/site/wasm/vota_web_wasm.js`: `_emscripten_sleep=()=>{abort("Please compile your
   program with async support …")}`). Inalcançável no simulador porque o build web nunca entra em `CAjusteInicial`; na urna
   isto é `sleep_for`. Info.
2. **`CPoliticaExecucaoEleitor::LimpaBufferInput`** (func 13564) dorme 3-6 × 50-149 ms por meio da
   mesma flag: abortaria, mas o build web registra sua própria política antes. A política web esvazia o teclado
   uma vez, então o tratamento de teclas pós-CORRIGE do simulador não é o da urna (teclas digitadas durante a
   janela de esvaziamento de ~0.15-0.9 s são mantidas no build web). Os restos são com sinal (`i32.rem_s`): um número
   aleatório negativo dá 0-2 passadas. Baixo.
3. **Lista de cargos vazia → reinício sem fim** (func 7377/7352): se o filtro de trânsito não deixar nenhum cargo,
   `StartState` define `m_estadoCargo = nullptr` enquanto `m_proximoEstado` continua `this`; o próximo evento
   faz `NeedChangeState` retornar true e a thread reexecuta `StartState` (novo push de contexto,
   mensagem 6 ao operador, limpeza de tela) para sempre; nenhum voto e nenhum comparecimento são registrados. Só
   uma suspensão pelo mesário sai disso. Baixo (requer um eleitor habilitado sem nenhum cargo elegível).
   O reinício é dirigido por eventos, não é um laço ocupado: `CThreadEleitor::Processar` (func 4349)
   chama `FinishState` e depois `StartState` em `GetNextState()` (= `this`) após cada tecla, tick ou
   mensagem. Estabelecido pela leitura do código (7377/7352/4349); não reproduzido em runtime (nenhum
   cenário tem um eleitor assim).
4. **Descartar votos confirmados é registrado no log e exibido como "não votou"** (func 4442 → 4449): com
   `formaSuspensaoComVoto == 2`, os votos que o eleitor já confirmou são descartados, o log diz
   "Eleitor foi suspenso e não confirmou nenhum voto", a tela diz "NÃO VOTOU" e o
   operador recebe a mensagem 2. Pode-se argumentar que a tela e a mensagem 2 são intencionais (a cédula é
   cancelada, então o eleitor é tratado como se não tivesse votado); o texto do log está claramente errado para esse
   caso (a linha de log anterior "Eleitor foi suspenso pelo mesário" é a única pista). O inverso
   também acontece: com `formaSuspensaoSemVoto` 1/2, um eleitor que não confirmou **nada** tem todas as
   escolhas preenchidas com branco/nulo e o log diz "Eleitor votou parcialmente e em seguida foi
   suspenso". Dirigido pela configuração. Baixo (redação da trilha de auditoria). Estabelecido pela leitura das funcs 4442/4449; a
   API do simulador (`votaPressKey` etc.) não tem como suspender um eleitor, então isso não foi executado.
5. **`g_votosEleitor` mantém as escolhas do último eleitor na memória** até a próxima habilitação
   (não é limpo por `GravaVotos`/`DescartaVotos`, apenas no próximo `StartState`), e
   `g_votoDigitado` mantém o último número digitado do último cargo. Inofensivo no simulador; em uma
   urna real, é estado apenas em RAM dentro do perímetro do sigilo do voto. Info.
6. **`CCandidatoInexistente` armazena um voto de *legenda*** (tipo 1 do RDV) com o número digitado completo, e
   `CCandidatoInapto` armazena um *nulo* com o número: correto segundo as regras eleitorais, mas vale
   saber disso ao ler as contagens do RDV/BU. Info. Os tipos são constantes nas funcs 11716/11739 (tipo 1 e
   4). O caminho foi executado: `node tools/run/headless.mjs --keys "91999  C  B  C  "` passa por
   `CPedeNominal → CConfereVotoEmCargo<CCandidatoInexistente,12> → CCandidatoInexistente` (adaptador
   `voteMode: "legenda"`) e registra no log "Voto confirmado para [Vereador]". O tipo armazenado em si não é
   visível de fora, porque o build web não persiste o RDV.
7. Os subestados de cargo são singletons compartilhados entre eleitores; seu estado por eleitor só é reiniciado em
   `StartState*` (p. ex., volume/velocidade no estado de acessibilidade), então qualquer campo não reiniciado ali vaza
   entre eleitores. Nada concreto encontrado. Info.

## 13. Questões em aberto

* Nomes exatos dos campos de data/hora de `CConfiguracaoEleicao` em +544/+552 e +556/+564 (zerésima
  vs. início da votação?).
* Nomes oficiais do slot 14 de `IScreen` e do slot 7 de `IInputKbd` (no-ops nos mocks web), dos
  slots de `IBeep`/`ISound` e dos valores 5/9/13 de `api::EInputResult` (as unidades u06/u08 usam Corrige/Confirma/Tecla).
* Se os helpers `CLogVota::Loga*` (família 2302, 4529) são membros de CLogVota ou helpers livres em
  celeitorvotando.cpp; seu primeiro argumento é a instância de CLogVota.
* `CDefineRotaPosReinicio +12` (definido como 0 antes do roteamento): significado desconhecido.
* Se alguma configuração real define `tempoConfirmacaoVoto` (`CConfiguracaoEleicao +176`) ≤ 0,
  o que remove a tela de conferência.
* Entre unidades: o `ctelasvota.h` da unidade u07 não declara as telas "FIM" `+156`/`+164` que
  `cfimvotoeleitor.cpp`/`celeitorvotando.cpp` usam (`m_telaFimVotou`/`m_telaFimNaoVotou`), e o
  `cvotacaostateaudio.h` da unidade u08 faz uma declaração antecipada de `CFormInterativoTelaVota` como classe, embora ele seja
  o alias de `shared_ptr` de `ctelasvota.h`/`ctelascargo.h`.

## 14. Tabela de mapeamento (todas as 83 funções da u06)

"executou" = observada em execução nos votos registrados. "reconstruído em" é relativo a
`src/uenux2/src/app/vota/eleitor/`; "(comentário)" significa que a função é explicada ali como um comentário
(corpos mesclados, instanciações de biblioteca, acessores de classes que pertencem a outras unidades).

| func | tamanho | executou | nome do analisador | símbolo reconstruído | arquivo original | reconstruído em (src/…/vota/eleitor/) | conf. |
|---:|---:|:-:|---|---|---|---|---|
| 233 | 14 | ✓ | `api_f233` | `comum::IEventosLog::Loga` | uenux2/src/app/comum/log/ieventoslog.cpp | celeitorvotando.cpp (pontos de chamada) | baixa |
| 1785 | 70 | ✓ | `vota_f1785` | `vota::CVotacaoStateAudio::CVotacaoStateAudio` | …/eleitor/cvotacaostateaudio.cpp | celeitorvotando.cpp (comentário no apêndice) | alta |
| 1961 | 79 | ✓ | `vota_f1961` | `vota::merged_GetInst_CConfirmaProporcional` | …/eleitor/cconferevotoemcargo.cpp | cconferevotoemcargo.cpp (comentário) | média |
| 2302 | 483 | ✓ | `vota_f2302` | `vota::CLogVota::merged_LogaVotoCargo` | uenux2/src/app/vota/log/clogvota.cpp | celeitorvotando.cpp (comentário) | média |
| 2386 | 941 | ✓ | `vota::CTelasCargo::CTelasCargo` | `vota::CTelasCargo::CTelasCargo` | …/eleitor/comum/ctelascargo.cpp | comum/ctelascargo.cpp | alta |
| 2466 | 59 |  | `vota::CInstrucaoVotacaoAcessibilidade::vf0` | `vota::CInstrucaoVotacaoAcessibilidade::~CInstrucaoVotacaoAcessibilidade` | …/eleitor/cinstrucaovotacaoacessibilidade.cpp | cinstrucaovotacaoacessibilidade.cpp | alta |
| 2483 | 13 |  | `vota::CEleitorVotando::vf0` | `vota::CEleitorVotando::~CEleitorVotando` | …/eleitor/celeitorvotando.cpp | celeitorvotando.cpp | alta |
| 2760 | 94 | ✓ | `vota_f2760` | `api::CSystem::SecureRemove` | uenux2/src/api/util/csystem.cpp | cajusteinicial.cpp (apêndice) | baixa |
| 2901 | 81 | ✓ | `vota_f2901` | `vota::merged_GetInst_CConfirmaMajoritario` | …/eleitor/cconferevotoemcargo.cpp | cconferevotoemcargo.cpp (comentário) | média |
| 3251 | 466 | ✓ | `vota_f3251` | `std::__tree<std::__value_type<vota::ETelaVotacao,CFormInterativoTelaVota>>::__find_equal` | — (helper inline/mesclado) | biblioteca (comentário em ctelascargo.cpp) | média |
| 3272 | 17 |  | `vota_f3272` | `vota::CLogVota::LogaVotoCargoSemCandidatoSuspensao` | uenux2/src/app/vota/log/clogvota.cpp | celeitorvotando.cpp (comentário) | baixa |
| 3783 | 378 | ✓ | `comum_f3783` | `std::vector<comum::CCargos::SItem>::__assign_with_size` | — (helper inline/mesclado) | biblioteca (comentário em celeitorvotando.cpp) | média |
| 3849 | 20 |  | `vota_f3849` | `vota::CPedeProporcional::GetInst` | …/eleitor/votaproporcional/cpedeproporcional.cpp | celeitorvotando.cpp (comentário) | média |
| 3851 | 781 | ✓ | `vota::IConfereVotoEmCargo::ExecutarAposAudioAtual` | `vota::IConfereVotoEmCargo::ExecutarAposAudioAtual` | …/eleitor/cconferevotoemcargo.cpp | cconferevotoemcargo.cpp | alta |
| 4135 | 1190 | ✓ | `vota::CTelasCargo::GetTela` | `vota::CTelasVota::GetTelaCargo` | …/eleitor/comum/ctelasvota.cpp | comum/ctelascargo.cpp | alta |
| 4152 | 297 |  | `vota_f4152` | `vota::CriaTelaStatusAguarde` | — (helper inline/mesclado) | cajusteinicial.cpp (comentário) | baixa |
| 4192 | 704 |  | `vota_f4192` | `vota::CEleitorVotando::InsereVoto` | …/eleitor/celeitorvotando.cpp | celeitorvotando.cpp | baixa |
| 4407 | 408 |  | `vota::CInstrucaoVotacaoAcessibilidade::GetKeyboardPosition` | `vota::CInstrucaoVotacaoAcessibilidade::GetMensagemAudio` | …/eleitor/cinstrucaovotacaoacessibilidade.cpp | cinstrucaovotacaoacessibilidade.cpp | média |
| 4420 | 141 | ✓ | `vota_f4420` | `vota::CInstrucaoVotacaoAcessibilidade::GetInst` | …/eleitor/cinstrucaovotacaoacessibilidade.cpp | cinstrucaovotacaoacessibilidade.cpp | média |
| 4442 | 1183 |  | `vota::CEleitorVotando::suspenderEleitor` | `vota::CEleitorVotando::suspenderEleitor` | …/eleitor/celeitorvotando.cpp | celeitorvotando.cpp | alta |
| 4449 | 352 |  | `vota::CEleitorVotando::DescartaVotos` | `vota::CEleitorVotando::DescartaVotos` | …/eleitor/celeitorvotando.cpp | celeitorvotando.cpp | alta |
| 4459 | 604 | ✓ | `vota::CEleitorVotando::ChamaEstadoProximoCargo` | `vota::CEleitorVotando::ChamaEstadoProximoCargo` | …/eleitor/celeitorvotando.cpp | celeitorvotando.cpp | alta |
| 4464 | 1272 | ✓ | `vota_f4464` | `vota::InformaCargoAoOperador` | …/eleitor/celeitorvotando.cpp | celeitorvotando.cpp | baixa |
| 4471 | 140 | ✓ | `vota_f4471` | `vota::CEleitorVotando::RegistraVoto` | …/eleitor/celeitorvotando.cpp | celeitorvotando.cpp | baixa |
| 4477 | 243 |  | `vota::CConfirmaVotoSemCandidato::EmiteEcoComInputField` | `vota::CConfirmaVotoSemCandidato::EmiteEcoComInputField` | …/eleitor/cconfirmavotosemcandidato.cpp | cconfirmavotosemcandidato.cpp | alta |
| 4483 | 20 |  | `vota::CConfirmaVotoSemCandidato::GetTelaCargoAtual` | `vota::CConfirmaVotoSemCandidato::GetTelaCargoAtual` | …/eleitor/cconfirmavotosemcandidato.cpp | cconfirmavotosemcandidato.cpp | alta |
| 4529 | 150 |  | `vota_f4529` | `vota::CLogVota::LogaAudioDesativadoFimVotacao` | uenux2/src/app/vota/log/clogvota.cpp | (apenas documentação) | baixa |
| 4537 | 17 | ✓ | `vota_f4537` | `vota::CLogVota::LogaVotoConfirmado` | uenux2/src/app/vota/log/clogvota.cpp | celeitorvotando.cpp (comentário) | baixa |
| 4541 | 17 |  | `vota_f4541` | `vota::CLogVota::LogaVotoNuloSuspensao` | uenux2/src/app/vota/log/clogvota.cpp | celeitorvotando.cpp (comentário) | baixa |
| 4542 | 17 |  | `vota_f4542` | `vota::CLogVota::LogaVotoCargoSemCandidato` | uenux2/src/app/vota/log/clogvota.cpp | celeitorvotando.cpp (comentário) | baixa |
| 4546 | 17 |  | `vota_f4546` | `vota::CLogVota::LogaVotoBrancoSuspensao` | uenux2/src/app/vota/log/clogvota.cpp | celeitorvotando.cpp (comentário) | baixa |
| 4640 | 15 |  | `vota_f4640` | `api::CThread::AdicionaTick` | — (helper inline/mesclado) | cestadocomdesligamentoautomatico.cpp (comentário) | baixa |
| 5474 | 12 |  | `vota_f5474` | `api::CDateTime::operator+=` | — (helper inline/mesclado) | cestadocomdesligamentoautomatico.cpp (comentário) | baixa |
| 5921 | 20 |  | `vota_f5921` | `vota::CPedeMajoritario::GetInst` | …/eleitor/votamajoritario/cpedemajoritario.cpp | celeitorvotando.cpp (comentário) | média |
| 5928 | 20 | ✓ | `vota::CConfirmaVotoEmCargo::GetTelaCargoAtual` | `vota::CConfirmaVotoEmCargo::GetTelaCargoAtual` | …/eleitor/cconfirmavotoemcargo.cpp | cconfirmavotoemcargo.cpp | alta |
| 5929 | 35 | ✓ | `vota_f5929` | `vota::CConfirmaVotoEmCargo::CConfirmaVotoEmCargo` | …/eleitor/cconfirmavotoemcargo.cpp | cconfirmavotoemcargo.cpp | alta |
| 5973 | 22 |  | `vota_f5973` | `vota::CImprimindoZeresima::GetInst` | …/eleitor/iniciovotacao/cimprimindozeresima.cpp | cajusteinicial.cpp (comentário) | média |
| 5979 | 202 |  | `vota_f5979` | `vota::CExibeAlertaDesligamento::GetInst` | …/eleitor/iniciovotacao/cexibealertadesligamento.cpp | cestadocomdesligamentoautomatico.cpp (comentário) | média |
| 6051 | 77 |  | `vota_f6051` | `vota::merged_GetInst_CVotacaoStateAudio28` | — (helper inline/mesclado) | celeitorvotando.cpp (comentário) | média |
| 6084 | 105 |  | `vota_f6084` | `vota::CGeraRelatorios::GetInst` | …/eleitor/fimvotacao/cgerarelatorios.cpp | cajusteinicial.cpp (comentário) | média |
| 6129 | 466 |  | `vota_f6129` | `vota::CGeraBU::GetInst` | …/eleitor/fimvotacao/cgerabu.cpp | cajusteinicial.cpp (comentário) | média |
| 6174 | 470 |  | `vota_f6174` | `vota::CCopiaResultadoParaMR::GetInst` | …/eleitor/fimvotacao/ccopiaresultadoparamr.cpp | cajusteinicial.cpp (comentário) | média |
| 6583 | 398 |  | `vota_f6583` | `vota::CriaTelaPreparandoDadosEncerramento` | — (helper inline/mesclado) | cajusteinicial.cpp (comentário) | baixa |
| 6718 | 2086 |  | `vota::NomeTela` | `vota::NomeTela` | …/eleitor/comum/ctelascargo.cpp | comum/ctelascargo.cpp | alta |
| 7160 | 5220 |  | `vota::CAjusteInicial::ValidaTemposDesligamento` | `vota::CAjusteInicial::StartState` | …/eleitor/cajusteinicial.cpp | cajusteinicial.cpp | alta |
| 7198 | 207 | ✓ | `vota::CFimVotoEleitor::StartState` | `vota::CFimVotoEleitor::StartState` | …/eleitor/cfimvotoeleitor.cpp | cfimvotoeleitor.cpp | alta |
| 7242 | 123 |  | `vota::CInstrucaoVotacaoAcessibilidade::StartStateAudio` | `vota::CInstrucaoVotacaoAcessibilidade::StartStateAudio` | …/eleitor/cinstrucaovotacaoacessibilidade.cpp | cinstrucaovotacaoacessibilidade.cpp | alta |
| 7248 | 1315 |  | `vota::CInstrucaoVotacaoAcessibilidade::ProcessInputAudio` | `vota::CInstrucaoVotacaoAcessibilidade::ProcessInputAudio` | …/eleitor/cinstrucaovotacaoacessibilidade.cpp | cinstrucaovotacaoacessibilidade.cpp | alta |
| 7255 | 13 |  | `vota::CInstrucaoVotacaoAcessibilidade::vf1` | `vota::CInstrucaoVotacaoAcessibilidade::~CInstrucaoVotacaoAcessibilidade(deleting)` | …/eleitor/cinstrucaovotacaoacessibilidade.cpp | cinstrucaovotacaoacessibilidade.cpp | alta |
| 7287 | 38 |  | `vota_f7287` | `vota::CInstrucaoVotacaoAcessibilidade::GetInst::__dtor_s_inst` | …/eleitor/cinstrucaovotacaoacessibilidade.cpp | cinstrucaovotacaoacessibilidade.cpp (comentário) | média |
| 7306 | 251 |  | `vota::CIniciodeCiclo::AjustaDataHora` | `vota::CIniciodeCiclo::StartState` | …/eleitor/ciniciodeciclo.cpp | ciniciodeciclo.cpp | alta |
| 7337 | 96 |  | `vota::CEleitorVotando::vf6` | `vota::CEleitorVotando::ProcessMessage` | …/eleitor/celeitorvotando.cpp | celeitorvotando.cpp | média |
| 7345 | 287 | ✓ | `vota::CEleitorVotando::vf7` | `vota::CEleitorVotando::ProcessInput` | …/eleitor/celeitorvotando.cpp | celeitorvotando.cpp | alta |
| 7349 | 162 | ✓ | `vota::CEleitorVotando::vf8` | `vota::CEleitorVotando::ProcessTick` | …/eleitor/celeitorvotando.cpp | celeitorvotando.cpp | alta |
| 7352 | 305 | ✓ | `vota::CEleitorVotando::NeedChangeState` | `vota::CEleitorVotando::NeedChangeState` | …/eleitor/celeitorvotando.cpp | celeitorvotando.cpp | alta |
| 7370 | 61 | ✓ | `vota::CEleitorVotando::vf5` | `vota::CEleitorVotando::FinishState` | …/eleitor/celeitorvotando.cpp | celeitorvotando.cpp | alta |
| 7377 | 2313 | ✓ | `vota::CEleitorVotando::IniciaCiclo` | `vota::CEleitorVotando::StartState` | …/eleitor/celeitorvotando.cpp | celeitorvotando.cpp | alta |
| 7385 | 13 |  | `vota::CEleitorVotando::vf1` | `vota::CEleitorVotando::~CEleitorVotando(deleting)` | …/eleitor/celeitorvotando.cpp | celeitorvotando.cpp | alta |
| 7416 | 38 |  | `vota_f7416` | `vota::CEleitorVotando::GetInst::__dtor_s_inst` | …/eleitor/celeitorvotando.cpp | celeitorvotando.cpp (comentário) | média |
| 7425 | 51 |  | `vota::CConfirmaVotoSemCandidato::vf15` | `vota::CConfirmaVotoSemCandidato::GetMensagemAudio` | …/eleitor/cconfirmavotosemcandidato.cpp | cconfirmavotosemcandidato.cpp | média |
| 7428 | 278 |  | `vota::CConfirmaVotoSemCandidato::vf9` | `vota::CConfirmaVotoSemCandidato::ProcessInputAudio` | …/eleitor/cconfirmavotosemcandidato.cpp | cconfirmavotosemcandidato.cpp | alta |
| 7441 | 267 |  | `vota::CConfirmaVotoSemCandidato::vf10` | `vota::CConfirmaVotoSemCandidato::StartStateAudio` | …/eleitor/cconfirmavotosemcandidato.cpp | cconfirmavotosemcandidato.cpp | alta |
| 11670 | 28 | ✓ | `vota::CConfereVotoEmCargo<vota::CMajoritarioValido, (vota::ETelaVotacao)2>::vf16` | `vota::CConfereVotoEmCargo<vota::CMajoritarioValido, (vota::ETelaVotacao)2>::GetProximoEstado` | …/eleitor/cconferevotoemcargo.h | cconferevotoemcargo.h | média |
| 11671 | 28 |  | `vota::CConfereVotoEmCargo<vota::CMajoritarioRepetido, (vota::ETelaVotacao)16>::vf16` | `vota::CConfereVotoEmCargo<vota::CMajoritarioRepetido, (vota::ETelaVotacao)16>::GetProximoEstado` | …/eleitor/cconferevotoemcargo.h | cconferevotoemcargo.h | média |
| 11672 | 28 | ✓ | `vota::CConfereVotoEmCargo<vota::CMajoritarioNulo, (vota::ETelaVotacao)7>::vf16` | `vota::CConfereVotoEmCargo<vota::CMajoritarioNulo, (vota::ETelaVotacao)7>::GetProximoEstado` | …/eleitor/cconferevotoemcargo.h | cconferevotoemcargo.h | média |
| 11673 | 28 |  | `vota::CConfereVotoEmCargo<vota::CMajoritarioBranco, (vota::ETelaVotacao)4>::vf16` | `vota::CConfereVotoEmCargo<vota::CMajoritarioBranco, (vota::ETelaVotacao)4>::GetProximoEstado` | …/eleitor/cconferevotoemcargo.h | cconferevotoemcargo.h | média |
| 11699 | 5 |  | `vota::CConfirmaMajoritario::vf16` | `vota::CConfirmaMajoritario::GetEstadoCorrige` | …/eleitor/cconfirmavotoemcargo.cpp | cconfirmavotoemcargo.cpp | média |
| 11700 | 25 | ✓ | `vota_f11700` | `vota::CConfirmaMajoritario::CConfirmaMajoritario` | …/eleitor/cconfirmavotoemcargo.cpp | cconfirmavotoemcargo.cpp | alta |
| 11707 | 26 |  | `vota::CConfereVotoEmCargo<vota::CProporcionalBranco, (vota::ETelaVotacao)4>::vf16` | `vota::CConfereVotoEmCargo<vota::CProporcionalBranco, (vota::ETelaVotacao)4>::GetProximoEstado` | …/eleitor/cconferevotoemcargo.h | cconferevotoemcargo.h | média |
| 11715 | 26 | ✓ | `vota::CConfereVotoEmCargo<vota::CConfirmaVotoNominal, (vota::ETelaVotacao)2>::vf16` | `vota::CConfereVotoEmCargo<vota::CConfirmaVotoNominal, (vota::ETelaVotacao)2>::GetProximoEstado` | …/eleitor/cconferevotoemcargo.h | cconferevotoemcargo.h | média |
| 11716 | 26 |  | `vota::CConfereVotoEmCargo<vota::CCandidatoInexistente, (vota::ETelaVotacao)12>::vf16` | `vota::CConfereVotoEmCargo<vota::CCandidatoInexistente, (vota::ETelaVotacao)12>::GetProximoEstado` | …/eleitor/cconferevotoemcargo.h | cconferevotoemcargo.h | média |
| 11717 | 26 | ✓ | `vota::CConfereVotoEmCargo<vota::CConfirmaVotoLegenda, (vota::ETelaVotacao)10>::vf16` | `vota::CConfereVotoEmCargo<vota::CConfirmaVotoLegenda, (vota::ETelaVotacao)10>::GetProximoEstado` | …/eleitor/cconferevotoemcargo.h | cconferevotoemcargo.h | média |
| 11738 | 26 | ✓ | `vota::CConfereVotoEmCargo<vota::CProporcionalNulo, (vota::ETelaVotacao)7>::vf16` | `vota::CConfereVotoEmCargo<vota::CProporcionalNulo, (vota::ETelaVotacao)7>::GetProximoEstado` | …/eleitor/cconferevotoemcargo.h | cconferevotoemcargo.h | média |
| 11739 | 26 |  | `vota::CConfereVotoEmCargo<vota::CCandidatoInapto, (vota::ETelaVotacao)14>::vf16` | `vota::CConfereVotoEmCargo<vota::CCandidatoInapto, (vota::ETelaVotacao)14>::GetProximoEstado` | …/eleitor/cconferevotoemcargo.h | cconferevotoemcargo.h | média |
| 11751 | 5 |  | `vota::CConfirmaProporcional::vf16` | `vota::CConfirmaProporcional::GetEstadoCorrige` | …/eleitor/cconfirmavotoemcargo.cpp | cconfirmavotoemcargo.cpp | média |
| 11752 | 25 | ✓ | `vota_f11752` | `vota::CConfirmaProporcional::CConfirmaProporcional` | …/eleitor/cconfirmavotoemcargo.cpp | cconfirmavotoemcargo.cpp | alta |
| 11755 | 267 | ✓ | `vota::CConfirmaVotoEmCargo::vf10` | `vota::CConfirmaVotoEmCargo::StartStateAudio` | …/eleitor/cconfirmavotoemcargo.cpp | cconfirmavotoemcargo.cpp | alta |
| 11791 | 71 | ✓ | `vota::IConfereVotoEmCargo::ProcessInputAudio` | `vota::IConfereVotoEmCargo::ProcessInputAudio` | …/eleitor/cconferevotoemcargo.cpp | cconferevotoemcargo.cpp | alta |
| 11793 | 333 | ✓ | `vota::(anonymous namespace)::GetCargoID` | `vota::IConfereVotoEmCargo::StartStateAudio` | …/eleitor/cconferevotoemcargo.cpp | cconferevotoemcargo.cpp | alta |
| 11858 | 562 |  | `vota::CDefineRotaPosReinicio::NeedChangeState` | `vota::CDefineRotaPosReinicio::NeedChangeState` | …/eleitor/cdefinerotaposreinicio.cpp | cdefinerotaposreinicio.cpp | alta |
| 11939 | 5 |  | `vota::CGeraResumoZeresima::vf10` | `vota::CGeraResumoZeresima::GetProximoEstado` | …/eleitor/iniciovotacao/cgeradorresumozeresima.cpp | cajusteinicial.cpp (comentário) | baixa |
| 12061 | 441 |  | `vota::CEstadoComDesligamentoAutomatico::ProcessTick(uebyte)::(lambda)::operator()` | `vota::CEstadoComDesligamentoAutomatico::ProcessTick` | …/eleitor/cestadocomdesligamentoautomatico.cpp | cestadocomdesligamentoautomatico.cpp | alta |
| 13564 | 211 |  | `vota::impl::CPoliticaExecucaoEleitor::LimpaBufferInput` | `vota::impl::CPoliticaExecucaoEleitor::LimpaBufferInput` | …/eleitor/comum/cpoliticaexecucaoeleitor.cpp | comum/cpoliticaexecucaoeleitor.cpp | alta |
