# u26 — thread do monitor de energia, erros fatais, sincronização de relatórios MI→MV, validação do pendrive de resultado, teste de teclado, estados de entrada do voto

A unidade u26 cobre **110 funções wasm**. As ferramentas as agruparam em seis diretórios da aplicação VOTA
(`uenux2`, o código do TSE do programa da urna para o dia da eleição):

| diretório | o que fica ali |
|---|---|
| `app/comum/util/` | `CMonitoraAlimentacao`: registra em log as mudanças da fonte de energia e da carga da bateria (além de `FormataTamanho`, caminho inferido) |
| `app/comum/validamidia/` | `CValidaMidia` / `CFabricaConteudoMidiaStart`: o que o pendrive USB de resultado (MR) pode conter antes que os resultados sejam copiados para ele |
| `app/vota/comum/` | `CSincronizaVota` (assina um relatório e o replica da flash interna para a externa; flag global "urna desligando"), `votadefs.cpp` (verificações de assinatura, número de vias extras do BU), `CThreadVota` (caminho inferido: o tratador de erros fatais das três threads do VOTA) |
| `app/vota/eleitor/iniciovotacao/` | `CVerificaHorarioZeresima` (espera o horário da zerésima), o teste de teclado (`testeteclado/`: `CPreZeresima`, `CTesteTeclado`, `CEsperaRetestar`) |
| `app/vota/eleitor/voto*/` | os subestados de entrada do voto `CPedeMajoritario`, `CPedeProporcional`, `CCompletaProporcional`, `CPedeNulo` |
| `app/vota/log/`, `app/vota/monitor/` | `CLogVota::GetInst` e três mensagens de log; `CThreadMonitor`, a thread de vigilância do hardware |

A unidade também recebeu **helpers de arquivos que pertencem a outras unidades**, porque os seus chamadores estão aqui. São
comparações de `api::CDateTime`, partes do construtor de relatórios (`CPaperFormBuilder::Show/AddCut`, `CSubReport`, `CLp::Imprime`), o
relatório "estado da urna" (`CImprimirExtratoCarga::Imprime`, construtor de `CRelatorioTesteImpressora`), os estados de "Mais informações"
(`CImpressaoEstadoUrna`, `CImpressaoListaEleitores`, `CInicioVotacao`, `CConfirmaImpressaoZeresima`), três
linhas de log do terminal do mesário, `CApplicationContextStack::Top`, `CApplication::ShowExceptionMsg`, `api::CTimer::Start`
e quatro instâncias da libc++. O §15 mapeia todas as 110.

**Runtime.** O profiler por amostragem de 20 µs viu 13 funções nos votos gravados: `CLogVota::GetInst` (184),
`CDateTime()`/`Compare` (479, 759), `CPath::GetPathMR` (949) e os estados de entrada de voto proporcional/majoritário (5923,
5930, 5931, 11683, 11684, 11711, 11712, 11756, 11757). A amostragem perde funções minúsculas, então algumas outras certamente foram executadas:
`CPedeMajoritario::GetTelaCargoAtual` (5920, um thunk de 16 bytes chamado diretamente pelas observadas 11683/11684) e o
construtor de `CCompletaProporcional` (5932, chamado pelo `CPedeNominal::GetInst` preguiçoso inlinado em 11711 na primeira vez que um
número de partido é digitado). `CPedeNulo::GetProximoEstado` (11744) é executada para um voto nulo proporcional, e `CTime::Compare`
(5448) sempre que 759 compara duas datas iguais. O resto não é alcançado no simulador: a thread do monitor nunca é
iniciada, a página nunca entra no fluxo do início do dia (zerésima, teste de teclado), e o fluxo do fim do dia (BU, pendrive
de resultado) nunca é alcançado.

**Fontes reconstruídos** (`// wasm func N` em toda função), em `src/uenux2/src/`:

* arquivos próprios: `app/comum/util/cmonitoraalimentacao.{h,cpp}`, `app/comum/util/formatatamanho.cpp` (caminho inferido),
  `app/comum/validamidia/cvalidamidia.{h,cpp}`, `app/comum/validamidia/cfabricaconteudomidiastart.{h,cpp}`,
  `app/vota/comum/csincronizavota.{h,cpp}`, `app/vota/comum/votadefs.{h,cpp}`, `app/vota/comum/cthreadvota.{h,cpp}` (caminho
  inferido), `app/vota/log/clogvota.{h,cpp}`, `app/vota/monitor/cthreadmonitor.{h,cpp}`,
  `app/vota/eleitor/iniciovotacao/cverificahorariozeresima.{h,cpp}`,
  `app/vota/eleitor/iniciovotacao/testeteclado/{ctesteteclado,cprezeresima}.{h,cpp}`,
  `app/vota/eleitor/votamajoritario/cpedemajoritario.{h,cpp}`,
  `app/vota/eleitor/votaproporcional/{cpedeproporcional,ccompletaproporcional,cpedenulo}.{h,cpp}`
* fragmentos de arquivos de outras unidades: `app/vota/eleitor/comum/ctelasvota.u26.cpp`,
  `app/vota/eleitor/iniciovotacao/testeteclado/cesperaretestar.u26.cpp`, `app/comum/u26-foreign-fragments.cpp`,
  `app/vota/u26-foreign-fragments.cpp`, `api/u26-foreign-fragments.cpp`

---

## 1. Glossário

| termo | significado |
|---|---|
| MI / MV / MR | *memória interna* (flash interna `/dsk/fi`), *memória de votação* (cartão de memória `/dsk/fe`), *mídia de resultado* (pendrive USB de resultado `/dsk/mr/`) |
| trab1 / trab2 | diretório de trabalho do 1º / 2º turno (`<flash>/dinamico/trab<N>/`) |
| SAVD, `.vsu` | o serviço de assinatura/verificação da urna; um `.vsu` é o pacote de assinatura de um arquivo em `dinamico/` |
| zerésima | relatório impresso antes da abertura da votação, que prova que a urna não contém nenhum voto |
| TE / MT | *terminal do eleitor* (a tela e o teclado do eleitor) / *microterminal* (o LCD de 2 linhas do mesário) |
| teste do teclado | o teste do início do dia em que o mesário pressiona as 13 teclas do teclado do eleitor em ordem aleatória |
| legenda | um número de partido (2 dígitos); um *voto de legenda* é um voto apenas no partido |
| inapto | um candidato registrado que não pode receber votos; o voto é nulo |
| vias | cópias impressas de um relatório (BU, estado da urna, lista de eleitores ...) |
| chave | a chave de energia da urna; "URNA DESLIGADA" = chave desligada |
| contexto da aplicação | `api::CApplicationContext` (unidade u15), o texto mostrado quando uma operação falha |

## 2. Classes (RTTI) e layouts

```
api::CThread (u18)
└─ vota::CThreadVota            typeinfo @1532484, vtable @1532460   (path inferred vota/comum/cthreadvota.cpp)
   ├─ CThreadEleitor, CThreadOperador   (units u07 / u10)
   └─ vota::CThreadMonitor      typeinfo @1600968, vtable @1600768, 48 bytes

comum::CAppState ─ vota::CEstadoComDesligamentoAutomatico (u06)
   ├─ vota::CVerificaHorarioZeresima          @1545864   52 B (+28 tela, +36 CDateTime horário, +48 tick 2 s)
   ├─ testeteclado::CBase ─ CPreZeresima      @1546044   (+28 tela, +36 CDateTime agora)
   │                     ─ CRetomada          (u07/u20)
   ├─ testeteclado::CTesteTeclado             @1547140   68 B
   ├─ testeteclado::CEsperaRetestar / CTesteFalhou / CEnviarManutencao / CErroTesteTecladoFim   (u02)
comum::CAppState ─ vota::CVotacaoStateAudio (u08)
   ├─ vota::CPedeMajoritario                  @1550288   28 B
   ├─ vota::CPedeProporcional                 @1549440   28 B
   └─ vota::CCompletaProporcional             @1548104   32 B (+28 ETelaVotacao)
        ├─ vota::CPedeNulo                    @1548532   (tela 5)
        └─ vota::CPedeNominal                 @1549088   (tela 8, other unit)
comum::impl::IValidaMidia ─ comum::impl::CValidaMidia  @1577032, 4 B (vptr only)
vota::testeteclado::impl::IGeradorTeclas ─ impl::CGeradorTeclasAleatorio  @1547296
comum::IEventosLog ─ vota::CLogVota            @1532792, 8 B (+4 ELogAplicativos = 1)
comum::CImprimirExtratoCarga (abstract) ─ comum::CRelatorioTesteImpressora  @1576660  (26 slots)
```

Nomes dos slots de `CThreadVota` (a partir dos corpos; os nomes da u07 são mantidos para 3/4, com os tipos de parâmetro corrigidos):
`[0] ~CThreadVota (2126) [1] deleting [2] Run = 0 [3] TrataExcecao(const ecourna::api::exception::CError&) (7710)
[4] TrataExcecaoDesconhecida(const std::exception&) (7709) [5] FinalizaExecucao = 0`. 7710 aplica `dynamic_cast` ao seu argumento,
de `CError` para `api::CUeDesligandoError`, e 7709 apenas chama `what()`. Os slots 3 e 4 já são puros na
vtable do próprio `api::CThread` (@1599960: slots 2, 3, 4 = `__cxa_pure_virtual`), então CThreadVota os *sobrescreve*; apenas
o slot 5 é novo. (O `cthread.h` da unidade u18 dá às declarações da base as listas de parâmetros erradas: os dois corpos recebem um segundo
argumento.) **Nenhum código despacha para os slots 3/4:** `CThread::ThreadProc` (10255) apenas chama `Run()` e define o estado como
3, e nem `CThreadEleitor::Run` (7061) nem `CThreadOperador::Run` (10204) têm um try/catch, então os dois tratadores são
inalcançáveis nesta build.

