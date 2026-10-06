# u09 — `uenux2/src/app/vota/eleitor`: arquivos de resultado, impressão do BU e QR codes, zerésima, visualizador de candidatos

A unidade u09 cobre **80 funções wasm** da aplicação VOTA (o programa da urna para o dia da eleição) dos
dois subdiretórios da máquina de estados do terminal do eleitor que emolduram o voto:

| diretório | o que faz |
|---|---|
| `eleitor/iniciovotacao/` (*início da votação*, início do dia) | constrói o banco de dados dinâmico dos eleitores, verifica a data da eleição, gera e imprime a **zerésima** (o relatório que comprova que a urna contém zero votos antes da abertura da votação) e o seu resumo, reimprime-a após um reinício, mostra o aviso de desligamento por bateria e oferece um visualizador de candidatos (`auxiliares/`) |
| `eleitor/fimvotacao/` (*fim da votação*, fim do dia) | imprime as cópias do **Boletim de Urna (BU)**, **grava e assina os arquivos oficiais de resultado** (`CGravaResultado`), imprime os outros relatórios (BUJ, BIM, BEHB), pede a retirada da mídia de resultado e mostra o BU como QR codes na tela |

Arquivos originais (todos atestados por registros `std::source_location`, exceto onde indicado):

```
fimvotacao/  cgravaresultado.cpp  cimprimindobu.cpp  cquerimprimirbu.cpp  cimprimirbuoutrasobrigatorias.cpp
             cimprimirbjust.cpp   cimprimindobim.cpp cimprimindobehb.cpp  cretirarmr.cpp
             cverificaqtdbusadicionais.cpp  climitecopiasbuatingido.cpp
             cmostraqrcodebu.cpp  cmostraqrcodecertificado.cpp
iniciovotacao/ cgeradadosdinamicos.cpp  cverificaeleicaopassou.cpp  cexibealertadesligamento.cpp
             cgeradorresumozeresima.cpp  cimprimindozeresima.cpp  creimprimindozeresima.cpp
             creimprimindoresumozeresima.cpp  cquerreimprimirzeresima.cpp  cinformacaozeresimatardia.cpp
iniciovotacao/auxiliares/ cvisualizarcandidatos.cpp  cmenuvisualizarcandidatos.cpp
             cmenufiltrarcandidatosporcargo.cpp  cmenufiltrarcandidatosporpartido.cpp
```

A unidade também recebeu funções de outros arquivos (os seus chamadores estão aqui): `CTelasVota::CriaTelaVisualizacaoCandidato`
(ctelasvota.cpp), três mensagens de `CLogVota` (clogvota.cpp), `CRelVotaUtil::CriaCabecalho`, os singletons de
`CAplicacaoEncerrada` e `CDefineRotaPosReinicio`, `api::CWait::Aguarda`, helpers de construção de relatórios de `api`/`comum`,
construtores de classes de `comum/gravadores` e algumas instâncias de `__tree` da libc++. A §12 mapeia todas as 80.

**Fontes reconstruídas** (`// wasm func N` em cada função):

* `src/uenux2/src/app/vota/eleitor/fimvotacao/`: `cgravaresultado.{h,cpp}`, `cimprimindobu.{h,cpp}`,
  `cquerimprimirbu.{h,cpp}`, `cimprimirbuoutrasobrigatorias.{h,cpp}`, `cimprimirbjust.{h,cpp}`, `cimprimindobim.{h,cpp}`,
  `cimprimindobehb.{h,cpp}`, `cretirarmr.{h,cpp}`, `cverificaqtdbusadicionais.{h,cpp}`, `climitecopiasbuatingido.{h,cpp}`,
  `cmostraqrcodebu.{h,cpp}`, `cmostraqrcodecertificado.{h,cpp}`, `caplicacaoencerrada.u09.cpp` (fragmento)
* `src/uenux2/src/app/vota/eleitor/iniciovotacao/`: `cgeradadosdinamicos.cpp`, `cverificaeleicaopassou.cpp`,
  `cexibealertadesligamento.{h,cpp}`, `cgeradorresumozeresima.cpp`, `cimprimindozeresima.cpp`, `creimprimindozeresima.cpp`,
  `creimprimindoresumozeresima.{h,cpp}`, `cquerreimprimirzeresima.{h,cpp}`, `cinformacaozeresimatardia.cpp`,
  `auxiliares/{cvisualizarcandidatos,cmenuvisualizarcandidatos,cmenufiltrarcandidatosporcargo,cmenufiltrarcandidatosporpartido}.{h,cpp}`
* fragmentos de arquivos pertencentes a outras unidades: `vota/eleitor/comum/ctelasvota.u09.cpp`, `vota/log/clogvota.u09.cpp`,
  `vota/comum/crelvotautil.u09.cpp`, `vota/eleitor/cdefinerotaposreinicio.u09.cpp`, `src/uenux2/src/api/util/cwait.u09.cpp`

**Runtime.** Apenas a func 1249 (o construtor de cópia de `CDadoCorrespondencia`) executou nos votos gravados, a partir de
`votaInit`. Todo o resto é código morto no simulador: a página web inicia a thread do eleitor em
`CAguardaMensagem` e só conduz sessões de eleitor (unidade u06 §10), de modo que nem a cadeia de início do dia nem a de
fim do dia são alcançadas.

---

## 1. Glossário

| termo | significado |
|---|---|
| BU, *Boletim de Urna* | o resultado por seção: um relatório impresso (várias cópias, *vias*) e um arquivo ASN.1 `…-bu.dat` |
| *via*, *vias obrigatórias / adicionais* | cópia impressa; a configuração fixa quantas são obrigatórias e quantas extras podem ser solicitadas |
| zerésima | relatório impresso antes da abertura da votação: todos os candidatos com zero votos mais um "extrato do RDV" |
| RDV, *Registro Digital do Voto* | o arquivo com o registro (embaralhado) de cada voto |
| BUJ / BIM / BEHB | Boletim de Justificativa, Boletim de Identificação de Mesários, Boletim de Eleitores Habilitados Biograficamente |
| MI / MV / MR | *memória interna* (flash interna, `/dsk/fi`), *memória de votação* (cartão de memória, `/dsk/fe`), *mídia de resultado* (pendrive USB de resultado, `/dsk/mr`) |
| SAVD | o serviço de assinatura/validação da urna (`comum::IInterfaceSavd`); HSM / MSE / MSD = o módulo de segurança (MSE nas urnas ≥ UE2020, MSD antes, segundo os textos de log) |
| *encerramento* | encerramento da votação; `EstadoGeralVota.estadoEncerramento` |
| *treinamento do eleitor* | urna de treinamento do eleitor (fase `'3'` + flag `EstadoGeralVota.treinamentoEleitor`): nenhum resultado oficial é gravado |
| *modo demonstração* | modo de demonstração (`IInterfaceInit::GetDemoMode()`): uma cópia de tudo, sem MR |
| *contingência* | urna reserva não vinculada a uma seção (seção 0): identificada apenas por município/zona |

`EEstadoVota` = ASN.1 `EstadoGeralVota.estadoVota` + `'1'` (49); `EEstadoEncerramento` da mesma forma
(`src/asn1/ModuloEstadoGeralVota.asn`). Os nomes dos enumeradores C++ vêm dos textos dos asserts
(`EAVINICIAL`, `EAVGERADADOSDINAMICOS`, `EAVIMPRIMIRBU`, `EAVGRAVARRESULTADOS`, `EAVENCERRADA`,
`EAEIMPRIMIROBRIGATORIABU`, `EAERETIRAMR`, `EAEFIMDOSTRABALHOS`, …).

## 2. Classes e hierarquia (RTTI)

```
api::CState
└─ comum::CAppState                       (+4 m_proximoEstado, +8..+10 message/key/tick flags; slots
   │                                        [2] StartState [3] NeedChangeState [4] GetNextState
   │                                        [5] FinishState [6] ProcessMessage [7] ProcessInput [8] ProcessTick)
   ├─ fimvotacao: CGravaResultado (12 B), CImprimindoBU (20), CQuerImprimirBU (20),
   │              CImprimirBUOutrasObrigatorias (12), CImprimirBJust (12), CImprimindoBim (12),
   │              CImprimindoBEHB (12), CRetirarMR (36), CVerificaQtdBUsAdicionais (12),
   │              CLimiteCopiasBUAtingido (20)
   ├─ vota::CEstadoComDesligamentoAutomatico (u06)
   │    ├─ CMostraQRCodeBU (36), CMostraQRCodeCertificado (36), CAplicacaoEncerrada (36)
   ├─ iniciovotacao: CGeraDadosDinamicos, CVerificaEleicaoPassou, CExibeAlertaDesligamento (40),
   │              CImprimindoZeresima, CReimprimindoZeresima, CReimprimindoResumoZeresima,
   │              CQuerReimprimirZeresima (20), CInformacaoZeresimaTardia (48)
   │              CGeraZeresimaBase (20) ─ CGeraZeresima, CRegerarZeresima          [9] hook [10] GetEstadoResumo
   │              CGeraResumoZeresimaBase ─ CGeraResumoZeresima, CRegeraResumoZeresima [10] GetProximoEstado
   └─ auxiliares: CVisualizarCandidatos (40), CMenuVisualizarCandidatos, CMenuFiltrarCandidatosPorCargo,
                  CMenuFiltrarCandidatosPorPartido (all 12 B except the viewer)
vota::IQRCodeBUDS            poly-singleton holding the BU QR payloads + current part (cmostraqrcodebu.cpp:38)
vota::CAssinadorVota : comum::CAssinador      signer of the result files (only a virtual dtor)
vota::CProgressoEncerramento : comum::CAbstractTelaProgresso   progress screen of CGravaResultado
comum::IResultado ─ comum::IGravador ─ CGravadorBU, CGravadorRDV, CGravadorRCSecao, CGravadorHashes,
                                       CGravadorWSQ, CGravadorVersoesArquivos, CGravadorLog,
                                       IGravadorEnvelope ─ CGravadorEnvelopeArquivo        (unit u23)
comum::CGravacaoResultados   (non-polymorphic, name inferred: ctor func 5824, Executa() inlined)
```

