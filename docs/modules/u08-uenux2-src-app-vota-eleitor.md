# u08 — `uenux2/src/app/vota/eleitor`: áudio do eleitor e os estados do BU / encerramento

A unidade u08 cobre 107 funções wasm atribuídas a cinco arquivos originais da aplicação VOTA
(o programa da urna para o dia da eleição):

| arquivo original | o que é |
|---|---|
| `uenux2/src/app/vota/eleitor/cvotacaostateaudio.cpp` | `vota::CVotacaoStateAudio`, a classe base de toda tela de votação por cargo. Ela fala mensagens para eleitores cegos ou com baixa visão, repete essas mensagens e ecoa as teclas |
| `uenux2/src/app/vota/eleitor/fimvotacao/cgerabu.cpp` | `vota::CGeraBU`: renderiza o relatório **Boletim de Urna** (BU) no encerramento da votação |
| `uenux2/src/app/vota/eleitor/fimvotacao/cgerarelatorios.cpp` | `vota::CGeraRelatorios`: renderiza os demais relatórios de fim do dia (BUJ, BIM, BEHB) |
| `uenux2/src/app/vota/eleitor/fimvotacao/ccopiaresultadoparamr.cpp` | `vota::CCopiaResultadoParaMR`: copia os arquivos de resultado para a mídia de resultado (MR, um pendrive USB) e verifica a cópia |
| `uenux2/src/app/vota/eleitor/fimvotacao/cemitirmaisbu.cpp` | `vota::CEmitirMaisBU`: pergunta ao mesário quantas vias extras do BU imprimir |