Os slots de hook 9/10 de `CVotacaoStateAudio` agora estão **atestados**. `CCompletaProporcional` passa o seu
`__PRETTY_FUNCTION__` para `GetTelaCargoAtual(const std::string&)`, e as strings são
`"virtual void vota::CCompletaProporcional::ProcessInputAudio()"` (slot 9) e `"…::StartStateAudio()"` (slot 10).

`CMonitoraAlimentacao` (8 bytes, @1911568): `+0 ELogAplicativos (1)`, `+4 tipo de alimentação`,
`+5/+6 status das baterias interna/externa` (todos `0xFF` = nada registrado ainda), `+7 contador de eventos`.

## 3. A thread do monitor (`CThreadMonitor`, `CMonitoraAlimentacao`)

**Criação.** `CThreadMonitor::GetInst` (1898) é um singleton preguiçoso @1911596. As ferramentas primeiro o nomearam a partir do registro
inlinado de `CMonitoraAlimentacao::CreateInst` (agora elas mostram `vota::CThreadMonitor::GetInst`). O seu construtor (cthreadmonitor.cpp:52) lê o estado do fone de ouvido de
`api::IPower`, define o próximo log periódico como "agora" e chama `CMonitoraAlimentacao::CreateInst(1)`, que lança
`CUeComumUtilError 9156 "Instância já criada"` se for chamado duas vezes. Chamadores: a política de execução da urna
`vota::CExecucaoVota` e `CThreadOperador::FinalizaExecucao`.

**Laço** (`Run`, 10226; `do … while (!m_bParar)`, sleep de 500 ms):

1. `CMonitoraAlimentacao::VerificaAlimentacao()` (cmonitoraalimentacao.cpp:52). Quando a fonte de energia muda
   (bits de status 1..2: 0 rede, 1 bateria interna, 2 bateria externa, 3 → lança 8850), registra `CLogComum::LogaTipoBateria()`
   ("Urna operando na rede elétrica/bateria interna/bateria externa") e reinicia os estados das baterias. Quando o estado de
   carga da bateria em uso muda (bits 3..4 interna, 5..6 externa), registra `LogaNivelBateria(1|2, estado)`
   ("Carga da [ALIMENTAÇÃO BATERIA …]: [PLENA|PARCIAL|CRÍTICA|AUSENTE]"). Após **20 mudanças de fonte**, registra
   "Suspenso monitoramento de alimentação devido repetição de eventos. [ 20 ] eventos" uma vez e para de vigiar.
2. `MonitorFoneOuvido` (:127): "Fone de ouvido conectado/desconectado" a cada mudança (palavra de status +8, bit 1).
3. `VotacaoSuspensa` (:93): bit de status 14 = chave de energia desligada. Então `SaiPorVotacaoSuspensa` registra
   "Mudança do estado da chave: URNA DESLIGADA", liga a flag de desligamento (func 3336), para as threads do eleitor e do
   operador (`m_bParar`) e inicia um bipe via `std::async` (lambda :103, func 10224, unidade u19).
4. `VerificaErroCriticoFlash` (:153/:156): fora da fase de *treinamento*, uma flash externa ausente lança
   `CUeVotaError 9390 "Erro na Mídia Externa - Mídia não está presente"`.
5. `VerificaErroCriticoKbdTE` (:165/:167): bit 0 do byte de status +19 → `CUeVotaError 9391 "Erro no teclado do eleitor -
   dispositivo desconectado"`. A conexão do teclado é reportada através do bloco de status de energia/controlador.
6. A cada **1800 s** (slot 0 de `ISystemDateTime`), três logs são executados:
   * `LogaEspaco` (:225) registra "Espaço livre/utilizado na MI/MV [x.y MB]" (`std::filesystem::space` de
     `CPath::GetPathDinamico(0|1)`, a sobrecarga *que lança*: `__space` (4677) teve o seu `error_code*` removido porque ele é
     sempre nulo, então uma falha de `statfs` lança `filesystem_error("space")`). Em seguida, chama
     `IInterfaceInit::LogDiskInfo` (comando 19).
   * `LogaMemoria` registra "Quantidade de memória total/livre/usada [..]" a partir de `/proc/meminfo` (func 5463).
   * `LogaStatusRedeAcBateria` (:258/:260) só é executada em modelos de urna ≥ 2020 e registra
     "Urna ligada conectada na|desconectada da rede CA em [v.vvV] e na Bateria Interna|Externa com [v.vvV/xA]"
     (slots 1..4 de IPower, valores em centésimos; a corrente é um `double` impresso com um `{}` simples, por exemplo "0.35A").
     "Interna" é escolhida quando o slot 3 (tensão da bateria externa) é ≤ 0, e a tensão impressa é então a do slot 1.

`FinalizaExecucao` (10225) liga `m_bParar` nas threads do operador e do eleitor. O registro source_location
`:117 MostraMensagemDesligamento` existe nos dados, mas nenhum código o referencia. O seu chamador provavelmente foi removido junto
com o código que vinha depois do `std::async` (veja o §12).

## 4. Erros fatais de uma thread do VOTA (`CThreadVota::TrataExcecao*`, 7710/7709)

Este caminho só é executado na urna; nesta build nada o chama (veja o §2).

1. Um `CUeDesligandoError` (a urna está sendo desligada) é ignorado (somente em 7710). Se a flag de desligamento @1832936 já
   estiver ligada, o tratador retorna.
2. `FinalizaExecucao()` (slot 5).
3. No microterminal do mesário (func 4633), o LED se apaga e a tela mostra "URNA ELETRÔNICA INOPERANTE" /
   "Siga as instruções na tela do eleitor".
4. `CSincronizaVota::MarcaUrnaDesligando()` (3336).
5. A tela do eleitor mostra a página de erro construída a partir do topo da pilha de contextos da aplicação (func 1695):
   * tipo: 2 para o contexto genérico de fallback, 1 nos demais casos (func 5569);
   * detalhe: a linha de detalhe do contexto;
   * título: `std::format("{} ({})", título, erro.GetCodigo())` para um `CError`, ou o título simples para uma `std::exception`;
   * mensagem: `mensagem + "\n\n" + ações joined by "\n"`. Para o contexto genérico, o texto da exceção vem primeiro. Para uma
     `std::exception`, a regex `.*?\).*?:\d+:-?\d+ - ((.|\n)*)` remove o prefixo "função arquivo:linha:coluna - "
     das strings `what()` do TSE.

   `api::CApplication::ShowExceptionMsg(tipo, detalhe, título, mensagem, what)` (5568) exibe a página.
6. `EnterLoopDoingNothing()` (5566) estaciona a thread para sempre.

Se a pilha estiver vazia, `Top()` retorna o contexto genérico com **detalhe "Erro inesperado" e título "Pilha de
contexto vazia"**. Essa é a ordem dos argumentos no binário; o comentário da unidade u15 os tem trocados.

## 5. `CSincronizaVota` e `votadefs.cpp`: mantendo a MV idêntica à MI

* **Flag de desligamento** `ms_desligando` (byte @1832936): ligada por 3336, testada inline por todo gravador.
  `VerificaUrnaDesligando()` lança `api::CUeDesligandoError(4201, "Urna desligando")` (csincronizavota.cpp:47). A func 1685 é
  apenas a construção fora de linha da exceção.
* **`SincronizaRelatorios(arquivo)`** (1836, :392). É executada depois que um arquivo de relatório foi gravado em `<MI>/dinamico/trab<N>/`.
  1. Verifica a flag de desligamento e depois chama `fsync` (`CSynchronizer`, vazio nesta build).
  2. Constrói um `vota::CAssinadorVota` vinculado ao pacote 122/123 (`vota.vsu` da MI, turno 1/2).
  3. Mapeia o nome para o seu id de arquivo SAVD: apenas `bu.dat` (86), `buj.dat` (84), `ze.dat` (88), `rze.dat` (89),
     `bim.dat` (106) e `behb.dat` (108) são aceitos. Qualquer outro nome lança `CUeVotaError 9302 "O arquivo<x> não é um
     relatório válido para sincronização"`. O literal @124100 é `"O arquivo"` sem espaço no final, então o
     nome fica colado à palavra (sic).
  4. `assinador.Assina(id)` (func 1277: requisição de assinatura SAVD 0x42 para a aplicação 1) e fsync.
  5. Copia `<MI trab>/arquivo` → `<MV trab>/arquivo` (`CSystem::CopyFile`).
  6. Copia o pacote de assinatura correspondente MI → MV (bu.vsu 126→128 / 127→129, buj.vsu 130→132, ze.vsu 138→140,
     rze.vsu 142→144, bim.vsu 191→193, behb.vsu 195→197; ids ímpares para o turno 2). O helper inlinado copiaria **todos os seis**
     pacotes para um nome desconhecido, mas esse ramo está morto porque o passo 3 já rejeitou os nomes desconhecidos.
  7. fsync.
* **`VerificaAssinaturaMI/MV(pacote)`** (3290/3288): `IInterfaceSavd::ValidarUE(<trab of MI|MV>/<pacote>)`. É usada após
  todo voto para `vota.vsu`, `rdv.vsu` e `uenux.vsu` (unidade u07). Na build web, `CWasmSavd` aceita tudo.