Todo estado é um singleton criado de forma preguiçosa; vários acessores estão inlinados no chamador, outros passam
pelo corpo mesclado `vota_f764(mutex, &instance, vtable, flags)`. Os slots de destrutor 0/1 dos
estados que possuem uma tela são corpos de ICF compartilhados por todas as classes com o mesmo layout (244/387, 785/1560,
1284/2884); os donos de corpos dedicados são 2871/11877, 2879/11984, 2883/12019.

## 3. Início do dia (`iniciovotacao`)

### 3.1 Fluxo

```
CGeraDadosDinamicos (11865)
   GeraDadosDinamicos (6737, u04/u05: uenux.db voter table unless voter training, empty rdv.dat)
   CEleitores::CompleteLoad (6734)
   then (only after the generation) assert estadoVota ∈ {inicial, gerabasedinamica} (:47)
   estadoVota = aguardahorazeresima (51); SalvaEstado
   -> CVerificaHorarioZeresima (u26)   waits for the configured zerésima time
CVerificaEleicaoPassou (11914)   (entered after a restart for estadoVota 49..51)
   fase != treinamento and now >= CConfiguracaoEleicao +580 (election date/limit):
       log(2) "Data da eleição inválida", show CTelasVota +76, post message 0 (prio 100) to the
       OPERATOR thread's queue (func 270 +36; u27: its Run loop shows "URNA ELETRÔNICA INOPERANTE" /
       "Siga as instruções na tela do eleitor" and stops that thread), IBeep slot 3,
       STAY here forever (no input/tick/message handler: slots 5..8 are the CAppState no-ops)
   estadoVota ∉ 49..51 -> LogaErro "Erro estado do aplicativo não esperado", throw 9376 "Estado nao esperado" (:70)
   -> testeteclado::CPreZeresima (keypad test before the zerésima, u26)
CGeraZeresima / CRegerarZeresima : CGeraZeresimaBase::StartState (11946)
   builds the report (§6) and writes trab/ze.dat, logs "Gerando relatório [ZERESIMA] [INÍCIO]/[TÉRMINO]",
   CSincronizaVota::SincronizaRelatorios("ze.dat"); slot 9 hook (CRegerarZeresima: cut paper);
   -> slot 10: CGeraResumoZeresima (or CRegeraResumoZeresima)  -> trab/rze.dat (func 11943, u20)
   (CGeraResumoZeresima's slot 9 hook, func 11940, sets estadoVota 53 and saves; CRegeraResumoZeresima's is a no-op)
CGeraResumoZeresima::GetProximoEstado (11939) -> CImprimindoZeresima (11991)
   "Por favor, aguarde…", cut paper; prints ze.dat then rze.dat, N copies each (N = CConfiguracaoEleicao +96),
   stops silently if the urna is shutting down (flag @1832936); estadoVota = zeresimaimpressa (54);
   -> CDefineRotaPreVotacao
CRegeraResumoZeresima::GetProximoEstado (11845) -> CReimprimindoZeresima
```

Após um reinício antes da votação, `CReinicioVotacao` (u07) vai para **CQuerReimprimirZeresima** (11849/11848):
recusado com `LogaErro` + 9375 "Erro na verificação do comparecimento" se `CRdvVota::Comparecimento() != 0`
(ninguém pode ter votado ainda); registra no log "Mesário indagado se quer imprimir a zerésima"; então
`'1'`+CONFIRMA → corta o papel → **CReimprimindoZeresima** (ze.dat, depois o resumo);
`'2'`+CONFIRMA → corta o papel → **CReimprimindoResumoZeresima** (rze.dat, estadoVota 54, → CDefineRotaPosReinicio);
outro texto → campo limpo; CORRIGE em um campo vazio → `CReinicioComparecimentoMesario` quando os mesários
precisam ser registrados (`IdentificaMesarios() && !EhTreinamentoEleitor()`, func 2520), senão `CInicioVotacao`;
BRANCO → `CMaisInformacoes`.

Todo laço de impressão é o mesmo (inlinado três vezes):
`MarcaImpressaoEmAndamento()` (func 1488: cria `<root>dinamico/imprimindo`), log
`Imprimindo relatório [<nome>] via nº [<n>]`, `CSigVerifier(trab, "<x>.dat", "<x>.vsu")`,
`IPaperRelatorios::ImprimeArquivo(trab/<x>.dat, verificador, "zerésima", "via n de N")` (slot 7),
`RemoveMarcaImpressaoEmAndamento()` (func 1487); após o laço, slot 10 (espera pela impressora).
O marcador `imprimindo` permite que uma urna reiniciada saiba que uma impressão foi interrompida.

### 3.2 Aviso de bateria — `CExibeAlertaDesligamento` (12018/12017)

Entra-se nele a partir de qualquer `CEstadoComDesligamentoAutomatico` (u06) quando a urna funcionou na bateria por
`limite − aviso` segundos. `StartState` mostra `CTelasVota +44`, copia `g_dataHoraDesligamento`
(@1833312) e emite um bipe por `IBeep` slot 1 (2000 Hz, 100 × 10 ms). A cada tick de 1 s: se o prazo passou →
log (nível 2) "Tempo limite de espera usando bateria interna atingido" e `IInterfaceInit::DesligarUrna()`;
senão, se `IPower` não reporta mais a bateria interna (`status & 6 != 2`) → volta ao estado anterior (+36).

### 3.3 Zerésima tardia — `CInformacaoZeresimaTardia` (11983/11982)

Alcançado quando o mesário diz que o relógio da urna está errado (`CImpressaoZeresimaTardia`, func 11931). Em CONFIRMA:
log "Finalização de aplicativo" e "Desligando a urna", `IBeep` slot 4, `CWait::Aguarda()` (prazo =
criação do singleton + 1 s, a func 5446 faz espera ativa com `usleep(200)`), `DesligarUrna()`,
`m_proximoEstado = nullptr`.

### 3.4 Visualizador de candidatos (`auxiliares/`)

`CMaisInformacoes` → **CMenuVisualizarCandidatos** (11881): registra no log "Opção de visualização de candidatos
selecionada" e mostra um menu cujos itens carregam um `std::any` contendo `std::pair<CAppState*, CAppState*>`:

| item | cadeia |
|---|---|
| Por cargo | CMenuFiltrarCandidatosPorCargo → CMenuFiltrarCandidatosPorPartido → CVisualizarCandidatos |
| Por partido | CMenuFiltrarCandidatosPorPartido → CMenuFiltrarCandidatosPorCargo → CVisualizarCandidatos |
| Candidato específico | CMenuFiltrarCandidatosPorCargo → CMenuFiltrarCandidatosPorNumero → CVisualizarCandidatos |

A seleção define `first.m_proximoEstado = second`, `second.m_proximoEstado = viewer` e limpa os três
filtros do visualizador (`std::optional` cargo +24, número +28, partido +36). Cada menu de filtro coleta, a partir
das candidaturas que passam pelos filtros já definidos, o conjunto de cargos (ordenados por id, func 5655) ou de números
de partido, oferece "Todos" (valor 0; o menu de cargo o esconde quando o próximo passo é a entrada do número) e
armazena a escolha. O menu de partido define o formato de todo o menu como `"%S"` (apenas o texto de busca) e dá apenas
ao item "Todos" o seu próprio `"%S - %T"`: os partidos são listados **apenas pelo número** ("0 - Todos", "12",
"13", …), sem sigla nem nome. Os menus leem o `CInputMenuField` diretamente
(`menu->Read(IInputKbd&)`, srcloc do próprio arquivo do menu), não por meio do formulário interativo. **CVisualizarCandidatos** (11876) filtra as candidaturas (9378 "Nenhuma candidatura a ser
exibida"), depois entra em laço *dentro de StartState*: mostra `CTelasVota::CriaTelaVisualizacaoCandidato(c, i, n)` (6569:
"VISUALIZAÇÃO", cargo, Partido, Número `{:0{digits}}`, Nome, Gênero, foto ou "Candidato não concorre." /
"Não há outras informações.", Situação APTO/INAPTO, Página i/n, teclas 4/6 com volta circular, CORRIGE =
Retornar), `WaitAndRead()`, e reconstrói a tela para o candidato anterior/seguinte.

## 4. Fim do dia (`fimvotacao`)