A unidade também recebeu funções auxiliares de outros arquivos: helpers de relatório de `comum/relatorios`,
métodos de log de `CLogVota`, `CInformacaoEleitor::GetInst`, destrutores at-exit e **50 instâncias de template do
`<regex>` da libc++** usadas pelo código de áudio (e compartilhadas com quatro outros módulos, §8). Todas elas
estão listadas na [§11](#11-tabela-de-mapeamento-completa-107-funções).

Arquivos-fonte reconstruídos (C++ legível, um comentário `// wasm func N` por função):

* `src/uenux2/src/app/vota/eleitor/cvotacaostateaudio.{h,cpp}`
* `src/uenux2/src/app/vota/eleitor/fimvotacao/cgerabu.{h,cpp}`
* `src/uenux2/src/app/vota/eleitor/fimvotacao/cgerarelatorios.{h,cpp}`
* `src/uenux2/src/app/vota/eleitor/fimvotacao/ccopiaresultadoparamr.{h,cpp}`
* `src/uenux2/src/app/vota/eleitor/fimvotacao/cemitirmaisbu.{h,cpp}`

## 1. Glossário

| termo | significado |
|---|---|
| eleitor | quem vota. O código "eleitor" roda no terminal do eleitor (a própria urna) |
| mesário / operador | membro da mesa receptora, que trabalha no *terminal do mesário* |
| cargo, escolha, vaga | cargo em disputa, uma das escolhas para ele (por exemplo, duas vagas no Senado), vaga |
| BU (Boletim de Urna) | resultado por seção: votos por candidato, partido, brancos e nulos, mais o comparecimento. Impresso, assinado, gravado nas mídias de resultado e publicado como QR codes |
| BUJ, BIM, BEHB | Boletim de Justificativa (eleitores que justificaram a ausência), Boletim de Identificação de Mesários (mesários presentes), relatório de Eleitores com Habilitação Biográfica (eleitores habilitados sem reconhecimento da impressão digital) |
| vias | cópias impressas de um relatório |
| zerésima | relatório impresso antes da votação que mostra todos os contadores zerados |
| encerramento | fechamento da votação |
| MI / MR / MV | memória interna (flash interna) / mídia de resultado (pendrive USB, `/dsk/mr/`) / memória de votação (cartão de flash) |
| RDV | Registro Digital do Voto, o registro embaralhado de todos os votos |
| CV (código verificador) | código de verificação impresso no BU, calculado por `comum::CCalculaCV` com a chave `cv.ber.pri` |
| carga, correspondência | a carga de software/dados da urna e o registro dessa carga (`DadoCorrespondencia`: carga + seção + assinatura) |
| fase | `o`ficial / `s`imulado / `t`reinamento; `EUrnaFase` '1' simulado, '2' oficial, '3' treinamento |
| tecla indevida | tecla que não é permitida na tela atual |

## 2. Classes e hierarquia (RTTI)

```
api::CState
 └─ comum::CAppState                         (+4 m_proximoEstado, +8/+9/+10 flags: messages|keyboard|ticks)
     ├─ vota::CVotacaoStateAudio             typeinfo @1535192, vtable @1534936 (16 slots)   abstract
     │    ├─ CInstrucaoVotacaoAcessibilidade, CConfirmaVotoSemCandidato, CPedeMajoritario,
     │    │  CPedeProporcional
     │    ├─ CCompletaProporcional ─ CPedeNominal, CPedeNulo
     │    ├─ CConfirmaVotoEmCargo
     │    │    ├─ CConfirmaProporcional ─ CCandidatoInapto, CConfirmaVotoLegenda, CCandidatoInexistente,
     │    │    │                          CConfirmaVotoNominal, CProporcionalBranco, CProporcionalNulo
     │    │    └─ CConfirmaMajoritario  ─ CMajoritarioBranco, CMajoritarioNulo, CMajoritarioRepetido,
     │    │                               CMajoritarioValido                     (other units)
     │    └─ IConfereVotoEmCargo ─ CConfereVotoEmCargo<TConfirma, ETelaVotacao>  (unit u06)
     ├─ vota::CGeraBU                        typeinfo @1540424, vtable @1540372, 28 bytes
     ├─ vota::CGeraRelatorios                typeinfo @1540576, vtable @1540524, 20 bytes
     ├─ vota::CCopiaResultadoParaMR          typeinfo @1540212, vtable @1540080, 20 bytes
     └─ vota::CEmitirMaisBU                  typeinfo @1541264, vtable @1541180, 12 bytes
```

A ordem dos slots de `api::CState` é recuperada de classes que têm registros de `source_location`: [0] destrutor
completo, [1] destrutor de deleção, [2] `StartState`, [3] `NeedChangeState` (`CAppState` = `GetNextState() != this`,
func 7480), [4] `GetNextState` (retorna `+4`), [5] `FinishState`, [6] `ProcessMessage`, [7]
`ProcessInput`, [8] `ProcessTick(uebyte)`. `CVotacaoStateAudio` acrescenta [9] `ProcessInputAudio`=0,
[10] `StartStateAudio`=0, [11] `FinishStateAudio` (no-op), [12] `ProcessTickAudio`=0, [13]
`EmiteEcoComInputField`, [14] `FormataMensagem`, [15] `GetMensagemAudio`=0. Os nomes de 10, 11, 12, 15
vêm dos fontes das subclasses da unidade u06 (srcloc `CInstrucaoVotacaoAcessibilidade::StartStateAudio`,
o tipo da lambda `IConfereVotoEmCargo::ProcessTickAudio(unsigned char)::$_0`). O nome de 14 é inferido.

Os quatro estados de encerramento são singletons lazy. As suas funções `GetInst` (6129, 6084, 6174, 3879)
pertencem a outras unidades, e os seus construtores estão inlinados lá. `CGeraBU` guarda duas telas
`shared_ptr<CPreShowFormVota>`: "Votação encerrada" (`telaVotacaoEncerrada`) e "Preparando
dados para encerramento" (`telaPreparandoDadosEncerramento`). `CGeraRelatorios` guarda a segunda.
`CCopiaResultadoParaMR` guarda "Gravando o resultado na mídia" (`telaCopiaResultadoParaMR`).

## 3. `CVotacaoStateAudio`: a máquina de estados de áudio do eleitor

Layout (28 bytes): `+11 bool m_audioHabilitado`, `+12 uebyte m_tickRepeticao` (tick de 2000 ms de
`CThreadEleitor`), `+13 uebyte m_tickInicio` (1500 ms), `+16 unique_ptr<?>` (apenas destruído),
`+20 shared_ptr<api::IEsperaAudio> m_esperaFimAudio`. O construtor é a func 1785, que as ferramentas
colocaram em `celeitorvotando.cpp`.

```
StartState (7028)   m_audioHabilitado = CInformacaoEleitor::m_modoAudio (+4) != 2 (SemAudio)
                    StartStateAudio()                         [subclass: draws the screen]
                    audio ? StartTick(m_tickInicio 1.5 s)
ProcessTick (7010)  tick is m_tickInicio or m_tickRepeticao?
                      no  -> ProcessTickAudio(tick)           [subclass]
                      yes -> ISound::GetStatus()==1 (playing)?  -> return (try again next tick)
                             cancel wait, stop both ticks
                             audio ? PlayInterruptibleMessage(FormataMensagem(GetMensagemAudio()))
                             IniciarEsperaFimAudio(): m_esperaFimAudio = ISound::EsperaFimAudio(
                                 []{return false;}, [this](bool fim){ fim && audio -> StartTick(m_tickRepeticao) })
ProcessInput (7029) audio ? {cancel wait; stop ticks; ISound::Stop(); stop ticks}
                    ProcessInputAudio()                        [subclass: reads the key, echoes it]
                    GetNextState()==this && audio -> StartTick(m_tickRepeticao)
FinishState (7022)  cancel wait; stop ticks; FinishStateAudio()
```

Assim, um estado fala a sua mensagem 1.5 s depois de entrar nele. Depois que cada fala termina, ele espera 2 s
e repete a mensagem, indefinidamente, até que uma tecla seja pressionada (a tecla para o áudio e reinicia o
temporizador de 2 s) ou o estado seja deixado. A repetição pode ser ouvida com
`node tools/run/headless.mjs --scenario municipal-t1 --audio --save-audio DIR --keys "  C  91001  C    12  C    "`.
Com o áudio ligado, a primeira tela é `CInstrucaoVotacaoAcessibilidade`, que só sai com CONFIRMA,
daí o primeiro `C` (sem ele, como na versão anterior deste comando, os dígitos `91001` são pressionados na
tela de instruções e ignorados, e `12` acaba no campo de Vereador). A execução (reconferida em 2026-09-23)
passa por CPedeProporcional → CConfirmaVotoNominal → CPedeMajoritario → CMajoritarioNulo → CAguardaMensagem
(done) e produziu 20 WAVs, com arquivos consecutivos idênticos para as mensagens repetidas (por exemplo
`speech-012/013/014`, 153 484 bytes cada: o prompt de CPedeMajoritario para Prefeito, repetido durante a pausa
de 6 s). Não ocorreu nenhum abort nem exceção.

**Eco de teclas** (`PlayKey`, func 1455, srcloc 136/226):

* tecla 0 (recusada): `IBeep::BeepErro()` (slot 6 = 554 Hz por 200 ms em `CWasmBeep`) e o log
  `"Tecla indevida pressionada"`. Isso acontece mesmo com o áudio desligado, e é por isso que o log web
  `logd.dat` contém essa linha.
* com áudio: dígitos, `B` (BRANCO) e `D` (CORRIGE) falam `api::KeyName(key)` e podem ser interrompidos.
  `C` (CONFIRMA) é enfileirado e aguardado (`PlayMessage`). Uma tecla recusada toca o arquivo
  `:/resource/sounds/tecE.wav` (`PlayFile`).

**Qual tecla é recusada** (`EmiteEcoComInputField`, func 3139, srcloc 188): o formulário deve ter exatamente um
campo de entrada (caso contrário, `CBaseError<EUeVotaError>` 9318 "Nao havia campos de input no formulario sem nada
digitado"). O campo é lido (`CInteractiveForm::Read`, inlinada). Se o resultado é CORRIGE (5) ou uma
tecla digitada (13) **e o texto não mudou**, a tecla é recusada. Isso cobre um dígito quando o campo já está
cheio e CORRIGE num campo vazio. A função retorna `{result, text}`.
`EmiteEcoCorrigeConfirma` (srcloc 209, inlinada na func 11754) é usada nas telas CONFIRMA/CORRIGE.
Nelas, qualquer tecla que não seja `C`/`D` é recusada.

**Templates de mensagem** (`GetMensagemAudio`, slot 15, nas subclasses; as strings são Latin-1 porque o
RHVoice lê Latin-1):

| estado (func do slot 15) | template |
|---|---|
| CPedeProporcional (11710), CPedeMajoritario (11682) | `Você está votando para {cargo-atual}. {quantidade-digitos}. Voto {progresso}.` / `{cargo-atual} {quantidade-digitos}. Voto {progresso}.` |
| CPedeNominal (11725) | `… Digite os demais dígitos do seu candidato, ou aperte confirma para prosseguir, ou corrige para reiniciar este voto.` |
| CConfirmaVotoNominal (11728) | `Você está votando para {cargo-atual} n{o/a} candidat{o/a} {voto}: {candidato}. Aperte confirma ou corrige.` |
| CMajoritarioValido (11687) | `… {voto}: {candidato-com-suplentes}. …` e, para um referendo, `Você está respondendo à pergunta: {cargo-atual}. Você escolheu a opção {voto}: {escolha-referendo}. …` |
| CConfirmaVotoLegenda (11735), CCandidatoInexistente (11731) | `… no número {voto}. [Candidato inexistente.] Aperte confirma para votar na legenda {legenda}, {nome-partido}, ou corrige …` |
| CPedeNulo (11745), CMajoritarioNulo/CProporcionalNulo (11693/11701) | `… no candidato {voto}. Número errado. [Se apertar confirma, este voto será nulo.] …` |
| CCandidatoInapto (11748), CMajoritarioRepetido (11690) | `… Candidat{o/a} não concorre …` / `… já foi escolhid{o/a} em voto anterior …`, `Você já votou na opção "{escolha-referendo}". …` |
| CMajoritarioBranco/CProporcionalBranco (11696/11704) | `Você está votando em branco para {cargo-atual}. Aperte confirma ou corrige.` |
| CConfirmaVotoSemCandidato (7425) | `Você está votando para {cargo-atual}, mas não há candidato para o cargo` |

**Substituição de tags** (`FormataMensagem`, func 6999): um `std::sregex_token_iterator` com o padrão
`\{.+?\}` e sub-matches `{-1, 0}` percorre o texto entre as tags e as próprias tags:

| tag | valor |
|---|---|
| `{cargo-atual}` | nome do cargo (para um referendo, o nome de áudio da consulta, se houver), mais `" {n}ª vaga"` (ou `"escolha"` para uma consulta) quando o cargo tem ≥ 2 escolhas. `n` = `g_numeroEscolha` @1536340 |
| `{voto}` | o número digitado `g_votoDigitado` (@1833288), com os dígitos separados para o TTS: `regex_replace(s, "\B\d", ", $&")`, então "123" vira "1, 2, 3" |
| `{legenda}` | os 2 primeiros dígitos digitados, separados da mesma forma |
| `{nome-partido}` | `FormatPartyName(first 2 digits)`: `atoi`, busca em `CPartidos`, lança 9321 "Nao foi encontrado o partido" depois de registrar no log "Erro partido não encontrado" |
| `{escolha-referendo}` | a resposta atual de `CRespostas` (nome de áudio em +16, ou então a descrição em +4) |
| `{o/a}` | `"a"` quando a candidatura atual tem código de sexo 2, senão `"o"` |
| `{candidato}` | o nome de áudio opcional do candidato (+32/+44), ou então o nome de urna (+20) |
| `{candidato-com-suplentes}` | o mesmo, mais, para cada suplente, `", " + CCargoDSNomeSexoCandidato{i, true}() + " " + name` |
| `{quantidade-digitos}` | `"{} dígitos"` / `"{} dígito"` |
| `{progresso}` | `"{n} de {total}"`, onde n = votos confirmados + 1 e total = a soma das escolhas de todos os cargos |

## 4. A cadeia de encerramento (fim da votação)

`EEstadoVota` = ASN.1 `EstadoGeralVota.estadoVota` + 49 e `EEstadoEncerramento` = `estadoEncerramento` + 49
(`src/asn1/ModuloEstadoGeralVota.asn`). O estado é persistido em `vota.bin` por `comum::SalvaEstado()`
(func 491, "Gravando o estado da urna").

```
EAVGERARBU (59)          CGeraBU            BU report -> trab/bu.dat            -> 60
EAVGERARRELATORIOS (60)  CGeraRelatorios    buj.dat, [bim.dat], [behb.dat]      -> 61, next CInicioBU
EAVIMPRIMIRBU (61)       CInicioBU / CQuerImprimirBU / CImprimindoBU  (other units: print the BU vias)
EAVGRAVARRESULTADOS (62) CGravaResultado    (other unit: ModuloBoletimUrna BU, RDV, hashes, signatures)
EAVCOPIARESULTADOSMR(63) CCopiaResultadoParaMR  copy to /dsk/mr/, verify       -> 64 + encerramento 50
EAVENCERRADA (64)        CImprimirBUOutrasObrigatorias (EAEIMPRIMIROBRIGATORIABU 50) -> CRetirarMR (EAERETIRAMR 51)
                         -> CVerificaQtdBUsAdicionais (EAEFIMDOSTRABALHOS 52) -> CEmitirMaisBU -> CMostraQRCodeBU …
```

Cada etapa de relatório registra no log `"Gerando relatório [<X>] [INÍCIO]"` e `"… [TÉRMINO]"` (func 1047,
`IEventosLog::ConverteERelatoriosUE`: 3 = `BU`, 4 = `BUJ`, 9 = `BIM`, 12 = `ELEITORES HABILITADOS
BIOGRAFICAMENTE`). Cada relatório é gravado dentro de um `api::CApplicationContextGuard(2, "", "Gerando … na
MI", "Ocorreu um erro durante a geração … na MI.")`, que fornece o texto da tela de erro se uma exceção
escapar. O guard cobre apenas a gravação: o gerador do BU (com `CCalculaCV` e `LeChave`) e o gerador do BUJ
são construídos *antes* dele, então uma exceção levantada durante a construção deles (8090/8091, 9051–9053, um
arquivo de chave ausente) escapa sem esse contexto. Depois que o guard é destruído, `CGeraBU` salva o estado e
espelha `bu.dat`; `CGeraRelatorios` espelha cada arquivo enquanto o guard ainda está vivo e salva o estado uma
vez no final.

### 4.1 `CGeraRelatorios::StartState` (func 12105)

1. `Assert(GetEstadoVota() == EAVGERARRELATORIOS)` (srcloc 51, EUeAssertError 3455), depois a tela "Preparando dados…".
2. **BUJ** (sempre). `comum::CGeradorBUJ` monta quatro partes, depois `GeraBUJ(path)` (cgeradorbuj.cpp:189)
   grava o arquivo:
   * o cabeçalho: "Boletim de Justificativa Eleitoral" + `IncluiCabecalhoEleicoesMZS` + datas de
     abertura/fechamento + resumo da correspondência;
   * a contagem: `"{:05} Justificativas"`;
   * um bloco de detalhe por entrada do `CDataMap` de justificativas (`Next()` além do fim lança
     "Operação inválida {}", EUeIoError 5977);
   * o rodapé: código da carga, `"Ver: 10.23.0.1"`, "ASSINATURAS:", "PRESIDENTE:", "MESÁRIOS:", "FISCAIS:", corte.

   O arquivo é `buj.dat`.
3. **BIM** (quando a func 1950 retorna true: fora do modo de demonstração e com a flag em
   `CConfiguracaoEleicao+488` ligada). `CImprimirIdentificacaoMesariosFinal(final)` está inlinada. Ela imprime
   "Mesários registrados na abertura/no fechamento" a partir da tabela SQLite `comparecimento_mesario` (func 815
   do DAO), com linhas "Nome:" e "Assinatura:". O arquivo é `bim.dat`.
4. **BEHB** (fora do modo de demonstração e com `CLocal::UrnaBiometrica()`). "Eleitores com habilitação
   biográfica": para cada seção (seção principal, "Seção agregada: {:04}", "Transferência temporária"),
   os eleitores que foram habilitados biograficamente, reunidos num `map<section, vector<{número, nome}>>`,
   cada vetor ordenado com `std::sort`. O arquivo é `behb.dat`.
5. `SetEstadoVota(EAVIMPRIMIRBU)`, `SalvaEstado()`, próximo estado `CInicioBU` (func 5983).

Cada arquivo é gravado em `CPath::GetPathTrab(INTERNA)/<name>` e espelhado por
`CSincronizaVota::SincronizaRelatorios(name)`.

### 4.2 `CCopiaResultadoParaMR::StartState` (func 12134)

1. `CApplicationContextGuard(10, "Copiando resultado para MR", "Erro de gravação", "Ocorreu um erro durante
   a cópia do resultado para a MR.")`, depois `Assert(EAVCOPIARESULTADOSMR)` (srcloc 59, 3451), depois a tela.
2. No modo de demonstração (`IInterfaceInit::GetDemoMode()`), nada é copiado.
3. Caso contrário:
   * `EnviarMensagemThrowVoid(10, "habilitando MR")` seguido de um sleep de 500 ms (func 2863);
   * a MR deve estar presente (9367, srcloc 94);
   * ela é remontada;
   * se `/dsk/mr/` não está vazio, `IValidaMidia::ValidaMidiaResultado(device, turno)` deve aceitá-la
     (9368, srcloc 112, mais o syslog "mídia inválida para gravação pelo motivo [%d]");
   * fora do treinamento, a MR não pode já conter resultados (9369, srcloc 126).
4. Cópia com `comum::CCopiadorMR` (9 arquivos) e `CCopiadorWSQMR` (3 arquivos, só em urnas biométricas), com um
   `fsync` (`CSynchronizer`) depois de cada um. Os nomes vêm de
   `DeterminaNomeArquivoSemLetra(mun, zona, secao, fase, tipo)` =
   `format("{:c}{:05}{}{:05}{:04}{:04}-", fase, pleito, <CLocal string>, mun, zona, secao) + CArquivosResultado[tipo]`:

   | tipo | sufixo | | tipo | sufixo |
   |---|---|---|---|---|
   | 4 | `bu.dat` | | 12 | `hash.dat` |
   | 6 | `rdv.dat` | | 13 | `log.jez` |
   | 8 | `jufa.dat` | | 1 | `vota.vsc` |
   | 9 | `imgbu.dat` | | 19 | `mr.ver` |
   | 11 | `imgze.dat` | | 16/17/18 | `wsqbio.jez`, `wsqman.jez`, `wsqmes.jez` |

5. Verificação: **apenas o arquivo do BU** (tipo 4) é comparado entre `GetPathResult(0)` e `/dsk/mr/`,
   com um `api::CSystem::AreFilesEqual` inlinado (mesmo tamanho, depois `fread` em blocos de 32 KiB para dois
   buffers estáticos e `memcmp`). Uma divergência registra no log "Erro na cópia dos resultados para MR" e lança
   9370 "Nao copiou" (srcloc 174). Em caso de sucesso, a MR é desmontada e "Mídia de resultado gravada" é
   registrado no log.
6. `EstadoVota = EAVENCERRADA (64)`, `EstadoEncerramento = EAEIMPRIMIROBRIGATORIABU (50)`, `SalvaEstado()`,
   próximo estado `CImprimirBUOutrasObrigatorias`.

### 4.3 `CEmitirMaisBU` (funcs 12082 / 12081)

`StartState` faz assert de `EAVENCERRADA` (64, srcloc :46) e de `EAEFIMDOSTRABALHOS` (52, srcloc :47), registra no
log "Mesário indagado sobre quantidade de vias adicionais" e mostra `telaEmitirMaisBU`. Em `ProcessInput`:

* CORRIGE: registra no log "Mesário não solicitou emissão de vias adicionais", depois vai para `CMostraQRCodeBU`.
* CONFIRMA:
  * Um campo vazio registra a mesma mensagem e continua perguntando.
  * Caso contrário, n = `CStringUtils::ToByte(text)` e o log recebe `"Solicitada a emissão de [n] via(s)
    adicional(is)"`.
  * Se n > `GetQuantidadeMaximaBUsAdicionais()`, registra no log "Quantidade de vias adicionais excede o
    máximo permitido", mostra a tela de limite, dorme 3 s e pergunta de novo.
  * Caso contrário, imprime n vias: para cada uma, `ImprimeBU(qtdBU+1, false)`, `++qtdBU`, `SalvaEstado()`.
    Para antes se a urna estiver desligando (flag @1832936).
* Próximo estado: `CMostraQRCodeBU`.

## 5. BOLETIM DE URNA: o que `CGeraBU` faz, passo a passo

`CGeraBU::StartState` (func 12110) é o único lugar do binário onde existem a construção do BU impresso
(`comum::CGeradorBU`, `cgeradorbu.cpp`) e o seu laço de geração (`CGeradorRelBase::GeraRelatorio`,
`cgeradorrelbase.h:61`), porque ambos estão inlinados. Sequência:

1. Mostra "Votação encerrada". `Assert(appInfo.GetVota().GetEstadoVota() == EAVGERARBU)` (cgerabu.cpp:54,
   EUeAssertError 3454). Espera 1 s (ver §9.1). Mostra "Preparando dados para encerramento". Registra no log
   `Gerando relatório [BU] [INÍCIO]`.
2. `CCargos::Inicio()`. `separadorFase = CRelUtil::GetSeparadorFase(EstadoGeral.fase)`:
   `==============TREINAMENTO=============`, `===============SIMULADO===============`, 38 `=` para
   oficial, ou `============DEMONSTRAÇÃO==============` no modo de demonstração.
3. **Hora de emissão.** Se `EstadoGeralVota.dhEmissao` (+80, opcional) não está definido, ele recebe
   `CDateTime::Now()`. Ele é relido via `GetDtHrEmissaoBU()`, que lança 8093 "A data/hora da
   emissão do BU não foi registrada".
4. **Histórico de cargas.** `GetGap()` retorna o `codigoCarga` de cada entrada de `EstadoGeralGap.correspondencias`
   (gap.bin) como um `vector<string>` (func 3788).
5. **Monta as 13 partes do relatório** (construtor de `CGeradorBU` → `CGeradorBUBase` → `CGeradorRelBase`, cada
   parte verificada como não nula, códigos de erro 9055–9067):
   * **cabeçalho** (`CriaHeader`):
     * `CRelUtil::IncluiCabecalhoEleicoesMZS(title "Boletim de Urna", município, zona, seção, nome do
       município)`;
     * `Eleitores aptos {:04}` (aptos + aptos da outra categoria, contados a partir de `CEleitores`);
     * `Comparecimento {:04}` = `CRdvVota::Comparecimento()` (o máximo entre as eleições);
     * em urnas biométricas: as contagens `Habilitação biométrica / biográfica / sem biometria`;
     * `Eleitores faltosos` = aptos − comparecimento;
     * `Código identificação UE {:08}` (= `carga.numeroInternoUrna`);
     * se houve votação, a data e a hora de abertura/fechamento (`dhIniAquisicao`/`dhFimAquisicao`, 8090/8091
       quando ausentes);
     * "RESUMO DA CORRESPONDÊNCIA".
   * **rodapé**:
     * `separadorFase`;
     * "Código de identificação da carga" + `CRelUtil::GetIDCargaFormatado(codigoCarga)` (func 2786: sete
       grupos de 3 e o resto, unidos por `.`; "ID de carga inválido" quando tem menos de 21 caracteres);
     * `Ver: 10.23.0.1` (de "10.23.0.1 - DESENVOLVIMENTO");
     * um texto de configuração cujo `<uf>` é substituído pela UF;
     * 20 linhas em branco e um corte de papel.
   * **qrcode** (`CriaQRCode`, cgeradorbu.cpp:124):
     * modo de demonstração: "DEMONSTRAÇÃO PRÉ-ELEIÇÃO" / "NÃO HÁ QR CODE";
     * quando o bit de configuração `CConfiguracaoEleicao+486 & 1` está ligado:
       `CGeradorBUQRCode(zona, seção, numeroInternoUrna, codigoCarga, histórico; comparecimento,
       dhEmissao)` produz até 1100 bytes por QR (func 5604, outra unidade). Os payloads começam com
       `QRBU:{i}:{n} VRQR:… ` e contêm campos `KEY:value` (PROC, PLEI, TURN, FASE, UNFE, ZONA/SECA,
       IDUE, IDCA/HASH, CARG, TIPO, NOMI, LEGP/LEGC, BRAN, NULO, TOTC, TOTP, … ` HASH:{}`, ` ASSI:{}`).
       Eles são impressos sob `============= BU DIGITAL =============`, cada um precedido por
       `-------------- {i:02} / {n:02} ---------------`. A impressão continua com
       `======== CERTIFICADO DIGITAL =========` (o certificado da urna em hex, dividido em
       ⌈2·bytes/1082⌉ QR codes com os campos `QRCE:i:n IDUE:… MDUE:… CERT:…`), depois
       `ASSINATURA BU DIGITAL: <signature>` se uma assinatura foi retornada;
     * caso contrário, nenhuma parte de QR.
   * **partes por cargo**: `headerProporcional` (separador + nome do cargo completado com `-` até 38 colunas),
     `headerProporcionalPartido`, `partidoApenasVotoLegenda` ("Não há votos nominais"),
     `trailerProporcionalPartido`, `cargoSemCandidato` ("Não há candidatos concorrendo"),
     `trailerCargoSemCandidato`, `trailerProporcional`, `headerMajoritario`, `detalheCandidato`,
     `trailerMajoritario`. Elas são feitas de campos `CDataText` (ids 1625, 1626, 1632, 1633, 1635/1636,
     2920–2923) que as fontes de dados do gerador preenchem com as contagens do RDV no momento da impressão.
6. **Código verificador.** `comum::CCalculaCV` (56 bytes) é criado e publicado como poly-singleton
   (`CPolySingletonList::push<CCalculaCV>`, erro 6756 se já existir):
   * *identificação* = `format("{:05}{:04}{:04}{}{}{}{}", município, zona, seção,
     format("{:05}{:05}{}", cfg[+0], pleito, dataPleito "YYYYMMDD"), numeroInternoUrna, pleito, fase-char)`.
     A string aninhada é o **4º** argumento (tipos de argumento empacotados `6,6,6,13,6,6,2`: três unsigned, um
     string_view, dois unsigned, um char). `pleito` é `CConfiguracaoEleicao+28`, `dataPleito` é `+44`. Ela deve ter
     pelo menos 13 caracteres de `0123456789sodtSODT` (9051 "Identificação inválida [..]").
   * primeiro CV = `"0000000000"`, que deve ser decimal (9052).
   * tipo = `DefineTipo(demo)`: `'A'` no modo de demonstração, `'F'` nos demais casos (cgeradorbu.cpp:161).
   * chave = `comum::util::LeChave("/dsk/fi/estatico/chave/cv.ber.pri")` (util.cpp:37). Isso lê um
     envelope ASN.1 `EntidadeChave`, e `CKeyLoader::DecipherKeyIfNeeded` o decifra. A chave deve ter pelo
     menos 16 bytes (9053 "Chave privada com tamanho pequeno…") e os primeiros 16 bytes são mantidos.
   * estado `buffer = tipo + identificação + cv` (ecourna_f5615). As fontes de dados do BU então acrescentam
     cada valor impresso com `CCalculaCV::IncluiString` (ccalculacv.cpp:109, outra unidade), e o CV impresso no
     BU é um MAC desse buffer sob a chave (unidade u25 / relatórios).
7. **Grava o relatório.** `CApplicationContextGuard(2, "", "Gerando boletim de urna na MI", "Ocorreu um erro
   durante a geração do boletim de urna na MI.")`. Em seguida, `GeraRelatorio(GetPathTrab(INTERNA)/"bu.dat")`
   executa:
   * `IPaperRelatorios::Abre(path)`, cabeçalho, `CGeradorBU::ImprimePreTexto()`;
   * o laço de cargos. Quando a lista contém mais de uma eleição, um banner `======` / nome da eleição / `======`
     precede os cargos de cada eleição (a primeira comparação é contra a eleição do
     *último* cargo). Cada cargo então vai para `ImprimeProporcionalPartido` (cargo de eleição proporcional),
     `ImprimeMajoritario` (cargo majoritário) ou `ImprimeConsulta` (referendo);
   * a parte de QR (se houver), o rodapé, `Fecha()`, `CSynchronizer`.
8. `SalvaEstado()`, `CSincronizaVota::SincronizaRelatorios("bu.dat")`, `~CGeradorBU`, log `[BU]
   [TÉRMINO]`, `SetEstadoVota(EAVGERARRELATORIOS)`, `SalvaEstado()`, próximo estado `CGeraRelatorios`.

O que acontece depois, em outras unidades: `CImprimindoBU::ImprimeBU(via, …)` imprime as vias,
`CGravaResultado` (EAVGRAVARRESULTADOS) monta e assina o BU ASN.1 `ModuloBoletimUrna` e os demais
arquivos de resultado, `CCopiaResultadoParaMR` (§4.2) copia `…-bu.dat`, `…-imgbu.dat`, `…-rdv.dat`, … para a MR,
e `CMostraQRCodeBU` / `CMostraQRCodeCertificado` mostram os QR codes na tela. Para a tela,
`CMostraQRCodeCertificado` usa a func 12039 (`MontaQRCodesCertificado(estado, false)`, um QR, depois
`MontaConteudoQRCode`).

No simulador, o BU não pode ser gerado. O emscripten_sleep do passo 1 abortaria antes (§9.1), e
a chave `/dsk/fi/estatico/chave/cv.ber.pri` não existe em nenhum cenário de `upstream/fs/`.

## 6. Dados lidos e gravados

| dado | acesso |
|---|---|
| `vota.bin` (`EstadoGeralVota`: estadoVota, estadoEncerramento, qtdBU +8, dhIni/FimAquisicao, dhEmissao +80) | leitura/gravação via `CAppInfo::GetVota()` + `SalvaEstado()` |
| `EstadoGeralUrna` (fase +48, município/zona/seção +20..+26, correspondência +60 com numeroInternoUrna e codigoCarga +28), `gap.bin` (correspondências) | leitura |
| `/dsk/fi/estatico/chave/cv.ber.pri` (EntidadeChave) | leitura (chave do CV) |
| tabela `comparecimento_mesario` de `uenux.db` | leitura (BIM) |
| `CPath::GetPathTrab(INTERNA)/{bu,buj,bim,behb}.dat` | gravação (relatórios em texto) |
| `CPath::GetPathResult(0)/<prefix>-<suffix>` → `/dsk/mr/<same>` | cópia e verificação (só o BU) |
| `dinamico/log/logd.dat` | linhas de log (Latin-1) |
| certificado obtido de `CEstadoGeral::RecuperarCertificado()` | leitura (QR "CERTIFICADO DIGITAL") |

## 7. Particularidades do build web

* O áudio do eleitor usa `simulador::CWasmWebSound` e o Web Audio do JS. `PlayInterruptibleMessage` passa
  o modo 0 para `js_wasm_web_sound_play_wav`, que para e limpa a fila, depois toca. `PlayMessage`
  passa o modo 1, que enfileira. `ISound::Wait` (func 9523) **não bloqueia** no build web, então
  `PlayMessage` retorna imediatamente, enquanto na urna ela segura a thread do eleitor até a mensagem terminar.
  `ISound::PlayFile` (func 9580) ignora o arquivo, então `tecE.wav` nunca é tocado; o beep de
  `CWasmBeep` continua soando.
* A página define `CInformacaoEleitor::m_modoAudio` via `votaInit` (`audioEleitorHabilitado`) e
  `votaSetAudioEnabled`. Com o áudio desligado (as sessões gravadas), apenas `StartState`, `FinishState`,
  `ProcessInput`, `ProcessTick`, `PlayKey`, `EmiteEcoComInputField` e `EmiteEcoCorrigeConfirma` executam.
  Essas são as 7 funções observadas.
* As classes de encerramento são compiladas sem alteração, mas nenhuma sessão gravada chega a elas.

## 8. Observações de wasm / Emscripten

* **O inlining do LTO** faz de `CGeraBU::StartState` (17 KB) e `CGeraRelatorios::StartState` (18.8 KB) as
  únicas cópias de grandes partes de `comum/relatorios` (CGeradorBU, CGeradorBUJ, CImprimirIdentificacaoMesariosFinal,
  construtor de CCalculaCV, LeChave, CPolySingletonList::push). `CopiaResultado` e `EmiteEcoCorrigeConfirma`
  existem apenas inlinadas.
* **Nomeadas errado pelas ferramentas** quando este capítulo foi escrito (o banco de dados agora mostra os nomes corrigidos):
  * a func 11754 ("CVotacaoStateAudio::EmiteEcoCorrigeConfirma") é `CConfirmaVotoEmCargo::ProcessInputAudio`;
  * a func 12134 ("CopiaResultado") é `StartState`;
  * a func 1950 ("EhModoDemonstracao") é um chamador que a inlina: ela retorna `!EhModoDemonstracao() &&
    CConfiguracaoEleicao+488` (a condição do BIM). A func 2286, que carrega o mesmo srcloc
    (cinformacaoeleicao.cpp:40), é de fato `CInformacaoEleicao::EhModoDemonstracao()` (ela apenas retorna
    `IInterfaceInit::GetDemoMode()`; o `this` não usado foi removido);
  * a func 5604 ("CCargos::GetCurrentEleicaoVersaoPacote") é o gerador de QR do BU;
  * a func 7787 (antes exibida pelas ferramentas como "CHKDFSeed::GetSeed") é a função de inicialização,
    `vota::CInformacaoEleitor::Inicializar` (nome inferido, u02).
* **ICF:** `~CVotacaoStateAudio` (1035) é o destrutor de toda subclasse que não acrescenta membros. A
  lambda `IniciarEsperaFimAudio()::$_0` é dobrada em `icf_ret_0_vf6` (return false). Os onze thunks at-exit
  desta unidade (7479, 11689…11737) compartilham o corpo 906, que quatro thunks de outras unidades (11686, 11727,
  11747, 11750) também chamam.
* **merge-similar-functions:** os helpers de mensagem de CLogVota (4556, 5885) passam ponteiros para o meio
  do mesmo literal a um corpo compartilhado (`api_f6115`, `vota_f3902`).
* **`<regex>` da libc++**: as ferramentas deram a esta unidade 50 instâncias de template `std::basic_regex<char>`
  (25.5 KB de código) porque `FormataMensagem` as chama. Elas não são usadas apenas por `FormataMensagem`: as
  mesmas instâncias também são chamadas por `api::CInputMenuField` (func 5492, padrão `%[ST]`), `CThreadMonitor`
  (7709), `CSerialMidia` do ecourna (9258, `([A-Fa-f0-9]{8})`) e `CRelatorioTesteImpressora` (5782). O
  Boost.Regex também está linkado (func auxiliar do TSE 2199). As constantes de regex são do ABI-v2 (`ECMAScript` = 512).
* **Codificação de strings:** as strings de relatório e de áudio são Latin-1 no segmento de dados
  (`"Gerando relat\xF3rio"`), enquanto as demais mensagens são UTF-8.
* Os códigos de erro são fixos por ponto de throw (`CBaseError<EUeVotaError>` 9318, 9319, 9321, 9367–9371).

## 9. Código suspeito / notável

1. **Caminhos de abort via `emscripten_sleep`.** As funcs 12110 (1000 ms), 12081 (3000 ms) e 2863 (500 ms, no
   caminho de `CopiaResultado`) chamam `emscripten_sleep` quando o byte @1584624 vale 1. Esse byte é
   inicializado com 1 e nada grava nele: uma execução do headless.mjs imprimiu 1 depois de `votaInit` e depois de
   um voto. O glue implementa `_emscripten_sleep` como `abort()` (sem Asyncify), então alcançar qualquer um desses
   estados mataria o simulador. O fluxo web não chega a eles.
2. **O BU não pode ser produzido no simulador.** `cv.ber.pri` está ausente (§5; não há diretório `chave/` em
   nenhum cenário de `upstream/fs/`), então `LeChave` lançaria exceção antes de qualquer relatório ser gravado,
   mesmo que o sleep fosse removido, e fora do contexto de erro "Gerando boletim de urna na MI".
3. **Apenas o arquivo do BU é verificado após a cópia para a MR** (§4.2). Um `rdv.dat`, `log.jez` ou `.vsc`
   truncado no pendrive não é detectado aqui, a menos que `CCopiadorMR::Copia` lance exceção por conta própria.
4. **O áudio se comporta de outra forma na web** (§7). `PlayMessage` não espera e `PlayFile` é silencioso.
   Eleitores cegos no simulador ouvem um ritmo diferente do da urna real. Informativo.
5. **`FormatPartyName` lança exceção dentro de `ProcessTick`** se o prefixo digitado não for um partido (9321).
   Os templates que usam `{nome-partido}` (confirmações de legenda e de candidato inexistente) só são alcançados
   com um partido válido, então isso é defensivo. Conferido com headless.mjs `--audio` em municipal-t1: `91999`
   (o partido 91 existe, sem candidato) chega a `CCandidatoInexistente`, e a sua mensagem é falada e repetida;
   `98999` (não há partido 98) vai para `CPedeNulo` / `CProporcionalNulo`, cujos templates não têm `{nome-partido}`.
   Nenhuma das execuções lançou exceção.
6. **Nome de variável em um assert:** o assert em cgerarelatorios.cpp:51 diz `poInfo.GetVota()`, enquanto os
   outros arquivos da cadeia dizem `appInfo.GetVota()`. A macro transforma o seu argumento em string, então a
   variável local em `CGeraRelatorios::StartState` de fato se chama `poInfo` (a reconstrução mantém esse nome).
   Cosmético.

## 10. Questões em aberto

* O tipo guardado em `CVotacaoStateAudio+16` (um `unique_ptr` que só é destruído).
* O significado dos ids de `CDataText` (1625…2961) usados nas partes do BU. Eles são resolvidos em `comum/relatorios` (unidade u25).
* Os nomes exatos dos slots de `ISound` / `ITextToSpeech` / `IEsperaAudio` / `IValidaMidia`. Eles são inferidos a partir do comportamento.
* A string de `CLocal` inserida por `DeterminaNomeArquivoSemLetra` (o campo `{}`) e o `CConfiguracaoEleicao[+0]` usado na identificação do CV.

## 11. Tabela de mapeamento completa (107 funções)

Colunas:

* **run**: observada executando nos votos gravados.
* **arquivo original**: onde a função fica na árvore do TSE. Caminhos não atestados por um srcloc são inferidos.
* **reconstruída em**: o arquivo em `src/uenux2/src/app/vota/eleitor/` que contém a sua reconstrução ou descrição.
* **tipo**: código do TSE, gerado pelo compilador ou biblioteca.

| idx | tamanho | run | nome nas ferramentas | símbolo reconstruído | arquivo original | reconstruída em | tipo / conf. |
|---|---|---|---|---|---|---|---|
| 509 | 102 |  | `vota_f509` | `vota::CInformacaoEleitor::GetInst` | uenux2/src/app/vota/eleitor/comum/cinformacaoeleitor.cpp | cvotacaostateaudio.cpp | TSE / média |
| 625 | 376 |  | `comum_f625` | `std::vector<std::pair<std::string,std::string>>::push_back(value_type&&)` | biblioteca: libcxx/include/vector | fimvotacao/cgerabu.cpp | biblioteca / alta |
| 906 | 32 |  | `vota_f906` | `vota::(anonymous namespace)::ResetaSingletonAudio` | gerado pelo compilador (estáticos de celeitorvotando.cpp) | cvotacaostateaudio.cpp | compilador / baixa |
| 950 | 23 |  | `vota_f950` | `std::__throw_regex_error(regex_constants::error_type)` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / alta |
| 965 | 458 |  | `ecourna_f965` | `std::__bracket_expression<char, regex_traits<char>>::__add_char` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / média |
| 1035 | 180 |  | `vota::CVotacaoStateAudio::vf0` | `vota::CVotacaoStateAudio::~CVotacaoStateAudio` | uenux2/src/app/vota/eleitor/cvotacaostateaudio.cpp | cvotacaostateaudio.cpp | TSE / alta |
| 1202 | 466 |  | `vota_f1202` | `std::regex_token_iterator<const char*>::regex_token_iterator(const regex_token_iterator&)` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / média |
| 1204 | 306 |  | `vota::CVotacaoStateAudio::PlayMessage` | `vota::CVotacaoStateAudio::PlayMessage` | uenux2/src/app/vota/eleitor/cvotacaostateaudio.cpp | cvotacaostateaudio.cpp | TSE / alta |
| 1276 | 32 |  | `vota_f1276` | `comum::CCopiadorMR::CCopiadorMR` | uenux2/src/app/comum/gravadores/ccopiadormr.cpp | fimvotacao/ccopiaresultadoparamr.cpp | TSE / média |
| 1321 | 226 |  | `vota_f1321` | `std::vector<std::pair<char,char>>::push_back` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / média |
| 1455 | 496 | sim | `vota::CVotacaoStateAudio::PlayKey` | `vota::CVotacaoStateAudio::PlayKey` | uenux2/src/app/vota/eleitor/cvotacaostateaudio.cpp | cvotacaostateaudio.cpp | TSE / alta |
| 1542 | 515 |  | `comum_f1542` | `comum::IncluiDataHora` | uenux2/src/app/comum/relatorios/crelutil.cpp | fimvotacao/cgerarelatorios.cpp | TSE / baixa |
| 1597 | 7 |  | `ecourna_f1597` | `std::__throw_regex_error<regex_constants::error_brack>` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / alta |
| 1599 | 7 |  | `ecourna_f1599` | `std::__throw_regex_error<regex_constants::error_escape>` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / alta |
| 2062 | 1365 |  | `vota_f2062` | `std::basic_regex<char>::__parse<const char*>` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / alta |
| 2248 | 554 |  | `comum_f2248` | `comum::IncluiLinhaQuantidade` | uenux2/src/app/comum/relatorios/cgeradorbu.cpp | fimvotacao/cgerabu.cpp | TSE / baixa |
| 2252 | 2402 |  | `comum_f2252` | `comum::GetIDCargaFormatado(const CDadoCorrespondencia&)` | uenux2/src/app/comum/relatorios/crelutil.cpp | fimvotacao/cgerarelatorios.cpp | TSE / baixa |
| 2391 | 7 |  | `ecourna_f2391` | `std::__throw_regex_error<regex_constants::error_collate>` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / alta |
| 2393 | 249 |  | `vota_f2393` | `std::basic_regex<char>::__push_back_ref` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / média |
| 2416 | 123 |  | `vota_f2416` | `std::basic_regex<char>::basic_regex(const char*, flag_type)` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / média |
| 2775 | 543 |  | `comum_f2775` | `api::CPaperFormBuilder::AddQRCode` | uenux2/src/api/gui/cpaperformbuilder.h | fimvotacao/cgerabu.cpp | TSE / baixa |
| 2785 | 105 |  | `comum_f2785` | `comum::IncluiSeparadorFase` | uenux2/src/app/comum/relatorios/crelutil.cpp | fimvotacao/cgerarelatorios.cpp | TSE / baixa |
| 2789 | 738 |  | `comum_f2789` | `std::__sort4<_ClassicAlgPolicy, __less<>, SEleitorBEHB*>` | biblioteca: libcxx/include/__algorithm/sort.h | fimvotacao/cgerarelatorios.cpp | biblioteca / média |
| 3081 | 124 |  | `vota_f3081` | `std::basic_regex<char>::__parse_QUOTED_CHAR` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / média |
| 3086 | 7 |  | `vota_f3086` | `std::__throw_regex_error<regex_constants::error_badbrace>` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / alta |
| 3087 | 7 |  | `vota_f3087` | `std::__throw_regex_error<regex_constants::error_brace>` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / alta |
| 3102 | 993 |  | `vota_f3102` | `std::basic_regex<char>::__parse_ERE_expression` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / média |
| 3107 | 7 |  | `vota_f3107` | `std::__throw_regex_error<regex_constants::__re_err_empty>` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / alta |
| 3108 | 309 |  | `vota_f3108` | `std::basic_regex<char>::__parse_extended_reg_exp` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / média |
| 3135 | 7 |  | `vota_f3135` | `std::__throw_regex_error<regex_constants::__re_err_parse>` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / alta |
| 3137 | 283 |  | `vota_f3137` | `std::regex_token_iterator<const char*>::operator==` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / média |
| 3139 | 525 | sim | `vota::CVotacaoStateAudio::EmiteEcoComInputField` | `vota::CVotacaoStateAudio::EmiteEcoComInputField` | uenux2/src/app/vota/eleitor/cvotacaostateaudio.cpp | cvotacaostateaudio.cpp | TSE / alta |
| 3143 | 224 |  | `vota::CVotacaoStateAudio::PlayInterruptibleMessage` | `vota::CVotacaoStateAudio::PlayInterruptibleMessage` | uenux2/src/app/vota/eleitor/cvotacaostateaudio.cpp | cvotacaostateaudio.cpp | TSE / alta |
| 3329 | 153 |  | `ecourna_f3329` | `std::__get_classname(const char*, bool)` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / média |
| 3646 | 666 |  | `comum_f3646` | `comum::MontaConteudoQRCode` | uenux2/src/app/comum/relatorios/cgeradorbu.cpp | fimvotacao/cgerabu.cpp | TSE / baixa |
| 3688 | 445 |  | `comum_f3688` | `comum::IncluiDatasAberturaFechamento` | uenux2/src/app/comum/relatorios/crelutil.cpp | fimvotacao/cgerabu.cpp | TSE / baixa |
| 3695 | 135 |  | `comum_f3695` | `std::__tree<...map<SChaveSecao, vector<SEleitorBEHB>>...>::destroy` | biblioteca: libcxx/include/__tree | fimvotacao/cgerarelatorios.cpp | biblioteca / média |
| 3827 | 29 |  | `vota_f3827` | `comum::CCopiadorWSQMR::CCopiadorWSQMR` | uenux2/src/app/comum/gravadores/ccopiadormr.cpp | fimvotacao/ccopiaresultadoparamr.cpp | TSE / média |
| 3872 | 180 |  | `comum_f3872` | `comum::CriaDetalheCandidato` | uenux2/src/app/comum/relatorios/cgeradorbu.cpp | fimvotacao/cgerabu.cpp | TSE / baixa |
| 4203 | 846 |  | `vota_f4203` | `std::basic_regex<char>::__parse_awk_escape` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / média |
| 4237 | 2688 |  | `vota_f4237` | `std::basic_regex<char>::__parse_bracket_expression` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / média |
| 4242 | 2302 |  | `vota_f4242` | `std::basic_regex<char>::__parse_ERE_dupl_symbol` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / média |
| 4257 | 326 |  | `vota_f4257` | `std::basic_regex<char>::__parse_basic_reg_exp` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / média |
| 4307 | 478 |  | `vota_f4307` | `std::regex_token_iterator<const char*>::regex_token_iterator(first, last, re, initializer_list<int>)` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / média |
| 4557 | 162 |  | `vota_f4557` | `vota::CLogVota::LogaMesarioNaoSolicitouVias` | uenux2/src/app/vota/log/clogvota.cpp | fimvotacao/cemitirmaisbu.cpp | TSE / média |
| 4563 | 174 |  | `vota_f4563` | `vota::CLogVota::LogaMesarioIndagadoQtdVias` | uenux2/src/app/vota/log/clogvota.cpp | fimvotacao/cemitirmaisbu.cpp | TSE / média |
| 4671 | 62 |  | `vota_f4671` | `std::regex_error::regex_error(regex_constants::error_type)` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / alta |
| 5185 | 886 |  | `ecourna_f5185` | `std::regex_traits<char>::__lookup_collatename` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / média |
| 5186 | 873 |  | `ecourna_f5186` | `std::basic_regex<char>::__parse_awk_escape (string* variant)` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / média |
| 5187 | 1151 |  | `ecourna_f5187` | `std::basic_regex<char>::__parse_character_escape (variant)` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / média |
| 5581 | 427 |  | `comum_f5581` | `comum::IncluiCabecalhoQRCode` | uenux2/src/app/comum/relatorios/cgeradorbu.cpp | fimvotacao/cgerabu.cpp | TSE / baixa |
| 5584 | 15 |  | `comum_f5584` | `comum::IncluiTextoComUF` | uenux2/src/app/comum/relatorios/crelutil.cpp | fimvotacao/cgerabu.cpp | TSE / baixa |
| 5599 | 1759 |  | `comum_f5599` | `std::__insertion_sort_incomplete<..., SEleitorBEHB*>` | biblioteca: libcxx/include/__algorithm/sort.h | fimvotacao/cgerarelatorios.cpp | biblioteca / média |
| 5600 | 6076 |  | `comum_f5600` | `std::__introsort<..., SEleitorBEHB*>` | biblioteca: libcxx/include/__algorithm/sort.h | fimvotacao/cgerarelatorios.cpp | biblioteca / média |
| 5602 | 320 |  | `comum_f5602` | `std::vector<SEleitorBEHB>::push_back(value_type&&)` | biblioteca: libcxx/include/vector | fimvotacao/cgerarelatorios.cpp | biblioteca / média |
| 5613 | 503 |  | `comum_f5613` | `comum::AddNomeCargo` | uenux2/src/app/comum/relatorios/cgeradorbu.cpp | fimvotacao/cgerabu.cpp | TSE / baixa |
| 5614 | 107 |  | `comum_f5614` | `comum::CCalculaCV::~CCalculaCV` | uenux2/src/app/comum/relatorios/ccalculacv.cpp | fimvotacao/cgerabu.cpp | TSE / média |
| 5634 | 2021 |  | `comum_f5634` | `comum::MontaQRCodesCertificado` | uenux2/src/app/comum/relatorios/cgeradorbu.cpp | fimvotacao/cgerabu.cpp | TSE / baixa |
| 5883 | 179 |  | `vota_f5883` | `vota::CLogVota::LogaMRInvalida` | uenux2/src/app/vota/log/clogvota.cpp | fimvotacao/ccopiaresultadoparamr.cpp | TSE / média |
| 5980 | 225 |  | `comum_f5980` | `std::vector<std::vector<std::pair<std::string,std::string>>>::~vector` | biblioteca: libcxx/include/vector | fimvotacao/cgerabu.cpp | biblioteca / alta |
| 6751 | 64 |  | `vota_f6751` | `std::basic_regex<char>::__test_back_ref(char)` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / média |
| 6762 | 439 |  | `ecourna_f6762` | `std::__bracket_expression<char, regex_traits<char>>::__add_neg_char ('_' constant-propagated)` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / média |
| 6763 | 171 |  | `vota_f6763` | `std::__bracket_expression<char, regex_traits<char>>::__add_digraph` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / média |
| 6764 | 1015 |  | `vota_f6764` | `std::__bracket_expression<char, regex_traits<char>>::__add_range` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / média |
| 6765 | 332 |  | `vota_f6765` | `std::basic_regex<char>::__parse_class_escape` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / média |
| 6770 | 7 |  | `vota_f6770` | `std::__throw_regex_error<regex_constants::error_ctype>` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / alta |
| 6788 | 1114 |  | `vota_f6788` | `std::basic_regex<char>::__parse_character_escape` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / média |
| 6826 | 1290 |  | `vota_f6826` | `std::basic_regex<char>::__parse_atom<const char*>` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / média |
| 6833 | 741 |  | `vota_f6833` | `std::basic_regex<char>::__parse_assertion<const char*>` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / média |
| 6862 | 1244 |  | `vota_f6862` | `std::basic_regex<char>::__parse_RE_dupl_symbol` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / média |
| 6865 | 637 |  | `vota_f6865` | `std::basic_regex<char>::__parse_simple_RE / __parse_RE_expression` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / média |
| 6869 | 83 |  | `vota_f6869` | `std::basic_regex<char>::__parse_term<const char*>` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / média |
| 6871 | 391 |  | `vota_f6871` | `std::basic_regex<char>::__parse_ecma_exp<const char*>` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / média |
| 6879 | 7 |  | `vota_f6879` | `std::__throw_regex_error<regex_constants::__re_err_grammar>` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / alta |
| 6891 | 94 |  | `vota_f6891` | `std::basic_regex<char>::__parse_alternative<const char*> (__parse_term inlined)` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / média |
| 6939 | 590 |  | `vota_f6939` | `std::regex_iterator<const char*>::operator++` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / média |
| 6942 | 504 |  | `vota_f6942` | `std::regex_iterator<const char*>::operator==` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / média |
| 6954 | 239 |  | `vota::CVotacaoStateAudio::FormatPartyName` | `vota::CVotacaoStateAudio::FormatPartyName` | uenux2/src/app/vota/eleitor/cvotacaostateaudio.cpp | cvotacaostateaudio.cpp | TSE / alta |
| 6958 | 863 |  | `vota_f6958` | `std::regex_replace<back_insert_iterator<string>, const char*, regex_traits<char>, char>` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / média |
| 6970 | 404 |  | `vota_f6970` | `std::regex_token_iterator<const char*>::operator++` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / média |
| 6999 | 4946 |  | `vota::CVotacaoStateAudio::vf14` | `vota::CVotacaoStateAudio::FormataMensagem` | uenux2/src/app/vota/eleitor/cvotacaostateaudio.cpp | cvotacaostateaudio.cpp | TSE / média |
| 7010 | 683 | sim | `vota::CVotacaoStateAudio::ProcessTick` | `vota::CVotacaoStateAudio::ProcessTick` | uenux2/src/app/vota/eleitor/cvotacaostateaudio.cpp | cvotacaostateaudio.cpp | TSE / alta |
| 7022 | 132 | sim | `vota::CVotacaoStateAudio::vf5` | `vota::CVotacaoStateAudio::FinishState` | uenux2/src/app/vota/eleitor/cvotacaostateaudio.cpp | cvotacaostateaudio.cpp | TSE / alta |
| 7028 | 55 | sim | `vota::CVotacaoStateAudio::vf2` | `vota::CVotacaoStateAudio::StartState` | uenux2/src/app/vota/eleitor/cvotacaostateaudio.cpp | cvotacaostateaudio.cpp | TSE / alta |
| 7029 | 282 | sim | `vota::CVotacaoStateAudio::ProcessInput` | `vota::CVotacaoStateAudio::ProcessInput` | uenux2/src/app/vota/eleitor/cvotacaostateaudio.cpp | cvotacaostateaudio.cpp | TSE / alta |
| 7479 | 12 |  | `vota_f7479` | `__dtor_static_unique_ptr@1832992` | gerado pelo compilador (estáticos de celeitorvotando.cpp) | cvotacaostateaudio.cpp | compilador / baixa |
| 9511 | 332 |  | `ecourna_f9511` | `std::basic_regex<char>::__parse_class_escape (instance used by libcxx_f3523)` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / média |
| 9512 | 211 |  | `ecourna_f9512` | `std::basic_regex<char>::__parse_collating_symbol` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / média |
| 9513 | 490 |  | `ecourna_f9513` | `std::basic_regex<char>::__parse_character_class` | biblioteca: libcxx/include/regex | cvotacaostateaudio.cpp | biblioteca / média |
| 11689 | 12 |  | `vota_f11689` | `__dtor_static_unique_ptr@1838312` | gerado pelo compilador (estáticos de cconferevotoemcargo.h) | cvotacaostateaudio.cpp | compilador / baixa |
| 11692 | 12 |  | `vota_f11692` | `__dtor_static_unique_ptr@1838284` | gerado pelo compilador (estáticos de cconferevotoemcargo.h) | cvotacaostateaudio.cpp | compilador / baixa |
| 11695 | 12 |  | `vota_f11695` | `__dtor_static_unique_ptr@1838256` | gerado pelo compilador (estáticos de cconferevotoemcargo.h) | cvotacaostateaudio.cpp | compilador / baixa |
| 11698 | 12 |  | `vota_f11698` | `__dtor_static_unique_ptr@1838228` | gerado pelo compilador (estáticos de cconferevotoemcargo.h) | cvotacaostateaudio.cpp | compilador / baixa |
| 11703 | 12 |  | `vota_f11703` | `__dtor_static_unique_ptr@1838200` | gerado pelo compilador (estáticos de cconferevotoemcargo.h) | cvotacaostateaudio.cpp | compilador / baixa |
| 11706 | 12 |  | `vota_f11706` | `__dtor_static_unique_ptr@1838172` | gerado pelo compilador (estáticos de cconferevotoemcargo.h) | cvotacaostateaudio.cpp | compilador / baixa |
| 11714 | 12 |  | `vota_f11714` | `__dtor_static_unique_ptr@1838108` | gerado pelo compilador (estáticos de celeitorvotando.cpp) | cvotacaostateaudio.cpp | compilador / baixa |
| 11730 | 12 |  | `vota_f11730` | `__dtor_static_unique_ptr@1837944` | gerado pelo compilador (estáticos de cconferevotoemcargo.h) | cvotacaostateaudio.cpp | compilador / baixa |
| 11734 | 12 |  | `vota_f11734` | `__dtor_static_unique_ptr@1837916` | gerado pelo compilador (estáticos de cconferevotoemcargo.h) | cvotacaostateaudio.cpp | compilador / baixa |
| 11737 | 12 |  | `vota_f11737` | `__dtor_static_unique_ptr@1837888` | gerado pelo compilador (estáticos de cconferevotoemcargo.h) | cvotacaostateaudio.cpp | compilador / baixa |
| 11754 | 653 | sim | `vota::CVotacaoStateAudio::EmiteEcoCorrigeConfirma` | `vota::CConfirmaVotoEmCargo::ProcessInputAudio` | uenux2/src/app/vota/eleitor/cconfirmavotoemcargo.cpp | cvotacaostateaudio.cpp | TSE / alta |
| 12039 | 422 |  | `comum_f12039` | `comum::ConteudoQRCodeCertificado` | uenux2/src/app/comum/relatorios/cgeradorbu.cpp | fimvotacao/cgerabu.cpp | TSE / baixa |
| 12081 | 1543 |  | `vota::CEmitirMaisBU::ProcessInput` | `vota::CEmitirMaisBU::ProcessInput` | uenux2/src/app/vota/eleitor/fimvotacao/cemitirmaisbu.cpp | fimvotacao/cemitirmaisbu.cpp | TSE / alta |
| 12082 | 250 |  | `vota::CEmitirMaisBU::StartState` | `vota::CEmitirMaisBU::StartState` | uenux2/src/app/vota/eleitor/fimvotacao/cemitirmaisbu.cpp | fimvotacao/cemitirmaisbu.cpp | TSE / alta |
| 12105 | 18808 |  | `vota::CGeraRelatorios::StartState` | `vota::CGeraRelatorios::StartState` | uenux2/src/app/vota/eleitor/fimvotacao/cgerarelatorios.cpp | fimvotacao/cgerarelatorios.cpp | TSE / alta |
| 12110 | 17349 |  | `vota::CGeraBU::vf2` | `vota::CGeraBU::StartState` | uenux2/src/app/vota/eleitor/fimvotacao/cgerabu.cpp | fimvotacao/cgerabu.cpp | TSE / alta |
| 12134 | 5966 |  | `vota::CCopiaResultadoParaMR::CopiaResultado` | `vota::CCopiaResultadoParaMR::StartState` | uenux2/src/app/vota/eleitor/fimvotacao/ccopiaresultadoparamr.cpp | fimvotacao/ccopiaresultadoparamr.cpp | TSE / alta |
| 13424 | 90 |  | `vota_f13424` | `vota::(anonymous namespace)::TextoStatusAudio` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | cvotacaostateaudio.cpp | TSE / baixa |