* **`GetQuantidadeMaximaBUsAdicionais()`** (4582, :135) retorna quantas vias a mais do BU podem ser impressas. Uma urna de
  treinamento do eleitor (fase '3' + `treinamentoEleitor`) sempre recebe 1. Caso contrário, o resultado é
  `obrigatórias − já impressas + adicionais` (funcs 3847/5914 de `CInformacaoEleicao`: 1 cada no modo de demonstração, senão os parâmetros
  da PU +12/+16; `EstadoGeralVota.qtdBU`), com o assert `numImpresso >= numObrigatorias` (3449).

## 6. Validação do pendrive de resultado (`CValidaMidia`, `CFabricaConteudoMidiaStart`)

`vota::CCopiaResultadoParaMR` (12134, unidade u09) usa dois slots antes de copiar os arquivos de BU/RDV/log para `/dsk/mr/`:
o slot 3 `ValidaMidiaResultado(aplicativo, turno)` se o pendrive não estiver vazio e, fora do treinamento, o slot 6
`MidiaContemResultados()` ("Mídia de resultado já contém arquivos de resultados" 9369).

**`ValidaConteudo(dir, esperados)`** (5572) é o núcleo:

1. Apaga os metadados de sistemas operacionais de desktop (5571). Um nome ausente não é erro (o `remove_all_impl` da libc++ limpa ENOENT), mas
   é usado o `remove_all(path)` *que lança* (ErrorHandler com `ec = nullptr`, sem try/catch em 5571/5572/11172), então qualquer
   outra falha (pendrive somente leitura, EACCES, EIO...) lança `std::filesystem::filesystem_error("remove_all")` até
   `CCopiaResultadoParaMR`. Para macOS: `.DS_Store .AppleDouble .LSOverride
   .DocumentRevisions-V100 .fseventsd .Spotlight-V100 .TemporaryItems .Trashes .VolumeIcon.icns
   .com.apple.timemachine.donotpresent`. Para Windows: `Thumbs.db Thumbs.db:encryptable ehthumbs.db ehthumbs_vista.db
   "System Volume Information" $RECYCLE.BIN`. Cada um é removido com `std::filesystem::remove_all(dir + name)`.
2. Se o diretório não existe ou está vazio, o resultado é válido (1) quando nada é esperado; caso contrário, é inválido (6).
3. Um subdiretório torna o pendrive inválido (7). Os arquivos regulares são coletados.
4. O resultado é inválido se forem esperados mais nomes do que há arquivos (8), se algum arquivo não corresponder a nenhum dos **14
   nomes permitidos** (9) ou se um nome esperado estiver ausente (10). Caso contrário, é válido (2).

Nomes permitidos (func 5572, nesta ordem): `infomidia.dat`, `infomidia.vsc`, `turno2.jez`, `turno2.vsc`,
`#regex#[ost][0-9]{5}[a-z]{2}-pkgsa\.jez`, `#regex#[ost][0-9]{5}[a-z]{2}-pkgsa\.vsc`,
`#regex#[ost][0-9]{5}[a-z]{2}[0-9]{13}-[a-z]+\.[a-z]+` (arquivos de resultado), `#regex#[ost][0-9]{5}[0-9]{4}[0-9]{4}[0-9]{8}-[a-z]+\.[a-z]+`,
`#regex#[0-9]{8}\.[ste|STE]`, `ste.config`, `#regex#[0-9]{2}.pub`, `#regex#[0-9]{2}.id`, `#regex#[0-9]{2}ue`,
`#regex#[0-9]{3}ue[0-9]{2}\.vpe`.
Um nome é ou `#regex#<pattern>`, testado com `boost::regex_search` **não ancorado** (func 2199), ou um literal do
mesmo tamanho em que `#` corresponde a um dígito, `@` a uma letra e `?` a uma letra/dígito/`-`/`_`, em qualquer uma das strings (5464).

`ConteudoIncializacao(aplicativo, turno)` (cfabricaconteudomidiastart.cpp:62) lista o que um pendrive inicializado precisa conter.
As aplicações 11–14 e 18 precisam de `infomidia.dat/.vsc`. As aplicações 15/16/17 precisam desses mais o pacote do SA
`t|o|s#####xx-pkgsa.jez/.vsc` (treinamento/oficial/simulado). A aplicação 20 precisa de `##.pub ##.id ##ue ###ue##.vpe`.
As aplicações 7–10 não precisam de nada. As outras (1–6 e 19) lançam 9200 "Não há mídia de resultado de inialização para a
aplicação '{}'." (sic) quando o turno é '1'/'2'; com qualquer outro turno, a sua lista é vazia e nada é lançado.
As aplicações 7–10 e 15–17 exigem turno '1'/'2' (senão, resultado 5), e valores fora de 1..20 dão resultado 4. O
`CWasmInit` da build web reporta **10** para o VOTA, então para o simulador a lista é vazia e só a verificação de nomes permitidos
se aplica. Quando o resultado é ≥ 4, os nomes *esperados* (e não os arquivos problemáticos) são registrados em LOG_INFO (5574).

O slot 8 (`MidiaContemSomenteResultados`, 11167) é uma variante mais estrita: sem limpeza de metadados, sem arquivos extras, sem
subdiretório. O slot 9 apenas executa a limpeza.

## 7. Início do dia: horário da zerésima, teste de teclado, "Mais informações"

* **`CVerificaHorarioZeresima`** (11871/11869/11870/11868; construtor em 5947). Antes de `CConfiguracaoEleicao +544` (data/hora
  da zerésima), registra "Aguardando data e hora para emissão da zerésima", mostra a tela "ATENÇÃO / Esta urna eletrônica
  só funcionará a partir de …" e verifica novamente a cada 2 s. Quando o horário já passou, com o assert
  `EstadoVota == EAVAGUARDAHORAZERESIMA` (:64/:80), passa para `CInicioZeresima`. Teclas: BRANCO abre
  `CMaisInformacoes`. CONFIRMA imprime o relatório de **estado da urna** (§9) enquanto `numRelatorioEstado` (PU) for maior
  que as vias já impressas (`EstadoGeralVota +73`), e então reconstrói a tela.