```
CGeraBU (u08)  -> trab/bu.dat (printed-BU image) + signature trab/bu.vsu      estadoVota 59 -> 60
CGeraRelatorios (u08) -> trab/buj.dat, bim.dat, behb.dat                        60 -> 61
CInicioBU (12062, other unit): voter training ? CQuerImprimirBU : CImprimindoBU
CQuerImprimirBU (12035/12034)   asserts 61 and qtdBU == 0 (texts say "poInfo")
    CORRIGE -> estadoVota 64, SalvaEstado -> CAplicacaoEncerrada      CONFIRMA -> CImprimindoBU
CImprimindoBU (12078/12077)     asserts 61 (:70) and qtdBU == 0 (:71); prints via 1; asks about quality
    CORRIGE (reprint): log, print via 1 again           CONFIRMA: log, qtdBU++;
       voter training -> estadoVota 64, encerramento 52 -> CEmitirMaisBU
       otherwise      -> estadoVota 62 (gravarresultados) -> CGravaResultado
CGravaResultado (12098)         writes + signs the result files (§5.2)      62 -> 63
CCopiaResultadoParaMR (u08)     copies them to /dsk/mr and verifies the BU  63 -> 64, encerramento 50
CImprimirBUOutrasObrigatorias (12065)  remaining mandatory vias (screen "telaDestinoBUs"), then
    ImprimeBoletimJustificativa ? CImprimirBJust : IdentificaMesarios ? CImprimindoBim : CRetirarMR
CImprimirBJust (12068)   buj.dat, then IdentificaMesarios ? CImprimindoBim : CRetirarMR
CImprimindoBim (12071)   bim.dat, then (!demo && biometric urna) ? CImprimindoBEHB : CRetirarMR
CImprimindoBEHB (12074)  behb.dat, then CRetirarMR      (only reachable through CImprimindoBim!)
CRetirarMR (12031/12030)        encerramento 51 (saved), waits until the MR is removed (250 ms polling),
                                encerramento 52 (saved); demo mode: 52 without waiting (not saved)
    CONFIRMA -> CEmitirMaisBU (u08)  -> extra vias -> CMostraQRCodeBU
CVerificaQtdBUsAdicionais (12023) (restart at encerramento 52): qtdBU >= obrigatórias + adicionais ?
    CLimiteCopiasBUAtingido ("Quantidade de vias adicionais excede o máximo permitido"; CONFIRMA ->
    CMostraQRCodeBU) : CEmitirMaisBU
CMostraQRCodeBU (12055/12051/5982) <-> CMostraQRCodeCertificado (12040/12038) -> CAplicacaoEncerrada
```

## 5. BOLETIM DE URNA: o que esta unidade faz, passo a passo

O BU existe em duas formas: o **relatório impresso** (arquivo de imagem `trab/bu.dat`, feito por `CGeraBU`, unidade u08)
e o **arquivo de resultado ASN.1** `…-bu.dat` (`EntidadeBoletimUrna`, gravado aqui por `CGravadorBU`). Esta unidade
imprime o primeiro, grava/assina o segundo junto com todos os outros arquivos de resultado e mostra os QR codes do BU
na tela.

### 5.1 Imprimindo uma cópia — `CImprimindoBU::ImprimeBU(uebyte numVia, PrintMessageMode modo)` (func 2890)

1. `auto& papel = api::IPaperRelatorios::GetInst()` (:111). `Assert (estVota.GetQtdBU() == numVia - 1)` (:114,
   3467): as cópias são impressas estritamente em ordem; `qtdBU` (`EstadoGeralVota.qtdBU`, +8, persistido em `vota.bin`)
   é incrementado pelo chamador somente depois que uma cópia foi aceita.
2. Formulário de cabeçalho: texto `"<n>a. VIA"` (fonte 2, centralizado) + 3 linhas em branco.
3. `MarcaImpressaoEmAndamento()` (cria `dinamico/imprimindo`).
4. `CSigVerifier(trab, "bu.dat", "bu.vsu")`: o serviço de impressão só imprime a imagem se a sua assinatura
   for verificada (a imagem e a assinatura foram produzidas por `CGeraBU`).
5. `modo == ComMensagem` (0): mensagem `"boletim de urna"` e `"<n>ª via"`; `SemMensagem` (1, cópias obrigatórias
   impressas por `CImprimirBUOutrasObrigatorias`): textos vazios.
6. `papel.ImprimeArquivo(trab/bu.dat, verificador, header, mensagem, via)` (IPaperRelatorios slot 8), slot 10,
   `RemoveMarcaImpressaoEmAndamento()`.

Chamadores e numeração:

| quem | via impressa | depois |
|---|---|---|
| `CImprimindoBU::StartState` / reimpressão em CORRIGE | 1 (repetida até o mesário confirmar a qualidade) | CONFIRMA → `qtdBU = 1` |
| `CImprimirBUOutrasObrigatorias` | `qtdBU+1` … `qtdBuObrigatorio` (1 em modo de demonstração, senão parâmetro +12 de `CConfiguracaoEleicao +88`) | cada cópia → `qtdBU++`, `SalvaEstado()` |
| `CEmitirMaisBU` (u08) | `qtdBU+1` …, no máximo `GetQuantidadeMaximaBUsAdicionais()` | cada uma → `qtdBU++`, salva |
| `CVerificaQtdBUsAdicionais` | — | recusa mais quando `qtdBU >= obrigatórias + adicionais` (parâmetros +12 e +16; 1 e 1 em modo de demonstração) |