* **Teste de teclado.**
  * `CPreZeresima::CriaTela` (11862) torna o teste opcional ("Quer testar o teclado?" Testar / Não testar, func 6581)
    enquanto hoje for anterior à véspera da data da zerésima (`i64.load offset=544` de `CConfiguracaoEleicao`, a parte de data
    da mesma data/hora da zerésima pela qual `CVerificaHorarioZeresima` espera, menos um dia com a func 5477), e
    obrigatório ("Por favor, teste o teclado", 6580) a partir dessa véspera. Como a zerésima é impressa no dia da eleição,
    na prática isso é a véspera da eleição. "Não testar" (slot 12, 11861) só funciona no período opcional.
  * Após o teste, `GetEstadoPassouNoTeste` (5945, :56) vai para `CGeraDadosDinamicos` (estado '1'/'2') ou
    `CVerificaHorarioZeresima` ('3'). Qualquer outro estado registra "Erro estado do aplicativo não esperado" e lança
    9377 "Estado nao esperado <n>" (com o código numérico do caractere, por exemplo "52").
  * `CTesteTeclado::StartState` (11805) registra "Início do teste de Teclado do TE" e desenha as 13 teclas como `CTextBox`es.
    O layout depende do modelo da urna (≤ 2019 ou ≥ 2020, `GetKeyboardLayout`, ctelasvota.cpp:1301). Os rótulos são
    `"1".."9","0","BRANCO","CORRIGE","CONFIRMA"` @1833360. Pede a `impl::IGeradorTeclas` (registro padrão
    :121: `CGeradorTeclasAleatorio`) uma ordem aleatória e destaca a primeira tecla. Erros: 9359 "Sem teclas para
    testar", 9360/9361/9362 "Tecla não encontrada: <x>".
  * `ProcessInput` (11804): a tecla certa volta ao normal e a próxima tecla é destacada. Após a última tecla,
    registra "Fim do teste de Teclado do TE - Sucesso" e vai para o próximo estado salvo. Uma **tecla errada** armazena as teclas esperada
    e pressionada em `CInformacaoEleitor` +9/+10 e vai para `CTesteFalhou` ("Teste Falhou", "Falha: Esperada X,
    pressionada Y", Repetir teste / Prosseguir). Ela **também registra a mensagem de sucesso** (veja o §13).
  * `CEsperaRetestar` (11823/11826) faz a contagem regressiva até `g_horaRetestar` @1835000 ("Por favor, espere {}s …") e depois volta
    para `CTesteTeclado`.
* **`CConfirmaImpressaoZeresima::ProcessInput`** (11927): BRANCO abre "Mais informações". CONFIRMA, após o prazo da
  zerésima (+20), vai para `CImpressaoZeresimaTardia`; caso contrário, define `EstadoVota = '4'`, salva o estado e vai para
  `CGeraZeresima`.
* **`CInicioVotacao`** (11988/11987) espera o horário de abertura (+28), consultando com o seu tick (+40), e depois vai para
  `CAguardaMensagem`.
  `IniciaVotacao` (5972) registra `dhIniAquisicao` e salva o estado quando `CRdvVota.Comparecimento() == 0`, mostra
  `CTelasVota +132` (primeira abertura) ou `+140` (retomada) e posta a mensagem 8 para a thread do operador.
* **Itens de "Mais informações"**: `CImpressaoEstadoUrna` (11911) imprime novamente o relatório de estado da urna, sem limite de
  vias (veja o §13). `CImpressaoListaEleitores` (11908, condensada) imprime a lista de eleitores com a legenda "AUD: Áudio / BIO:
  Biometria / IMP: Impedido / TTE: Transferência Temporária" e conta as vias em `EstadoGeralVota +74`.
  `CGeradorRelVersaoPacoteDados::Imprime` (5592/5596) imprime o relatório de versões dos pacotes.

## 8. Entrada do voto (observada em execução)

```
CPedeProporcional (screen "party", 2 digits)
   BRANCO ─────────────► CConfereVotoEmCargo<CProporcionalBranco, 4>
   2 digits complete ──► party exists && LegendaValida(cargo, party) (func 5922) ─► CPedeNominal (tela 8)
                                                                              else ─► CPedeNulo (tela 5)
CCompletaProporcional (CPedeNominal / CPedeNulo): the remaining digits
   CORRIGE ────────────► CPedeProporcional
   complete ───────────► g_votoDigitado += digits; GetProximoEstado(g_votoDigitado)   (slot 16)
      CPedeNulo:  full-length number of an "inapto" candidate ─► <CCandidatoInapto, 14>, else <CProporcionalNulo, 7>
CPedeMajoritario (full number)
   BRANCO ─────────────► <CMajoritarioBranco, 4>
   complete ───────────► !ExisteCandidatoValidoResposta ─► <CMajoritarioNulo, 7>
                         VotoRepetido (same nominal vote for an earlier vaga of this cargo) ─► <CMajoritarioRepetido, 16>
                         else ─► <CMajoritarioValido, 2>
```

* `ExisteCandidatoValidoResposta` (:125). Para uma consulta (plebiscito), procura a resposta em `CRespostas`.
  Caso contrário, o candidato precisa existir e não estar marcado em `CCandidatura +52` (inapto), e o seu partido precisa existir:
  um partido ausente registra "Erro partido não encontrado" e lança 9382. Na prática, um candidato majoritário inapto leva
  à tela de **voto nulo** (não há tela de "inapto" para cargos majoritários). Os dois caminhos armazenam um "nulo" no RDV.
* `VotoRepetido` só é executada quando `CCargo.qtdEscolhas > 1`, `CCargo +14 == 0` e `g_numeroEscolha > 1`. Ela percorre as
  últimas `g_numeroEscolha − 1` entradas de `g_votosEleitor` com `.at()`.
* `EInputResult 9` ("Confirma" no enum) é o que o campo de entrada retorna quando está cheio. O eco de
  `CCompletaProporcional` (5930) recusa um dígito apenas quando o texto não mudou. A versão base (3139) também recusa um CORRIGE sem efeito.
* Templates de áudio: majoritário e proporcional usam "Você está votando para {cargo-atual}. {quantidade-digitos}. Voto
  {progresso}." (uma consulta descarta o prefixo). `CPedeNulo` usa "… no candidato {voto}. Número errado. Aperte confirma
  para prosseguir, ou corrige para reiniciar este voto."

## 9. Relatórios impressos tocados aqui

* **Estado da urna** (`CImprimirExtratoCarga::Imprime`, 5591, um template method sobre 24 hooks virtuais de
  `CRelatorioTesteImpressora`). O relatório contém, em ordem:
  * corte de papel, "IMPRESSO EM MODO DEMONSTRAÇÃO" (modo de demonstração), o título da carga com `<uf>`, "Eleições Comunitárias",
    nome da eleição, nome do pleito e "(DD/MM/YYYY)", os nomes das eleições;
  * o separador `====`, "ESTADO DA URNA", o separador de fase, "UE DE SEÇÃO|CONTINGÊNCIA", "UF …",
    município/zona (rótulos `<MCSN>`, `<ZCSN>` traduzidos por `CTradutorFrase`), linhas do local;
  * "Código identificação UE {:08}" / "MC {:.8}", "Código de identificação carga" + código, "Data da carga" / "Hora da
    carga", "EMISSÃO DO RELATÓRIO", "RESUMO DA CORRESPONDÊNCIA";
  * a previsão de horário de verão ("Previsão de horário de verão:", "Início: …, fim: ….", "-> Urna em horário de verão
    <-" / "Não existe previsão de horário de verão."), pulada em treinamento;
  * QR code(s), **"Ver: 10.23.0.1 - DESENVOLVIMENTO"** (literal @326597), "URNA OPERANDO EM PERFEITAS / CONDIÇÕES DE
    FUNCIONAMENTO", "ASSINATURAS", 8 linhas em branco e um corte.

  O trabalho passa por `CPaperFormBuilder::Show(título, via)` (3671) → `CLp::Imprime` (3875) → o
  singleton `IImpressoraRelatorios`, que é `CWasmNullPrinter` na build web (nada é impresso).
* `CRelVotaUtil::CortaPapel` (2882): 2 linhas em branco e um corte como um trabalho sem título (entre as vias da zerésima).
* Rodapé de `CGeradorRelVersaoPacoteDados` (5596): "Código de identificação da carga", código, "Ver: 10.23.0.1" (o
  `GetVersionNumber` do mesmo literal), corte.

## 10. BOLETIM DE URNA (BU): o que esta unidade contribui

1. **Após a geração** (`CGeraBU` das unidades u08/u09, 12110, grava `<MI>/dinamico/trab<N>/bu.dat`), `CGeraBU` chama
   `CSincronizaVota::SincronizaRelatorios("bu.dat")` (§5). Ela para com `CUeDesligandoError 4201` se a urna estiver
   sendo desligada. Caso contrário, pede ao SAVD para assinar `bu.dat` (id de arquivo 86, comando de assinatura 0x42, aplicação 1), faz fsync, copia
   `bu.dat` para `<MV>/dinamico/trab<N>/bu.dat` e copia o pacote `bu.vsu` (ids 126→128 para o turno 1, 127→129 para o turno 2).
   O mesmo acontece para `buj.dat`, `bim.dat`, `behb.dat` (CGeraRelatorios) e `ze.dat`, `rze.dat` (zerésima).
   A cópia do BU na MV é, portanto, uma cópia byte a byte da que está na MI, e o `.vsu` da MV é o pacote da MI (o arquivo da MV não é
   assinado separadamente).
2. **Número de vias**: `GetQuantidadeMaximaBUsAdicionais()` (4582) limita as vias extras ("vias adicionais") que
   `CEmitirMaisBU` oferece: `obrigatórias + adicionais − impressas`, ou 1 numa urna de treinamento do eleitor. O contador de
   vias impressas é `EstadoGeralVota.qtdBU`.
3. **Pendrive de resultado**: antes que `bu.dat`, `rdv.dat`, `jufa.dat`, `imgbu.dat`, `imgze.dat`, `hash.dat`, `log.jez`, `vota.vsc`, `mr.ver`
   (e os `wsq*.jez` das urnas biométricas) sejam copiados para `/dsk/mr/`, um pendrive não vazio precisa passar por
   `ValidaMidiaResultado(aplicativo, turno)` (§6). Fora do treinamento, ele não pode já conter arquivos de resultado
   (`MidiaContemResultados()`, regex `[ost]#####xx#############-<tipo>.<ext>`). O padrão de nome de arquivo dos resultados
   da seção é `<fase o|s|t><pleito:05><uf><município:05><zona:04><seção:04>-<tipo>.<ext>` (slot 5, func 11170).
4. **Falha**: uma exceção durante a gravação do BU (contexto "Gerando boletim de urna na MI", unidade u09) terminaria na
   tela de erro fatal do §4 numa urna real. Na build web, nada disso é alcançado.

## 11. Particularidades da build web

* Não iniciados ou não alcançados: `CThreadMonitor`, a cadeia do início do dia (`CVerificaHorarioZeresima`, teste de teclado,
  zerésima, `CInicioVotacao`), a cadeia do fim do dia (sincronização do BU, `CValidaMidia`) e os tratadores de `CThreadVota`.
* Os singletons de plataforma usados por este código são mocks. `IPower` = `api::teste::CPowerMock` (rede elétrica, bateria cheia).
  `IUrna` = `CUrnaMock` (modelo 2020). `IImpressoraRelatorios` = `CWasmNullPrinter`. `IInterfaceSavd` = `(anon)::CWasmSavd`
  (não assina/valida nada). `IInterfaceInit` = `CWasmInit` (aplicativo 10).
* `/proc/meminfo`, `/dsk/mr/`, `/dev/urna` não existem no MEMFS: os logs de memória seriam 0, o pendrive "ausente", e
  `LogInfo`/`DebugUenux` silenciosos (sem a variável de ambiente `DEBUG_UENUX`).

## 12. Observações sobre WebAssembly / Emscripten

* **Threads que não podem existir.** Sem pthreads, o compilador provou que `std::thread`/`std::async` sempre lançam exceção.
  `SaiPorVotacaoSuspensa`, `CApplication::ShowExceptionMsg` (5568) e `api::CTimer::Start` (10850) ficam reduzidas a
  "alocar o estado compartilhado + `std::__thread_struct` (2541) + lançar `std::system_error("thread constructor failed")`",
  e **tudo o que vinha depois da chamada foi apagado** (o desenho da tela de erro de 5568, o resto de `SaiPorVotacaoSuspensa`).
  É por isso que o registro `:117` não tem mais nenhuma referência.
* `std::this_thread::sleep_for(500ms)` em `CThreadMonitor::Run` virou `if (byte@1584624 == 1) emscripten_sleep(500)`,
  que aborta nesta build (sem Asyncify).
* **merge-similar-functions**: `CLogVota` 3902 é o corpo de todo literal de log de 38 bytes. Os seus cinco blocos de 8 bytes viraram
  parâmetros ponteiro: 5880 "Imprimindo relatório de estado da urna", 5881 "Erro estado do aplicativo não esperado",
  5885 "Mídia de resultado não estava presente". Os thunks de `GetTelaCargoAtual` 5920/5923/5931 passam o srcloc e o código
  de erro para os corpos compartilhados 6050/3921. O construtor do contexto genérico 5558 e os singletons preguiçosos 6051/764 são o mesmo
  tipo de mesclagem.
* **Eliminação de argumentos mortos / propagação de constantes**: `CPaperFormBuilder::Show` mantém um `bool` no nome RTTI da sua lambda,
  mas o corpo wasm tem só dois parâmetros, porque todo chamador passa `true`.
* **`size_t` do wasm32**: `FormataTamanho(size_t)` trunca as contagens de 64 bits de statvfs/memória módulo 4 GiB.
* **Detalhes da ABI v2 da libc++**: `std::filesystem::space` inlinado via `__space` (4677); construtor de `std::map` por lista de
  inicialização (6579) para o layout do teclado; `__tree::destroy` (4148); wrapper de `boost::regex_search` não ancorado (2199)
  que engole exceções.

## 13. Código suspeito ou estranho

1. **Teste de teclado com falha registrado como sucesso** (11804 → 4511): uma tecla errada grava "Fim do teste de Teclado do TE -
   Sucesso" em `logd.dat`. Não existe nenhum registro de falha em lugar nenhum do binário.
2. **Verificação fraca de arquivos permitidos no pendrive de resultado** (5572/5465): os padrões `#regex#` são buscados, não ancorados. Por
   exemplo, `[0-9]{2}ue` aceita qualquer nome que *contenha* dois dígitos seguidos de "ue", e `[0-9]{8}\.[ste|STE]` é uma
   classe de caracteres (aceita `12345678.|` ou `12345678.e…`).
3. **Possível leitura além do limite do heap em `LeValorMemInfo`** (5463): um buffer de 4096 bytes, `read(…, 4096)`, depois `strlen(buffer)`.
   Se `/proc/meminfo` tiver 4096 bytes ou mais, não há NUL, e `strlen` passa do fim da alocação.
4. **Tamanhos truncados no log** (1947): parâmetro `size_t` no wasm32 (§12).
5. **Contexto de fallback trocado** (1695): "Pilha de contexto vazia" se torna o *título* (contexto +12, impresso como
   "<título> (<código>)") e "Erro inesperado" o detalhe (+0). `InitApplication` usa "Erro inesperado" como título
   (detalhe "Não é possível continuar a execução"). O fato no binário é certo; que se trate de uma troca não intencional é uma
   inferência.
6. **Tratadores de exceção inalcançáveis** (7709/7710): nenhum try/catch despacha para os slots 3/4 de `CThreadVota` neste binário.
   Na urna, uma exceção que escapasse de `Run()` terminaria o programa em vez de mostrar a página de erro, a menos que a build
   nativa seja diferente.
7. **Diagnóstico enganoso** (11172): quando o pendrive é inválido, o log lista os nomes *esperados*, não os arquivos que
   causaram a rejeição.
8. **Vias ilimitadas do estado da urna a partir de "Mais informações"** (11911): não há verificação contra `numRelatorioEstado`, ao contrário de
   11869. O próprio menu (`CMaisInformacoes::ProcessInput`, 11896) também não restringe a opção 1: ele mapeia o número
   digitado 1..5 diretamente para os estados dos itens.
9. **Monitoramento suprimido após 20 eventos** (`VerificaAlimentacao`): mudanças de fonte repetidas silenciam todos os logs de energia
   posteriores (por projeto, contra inundação de logs).
10. **Versão de desenvolvimento fixa no papel** (5591): o relatório de estado da urna imprime o literal @326597 por inteiro,
    "Ver: 10.23.0.1 - DESENVOLVIMENTO". O rodapé de versões dos pacotes (5596) e a lista de eleitores (11908) passam o mesmo
    literal por `api::CStringUtils::GetVersionNumber` (regex `^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+`) e imprimem apenas
    "Ver: 10.23.0.1".

## 14. Questões em aberto

* O arquivo real de `CThreadVota` (`vota/comum/` aqui, `vota/` no include da u07) e de `FormataTamanho`.
* Os nomes dos slots 1..4 de IPower (tensões/corrente) e dos bits de status além dos impressos.
* `CCargo +14` (pula a verificação de voto repetido) e `CCandidatura +52` (flag de inapto) precisam de confirmação no modelo de dados.
* `ETipoUrnaOperacao '2'` no construtor de `CRelatorioTesteImpressora` (uso do pleito 2).
* O significado da flag de `CSubReport` e do slot 9 de IImpressoraRelatorios em `CLp::Imprime` / dos slots 5, 6 de CSubReport.
* Se a build nativa mantém, em torno de `Run()`, um try/catch que chama os slots 3/4.

## 15. Tabela de mapeamento completa (110 funções)

* **executou**: ✓ = observada em execução nos votos gravados (`analysis/runtime/*.functions.tsv`).
* **reconstruído em**: caminho em `src/uenux2/src/`. "biblioteca" = código da libc++, não reconstruído.

| idx | tamanho | executou | nome nas ferramentas | símbolo reconstruído | arquivo original | reconstruído em | tipo / conf. |
|---|---|---|---|---|---|---|---|
| 184 | 2232 | ✓ | `vota::CLogVota::GetInst` | `vota::CLogVota::GetInst` | uenux2/src/app/vota/log/clogvota.cpp | app/vota/log/clogvota.cpp | TSE (srcloc :37) / alta |
| 479 | 24 | ✓ | `api_f479` | `api::CDateTime::CDateTime() [current local time]` | uenux2/src/api/util/cdatetime.cpp | api/u26-foreign-fragments.cpp | helper do TSE / média |
| 759 | 36 | ✓ | `vota_f759` | `api::CDateTime::Compare` | uenux2/src/api/util/cdatetime.cpp | api/u26-foreign-fragments.cpp | helper do TSE / média |
| 940 | 77 |  | `api_f940` | `api::IField::SetEstado` | uenux2/src/api/gui/ifield.h (caminho inferido) | api/u26-foreign-fragments.cpp | helper de GUI do TSE / média |
| 949 | 15 | ✓ | `comum_f949` | `comum::CPath::GetPathMR` | uenux2/src/app/comum/cpath.cpp | app/comum/cpath.cpp (unidade u22); nota em app/comum/u26-foreign-fragments.cpp | TSE / alta |
| 1156 | 169 |  | `comum_f1156` | `comum::CRelUtil::IncluiSeparador` | uenux2/src/app/comum/relatorios/crelutil.cpp (caminho inferido) | app/comum/u26-foreign-fragments.cpp | helper de relatório do TSE / baixa |
| 1264 | 371 |  | `comum_f1264` | `api::CPaperFormBuilder::AddCut` | uenux2/src/api/gui/cpaperformbuilder.h (caminho inferido) | api/u26-foreign-fragments.cpp | helper de relatório do TSE / média |
| 1280 | 123 |  | `api_f1280` | `vota::CMaisInformacoes::GetInst` | uenux2/src/app/vota/eleitor/iniciovotacao/cmaisinformacoes.cpp (caminho inferido) | app/vota/u26-foreign-fragments.cpp | TSE (singleton preguiçoso) / média |
| 1398 | 14 |  | `api_f1398` | `comum::IEventosLog::LogaAviso` | uenux2/src/app/comum/log/ieventoslog.h | nota em app/comum/u26-foreign-fragments.cpp | TSE inline (cópia de severidade 2) / média |
| 1685 | 136 |  | `vota::CSincronizaVota::VerificaUrnaDesligando` | `vota::CSincronizaVota::VerificaUrnaDesligando [throw-expression: api::CUeDesligandoError ctor]` | uenux2/src/app/vota/comum/csincronizavota.cpp | app/vota/comum/csincronizavota.cpp | TSE (srcloc :47) / alta |
| 1695 | 354 |  | `vota_f1695` | `api::CApplicationContextStack::Top` | uenux2/src/api/gui/capplicationcontextstack.cpp (caminho inferido) | api/u26-foreign-fragments.cpp | TSE / média |
| 1836 | 5000 |  | `vota::CSincronizaVota::SincronizaRelatorios` | `vota::CSincronizaVota::SincronizaRelatorios` | uenux2/src/app/vota/comum/csincronizavota.cpp | app/vota/comum/csincronizavota.cpp | TSE (srcloc :392) / alta |
| 1898 | 450 |  | `vota::CThreadMonitor::GetInst` (antes mostrada pelas ferramentas como `comum::util::CMonitoraAlimentacao::CreateInst`) | `vota::CThreadMonitor::GetInst` | uenux2/src/app/vota/monitor/cthreadmonitor.cpp | app/vota/monitor/cthreadmonitor.cpp (construtor inlinado; CMonitoraAlimentacao::CreateInst em app/comum/util/cmonitoraalimentacao.cpp) | TSE (srclocs cthreadmonitor.cpp:52, cmonitoraalimentacao.cpp:29 inlinados) / média |
| 1947 | 1448 |  | `vota_f1947` | `comum::util::FormataTamanho` | uenux2/src/app/comum/util/formatatamanho.cpp (caminho e nome inferidos) | app/comum/util/formatatamanho.cpp | helper do TSE / baixa |
| 2126 | 64 |  | `vota::CThreadMonitor::vf0` | `vota::CThreadVota::~CThreadVota` | uenux2/src/app/vota/comum/cthreadvota.cpp (caminho inferido) | app/vota/comum/cthreadvota.cpp | TSE (destrutor, slot 0 de CThreadVota e CThreadMonitor) / alta |
| 2541 | 39 |  | `vota_f2541` | `std::__thread_struct::__thread_struct` | biblioteca: libcxx/src/thread.cpp | comentário em api/u26-foreign-fragments.cpp | biblioteca (suporte a std::thread / std::async) / média |
| 2758 | 507 |  | `comum_f2758` | `comum::impl::(anonymous)::TodosCorrespondem` | uenux2/src/app/comum/validamidia/cvalidamidia.cpp | app/comum/validamidia/cvalidamidia.cpp | TSE / média |
| 2784 | 37 |  | `comum_f2784` | `comum::impl::(anonymous)::ConteudoValido` | uenux2/src/app/comum/validamidia/cvalidamidia.cpp | app/comum/validamidia/cvalidamidia.cpp | TSE / média |
| 2868 | 171 |  | `vota::testeteclado::CTesteTeclado::vf0` | `vota::testeteclado::CTesteTeclado::~CTesteTeclado` | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.cpp | app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.cpp | TSE (destrutor) / alta |
| 2882 | 465 |  | `vota::CRelVotaUtil::CortaPapel` | `vota::CRelVotaUtil::CortaPapel` | uenux2/src/app/vota/comum/crelvotautil.cpp (caminho inferido) | app/vota/u26-foreign-fragments.cpp | TSE / alta |
| 3054 | 290 |  | `api_f3054` | `vota::testeteclado::(anonymous)::AdicionaCabecalhoTesteTeclado` | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.cpp | app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.cpp | GUI do TSE / baixa |
| 3268 | 538 |  | `vota_f3268` | `vota::CLogVota::LogaQuantidade` | uenux2/src/app/vota/log/clogvota.cpp | app/vota/log/clogvota.cpp | TSE / baixa |
| 3288 | 356 |  | `vota::VerificaAssinaturaMV` | `vota::VerificaAssinaturaMV` | uenux2/src/app/vota/comum/votadefs.cpp | app/vota/comum/votadefs.cpp | TSE (srcloc :110) / alta |
| 3290 | 356 |  | `vota::VerificaAssinaturaMI` | `vota::VerificaAssinaturaMI` | uenux2/src/app/vota/comum/votadefs.cpp | app/vota/comum/votadefs.cpp | TSE (srcloc :103) / alta |
| 3336 | 12 |  | `vota_f3336` | `vota::CSincronizaVota::MarcaUrnaDesligando` | uenux2/src/app/vota/comum/csincronizavota.h | app/vota/comum/csincronizavota.h (inline) | TSE / média |
| 3660 | 116 |  | `vota_f3660` | `api::CSubReport::CSubReport` | uenux2/src/api/gui/reports/csubreport.cpp (caminho inferido) | api/u26-foreign-fragments.cpp | helper de relatório do TSE / média |
| 3671 | 483 |  | `comum_f3671` | `api::CPaperFormBuilder::Show` | uenux2/src/api/gui/cpaperformbuilder.cpp (caminho inferido) | api/u26-foreign-fragments.cpp | TSE (o RTTI da lambda dá a assinatura) / alta |
| 3855 | 143 |  | `vota_f3855` | `vota::testeteclado::CTesteTeclado::GetInst` | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.cpp | app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.cpp | TSE (singleton preguiçoso) / alta |
| 3865 | 22 |  | `vota_f3865` | `vota::CInicioZeresima::GetInst` | uenux2/src/app/vota/eleitor/iniciovotacao/ciniciozeresima.cpp (caminho inferido) | nota em app/vota/u26-foreign-fragments.cpp | TSE (thunk de singleton mesclado) / alta |
| 3875 | 82 |  | `vota_f3875` | `api::CLp::Imprime` | uenux2/mock/app/simulador/wasm/cwasmclp.cpp (ou api/print/clp.cpp; caminho inferido) | api/u26-foreign-fragments.cpp | TSE / baixa |
| 3902 | 145 |  | `vota_f3902` | `vota::CLogVota::Loga38 [merged body of 38-byte log literals]` | uenux2/src/app/vota/log/clogvota.cpp | app/vota/log/clogvota.cpp | corpo mesclado pelo wasm-opt / média |
| 4148 | 53 |  | `api_f4148` | `std::__tree<std::string,...>::destroy (keypad layout map)` | biblioteca: libcxx/include/__tree | comentário em app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.cpp | instanciação de biblioteca / média |
| 4511 | 150 |  | `vota_f4511` | `vota::CLogVota::LogaFimTesteTecladoSucesso` | uenux2/src/app/vota/log/clogvota.cpp | app/vota/log/clogvota.cpp | TSE / média |
| 4582 | 157 |  | `vota::GetQuantidadeMaximaBUsAdicionais` | `vota::GetQuantidadeMaximaBUsAdicionais` | uenux2/src/app/vota/comum/votadefs.cpp | app/vota/comum/votadefs.cpp | TSE (srcloc :135) / alta |
| 4633 | 675 |  | `vota_f4633` | `vota::(anonymous)::MostraUrnaInoperanteNoMicroterminal` | uenux2/src/app/vota/comum/cthreadvota.cpp (caminho inferido) | app/vota/comum/cthreadvota.cpp | TSE / baixa |
| 4677 | 494 |  | `vota_f4677` | `std::filesystem::__space` | biblioteca: libcxx/src/filesystem/operations.cpp | comentário em app/vota/monitor/cthreadmonitor.cpp | biblioteca / alta |
| 5448 | 23 |  | `vota_f5448` | `api::CTime::Compare` | uenux2/src/api/util/ctime.cpp | api/u26-foreign-fragments.cpp | helper do TSE / média |
| 5453 | 132 |  | `vota_f5453` | `vota::(anonymous)::EspacoUtilizado` | uenux2/src/app/vota/monitor/cthreadmonitor.cpp | app/vota/monitor/cthreadmonitor.cpp | TSE / baixa |
| 5454 | 122 |  | `vota_f5454` | `vota::(anonymous)::EspacoLivre` | uenux2/src/app/vota/monitor/cthreadmonitor.cpp | app/vota/monitor/cthreadmonitor.cpp | TSE / baixa |
| 5463 | 1093 |  | `vota_f5463` | `vota::(anonymous)::LeValorMemInfo` | uenux2/src/app/vota/monitor/cthreadmonitor.cpp | app/vota/monitor/cthreadmonitor.cpp | TSE / baixa |
| 5464 | 110 |  | `comum_f5464` | `comum::impl::(anonymous)::CaractereCoringa` | uenux2/src/app/comum/validamidia/cvalidamidia.cpp | app/comum/validamidia/cvalidamidia.cpp | TSE / média |
| 5465 | 258 |  | `comum_f5465` | `comum::impl::(anonymous)::BuscaRegex` | uenux2/src/app/comum/validamidia/cvalidamidia.cpp | app/comum/validamidia/cvalidamidia.cpp | TSE / média |
| 5471 | 242 |  | `vota_f5471` | `api::CDateTime::DiferencaSegundos` | uenux2/src/api/util/cdatetime.cpp | api/u26-foreign-fragments.cpp | helper do TSE / baixa |
| 5568 | 100 |  | `vota_f5568` | `api::CApplication::ShowExceptionMsg` | uenux2/src/api/gui/capplication.cpp | api/u26-foreign-fragments.cpp | TSE (corpo cortado após std::async) / média |
| 5569 | 217 |  | `vota_f5569` | `vota::(anonymous)::TipoContexto` | uenux2/src/app/vota/comum/cthreadvota.cpp (caminho inferido) | app/vota/comum/cthreadvota.cpp | TSE / baixa |
| 5571 | 2179 |  | `comum_f5571` | `comum::impl::(anonymous)::RemoveArquivosSO` | uenux2/src/app/comum/validamidia/cvalidamidia.cpp | app/comum/validamidia/cvalidamidia.cpp | TSE / média |
| 5572 | 2128 |  | `comum_f5572` | `comum::impl::(anonymous)::ValidaConteudo` | uenux2/src/app/comum/validamidia/cvalidamidia.cpp | app/comum/validamidia/cvalidamidia.cpp | TSE / média |
| 5574 | 16 |  | `comum_f5574` | `comum::impl::(anonymous)::LogInfo` | uenux2/src/app/comum/validamidia/cvalidamidia.cpp | app/comum/validamidia/cvalidamidia.cpp | TSE (wrapper variádico de vota_f2921) / média |
| 5586 | 205 |  | `comum_f5586` | `comum::CRelatorioTesteImpressora::CRelatorioTesteImpressora` | uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp | app/comum/u26-foreign-fragments.cpp | TSE (construtor) / alta |
| 5591 | 8678 |  | `vota_f5591` | `comum::CImprimirExtratoCarga::Imprime` | uenux2/src/app/comum/relatorios/cimprimirextratocarga.cpp (caminho inferido) | app/comum/u26-foreign-fragments.cpp | TSE (template method) / média |
| 5592 | 53 |  | `comum::CGeradorRelVersaoPacoteDados::vf2` | `comum::CGeradorRelVersaoPacoteDados::Imprime` | uenux2/src/app/comum/relatorios/cgeradorrelversaopacotedados.cpp (caminho inferido) | app/comum/u26-foreign-fragments.cpp | TSE (slot 2) / média |
| 5596 | 826 |  | `comum::CGeradorRelVersaoPacoteDados::vf5` | `comum::CGeradorRelVersaoPacoteDados::MontaRodape` | uenux2/src/app/comum/relatorios/cgeradorrelversaopacotedados.cpp (caminho inferido) | app/comum/u26-foreign-fragments.cpp | TSE (slot 5) / baixa |
| 5880 | 29 |  | `vota_f5880` | `vota::CLogVota::LogaImprimindoRelatorioEstadoUrna` | uenux2/src/app/vota/log/clogvota.cpp | app/vota/log/clogvota.cpp | TSE / média |
| 5920 | 16 |  | `vota::CPedeMajoritario::GetTelaCargoAtual` | `vota::CPedeMajoritario::GetTelaCargoAtual` | uenux2/src/app/vota/eleitor/votamajoritario/cpedemajoritario.cpp | app/vota/eleitor/votamajoritario/cpedemajoritario.u07.cpp (corpo 6050); nota em cpedemajoritario.cpp | TSE (srcloc :44; thunk) / alta |
| 5923 | 16 | ✓ | `vota::CPedeProporcional::GetTelaCargoAtual` | `vota::CPedeProporcional::GetTelaCargoAtual` | uenux2/src/app/vota/eleitor/votaproporcional/cpedeproporcional.cpp | app/vota/eleitor/votaproporcional/cpedeproporcional.cpp (nota; corpo 6050 em cpedemajoritario.u07.cpp) | TSE (srcloc :42; thunk) / alta |
| 5930 | 522 | ✓ | `vota::CCompletaProporcional::EmiteEcoComInputField` | `vota::CCompletaProporcional::EmiteEcoComInputField` | uenux2/src/app/vota/eleitor/votaproporcional/ccompletaproporcional.cpp | app/vota/eleitor/votaproporcional/ccompletaproporcional.cpp | TSE (srcloc :60) / alta |
| 5931 | 20 | ✓ | `vota::CCompletaProporcional::GetTelaCargoAtual` | `vota::CCompletaProporcional::GetTelaCargoAtual` | uenux2/src/app/vota/eleitor/votaproporcional/ccompletaproporcional.cpp | app/vota/eleitor/votaproporcional/ccompletaproporcional.cpp (nota; corpo 3921 em cconfirmavotoemcargo.cpp, u06) | TSE (srcloc :83; thunk) / alta |
| 5932 | 28 |  | `vota_f5932` | `vota::CCompletaProporcional::CCompletaProporcional` | uenux2/src/app/vota/eleitor/votaproporcional/ccompletaproporcional.h | app/vota/eleitor/votaproporcional/ccompletaproporcional.h (construtor inline) | TSE (construtor) / média |
| 5945 | 207 |  | `vota::testeteclado::CPreZeresima::GetEstadoPassouNoTeste` | `vota::testeteclado::CPreZeresima::GetEstadoPassouNoTeste` | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cprezeresima.cpp | app/vota/eleitor/iniciovotacao/testeteclado/cprezeresima.cpp | TSE (srcloc :56) / alta |
| 5972 | 108 |  | `vota_f5972` | `vota::(anonymous)::IniciaVotacao` | uenux2/src/app/vota/eleitor/iniciovotacao/ciniciovotacao.cpp (caminho inferido) | app/vota/u26-foreign-fragments.cpp | TSE / baixa |
| 6579 | 572 |  | `api_f6579` | `std::map<std::string, SLayoutTecla>::map(initializer_list)` | biblioteca: libcxx/include/map | comentário em app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.cpp | instanciação de biblioteca / média |
| 6580 | 201 |  | `vota_f6580` | `vota::CTelasVota::CriaTelaTesteTeclado` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | app/vota/eleitor/comum/ctelasvota.u26.cpp | TSE / média |
| 6581 | 211 |  | `vota_f6581` | `vota::CTelasVota::CriaTelaTesteTecladoOpcional` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | app/vota/eleitor/comum/ctelasvota.u26.cpp | TSE / média |
| 7709 | 2883 |  | `vota::CThreadMonitor::vf4` | `vota::CThreadVota::TrataExcecaoDesconhecida` | uenux2/src/app/vota/comum/cthreadvota.cpp (caminho inferido) | app/vota/comum/cthreadvota.cpp | TSE (slot 4 da vtable) / média |
| 7710 | 2317 |  | `vota::CThreadMonitor::vf3` | `vota::CThreadVota::TrataExcecao` | uenux2/src/app/vota/comum/cthreadvota.cpp (caminho inferido) | app/vota/comum/cthreadvota.cpp | TSE (slot 3 da vtable) / média |
| 10225 | 18 |  | `vota::CThreadMonitor::vf5` | `vota::CThreadMonitor::FinalizaExecucao` | uenux2/src/app/vota/monitor/cthreadmonitor.cpp | app/vota/monitor/cthreadmonitor.cpp | TSE (slot 5 da vtable) / alta |
| 10226 | 7171 |  | `vota::CThreadMonitor::vf2` | `vota::CThreadMonitor::Run` | uenux2/src/app/vota/monitor/cthreadmonitor.cpp | app/vota/monitor/cthreadmonitor.cpp | TSE (slot 2 da vtable; muitos srclocs) / alta |
| 10227 | 13 |  | `vota::CThreadMonitor::vf1` | `vota::CThreadMonitor::~CThreadMonitor [deleting]` | uenux2/src/app/vota/monitor/cthreadmonitor.cpp | app/vota/monitor/cthreadmonitor.cpp (comentário) | gerado pelo compilador (slot 1) / alta |
| 10229 | 38 |  | `vota_f10229` | `vota::CThreadMonitor::s_instancia at-exit reset` | uenux2/src/app/vota/monitor/cthreadmonitor.cpp | app/vota/monitor/cthreadmonitor.cpp (comentário) | gerado pelo compilador / média |
| 10416 | 176 |  | `vota::CPerguntaEleitorVotando::vf2` | `vota::CPerguntaEleitorVotando::StartState` | uenux2/src/app/vota/operador/... (arquivo desconhecido) | app/vota/u26-foreign-fragments.cpp | TSE (slot 2 da vtable) / média |
| 10642 | 164 |  | `vota::CEleitorJaVotou::vf2` | `vota::CEleitorJaVotou::StartState` | uenux2/src/app/vota/operador/... (arquivo desconhecido) | app/vota/u26-foreign-fragments.cpp | TSE (slot 2 da vtable) / média |
| 10774 | 409 |  | `vota::CControladorRegistraMesariosVota::vf28` | `vota::CControladorRegistraMesariosVota::LogaLimiteMesariosAtingido` | uenux2/src/app/vota/operador/... (arquivo desconhecido) | app/vota/u26-foreign-fragments.cpp | TSE (slot 28 da vtable) / baixa |
| 10850 | 80 |  | `api::CTimer::vf2` | `api::CTimer::Start` | uenux2/src/api/util/ctimer.cpp (caminho inferido) | api/u26-foreign-fragments.cpp | TSE (slot 2 da vtable) / baixa |
| 11166 | 138 |  | `comum::impl::CValidaMidia::vf9` | `comum::impl::CValidaMidia::RemoveArquivosSistemaOperacional` | uenux2/src/app/comum/validamidia/cvalidamidia.cpp | app/comum/validamidia/cvalidamidia.cpp | TSE (slot 9) / média |
| 11167 | 1260 |  | `comum::impl::CValidaMidia::vf8` | `comum::impl::CValidaMidia::MidiaContemSomenteResultados` | uenux2/src/app/comum/validamidia/cvalidamidia.cpp | app/comum/validamidia/cvalidamidia.cpp | TSE (slot 8) / baixa |
| 11168 | 414 |  | `comum::impl::CValidaMidia::vf7` | `comum::impl::CValidaMidia::MidiaContemArquivo` | uenux2/src/app/comum/validamidia/cvalidamidia.cpp | app/comum/validamidia/cvalidamidia.cpp | TSE (slot 7) / baixa |
| 11169 | 388 |  | `comum::impl::CValidaMidia::vf6` | `comum::impl::CValidaMidia::MidiaContemResultados` | uenux2/src/app/comum/validamidia/cvalidamidia.cpp | app/comum/validamidia/cvalidamidia.cpp | TSE (slot 6; nome como na u09) / média |
| 11170 | 887 |  | `comum::impl::CValidaMidia::vf5` | `comum::impl::CValidaMidia::MidiaContemResultadosSecao` | uenux2/src/app/comum/validamidia/cvalidamidia.cpp | app/comum/validamidia/cvalidamidia.cpp | TSE (slot 5) / baixa |
| 11171 | 259 |  | `comum::impl::CValidaMidia::vf4` | `comum::impl::CValidaMidia::MidiaSemArquivosIndevidos` | uenux2/src/app/comum/validamidia/cvalidamidia.cpp | app/comum/validamidia/cvalidamidia.cpp | TSE (slot 4) / baixa |
| 11172 | 2432 |  | `comum::impl::CValidaMidia::vf3` | `comum::impl::CValidaMidia::ValidaMidiaResultado` | uenux2/src/app/comum/validamidia/cvalidamidia.cpp | app/comum/validamidia/cvalidamidia.cpp (+ cfabricaconteudomidiastart.cpp inlinado, srcloc :62) | TSE (slot 3; nome como na u09) / média |
| 11173 | 47 |  | `comum::impl::CValidaMidia::vf2` | `comum::impl::CValidaMidia::MidiaResultadoValida` | uenux2/src/app/comum/validamidia/cvalidamidia.cpp | app/comum/validamidia/cvalidamidia.cpp | TSE (slot 2) / baixa |
| 11682 | 191 |  | `vota::CPedeMajoritario::vf15` | `vota::CPedeMajoritario::GetMensagemAudio` | uenux2/src/app/vota/eleitor/votamajoritario/cpedemajoritario.cpp | app/vota/eleitor/votamajoritario/cpedemajoritario.cpp | TSE (slot 15) / alta |
| 11683 | 1210 | ✓ | `vota::CPedeMajoritario::ProcessInputAudio` | `vota::CPedeMajoritario::ProcessInputAudio` | uenux2/src/app/vota/eleitor/votamajoritario/cpedemajoritario.cpp | app/vota/eleitor/votamajoritario/cpedemajoritario.cpp | TSE (srcloc :64, :125) / alta |
| 11684 | 103 | ✓ | `vota::CPedeMajoritario::vf10` | `vota::CPedeMajoritario::StartStateAudio` | uenux2/src/app/vota/eleitor/votamajoritario/cpedemajoritario.cpp | app/vota/eleitor/votamajoritario/cpedemajoritario.cpp | TSE (slot 10) / alta |
| 11710 | 51 |  | `vota::CPedeProporcional::vf15` | `vota::CPedeProporcional::GetMensagemAudio` | uenux2/src/app/vota/eleitor/votaproporcional/cpedeproporcional.cpp | app/vota/eleitor/votaproporcional/cpedeproporcional.cpp | TSE (slot 15) / alta |
| 11711 | 811 | ✓ | `vota::CPedeProporcional::ProcessInputAudio` | `vota::CPedeProporcional::ProcessInputAudio` | uenux2/src/app/vota/eleitor/votaproporcional/cpedeproporcional.cpp | app/vota/eleitor/votaproporcional/cpedeproporcional.cpp | TSE (srcloc :84) / alta |
| 11712 | 103 | ✓ | `vota::CPedeProporcional::vf10` | `vota::CPedeProporcional::StartStateAudio` | uenux2/src/app/vota/eleitor/votaproporcional/cpedeproporcional.cpp | app/vota/eleitor/votaproporcional/cpedeproporcional.cpp | TSE (slot 10) / alta |
| 11744 | 433 |  | `vota::CPedeNulo::GetProximoEstado` | `vota::CPedeNulo::GetProximoEstado` | uenux2/src/app/vota/eleitor/votaproporcional/cpedenulo.cpp | app/vota/eleitor/votaproporcional/cpedenulo.cpp | TSE (srcloc :44) / alta |
| 11745 | 52 |  | `vota::CPedeNulo::vf15` | `vota::CPedeNulo::GetMensagemAudio` | uenux2/src/app/vota/eleitor/votaproporcional/cpedenulo.cpp | app/vota/eleitor/votaproporcional/cpedenulo.cpp | TSE (slot 15) / alta |
| 11756 | 786 | ✓ | `vota::CCompletaProporcional::vf9` | `vota::CCompletaProporcional::ProcessInputAudio` | uenux2/src/app/vota/eleitor/votaproporcional/ccompletaproporcional.cpp | app/vota/eleitor/votaproporcional/ccompletaproporcional.cpp | TSE (slot 9; literal __PRETTY_FUNCTION__) / alta |
| 11757 | 267 | ✓ | `vota::CCompletaProporcional::vf10` | `vota::CCompletaProporcional::StartStateAudio` | uenux2/src/app/vota/eleitor/votaproporcional/ccompletaproporcional.cpp | app/vota/eleitor/votaproporcional/ccompletaproporcional.cpp | TSE (slot 10; literal __PRETTY_FUNCTION__) / alta |
| 11804 | 1437 |  | `vota::testeteclado::CTesteTeclado::ProcessInput` | `vota::testeteclado::CTesteTeclado::ProcessInput` | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.cpp | app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.cpp | TSE (srcloc :50/:60/:81) / alta |
| 11805 | 10638 |  | `vota::testeteclado::CTesteTeclado::StartState` | `vota::testeteclado::CTesteTeclado::StartState` | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.cpp | app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.cpp (+ ctelasvota.u26.cpp GetKeyboardLayout) | TSE (srcloc :38/:42/:121) / alta |
| 11806 | 13 |  | `vota::testeteclado::CTesteTeclado::vf1` | `vota::testeteclado::CTesteTeclado::~CTesteTeclado [deleting]` | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.cpp | app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.cpp (comentário) | gerado pelo compilador (slot 1) / alta |
| 11808 | 38 |  | `vota_f11808` | `vota::testeteclado::CTesteTeclado s_instancia at-exit reset` | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.cpp | app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.h (comentário) | gerado pelo compilador / média |
| 11823 | 67 |  | `vota::testeteclado::CEsperaRetestar::vf9` | `vota::testeteclado::CEsperaRetestar::ProcessTickNaoDesligamento` | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cesperaretestar.cpp (caminho inferido) | app/vota/eleitor/iniciovotacao/testeteclado/cesperaretestar.u26.cpp | TSE (slot 9) / média |
| 11826 | 483 |  | `vota_f11826` | `vota::testeteclado::TextoEsperaRetestar` | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cesperaretestar.cpp (caminho inferido) | app/vota/eleitor/iniciovotacao/testeteclado/cesperaretestar.u26.cpp | TSE (fonte de texto, slot de tabela 1862) / baixa |
| 11842 | 5 |  | `vota::CRegerarZeresima::vf9` | `vota::CRegerarZeresima::AposImpressao` | uenux2/src/app/vota/eleitor/iniciovotacao/cregerarzeresima.cpp (caminho inferido) | app/vota/u26-foreign-fragments.cpp | TSE (slot 9) / baixa |
| 11861 | 70 |  | `vota::testeteclado::CPreZeresima::vf12` | `vota::testeteclado::CPreZeresima::GetEstadoSemTeste` | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cprezeresima.cpp | app/vota/eleitor/iniciovotacao/testeteclado/cprezeresima.cpp | TSE (slot 12) / média |
| 11862 | 107 |  | `vota::testeteclado::CPreZeresima::vf10` | `vota::testeteclado::CPreZeresima::CriaTela` | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cprezeresima.cpp | app/vota/eleitor/iniciovotacao/testeteclado/cprezeresima.cpp | TSE (slot 10) / média |
| 11868 | 13 |  | `vota::CVerificaHorarioZeresima::vf5` | `vota::CVerificaHorarioZeresima::FinishState` | uenux2/src/app/vota/eleitor/iniciovotacao/cverificahorariozeresima.cpp | app/vota/eleitor/iniciovotacao/cverificahorariozeresima.cpp | TSE (slot 5) / alta |
| 11869 | 861 |  | `vota::CVerificaHorarioZeresima::vf7` | `vota::CVerificaHorarioZeresima::ProcessInput` | uenux2/src/app/vota/eleitor/iniciovotacao/cverificahorariozeresima.cpp | app/vota/eleitor/iniciovotacao/cverificahorariozeresima.cpp | TSE (slot 7) / alta |
| 11870 | 124 |  | `vota::CVerificaHorarioZeresima::ProcessTickNaoDesligamento` | `vota::CVerificaHorarioZeresima::ProcessTickNaoDesligamento` | uenux2/src/app/vota/eleitor/iniciovotacao/cverificahorariozeresima.cpp | app/vota/eleitor/iniciovotacao/cverificahorariozeresima.cpp | TSE (srcloc :80) / alta |
| 11871 | 312 |  | `vota::CVerificaHorarioZeresima::StartState` | `vota::CVerificaHorarioZeresima::StartState` | uenux2/src/app/vota/eleitor/iniciovotacao/cverificahorariozeresima.cpp | app/vota/eleitor/iniciovotacao/cverificahorariozeresima.cpp | TSE (srcloc :64) / alta |
| 11908 | 4670 |  | `vota::CImpressaoListaEleitores::vf2` | `vota::CImpressaoListaEleitores::StartState` | uenux2/src/app/vota/eleitor/iniciovotacao/cimpressaolistaeleitores.cpp (caminho inferido) | app/vota/u26-foreign-fragments.cpp (condensado) | TSE (slot 2) / média |
| 11911 | 565 |  | `vota::CImpressaoEstadoUrna::vf2` | `vota::CImpressaoEstadoUrna::StartState` | uenux2/src/app/vota/eleitor/iniciovotacao/cimpressaoestadourna.cpp (caminho inferido) | app/vota/u26-foreign-fragments.cpp | TSE (slot 2) / alta |
| 11927 | 337 |  | `vota::CConfirmaImpressaoZeresima::vf7` | `vota::CConfirmaImpressaoZeresima::ProcessInput` | uenux2/src/app/vota/eleitor/iniciovotacao/cconfirmaimpressaozeresima.cpp (caminho inferido) | app/vota/u26-foreign-fragments.cpp | TSE (slot 7) / alta |
| 11987 | 70 |  | `vota::CInicioVotacao::vf8` | `vota::CInicioVotacao::ProcessTick` | uenux2/src/app/vota/eleitor/iniciovotacao/ciniciovotacao.cpp (caminho inferido) | app/vota/u26-foreign-fragments.cpp | TSE (slot 8) / média |
| 11988 | 117 |  | `vota::CInicioVotacao::vf2` | `vota::CInicioVotacao::StartState` | uenux2/src/app/vota/eleitor/iniciovotacao/ciniciovotacao.cpp (caminho inferido) | app/vota/u26-foreign-fragments.cpp | TSE (slot 2) / média |
| 13012 | 475 |  | `vota_f13012` | `vota::(anonymous)::TextoSegundosParaDesligamento` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | app/vota/eleitor/comum/ctelasvota.u26.cpp | TSE (fonte de texto, slot de tabela 1102) / baixa |