Linhas de log (`logd.dat`): `Imprimindo relatório [BU] via nº [<n>]`, `Mesário indagado sobre qualidade do Boletim
de Urna`, `Mesário indicou reimprimir Boletim de Urna`, `Mesário indicou qualidade OK para Boletim de Urna`.
Os outros relatórios usam o mesmo mecanismo com uma cópia ("via única"): `buj.dat/buj.vsu` ("boletim de
justificativa eleitoral", ERelatoriosUE 4), `bim.dat/bim.vsu` ("boletim de mesários", 9), `behb.dat/behb.vsu`
("boletim de eleitores\nhabilitados biograficamente", 12), IPaperRelatorios slot 7 (sem cabeçalho).

### 5.2 Gravando e assinando os arquivos de resultado — `CGravaResultado::StartState` (func 12098)

Pré-condições (cada falha lança uma exceção, o encerramento para e a tela de erro do
`CApplicationContextGuard` ao redor é mostrada):

| verificação | erro |
|---|---|
| `estadoVota == EAVGRAVARRESULTADOS` | assert 3456 (cgravaresultado.cpp:63) |
| `dhIniAquisicao` / `dhFimAquisicao` definidos | 8090 "Início da aquisição não marcado" / 8091 "Fim da aquisição não marcado" |
| `CEleitores::GetNumComparecimentos() == CRdvVota::Comparecimento()` (eleitores marcados como tendo votado = cédulas no RDV) | assert 3457 (:79) |
| integridade referencial RDV / cargos / respostas | `comum::VerificaIntegridadeReferencial()` (func 2543) |
| `dhEmissao` (definido por CGeraBU) | 8093 "A data/hora da emissão do BU não foi registrada" |
| fase ∈ {`o`,`s`,`t`} | 8657 "Fase inválida: {}" (IResultado) / 8698 "Fase inválida: {:#x}" (ConverteFaseEcourna) |
| `etc/dependencias.properties` e `etc/versoes.properties` têm `tag=20260601173148` | 8662 / 8696 "Tag do arquivo de … contratos inválida: [{}]. Tag atual: [{}]" |

Dados comuns: `fase` = caractere da fase da carga de `EstadoGeralUrna`, município / zona / local / seção e UF de
`CLocal`, `agora` = data/hora atual (a *data de geração* armazenada em cada gravador), o
`CDadoCorrespondencia` de `EstadoGeralUrna +60` (correspondência: numeroInternoUrna, código da carga…).

**Gravadores** (`std::vector<std::shared_ptr<comum::IGravador>>`, nesta ordem). Cada um é um `comum::IResultado`
(`+4 município, +8 zona, +12 local, +16 seção, +18 fase, +20 nome, +32 extensão, +36 arquivo SAVD`); o nome do
arquivo é `"<fase><pleito:05><UF><município:05><zona:04><seção:04>-<sufixo>"`
(`CGravadorUtil::DeterminaNomeArquivoSemLetra`):

| # | gravador (unidade u23) | extensão → sufixo | id SAVD | conteúdo / argumentos do construtor vistos aqui |
|---|---|---|---|---|
| 1 | `CGravadorBU` (304 B) | 4 → `bu.dat` | 35 | **ASN.1 `EntidadeBoletimUrna`**: dhGeração, **dhEmissão**, `ConverteFase(fase)`, correspondência, histórico de cargas (`GetGap().GetCodigosCarga()`), eleitores aptos (mapa `CEleitores::GetQtdAptos()`), `CRdvVota&` (os votos), comparecimento, urna biométrica, habilitações {sem biometria, biométrica, biográfica} (cada uma verificada em 0..9999), dhInício/dhFim da aquisição, a ordem dos cargos (`map<TCargoID, n>`: primeira posição de cada cargo em `CCargos`) e o nome do arquivo de chave **`bu.pk1`** |
| 2 | `CGravadorRDV` (168 B) | 6 → `rdv.dat` | 37 | o RDV (`CRdvVota&`), dhGeração, correspondência |
| 3 | `CGravadorRCSecao` (60 B) | 8 → `jufa.dat` | 39 | `ModuloResultadoUrnaCadastro` (justificativas/faltosos); fase como valor do ecourna: `o`→2, `s`→1, `t`→3 |
| 4 | `CGravadorEnvelopeArquivo` | 9 → `imgbu.dat` | 43 | `EntidadeEnvelopeGenerico` envolvendo **`trab/bu.dat`** (a imagem do BU impresso), `tipoEnvelope = envelopeBoletimUrnaImpresso` (C++ `CEnvelopeGenerico::Tipo` 2) |
| 5 | `CGravadorEnvelopeArquivo` | 11 → `imgze.dat` | 44 | envelope envolvendo `trab/ze.dat` (zerésima impressa), `envelopeZeresimaImpressa` (C++ 4) |
| 6 | `CGravadorHashes` (96 B) | 12 → `hash.dat` | 46 | `ModuloHashes::EntidadeHashes`: UF, fase, versão **"10.23.0.1 - DESENVOLVIMENTO"**, identificação = `CIdentificacaoSecao(município, zona, local **= 1**, seção)` ou, para a seção 0, `CIdentificacaoUrnaContingencia(município, zona)` |
| 7-9 | `CGravadorWSQ` ×3 (apenas urnas biométricas) | 16/17/18 → `wsqbio.jez`, `wsqman.jez`, `wsqmes.jez` | 48/49/50 | imagens de impressões digitais, tipo 0/1/2, último argumento `!EhFaseTreinamento()` (criptografia, inferido) |
| 10 | `CGravadorVersoesArquivos` (64 B) | 19 → `mr.ver` | 70 | `ModuloVersaoArquivos`: tag `20260601173148` + mapa módulo → versão (abaixo) |
| 11 | `CGravadorLog` (76 B) | 13 → `log.jez` | 60 | o log: `dinamico/log/arquivados/`, `dinamico/log/logd.dat`, diretório trab |

**Versões dos contratos ASN.1 (mr.ver).** Os módulos ASN.1 usados pelos gravadores são coletados
(`ModuloBoletimUrna`, `ModuloEnvelopeGenerico`, `ModuloResultadoUrnaCadastro`, `ModuloHashes`,
`ModuloAssinaturaEcourna`, `ModuloVersaoArquivos`), expandidos com as suas dependências transitivas de
`etc/dependencias.properties` (`Modulo=Dep1,Dep2`, func 5869), e cada módulo recebe a sua versão de
`etc/versoes.properties`. Ambos os arquivos são lidos uma vez para o poly-singleton `CGeracaoVersoesContratos`, que
exige `tag == "20260601173148"` em cada um (a tag é compilada no binário).

**Assinador.** `vota::CAssinadorVota(pacote)`, com pacote (ESavdPacote) = 158 no 1º turno e 159 no
2º. O seu construtor `comum::CAssinador` inlinado define:

* `+4` aplicação (ESavdAplic) = 1 (presumivelmente VOTA), `+8` pacote;
* `+36` local = `std::format("{}{:05}{:04}{:04}", UF, município, zona, seção)` (deve ter 15 caracteres);
* o arquivo do pacote = `std::vformat(CArquivosSavd[pacote], fase, pleito (cfg +28), UF, município, zona, seção)`
  = `<result dir MI of the turno>/<fase><pleito:05><UF><município:05><zona:04><seção:04>-vota.vsc`, dividido
  em `+24` diretório (`Diretorio()` + "/", func 5462) e `+12` nome do arquivo.

**Execução** (`comum::CGravacaoResultados::Executa`, inlinado; cada bloco roda dentro de um
`CApplicationContextGuard` cujos textos são a tela de erro; a tela de progresso
`telaProgressoEncerramento` — "Preparando dados para encerramento", "O processo pode levar alguns minutos",
uma barra de 30 passos — avança após cada arquivo):

1. **Gerar no diretório de trabalho** (`CApplicationContextGuard(2, "", "Gerando os resultados na MI", …)`):
   para cada gravador: log `LogaGerandoResultados(ext, INÍCIO)`, `IGravador::Grava()` (slot 4: abre
   `GetPathTrab(MI)/<nome>` com o modo `"wb"` e chama o slot 7 do gravador, `GravaResultado(CFile&)`), log TÉRMINO.
2. **Copiar para o diretório de resultados da MI** ("Copiando arquivos para o dir. de resultados"): slot 2
   `CopiaParaResultado()`; o id SAVD do arquivo é acrescentado à lista a assinar; log
   `LogaCopiandoArqResParaFI`. (Uma segunda lista de gravadores, sempre vazia, recebe o mesmo tratamento.)
3. **Assinar** ("Assinando resultados da MI"): `LogaPreparandoAssinaturaArquivosResultado`,
   `LogaInicioProcedimentoAssinatura`, depois `CAssinador::AssinaArquivosResultado(ids)` (cassinador.cpp:141-149):
   * lista vazia → 8636 "Não foi passado nenhum arquivo de resultado";
   * `EnviarAcaoHSM(savd, aplicação, ABRE)` (func 5889; mensagem "Falha ao enviar ação ao HSM. {}" em caso de erro);
   * log "Inicia uma sessão no MSE" se `api::IUrna::GetInst()` slot 0 (modelo) ≥ 2020, senão "…no MSD";
   * envia o "local" (0xFE + o local de 15 caracteres + 0x0404 + aplicação) — **o resultado não é verificado**;
   * para cada arquivo: `CApplicationContextGuard(11, "", "Erro na assinatura dos arquivos de resultado",
     "Ocorreu um erro durante a assinatura dos arquivos: <nome SAVD>")` e
     `AssinarEcourna(savd, aplicação, 128, id)` (iinterfacesavd.cpp:1022): requisição `{0xFE, '=', id, u16 128,
     u8 aplicação}`, falha → 7338 `"Falha ao assinar ({}-Ecourna[{}])"`;
   * `EnviarAcaoHSM(…, FECHA)`, "Finaliza a sessão no MSE/MSD", `LogaTerminoProcedimentoAssinatura`.
   As assinaturas se acumulam no pacote SAVD `…-vota.vsc` (pacote 158/159).
4. `CSynchronizer::CreateInst()->Sincroniza()` (fsync).
5. **Copiar para a MV**, a menos que a fase seja de treinamento (`m_copiaParaMV = !EhFaseTreinamento()`): o slot 3 de cada gravador,
   `CopiaParaMV()` (log `LogaResultadoCopiadoResFE`), depois o pacote de assinaturas `.vsc` é **copiado**
   (não reassinado) para `GetPathResult(MV)`; sync.
6. Barra de progresso até o fim; `estadoVota = EAVCOPIARESULTADOSMR (63)`, `SalvaEstado()`,
   em seguida `CCopiaResultadoParaMR` (u08), que copia `bu.dat, rdv.dat, jufa.dat, imgbu.dat, imgze.dat, hash.dat,
   log.jez, vota.vsc, mr.ver` (+ wsq) para `/dsk/mr/` e verifica a cópia do BU byte a byte.

### 5.3 QR codes do BU na tela — `CMostraQRCodeBU` / `CMostraQRCodeCertificado`

* Os payloads vêm de `IQRCodeBUDS` (`GetQRDSInst()`, cmostraqrcodebu.cpp:31/38, func 1956): um vetor de
  strings construído com `CGeradorBUQRCode` (os mesmos payloads "BU DIGITAL" impressos no BU; o
  construtor rejeita um vetor vazio com "Vetor de partes vazio") e o índice da parte mostrada.
* Modo de demonstração: sem tela de QR, direto para `CAplicacaoEncerrada`.
* Tela `telaQRCodeBU`: imagem QR de 380 px da parte atual, "Versão: <versão>", "BU digital", instrução
  "O QR code ao lado contém o resultado da votação para esta urna." (+ " Use as teclas 3 e 9 para navegar
  pelas partes do BU." quando há várias partes), "Pagina i/n", teclas `9` "para próxima parte", `3`
  "para parte anterior", CONFIRMA "Continuar", BRANCO "Ver certificado".
* `AjustaTela` (:195) habilita/desabilita os rótulos: uma parte → navegação escondida, CONFIRMA habilitado; primeira
  parte → voltar e CONFIRMA desabilitados; última parte → próxima desabilitada, CONFIRMA habilitado; meio → CONFIRMA desabilitado.
  **CONFIRMA só é aceito na última parte** (de modo que toda parte é mostrada antes de sair).
* BRANCO → `CMostraQRCodeCertificado`: QR com `QRCE:1:1 IDUE:… MDUE:… CERT:<hex>` (func 12039), "Certificado
  digital", "O QR code ao lado contém o certificado desta urna.", CORRIGE "Retornar".
* Ambos os estados são `CEstadoComDesligamentoAutomatico`: enquanto são mostrados, o watchdog da bateria continua rodando.

## 6. O relatório da zerésima (`cgeradorresumozeresima.cpp`)

Partes montadas por `CGeraZeresimaBase::StartState` (11946) em um `api::CReport` e gravadas com
`CScopedReportFile` (`IPaperRelatorios::Abre(trab/ze.dat)` … `Fecha()`):

1. cabeçalho `CRelVotaUtil::CriaCabecalho("Zerésima")` (5977): `IncluiCabecalhoEleicoesMZS` (município, zona, seção,
   nome), "Eleitores aptos {:04}" (+ originais/temporários), código de identificação da UE, "Data"/"Hora" da
   emissão, resumo da correspondência;
2. **extrato do RDV** (5971, inserido na posição 1): verifica por assert que não há justificativa (:43) nem
   voto em nenhum cargo (:47), imprime `======` / "Não há votos ou justificativas registrados" / `======` /
   "-----------EXTRATO DO RDV-------------" e um `CParteCargos(separador de eleição, CParteRdv(…))`;
3. "---------LISTA DE CANDIDATOS----------";
4. `CParteCargos(separador de eleição, CParteCandidatos(header do cargo, "Não há candidatos concorrendo",
   majoritários, proporcionais (per party, "Não há candidato registrado para <este/esta> <PLSB>"),
   consultas))`;
5. rodapé (5579): separador de fase, "Código de identificação da carga" (formatado), "Ver: 10.23.0.1",
   texto da UF (`CConfiguracaoEleicao +200`), 20 linhas em branco, corte do papel.

Os números impressos vêm de fontes de dados `CDataText<std::string (*)()>` avaliadas no momento da impressão. Os
"ids" que aparecem no código (1625, 1626, 1632, 1634, 1635, 3049, 3050, 3051, 1567…) são **slots da tabela de
funções** dessas fontes de dados (ex.: 1625 → `CDataSourcesRelatorio<CRdvVota,CEleitores>::TrailerProporcional`,
1634 → `CCargoDSLabelRelatorio::HeaderDetalheZE`), não ids de strings.

## 7. Dados lidos e gravados

| dado | acesso |
|---|---|
| `vota.bin` (`EstadoGeralVota`: estadoVota, estadoEncerramento, qtdBU, dhIni/Fim/Emissão, treinamentoEleitor) | lido / gravado + `SalvaEstado()` |
| `EstadoGeralUrna` (fase +48, turno +32, município/zona/seção, correspondência +60), `gap.bin` | lido |
| `CConfiguracaoEleicao`: +28 pleito, +88 parâmetros (vias obrigatórias +12, adicionais +16, BJust +394, mesários +400), +96 vias da zerésima, +200 texto com UF, +260 destino das vias, +580 data limite da eleição | lido |
| `trab/{bu,buj,bim,behb,ze,rze}.dat` + `.vsu` | impresso (com verificação de assinatura) |
| `trab/ze.dat` | gravado (imagem da zerésima) |
| `<root>dinamico/imprimindo` | criado/removido em torno de cada impressão |
| `etc/dependencias.properties`, `etc/versoes.properties` | lido (tag + versões dos módulos) |
| diretórios trab/result da MI e da MV: `…-bu.dat, rdv.dat, jufa.dat, imgbu.dat, imgze.dat, hash.dat, wsq*.jez, mr.ver, log.jez`, `…-vota.vsc` | gravado / copiado / assinado |
| `dinamico/log/logd.dat` | linhas de log (Latin-1) |
| SAVD / HSM / `api::IUrna` / `IInterfaceInit` (MR presente, modo de demonstração, desligamento) / `IPower` / `IBeep` | serviços |

## 8. Especificidades do build web

* Nenhum destes estados é alcançado (§ Runtime). Se fossem: a impressora é `simulador::CWasmNullPaper`
  (slots 7/10 sem efeito, o slot 8 só descarta o cabeçalho) — **nada é impresso e o `CSigVerifier` nunca é
  consultado**; `CWasmInit` reporta a MR como ausente (mensagem 18 → 1 → "não presente"); o SAVD é o
  `(anonymous)::CWasmSavd` web registrado por `main`; `IPower` nunca reporta a bateria.
* `CGravaResultado` não conseguiria concluir no simulador: `etc/dependencias.properties` e
  `etc/versoes.properties` são stubs de 0 bytes em `upstream/fs/vota_web_wasm/etc/` (a raiz de CPath @1838600 é
  `"/"`). O arquivo vazio é interpretado como um `api::CIniStrings` vazio, e a própria busca da tag,
  `CDependenciasContratos::GetValor("tag")` (func 3826 → 6044, cdependenciascontratos.cpp:59), lança
  **8663 "Propriedade inexistente: tag"** antes que a comparação de tag 8662 seja alcançada (leitura estática; o
  estado não é alcançável a partir da página).
* `emscripten_sleep` é importado, mas aborta (sem Asyncify); veja §10.

## 9. Observações sobre wasm / Emscripten

* **Inlining de LTO**: `CGravaResultado::StartState` (26,7 KB) é a única cópia dos construtores de
  `CGravadorBU/RDV/RCSecao/Hashes/VersoesArquivos/Log`, de `CAssinador::AssinaArquivosResultado`,
  `AssinarEcourna`, da criação de `CGeracaoVersoesContratos` e do construtor de `CProgressoEncerramento`.
  `CGeraZeresimaBase::StartState` (6,9 KB) é a única cópia do layout da zerésima.
* **Nomeados errado pelas ferramentas** (nome do analisador → real): 11991 `ImprimeZeresima` → `CImprimindoZeresima::StartState`;
  11852/11855 da mesma forma; 5971 `CriaTituloExtratoRDV` → `CriaExtratoRDV` externa; 11946/12098/12040 `vf2` →
  `StartState`; 12034/12026/12077/11848 `vf7` → `ProcessInput`; 3847/5914/1950/2286 "EhModoDemonstracao"
  → os getters de `CInformacaoEleicao` que a inlinam; 11556 "CCargos::GetInst" (slot 3049) → fonte de dados do
  nome da eleição atual; 3796 "ValidaTipoBiometria" → construtor de `CGravadorWSQ`.
* **Códigos de erro são números que o anotador pode imprimir como strings**: ex. 8090, 9375, 9408, 3476 são
  códigos de erro, não endereços ("Falha ao validar assinatura UE…", "turno2.jez", "In RTree…" no pseudocódigo são
  coincidências).
* **Latin-1**: todas as strings da aplicação são Latin-1 (`"zer\xE9sima"`, `"via \xFAnica"`, `"{}\xAA via"`).
* Os handlers `std::any` dos itens de menu são os slots de tabela 1107 (`pair<CAppState*,CAppState*>`), 1108
  (`uebyte`), 1109 (`uint16_t`).
* Os mutexes dos singletons sobrevivem apenas como resíduos de `mutex_unlock` (build single-threaded).

## 10. Código suspeito / notável

1. **Caminhos de aborto por `emscripten_sleep`** (flag @1584624 = 1): `CVisualizarCandidatos::StartState`
   (11876, `WaitAndRead` com 10 ms), e os três menus 11881/11884/11888 cujo `CInputMenuField::Read`
   (10894) **esvazia o teclado e depois dorme 5 ms esperando uma tecla**: no build web isso aborta
   imediatamente. Inalcançável a partir do terminal do eleitor que a página conduz. `CRetirarMR::AguardaRetiradaMR`
   (12031) também tem `emscripten_sleep(250)` no seu laço "MR ainda presente", mas ele **não** é um caminho de
   aborto no simulador: `CWasmInit` responde à mensagem 18 com 1 (`"= no"`), que `IsMRPresenteSemHabilitar`
   (2862 → 6046) transforma em "ausente", então o corpo do laço nunca roda.
2. **Laços modais dentro de `StartState`** (11876, 11881, 11884, 11888): o laço de eventos da thread não roda
   enquanto o operador navega pelos candidatos (sem ticks, sem mensagens). Na urna isso é proposital (a
   thread bloqueia no teclado); em um port cooperativo (single-thread) não pode funcionar.
3. **`EntidadeHashes` identifica a seção com local = 1** (12098): `CIdentificacaoSecao(município, zona,
   1, seção)`, embora `CLocal::GetLocalID()` seja lido e passado a todos os outros gravadores. Ou o campo é
   intencionalmente constante, ou `hash.dat` carrega um "local" errado.
4. **O resultado da requisição SAVD do "local" é ignorado** (12098, `AssinaArquivosResultado` inlinado): os erros
   "Argumento inválido para local." / "…tamanho do local." ou uma resposta de falha só vão parar na string de
   último erro do serviço; a assinatura prossegue.
5. **`CRetirarMR::StartState` persiste antes de validar**: grava `EAERETIRAMR` e chama `SalvaEstado()`
   *antes* de `Assert(estadoVota == EAVENCERRADA)`, e o segundo assert é tautológico. Em modo de demonstração o
   `EAEFIMDOSTRABALHOS` final não é salvo.
6. **A verificação de assinatura dos relatórios impressos é pulada no build web**: `CSigVerifier` (func 1540) só
   armazena o diretório e os dois nomes de arquivo; a verificação cabe ao serviço de impressão, e
   o slot 7 de `CWasmNullPaper` (8372) é vazio e o slot 8 (8371) só libera o cabeçalho, então o verificador
   nunca é consultado (nada é "simulado"); na urna o serviço de impressão verifica `*.vsu`.
7. **Strings de identidade fixas no código**: `hash.dat` registra a versão `"10.23.0.1 - DESENVOLVIMENTO"` e a
   tag de contrato `20260601173148` é compilada no binário; uma urna com outros `.properties` se recusa a encerrar a votação
   (8662/8696).
8. **Beco sem saída de `CVerificaEleicaoPassou`**: quando a data da eleição já passou, o estado nunca muda (sem
   handler de entrada): só desligar e religar a urna sai dele.
9. **A impressão do BEHB depende do relatório de mesários**: `CImprimindoBEHB` só é alcançado a partir de
   `CImprimindoBim`, ou seja, apenas quando `IdentificaMesarios()` está configurado. `CGeraRelatorios` (u08) gera
   `behb.dat` sempre que a urna é biométrica e não está em modo de demonstração, então, com a identificação de mesários desabilitada,
   o BEHB é gerado em `trab/` mas nunca impresso (também não é um dos arquivos copiados para a MR).
10. **Slicing**: `CGravacaoResultados` armazena um `comum::CAssinador` copiado de `vota::CAssinadorVota`; inofensivo
   hoje (a subclasse não adiciona comportamento virtual).
11. Cosmético: os textos dos asserts dizem `poInfo.GetVota()` (cquerimprimirbu.cpp:40/41).
12. **Desreferência nula nos menus de filtro** (11888, 11884): quando nenhum cargo / nenhum partido passa pelos filtros,
   a tela mostra "Nenhum cargo disponível!" / "Nenhum partido disponível!" e o `CInputMenuField`
   nunca é criado (o seu shared_ptr permanece nulo: locais inicializados com 0 no wasm), mas o laço de leitura
   logo após `Exibe()` chama `menu->Read(kbd)` (slot 10 da vtable) por meio dele. No wasm isso carrega um vptr
   do endereço 0 e faz `call_indirect` da entrada 0 da tabela (trap); nativamente é um segfault. Alcançável
   apenas com uma eleição que não tenha nenhuma candidatura, porque cada passo de filtro posterior lista valores tirados
   das candidaturas.
13. **O filtro de partido mostra apenas números** (11884): o formato de todo o menu é `"%S"` e só "Todos" tem
   `"%S - %T"`, então o operador escolhe um partido pelo número sem ver a sua sigla nem o seu nome.

## 11. Questões em aberto

* Onde `CGeraZeresimaBase::StartState` (11946) realmente fica. O grafo de chamadas não decide: o
  srcloc de namespace anônimo da func 5971 pertence a `CriaTituloExtratoRDV`, que está *inlinada* em 5971;
  a função externa 5971 pode ter linkage externo, e o LTO faz inlining entre unidades de tradução de qualquer forma.
  11946 contém dois frames inlinados aninhados (64 e 400 bytes) em torno da construção do relatório, o que se encaixa
  com um construtor de relatório de `cgeradorresumozeresima.cpp` inlinado em um StartState definido em
  `cgerazeresima.cpp` (a leitura da unidade u07). A reconstrução mantém 11946 em `cgeradorresumozeresima.cpp`
  apenas porque a definição da unidade o atribui ali.
* Semântica exata dos slots 7/8/10 de IPaperRelatorios e dos slots 3/4 de IBeep (nomes inferidos pelo uso).
* O nome da classe do objeto construído pela func 5824 (`CGravacaoResultados` é um palpite) e o significado do
  `128` passado a `AssinarEcourna`.
* Se `CIdentificacaoSecao(…, 1, …)` nos hashes é intencional.
* `CConfiguracaoEleicao +580`: data da eleição ou uma "data limite"? `agora >= valor` é tratado como "passou".

## 12. Tabela de mapeamento completa (80 funções)

* **run**: observada em execução nos votos gravados (`analysis/runtime/*.functions.tsv`).
* **arquivo original**: onde a função fica na árvore do TSE ("caminho inferido" quando não atestado).
* **reconstruída em**: caminho sob `src/uenux2/src/app/` (fragmentos de arquivos de outras unidades terminam em `.u09.cpp`).

| idx | tamanho | run | nome das ferramentas | símbolo reconstruído | arquivo original | reconstruída em (sob `src/uenux2/src/app/`) | tipo / conf. |
|---|---|---|---|---|---|---|---|
| 198 | 19 |  | `comum_f198` | `api::CPaperFormBuilder::AddNewLine` | uenux2/src/api/gui/cpaperformbuilder.h (caminho inferido) | vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp (apêndice) | helper do tipo biblioteca / média |
| 601 | 34 |  | `vota_f601` | `api::CFormPart::CFormPart` | uenux2/src/api/gui/cformpart.h (caminho inferido) | vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp (apêndice) | construtor inline / média |
| 604 | 490 |  | `comum_f604` | `api::CPaperFormBuilder::AddData` | uenux2/src/api/gui/cpaperformbuilder.h (caminho inferido) | vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp (apêndice) | helper do tipo biblioteca / média |
| 1127 | 491 |  | `vota_f1127` | `vota::CLogVota::LogaImpressaoRelatorio` | uenux2/src/app/vota/log/clogvota.cpp | vota/log/clogvota.u09.cpp | TSE / média |
| 1249 | 262 | ✓ | `comum_f1249` | `comum::md::estadoaplicacao::CDadoCorrespondencia::CDadoCorrespondencia(const CDadoCorrespondencia&)` | uenux2/src/app/comum/dados/md/estadoaplicacao/cdadocorrespondencia.h (implícito, caminho inferido) | vota/eleitor/fimvotacao/cgravaresultado.cpp (comentário no apêndice) | construtor de cópia gerado pelo compilador / média |
| 1541 | 197 |  | `comum_f1541` | `comum::CRelUtil::IncluiResumoCorrespondencia` | uenux2/src/app/comum/relatorios/crelutil.cpp (caminho inferido) | vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp (apêndice) | helper do TSE / média |
| 1588 | 640 |  | `vota_f1588` | `vota::(anonymous namespace)::AdicionaCampo` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | vota/eleitor/comum/ctelasvota.u09.cpp | TSE / baixa |
| 2859 | 95 |  | `api_f2859` | `std::map<std::string, api::CIniSection>::__tree::destroy` (nó: chave +16, CIniSection = nome +28 + map<string,CIniKey> +40 via func 1397; os mapas de seções de CDependenciasContratos / CVersoesContratos) | biblioteca: libcxx/include/__tree | vota/eleitor/fimvotacao/cgravaresultado.cpp (comentário no apêndice) | instanciação de biblioteca / média |
| 2871 | 217 |  | `vota::CVisualizarCandidatos::vf0` | `vota::CVisualizarCandidatos::~CVisualizarCandidatos` | uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cvisualizarcandidatos.cpp | vota/eleitor/iniciovotacao/auxiliares/cvisualizarcandidatos.cpp | TSE (destrutor) / alta |
| 2877 | 135 |  | `vota_f2877` | `std::map<K, std::vector<{int,std::string}>>::__tree::destroy` | biblioteca: libcxx/include/__tree | vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp (apêndice) | instanciação de biblioteca / média |
| 2879 | 56 |  | `vota::CInformacaoZeresimaTardia::vf0` | `vota::CInformacaoZeresimaTardia::~CInformacaoZeresimaTardia` | uenux2/src/app/vota/eleitor/iniciovotacao/cinformacaozeresimatardia.cpp | vota/eleitor/iniciovotacao/cinformacaozeresimatardia.cpp | TSE (destrutor) / alta |
| 2883 | 56 |  | `vota::CExibeAlertaDesligamento::vf0` | `vota::CExibeAlertaDesligamento::~CExibeAlertaDesligamento` | uenux2/src/app/vota/eleitor/iniciovotacao/cexibealertadesligamento.cpp | vota/eleitor/iniciovotacao/cexibealertadesligamento.cpp | TSE (destrutor) / alta |
| 2890 | 1886 |  | `vota::CImprimindoBU::ImprimeBU` | `vota::CImprimindoBU::ImprimeBU` | uenux2/src/app/vota/eleitor/fimvotacao/cimprimindobu.cpp | vota/eleitor/fimvotacao/cimprimindobu.cpp | TSE / alta |
| 3699 | 238 |  | `comum_f3699` | `comum::CriaHeaderCargo` | uenux2/src/app/comum/relatorios/crelutil.cpp (caminho inferido) | vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp (apêndice) | helper do TSE / baixa |
| 3862 | 72 |  | `vota_f3862` | `std::set<comum::md::CCargo>::__tree::destroy` | biblioteca: libcxx/include/__tree | vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatosporcargo.cpp (comentário) | instanciação de biblioteca / média |
| 3873 | 230 |  | `vota_f3873` | `vota::(anonymous namespace)::CriaHeaderDetalheZE` | uenux2/src/app/vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp (caminho inferido) | vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp | TSE / baixa |
| 3877 | 140 |  | `vota_f3877` | `vota::CAplicacaoEncerrada::GetInst` | uenux2/src/app/vota/eleitor/fimvotacao/caplicacaoencerrada.cpp (caminho inferido) | vota/eleitor/fimvotacao/caplicacaoencerrada.u09.cpp | TSE / alta |
| 4551 | 174 |  | `vota_f4551` | `vota::CLogVota::LogaMesarioIndagadoQualidadeBU` | uenux2/src/app/vota/log/clogvota.cpp | vota/log/clogvota.u09.cpp | TSE / baixa |
| 5446 | 101 |  | `api_f5446` | `api::CWait::Aguarda` | uenux2/src/api/util/cwait.cpp | ../api/util/cwait.u09.cpp | TSE / média |
| 5462 | 227 |  | `comum_f5462` | `comum::(anonymous namespace)::Diretorio` | uenux2/src/app/comum/gravadores/cassinador.cpp (caminho inferido) | vota/eleitor/fimvotacao/cgravaresultado.cpp (apêndice) | helper do TSE / baixa |
| 5579 | 638 |  | `vota_f5579` | `vota::(anonymous namespace)::CriaTrailer` | uenux2/src/app/vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp (caminho inferido) | vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp | TSE / baixa |
| 5580 | 489 |  | `vota_f5580` | `vota::(anonymous namespace)::CriaSeparadorEleicao` | uenux2/src/app/vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp | vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp | TSE / baixa |
| 5587 | 54 |  | `vota_f5587` | `comum::CParteCargos::CParteCargos` | uenux2/src/app/comum/relatorios/cpartecandidatos.h (inline, caminho inferido) | vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp (apêndice) | construtor inline / média |
| 5655 | 23 |  | `vota_f5655` | `std::compare_three_way for TCargoID (uebyte)` | biblioteca / comparador inline | vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatosporcargo.cpp | biblioteca/helper inlinado / média |
| 5824 | 562 |  | `comum_f5824` | `comum::CGravacaoResultados::CGravacaoResultados` | uenux2/src/app/comum/gravadores/ (arquivo desconhecido; nome da classe inferido) | vota/eleitor/fimvotacao/cgravaresultado.cpp | TSE / baixa |
| 5856 | 176 |  | `comum_f5856` | `comum::CGravadorEnvelopeArquivo::CGravadorEnvelopeArquivo` | uenux2/src/app/comum/gravadores/cgravadorenvelopearquivo.cpp (caminho inferido) | vota/eleitor/fimvotacao/cgravaresultado.cpp (apêndice) | TSE / alta |
| 5866 | 51 |  | `api_f5866` | `comum::CGeracaoVersoesContratos::~CGeracaoVersoesContratos` (dois api::CIniStrings: mapas de seções +0/+24 via 2859, mapas de chaves +12/+36 via 1397) | uenux2/src/app/comum/gravadores/cgeracaoversoescontratos.cpp | vota/eleitor/fimvotacao/cgravaresultado.cpp (apêndice) | TSE (corpo do destrutor) / média |
| 5868 | 367 |  | `comum_f5868` | `std::set<std::string>::insert(const std::string&)` | biblioteca: libcxx/include/__tree | vota/eleitor/fimvotacao/cgravaresultado.cpp (apêndice) | instanciação de biblioteca / média |
| 5869 | 402 |  | `comum_f5869` | `comum::(anonymous namespace)::ColetaDependencias` | uenux2/src/app/vota/eleitor/fimvotacao/cgravaresultado.cpp (provável) | vota/eleitor/fimvotacao/cgravaresultado.cpp | TSE / baixa |
| 5870 | 87 |  | `comum_f5870` | `std::make_format_args for the contract-tag error (tag, "20260601173148")` | uenux2/src/app/vota/eleitor/fimvotacao/cgravaresultado.cpp (helper de formatação inlinado) | vota/eleitor/fimvotacao/cgravaresultado.cpp (apêndice) | biblioteca/helper inlinado / baixa |
| 5882 | 167 |  | `vota_f5882` | `vota::CLogVota::LogaMesarioIndagadoImprimirZeresima` | uenux2/src/app/vota/log/clogvota.cpp | vota/log/clogvota.u09.cpp | TSE / baixa |
| 5941 | 22 |  | `vota_f5941` | `vota::CReimprimindoZeresima::GetInst` | uenux2/src/app/vota/eleitor/iniciovotacao/creimprimindozeresima.cpp | vota/eleitor/iniciovotacao/cquerreimprimirzeresima.cpp | TSE / alta |
| 5943 | 98 |  | `api_f5943` | `vota::CDefineRotaPosReinicio::GetInst` | uenux2/src/app/vota/eleitor/cdefinerotaposreinicio.cpp | vota/eleitor/cdefinerotaposreinicio.u09.cpp | TSE / alta |
| 5970 | 181 |  | `vota_f5970` | `vota::(anonymous namespace)::CriaTrailerMajoritario` | uenux2/src/app/vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp | vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp | TSE / baixa |
| 5971 | 2742 |  | `vota::(anonymous namespace)::CriaTituloExtratoRDV` | `vota::CriaExtratoRDV` (inlina a CriaTituloExtratoRDV de namespace anônimo; o linkage/namespace da função externa é desconhecido) | uenux2/src/app/vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp | vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp | TSE / baixa |
| 5977 | 565 |  | `vota_f5977` | `vota::CRelVotaUtil::CriaCabecalho` | uenux2/src/app/vota/comum/crelvotautil.cpp (caminho inferido) | vota/comum/crelvotautil.u09.cpp | TSE / alta |
| 5982 | 1514 |  | `vota::CMostraQRCodeBU::AjustaTela` | `vota::CMostraQRCodeBU::AjustaTela` | uenux2/src/app/vota/eleitor/fimvotacao/cmostraqrcodebu.cpp | vota/eleitor/fimvotacao/cmostraqrcodebu.cpp | TSE / alta |
| 5986 | 140 |  | `vota_f5986` | `vota::CImprimindoBU::GetInst` | uenux2/src/app/vota/eleitor/fimvotacao/cimprimindobu.cpp | vota/eleitor/fimvotacao/cquerimprimirbu.cpp | TSE / alta |
| 6569 | 4055 |  | `vota_f6569` | `vota::CTelasVota::CriaTelaVisualizacaoCandidato` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | vota/eleitor/comum/ctelasvota.u09.cpp | TSE / alta |
| 11845 | 5 |  | `vota::CRegeraResumoZeresima::vf10` | `vota::CRegeraResumoZeresima::GetProximoEstado` | uenux2/src/app/vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp (provável; u07: cgeraresumozeresima.cpp) | vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp | TSE / média |
| 11848 | 349 |  | `vota::CQuerReimprimirZeresima::vf7` | `vota::CQuerReimprimirZeresima::ProcessInput` | uenux2/src/app/vota/eleitor/iniciovotacao/cquerreimprimirzeresima.cpp | vota/eleitor/iniciovotacao/cquerreimprimirzeresima.cpp | TSE / alta |
| 11849 | 250 |  | `vota::CQuerReimprimirZeresima::StartState` | `vota::CQuerReimprimirZeresima::StartState` | uenux2/src/app/vota/eleitor/iniciovotacao/cquerreimprimirzeresima.cpp | vota/eleitor/iniciovotacao/cquerreimprimirzeresima.cpp | TSE / alta |
| 11852 | 1153 |  | `vota::CReimprimindoZeresima::ImprimeZeresima` | `vota::CReimprimindoZeresima::StartState` | uenux2/src/app/vota/eleitor/iniciovotacao/creimprimindozeresima.cpp | vota/eleitor/iniciovotacao/creimprimindozeresima.cpp | TSE / alta |
| 11855 | 1305 |  | `vota::CReimprimindoResumoZeresima::ImprimeResumoZeresima` | `vota::CReimprimindoResumoZeresima::StartState` | uenux2/src/app/vota/eleitor/iniciovotacao/creimprimindoresumozeresima.cpp | vota/eleitor/iniciovotacao/creimprimindoresumozeresima.cpp | TSE / alta |
| 11865 | 114 |  | `vota::CGeraDadosDinamicos::StartState` | `vota::CGeraDadosDinamicos::StartState` | uenux2/src/app/vota/eleitor/iniciovotacao/cgeradadosdinamicos.cpp | vota/eleitor/iniciovotacao/cgeradadosdinamicos.cpp | TSE / alta |
| 11876 | 927 |  | `vota::CVisualizarCandidatos::StartState` | `vota::CVisualizarCandidatos::StartState` | uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cvisualizarcandidatos.cpp | vota/eleitor/iniciovotacao/auxiliares/cvisualizarcandidatos.cpp | TSE / alta |
| 11877 | 13 |  | `vota::CVisualizarCandidatos::vf1` | `vota::CVisualizarCandidatos::~CVisualizarCandidatos (deleting)` | uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cvisualizarcandidatos.cpp | vota/eleitor/iniciovotacao/auxiliares/cvisualizarcandidatos.cpp | TSE (destrutor) / alta |
| 11880 | 38 |  | `vota_f11880` | `vota::CVisualizarCandidatos singleton at-exit reset` | uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cvisualizarcandidatos.cpp (gerado pelo compilador) | vota/eleitor/iniciovotacao/auxiliares/cvisualizarcandidatos.cpp | gerado pelo compilador / média |
| 11881 | 2561 |  | `vota::CMenuVisualizarCandidatos::StartState` | `vota::CMenuVisualizarCandidatos::StartState` | uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cmenuvisualizarcandidatos.cpp | vota/eleitor/iniciovotacao/auxiliares/cmenuvisualizarcandidatos.cpp | TSE / alta |
| 11884 | 3643 |  | `vota::CMenuFiltrarCandidatosPorPartido::StartState` | `vota::CMenuFiltrarCandidatosPorPartido::StartState` | uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatosporpartido.cpp | vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatosporpartido.cpp | TSE / alta |
| 11888 | 3084 |  | `vota::CMenuFiltrarCandidatosPorCargo::StartState` | `vota::CMenuFiltrarCandidatosPorCargo::StartState` | uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatosporcargo.cpp | vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatosporcargo.cpp | TSE / alta |
| 11914 | 580 |  | `vota::CVerificaEleicaoPassou::StartState` | `vota::CVerificaEleicaoPassou::StartState` | uenux2/src/app/vota/eleitor/iniciovotacao/cverificaeleicaopassou.cpp | vota/eleitor/iniciovotacao/cverificaeleicaopassou.cpp | TSE / alta |
| 11946 | 6878 |  | `vota::CGeraZeresimaBase::vf2` | `vota::CGeraZeresimaBase::StartState` | arquivo não definido: cgerazeresima.cpp (u07) ou cgeradorresumozeresima.cpp; o construtor de relatório inlinado nela (frames aninhados de 64/400 bytes) vem de cgeradorresumozeresima.cpp (§11) | vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp | TSE / média |
| 11982 | 479 |  | `vota::CInformacaoZeresimaTardia::ProcessInput` | `vota::CInformacaoZeresimaTardia::ProcessInput` | uenux2/src/app/vota/eleitor/iniciovotacao/cinformacaozeresimatardia.cpp | vota/eleitor/iniciovotacao/cinformacaozeresimatardia.cpp | TSE / alta |
| 11983 | 29 |  | `vota::CInformacaoZeresimaTardia::vf2` | `vota::CInformacaoZeresimaTardia::StartState` | uenux2/src/app/vota/eleitor/iniciovotacao/cinformacaozeresimatardia.cpp | vota/eleitor/iniciovotacao/cinformacaozeresimatardia.cpp | TSE / alta |
| 11984 | 13 |  | `vota::CInformacaoZeresimaTardia::vf1` | `vota::CInformacaoZeresimaTardia::~CInformacaoZeresimaTardia (deleting)` | uenux2/src/app/vota/eleitor/iniciovotacao/cinformacaozeresimatardia.cpp | vota/eleitor/iniciovotacao/cinformacaozeresimatardia.cpp | TSE (destrutor) / alta |
| 11986 | 38 |  | `vota_f11986` | `vota::CInformacaoZeresimaTardia singleton at-exit reset` | uenux2/src/app/vota/eleitor/iniciovotacao/cinformacaozeresimatardia.cpp (gerado pelo compilador) | vota/eleitor/iniciovotacao/cinformacaozeresimatardia.cpp | gerado pelo compilador / média |
| 11991 | 2577 |  | `vota::CImprimindoZeresima::ImprimeZeresima` | `vota::CImprimindoZeresima::StartState` | uenux2/src/app/vota/eleitor/iniciovotacao/cimprimindozeresima.cpp | vota/eleitor/iniciovotacao/cimprimindozeresima.cpp | TSE / alta |
| 12017 | 337 |  | `vota::CExibeAlertaDesligamento::ProcessTick` | `vota::CExibeAlertaDesligamento::ProcessTick` | uenux2/src/app/vota/eleitor/iniciovotacao/cexibealertadesligamento.cpp | vota/eleitor/iniciovotacao/cexibealertadesligamento.cpp | TSE / alta |
| 12018 | 119 |  | `vota::CExibeAlertaDesligamento::StartState` | `vota::CExibeAlertaDesligamento::StartState` | uenux2/src/app/vota/eleitor/iniciovotacao/cexibealertadesligamento.cpp | vota/eleitor/iniciovotacao/cexibealertadesligamento.cpp | TSE / alta |
| 12019 | 13 |  | `vota::CExibeAlertaDesligamento::vf1` | `vota::CExibeAlertaDesligamento::~CExibeAlertaDesligamento (deleting)` | uenux2/src/app/vota/eleitor/iniciovotacao/cexibealertadesligamento.cpp | vota/eleitor/iniciovotacao/cexibealertadesligamento.cpp | TSE (destrutor) / alta |
| 12022 | 38 |  | `vota_f12022` | `vota::CExibeAlertaDesligamento singleton at-exit reset` | uenux2/src/app/vota/eleitor/iniciovotacao/cexibealertadesligamento.cpp (gerado pelo compilador) | vota/eleitor/iniciovotacao/cexibealertadesligamento.cpp | gerado pelo compilador / média |
| 12023 | 316 |  | `vota::CVerificaQtdBUsAdicionais::StartState` | `vota::CVerificaQtdBUsAdicionais::StartState` | uenux2/src/app/vota/eleitor/fimvotacao/cverificaqtdbusadicionais.cpp | vota/eleitor/fimvotacao/cverificaqtdbusadicionais.cpp | TSE / alta |
| 12026 | 130 |  | `vota::CLimiteCopiasBUAtingido::vf7` | `vota::CLimiteCopiasBUAtingido::ProcessInput` | uenux2/src/app/vota/eleitor/fimvotacao/climitecopiasbuatingido.cpp | vota/eleitor/fimvotacao/climitecopiasbuatingido.cpp | TSE / alta |
| 12027 | 108 |  | `vota::CLimiteCopiasBUAtingido::StartState` | `vota::CLimiteCopiasBUAtingido::StartState` | uenux2/src/app/vota/eleitor/fimvotacao/climitecopiasbuatingido.cpp | vota/eleitor/fimvotacao/climitecopiasbuatingido.cpp | TSE / alta |
| 12030 | 208 |  | `vota::CRetirarMR::ProcessInput` | `vota::CRetirarMR::ProcessInput` | uenux2/src/app/vota/eleitor/fimvotacao/cretirarmr.cpp | vota/eleitor/fimvotacao/cretirarmr.cpp | TSE / alta |
| 12031 | 621 |  | `vota::CRetirarMR::StartState` | `vota::CRetirarMR::StartState` | uenux2/src/app/vota/eleitor/fimvotacao/cretirarmr.cpp | vota/eleitor/fimvotacao/cretirarmr.cpp | TSE / alta |
| 12034 | 169 |  | `vota::CQuerImprimirBU::vf7` | `vota::CQuerImprimirBU::ProcessInput` | uenux2/src/app/vota/eleitor/fimvotacao/cquerimprimirbu.cpp | vota/eleitor/fimvotacao/cquerimprimirbu.cpp | TSE / alta |
| 12035 | 158 |  | `vota::CQuerImprimirBU::StartState` | `vota::CQuerImprimirBU::StartState` | uenux2/src/app/vota/eleitor/fimvotacao/cquerimprimirbu.cpp | vota/eleitor/fimvotacao/cquerimprimirbu.cpp | TSE / alta |
| 12038 | 184 |  | `vota::CMostraQRCodeCertificado::ProcessInput` | `vota::CMostraQRCodeCertificado::ProcessInput` | uenux2/src/app/vota/eleitor/fimvotacao/cmostraqrcodecertificado.cpp | vota/eleitor/fimvotacao/cmostraqrcodecertificado.cpp | TSE / alta |
| 12040 | 1932 |  | `vota::CMostraQRCodeCertificado::vf2` | `vota::CMostraQRCodeCertificado::StartState` | uenux2/src/app/vota/eleitor/fimvotacao/cmostraqrcodecertificado.cpp | vota/eleitor/fimvotacao/cmostraqrcodecertificado.cpp | TSE / alta |
| 12051 | 446 |  | `vota::CMostraQRCodeBU::ProcessInput` | `vota::CMostraQRCodeBU::ProcessInput` | uenux2/src/app/vota/eleitor/fimvotacao/cmostraqrcodebu.cpp | vota/eleitor/fimvotacao/cmostraqrcodebu.cpp | TSE / alta |
| 12055 | 5287 |  | `vota::CMostraQRCodeBU::StartState` | `vota::CMostraQRCodeBU::StartState` | uenux2/src/app/vota/eleitor/fimvotacao/cmostraqrcodebu.cpp | vota/eleitor/fimvotacao/cmostraqrcodebu.cpp | TSE / alta |
| 12065 | 1790 |  | `vota::CImprimirBUOutrasObrigatorias::StartState` | `vota::CImprimirBUOutrasObrigatorias::StartState` | uenux2/src/app/vota/eleitor/fimvotacao/cimprimirbuoutrasobrigatorias.cpp | vota/eleitor/fimvotacao/cimprimirbuoutrasobrigatorias.cpp | TSE / alta |
| 12068 | 1001 |  | `vota::CImprimirBJust::StartState` | `vota::CImprimirBJust::StartState` | uenux2/src/app/vota/eleitor/fimvotacao/cimprimirbjust.cpp | vota/eleitor/fimvotacao/cimprimirbjust.cpp | TSE / alta |
| 12071 | 1074 |  | `vota::CImprimindoBim::StartState` | `vota::CImprimindoBim::StartState` | uenux2/src/app/vota/eleitor/fimvotacao/cimprimindobim.cpp | vota/eleitor/fimvotacao/cimprimindobim.cpp | TSE / alta |
| 12074 | 964 |  | `vota::CImprimindoBEHB::StartState` | `vota::CImprimindoBEHB::StartState` | uenux2/src/app/vota/eleitor/fimvotacao/cimprimindobehb.cpp | vota/eleitor/fimvotacao/cimprimindobehb.cpp | TSE / alta |
| 12077 | 600 |  | `vota::CImprimindoBU::vf7` | `vota::CImprimindoBU::ProcessInput` | uenux2/src/app/vota/eleitor/fimvotacao/cimprimindobu.cpp | vota/eleitor/fimvotacao/cimprimindobu.cpp | TSE / alta |
| 12078 | 199 |  | `vota::CImprimindoBU::StartState` | `vota::CImprimindoBU::StartState` | uenux2/src/app/vota/eleitor/fimvotacao/cimprimindobu.cpp | vota/eleitor/fimvotacao/cimprimindobu.cpp | TSE / alta |
| 12098 | 26731 |  | `vota::CGravaResultado::vf2` | `vota::CGravaResultado::StartState` | uenux2/src/app/vota/eleitor/fimvotacao/cgravaresultado.cpp | vota/eleitor/fimvotacao/cgravaresultado.cpp | TSE / alta |
